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
    streamCollector_.CapturerMutedFlagChange(sessionId, muteFlag);
}

void AudioInterruptService::SetLatestMuteState(const InterruptEventInternal &interruptEvent,
    const uint32_t &streamId)
{
    CHECK_AND_RETURN_LOG(interruptEvent.hintType == INTERRUPT_HINT_MUTE ||
        interruptEvent.hintType == INTERRUPT_HINT_UNMUTE, "unsupported hintType: %{public}d",
        interruptEvent.hintType);
    bool muteFlag = interruptEvent.hintType == INTERRUPT_HINT_MUTE;
    AudioServerProxy::GetInstance().SetLatestMuteState(streamId, muteFlag);
    streamCollector_.CapturerMutedFlagChange(streamId, muteFlag);
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
        CHECK_AND_CALL_FUNC_RETURN_RET(focusCfgMap_.find(focusPair) != focusCfgMap_.end(), ERR_INVALID_PARAM,
            HILOG_COMM_ERROR("[ProcessActiveStreamFocus]no focus cfg, active stream type = %{public}d, "
                "incoming stream type = %{public}d",
                static_cast<int32_t>(focusPair.first.streamType),
                static_cast<int32_t>(focusPair.second.streamType)));
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

            HILOG_COMM_INFO("[ProcessActiveStreamFocus]the incoming stream is rejected by streamId:%{public}d, "
                "pid:%{public}d", (iterActive->first).streamId, (iterActive->first).pid);
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
        activeInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID, "not recording");
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

    AUDIO_INFO_LOG("start check capturer focus, zoneId:%{public}d, sourceType:%{public}d",
        zoneId, capturerInfo.sourceType);
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = capturerInfo.sourceType;
    incomingInterrupt.audioFocusType.isPlay = false;
    AudioFocuState incomingState = ACTIVE;
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_CALL_FUNC_RETURN_RET(itZone != zonesMap_.end(), false,
        HILOG_COMM_ERROR("[IsCapturerFocusAvailable]can not find zoneid"));
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

void AudioInterruptService::RemoveStreamIdSuggestionRecord(int32_t streamId)
{
    bool hasRecord = false;
    for (auto it = suggestionStreamIdRecords_.begin(); it != suggestionStreamIdRecords_.end(); ++it) {
        if (it->second.count(streamId) > 0) {
            it->second.erase(streamId);
            hasRecord = true;
            AUDIO_INFO_LOG("remove mute stream record %{public}d for streamId %{public}d", streamId, it->first);
        }
    }
    if (hasRecord) {
        RemoveMuteSuggestionRecord();
    }
}
 
void AudioInterruptService::RemovePidSuggestionRecord(int32_t pid)
{
    if (suggestionPidRecords_.count(pid) > 0) {
        suggestionInterrupts_.erase(pid);
        suggestionStreamIdRecords_.erase(pid);
        suggestionPidRecords_.erase(pid);
    }
    bool hasRecord = false;
    for (auto it = suggestionPidRecords_.begin(); it != suggestionPidRecords_.end(); ++it) {
        it->second.erase(pid);
        if (it->second.count(pid) > 0) {
            it->second.erase(pid);
            hasRecord = true;
            AUDIO_INFO_LOG("remove mute session record %{public}d for streamId %{public}d", pid, it->first);
        }
    }
    if (hasRecord) {
        RemoveMuteSuggestionRecord();
    }
}
 
bool AudioInterruptService::HasMuteSuggestionRecord(uint32_t currentpid)
{
    auto pidRecord = suggestionPidRecords_.find(currentpid);
    if (pidRecord != suggestionPidRecords_.end() && pidRecord->second.size() > 0) {
        return true;
    }
    auto streamIdRecord = suggestionStreamIdRecords_.find(currentpid);
    if (streamIdRecord != suggestionStreamIdRecords_.end() && streamIdRecord->second.size() > 0) {
        return true;
    }
    return false;
}
 
void AudioInterruptService::SendUnMuteSuggestionInterruptEvent(uint32_t currentpid)
{
    if (!HasMuteSuggestionRecord(currentpid)) {
        auto it = suggestionInterrupts_.find(currentpid);
        if (it == suggestionInterrupts_.end()) {
            return;
        }
        auto currentInterrupt = it->second;
        AUDIO_INFO_LOG("Send unmute suggestion for currentpid %{public}d", currentpid);
        InterruptEventInternal interruptEvent = {
            INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_UNMUTE_SUGGESTION, 1.0f};
        SendInterruptEventCallback(interruptEvent, currentInterrupt->streamId, *currentInterrupt);
        suggestionInterrupts_.erase(currentpid);
        suggestionStreamIdRecords_.erase(currentpid);
        suggestionPidRecords_.erase(currentpid);
    }
}
 
void AudioInterruptService::DelayRemoveMuteSuggestionRecord(uint32_t currentpid)
{
    auto audioInterruptService = shared_from_this();
    auto removeTask = [audioInterruptService, currentpid] {
        if (audioInterruptService == nullptr) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        audioInterruptService->SendUnMuteSuggestionInterruptEvent(currentpid);
    };
 
    std::thread(removeTask).detach();
    AUDIO_INFO_LOG("Started unmute suggestion for currentpid %{public}d with 1s delay", currentpid);
}
 
void AudioInterruptService::UpdateMuteSuggestionRecords(uint32_t currentpid)
{
    for (auto record = suggestionPidRecords_.begin(); record != suggestionPidRecords_.end(); ++record) {
        if (record->first != currentpid) {
            record->second.insert(currentpid);
        }
    }
}

void AudioInterruptService::RemoveMuteSuggestionRecord()
{
    for (auto it = suggestionInterrupts_.begin(); it != suggestionInterrupts_.end(); ++it) {
        auto currentpid = it->first;
        if (!HasMuteSuggestionRecord(currentpid)) {
            DelayRemoveMuteSuggestionRecord(currentpid);
            UpdateMuteSuggestionRecords(currentpid);
            return;
        }
    }
}

void AudioInterruptService::AddMuteSuggestionRecord(const AudioFocusEntry &focusEntry,
    const AudioInterrupt &muteInterrupt, const AudioInterrupt &recordInterrupt)
{
    auto mutePid = muteInterrupt.pid;
    auto it = suggestionInterrupts_.find(mutePid);
    if (it == suggestionInterrupts_.end()) {
        std::shared_ptr<AudioInterrupt> sp = std::make_shared<AudioInterrupt>(muteInterrupt);
        suggestionInterrupts_[mutePid] = sp;
        AUDIO_INFO_LOG("Send mute suggestion for pid %{public}d", mutePid);
        InterruptEventInternal interruptEvent = {INTERRUPT_TYPE_BEGIN, focusEntry.forceType,
            INTERRUPT_HINT_MUTE_SUGGESTION, 1.0f};
        SendInterruptEventCallback(interruptEvent, muteInterrupt.streamId, muteInterrupt);
    }
    if (sessionService_.IsAudioSessionFocusMode(recordInterrupt.pid) &&
        suggestionPidRecords_[mutePid].count(recordInterrupt.pid) <= 0) {
        suggestionPidRecords_[mutePid].insert(recordInterrupt.pid);
        AUDIO_INFO_LOG("add mute session record %{public}d for sessionId %{public}d",
            recordInterrupt.pid, mutePid);
    } else if (!sessionService_.IsAudioSessionFocusMode(recordInterrupt.pid) &&
        suggestionStreamIdRecords_[mutePid].count(recordInterrupt.streamId) <= 0) {
        suggestionStreamIdRecords_[mutePid].insert(recordInterrupt.streamId);
        AUDIO_INFO_LOG("add mute stream record %{public}d for sessionId %{public}d",
            recordInterrupt.streamId, mutePid);
    }
}
 
void AudioInterruptService::SuggestionProcessWhenMixWithOthers(const AudioFocusEntry &focusEntry,
    const AudioInterrupt &currentInterrupt, const AudioInterrupt &incomingInterrupt)
{
    if (focusEntry.hintType != INTERRUPT_HINT_PAUSE && focusEntry.hintType != INTERRUPT_HINT_STOP) {
        return;
    }
    if (currentInterrupt.audioFocusType.streamType > 0 &&
        sessionService_.IsAudioSessionFocusMode(currentInterrupt.pid) &&
        sessionService_.GetSessionStrategy(currentInterrupt.pid) == AudioConcurrencyMode::MIX_WITH_OTHERS &&
        sessionService_.IsMuteSuggestionWhenMixEnabled(currentInterrupt.pid)) {
        AddMuteSuggestionRecord(focusEntry, currentInterrupt, incomingInterrupt);
    }
 
    if (incomingInterrupt.audioFocusType.streamType > 0 &&
        sessionService_.IsAudioSessionFocusMode(incomingInterrupt.pid) &&
        sessionService_.GetSessionStrategy(incomingInterrupt.pid) == AudioConcurrencyMode::MIX_WITH_OTHERS &&
        sessionService_.IsMuteSuggestionWhenMixEnabled(incomingInterrupt.pid) &&
        !sessionService_.IsMuteSuggestionWhenMixEnabled(currentInterrupt.pid)) {
        AddMuteSuggestionRecord(focusEntry, incomingInterrupt, currentInterrupt);
    }
}

int32_t AudioInterruptService::EnableMuteSuggestionWhenMixWithOthers(int32_t callerPid, bool enable)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return sessionService_.EnableMuteSuggestionWhenMixWithOthers(callerPid, enable);
}

void AudioInterruptService::RemoveInterruptFocusInfoList(
    const std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo)
{
    AudioInterrupt activeInterrupt = audioFocusInfo.first;
    int32_t zoneId = zoneManager_.FindZoneByPid(activeInterrupt.pid);
    auto itZone = zonesMap_.find(zoneId);
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        auto& audioFocusInfoList = itZone->second->audioFocusInfoList;
        audioFocusInfoList.remove(audioFocusInfo);
    }
    if (sessionService_.IsAudioSessionActivated(activeInterrupt.pid) &&
        HandleLowPriorityEvent(activeInterrupt.pid, activeInterrupt.streamId)) {
        RemovePlaceholderInterruptForSession(activeInterrupt.pid);
    }
    for (auto&[streamId, activeFocusList] : muteAudioFocus_) {
        activeFocusList.remove_if([&activeInterrupt](const std::pair<AudioInterrupt, AudioFocuState>& pair) {
            return pair.first.streamId == activeInterrupt.streamId;
        });
    }
}

void AudioInterruptService::NotifyStreamSilentChange(uint32_t streamId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (muteAudioFocus_.count(streamId) > 0) {
        std::list<std::pair<AudioInterrupt, AudioFocuState>> tempList(muteAudioFocus_[streamId]);
        for (const auto& audioFocusInfo : tempList) {
            AudioInterrupt activeInterrupt = audioFocusInfo.first;
            InterruptEventInternal interruptEvent = {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
            AUDIO_INFO_LOG("NotifyStreamSilentChange:streamId %{public}d is stopped by streamId: %{public}d",
                activeInterrupt.streamId, streamId);
            SendInterruptEventCallback(interruptEvent, activeInterrupt.streamId, activeInterrupt);
            RemoveInterruptFocusInfoList(audioFocusInfo);
        }
        muteAudioFocus_.erase(streamId);
    }
}

void AudioInterruptService::MuteCheckFocusStrategy(AudioFocusEntry& focusEntry,
    const std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo, const AudioInterrupt &incomingInterrupt,
    bool &removeFocusInfo, InterruptEventInternal &interruptEvent)
{
    if (sessionService_.IsAudioSessionFocusMode(incomingInterrupt.pid)) {
        return;
    }

    AudioInterrupt currentInterrupt = audioFocusInfo.first;
    AudioStreamType currentStreamType = currentInterrupt.audioFocusType.streamType;
    AudioStreamType incomingStreamType = incomingInterrupt.audioFocusType.streamType;
    if (focusEntry.hintType != INTERRUPT_HINT_STOP ||
        !IsMediaStream(currentStreamType) || !IsMediaStream(incomingStreamType)) {
        return;
    }

    bool isInMuteCheckList = false;
    auto bundleName = GetAudioInterruptBundleName(incomingInterrupt);
    string muteCheckAppName = bundleName + "_check";
    if (queryBundleNameListCallback_ != nullptr) {
        queryBundleNameListCallback_->OnQueryBundleNameIsInList(muteCheckAppName, "audio_param",
            isInMuteCheckList);
    }
    if (isInMuteCheckList) {
        focusEntry.hintType = INTERRUPT_HINT_NONE;
        interruptEvent.hintType = INTERRUPT_HINT_NONE;
        removeFocusInfo = false;
        muteAudioFocus_[incomingInterrupt.streamId].push_back(audioFocusInfo);
        AUDIO_INFO_LOG("%{public}s update muteCheck focusStrategy", bundleName.c_str());
    }
}
}
} // namespace OHOS
