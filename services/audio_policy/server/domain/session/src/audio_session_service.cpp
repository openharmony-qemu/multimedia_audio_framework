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
#include "ipc_skeleton.h"
#include "audio_session_device_info.h"

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
    {STREAM_NAVIGATION, AudioSessionType::MEDIA},
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

int32_t AudioSessionService::ActivateAudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy)
{
    AUDIO_INFO_LOG("ActivateAudioSession: callerPid %{public}d, concurrencyMode %{public}d",
        callerPid, static_cast<int32_t>(strategy.concurrencyMode));
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);

    auto audioSession = CreateAudioSession(callerPid, strategy);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("Create audio session fail, pid: %{public}d!", callerPid);
        return ERROR;
    }

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
        if (audioSession->IsSceneParameterSet() && audioSession->IsBackGroundApp()) {
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
    int32_t callerPid, AudioSessionStrategy strategy)
{
    std::shared_ptr<AudioSession> audioSession = nullptr;
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        audioSession = session->second;
        AUDIO_INFO_LOG("The audio seesion of pid %{public}d has already been created", callerPid);
    } else {
        audioSession = std::make_shared<AudioSession>(callerPid, strategy, *this);
        CHECK_AND_RETURN_RET_LOG(audioSession != nullptr, audioSession, "Create AudioSession fail");
        sessionMap_[callerPid] = audioSession;
    }

    return audioSession;
}

int32_t AudioSessionService::SetAudioSessionScene(int32_t callerPid, AudioSessionScene scene)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto audioSession = CreateAudioSession(callerPid);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("Create audio session fail, pid: %{public}d!", callerPid);
        return ERROR;
    }

    return audioSession->SetAudioSessionScene(scene);
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
           hintType == INTERRUPT_HINT_UNDUCK;
}

bool AudioSessionService::ShouldAudioStreamProcessHintType(InterruptHint hintType)
{
    return hintType == INTERRUPT_HINT_PAUSE ||
           hintType == INTERRUPT_HINT_STOP ||
           hintType == INTERRUPT_HINT_DUCK ||
           hintType == INTERRUPT_HINT_UNDUCK;
}

std::vector<AudioInterrupt> AudioSessionService::GetStreams(int32_t callerPid)
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
    } else {
        AUDIO_ERR_LOG("This failure should not have occurred, possibly due to calling the function incorrectly!");
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

    std::vector<AudioInterrupt> streamsInSession = session->second->GetStreams();
    for (const auto &stream : streamsInSession) {
        if (streamIds.find(stream.streamId) != streamIds.end()) {
            return true;
        }
    }

    return false;
}

void AudioSessionService::GenerateFakeStreamId(int32_t callerPid)
{
    uint32_t fakeStreamId = AudioStreamIdAllocator::GetAudioStreamIdAllocator().GenerateStreamId();

    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end()) {
        session->second->SaveFakeStreamId(fakeStreamId);
    }
}

void AudioSessionService::AddStreamInfo(const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    // No need to handle fake focus.
    if (audioInterrupt.isAudioSessionInterrupt) {
        return;
    }

    if (IsAudioSessionFocusModeInner(audioInterrupt.pid) && ShouldExcludeStreamTypeInner(audioInterrupt)) {
        return;
    }

    auto session = sessionMap_.find(audioInterrupt.pid);
    if (session != sessionMap_.end()) {
        session->second->AddStreamInfo(audioInterrupt);
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
        std::shared_ptr<AudioSession> audioSession = iterAudioSession->second;
        if (audioSession == nullptr) {
            AppendFormat(dumpString, "    - pid: %d, AudioSession is null.\n", pid);
            continue;
        }
        audioSession->Dump(dumpString);
    }
    dumpString += "\n";
}

int32_t AudioSessionService::SetSessionDefaultOutputDevice(const int32_t callerPid, const DeviceType &deviceType)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    AUDIO_INFO_LOG("SetSessionDefaultOutputDevice: callerPid %{public}d, deviceType %{public}d",
        callerPid, static_cast<int32_t>(deviceType));

    auto audioSession = CreateAudioSession(callerPid);
    if (audioSession == nullptr) {
        AUDIO_ERR_LOG("Create audio session fail, pid: %{public}d!", callerPid);
        return ERROR;
    }

    return audioSession->SetSessionDefaultOutputDevice(deviceType);
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
        if ((pair.second != nullptr) && (pair.second->IsStreamContainedInCurrentSession(streamId))) {
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
    if ((session == sessionMap_.end()) || (session->second == nullptr)) {
        return ERROR;
    }

    if (deviceChangedEvent.devices.size() == 0) {
        AUDIO_ERR_LOG("Device info is empty, pid: %{public}d!", callerPid);
        return ERROR;
    }

    CHECK_AND_RETURN_RET((session->second->IsSessionOutputDeviceChanged(deviceChangedEvent.devices[0]) ||
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
               audioInterrupt.audioFocusType.sourceType != SOURCE_TYPE_VOICE_COMMUNICATION;
    }

    return false;
}

} // namespace AudioStandard
} // namespace OHOS
