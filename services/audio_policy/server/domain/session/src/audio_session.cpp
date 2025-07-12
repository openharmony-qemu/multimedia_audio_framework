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
#include "audio_device_manager.h"
#include "audio_pipe_manager.h"
#include "audio_stream_descriptor.h"
#include "audio_active_device.h"
#include "audio_device_common.h"

namespace OHOS {
namespace AudioStandard {

AudioSession::AudioSession(const int32_t callerPid, const AudioSessionStrategy &strategy,
    const std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor)
{
    AUDIO_INFO_LOG("AudioSession()");
    callerPid_ = callerPid;
    strategy_ = strategy;
    audioSessionStateMonitor_ = audioSessionStateMonitor;
    state_ = AudioSessionState::SESSION_NEW;
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
    return bypassStreamInfoVec_;
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
    for (auto stream : bypassStreamInfoVec_) {
        if (stream.streamId == incomingInterrupt.streamId) {
            AUDIO_INFO_LOG("stream aready exist.");
            return;
        }
    }

    bypassStreamInfoVec_.push_back(incomingInterrupt);
}

void AudioSession::RemoveStreamInfo(uint32_t streamId)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    for (auto it = bypassStreamInfoVec_.begin(); it != bypassStreamInfoVec_.end(); ++it) {
        if (it->streamId == streamId) {
            bypassStreamInfoVec_.erase(it);
            break;
        }
    }
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
    AppendFormat(dumpString, "    - pid: %d, AudioSession state is: %u.\n",
        callerPid_, static_cast<uint32_t>(state_));
    AppendFormat(dumpString, "    - pid: %d, Stream in interruptMap are:\n", callerPid_);
    for (auto &it : interruptMap_) {
        AppendFormat(dumpString, "        - StreamId is: %u, streamType is: %u\n",
            it.first, static_cast<uint32_t>(it.second.first.audioFocusType.streamType));
    }
    AppendFormat(dumpString, "    - pid: %d, Bypass streams are:\n", callerPid_);
    for (auto &it : bypassStreamInfoVec_) {
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
    rendererInfo.streamUsage = GetStreamUsageByAudioSessionScene(audioSessionScene_);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredOutputDevices =
        AudioDeviceCommon::GetInstance().GetPreferredOutputDeviceDescInner(rendererInfo, LOCAL_NETWORK_ID);
    if ((preferredOutputDevices.size() == 0) || (preferredOutputDevices[0] == nullptr)) {
        deviceDescriptor_ = AudioActiveDevice::GetInstance().GetCurrentOutputDevice();
    } else {
        deviceDescriptor_ = AudioDeviceDescriptor(preferredOutputDevices[0]);
    }
    return SUCCESS;
}

int32_t AudioSession::Deactivate()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    state_ = AudioSessionState::SESSION_DEACTIVE;
    interruptMap_.clear();
    needToFetch_ = false;
    AUDIO_INFO_LOG("Audio session state change: pid %{public}d, state %{public}d",
        callerPid_, static_cast<int32_t>(state_));
    return SUCCESS;
}

int32_t AudioSession::EnableDefaultDevice()
{
    if ((state_ != AudioSessionState::SESSION_ACTIVE) || (defaultDeviceType_ == DEVICE_TYPE_INVALID)) {
        return SUCCESS;
    }

    int32_t ret = AudioDeviceManager::GetAudioDeviceManager().SetDefaultOutputDevice(defaultDeviceType_, fakeStreamId_,
        GetStreamUsageByAudioSessionScene(audioSessionScene_), true);
    if ((ret != NEED_TO_FETCH) && (ret != SUCCESS)) {
        AUDIO_ERR_LOG("SetDefaultOutputDevice for session failed, ret is %{public}d", ret);
    }

    return ret;
}

bool AudioSession::IsNeedToFetchDefaultDevice()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return needToFetch_;
}

StreamUsage AudioSession::GetStreamUsageByAudioSessionScene(const AudioSessionScene audioSessionScene)
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

    return StreamUsage::STREAM_USAGE_UNKNOWN;
}

AudioSessionState AudioSession::GetSessionState()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    AUDIO_INFO_LOG("pid %{public}d, state %{public}d", callerPid_, static_cast<int32_t>(state_));
    return state_;
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

int32_t AudioSession::AddAudioInterrpt(const std::pair<AudioInterrupt, AudioFocuState> interruptPair)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    if (interruptPair.first.isAudioSessionInterrupt) {
        return SUCCESS;
    }

    if (state_ == AudioSessionState::SESSION_ACTIVE &&
        audioSessionScene_ != AudioSessionScene::INVALID &&
        ShouldExcludeStreamType(interruptPair.first)) {
        return SUCCESS;
    }

    uint32_t streamId = interruptPair.first.streamId;
    AUDIO_INFO_LOG("AddAudioInterrpt: streamId %{public}u", streamId);

    if (interruptMap_.count(streamId) != 0) {
        AUDIO_WARNING_LOG("The streamId has been added. The old interrupt will be coverd.");
    }
    interruptMap_[streamId] = interruptPair;
    auto monitor = audioSessionStateMonitor_.lock();
    if (monitor != nullptr) {
        monitor->StopMonitor(callerPid_);
    }
    return SUCCESS;
}

int32_t AudioSession::RemoveAudioInterrpt(const std::pair<AudioInterrupt, AudioFocuState> interruptPair)
{
    uint32_t streamId = interruptPair.first.streamId;
    AUDIO_INFO_LOG("RemoveAudioInterrpt: streamId %{public}u", streamId);

    std::lock_guard<std::mutex> lock(sessionMutex_);
    if (interruptMap_.count(streamId) == 0) {
        AUDIO_WARNING_LOG("The streamId has been removed.");
        return SUCCESS;
    }
    interruptMap_.erase(streamId);

    auto monitor = audioSessionStateMonitor_.lock();
    if (interruptMap_.empty() && monitor != nullptr) {
        monitor->StartMonitor(callerPid_);
    }
    return SUCCESS;
}

int32_t AudioSession::RemoveAudioInterrptByStreamId(const uint32_t &streamId)
{
    AUDIO_INFO_LOG("RemoveAudioInterrptByStreamId: streamId %{public}u", streamId);

    std::lock_guard<std::mutex> lock(sessionMutex_);
    if (interruptMap_.count(streamId) == 0) {
        AUDIO_WARNING_LOG("The streamId has been removed.");
        return SUCCESS;
    }
    interruptMap_.erase(streamId);

    auto monitor = audioSessionStateMonitor_.lock();
    if (interruptMap_.empty() && monitor != nullptr) {
        monitor->StartMonitor(callerPid_);
    }

    return SUCCESS;
}

bool AudioSession::IsAudioSessionEmpty()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    return interruptMap_.size() == 0;
}

bool AudioSession::IsAudioRendererEmpty()
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    for (const auto &iter : interruptMap_) {
        if (iter.second.first.audioFocusType.streamType != STREAM_DEFAULT) {
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

    if (state_ == AudioSessionState::SESSION_ACTIVE) {
        int32_t ret = EnableDefaultDevice();
        if ((ret == NEED_TO_FETCH) || (ret == SUCCESS)) {
            return ret;
        } else {
            AUDIO_ERR_LOG("Audio session enable default device failed, ret is %{public}d.", ret);
        }
    }

    return SUCCESS;
}

void AudioSession::GetSessionDefaultOutputDevice(DeviceType &deviceType)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    deviceType = defaultDeviceType_;
}

bool AudioSession::IsStreamContainedInCurrentSession(const uint32_t &streamId)
{
    std::lock_guard<std::mutex> lock(sessionMutex_);
    for (auto streamInfo : bypassStreamInfoVec_) {
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
    AudioDeviceManager& deviceManager = AudioDeviceManager::GetAudioDeviceManager();

    return IsDeviceContainedInVector(deviceManager.GetCommRenderPrivacyDevices(), desc) ||
        IsDeviceContainedInVector(deviceManager.GetMediaRenderPrivacyDevices(), desc);
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
    return GetStreamUsageByAudioSessionScene(audioSessionScene_);
}
} // namespace AudioStandard
} // namespace OHOS