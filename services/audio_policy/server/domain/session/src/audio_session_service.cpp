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
#undef LOG_TAG
#define LOG_TAG "AudioSessionService"

#include "audio_session_service.h"

#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "audio_stream_id_allocator.h"
#include "audio_stream_collector.h"
#include "audio_effect_service.h"
#include "ipc_skeleton.h"
#include "audio_session_device_info.h"
#include "stream_dfx_manager.h"
#include <unistd.h>

namespace OHOS {
namespace AudioStandard {

static constexpr time_t AUDIO_SESSION_TIME_OUT_DURATION_S = 60; // Audio session timeout duration : 60 seconds
static constexpr time_t AUDIO_SESSION_SCENE_TIME_OUT_DURATION_S = 10; // Audio sessionV2 timeout duration : 10 seconds

static const std::unordered_map<AudioStreamType, AudioSessionType> SESSION_TYPE_MAP = {
    {STREAM_ALARM, AudioSessionType::SONIFICATION},
    {STREAM_RING, AudioSessionType::SONIFICATION},
    {STREAM_MUSIC, AudioSessionType::MEDIA},
    {STREAM_MOVIE, AudioSessionType::MEDIA},
    {STREAM_GAME, AudioSessionType::MEDIA},
    {STREAM_SPEECH, AudioSessionType::MEDIA},
    {STREAM_NAVIGATION, AudioSessionType::NAVIGATION},
    {STREAM_VOICE_MESSAGE, AudioSessionType::MEDIA},
    {STREAM_VOICE_CALL, AudioSessionType::CALL},
    {STREAM_VOICE_CALL_ASSISTANT, AudioSessionType::CALL},
    {STREAM_VOICE_COMMUNICATION, AudioSessionType::VOIP},
    {STREAM_SYSTEM, AudioSessionType::SYSTEM},
    {STREAM_SYSTEM_ENFORCED, AudioSessionType::SYSTEM},
    {STREAM_ACCESSIBILITY, AudioSessionType::SYSTEM},
    {STREAM_ULTRASONIC, AudioSessionType::SYSTEM},
    {STREAM_NOTIFICATION, AudioSessionType::NOTIFICATION},
    {STREAM_DTMF, AudioSessionType::DTMF},
    {STREAM_VOICE_ASSISTANT, AudioSessionType::VOICE_ASSISTANT},
#ifdef MULTI_ALARM_LEVEL
    {STREAM_ANNOUNCEMENT, AudioSessionType::SONIFICATION},
    {STREAM_EMERGENCY, AudioSessionType::SONIFICATION},
#endif
};

AudioSessionService::AudioSessionService()
{
}

AudioSessionService::~AudioSessionService()
{
}

bool AudioSessionService::IsSameTypeForAudioSession(const AudioStreamType incomingType,
    const AudioStreamType existedType)
{
    if (SESSION_TYPE_MAP.count(incomingType) == 0 || SESSION_TYPE_MAP.count(existedType) == 0) {
        AUDIO_WARNING_LOG("The stream type (new:%{public}d or old:%{public}d) is invalid!", incomingType, existedType);
        return false;
    }
    return SESSION_TYPE_MAP.at(incomingType) == SESSION_TYPE_MAP.at(existedType);
}

int32_t AudioSessionService::ActivateAudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy,
    const bool stateChangeCallbackFlag, int32_t callerUid)
{
    AUDIO_INFO_LOG("ActivateAudioSession: callerPid %{public}d, concurrencyMode %{public}d",
        callerPid, static_cast<int32_t>(strategy.concurrencyMode));
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);

    auto audioSession = CreateAudioSession(callerPid, strategy, callerUid);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("Create audio session fail, pid: %{public}d!", callerPid);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_START_OPERATION_FAILED, "Create audio session fail", false);
        return ERROR;
    }

    audioSession->SetRegisterStateChangeCallbackFlag(stateChangeCallbackFlag);

    if (audioSession->IsSceneParameterSet()) {
        GenerateFakeStreamId(callerPid);
    }

    audioSession->Activate(strategy);

    StopMonitor(callerPid);
    if (audioSession->IsAudioSessionEmpty()) {
        // session v1 60s
        if (!audioSession->IsSceneParameterSet()) {
            StartMonitor(callerPid, AUDIO_SESSION_TIME_OUT_DURATION_S);
        }

        // session v2 background 10s
        if (audioSession->IsSceneParameterSet() &&
            audioSession->GetFakeFocusInterruptEvent().hintType != INTERRUPT_HINT_PAUSE &&
            audioSession->IsBackGroundApp()) {
            StartMonitor(callerPid, AUDIO_SESSION_SCENE_TIME_OUT_DURATION_S);
        }
    }

    return SUCCESS;
}

int32_t AudioSessionService::DeactivateAudioSession(const int32_t callerPid)
{
    AUDIO_INFO_LOG("DeactivateAudioSession: callerPid %{public}d", callerPid);
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    return DeactivateAudioSessionInternal(callerPid);
}

int32_t AudioSessionService::DeactivateAudioSessionInternal(const int32_t callerPid, bool isSessionTimeout)
{
    AUDIO_INFO_LOG("DeactivateAudioSessionInternal: callerPid %{public}d", callerPid);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        // The audio session of the callerPid is not existed or has been released.
        AUDIO_ERR_LOG("The audio seesion of pid %{public}d is not found!", callerPid);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_START_ILLEGAL_STATE, "The audio session is not found", false);
        return ERR_ILLEGAL_STATE;
    }

    session->second->Deactivate();
    sessionMap_.erase(callerPid);

    if (!isSessionTimeout) {
        StopMonitor(callerPid);
    }

    return SUCCESS;
}

bool AudioSessionService::IsAudioSessionActivated(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        // The audio session of the callerPid is not existed or has been released.
        AUDIO_WARNING_LOG("The audio seesion of pid %{public}d is not found!", callerPid);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_QUERY_ILLEGAL_STATE, "The audio session is not found", false);
        return false;
    }

    return session->second->IsActivated();
}

int32_t AudioSessionService::SetSessionTimeOutCallback(
    const std::shared_ptr<SessionTimeOutCallback> &timeOutCallback)
{
    AUDIO_INFO_LOG("SetSessionTimeOutCallback is nullptr!");
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if (timeOutCallback == nullptr) {
        AUDIO_ERR_LOG("timeOutCallback is nullptr!");
        return AUDIO_INVALID_PARAM;
    }
    timeOutCallback_ = timeOutCallback;
    return SUCCESS;
}

// Audio session monitor callback
void AudioSessionService::OnAudioSessionTimeOut(int32_t callerPid)
{
    AUDIO_INFO_LOG("OnAudioSessionTimeOut: callerPid %{public}d", callerPid);
    std::unique_lock<std::mutex> lock(sessionServiceMutex_);
    DeactivateAudioSessionInternal(callerPid, true);
    lock.unlock();

    auto cb = timeOutCallback_.lock();
    if (cb == nullptr) {
        AUDIO_ERR_LOG("timeOutCallback_ is nullptr!");
        return;
    }
    cb->OnSessionTimeout(callerPid);
}

std::shared_ptr<AudioSession> AudioSessionService::CreateAudioSession(
    int32_t callerPid, AudioSessionStrategy strategy, int32_t callerUid)
{
    std::shared_ptr<AudioSession> audioSession = nullptr;
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        audioSession = session->second;
        AUDIO_INFO_LOG("The audio seesion of pid %{public}d has already been created", callerPid);
    } else {
        audioSession = std::make_shared<AudioSession>(callerPid, strategy, *this, callerUid);
        CHECK_AND_RETURN_RET_LOG(audioSession != nullptr, audioSession, "Create AudioSession fail");
        sessionMap_[callerPid] = audioSession;
    }

    return audioSession;
}

int32_t AudioSessionService::SetAudioSessionScene(int32_t callerPid, AudioSessionScene scene,
    int32_t callerUid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    AudioSessionStrategy strategy = { AudioConcurrencyMode::INVALID };
    auto audioSession = CreateAudioSession(callerPid, strategy, callerUid);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("Create audio session fail, pid: %{public}d!", callerPid);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_CONFIG_OPERATION_FAILED, "Create audio session fail", false);
        return ERROR;
    }

    return audioSession->SetAudioSessionScene(scene);
}

void AudioSessionService::SetRegisterStateChangeCallbackFlag(int32_t callerPid, const bool stateChangeCallbackFlag)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        session->second->SetRegisterStateChangeCallbackFlag(stateChangeCallbackFlag, stateChangeCallbackFlag);
    }
}

StreamUsage AudioSessionService::GetAudioSessionStreamUsage(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        return session->second->GetSessionStreamUsage();
    }

    return STREAM_USAGE_INVALID;
}

void AudioSessionService::GetStateChangeCallbackFlag(int32_t callerPid,
    bool &registerCallbackFlag, bool &activatedRegisterCallbackFlag)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        session->second->GetStateChangeCallbackFlag(registerCallbackFlag, activatedRegisterCallbackFlag);
    }
}

StreamUsage AudioSessionService::GetAudioSessionStreamUsageForDevice(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        return session->second->GetAudioSessionStreamUsageForDevice();
    }

    return STREAM_USAGE_INVALID;
}

bool AudioSessionService::IsAudioSessionFocusMode(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    return IsAudioSessionFocusModeInner(callerPid);
}

bool AudioSessionService::IsAudioSessionFocusModeInner(int32_t callerPid)
{
    auto session = sessionMap_.find(callerPid);
    return session != sessionMap_.end() &&
           session->second->IsSceneParameterSet() &&
           session->second->IsActivated();
}

bool AudioSessionService::ShouldExcludeStreamType(const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if (!IsAudioSessionFocusModeInner(audioInterrupt.pid)) {
        return false;
    }

    return ShouldExcludeStreamTypeInner(audioInterrupt);
}

// For audio session v2
bool AudioSessionService::ShouldExcludeStreamTypeInner(const AudioInterrupt &audioInterrupt)
{
    bool isExcludedStream = audioInterrupt.audioFocusType.streamType == STREAM_NOTIFICATION ||
                            audioInterrupt.audioFocusType.streamType == STREAM_DTMF ||
                            audioInterrupt.audioFocusType.streamType == STREAM_ALARM ||
                            audioInterrupt.audioFocusType.streamType == STREAM_VOICE_CALL_ASSISTANT ||
                            audioInterrupt.audioFocusType.streamType == STREAM_ULTRASONIC ||
                            audioInterrupt.audioFocusType.streamType == STREAM_ACCESSIBILITY;
    if (isExcludedStream) {
        return true;
    }

    bool isExcludedStreamType = audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID;
    if (isExcludedStreamType) {
        return true;
    }

    return false;
}


bool AudioSessionService::ShouldBypassFocusForStream(const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if (!IsAudioSessionFocusModeInner(audioInterrupt.pid)) {
        return false;
    }

    if (ShouldExcludeStreamTypeInner(audioInterrupt)) {
        return false;
    }

    return true;
}

bool AudioSessionService::ShouldAudioSessionProcessHintType(InterruptHint hintType)
{
    return hintType == INTERRUPT_HINT_RESUME ||
           hintType == INTERRUPT_HINT_PAUSE ||
           hintType == INTERRUPT_HINT_STOP ||
           hintType == INTERRUPT_HINT_DUCK ||
           hintType == INTERRUPT_HINT_UNDUCK ||
           hintType == INTERRUPT_HINT_MUTE_SUGGESTION ||
           hintType == INTERRUPT_HINT_UNMUTE_SUGGESTION ||
           hintType == INTERRUPT_HINT_MUTE ||
           hintType == INTERRUPT_HINT_UNMUTE;
}

bool AudioSessionService::ShouldAudioStreamProcessHintType(InterruptHint hintType)
{
    return hintType == INTERRUPT_HINT_PAUSE ||
           hintType == INTERRUPT_HINT_STOP ||
           hintType == INTERRUPT_HINT_DUCK ||
           hintType == INTERRUPT_HINT_UNDUCK ||
           hintType == INTERRUPT_HINT_MUTE_SUGGESTION ||
           hintType == INTERRUPT_HINT_UNMUTE_SUGGESTION ||
           hintType == INTERRUPT_HINT_MUTE ||
           hintType == INTERRUPT_HINT_UNMUTE;
}

std::vector<std::pair<AudioInterrupt, AudioFocuState>> AudioSessionService::GetStreams(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        return {};
    }
    return session->second->GetStreams();
}

AudioInterrupt AudioSessionService::GenerateFakeAudioInterrupt(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    AudioInterrupt fakeAudioInterrupt;
    fakeAudioInterrupt.pid = callerPid;
    fakeAudioInterrupt.uid = IPCSkeleton::GetCallingUid();
    fakeAudioInterrupt.isAudioSessionInterrupt = true;
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        fakeAudioInterrupt.streamId = session->second->GetFakeStreamId();
        fakeAudioInterrupt.audioFocusType.streamType = session->second->GetFakeStreamType();
        fakeAudioInterrupt.streamUsage = session->second->GetSessionStreamUsage();
        auto sessionBehaviors = session->second->GetSessionBehaviors();
        fakeAudioInterrupt.behaviorFlags_.muteWhenInterruptFlag = sessionBehaviors.muteWhenInterruptFlag;
    } else {
        AUDIO_ERR_LOG("This failure should not have occurred, possibly due to calling the function incorrectly!");
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_QUERY_ILLEGAL_STATE, "audio session not found when generate fake interrupt", false);
    }

    return fakeAudioInterrupt;
}

bool AudioSessionService::HasStreamForDeviceType(int32_t callerPid, DeviceType deviceType)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        return false;
    }

    if (session->second == nullptr) {
        return false;
    }

    if (session->second->IsAudioSessionEmpty()) {
        return false;
    }

    std::set<int32_t> streamIds =
        AudioStreamCollector::GetAudioStreamCollector().GetSessionIdsOnRemoteDeviceByDeviceType(deviceType);

    const auto &streamsInSession = session->second->GetStreams();
    for (const auto &[stream, state] : streamsInSession) {
        if (streamIds.find(stream.streamId) != streamIds.end()) {
            return true;
        }
    }

    return false;
}

void AudioSessionService::SetFakeFocusInterruptEvent(int32_t callerPid, const InterruptEventInternal &interruptEvent)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        session->second->SetFakeFocusInterruptEvent(interruptEvent);
    } else {
        AUDIO_ERR_LOG("This failure should not have occurred, possibly due to calling the function incorrectly!");
    }
}

void AudioSessionService::GenerateFakeStreamId(int32_t callerPid)
{
    uint32_t fakeStreamId = AudioStreamIdAllocator::GetAudioStreamIdAllocator().GenerateStreamId();

    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        session->second->SaveFakeStreamId(fakeStreamId);
    }
}

void AudioSessionService::AddStreamInfo(const std::pair<AudioInterrupt, AudioFocuState> &interruptPair)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    // No need to handle fake focus.
    if (interruptPair.first.isAudioSessionInterrupt) {
        return;
    }

    if (IsAudioSessionFocusModeInner(interruptPair.first.pid) && ShouldExcludeStreamTypeInner(interruptPair.first)) {
        return;
    }

    auto session = sessionMap_.find(interruptPair.first.pid);
    if (session != sessionMap_.end()) {
        session->second->AddStreamInfo(interruptPair);
    }
}

void AudioSessionService::RemoveStreamInfo(const int32_t callerPid, const uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        session->second->RemoveStreamInfo(streamId);
    }
}

void AudioSessionService::ClearStreamInfo(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        session->second->ClearStreamInfo();
    }
}

int32_t AudioSessionService::UpdateStreamFocusState(const int32_t callerPid, uint32_t streamId,
    AudioFocuState focuState)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        AUDIO_ERR_LOG("session not found for pid %d", callerPid);
        return ERR_INVALID_PARAM;
    }
    return session->second->UpdateStreamFocusState(streamId, focuState);
}

bool AudioSessionService::IsAllStreamsInState(const int32_t callerPid, AudioFocuState state)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        return false;
    }
    return session->second->IsAllStreamsInState(state);
}

bool AudioSessionService::IsStreamInfoEmpty(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        return true;
    }

    return session->second->IsAudioSessionEmpty();
}

bool AudioSessionService::IsAudioRendererEmpty(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        return true;
    }

    return session->second->IsAudioRendererEmpty();
}

AudioConcurrencyMode AudioSessionService::GetSessionStrategy(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        return AudioConcurrencyMode::INVALID;
    }

    if (!session->second->IsActivated()) {
        return AudioConcurrencyMode::INVALID;
    }

    return (session->second->GetSessionStrategy()).concurrencyMode;
}

void AudioSessionService::AudioSessionInfoDump(std::string &dumpString)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if (sessionMap_.empty()) {
        AppendFormat(dumpString, "    - The AudioSessionMap is empty.\n");
        return;
    }
    for (auto iterAudioSession = sessionMap_.begin(); iterAudioSession != sessionMap_.end(); ++iterAudioSession) {
        dumpString += "\n";
        int32_t pid = iterAudioSession->first;
        iterAudioSession->second->Dump(dumpString);
    }
    dumpString += "\n";
}

int32_t AudioSessionService::SetSessionDefaultOutputDevice(const int32_t callerPid, const DeviceType &deviceType,
    bool forceFetch, int32_t callerUid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    AUDIO_INFO_LOG("SetSessionDefaultOutputDevice: callerPid %{public}d, deviceType %{public}d",
        callerPid, static_cast<int32_t>(deviceType));

    AudioSessionStrategy strategy = { AudioConcurrencyMode::INVALID };
    auto audioSession = CreateAudioSession(callerPid, strategy, callerUid);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("Create audio session fail, pid: %{public}d!", callerPid);
        return ERROR;
    }

    return audioSession->SetSessionDefaultOutputDevice(deviceType, forceFetch);
}

DeviceType AudioSessionService::GetSessionDefaultOutputDevice(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    DeviceType deviceType = DEVICE_TYPE_INVALID;
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        session->second->GetSessionDefaultOutputDevice(deviceType);
    }

    return deviceType;
}

bool AudioSessionService::IsStreamAllowedToSetDevice(const uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    for (const auto& pair : sessionMap_) {
        if (pair.second->IsStreamContainedInCurrentSession(streamId)) {
            // for inactivate session, its default device cannot be used, so set it to DEVICE_TYPE_INVALID
            if (!pair.second->IsActivated()) {
                return true;
            } else {
                DeviceType deviceType;
                pair.second->GetSessionDefaultOutputDevice(deviceType);
                return deviceType == DEVICE_TYPE_INVALID;
            }
            return true;
        }
    }

    return true;
}

bool AudioSessionService::IsSessionNeedToFetchOutputDevice(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        return session->second->GetAndClearNeedToFetchFlag();
    }

    return false;
}

void AudioSessionService::NotifyAppStateChange(const int32_t pid, bool isBackState)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(pid);
    if (session == sessionMap_.end()) {
        return;
    }

    // v2 foreground
    if (!isBackState && session->second->IsSceneParameterSet()) {
        StopMonitor(pid);
        return;
    }

    // v2 background
    if (session->second->IsActivated() &&
        session->second->IsSceneParameterSet() &&
        session->second->GetFakeFocusInterruptEvent().hintType != INTERRUPT_HINT_PAUSE &&
        session->second->IsAudioSessionEmpty()) {
        StartMonitor(pid, AUDIO_SESSION_SCENE_TIME_OUT_DURATION_S);
    }
}

int32_t AudioSessionService::FillCurrentOutputDeviceChangedEvent(
    int32_t callerPid,
    AudioStreamDeviceChangeReason changeReason,
    CurrentOutputDeviceChangedEvent &deviceChangedEvent)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        return ERROR;
    }

    if (deviceChangedEvent.devices.size() == 0) {
        AUDIO_ERR_LOG("Device info is empty, pid: %{public}d!", callerPid);
        return ERROR;
    }

    CHECK_AND_RETURN_RET((session->second->IsSessionOutputDeviceChanged(
        deviceChangedEvent.devices[0], deviceChangedEvent.preDevices) ||
        (changeReason == AudioStreamDeviceChangeReason::AUDIO_SESSION_ACTIVATE)), ERROR,
        "device of session %{public}d is not changed", callerPid);

    deviceChangedEvent.changeReason = changeReason;
    deviceChangedEvent.recommendedAction = session->second->IsRecommendToStopAudio(changeReason,
        deviceChangedEvent.devices[0]) ? OutputDeviceChangeRecommendedAction::RECOMMEND_TO_STOP :
        OutputDeviceChangeRecommendedAction::RECOMMEND_TO_CONTINUE;

    return SUCCESS;
}

bool AudioSessionService::IsSessionInputDeviceChanged(
    int32_t callerPid, const std::shared_ptr<AudioDeviceDescriptor> desc)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        return session->second->IsSessionInputDeviceChanged(desc);
    }

    return false;
}

void AudioSessionService::MarkSystemApp(int32_t pid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(pid);
    if (session != sessionMap_.end()) {
        session->second->MarkSystemApp();
    }
}

bool AudioSessionService::IsSystemApp(int32_t pid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(pid);
    if (session != sessionMap_.end()) {
        return session->second->IsActivated() && session->second->IsSystemApp();
    }

    return false;
}

bool AudioSessionService::IsSystemAppWithMixStrategy(const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(audioInterrupt.pid);
    if (session != sessionMap_.end()) {
        return session->second->IsActivated() && session->second->IsSystemApp() &&
               session->second->GetSessionStrategy().concurrencyMode == AudioConcurrencyMode::MIX_WITH_OTHERS &&
               audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_INVALID &&
               audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_VOICE_CALL &&
               audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_VIRTUAL_CAPTURE &&
               audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_VOICE_COMMUNICATION;
    }

    return false;
}

int32_t AudioSessionService::EnableMuteSuggestionWhenMixWithOthers(int32_t callerPid, bool enable)
{
    AUDIO_INFO_LOG("callerPid %{public}d, isMuteSuggestionEnabled %{public}d",
        callerPid, enable);
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);

    auto session = sessionMap_.find(callerPid);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(session != sessionMap_.end(), ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_CONFIG_ILLEGAL_STATE, "The audio session has not been created", false),
        "The audio session of pid %{public}d has not been created", callerPid);

    std::shared_ptr<AudioSession> audioSession = session->second;
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioSession->IsSceneParameterSet(), ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_CONFIG_ILLEGAL_STATE, "The audio session not set audio session scene", false),
        "The audio session of pid %{public}d not set audio session scene", callerPid);

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!audioSession->IsActivated(), ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_CONFIG_ILLEGAL_STATE, "mute suggestion must be set before activating the audio session", false),
        "The audio session of pid %{public}d mute suggestion must be set before activating the audio session",
        callerPid);

    audioSession->EnableMuteSuggestionWhenMixWithOthers(enable);
    return SUCCESS;
}

bool AudioSessionService::IsMuteSuggestionWhenMixEnabled(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        AUDIO_WARNING_LOG("The audio session of pid %{public}d has not been created", callerPid);
        return false;
    }

    return session->second->IsMuteSuggestionWhenMixEnabled();
}

int32_t AudioSessionService::SetAudioSessionCapturerMuteHint(int32_t callerPid, bool mute)
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> currentCapturerInfos;
    AudioStreamCollector::GetAudioStreamCollector().GetCurrentCapturerChangeInfos(currentCapturerInfos);
    AudioEffectService::GetAudioEffectService().PruneInvalidAppMuteHintState(currentCapturerInfos);

    std::vector<uint32_t> runningCapturerStreamIds;
    for (const auto &capturerInfo : currentCapturerInfos) {
        if (capturerInfo == nullptr) {
            continue;
        }
        if (capturerInfo->capturerState != CAPTURER_RUNNING) {
            continue;
        }
        if (capturerInfo->clientPid != callerPid && capturerInfo->callerPid != callerPid) {
            continue;
        }
        runningCapturerStreamIds.emplace_back(static_cast<uint32_t>(capturerInfo->sessionId));
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!runningCapturerStreamIds.empty(), ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_CONFIG_INVALID_PARAM, "no running capturer stream", false),
        "no running capturer stream for pid %{public}d", callerPid);

    bool hasAppConfigured = false;
    for (auto streamId : runningCapturerStreamIds) {
        bool isApplied = AudioEffectService::GetAudioEffectService()
            .SetAppConfiguredSessionMuteHintIfNoCapturerOverride(streamId, mute);
        if (!isApplied) {
            continue;
        }
        hasAppConfigured = true;
    }
    if (!hasAppConfigured) {
        AUDIO_INFO_LOG("all running capturer streams are overridden by app capturer-level mute hint");
    }
    AudioEffectService::GetAudioEffectService().EvaluateVoipBypassByAppMuteHint(currentCapturerInfos);
    return SUCCESS;
}

int32_t AudioSessionService::SetAudioSessionBehavior(const int32_t callerPid,
    const uint32_t behavior, int32_t callerUid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    // check session is active
    AudioSessionStrategy strategy = { AudioConcurrencyMode::INVALID };
    auto audioSession = CreateAudioSession(callerPid, strategy, callerUid);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("Create audio session fail, pid: %{public}d!", callerPid);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            SESSION_CONFIG_OPERATION_FAILED, "Create audio session fail", false);
        return ERROR;
    }

    return audioSession->SetAudioSessionBehavior(behavior);
}

AudioSessionActiveBehaviors AudioSessionService::GetSessionBehaviors(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session == sessionMap_.end()) {
        AUDIO_WARNING_LOG("The audio session of pid %{public}d has not been created", callerPid);
        return {};
    }

    return session->second->GetSessionBehaviors();
}

void AudioSessionService::GetSessionDebugInfo(AudioSessionDebugInfo &sessionDebugInfo,
    const bool isAudioSessionInterrupt)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(sessionDebugInfo.pid);
    if (session != sessionMap_.end()) {
        session->second->GetSessionDebugInfo(sessionDebugInfo, isAudioSessionInterrupt);
    }
}
} // namespace AudioStandard
} // namespace OHOS
