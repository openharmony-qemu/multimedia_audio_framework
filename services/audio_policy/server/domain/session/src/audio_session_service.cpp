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

std::shared_ptr<AudioSessionService> AudioSessionService::GetAudioSessionService()
{
    static std::shared_ptr<AudioSessionService> audioSessionService = std::make_shared<AudioSessionService>();
    return audioSessionService;
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

    if (sessionMap_.count(callerPid) != 0) {
        // The audio session of the callerPid is already created. The strategy will be updated.
        AUDIO_INFO_LOG("The audio seesion of pid %{public}d has already been created! Update strategy.", callerPid);
    } else {
        sessionMap_[callerPid] = std::make_shared<AudioSession>(callerPid, strategy, shared_from_this());
    }

    if (sessionMap_[callerPid] == nullptr) {
        AUDIO_ERR_LOG("Create audio seesion fail, pid: %{public}d!", callerPid);
        return ERROR;
    }

    if (sessionMap_[callerPid]->IsSceneParameterSet()) {
        GenerateFakeStreamId(callerPid);
    }

    sessionMap_[callerPid]->Activate(strategy);

    StopMonitor(callerPid);
    if (sessionMap_[callerPid]->IsAudioSessionEmpty()) {
        // session v1 60s
        if (!sessionMap_[callerPid]->IsSceneParameterSet()) {
            StartMonitor(callerPid, AUDIO_SESSION_TIME_OUT_DURATION_S);
        }

        // session v2 background 10s
        if (sessionMap_[callerPid]->IsSceneParameterSet() && sessionMap_[callerPid]->IsBackGroundApp()) {
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
    if (sessionMap_.count(callerPid) == 0) {
        // The audio session of the callerPid is not existed or has been released.
        AUDIO_ERR_LOG("The audio seesion of pid %{public}d is not found!", callerPid);
        return ERR_ILLEGAL_STATE;
    }

    if (sessionMap_[callerPid] == nullptr) {
        AUDIO_ERR_LOG("The audio seesion obj of pid %{public}d is nullptr!", callerPid);
        return ERR_ILLEGAL_STATE;
    }

    sessionMap_[callerPid]->Deactivate();
    sessionMap_.erase(callerPid);

    if (!isSessionTimeout) {
        StopMonitor(callerPid);
    }

    return SUCCESS;
}

bool AudioSessionService::IsAudioSessionActivated(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if (sessionMap_.count(callerPid) == 0 || sessionMap_[callerPid] == nullptr) {
        // The audio session of the callerPid is not existed or has been released.
        AUDIO_WARNING_LOG("The audio seesion of pid %{public}d is not found!", callerPid);
        return false;
    }
    return sessionMap_[callerPid]->IsActivated();
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

std::shared_ptr<AudioSession> AudioSessionService::GetAudioSessionByPid(const int32_t callerPid)
{
    AUDIO_INFO_LOG("GetAudioSessionByPid: callerPid %{public}d", callerPid);
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if (sessionMap_.count(callerPid) == 0) {
        AUDIO_ERR_LOG("The audio seesion of pid %{public}d is not found!", callerPid);
        return nullptr;
    }
    return sessionMap_[callerPid];
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

std::shared_ptr<AudioSessionStateMonitor> AudioSessionService::GetSelfSharedPtr()
{
    return shared_from_this();
}

int32_t AudioSessionService::SetAudioSessionScene(int32_t callerPid, AudioSessionScene scene)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if (sessionMap_.count(callerPid) != 0 && sessionMap_[callerPid] != nullptr) {
        // The audio session of the callerPid is already created. The strategy will be updated.
        AUDIO_INFO_LOG("The audio seesion of pid %{public}d has already been created! Update scene.", callerPid);
    } else {
        AudioSessionStrategy strategy;
        strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
        sessionMap_[callerPid] = std::make_shared<AudioSession>(callerPid, strategy, shared_from_this());
        CHECK_AND_RETURN_RET_LOG(sessionMap_[callerPid] != nullptr, ERROR, "Create AudioSession fail");
    }

    return sessionMap_[callerPid]->SetAudioSessionScene(scene);
}

StreamUsage AudioSessionService::GetAudioSessionStreamUsage(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    if (session != sessionMap_.end() && sessionMap_[callerPid] != nullptr) {
        return sessionMap_[callerPid]->GetSessionStreamUsage();
    }

    return STREAM_USAGE_INVALID;
}

bool AudioSessionService::IsAudioSessionFocusMode(int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(callerPid);
    return session != sessionMap_.end() && sessionMap_[callerPid] != nullptr &&
           sessionMap_[callerPid]->IsSceneParameterSet() && sessionMap_[callerPid]->IsActivated();
}

// For audio session v2
bool AudioSessionService::ShouldExcludeStreamType(const AudioInterrupt &audioInterrupt)
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
    if (!IsAudioSessionFocusMode(audioInterrupt.pid)) {
        return false;
    }

    if (ShouldExcludeStreamType(audioInterrupt)) {
        return false;
    }

    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(audioInterrupt.pid);
    if (session != sessionMap_.end() && sessionMap_[audioInterrupt.pid] != nullptr) {
        sessionMap_[audioInterrupt.pid]->AddStreamInfo(audioInterrupt);
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
    if (session != sessionMap_.end() && sessionMap_[callerPid] != nullptr) {
        fakeAudioInterrupt.streamId = sessionMap_[callerPid]->GetFakeStreamId();
        fakeAudioInterrupt.audioFocusType.streamType = sessionMap_[callerPid]->GetFakeStreamType();
        fakeAudioInterrupt.streamUsage = sessionMap_[callerPid]->GetSessionStreamUsage();
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
    if (session != sessionMap_.end() && sessionMap_[callerPid] != nullptr) {
        sessionMap_[callerPid]->SaveFakeStreamId(fakeStreamId);
    }
}

void AudioSessionService::RemoveStreamInfo(const AudioInterrupt &audioInterrupt)
{
    // No need to handle fake focus.
    if (audioInterrupt.isAudioSessionInterrupt) {
        return;
    }

    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    auto session = sessionMap_.find(audioInterrupt.pid);
    if (session == sessionMap_.end()) {
        return;
    }
    return session->second->RemoveStreamInfo(audioInterrupt.streamId);
}

void AudioSessionService::ClearStreamInfo(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if ((sessionMap_.count(callerPid) == 0) || (sessionMap_[callerPid] == nullptr)) {
        return;
    }

    sessionMap_[callerPid]->ClearStreamInfo();
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
    if ((sessionMap_.count(callerPid) > 0) && (sessionMap_[callerPid] != nullptr)) {
        AUDIO_INFO_LOG("SetSessionDefaultOutputDevice: callerPid %{public}d, deviceType %{public}d",
            callerPid, static_cast<int32_t>(deviceType));
    } else {
        AudioSessionStrategy strategy;
        strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
        sessionMap_[callerPid] = std::make_shared<AudioSession>(callerPid, strategy, shared_from_this());
        CHECK_AND_RETURN_RET_LOG(sessionMap_[callerPid] != nullptr, ERROR, "Create AudioSession fail");
    }

    return sessionMap_[callerPid]->SetSessionDefaultOutputDevice(deviceType);
}

DeviceType AudioSessionService::GetSessionDefaultOutputDevice(const int32_t callerPid)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if ((sessionMap_.count(callerPid) > 0) && (sessionMap_[callerPid] != nullptr)) {
        DeviceType deviceType;
        sessionMap_[callerPid]->GetSessionDefaultOutputDevice(deviceType);
        return deviceType;
    }

    return DEVICE_TYPE_INVALID;
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
    if ((sessionMap_.count(callerPid) != 0) && (sessionMap_[callerPid] != nullptr)) {
        return sessionMap_[callerPid]->GetAndClearNeedToFetchFlag();
    }

    return false;
}

void AudioSessionService::NotifyAppStateChange(const int32_t pid, bool isBackState)
{
    std::lock_guard<std::mutex> lock(sessionServiceMutex_);
    if (sessionMap_.count(pid) == 0) {
        return;
    }

    if (sessionMap_[pid] == nullptr) {
        AUDIO_WARNING_LOG("audio session is nullptr, pid: %{public}d!", pid);
        return;
    }

    // v2 foreground
    if (!isBackState && sessionMap_[pid]->IsSceneParameterSet()) {
        StopMonitor(pid);
        return;
    }

    // v2 background
    if (sessionMap_[pid]->IsActivated() &&
        sessionMap_[pid]->IsSceneParameterSet() &&
        sessionMap_[pid]->IsAudioSessionEmpty()) {
        StartMonitor(pid, AUDIO_SESSION_SCENE_TIME_OUT_DURATION_S);
    }
}

} // namespace AudioStandard
} // namespace OHOS
