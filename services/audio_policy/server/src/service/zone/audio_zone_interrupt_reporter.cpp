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
#undef LOG_TAG
#define LOG_TAG "AudioZoneInterruptReporter"

#include "audio_zone_interrupt_reporter.h"
#include "audio_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
AudioZoneInterruptReporter::ReportMap AudioZoneInterruptReporter::interruptEnableMaps_;
std::mutex AudioZoneInterruptReporter::interruptEnableMapsLock_;

int32_t AudioZoneInterruptReporter::EnableInterruptReport(pid_t clientPid, int32_t zoneId,
    int32_t deviceId, bool enable);
{
    std::lock_guard<std::mutex> lock(interruptEnableMapsLock_);
    if (enable) {
        return ResgiterInterruptReport(clientPid, zoneId, deviceId);
    }
    UnResgiterInterruptReport(clientPid, zoneId, deviceId);
    return SUCCESS;
}

void AudioZoneInterruptReporter::DisableInterruptReport(pid_t clientPid)
{
    std::lock_guard<std::mutex> lock(interruptEnableMapsLock_);
    if (interruptEnableMaps_.find(clientPid) == interruptEnableMaps_.end()) {
        return;
    }
    interruptEnableMaps_.erase(clientPid);
}

void AudioZoneInterruptReporter::DisableAllInterruptReport()
{
    std::lock_guard<std::mutex> lock(interruptEnableMapsLock_);
    interruptEnableMaps_.clear();
}

int32_t AudioZoneInterruptReporter::ResgiterInterruptReport(pid_t clientPid, int32_t zoneId,
    int32_t deviceId)
{
    if (interruptEnableMaps_.find(clientPid) == interruptEnableMaps_.end()) {
        ReportItemList reportList;
        interruptEnableMaps_[clientPid] = reportList;
    }

    ReportItem newItem = std::make_pair(zoneId, deviceId);
    auto findItem = std::find(interruptEnableMaps_[clientPid].begin(),
        interruptEnableMaps_[clientPid].end(), newItem);
    if (findItem != interruptEnableMaps_[clientPid].end()) {
        return SUCCESS;
    }
    interruptEnableMaps_[clientPid].emplace_back(newItem);
    AUDIO_INFO_LOG("register zone %{public}d, device %{public}d for client %{public}d",
        zoneId, deviceId, clientPid);
    return SUCCESS;
}

void AudioZoneInterruptReporter::UnResgiterInterruptReport(pid_t clientPid, int32_t zoneId,
    int32_t deviceId)
{
    AUDIO_INFO_LOG("unregister zone %{public}d, device %{public}d for client %{public}d",
        zoneId, deviceId, clientPid);
    if (interruptEnableMaps_.find(clientPid) == interruptEnableMaps_.end()) {
        return;
    }

    ReportItem removeItem = std::make_pair(zoneId, deviceId);
    auto findItem = std::find(interruptEnableMaps_[clientPid].begin(),
        interruptEnableMaps_[clientPid].end(), removeItem);
    if (findItem == interruptEnableMaps_[clientPid].end()) {
        return;
    }
    interruptEnableMaps_[clientPid].erase(findItem);
    if (interruptEnableMaps_[clientPid].empty()) {
        interruptEnableMaps_.erase(clientPid);
    }
}

AudioZoneInterruptReporter::ReporterVector AudioZoneInterruptReporter::CreateReporter(
    std::shared_ptr<AudioInterruptService> interruptService,
    std::shared_ptr<AudioZoneClientManager> zoneClientManager,
    AudioZoneInterruptReason reason)
{
    return CreateReporter(-1, interruptService, zoneClientManager, reason);
}

AudioZoneInterruptReporter::ReporterVector AudioZoneInterruptReporter::CreateReporter(
    int32_t zoneId,
    std::shared_ptr<AudioInterruptService> interruptService,
    std::shared_ptr<AudioZoneClientManager> zoneClientManager,
    AudioZoneInterruptReason reason)
{
    ReporterVector vec;
    if (zoneClientManager == nullptr || interruptService == nullptr) {
        return vec;
    }
    
    std::lock_guard<std::mutex> lock(interruptEnableMapsLock_);
    for (auto &item : interruptEnableMaps_) {
        for (auto &it : item.second) {
            if (zoneId != -1 && zoneId != it.first) {
                continue;
            }
            Reporter rep = std::make_shared<AudioZoneInterruptReporter>();
            if (rep == nullptr) {
                return vec;
            }
            rep->interruptService_ = interruptService;
            rep->zoneClientManager_ = zoneClientManager;
            rep->clientPid_ = item.first;
            rep->zoneId_ = it.first;
            rep->deviceId_ = it.second;
            rep->reportReason_ = reason;
            rep->oldFoucsList_ = rep->GetFocusList();
            vec.emplace_back(rep);
            AUDIO_DEBUG_LOG("create reporter with zone %{public}d, device %{public}d"
                " for client %{public}d of reason %{public}d",
                rep->zoneId_, rep->deviceId_, rep->clientPid_,
                rep->reportReason_);
        }
    }
    AUDIO_DEBUG_LOG("create reporter num is %{public}zu", vec.size());
    return vec;
}

AudioZoneFocusList AudioZoneInterruptReporter::GetFocusList()
{
    AudioZoneFocusList focusList;
    if (deviceId_ == -1) {
        interruptService_->GetAudioFocusInfoList(zoneId_, focusList);
    } else {
        interruptService_->GetAudioFocusInfoList(zoneId_, deviceId_, focusList);
    }
    return focusList;
}

void AudioZoneInterruptReporter::ReportInterrupt()
{
    if (zoneClientManager_ == nullptr || interruptService_ == nullptr) {
        return;
    }

    AudioZoneFocusList newFocusList = GetFocusList();
    if (IsFocusListEqual(oldFoucsList_, newFocusList)) {
        return;
    }

    AUDIO_INFO_LOG("report audio zone %{public}d device %{public}d interrupt to"
        " client %{public}d of reason %{public}d ", zoneId_, deviceId_, clientPid_,
        reportReason_);
    zoneClientManager_->ReportInterrupt(clientPid_, zoneId_, deviceId_,
        newFocusList, reportReason_);
}

bool AudioZoneInterruptReporter::IsFocusListEqual(const AudioZoneFocusList &a,
    const AudioZoneFocusList &b)
{
    return std::equal(std::begin(a), std::end(a), std::begin(b), std::end(b),
        [](const std::pair<AudioInterrupt, AudioFocuState> &p1,
            const std::pair<AudioInterrupt, AudioFocuState> &p2) {
            return p1.first.streamUsage == p2.first.streamUsage &&
                p1.first.contentType == p2.first.contentType &&
                p1.first.audioFocusType.streamType == p2.first.audioFocusType.streamType &&
                p1.first.audioFocusType.sourceType == p2.first.audioFocusType.sourceType &&
                p1.first.sessionId == p2.first.sessionId &&
                p1.first.pid == p2.first.pid &&
                p1.first.uid == p2.first.uid &&
                p1.first.deviceId == p2.first.deviceId &&
                p1.first.mode == p2.first.mode &&
                p1.second == p2.second;
            });
}
} // namespace AudioStandard
} // namespace OHO