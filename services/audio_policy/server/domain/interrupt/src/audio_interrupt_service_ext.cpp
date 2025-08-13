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
#include "standard_audio_policy_manager_listener_proxy.h"
#include "media_monitor_manager.h"

#include "dfx_utils.h"
#include "app_mgr_client.h"
#include "dfx_msg_manager.h"
#include "audio_server_proxy.h"

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
            AppendFormat(dumpString, "    - isAudioSessionInterrupt: %d\n", (iter->first).isAudioSessionInterrupt);
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
    return callingUid_;
}

void AudioInterruptService::SetSessionMuteState(uint32_t sessionId, bool insert, bool muteFlag)
{
    AudioServerProxy::GetInstance().SetSessionMuteState(sessionId, insert, muteFlag);
}

void AudioInterruptService::SetLatestMuteState(const InterruptEventInternal &interruptEvent,
    const uint32_t &streamId)
{
    CHECK_AND_RETURN_LOG(interruptEvent.hintType == INTERRUPT_HINT_MUTE ||
        interruptEvent.hintType == INTERRUPT_HINT_UNMUTE, "unsupported hintType: %{public}d",
        interruptEvent.hintType);
    bool muteFlag = interruptEvent.hintType == INTERRUPT_HINT_MUTE;
    AudioServerProxy::GetInstance().SetLatestMuteState(streamId, muteFlag);
}

void AudioInterruptService::UpdateMuteAudioFocusStrategy(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    if (currentInterrupt.strategy == InterruptStrategy::DEFAULT &&
        incomingInterrupt.strategy == InterruptStrategy::DEFAULT) {
        return;
    }

    if ((focusEntry.hintType != INTERRUPT_HINT_STOP &&
        focusEntry.hintType != INTERRUPT_HINT_PAUSE) ||
        incomingInterrupt.audioFocusType.streamType == STREAM_INTERNAL_FORCE_STOP) {
        AUDIO_INFO_LOG("streamId: %{public}u, keep current hintType=%{public}d",
            currentInterrupt.streamId, focusEntry.hintType);
        return;
    }

    if (currentInterrupt.strategy == InterruptStrategy::MUTE) {
        focusEntry.actionOn = CURRENT;
    }
    if (incomingInterrupt.strategy == InterruptStrategy::MUTE) {
        focusEntry.actionOn = INCOMING;
    }

    AUDIO_INFO_LOG("currentStreamId:%{public}u, incomingStreamId:%{public}u, action:%{public}u",
        currentInterrupt.streamId, incomingInterrupt.streamId, focusEntry.actionOn);
    focusEntry.isReject = false;
    focusEntry.hintType = INTERRUPT_HINT_MUTE;
}

int32_t AudioInterruptService::ProcessActiveStreamFocus(
    std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList,
    const AudioInterrupt &incomingInterrupt, AudioFocuState &incomingState,
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &activeInterrupt)
{
    incomingState = ACTIVE;
    activeInterrupt = audioFocusInfoList.end();

    for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
        activeInterrupt = iterActive;
        if (IsSameAppInShareMode(incomingInterrupt, iterActive->first)) { continue; }
        // if peeling is the incomming interrupt while at the momount there are already some existing recordings
        // peeling should be rejected
        if (IsLowestPriorityRecording(incomingInterrupt) && IsRecordingInterruption(iterActive->first)) {
            incomingState = STOP;
            AUDIO_INFO_LOG("PEELING AUDIO fail, there's a device recording");
            break;
        }

        std::pair<AudioFocusType, AudioFocusType> focusPair =
            std::make_pair((iterActive->first).audioFocusType, incomingInterrupt.audioFocusType);
        CHECK_AND_RETURN_RET_LOG(focusCfgMap_.find(focusPair) != focusCfgMap_.end(),
            ERR_INVALID_PARAM,
            "no focus cfg, active stream type = %{public}d, incoming stream type = %{public}d",
            static_cast<int32_t>(focusPair.first.streamType),
            static_cast<int32_t>(focusPair.second.streamType));
        AudioFocusEntry focusEntry = focusCfgMap_[focusPair];
        UpdateAudioFocusStrategy(iterActive->first, incomingInterrupt, focusEntry);
        CheckIncommingFoucsValidity(focusEntry, incomingInterrupt, incomingInterrupt.currencySources.sourcesTypes);
        if (FocusEntryContinue(iterActive, focusEntry, incomingInterrupt)) { continue; }
        if (focusEntry.isReject) {
            if (IsGameAvoidCallbackCase(incomingInterrupt)) {
                incomingState = PAUSE;
                AUDIO_INFO_LOG("incomingState: %{public}d", incomingState);
                continue;
            }

            AUDIO_INFO_LOG("the incoming stream is rejected by streamId:%{public}d, pid:%{public}d",
                (iterActive->first).streamId, (iterActive->first).pid);
            incomingState = STOP;
            break;
        }
        incomingState = GetNewIncomingState(focusEntry.hintType, incomingState);
    }
    if (incomingState == STOP && !incomingInterrupt.deviceTag.empty()) {
        incomingState = ACTIVE;
    }
    return SUCCESS;
}

void AudioInterruptService::ReportRecordGetFocusFail(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt, int32_t reason)
{
    CHECK_AND_RETURN_LOG(incomingInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID &&
        incomingInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID, "not recording");
    AUDIO_INFO_LOG("recording failed to start, incoming: sourceType %{public}d pid %{public}d uid %{public}d"\
        "active: sourceType %{public}d pid %{public}d uid %{public}d",
        incomingInterrupt.audioFocusType.sourceType, incomingInterrupt.pid, incomingInterrupt.uid,
        activeInterrupt.audioFocusType.sourceType, activeInterrupt.pid, activeInterrupt.uid);

    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::RECORD_ERROR,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    CHECK_AND_RETURN_LOG(bean != nullptr, "bean is nullptr");

    bean->Add("INCOMING_SOURCE", incomingInterrupt.audioFocusType.sourceType);
    bean->Add("INCOMING_PID", incomingInterrupt.pid);
    bean->Add("INCOMING_UID", incomingInterrupt.uid);
    bean->Add("ACTIVE_SOURCE", activeInterrupt.audioFocusType.sourceType);
    bean->Add("ACTIVE_PID", activeInterrupt.pid);
    bean->Add("ACTIVE_UID", activeInterrupt.uid);
    bean->Add("REASON", reason);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

bool AudioInterruptService::IsCapturerFocusAvailable(const int32_t zoneId, const AudioCapturerInfo &capturerInfo)
{
    if (isPreemptMode_) {
        AUDIO_INFO_LOG("Preempt mode, recording is not allowed");
        return false;
    }

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = capturerInfo.sourceType;
    incomingInterrupt.audioFocusType.isPlay = false;
    AudioFocuState incomingState = ACTIVE;
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG(itZone != zonesMap_.end(), false, "can not find zoneid");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    if (itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator activeInterrupt = audioFocusInfoList.end();
    int32_t res = ProcessActiveStreamFocus(audioFocusInfoList, incomingInterrupt, incomingState, activeInterrupt);
    return res == SUCCESS && incomingState < PAUSE;
}

int32_t AudioInterruptService::ClearAudioFocusBySessionID(const int32_t &sessionID)
{
    AUDIO_INFO_LOG("start clear audio focus, target sessionID:%{public}d", sessionID);

    int32_t targetZoneId = -1;
    AudioInterrupt targetInterrupt;
    const uint32_t targetSessionID = static_cast<uint32_t>(sessionID);
    bool clearFlag = false;
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};

    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (const auto&[zoneId, audioInterruptZone] : zonesMap_) {
            CHECK_AND_CONTINUE_LOG(audioInterruptZone != nullptr, "audioInterruptZone is nullptr");

            auto match = [&](const auto& item) {
                return sessionID >= 0 && item.first.streamId == targetSessionID;
            };

            auto it = std::find_if(audioInterruptZone->audioFocusInfoList.begin(),
                audioInterruptZone->audioFocusInfoList.end(), match);
            if (it != audioInterruptZone->audioFocusInfoList.end()) {
                targetZoneId = zoneId;
                targetInterrupt = it->first;
                clearFlag = true;
                break;
            }
        }
    }

    if (clearFlag) {
        (void)DeactivateAudioInterrupt(targetZoneId, targetInterrupt);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            CHECK_AND_RETURN_RET_LOG(handler_ != nullptr, ERROR, "handler is nullptr");
            SendInterruptEventCallback(interruptEvent, targetInterrupt.streamId, targetInterrupt);
        }
    }

    return SUCCESS;
}
}
} // namespace OHOS
