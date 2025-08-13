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
        InsertIntoAppStatesMap(pid, appState);
    }

    AppState &appState = appStatesMap_[pid];
    AUDIO_INFO_LOG("appStatesMap_ start pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
        "hasBackgroundTask: %{public}d, isFreeze: %{public}d", pid, appState.hasSession, appState.isBack,
        appState.hasBackTask, appState.isFreeze);
    if (appState.isBack) {
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
            InsertIntoAppStatesMap(pid, appState);
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
            "hasBackgroundTask: %{public}d, isFreeze: %{public}d", pid, appState.hasSession, appState.isBack,
            appState.hasBackTask, appState.isFreeze);
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
        InsertIntoAppStatesMap(pid, appState);
    } else {
        AppState &appState = appStatesMap_[pid];
        CHECK_AND_RETURN(appState.hasBackTask != hasBackgroundTask);
        appState.hasBackTask = hasBackgroundTask;
        AUDIO_INFO_LOG("appStatesMap_ change pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
            "hasBackgroundTask: %{public}d, isFreeze: %{public}d", pid, appState.hasSession, appState.isBack,
            appState.hasBackTask, appState.isFreeze);
        if (appState.hasBackTask && !appState.isFreeze) {
            streamCollector_.HandleBackTaskStateChange(uid, appState.hasSession);
        }
    }
}

int32_t AudioBackgroundManager::NotifySessionStateChange(const int32_t uid, const int32_t pid, const bool hasSession)
{
    std::lock_guard<std::mutex> lock(appStatesMapMutex_);
    if (!FindKeyInMap(pid)) {
        AppState appState;
        appState.hasSession = hasSession;
        appState.isSystem = CheckoutSystemAppUtil::CheckoutSystemApp(uid);
        InsertIntoAppStatesMap(pid, appState);
    } else {
        AppState &appState = appStatesMap_[pid];
        CHECK_AND_RETURN_RET(appState.hasSession != hasSession, SUCCESS);
        appState.hasSession = hasSession;
        AUDIO_INFO_LOG("appStatesMap_ change pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
            "hasBackgroundTask: %{public}d, isFreeze: %{public}d", pid, appState.hasSession, appState.isBack,
            appState.hasBackTask, appState.isFreeze);
        HandleSessionStateChange(uid, pid);
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
            InsertIntoAppStatesMap(pid, appState);
        } else {
            AppState &appState = appStatesMap_[pid];
            CHECK_AND_RETURN_RET(appState.isFreeze != isFreeze, SUCCESS);
            appState.isFreeze = isFreeze;
            appState.isBinder = !isFreeze;
            AUDIO_INFO_LOG("appStatesMap_ change pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
                "hasBackgroundTask: %{public}d, isFreeze: %{public}d", pid, appState.hasSession, appState.isBack,
                appState.hasBackTask, appState.isFreeze);
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
    if (isFreeze) {
        if (!appState.hasBackTask) {
            streamCollector_.HandleFreezeStateChange(pid, true, appState.hasSession);
        }
    } else {
        if (appState.hasBackTask) {
            streamCollector_.HandleFreezeStateChange(pid, false, appState.hasSession);
        }
    }
}

void AudioBackgroundManager::InsertIntoAppStatesMap(int32_t pid, AppState appState)
{
    appStatesMap_.insert(std::make_pair(pid, appState));
    AUDIO_INFO_LOG("appStatesMap_ add pid: %{public}d with hasSession: %{public}d, isBack: %{public}d, "
        "hasBackgroundTask: %{public}d, isFreeze: %{public}d", pid, appState.hasSession, appState.isBack,
        appState.hasBackTask, appState.isFreeze);
}

void AudioBackgroundManager::DeleteFromMap(int32_t pid)
{
    if (FindKeyInMap(pid)) {
        appStatesMap_.erase(pid);
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
