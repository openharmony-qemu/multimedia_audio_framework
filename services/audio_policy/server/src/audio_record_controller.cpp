/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRecordController"
#endif

#include "audio_record_controller.h"
#include <algorithm>
#include "audio_policy_server_handler.h"
#include "audio_manager_log.h"
#include "audio_background_manager.h"
#include "singleton.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint8_t PRIORITY_SOURCE_TYPE_MIC = 1;
constexpr uint8_t PRIORITY_SOURCE_TYPE_CAMCORDER = PRIORITY_SOURCE_TYPE_MIC + 1;
constexpr uint8_t PRIORITY_SOURCE_TYPE_LIVE = PRIORITY_SOURCE_TYPE_CAMCORDER + 1;

static uint8_t GetPriority(uint32_t sessionId, SourceType sourceType)
{
    if (sessionId == 0) {
        return 0;
    }
    switch (sourceType) {
        case SOURCE_TYPE_LIVE:
            return PRIORITY_SOURCE_TYPE_LIVE;
        case SOURCE_TYPE_CAMCORDER:
            return PRIORITY_SOURCE_TYPE_CAMCORDER;
        case SOURCE_TYPE_MIC:
            return PRIORITY_SOURCE_TYPE_MIC;
        default:
            return 0;
    }
}

AudioRecordController &AudioRecordController::GetInstance()
{
    static AudioRecordController instance;
    return instance;
}

int32_t AudioRecordController::EnableSystemRecordController(bool show,
    const SystemRecordControllerConfig &config, int32_t uid, int32_t pid)
{
    AUDIO_INFO_LOG("show[%{public}d] uid[%{public}d] pid[%{public}d]", show, uid, pid);
    if (!IsValidSourceType(config.sourceType)) {
        AUDIO_ERR_LOG("invalid sourceType[%{public}d]", config.sourceType);
        return ERR_INVALID_PARAM;
    }
    bool isForeground = show ? !AudioBackgroundManager::GetInstance().IsAppInBackState(pid) : false;
    std::lock_guard<std::mutex> lock(mutex_);
    if (show) {
        StartMonitoringApp(uid, pid, 0, isForeground, config.sourceType);
    } else {
        StopMonitoringApp(uid, pid, 0, config.sourceType);
    }
    NotifyEnabledChange();
    return SUCCESS;
}

void AudioRecordController::OnRecordStart(uint32_t sessionId, int32_t uid, int32_t pid, SourceType sourceType)
{
    AUDIO_INFO_LOG("sessionId[%{public}u]", sessionId);
    if (!IsValidSourceType(sourceType)) {
        AUDIO_DEBUG_LOG("sourceType[%{public}d] is not record type", sourceType);
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    StartMonitoringApp(uid, pid, sessionId, true, sourceType);
    NotifyEnabledChange();
}

void AudioRecordController::OnRecordStop(uint32_t sessionId, int32_t uid, int32_t pid, SourceType sourceType)
{
    AUDIO_INFO_LOG("sessionId[%{public}u]", sessionId);
    if (!IsValidSourceType(sourceType)) {
        AUDIO_DEBUG_LOG("sourceType[%{public}d] is not record type", sourceType);
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    StopMonitoringApp(uid, pid, sessionId, sourceType);
    NotifyEnabledChange();
}

void AudioRecordController::OnAppStateChanged(int32_t uid, int32_t pid, bool isForeground)
{
    AUDIO_INFO_LOG("uid[%{public}d] pid[%{public}d] isForeground[%{public}d]", uid, pid, isForeground);
    bool monitored = false;
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : monitoredApps_) {
        if (item.uid == uid && item.pid == pid) {
            item.isForeground = isForeground;
            monitored = true;
        }
    }
    if (monitored) {
        NotifyEnabledChange();
    }
}

void AudioRecordController::OnAppDied(int32_t uid, int32_t pid)
{
    AUDIO_INFO_LOG("uid[%{public}d] pid[%{public}d]", uid, pid);
    auto pred = [uid, pid](const MonitoredAppInfo &item) {
        return item.uid == uid && item.pid == pid;
    };
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = remove_if(monitoredApps_.begin(), monitoredApps_.end(), pred);
    if (it != monitoredApps_.end()) {
        monitoredApps_.erase(it, monitoredApps_.end());
        NotifyEnabledChange();
    }
}

void AudioRecordController::NotifyEnabledChange()
{
    auto pred = [](const MonitoredAppInfo &item) {
        return item.sessionId > 0 || item.isForeground;
    };
    auto rIt = std::find_if(monitoredApps_.rbegin(), monitoredApps_.rend(), pred);
    SystemRecordControllerChangeInfo changeInfo;
    if (rIt != monitoredApps_.rend()) {
        changeInfo.enabled = true;
        changeInfo.uid = rIt->uid;
        changeInfo.sourceType = rIt->sourceType;
    } else {
        changeInfo.enabled = false;
        changeInfo.uid = lastChangeInfo_.uid;
        changeInfo.sourceType = lastChangeInfo_.sourceType;
    }
    if (changeInfo == lastChangeInfo_) {
        AUDIO_DEBUG_LOG("duplicate changeInfo, skip");
        return;
    }
    lastChangeInfo_ = changeInfo;
    auto handler = DelayedSingleton<AudioPolicyServerHandler>::GetInstance();
    if (handler != nullptr) {
        handler->SendRecordControllerEnabledChangeCallback(changeInfo);
    }
}

bool AudioRecordController::IsValidSourceType(SourceType sourceType)
{
    return sourceType == SOURCE_TYPE_MIC || sourceType == SOURCE_TYPE_CAMCORDER || sourceType == SOURCE_TYPE_LIVE;
}

void AudioRecordController::StartMonitoringApp(
    int32_t uid, int32_t pid, uint32_t sessionId, bool isForeground, SourceType sourceType)
{
    StopMonitoringApp(uid, pid, sessionId, sourceType);
    auto it = monitoredApps_.begin();
    auto priority = GetPriority(sessionId, sourceType);
    for (; it != monitoredApps_.end(); it++) {
        if (GetPriority(it->sessionId, it->sourceType) > priority) {
            break;
        }
    }
    monitoredApps_.insert(it, {uid, pid, sessionId, isForeground, sourceType});
    AUDIO_DEBUG_LOG("uid[%{public}d] pid[%{public}d] sourceType[%{public}d]", uid, pid, sourceType);
}

void AudioRecordController::StopMonitoringApp(int32_t uid, int32_t pid, uint32_t sessionId, SourceType sourceType)
{
    auto pred = [uid, pid, sessionId, sourceType](const MonitoredAppInfo &item) {
        return item.uid == uid && item.pid == pid && item.sessionId == sessionId && item.sourceType == sourceType;
    };
    monitoredApps_.erase(remove_if(monitoredApps_.begin(), monitoredApps_.end(), pred), monitoredApps_.end());
    AUDIO_DEBUG_LOG("uid[%{public}d] pid[%{public}d] sourceType[%{public}d]", uid, pid, sourceType);
}

} // namespace AudioStandard
} // namespace OHOS