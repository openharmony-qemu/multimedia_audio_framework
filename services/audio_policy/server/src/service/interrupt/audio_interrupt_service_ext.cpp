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
#define LOG_TAG "AudioInterruptService"
#endif

#include "audio_interrupt_service.h"

#include "audio_focus_parser.h"
#include "audio_policy_manager_listener_proxy.h"
#include "media_monitor_manager.h"

#include "dfx_utils.h"
#include "app_mgr_client.h"
#include "dfx_msg_manager.h"

namespace OHOS {
namespace AudioStandard {
void AudioInterruptService::AudioInterruptZoneDump(std::string &dumpString)
{
    std::unordered_map<int32_t, std::shared_ptr<AudioInterruptZone>> audioInterruptZonesMapDump;
    AddDumpInfo(audioInterruptZonesMapDump);
    dumpString += "\nAudioInterrupt Zone:\n";
    AppendFormat(dumpString, "- %zu AudioInterruptZoneDump (s) available:\n",
        zonesMap_.size());
    for (const auto&[zoneID, audioInterruptZoneDump] : audioInterruptZonesMapDump) {
        if (zoneID < 0) {
            continue;
        }
        AppendFormat(dumpString, "  - Zone ID: %d\n", zoneID);
        AppendFormat(dumpString, "  - Pids size: %zu\n", audioInterruptZoneDump->pids.size());
        for (auto pid : audioInterruptZoneDump->pids) {
            AppendFormat(dumpString, "    - pid: %d\n", pid);
        }

        AppendFormat(dumpString, "  - Interrupt callback size: %zu\n",
            audioInterruptZoneDump->interruptCbStreamIdsMap.size());
        AppendFormat(dumpString, "    - The streamIds as follow:\n");
        for (auto streamId : audioInterruptZoneDump->interruptCbStreamIdsMap) {
            AppendFormat(dumpString, "      - StreamId: %u -- have interrupt callback.\n", streamId);
        }

        AppendFormat(dumpString, "  - Audio policy client proxy callback size: %zu\n",
            audioInterruptZoneDump->audioPolicyClientProxyCBClientPidMap.size());
        AppendFormat(dumpString, "    - The clientPids as follow:\n");
        for (auto pid : audioInterruptZoneDump->audioPolicyClientProxyCBClientPidMap) {
            AppendFormat(dumpString, "      - ClientPid: %d -- have audiopolicy client proxy callback.\n", pid);
        }

        std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList
            = audioInterruptZoneDump->audioFocusInfoList;
        AppendFormat(dumpString, "  - %zu Audio Focus Info (s) available:\n", audioFocusInfoList.size());
        uint32_t invalidStreamId = static_cast<uint32_t>(-1);
        for (auto iter = audioFocusInfoList.begin(); iter != audioFocusInfoList.end(); ++iter) {
            if ((iter->first).streamId == invalidStreamId) {
                continue;
            }
            AppendFormat(dumpString, "    - Pid: %d\n", (iter->first).pid);
            AppendFormat(dumpString, "    - StreamId: %u\n", (iter->first).streamId);
            AppendFormat(dumpString, "    - Audio Focus isPlay Id: %d\n", (iter->first).audioFocusType.isPlay);
            AppendFormat(dumpString, "    - Stream Name: %s\n",
                AudioInfoDumpUtils::GetStreamName((iter->first).audioFocusType.streamType).c_str());
            AppendFormat(dumpString, "    - Source Name: %s\n",
                AudioInfoDumpUtils::GetSourceName((iter->first).audioFocusType.sourceType).c_str());
            AppendFormat(dumpString, "    - Audio Focus State: %d\n", iter->second);
            dumpString += "\n";
        }
        dumpString += "\n";
    }
    return;
}

void AudioInterruptService::AudioSessionInfoDump(std::string &dumpString)
{
    AppendFormat(dumpString, "    - The AudioSession as follow:\n");
    if (sessionService_ == nullptr) {
        AppendFormat(dumpString, "    - The AudioSessionService is null.\n");
        return;
    }
    sessionService_->AudioSessionInfoDump(dumpString);
}

// AudioInterruptDeathRecipient impl begin
AudioInterruptService::AudioInterruptDeathRecipient::AudioInterruptDeathRecipient(
    const std::shared_ptr<AudioInterruptService> &service,
    uint32_t streamId)
    : service_(service), streamId_(streamId)
{
}

void AudioInterruptService::AudioInterruptDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    std::shared_ptr<AudioInterruptService> service = service_.lock();
    if (service != nullptr) {
        service->RemoveClient(ZONEID_DEFAULT, streamId_);
    }
}

// AudioInterruptClient impl begin
AudioInterruptService::AudioInterruptClient::AudioInterruptClient(
    const std::shared_ptr<AudioInterruptCallback> &callback,
    const sptr<IRemoteObject> &object,
    const sptr<AudioInterruptDeathRecipient> &deathRecipient)
    : callback_(callback), object_(object), deathRecipient_(deathRecipient)
{
}

AudioInterruptService::AudioInterruptClient::~AudioInterruptClient()
{
    if (object_ != nullptr) {
        object_->RemoveDeathRecipient(deathRecipient_);
    }
}

void AudioInterruptService::AudioInterruptClient::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    if (callback_ != nullptr) {
        callback_->OnInterrupt(interruptEvent);
    }
}

void AudioInterruptService::AudioInterruptClient::SetCallingUid(uint32_t uid)
{
    AUDIO_INFO_LOG("uid: %{public}u", uid);
    callingUid_ = uid;
}

uint32_t AudioInterruptService::AudioInterruptClient::GetCallingUid()
{
    AUDIO_INFO_LOG("callingUid_: %{public}u", callingUid_);
    return callingUid_;
}

}
} // namespace OHOS
