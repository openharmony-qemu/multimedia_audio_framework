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

StandaloneModeManager &StandaloneModeManager::GetInstance()
{
    static StandaloneModeManager standaloneModeManager;
    return standaloneModeManager;
}

void StandaloneModeManager::InIt(std::shared_ptr<AudioInterruptService> interruptService)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(interruptService != nullptr,
        "interruptService is nullptr");
    interruptService_ = interruptService;
}

StandaloneModeManager::~StandaloneModeManager()
{
    CleanAllStandaloneInfo();
}

int32_t StandaloneModeManager::SetAppSlientOnDisplay(const int32_t ownerPid, const int32_t displayId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!CheckOwnerPidPermissions(ownerPid)) {
        return -1;
    }
    isSetSlientDisplay_ = displayId > 0 ? true : false;
    displayId_ = isSetSlientDisplay_ ? displayId : INVALID_ID;
    return 0;
}

bool StandaloneModeManager::CheckOwnerPidPermissions(const int32_t ownerPid)
{
    if (ownerPid_ == INVALID_ID) {
        ownerPid_ = ownerPid;
        AUDIO_INFO_LOG("Init Owner Pid is %{public}d", ownerPid);
    } else if (ownerPid != ownerPid_) {
        AUDIO_ERR_LOG("Access Not Allowed");
        return false;
    }
    return true;
}

void StandaloneModeManager::ExitStandaloneAndResumeFocus(const int32_t appUid)
{
    if (interruptService_ == nullptr) {
        return;
    }
    if (activedZoneSessionsMap_.find(appUid) == activedZoneSessionsMap_.end()) {
        AUDIO_ERR_LOG("Exit Standalone Focus Not Find");
        return;
    }

    auto standaloneAppSessionsList = activedZoneSessionsMap_[appUid];
    for (auto &sessionId : standaloneAppSessionsList) {
        InterruptEventInternal interruptEventResume {INTERRUPT_TYPE_BEGIN,
            INTERRUPT_SHARE, INTERRUPT_HINT_EXIT_STANDALONE, 1.0f};
        interruptService_->ResumeFocusByStreamId(sessionId, interruptEventResume);
    }
    activedZoneSessionsMap_.erase(appUid);
}

void StandaloneModeManager::ResumeAllStandaloneApp(const int32_t appPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (appPid != ownerPid_) {
        return;
    }
    AUDIO_INFO_LOG("Begin Resume All Standalone App");
    if (!activedZoneSessionsMap_.empty()) {
        for (auto &[appUid, _] : activedZoneSessionsMap_) {
            ExitStandaloneAndResumeFocus(appUid);
            AudioVolume::GetInstance()->SetAppVolumeMute(appUid, false);
        }
    }
    CleanAllStandaloneInfo();
}

void StandaloneModeManager::CleanAllStandaloneInfo()
{
    ownerPid_ = INVALID_ID;
    displayId_ = INVALID_ID;
    isSetSlientDisplay_ = false;
    activedZoneSessionsMap_.clear();
}

void StandaloneModeManager::RemoveExistingFocus(const int32_t appUid)
{
    if (interruptService_ == nullptr) {
        return;
    }
    std::unordered_set<int32_t> uidActivedSessions = {};
    interruptService_->RemoveExistingFocus(appUid, uidActivedSessions);
    if (!uidActivedSessions.empty()) {
        for (auto sessionId : uidActivedSessions) {
            activedZoneSessionsMap_[appUid].insert(sessionId);
        }
    }
}

int32_t StandaloneModeManager::SetAppConcurrencyMode(const int32_t ownerPid,
    const int32_t appUid, const int32_t mode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("Set App Concurrency Mode : ownerPid = %{public}d  Standalone"
        "appPid = %{public}d concurrencyMode = %{public}d", ownerPid, appUid, mode);
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
    return SUCCESS;
}

bool StandaloneModeManager::CheckAppOnVirtualScreenByUid(const int32_t appUid)
{
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
    std::lock_guard<std::mutex> lock(mutex_);
    if (ownerPid_ == INVALID_ID && !isSetSlientDisplay_) {
        AUDIO_ERR_LOG("Standalone Mode Not Activation");
        return false;
    }
    if (activedZoneSessionsMap_.find(appUid) != activedZoneSessionsMap_.end()) {
        RecordStandaloneAppSessionIdInfo(appUid, isOnlyRecordUid, sessionId);
        return true;
    }
    if (isSetSlientDisplay_ && CheckAppOnVirtualScreenByUid(appUid)) {
        RecordStandaloneAppSessionIdInfo(appUid, isOnlyRecordUid, sessionId);
        AudioVolume::GetInstance()->SetAppVolumeMute(appUid, true);
        return true;
    }
    return false;
}

void StandaloneModeManager::RecordStandaloneAppSessionIdInfo(const int32_t appUid,
    const bool isOnlyRecordUid, const int32_t sessionId)
{
    if (isOnlyRecordUid) {
        std::unordered_set<int32_t> sessionIdInfoMap = {};
        activedZoneSessionsMap_[appUid] = std::move(sessionIdInfoMap);
        return;
    }
    activedZoneSessionsMap_[appUid].insert(sessionId);
}

void StandaloneModeManager::EraseDeactivateAudioStream(const int32_t &appUid,
    const int32_t &sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (activedZoneSessionsMap_.find(appUid) == activedZoneSessionsMap_.end()) {
            return;
    }
    if (activedZoneSessionsMap_[appUid].empty()) {
        return;
    }
    activedZoneSessionsMap_[appUid].erase(sessionId);
}

} // namespace AudioStandard
} // namespace OHOS