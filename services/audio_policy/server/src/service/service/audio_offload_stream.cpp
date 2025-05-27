/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioOffloadStream"
#endif

#include "audio_offload_stream.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "media_monitor_manager.h"
#include "audio_spatialization_service.h"

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"
#include "audio_config_manager.h"

namespace OHOS {
namespace AudioStandard {

const int32_t UID_AUDIO = 1041;
const int32_t dAudioClientUid = 3055;
static const int32_t WAIT_OFFLOAD_CLOSE_TIME_S = 10; // 10s
static const int32_t SELECT_PIPE_TYPE_OFFLOAD_MUTE_US = 200000; // 200ms

inline std::string PrintSinkInput(SinkInput sinkInput)
{
    std::stringstream value;
    value << "streamId:[" << sinkInput.streamId << "] ";
    value << "streamType:[" << sinkInput.streamType << "] ";
    value << "uid:[" << sinkInput.uid << "] ";
    value << "pid:[" << sinkInput.pid << "] ";
    value << "statusMark:[" << sinkInput.statusMark << "] ";
    value << "sinkName:[" << sinkInput.sinkName << "] ";
    value << "startTime:[" << sinkInput.startTime << "]";
    return value.str();
}

void AudioOffloadStream::HandlePowerStateChanged(PowerMgr::PowerState state)
{
    if (currentPowerState_ == state) {
        return;
    }
    currentPowerState_ = state;
    if (!audioActiveDevice_.CheckActiveOutputDeviceSupportOffload()) {
        return;
    }
    if (offloadSessionID_.has_value()) {
        AUDIO_DEBUG_LOG("SetOffloadMode! Offload power is state = %{public}d", state);
        SetOffloadMode();
    }
}

void AudioOffloadStream::SetOffloadAvailableFromXML(AudioModuleInfo &moduleInfo)
{
    if (moduleInfo.name == "Speaker") {
        for (const auto &portInfo : moduleInfo.ports) {
            if ((portInfo.adapterName == "primary") && (portInfo.offloadEnable == "1")) {
                isOffloadAvailable_ = true;
            }
        }
    }
}

bool AudioOffloadStream::GetOffloadAvailableFromXml() const
{
    return isOffloadAvailable_;
}

void AudioOffloadStream::SetOffloadMode()
{
    if (!GetOffloadAvailableFromXml()) {
        AUDIO_INFO_LOG("Offload not available, skipped");
        return;
    }

    AUDIO_INFO_LOG("sessionId: %{public}d, PowerState: %{public}d, isAppBack: %{public}d",
        *offloadSessionID_, static_cast<int32_t>(currentPowerState_), currentOffloadSessionIsBackground_);
    AudioServerProxy::GetInstance().SetOffloadModeProxy(*offloadSessionID_, static_cast<int32_t>(currentPowerState_),
        currentOffloadSessionIsBackground_);
}

void AudioOffloadStream::OffloadStreamSetCheck(uint32_t sessionId)
{
    AudioPipeType pipeType = PIPE_TYPE_OFFLOAD;
    int32_t ret = AudioStreamCollector::GetAudioStreamCollector().ActivateAudioConcurrency(pipeType);
    if (ret != SUCCESS) {
        return;
    }
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    std::string curOutputNetworkId = audioActiveDevice_.GetCurrentOutputDeviceNetworkId();
    std::string curOutputMacAddr = audioActiveDevice_.GetCurrentOutputDeviceMacAddr();
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    ret = streamCollector_.GetRendererDeviceInfo(sessionId, deviceInfo);
    if (ret != SUCCESS || curOutputNetworkId != LOCAL_NETWORK_ID ||
        curOutputDeviceType == DEVICE_TYPE_REMOTE_CAST ||
        deviceInfo.deviceType_ != curOutputDeviceType ||
        deviceInfo.networkId_ != curOutputNetworkId ||
        deviceInfo.macAddress_ != curOutputMacAddr) {
        AUDIO_INFO_LOG("sessionId[%{public}d] not fetch device, Offload Skipped", sessionId);
        return;
    }

    AudioStreamType streamType = streamCollector_.GetStreamType(sessionId);
    if (!CheckStreamOffloadMode(sessionId, streamType)) {
        return;
    }

    Trace trace("AudioOffloadStream::OffloadStreamSetCheck:Getting offload stream:" + std::to_string(sessionId));
    auto CallingUid = IPCSkeleton::GetCallingUid();
    AUDIO_INFO_LOG("sessionId[%{public}d]  CallingUid[%{public}d] StreamType[%{public}d] "
                   "Getting offload stream", sessionId, CallingUid, streamType);
    std::lock_guard<std::mutex> lock(offloadMutex_);

    if (!offloadSessionID_.has_value()) {
        offloadSessionID_ = sessionId;
        audioPolicyManager_.SetOffloadSessionId(sessionId);

        AUDIO_DEBUG_LOG("sessionId[%{public}d] try get offload stream", sessionId);
        if (MoveToNewPipeInner(sessionId, PIPE_TYPE_OFFLOAD) != SUCCESS) {
            AUDIO_ERR_LOG("sessionId[%{public}d]  CallingUid[%{public}d] StreamType[%{public}d] "
                "failed to offload stream", sessionId, CallingUid, streamType);
            offloadSessionID_.reset();
            audioPolicyManager_.ResetOffloadSessionId();
            return;
        }
        SetOffloadMode();
    } else {
        if (sessionId == *(offloadSessionID_)) {
            AUDIO_DEBUG_LOG("sessionId[%{public}d] is already get offload stream", sessionId);
        } else {
            AUDIO_DEBUG_LOG("sessionId[%{public}d] no get offload, current offload sessionId[%{public}d]",
                sessionId, *(offloadSessionID_));
        }
    }

    return;
}

bool AudioOffloadStream::CheckStreamOffloadMode(int64_t activateSessionId, AudioStreamType streamType)
{
    if (!GetOffloadAvailableFromXml()) {
        AUDIO_INFO_LOG("Offload not available, skipped for set");
        return false;
    }

    if (!audioActiveDevice_.CheckActiveOutputDeviceSupportOffload()) {
        AUDIO_PRERELEASE_LOGI("Offload not available on current output device, skipped");
        return false;
    }

    if (!streamCollector_.IsOffloadAllowed(activateSessionId)) {
        AUDIO_PRERELEASE_LOGI("Offload is not allowed, Skipped");
        return false;
    }

    if ((streamType != STREAM_MUSIC) && (streamType != STREAM_SPEECH)) {
        AUDIO_DEBUG_LOG("StreamType not allowed get offload mode, Skipped");
        return false;
    }

    AudioPipeType pipeType;
    streamCollector_.GetPipeType(activateSessionId, pipeType);
    if (pipeType == PIPE_TYPE_DIRECT_MUSIC) {
        AUDIO_INFO_LOG("stream is direct, Skipped");
        return false;
    }

    int32_t channelCount = streamCollector_.GetChannelCount(activateSessionId);
    if ((channelCount != AudioChannel::MONO) && (channelCount != AudioChannel::STEREO)) {
        AUDIO_DEBUG_LOG("ChannelNum not allowed get offload mode, Skipped");
        return false;
    }

    int32_t offloadUID = streamCollector_.GetUid(activateSessionId);
    if (offloadUID == -1) {
        AUDIO_DEBUG_LOG("offloadUID not valid, Skipped");
        return false;
    }
    if (offloadUID == UID_AUDIO) {
        AUDIO_DEBUG_LOG("Skip anco_audio out of offload mode");
        return false;
    }

    if (CheckSpatializationAndEffectState()) {
        AUDIO_INFO_LOG("spatialization effect in arm, Skipped");
        return false;
    }
    return true;
}

AudioModuleInfo AudioOffloadStream::ConstructMchAudioModuleInfo(DeviceType deviceType)
{
    AudioModuleInfo audioModuleInfo = {};
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.format = "s32le"; // 32bit little endian
    audioModuleInfo.fixedLatency = "1"; // here we need to set latency fixed for a fixed buffer size.

    // used as "sink_name" in hdi_sink.c, hope we could use name to find target sink.
    audioModuleInfo.name = MCH_PRIMARY_SPEAKER;

    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(deviceType);
    audioModuleInfo.deviceType = typeValue.str();

    audioModuleInfo.defaultAdapterEnable = AudioConfigManager::GetInstance().GetDefaultAdapterEnable() ? "1" : "0";
    audioModuleInfo.adapterName = "primary";
    audioModuleInfo.className = "multichannel"; // used in renderer_sink_adapter.c
    audioModuleInfo.fileName = "mch_dump_file";

    audioModuleInfo.channels = "6";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.bufferSize = "7680";

    return audioModuleInfo;
}

bool AudioOffloadStream::CheckSpatializationAndEffectState()
{
    AudioSpatializationState spatialState =
        AudioSpatializationService::GetAudioSpatializationService().GetSpatializationState();
    bool effectOffloadFlag = AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
    return spatialState.spatializationEnabled && !effectOffloadFlag;
}

int32_t AudioOffloadStream::LoadMchModule()
{
    AUDIO_INFO_LOG("load multichannel mode");
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioModuleInfo moduleInfo = ConstructMchAudioModuleInfo(deviceType);
    audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
    return SUCCESS;
}

int32_t AudioOffloadStream::UnloadMchModule()
{
    AUDIO_INFO_LOG("unload multichannel module");
    return audioIOHandleMap_.ClosePortAndEraseIOHandle(MCH_PRIMARY_SPEAKER);
}

int32_t AudioOffloadStream::MoveToNewPipeInner(uint32_t sessionId, AudioPipeType pipeType)
{
    AudioPipeType oldPipeType;
    streamCollector_.GetPipeType(sessionId, oldPipeType);
    Trace tracePipe("AudioOffloadStream::MoveToNewPipeInner:sessionId:" + std::to_string(sessionId) +
        " from " + std::to_string(oldPipeType) + " to " + std::to_string(pipeType));
    if (oldPipeType == pipeType) {
        AUDIO_ERR_LOG("the same type [%{public}d],no need to move", pipeType);
        return SUCCESS;
    }
    Trace trace("AudioOffloadStream::MoveToNewPipeInner");
    AUDIO_INFO_LOG("start move stream %{public}d from %{public}d into new pipe %{public}d", sessionId,
        oldPipeType, pipeType);
    int32_t ret = SwitchToNewPipe(sessionId, pipeType);

    return ret;
}

int32_t AudioOffloadStream::SwitchToNewPipe(const uint32_t sessionId, const AudioPipeType pipeType)
{
    int32_t ret = ERROR;
    std::string portName = PORT_NONE;
    AudioStreamType streamType = streamCollector_.GetStreamType(sessionId);
    DeviceType deviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    switch (pipeType) {
        case PIPE_TYPE_OFFLOAD: {
            if (!CheckStreamOffloadMode(sessionId, streamType)) {
                return ERROR;
            }
            if (LoadOffloadModule() != SUCCESS) {
                return ERROR;
            }
            portName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
            audioIOHandleMap_.MuteSinkPort(portName, SELECT_PIPE_TYPE_OFFLOAD_MUTE_US, true, false);
            ret = MoveToOutputDevice(sessionId, portName);
            break;
        }
        case PIPE_TYPE_MULTICHANNEL: {
            if (!CheckStreamMultichannelMode(sessionId)) {
                return ERROR;
            }
            if (audioIOHandleMap_.CheckIOHandleExist(MCH_PRIMARY_SPEAKER) == false) {
                // load moudle and move into new sink
                LoadMchModule();
            }
            portName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
            ret = MoveToOutputDevice(sessionId, portName);
            break;
        }
        case PIPE_TYPE_NORMAL_OUT: {
            portName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
            ret = MoveToOutputDevice(sessionId, portName);
            break;
        }
        default:
            AUDIO_WARNING_LOG("not supported for pipe type %{public}d", pipeType);
            break;
    }
    if (ret == SUCCESS) {
        streamCollector_.UpdateRendererPipeInfo(sessionId, pipeType);
    }
    return ret;
}

void AudioOffloadStream::ResetOffloadMode(int32_t sessionId)
{
    AUDIO_DEBUG_LOG("Doing reset offload mode!");

    if (!audioActiveDevice_.CheckActiveOutputDeviceSupportOffload()) {
        AUDIO_DEBUG_LOG("Resetting offload not available on this output device! Release.");
        OffloadStreamReleaseCheck(*offloadSessionID_);
        return;
    }

    OffloadStreamSetCheck(sessionId);
}

void AudioOffloadStream::OffloadStreamReleaseCheck(uint32_t sessionId)
{
    if (!GetOffloadAvailableFromXml()) {
        AUDIO_INFO_LOG("Offload not available, skipped for release");
        return;
    }

    std::lock_guard<std::mutex> lock(offloadMutex_);

    if (((*offloadSessionID_) == sessionId) && offloadSessionID_.has_value()) {
        AUDIO_DEBUG_LOG("Doing unset offload mode!");
        AudioServerProxy::GetInstance().UnsetOffloadModeProxy(*offloadSessionID_);
        AudioPipeType normalPipe = PIPE_TYPE_NORMAL_OUT;
        MoveToNewPipe(sessionId, normalPipe);
        streamCollector_.UpdateRendererPipeInfo(sessionId, normalPipe);
        DynamicUnloadOffloadModule();
        offloadSessionID_.reset();
        audioPolicyManager_.ResetOffloadSessionId();
        AUDIO_DEBUG_LOG("sessionId[%{public}d] release offload stream", sessionId);
    } else {
        if (offloadSessionID_.has_value()) {
            AUDIO_DEBUG_LOG("sessionId[%{public}d] stopping stream not get offload, current offload [%{public}d]",
                sessionId, *offloadSessionID_);
        } else {
            AUDIO_DEBUG_LOG("sessionId[%{public}d] stopping stream not get offload, current offload stream is None",
                sessionId);
        }
    }
    return;
}

int32_t AudioOffloadStream::MoveToNewPipe(uint32_t sessionId, AudioPipeType pipeType)
{
    // Check if the stream exists
    int32_t defaultUid = -1;
    if (defaultUid == streamCollector_.GetUid(sessionId)) {
        AUDIO_ERR_LOG("The audio stream information [%{public}d] is illegal", sessionId);
        return ERROR;
    }
    // move the stream to new pipe
    return MoveToNewPipeInner(sessionId, pipeType);
}

AudioModuleInfo AudioOffloadStream::ConstructOffloadAudioModuleInfo(DeviceType deviceType)
{
    AudioModuleInfo audioModuleInfo = {};
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.format = "s32le"; // 32bit little endian
    audioModuleInfo.fixedLatency = "1"; // here we need to set latency fixed for a fixed buffer size.

    // used as "sink_name" in hdi_sink.c, hope we could use name to find target sink.
    audioModuleInfo.name = OFFLOAD_PRIMARY_SPEAKER;

    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(deviceType);
    audioModuleInfo.deviceType = typeValue.str();

    audioModuleInfo.adapterName = "primary";
    audioModuleInfo.className = "offload"; // used in renderer_sink_adapter.c
    audioModuleInfo.fileName = "offload_dump_file";
    audioModuleInfo.offloadEnable = "1";

    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.bufferSize = "7680";

    return audioModuleInfo;
}

int32_t AudioOffloadStream::LoadOffloadModule()
{
    AUDIO_INFO_LOG("load offload mode");
    std::unique_lock<std::mutex> lock(offloadCloseMutex_);
    isOffloadOpened_.store(true);
    offloadCloseCondition_.notify_all();
    {
        std::lock_guard<std::mutex> lk(offloadOpenMutex_);
        if (audioIOHandleMap_.CheckIOHandleExist(OFFLOAD_PRIMARY_SPEAKER)) {
            AUDIO_INFO_LOG("offload is open");
            return SUCCESS;
        }

        DeviceType deviceType = DEVICE_TYPE_SPEAKER;
        AudioModuleInfo moduleInfo = ConstructOffloadAudioModuleInfo(deviceType);
        return audioIOHandleMap_.OpenPortAndInsertIOHandle(moduleInfo.name, moduleInfo);
    }
    return SUCCESS;
}

int32_t AudioOffloadStream::UnloadOffloadModule()
{
    AUDIO_INFO_LOG("unload offload module");
    std::unique_lock<std::mutex> lock(offloadCloseMutex_);
    // Try to wait 3 seconds before unloading the module, because the audio driver takes some time to process
    // the shutdown process..
    offloadCloseCondition_.wait_for(lock, std::chrono::seconds(WAIT_OFFLOAD_CLOSE_TIME_S),
        [this] () { return isOffloadOpened_.load(); });
    {
        std::lock_guard<std::mutex> lk(offloadOpenMutex_);
        if (isOffloadOpened_.load()) {
            AUDIO_INFO_LOG("offload restart");
            return ERROR;
        }
        audioIOHandleMap_.ClosePortAndEraseIOHandle(OFFLOAD_PRIMARY_SPEAKER);
    }
    return SUCCESS;
}


int32_t AudioOffloadStream::DynamicUnloadOffloadModule()
{
    if (isOffloadOpened_.load()) {
        isOffloadOpened_.store(false);
        auto unloadFirOffloadThrd = [this] { this->UnloadOffloadModule(); };
        std::thread unloadOffloadThrd(unloadFirOffloadThrd);
        unloadOffloadThrd.detach();
    }
    return SUCCESS;
}

void AudioOffloadStream::RemoteOffloadStreamRelease(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(offloadMutex_);
    if (offloadSessionID_.has_value() && ((*offloadSessionID_) == sessionId)) {
        AUDIO_DEBUG_LOG("Doing unset offload mode!");
        AudioServerProxy::GetInstance().UnsetOffloadModeProxy(*offloadSessionID_);
        AudioPipeType normalPipe = PIPE_TYPE_UNKNOWN;
        MoveToNewPipe(sessionId, normalPipe);
        streamCollector_.UpdateRendererPipeInfo(sessionId, normalPipe);
        DynamicUnloadOffloadModule();
        offloadSessionID_.reset();
        audioPolicyManager_.ResetOffloadSessionId();
        AUDIO_DEBUG_LOG("sessionId[%{public}d] release offload stream", sessionId);
    }
}

int32_t AudioOffloadStream::MoveToOutputDevice(uint32_t sessionId, std::string portName)
{
    std::vector<SinkInput> sinkInputs;
    audioPolicyManager_.GetAllSinkInputs(sinkInputs);
    std::vector<SinkInput> sinkInputIds = FilterSinkInputs(sessionId, sinkInputs);

    if (portName == BLUETOOTH_SPEAKER) {
        std::string activePort = BLUETOOTH_SPEAKER;
        audioPolicyManager_.SuspendAudioDevice(activePort, false);
    }
    AUDIO_INFO_LOG("move for session [%{public}d], portName %{public}s", sessionId, portName.c_str());
    // start move.
    uint32_t sinkId = -1; // invalid sink id, use sink name instead.
    for (size_t i = 0; i < sinkInputIds.size(); i++) {
        int32_t ret = audioPolicyManager_.MoveSinkInputByIndexOrName(sinkInputIds[i].paStreamId, sinkId, portName);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR,
            "move [%{public}d] to local failed", sinkInputIds[i].streamId);
        audioRouteMap_.AddRouteMapInfo(sinkInputIds[i].uid, LOCAL_NETWORK_ID, sinkInputIds[i].pid);
    }
    return SUCCESS;
}

bool AudioOffloadStream::CheckStreamMultichannelMode(const int64_t activateSessionId)
{
    if (audioActiveDevice_.GetCurrentOutputDeviceNetworkId() != LOCAL_NETWORK_ID ||
        audioActiveDevice_.GetCurrentOutputDeviceType() == DEVICE_TYPE_REMOTE_CAST) {
        return false;
    }

    // Multi-channel mode only when the number of channels is greater than 2.
    int32_t channelCount = streamCollector_.GetChannelCount(activateSessionId);
    if (channelCount < AudioChannel::CHANNEL_3) {
        AUDIO_DEBUG_LOG("ChannelNum not allowed get multichannel mode, Skipped");
        return false;
    }

    // The multi-channel algorithm needs to be supported in the DSP
    return AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
}
void AudioOffloadStream::CheckStreamMode(const int64_t activateSessionId)
{
    Trace trace("AudioOffloadStream::CheckStreamMode:activateSessionId:" + std::to_string(activateSessionId));
    if (CheckStreamMultichannelMode(activateSessionId)) {
        AudioPipeType pipeMultiChannel = PIPE_TYPE_MULTICHANNEL;
        int32_t ret = streamCollector_.ActivateAudioConcurrency(pipeMultiChannel);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "concede incoming multichannel");
        MoveToNewPipeInner(activateSessionId, PIPE_TYPE_MULTICHANNEL);
    }
}

std::vector<SinkInput> AudioOffloadStream::FilterSinkInputs(int32_t sessionId, std::vector<SinkInput> sinkInputs)
{
    // find sink-input id with audioRendererFilter
    std::vector<SinkInput> targetSinkInputs = {};

    for (size_t i = 0; i < sinkInputs.size(); i++) {
        CHECK_AND_CONTINUE_LOG(sinkInputs[i].uid != dAudioClientUid,
            "Find sink-input with daudio[%{public}d]", sinkInputs[i].pid);
        CHECK_AND_CONTINUE_LOG(sinkInputs[i].streamType != STREAM_DEFAULT,
            "Sink-input[%{public}zu] of effect sink, don't move", i);
        AUDIO_DEBUG_LOG("sinkinput[%{public}zu]:%{public}s", i, PrintSinkInput(sinkInputs[i]).c_str());
        if (sessionId == sinkInputs[i].streamId) {
            targetSinkInputs.push_back(sinkInputs[i]);
        }
    }
    return targetSinkInputs;
}

void AudioOffloadStream::ResetOffloadModeOnSpatializationChanged(std::vector<int32_t> &allSessions)
{
    AudioSpatializationState spatialState =
        AudioSpatializationService::GetAudioSpatializationService().GetSpatializationState();
    bool effectOffloadFlag = AudioServerProxy::GetInstance().GetEffectOffloadEnabledProxy();
    AUDIO_INFO_LOG("spatialization: %{public}d, headTracking: %{public}d, effectOffloadFlag: %{public}d",
        spatialState.spatializationEnabled, spatialState.headTrackingEnabled, effectOffloadFlag);
    if (spatialState.spatializationEnabled) {
        if (effectOffloadFlag) {
            for (auto it = allSessions.begin(); it != allSessions.end(); it++) {
                OffloadStreamSetCheck(*it);
            }
        } else {
            OffloadStreamReleaseCheck(*offloadSessionID_);
        }
    }
}

int32_t AudioOffloadStream::ActivateConcurrencyFromServer(AudioPipeType incomingPipe)
{
    std::lock_guard<std::mutex> lock(offloadMutex_);
    CHECK_AND_RETURN_RET_LOG(!offloadSessionID_.has_value(),
        ERR_ILLEGAL_STATE, "Offload stream existing, concede incoming lowlatency stream");
    return SUCCESS;
}

}
}