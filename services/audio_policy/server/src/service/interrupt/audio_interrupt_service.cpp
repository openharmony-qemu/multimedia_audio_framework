/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "audio_utils.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
const int32_t DEFAULT_ZONE_ID = 0;
constexpr uint32_t MEDIA_SA_UID = 1013;
static sptr<IStandardAudioService> g_adProxy = nullptr;

static const map<InterruptHint, AudioFocuState> HINT_STATE_MAP = {
    {INTERRUPT_HINT_PAUSE, PAUSE},
    {INTERRUPT_HINT_DUCK, DUCK},
    {INTERRUPT_HINT_NONE, ACTIVE},
    {INTERRUPT_HINT_RESUME, ACTIVE},
    {INTERRUPT_HINT_UNDUCK, ACTIVE}
};

inline AudioScene GetAudioSceneFromAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    if (audioInterrupt.audioFocusType.streamType == STREAM_RING) {
        return AUDIO_SCENE_RINGING;
    } else if (audioInterrupt.audioFocusType.streamType == STREAM_VOICE_CALL ||
               audioInterrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION) {
        return audioInterrupt.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ?
            AUDIO_SCENE_PHONE_CALL : AUDIO_SCENE_PHONE_CHAT;
    } else if (audioInterrupt.audioFocusType.streamType == STREAM_VOICE_RING) {
        return AUDIO_SCENE_VOICE_RINGING;
    }
    return AUDIO_SCENE_DEFAULT;
}

static const std::unordered_map<const AudioScene, const int> SCENE_PRIORITY = {
    // from high to low
    {AUDIO_SCENE_PHONE_CALL, 5},
    {AUDIO_SCENE_VOICE_RINGING, 4},
    {AUDIO_SCENE_PHONE_CHAT, 3},
    {AUDIO_SCENE_RINGING, 2},
    {AUDIO_SCENE_DEFAULT, 1}
};

static const unordered_map<AudioStreamType, int> DEFAULT_STREAM_PRIORITY = {
    {STREAM_VOICE_CALL, 0},
    {STREAM_VOICE_COMMUNICATION, 0},
    {STREAM_VOICE_MESSAGE, 1},
    {STREAM_NOTIFICATION, 2},
    {STREAM_VOICE_ASSISTANT, 3},
    {STREAM_RING, 4},
    {STREAM_VOICE_RING, 4},
    {STREAM_ALARM, 5},
    {STREAM_NAVIGATION, 6},
    {STREAM_MUSIC, 7},
    {STREAM_MOVIE, 7},
    {STREAM_SPEECH, 7},
    {STREAM_GAME, 7},
    {STREAM_DTMF, 8},
    {STREAM_SYSTEM, 8},
    {STREAM_SYSTEM_ENFORCED, 9},
};

inline int GetAudioScenePriority(const AudioScene audioScene)
{
    if (SCENE_PRIORITY.count(audioScene) == 0) {
        return SCENE_PRIORITY.at(AUDIO_SCENE_DEFAULT);
    }
    return SCENE_PRIORITY.at(audioScene);
}

AudioInterruptService::AudioInterruptService()
{
}

AudioInterruptService::~AudioInterruptService()
{
    AUDIO_ERR_LOG("should not happen");
}

void AudioInterruptService::Init(sptr<AudioPolicyServer> server)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // load configuration
    std::unique_ptr<AudioFocusParser> parser = make_unique<AudioFocusParser>();
    if (parser == nullptr) {
        WriteServiceStartupError();
    }

    int32_t ret = parser->LoadConfig(focusCfgMap_);
    if (ret != SUCCESS) {
        WriteServiceStartupError();
    }
    CHECK_AND_RETURN_LOG(!ret, "load fail");

    AUDIO_DEBUG_LOG("configuration loaded. mapSize: %{public}zu", focusCfgMap_.size());

    policyServer_ = server;
    clientOnFocus_ = 0;
    focussedAudioInterruptInfo_ = nullptr;

    CreateAudioInterruptZoneInternal(ZONEID_DEFAULT, {});

    sessionService_ = std::make_shared<AudioSessionService>();
    sessionService_->Init();
    sessionService_->SetSessionTimeOutCallback(shared_from_this());
}

const sptr<IStandardAudioService> AudioInterruptService::GetAudioServerProxy()
{
    lock_guard<mutex> lock(audioServerProxyMutex_);

    if (g_adProxy == nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "Get samgr failed.");

        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr,
            "audio service remote object is NULL.");

        g_adProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(g_adProxy != nullptr, nullptr,
            "init g_adProxy is NULL.");
    }
    const sptr<IStandardAudioService> gsp = g_adProxy;
    return gsp;
}

void AudioInterruptService::OnSessionTimeout(const int32_t pid)
{
    AUDIO_INFO_LOG("OnSessionTimeout pid %{public}d", pid);
    std::lock_guard<std::mutex> lock(mutex_);
    HandleSessionTimeOutEvent(pid);
}

void AudioInterruptService::HandleSessionTimeOutEvent(const int32_t pid)
{
    RemovePlaceholderInterruptForSession(pid, true);

    AudioSessionDeactiveEvent deactiveEvent;
    deactiveEvent.deactiveReason = AudioSessionDeactiveReason::TIMEOUT;
    std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair = {pid, deactiveEvent};
    if (handler_ != nullptr) {
        AUDIO_INFO_LOG("AudioSessionService::handler_ is not null. Send event!");
        handler_->SendAudioSessionDeactiveCallback(sessionDeactivePair);
    }
}

int32_t AudioInterruptService::ActivateAudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }
    int32_t result = sessionService_->ActivateAudioSession(callerPid, strategy);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("Failed to activate audio session for pid %{public}d!", callerPid);
        return result;
    }

    AddActiveInterruptToSession(callerPid);
    return SUCCESS;
}

void AudioInterruptService::AddActiveInterruptToSession(const int32_t callerPid)
{
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return;
    }
    if (!sessionService_->IsAudioSessionActivated(callerPid)) {
        AUDIO_ERR_LOG("The audio session for pid %{public}d is not active!", callerPid);
        return;
    }
    auto audioSession = sessionService_->GetAudioSessionByPid(callerPid);

    auto itZone = zonesMap_.find(DEFAULT_ZONE_ID);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end(), "can not find zoneid");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }
    for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
        if ((iterActive->first).pid == callerPid && audioSession != nullptr) {
            audioSession->AddAudioInterrpt(*iterActive);
        }
    }
}

int32_t AudioInterruptService::DeactivateAudioSession(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    int32_t result = sessionService_->DeactivateAudioSession(callerPid);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("Failed to deactivate audio session for pid %{public}d!", callerPid);
        return result;
    }

    RemovePlaceholderInterruptForSession(callerPid);
    return SUCCESS;
}

void AudioInterruptService::RemovePlaceholderInterruptForSession(const int32_t callerPid, bool isSessionTimeout)
{
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return;
    }
    if (sessionService_->IsAudioSessionActivated(callerPid)) {
        AUDIO_ERR_LOG("The audio session for pid %{public}d is still active!", callerPid);
        return;
    }

    auto itZone = zonesMap_.find(DEFAULT_ZONE_ID);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end(), "can not find zoneid");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    for (auto iter = audioFocusInfoList.begin(); iter != audioFocusInfoList.end(); ++iter) {
        if (iter->first.pid == callerPid && iter->second == PLACEHOLDER) {
            AudioInterrupt placeholder = iter->first;
            AUDIO_INFO_LOG("Remove stream id %{public}u (placeholder for pid%{public}d)",
                placeholder.sessionId, callerPid);
            DeactivateAudioInterruptInternal(DEFAULT_ZONE_ID, placeholder, isSessionTimeout);
        }
    }
}

bool AudioInterruptService::IsAudioSessionActivated(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }
    return sessionService_->IsAudioSessionActivated(callerPid);
}

bool AudioInterruptService::IsCanMixInterrupt(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt)
{
    if (incomingInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID &&
        (activeInterrupt.audioFocusType.streamType == STREAM_VOICE_CALL ||
        activeInterrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION)) {
        AUDIO_INFO_LOG("The capturer can not mix with voice call");
        return false;
    }
    if ((incomingInterrupt.audioFocusType.streamType == STREAM_VOICE_CALL ||
        incomingInterrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION) &&
        activeInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID) {
        AUDIO_INFO_LOG("The voice call can not mix with capturer");
        return false;
    }
    if (incomingInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID &&
        activeInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID) {
        AUDIO_INFO_LOG("The capturer can not mix with another capturer");
        return false;
    }
    return true;
}

bool AudioInterruptService::CanMixForSession(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt, const AudioFocusEntry &focusEntry)
{
    if (focusEntry.isReject && incomingInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID) {
        // The incoming stream is a capturer and the default policy is deny incoming.
        AUDIO_INFO_LOG("The incoming audio capturer should be denied!");
        return false;
    }
    if (!IsCanMixInterrupt(incomingInterrupt, activeInterrupt)) {
        AUDIO_INFO_LOG("Two Stream Cannot Mix! incoming=%{public}d, active=%{public}d",
            incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
        return false;
    }
    if (incomingInterrupt.audioFocusType.streamType == STREAM_INTERNAL_FORCE_STOP ||
        activeInterrupt.audioFocusType.streamType == STREAM_INTERNAL_FORCE_STOP) {
        AUDIO_INFO_LOG("STREAM_INTERNAL_FORCE_STOP! incomingInterrupt=%{public}d, activeInterrupt=%{public}d",
            incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
        return false;
    }
    bool result = false;
    result = CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    if (result) {
        AUDIO_INFO_LOG("Two streams can mix because of the incoming session. incoming %{public}d, active %{public}d",
            incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
        return result;
    }
    result = CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    if (result) {
        AUDIO_INFO_LOG("Two streams can mix because of the active session. incoming %{public}d, active %{public}d",
            incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
        return result;
    }
    AUDIO_INFO_LOG("Two streams can not mix. incoming %{public}d, active %{public}d",
        incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
    return result;
}

bool AudioInterruptService::CanMixForIncomingSession(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt, const AudioFocusEntry &focusEntry)
{
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return false;
    }
    if (incomingInterrupt.sessionStrategy.concurrencyMode == AudioConcurrencyMode::SLIENT) {
        AUDIO_INFO_LOG("incoming stream is explicitly SLIENT");
        return true;
    }
    if (incomingInterrupt.sessionStrategy.concurrencyMode == AudioConcurrencyMode::MIX_WITH_OTHERS) {
        AUDIO_INFO_LOG("incoming stream is explicitly MIX_WITH_OTHERS");
        return true;
    }
    if (!sessionService_->IsAudioSessionActivated(incomingInterrupt.pid)) {
        AUDIO_INFO_LOG("No active audio session for the pid of incomming stream");
        return false;
    }

    std::shared_ptr<AudioSession> incomingSession = sessionService_->GetAudioSessionByPid(incomingInterrupt.pid);
    if (incomingSession == nullptr) {
        AUDIO_ERR_LOG("incomingSession is nullptr!");
        return false;
    }
    AudioConcurrencyMode concurrencyMode = (incomingSession->GetSessionStrategy()).concurrencyMode;
    if (concurrencyMode != AudioConcurrencyMode::MIX_WITH_OTHERS) {
        AUDIO_INFO_LOG("The concurrency mode of incoming session is not MIX_WITH_OTHERS");
        return false;
    }

    if (IsIncomingStreamLowPriority(focusEntry)) {
        bool isSameType = AudioSessionService::IsSameTypeForAudioSession(
            incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
        AUDIO_INFO_LOG("The incoming stream is low priority. isSameType: %{public}d.", isSameType);
        return isSameType;
    } else {
        AUDIO_INFO_LOG("The concurrency mode of incoming session is MIX_WITH_OTHERS. Skip the interrupt operation");
        return true;
    }
}

bool AudioInterruptService::CanMixForActiveSession(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt, const AudioFocusEntry &focusEntry)
{
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return false;
    }
    if (activeInterrupt.sessionStrategy.concurrencyMode == AudioConcurrencyMode::SLIENT) {
        AUDIO_INFO_LOG("The concurrency mode of active session is SLIENT");
        return true;
    }
    if (!sessionService_->IsAudioSessionActivated(activeInterrupt.pid)) {
        AUDIO_INFO_LOG("No active audio session for the pid of active stream");
        return false;
    }

    std::shared_ptr<AudioSession> activeSession = sessionService_->GetAudioSessionByPid(activeInterrupt.pid);
    if (activeSession == nullptr) {
        AUDIO_ERR_LOG("activeSession is nullptr!");
        return false;
    }
    AudioConcurrencyMode concurrencyMode = (activeSession->GetSessionStrategy()).concurrencyMode;
    if (concurrencyMode != AudioConcurrencyMode::MIX_WITH_OTHERS) {
        AUDIO_INFO_LOG("The concurrency mode of active session is not MIX_WITH_OTHERS");
        return false;
    }

    if (IsActiveStreamLowPriority(focusEntry)) {
        bool isSameType = AudioSessionService::IsSameTypeForAudioSession(
            incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
        AUDIO_INFO_LOG("The active stream is low priority. isSameType: %{public}d.", isSameType);
        return isSameType;
    } else {
        AUDIO_INFO_LOG("The concurrency mode of active session is MIX_WITH_OTHERS. Skip the interrupt operation");
        return true;
    }
}

bool AudioInterruptService::IsIncomingStreamLowPriority(const AudioFocusEntry &focusEntry)
{
    if (focusEntry.isReject) {
        return true;
    }
    if (focusEntry.actionOn == INCOMING) {
        if (focusEntry.hintType == INTERRUPT_HINT_PAUSE ||
            focusEntry.hintType == INTERRUPT_HINT_STOP ||
            focusEntry.hintType == INTERRUPT_HINT_DUCK) {
            return true;
        }
    }
    return false;
}

bool AudioInterruptService::IsActiveStreamLowPriority(const AudioFocusEntry &focusEntry)
{
    if (focusEntry.actionOn == CURRENT) {
        if (focusEntry.hintType == INTERRUPT_HINT_PAUSE ||
            focusEntry.hintType == INTERRUPT_HINT_STOP ||
            focusEntry.hintType == INTERRUPT_HINT_DUCK) {
            return true;
        }
    }
    return false;
}

void AudioInterruptService::WriteServiceStartupError()
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_SERVICE_STARTUP_ERROR,
        Media::MediaMonitor::FAULT_EVENT);
    bean->Add("SERVICE_ID", static_cast<int32_t>(Media::MediaMonitor::AUDIO_POLICY_SERVICE_ID));
    bean->Add("ERROR_CODE", static_cast<int32_t>(Media::MediaMonitor::AUDIO_INTERRUPT_SERVER));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioInterruptService::AddDumpInfo(std::unordered_map<int32_t, std::shared_ptr<AudioInterruptZone>>
    &audioInterruptZonesMapDump)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto&[zoneId, audioInterruptZone] : zonesMap_) {
        std::shared_ptr<AudioInterruptZone> zoneDump = make_shared<AudioInterruptZone>();
        zoneDump->zoneId = zoneId;
        zoneDump->pids = audioInterruptZone->pids;
        for (auto interruptCbInfo : audioInterruptZone->interruptCbsMap) {
            zoneDump->interruptCbSessionIdsMap.insert(interruptCbInfo.first);
        }
        for (auto audioPolicyClientProxyCBInfo : audioInterruptZone->audioPolicyClientProxyCBMap) {
            zoneDump->audioPolicyClientProxyCBClientPidMap.insert(audioPolicyClientProxyCBInfo.first);
        }
        zoneDump->audioFocusInfoList = audioInterruptZone->audioFocusInfoList;
        audioInterruptZonesMapDump[zoneId] = zoneDump;
    }
}

void AudioInterruptService::SetCallbackHandler(std::shared_ptr<AudioPolicyServerHandler> handler)
{
    handler_ = handler;
}

int32_t AudioInterruptService::SetAudioManagerInterruptCallback(const sptr<IRemoteObject> &object)
{
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM,
        "object is nullptr");

    sptr<IStandardAudioPolicyManagerListener> listener = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM,
        "obj cast failed");

    std::shared_ptr<AudioInterruptCallback> callback = std::make_shared<AudioPolicyManagerListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        "create cb failed");

    int32_t callerPid = IPCSkeleton::GetCallingPid();

    if (handler_ != nullptr) {
        handler_->AddExternInterruptCbsMap(callerPid, callback);
    }

    AUDIO_DEBUG_LOG("for client id %{public}d done", callerPid);

    return SUCCESS;
}

int32_t AudioInterruptService::UnsetAudioManagerInterruptCallback()
{
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    if (handler_ != nullptr) {
        return handler_->RemoveExternInterruptCbsMap(callerPid);
    }

    return SUCCESS;
}

int32_t AudioInterruptService::RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(mutex_);

    if (clientOnFocus_ == clientId) {
        AUDIO_INFO_LOG("client already has focus");
        NotifyFocusGranted(clientId, audioInterrupt);
        return SUCCESS;
    }

    if (focussedAudioInterruptInfo_ != nullptr) {
        AUDIO_INFO_LOG("Existing stream: %{public}d, incoming stream: %{public}d",
            (focussedAudioInterruptInfo_->audioFocusType).streamType, audioInterrupt.audioFocusType.streamType);
        NotifyFocusAbandoned(clientOnFocus_, *focussedAudioInterruptInfo_);
        AbandonAudioFocusInternal(clientOnFocus_, *focussedAudioInterruptInfo_);
    }

    NotifyFocusGranted(clientId, audioInterrupt);

    return SUCCESS;
}

int32_t AudioInterruptService::AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("in");
    std::lock_guard<std::mutex> lock(mutex_);

    return AbandonAudioFocusInternal(clientId, audioInterrupt);
}

int32_t AudioInterruptService::SetAudioInterruptCallback(const int32_t zoneId, const uint32_t sessionId,
    const sptr<IRemoteObject> &object, uint32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // maybe add check session id validation here

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "object is nullptr");

    sptr<IStandardAudioPolicyManagerListener> listener = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "obj cast failed");

    std::shared_ptr<AudioInterruptCallback> callback = std::make_shared<AudioPolicyManagerListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "create cb failed");

    if (interruptClients_.find(sessionId) == interruptClients_.end()) {
        // Register client death recipient first
        sptr<AudioInterruptDeathRecipient> deathRecipient =
            new AudioInterruptDeathRecipient(shared_from_this(), sessionId);
        object->AddDeathRecipient(deathRecipient);

        std::shared_ptr<AudioInterruptClient> client =
            std::make_shared<AudioInterruptClient>(callback, object, deathRecipient);
        uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
        if (callingUid == MEDIA_SA_UID) {
            callingUid = uid;
        }
        client->SetCallingUid(callingUid);

        interruptClients_[sessionId] = client;

        // just record in zone map, not used currently
        auto it = zonesMap_.find(zoneId);
        if (it != zonesMap_.end() && it->second != nullptr) {
            it->second->interruptCbsMap[sessionId] = callback;
            zonesMap_[zoneId] = it->second;
        }
    } else {
        AUDIO_ERR_LOG("%{public}u callback already exist", sessionId);
        return ERR_INVALID_PARAM;
    }

    return SUCCESS;
}

int32_t AudioInterruptService::UnsetAudioInterruptCallback(const int32_t zoneId, const uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (interruptClients_.erase(sessionId) == 0) {
        AUDIO_ERR_LOG("session %{public}u not present", sessionId);
        return ERR_INVALID_PARAM;
    }

    auto it = zonesMap_.find(zoneId);
    if (it != zonesMap_.end() && it->second != nullptr &&
        it->second->interruptCbsMap.find(sessionId) != it->second->interruptCbsMap.end()) {
        it->second->interruptCbsMap.erase(it->second->interruptCbsMap.find(sessionId));
        zonesMap_[zoneId] = it->second;
    }

    return SUCCESS;
}

bool AudioInterruptService::AudioInterruptIsActiveInFocusList(const int32_t zoneId, const uint32_t incomingSessionId)
{
    auto itZone = zonesMap_.find(zoneId);
    if (itZone == zonesMap_.end()) {
        AUDIO_ERR_LOG("Can not find zoneid");
        return false;
    }
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end()) { audioFocusInfoList = itZone->second->audioFocusInfoList; }
    auto isPresent = [incomingSessionId] (const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.sessionId == incomingSessionId && pair.second == ACTIVE;
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isPresent);
    if (iter != audioFocusInfoList.end()) {
        return true;
    }
    return false;
}

int32_t AudioInterruptService::ActivateAudioInterrupt(
    const int32_t zoneId, const AudioInterrupt &audioInterrupt, const bool isUpdatedAudioStrategy)
{
    std::unique_lock<std::mutex> lock(mutex_);

    AudioStreamType streamType = audioInterrupt.audioFocusType.streamType;
    uint32_t incomingSessionId = audioInterrupt.sessionId;
    AUDIO_INFO_LOG("sessionId: %{public}u pid: %{public}d streamType: %{public}d "\
        "usage: %{public}d source: %{public}d",
        incomingSessionId, audioInterrupt.pid, streamType,
        audioInterrupt.streamUsage, (audioInterrupt.audioFocusType).sourceType);

    if (AudioInterruptIsActiveInFocusList(zoneId, incomingSessionId) && !isUpdatedAudioStrategy) {
        AUDIO_INFO_LOG("Stream is active in focus list, no need to active audio interrupt.");
        return SUCCESS;
    }

    if (audioInterrupt.parallelPlayFlag) {
        AUDIO_PRERELEASE_LOGI("allow parallel play");
        return SUCCESS;
    }
    ResetNonInterruptControl(incomingSessionId);

    policyServer_->CheckStreamMode(incomingSessionId);
    policyServer_->OffloadStreamCheck(incomingSessionId, OFFLOAD_NO_SESSION_ID);

    bool shouldReturnSuccess = false;
    ProcessAudioScene(audioInterrupt, incomingSessionId, zoneId, shouldReturnSuccess);
    if (shouldReturnSuccess) {
        return SUCCESS;
    }

    // Process ProcessFocusEntryTable for current audioFocusInfoList
    int32_t ret = ProcessFocusEntry(zoneId, audioInterrupt);
    CHECK_AND_RETURN_RET_LOG(!ret, ERR_FOCUS_DENIED, "request rejected");

    AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);

    // If there is an event of (interrupt + set scene), ActivateAudioInterrupt and DeactivateAudioInterrupt may
    // experience deadlocks, due to mutex_ and deviceStatusUpdateSharedMutex_ waiting for each other
    lock.unlock();
    UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT);
    return SUCCESS;
}

void AudioInterruptService::ResetNonInterruptControl(uint32_t sessionId)
{
    if (GetClientTypeBySessionId(sessionId) != CLIENT_TYPE_GAME) {
        return;
    }
    AUDIO_INFO_LOG("Reset non-interrupt control for %{public}u", sessionId);
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "error for audio server proxy null");
    gsp->SetNonInterruptMute(sessionId, false);
    IPCSkeleton::SetCallingIdentity(identity);
}

int32_t AudioInterruptService::DeactivateAudioInterrupt(const int32_t zoneId, const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("sessionId: %{public}u pid: %{public}d streamType: %{public}d "\
        "usage: %{public}d source: %{public}d",
        audioInterrupt.sessionId, audioInterrupt.pid, (audioInterrupt.audioFocusType).streamType,
        audioInterrupt.streamUsage, (audioInterrupt.audioFocusType).sourceType);

    if (audioInterrupt.parallelPlayFlag) {
        AUDIO_PRERELEASE_LOGI("allow parallel play");
        return SUCCESS;
    }

    DeactivateAudioInterruptInternal(zoneId, audioInterrupt);

    return SUCCESS;
}

void AudioInterruptService::ClearAudioFocusInfoListOnAccountsChanged(const int &id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("start DeactivateAudioInterrupt, current id:%{public}d", id);
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
    for (const auto&[zoneId, audioInterruptZone] : zonesMap_) {
        CHECK_AND_CONTINUE_LOG(audioInterruptZone != nullptr, "audioInterruptZone is nullptr");
        std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator it =
            audioInterruptZone->audioFocusInfoList.begin();
        while (it != audioInterruptZone->audioFocusInfoList.end()) {
            if ((*it).first.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
                (*it).first.streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
                AUDIO_INFO_LOG("usage is voice modem communication or voice ring, skip");
                ++it;
            } else {
                CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is nullptr");
                handler_->SendInterruptEventWithSessionIdCallback(interruptEvent, (*it).first.sessionId);
                it = audioInterruptZone->audioFocusInfoList.erase(it);
            }
        }
    }
}

int32_t AudioInterruptService::CreateAudioInterruptZone(const int32_t zoneId, const std::set<int32_t> &pids)
{
    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM, "permission deny");

    return CreateAudioInterruptZoneInternal(zoneId, pids);
}

int32_t AudioInterruptService::ReleaseAudioInterruptZone(const int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
        "permission deny");

    if (zonesMap_.find(zoneId) == zonesMap_.end()) {
        AUDIO_INFO_LOG("no such zone:%{public}d, do not release", zoneId);
        return SUCCESS;
    }

    auto it = zonesMap_.find(zoneId);
    if (it->second == nullptr) {
        zonesMap_.erase(it);
        AUDIO_INFO_LOG("zoneId:(%{public}d) invalid, do not release", zoneId);
        return SUCCESS;
    }
    ArchiveToNewAudioInterruptZone(zoneId, ZONEID_DEFAULT);
    return SUCCESS;
}

int32_t AudioInterruptService::AddAudioInterruptZonePids(const int32_t zoneId, const std::set<int32_t> &pids)
{
    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
        "permission deny");

    bool shouldCreateNew = true;
    auto it = zonesMap_.find(zoneId);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = nullptr;
    if (it != zonesMap_.end()) {
        shouldCreateNew = false;
        audioInterruptZone = it->second;
        if (audioInterruptZone == nullptr) {
            zonesMap_.erase(it);
            shouldCreateNew = true;
        }
    }

    if (shouldCreateNew) {
        CreateAudioInterruptZoneInternal(zoneId, pids);
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(audioInterruptZone != nullptr, ERROR, "Invalid audio interrupt zone.");
    for (int32_t pid : pids) {
        std::pair<set<int32_t>::iterator, bool> ret = audioInterruptZone->pids.insert(pid);
        if (!ret.second) {
            AUDIO_ERR_LOG("Add the same pid:%{public}d, add new pid failed.", pid);
        }
    }

    int32_t hitZoneId;
    HitZoneIdHaveTheSamePidsZone(audioInterruptZone->pids, hitZoneId);

    NewAudioInterruptZoneByPids(audioInterruptZone, audioInterruptZone->pids, zoneId);

    ArchiveToNewAudioInterruptZone(hitZoneId, zoneId);

    return SUCCESS;
}

int32_t AudioInterruptService::RemoveAudioInterruptZonePids(const int32_t zoneId, const std::set<int32_t> &pids)
{
    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_RET_LOG(CheckAudioInterruptZonePermission(), ERR_INVALID_PARAM,
        "permission deny");

    if (zonesMap_.find(zoneId) == zonesMap_.end()) {
        AUDIO_INFO_LOG("no such zone:%{public}d, no need to remove", zoneId);
        return SUCCESS;
    }

    auto it = zonesMap_.find(zoneId);
    if (it->second == nullptr) {
        zonesMap_.erase(it);
        AUDIO_INFO_LOG("zoneId:(%{public}d) invalid, no need to remove", zoneId);
        return SUCCESS;
    }

    for (int32_t pid : pids) {
        auto pidIt = it->second->pids.find(pid);
        if (pidIt != it->second->pids.end()) {
            it->second->pids.erase(pidIt);
        } else {
            AUDIO_ERR_LOG("no pid:%{public}d, no need to remove", pid);
        }

        if (it->second->audioPolicyClientProxyCBMap.find(pid) != it->second->audioPolicyClientProxyCBMap.end()) {
            it->second->audioPolicyClientProxyCBMap.erase(it->second->audioPolicyClientProxyCBMap.find(pid));
        }
    }

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    audioInterruptZone = it->second;
    zonesMap_.insert_or_assign(zoneId, audioInterruptZone);

    ArchiveToNewAudioInterruptZone(zoneId, ZONEID_DEFAULT);
    return SUCCESS;
}

// LCOV_EXCL_START
int32_t AudioInterruptService::GetAudioFocusInfoList(const int32_t zoneId,
    std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto itZone = zonesMap_.find(zoneId);
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        focusInfoList = itZone->second->audioFocusInfoList;
    } else {
        focusInfoList = {};
    }

    return SUCCESS;
}

int32_t AudioInterruptService::GetStreamTypePriority(AudioStreamType streamType)
{
    const std::unordered_map<AudioStreamType, int> &priorityMap = GetStreamPriorityMap();
    if (priorityMap.find(streamType) != priorityMap.end()) {
        return priorityMap.at(streamType);
    }
    return STREAM_DEFAULT_PRIORITY;
}

unordered_map<AudioStreamType, int> AudioInterruptService::GetStreamPriorityMap() const
{
    return DEFAULT_STREAM_PRIORITY;
}

AudioStreamType AudioInterruptService::GetStreamInFocus(const int32_t zoneId)
{
    AudioStreamType streamInFocus = STREAM_DEFAULT;

    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    int32_t focusPriority = STREAM_DEFAULT_PRIORITY;
    for (auto iter = audioFocusInfoList.begin(); iter != audioFocusInfoList.end(); ++iter) {
        if ((iter->second != ACTIVE && iter->second != DUCK) ||
            (iter->first).audioFocusType.sourceType != SOURCE_TYPE_INVALID) {
            // if the steam is not active or the active stream is an audio capturer stream, skip it.
            continue;
        }
        int32_t curPriority = GetStreamTypePriority((iter->first).audioFocusType.streamType);
        if (curPriority < focusPriority) {
            focusPriority = curPriority;
            streamInFocus = (iter->first).audioFocusType.streamType;
        }
    }
    return streamInFocus == STREAM_DEFAULT ? STREAM_MUSIC : streamInFocus;
}

int32_t AudioInterruptService::GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneId)
{
    uint32_t invalidSessionId = static_cast<uint32_t>(-1);
    audioInterrupt = {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN,
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_INVALID, true}, invalidSessionId};

    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    for (auto iter = audioFocusInfoList.begin(); iter != audioFocusInfoList.end(); ++iter) {
        if (iter->second == ACTIVE) {
            audioInterrupt = iter->first;
        }
    }

    return SUCCESS;
}

void AudioInterruptService::NotifyFocusGranted(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("Notify focus granted in: %{public}d", clientId);

    InterruptEventInternal interruptEvent = {};
    interruptEvent.eventType = INTERRUPT_TYPE_END;
    interruptEvent.forceType = INTERRUPT_SHARE;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;
    interruptEvent.duckVolume = 0;

    if (handler_ != nullptr) {
        handler_->SendInterruptEventWithClientIdCallback(interruptEvent, clientId);
        unique_ptr<AudioInterrupt> tempAudioInterruptInfo = make_unique<AudioInterrupt>();
        tempAudioInterruptInfo->streamUsage = audioInterrupt.streamUsage;
        tempAudioInterruptInfo->contentType = audioInterrupt.contentType;
        (tempAudioInterruptInfo->audioFocusType).streamType = audioInterrupt.audioFocusType.streamType;
        tempAudioInterruptInfo->pauseWhenDucked = audioInterrupt.pauseWhenDucked;
        focussedAudioInterruptInfo_ = move(tempAudioInterruptInfo);
        clientOnFocus_ = clientId;
    }
}

int32_t AudioInterruptService::NotifyFocusAbandoned(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    AUDIO_INFO_LOG("Notify focus abandoned in: %{public}d", clientId);

    InterruptEventInternal interruptEvent = {};
    interruptEvent.eventType = INTERRUPT_TYPE_BEGIN;
    interruptEvent.forceType = INTERRUPT_SHARE;
    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    interruptEvent.duckVolume = 0;
    if (handler_ != nullptr) {
        handler_->SendInterruptEventWithClientIdCallback(interruptEvent, clientId);
    }

    return SUCCESS;
}

int32_t AudioInterruptService::AbandonAudioFocusInternal(const int32_t clientId, const AudioInterrupt &audioInterrupt)
{
    if (clientId == clientOnFocus_) {
        AUDIO_INFO_LOG("remove app focus");
        focussedAudioInterruptInfo_.reset();
        focussedAudioInterruptInfo_ = nullptr;
        clientOnFocus_ = 0;
    }

    return SUCCESS;
}

bool AudioInterruptService::IsSameAppInShareMode(const AudioInterrupt incomingInterrupt,
    const AudioInterrupt activeInterrupt)
{
    if (incomingInterrupt.mode != SHARE_MODE || activeInterrupt.mode != SHARE_MODE) {
        return false;
    }
    if (incomingInterrupt.pid == DEFAULT_APP_PID || activeInterrupt.pid == DEFAULT_APP_PID) {
        return false;
    }
    return incomingInterrupt.pid == activeInterrupt.pid;
}

bool AudioInterruptService::CheckAudioSessionExistence(const AudioInterrupt &incomingInterrupt,
    AudioFocusEntry &focusEntry)
{
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return false;
    }
    if (!sessionService_->IsAudioSessionActivated(incomingInterrupt.pid)) {
        AUDIO_INFO_LOG("No active audio session for the pid of incomming stream");
        return false;
    }
    if (focusEntry.actionOn != CURRENT) {
        AUDIO_INFO_LOG("The interrupt event is not for the existed stream.");
        return false;
    }

    std::shared_ptr<AudioSession> incomingSession = sessionService_->GetAudioSessionByPid(incomingInterrupt.pid);
    if (incomingSession == nullptr) {
        AUDIO_ERR_LOG("incomingSession is nullptr!");
        return false;
    }
    return true;
}

void AudioInterruptService::UpdateHintTypeForExistingSession(const AudioInterrupt &incomingInterrupt,
    AudioFocusEntry &focusEntry)
{
    AudioConcurrencyMode concurrencyMode = incomingInterrupt.sessionStrategy.concurrencyMode;
    
    if (CheckAudioSessionExistence(incomingInterrupt, focusEntry)) {
        std::shared_ptr<AudioSession> incomingSession = sessionService_->GetAudioSessionByPid(incomingInterrupt.pid);
        concurrencyMode = (incomingSession->GetSessionStrategy()).concurrencyMode;
    }
    switch (concurrencyMode) {
        case AudioConcurrencyMode::DUCK_OTHERS:
            if (focusEntry.hintType == INTERRUPT_HINT_DUCK ||
                focusEntry.hintType == INTERRUPT_HINT_PAUSE ||
                focusEntry.hintType == INTERRUPT_HINT_STOP) {
                AUDIO_INFO_LOG("The concurrency mode is DUCK_OTHERS. Use INTERRUPT_HINT_DUCK.");
                focusEntry.hintType = INTERRUPT_HINT_DUCK;
            }
            break;
        case AudioConcurrencyMode::PAUSE_OTHERS:
            if (focusEntry.hintType == INTERRUPT_HINT_PAUSE ||
                focusEntry.hintType == INTERRUPT_HINT_STOP) {
                AUDIO_INFO_LOG("The concurrency mode is PAUSE_OTHERS. Use INTERRUPT_HINT_PAUSE.");
                focusEntry.hintType = INTERRUPT_HINT_PAUSE;
            }
            break;
        default:
            AUDIO_INFO_LOG("The concurrency mode is %{public}d. No need to update hint type",
                static_cast<int32_t>(concurrencyMode));
            break;
    }
}

void AudioInterruptService::ProcessExistInterrupt(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator
    &iterActive, AudioFocusEntry &focusEntry, const AudioInterrupt &incomingInterrupt,
    bool &removeFocusInfo, InterruptEventInternal &interruptEvent)
{
    SourceType incomingSourceType = incomingInterrupt.audioFocusType.sourceType;
    std::vector<SourceType> incomingConcurrentSources = incomingInterrupt.currencySources.sourcesTypes;
    SourceType existSourceType = (iterActive->first).audioFocusType.sourceType;
    std::vector<SourceType> existConcurrentSources = (iterActive->first).currencySources.sourcesTypes;

    // if the callerPid has an active audio session, the hint type need to be updated.
    if (IsCanMixInterrupt(incomingInterrupt, iterActive->first)) {
        UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);
    }
    switch (focusEntry.hintType) {
        case INTERRUPT_HINT_STOP:
            if (IsAudioSourceConcurrency(existSourceType, incomingSourceType, existConcurrentSources,
                incomingConcurrentSources)) {
                break;
            }
            interruptEvent.hintType = focusEntry.hintType;
            if (GetClientTypeBySessionId((iterActive->first).sessionId) == CLIENT_TYPE_GAME) {
                interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
                iterActive->second = PAUSE;
                AUDIO_INFO_LOG("incomingInterrupt.hintType: %{public}d", interruptEvent.hintType);
                break;
            }
            removeFocusInfo = true;
            break;
        case INTERRUPT_HINT_PAUSE:
            if (IsAudioSourceConcurrency(existSourceType, incomingSourceType, existConcurrentSources,
                incomingConcurrentSources)) {
                break;
            }
            if (iterActive->second == ACTIVE || iterActive->second == DUCK) {
                iterActive->second = PAUSE;
                interruptEvent.hintType = focusEntry.hintType;
            }
            break;
        case INTERRUPT_HINT_DUCK:
            if (iterActive->second == ACTIVE) {
                iterActive->second = DUCK;
                interruptEvent.duckVolume = DUCK_FACTOR;
                interruptEvent.hintType = focusEntry.hintType;
            }
            break;
        default:
            break;
    }
}

void AudioInterruptService::SwitchHintType(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive,
    InterruptEventInternal &interruptEvent, std::list<std::pair<AudioInterrupt, AudioFocuState>> &tmpFocusInfoList)
{
    switch (interruptEvent.hintType) {
        case INTERRUPT_HINT_STOP:
            if (GetClientTypeBySessionId((iterActive->first).sessionId) == CLIENT_TYPE_GAME) {
                iterActive->second = PAUSEDBYREMOTE;
                break;
            }
            iterActive = tmpFocusInfoList.erase(iterActive);
            break;
        case INTERRUPT_HINT_PAUSE:
            if (iterActive->second == ACTIVE || iterActive->second == DUCK) {
                iterActive->second = PAUSEDBYREMOTE;
            }
            break;
        case INTERRUPT_HINT_RESUME:
            if (iterActive->second == PAUSEDBYREMOTE) {
                iterActive = tmpFocusInfoList.erase(iterActive);
            }
        default:
            break;
    }
    return;
}

void AudioInterruptService::ProcessRemoteInterrupt(std::set<int32_t> sessionIds, InterruptEventInternal interruptEvent)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto targetZoneIt = zonesMap_.find(0);
    CHECK_AND_RETURN_LOG(targetZoneIt != zonesMap_.end(), "can not find zone id");

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList {};
    if (targetZoneIt != zonesMap_.end()) {
        tmpFocusInfoList = targetZoneIt->second->audioFocusInfoList;
        targetZoneIt->second->zoneId = 0;
    }
    for (auto iterActive = tmpFocusInfoList.begin(); iterActive != tmpFocusInfoList.end();) {
        for (auto sessionId : sessionIds) {
            if (sessionId != static_cast<int32_t> (iterActive->first.sessionId)) {
                continue;
            }
            SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
            if (handler_ != nullptr) {
                handler_->SendInterruptEventWithSessionIdCallback(interruptEvent, sessionId);
            }
        }
        ++iterActive;
    }
    targetZoneIt->second->audioFocusInfoList = tmpFocusInfoList;
}

void AudioInterruptService::ProcessActiveInterrupt(const int32_t zoneId, const AudioInterrupt &incomingInterrupt)
{
    // Use local variable to record target focus info list, can be optimized
    auto targetZoneIt = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG(targetZoneIt != zonesMap_.end(), "can not find zone id");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList {};
    if (targetZoneIt != zonesMap_.end()) {
        tmpFocusInfoList = targetZoneIt->second->audioFocusInfoList;
        targetZoneIt->second->zoneId = zoneId;
    }

    for (auto iterActive = tmpFocusInfoList.begin(); iterActive != tmpFocusInfoList.end();) {
        AudioFocusEntry focusEntry =
            focusCfgMap_[std::make_pair((iterActive->first).audioFocusType, incomingInterrupt.audioFocusType)];
        if (focusEntry.actionOn != CURRENT || IsSameAppInShareMode(incomingInterrupt, iterActive->first) ||
            iterActive->second == PLACEHOLDER || CanMixForSession(incomingInterrupt, iterActive->first, focusEntry)) {
            ++iterActive;
            continue;
        }

        InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, focusEntry.forceType, INTERRUPT_HINT_NONE, 1.0f};
        uint32_t activeSessionId = (iterActive->first).sessionId;
        bool removeFocusInfo = false;
        ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt, removeFocusInfo, interruptEvent);
        if (removeFocusInfo) {
            // execute remove from list, iter move to next by erase
            int32_t pidToRemove = (iterActive->first).pid;
            uint32_t streamId = (iterActive->first).sessionId;
            auto pidIt = targetZoneIt->second->pids.find(pidToRemove);
            if (pidIt != targetZoneIt->second->pids.end()) {
                targetZoneIt->second->pids.erase(pidIt);
            }
            iterActive = tmpFocusInfoList.erase(iterActive);
            targetZoneIt->second->audioFocusInfoList = tmpFocusInfoList;
            if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(pidToRemove)) {
                HandleLowPriorityEvent(pidToRemove, streamId);
            }
        } else {
            ++iterActive;
        }

        SendActiveInterruptEvent(activeSessionId, interruptEvent, incomingInterrupt);
    }

    targetZoneIt->second->audioFocusInfoList = tmpFocusInfoList;
    zonesMap_[zoneId] = targetZoneIt->second;
}

void AudioInterruptService::HandleLowPriorityEvent(const int32_t pid, const uint32_t streamId)
{
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return;
    }
    auto audioSession = sessionService_->GetAudioSessionByPid(pid);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("audioSession is nullptr!");
        return;
    }

    audioSession->RemoveAudioInterrptByStreamId(streamId);
    if (audioSession->IsAudioSessionEmpty()) {
        AUDIO_INFO_LOG("The audio session is empty because the last one stream is interruptted!");
        sessionService_->DeactivateAudioSession(pid);

        AudioSessionDeactiveEvent deactiveEvent;
        deactiveEvent.deactiveReason = AudioSessionDeactiveReason::LOW_PRIORITY;
        std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair = {pid, deactiveEvent};
        if (handler_ != nullptr) {
            AUDIO_INFO_LOG("AudioSessionService::handler_ is not null. Send event!");
            handler_->SendAudioSessionDeactiveCallback(sessionDeactivePair);
        }
    }
}

void AudioInterruptService::SendActiveInterruptEvent(const uint32_t activeSessionId,
    const InterruptEventInternal &interruptEvent, const AudioInterrupt &incomingInterrupt)
{
    if (interruptEvent.hintType != INTERRUPT_HINT_NONE) {
        AUDIO_INFO_LOG("OnInterrupt for active sessionId:%{public}d, hintType:%{public}d. By sessionId:%{public}d",
            activeSessionId, interruptEvent.hintType, incomingInterrupt.sessionId);
        if (handler_ != nullptr) {
            handler_->SendInterruptEventWithSessionIdCallback(interruptEvent, activeSessionId);
        }
        // focus remove or state change
        SendFocusChangeEvent(ZONEID_DEFAULT, AudioPolicyServerHandler::NONE_CALLBACK_CATEGORY,
            incomingInterrupt);
    }
}

void AudioInterruptService::ProcessAudioScene(const AudioInterrupt &audioInterrupt, const uint32_t &incomingSessionId,
    const int32_t &zoneId, bool &shouldReturnSuccess)
{
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end(), "can not find zoneId");

    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if ((itZone != zonesMap_.end()) && (itZone->second != nullptr)) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
        itZone->second->zoneId = zoneId;
    }
    int32_t pid = audioInterrupt.pid;
    if (!audioFocusInfoList.empty() && (itZone->second != nullptr)) {
        // If the session is present in audioFocusInfoList and not Capturer, remove and treat it as a new request
        AUDIO_DEBUG_LOG("audioFocusInfoList is not empty, check whether the session is present");
        audioFocusInfoList.remove_if(
            [&audioInterrupt, &pid](const std::pair<AudioInterrupt, AudioFocuState> &audioFocus) {
            return audioFocus.first.sessionId == audioInterrupt.sessionId ||
                (audioFocus.first.pid == pid && audioFocus.second == PLACEHOLDER &&
                audioInterrupt.audioFocusType.sourceType == SOURCE_TYPE_INVALID &&
                audioFocus.first.audioFocusType.streamType != STREAM_VOICE_COMMUNICATION);
        });

        if (itZone->second->pids.find(pid) != itZone->second->pids.end()) {
            itZone->second->pids.erase(itZone->second->pids.find(pid));
        }
        itZone->second->audioFocusInfoList = audioFocusInfoList;
        zonesMap_[zoneId] = itZone->second;
        if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(pid)) {
            std::shared_ptr<AudioSession> session = sessionService_->GetAudioSessionByPid(pid);
            if (session != nullptr) {
                sessionService_->GetAudioSessionByPid(pid)->RemoveAudioInterrptByStreamId(incomingSessionId);
            }
        }
    }
    if (audioFocusInfoList.empty()) {
        // If audioFocusInfoList is empty, directly activate interrupt
        AUDIO_INFO_LOG("audioFocusInfoList is empty, add the session into it directly");
        if (itZone->second != nullptr) {
            itZone->second->pids.insert(pid);
            itZone->second->audioFocusInfoList.emplace_back(std::make_pair(audioInterrupt, ACTIVE));
            zonesMap_[zoneId] = itZone->second;
        }
        if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(pid)) {
            std::shared_ptr<AudioSession> tempAudioSession = sessionService_->GetAudioSessionByPid(pid);
            if (tempAudioSession != nullptr) {
                tempAudioSession->AddAudioInterrpt(std::make_pair(audioInterrupt, ACTIVE));
            }
        }
        SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::REQUEST_CALLBACK_CATEGORY, audioInterrupt);
        UpdateAudioSceneFromInterrupt(GetHighestPriorityAudioScene(zoneId), ACTIVATE_AUDIO_INTERRUPT);
        shouldReturnSuccess = true;
        return;
    }
    shouldReturnSuccess = false;
}

bool AudioInterruptService::IsAudioSourceConcurrency(const SourceType &existSourceType,
    const SourceType &incomingSourceType, const std::vector<SourceType> &existConcurrentSources,
    const std::vector<SourceType> &incomingConcurrentSources)
{
    if ((incomingConcurrentSources.size() > 0 && existSourceType >= 0 && find(incomingConcurrentSources.begin(),
        incomingConcurrentSources.end(), existSourceType) != incomingConcurrentSources.end()) ||
        (existConcurrentSources.size() > 0 && incomingSourceType >= 0 && find(existConcurrentSources.begin(),
        existConcurrentSources.end(), incomingSourceType) != existConcurrentSources.end())) {
        return true;
    }
    return false;
}

int32_t AudioInterruptService::ProcessFocusEntry(const int32_t zoneId, const AudioInterrupt &incomingInterrupt)
{
    AudioFocuState incomingState = ACTIVE;
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_NONE, 1.0f};
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG(itZone != zonesMap_.end(), ERROR, "can not find zoneid");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end()) { audioFocusInfoList = itZone->second->audioFocusInfoList; }

    SourceType incomingSourceType = incomingInterrupt.audioFocusType.sourceType;
    std::vector<SourceType> incomingConcurrentSources = incomingInterrupt.currencySources.sourcesTypes;
    for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
        if (IsSameAppInShareMode(incomingInterrupt, iterActive->first)) { continue; }
        std::pair<AudioFocusType, AudioFocusType> audioFocusTypePair =
            std::make_pair((iterActive->first).audioFocusType, incomingInterrupt.audioFocusType);
        CHECK_AND_RETURN_RET_LOG(focusCfgMap_.find(audioFocusTypePair) != focusCfgMap_.end(), ERR_INVALID_PARAM,
            "audio focus type pair is invalid");
        AudioFocusEntry focusEntry = focusCfgMap_[audioFocusTypePair];
        if (focusEntry.actionOn == CURRENT || iterActive->second == PLACEHOLDER ||
            CanMixForSession(incomingInterrupt, iterActive->first, focusEntry)) {
            continue;
        }
        if ((focusEntry.actionOn == INCOMING && focusEntry.hintType == INTERRUPT_HINT_PAUSE) || focusEntry.isReject) {
            SourceType existSourceType = (iterActive->first).audioFocusType.sourceType;
            std::vector<SourceType> existConcurrentSources = (iterActive->first).currencySources.sourcesTypes;
            if (IsAudioSourceConcurrency(existSourceType, incomingSourceType, existConcurrentSources,
                incomingConcurrentSources)) {
                continue;
            }
        }
        if (focusEntry.isReject) {
            if (GetClientTypeBySessionId((iterActive->first).sessionId) == CLIENT_TYPE_GAME) {
                incomingState = PAUSE;
                AUDIO_INFO_LOG("incomingState: %{public}d", incomingState);
                continue;
            }

            AUDIO_INFO_LOG("the incoming stream is rejected by sessionId:%{public}d, pid:%{public}d",
                (iterActive->first).sessionId, (iterActive->first).pid);
            incomingState = STOP;
            break;
        }
        auto pos = HINT_STATE_MAP.find(focusEntry.hintType);
        AudioFocuState newState = (pos == HINT_STATE_MAP.end()) ? ACTIVE : pos->second;
        incomingState = (newState > incomingState) ? newState : incomingState;
    }
    HandleIncomingState(zoneId, incomingState, interruptEvent, incomingInterrupt);
    AddToAudioFocusInfoList(itZone->second, zoneId, incomingInterrupt, incomingState);
    SendInterruptEventToIncomingStream(interruptEvent, incomingInterrupt);
    return incomingState >= PAUSE ? ERR_FOCUS_DENIED : SUCCESS;
}

void AudioInterruptService::SendInterruptEventToIncomingStream(InterruptEventInternal &interruptEvent,
    const AudioInterrupt &incomingInterrupt)
{
    if (interruptEvent.hintType != INTERRUPT_HINT_NONE && handler_ != nullptr) {
        AUDIO_INFO_LOG("OnInterrupt for incoming sessionId: %{public}d, hintType: %{public}d",
            incomingInterrupt.sessionId, interruptEvent.hintType);
        handler_->SendInterruptEventWithSessionIdCallback(interruptEvent, incomingInterrupt.sessionId);
    }
}

void AudioInterruptService::AddToAudioFocusInfoList(std::shared_ptr<AudioInterruptZone> &audioInterruptZone,
    const int32_t &zoneId, const AudioInterrupt &incomingInterrupt, const AudioFocuState &incomingState)
{
    if (incomingState == STOP) {
        // Deny incoming. No need to add it.
        return;
    }

    int32_t inComingPid = incomingInterrupt.pid;
    audioInterruptZone->zoneId = zoneId;
    audioInterruptZone->pids.insert(inComingPid);
    audioInterruptZone->audioFocusInfoList.emplace_back(std::make_pair(incomingInterrupt, incomingState));
    zonesMap_[zoneId] = audioInterruptZone;
    SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::REQUEST_CALLBACK_CATEGORY, incomingInterrupt);
    if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(incomingInterrupt.pid)) {
        auto audioSession = sessionService_->GetAudioSessionByPid(incomingInterrupt.pid);
        if (audioSession == nullptr) {
            AUDIO_ERR_LOG("audioSession is nullptr!");
            return;
        }
        audioSession->AddAudioInterrpt(std::make_pair(incomingInterrupt, incomingState));
    }
}

void AudioInterruptService::HandleIncomingState(const int32_t &zoneId, AudioFocuState &incomingState,
    InterruptEventInternal &interruptEvent, const AudioInterrupt &incomingInterrupt)
{
    if (incomingState == STOP) {
        interruptEvent.hintType = INTERRUPT_HINT_STOP;
    } else {
        if (incomingState == PAUSE) {
            interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
        } else if (incomingState == DUCK) {
            interruptEvent.hintType = INTERRUPT_HINT_DUCK;
            interruptEvent.duckVolume = DUCK_FACTOR;
        }
        // Handle existing focus state
        ProcessActiveInterrupt(zoneId, incomingInterrupt);
    }
}

AudioScene AudioInterruptService::GetHighestPriorityAudioScene(const int32_t zoneId) const
{
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    int audioScenePriority = GetAudioScenePriority(audioScene);

    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }
    for (const auto&[interrupt, focuState] : audioFocusInfoList) {
        AudioScene itAudioScene = GetAudioSceneFromAudioInterrupt(interrupt);
        int itAudioScenePriority = GetAudioScenePriority(itAudioScene);
        if (itAudioScenePriority > audioScenePriority) {
            audioScene = itAudioScene;
            audioScenePriority = itAudioScenePriority;
        }
    }
    return audioScene;
}

bool AudioInterruptService::HadVoipStatus(const AudioInterrupt &audioInterrupt,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList)
{
    for (const auto &[interrupt, focusState] : audioFocusInfoList) {
        if (audioInterrupt.pid == interrupt.pid && focusState == PLACEHOLDER &&
            interrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION &&
            interrupt.sessionId != audioInterrupt.sessionId) {
            AUDIO_WARNING_LOG("The audio session pid: %{public}d had voip status", audioInterrupt.pid);
            return true;
        }
    }
    return false;
}

// LCOV_EXCL_STOP
void AudioInterruptService::DeactivateAudioInterruptInternal(const int32_t zoneId,
    const AudioInterrupt &audioInterrupt, bool isSessionTimeout)
{
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG((itZone != zonesMap_.end()) && (itZone->second != nullptr), "can not find zone");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList = itZone->second->audioFocusInfoList;

    bool needPlaceHolder = false;
    if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(audioInterrupt.pid)) {
        // if this stream is the last renderer for audio session, change the state to PLACEHOLDER.
        auto audioSession = sessionService_->GetAudioSessionByPid(audioInterrupt.pid);
        if (audioSession != nullptr) {
            audioSession->RemoveAudioInterrptByStreamId(audioInterrupt.sessionId);
            needPlaceHolder = audioInterrupt.audioFocusType.streamType != STREAM_DEFAULT &&
                audioSession->IsAudioRendererEmpty() &&
                !HadVoipStatus(audioInterrupt, audioFocusInfoList);
        }
    }

    auto isPresent = [audioInterrupt] (std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.sessionId == audioInterrupt.sessionId;
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isPresent);
    if (iter != audioFocusInfoList.end()) {
        if (needPlaceHolder) {
            // Change the state to PLACEHOLDER because of the active audio session.
            // No need to release interrupt until the audio session is deactivated.
            iter->second = PLACEHOLDER;
            itZone->second->audioFocusInfoList = audioFocusInfoList;
            zonesMap_[zoneId] = itZone->second;
            AUDIO_INFO_LOG("Change the state of sessionId %{public}u to PLACEHOLDER! (pid %{public}d)",
                audioInterrupt.sessionId, audioInterrupt.pid);
            return;
        }
        ResetNonInterruptControl(audioInterrupt.sessionId);
        int32_t deactivePid = audioInterrupt.pid;
        audioFocusInfoList.erase(iter);
        itZone->second->zoneId = zoneId;
        if (itZone->second->pids.find(deactivePid) != itZone->second->pids.end()) {
            itZone->second->pids.erase(itZone->second->pids.find(deactivePid));
        }
        itZone->second->audioFocusInfoList = audioFocusInfoList;
        zonesMap_[zoneId] = itZone->second;
        SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY, audioInterrupt);
    } else {
        // If it was not in the audioFocusInfoList, no need to take any action on other sessions, just return.
        AUDIO_DEBUG_LOG("stream (sessionId %{public}u) is not active now", audioInterrupt.sessionId);
        return;
    }

    policyServer_->OffloadStreamCheck(OFFLOAD_NO_SESSION_ID, audioInterrupt.sessionId);
    policyServer_->OffloadStopPlaying(audioInterrupt);

    // resume if other session was forced paused or ducked
    ResumeAudioFocusList(zoneId, isSessionTimeout);

    return;
}

void AudioInterruptService::UpdateAudioSceneFromInterrupt(const AudioScene audioScene,
    AudioInterruptChangeType changeType)
{
    AudioScene currentAudioScene = policyServer_->GetAudioScene();

    AUDIO_PRERELEASE_LOGI("currentScene: %{public}d, targetScene: %{public}d, changeType: %{public}d",
        currentAudioScene, audioScene, changeType);

    switch (changeType) {
        case ACTIVATE_AUDIO_INTERRUPT:
            break;
        case DEACTIVATE_AUDIO_INTERRUPT:
            if (GetAudioScenePriority(audioScene) >= GetAudioScenePriority(currentAudioScene)) {
                return;
            }
            break;
        default:
            AUDIO_ERR_LOG("unexpected changeType: %{public}d", changeType);
            return;
    }
    policyServer_->SetAudioSceneInternal(audioScene);
}

bool AudioInterruptService::EvaluateWhetherContinue(const AudioInterrupt &incoming, const AudioInterrupt
    &inprocessing, AudioFocusEntry &focusEntry, bool bConcurrency)
{
    if (CanMixForSession(incoming, inprocessing, focusEntry) ||
        ((focusEntry.hintType == INTERRUPT_HINT_PAUSE || focusEntry.hintType == INTERRUPT_HINT_STOP) && bConcurrency)) {
        return true;
    }
    UpdateHintTypeForExistingSession(incoming, focusEntry);
    if (GetClientTypeBySessionId(incoming.sessionId) == CLIENT_TYPE_GAME &&
        focusEntry.hintType == INTERRUPT_HINT_STOP) {
        focusEntry.hintType = INTERRUPT_HINT_PAUSE;
        AUDIO_INFO_LOG("focusEntry.hintType: %{public}d", focusEntry.hintType);
    }
    return false;
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioInterruptService::SimulateFocusEntry(const int32_t zoneId)
{
    AUDIO_INFO_LOG("Simulate a new focus list to check whether any streams need to be restored");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> newAudioFocuInfoList;
    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
        AudioInterrupt incoming = iterActive->first;
        AudioFocuState incomingState = ACTIVE;
        SourceType incomingSourceType = incoming.audioFocusType.sourceType;
        std::vector<SourceType> incomingConcurrentSources = incoming.currencySources.sourcesTypes;
        std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpAudioFocuInfoList = newAudioFocuInfoList;
        for (auto iter = newAudioFocuInfoList.begin(); iter != newAudioFocuInfoList.end(); ++iter) {
            AudioInterrupt inprocessing = iter->first;
            if (IsSameAppInShareMode(incoming, inprocessing) || iter->second == PLACEHOLDER) { continue; }
            auto audioFocusTypePair = std::make_pair(inprocessing.audioFocusType, incoming.audioFocusType);
            if (focusCfgMap_.find(audioFocusTypePair) == focusCfgMap_.end()) {
                AUDIO_WARNING_LOG("focus type is invalid");
                incomingState = iterActive->second;
                break;
            }
            AudioFocusEntry focusEntry = focusCfgMap_[audioFocusTypePair];
            SourceType existSourceType = inprocessing.audioFocusType.sourceType;
            std::vector<SourceType> existConcurrentSources = inprocessing.currencySources.sourcesTypes;
            bool bConcurrency = IsAudioSourceConcurrency(existSourceType, incomingSourceType,
                existConcurrentSources, incomingConcurrentSources);
            if (EvaluateWhetherContinue(incoming, inprocessing, focusEntry, bConcurrency)) { continue; }
            auto pos = HINT_STATE_MAP.find(focusEntry.hintType);
            if (pos == HINT_STATE_MAP.end()) { continue; }
            if (focusEntry.actionOn == CURRENT) {
                iter->second = (pos->second > iter->second) ? pos->second : iter->second;
            } else if (focusEntry.actionOn == INCOMING) {
                AudioFocuState newState = pos->second;
                incomingState = (newState > incomingState) ? newState : incomingState;
            }
        }

        if (incomingState == PAUSE) { newAudioFocuInfoList = tmpAudioFocuInfoList; }
        if (iterActive->second == PLACEHOLDER) { incomingState = PLACEHOLDER; }
        newAudioFocuInfoList.emplace_back(std::make_pair(incoming, incomingState));
    }

    return newAudioFocuInfoList;
}

void AudioInterruptService::SendInterruptEvent(AudioFocuState oldState, AudioFocuState newState,
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive, bool &removeFocusInfo)
{
    AudioInterrupt audioInterrupt = iterActive->first;
    uint32_t sessionId = audioInterrupt.sessionId;

    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is nullptr");

    InterruptEventInternal forceActive {INTERRUPT_TYPE_END, INTERRUPT_SHARE, INTERRUPT_HINT_RESUME, 1.0f};
    InterruptEventInternal forceUnduck {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_UNDUCK, 1.0f};
    InterruptEventInternal forceDuck {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_DUCK, DUCK_FACTOR};
    InterruptEventInternal forcePause {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_PAUSE, 1.0f};
    switch (newState) {
        case ACTIVE:
            if (oldState == PAUSE) {
                handler_->SendInterruptEventWithSessionIdCallback(forceActive, sessionId);
                removeFocusInfo = true;
            }
            if (oldState == DUCK) {
                handler_->SendInterruptEventWithSessionIdCallback(forceUnduck, sessionId);
            }
            break;
        case DUCK:
            if (oldState == PAUSE) {
                handler_->SendInterruptEventWithSessionIdCallback(forceActive, sessionId);
                removeFocusInfo = true;
            } else if (oldState == ACTIVE) {
                handler_->SendInterruptEventWithSessionIdCallback(forceDuck, sessionId);
            }
            break;
        case PAUSE:
            if (oldState == DUCK) {
                handler_->SendInterruptEventWithSessionIdCallback(forceUnduck, sessionId);
            }
            handler_->SendInterruptEventWithSessionIdCallback(forcePause, sessionId);
            break;
        default:
            break;
    }
    iterActive->second = newState;
}

bool AudioInterruptService::IsHandleIter(
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive, AudioFocuState oldState,
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterNew)
{
    if (oldState == PAUSEDBYREMOTE) {
        AUDIO_INFO_LOG("old State is PAUSEDBYREMOTE");
        ++iterActive;
        ++iterNew;
        return true;
    }
    return false;
}

void AudioInterruptService::ResumeAudioFocusList(const int32_t zoneId, bool isSessionTimeout)
{
    AudioScene highestPriorityAudioScene = AUDIO_SCENE_DEFAULT;

    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    std::list<std::pair<AudioInterrupt, AudioFocuState>> newAudioFocuInfoList = SimulateFocusEntry(zoneId);
    for (auto iterActive = audioFocusInfoList.begin(), iterNew = newAudioFocuInfoList.begin();
        iterActive != audioFocusInfoList.end() && iterNew != newAudioFocuInfoList.end();) {
        AudioFocuState oldState = iterActive->second;
        if (IsHandleIter(iterActive, oldState, iterNew)) {
            continue;
        }
        AudioFocuState newState = iterNew->second;
        bool removeFocusInfo = false;
        if (oldState != newState) {
            if (isSessionTimeout && oldState == PAUSE && (newState == ACTIVE || newState == DUCK)) {
                // When the audio session is timeout, just send unduck event and skip resume event.
                AudioInterrupt interruptToRemove = iterActive->first;
                iterActive = audioFocusInfoList.erase(iterActive);
                iterNew = newAudioFocuInfoList.erase(iterNew);
                AUDIO_INFO_LOG("Audio session time out. Treat resume event as stop event. streamId %{public}d",
                    interruptToRemove.sessionId);
                SendSessionTimeOutStopEvent(zoneId, interruptToRemove, audioFocusInfoList);
                continue;
            }
            AUDIO_INFO_LOG("State change: sessionId %{public}d, oldstate %{public}d, "\
                "newState %{public}d", (iterActive->first).sessionId, oldState, newState);
            SendInterruptEvent(oldState, newState, iterActive, removeFocusInfo);
        }

        if (removeFocusInfo && GetClientTypeBySessionId((iterActive->first).sessionId) != CLIENT_TYPE_GAME) {
            AudioInterrupt interruptToRemove = iterActive->first;
            iterActive = audioFocusInfoList.erase(iterActive);
            iterNew = newAudioFocuInfoList.erase(iterNew);
            AUDIO_INFO_LOG("Remove focus info from focus list, streamId: %{public}d", interruptToRemove.sessionId);
            SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY, interruptToRemove);
        } else {
            AudioScene targetAudioScene = GetAudioSceneFromAudioInterrupt(iterActive->first);
            if (GetAudioScenePriority(targetAudioScene) > GetAudioScenePriority(highestPriorityAudioScene)) {
                highestPriorityAudioScene = targetAudioScene;
            }
            ++iterActive;
            ++iterNew;
        }
    }

    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        itZone->second->audioFocusInfoList = audioFocusInfoList;
    }

    UpdateAudioSceneFromInterrupt(highestPriorityAudioScene, DEACTIVATE_AUDIO_INTERRUPT);
}

void AudioInterruptService::SendSessionTimeOutStopEvent(const int32_t zoneId, const AudioInterrupt &audioInterrupt,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList)
{
    // When the audio session is timeout, change resume event to stop event and delete the interttupt.
    InterruptEventInternal stopEvent {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
    if (handler_ != nullptr) {
        handler_->SendInterruptEventWithSessionIdCallback(stopEvent, audioInterrupt.sessionId);
    }

    auto itZone = zonesMap_.find(zoneId);
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        itZone->second->zoneId = zoneId;
        if (itZone->second->pids.find(audioInterrupt.pid) != itZone->second->pids.end()) {
            itZone->second->pids.erase(itZone->second->pids.find(audioInterrupt.pid));
        }
        itZone->second->audioFocusInfoList = audioFocusInfoList;
        zonesMap_[zoneId] = itZone->second;
    }
    SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY, audioInterrupt);
}

void AudioInterruptService::SendFocusChangeEvent(const int32_t zoneId, int32_t callbackCategory,
    const AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is null");
    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    handler_->SendAudioFocusInfoChangeCallback(callbackCategory, audioInterrupt, audioFocusInfoList);
}

// LCOV_EXCL_START
bool AudioInterruptService::CheckAudioInterruptZonePermission()
{
    auto callerUid = IPCSkeleton::GetCallingUid();
    if (callerUid == UID_AUDIO) {
        return true;
    }
    return false;
}

int32_t AudioInterruptService::CreateAudioInterruptZoneInternal(const int32_t zoneId, const std::set<int32_t> &pids)
{
    if (zonesMap_.find(zoneId) != zonesMap_.end()) {
        AUDIO_INFO_LOG("zone:(%{public}d) already exists.", zoneId);
        return SUCCESS;
    }

    int32_t hitZoneId;
    HitZoneIdHaveTheSamePidsZone(pids, hitZoneId);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
    NewAudioInterruptZoneByPids(audioInterruptZone, pids, zoneId);

    ArchiveToNewAudioInterruptZone(hitZoneId, zoneId);

    return SUCCESS;
}

int32_t AudioInterruptService::HitZoneId(const std::set<int32_t> &pids,
    const std::shared_ptr<AudioInterruptZone> &audioInterruptZone,
    const int32_t &zoneId, int32_t &hitZoneId, bool &haveSamePids)
{
    for (int32_t pid : pids) {
        for (int32_t pidTmp : audioInterruptZone->pids) {
            if (pid != pidTmp) {
                haveSamePids = false;
                break;
            }
        }
        if (!haveSamePids) {
            break;
        } else {
            hitZoneId = zoneId;
        }
    }
    return SUCCESS;
}

int32_t AudioInterruptService::HitZoneIdHaveTheSamePidsZone(const std::set<int32_t> &pids,
    int32_t &hitZoneId)
{
    for (const auto&[zoneId, audioInterruptZone] : zonesMap_) {
        if (zoneId == ZONEID_DEFAULT) {
            continue;
        }
        // Find the same count pid's zone
        bool haveSamePids = true;
        if (audioInterruptZone != nullptr && pids.size() == audioInterruptZone->pids.size()) {
            HitZoneId(pids, audioInterruptZone, zoneId, hitZoneId, haveSamePids);
        }
        if (haveSamePids) {
            break;
        }
    }
    return SUCCESS;
}

int32_t AudioInterruptService::DealAudioInterruptZoneData(const int32_t pid,
    const std::shared_ptr<AudioInterruptZone> &audioInterruptZoneTmp,
    std::shared_ptr<AudioInterruptZone> &audioInterruptZone)
{
    if (audioInterruptZoneTmp == nullptr || audioInterruptZone == nullptr) {
        return SUCCESS;
    }

    for (auto audioFocusInfoTmp : audioInterruptZoneTmp->audioFocusInfoList) {
        int32_t audioFocusInfoPid = (audioFocusInfoTmp.first).pid;
        uint32_t audioFocusInfoSessionId = (audioFocusInfoTmp.first).sessionId;
        if (audioFocusInfoPid == pid) {
            audioInterruptZone->audioFocusInfoList.emplace_back(audioFocusInfoTmp);
        }
        if (audioInterruptZoneTmp->interruptCbsMap.find(audioFocusInfoSessionId) !=
            audioInterruptZoneTmp->interruptCbsMap.end()) {
            audioInterruptZone->interruptCbsMap.emplace(audioFocusInfoSessionId,
                audioInterruptZoneTmp->interruptCbsMap.find(audioFocusInfoSessionId)->second);
        }
    }
    if (audioInterruptZoneTmp->audioPolicyClientProxyCBMap.find(pid) !=
        audioInterruptZoneTmp->audioPolicyClientProxyCBMap.end()) {
        audioInterruptZone->audioPolicyClientProxyCBMap.emplace(pid,
            audioInterruptZoneTmp->audioPolicyClientProxyCBMap.find(pid)->second);
    }

    return SUCCESS;
}

int32_t AudioInterruptService::NewAudioInterruptZoneByPids(std::shared_ptr<AudioInterruptZone> &audioInterruptZone,
    const std::set<int32_t> &pids, const int32_t &zoneId)
{
    audioInterruptZone->zoneId = zoneId;
    audioInterruptZone->pids = pids;

    for (int32_t pid : pids) {
        for (const auto&[zoneIdTmp, audioInterruptZoneTmp] : zonesMap_) {
            if (audioInterruptZoneTmp != nullptr) {
                DealAudioInterruptZoneData(pid, audioInterruptZoneTmp, audioInterruptZone);
            }
        }
    }
    zonesMap_.insert_or_assign(zoneId, audioInterruptZone);
    return SUCCESS;
}

int32_t AudioInterruptService::ArchiveToNewAudioInterruptZone(const int32_t &fromZoneId, const int32_t &toZoneId)
{
    if (fromZoneId == toZoneId || fromZoneId == ZONEID_DEFAULT) {
        AUDIO_ERR_LOG("From zone:%{public}d == To zone:%{public}d, dont archive.", fromZoneId, toZoneId);
        return SUCCESS;
    }
    auto fromZoneIt = zonesMap_.find(fromZoneId);
    if (fromZoneIt == zonesMap_.end()) {
        AUDIO_ERR_LOG("From zone invalid. -- fromZoneId:%{public}d, toZoneId:(%{public}d).", fromZoneId, toZoneId);
        return SUCCESS;
    }
    std::shared_ptr<AudioInterruptZone> fromZoneAudioInterruptZone = fromZoneIt->second;
    if (fromZoneAudioInterruptZone == nullptr) {
        AUDIO_ERR_LOG("From zone element invalid. -- fromZoneId:%{public}d, toZoneId:(%{public}d).",
            fromZoneId, toZoneId);
        zonesMap_.erase(fromZoneIt);
        return SUCCESS;
    }
    auto toZoneIt = zonesMap_.find(toZoneId);
    if (toZoneIt == zonesMap_.end()) {
        AUDIO_ERR_LOG("To zone invalid. -- fromZoneId:%{public}d, toZoneId:(%{public}d).", fromZoneId, toZoneId);
        return SUCCESS;
    }
    std::shared_ptr<AudioInterruptZone> toZoneAudioInterruptZone = toZoneIt->second;
    if (toZoneAudioInterruptZone != nullptr) {
        for (auto pid : fromZoneAudioInterruptZone->pids) {
            toZoneAudioInterruptZone->pids.insert(pid);
        }
        for (auto fromZoneAudioPolicyClientProxyCb : fromZoneAudioInterruptZone->audioPolicyClientProxyCBMap) {
            toZoneAudioInterruptZone->audioPolicyClientProxyCBMap.insert_or_assign(
                fromZoneAudioPolicyClientProxyCb.first, fromZoneAudioPolicyClientProxyCb.second);
        }
        for (auto fromZoneInterruptCb : fromZoneAudioInterruptZone->interruptCbsMap) {
            toZoneAudioInterruptZone->interruptCbsMap.insert_or_assign(
                fromZoneInterruptCb.first, fromZoneInterruptCb.second);
        }
        for (auto fromAudioFocusInfo : fromZoneAudioInterruptZone->audioFocusInfoList) {
            toZoneAudioInterruptZone->audioFocusInfoList.emplace_back(fromAudioFocusInfo);
        }
        std::shared_ptr<AudioInterruptZone> audioInterruptZone = make_shared<AudioInterruptZone>();
        audioInterruptZone->zoneId = toZoneId;
        toZoneAudioInterruptZone->pids.swap(audioInterruptZone->pids);
        toZoneAudioInterruptZone->interruptCbsMap.swap(audioInterruptZone->interruptCbsMap);
        toZoneAudioInterruptZone->audioPolicyClientProxyCBMap.swap(audioInterruptZone->audioPolicyClientProxyCBMap);
        toZoneAudioInterruptZone->audioFocusInfoList.swap(audioInterruptZone->audioFocusInfoList);
        zonesMap_.insert_or_assign(toZoneId, audioInterruptZone);
        zonesMap_.erase(fromZoneIt);
    }
    WriteFocusMigrateEvent(toZoneId);
    return SUCCESS;
}

void AudioInterruptService::WriteFocusMigrateEvent(const int32_t &toZoneId)
{
    auto uid = IPCSkeleton::GetCallingUid();
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_FOCUS_MIGRATE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("CLIENT_UID", static_cast<int32_t>(uid));
    bean->Add("MIGRATE_DIRECTION", toZoneId);
    bean->Add("DEVICE_DESC", (toZoneId == 1) ? REMOTE_NETWORK_ID : LOCAL_NETWORK_ID);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioInterruptService::DispatchInterruptEventWithSessionId(uint32_t sessionId,
    InterruptEventInternal &interruptEvent)
{
    CHECK_AND_RETURN_LOG(sessionId >= MIN_SESSIONID && sessionId <= MAX_SESSIONID,
        "EntryPoint Taint Mark:arg sessionId: %{public}u is tained", sessionId);
    std::lock_guard<std::mutex> lock(mutex_);

    // call all clients
    if (sessionId == 0) {
        for (auto &it : interruptClients_) {
            (it.second)->OnInterrupt(interruptEvent);
        }
        return;
    }

    if (interruptClients_.find(sessionId) != interruptClients_.end()) {
#ifdef FEATURE_APPGALLERY
        if (ShouldCallbackToClient(interruptClients_[sessionId]->GetCallingUid(), sessionId, interruptEvent)) {
            interruptClients_[sessionId]->OnInterrupt(interruptEvent);
        }
#else
        interruptClients_[sessionId]->OnInterrupt(interruptEvent);
#endif
    }
}

ClientType AudioInterruptService::GetClientTypeBySessionId(int32_t sessionId)
{
#ifdef FEATURE_APPGALLERY
    uint32_t uid = 0;
    if (interruptClients_.find(sessionId) != interruptClients_.end()) {
        uid = interruptClients_[sessionId]->GetCallingUid();
    }
    if (uid == 0) {
        AUDIO_ERR_LOG("Cannot find sessionid %{public}d", sessionId);
        return CLIENT_TYPE_OTHERS;
    }
    return ClientTypeManager::GetInstance()->GetClientTypeByUid(uid);
#else
    return CLIENT_TYPE_OTHERS;
#endif
}

bool AudioInterruptService::ShouldCallbackToClient(uint32_t uid, int32_t sessionId,
    InterruptEventInternal &interruptEvent)
{
    AUDIO_INFO_LOG("uid: %{public}u, sessionId: %{public}d, hintType: %{public}d", uid, sessionId,
        interruptEvent.hintType);
    ClientType clientType = ClientTypeManager::GetInstance()->GetClientTypeByUid(uid);
    if (clientType != CLIENT_TYPE_GAME) {
        return true;
    }
    if (interruptEvent.hintType == INTERRUPT_HINT_DUCK || interruptEvent.hintType == INTERRUPT_HINT_UNDUCK) {
        interruptEvent.callbackToApp = false;
        return true;
    }

    bool muteFlag = true;
    switch (interruptEvent.hintType) {
        case INTERRUPT_HINT_RESUME:
            muteFlag = false;
            policyServer_->UpdateDefaultOutputDeviceWhenStarting(sessionId);
            break;
        case INTERRUPT_HINT_PAUSE:
        case INTERRUPT_HINT_STOP:
            policyServer_->UpdateDefaultOutputDeviceWhenStopping(sessionId);
            break;
        default:
            return false;
    }
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, true, "error for g_adProxy null");
    AUDIO_INFO_LOG("mute flag is: %{public}d", muteFlag);
    gsp->SetNonInterruptMute(sessionId, muteFlag);
    IPCSkeleton::SetCallingIdentity(identity);
    return false;
}

// called when the client remote object dies
void AudioInterruptService::RemoveClient(const int32_t zoneId, uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("Remove session: %{public}u in audioFocusInfoList", sessionId);

    auto itActiveZone = zonesMap_.find(ZONEID_DEFAULT);

    auto isSessionPresent = [&sessionId] (const std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo) {
        return audioFocusInfo.first.sessionId == sessionId;
    };
    auto iterActive = std::find_if((itActiveZone->second->audioFocusInfoList).begin(),
        (itActiveZone->second->audioFocusInfoList).end(), isSessionPresent);
    if (iterActive != (itActiveZone->second->audioFocusInfoList).end()) {
        AudioInterrupt interruptToRemove = iterActive->first;
        DeactivateAudioInterruptInternal(ZONEID_DEFAULT, interruptToRemove);
    }

    interruptClients_.erase(sessionId);

    // callback in zones map also need to be removed
    auto it = zonesMap_.find(zoneId);
    if (it != zonesMap_.end() && it->second != nullptr &&
        it->second->interruptCbsMap.find(sessionId) != it->second->interruptCbsMap.end()) {
        it->second->interruptCbsMap.erase(it->second->interruptCbsMap.find(sessionId));
        zonesMap_[zoneId] = it->second;
    }
}

// AudioInterruptDeathRecipient impl begin
AudioInterruptService::AudioInterruptDeathRecipient::AudioInterruptDeathRecipient(
    const std::shared_ptr<AudioInterruptService> &service,
    uint32_t sessionId)
    : service_(service), sessionId_(sessionId)
{
}

void AudioInterruptService::AudioInterruptDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    std::shared_ptr<AudioInterruptService> service = service_.lock();
    if (service != nullptr) {
        service->RemoveClient(ZONEID_DEFAULT, sessionId_);
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
            audioInterruptZoneDump->interruptCbSessionIdsMap.size());
        AppendFormat(dumpString, "    - The sessionIds as follow:\n");
        for (auto sessionId : audioInterruptZoneDump->interruptCbSessionIdsMap) {
            AppendFormat(dumpString, "      - SessionId: %u -- have interrupt callback.\n", sessionId);
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
        uint32_t invalidSessionId = static_cast<uint32_t>(-1);
        for (auto iter = audioFocusInfoList.begin(); iter != audioFocusInfoList.end(); ++iter) {
            if ((iter->first).sessionId == invalidSessionId) {
                continue;
            }
            AppendFormat(dumpString, "    - Pid: %d\n", (iter->first).pid);
            AppendFormat(dumpString, "    - SessionId: %u\n", (iter->first).sessionId);
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
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
