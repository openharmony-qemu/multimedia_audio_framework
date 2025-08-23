/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "FastAudioStream"
#endif

#include <chrono>
#include <thread>
#include <vector>

#include "audio_errors.h"
#include "audio_capturer_log.h"

#include "fast_audio_stream.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
FastAudioStream::FastAudioStream(AudioStreamType eStreamType, AudioMode eMode, int32_t appUid)
    : eStreamType_(eStreamType),
      eMode_(eMode),
      state_(NEW),
      renderMode_(RENDER_MODE_CALLBACK),
      captureMode_(CAPTURE_MODE_CALLBACK)
{
    AUDIO_INFO_LOG("FastAudioStream ctor, appUID = %{public}d", appUid);
    audioStreamTracker_ = std::make_unique<AudioStreamTracker>(eMode, appUid);
    AUDIO_DEBUG_LOG("AudioStreamTracker created");
}

FastAudioStream::~FastAudioStream()
{
    if (state_ != RELEASED && state_ != NEW) {
        ReleaseAudioStream(false);
    }
    AUDIO_INFO_LOG("FastAudioStream dtor, session %{public}u", sessionId_);
}

void FastAudioStream::SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId, uint64_t fullTokenId)
{
    AUDIO_INFO_LOG("Set fast stream client PID:%{public}d UID:%{public}d appTokenId:%{public}u "
        "fullTokenId:%{public}" PRIu64, clientPid, clientUid, appTokenId, fullTokenId);
    clientPid_ = clientPid;
    clientUid_ = clientUid;
    appTokenId_ = appTokenId;
    fullTokenId_ = fullTokenId;
}

int32_t FastAudioStream::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    AUDIO_ERR_LOG("Unsupported operation!");
    return ERR_NOT_SUPPORTED;
}

void FastAudioStream::SetRendererInfo(const AudioRendererInfo &rendererInfo)
{
    rendererInfo_ = rendererInfo;
    rendererInfo_.samplingRate = static_cast<AudioSamplingRate>(streamInfo_.samplingRate);
}

void FastAudioStream::GetRendererInfo(AudioRendererInfo &rendererInfo)
{
    rendererInfo = rendererInfo_;
}

void FastAudioStream::SetCapturerInfo(const AudioCapturerInfo &capturerInfo)
{
    capturerInfo_ = capturerInfo;
    capturerInfo_.samplingRate = static_cast<AudioSamplingRate>(streamInfo_.samplingRate);
}

int32_t FastAudioStream::InitializeAudioProcessConfig(AudioProcessConfig &config, const AudioStreamParams &info)
{
    config.appInfo.appPid = clientPid_;
    config.appInfo.appUid = clientUid_;
    config.appInfo.appTokenId = appTokenId_;
    config.appInfo.appFullTokenId = fullTokenId_;
    config.audioMode = eMode_;
    config.streamInfo.channels = static_cast<AudioChannel>(info.channels);
    config.streamInfo.encoding = static_cast<AudioEncodingType>(info.encoding);
    config.streamInfo.format = static_cast<AudioSampleFormat>(info.format);
    config.streamInfo.samplingRate = static_cast<AudioSamplingRate>(info.samplingRate);
    config.streamType = eStreamType_;
    config.originalSessionId = info.originalSessionId;
    if (eMode_ == AUDIO_MODE_PLAYBACK) {
        AUDIO_DEBUG_LOG("FastAudioStream: Initialize playback");
        config.rendererInfo.contentType = rendererInfo_.contentType;
        config.rendererInfo.streamUsage = rendererInfo_.streamUsage;
        config.rendererInfo.rendererFlags = STREAM_FLAG_FAST;
        config.rendererInfo.volumeMode = rendererInfo_.volumeMode;
        config.rendererInfo.isVirtualKeyboard = rendererInfo_.isVirtualKeyboard;
        config.rendererInfo.originalFlag = rendererInfo_.originalFlag;
        config.rendererInfo.playerType = rendererInfo_.playerType;
        config.rendererInfo.expectedPlaybackDurationBytes = rendererInfo_.expectedPlaybackDurationBytes;
        config.rendererInfo.isLoopback = rendererInfo_.isLoopback;
        config.rendererInfo.loopbackMode = rendererInfo_.loopbackMode;
    } else if (eMode_ == AUDIO_MODE_RECORD) {
        AUDIO_DEBUG_LOG("FastAudioStream: Initialize recording");
        config.capturerInfo.sourceType = capturerInfo_.sourceType;
        config.capturerInfo.capturerFlags = STREAM_FLAG_FAST;
        config.capturerInfo.originalFlag = capturerInfo_.originalFlag;
        config.capturerInfo.recorderType = capturerInfo_.recorderType;
        config.capturerInfo.isLoopback = capturerInfo_.isLoopback;
        config.capturerInfo.loopbackMode = capturerInfo_.loopbackMode;
    } else {
        return ERR_INVALID_OPERATION;
    }
    return SUCCESS;
}

int32_t FastAudioStream::SetAudioStreamInfo(const AudioStreamParams info,
    const std::shared_ptr<AudioClientTracker> &proxyObj,
    const AudioPlaybackCaptureConfig &filterConfig)
{
    AUDIO_INFO_LOG("FastAudioStreamInfo, Sampling rate: %{public}d, channels: %{public}d, format: %{public}d,"
        " stream type: %{public}d", info.samplingRate, info.channels, info.format, eStreamType_);
    CHECK_AND_RETURN_RET_LOG(processClient_ == nullptr, ERR_INVALID_OPERATION,
        "Process is already inited, reset stream info is not supported.");
    streamInfo_ = info;
    if (state_ != NEW) {
        AUDIO_INFO_LOG("FastAudioStream: State is not new, release existing stream");
        StopAudioStream();
        ReleaseAudioStream(false);
    }
    AudioProcessConfig config;
    int32_t ret = InitializeAudioProcessConfig(config, info);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Initialize failed.");
    CHECK_AND_RETURN_RET_LOG(AudioProcessInClient::CheckIfSupport(config), ERR_INVALID_PARAM,
        "Stream is not supported.");
    processconfig_ = config;
    // OS_AudioPlayCb/RecordCb should lock weak_ptr of FastAudioStream before calling OnWriteData to
    // avoid using FastAudioStream after free in callback.
    auto weakStream = weak_from_this();
    processClient_ = AudioProcessInClient::Create(config, weakStream);
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_INVALID_PARAM,
        "Client test creat process client fail.");
    state_ = PREPARED;
    proxyObj_ = proxyObj;

    if (audioStreamTracker_ != nullptr && audioStreamTracker_.get()) {
        processClient_->GetSessionID(sessionId_);

        AudioRegisterTrackerInfo registerTrackerInfo;
        UpdateRegisterTrackerInfo(registerTrackerInfo);
        audioStreamTracker_->RegisterTracker(registerTrackerInfo, proxyObj);
    }
    InitCallbackHandler();
    return SUCCESS;
}

void FastAudioStream::InitCallbackHandler()
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (callbackHandler_ == nullptr) {
        callbackHandler_ = CallbackHandler::GetInstance(shared_from_this(), "OS_AudioStateCB");
    }
}

void FastAudioStream::SafeSendCallbackEvent(uint32_t eventCode, int64_t data)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    AUDIO_INFO_LOG("Send callback event, code: %{public}u, data: %{public}" PRId64, eventCode, data);
    CHECK_AND_RETURN_LOG(callbackHandler_ != nullptr && runnerReleased_ == false, "Runner is Released");
    callbackHandler_->SendCallbackEvent(eventCode, data);
}

void FastAudioStream::OnHandle(uint32_t code, int64_t data)
{
    AUDIO_DEBUG_LOG("On handle event, event code: %{public}u, data: %{public}" PRId64, code, data);
    switch (code) {
        case STATE_CHANGE_EVENT:
            HandleStateChangeEvent(data);
            break;
        default:
            break;
    }
}

void FastAudioStream::HandleStateChangeEvent(int64_t data)
{
    State state = INVALID;
    StateChangeCmdType cmdType = CMD_FROM_CLIENT;
    ParamsToStateCmdType(data, state, cmdType);
    std::unique_lock<std::mutex> lock(streamCbMutex_);
    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    if (streamCb != nullptr) {
        state = state != STOPPING ? state : STOPPED; // client only need STOPPED
        streamCb->OnStateChange(state, cmdType);
    }
}

int32_t FastAudioStream::ParamsToStateCmdType(int64_t params, State &state, StateChangeCmdType &cmdType)
{
    cmdType = CMD_FROM_CLIENT;
    switch (params) {
        case HANDLER_PARAM_NEW:
            state = NEW;
            break;
        case HANDLER_PARAM_PREPARED:
            state = PREPARED;
            break;
        case HANDLER_PARAM_RUNNING:
            state = RUNNING;
            break;
        case HANDLER_PARAM_STOPPED:
            state = STOPPED;
            break;
        case HANDLER_PARAM_RELEASED:
            state = RELEASED;
            break;
        case HANDLER_PARAM_PAUSED:
            state = PAUSED;
            break;
        case HANDLER_PARAM_STOPPING:
            state = STOPPING;
            break;
        case HANDLER_PARAM_RUNNING_FROM_SYSTEM:
            state = RUNNING;
            cmdType = CMD_FROM_SYSTEM;
            break;
        case HANDLER_PARAM_PAUSED_FROM_SYSTEM:
            state = PAUSED;
            cmdType = CMD_FROM_SYSTEM;
            break;
        default:
            state = INVALID;
            break;
    }
    return SUCCESS;
}

int32_t FastAudioStream::GetAudioStreamInfo(AudioStreamParams &audioStreamInfo)
{
    AUDIO_INFO_LOG("enter.");
    audioStreamInfo = streamInfo_;
    return SUCCESS;
}

int32_t FastAudioStream::GetAudioSessionID(uint32_t &sessionID)
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED,
        "GetAudioSessionID failed: null process");
    int32_t ret = processClient_->GetSessionID(sessionID);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSessionID error.");
    return ret;
}

void FastAudioStream::GetAudioPipeType(AudioPipeType &pipeType)
{
    pipeType = eMode_ == AUDIO_MODE_PLAYBACK ? rendererInfo_.pipeType : capturerInfo_.pipeType;
}

State FastAudioStream::GetState()
{
    std::lock_guard lock(switchingMutex_);
    if (switchingInfo_.isSwitching_) {
        AUDIO_INFO_LOG("switching, return state in switchingInfo");
        return switchingInfo_.state_;
    }
    return state_;
}

bool FastAudioStream::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(base == Timestamp::MONOTONIC, false, "GetAudioTime failed: invalid base");

    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, false, "GetAudioTime failed: null process");
    int64_t timeSec = 0;
    int64_t timeNsec = 0;
    bool ret = processClient_->GetAudioTime(timestamp.framePosition, timeSec, timeNsec);
    CHECK_AND_RETURN_RET_LOG(ret, false, "GetBufferSize error.");
    timestamp.time.tv_sec = timeSec;
    timestamp.time.tv_nsec = timeNsec;
    return true;
}

void FastAudioStream::SetSwitchInfoTimestamp(
    std::vector<std::pair<uint64_t, uint64_t>> lastFramePosAndTimePair,
    std::vector<std::pair<uint64_t, uint64_t>> lastFramePosAndTimePairWithSpeed)
{
    (void)lastFramePosAndTimePair;
    (void)lastFramePosAndTimePairWithSpeed;
    AUDIO_INFO_LOG("fast stream not support timestamp re-set when stream switching");
}

bool FastAudioStream::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    return GetAudioTime(timestamp, base);
}

int32_t FastAudioStream::GetBufferSize(size_t &bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED, "GetBufferSize failed: null process");
    int32_t ret = processClient_->GetBufferSize(bufferSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetBufferSize error.");
    return ret;
}

int32_t FastAudioStream::GetFrameCount(uint32_t &frameCount)
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED, "GetFrameCount failed: null process");
    int32_t ret = processClient_->GetFrameCount(frameCount);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetFrameCount error.");
    return ret;
}

int32_t FastAudioStream::GetLatency(uint64_t &latency)
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED, "GetLatency failed: null process");
    int32_t ret = processClient_->GetLatency(latency);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetLatency error.");
    return ret;
}

int32_t FastAudioStream::SetAudioStreamType(AudioStreamType audioStreamType)
{
    // Stream type can only be set when create.
    AUDIO_ERR_LOG("Unsupported operation: SetAudioStreamType");
    return ERR_INVALID_OPERATION;
}

int32_t FastAudioStream::SetVolume(float volume)
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED, "SetVolume failed: null process");
    int32_t ret = SUCCESS;
    ret = processClient_->SetVolume(volume);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetVolume error.");
    return ret;
}

float FastAudioStream::GetVolume()
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, 1.0f, "SetVolume failed: null process"); // 1.0f for default
    return processClient_->GetVolume();
}

int32_t FastAudioStream::SetLoudnessGain(float loudnessGain)
{
    AUDIO_WARNING_LOG("SetLoudnessGain is only for renderer");
    return ERROR;
}

float FastAudioStream::GetLoudnessGain()
{
    AUDIO_WARNING_LOG("GetLoudnessGain is only for renderer");
    return 0.0;
}

int32_t FastAudioStream::SetMute(bool mute, StateChangeCmdType cmdType)
{
    muteCmd_ = cmdType;
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED, "SetMute failed: null process");
    int32_t ret = processClient_->SetMute(mute);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetMute error.");
    return ret;
}

bool FastAudioStream::GetMute()
{
    return processClient_->GetMute();
}

int32_t FastAudioStream::SetSourceDuration(int64_t duration)
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED, "SetMute failed: null process");
    int32_t ret = processClient_->SetSourceDuration(duration);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSourceDuration error.");
    return ret;
}

int32_t FastAudioStream::SetDuckVolume(float volume)
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED, "SetDuckVolume failed: null process");
    int32_t ret = processClient_->SetDuckVolume(volume);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetDuckVolume error.");
    return ret;
}

float FastAudioStream::GetDuckVolume()
{
    return processClient_->GetDuckVolume();
}

void FastAudioStream::SetSilentModeAndMixWithOthers(bool on)
{
    AUDIO_PRERELEASE_LOGI("%{public}d", on);
    silentModeAndMixWithOthers_ = on;
    CHECK_AND_RETURN_LOG(processClient_ != nullptr, "processClient is null.");
    processClient_->SetSilentModeAndMixWithOthers(on);
}

bool FastAudioStream::GetSilentModeAndMixWithOthers()
{
    return silentModeAndMixWithOthers_;
}

int32_t FastAudioStream::SetRenderRate(AudioRendererRate renderRate)
{
    CHECK_AND_RETURN_RET(RENDER_RATE_NORMAL != renderRate, SUCCESS);
    AUDIO_ERR_LOG("Unsupported operation: SetRenderRate");
    return ERR_INVALID_OPERATION;
}

AudioRendererRate FastAudioStream::GetRenderRate()
{
    return renderRate_;
}

int32_t FastAudioStream::SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback)
{
    AUDIO_INFO_LOG("enter.");

    if (callback == nullptr) {
        AUDIO_ERR_LOG("SetStreamCallback failed. callback == nullptr");
        return ERR_INVALID_PARAM;
    }

    std::unique_lock<std::mutex> lock(streamCbMutex_);
    streamCallback_ = callback;
    lock.unlock();

    if (state_ != PREPARED) {
        return SUCCESS;
    }
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, PREPARED);
    return SUCCESS;
}

int32_t FastAudioStream::SetRenderMode(AudioRenderMode renderMode)
{
    CHECK_AND_RETURN_RET_LOG(renderMode == RENDER_MODE_CALLBACK && eMode_ == AUDIO_MODE_PLAYBACK,
        ERR_INVALID_OPERATION, "SetRenderMode is not supported.");
    return SUCCESS;
}

AudioRenderMode FastAudioStream::GetRenderMode()
{
    AUDIO_INFO_LOG("enter.");
    return renderMode_;
}

int32_t FastAudioStream::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    AUDIO_INFO_LOG("SetRendererWriteCallback enter.");
    CHECK_AND_RETURN_RET_LOG(callback && processClient_ && eMode_ == AUDIO_MODE_PLAYBACK,
        ERR_INVALID_PARAM, "callback is nullptr");
    spkProcClientCb_ = std::make_shared<FastAudioStreamRenderCallback>(callback, *this);
    int32_t ret = processClient_->SaveDataCallback(spkProcClientCb_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Client test save data callback fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t FastAudioStream::SetRendererFirstFrameWritingCallback(
    const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback)
{
    AUDIO_INFO_LOG("SetRendererFirstFrameWritingCallback in.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    firstFrameWritingCb_ = callback;
    return SUCCESS;
}

int32_t FastAudioStream::SetCaptureMode(AudioCaptureMode captureMode)
{
    CHECK_AND_RETURN_RET_LOG(captureMode == CAPTURE_MODE_CALLBACK && eMode_ == AUDIO_MODE_RECORD,
        ERR_INVALID_OPERATION, "SetCaptureMode is not supported.");
    return SUCCESS;
}

AudioCaptureMode FastAudioStream::GetCaptureMode()
{
    return captureMode_;
}

int32_t FastAudioStream::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    AUDIO_INFO_LOG("SetCapturerReadCallback enter.");
    CHECK_AND_RETURN_RET_LOG(callback && processClient_ && eMode_ == AUDIO_MODE_RECORD,
        ERR_INVALID_PARAM, "callback or client is nullptr or mode is not record.");
    micProcClientCb_ = std::make_shared<FastAudioStreamCaptureCallback>(callback);
    int32_t ret = processClient_->SaveDataCallback(micProcClientCb_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Client save data callback fail, ret %{public}d.", ret);
    return SUCCESS;
}

int32_t FastAudioStream::GetBufferDesc(BufferDesc &bufDesc)
{
    AUDIO_DEBUG_LOG("enter.");
    CHECK_AND_RETURN_RET_LOG(processClient_, ERR_INVALID_OPERATION, "spkClient is null.");
    int32_t ret = processClient_->GetBufferDesc(bufDesc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS && bufDesc.buffer != nullptr && bufDesc.bufLength != 0,
        -1, "GetBufferDesc failed.");
    return SUCCESS;
}

int32_t FastAudioStream::GetBufQueueState(BufferQueueState &bufState)
{
    AUDIO_INFO_LOG("enter.");
    // note: add support
    return SUCCESS;
}

int32_t FastAudioStream::Enqueue(const BufferDesc &bufDesc)
{
    AUDIO_DEBUG_LOG("Enqueue enter.");
    CHECK_AND_RETURN_RET_LOG(processClient_, ERR_INVALID_OPERATION,
        "spkClient is null.");
    int32_t ret = processClient_->Enqueue(bufDesc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, -1, "Enqueue failed.");
    return SUCCESS;
}

void FastAudioStream::SetPreferredFrameSize(int32_t frameSize)
{
    std::lock_guard<std::mutex> lockSetPreferredFrameSize(setPreferredFrameSizeMutex_);
    userSettedPreferredFrameSize_ = frameSize;
    CHECK_AND_RETURN_LOG(processClient_ != nullptr, "process client is null.");
    processClient_->SetPreferredFrameSize(frameSize);
}

void FastAudioStream::UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer)
{
    CHECK_AND_RETURN_LOG(processClient_ != nullptr, "process client is null.");
    processClient_->UpdateLatencyTimestamp(timestamp, isRenderer);
}

int32_t FastAudioStream::Clear()
{
    AUDIO_INFO_LOG("Clear will do nothing.");

    return SUCCESS;
}

int32_t FastAudioStream::SetLowPowerVolume(float volume)
{
    AUDIO_INFO_LOG("enter.");
    return SUCCESS;
}

float FastAudioStream::GetLowPowerVolume()
{
    AUDIO_INFO_LOG("enter.");
    return 1.0f;
}

int32_t FastAudioStream::SetOffloadMode(int32_t state, bool isAppBack)
{
    AUDIO_WARNING_LOG("enter.");
    return ERR_NOT_SUPPORTED;
}

int32_t FastAudioStream::UnsetOffloadMode()
{
    AUDIO_WARNING_LOG("enter.");
    return ERR_NOT_SUPPORTED;
}

float FastAudioStream::GetSingleStreamVolume()
{
    AUDIO_INFO_LOG("enter.");
    return 1.0f;
}

AudioEffectMode FastAudioStream::GetAudioEffectMode()
{
    AUDIO_ERR_LOG("GetAudioEffectMode not supported");
    return EFFECT_NONE;
}

int32_t FastAudioStream::SetAudioEffectMode(AudioEffectMode effectMode)
{
    AUDIO_ERR_LOG("SetAudioEffectMode not supported");
    return ERR_NOT_SUPPORTED;
}

int64_t FastAudioStream::GetFramesWritten()
{
    int64_t result = -1; // -1 invalid frame
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, result, "GetFramesWritten failed: null process");
    result = processClient_->GetFramesWritten();
    return result;
}

int64_t FastAudioStream::GetFramesRead()
{
    int64_t result = -1; // -1 invalid frame
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, result, "GetFramesRead failed: null process");
    result = processClient_->GetFramesRead();
    return result;
}

int32_t FastAudioStream::SetSpeed(float speed)
{
    AUDIO_ERR_LOG("SetSpeed is not supported");
    return ERR_OPERATION_FAILED;
}

int32_t FastAudioStream::SetPitch(float pitch)
{
    AUDIO_ERR_LOG("SetPitch is not supported");
    return ERR_OPERATION_FAILED;
}

float FastAudioStream::GetSpeed()
{
    AUDIO_ERR_LOG("GetSpeed is not supported");
    return static_cast<float>(ERROR);
}

// only call from StartAudioStream
void FastAudioStream::RegisterThreadPriorityOnStart(StateChangeCmdType cmdType)
{
    pid_t tid;
    switch (rendererInfo_.playerType) {
        case PLAYER_TYPE_ARKTS_AUDIO_RENDERER:
            // main thread
            tid = getpid();
            break;
        case PLAYER_TYPE_OH_AUDIO_RENDERER:
            tid = gettid();
            break;
        default:
            return;
    }

    if (cmdType == CMD_FROM_CLIENT) {
        std::lock_guard lock(lastCallStartByUserTidMutex_);
        lastCallStartByUserTid_ = tid;
    } else if (cmdType == CMD_FROM_SYSTEM) {
        std::lock_guard lock(lastCallStartByUserTidMutex_);
        CHECK_AND_RETURN_LOG(lastCallStartByUserTid_.has_value(), "has not value");
        tid = lastCallStartByUserTid_.value();
    } else {
        AUDIO_ERR_LOG("illegal param");
        return;
    }

    processClient_->RegisterThreadPriority(tid,
        AudioSystemManager::GetInstance()->GetSelfBundleName(processconfig_.appInfo.appUid), METHOD_START);
}

bool FastAudioStream::StartAudioStream(StateChangeCmdType cmdType,
    AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_PRERELEASE_LOGI("StartAudioStream enter.");
    CHECK_AND_RETURN_RET_LOG((state_ == PREPARED) || (state_ == STOPPED) || (state_ == PAUSED),
        false, "Illegal state:%{public}u", state_);

    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, false, "Start failed, process is null.");
    if (spkProcClientCb_ != nullptr) {
        AUDIO_DEBUG_LOG("StartAudioStream: reset the first frame state before starting");
        spkProcClientCb_->ResetFirstFrameState();
    }
    if (audioStreamTracker_ != nullptr && audioStreamTracker_.get()) {
        audioStreamTracker_->FetchOutputDeviceForTrack(sessionId_, RUNNING, clientPid_, rendererInfo_, reason);
        audioStreamTracker_->FetchInputDeviceForTrack(sessionId_, RUNNING, clientPid_, capturerInfo_);
    }
    int32_t ret = ERROR;
    if (state_ == PAUSED || state_ == STOPPED) {
        ret = processClient_->Resume();
    } else {
        ret = processClient_->Start();
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "Client test stop fail, ret %{public}d.", ret);
    state_ = RUNNING;

    AUDIO_DEBUG_LOG("StartAudioStream SUCCESS, sessionId: %{public}d", sessionId_);

    if (audioStreamTracker_ != nullptr && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("AudioStream:Calling Update tracker for Running");
        audioStreamTracker_->UpdateTracker(sessionId_, state_, clientPid_, rendererInfo_, capturerInfo_);
    }

    RegisterThreadPriorityOnStart(cmdType);

    SafeSendCallbackEvent(STATE_CHANGE_EVENT, state_);
    return true;
}

bool FastAudioStream::PauseAudioStream(StateChangeCmdType cmdType)
{
    AUDIO_PRERELEASE_LOGI("PauseAudioStream enter.");
    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, false,
        "state is not RUNNING. Illegal state:%{public}u", state_);
    State oldState = state_;

    state_ = PAUSED;
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, false, "Pause failed, process is null.");
    int32_t ret = processClient_->Pause();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("StreamPause fail,ret:%{public}d", ret);
        state_ = oldState;
        return false;
    }

    AUDIO_DEBUG_LOG("PauseAudioStream SUCCESS, sessionId: %{public}d", sessionId_);
    if (audioStreamTracker_ != nullptr && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("AudioStream:Calling Update tracker for Pause");
        audioStreamTracker_->UpdateTracker(sessionId_, state_, clientPid_, rendererInfo_, capturerInfo_);
    }

    SafeSendCallbackEvent(STATE_CHANGE_EVENT, state_);
    return true;
}

bool FastAudioStream::StopAudioStream()
{
    CHECK_AND_RETURN_RET_LOG((state_ == RUNNING) || (state_ == PAUSED), false,
        "State is not RUNNING. Illegal state:%{public}u", state_);
    State oldState = state_;
    state_ = STOPPED; // Set it before stopping as Read/Write and Stop can be called from different threads

    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, false, "Stop failed, process is null.");
    int32_t ret = processClient_->Stop();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("StreamStop fail,ret:%{public}d", ret);
        state_ = oldState;
        return false;
    }

    AUDIO_INFO_LOG("StopAudioStream SUCCESS, sessionId: %{public}d", sessionId_);
    if (audioStreamTracker_ != nullptr && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("AudioStream:Calling Update tracker for stop");
        audioStreamTracker_->UpdateTracker(sessionId_, state_, clientPid_, rendererInfo_, capturerInfo_);
    }

    SafeSendCallbackEvent(STATE_CHANGE_EVENT, state_);
    return true;
}

bool FastAudioStream::FlushAudioStream()
{
    AUDIO_PRERELEASE_LOGI("FlushAudioStream enter.");
    return true;
}

bool FastAudioStream::DrainAudioStream(bool stopFlag)
{
    AUDIO_INFO_LOG("Drain stream SUCCESS");
    return true;
}

bool FastAudioStream::ReleaseAudioStream(bool releaseRunner, bool isSwitchStream)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED && state_ != NEW,
        false, "Illegal state: state = %{public}u", state_);
    // If state_ is RUNNING try to Stop it first and Release
    if (state_ == RUNNING) {
        StopAudioStream();
    }

    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, false, "Release failed, process is null.");
    processClient_->Release(isSwitchStream);
    state_ = RELEASED;
    AUDIO_INFO_LOG("ReleaseAudiostream SUCCESS, sessionId: %{public}d", sessionId_);
    if (audioStreamTracker_ != nullptr && audioStreamTracker_.get()) {
        AUDIO_DEBUG_LOG("AudioStream:Calling Update tracker for release");
        audioStreamTracker_->UpdateTracker(sessionId_, state_, clientPid_, rendererInfo_, capturerInfo_);
    }

    std::unique_lock<std::mutex> lock(streamCbMutex_);
    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    if (streamCb != nullptr) {
        AUDIO_INFO_LOG("Notify client the state is released");
        streamCb->OnStateChange(RELEASED, CMD_FROM_CLIENT);
    }
    lock.unlock();
    return true;
}

int32_t FastAudioStream::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead)
{
    AUDIO_ERR_LOG("Unsupported operation: read");
    return ERR_INVALID_OPERATION;
}

int32_t FastAudioStream::Write(uint8_t *buffer, size_t buffer_size)
{
    AUDIO_ERR_LOG("Unsupported operation: Write");
    return ERR_INVALID_OPERATION;
}

int32_t FastAudioStream::Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer, size_t metaBufferSize)
{
    AUDIO_ERR_LOG("Unsupported operation: Write");
    return ERR_INVALID_OPERATION;
}

uint32_t FastAudioStream::GetUnderflowCount()
{
    AUDIO_INFO_LOG("GetUnderflowCount enter.");
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, 0, "process client is null.");
    underflowCount_ = processClient_->GetUnderflowCount();
    return underflowCount_;
}

uint32_t FastAudioStream::GetOverflowCount()
{
    AUDIO_INFO_LOG("enter.");
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, 0, "process client is null.");
    overflowCount_ = processClient_->GetOverflowCount();
    return overflowCount_;
}

void FastAudioStream::SetUnderflowCount(uint32_t underflowCount)
{
    CHECK_AND_RETURN_LOG(processClient_ != nullptr, "process client is null.");
    processClient_->SetUnderflowCount(underflowCount);
}

void FastAudioStream::SetOverflowCount(uint32_t overflowCount)
{
    CHECK_AND_RETURN_LOG(processClient_ != nullptr, "process client is null.");
    processClient_->SetOverflowCount(overflowCount);
}

void FastAudioStream::SetRendererPositionCallback(int64_t markPosition,
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    AUDIO_INFO_LOG("Registering render frame position callback mark position");
    // note: need support
}

void FastAudioStream::UnsetRendererPositionCallback()
{
    AUDIO_INFO_LOG("Unregistering render frame position callback");
    // note: need support
}

void FastAudioStream::SetRendererPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    AUDIO_INFO_LOG("Registering render period position callback");
}

void FastAudioStream::UnsetRendererPeriodPositionCallback()
{
    AUDIO_INFO_LOG("Unregistering render period position callback");
}

void FastAudioStream::SetCapturerPositionCallback(int64_t markPosition,
    const std::shared_ptr<CapturerPositionCallback> &callback)
{
    AUDIO_INFO_LOG("Registering capture frame position callback, mark position");
}

void FastAudioStream::UnsetCapturerPositionCallback()
{
    AUDIO_INFO_LOG("Unregistering capture frame position callback");
}

void FastAudioStream::SetCapturerPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    AUDIO_INFO_LOG("Registering period position callback");
}

void FastAudioStream::UnsetCapturerPeriodPositionCallback()
{
    AUDIO_INFO_LOG("Unregistering period position callback");
}

int32_t FastAudioStream::SetRendererSamplingRate(uint32_t sampleRate)
{
    AUDIO_ERR_LOG("SetRendererSamplingRate  is not supported");

    return ERR_OPERATION_FAILED;
}

uint32_t FastAudioStream::GetRendererSamplingRate()
{
    AUDIO_INFO_LOG("GetRendererSamplingRate enter.");
    return streamInfo_.samplingRate;
}

int32_t FastAudioStream::SetBufferSizeInMsec(int32_t bufferSizeInMsec)
{
    AUDIO_ERR_LOG("SetBufferSizeInMsec is not supported");
    // note: add support
    return ERR_NOT_SUPPORTED;
}

void FastAudioStream::SetInnerCapturerState(bool isInnerCapturer)
{
    AUDIO_ERR_LOG("SetInnerCapturerState is not supported");
}

void FastAudioStream::SetWakeupCapturerState(bool isWakeupCapturer)
{
    AUDIO_ERR_LOG("SetWakeupCapturerState is not supported");
}

void FastAudioStream::SetCapturerSource(int capturerSource)
{
    AUDIO_ERR_LOG("SetCapturerSource is not supported");
}

void FastAudioStream::SetPrivacyType(AudioPrivacyType privacyType)
{
    AUDIO_ERR_LOG("SetPrivacyType is not supported");
}

IAudioStream::StreamClass FastAudioStream::GetStreamClass()
{
    return IAudioStream::StreamClass::FAST_STREAM;
}

void FastAudioStream::SetStreamTrackerState(bool trackerRegisteredState)
{
    streamTrackerRegistered_ = trackerRegisteredState;
}

void FastAudioStream::GetSwitchInfo(IAudioStream::SwitchInfo& info)
{
    GetAudioStreamInfo(info.params);
    info.rendererInfo = rendererInfo_;
    info.capturerInfo = capturerInfo_;
    info.eStreamType = eStreamType_;
    info.state = state_;
    info.sessionId = sessionId_;

    info.clientPid = clientPid_;
    info.clientUid = clientUid_;

    info.volume = GetVolume();
    info.duckVolume = GetDuckVolume();
    info.effectMode = GetAudioEffectMode();
    info.renderMode = renderMode_;
    info.captureMode = captureMode_;
    info.renderRate = renderRate_;

    info.underFlowCount = GetUnderflowCount();
    info.overFlowCount = GetOverflowCount();

    info.silentModeAndMixWithOthers = silentModeAndMixWithOthers_;
    info.defaultOutputDevice = defaultOutputDevice_;

    {
        std::lock_guard<std::mutex> lock(setPreferredFrameSizeMutex_);
        info.userSettedPreferredFrameSize = userSettedPreferredFrameSize_;
    }

    {
        std::lock_guard<std::mutex> lock(lastCallStartByUserTidMutex_);
        info.lastCallStartByUserTid = lastCallStartByUserTid_;
    }

    if (spkProcClientCb_) {
        info.rendererWriteCallback = spkProcClientCb_->GetRendererWriteCallback();
    }
    if (micProcClientCb_) {
        info.capturerReadCallback = micProcClientCb_->GetCapturerReadCallback();
    }
    if (firstFrameWritingCb_) {
        info.rendererFirstFrameWritingCallback = firstFrameWritingCb_;
    }
}

void FastAudioStream::OnFirstFrameWriting()
{
    CHECK_AND_RETURN(firstFrameWritingCb_!= nullptr);
    uint64_t latency = 0;
    this->GetLatency(latency);
    firstFrameWritingCb_->OnFirstFrameWriting(latency);
}

void FastAudioStream::ResetFirstFrameState()
{
    if (spkProcClientCb_ != nullptr) {
        AUDIO_DEBUG_LOG("FastAudioStream::ResetFirstFrameState: reset the first frame state");
        spkProcClientCb_->ResetFirstFrameState();
    }
}

void FastAudioStream::SetAudioHapticsSyncId(const int32_t &audioHapticsSyncId)
{
    CHECK_AND_RETURN_LOG(processClient_ != nullptr, "Start failed, process is null.");
    processClient_->SetAudioHapticsSyncId(audioHapticsSyncId);
}

void FastAudioStreamRenderCallback::OnHandleData(size_t length)
{
    CHECK_AND_RETURN_LOG(rendererWriteCallback_!= nullptr, "OnHandleData failed: rendererWriteCallback_ is null.");
    if (!hasFirstFrameWrited_.load()) {
        AUDIO_DEBUG_LOG("OnHandleData: send the first frame writing event to audio haptic player");
        audioStreamImpl_.OnFirstFrameWriting();
        hasFirstFrameWrited_.store(true);
    }
    rendererWriteCallback_->OnWriteData(length);
}

void FastAudioStreamRenderCallback::ResetFirstFrameState()
{
    AUDIO_DEBUG_LOG("ResetFirstFrameState: set the hasFirstFrameWrited_ to false");
    hasFirstFrameWrited_.store(false);
}

std::shared_ptr<AudioRendererWriteCallback> FastAudioStreamRenderCallback::GetRendererWriteCallback() const
{
    return rendererWriteCallback_;
}

std::shared_ptr<AudioCapturerReadCallback> FastAudioStreamCaptureCallback::GetCapturerReadCallback() const
{
    return captureCallback_;
}

void FastAudioStreamCaptureCallback::OnHandleData(size_t length)
{
    CHECK_AND_RETURN_LOG(captureCallback_!= nullptr, "OnHandleData failed: captureCallback_ is null.");
    captureCallback_->OnReadData(length);
}

int32_t FastAudioStream::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    AUDIO_ERR_LOG("SetChannelBlendMode is not supported");
    return SUCCESS;
}

int32_t FastAudioStream::SetVolumeWithRamp(float volume, int32_t duration)
{
    AUDIO_ERR_LOG("SetVolumeWithRamp is not supported");
    return SUCCESS;
}

void FastAudioStream::UpdateRegisterTrackerInfo(AudioRegisterTrackerInfo &registerTrackerInfo)
{
    rendererInfo_.samplingRate = static_cast<AudioSamplingRate>(streamInfo_.samplingRate);
    capturerInfo_.samplingRate = static_cast<AudioSamplingRate>(streamInfo_.samplingRate);

    registerTrackerInfo.sessionId = sessionId_;
    registerTrackerInfo.clientPid = clientPid_;
    registerTrackerInfo.state = state_;
    registerTrackerInfo.rendererInfo = rendererInfo_;
    registerTrackerInfo.capturerInfo = capturerInfo_;
}

bool FastAudioStream::RestoreAudioStream(bool needStoreState)
{
    CHECK_AND_RETURN_RET_LOG(proxyObj_ != nullptr, false, "proxyObj_ is null");
    CHECK_AND_RETURN_RET_LOG(state_ != NEW && state_ != INVALID && state_ != RELEASED, true,
        "state_ is %{public}d, no need for restore", state_);
    bool result = false;
    State oldState = state_;
    state_ = NEW;
    SetStreamTrackerState(false);
    if (processClient_ != nullptr) {
        processClient_->Stop();
        processClient_->Release();
        processClient_ = nullptr;
    }
    if (SetAudioStreamInfo(streamInfo_, proxyObj_) != SUCCESS || SetCallbacksWhenRestore() != SUCCESS) {
        goto error;
    }

    SetDefaultOutputDevice(defaultOutputDevice_);

    switch (oldState) {
        case RUNNING:
            result = StartAudioStream();
            break;
        case PAUSED:
            result = StartAudioStream() && PauseAudioStream();
            break;
        case STOPPED:
            [[fallthrough]];
        case STOPPING:
            result = StartAudioStream() && StopAudioStream();
            break;
        default:
            break;
    }
    if (!result) {
        goto error;
    }
    return result;
error:
    AUDIO_ERR_LOG("RestoreAudioStream failed");
    state_ = oldState;
    return false;
}

void FastAudioStream::JoinCallbackLoop()
{
    if (processClient_ != nullptr) {
        processClient_->JoinCallbackLoop();
    } else {
        AUDIO_WARNING_LOG("processClient_ is nullptr!");
    }
}

bool FastAudioStream::GetOffloadEnable()
{
    AUDIO_WARNING_LOG("not supported in fast audio stream");
    return false;
}

bool FastAudioStream::GetSpatializationEnabled()
{
    AUDIO_WARNING_LOG("not supported in fast audio stream");
    return false;
}

bool FastAudioStream::GetHighResolutionEnabled()
{
    AUDIO_WARNING_LOG("not supported in fast audio stream");
    return false;
}

int32_t FastAudioStream::SetDefaultOutputDevice(const DeviceType defaultOutputDevice, bool skipForce)
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERR_OPERATION_FAILED, "set failed: null process");
    int32_t ret = processClient_->SetDefaultOutputDevice(defaultOutputDevice, skipForce);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetDefaultOutputDevice error.");
    defaultOutputDevice_ = defaultOutputDevice;
    return SUCCESS;
}

FastStatus FastAudioStream::GetFastStatus()
{
    return FASTSTATUS_FAST;
}

DeviceType FastAudioStream::GetDefaultOutputDevice()
{
    return defaultOutputDevice_;
}

// diffrence from GetAudioPosition only when set speed
int32_t FastAudioStream::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    return GetAudioTime(timestamp, base) ? SUCCESS : ERR_OPERATION_FAILED;
}

void FastAudioStream::SetSwitchingStatus(bool isSwitching)
{
    std::lock_guard lock(switchingMutex_);
    if (isSwitching) {
        switchingInfo_ = {true, state_};
    } else {
        switchingInfo_ = {false, INVALID};
    }
}

int32_t FastAudioStream::SetCallbacksWhenRestore()
{
    int32_t ret = SUCCESS;
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, ERROR_INVALID_PARAM, "processClient_ is null");
    if (eMode_ == AUDIO_MODE_PLAYBACK) {
        ret = processClient_->SaveDataCallback(spkProcClientCb_);
    } else if (eMode_ == AUDIO_MODE_RECORD) {
        ret = processClient_->SaveDataCallback(micProcClientCb_);
    }
    return ret;
}

void FastAudioStream::GetRestoreInfo(RestoreInfo &restoreInfo)
{
    processClient_->GetRestoreInfo(restoreInfo);
    return;
}

void FastAudioStream::SetRestoreInfo(RestoreInfo &restoreInfo)
{
    processClient_->SetRestoreInfo(restoreInfo);
    return;
}

RestoreStatus FastAudioStream::CheckRestoreStatus()
{
    if (spkProcClientCb_ == nullptr && micProcClientCb_ == nullptr) {
        AUDIO_INFO_LOG("Fast stream without callback, restore to normal");
        renderMode_ = RENDER_MODE_NORMAL;
        captureMode_ = CAPTURE_MODE_NORMAL;
        return NEED_RESTORE_TO_NORMAL;
    }
    return processClient_->CheckRestoreStatus();
}

RestoreStatus FastAudioStream::SetRestoreStatus(RestoreStatus restoreStatus)
{
    return processClient_->SetRestoreStatus(restoreStatus);
}

void FastAudioStream::FetchDeviceForSplitStream()
{
    AUDIO_WARNING_LOG("Fast stream does not support split stream");
    if (processClient_) {
        processClient_->SetRestoreStatus(NO_NEED_FOR_RESTORE);
    }
}

void FastAudioStream::SetCallStartByUserTid(pid_t tid)
{
    std::lock_guard lock(lastCallStartByUserTidMutex_);
    lastCallStartByUserTid_ = tid;
}

void FastAudioStream::SetCallbackLoopTid(int32_t tid)
{
    AUDIO_INFO_LOG("Callback loop tid: %{public}d", tid);
    callbackLoopTid_ = tid;
    callbackLoopTidCv_.notify_all();
}

int32_t FastAudioStream::GetCallbackLoopTid()
{
    std::unique_lock<std::mutex> waitLock(callbackLoopTidMutex_);
    bool stopWaiting = callbackLoopTidCv_.wait_for(waitLock, std::chrono::seconds(1), [this] {
        return callbackLoopTid_ != -1; // callbackLoopTid_ will change when got notified.
    });

    if (!stopWaiting) {
        AUDIO_WARNING_LOG("Wait timeout");
        callbackLoopTid_ = 0; // set tid to prevent get operation from getting stuck
    }
    return callbackLoopTid_;
}

void FastAudioStream::ResetCallbackLoopTid()
{
    AUDIO_INFO_LOG("to -1");
    callbackLoopTid_ = -1;
}

bool FastAudioStream::GetStopFlag() const
{
    CHECK_AND_RETURN_RET_LOG(processClient_ != nullptr, false, "processClient_ is null");
    return processClient_->GetStopFlag();
}
} // namespace AudioStandard
} // namespace OHOS
