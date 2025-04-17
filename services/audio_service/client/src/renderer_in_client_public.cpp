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
#define LOG_TAG "RendererInClientInnerPublic"
#endif

#include "renderer_in_client.h"
#include "renderer_in_client_private.h"

#include <atomic>
#include <cinttypes>
#include <condition_variable>
#include <sstream>
#include <string>
#include <mutex>
#include <thread>

#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "securec.h"
#include "hisysevent.h"

#include "audio_errors.h"
#include "audio_policy_manager.h"
#include "audio_manager_base.h"
#include "audio_renderer_log.h"
#include "audio_ring_cache.h"
#include "audio_channel_blend.h"
#include "audio_server_death_recipient.h"
#include "audio_stream_tracker.h"
#include "audio_system_manager.h"
#include "futex_tool.h"
#include "ipc_stream_listener_impl.h"
#include "ipc_stream_listener_stub.h"
#include "volume_ramp.h"
#include "callback_handler.h"
#include "audio_speed.h"
#include "audio_spatial_channel_converter.h"
#include "audio_policy_manager.h"
#include "audio_spatialization_manager.h"
#include "policy_handler.h"
#include "volume_tools.h"
#include "audio_manager_util.h"
#include "audio_effect_map.h"

#include "media_monitor_manager.h"

using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AudioStandard {
namespace {
const uint64_t OLD_BUF_DURATION_IN_USEC = 92880; // This value is used for compatibility purposes.
const uint64_t AUDIO_US_PER_MS = 1000;
const uint64_t AUDIO_NS_PER_US = 1000;
const uint64_t AUDIO_US_PER_S = 1000000;
const uint64_t AUDIO_MS_PER_S = 1000;
static constexpr int CB_QUEUE_CAPACITY = 3;
const uint64_t AUDIO_FIRST_FRAME_LATENCY = 120; //ms
static const int32_t CREATE_TIMEOUT_IN_SECOND = 9; // 9S
constexpr int32_t MAX_BUFFER_SIZE = 100000;
const uint64_t MAX_CBBUF_IN_USEC = 100000;
const uint64_t MIN_CBBUF_IN_USEC = 20000;
static const int32_t OPERATION_TIMEOUT_IN_MS = 1000; // 1000ms
static const int32_t SHORT_TIMEOUT_IN_MS = 20; // ms
static const int32_t DATA_CONNECTION_TIMEOUT_IN_MS = 300; // ms
} // namespace
std::shared_ptr<RendererInClient> RendererInClient::GetInstance(AudioStreamType eStreamType, int32_t appUid)
{
    return std::make_shared<RendererInClientInner>(eStreamType, appUid);
}

RendererInClientInner::RendererInClientInner(AudioStreamType eStreamType, int32_t appUid)
    : eStreamType_(eStreamType), appUid_(appUid), cbBufferQueue_(CB_QUEUE_CAPACITY)
{
    AUDIO_INFO_LOG("Create with StreamType:%{public}d appUid:%{public}d ", eStreamType_, appUid_);
    audioStreamTracker_ = std::make_unique<AudioStreamTracker>(AUDIO_MODE_PLAYBACK, appUid);
    state_ = NEW;
}

RendererInClientInner::~RendererInClientInner()
{
    AUDIO_INFO_LOG("~RendererInClientInner()");
    DumpFileUtil::CloseDumpFile(&dumpOutFd_);
    RendererInClientInner::ReleaseAudioStream(true);
    std::lock_guard<std::mutex> runnerlock(runnerMutex_);
    if (!runnerReleased_ && callbackHandler_ != nullptr) {
        AUDIO_INFO_LOG("runner remove");
        callbackHandler_->ReleaseEventRunner();
        runnerReleased_ = true;
        callbackHandler_ = nullptr;
    }
    UnregisterSpatializationStateEventListener(spatializationRegisteredSessionID_);
    AUDIO_INFO_LOG("[%{public}s] volume data counts: %{public}" PRId64, logUtilsTag_.c_str(), volumeDataCount_);
}

int32_t RendererInClientInner::OnOperationHandled(Operation operation, int64_t result)
{
    Trace trace(traceTag_ + " OnOperationHandled:" + std::to_string(operation));
    AUDIO_INFO_LOG("sessionId %{public}d recv operation:%{public}d result:%{public}" PRId64".", sessionId_, operation,
        result);
    if (operation == SET_OFFLOAD_ENABLE) {
        AUDIO_INFO_LOG("SET_OFFLOAD_ENABLE result:%{public}" PRId64".", result);
        if (!offloadEnable_ && static_cast<bool>(result)) {
            offloadStartReadPos_ = 0;
        }
        offloadEnable_ = static_cast<bool>(result);
        rendererInfo_.pipeType = offloadEnable_ ? PIPE_TYPE_OFFLOAD : PIPE_TYPE_NORMAL_OUT;
        return SUCCESS;
    } else if (operation == DATA_LINK_CONNECTING) {
        isDataLinkConnected_ = false;
        return SUCCESS;
    } else if (operation == DATA_LINK_CONNECTED) {
        isDataLinkConnected_ = true;
        dataConnectionCV_.notify_all();
        return SUCCESS;
    }

    if (operation == RESTORE_SESSION) {
        // fix it when restoreAudioStream work right
        if (audioStreamTracker_ && audioStreamTracker_.get()) {
            audioStreamTracker_->FetchOutputDeviceForTrack(sessionId_,
                state_, clientPid_, rendererInfo_, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
        }
        return SUCCESS;
    }

    std::unique_lock<std::mutex> lock(callServerMutex_);
    notifiedOperation_ = operation;
    notifiedResult_ = result;

    if (notifiedResult_ == SUCCESS) {
        HandleStatusChangeOperation(operation);
    } else {
        AUDIO_ERR_LOG("operation %{public}d failed, result: %{public}" PRId64 "", operation, result);
    }

    callServerCV_.notify_all();
    return SUCCESS;
}

void RendererInClientInner::HandleStatusChangeOperation(Operation operation)
{
    std::unique_lock<std::mutex> lock(streamCbMutex_);
    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    switch (operation) {
        case START_STREAM :
            state_ = RUNNING;
            break;
        case PAUSE_STREAM :
            state_ = PAUSED;
            break;
        case STOP_STREAM :
            state_ = STOPPED;
            break;
        default :
            break;
    }
    if (streamCb != nullptr) {
        streamCb->OnStateChange(state_, CMD_FROM_SYSTEM);
    }
}

void RendererInClientInner::SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId, uint64_t fullTokenId)
{
    AUDIO_INFO_LOG("PID:%{public}d UID:%{public}d.", clientPid, clientUid);
    clientPid_ = clientPid;
    clientUid_ = clientUid;
    appTokenId_ = appTokenId;
    fullTokenId_ = fullTokenId;
}

int32_t RendererInClientInner::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    AUDIO_ERR_LOG("Unsupported operation!");
    return ERR_NOT_SUPPORTED;
}

void RendererInClientInner::SetRendererInfo(const AudioRendererInfo &rendererInfo)
{
    rendererInfo_ = rendererInfo;

    rendererInfo_.sceneType = AudioManagerUtil::GetEffectSceneName(rendererInfo_.streamUsage);

    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();

    if (rendererInfo_.sceneType == audioSupportedSceneTypes.find(SCENE_OTHERS)->second) {
        effectMode_ = EFFECT_NONE;
        rendererInfo_.effectMode = EFFECT_NONE;
    }

    AUDIO_PRERELEASE_LOGI("SetRendererInfo with flag %{public}d, sceneType %{public}s", rendererInfo_.rendererFlags,
        rendererInfo_.sceneType.c_str());
    AudioSpatializationState spatializationState =
        AudioPolicyManager::GetInstance().GetSpatializationState(rendererInfo_.streamUsage);
    rendererInfo_.spatializationEnabled = spatializationState.spatializationEnabled;
    rendererInfo_.headTrackingEnabled = spatializationState.headTrackingEnabled;
    rendererInfo_.encodingType = curStreamParams_.encoding;
    rendererInfo_.channelLayout = curStreamParams_.channelLayout;
    UpdateTracker("UPDATE");
}

void RendererInClientInner::SetCapturerInfo(const AudioCapturerInfo &capturerInfo)
{
    AUDIO_WARNING_LOG("SetCapturerInfo is not supported");
    return;
}

int32_t RendererInClientInner::SetAudioStreamInfo(const AudioStreamParams info,
    const std::shared_ptr<AudioClientTracker> &proxyObj,
    const AudioPlaybackCaptureConfig &config)
{
    // In plan: If paramsIsSet_ is true, and new info is same as old info, return
    AUDIO_INFO_LOG("AudioStreamInfo, Sampling rate: %{public}d, channels: %{public}d, format: %{public}d,"
        " stream type: %{public}d, encoding type: %{public}d", info.samplingRate, info.channels, info.format,
        eStreamType_, info.encoding);

    AudioXCollie guard("RendererInClientInner::SetAudioStreamInfo", CREATE_TIMEOUT_IN_SECOND);
    if (!IsFormatValid(info.format) || !IsSamplingRateValid(info.samplingRate) || !IsEncodingTypeValid(info.encoding)) {
        AUDIO_ERR_LOG("Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }

    streamParams_ = curStreamParams_ = info; // keep it for later use
    if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
        ConverterConfig cfg = AudioPolicyManager::GetInstance().GetConverterConfig();
        converter_ = std::make_unique<AudioSpatialChannelConverter>();
        if (converter_ == nullptr || !converter_->Init(curStreamParams_, cfg) || !converter_->AllocateMem()) {
            AUDIO_ERR_LOG("AudioStream: converter construct error");
            return ERR_NOT_SUPPORTED;
        }
        converter_->ConverterChannels(curStreamParams_.channels, curStreamParams_.channelLayout);
    }

    if (!IsPlaybackChannelRelatedInfoValid(curStreamParams_.channels, curStreamParams_.channelLayout)) {
        return ERR_NOT_SUPPORTED;
    }

    CHECK_AND_RETURN_RET_LOG(IAudioStream::GetByteSizePerFrame(curStreamParams_, sizePerFrameInByte_) == SUCCESS,
        ERROR_INVALID_PARAM, "GetByteSizePerFrame failed with invalid params");

    if (state_ != NEW) {
        AUDIO_ERR_LOG("State is not new, release existing stream and recreate, state %{public}d", state_.load());
        int32_t ret = DeinitIpcStream();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "release existing stream failed.");
    }
    paramsIsSet_ = true;
    int32_t initRet = InitIpcStream();
    CHECK_AND_RETURN_RET_LOG(initRet == SUCCESS, initRet, "Init stream failed: %{public}d", initRet);
    state_ = PREPARED;

    // eg: 100005_44100_2_1_client_out.pcm
    dumpOutFile_ = std::to_string(sessionId_) + "_" + std::to_string(curStreamParams_.samplingRate) + "_" +
        std::to_string(curStreamParams_.channels) + "_" + std::to_string(curStreamParams_.format) + "_client_out.pcm";

    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, dumpOutFile_, &dumpOutFd_);
    logUtilsTag_ = "[" + std::to_string(sessionId_) + "]NormalRenderer";
    InitDirectPipeType();

    proxyObj_ = proxyObj;
    RegisterTracker(proxyObj);
    RegisterSpatializationStateEventListener();
    return SUCCESS;
}

int32_t RendererInClientInner::GetAudioStreamInfo(AudioStreamParams &info)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, ERR_OPERATION_FAILED, "Params is not set");
    info = streamParams_;
    return SUCCESS;
}

int32_t RendererInClientInner::GetAudioSessionID(uint32_t &sessionID)
{
    CHECK_AND_RETURN_RET_LOG((state_ != RELEASED) && (state_ != NEW), ERR_ILLEGAL_STATE,
        "State error %{public}d", state_.load());
    sessionID = sessionId_;
    return SUCCESS;
}

void RendererInClientInner::GetAudioPipeType(AudioPipeType &pipeType)
{
    pipeType = rendererInfo_.pipeType;
}

State RendererInClientInner::GetState()
{
    std::lock_guard lock(switchingMutex_);
    if (switchingInfo_.isSwitching_) {
        AUDIO_INFO_LOG("switching, return state in switchingInfo");
        return switchingInfo_.state_;
    }
    return state_;
}

bool RendererInClientInner::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(paramsIsSet_ == true, false, "Params is not set");
    CHECK_AND_RETURN_RET_LOG(state_ != STOPPED, false, "Invalid status:%{public}d", state_.load());

    uint64_t readPos = 0;
    int64_t handleTime = 0;
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, false, "invalid buffer status");
    clientBuffer_->GetHandleInfo(readPos, handleTime);
    if (readPos == 0 || handleTime == 0) {
        AUDIO_WARNING_LOG("GetHandleInfo may failed");
    }

    timestamp.framePosition = readPos;
    int64_t audioTimeResult = handleTime;

    if (offloadEnable_) {
        uint64_t timestampHdi = 0;
        uint64_t paWriteIndex = 0;
        uint64_t cacheTimeDsp = 0;
        uint64_t cacheTimePa = 0;
        ipcStream_->GetOffloadApproximatelyCacheTime(timestampHdi, paWriteIndex, cacheTimeDsp, cacheTimePa);
        int64_t cacheTime = static_cast<int64_t>(cacheTimeDsp + cacheTimePa) * AUDIO_NS_PER_US;
        int64_t timeNow = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        int64_t deltaTimeStamp = (static_cast<int64_t>(timeNow) - static_cast<int64_t>(timestampHdi)) * AUDIO_NS_PER_US;
        uint64_t paWriteIndexNs = paWriteIndex * AUDIO_NS_PER_US;
        uint64_t readPosNs = readPos * AUDIO_MS_PER_SECOND / curStreamParams_.samplingRate * AUDIO_US_PER_S;

        int64_t deltaPaWriteIndexNs = static_cast<int64_t>(readPosNs) - static_cast<int64_t>(paWriteIndexNs);
        int64_t cacheTimeNow = cacheTime - deltaTimeStamp + deltaPaWriteIndexNs;
        if (offloadStartReadPos_ == 0) {
            offloadStartReadPos_ = readPosNs;
            offloadStartHandleTime_ = handleTime;
        }
        int64_t offloadDelta = 0;
        if (offloadStartReadPos_ != 0) {
            offloadDelta = (static_cast<int64_t>(readPosNs) - static_cast<int64_t>(offloadStartReadPos_)) -
                           (handleTime - offloadStartHandleTime_) - cacheTimeNow;
        }
        audioTimeResult += offloadDelta;
    }

    timestamp.time.tv_sec = static_cast<time_t>(audioTimeResult / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(audioTimeResult % AUDIO_NS_PER_SECOND);
    AUDIO_DEBUG_LOG("audioTimeResult: %{public}" PRIi64, audioTimeResult);
    return true;
}

bool RendererInClientInner::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, false, "Renderer stream state is not RUNNING");
    uint64_t readIdx = 0;
    uint64_t timestampVal = 0;
    uint64_t latency = 0;
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->GetAudioPosition(readIdx, timestampVal, latency);

    uint64_t framePosition = readIdx > lastFlushReadIndex_ ? readIdx - lastFlushReadIndex_ : 0;
    framePosition = framePosition > latency ? framePosition - latency : 0;

    // add MCR latency
    uint32_t mcrLatency = 0;
    if (converter_ != nullptr) {
        mcrLatency = converter_->GetLatency() * curStreamParams_.samplingRate / AUDIO_MS_PER_S;
        framePosition = framePosition > mcrLatency ? framePosition - mcrLatency : 0;
    }

    if (lastFramePosition_ < framePosition) {
        lastFramePosition_ = framePosition;
        lastFrameTimestamp_ = timestampVal;
    } else {
        AUDIO_DEBUG_LOG("The frame position should be continuously increasing");
        framePosition = lastFramePosition_;
        timestampVal = lastFrameTimestamp_;
    }
    AUDIO_DEBUG_LOG("[CLIENT]Latency info: framePosition: %{public}" PRIu64 ", lastFlushReadIndex_ %{public}" PRIu64
        ", timestamp %{public}" PRIu64 ", mcrLatency %{public}u, Sinklatency %{public}" PRIu64, framePosition,
        lastFlushReadIndex_, timestampVal, mcrLatency, latency);

    timestamp.framePosition = framePosition;
    timestamp.time.tv_sec = static_cast<time_t>(timestampVal / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(timestampVal % AUDIO_NS_PER_SECOND);
    return ret == SUCCESS;
}

int32_t RendererInClientInner::GetBufferSize(size_t &bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Renderer stream is released");
    bufferSize = clientSpanSizeInByte_;
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        bufferSize = cbBufferSize_;
    }

    if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
        CHECK_AND_RETURN_RET(converter_ != nullptr && converter_->GetInputBufferSize(bufferSize), ERR_OPERATION_FAILED);
    }

    AUDIO_INFO_LOG("Buffer size is %{public}zu, mode is %{public}s", bufferSize, renderMode_ == RENDER_MODE_NORMAL ?
        "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    return SUCCESS;
}

int32_t RendererInClientInner::GetFrameCount(uint32_t &frameCount)
{
    CHECK_AND_RETURN_RET_LOG(state_ != RELEASED, ERR_ILLEGAL_STATE, "Renderer stream is released");
    CHECK_AND_RETURN_RET_LOG(sizePerFrameInByte_ != 0, ERR_ILLEGAL_STATE, "sizePerFrameInByte_ is 0!");
    frameCount = spanSizeInFrame_;
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        frameCount = cbBufferSize_ / sizePerFrameInByte_;
        if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
            frameCount = frameCount * curStreamParams_.channels / streamParams_.channels;
        }
    }
    AUDIO_INFO_LOG("Frame count is %{public}u, mode is %{public}s", frameCount, renderMode_ == RENDER_MODE_NORMAL ?
        "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    return SUCCESS;
}

int32_t RendererInClientInner::GetLatency(uint64_t &latency)
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    return ipcStream_->GetLatency(latency);
}

int32_t RendererInClientInner::SetAudioStreamType(AudioStreamType audioStreamType)
{
    AUDIO_ERR_LOG("Change stream type %{public}d to %{public}d is not supported", eStreamType_, audioStreamType);
    return SUCCESS;
}

int32_t RendererInClientInner::SetVolume(float volume)
{
    Trace trace("RendererInClientInner::SetVolume:" + std::to_string(volume));
    AUDIO_INFO_LOG("[%{public}s]sessionId:%{public}d volume:%{public}f", (offloadEnable_ ? "offload" : "normal"),
        sessionId_, volume);
    if (volume < 0.0 || volume > 1.0) {
        AUDIO_ERR_LOG("SetVolume with invalid volume %{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    if (volumeRamp_.IsActive()) {
        volumeRamp_.Terminate();
    }
    clientVolume_ = volume;

    return SetInnerVolume(volume);
}

float RendererInClientInner::GetVolume()
{
    Trace trace("RendererInClientInner::GetVolume:" + std::to_string(clientVolume_));
    return clientVolume_;
}

int32_t RendererInClientInner::SetDuckVolume(float volume)
{
    Trace trace("RendererInClientInner::SetDuckVolume:" + std::to_string(volume));
    AUDIO_INFO_LOG("sessionId:%{public}d SetDuck:%{public}f", sessionId_, volume);
    if (volume < 0.0 || volume > 1.0) {
        AUDIO_ERR_LOG("SetDuckVolume with invalid volume %{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    duckVolume_ = volume;
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, ERR_OPERATION_FAILED, "buffer is not inited");
    clientBuffer_->SetDuckFactor(volume);
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetDuckFactor(volume);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Set Duck failed:%{public}u", ret);
        return ERROR;
    }
    return SUCCESS;
}

int32_t RendererInClientInner::SetMute(bool mute)
{
    Trace trace("RendererInClientInner::SetMute:" + std::to_string(mute));
    AUDIO_INFO_LOG("sessionId:%{public}d SetMute:%{public}d", sessionId_, mute);
    muteVolume_ = mute ? 0.0f : 1.0f;
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, ERR_OPERATION_FAILED, "buffer is not inited");
    clientBuffer_->SetMuteFactor(muteVolume_);
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetMute(mute);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Set Mute failed:%{public}u", ret);
        return ERROR;
    }
    return SUCCESS;
}

int32_t RendererInClientInner::SetRenderRate(AudioRendererRate renderRate)
{
    if (rendererRate_ == renderRate) {
        AUDIO_INFO_LOG("Set same rate");
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is not inited!");
    rendererRate_ = renderRate;
    return ipcStream_->SetRate(renderRate);
}

AudioRendererRate RendererInClientInner::GetRenderRate()
{
    AUDIO_INFO_LOG("Get RenderRate %{public}d", rendererRate_);
    return rendererRate_;
}

int32_t RendererInClientInner::SetStreamCallback(const std::shared_ptr<AudioStreamCallback> &callback)
{
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

int32_t RendererInClientInner::SetRendererFirstFrameWritingCallback(
    const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback)
{
    AUDIO_INFO_LOG("SetRendererFirstFrameWritingCallback in.");
    CHECK_AND_RETURN_RET_LOG(callback, ERR_INVALID_PARAM, "callback is nullptr");
    std::lock_guard lock(firstFrameWritingMutex_);
    firstFrameWritingCb_ = callback;
    return SUCCESS;
}

void RendererInClientInner::OnFirstFrameWriting()
{
    AUDIO_DEBUG_LOG("In");
    uint64_t latency = AUDIO_FIRST_FRAME_LATENCY;

    std::shared_ptr<AudioRendererFirstFrameWritingCallback> cb = nullptr;
    {
        std::lock_guard lock(firstFrameWritingMutex_);
        CHECK_AND_RETURN_LOG(firstFrameWritingCb_!= nullptr, "firstFrameWritingCb_ is null.");
        cb = firstFrameWritingCb_;
    }
    AUDIO_DEBUG_LOG("OnFirstFrameWriting: latency %{public}" PRIu64 "", latency);
    cb->OnFirstFrameWriting(latency);
}

int32_t RendererInClientInner::SetSpeed(float speed)
{
    if (audioSpeed_ == nullptr) {
        audioSpeed_ = std::make_unique<AudioSpeed>(curStreamParams_.samplingRate, curStreamParams_.format,
            curStreamParams_.channels);
        GetBufferSize(bufferSize_);
        speedBuffer_ = std::make_unique<uint8_t[]>(MAX_BUFFER_SIZE);
    }
    audioSpeed_->SetSpeed(speed);
    speed_ = speed;
    AUDIO_DEBUG_LOG("SetSpeed %{public}f, OffloadEnable %{public}d", speed_, offloadEnable_);
    return SUCCESS;
}

float RendererInClientInner::GetSpeed()
{
    return speed_;
}

int32_t RendererInClientInner::ChangeSpeed(uint8_t *buffer, int32_t bufferSize, std::unique_ptr<uint8_t []> &outBuffer,
    int32_t &outBufferSize)
{
    return audioSpeed_->ChangeSpeedFunc(buffer, bufferSize, outBuffer, outBufferSize);
}

void RendererInClientInner::InitCallbackLoop()
{
    cbThreadReleased_ = false;
    auto weakRef = weak_from_this();
    // OS_AudioWriteCB
    std::thread callbackLoop = std::thread([weakRef] {
        bool keepRunning = true;
        std::shared_ptr<RendererInClientInner> strongRef = weakRef.lock();
        if (strongRef != nullptr) {
            strongRef->cbThreadCv_.notify_one();
            AUDIO_INFO_LOG("WriteCallbackFunc start, sessionID :%{public}d", strongRef->sessionId_);
        } else {
            AUDIO_WARNING_LOG("Strong ref is nullptr, could cause error");
        }
        strongRef = nullptr;
        // start loop
        while (keepRunning) {
            strongRef = weakRef.lock();
            if (strongRef == nullptr) {
                AUDIO_INFO_LOG("RendererInClientInner destroyed");
                break;
            }
            keepRunning = strongRef->WriteCallbackFunc(); // Main operation in callback loop
        }
        if (strongRef != nullptr) {
            AUDIO_INFO_LOG("CBThread end sessionID :%{public}d", strongRef->sessionId_);
        }
    });
    pthread_setname_np(callbackLoop.native_handle(), "OS_AudioWriteCB");
    callbackLoop.detach();
}

int32_t RendererInClientInner::SetRenderMode(AudioRenderMode renderMode)
{
    AUDIO_INFO_LOG("SetRenderMode to %{public}s", renderMode == RENDER_MODE_NORMAL ? "RENDER_MODE_NORMAL" :
        "RENDER_MODE_CALLBACK");
    if (renderMode_ == renderMode) {
        return SUCCESS;
    }

    // renderMode_ is inited as RENDER_MODE_NORMAL, can only be set to RENDER_MODE_CALLBACK.
    if (renderMode_ == RENDER_MODE_CALLBACK && renderMode == RENDER_MODE_NORMAL) {
        AUDIO_ERR_LOG("SetRenderMode from callback to normal is not supported.");
        return ERR_INCORRECT_MODE;
    }

    // state check
    if (state_ != PREPARED && state_ != NEW) {
        AUDIO_ERR_LOG("SetRenderMode failed. invalid state:%{public}d", state_.load());
        return ERR_ILLEGAL_STATE;
    }
    renderMode_ = renderMode;

    // init callbackLoop_
    InitCallbackLoop();

    std::unique_lock<std::mutex> threadStartlock(statusMutex_);
    bool stopWaiting = cbThreadCv_.wait_for(threadStartlock, std::chrono::milliseconds(SHORT_TIMEOUT_IN_MS), [this] {
        return cbThreadReleased_ == false; // When thread is started, cbThreadReleased_ will be false. So stop waiting.
    });
    if (!stopWaiting) {
        AUDIO_WARNING_LOG("Init OS_AudioWriteCB thread time out");
    }

    InitCallbackBuffer(OLD_BUF_DURATION_IN_USEC);
    return SUCCESS;
}

AudioRenderMode RendererInClientInner::GetRenderMode()
{
    AUDIO_INFO_LOG("Render mode is %{public}s", renderMode_ == RENDER_MODE_NORMAL ? "RENDER_MODE_NORMAL" :
        "RENDER_MODE_CALLBACK");
    return renderMode_;
}

int32_t RendererInClientInner::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Invalid null callback");
    CHECK_AND_RETURN_RET_LOG(renderMode_ == RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE, "incorrect render mode");
    std::lock_guard<std::mutex> lock(writeCbMutex_);
    writeCb_ = callback;
    return SUCCESS;
}

int32_t RendererInClientInner::SetCaptureMode(AudioCaptureMode captureMode)
{
    AUDIO_ERR_LOG("SetCaptureMode is not supported");
    return ERROR;
}

AudioCaptureMode RendererInClientInner::GetCaptureMode()
{
    AUDIO_ERR_LOG("GetCaptureMode is not supported");
    return CAPTURE_MODE_NORMAL; // not supported
}

int32_t RendererInClientInner::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    AUDIO_ERR_LOG("SetCapturerReadCallback is not supported");
    return ERROR;
}

int32_t RendererInClientInner::GetBufferDesc(BufferDesc &bufDesc)
{
    Trace trace("RendererInClientInner::GetBufferDesc");
    if (renderMode_ != RENDER_MODE_CALLBACK) {
        AUDIO_ERR_LOG("GetBufferDesc is not supported. Render mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    bufDesc.buffer = cbBuffer_.get();
    bufDesc.bufLength = cbBufferSize_;
    bufDesc.dataLength = cbBufferSize_;
    if (curStreamParams_.encoding == ENCODING_AUDIOVIVID) {
        CHECK_AND_RETURN_RET_LOG(converter_ != nullptr, ERR_INVALID_OPERATION, "converter is not inited");
        bufDesc.metaBuffer = bufDesc.buffer + cbBufferSize_;
        bufDesc.metaLength = converter_->GetMetaSize();
    }
    return SUCCESS;
}

int32_t RendererInClientInner::GetBufQueueState(BufferQueueState &bufState)
{
    Trace trace("RendererInClientInner::GetBufQueueState");
    if (renderMode_ != RENDER_MODE_CALLBACK) {
        AUDIO_ERR_LOG("GetBufQueueState is not supported. Render mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    // only one buffer in queue.
    bufState.numBuffers = 1;
    bufState.currentIndex = 0;
    return SUCCESS;
}

int32_t RendererInClientInner::Enqueue(const BufferDesc &bufDesc)
{
    Trace trace("RendererInClientInner::Enqueue " + std::to_string(bufDesc.bufLength));
    if (renderMode_ != RENDER_MODE_CALLBACK) {
        AUDIO_ERR_LOG("Enqueue is not supported. Render mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    CHECK_AND_RETURN_RET_LOG(bufDesc.buffer != nullptr && bufDesc.bufLength != 0, ERR_INVALID_PARAM, "Invalid buffer");
    CHECK_AND_RETURN_RET_LOG(curStreamParams_.encoding != ENCODING_AUDIOVIVID ||
            converter_ != nullptr && converter_->CheckInputValid(bufDesc),
        ERR_INVALID_PARAM, "Invalid buffer desc");
    if (bufDesc.bufLength > cbBufferSize_ || bufDesc.dataLength > cbBufferSize_) {
        AUDIO_WARNING_LOG("Invalid bufLength:%{public}zu or dataLength:%{public}zu, should be %{public}zu",
            bufDesc.bufLength, bufDesc.dataLength, cbBufferSize_);
    }

    BufferDesc temp = bufDesc;

    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("Invalid state: %{public}d", state_.load());
        return ERR_ILLEGAL_STATE;
    }
    // Call write here may block, so put it in loop callbackLoop_
    cbBufferQueue_.Push(temp);
    return SUCCESS;
}

int32_t RendererInClientInner::Clear()
{
    Trace trace("RendererInClientInner::Clear");
    if (renderMode_ != RENDER_MODE_CALLBACK) {
        AUDIO_ERR_LOG("Clear is not supported. Render mode is not callback.");
        return ERR_INCORRECT_MODE;
    }
    std::unique_lock<std::mutex> lock(cbBufferMutex_);
    int32_t ret = memset_s(cbBuffer_.get(), cbBufferSize_, 0, cbBufferSize_);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ERR_OPERATION_FAILED, "Clear buffer fail, ret %{public}d.", ret);
    lock.unlock();
    FlushAudioStream();
    return SUCCESS;
}

int32_t RendererInClientInner::SetLowPowerVolume(float volume)
{
    AUDIO_INFO_LOG("Volume number: %{public}f", volume);
    if (volume < 0.0 || volume > 1.0) {
        AUDIO_ERR_LOG("Invalid param: %{public}f", volume);
        return ERR_INVALID_PARAM;
    }
    lowPowerVolume_ = volume;

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is null!");
    return ipcStream_->SetLowPowerVolume(lowPowerVolume_);
}

float RendererInClientInner::GetLowPowerVolume()
{
    return lowPowerVolume_;
}

int32_t RendererInClientInner::SetOffloadMode(int32_t state, bool isAppBack)
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is null!");
    return ipcStream_->SetOffloadMode(state, isAppBack);
}

int32_t RendererInClientInner::UnsetOffloadMode()
{
    rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is null!");
    return ipcStream_->UnsetOffloadMode();
}

float RendererInClientInner::GetSingleStreamVolume()
{
    // in plan. For now, keep it consistent with fast_audio_stream
    return 1.0f;
}

AudioEffectMode RendererInClientInner::GetAudioEffectMode()
{
    AUDIO_DEBUG_LOG("Current audio effect mode is %{public}d", effectMode_);
    return effectMode_;
}

int32_t RendererInClientInner::SetAudioEffectMode(AudioEffectMode effectMode)
{
    if (effectMode_ == effectMode) {
        AUDIO_INFO_LOG("Set same effect mode");
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetAudioEffectMode(effectMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED, "Set audio effect mode failed");
    effectMode_ = effectMode;
    return SUCCESS;
}

int64_t RendererInClientInner::GetFramesWritten()
{
    return totalBytesWritten_ / static_cast<int64_t>(sizePerFrameInByte_);
}

int64_t RendererInClientInner::GetFramesRead()
{
    AUDIO_ERR_LOG("not supported");
    return -1;
}

void RendererInClientInner::SetInnerCapturerState(bool isInnerCapturer)
{
    AUDIO_ERR_LOG("SetInnerCapturerState is not supported");
    return;
}

void RendererInClientInner::SetWakeupCapturerState(bool isWakeupCapturer)
{
    AUDIO_ERR_LOG("SetWakeupCapturerState is not supported");
    return;
}

void RendererInClientInner::SetCapturerSource(int capturerSource)
{
    AUDIO_ERR_LOG("SetCapturerSource is not supported");
    return;
}

void RendererInClientInner::SetPrivacyType(AudioPrivacyType privacyType)
{
    if (privacyType_ == privacyType) {
        AUDIO_INFO_LOG("Set same privacy type");
        return;
    }
    privacyType_ = privacyType;
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetPrivacyType(privacyType);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Set privacy type failed");
}

bool RendererInClientInner::StartAudioStream(StateChangeCmdType cmdType,
    AudioStreamDeviceChangeReasonExt reason)
{
    Trace trace("RendererInClientInner::StartAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ != PREPARED && state_ != STOPPED && state_ != PAUSED) {
        AUDIO_ERR_LOG("Start failed Illegal state:%{public}d", state_.load());
        return false;
    }

    hasFirstFrameWrited_ = false;
    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        audioStreamTracker_->FetchOutputDeviceForTrack(sessionId_, RUNNING, clientPid_, rendererInfo_, reason);
    }
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Start();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Start call server failed:%{public}u", ret);
        return false;
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return state_ == RUNNING; // will be false when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Start failed: timeout");
        ipcStream_->Stop();
        return false;
    }

    waitLock.unlock();

    AUDIO_INFO_LOG("Start SUCCESS, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);
    UpdateTracker("RUNNING");

    std::unique_lock<std::mutex> dataConnectionWaitLock(dataConnectionMutex_);
    if (!isDataLinkConnected_) {
        AUDIO_INFO_LOG("data-connection blocking starts.");
        stopWaiting = dataConnectionCV_.wait_for(
            dataConnectionWaitLock, std::chrono::milliseconds(DATA_CONNECTION_TIMEOUT_IN_MS), [this] {
                return isDataLinkConnected_;
            });
        AUDIO_INFO_LOG("data-connection blocking ends.");
    }
    dataConnectionWaitLock.unlock();

    offloadStartReadPos_ = 0;
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        // start the callback-write thread
        cbThreadCv_.notify_all();
    }
    statusLock.unlock();
    // in plan: call HiSysEventWrite
    int64_t param = -1;
    StateCmdTypeToParams(param, state_, cmdType);
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, param);
    preWriteEndTime_ = 0;
    return true;
}

bool RendererInClientInner::PauseAudioStream(StateChangeCmdType cmdType)
{
    Trace trace("RendererInClientInner::PauseAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ != RUNNING) {
        AUDIO_ERR_LOG("State is not RUNNING. Illegal state:%{public}u", state_.load());
        return false;
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Pause();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("call server failed:%{public}u", ret);
        return false;
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return state_ == PAUSED; // will be false when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Pause failed: timeout");
        return false;
    }

    waitLock.unlock();

    FutexTool::FutexWake(clientBuffer_->GetFutex());
    statusLock.unlock();

    // in plan: call HiSysEventWrite
    int64_t param = -1;
    StateCmdTypeToParams(param, state_, cmdType);
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, param);

    AUDIO_INFO_LOG("Pause SUCCESS, sessionId %{public}d, uid %{public}d, mode %{public}s", sessionId_,
        clientUid_, renderMode_ == RENDER_MODE_NORMAL ? "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    UpdateTracker("PAUSED");
    return true;
}

bool RendererInClientInner::StopAudioStream()
{
    Trace trace("RendererInClientInner::StopAudioStream " + std::to_string(sessionId_));
    AUDIO_INFO_LOG("Stop begin for sessionId %{public}d uid: %{public}d", sessionId_, clientUid_);
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    std::unique_lock<std::mutex> lock(writeMutex_, std::defer_lock);
    if (!offloadEnable_) {
        lock.lock();
        DrainAudioStreamInner(true);
    }

    if (state_ == STOPPED) {
        AUDIO_INFO_LOG("Renderer in client is already stopped");
        return true;
    }
    if ((state_ != RUNNING) && (state_ != PAUSED)) {
        AUDIO_ERR_LOG("Stop failed. Illegal state:%{public}u", state_.load());
        return false;
    }

    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        state_ = STOPPING;
        AUDIO_INFO_LOG("Stop begin in callback mode sessionId %{public}d uid: %{public}d", sessionId_, clientUid_);
    }

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Stop();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Stop call server failed:%{public}u", ret);
        return false;
    }

    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return state_ == STOPPED; // will be false when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("Stop failed: timeout");
        state_ = INVALID;
        return false;
    }

    waitLock.unlock();

    FutexTool::FutexWake(clientBuffer_->GetFutex());
    statusLock.unlock();

    // in plan: call HiSysEventWrite
    SafeSendCallbackEvent(STATE_CHANGE_EVENT, state_);

    AUDIO_INFO_LOG("Stop SUCCESS, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);
    UpdateTracker("STOPPED");
    return true;
}

bool RendererInClientInner::ReleaseAudioStream(bool releaseRunner, bool isSwitchStream)
{
    (void)isSwitchStream;
    AUDIO_PRERELEASE_LOGI("Enter");
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("Already released, do nothing");
        return true;
    }
    state_ = RELEASED;
    statusLock.unlock();

    Trace trace("RendererInClientInner::ReleaseAudioStream " + std::to_string(sessionId_));
    if (ipcStream_ != nullptr) {
        ipcStream_->Release();
    } else {
        AUDIO_WARNING_LOG("release while ipcStream is null");
    }

    // no lock, call release in any case, include blocked case.
    std::unique_lock<std::mutex> runnerlock(runnerMutex_);
    if (releaseRunner && callbackHandler_ != nullptr) {
        callbackHandler_->ReleaseEventRunner();
        runnerReleased_ = true;
        callbackHandler_ = nullptr;
    }
    runnerlock.unlock();

    // clear write callback
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        cbThreadReleased_ = true; // stop loop
        cbThreadCv_.notify_all();
        FutexTool::FutexWake(clientBuffer_->GetFutex(), IS_PRE_EXIT);
    }
    paramsIsSet_ = false;

    std::unique_lock<std::mutex> lock(streamCbMutex_);
    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    if (streamCb != nullptr) {
        AUDIO_INFO_LOG("Notify client the state is released");
        streamCb->OnStateChange(RELEASED, CMD_FROM_CLIENT);
    }
    lock.unlock();

    UpdateTracker("RELEASED");
    AUDIO_INFO_LOG("Release end, sessionId: %{public}d, uid: %{public}d", sessionId_, clientUid_);

    audioSpeed_.reset();
    audioSpeed_ = nullptr;
    return true;
}

bool RendererInClientInner::FlushAudioStream()
{
    Trace trace("RendererInClientInner::FlushAudioStream " + std::to_string(sessionId_));
    std::unique_lock<std::mutex> statusLock(statusMutex_);
    std::lock_guard<std::mutex>lock(writeMutex_);
    if ((state_ != RUNNING) && (state_ != PAUSED) && (state_ != STOPPED)) {
        AUDIO_ERR_LOG("Flush failed. Illegal state:%{public}u", state_.load());
        return false;
    }

    // clear cbBufferQueue
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        cbBufferQueue_.Clear();
        int chToFill = (clientConfig_.streamInfo.format == SAMPLE_U8) ? 0x7f : 0;
        if (memset_s(cbBuffer_.get(), cbBufferSize_, chToFill, cbBufferSize_) != EOK) {
            AUDIO_ERR_LOG("memset_s buffer failed");
        }
    }

    CHECK_AND_RETURN_RET_LOG(FlushRingCache() == SUCCESS, false, "Flush cache failed");

    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, false, "ipcStream is not inited!");
    int32_t ret = ipcStream_->Flush();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Flush call server failed:%{public}u", ret);
        return false;
    }

    // clear multichannel render buffer
    if (converter_) {
        ret = converter_->Flush();
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("Flush mcr buffer failed.");
        }
    }
    std::unique_lock<std::mutex> waitLock(callServerMutex_);
    bool stopWaiting = callServerCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return notifiedOperation_ == FLUSH_STREAM; // will be false when got notified.
    });

    if (notifiedOperation_ != FLUSH_STREAM || notifiedResult_ != SUCCESS) {
        AUDIO_ERR_LOG("Flush failed: %{public}s Operation:%{public}d result:%{public}" PRId64".",
            (!stopWaiting ? "timeout" : "no timeout"), notifiedOperation_, notifiedResult_);
        notifiedOperation_ = MAX_OPERATION_CODE;
        return false;
    }
    notifiedOperation_ = MAX_OPERATION_CODE;
    waitLock.unlock();
    ResetFramePosition();
    AUDIO_INFO_LOG("Flush stream SUCCESS, sessionId: %{public}d", sessionId_);
    return true;
}

bool RendererInClientInner::DrainAudioStream(bool stopFlag)
{
    std::lock_guard<std::mutex> statusLock(statusMutex_);
    std::lock_guard<std::mutex> lock(writeMutex_);
    bool ret = DrainAudioStreamInner(stopFlag);
    return ret;
}

int32_t RendererInClientInner::Write(uint8_t *pcmBuffer, size_t pcmBufferSize, uint8_t *metaBuffer,
    size_t metaBufferSize)
{
    CHECK_AND_RETURN_RET_LOG(renderMode_ != RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "Write with callback is not supported");
    int32_t ret = WriteInner(pcmBuffer, pcmBufferSize, metaBuffer, metaBufferSize);
    return ret <= 0 ? ret : static_cast<int32_t>(pcmBufferSize);
}

int32_t RendererInClientInner::Write(uint8_t *buffer, size_t bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(renderMode_ != RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE,
        "Write with callback is not supported");
    return WriteInner(buffer, bufferSize);
}

void RendererInClientInner::SetPreferredFrameSize(int32_t frameSize)
{
    std::lock_guard<std::mutex> lockSetPreferredFrameSize(setPreferredFrameSizeMutex_);
    userSettedPreferredFrameSize_ = frameSize;
    CHECK_AND_RETURN_LOG(curStreamParams_.encoding != ENCODING_AUDIOVIVID,
        "playing audiovivid, frameSize is always 1024.");
    size_t maxCbBufferSize =
        static_cast<size_t>(MAX_CBBUF_IN_USEC * curStreamParams_.samplingRate / AUDIO_US_PER_S) * sizePerFrameInByte_;
    size_t minCbBufferSize =
        static_cast<size_t>(MIN_CBBUF_IN_USEC * curStreamParams_.samplingRate / AUDIO_US_PER_S) * sizePerFrameInByte_;
    size_t preferredCbBufferSize = static_cast<size_t>(frameSize) * sizePerFrameInByte_;
    std::lock_guard<std::mutex> lock(cbBufferMutex_);
    cbBufferSize_ = (preferredCbBufferSize > maxCbBufferSize || preferredCbBufferSize < minCbBufferSize) ?
        (preferredCbBufferSize > maxCbBufferSize ? maxCbBufferSize : minCbBufferSize) : preferredCbBufferSize;
    AUDIO_INFO_LOG("Set CallbackBuffer with byte size: %{public}zu", cbBufferSize_);
    cbBuffer_ = std::make_unique<uint8_t[]>(cbBufferSize_);
    return;
}

int32_t RendererInClientInner::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead)
{
    AUDIO_ERR_LOG("Read is not supported");
    return ERROR;
}

uint32_t RendererInClientInner::GetUnderflowCount()
{
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, 0, "buffer is not inited");

    return clientBuffer_->GetUnderrunCount();
}

uint32_t RendererInClientInner::GetOverflowCount()
{
    AUDIO_WARNING_LOG("No Overflow in renderer");
    return 0;
}

void RendererInClientInner::SetUnderflowCount(uint32_t underflowCount)
{
    CHECK_AND_RETURN_LOG(clientBuffer_ != nullptr, "buffer is not inited");
    clientBuffer_->SetUnderrunCount(underflowCount);
}

void RendererInClientInner::SetOverflowCount(uint32_t overflowCount)
{
    // not support for renderer
    AUDIO_WARNING_LOG("No Overflow in renderer");
    return;
}

void RendererInClientInner::SetRendererPositionCallback(int64_t markPosition,
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    // waiting for review
    std::lock_guard<std::mutex> lock(markReachMutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "RendererPositionCallback is nullptr");
    rendererPositionCallback_ = callback;
    rendererMarkPosition_ = markPosition;
    rendererMarkReached_ = false;
}

void RendererInClientInner::UnsetRendererPositionCallback()
{
    // waiting for review
    std::lock_guard<std::mutex> lock(markReachMutex_);
    rendererPositionCallback_ = nullptr;
    rendererMarkPosition_ = 0;
    rendererMarkReached_ = false;
}

void RendererInClientInner::SetRendererPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    // waiting for review
    std::lock_guard<std::mutex> lock(periodReachMutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr, "RendererPeriodPositionCallback is nullptr");
    rendererPeriodPositionCallback_ = callback;
    rendererPeriodSize_ = periodPosition;
    totalBytesWritten_ = 0;
    rendererPeriodWritten_ = 0;
}

void RendererInClientInner::UnsetRendererPeriodPositionCallback()
{
    // waiting for review
    std::lock_guard<std::mutex> lock(periodReachMutex_);
    rendererPeriodPositionCallback_ = nullptr;
    rendererPeriodSize_ = 0;
    totalBytesWritten_ = 0;
    rendererPeriodWritten_ = 0;
}

void RendererInClientInner::SetCapturerPositionCallback(int64_t markPosition,
    const std::shared_ptr<CapturerPositionCallback> &callback)
{
    AUDIO_ERR_LOG("SetCapturerPositionCallback is not supported");
    return;
}

void RendererInClientInner::UnsetCapturerPositionCallback()
{
    AUDIO_ERR_LOG("SetCapturerPositionCallback is not supported");
    return;
}

void RendererInClientInner::SetCapturerPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    AUDIO_ERR_LOG("SetCapturerPositionCallback is not supported");
    return;
}

void RendererInClientInner::UnsetCapturerPeriodPositionCallback()
{
    AUDIO_ERR_LOG("SetCapturerPositionCallback is not supported");
    return;
}

int32_t RendererInClientInner::SetRendererSamplingRate(uint32_t sampleRate)
{
    AUDIO_ERR_LOG("SetRendererSamplingRate to %{public}d is not supported", sampleRate);
    return ERROR;
}

uint32_t RendererInClientInner::GetRendererSamplingRate()
{
    return curStreamParams_.samplingRate;
}

int32_t RendererInClientInner::SetBufferSizeInMsec(int32_t bufferSizeInMsec)
{
    // bufferSizeInMsec is checked between 5ms and 20ms.
    bufferSizeInMsec_ = static_cast<uint32_t>(bufferSizeInMsec);
    AUDIO_INFO_LOG("SetBufferSizeInMsec to %{public}d", bufferSizeInMsec_);
    if (renderMode_ == RENDER_MODE_CALLBACK) {
        uint64_t bufferDurationInUs = bufferSizeInMsec_ * AUDIO_US_PER_MS;
        InitCallbackBuffer(bufferDurationInUs);
    }
    return SUCCESS;
}

int32_t RendererInClientInner::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    if ((state_ != PREPARED) && (state_ != NEW)) {
        AUDIO_ERR_LOG("SetChannelBlendMode in invalid status:%{public}d", state_.load());
        return ERR_ILLEGAL_STATE;
    }
    isBlendSet_ = true;
    audioBlend_.SetParams(blendMode, curStreamParams_.format, curStreamParams_.channels);
    return SUCCESS;
}

int32_t RendererInClientInner::SetVolumeWithRamp(float volume, int32_t duration)
{
    CHECK_AND_RETURN_RET_LOG((state_ != RELEASED) && (state_ != INVALID) && (state_ != STOPPED),
        ERR_ILLEGAL_STATE, "Illegal state %{public}d", state_.load());

    if (FLOAT_COMPARE_EQ(clientVolume_, volume)) {
        AUDIO_INFO_LOG("set same volume %{public}f", volume);
        return SUCCESS;
    }

    volumeRamp_.SetVolumeRampConfig(volume, clientVolume_, duration);
    return SUCCESS;
}

void RendererInClientInner::SetStreamTrackerState(bool trackerRegisteredState)
{
    streamTrackerRegistered_ = trackerRegisteredState;
}

void RendererInClientInner::GetSwitchInfo(IAudioStream::SwitchInfo& info)
{
    info.params = streamParams_;

    info.rendererInfo = rendererInfo_;
    info.capturerInfo = capturerInfo_;
    info.eStreamType = eStreamType_;
    info.renderMode = renderMode_;
    info.state = state_;
    info.sessionId = sessionId_;
    info.streamTrackerRegistered = streamTrackerRegistered_;
    info.defaultOutputDevice = defaultOutputDevice_;
    GetStreamSwitchInfo(info);

    {
        std::lock_guard<std::mutex> lock(setPreferredFrameSizeMutex_);
        info.userSettedPreferredFrameSize = userSettedPreferredFrameSize_;
    }
}

void RendererInClientInner::GetStreamSwitchInfo(IAudioStream::SwitchInfo& info)
{
    info.underFlowCount = GetUnderflowCount();
    info.effectMode = effectMode_;
    info.renderRate = rendererRate_;
    info.clientPid = clientPid_;
    info.clientUid = clientUid_;
    info.volume = clientVolume_;
    info.silentModeAndMixWithOthers = silentModeAndMixWithOthers_;

    info.frameMarkPosition = static_cast<uint64_t>(rendererMarkPosition_);
    info.renderPositionCb = rendererPositionCallback_;

    info.framePeriodNumber = static_cast<uint64_t>(rendererPeriodSize_);
    info.renderPeriodPositionCb = rendererPeriodPositionCallback_;

    info.rendererWriteCallback = writeCb_;
}

IAudioStream::StreamClass RendererInClientInner::GetStreamClass()
{
    return PA_STREAM;
}

void RendererInClientInner::OnHandle(uint32_t code, int64_t data)
{
    AUDIO_DEBUG_LOG("On handle event, event code: %{public}d, data: %{public}" PRIu64 "", code, data);
    switch (code) {
        case STATE_CHANGE_EVENT:
            HandleStateChangeEvent(data);
            break;
        case RENDERER_MARK_REACHED_EVENT:
            HandleRenderMarkReachedEvent(data);
            break;
        case RENDERER_PERIOD_REACHED_EVENT:
            HandleRenderPeriodReachedEvent(data);
            break;
        default:
            break;
    }
}

void RendererInClientInner::InitCallbackHandler()
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (callbackHandler_ == nullptr) {
        callbackHandler_ = CallbackHandler::GetInstance(shared_from_this(), "OS_AudioStateCB");
    }
}

void RendererInClientInner::SafeSendCallbackEvent(uint32_t eventCode, int64_t data)
{
    std::lock_guard<std::mutex> lock(runnerMutex_);
    AUDIO_INFO_LOG("Send callback event, code: %{public}u, data: %{public}" PRId64 "", eventCode, data);
    CHECK_AND_RETURN_LOG(callbackHandler_ != nullptr && runnerReleased_ == false, "Runner is Released");
    callbackHandler_->SendCallbackEvent(eventCode, data);
}

int32_t RendererInClientInner::StateCmdTypeToParams(int64_t &params, State state, StateChangeCmdType cmdType)
{
    if (cmdType == CMD_FROM_CLIENT) {
        params = static_cast<int64_t>(state);
        return SUCCESS;
    }
    switch (state) {
        case RUNNING:
            params = HANDLER_PARAM_RUNNING_FROM_SYSTEM;
            break;
        case PAUSED:
            params = HANDLER_PARAM_PAUSED_FROM_SYSTEM;
            break;
        default:
            params = HANDLER_PARAM_INVALID;
            break;
    }
    return SUCCESS;
}

int32_t RendererInClientInner::ParamsToStateCmdType(int64_t params, State &state, StateChangeCmdType &cmdType)
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

// OnRenderMarkReach by eventHandler
void RendererInClientInner::SendRenderMarkReachedEvent(int64_t rendererMarkPosition)
{
    SafeSendCallbackEvent(RENDERER_MARK_REACHED_EVENT, rendererMarkPosition);
}

// OnRenderPeriodReach by eventHandler
void RendererInClientInner::SendRenderPeriodReachedEvent(int64_t rendererPeriodSize)
{
    SafeSendCallbackEvent(RENDERER_PERIOD_REACHED_EVENT, rendererPeriodSize);
}

void RendererInClientInner::HandleRendererPositionChanges(size_t bytesWritten)
{
    totalBytesWritten_ += static_cast<int64_t>(bytesWritten);
    if (sizePerFrameInByte_ == 0) {
        AUDIO_ERR_LOG("HandleRendererPositionChanges: sizePerFrameInByte_ is 0");
        return;
    }
    int64_t writtenFrameNumber = totalBytesWritten_ / static_cast<int64_t>(sizePerFrameInByte_);
    AUDIO_DEBUG_LOG("frame size: %{public}zu", sizePerFrameInByte_);

    {
        std::lock_guard<std::mutex> lock(markReachMutex_);
        if (!rendererMarkReached_) {
            AUDIO_DEBUG_LOG("Frame mark position: %{public}" PRId64", Total frames written: %{public}" PRId64,
                static_cast<int64_t>(rendererMarkPosition_), static_cast<int64_t>(writtenFrameNumber));
            if (writtenFrameNumber >= rendererMarkPosition_) {
                AUDIO_DEBUG_LOG("OnMarkReached %{public}" PRId64".", rendererMarkPosition_);
                SendRenderMarkReachedEvent(rendererMarkPosition_);
                rendererMarkReached_ = true;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(periodReachMutex_);
        rendererPeriodWritten_ += static_cast<int64_t>((bytesWritten / sizePerFrameInByte_));
        AUDIO_DEBUG_LOG("Frame period number: %{public}" PRId64", Total frames written: %{public}" PRId64,
            static_cast<int64_t>(rendererPeriodWritten_), static_cast<int64_t>(totalBytesWritten_));
        if (rendererPeriodWritten_ >= rendererPeriodSize_ && rendererPeriodSize_ > 0) {
            rendererPeriodWritten_ %= rendererPeriodSize_;
            AUDIO_DEBUG_LOG("OnPeriodReached, remaining frames: %{public}" PRId64,
                static_cast<int64_t>(rendererPeriodWritten_));
            SendRenderPeriodReachedEvent(rendererPeriodSize_);
        }
    }
}

void RendererInClientInner::HandleStateChangeEvent(int64_t data)
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

void RendererInClientInner::HandleRenderMarkReachedEvent(int64_t rendererMarkPosition)
{
    AUDIO_DEBUG_LOG("Start HandleRenderMarkReachedEvent");
    std::unique_lock<std::mutex> lock(markReachMutex_);
    if (rendererPositionCallback_) {
        rendererPositionCallback_->OnMarkReached(rendererMarkPosition);
    }
}

void RendererInClientInner::HandleRenderPeriodReachedEvent(int64_t rendererPeriodNumber)
{
    AUDIO_DEBUG_LOG("Start HandleRenderPeriodReachedEvent");
    std::unique_lock<std::mutex> lock(periodReachMutex_);
    if (rendererPeriodPositionCallback_) {
        rendererPeriodPositionCallback_->OnPeriodReached(rendererPeriodNumber);
    }
}

void RendererInClientInner::OnSpatializationStateChange(const AudioSpatializationState &spatializationState)
{
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "Object ipcStream is nullptr");
    CHECK_AND_RETURN_LOG(ipcStream_->UpdateSpatializationState(spatializationState.spatializationEnabled,
        spatializationState.headTrackingEnabled) == SUCCESS, "Update spatialization state failed");
}

void RendererInClientInner::UpdateLatencyTimestamp(std::string &timestamp, bool isRenderer)
{
    sptr<IStandardAudioService> gasp = RendererInClientInner::GetAudioServerProxy();
    if (gasp == nullptr) {
        AUDIO_ERR_LOG("LatencyMeas failed to get AudioServerProxy");
        return;
    }
    gasp->UpdateLatencyTimestamp(timestamp, isRenderer);
}

int32_t RendererInClientInner::SetSourceDuration(int64_t duration)
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_OPERATION_FAILED, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetSourceDuration(duration);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Set Source Duration failed:%{public}d", ret);
        return ERROR;
    }
    return SUCCESS;
}

bool RendererInClientInner::GetOffloadEnable()
{
    return offloadEnable_;
}

bool RendererInClientInner::GetSpatializationEnabled()
{
    return rendererInfo_.spatializationEnabled;
}

bool RendererInClientInner::GetHighResolutionEnabled()
{
    return AudioPolicyManager::GetInstance().IsHighResolutionExist();
}

void RendererInClientInner::SetSilentModeAndMixWithOthers(bool on)
{
    AUDIO_PRERELEASE_LOGI("SetSilentModeAndMixWithOthers %{public}d", on);
    silentModeAndMixWithOthers_ = on;
    CHECK_AND_RETURN_LOG(ipcStream_ != nullptr, "Object ipcStream is nullptr");
    ipcStream_->SetSilentModeAndMixWithOthers(on);
    return;
}

bool RendererInClientInner::GetSilentModeAndMixWithOthers()
{
    return silentModeAndMixWithOthers_;
}

bool RendererInClientInner::RestoreAudioStream(bool needStoreState)
{
    CHECK_AND_RETURN_RET_LOG(proxyObj_ != nullptr, false, "proxyObj_ is null");
    CHECK_AND_RETURN_RET_LOG(state_ != NEW && state_ != INVALID && state_ != RELEASED, true,
        "state_ is %{public}d, no need for restore", state_.load());
    bool result = true;
    State oldState = state_;
    state_ = NEW;
    SetStreamTrackerState(false);
    // If pipe type is offload, need reset to normal.
    // Otherwise, unable to enter offload mode.
    if (rendererInfo_.pipeType == PIPE_TYPE_OFFLOAD) {
        rendererInfo_.pipeType = PIPE_TYPE_NORMAL_OUT;
    }
    int32_t ret = SetAudioStreamInfo(streamParams_, proxyObj_);
    if (ret != SUCCESS) {
        goto error;
    }
    if (!needStoreState) {
        AUDIO_INFO_LOG("telephony scene, return directly");
        return ret == SUCCESS;
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
        case STOPPING:
            result = StartAudioStream() && StopAudioStream();
            break;
        default:
            state_ = oldState;
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

int32_t RendererInClientInner::SetDefaultOutputDevice(const DeviceType defaultOutputDevice)
{
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is not inited!");
    int32_t ret = ipcStream_->SetDefaultOutputDevice(defaultOutputDevice);
    if (ret == SUCCESS) {
        defaultOutputDevice_ = defaultOutputDevice;
    }
    return ret;
}

DeviceType RendererInClientInner::GetDefaultOutputDevice()
{
    return defaultOutputDevice_;
}

int32_t RendererInClientInner::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    CHECK_AND_RETURN_RET_LOG(state_ == RUNNING, ERR_ILLEGAL_STATE, "Renderer stream state is not RUNNING");
    CHECK_AND_RETURN_RET_LOG(ipcStream_ != nullptr, ERR_ILLEGAL_STATE, "ipcStream is not inited!");
    uint64_t readIdx = 0;
    uint64_t timestampVal = 0;
    uint64_t latency = 0;
    int32_t ret = ipcStream_->GetAudioPosition(readIdx, timestampVal, latency);
    readIdx = readIdx > lastFlushReadIndex_ ? readIdx - lastFlushReadIndex_ : 0;
    uint64_t framePosition = lastFramePosition_;
    if (readIdx >= latency + lastReadIdx_) { // happen when last speed latency consumed
        framePosition += lastLatencyPosition_ + (readIdx - lastReadIdx_ - latency) * speed_;
        lastLatency_ = latency;
        lastLatencyPosition_ = latency * speed_;
        lastReadIdx_ = readIdx;
    } else { // happen when last speed latency not consumed
        if (lastLatency_ + readIdx > latency + lastReadIdx_) {
            framePosition += lastLatencyPosition_ * (lastLatency_ + readIdx - latency - lastReadIdx_) / lastLatency_;
            lastLatencyPosition_ = lastLatencyPosition_ * (latency + lastReadIdx_ - readIdx) / lastLatency_;
            lastLatency_ = latency + lastReadIdx_ - readIdx;
        }
    }
    // add MCR latency
    uint32_t mcrLatency = 0;
    if (converter_ != nullptr) {
        mcrLatency = converter_->GetLatency() * curStreamParams_.samplingRate / AUDIO_MS_PER_S;
        framePosition = framePosition > mcrLatency ? framePosition - mcrLatency : 0;
    }
 
    if (lastFramePosition_ < framePosition) {
        lastFramePosition_ = framePosition;
        lastFrameTimestamp_ = timestampVal;
    } else {
        AUDIO_DEBUG_LOG("The frame position should be continuously increasing");
        framePosition = lastFramePosition_;
        timestampVal = lastFrameTimestamp_;
    }
    AUDIO_DEBUG_LOG("[CLIENT]Latency info: framePosition: %{public}" PRIu64 ", lastFlushReadIndex_ %{public}" PRIu64
        ", timestamp %{public}" PRIu64 ", lastLatencyPosition_ %{public}" PRIu64 ", totlatency %{public}" PRIu64,
        framePosition, lastFlushReadIndex_, timestampVal, lastLatencyPosition_, latency + mcrLatency);
 
    timestamp.framePosition = framePosition;
    timestamp.time.tv_sec = static_cast<time_t>(timestampVal / AUDIO_NS_PER_SECOND);
    timestamp.time.tv_nsec = static_cast<time_t>(timestampVal % AUDIO_NS_PER_SECOND);
    return ret;
}

void RendererInClientInner::SetSwitchingStatus(bool isSwitching)
{
    std::lock_guard lock(switchingMutex_);
    if (isSwitching) {
        switchingInfo_ = {true, state_};
    } else {
        switchingInfo_ = {false, INVALID};
    }
}

void RendererInClientInner::GetRestoreInfo(RestoreInfo &restoreInfo)
{
    CHECK_AND_RETURN_LOG(clientBuffer_ != nullptr, "Client OHAudioBuffer is nullptr");
    clientBuffer_->GetRestoreInfo(restoreInfo);
    return;
}

void RendererInClientInner::SetRestoreInfo(RestoreInfo &restoreInfo)
{
    if (restoreInfo.restoreReason == SERVER_DIED) {
        cbThreadReleased_ = true;
    }
    CHECK_AND_RETURN_LOG(clientBuffer_ != nullptr, "Client OHAudioBuffer is nullptr");
    clientBuffer_->SetRestoreInfo(restoreInfo);
    return;
}

RestoreStatus RendererInClientInner::CheckRestoreStatus()
{
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, RESTORE_ERROR, "Client OHAudioBuffer is nullptr");
    return clientBuffer_->CheckRestoreStatus();
}

RestoreStatus RendererInClientInner::SetRestoreStatus(RestoreStatus restoreStatus)
{
    CHECK_AND_RETURN_RET_LOG(clientBuffer_ != nullptr, RESTORE_ERROR, "Client OHAudioBuffer is nullptr");
    return clientBuffer_->SetRestoreStatus(restoreStatus);
}

void RendererInClientInner::FetchDeviceForSplitStream()
{
    AUDIO_INFO_LOG("Fetch output device for split stream %{public}u", sessionId_);
    if (audioStreamTracker_ && audioStreamTracker_.get()) {
        audioStreamTracker_->FetchOutputDeviceForTrack(sessionId_,
            state_, clientPid_, rendererInfo_, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    } else {
        AUDIO_WARNING_LOG("Tracker is nullptr, fail to split stream %{public}u", sessionId_);
    }
    SetRestoreStatus(NO_NEED_FOR_RESTORE);
}
} // namespace AudioStandard
} // namespace OHOS