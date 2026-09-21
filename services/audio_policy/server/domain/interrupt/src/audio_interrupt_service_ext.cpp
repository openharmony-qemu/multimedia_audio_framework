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
#include "audio_policy_server.h"
#include "audio_focus_parser.h"
#include "standard_audio_policy_manager_listener_proxy.h"
#include "media_monitor_manager.h"

#include "dfx_utils.h"
#include "app_mgr_client.h"
#include "dfx_msg_manager.h"
#include "audio_server_proxy.h"
#include "audio_interrupt_utils.h"
#include "stream_dfx_manager.h"
#include <unistd.h>

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {

class InterruptResponseCheckAction : public AsyncActionHandler::AsyncAction {
public:
    explicit InterruptResponseCheckAction(std::shared_ptr<AudioInterruptService> service,
        const int32_t zoneId, InterruptResponseCheckInfo info)
        : service_(service), zoneId_(zoneId), info_(std::move(info)) {}

    void Exec() override
    {
        if (service_ == nullptr) {
            return;
        }
        service_->CheckInterruptResponseAfterDelay(zoneId_, info_);
    }

private:
    std::shared_ptr<AudioInterruptService> service_;
    const int32_t zoneId_;
    InterruptResponseCheckInfo info_;
};

PausedStreamTimeoutMonitor::PausedStreamTimeoutMonitor(int32_t zoneId, uint32_t streamId,
    std::weak_ptr<AudioInterruptService> service)
    : zoneId_(zoneId), streamId_(streamId), service_(std::move(service)) {}

PausedStreamTimeoutMonitor::~PausedStreamTimeoutMonitor()
{
    StopMonitor();
}

void PausedStreamTimeoutMonitor::OnTimeOut()
{
    auto service = service_.lock();
    CHECK_AND_RETURN_LOG(service != nullptr, "AudioInterruptService is null");
    service->HandlePausedTimeout(zoneId_, streamId_);
}

void PausedStreamTimeoutMonitor::StartMonitor(int32_t duration, std::shared_ptr<PausedStreamTimeoutMonitor> cb)
{
    cbId_ = DelayedSingleton<AudioPolicyStateMonitor>::GetInstance()->RegisterCallback(
        cb, duration, CallbackType::ONE_TIME);
    if (cbId_ == INVALID_CB_ID) {
        AUDIO_ERR_LOG("Register PausedStreamTimeoutMonitor failed for streamId %{public}u", streamId_);
    }
}

void PausedStreamTimeoutMonitor::StopMonitor()
{
    if (cbId_ != INVALID_CB_ID) {
        DelayedSingleton<AudioPolicyStateMonitor>::GetInstance()->UnRegisterCallback(cbId_);
        cbId_ = INVALID_CB_ID;
    }
}

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
        service->RemoveClient(streamId_);
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

void AudioInterruptService::SetLatestMuteState(const AudioInterrupt &audioInterrupt, bool muteFlag)
{
    uint32_t streamId = audioInterrupt.streamId;
    AudioServerProxy::GetInstance().SetLatestMuteState(streamId, muteFlag);
    streamCollector_.CapturerMutedFlagChange(streamId, muteFlag);
    policyServer_->SetSourceOutputStreamMuteByStreamId(streamId, muteFlag);
    if (audioInterrupt.audioFocusType.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) {
        policyServer_->NotifyVoipPriorityLoweredByMute(streamId, muteFlag);
    }
}

int32_t AudioInterruptService::ProcessActiveStreamFocus(
    std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList,
    const AudioInterrupt &incomingInterrupt, AudioFocuState &incomingState,
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &activeInterrupt)
{
    incomingState = ACTIVE;
    activeInterrupt = audioFocusInfoList.end();

    for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
        if (iterActive->second == AudioFocuState::STANDBY) {
            continue;
        }
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
        bool isRecord = iterActive->first.audioFocusType.sourceType != SOURCE_TYPE_INVALID;
        CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(focusCfgMap_.find(focusPair) != focusCfgMap_.end(),
            ERR_INVALID_PARAM,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                isRecord ? RECORD_INTERRUPT_CONFIG_OPERATION_FAILED : INTERRUPT_CONFIG_OPERATION_FAILED,
                "no focus cfg found", false),
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

bool AudioInterruptService::IsCapturerFocusAvailable(const int32_t zoneId, const AudioCapturerInfo &capturerInfo)
{
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

void AudioInterruptService::RemoveSuggestionRecord(const AudioInterrupt &audioInterrupt)
{
    if (audioInterrupt.isAudioSessionInterrupt) {
        RemovePidSuggestionRecord(audioInterrupt.pid);
    } else {
        RemoveStreamIdSuggestionRecord(audioInterrupt.streamId);
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
        InterruptEventInternal interruptEvent = {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE,
            INTERRUPT_HINT_UNMUTE_SUGGESTION, 1.0f};
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

bool AudioInterruptService::IsInterruptExist(uint32_t streamId,
    std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList,
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive)
{
    auto isPresent = [streamId] (const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.streamId == streamId;
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isPresent);
    if (iter == audioFocusInfoList.end()) {
        AUDIO_INFO_LOG("streamId %{public}d has stopped", streamId);
        return false;
    }
    iterActive = iter;
    return true;
}

void AudioInterruptService::UpdateIsCheckMuteStreamByPid(int32_t callerPid)
{
    std::vector<int32_t> zoneIds = zoneManager_.FindAudioZonesByPid(callerPid);
    for (const auto zoneId : zoneIds) {
        auto itZone = zonesMap_.find(zoneId);
        CHECK_AND_CONTINUE_LOG((itZone != zonesMap_.end()) && (itZone->second != nullptr), "can not find zone");
        auto &audioFocusInfoList = itZone->second->audioFocusInfoList;
        for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
            CHECK_AND_CONTINUE((iterActive->first).pid == callerPid);
            (iterActive->first).isCheckMuteStream = false;
        }
    }
}

void AudioInterruptService::ProcessSilentChangeInterrupt(const MuteFocusInfo &audioFocusInfo)
{
    const AudioInterrupt& activeInterrupt = audioFocusInfo.currInterrupt;
    const AudioInterrupt& incomingInterrupt = audioFocusInfo.incomingInterrupt;
    int32_t activeZoneId = FindAudioZoneByStreamIdInternal(activeInterrupt.sessionId);
    int32_t incomingZoneId = FindAudioZoneByStreamIdInternal(incomingInterrupt.sessionId);
    CHECK_AND_RETURN_LOG(activeZoneId == incomingZoneId,
        "NotifyStreamSilentChange:streamId %{public}d(zoneId:%{public}d) and streamId "
        "%{public}d(zoneId:%{public}d) are not in the same zone, skip interrupt",
        activeInterrupt.sessionId, activeZoneId, incomingInterrupt.sessionId, incomingZoneId);

    auto itZone = zonesMap_.find(activeZoneId);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end() && itZone->second != nullptr,
        "zone not found or zone is null");
    auto audioFocusInfoList = itZone->second->audioFocusInfoList;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator activeIter;
    CHECK_AND_RETURN_LOG(IsInterruptExist(activeInterrupt.sessionId, audioFocusInfoList, activeIter),
        "the active interrupt is not exist");

    bool isIncomingBundleNameExist = false;
    if (interruptStrategy_ != nullptr) {
        std::string incomingBundleName = AudioInterruptUtils::GetAudioInterruptBundleName(incomingInterrupt);
        interruptStrategy_->OnQueryBundleNameIsInList(incomingBundleName, "audio_param",
            isIncomingBundleNameExist);
    }
    InterruptHint hintType = INTERRUPT_HINT_STOP;
    if (activeInterrupt.behaviorFlags_.pauseWhenInterruptFlag || isIncomingBundleNameExist) {
        hintType = INTERRUPT_HINT_PAUSE;
        activeIter->second  = PAUSE;
        AUDIO_INFO_LOG("Update hintType for whiteList or pauseWhenInterruptFlag");
    }
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, hintType, 1.0f};
    AUDIO_INFO_LOG("NotifyStreamSilentChange:streamId %{public}d is stopped by streamId: %{public}d",
        activeInterrupt.sessionId, incomingInterrupt.sessionId);
    StartPausedStreamMonitor(activeZoneId, interruptEvent, incomingInterrupt, activeInterrupt);
    SendInterruptEventCallback(interruptEvent, activeInterrupt.sessionId, activeInterrupt);
    if (hintType == INTERRUPT_HINT_STOP) {
        DeactivateAudioInterruptInternal(activeZoneId, activeInterrupt);
    }
    itZone->second->audioFocusInfoList = audioFocusInfoList;
    zonesMap_[activeZoneId] = itZone->second;
}

bool AudioInterruptService::NotifyStreamSilentChange(int32_t callerPid, uint32_t streamId)
{
    bool ret = false;
    if (interruptDfx_ != nullptr) {
        interruptDfx_->NotifyStreamSilentChange(streamId);
    }
    std::lock_guard<std::mutex> lock(mutex_);
    UpdateIsCheckMuteStreamByPid(callerPid);
    std::list<MuteFocusInfo> tempMuteList;
    if (muteAudioFocus_.count(callerPid) > 0) {
        tempMuteList = muteAudioFocus_[callerPid];
        muteAudioFocus_.erase(callerPid);
        AUDIO_INFO_LOG("pid: %{public}d has been removed from muteAudioFocus_ record, "
            "now record size: %{public}zu", callerPid, muteAudioFocus_.size());
    }
    for (const auto& audioFocusInfo : tempMuteList) {
        ProcessSilentChangeInterrupt(audioFocusInfo);
        ret = true;
    }
    return ret;
}

void AudioInterruptService::RemoveMuteCheckRecord(const AudioInterrupt &audioInterrupt)
{
    auto it = muteAudioFocus_.find(audioInterrupt.pid);
    if (it == muteAudioFocus_.end()) {
        return;
    }

    auto& focusList = it->second;
    auto targetStreamId = audioInterrupt.streamId;

    focusList.remove_if([targetStreamId](const MuteFocusInfo& info) {
        return info.incomingStreamId == targetStreamId;
    });

    if (focusList.empty()) {
        muteAudioFocus_.erase(it);
        AUDIO_INFO_LOG("pid: %{public}d has been removed from muteAudioFocus_ record, "
            "now record size: %{public}zu", audioInterrupt.pid, muteAudioFocus_.size());
    }
}

void AudioInterruptService::MuteCheckFocusStrategy(AudioFocusEntry& focusEntry,
    const std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo, const AudioInterrupt &incomingInterrupt)
{
    CHECK_AND_RETURN(IsAudioStreamActive(audioFocusInfo.second) ||
        audioFocusInfo.first.behaviorFlags_.pauseWhenInterruptFlag);
    if (!incomingInterrupt.isCheckMuteStream) {
        AUDIO_INFO_LOG("is not check mute stream");
        return;
    }

    AudioInterrupt currentInterrupt = audioFocusInfo.first;
    AudioStreamType currentStreamType = currentInterrupt.audioFocusType.streamType;
    AudioStreamType incomingStreamType = incomingInterrupt.audioFocusType.streamType;
    if ((focusEntry.hintType != INTERRUPT_HINT_STOP &&
        focusEntry.hintType != INTERRUPT_HINT_PAUSE &&
        focusEntry.hintType != INTERRUPT_HINT_MUTE) ||
        !AudioInterruptUtils::IsMediaStream(currentStreamType) ||
        !AudioInterruptUtils::IsMediaStream(incomingStreamType)) {
        AUDIO_INFO_LOG("focusEntry.hintType = %{public}d", focusEntry.hintType);
        return;
    }

    auto& focusList = muteAudioFocus_[incomingInterrupt.pid];
    bool exists = std::any_of(focusList.begin(), focusList.end(), [&](const MuteFocusInfo& info) {
        return info.currStreamId == currentInterrupt.streamId &&
               info.incomingStreamId == incomingInterrupt.streamId;
    });
    if (!exists) {
        MuteFocusInfo muteFocusInfo;
        muteFocusInfo.currStreamId = currentInterrupt.streamId;
        muteFocusInfo.incomingStreamId = incomingInterrupt.streamId;
        muteFocusInfo.currInterrupt = currentInterrupt;
        muteFocusInfo.incomingInterrupt = incomingInterrupt;
        muteFocusInfo.focusEntry = focusEntry;
        focusList.emplace_back(muteFocusInfo);
    } else {
        AUDIO_INFO_LOG("current %{public}d is already recorded with incoming %{public}d",
            currentInterrupt.streamId, incomingInterrupt.streamId);
    }
    focusEntry.hintType = INTERRUPT_HINT_NONE;
    AUDIO_INFO_LOG("%{public}d update muteCheck focusStrategy", incomingInterrupt.streamId);
}

void AudioInterruptService::ResetInterruptMuteState(const AudioInterrupt &audioInterrupt)
{
    uint32_t sessionId = audioInterrupt.streamId;
    InterruptEventInternal forceUnmute {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_UNMUTE, 1.0f};
    SendInterruptEventCallback(forceUnmute, sessionId, audioInterrupt);
    AudioServerProxy::GetInstance().SetLatestMuteState(sessionId, 0);
    streamCollector_.CapturerMutedFlagChange(sessionId, 0);
    CHECK_AND_RETURN_LOG(policyServer_ != nullptr, "policyServer_ is nullptr");
}

int32_t AudioInterruptService::SetAudioSessionBehavior(const int32_t callerPid, const uint32_t behavior,
    int32_t callerUid)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return sessionService_.SetAudioSessionBehavior(callerPid, behavior, callerUid);
}

void AudioInterruptService::HandleMuteInterruptEvent(const AudioInterrupt &audioInterrupt, bool muteFlag)
{
    if (audioInterrupt.isAudioSessionInterrupt) {
        AUDIO_INFO_LOG("Handle all streams in the session of pid:%{public}d, mute:%{public}d",
            audioInterrupt.pid, muteFlag);
        const auto &interruptsInSessionPair = sessionService_.GetStreams(audioInterrupt.pid);
        for (const auto &[interrupt, state] : interruptsInSessionPair) {
            SetNonInterruptMute(interrupt.streamId, muteFlag);
        }
    } else if (audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID) {
        SetLatestMuteState(audioInterrupt, muteFlag);
    } else {
        SetNonInterruptMute(audioInterrupt.streamId, muteFlag);
    }
}

bool AudioInterruptService::IsStreamActive(uint32_t streamId)
{
    std::unique_lock<std::mutex> lock(mutex_);
    for (const auto &zonePair : zonesMap_) {
        if (zonePair.second == nullptr) {
            continue;
        }
        for (const auto &iter : zonePair.second->audioFocusInfoList) {
            if (iter.first.streamId == streamId) {
                return IsAudioStreamActive(iter.second);
            }
        }
    }
    return false;
}

void AudioInterruptService::SetStreamInterruptMute(const InterruptEventInternal &interruptEvent,
    const uint32_t &streamId, const AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN_LOG(audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID, "not capturer stream");
    CHECK_AND_RETURN_LOG(policyServer_ != nullptr, "policyServer_ is nullptr");
    if (interruptEvent.hintType == INTERRUPT_HINT_PAUSE || interruptEvent.hintType == INTERRUPT_HINT_STOP) {
        policyServer_->SetStreamInterruptMute(streamId, true);
    } else if (interruptEvent.hintType == INTERRUPT_HINT_RESUME) {
        policyServer_->SetStreamInterruptMute(streamId, false);
    }
}

bool AudioInterruptService::IsIndependentStrategyValid(const AudioInterrupt &interrupt)
{
    auto concurrencyMode = interrupt.independentStrategy.concurrencyMode;
    return concurrencyMode != AudioConcurrencyMode::INVALID &&
        concurrencyMode != AudioConcurrencyMode::DEFAULT;
}

static bool IsResponseCheckSupported(InterruptHint hintType)
{
    return hintType == INTERRUPT_HINT_RESUME || hintType == INTERRUPT_HINT_STOP ||
        hintType == INTERRUPT_HINT_PAUSE || hintType == INTERRUPT_HINT_DUCK ||
        hintType == INTERRUPT_HINT_MUTE;
}

void AudioInterruptService::PostInterruptResponseCheckAction(
    const InterruptEventInternal &interruptEvent, const AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN_LOG(!IsGameAvoidCallbackCase(audioInterrupt), "game app skip interrupt response check");
    CHECK_AND_RETURN_LOG(IsResponseCheckSupported(interruptEvent.hintType),
        "hintType %{public}d not in response check range", interruptEvent.hintType);

    InterruptResponseCheckInfo info;
    info.hintType = interruptEvent.hintType;
    info.streamId = audioInterrupt.streamId;
    info.pid = audioInterrupt.pid;
    info.streamType = audioInterrupt.audioFocusType.streamType;
    info.sourceType = audioInterrupt.audioFocusType.sourceType;
    info.appName = AudioInterruptUtils::GetAudioInterruptBundleName(audioInterrupt);

    CHECK_AND_RETURN_LOG(asyncHandler_ != nullptr, "asyncHandler is nullptr");
    int32_t zoneId = FindAudioZoneByStreamIdInternal(audioInterrupt.streamId);
    auto action = std::make_shared<InterruptResponseCheckAction>(
        shared_from_this(), zoneId, std::move(info));
    AsyncActionHandler::AsyncActionDesc desc;
    desc.action = std::static_pointer_cast<AsyncActionHandler::AsyncAction>(action);
    desc.delayTimeMs = INTERRUPT_RESPONSE_CHECK_DELAY;
    desc.priority = AsyncActionHandler::ActionPriority::LOW;
    asyncHandler_->PostAsyncAction(desc);
    AUDIO_INFO_LOG("PostInterruptResponseCheckAction for streamId %{public}u zoneId %{public}d",
        info.streamId, zoneId);
}

void AudioInterruptService::CheckInterruptResponseAfterDelay(const int32_t zoneId,
    const InterruptResponseCheckInfo &info)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> currentFocusList;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto currZone = zonesMap_.find(zoneId);
        if (currZone != zonesMap_.end() && currZone->second != nullptr) {
            currentFocusList = currZone->second->audioFocusInfoList;
        }
    }
    CHECK_AND_RETURN(interruptDfx_ != nullptr);
    interruptDfx_->CheckInterruptResponse(info, currentFocusList);
}

int32_t AudioInterruptService::SetDuckPassthroughMode(bool enable)
{
    std::lock_guard<std::mutex> lock(mutex_);
    duckPassthroughMode_.store(enable);
    AUDIO_INFO_LOG("SetDuckPassthroughMode: %{public}d", enable);
    return SUCCESS;
}

void AudioInterruptService::UpdateAudioFocusCheckMuteFlag(const int32_t zoneId, AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN(AudioInterruptUtils::IsMediaStream(audioInterrupt.audioFocusType.streamType));
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end() && itZone->second != nullptr,
        "zone not found or zone is null");
    auto audioFocusInfoList = itZone->second->audioFocusInfoList;
    auto sameFocusExists = std::any_of(audioFocusInfoList.begin(), audioFocusInfoList.end(),
        [&](const std::pair<AudioInterrupt, AudioFocuState>& info) {
        return info.first.pid == audioInterrupt.pid &&
            info.first.audioFocusType.streamType == audioInterrupt.audioFocusType.streamType &&
            !info.first.isCheckMuteStream;
        });
    if (audioInterrupt.isCheckMuteStream && sameFocusExists) {
        AUDIO_INFO_LOG("Same app has same-type stream, update isCheckMuteStream flag");
        audioInterrupt.isCheckMuteStream = false;
    }
}

void AudioInterruptService::HandlePausedTimeout(int32_t zoneId, uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pauseStreamMonitors_.find(streamId);
    CHECK_AND_RETURN_LOG(it != pauseStreamMonitors_.end(),
        "No pause monitor record for streamId %{public}u", streamId);

    pauseStreamMonitors_.erase(it);
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end() && itZone->second != nullptr,
        "zone not found or zone is null");
    auto audioFocusInfoList = itZone->second->audioFocusInfoList;
    auto isPresent = [streamId] (const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.streamId == streamId;
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isPresent);
    CHECK_AND_RETURN_LOG(iter != audioFocusInfoList.end() && iter->second == PAUSE,
        "streamId %{public}u not found or not in PAUSE state", streamId);
    auto audioInterrupt = iter->first;

    if (sessionService_.IsAudioSessionActivated(audioInterrupt.pid)) {
        if (HandleLowPriorityEvent(audioInterrupt.pid, audioInterrupt.streamId)) {
            RemovePlaceholderInterruptForSession(audioInterrupt.pid);
        }
    }
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
    SendInterruptEventCallback(interruptEvent, audioInterrupt.streamId, audioInterrupt);
    DeactivateAudioInterruptInternal(zoneId, audioInterrupt);
    AUDIO_INFO_LOG("Stream %{public}d paused for timeout. Triggering stop callback.",
        audioInterrupt.streamId);
}

void AudioInterruptService::StopPausedStreamMonitor(uint32_t streamId)
{
    auto it = pauseStreamMonitors_.find(streamId);
    CHECK_AND_RETURN(it != pauseStreamMonitors_.end() && it->second != nullptr);
    AUDIO_INFO_LOG("Stop pause monitor for streamId %{public}u", streamId);
    it->second->StopMonitor();
    pauseStreamMonitors_.erase(it);
}

void AudioInterruptService::StartPausedStreamMonitor(const int32_t zoneId,
    const InterruptEventInternal &interruptEvent, const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt)
{
    bool isIncomingBundleNameExist = false;
    int32_t pauseTimeout = DEFAULT_PAUSE_TIMEOUT_S;
    if (interruptStrategy_ != nullptr) {
        std::string incomingBundleName = AudioInterruptUtils::GetAudioInterruptBundleName(incomingInterrupt);
        interruptStrategy_->OnQueryBundleNameIsInList(incomingBundleName, "audio_param",
            isIncomingBundleNameExist);
        pauseTimeout = interruptStrategy_->OnQueryPausedTime();
        CHECK_AND_RETURN(pauseTimeout > 0);
    }
    bool isPausedForBehavior = interruptEvent.hintType == INTERRUPT_HINT_PAUSE &&
        activeInterrupt.behaviorFlags_.pauseWhenInterruptFlag &&
        AudioInterruptUtils::IsMediaStream(activeInterrupt.audioFocusType.streamType) &&
        AudioInterruptUtils::IsMediaStream(incomingInterrupt.audioFocusType.streamType) &&
        sessionService_.GetSessionStrategy(incomingInterrupt.pid) != AudioConcurrencyMode::PAUSE_OTHERS &&
        incomingInterrupt.independentStrategy.concurrencyMode != AudioConcurrencyMode::PAUSE_OTHERS &&
        !isIncomingBundleNameExist;
    CHECK_AND_RETURN(isPausedForBehavior);

    uint32_t streamId = activeInterrupt.streamId;
    auto it = pauseStreamMonitors_.find(streamId);
    CHECK_AND_RETURN_LOG(it == pauseStreamMonitors_.end(),
        "streamId %{public}u already has a pause monitor", streamId);

    auto monitor = std::make_shared<PausedStreamTimeoutMonitor>(zoneId, streamId, shared_from_this());
    monitor->StartMonitor(pauseTimeout, monitor);
    pauseStreamMonitors_[streamId] = monitor;
    AUDIO_INFO_LOG("streamId %{public}u pause monitor started, duration %{public}d s",
        streamId, pauseTimeout);
}
}
} // namespace OHOS
