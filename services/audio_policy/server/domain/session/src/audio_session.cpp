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
#define LOG_TAG "AudioSession"

#include "audio_session.h"
#include "audio_utils.h"
#include "audio_policy_log.h"
#include "audio_errors.h"
#include "audio_session_state_monitor.h"
#include "audio_stream_descriptor.h"
#include "audio_active_device.h"
#include "audio_device_common.h"
#include "app_mgr_client.h"
#include "audio_device_manager.h"
#include "audio_pipe_manager.h"

namespace OHOS {
namespace AudioStandard {

static constexpr time_t AUDIO_SESSION_TIME_OUT_DURATION_S = 60; // Audio session timeout duration : 60 seconds
static constexpr time_t AUDIO_SESSION_SCENE_TIME_OUT_DURATION_S = 10; // Audio sessionV2 timeout duration : 10 seconds

AudioSession::AudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy,
    const std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor)
    : deviceManager_(AudioDeviceManager::GetAudioDeviceManager())
{
    AUDIO_INFO_LOG("AudioSession()");
    callerPid_ = callerPid;
    strategy_ = strategy;
    audioSessionStateMonitor_ = audioSessionStateMonitor;
    state_ = AudioSessionState::SESSION_NEW;
    pipeManager_ = AudioPipeManager::GetPipeManager();
}

AudioSession::~AudioSession()
{
    AUDIO_ERR_LOG("~AudioSession()");
}

bool AudioSession::IsSceneParameterSet()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return audioSessionScene_ != AudioSessionScene::INVALID;
}

int32_t AudioSession::SetAudioSessionScene(AudioSessionScene scene)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);

    if (scene != AudioSessionScene::MEDIA &&
        scene != AudioSessionScene::GAME &&
        scene != AudioSessionScene::VOICE_COMMUNICATION) {
        AUDIO_ERR_LOG("AudioSessionScene = %{public}d out of range.", static_cast<int32_t>(scene));
        return ERR_INVALID_PARAM;
    }

    audioSessionScene_ = scene;
    return SUCCESS;
}

bool AudioSession::IsActivated()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return state_ == AudioSessionState::SESSION_ACTIVE;
}

std::vector<AudioInterrupt> AudioSession::GetStreams()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return streamsInSession_;
}

AudioStreamType AudioSession::GetFakeStreamType()
{
    static const std::unordered_map<AudioSessionScene, AudioStreamType> mapping = {
        {AudioSessionScene::MEDIA, AudioStreamType::STREAM_MUSIC},
        {AudioSessionScene::GAME, AudioStreamType::STREAM_GAME},
        {AudioSessionScene::VOICE_COMMUNICATION, AudioStreamType::STREAM_VOICE_COMMUNICATION}
    };

    std::lock_guard<std::mutex> lock(sessionMutex_);
    auto it = mapping.find(audioSessionScene_);
    if (it != mapping.end()) {
        return it->second;
    }

    return AudioStreamType::STREAM_DEFAULT;
}

void AudioSession::AddStreamInfo(const AudioInterrupt &incomingInterrupt)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    if (incomingInterrupt.isAudioSessionInterrupt) {
        return;
    }

    if (state_ == AudioSessionState::SESSION_ACTIVE &&
        audioSessionScene_ != AudioSessionScene::INVALID &&
        ShouldExcludeStreamType(incomingInterrupt)) {
        return;
    }

    for (const auto &stream : streamsInSession_) {
        if (stream.streamId == incomingInterrupt.streamId) {
            AUDIO_INFO_LOG("stream aready exist.");
            return;
        }
    }

    streamsInSession_.push_back(incomingInterrupt);

    if (IsSessionDefaultDeviceEnabled()) {
        (void)EnableSingleVoipStreamDefaultOutputDevice(incomingInterrupt);
    }

    auto monitor = audioSessionStateMonitor_.lock();
    if (monitor != nullptr) {
        monitor->StopMonitor(callerPid_);
    }
}

bool AudioSession::IsSessionDefaultDeviceEnabled()
{
    return (defaultDeviceType_ != DEVICE_TYPE_INVALID) && (state_ == AudioSessionState::SESSION_ACTIVE);
}

void AudioSession::RemoveStreamInfo(uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    for (auto it = streamsInSession_.begin(); it != streamsInSession_.end(); ++it) {
        if (it->streamId == streamId) {
            if (IsSessionDefaultDeviceEnabled()) {
                UpdateSingleVoipStreamDefaultOutputDevice(*it);
            }
            streamsInSession_.erase(it);
            break;
        }
    }

    auto monitor = audioSessionStateMonitor_.lock();
    if ((streamsInSession_.size() == 0) && monitor != nullptr) {
        // session v1 60s
        if (audioSessionScene_ == AudioSessionScene::INVALID) {
            monitor->StartMonitor(callerPid_, AUDIO_SESSION_TIME_OUT_DURATION_S);
        }

        // session v2 background 10s
        if ((audioSessionScene_ != AudioSessionScene::INVALID) && IsBackGroundApp()) {
            monitor->StartMonitor(callerPid_, AUDIO_SESSION_SCENE_TIME_OUT_DURATION_S);
        }
    }
}

void AudioSession::ClearStreamInfo(void)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    streamsInSession_.clear();
}

uint32_t AudioSession::GetFakeStreamId()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return fakeStreamId_;
}

void AudioSession::SaveFakeStreamId(uint32_t fakeStreamId)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    fakeStreamId_ = fakeStreamId;
}

void AudioSession::Dump(std::string &dumpString)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    AppendFormat(dumpString, "    - pid: %d, AudioSession strategy is: %d.\n",
        callerPid_, static_cast<uint32_t>(strategy_.concurrencyMode));
    AppendFormat(dumpString, "    - pid: %d, AudioSession scene is: %d.\n",
        callerPid_, static_cast<uint32_t>(audioSessionScene_));
    AppendFormat(dumpString, "    - pid: %d, AudioSession fakeStreamId is: %u.\n",
        callerPid_, fakeStreamId_);
    AppendFormat(dumpString, "    - pid: %d, AudioSession defaultDeviceType is: %d.\n",
        callerPid_, static_cast<int32_t>(defaultDeviceType_));
    AppendFormat(dumpString, "    - pid: %d, AudioSession state is: %u.\n",
        callerPid_, static_cast<uint32_t>(state_));
    AppendFormat(dumpString, "    - pid: %d, Streams in session are:\n", callerPid_);
    for (auto &it : streamsInSession_) {
        AppendFormat(dumpString, "        - StreamId is: %u, streamType is: %u\n",
            it.streamId, static_cast<uint32_t>(it.audioFocusType.streamType));
    }
}

int32_t AudioSession::Activate(const AudioSessionStrategy strategy)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    strategy_ = strategy;
    state_ = AudioSessionState::SESSION_ACTIVE;
    AUDIO_INFO_LOG("Audio session state change: pid %{public}d, state %{public}d",
        callerPid_, static_cast<int32_t>(state_));
    needToFetch_ = (EnableDefaultDevice() == NEED_TO_FETCH) ? true : false;
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = GetStreamUsageInner();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredOutputDevices =
        AudioDeviceCommon::GetInstance().GetPreferredOutputDeviceDescInner(rendererInfo, LOCAL_NETWORK_ID);
    if ((preferredOutputDevices.size() == 0) || (preferredOutputDevices[0] == nullptr)) {
        deviceDescriptor_ = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    } else {
        deviceDescriptor_ = AudioDeviceDescriptor(preferredOutputDevices[0]);
    }
    return SUCCESS;
}

void AudioSession::UpdateSingleVoipStreamDefaultOutputDevice(const AudioInterrupt &interrupt)
{
    if (CanCurrentStreamSetDefaultOutputDevice(interrupt)) {
        deviceManager_.UpdateDefaultOutputDeviceWhenStopping(interrupt.streamId);

        CHECK_AND_RETURN_LOG(pipeManager_ != nullptr, "pipeManager_ is nullptr");

        std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor =
            pipeManager_->GetStreamDescById(interrupt.streamId);

        CHECK_AND_RETURN_LOG(audioStreamDescriptor != nullptr, "cannot find stream desc of id %{public}d",
            interrupt.streamId);

        deviceManager_.SetDefaultOutputDevice(DEVICE_TYPE_DEFAULT,
            interrupt.streamId, audioStreamDescriptor->rendererInfo_.streamUsage,
            audioStreamDescriptor->streamStatus_ == STREAM_STATUS_STARTED);
    }
}

void AudioSession::UpdateVoipStreamsDefaultOutputDevice()
{
    for (auto interrupt : streamsInSession_) {
        UpdateSingleVoipStreamDefaultOutputDevice(interrupt);
    }
    deviceManager_.UpdateDefaultOutputDeviceWhenStopping(fakeStreamId_);
}

int32_t AudioSession::Deactivate()
{
    CHECK_AND_RETURN_RET(state_ != AudioSessionState::SESSION_DEACTIVE, SUCCESS);

    std::lock_guard<std::mutex> lock(sessionMutex_);
    state_ = AudioSessionState::SESSION_DEACTIVE;

    if (defaultDeviceType_ != DEVICE_TYPE_INVALID) {
        UpdateVoipStreamsDefaultOutputDevice();
    }

    streamsInSession_.clear();
    needToFetch_ = false;
    AUDIO_INFO_LOG("Audio session state change: pid %{public}d, state %{public}d",
        callerPid_, static_cast<int32_t>(state_));
    return SUCCESS;
}

bool AudioSession::IsOutputDeviceConfigurableByStreamUsage(const StreamUsage &streamUsage)
{
    return (streamUsage == STREAM_USAGE_VOICE_MESSAGE) ||
        (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION) ||
        (streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION) ||
        (streamUsage == STREAM_USAGE_VOICE_MODEM_COMMUNICATION);
}

bool AudioSession::CanCurrentStreamSetDefaultOutputDevice(const AudioInterrupt &interrupt)
{
    return (!(IsOutputDeviceConfigurableByStreamUsage(GetStreamUsageInner())) &&
        IsOutputDeviceConfigurableByStreamUsage(interrupt.streamUsage));
}

int32_t AudioSession::EnableSingleVoipStreamDefaultOutputDevice(const AudioInterrupt &interrupt)
{
    int32_t ret = SUCCESS;

    if (CanCurrentStreamSetDefaultOutputDevice(interrupt) && (pipeManager_ != nullptr)) {
        std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor =
            pipeManager_->GetStreamDescById(interrupt.streamId);
        if (audioStreamDescriptor != nullptr) {
            ret = deviceManager_.SetDefaultOutputDevice(defaultDeviceType_,
                interrupt.streamId, audioStreamDescriptor->rendererInfo_.streamUsage,
                audioStreamDescriptor->streamStatus_ == STREAM_STATUS_STARTED);
        }
    }

    if (ret == NEED_TO_FETCH) {
        needToFetch_ = true;
        ret = SUCCESS;
    }

    return ret;
}

int32_t AudioSession::EnableVoipStreamsDefaultOutputDevice()
{
    int32_t ret = SUCCESS;
    bool success = false;

    for (const auto &interrupt : streamsInSession_) {
        ret = EnableSingleVoipStreamDefaultOutputDevice(interrupt);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("enable default output device for stream %d failed, ret is %d", interrupt.streamId, ret);
        } else {
            success = true;
        }
    }

    return success ? SUCCESS : ret;
}

int32_t AudioSession::EnableDefaultDevice()
{
    if (!IsSessionDefaultDeviceEnabled()) {
        return SUCCESS;
    }

    int32_t ret = deviceManager_.SetDefaultOutputDevice(defaultDeviceType_, fakeStreamId_,
        GetStreamUsageInner(), true);
    AUDIO_INFO_LOG("enable default output device for session %d, ret is %d", fakeStreamId_, ret);
    if (ret == NEED_TO_FETCH) {
        needToFetch_ = true;
        ret = SUCCESS;
    }

    if (EnableVoipStreamsDefaultOutputDevice() == SUCCESS) {
        return SUCCESS;
    }

    return ret;
}

bool AudioSession::GetAndClearNeedToFetchFlag()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    bool ret = needToFetch_;
    needToFetch_ = false;
    return ret;
}

StreamUsage AudioSession::GetStreamUsageInner()
{
    static const std::unordered_map<AudioSessionScene, StreamUsage> mapping = {
        {AudioSessionScene::MEDIA, StreamUsage::STREAM_USAGE_MUSIC},
        {AudioSessionScene::GAME, StreamUsage::STREAM_USAGE_GAME},
        {AudioSessionScene::VOICE_COMMUNICATION, StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION},
    };

    auto it = mapping.find(audioSessionScene_);
    if (it != mapping.end()) {
        return it->second;
    }

    return StreamUsage::STREAM_USAGE_INVALID;
}

AudioSessionStrategy AudioSession::GetSessionStrategy()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    AUDIO_INFO_LOG("GetSessionStrategy: pid %{public}d, strategy_.concurrencyMode %{public}d",
        callerPid_, static_cast<int32_t>(strategy_.concurrencyMode));
    return strategy_;
}

// For audio session v2
bool AudioSession::ShouldExcludeStreamType(const AudioInterrupt &audioInterrupt)
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

bool AudioSession::IsAudioSessionEmpty()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return streamsInSession_.size() == 0;
}

bool AudioSession::IsAudioRendererEmpty()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    for (const auto &iter : streamsInSession_) {
        if (iter.audioFocusType.streamType != STREAM_DEFAULT) {
            return false;
        }
    }
    return true;
}

bool AudioSession::IsLegalDevice(const DeviceType deviceType)
{
    return (deviceType == DEVICE_TYPE_EARPIECE) ||
            (deviceType == DEVICE_TYPE_SPEAKER) ||
            (deviceType == DEVICE_TYPE_DEFAULT);
}

int32_t AudioSession::SetSessionDefaultOutputDevice(const DeviceType &deviceType)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    AUDIO_INFO_LOG("The session default output device is set to %{public}d", deviceType);
    if (!IsLegalDevice(deviceType)) {
        AUDIO_INFO_LOG("The deviceType is illegal, the default device will not be changed.");
        return ERROR_INVALID_PARAM;
    }

    defaultDeviceType_ = deviceType;

    return EnableDefaultDevice();
}

void AudioSession::GetSessionDefaultOutputDevice(DeviceType &deviceType)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    deviceType = defaultDeviceType_;
}

bool AudioSession::IsStreamContainedInCurrentSession(const uint32_t &streamId)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    for (const auto &streamInfo : streamsInSession_) {
        if (streamInfo.streamId == streamId) {
            return true;
        }
    }

    return false;
}

bool AudioSession::IsDeviceContainedInVector(std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices,
    const std::shared_ptr<AudioDeviceDescriptor> desc)
{
    return std::find(devices.begin(), devices.end(), desc) != devices.end();
}

bool AudioSession::IsCurrentDevicePrivateDevice(const std::shared_ptr<AudioDeviceDescriptor> desc)
{
    return IsDeviceContainedInVector(deviceManager_.GetCommRenderPrivacyDevices(), desc) ||
        IsDeviceContainedInVector(deviceManager_.GetMediaRenderPrivacyDevices(), desc);
}

bool AudioSession::IsRecommendToStopAudio(
    const std::shared_ptr<AudioPolicyServerHandler::EventContextObj> eventContextObj)
{
    bool ret = false;

    if ((eventContextObj == nullptr) || (eventContextObj->reason_ == AudioStreamDeviceChangeReason::OVERRODE) ||
        (eventContextObj->descriptor == nullptr)) {
        return ret;
    }

    std::lock_guard<std::mutex> lock(sessionMutex_);

    if (IsCurrentDevicePrivateDevice(std::make_shared<AudioDeviceDescriptor>(deviceDescriptor_)) &&
        (!IsCurrentDevicePrivateDevice(eventContextObj->descriptor))) {
        ret = true;
    }

    deviceDescriptor_ = AudioDeviceDescriptor(eventContextObj->descriptor);
    return ret;
}

bool AudioSession::IsSessionOutputDeviceChanged(const std::shared_ptr<AudioDeviceDescriptor> desc)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, true, "input device desc is nullptr");
    return deviceDescriptor_.IsSameDeviceDescPtr(desc);
}

StreamUsage AudioSession::GetSessionStreamUsage()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return GetStreamUsageInner();
}

bool AudioSession::IsBackGroundApp(void)
{
    OHOS::AppExecFwk::AppMgrClient appManager;
    OHOS::AppExecFwk::RunningProcessInfo infos;
    uint8_t state = 0;

    appManager.GetRunningProcessInfoByPid(callerPid_, infos);
    state = static_cast<uint8_t>(infos.state_);
    if (state == 0) {
        AUDIO_WARNING_LOG("Get app foreground and background state failed, callerPid_=%{public}d", callerPid_);
        return false;
    }

    return state == static_cast<uint8_t>(AppExecFwk::AppProcessState::APP_STATE_BACKGROUND);
}

} // namespace AudioStandard
} // namespace OHOS