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
std::mutex AudioZoneInterruptReporter::interruptEnableMutex_;

int32_t AudioZoneInterruptReporter::EnableInterruptReport(pid_t clientPid, int32_t zoneId,
    const std::string &deviceTag, bool enable)
{
    std::lock_guard<std::mutex> lock(interruptEnableMutex_);
    if (enable) {
        return RegisterInterruptReport(clientPid, zoneId, deviceTag);
    }
    UnRegisterInterruptReport(clientPid, zoneId, deviceTag);
    return SUCCESS;
}

void AudioZoneInterruptReporter::DisableInterruptReport(pid_t clientPid)
{
    std::lock_guard<std::mutex> lock(interruptEnableMutex_);
    CHECK_AND_RETURN_LOG(interruptEnableMaps_.find(clientPid) != interruptEnableMaps_.end(), "client not register");

    interruptEnableMaps_.erase(clientPid);
}

void AudioZoneInterruptReporter::DisableAllInterruptReport()
{
    std::lock_guard<std::mutex> lock(interruptEnableMutex_);
    interruptEnableMaps_.clear();
}

int32_t AudioZoneInterruptReporter::RegisterInterruptReport(pid_t clientPid, int32_t zoneId,
    const std::string &deviceTag)
{
    if (interruptEnableMaps_.find(clientPid) == interruptEnableMaps_.end()) {
        ReportItemList reportList;
        interruptEnableMaps_[clientPid] = reportList;
    }

    ReportItem newItem = std::make_pair(zoneId, deviceTag);
    auto findItem = std::find(interruptEnableMaps_[clientPid].begin(),
        interruptEnableMaps_[clientPid].end(), newItem);
    CHECK_AND_RETURN_RET_LOG(findItem == interruptEnableMaps_[clientPid].end(),
        SUCCESS, "client %{public}d is already register", clientPid);

    interruptEnableMaps_[clientPid].emplace_back(newItem);
    AUDIO_INFO_LOG("register zone %{public}d, device %{public}s for client %{public}d",
        zoneId, deviceTag.c_str(), clientPid);
    return SUCCESS;
}

void AudioZoneInterruptReporter::UnRegisterInterruptReport(pid_t clientPid, int32_t zoneId,
    const std::string &deviceTag)
{
    AUDIO_INFO_LOG("unregister zone %{public}d, device %{public}s for client %{public}d",
        zoneId, deviceTag.c_str(), clientPid);
    CHECK_AND_RETURN_LOG(interruptEnableMaps_.find(clientPid) != interruptEnableMaps_.end(),
        "client %{public}d not register", clientPid);

    ReportItem removeItem = std::make_pair(zoneId, deviceTag);
    auto findItem = std::find(interruptEnableMaps_[clientPid].begin(),
        interruptEnableMaps_[clientPid].end(), removeItem);
    
    CHECK_AND_RETURN_LOG(findItem != interruptEnableMaps_[clientPid].end(),
        "client %{public}d is not find %{public}d", clientPid, zoneId);

    interruptEnableMaps_[clientPid].erase(findItem);
    CHECK_AND_RETURN_LOG(!interruptEnableMaps_[clientPid].empty(),
        "client %{public}d is not exist", clientPid);

    interruptEnableMaps_.erase(clientPid);
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
    CHECK_AND_RETURN_RET_LOG(interruptService != nullptr && zoneClientManager != nullptr, vec,
        "interruptService or zoneClientManager is null");

    std::lock_guard<std::mutex> lock(interruptEnableMutex_);
    for (auto &item : interruptEnableMaps_) {
        for (auto &it : item.second) {
            CHECK_AND_CONTINUE(zoneId == -1 || zoneId == it.first);

            Reporter rep = std::make_shared<AudioZoneInterruptReporter>();
            CHECK_AND_RETURN_RET_LOG(rep != nullptr, vec, "create reporter failed");

            rep->interruptService_ = interruptService;
            rep->zoneClientManager_ = zoneClientManager;
            rep->clientPid_ = item.first;
            rep->zoneId_ = it.first;
            rep->deviceTag_ = it.second;
            rep->reportReason_ = reason;
            rep->oldFocusList_ = rep->GetFocusList();
            vec.emplace_back(rep);
            AUDIO_DEBUG_LOG("create reporter with zone %{public}d, device %{public}s"
                " for client %{public}d of reason %{public}d",
                rep->zoneId_, rep->deviceTag_.c_str(), rep->clientPid_,
                rep->reportReason_);
        }
    }
    AUDIO_DEBUG_LOG("create reporter num is %{public}zu", vec.size());
    return vec;
}

AudioZoneFocusList AudioZoneInterruptReporter::GetFocusList()
{
    AudioZoneFocusList focusList;
    if (deviceTag_.empty()) {
        interruptService_->GetAudioFocusInfoList(zoneId_, focusList);
    } else {
        interruptService_->GetAudioFocusInfoList(zoneId_, deviceTag_, focusList);
    }
    return focusList;
}

void AudioZoneInterruptReporter::ReportInterrupt()
{
    CHECK_AND_RETURN_LOG(interruptService_!= nullptr && zoneClientManager_!= nullptr,
        "interruptService or zoneClientManager is null");

    AudioZoneFocusList newFocusList = GetFocusList();
    CHECK_AND_RETURN_LOG(!IsFocusListEqual(oldFocusList_, newFocusList), "focus not change");

    AUDIO_INFO_LOG("report audio zone %{public}d device %{public}s interrupt to"
        " client %{public}d of reason %{public}d ", zoneId_, deviceTag_.c_str(), clientPid_,
        reportReason_);
    zoneClientManager_->SendZoneInterruptEvent(clientPid_, zoneId_, deviceTag_,
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
                p1.first.streamId == p2.first.streamId &&
                p1.first.pid == p2.first.pid &&
                p1.first.uid == p2.first.uid &&
                p1.first.deviceTag == p2.first.deviceTag &&
                p1.first.mode == p2.first.mode &&
                p1.second == p2.second;
            });
}
} // namespace AudioStandard
} // namespace OHOS