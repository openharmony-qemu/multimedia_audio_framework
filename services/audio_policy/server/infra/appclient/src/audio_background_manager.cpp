/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef LOG_TAG
#define LOG_TAG "AudioBackgroundManager"
#endif

#include "audio_background_manager.h"
#include "audio_policy_log.h"
#include "audio_log.h"
#include "audio_policy_utils.h"
#include "audio_inner_call.h"
#include "i_policy_provider.h"

#include "audio_server_proxy.h"
#include "continuous_task_callback_info.h"
#include "background_task_listener.h"
#include "background_task_subscriber.h"
#include "background_task_mgr_helper.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {

constexpr int32_t BOOTUP_MUSIC_UID = 1003;
static const int64_t WATI_PLAYBACK_TIME = 200000; // 200ms
mutex g_isAllowedPlaybackListenerMutex;
mutex g_backgroundMuteListenerMutex;

int32_t AudioBackgroundManager::SetQueryAllowedPlaybackCallback(const sptr<IRemoteObject> &object)
{
    lock_guard<mutex> lock(g_isAllowedPlaybackListenerMutex);
    isAllowedPlaybackListener_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    return SUCCESS;
}

int32_t AudioBackgroundManager::SetBackgroundMuteCallback(const sptr<IRemoteObject> &object)
{
    lock_guard<mutex> lock(g_backgroundMuteListenerMutex);
    backgroundMuteListener_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    return SUCCESS;
}

void AudioBackgroundManager::SubscribeBackgroundTask()
{
    AUDIO_INFO_LOG("in");
    if (backgroundTaskListener_ == nullptr) {
        backgroundTaskListener_ = std::make_shared<BackgroundTaskListener>();
    }
    auto ret = BackgroundTaskMgr::BackgroundTaskMgrHelper::SubscribeBackgroundTask(*backgroundTaskListener_);
    if (ret != 0) {
        AUDIO_INFO_LOG(" failed, err:%{public}d", ret);
    }
}

bool AudioBackgroundManager::IsAllowedPlayback(const int32_t &uid, const int32_t &pid)
{
    std::lock_guard<std::mutex> lock(appStatesMapMutex_);
    if (!FindKeyInMap(pid)) {
        AppState appState;
        appState.isSystem = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
        InsertIntoAppStatesMap(pid, uid, appState);
    }

    AppState &appState = appStatesMap_[pid];
    AUDIO_INFO_LOG("appStatesMap_ start pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
        "hasBackgroundTask: %{public}d, isFreeze: %{public}d, isSystem: %{public}d", pid, appState.hasSession,
        appState.isBack, appState.hasBackTask, appState.isFreeze, appState.isSystem);
    if (appState.isBack && !appState.isSystem) {
        bool mute = appState.hasBackTask ? false : (appState.isBinder ? true : false);
        if (!appState.hasSession) {
            // for media
            HandleSessionStateChange(uid, pid);
            // for others
            streamCollector_.HandleStartStreamMuteState(uid, pid, mute, true);
        } else {
            streamCollector_.HandleStartStreamMuteState(uid, pid, mute, false);
        }
    } else {
        streamCollector_.HandleStartStreamMuteState(uid, pid, false, false);
    }
    return true;
}

void AudioBackgroundManager::NotifyAppStateChange(const int32_t uid, const int32_t pid, AppIsBackState state)
{
    bool isBack = (state != STATE_FOREGROUND);
    {
        std::lock_guard<std::mutex> lock(appStatesMapMutex_);
        if (state == STATE_END) {
            return DeleteFromMap(pid);
        }
        if (!FindKeyInMap(pid)) {
            AppState appState;
            appState.isBack = isBack;
            appState.isSystem = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
            InsertIntoAppStatesMap(pid, uid, appState);
        }
        return;
    }

    bool notifyMute = false;
    {
        std::lock_guard<std::mutex> lock(appStatesMapMutex_);
        AppState &appState = appStatesMap_[pid];
        CHECK_AND_RETURN(appState.isBack != isBack);
        appState.isBack = isBack;
        appState.isFreeze = isBack ? appState.isFreeze : false;
        appState.isBinder = isBack ? appState.isBinder : false;
        AUDIO_INFO_LOG("appStatesMap_ change pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
            "hasBackgroundTask: %{public}d, isFreeze: %{public}d, isSystem: %{public}d", pid, appState.hasSession,
            appState.isBack, appState.hasBackTask, appState.isFreeze, appState.isSystem);
        if (!isBack) {
            return streamCollector_.HandleForegroundUnmute(uid, pid);
        }
        bool needMute = !appState.hasSession && appState.isBack && !CheckoutSystemAppUtil::CheckoutSystemApp(uid);
        streamCollector_.HandleAppStateChange(uid, pid, needMute, notifyMute, appState.hasBackTask);
        streamCollector_.HandleKaraokeAppToBack(uid, pid);
    }
    if (notifyMute && !VolumeUtils::IsPCVolumeEnable()) {
        lock_guard<mutex> lock(g_backgroundMuteListenerMutex);
        CHECK_AND_RETURN_LOG(backgroundMuteListener_ != nullptr, "backgroundMuteListener_ is nulptr");
        AUDIO_INFO_LOG("OnBackground with uid: %{public}d", uid);
        backgroundMuteListener_->OnBackgroundMute(uid);
    }
}

void AudioBackgroundManager::NotifyBackgroundTaskStateChange(const int32_t uid, const int32_t pid, bool hasBackgroundTask)
{
    std::lock_guard<std::mutex> lock(appStatesMapMutex_);
    if (!FindKeyInMap(pid)) {
        AppState appState;
        appState.hasBackTask = hasBackgroundTask;
        appState.isSystem = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
        InsertIntoAppStatesMap(pid, uid, appState);
        WriteAppStateChangeSysEvent(pid, appStatesMap_[pid], true);
    } else {
        AppState &appState = appStatesMap_[pid];
        CHECK_AND_RETURN(appState.hasBackTask != hasBackgroundTask);
        appState.hasBackTask = hasBackgroundTask;
        AUDIO_INFO_LOG("appStatesMap_ change pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
            "hasBackgroundTask: %{public}d, isFreeze: %{public}d, isSystem: %{public}d", pid, appState.hasSession,
            appState.isBack, appState.hasBackTask, appState.isFreeze, appState.isSystem);
        if (appState.hasBackTask && !appState.isFreeze) {
            streamCollector_.HandleBackTaskStateChange(uid, appState.hasSession);
        }
        WriteAppStateChangeSysEvent(pid, appStatesMap_[pid], true);
    }
}

int32_t AudioBackgroundManager::NotifySessionStateChange(const int32_t uid, const int32_t pid, const bool hasSession)
{
    std::lock_guard<std::mutex> lock(appStatesMapMutex_);
    if (!FindKeyInMap(pid)) {
        AppState appState;
        appState.hasSession = hasSession;
        appState.isSystem = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
        InsertIntoAppStatesMap(pid, uid, appState);
        WriteAppStateChangeSysEvent(pid, appStatesMap_[pid], true);
    } else {
        AppState &appState = appStatesMap_[pid];
        CHECK_AND_RETURN_RET(appState.hasSession != hasSession, SUCCESS);
        appState.hasSession = hasSession;
        AUDIO_INFO_LOG("appStatesMap_ change pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
            "hasBackgroundTask: %{public}d, isFreeze: %{public}d, isSystem: %{public}d", pid, appState.hasSession,
            appState.isBack, appState.hasBackTask, appState.isFreeze, appState.isSystem);
        HandleSessionStateChange(uid, pid);
        WriteAppStateChangeSysEvent(pid, appStatesMap_[pid], true);
    }
    return SUCCESS;
}

void AudioBackgroundManager::HandleSessionStateChange(const int32_t uid, const int32_t pid)
{
    auto it = appStatesMap_.find(pid);
    AppState &appState = (it != appStatesMap_.end()) ? it->second : appStatesMap_[pid];
    if (it == appStatesMap_.end()) {
        appState.isSystem = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
    }
    bool needMute = !appState.hasSession && appState.isBack && !appState.isSystem;
    bool notifyMute = false;
    streamCollector_.HandleAppStateChange(uid, pid, needMute, notifyMute, appState.hasBackTask);
    if (notifyMute && !VolumeUtils::IsPCVolumeEnable()) {
        lock_guard<mutex> lock(g_backgroundMuteListenerMutex);
        CHECK_AND_RETURN_LOG(backgroundMuteListener_ != nullptr, "backgroundMuteListener_ is nulptr");
        AUDIO_INFO_LOG("OnBackground with uid: %{public}d", uid);
        backgroundMuteListener_->OnBackgroundMute(uid);
    }
}

int32_t AudioBackgroundManager::NotifyFreezeStateChange(const std::set<int32_t> &pidList, const bool isFreeze)
{
    std::lock_guard<std::mutex> lock(appStatesMapMutex_);
    for (auto pid : pidList) {
        if (!FindKeyInMap(pid)) {
            AppState appState;
            appState.isFreeze = isFreeze;
            InsertIntoAppStatesMapWithoutUid(pid, appState);
        } else {
            AppState &appState = appStatesMap_[pid];
            CHECK_AND_RETURN_RET(appState.isFreeze != isFreeze, SUCCESS);
            appState.isFreeze = isFreeze;
            appState.isBinder = !isFreeze;
            AUDIO_INFO_LOG("appStatesMap_ change pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
                "hasBackgroundTask: %{public}d, isFreeze: %{public}d, isSystem: %{public}d", pid, appState.hasSession,
                appState.isBack, appState.hasBackTask, appState.isFreeze, appState.isSystem);
            HandleFreezeStateChange(pid, isFreeze);
        }
    }
    return SUCCESS;
}

int32_t AudioBackgroundManager::ResetAllProxy()
{
    AUDIO_INFO_LOG("RSS reset all proxy to unfreeze");
    std::lock_guard<std::mutex> lock(appStatesMapMutex_);
    for (auto& it : appStatesMap_) {
        it.second.isFreeze = false;
        it.second.isBinder = false;
    }
    return SUCCESS;
}

void AudioBackgroundManager::HandleFreezeStateChange(const int32_t pid, bool isFreeze)
{
    AppState& appState = appStatesMap_[pid];
    if (isFreeze && !appState.isSystem) {
        if (!appState.hasBackTask) {
            streamCollector_.HandleFreezeStateChange(pid, true, appState.hasSession);
        }
    } else {
        if (appState.hasBackTask) {
            streamCollector_.HandleFreezeStateChange(pid, false, appState.hasSession);
        }
    }
}

void AudioBackgroundManager::WriteAppStateChangeSysEvent(int32_t pid, AppState appState, bool isAdd)
{
    AUDIO_INFO_LOG("pid %{public}d is add %{public}d, isFreeze %{public}d, isBack %{public}d, hasSession %{public}d,"
        "hasBackTask %{public}d, isBinder %{public}d", pid, isAdd, appState.isFreeze, appState.isBack,
        appState.hasSession, appState.hasBackTask, appState.isBinder);
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::APP_BACKGROUND_STATE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("PID", static_cast<int32_t>(pid));
    bean->Add("IS_FREEZE", static_cast<int32_t>(appState.isFreeze));
    bean->Add("IS_BACK", static_cast<int32_t>(appState.isBack));
    bean->Add("HAS_SESSION", static_cast<int32_t>(appState.hasSession));
    bean->Add("HAS_BACK_TASK", static_cast<int32_t>(appState.hasBackTask));
    bean->Add("IS_BINDER", static_cast<int32_t>(appState.isBinder));
    bean->Add("IS_ADD", isAdd);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioBackgroundManager::InsertIntoAppStatesMapWithoutUid(int32_t pid, AppState appState)
{
    int32_t uid = 0;
    std::string appBundleName = "";
    appMgrClient_->GetBundleNameByPid(pid, appBundleName, uid);
    appState.isSystem = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
    appStatesMap_.insert(std::make_pair(pid, appState));
    AUDIO_INFO_LOG("appStatesMap_ add pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
        "hasBackgroundTask: %{public}d, isFreeze: %{public}d, isSystem: %{public}d", pid, appState.hasSession,
        appState.isBack, appState.hasBackTask, appState.isFreeze, appState.isSystem);
}

void AudioBackgroundManager::InsertIntoAppStatesMap(int32_t pid, int32_t uid, AppState appState)
{
    appState.isSystem = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
    appStatesMap_.insert(std::make_pair(pid, appState));
    AUDIO_INFO_LOG("appStatesMap_ add pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
        "hasBackgroundTask: %{public}d, isFreeze: %{public}d, isSystem: %{public}d", pid, appState.hasSession,
        appState.isBack, appState.hasBackTask, appState.isFreeze, appState.isSystem);
}

void AudioBackgroundManager::RecoryAppState()
{
    std::lock_guard<std::mutex> lock(appStatesMapMutex_);
    AUDIO_INFO_LOG("Start recovery app state.");
    std::map<int32_t, std::shared_ptr<Media::MediaMonitor::MonitorAppStateInfo>> appStateMap;
    Media::MediaMonitor::MediaMonitorManager::GetInstance().GetAudioAppStateMsg(appStateMap);
    if (appStateMap.size() == 0) {
        AUDIO_INFO_LOG("the length of appStateMap is 0 and does not need to recory");
    } else {
        for (auto &appStateInfo : appStateMap) {
            std::shared_ptr<Media::MediaMonitor::MonitorAppStateInfo> info = appStateInfo.second;
            AppState appState;
            appState.isFreeze = info->isFreeze_;
            appState.isBack = info->isBack_;
            appState.hasSession = info->hasSession_;
            appState.hasBackTask = info->hasBackTask_;
            appState.isBinder = info->isBinder_;
            appStatesMap_.emplace(appStateInfo.first, appState);
            AUDIO_INFO_LOG("pid %{public}d, isFreeze %{public}d, isBack %{public}d,"
                "hasSession %{public}d, hasBackTask %{public}d, isBinder %{public}d", appStateInfo.first,
                appState.isFreeze, appState.isBack, appState.hasSession, appState.hasBackTask, appState.isBinder);
        }
    }
}

void AudioBackgroundManager::DeleteFromMap(int32_t pid)
{
    if (FindKeyInMap(pid)) {
        std::lock_guard<std::mutex> lock(appStatesMapMutex_);
        AppState appState = appStatesMap_[pid];
        appStatesMap_.erase(pid);
        WriteAppStateChangeSysEvent(pid, appState, false);
        AUDIO_INFO_LOG("Delete pid: %{public}d success.", pid);
    } else {
        AUDIO_DEBUG_LOG("Delete pid: %{public}d failed. It does nt exist", pid);
    }
}

bool AudioBackgroundManager::FindKeyInMap(int32_t pid)
{
    return appStatesMap_.find(pid) != appStatesMap_.end();
}
}
}
