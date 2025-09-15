/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "audio_utils_c.h"
#include "standard_audio_policy_manager_listener_proxy.h"
#include "audio_policy_manager_listener_stub_impl.h"
#include "audio_policy_manager_listener.h"
#include "media_monitor_manager.h"
#include "audio_log.h"

#include "dfx_utils.h"
#include "app_mgr_client.h"
#include "dfx_msg_manager.h"
#include "audio_bundle_manager.h"
#include "istandard_audio_service.h"
#include "session_manager_lite.h"
#include "audio_zone_service.h"
#include "standalone_mode_manager.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t BOOTUP_MUSIC_UID = 1003;
constexpr uint32_t MEDIA_SA_UID = 1013;
constexpr uint32_t THP_EXTRA_SA_UID = 5000;
static const int32_t INTERRUPT_SERVICE_TIMEOUT = 10; // 10s
static sptr<IStandardAudioService> g_adProxy = nullptr;
const std::string DEFAULT_VOLUME_KEY = "default_volume_key_control";

static const map<InterruptHint, AudioFocuState> HINT_STATE_MAP = {
    {INTERRUPT_HINT_PAUSE, PAUSE},
    {INTERRUPT_HINT_DUCK, DUCK},
    {INTERRUPT_HINT_NONE, ACTIVE},
    {INTERRUPT_HINT_RESUME, ACTIVE},
    {INTERRUPT_HINT_UNDUCK, ACTIVE},
    {INTERRUPT_HINT_MUTE, MUTED}
};

static const map<InterruptHint, InterruptStage> HINT_STAGE_MAP = {
    {INTERRUPT_HINT_PAUSE, INTERRUPT_STAGE_PAUSED},
    {INTERRUPT_HINT_DUCK, INTERRUPT_STAGE_DUCK_BEGIN},
    {INTERRUPT_HINT_STOP, INTERRUPT_STAGE_STOPPED},
    {INTERRUPT_HINT_RESUME, INTERRUPT_STAGE_RESUMED},
    {INTERRUPT_HINT_UNDUCK, INTERRUPT_STAGE_DUCK_END}
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

static const std::unordered_map<const AudioScene, const int32_t> SCENE_PRIORITY = {
    // from high to low
    {AUDIO_SCENE_PHONE_CALL, 5},
    {AUDIO_SCENE_VOICE_RINGING, 4},
    {AUDIO_SCENE_PHONE_CHAT, 3},
    {AUDIO_SCENE_RINGING, 2},
    {AUDIO_SCENE_DEFAULT, 1}
};

static const unordered_map<AudioStreamType, int32_t> DEFAULT_STREAM_PRIORITY = {
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

inline int32_t GetAudioScenePriority(const AudioScene audioScene)
{
    if (SCENE_PRIORITY.count(audioScene) == 0) {
        return SCENE_PRIORITY.at(AUDIO_SCENE_DEFAULT);
    }
    return SCENE_PRIORITY.at(audioScene);
}

AudioInterruptService::AudioInterruptService()
{
    zoneManager_.InitService(this);
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
    int32_t ret = parser->LoadConfig(focusCfgMap_);
    if (ret != SUCCESS) {
        WriteServiceStartupError();
    }
    CHECK_AND_RETURN_LOG(!ret, "load fail");

    AUDIO_DEBUG_LOG("configuration loaded. mapSize: %{public}zu", focusCfgMap_.size());

    policyServer_ = server;
    clientOnFocus_ = 0;
    focussedAudioInterruptInfo_ = nullptr;

    zoneManager_.CreateAudioInterruptZone(ZONEID_DEFAULT,
        AudioZoneFocusStrategy::LOCAL_FOCUS_STRATEGY, false);

    sessionService_ = AudioSessionService::GetAudioSessionService();
    sessionService_->SetSessionTimeOutCallback(shared_from_this());
    dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
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
    AudioXCollie audioXCollie("AudioInterruptService::OnSessionTimeout", INTERRUPT_SERVICE_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("OnSessionTimeout timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::lock_guard<std::mutex> lock(mutex_);
    HandleSessionTimeOutEvent(pid);
}

int32_t AudioInterruptService::GetAudioSessionZoneidByPid(const int32_t pid)
{
    for (const auto &zonePair : zonesMap_) {
        CHECK_AND_CONTINUE(zonePair.second != nullptr);
        for (const auto &audioFocusPair : zonePair.second->audioFocusInfoList) {
            if ((audioFocusPair.first.pid == pid) && (audioFocusPair.first.isAudioSessionInterrupt)) {
                return zonePair.second->zoneId;
            }
        }
    }
    AUDIO_ERR_LOG("get audio session zoneid by pid failed!");
    return ZONEID_INVALID;
}

void AudioInterruptService::HandleSessionTimeOutEvent(const int32_t pid)
{
    int32_t zoneId = GetAudioSessionZoneidByPid(pid);
    if (sessionService_ != nullptr && zoneId != ZONEID_INVALID) {
        // If there is a fake interrupt, it needs to be deactivated.
        DeactivateAudioSessionFakeInterrupt(zoneId, pid, true);
        if (handler_ != nullptr) {
            // duckVolume = -1.0f, means timeout stop
            InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, -1.0f};
            AudioInterrupt audioInterrupt;
            audioInterrupt.pid = pid;
            handler_->SendInterruptEventCallbackForAudioSession(interruptEvent, audioInterrupt);
        }
    }

    WriteSessionTimeoutDfxEvent(pid);
    RemovePlaceholderInterruptForSession(pid, true);

    AudioSessionDeactiveEvent deactiveEvent;
    deactiveEvent.deactiveReason = AudioSessionDeactiveReason::TIMEOUT;
    std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair = {pid, deactiveEvent};
    if (handler_ != nullptr) {
        AUDIO_INFO_LOG("AudioSessionService::handler_ is not null. Send event!");
        handler_->SendAudioSessionDeactiveCallback(sessionDeactivePair);
    }
}

int32_t AudioInterruptService::ActivateAudioSession(const int32_t zoneId, const int32_t callerPid,
    const AudioSessionStrategy &strategy, const bool isStandalone)
{
    AudioXCollie audioXCollie("AudioInterruptService::ActivateAudioSession", INTERRUPT_SERVICE_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("ActivateAudioSession timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::unique_lock<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }
    bool isActivated = sessionService_->IsAudioSessionActivated(callerPid);
    int32_t result = sessionService_->ActivateAudioSession(callerPid, strategy);
    if (result != SUCCESS) {
        AUDIO_ERR_LOG("Failed to activate audio session for pid %{public}d!", callerPid);
        return result;
    }
    if (!isActivated) {
        AUDIO_INFO_LOG("The audio session is activated for the first time. Add active streams");
        AddActiveInterruptToSession(callerPid);
    }

    if (sessionService_->IsAudioSessionFocusMode(callerPid)) {
        AUDIO_INFO_LOG("Enter audio session focus mode, pid = %{public}d", callerPid);
        if (isStandalone) {
            AUDIO_INFO_LOG("Current audio session focus mode is Standalone and return");
            return SUCCESS;
        }
        bool updateScene = false;
        result = ProcessFocusEntryForAudioSession(zoneId, callerPid, updateScene);
        if (result != SUCCESS || !updateScene) {
            AUDIO_INFO_LOG(
                "Process focus for AudioSession, pid: %{public}d, result: %{public}d, updateScene: %{public}d",
                callerPid,
                result,
                updateScene);
            return result;
        }

        AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
        // If there is an event of (interrupt + set scene), ActivateAudioInterrupt and DeactivateAudioInterrupt may
        // experience deadlocks, due to mutex_ and deviceStatusUpdateSharedMutex_ waiting for each other
        lock.unlock();
        UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT, zoneId);
        return SUCCESS;
    }

    return SUCCESS;
}

bool AudioInterruptService::IsSessionNeedToFetchOutputDevice(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return false;
    }

    return sessionService_->IsSessionNeedToFetchOutputDevice(callerPid);
}

int32_t AudioInterruptService::SetAudioSessionScene(int32_t callerPid, AudioSessionScene scene)
{
    std::unique_lock<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    return sessionService_->SetAudioSessionScene(callerPid, scene);
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

    int32_t zoneId = zoneManager_.FindZoneByPid(callerPid);
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end(), "can not find zoneid");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }
    for (auto iterActive = audioFocusInfoList.begin(); iterActive != audioFocusInfoList.end(); ++iterActive) {
        if ((iterActive->first).pid == callerPid && audioSession != nullptr) {
            audioSession->AddStreamInfo(iterActive->first);
        }
    }
}

int32_t AudioInterruptService::DeactivateAudioSession(const int32_t zoneId, const int32_t callerPid)
{
    AudioXCollie audioXCollie("AudioInterruptService::DeactivateAudioSession", INTERRUPT_SERVICE_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("DeactivateAudioSession timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::unique_lock<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    // audio session v2
    if (HasAudioSessionFakeInterrupt(zoneId, callerPid)) {
        std::vector<AudioInterrupt> streamsInSession = sessionService_->GetStreams(callerPid);
        if (streamsInSession.size() > 0) {
            // Wait for the streams managed by session to stop
            DelayToDeactivateStreamsInAudioSession(zoneId, callerPid, streamsInSession);
        } else {
            // If there is a fake interrupt, it needs to be deactivated.
            DeactivateAudioSessionFakeInterrupt(zoneId, callerPid);
        }
    }

    int32_t result = sessionService_->DeactivateAudioSession(callerPid);
    if (result != SUCCESS) {
        AUDIO_INFO_LOG("Failed to deactivate audio session for pid %{public}d, result %{public}d", callerPid, result);
        return result;
    }

    RemovePlaceholderInterruptForSession(callerPid);

    return SUCCESS;
}

void AudioInterruptService::DelayToDeactivateStreamsInAudioSession(
    const int32_t zoneId, const int32_t callerPid, const std::vector<AudioInterrupt> &streamsInSession)
{
    auto deactivateTask = [this, zoneId, callerPid, streamsInSession] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::unique_lock<std::mutex> lock(mutex_);
        if (sessionService_ == nullptr) {
            AUDIO_ERR_LOG("sessionService_ is nullptr!");
            return;
        }

        // If the audio session is reactivated, there is no need to clean up the session resources.
        if (sessionService_->IsAudioSessionActivated(callerPid)) {
            AUDIO_ERR_LOG("Session is reactivated, no need to deactivate interrupt, pid %{public}d", callerPid);
            return;
        }

        // If the application deactivates a session, the streams managed by session needs to be stoped.
        if (handler_ != nullptr) {
            AUDIO_INFO_LOG("Send InterruptCallbackEvent to all streams for pid %{public}d", callerPid);
            InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
            for (auto &it : streamsInSession) {
                handler_->SendInterruptEventWithStreamIdCallback(interruptEvent, it.streamId);
            }
        }

        lock.unlock();

        // Sleep for 50 milliseconds to allow streams in the session to stop.
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        // Before deactivating the fake interrupt, all stream interrupts within the session must be stopped.
        lock.lock();
        DeactivateAudioSessionFakeInterrupt(zoneId, callerPid);
    };

    std::thread(deactivateTask).detach();
    AUDIO_INFO_LOG("Started deactivation thread for pid %{public}d with 1s delay", callerPid);
}

// Deactivate session when fake focus is stopped.
void AudioInterruptService::DeactivateAudioSessionInFakeFocusMode(const int32_t pid, InterruptHint hintType)
{
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return;
    }

    /*
    Both stop and resume will delete the fake focus, so, need to deactivate audio session,
    but only stop needs to trigger a callback to the streams managed by audio session.
    */
    std::vector<AudioInterrupt> streamsInSession = sessionService_->GetStreams(pid);
    if (handler_ != nullptr && hintType == INTERRUPT_HINT_STOP) {
        AUDIO_INFO_LOG("Send InterruptCallbackEvent to all streams for pid %{public}d", pid);
        InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
        for (auto &it : streamsInSession) {
            handler_->SendInterruptEventWithStreamIdCallback(interruptEvent, it.streamId);
        }
    }

    int32_t result = sessionService_->DeactivateAudioSession(pid);
    if (result != SUCCESS) {
        AUDIO_INFO_LOG("Failed to deactivate audio session for pid %{public}d, result %{public}d", pid, result);
        return;
    }

    RemovePlaceholderInterruptForSession(pid);

    AudioSessionDeactiveEvent deactiveEvent;
    deactiveEvent.deactiveReason = AudioSessionDeactiveReason::LOW_PRIORITY;
    std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair = {pid, deactiveEvent};
    if (handler_ != nullptr) {
        AUDIO_INFO_LOG("AudioSessionService::handler_ is not null. Send event!");
        handler_->SendAudioSessionDeactiveCallback(sessionDeactivePair);
    }
}

void AudioInterruptService::DeactivateAudioSessionFakeInterrupt(
    const int32_t zoneId, const int32_t callerPid, bool isSessionTimeout)
{
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG((itZone != zonesMap_.end()) && (itZone->second != nullptr), "can not find zone");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList = itZone->second->audioFocusInfoList;

    auto isPresent = [callerPid] (const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.pid == callerPid && pair.first.isAudioSessionInterrupt;
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isPresent);
    if (iter == audioFocusInfoList.end()) {
        AUDIO_INFO_LOG("Can not find audio session fake interrupt for pid %{public}d", callerPid);
        return;
    }

    DeactivateAudioInterruptInternal(zoneId, iter->first, isSessionTimeout);
}

bool AudioInterruptService::HasAudioSessionFakeInterrupt(const int32_t zoneId, const int32_t callerPid)
{
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG((itZone != zonesMap_.end()) && (itZone->second != nullptr), false, "can not find zone");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList = itZone->second->audioFocusInfoList;

    auto isPresent = [callerPid] (const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.pid == callerPid && pair.first.isAudioSessionInterrupt;
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isPresent);
    return iter != audioFocusInfoList.end();
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

    int32_t zoneId = zoneManager_.FindZoneByPid(callerPid);
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end(), "can not find zoneid");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    for (auto iter = audioFocusInfoList.begin(); iter != audioFocusInfoList.end(); ++iter) {
        if (iter->first.pid == callerPid && iter->second == PLACEHOLDER) {
            AudioInterrupt placeholder = iter->first;
            AUDIO_INFO_LOG("Remove stream id %{public}u (placeholder for pid%{public}d)",
                placeholder.streamId, callerPid);
            DeactivateAudioInterruptInternal(zoneId, placeholder, isSessionTimeout);
        }
    }
}

bool AudioInterruptService::IsAudioSessionActivated(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return false;
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
    if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(incomingInterrupt.pid)) {
        // The strategy of activated AudioSession is the one with the highest priority.
        std::shared_ptr<AudioSession> incomingSession = sessionService_->GetAudioSessionByPid(incomingInterrupt.pid);
        if (incomingSession == nullptr) {
            AUDIO_ERR_LOG("incomingSession is nullptr!");
            return false;
        }
        AudioConcurrencyMode concurrencyMode = (incomingSession->GetSessionStrategy()).concurrencyMode;
        if (concurrencyMode != AudioConcurrencyMode::MIX_WITH_OTHERS) {
            AUDIO_INFO_LOG("The concurrency mode of incoming session is %{public}d",
                static_cast<int32_t>(concurrencyMode));
            return false;
        }
        // The concurrencyMode of incoming session is MIX_WITH_OTHERS. Need to check the priority.
        if (IsIncomingStreamLowPriority(focusEntry)) {
            bool isSameType = AudioSessionService::IsSameTypeForAudioSession(
                incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
            AUDIO_INFO_LOG("The incoming stream is low priority. isSameType: %{public}d.", isSameType);
            return isSameType;
        }
        AUDIO_INFO_LOG("The concurrency mode of incoming session is MIX_WITH_OTHERS. Skip the interrupt operation");
        return true;
    } else {
        // There is no activated AudioSession for incoming stream. Check the strategy of AudioInterrupt.
        AudioConcurrencyMode concurrencyMode = incomingInterrupt.sessionStrategy.concurrencyMode;
        AUDIO_INFO_LOG("The concurrency mode of incoming interrupt: %{public}d", static_cast<int32_t>(concurrencyMode));
        if (concurrencyMode == AudioConcurrencyMode::SILENT ||
            concurrencyMode == AudioConcurrencyMode::MIX_WITH_OTHERS) {
            AUDIO_INFO_LOG("incoming stream is explicitly SILENT or MIX_WITH_OTHERS.");
            return true;
        }
    }
    return false;
}

bool AudioInterruptService::CanMixForActiveSession(const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt, const AudioFocusEntry &focusEntry)
{
    if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(activeInterrupt.pid)) {
        // The strategy of activated AudioSession is the one with the highest priority.
        std::shared_ptr<AudioSession> activeSession = sessionService_->GetAudioSessionByPid(activeInterrupt.pid);
        if (activeSession == nullptr) {
            AUDIO_ERR_LOG("activeSession is nullptr!");
            return false;
        }
        AudioConcurrencyMode concurrencyMode = (activeSession->GetSessionStrategy()).concurrencyMode;
        if (concurrencyMode != AudioConcurrencyMode::MIX_WITH_OTHERS) {
            AUDIO_INFO_LOG("The concurrency mode of active session is %{public}d",
                static_cast<int32_t>(concurrencyMode));
            return false;
        }
        // The concurrencyMode of active session is MIX_WITH_OTHERS. Need to check the priority.
        if (IsActiveStreamLowPriority(focusEntry)) {
            bool isSameType = AudioSessionService::IsSameTypeForAudioSession(
                incomingInterrupt.audioFocusType.streamType, activeInterrupt.audioFocusType.streamType);
            AUDIO_INFO_LOG("The active stream is low priority. isSameType: %{public}d.", isSameType);
            return isSameType;
        }
        AUDIO_INFO_LOG("The concurrency mode of active session is MIX_WITH_OTHERS. Skip the interrupt operation");
        return true;
    } else {
        // There is no active AudioSession for active stream. Check the strategy of AudioInterrupt.
        AudioConcurrencyMode concurrencyMode = activeInterrupt.sessionStrategy.concurrencyMode;
        AUDIO_INFO_LOG("The concurrency mode of active interrupt: %{public}d", static_cast<int32_t>(concurrencyMode));
        if (concurrencyMode == AudioConcurrencyMode::SILENT ||
            concurrencyMode == AudioConcurrencyMode::MIX_WITH_OTHERS) {
            AUDIO_INFO_LOG("active stream is explicitly SILENT or MIX_WITH_OTHERS.");
            return true;
        }
    }
    return false;
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
    AUTO_CTRACE("SYSEVENT FAULT EVENT AUDIO_SERVICE_STARTUP_ERROR, SERVICE_ID: %d, ERROR_CODE: %d",
        Media::MediaMonitor::AUDIO_POLICY_SERVICE_ID, Media::MediaMonitor::AUDIO_INTERRUPT_SERVER);
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
        for (auto interruptCbInfo : audioInterruptZone->interruptCbsMap) {
            zoneDump->interruptCbStreamIdsMap.insert(interruptCbInfo.first);
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

int32_t AudioInterruptService::SetAudioInterruptCallback(const int32_t zoneId, const uint32_t streamId,
    const sptr<IRemoteObject> &object, uint32_t uid)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // maybe add check session id validation here

    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERR_INVALID_PARAM, "object is nullptr");

    sptr<IStandardAudioPolicyManagerListener> listener = iface_cast<IStandardAudioPolicyManagerListener>(object);
    CHECK_AND_RETURN_RET_LOG(listener != nullptr, ERR_INVALID_PARAM, "obj cast failed");

    std::shared_ptr<AudioInterruptCallback> callback = std::make_shared<AudioPolicyManagerListenerCallback>(listener);
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "create cb failed");

    if (interruptClients_.find(streamId) == interruptClients_.end()) {
        // Register client death recipient first
        sptr<AudioInterruptDeathRecipient> deathRecipient =
            new AudioInterruptDeathRecipient(shared_from_this(), streamId);
        object->AddDeathRecipient(deathRecipient);

        std::shared_ptr<AudioInterruptClient> client =
            std::make_shared<AudioInterruptClient>(callback, object, deathRecipient);
        uint32_t callingUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
        if (callingUid == MEDIA_SA_UID) {
            callingUid = uid;
        }
        client->SetCallingUid(callingUid);

        interruptClients_[streamId] = client;

        // just record in zone map, not used currently
        auto it = zonesMap_.find(zoneId);
        if (it != zonesMap_.end() && it->second != nullptr) {
            it->second->interruptCbsMap[streamId] = callback;
            zonesMap_[zoneId] = it->second;
        }
    } else {
        AUDIO_ERR_LOG("%{public}u callback already exist", streamId);
        return ERR_INVALID_PARAM;
    }

    return SUCCESS;
}

int32_t AudioInterruptService::UnsetAudioInterruptCallback(const int32_t zoneId, const uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (interruptClients_.erase(streamId) == 0) {
        AUDIO_ERR_LOG("streamId %{public}u not present", streamId);
        return ERR_INVALID_PARAM;
    }

    auto it = zonesMap_.find(zoneId);
    if (it != zonesMap_.end() && it->second != nullptr &&
        it->second->interruptCbsMap.find(streamId) != it->second->interruptCbsMap.end()) {
        it->second->interruptCbsMap.erase(it->second->interruptCbsMap.find(streamId));
        zonesMap_[zoneId] = it->second;
    }

    return SUCCESS;
}

bool AudioInterruptService::AudioInterruptIsActiveInFocusList(const int32_t zoneId, const uint32_t incomingStreamId)
{
    auto itZone = zonesMap_.find(zoneId);
    if (itZone == zonesMap_.end()) {
        AUDIO_ERR_LOG("Can not find zoneid");
        return false;
    }
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    audioFocusInfoList = itZone->second->audioFocusInfoList;
    auto isPresent = [incomingStreamId] (const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        // If the stream id has been active or ducked, no need to activate audio interrupt again.
        return pair.first.streamId == incomingStreamId && (pair.second == ACTIVE || pair.second == DUCK);
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isPresent);
    if (iter != audioFocusInfoList.end()) {
        return true;
    }
    return false;
}

void AudioInterruptService::HandleAppStreamType(const int32_t zoneId, AudioInterrupt &audioInterrupt)
{
    // In audio session mode, the focus policy is uniformly managed by the session and not handled separately here.
    if (sessionService_ != nullptr && sessionService_->IsAudioSessionFocusMode(audioInterrupt.pid)) {
        AUDIO_DEBUG_LOG(
            "In audio session focus mode, no need to check app stream type. pid = %{public}d", audioInterrupt.pid);
        return;
    }

    if (HasAudioSessionFakeInterrupt(zoneId, audioInterrupt.pid)) {
        return;
    }

    // Force game app use game interrupt strategy, not affected by InterruptEventCallbackType.
    // DO NOT use IsGameAvoidCallbackCase to replace it.
    if (GetClientTypeByStreamId(audioInterrupt.streamId) != CLIENT_TYPE_GAME) {
        return;
    }

    if (audioInterrupt.audioFocusType.streamType == STREAM_MUSIC) {
        AUDIO_INFO_LOG("game create STREAM_MUSIC, turn into STREAM_GAME");
        audioInterrupt.audioFocusType.streamType = STREAM_GAME;
    }
}

int32_t AudioInterruptService::ActivateAudioInterrupt(
    const int32_t zoneId, const AudioInterrupt &audioInterrupt, const bool isUpdatedAudioStrategy)
{
    AudioXCollie audioXCollie("AudioInterruptService::ActivateAudioInterrupt", INTERRUPT_SERVICE_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("ActivateAudioInterrupt timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::unique_lock<std::mutex> lock(mutex_);
    bool updateScene = false;
    int32_t ret = ActivateAudioInterruptCoreProcedure(zoneId, audioInterrupt, isUpdatedAudioStrategy, updateScene);
    if (ret != SUCCESS || !updateScene) {
        return ret;
    }

    AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
    // If there is an event of (interrupt + set scene), ActivateAudioInterrupt and DeactivateAudioInterrupt may
    // experience deadlocks, due to mutex_ and deviceStatusUpdateSharedMutex_ waiting for each other
    lock.unlock();
    UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT, zoneId);
    return SUCCESS;
}

int32_t AudioInterruptService::ActivateAudioInterruptCoreProcedure(
    const int32_t zoneId, const AudioInterrupt &audioInterrupt, const bool isUpdatedAudioStrategy, bool &updateScene)
{
    PrintLogsOfFocusStrategyBaseMusic(audioInterrupt); // Print logs for automatic detection tools.
    if (isPreemptMode_) {
        InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
        SendInterruptEventToIncomingStream(interruptEvent, audioInterrupt);
        return ERR_FOCUS_DENIED;
    }

    return ActivateAudioInterruptInternal(zoneId, audioInterrupt, isUpdatedAudioStrategy, updateScene);
}

int32_t AudioInterruptService::ActivateAudioInterruptInternal(const int32_t zoneId,
    const AudioInterrupt &audioInterrupt, const bool isUpdatedAudioStrategy, bool &updateScene)
{
    AudioInterrupt currAudioInterrupt = audioInterrupt;
    HandleAppStreamType(zoneId, currAudioInterrupt);
    AudioStreamType streamType = currAudioInterrupt.audioFocusType.streamType;
    uint32_t incomingStreamId = currAudioInterrupt.streamId;
    AUDIO_INFO_LOG("streamId: %{public}u pid: %{public}d streamType: %{public}d zoneId: %{public}d"\
        "usage: %{public}d source: %{public}d",
        incomingStreamId, currAudioInterrupt.pid, streamType, zoneId,
        currAudioInterrupt.streamUsage, (currAudioInterrupt.audioFocusType).sourceType);

    if (AudioInterruptIsActiveInFocusList(zoneId, incomingStreamId) && !isUpdatedAudioStrategy) {
        AUDIO_INFO_LOG("Stream is active in focus list, no need to active audio interrupt.");
        return SUCCESS;
    }
    ResetNonInterruptControl(currAudioInterrupt);
    bool shouldReturnSuccess = false;
    ProcessAudioScene(currAudioInterrupt, incomingStreamId, zoneId, shouldReturnSuccess);
    if (shouldReturnSuccess) {
        return SUCCESS;
    }

    if (ShouldBypassAudioSessionFocus(zoneId, audioInterrupt)) {
        TryHandleStreamCallbackInSession(zoneId, audioInterrupt);
        SendActiveVolumeTypeChangeEvent(zoneId);
        updateScene = true;
        AUDIO_INFO_LOG("Bypass Audio session focus, pid = %{public}d", audioInterrupt.pid);
        return SUCCESS;
    }

    // Process ProcessFocusEntryTable for current audioFocusInfoList
    int32_t ret = ProcessFocusEntry(zoneId, currAudioInterrupt);
    CHECK_AND_RETURN_RET_LOG(!ret, ERR_FOCUS_DENIED, "request rejected");
    if (zoneId == ZONEID_DEFAULT) {
        updateScene = true;
    }
    return SUCCESS;
}

void AudioInterruptService::PrintLogsOfFocusStrategyBaseMusic(const AudioInterrupt &audioInterrupt)
{
    // The log printed by this function is critical, so please do not modify it.
    AudioFocusType audioFocusType;
    audioFocusType.streamType = AudioStreamType::STREAM_MUSIC;
    std::pair<AudioFocusType, AudioFocusType> focusPair =
        std::make_pair(audioFocusType, audioInterrupt.audioFocusType);
    CHECK_AND_RETURN_LOG(focusCfgMap_.find(focusPair) != focusCfgMap_.end(), "no focus cfg");
    AudioFocusEntry focusEntry = focusCfgMap_[focusPair];
    if (focusEntry.actionOn != CURRENT) {
        AUDIO_WARNING_LOG("The audio focus strategy based on music: forceType: %{public}d, hintType: %{public}d, " \
            "actionOn: %{public}d", focusEntry.forceType, focusEntry.hintType, focusEntry.actionOn);
        return;
    }
    // Update focus strategy by audio session.
    AudioConcurrencyMode concurrencyMode = AudioConcurrencyMode::INVALID;
    if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(audioInterrupt.pid)) {
        std::shared_ptr<AudioSession> incomingSession = sessionService_->GetAudioSessionByPid(audioInterrupt.pid);
        if (incomingSession != nullptr) {
            concurrencyMode = (incomingSession->GetSessionStrategy()).concurrencyMode;
        }
    } else {
        concurrencyMode = audioInterrupt.sessionStrategy.concurrencyMode;
    }
    switch (concurrencyMode) {
        case AudioConcurrencyMode::MIX_WITH_OTHERS:
        case AudioConcurrencyMode::SILENT:
            if (focusEntry.hintType == INTERRUPT_HINT_DUCK ||
                focusEntry.hintType == INTERRUPT_HINT_PAUSE ||
                focusEntry.hintType == INTERRUPT_HINT_STOP) {
                focusEntry.hintType = INTERRUPT_HINT_NONE;
            }
            break;

        case AudioConcurrencyMode::DUCK_OTHERS:
            if (focusEntry.hintType == INTERRUPT_HINT_DUCK ||
                focusEntry.hintType == INTERRUPT_HINT_PAUSE ||
                focusEntry.hintType == INTERRUPT_HINT_STOP) {
                focusEntry.hintType = INTERRUPT_HINT_DUCK;
            }
            break;
        case AudioConcurrencyMode::PAUSE_OTHERS:
            if (focusEntry.hintType == INTERRUPT_HINT_PAUSE ||
                focusEntry.hintType == INTERRUPT_HINT_STOP) {
                focusEntry.hintType = INTERRUPT_HINT_PAUSE;
            }
            break;
        default:
            break;
    }
    AUDIO_WARNING_LOG("The audio focus strategy based on music: forceType: %{public}d, hintType: %{public}d, " \
        "actionOn: %{public}d", focusEntry.forceType, focusEntry.hintType, focusEntry.actionOn);
    return;
}

void AudioInterruptService::ResetNonInterruptControl(AudioInterrupt audioInterrupt)
{
    if (!IsGameAvoidCallbackCase(audioInterrupt)) {
        return;
    }
    AUDIO_INFO_LOG("Reset non-interrupt control for %{public}u", audioInterrupt.streamId);
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    CHECK_AND_RETURN_LOG(gsp != nullptr, "error for audio server proxy null");
    gsp->SetNonInterruptMute(audioInterrupt.streamId, false);
    IPCSkeleton::SetCallingIdentity(identity);
}

int32_t AudioInterruptService::DeactivateAudioInterrupt(const int32_t zoneId, const AudioInterrupt &audioInterrupt)
{
    AudioXCollie audioXCollie("AudioInterruptService::DeactivateAudioInterrupt", INTERRUPT_SERVICE_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("DeactivateAudioInterrupt timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    std::unique_lock<std::mutex> lock(mutex_);

    AudioInterrupt currAudioInterrupt = audioInterrupt;
    HandleAppStreamType(zoneId, currAudioInterrupt);
    AUDIO_INFO_LOG("streamId: %{public}u pid: %{public}d streamType: %{public}d "\
        "usage: %{public}d source: %{public}d",
        currAudioInterrupt.streamId, currAudioInterrupt.pid, (currAudioInterrupt.audioFocusType).streamType,
        currAudioInterrupt.streamUsage, (currAudioInterrupt.audioFocusType).sourceType);

    DeactivateAudioInterruptInternal(zoneId, currAudioInterrupt);

    if (HasAudioSessionFakeInterrupt(zoneId, currAudioInterrupt.pid)) {
        AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
        // If there is an event of (interrupt + set scene), ActivateAudioInterrupt and DeactivateAudioInterrupt may
        // experience deadlocks, due to mutex_ and deviceStatusUpdateSharedMutex_ waiting for each other
        lock.unlock();
        UpdateAudioSceneFromInterrupt(targetAudioScene, DEACTIVATE_AUDIO_INTERRUPT, zoneId);
    }

    return SUCCESS;
}

void AudioInterruptService::ClearAudioFocusInfoListOnAccountsChanged(const int32_t &id)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("start DeactivateAudioInterrupt, current id:%{public}d", id);
    ClearAudioFocusInfoList();
}

int32_t AudioInterruptService::ClearAudioFocusInfoList()
{
    AUDIO_INFO_LOG("start clear audio focusInfo list");
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
    for (const auto&[zoneId, audioInterruptZone] : zonesMap_) {
        CHECK_AND_CONTINUE_LOG(audioInterruptZone != nullptr, "audioInterruptZone is nullptr");
        std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator it =
            audioInterruptZone->audioFocusInfoList.begin();
        while (it != audioInterruptZone->audioFocusInfoList.end()) {
            if (!isPreemptMode_ &&
                ((*it).first.streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION ||
                (*it).first.streamUsage == STREAM_USAGE_VOICE_RINGTONE)) {
                AUDIO_INFO_LOG("usage is voice modem communication or voice ring, skip");
                ++it;
            } else {
                CHECK_AND_RETURN_RET_LOG(handler_ != nullptr, ERROR, "handler is nullptr");
                SendInterruptEventCallback(interruptEvent, (*it).first.streamId, (*it).first);
                it = audioInterruptZone->audioFocusInfoList.erase(it);
            }
        }
    }
    return SUCCESS;
}

int32_t AudioInterruptService::ActivatePreemptMode()
{
    AUDIO_INFO_LOG("start activate preempt mode");
    std::lock_guard<std::mutex> lock(mutex_);
    isPreemptMode_ = true;
    int32_t ret = ClearAudioFocusInfoList();
    if (ret) {
        isPreemptMode_ = false;
    }
    AUDIO_INFO_LOG("isPreemptMode_ = %{public}d", isPreemptMode_);
    return ret;
}

int32_t AudioInterruptService::DeactivatePreemptMode()
{
    AUDIO_INFO_LOG("start deactivate preempt mode");
    std::lock_guard<std::mutex> lock(mutex_);
    isPreemptMode_ = false;
    return SUCCESS;
}

int32_t AudioInterruptService::CreateAudioInterruptZone(const int32_t zoneId,
    AudioZoneFocusStrategy focusStrategy)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return zoneManager_.CreateAudioInterruptZone(zoneId, focusStrategy);
}

int32_t AudioInterruptService::ReleaseAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func)
{
    std::unique_lock<std::mutex> lock(mutex_);
    int32_t ret = zoneManager_.ReleaseAudioInterruptZone(zoneId, func);
    if (ret != SUCCESS) {
        return ret;
    }
    AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
    lock.unlock();
    UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT, zoneId);
    return SUCCESS;
}

int32_t AudioInterruptService::MigrateAudioInterruptZone(const int32_t zoneId, GetZoneIdFunc func)
{
    std::unique_lock<std::mutex> lock(mutex_);
    int32_t ret = zoneManager_.MigrateAudioInterruptZone(zoneId, func);
    if (ret != SUCCESS) {
        return ret;
    }
    AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
    lock.unlock();
    UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT, zoneId);
    return SUCCESS;
}

int32_t AudioInterruptService::InjectInterruptToAudioZone(const int32_t zoneId,
    const AudioFocusList &interrupts)
{
    std::unique_lock<std::mutex> lock(mutex_);
    int32_t ret = zoneManager_.InjectInterruptToAudioZone(zoneId, interrupts);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InjectInterruptToAudioZone failed");
    CHECK_AND_RETURN_RET_LOG(zoneId != ZONEID_DEFAULT, SUCCESS, "zone id is default");

    AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
    lock.unlock();
    UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT, zoneId);
    return SUCCESS;
}

int32_t AudioInterruptService::InjectInterruptToAudioZone(const int32_t zoneId,
    const std::string &deviceTag, const AudioFocusList &interrupts)
{
    std::unique_lock<std::mutex> lock(mutex_);
    int32_t ret = zoneManager_.InjectInterruptToAudioZone(zoneId, deviceTag, interrupts);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InjectInterruptToAudioZone failed");
    CHECK_AND_RETURN_RET_LOG(zoneId != ZONEID_DEFAULT, SUCCESS, "zone id is default");

    AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
    lock.unlock();
    UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT, zoneId);
    return SUCCESS;
}

int32_t AudioInterruptService::GetAudioFocusInfoList(const int32_t zoneId, AudioFocusList &focusInfoList)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return zoneManager_.GetAudioFocusInfoList(zoneId, focusInfoList);
}

int32_t AudioInterruptService::GetAudioFocusInfoList(const int32_t zoneId, const std::string &deviceTag,
    AudioFocusList &focusInfoList)
{
    std::unique_lock<std::mutex> lock(mutex_);
    return zoneManager_.GetAudioFocusInfoList(zoneId, deviceTag, focusInfoList);
}

int32_t AudioInterruptService::GetStreamTypePriority(AudioStreamType streamType)
{
    if (DEFAULT_STREAM_PRIORITY.find(streamType) != DEFAULT_STREAM_PRIORITY.end()) {
        return DEFAULT_STREAM_PRIORITY.at(streamType);
    }
    return STREAM_DEFAULT_PRIORITY;
}

AudioStreamType AudioInterruptService::GetStreamInFocus(const int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetStreamInFocusInternal(0, zoneId);
}

AudioStreamType AudioInterruptService::GetStreamInFocusByUid(const int32_t uid, const int32_t zoneId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return GetStreamInFocusInternal(uid, zoneId);
}

AudioStreamType AudioInterruptService::GetStreamInFocusInternal(const int32_t uid, const int32_t zoneId)
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
        if (uid != 0 && (iter->first).uid != uid) {
            continue;
        }
        if ((iter->first).audioFocusType.streamType == STREAM_VOICE_ASSISTANT &&
            !CheckoutSystemAppUtil::CheckoutSystemApp((iter->first).uid)) {
            (iter->first).audioFocusType.streamType = STREAM_MUSIC;
        }
        if (sessionService_ != nullptr && iter->first.isAudioSessionInterrupt) {
            std::vector<AudioInterrupt> sessionStreams = sessionService_->GetStreams(iter->first.pid);
            for (auto stream : sessionStreams) {
                int32_t curPriority = GetStreamTypePriority(stream.audioFocusType.streamType);
                if (curPriority < focusPriority) {
                    focusPriority = curPriority;
                    streamInFocus = stream.audioFocusType.streamType;
                }
            }
        } else {
            int32_t curPriority = GetStreamTypePriority((iter->first).audioFocusType.streamType);
            if (curPriority < focusPriority) {
                focusPriority = curPriority;
                streamInFocus = (iter->first).audioFocusType.streamType;
            }
        }
    }
    return streamInFocus == STREAM_DEFAULT ? defaultVolumeType_ : streamInFocus;
}

int32_t AudioInterruptService::GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneId)
{
    uint32_t invalidStreamId = static_cast<uint32_t>(-1);
    audioInterrupt = {STREAM_USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN,
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_INVALID, true}, invalidStreamId};

    std::unique_lock<std::mutex> lock(mutex_);
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
            if (IsGameAvoidCallbackCase(iterActive->first)) {
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
        case INTERRUPT_HINT_MUTE:
            if (iterActive->second == ACTIVE) {
                iterActive->second = MUTED;
                interruptEvent.hintType = focusEntry.hintType;
            }
            break;
        default:
            break;
    }
}

bool AudioInterruptService::SwitchHintType(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive,
    InterruptEventInternal &interruptEvent, std::list<std::pair<AudioInterrupt, AudioFocuState>> &tmpFocusInfoList)
{
    bool needRemoveCurIter = false;
    switch (interruptEvent.hintType) {
        case INTERRUPT_HINT_STOP:
            if (IsGameAvoidCallbackCase(iterActive->first)) {
                iterActive->second = PAUSEDBYREMOTE;
                break;
            }
            needRemoveCurIter = true;
            break;
        case INTERRUPT_HINT_PAUSE:
            if (iterActive->second == ACTIVE || iterActive->second == DUCK) {
                iterActive->second = PAUSEDBYREMOTE;
            }
            break;
        case INTERRUPT_HINT_RESUME:
            if (iterActive->second == PAUSEDBYREMOTE) {
                needRemoveCurIter = true;
            }
            break;
        default:
            break;
    }
    return needRemoveCurIter;
}

std::set<int32_t> AudioInterruptService::GetStreamIdsForAudioSessionByStreamUsage(
    const int32_t zoneId, const std::set<StreamUsage> &streamUsageSet)
{
    std::set<int32_t> streamIds;

    std::unique_lock<std::mutex> lock(mutex_);
    auto targetZoneIt = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG(targetZoneIt != zonesMap_.end(), streamIds, "can not find zone id");
    auto &tmpFocusInfoList = targetZoneIt->second->audioFocusInfoList;
    for (auto focusIter = tmpFocusInfoList.begin(); focusIter != tmpFocusInfoList.end(); ++focusIter) {
        const auto &audioInterrupt = focusIter->first;
        if (audioInterrupt.isAudioSessionInterrupt &&
            streamUsageSet.find(audioInterrupt.streamUsage) != streamUsageSet.end()) {
            streamIds.insert(static_cast<int32_t>(audioInterrupt.streamId));
        }
    }
    return streamIds;
}

std::set<int32_t> AudioInterruptService::GetStreamIdsForAudioSessionByDeviceType(
    const int32_t zoneId, DeviceType deviceType)
{
    std::set<int32_t> streamIds;

    std::unique_lock<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sessionService_ != nullptr, streamIds, "sessionService_ is nullptr");
    auto targetZoneIt = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG(targetZoneIt != zonesMap_.end(), streamIds, "can not find zone id");
    auto &tmpFocusInfoList = targetZoneIt->second->audioFocusInfoList;
    for (auto focusIter = tmpFocusInfoList.begin(); focusIter != tmpFocusInfoList.end(); ++focusIter) {
        const auto &audioInterrupt = focusIter->first;
        if (audioInterrupt.isAudioSessionInterrupt &&
            sessionService_->HasStreamForDeviceType(audioInterrupt.pid, deviceType)) {
                streamIds.insert(static_cast<int32_t>(audioInterrupt.streamId));
        }
    }

    return streamIds;
}

std::vector<int32_t> AudioInterruptService::GetAudioSessionUidList(int32_t zoneId)
{
    std::vector<int32_t> uidList;
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("start to find audio session uid in zone %{public}d", zoneId);
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG((itZone != zonesMap_.end()) && (itZone->second != nullptr),
        uidList, "cannot find zoneid in zonesMap");

    auto audioFocusInfoList = itZone->second->audioFocusInfoList;
    for (const auto &iter : audioFocusInfoList) {
        if (iter.first.isAudioSessionInterrupt) {
            AUDIO_INFO_LOG("find uid : %{public}d", iter.first.uid);
            uidList.push_back(iter.first.uid);
        }
    }
    return uidList;
}

StreamUsage AudioInterruptService::GetAudioSessionStreamUsage(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(sessionService_ != nullptr, STREAM_USAGE_INVALID, "sessionService_ is nullptr!");
    return sessionService_->GetAudioSessionStreamUsage(callerPid);
}

void AudioInterruptService::ProcessRemoteInterrupt(std::set<int32_t> streamIds, InterruptEventInternal interruptEvent)
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
        bool needRemoveCurIter = false;
        for (auto streamId : streamIds) {
            if (streamId != static_cast<int32_t> (iterActive->first.streamId)) {
                continue;
            }
            AudioInterrupt currentInterrupt = iterActive->first;
            needRemoveCurIter = SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
            SendInterruptEventCallback(interruptEvent, streamId, currentInterrupt);
            if (interruptEvent.hintType == INTERRUPT_HINT_PAUSE || interruptEvent.hintType == INTERRUPT_HINT_STOP) {
                SendFocusChangeEvent(ZONEID_DEFAULT, AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY,
                    currentInterrupt);
            }
        }
        if (needRemoveCurIter) {
            iterActive = tmpFocusInfoList.erase(iterActive);
        } else {
            ++iterActive;
        }
    }
    targetZoneIt->second->audioFocusInfoList = tmpFocusInfoList;
}

void AudioInterruptService::ProcessActiveInterrupt(const int32_t zoneId, const AudioInterrupt &incomingInterrupt)
{
    // Use local variable to record target focus info list, can be optimized
    auto targetZoneIt = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG(targetZoneIt != zonesMap_.end(), "can not find zone id");
    CHECK_AND_RETURN_LOG(policyServer_ != nullptr, "policyServer nullptr");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList {};
    if (targetZoneIt != zonesMap_.end()) {
        tmpFocusInfoList = targetZoneIt->second->audioFocusInfoList;
        targetZoneIt->second->zoneId = zoneId;
    }

    std::list<int32_t> removeFocusInfoPidList = {};
    InterruptDfxBuilder dfxBuilder;
    for (auto iterActive = tmpFocusInfoList.begin(); iterActive != tmpFocusInfoList.end();) {
        AudioFocusEntry focusEntry =
            focusCfgMap_[std::make_pair((iterActive->first).audioFocusType, incomingInterrupt.audioFocusType)];
        UpdateAudioFocusStrategy(iterActive->first, incomingInterrupt, focusEntry);
        if (focusEntry.actionOn != CURRENT || IsSameAppInShareMode(incomingInterrupt, iterActive->first) ||
            iterActive->second == PLACEHOLDER || CanMixForSession(incomingInterrupt, iterActive->first, focusEntry) ||
            // incomming peeling should not stop/pause/duck other playing instances
            (IsLowestPriorityRecording(incomingInterrupt) && !IsRecordingInterruption(iterActive->first))) {
            ++iterActive;
            continue;
        }

        // other new recording should stop the existing peeling anyway
        if (IsLowestPriorityRecording(iterActive->first) && IsRecordingInterruption(incomingInterrupt)) {
            focusEntry.actionOn = CURRENT;
            focusEntry.forceType = INTERRUPT_FORCE;
            focusEntry.hintType = INTERRUPT_HINT_STOP;
        }

        InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, focusEntry.forceType, INTERRUPT_HINT_NONE, 1.0f};
        uint32_t activeStreamId = (iterActive->first).streamId;
        bool removeFocusInfo = false;
        ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt, removeFocusInfo, interruptEvent);
        AudioInterrupt currentInterrupt = iterActive->first;
        if (removeFocusInfo) {
            RemoveFocusInfo(iterActive, tmpFocusInfoList, targetZoneIt->second, removeFocusInfoPidList);
        } else {
            ++iterActive;
        }
        uint8_t appstate = GetAppState(currentInterrupt.pid);
        auto info = AudioBundleManager::GetBundleInfoFromUid(currentInterrupt.uid);
        dfxBuilder.WriteEffectMsg(appstate, info.name, currentInterrupt, interruptEvent.hintType);
        SendActiveInterruptEvent(activeStreamId, interruptEvent, incomingInterrupt, currentInterrupt);
    }

    WriteStartDfxMsg(dfxBuilder, incomingInterrupt);
    targetZoneIt->second->audioFocusInfoList = tmpFocusInfoList;
    zonesMap_[zoneId] = targetZoneIt->second;
    SendActiveVolumeTypeChangeEvent(zoneId);
    RemoveAllPlaceholderInterrupt(removeFocusInfoPidList);
}

void AudioInterruptService::RemoveAllPlaceholderInterrupt(std::list<int32_t> &removeFocusInfoPidList)
{
    for (auto pid : removeFocusInfoPidList) {
        RemovePlaceholderInterruptForSession(pid);
    }
}

void AudioInterruptService::RemoveFocusInfo(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive,
    std::list<std::pair<AudioInterrupt, AudioFocuState>> &tmpFocusInfoList,
    std::shared_ptr<AudioInterruptZone> &zoneInfo,
    std::list<int32_t> &removeFocusInfoPidList)
{
    int32_t pidToRemove = (iterActive->first).pid;
    uint32_t streamId = (iterActive->first).streamId;
    iterActive = tmpFocusInfoList.erase(iterActive);
    zoneInfo->audioFocusInfoList = tmpFocusInfoList;
    bool isAudioSessionDeactivated = false;
    if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(pidToRemove)) {
        isAudioSessionDeactivated = HandleLowPriorityEvent(pidToRemove, streamId);
    }
    if (isAudioSessionDeactivated) {
        removeFocusInfoPidList.push_back(pidToRemove);
    }
}

bool AudioInterruptService::HandleLowPriorityEvent(const int32_t pid, const uint32_t streamId)
{
    // If AudioSession is deactivated, return true, otherwise, return false.
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return false;
    }
    auto audioSession = sessionService_->GetAudioSessionByPid(pid);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("audioSession is nullptr!");
        return false;
    }

    audioSession->RemoveStreamInfo(streamId);
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
        return true;
    }
    return false;
}

void AudioInterruptService::SendActiveInterruptEvent(const uint32_t activeStreamId,
    const InterruptEventInternal &interruptEvent, const AudioInterrupt &incomingInterrupt,
    const AudioInterrupt &activeInterrupt)
{
    if (interruptEvent.hintType != INTERRUPT_HINT_NONE) {
        AUDIO_INFO_LOG("OnInterrupt for active streamId:%{public}d, hintType:%{public}d. By streamId:%{public}d",
            activeStreamId, interruptEvent.hintType, incomingInterrupt.streamId);
        SendInterruptEventCallback(interruptEvent, activeStreamId, activeInterrupt);
        // focus remove or state change
        SendFocusChangeEvent(ZONEID_DEFAULT, AudioPolicyServerHandler::NONE_CALLBACK_CATEGORY,
            incomingInterrupt);
    }
}

void AudioInterruptService::ProcessAudioScene(const AudioInterrupt &audioInterrupt, const uint32_t &incomingStreamId,
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
        // If the session is present in audioFocusInfoList and the placeholder's stream type is not VoIP communication,
        // and the incoming stream type is not Capturer, remove and treat it as a new request
        AUDIO_DEBUG_LOG("audioFocusInfoList is not empty");
        audioFocusInfoList.remove_if(
            [&audioInterrupt, this](const std::pair<AudioInterrupt, AudioFocuState> &audioFocus) {
            return AudioFocusInfoListRemovalCondition(audioInterrupt, audioFocus);
        });

        itZone->second->audioFocusInfoList = audioFocusInfoList;
        zonesMap_[zoneId] = itZone->second;
        if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(pid)) {
            std::shared_ptr<AudioSession> tempSession = sessionService_->GetAudioSessionByPid(pid);
            CHECK_AND_RETURN_LOG(tempSession != nullptr, "audio session is null");
            tempSession->RemoveStreamInfo(incomingStreamId);
        }
    }

    if (audioFocusInfoList.empty()) {
        InterruptDfxBuilder dfxBuilder;
        WriteStartDfxMsg(dfxBuilder, audioInterrupt);
        AUDIO_INFO_LOG("audioFocusInfoList is empty");
        if (itZone->second != nullptr) {
            itZone->second->audioFocusInfoList.emplace_back(std::make_pair(audioInterrupt, ACTIVE));
            zonesMap_[zoneId] = itZone->second;
        }
        if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(pid)) {
            std::shared_ptr<AudioSession> tempAudioSession = sessionService_->GetAudioSessionByPid(pid);
            CHECK_AND_RETURN_LOG(tempAudioSession != nullptr, "audio session is null");
            tempAudioSession->AddStreamInfo(audioInterrupt);
        }
        SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::REQUEST_CALLBACK_CATEGORY, audioInterrupt);
        SendActiveVolumeTypeChangeEvent(zoneId);
        AudioScene targetAudioScene = GetHighestPriorityAudioScene(zoneId);
        UpdateAudioSceneFromInterrupt(targetAudioScene, ACTIVATE_AUDIO_INTERRUPT, zoneId);
        shouldReturnSuccess = true;
        return;
    }
    shouldReturnSuccess = false;
}

bool AudioInterruptService::AudioFocusInfoListRemovalCondition(const AudioInterrupt &audioInterrupt,
    const std::pair<AudioInterrupt, AudioFocuState> &audioFocus)
{
    return audioFocus.first.streamId == audioInterrupt.streamId ||
        (audioFocus.first.pid == audioInterrupt.pid && audioFocus.second == PLACEHOLDER &&
        audioInterrupt.audioFocusType.sourceType == SOURCE_TYPE_INVALID &&
        audioFocus.first.audioFocusType.streamType != STREAM_VOICE_COMMUNICATION);
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

bool AudioInterruptService::IsMediaStream(AudioStreamType audioStreamType)
{
    if (audioStreamType == STREAM_MUSIC || audioStreamType == STREAM_MOVIE || audioStreamType == STREAM_SPEECH) {
        return true;
    }
    return false;
}

int32_t AudioInterruptService::SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object)
{
    AUDIO_INFO_LOG("Set query bundle name list callback");
    queryBundleNameListCallback_ = iface_cast<IStandardAudioPolicyManagerListener>(object);
    if (queryBundleNameListCallback_ == nullptr) {
        AUDIO_ERR_LOG("Client type callback is null");
        return ERR_CALLBACK_NOT_REGISTERED;
    }
    return SUCCESS;
}

std::string AudioInterruptService::GetRealBundleName(uint32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(policyServer_ != nullptr, "", "policyServer nullptr");
    if (IPCSkeleton::GetCallingUid() == MEDIA_SA_UID) {
        auto info = AudioBundleManager::GetBundleInfoFromUid(uid);
        return info.name;
    }
    return AudioBundleManager::GetBundleName();
}

void AudioInterruptService::UpdateAudioFocusStrategy(const AudioInterrupt &currentInterrupt,
    const AudioInterrupt &incomingInterrupt, AudioFocusEntry &focusEntry)
{
    int32_t uid = incomingInterrupt.uid;
    int32_t currentPid = currentInterrupt.pid;
    int32_t incomingPid = incomingInterrupt.pid;
    AudioFocusType incomingAudioFocusType = incomingInterrupt.audioFocusType;
    AudioFocusType existAudioFocusType = currentInterrupt.audioFocusType;
    std::string bundleName = GetRealBundleName(static_cast<uint32_t>(uid));
    CHECK_AND_RETURN_LOG(!bundleName.empty(), "bundleName is empty");
    AudioStreamType existStreamType = existAudioFocusType.streamType;
    AudioStreamType incomingStreamType = incomingAudioFocusType.streamType;
    SourceType existSourceType = existAudioFocusType.sourceType;
    SourceType incomingSourceType = incomingAudioFocusType.sourceType;
    UpdateFocusStrategy(bundleName, focusEntry, IsMediaStream(existStreamType), IsMediaStream(incomingStreamType));
    if (uid == static_cast<int32_t>(AUDIO_ID)) {
        AUDIO_INFO_LOG("lake app:%{public}s access", std::to_string(uid).c_str());
        UpdateMicFocusStrategy(existSourceType, incomingSourceType, std::to_string(uid), focusEntry);
    } else {
        UpdateMicFocusStrategy(existSourceType, incomingSourceType, bundleName, focusEntry);
    }
    UpdateWindowFocusStrategy(currentPid, incomingPid, existStreamType, incomingStreamType, focusEntry);
    UpdateMuteAudioFocusStrategy(currentInterrupt, incomingInterrupt, focusEntry);
}

void AudioInterruptService::UpdateFocusStrategy(const std::string &bundleName,
    AudioFocusEntry &focusEntry, bool isExistMediaStream, bool isIncomingMediaStream)
{
    bool ret = false;
    if (queryBundleNameListCallback_ != nullptr) {
        queryBundleNameListCallback_->OnQueryBundleNameIsInList(bundleName, "audio_param", ret);
    }
    if (isExistMediaStream && isIncomingMediaStream && ret &&
        focusEntry.hintType == INTERRUPT_HINT_STOP) {
        focusEntry.hintType = INTERRUPT_HINT_PAUSE;
        AUDIO_INFO_LOG("%{public}s update audio focus strategy", bundleName.c_str());
    }
}

void AudioInterruptService::UpdateMicFocusStrategy(SourceType existSourceType,
    SourceType incomingSourceType, const std::string &bundleName, AudioFocusEntry &focusEntry)
{
    if (incomingSourceType == SOURCE_TYPE_INVALID || existSourceType == SOURCE_TYPE_INVALID) {
        AUDIO_INFO_LOG("Not a recording stream access");
        return;
    }
    bool ret = false;
    if (queryBundleNameListCallback_ != nullptr) {
        queryBundleNameListCallback_->OnQueryBundleNameIsInList(bundleName, "audio_micfocus_list", ret);
    }
    if (existSourceType == SOURCE_TYPE_MIC && IsMicSource(incomingSourceType) && ret) {
        focusEntry.hintType = INTERRUPT_HINT_NONE;
        AUDIO_INFO_LOG("audio_micfocus_list : %{public}s update mic focus strategy", bundleName.c_str());
    }
}

bool AudioInterruptService::IsMicSource(SourceType sourceType)
{
    return (sourceType == SOURCE_TYPE_VOICE_CALL ||
            sourceType == SOURCE_TYPE_VOICE_TRANSCRIPTION||
            sourceType == SOURCE_TYPE_VOICE_COMMUNICATION);
}

bool AudioInterruptService::CheckWindowState(const int32_t pid)
{
    auto sceneSessionManager = Rosen::SessionManagerLite::GetInstance().GetSceneSessionManagerLiteProxy();
    if (sceneSessionManager == nullptr) {
        AUDIO_INFO_LOG("AudioInterruptService null manager");
        return false;
    }
    std::vector<Rosen::MainWindowState> windowStates;
    Rosen::WSError ret = sceneSessionManager->GetMainWindowStatesByPid(pid, windowStates);
    if (ret != Rosen::WSError::WS_OK || windowStates.empty()) {
        AUDIO_INFO_LOG("AudioInterruptService fail GetWindow");
        return false;
    }
    for (auto &windowState : windowStates) {
        if (windowState.isVisible_ && (windowState.state_ == (int32_t) Rosen::SessionState::STATE_ACTIVE ||
            windowState.state_ == (int32_t) Rosen::SessionState::STATE_FOREGROUND)) {
            AUDIO_INFO_LOG("AudioInterruptService app window front desk");
            return true;
        }
    }
    return false;
}

void AudioInterruptService::UpdateWindowFocusStrategy(const int32_t &currentPid, const int32_t &incomingPid,
    const AudioStreamType &existStreamType, const AudioStreamType &incomingStreamType, AudioFocusEntry &focusEntry)
{
    if (!CheckWindowState(currentPid) || !CheckWindowState(incomingPid)) {
        AUDIO_INFO_LOG("currentWindowState: %{public}d incomingWindowState: %{public}d"
            " Not all front desk audio access", CheckWindowState(currentPid), CheckWindowState(incomingPid));
        return;
    }
    if ((existStreamType == STREAM_MUSIC ||
        existStreamType == STREAM_MOVIE || existStreamType == STREAM_SPEECH) &&
        (incomingStreamType == STREAM_MUSIC || incomingStreamType == STREAM_MOVIE ||
        incomingStreamType == STREAM_SPEECH)) {
        focusEntry.hintType = INTERRUPT_HINT_NONE;
        focusEntry.actionOn = INCOMING;
        AUDIO_INFO_LOG("The media windowStates concurrent");
        return;
    }
}

bool AudioInterruptService::FocusEntryContinue(std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator
    &iterActive, AudioFocusEntry &focusEntry, const AudioInterrupt &incomingInterrupt)
{
    SourceType incomingSourceType = incomingInterrupt.audioFocusType.sourceType;
    std::vector<SourceType> incomingConcurrentSources = incomingInterrupt.currencySources.sourcesTypes;
    if (focusEntry.actionOn == CURRENT || iterActive->second == PLACEHOLDER ||
            CanMixForSession(incomingInterrupt, iterActive->first, focusEntry)) {
        return true;
    }
    if (((focusEntry.actionOn == INCOMING && focusEntry.hintType == INTERRUPT_HINT_PAUSE) || focusEntry.isReject) &&
        (IsAudioSourceConcurrency((iterActive->first).audioFocusType.sourceType, incomingSourceType,
        (iterActive->first).currencySources.sourcesTypes, incomingConcurrentSources) ||
        // if the rejection is caused by the existing peeling recording, just ignore it
        IsLowestPriorityRecording(iterActive->first))) {
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

    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator activeInterrupt = audioFocusInfoList.end();
    int32_t res = ProcessActiveStreamFocus(audioFocusInfoList, incomingInterrupt, incomingState, activeInterrupt);
    if ((incomingState >= PAUSE || res != SUCCESS) && activeInterrupt != audioFocusInfoList.end()) {
        ReportRecordGetFocusFail(incomingInterrupt, activeInterrupt->first,
            res == SUCCESS ? RECORD_ERROR_GET_FOCUS_FAIL : RECORD_ERROR_NO_FOCUS_CFG);
    }
    CHECK_AND_RETURN_RET_LOG(res == SUCCESS, res, "ProcessActiveStreamFocus fail");
    HandleIncomingState(zoneId, incomingState, interruptEvent, incomingInterrupt);
    AddToAudioFocusInfoList(itZone->second, zoneId, incomingInterrupt, incomingState);
    SendInterruptEventToIncomingStream(interruptEvent, incomingInterrupt);
    if (IsGameAvoidCallbackCase(incomingInterrupt) && incomingState == PAUSE) {
        return SUCCESS;
    }
    return incomingState >= PAUSE ? ERR_FOCUS_DENIED : SUCCESS;
}

int32_t AudioInterruptService::ProcessFocusEntryForAudioSession(
    const int32_t zoneId, const int32_t callerPid, bool &updateScene)
{
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr!");
        return ERR_UNKNOWN;
    }

    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG((itZone != zonesMap_.end()) && (itZone->second != nullptr), ERROR, "can not find zone");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList = itZone->second->audioFocusInfoList;

    auto isAudioSessionFocusPresent = [callerPid] (const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.pid == callerPid && pair.first.isAudioSessionInterrupt;
    };

    AudioInterrupt audioInterrupt = sessionService_->GenerateFakeAudioInterrupt(callerPid);
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isAudioSessionFocusPresent);
    // It is possible that the reactivation of the audio session was caused by changing the session scene or strategy.
    bool isFirstTimeActiveAudioSession = true;
    if (iter != audioFocusInfoList.end()) {
        audioFocusInfoList.erase(iter);
        isFirstTimeActiveAudioSession = false;
    }

    itZone->second->audioFocusInfoList = audioFocusInfoList;

    bool tempUpdateScene = false;
    int32_t ret = ActivateAudioInterruptCoreProcedure(zoneId, audioInterrupt, false, tempUpdateScene);
    if (tempUpdateScene) {
        updateScene = true;
    }
    if (ret == SUCCESS) {
        ResumeAudioFocusList(zoneId, false);
    } else {
        return ret;
    }

    if (isFirstTimeActiveAudioSession) {
        sessionService_->ClearStreamInfo(callerPid);
        return HandleExistStreamsForSession(zoneId, callerPid, updateScene);
    }

    return SUCCESS;
}

int32_t AudioInterruptService::HandleExistStreamsForSession(
    const int32_t zoneId, const int32_t callerPid, bool &updateScene)
{
    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_RET_LOG((itZone != zonesMap_.end()) && (itZone->second != nullptr), ERROR, "can not find zone");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList = itZone->second->audioFocusInfoList;

    /* The focus of a single stream should be managed by audio session focus.
    1. This mainly handles streams that already exist before audio session activation.
    2. and to handle the state transition when the audio session resumes from a paused state.
    */
    auto isStreamFocusPresent = [&](const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.pid == callerPid &&
            !pair.first.isAudioSessionInterrupt &&
            !sessionService_->ShouldExcludeStreamType(pair.first);
    };

    bool tempUpdateScene = false;
    for (const auto &it : audioFocusInfoList) {
        if (isStreamFocusPresent(it)) {
            updateScene = true;
            int32_t ret = ActivateAudioInterruptCoreProcedure(zoneId, it.first, true, tempUpdateScene);
            if (ret != SUCCESS) {
                return ret;
            }
        }
    }

    return SUCCESS;
}

bool AudioInterruptService::ShouldBypassAudioSessionFocus(const int32_t zoneId, const AudioInterrupt &incomingInterrupt)
{
    AUDIO_INFO_LOG("incomingInterrupt info: pid = %{public}d, isAudioSessionInterrupt = %{public}d,"
                   "streamId = %{public}u, streamType = %{public}d",
                   incomingInterrupt.pid,
                   incomingInterrupt.isAudioSessionInterrupt,
                   incomingInterrupt.streamId,
                   incomingInterrupt.audioFocusType.streamType);
    if (!HasAudioSessionFakeInterrupt(zoneId, incomingInterrupt.pid)) {
        return false;
    }

    if (incomingInterrupt.isAudioSessionInterrupt) {
        return false;
    }

    if (sessionService_ != nullptr && sessionService_->ShouldBypassFocusForStream(incomingInterrupt)) {
        return true;
    }

    return false;
}

void AudioInterruptService::TryHandleStreamCallbackInSession(
    const int32_t zoneId, const AudioInterrupt &incomingInterrupt)
{
    if (handler_ == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyServerHandler is nullptr");
        return;
    }

    auto itZone = zonesMap_.find(zoneId);
    CHECK_AND_RETURN_LOG((itZone != zonesMap_.end()) && (itZone->second != nullptr), "can not find zone");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList = itZone->second->audioFocusInfoList;

    auto isAudioSessionFocusPresent = [&incomingInterrupt](const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.pid == incomingInterrupt.pid && pair.first.isAudioSessionInterrupt &&
               (pair.second == AudioFocuState::PAUSE || pair.second == AudioFocuState::DUCK);
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isAudioSessionFocusPresent);
    if (iter == audioFocusInfoList.end()) {
        return;
    }

    if (iter->second == AudioFocuState::DUCK) {
        InterruptEventInternal interruptEvent {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_DUCK, DUCK_FACTOR};
        handler_->SendInterruptEventWithStreamIdCallback(interruptEvent, incomingInterrupt.streamId);
    }

    if (iter->second == AudioFocuState::PAUSE) {
        InterruptEventInternal interruptEvent {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_PAUSE, 1.0f};
        handler_->SendInterruptEventWithStreamIdCallback(interruptEvent, incomingInterrupt.streamId);
    }
}

AudioFocuState AudioInterruptService::GetNewIncomingState(InterruptHint hintType, AudioFocuState oldState)
{
    auto pos = HINT_STATE_MAP.find(hintType);
    AudioFocuState newState = (pos == HINT_STATE_MAP.end()) ? ACTIVE : pos->second;
    return (newState > oldState) ? newState : oldState;
}

bool AudioInterruptService::IsLowestPriorityRecording(const AudioInterrupt &audioInterrupt)
{
    if (audioInterrupt.currencySources.sourcesTypes.size() == 1 &&
        audioInterrupt.currencySources.sourcesTypes[0] == SOURCE_TYPE_INVALID) {
        AUDIO_INFO_LOG("PEELING AUDIO IsLowestPriorityRecording:%{public}d", audioInterrupt.streamId);
        return true;
    }
    return false;
}

bool AudioInterruptService::IsRecordingInterruption(const AudioInterrupt &audioInterrupt)
{
    return audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID ? true : false;
}

void AudioInterruptService::CheckIncommingFoucsValidity(AudioFocusEntry &focusEntry,
    const AudioInterrupt &incomingInterrupt, std::vector<SourceType> incomingConcurrentSources)
{
    CHECK_AND_RETURN_LOG(interruptClients_.find(incomingInterrupt.streamId) != interruptClients_.end(),
        "interruptClients is nullptr");
    auto uid = interruptClients_[incomingInterrupt.streamId]->GetCallingUid();
    if (IsRecordingInterruption(incomingInterrupt) && incomingConcurrentSources.size() != 0 &&
        (uid == THP_EXTRA_SA_UID || uid == MEDIA_SA_UID)) {
            focusEntry.actionOn = INCOMING;
            focusEntry.isReject = true;
    }
}

void AudioInterruptService::SendInterruptEventToIncomingStream(InterruptEventInternal &interruptEvent,
    const AudioInterrupt &incomingInterrupt)
{
    if (interruptEvent.hintType != INTERRUPT_HINT_NONE) {
        AUDIO_INFO_LOG("OnInterrupt for incoming streamId: %{public}d, hintType: %{public}d",
            incomingInterrupt.streamId, interruptEvent.hintType);
        SendInterruptEventCallback(interruptEvent, incomingInterrupt.streamId, incomingInterrupt);
    }
}

void AudioInterruptService::AddToAudioFocusInfoList(std::shared_ptr<AudioInterruptZone> &audioInterruptZone,
    const int32_t &zoneId, const AudioInterrupt &incomingInterrupt, const AudioFocuState &incomingState)
{
    if (incomingState == STOP) {
        // Deny incoming. No need to add it.
        return;
    }

    audioInterruptZone->zoneId = zoneId;
    audioInterruptZone->audioFocusInfoList.emplace_back(std::make_pair(incomingInterrupt, incomingState));
    zonesMap_[zoneId] = audioInterruptZone;
    SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::REQUEST_CALLBACK_CATEGORY, incomingInterrupt);
    SendActiveVolumeTypeChangeEvent(zoneId);
    if (sessionService_ != nullptr && sessionService_->IsAudioSessionActivated(incomingInterrupt.pid)) {
        auto audioSession = sessionService_->GetAudioSessionByPid(incomingInterrupt.pid);
        if (audioSession == nullptr) {
            AUDIO_ERR_LOG("audioSession is nullptr!");
            return;
        }
        audioSession->AddStreamInfo(incomingInterrupt);
    }
}

void AudioInterruptService::HandleIncomingState(const int32_t &zoneId, const AudioFocuState &incomingState,
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
        } else if (incomingState == MUTED) {
            interruptEvent.hintType = INTERRUPT_HINT_MUTE;
        }
        // Handle existing focus state
        ProcessActiveInterrupt(zoneId, incomingInterrupt);
    }
}

AudioScene AudioInterruptService::GetHighestPriorityAudioScene(const int32_t zoneId) const
{
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    int32_t audioScenePriority = GetAudioScenePriority(audioScene);

    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }
    for (const auto &[interrupt, focuState] : audioFocusInfoList) {
        if (interrupt.isAudioSessionInterrupt) {
            audioScene = GetHighestPriorityAudioSceneFromAudioSession(interrupt, audioScene);
            continue;
        }
        AudioScene itAudioScene = GetAudioSceneFromAudioInterrupt(interrupt);
        int32_t itAudioScenePriority = GetAudioScenePriority(itAudioScene);
        if (itAudioScenePriority >= audioScenePriority) {
            audioScene = itAudioScene;
            audioScenePriority = itAudioScenePriority;
            ownerPid_ = interrupt.pid;
            ownerUid_ = interrupt.uid;
        }
    }

    return audioScene;
}

AudioScene AudioInterruptService::GetHighestPriorityAudioSceneFromAudioSession(
    const AudioInterrupt &audioInterrupt, const AudioScene &audioScene) const
{
    if (sessionService_ == nullptr) {
        return audioScene;
    }

    int32_t audioScenePriority = GetAudioScenePriority(audioScene);
    AudioScene finalAudioScene = audioScene;
    bool hasRingtoneInVoip = false;

    // Handle streams in audio session
    const auto &streamsInSession = sessionService_->GetStreams(audioInterrupt.pid);
    for (auto &it : streamsInSession) {
        AudioScene innerAudioScene = GetAudioSceneFromAudioInterrupt(it);
        int32_t innerAudioScenePriority = GetAudioScenePriority(innerAudioScene);
        if (innerAudioScenePriority >= audioScenePriority) {
            finalAudioScene = innerAudioScene;
            audioScenePriority = innerAudioScenePriority;
            ownerPid_ = audioInterrupt.pid;
            ownerUid_ = audioInterrupt.uid;
        }

        // In the VoIP session scene, the STREAM_VOICE_RING AudioScene should be kept independent
        if (it.audioFocusType.streamType == STREAM_RING &&
            audioInterrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION) {
            hasRingtoneInVoip = true;
        }
    }

    if (hasRingtoneInVoip) {
        return finalAudioScene;
    }

    // Update audio scene for audio session fake audioInterrupt
    AudioScene itAudioScene = GetAudioSceneFromAudioInterrupt(audioInterrupt);
    int32_t itAudioScenePriority = GetAudioScenePriority(itAudioScene);
    if (itAudioScenePriority >= audioScenePriority) {
        finalAudioScene = itAudioScene;
        audioScenePriority = itAudioScenePriority;
        ownerPid_ = audioInterrupt.pid;
        ownerUid_ = audioInterrupt.uid;
    }

    return finalAudioScene;
}

bool AudioInterruptService::HadVoipStatus(const AudioInterrupt &audioInterrupt,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList)
{
    for (const auto &[interrupt, focusState] : audioFocusInfoList) {
        if (audioInterrupt.pid == interrupt.pid && focusState == PLACEHOLDER &&
            interrupt.audioFocusType.streamType == STREAM_VOICE_COMMUNICATION &&
            interrupt.streamId != audioInterrupt.streamId) {
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
            audioSession->RemoveStreamInfo(audioInterrupt.streamId);
            needPlaceHolder = !audioInterrupt.isAudioSessionInterrupt &&
                audioInterrupt.audioFocusType.streamType != STREAM_DEFAULT &&
                audioSession->IsAudioRendererEmpty() &&
                !HadVoipStatus(audioInterrupt, audioFocusInfoList);
        }
    }

    WriteStopDfxMsg(audioInterrupt);
    auto isPresent = [audioInterrupt] (const std::pair<AudioInterrupt, AudioFocuState> &pair) {
        return pair.first.streamId == audioInterrupt.streamId;
    };
    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), isPresent);
    if (iter != audioFocusInfoList.end()) {
        if (needPlaceHolder) {
            // Change the state to PLACEHOLDER because of the active audio session.
            // No need to release interrupt until the audio session is deactivated.
            iter->second = PLACEHOLDER;
            itZone->second->audioFocusInfoList = audioFocusInfoList;
            zonesMap_[zoneId] = itZone->second;
            SendActiveVolumeTypeChangeEvent(zoneId);
            AUDIO_INFO_LOG("Change the state of streamId %{public}u to PLACEHOLDER! (pid %{public}d)",
                audioInterrupt.streamId, audioInterrupt.pid);
            return;
        }
        ResetNonInterruptControl(audioInterrupt);
        audioFocusInfoList.erase(iter);
        itZone->second->zoneId = zoneId;
        itZone->second->audioFocusInfoList = audioFocusInfoList;
        zonesMap_[zoneId] = itZone->second;
        SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY, audioInterrupt);
        SendActiveVolumeTypeChangeEvent(zoneId);
    } else {
        // If it was not in the audioFocusInfoList, no need to take any action on other sessions, just return.
        AUDIO_DEBUG_LOG("stream (streamId %{public}u) is not active now", audioInterrupt.streamId);
        return;
    }

    if (itZone->second->focusStrategy == AudioZoneFocusStrategy::DISTRIBUTED_FOCUS_STRATEGY) {
        AUDIO_INFO_LOG("zone: %{public}d distributed focus strategy not resume when deactivate interrupt",
            itZone->first);
        return;
    }
    // resume if other session was forced paused or ducked
    ResumeAudioFocusList(zoneId, isSessionTimeout);

    return;
}

void AudioInterruptService::UpdateAudioSceneFromInterrupt(const AudioScene audioScene,
    AudioInterruptChangeType changeType, int32_t zoneId)
{
    CHECK_AND_RETURN_LOG(policyServer_ != nullptr, "policyServer nullptr");
    CHECK_AND_RETURN_LOG(zoneId == ZONEID_DEFAULT, "zoneId %{public}d is not default", zoneId);
    int32_t scene = AUDIO_SCENE_INVALID;
    policyServer_->GetAudioScene(scene);
    AudioScene currentAudioScene = static_cast<AudioScene>(scene);
    if (currentAudioScene != audioScene) {
        AUDIO_PRERELEASE_LOGI("currentScene: %{public}d, targetScene: %{public}d, changeType: %{public}d",
            currentAudioScene, audioScene, changeType);
    }

    switch (changeType) {
        case ACTIVATE_AUDIO_INTERRUPT:
            break;
        case DEACTIVATE_AUDIO_INTERRUPT:
            if (GetAudioScenePriority(audioScene) >= GetAudioScenePriority(currentAudioScene)) {
                AudioStateManager::GetAudioStateManager().SetAudioSceneOwnerUid(audioScene == 0 ? 0 : ownerUid_);
                return;
            }
            break;
        default:
            AUDIO_ERR_LOG("unexpected changeType: %{public}d", changeType);
            return;
    }
    policyServer_->SetAudioSceneInternal(audioScene, ownerUid_, ownerPid_);
}

bool AudioInterruptService::EvaluateWhetherContinue(const AudioInterrupt &incoming, const AudioInterrupt
    &inprocessing, AudioFocusEntry &focusEntry, bool bConcurrency)
{
    if (focusEntry.hintType == INTERRUPT_HINT_MUTE) {
        AUDIO_INFO_LOG("sessionId: %{public}u can not skip", inprocessing.streamId);
        return false;
    }

    if (CanMixForSession(incoming, inprocessing, focusEntry) ||
        ((focusEntry.hintType == INTERRUPT_HINT_PAUSE || focusEntry.hintType == INTERRUPT_HINT_STOP) && bConcurrency)) {
        return true;
    }
    UpdateHintTypeForExistingSession(incoming, focusEntry);
    if (IsGameAvoidCallbackCase(incoming) &&
        focusEntry.hintType == INTERRUPT_HINT_STOP) {
        focusEntry.hintType = INTERRUPT_HINT_PAUSE;
        AUDIO_INFO_LOG("focusEntry.hintType: %{public}d", focusEntry.hintType);
    }
    return false;
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioInterruptService::SimulateFocusEntry(const int32_t zoneId)
{
    AUDIO_INFO_LOG("Simulate in");
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
            UpdateAudioFocusStrategy(inprocessing, incoming, focusEntry);
            SourceType existSourceType = inprocessing.audioFocusType.sourceType;
            std::vector<SourceType> existConcurrentSources = inprocessing.currencySources.sourcesTypes;
            bool bConcurrency = IsAudioSourceConcurrency(existSourceType, incomingSourceType,
                existConcurrentSources, incomingConcurrentSources);
            if (EvaluateWhetherContinue(incoming, inprocessing, focusEntry, bConcurrency)) { continue; }
            if (focusEntry.hintType == INTERRUPT_HINT_STOP &&
                IsGameAvoidCallbackCase(inprocessing)) {
                focusEntry.hintType = INTERRUPT_HINT_PAUSE;
            }
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
    uint32_t streamId = audioInterrupt.streamId;

    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is nullptr");

    InterruptEventInternal forceActive {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_RESUME, 1.0f};
    // RESUME event should be INTERRUPT_SHARE. But mark it as INTERRUPT_FORCE here for state checking.
    // The force type will be changed to INTERRUPT_SHARE in client.
    InterruptEventInternal forceUnduck {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_UNDUCK, 1.0f};
    InterruptEventInternal forceDuck {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_DUCK, DUCK_FACTOR};
    InterruptEventInternal forcePause {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_PAUSE, 1.0f};
    InterruptEventInternal forceUnmute {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_UNMUTE, 1.0f};
    switch (newState) {
        case ACTIVE:
            if (oldState == PAUSE) {
                SendInterruptEventCallback(forceActive, streamId, audioInterrupt);
                removeFocusInfo = true;
            } else if (oldState == DUCK) {
                SendInterruptEventCallback(forceUnduck, streamId, audioInterrupt);
            } else if (oldState == MUTED) {
                SendInterruptEventCallback(forceUnmute, streamId, audioInterrupt);
            }
            break;
        case DUCK:
            if (oldState == PAUSE) {
                SendInterruptEventCallback(forceActive, streamId, audioInterrupt);
                removeFocusInfo = true;
            } else if (oldState == ACTIVE) {
                SendInterruptEventCallback(forceDuck, streamId, audioInterrupt);
            }
            break;
        case PAUSE:
            if (oldState == DUCK) {
                SendInterruptEventCallback(forceUnduck, streamId, audioInterrupt);
            }
            SendInterruptEventCallback(forcePause, streamId, audioInterrupt);
            break;
        default:
            break;
    }
    iterActive->second = newState;
}

void AudioInterruptService::SendInterruptEventCallback(const InterruptEventInternal &interruptEvent,
    const uint32_t &streamId, const AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "dfxCollector is null");
    AUDIO_INFO_LOG("hintType= %{public}d", interruptEvent.hintType);
    InterruptDfxBuilder dfxBuilder;
    auto& [infoIdx, effectIdx] = dfxCollector_->GetDfxIndexes(audioInterrupt.streamId);

    auto pos = HINT_STAGE_MAP.find(interruptEvent.hintType);
    auto stage = (pos == HINT_STAGE_MAP.end()) ? INTERRUPT_STAGE_STOPPED : pos->second;
    dfxBuilder.WriteActionMsg(infoIdx, effectIdx, stage);
    dfxCollector_->AddDfxMsg(audioInterrupt.streamId, dfxBuilder.GetResult());

    if (audioInterrupt.strategy == InterruptStrategy::MUTE) {
        SetLatestMuteState(interruptEvent, streamId);
    }

    if (handler_ == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyServerHandler is nullptr");
        return;
    }

    if (audioInterrupt.isAudioSessionInterrupt) {
        SendAudioSessionInterruptEventCallback(interruptEvent, audioInterrupt);
    } else {
        handler_->SendInterruptEventWithStreamIdCallback(interruptEvent, streamId);
    }
}

void AudioInterruptService::SendAudioSessionInterruptEventCallback(
    const InterruptEventInternal &interruptEvent, const AudioInterrupt &audioInterrupt)
{
    if (handler_ == nullptr) {
        AUDIO_ERR_LOG("AudioPolicyServerHandler is nullptr");
        return;
    }

    /*
    For audio session focus, the session's callbacks must be processed first,
    then process all stream callbacks managed under that session.
    */
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("sessionService_ is nullptr");
        return;
    }

    // Processes the situation where the audio session fake interrupt is preempted by other applications.
    if (sessionService_->ShouldAudioSessionProcessHintType(interruptEvent.hintType)) {
        handler_->SendInterruptEventCallbackForAudioSession(interruptEvent, audioInterrupt);
        // Simulate the deactivation of the audio session.
        if (interruptEvent.hintType == INTERRUPT_HINT_STOP || interruptEvent.hintType == INTERRUPT_HINT_RESUME) {
            DeactivateAudioSessionInFakeFocusMode(audioInterrupt.pid, interruptEvent.hintType);
        }
    }
    /*
    Callback for all streams when the audio session's fake interrupt state changes.
    INTERRUPT_HINT_STOP should not be processed here, because the audio session has been deactivated.
    */
    if (sessionService_->ShouldAudioStreamProcessHintType(interruptEvent.hintType)) {
        const auto &audioInterrupts = sessionService_->GetStreams(audioInterrupt.pid);
        for (auto &it : audioInterrupts) {
            handler_->SendInterruptEventWithStreamIdCallback(interruptEvent, it.streamId);
        }
    }
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
                    interruptToRemove.streamId);
                SendSessionTimeOutStopEvent(zoneId, interruptToRemove, audioFocusInfoList);
                continue;
            }
            AUDIO_INFO_LOG("State change: streamId %{public}d, oldstate %{public}d, "\
                "newState %{public}d", (iterActive->first).streamId, oldState, newState);
            SendInterruptEvent(oldState, newState, iterActive, removeFocusInfo);
        }

        if (removeFocusInfo && !IsGameAvoidCallbackCase(iterActive->first)) {
            AudioInterrupt interruptToRemove = iterActive->first;
            iterActive = audioFocusInfoList.erase(iterActive);
            iterNew = newAudioFocuInfoList.erase(iterNew);
            AUDIO_INFO_LOG("Remove focus info from focus list, streamId: %{public}d", interruptToRemove.streamId);
            SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY, interruptToRemove);
        } else {
            highestPriorityAudioScene =
                RefreshAudioSceneFromAudioInterrupt(iterActive->first, highestPriorityAudioScene);
            ++iterActive;
            ++iterNew;
        }
    }

    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        itZone->second->audioFocusInfoList = audioFocusInfoList;
        SendActiveVolumeTypeChangeEvent(zoneId);
    }
    UpdateAudioSceneFromInterrupt(highestPriorityAudioScene, DEACTIVATE_AUDIO_INTERRUPT, zoneId);
}

AudioScene AudioInterruptService::RefreshAudioSceneFromAudioInterrupt(const AudioInterrupt &audioInterrupt,
    AudioScene &highestPriorityAudioScene)
{
    if (audioInterrupt.isAudioSessionInterrupt) {
        return GetHighestPriorityAudioSceneFromAudioSession(audioInterrupt, highestPriorityAudioScene);
    }

    AudioScene targetAudioScene = GetAudioSceneFromAudioInterrupt(audioInterrupt);
    if (GetAudioScenePriority(targetAudioScene) >= GetAudioScenePriority(highestPriorityAudioScene)) {
        highestPriorityAudioScene = targetAudioScene;
        ownerPid_ = audioInterrupt.pid;
        ownerUid_ = audioInterrupt.uid;
    }
    return highestPriorityAudioScene;
}

void AudioInterruptService::SendSessionTimeOutStopEvent(const int32_t zoneId, const AudioInterrupt &audioInterrupt,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList)
{
    // When the audio session is timeout, change resume event to stop event and delete the interttupt.
    InterruptEventInternal stopEvent {INTERRUPT_TYPE_END, INTERRUPT_FORCE, INTERRUPT_HINT_STOP, 1.0f};
    SendInterruptEventCallback(stopEvent, audioInterrupt.streamId, audioInterrupt);

    auto itZone = zonesMap_.find(zoneId);
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        itZone->second->zoneId = zoneId;
        itZone->second->audioFocusInfoList = audioFocusInfoList;
        zonesMap_[zoneId] = itZone->second;
    }
    SendFocusChangeEvent(zoneId, AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY, audioInterrupt);
    SendActiveVolumeTypeChangeEvent(zoneId);
}

void AudioInterruptService::SendActiveVolumeTypeChangeEvent(const int32_t zoneId)
{
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is null");

    const uint32_t DEFAUFT_UID = 0;
    AudioStreamType streamInFocus = GetStreamInFocusInternal(DEFAUFT_UID, zoneId);
    streamInFocus = VolumeUtils::GetVolumeTypeFromStreamType(streamInFocus);
    if (activeStreamType_ != streamInFocus) {
        AUDIO_INFO_LOG("activeStreamType_: %{public}d, streamInFocus: %{public}d",
            activeStreamType_, streamInFocus);

        activeStreamType_ = streamInFocus;
        handler_->SendActiveVolumeTypeChangeCallback(activeStreamType_);
    }
}

void AudioInterruptService::SendFocusChangeEvent(const int32_t zoneId, int32_t callbackCategory,
    const AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is null");
    CHECK_AND_RETURN_LOG(zoneId == ZONEID_DEFAULT, "zoneId %{public}d is not default", zoneId);

    auto itZone = zonesMap_.find(zoneId);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList {};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    if (IsRecordingInterruption(audioInterrupt)) {
        if (callbackCategory == static_cast<int32_t>(AudioPolicyServerHandler::REQUEST_CALLBACK_CATEGORY)) {
            SetSessionMuteState(audioInterrupt.streamId, true, audioInterrupt.strategy != InterruptStrategy::DEFAULT);
        } else if (callbackCategory == static_cast<int32_t>(AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY)) {
            SetSessionMuteState(audioInterrupt.streamId, false, audioInterrupt.strategy != InterruptStrategy::DEFAULT);
        }
    }

    handler_->SendAudioFocusInfoChangeCallback(callbackCategory, audioInterrupt, audioFocusInfoList);
}

void AudioInterruptService::RemoveExistingFocus(
    const int32_t appUid, std::unordered_set<int32_t> &uidActivedSessions)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (zonesMap_.empty()) {
        AUDIO_ERR_LOG("zonesMap is empty");
        return;
    }

    for (auto itZone : zonesMap_) {
        bool isNeedRefresh = false;
        auto audioFocusInfoList = itZone.second->audioFocusInfoList;
        for (auto iter = audioFocusInfoList.begin(); iter != audioFocusInfoList.end();) {
            if (iter->first.uid != appUid) {
                iter++;
                continue;
            }
            AUDIO_INFO_LOG("itZone = %{public}d, streamId = %{public}d",
                itZone.first, iter->first.streamId);
            uidActivedSessions.insert(iter->first.streamId);
            iter = audioFocusInfoList.erase(iter);
            isNeedRefresh = true;
        }
        if (isNeedRefresh) {
            zonesMap_[itZone.first]->audioFocusInfoList = audioFocusInfoList;
            ResumeAudioFocusList(itZone.first, false);
        }
    }
}

void AudioInterruptService::ResumeFocusByStreamId(
    const int32_t streamId, const InterruptEventInternal interruptEventResume)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("Remove Focus By StreamId = %{public}d", streamId);
    if (interruptClients_.find(streamId) != interruptClients_.end() && handler_ != nullptr) {
        handler_->SendInterruptEventWithStreamIdCallback(interruptEventResume, streamId);
    }
}

// LCOV_EXCL_START
void AudioInterruptService::DispatchInterruptEventWithStreamId(uint32_t streamId,
    InterruptEventInternal &interruptEvent)
{
    CHECK_AND_RETURN_LOG(streamId >= MIN_STREAMID && streamId <= MAX_STREAMID,
        "EntryPoint Taint Mark:arg streamId: %{public}u is tained", streamId);
    std::lock_guard<std::mutex> lock(mutex_);

    // call all clients
    if (streamId == 0) {
        for (auto &it : interruptClients_) {
            (it.second)->OnInterrupt(interruptEvent);
        }
        return;
    }

    if (interruptClients_.find(streamId) != interruptClients_.end()) {
#ifdef FEATURE_APPGALLERY
        if (ShouldCallbackToClient(interruptClients_[streamId]->GetCallingUid(), streamId, interruptEvent)) {
            interruptClients_[streamId]->OnInterrupt(interruptEvent);
        }
#else
        interruptClients_[streamId]->OnInterrupt(interruptEvent);
#endif
    }
}

void AudioInterruptService::DispatchInterruptEventForAudioSession(
    InterruptEventInternal &interruptEvent, const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sessionService_ == nullptr) {
        AUDIO_ERR_LOG("[sessionService_ is null");
        return;
    }

    std::vector<AudioInterrupt> sessionStreams = sessionService_->GetStreams(audioInterrupt.pid);
    for (auto it : sessionStreams) {
        if (interruptClients_.find(it.streamId) != interruptClients_.end() &&
            interruptClients_[it.streamId] != nullptr) {
            interruptClients_[it.streamId]->OnInterrupt(interruptEvent);
        }
    }
}

bool AudioInterruptService::IsGameAvoidCallbackCase(const AudioInterrupt &audioInterrupt)
{
    return GetClientTypeByStreamId(audioInterrupt.streamId) == CLIENT_TYPE_GAME &&
        audioInterrupt.callbackType != INTERRUPT_EVENT_CALLBACK_SEPERATED;
}

ClientType AudioInterruptService::GetClientTypeByStreamId(int32_t streamId)
{
#ifdef FEATURE_APPGALLERY
    uint32_t uid = 0;
    if (interruptClients_.find(streamId) != interruptClients_.end()) {
        uid = interruptClients_[streamId]->GetCallingUid();
    }
    if (uid == 0) {
        AUDIO_ERR_LOG("Cannot find streamId %{public}d", streamId);
        return CLIENT_TYPE_OTHERS;
    }
    return ClientTypeManager::GetInstance()->GetClientTypeByUid(uid);
#else
    return CLIENT_TYPE_OTHERS;
#endif
}

bool AudioInterruptService::ShouldCallbackToClient(uint32_t uid, int32_t streamId,
    InterruptEventInternal &interruptEvent)
{
    AUDIO_INFO_LOG("uid: %{public}u, streamId: %{public}d, hintType: %{public}d", uid, streamId,
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
            policyServer_->UpdateDefaultOutputDeviceWhenStarting(streamId);
            break;
        case INTERRUPT_HINT_PAUSE:
        case INTERRUPT_HINT_STOP:
            policyServer_->UpdateDefaultOutputDeviceWhenStopping(streamId);
            break;
        default:
            return false;
    }
    const sptr<IStandardAudioService> gsp = GetAudioServerProxy();
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    CHECK_AND_RETURN_RET_LOG(gsp != nullptr, true, "error for g_adProxy null");
    AUDIO_INFO_LOG("mute flag is: %{public}d", muteFlag);
    gsp->SetNonInterruptMute(streamId, muteFlag);
    IPCSkeleton::SetCallingIdentity(identity);
    return false;
}

// called when the client remote object dies
void AudioInterruptService::RemoveClient(const int32_t zoneId, uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(mutex_);

    AUDIO_INFO_LOG("Remove session: %{public}u in audioFocusInfoList", streamId);

    auto itActiveZone = zonesMap_.find(ZONEID_DEFAULT);

    auto isSessionPresent = [&streamId] (const std::pair<AudioInterrupt, AudioFocuState> &audioFocusInfo) {
        return audioFocusInfo.first.streamId == streamId;
    };
    auto iterActive = std::find_if((itActiveZone->second->audioFocusInfoList).begin(),
        (itActiveZone->second->audioFocusInfoList).end(), isSessionPresent);
    if (iterActive != (itActiveZone->second->audioFocusInfoList).end()) {
        AudioInterrupt interruptToRemove = iterActive->first;
        DeactivateAudioInterruptInternal(ZONEID_DEFAULT, interruptToRemove);
    }

    interruptClients_.erase(streamId);

    // callback in zones map also need to be removed
    auto it = zonesMap_.find(zoneId);
    if (it != zonesMap_.end() && it->second != nullptr &&
        it->second->interruptCbsMap.find(streamId) != it->second->interruptCbsMap.end()) {
        it->second->interruptCbsMap.erase(it->second->interruptCbsMap.find(streamId));
        zonesMap_[zoneId] = it->second;
    }
}

void AudioInterruptService::WriteFocusMigrateEvent(const int32_t &toZoneId)
{
    auto uid = IPCSkeleton::GetCallingUid();
    std::string deviceDesc = (toZoneId == 1) ? REMOTE_NETWORK_ID : LOCAL_NETWORK_ID;
    AUTO_CTRACE("SYSEVENT BEHAVIOR EVENT AUDIO_FOCUS_MIGRATE, CLIENT_UID: %d, MIGRATE_DIRECTION: %d, DEVICE_DESC: %s",
        static_cast<int32_t>(uid), toZoneId, deviceDesc.c_str());
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::AUDIO_FOCUS_MIGRATE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("CLIENT_UID", static_cast<int32_t>(uid));
    bean->Add("MIGRATE_DIRECTION", toZoneId);
    bean->Add("DEVICE_DESC", deviceDesc);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

uint8_t AudioInterruptService::GetAppState(int32_t appPid)
{
    OHOS::AppExecFwk::AppMgrClient appManager;
    OHOS::AppExecFwk::RunningProcessInfo infos;
    uint8_t state = 0;
    appManager.GetRunningProcessInfoByPid(appPid, infos);
    state = static_cast<uint8_t>(infos.state_);
    if (state == 0) {
        AUDIO_WARNING_LOG("GetAppState failed, appPid=%{public}d", appPid);
    }
    return state;
}

void AudioInterruptService::WriteStartDfxMsg(InterruptDfxBuilder &dfxBuilder, const AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN_LOG(audioInterrupt.uid != BOOTUP_MUSIC_UID, "The caller is BootAnimation. Don't write dfx msg.");
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "dfxCollector is null");
    auto& [infoIdx, effectIdx] = dfxCollector_->GetDfxIndexes(audioInterrupt.streamId);
    if (!dfxBuilder.GetResult().interruptEffectVec.empty()) {
        ++effectIdx;
    }

    if (audioInterrupt.state == State::PREPARED) {
        auto &manager = DfxMsgManager::GetInstance();
        DfxAppState appStartState = static_cast<AppExecFwk::AppProcessState>(GetAppState(audioInterrupt.pid)) ==
            AppExecFwk::AppProcessState::APP_STATE_BACKGROUND ?
            DFX_APP_STATE_BACKGROUND : DFX_APP_STATE_FOREGROUND;
        manager.UpdateAppState(audioInterrupt.uid, appStartState, true);
    }

    InterruptStage stage = dfxCollector_->IsExist(audioInterrupt.streamId) ?
        INTERRUPT_STAGE_RESTART : INTERRUPT_STAGE_START;

    AudioSessionStrategy strategy = audioInterrupt.sessionStrategy;
    auto audioSession = sessionService_->GetAudioSessionByPid(audioInterrupt.pid);
    InterruptRole interruptType = InterruptRole::INTERRUPT_ROLE_DEFAULT;
    if (audioSession != nullptr) {
        strategy = audioSession->GetSessionStrategy();
        interruptType = INTERRUPT_ROLE_AUDIO_SESSION;
    }

    dfxBuilder.WriteActionMsg(++infoIdx, effectIdx, stage).WriteInfoMsg(audioInterrupt, strategy, interruptType);
    dfxCollector_->AddDfxMsg(audioInterrupt.streamId, dfxBuilder.GetResult());
}

void AudioInterruptService::WriteSessionTimeoutDfxEvent(const int32_t pid)
{
    CHECK_AND_RETURN_LOG(dfxCollector_ != nullptr, "dfxCollector is null");
    auto itZone = zonesMap_.find(ZONEID_DEFAULT);
    CHECK_AND_RETURN_LOG(itZone != zonesMap_.end(), "can not find zoneid");
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList{};
    if (itZone != zonesMap_.end() && itZone->second != nullptr) {
        audioFocusInfoList = itZone->second->audioFocusInfoList;
    }

    auto iter = std::find_if(audioFocusInfoList.begin(), audioFocusInfoList.end(), [pid](const auto &item) {
        return pid == item.first.pid;
    });
    if (iter == audioFocusInfoList.end()) {
        AUDIO_WARNING_LOG("audioFocusInfoList have no match object");
        return;
    }

    auto audioInterrupt = iter->first;
    InterruptDfxBuilder dfxBuilder;
    auto& [infoIdx, effectIdx] = dfxCollector_->GetDfxIndexes(audioInterrupt.streamId);
    dfxBuilder.WriteActionMsg(infoIdx, effectIdx, INTERRUPT_STAGE_TIMEOUT);
    dfxCollector_->AddDfxMsg(audioInterrupt.streamId, dfxBuilder.GetResult());
}

void AudioInterruptService::WriteStopDfxMsg(const AudioInterrupt &audioInterrupt)
{
    CHECK_AND_RETURN_LOG((dfxCollector_ != nullptr && policyServer_ != nullptr), "WriteStopDfxMsg nullptr");
    InterruptDfxBuilder dfxBuilder;
    auto& [infoIdx, effectIdx] = dfxCollector_->GetDfxIndexes(audioInterrupt.streamId);
    dfxBuilder.WriteActionMsg(infoIdx, effectIdx, INTERRUPT_STAGE_STOP);
    dfxCollector_->AddDfxMsg(audioInterrupt.streamId, dfxBuilder.GetResult());

    if (audioInterrupt.state == State::RELEASED) {
        dfxCollector_->FlushDfxMsg(audioInterrupt.streamId, audioInterrupt.uid);
    }
}

void AudioInterruptService::RegisterDefaultVolumeTypeListener()
{
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFuncMono = [this, &settingProvider](const std::string &key) {
        int32_t currentValueType = STREAM_MUSIC;
        ErrCode ret = settingProvider.GetIntValue(DEFAULT_VOLUME_KEY, currentValueType, "system");
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "DEFAULT_VOLUME_KEY get mono value failed");
        if (currentValueType == STREAM_RING) {
            defaultVolumeType_ = STREAM_RING;
        } else {
            defaultVolumeType_ = STREAM_MUSIC;
        }
        AUDIO_INFO_LOG("Get defaultVolumeType: %{public}d", defaultVolumeType_);
    };
    sptr observer = settingProvider.CreateObserver(DEFAULT_VOLUME_KEY, updateFuncMono);
    ErrCode ret = settingProvider.RegisterObserver(observer, "system");
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterDefaultVolumeTypeListener mono failed");
    }
    updateFuncMono(DEFAULT_VOLUME_KEY);
    AUDIO_INFO_LOG("DefaultVolumeTypeListener mono successfully, defaultVolumeType:%{public}d", defaultVolumeType_);
}

// LCOV_EXCL_STOP
} // namespace AudioStandard
} // namespace OHOS
