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
    {STREAM_VOICE_CALL_ASSISTANT, 0},
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
    const sptr<IRemoteObject> &object)
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

int32_t AudioInterruptService::ActivateAudioInterrupt(const int32_t zoneId, const AudioInterrupt &audioInterrupt)
{
    std::unique_lock<std::mutex> lock(mutex_);

    AudioStreamType streamType = audioInterrupt.audioFocusType.streamType;
    uint32_t incomingSessionId = audioInterrupt.sessionId;
    AUDIO_INFO_LOG("sessionId: %{public}u pid: %{public}d streamType: %{public}d "\
        "usage: %{public}d source: %{public}d",
        incomingSessionId, audioInterrupt.pid, streamType,
        audioInterrupt.streamUsage, (audioInterrupt.audioFocusType).sourceType);

    if (audioInterrupt.parallelPlayFlag) {
        AUDIO_INFO_LOG("allow parallel play");
        return SUCCESS;
    }

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

int32_t AudioInterruptService::DeactivateAudioInterrupt(const int32_t zoneId, const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("sessionId: %{public}u pid: %{public}d streamType: %{public}d "\
        "usage: %{public}d source: %{public}d",
        audioInterrupt.sessionId, audioInterrupt.pid, (audioInterrupt.audioFocusType).streamType,
        audioInterrupt.streamUsage, (audioInterrupt.audioFocusType).sourceType);

    if (audioInterrupt.parallelPlayFlag) {
        AUDIO_INFO_LOG("allow parallel play");
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
        std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator it =
            audioInterruptZone->audioFocusInfoList.begin();
        while (it != audioInterruptZone->audioFocusInfoList.end()) {
            if ((*it).first.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
                (*it).first.streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
                AUDIO_INFO_LOG("usage is voice modem communication or voice ring, skip");
                it++;
            } else {
                handler_->SendInterruptEventWithSessionIdCallback(interruptEvent, (*it).first.sessionId);
                audioInterruptZone->audioFocusInfoList.erase(it);
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
    const AudioInterrupt activateInterrupt)
{
    if (incomingInterrupt.mode != SHARE_MODE || activateInterrupt.mode != SHARE_MODE) {
        return false;
    }
    if (incomingInterrupt.pid == DEFAULT_APP_PID || activateInterrupt.pid == DEFAULT_APP_PID) {
        return false;
    }
    return incomingInterrupt.pid == activateInterrupt.pid;
}

void AudioInterruptService::ProcessExistInterrupt(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator
    &iterActive, const AudioFocusEntry &focusEntry, const AudioInterrupt &incomingInterrupt,
    bool &removeFocusInfo, InterruptEventInternal &interruptEvent)
{
    SourceType incomingSourceType = incomingInterrupt.audioFocusType.sourceType;
    std::vector<SourceType> incomingConcurrentSources = incomingInterrupt.currencySources.sourcesTypes;
    SourceType existSourceType = (iterActive->first).audioFocusType.sourceType;
    std::vector<SourceType> existConcurrentSources = (iterActive->first).currencySources.sourcesTypes;
    switch (focusEntry.hintType) {
        case INTERRUPT_HINT_STOP:
            if (IsAudioSourceConcurrency(existSourceType, incomingSourceType, existConcurrentSources,
                incomingConcurrentSources)) {
                interruptEvent.hintType = INTERRUPT_HINT_NONE;
                break;
            }
            removeFocusInfo = true;
            break;
        case INTERRUPT_HINT_PAUSE:
            iterActive->second = PAUSE;
            break;
        case INTERRUPT_HINT_DUCK:
            iterActive->second = DUCK;
            interruptEvent.duckVolume = DUCK_FACTOR;
            break;
        default:
            break;
    }
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
        if (iterActive->second == PAUSE || focusEntry.actionOn != CURRENT
                || IsSameAppInShareMode(incomingInterrupt, iterActive->first)) {
            ++iterActive;
            continue;
        }

        InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, focusEntry.forceType, focusEntry.hintType, 1.0f};
        uint32_t activeSessionId = (iterActive->first).sessionId;
        bool removeFocusInfo = false;
        ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt, removeFocusInfo, interruptEvent);
        if (removeFocusInfo) {
            // execute remove from list, iter move to next by erase
            auto pidIt = targetZoneIt->second->pids.find((iterActive->first).pid);
            if (pidIt != targetZoneIt->second->pids.end()) {
                targetZoneIt->second->pids.erase(pidIt);
            }
            iterActive = tmpFocusInfoList.erase(iterActive);
            targetZoneIt->second->audioFocusInfoList = tmpFocusInfoList;
        } else {
            ++iterActive;
        }

        SendActiveInterruptEvent(activeSessionId, interruptEvent, incomingInterrupt);
    }

    targetZoneIt->second->audioFocusInfoList = tmpFocusInfoList;
    zonesMap_[zoneId] = targetZoneIt->second;
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
        // If the session is present in audioFocusInfoList, remove and treat it as a new request
        AUDIO_DEBUG_LOG("audioFocusInfoList is not empty, check whether the session is present");
        audioFocusInfoList.remove_if(
            [&incomingSessionId](const std::pair<AudioInterrupt, AudioFocuState> &audioFocus) {
            return (audioFocus.first).sessionId == incomingSessionId;
        });

        if (itZone->second->pids.find(pid) != itZone->second->pids.end()) {
            itZone->second->pids.erase(itZone->second->pids.find(pid));
        }
        itZone->second->audioFocusInfoList = audioFocusInfoList;
        zonesMap_[zoneId] = itZone->second;
    }
    if (audioFocusInfoList.empty()) {
        // If audioFocusInfoList is empty, directly activate interrupt
        AUDIO_INFO_LOG("audioFocusInfoList is empty, add the session into it directly");
        if (itZone->second != nullptr) {
            itZone->second->pids.insert(pid);
            itZone->second->audioFocusInfoList.emplace_back(std::make_pair(audioInterrupt, ACTIVE));
            zonesMap_[zoneId] = itZone->second;
        }
        SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::REQUEST_CALLBACK_CATEGORY, audioInterrupt);
        AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
        UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT);
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
    if (itZone != zonesMap_.end()) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    SourceType incomingSourceType = incomingInterrupt.audioFocusType.sourceType;
    std::vector<SourceType> incomingConcurrentSources = incomingInterrupt.currencySources.sourcesTypes;
    for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
        if (IsSameAppInShareMode(incomingInterrupt, iterActive->first)) {
            continue;
        }
        std::pair<AudioFocusType, AudioFocusType> audioFocusTypePair =
            std::make_pair((iterActive->first).audioFocusType, incomingInterrupt.audioFocusType);
        CHECK_AND_RETURN_RET_LOG(focusCfgMap_.find(audioFocusTypePair) != focusCfgMap_.end(), ERR_INVALID_PARAM,
            "audio focus type pair is invalid");
        AudioFocusEntry focusEntry = focusCfgMap_[audioFocusTypePair];
        if (iterActive->second == PAUSE || focusEntry.actionOn == CURRENT) {
            continue;
        }
        if (focusEntry.isReject) {
            SourceType existSourceType = (iterActive->first).audioFocusType.sourceType;
            std::vector<SourceType> existConcurrentSources = (iterActive->first).currencySources.sourcesTypes;
            if (IsAudioSourceConcurrency(existSourceType, incomingSourceType, existConcurrentSources,
                incomingConcurrentSources)) {
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
    if (interruptEvent.hintType != INTERRUPT_HINT_NONE && handler_ != nullptr) {
        AUDIO_INFO_LOG("OnInterrupt for incoming sessionId: %{public}d, hintType: %{public}d",
            incomingInterrupt.sessionId, interruptEvent.hintType);
        handler_->SendInterruptEventWithSessionIdCallback(interruptEvent,
            incomingInterrupt.sessionId);
    }
    return incomingState >= PAUSE ? ERR_FOCUS_DENIED : SUCCESS;
}

void AudioInterruptService::AddToAudioFocusInfoList(std::shared_ptr<AudioInterruptZone> &audioInterruptZone,
    const int32_t &zoneId, const AudioInterrupt &incomingInterrupt, const AudioFocuState &incomingState)
{
    if (incomingState != STOP) {
        int32_t inComingPid = incomingInterrupt.pid;
        audioInterruptZone->zoneId = zoneId;
        audioInterruptZone->pids.insert(inComingPid);
        audioInterruptZone->audioFocusInfoList.emplace_back(std::make_pair(incomingInterrupt, incomingState));
        zonesMap_[zoneId] = audioInterruptZone;
        SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::REQUEST_CALLBACK_CATEGORY, incomingInterrupt);
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

// LCOV_EXCL_STOP
void AudioInterruptService::DeactivateAudioInterruptInternal(const int32_t zoneId,
    const AudioInterrupt &audioInterrupt)
{
    AudioScene highestPriorityAudioScene = AUDIO_SCENE_DEFAULT;
    bool isInterruptActive = false;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    auto itZone = zonesMap_.find(zoneId);
    if (itZone != zonesMap_.end()) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    } else {
        AUDIO_ERR_LOG("can not find zone");
        return;
    }

    for (auto it = audioFocusInfoList.begin(); it != audioFocusInfoList.end();) {
        if ((it->first).sessionId == audioInterrupt.sessionId) {
            int32_t deactivePid = audioInterrupt.pid;
            it = audioFocusInfoList.erase(it);
            itZone->second->zoneId = zoneId;
            if (itZone->second->pids.find(deactivePid) != itZone->second->pids.end()) {
                itZone->second->pids.erase(itZone->second->pids.find(deactivePid));
            }
            itZone->second->audioFocusInfoList = audioFocusInfoList;
            zonesMap_[zoneId] = itZone->second;
            isInterruptActive = true;
            SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY, audioInterrupt);
        } else {
            AudioScene targetAudioScene = GetAudioSceneFromAudioInterrupt(it->first);
            if (GetAudioScenePriority(targetAudioScene) > GetAudioScenePriority(highestPriorityAudioScene)) {
                highestPriorityAudioScene = targetAudioScene;
            }
            ++it;
        }
    }

    // If it was not in the audioFocusInfoList, no need to take any action on other sessions, just return.
    if (!isInterruptActive) {
        AUDIO_DEBUG_LOG("stream (sessionId %{public}d) is not active now", audioInterrupt.sessionId);
        return;
    }

    UpdateAudioSceneFromInterrupt(highestPriorityAudioScene, DEACTIVATE_AUDIO_INTERRUPT);

    policyServer_->OffloadStreamCheck(OFFLOAD_NO_SESSION_ID, audioInterrupt.sessionId);
    policyServer_->OffloadStopPlaying(audioInterrupt);

    // resume if other session was forced paused or ducked
    ResumeAudioFocusList(zoneId);

    return;
}

void AudioInterruptService::UpdateAudioSceneFromInterrupt(const AudioScene audioScene,
    AudioInterruptChangeType changeType)
{
    if (policyServer_ == nullptr) {
        return;
    }
    AudioScene currentAudioScene = policyServer_->GetAudioScene();

    AUDIO_INFO_LOG("currentScene: %{public}d, targetScene: %{public}d, changeType: %{public}d",
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
    if (policyServer_->isAvSessionSetVoipStart && audioScene == AUDIO_SCENE_DEFAULT) {
        AUDIO_INFO_LOG("default scene is blocked for call state set by avsession");
        return;
    }
    policyServer_->SetAudioSceneInternal(audioScene);
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioInterruptService::SimulateFocusEntry(const int32_t zoneId)
{
    std::list<std::pair<AudioInterrupt, AudioFocuState>> newAudioFocuInfoList;
    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
        AudioInterrupt incoming = iterActive->first;
        AudioFocuState incomingState = ACTIVE;
        std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpAudioFocuInfoList = newAudioFocuInfoList;
        for (auto iter = newAudioFocuInfoList.begin(); iter != newAudioFocuInfoList.end(); ++iter) {
            AudioInterrupt inprocessing = iter->first;
            if (iter->second == PAUSE || IsSameAppInShareMode(incoming, inprocessing)) {
                continue;
            }
            AudioFocusType activeFocusType = inprocessing.audioFocusType;
            AudioFocusType incomingFocusType = incoming.audioFocusType;
            std::pair<AudioFocusType, AudioFocusType> audioFocusTypePair =
                std::make_pair(activeFocusType, incomingFocusType);
            if (focusCfgMap_.find(audioFocusTypePair) == focusCfgMap_.end()) {
                AUDIO_WARNING_LOG("focus type is invalid");
                incomingState = iterActive->second;
                break;
            }
            AudioFocusEntry focusEntry = focusCfgMap_[audioFocusTypePair];
            auto pos = HINT_STATE_MAP.find(focusEntry.hintType);
            if (pos == HINT_STATE_MAP.end()) {
                continue;
            }
            if (focusEntry.actionOn == CURRENT) {
                iter->second = pos->second;
            } else {
                AudioFocuState newState = pos->second;
                incomingState = newState > incomingState ? newState : incomingState;
            }
        }

        if (incomingState == PAUSE) {
            newAudioFocuInfoList = tmpAudioFocuInfoList;
        }
        newAudioFocuInfoList.emplace_back(std::make_pair(incoming, incomingState));
    }

    return newAudioFocuInfoList;
}

void AudioInterruptService::SendInterruptEvent(AudioFocuState oldState, AudioFocuState newState,
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive)
{
    AudioInterrupt audioInterrupt = iterActive->first;
    uint32_t sessionId = audioInterrupt.sessionId;

    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is nullptr");

    InterruptEventInternal forceActive {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_RESUME, 1.0f};
    InterruptEventInternal forceUnduck {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_UNDUCK, 1.0f};
    InterruptEventInternal forceDuck {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_DUCK, DUCK_FACTOR};
    InterruptEventInternal forcePause {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_PAUSE, 1.0f};
    switch (newState) {
        case ACTIVE:
            if (oldState == PAUSE) {
                handler_->SendInterruptEventWithSessionIdCallback(forceActive, sessionId);
            }
            if (oldState == DUCK) {
                handler_->SendInterruptEventWithSessionIdCallback(forceUnduck, sessionId);
            }
            break;
        case DUCK:
            if (oldState == PAUSE) {
                handler_->SendInterruptEventWithSessionIdCallback(forceActive, sessionId);
            }
            handler_->SendInterruptEventWithSessionIdCallback(forceDuck, sessionId);
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

void AudioInterruptService::ResumeAudioFocusList(const int32_t zoneId)
{
    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    std::list<std::pair<AudioInterrupt, AudioFocuState>> newAudioFocuInfoList = SimulateFocusEntry(zoneId);
    for (auto iterActive = audioFocusInfoList.begin(), iterNew = newAudioFocuInfoList.begin(); iterActive !=
        audioFocusInfoList.end() && iterNew != newAudioFocuInfoList.end(); ++iterActive, ++iterNew) {
        AudioFocuState oldState = iterActive->second;
        AudioFocuState newState = iterNew->second;
        if (oldState != newState) {
            AUDIO_INFO_LOG("State change: sessionId %{public}d, oldstate %{public}d, "\
                "newState %{public}d", (iterActive->first).sessionId, oldState, newState);
            SendInterruptEvent(oldState, newState, iterActive);
        }
    }
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
#ifdef AUDIO_BUILD_VARIANT_ROOT
    // 0 for root uid
    if (callerUid == 0) {
        AUDIO_INFO_LOG("skip root calling.");
        return true;
    }
#endif
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

void AudioInterruptService::DispatchInterruptEventWithSessionId(uint32_t sessionId,
    const InterruptEventInternal &interruptEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // call all clients
    if (sessionId == 0) {
        for (auto &it : interruptClients_) {
            (it.second)->OnInterrupt(interruptEvent);
        }
        return;
    }

    if (interruptClients_.find(sessionId) != interruptClients_.end()) {
        interruptClients_[sessionId]->OnInterrupt(interruptEvent);
    }
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
// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
