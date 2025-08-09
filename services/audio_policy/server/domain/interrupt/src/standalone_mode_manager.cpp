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
#include "window_manager.h"
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
    isSetlientDisplay_ = displayId > 0 ? true : false;
    displayId_ = isSetlientDisplay_ ? displayId : INVALID_ID;
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

    auto uidActivedSessions = activedZoneSessionsMap_[appUid];
    for (auto [_, standaloneAppSessionsList] : uidActivedSessions) {
        for (auto &sessionId : standaloneAppSessionsList) {
            InterruptEventInternal interruptEventResume {INTERRUPT_TYPE_BEGIN,
                INTERRUPT_SHARE, INTERRUPT_HINT_EXIT_STANDALONE, 1.0f};
            interruptService_->ResumeFocusByStreamId(sessionId, interruptEventResume);
        }
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
            ExistStandaloneAndResumeFocus(appUid);
            AudioVolume::GetInstance()->SetAppVolumeMute(appUid, false);
        }
    }
    CleanAllStandaloneInfo();
}

void StandaloneModeManager::CleanAllStandaloneInfo()
{
    ownerPid_ = INVALID_ID;
    displayId_ = INVALID_ID;
    isSetlientDisplay_ = false;
    activedZoneSessionsMap_.clear();
}

void StandaloneModeManager::RemoveExistingFocus(const int32_t appUid)
{
    if (interruptService_ == nullptr) {
        return;
    }
    std::unordered_map<int32_t, std::unordered_set<int32_t>> uidActivedSession = {};
    interruptService_->RemoveExistingFocus(appUid, uidActivedSession);
    activedZoneSessionsMap_[appUid] = uidActivedSession;
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
    case AudioConcurrencyMode::
        isSetlientDisplay_ = true;
        ownerPid_ = ownerPid;
        RemoveExistingFocus(appUid);
    case AudioConcurrencyMode::
        locked_ = false;
        ExitStandaloneAndResumeFocus(appUid);
    default:
        break;
    }
    return SUCCESS;
}







StandaloneModeManager::
StandaloneModeManager::
StandaloneModeManager::
        void 
    void 

} // namespace AudioStandard
} // namespace OHOS