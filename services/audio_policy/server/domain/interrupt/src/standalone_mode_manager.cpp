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
#define LOG_TAG "StandaloneModeManager"
#endif

#include "standalone_mode_manager.h"
#include "audio_log.h"
#include "audio_session_info.h"
#include "audio_bundle_manager.h"
#include "window_manager_lite.h"
#include "audio_volume.h"
#include "audio_interrupt_service.h"

namespace OHOS {
namespace AudioStandard {

std::mutex StandaloneModeManager::instanceMutex;
StandaloneModeManager* StandaloneModeManager::instance;

StandaloneModeManager &StandaloneModeManager::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex);
    if (instance == nullptr) {
        instance = new StandaloneModeManager();
    }
    return *instance;
}

void StandaloneModeManager::Init(std::shared_ptr<AudioInterruptService> interruptService)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(interruptService != nullptr,
        "interruptService is nullptr");
    interruptService_ = interruptService;
}

StandaloneModeManager::~StandaloneModeManager()
{
    CleanAllStandaloneInfo();
}

int32_t StandaloneModeManager::SetAppSilentOnDisplay(const int32_t ownerPid, const int32_t displayId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!CheckOwnerPidPermissions(ownerPid)) {
        return -1;
    }
    isSetSilentDisplay_ = displayId > 0 ? true : false;
    displayId_ = isSetSilentDisplay_ ? displayId : INVALID_ID;
    if (displayId == -1) {
        ResumeAllStandaloneApp(ownerPid);
    }
    return 0;
}

bool StandaloneModeManager::CheckOwnerPidPermissions(const int32_t ownerPid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (ownerPid_ == INVALID_ID) {
        ownerPid_ = ownerPid;
        AUDIO_INFO_LOG("Init Owner Pid is %{public}d", ownerPid);
    } else if (ownerPid != ownerPid_) {
        AUDIO_ERR_LOG("currentAppPid %{public}d ownerPid %{public}d ", ownerPid, ownerPid_);
        return false;
    }
    return true;
}

void StandaloneModeManager::ExitStandaloneAndResumeFocus(const int32_t appUid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (interruptService_ == nullptr) {
        return;
    }
    if (activeZoneSessionsMap_.find(appUid) == activeZoneSessionsMap_.end()) {
        AUDIO_ERR_LOG("Exit Standalone Focus Not Find");
        return;
    }

    auto standaloneAppSessionsList = activeZoneSessionsMap_[appUid];
    for (auto &sessionId : standaloneAppSessionsList) {
        InterruptEventInternal interruptEventResume {INTERRUPT_TYPE_BEGIN,
            INTERRUPT_SHARE, INTERRUPT_HINT_EXIT_STANDALONE, 1.0f};
        interruptService_->ResumeFocusByStreamId(sessionId, interruptEventResume);
    }
    activeZoneSessionsMap_.erase(appUid);
}

void StandaloneModeManager::ResumeAllStandaloneApp(const int32_t appPid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (appPid != ownerPid_) {
        return;
    }
    AUDIO_INFO_LOG("Begin Resume All Standalone App");
    if (!activeZoneSessionsMap_.empty()) {
        auto tempActiveZoneSessionsMap = activeZoneSessionsMap_;
        for (auto &[appUid, _] : tempActiveZoneSessionsMap) {
            ExitStandaloneAndResumeFocus(appUid);
            AudioVolume::GetInstance()->SetAppVolumeMute(appUid, false);
        }
    }
    CleanAllStandaloneInfo();
}

void StandaloneModeManager::CleanAllStandaloneInfo()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    ownerPid_ = INVALID_ID;
    displayId_ = INVALID_ID;
    isSetSilentDisplay_ = false;
    activeZoneSessionsMap_.clear();
}

void StandaloneModeManager::RemoveExistingFocus(const int32_t appUid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (interruptService_ == nullptr) {
        return;
    }
    std::unordered_set<int32_t> uidActivedSessions = {};
    interruptService_->RemoveExistingFocus(appUid, uidActivedSessions);
    if (!uidActivedSessions.empty()) {
        for (auto sessionId : uidActivedSessions) {
            activeZoneSessionsMap_[appUid].insert(sessionId);
        }
    }
}

int32_t StandaloneModeManager::SetAppConcurrencyMode(const int32_t ownerPid,
    const int32_t appUid, const int32_t mode)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    AUDIO_INFO_LOG("Set App Concurrency Mode : ownerPid = %{public}d  Standalone"
        "appUid = %{public}d concurrencyMode = %{public}d", ownerPid, appUid, mode);
    if (!CheckOwnerPidPermissions(ownerPid)) {
        return -1;
    }
    AudioConcurrencyMode concurrencyMode = static_cast<AudioConcurrencyMode>(mode);
    switch (concurrencyMode) {
        case AudioConcurrencyMode::STANDALONE:
            RecordStandaloneAppSessionIdInfo(appUid);
            RemoveExistingFocus(appUid);
            break;
        case AudioConcurrencyMode::DEFAULT:
            ExitStandaloneAndResumeFocus(appUid);
            break;
        default:
            break;
    }
    return 0;
}

bool StandaloneModeManager::CheckAppOnVirtualScreenByUid(const int32_t appUid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::string bundleName = AudioBundleManager::GetBundleNameFromUid(appUid);
    if (bundleName.empty()) {
        AUDIO_ERR_LOG("Get BundleName From Uid Fail");
        return false;
    }
    OHOS::Rosen::WindowInfoOption windowInfoOption = {};
    windowInfoOption.displayId = displayId_;
    std::vector<sptr<OHOS::Rosen::WindowInfo>> ogInfos = {};
    auto ret = OHOS::Rosen::WindowManagerLite::GetInstance().ListWindowInfo(windowInfoOption, ogInfos);
    AUDIO_INFO_LOG("ListWindowIfo size is %{public}d, ret = %{public}d",
        static_cast<int>(ogInfos.size()), ret);
    for (auto &iter : ogInfos) {
        if (iter->windowMetaInfo.bundleName == bundleName) {
            AUDIO_INFO_LOG("Exist Standalone App On Virtual Screen ownerUid"
                " = %{public}d, bundleName = %{public}s", appUid, bundleName.c_str());
            return true;
        }
    }
    return false;
}

bool StandaloneModeManager::CheckAndRecordStandaloneApp(const int32_t appUid,
    const bool isOnlyRecordUid, const int32_t sessionId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (ownerPid_ == INVALID_ID && !isSetSilentDisplay_) {
        return false;
    }
    if (activeZoneSessionsMap_.find(appUid) != activeZoneSessionsMap_.end()) {
        RecordStandaloneAppSessionIdInfo(appUid, isOnlyRecordUid, sessionId);
        return true;
    }
    if (isSetSilentDisplay_ && CheckAppOnVirtualScreenByUid(appUid)) {
        RecordStandaloneAppSessionIdInfo(appUid, isOnlyRecordUid, sessionId);
        AudioVolume::GetInstance()->SetAppVolumeMute(appUid, true);
        return true;
    }
    return false;
}

void StandaloneModeManager::RecordStandaloneAppSessionIdInfo(const int32_t appUid,
    const bool isOnlyRecordUid, const int32_t sessionId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (isOnlyRecordUid) {
        std::unordered_set<int32_t> sessionIdInfoMap = {};
        activeZoneSessionsMap_[appUid] = std::move(sessionIdInfoMap);
        return;
    }
    activeZoneSessionsMap_[appUid].insert(sessionId);
}

void StandaloneModeManager::EraseDeactivateAudioStream(const int32_t appUid,
    const int32_t sessionId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (activeZoneSessionsMap_.find(appUid) == activeZoneSessionsMap_.end()) {
            return;
    }
    if (activeZoneSessionsMap_[appUid].empty()) {
        return;
    }
    activeZoneSessionsMap_[appUid].erase(sessionId);
}

} // namespace AudioStandard
} // namespace OHOS