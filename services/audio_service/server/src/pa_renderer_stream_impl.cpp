/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "PaRendererStreamImpl"
#endif

#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#endif

#include "pa_renderer_stream_impl.h"

#include <chrono>

#include "safe_map.h"
#include "pa_adapter_tools.h"
#include "audio_effect_chain_manager.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "i_audio_renderer_sink.h"
#include "policy_handler.h"
#include "audio_volume.h"

namespace OHOS {
namespace AudioStandard {
static SafeMap<void *, std::weak_ptr<PaRendererStreamImpl>> rendererStreamInstanceMap_;
static const int32_t PA_STREAM_IMPL_TIMEOUT = 5; // 5s
const uint32_t DOUBLE_VALUE = 2;
const uint32_t MAX_LENGTH_OFFLOAD = 500;
const int32_t OFFLOAD_HDI_CACHE1 = 200; // ms, should equal with val in hdi_sink.c
const int32_t OFFLOAD_HDI_CACHE2 = 7000; // ms, should equal with val in hdi_sink.c
const uint32_t OFFLOAD_BUFFER = 50;
const uint64_t AUDIO_US_PER_MS = 1000;
const uint64_t AUDIO_NS_PER_US = 1000;
const uint64_t AUDIO_MS_PER_S = 1000;
const uint64_t AUDIO_US_PER_S = 1000000;
const uint64_t AUDIO_NS_PER_S = 1000000000;
const uint64_t AUDIO_CYCLE_TIME_US = 20000;

static int32_t CheckReturnIfStreamInvalid(pa_stream *paStream, const int32_t retVal)
{
    do {
        if (!(paStream && PA_STREAM_IS_GOOD(pa_stream_get_state(paStream)))) {
            return retVal;
        }
    } while (false);
    return SUCCESS;
}

PaRendererStreamImpl::PaRendererStreamImpl(pa_stream *paStream, AudioProcessConfig processConfig,
    pa_threaded_mainloop *mainloop)
{
    mainloop_ = mainloop;
    paStream_ = paStream;
    processConfig_ = processConfig;
}

PaRendererStreamImpl::~PaRendererStreamImpl()
{
    AUDIO_DEBUG_LOG("~PaRendererStreamImpl");

    PaLockGuard lock(mainloop_);
    rendererStreamInstanceMap_.Erase(this);
    if (paStream_) {
        if (!releasedFlag_) {
            pa_stream_set_state_callback(paStream_, nullptr, nullptr);
            pa_stream_set_write_callback(paStream_, nullptr, nullptr);
            pa_stream_set_latency_update_callback(paStream_, nullptr, nullptr);
            pa_stream_set_underflow_callback(paStream_, nullptr, nullptr);
            pa_stream_set_moved_callback(paStream_, nullptr, nullptr);
            pa_stream_set_started_callback(paStream_, nullptr, nullptr);
            pa_stream_disconnect(paStream_);
        }
        pa_stream_unref(paStream_);
        paStream_ = nullptr;
    }
}

int32_t PaRendererStreamImpl::InitParams()
{
    PaLockGuard lock(mainloop_);
    rendererStreamInstanceMap_.Insert(this, weak_from_this());
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) { return ERR_ILLEGAL_STATE; }

    sinkInputIndex_ = pa_stream_get_index(paStream_);
    pa_stream_set_moved_callback(paStream_, PAStreamMovedCb,
        reinterpret_cast<void *>(this)); // used to notify sink/source moved
    pa_stream_set_write_callback(paStream_, PAStreamWriteCb, reinterpret_cast<void *>(this));
    pa_stream_set_underflow_callback(paStream_, PAStreamUnderFlowCb, reinterpret_cast<void *>(this));
    pa_stream_set_started_callback(paStream_, PAStreamSetStartedCb, reinterpret_cast<void *>(this));
    pa_stream_set_underflow_ohos_callback(paStream_, PAStreamUnderFlowCountAddCb, reinterpret_cast<void *>(this));

    // Get byte size per frame
    const pa_sample_spec *sampleSpec = pa_stream_get_sample_spec(paStream_);
    CHECK_AND_RETURN_RET_LOG(sampleSpec != nullptr, ERR_OPERATION_FAILED, "pa_sample_spec sampleSpec is nullptr");
    AUDIO_INFO_LOG("sampleSpec: channels: %{public}u, formats: %{public}d, rate: %{public}d", sampleSpec->channels,
        sampleSpec->format, sampleSpec->rate);

    if (sampleSpec->channels != processConfig_.streamInfo.channels) {
        AUDIO_WARNING_LOG("Unequal channels, in server: %{public}d, in client: %{public}d", sampleSpec->channels,
            processConfig_.streamInfo.channels);
    }
    if (static_cast<uint8_t>(sampleSpec->format) != processConfig_.streamInfo.format) { // In plan
        AUDIO_WARNING_LOG("Unequal format, in server: %{public}d, in client: %{public}d", sampleSpec->format,
            processConfig_.streamInfo.format);
    }
    byteSizePerFrame_ = pa_frame_size(sampleSpec);

    // Get min buffer size in frame
    const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(paStream_);
    if (bufferAttr == nullptr) {
        int32_t count = ++bufferNullCount_;
        AUDIO_ERR_LOG("pa_stream_get_buffer_attr returned nullptr count is %{public}d", count);
        if (count >= 5) { // bufferAttr is nullptr 5 times, reboot audioserver
            sleep(3); // sleep 3 seconds to dump stacktrace
            AudioXCollie audioXCollie("AudioServer::Kill", 1, nullptr, nullptr, 2); // 2 means RECOVERY
            sleep(2); // sleep 2 seconds to dump stacktrace
        }
        return ERR_OPERATION_FAILED;
    }
    bufferNullCount_ = 0;
    minBufferSize_ = (size_t)bufferAttr->minreq;
    if (byteSizePerFrame_ == 0) {
        AUDIO_ERR_LOG("byteSizePerFrame_ should not be zero.");
        return ERR_INVALID_PARAM;
    }
    spanSizeInFrame_ = minBufferSize_ / byteSizePerFrame_;

    lock.Unlock();

    AudioVolume::GetInstance()->SetFadeoutState(sinkInputIndex_, NO_FADE);
    // In plan: Get data from xml
    effectSceneName_ = processConfig_.rendererInfo.sceneType;

    clientVolume_ = 1.0f;
    ResetOffload();

    return SUCCESS;
}

int32_t PaRendererStreamImpl::Start()
{
    AUDIO_INFO_LOG("Enter");
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }
    pa_operation *operation = nullptr;

    pa_stream_state_t state = pa_stream_get_state(paStream_);
    if (state != PA_STREAM_READY) {
        return ERR_OPERATION_FAILED;
    }

    streamCmdStatus_ = 0;
    operation = pa_stream_cork(paStream_, 0, PAStreamStartSuccessCb, reinterpret_cast<void *>(this));
    CHECK_AND_RETURN_RET_LOG(operation != nullptr, ERR_OPERATION_FAILED, "pa_stream_cork operation is null");
    pa_operation_unref(operation);

    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume != nullptr) {
        std::string sessionIDTemp = std::to_string(streamIndex_);
        audioEffectVolume->SetStreamVolume(sessionIDTemp, clientVolume_);
    }
    initEffectFlag_ = false;
    return SUCCESS;
}

int32_t PaRendererStreamImpl::Pause(bool isStandby)
{
    AUDIO_INFO_LOG("Enter");
    PaLockGuard palock(mainloop_, 1);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }
    pa_operation *operation = nullptr;
    pa_stream_state_t state = pa_stream_get_state(paStream_);
    if (state != PA_STREAM_READY) {
        AUDIO_ERR_LOG("Stream Stop Failed");
        return ERR_OPERATION_FAILED;
    }

    AudioVolume::GetInstance()->SetFadeoutState(sinkInputIndex_, DO_FADE);
    if (!offloadEnable_) {
        palock.Unlock();
        {
            std::unique_lock<std::mutex> lock(fadingMutex_);
            const int32_t WAIT_TIME_MS = 40;
            fadingCondition_.wait_for(lock, std::chrono::milliseconds(WAIT_TIME_MS));
        }
        palock.Relock();
    }

    isStandbyPause_ = isStandby;
    operation = pa_stream_cork(paStream_, 1, PAStreamPauseSuccessCb, reinterpret_cast<void *>(this));
    pa_operation_unref(operation);
    CHECK_AND_RETURN_RET_LOG(operation != nullptr, ERR_OPERATION_FAILED, "pa_stream_cork operation is null");
    palock.Unlock();
    
    if (effectMode_ == EFFECT_DEFAULT && !IsEffectNone(processConfig_.rendererInfo.streamUsage) &&
        initEffectFlag_ == false) {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        if (audioEffectChainManager == nullptr) {
            AUDIO_INFO_LOG("audioEffectChainManager is null");
        } else {
            std::string sessionIDTemp = std::to_string(streamIndex_);
            audioEffectChainManager->InitEffectBuffer(sessionIDTemp);
            initEffectFlag_ = true;
        }
    }

    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume != nullptr) {
        std::string sessionIDTemp = std::to_string(streamIndex_);
        audioEffectVolume->StreamVolumeDelete(sessionIDTemp);
    }

    return SUCCESS;
}

bool PaRendererStreamImpl::IsEffectNone(StreamUsage streamUsage)
{
    if (streamUsage == STREAM_USAGE_SYSTEM || streamUsage == STREAM_USAGE_DTMF ||
        streamUsage == STREAM_USAGE_ENFORCED_TONE || streamUsage == STREAM_USAGE_ULTRASONIC ||
        streamUsage == STREAM_USAGE_NAVIGATION || streamUsage == STREAM_USAGE_NOTIFICATION) {
        return true;
    }
    return false;
}

int32_t PaRendererStreamImpl::Flush()
{
    AUDIO_PRERELEASE_LOGI("Enter");
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }

    pa_operation *operation = nullptr;
    pa_stream_state_t state = pa_stream_get_state(paStream_);
    if (state != PA_STREAM_READY) {
        AUDIO_ERR_LOG("Stream Flush Failed");
        return ERR_OPERATION_FAILED;
    }

    streamFlushStatus_ = 0;
    operation = pa_stream_flush(paStream_, PAStreamFlushSuccessCb, reinterpret_cast<void *>(this));
    if (operation == nullptr) {
        AUDIO_ERR_LOG("Stream Flush Operation Failed");
        return ERR_OPERATION_FAILED;
    }
    Trace trace("PaRendererStreamImpl::InitAudioEffectChainDynamic");
    if (effectMode_ == EFFECT_DEFAULT && !IsEffectNone(processConfig_.rendererInfo.streamUsage) &&
        initEffectFlag_ == false) {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        if (audioEffectChainManager == nullptr) {
            AUDIO_INFO_LOG("audioEffectChainManager is null");
        } else {
            std::string sessionIDTemp = std::to_string(streamIndex_);
            audioEffectChainManager->InitEffectBuffer(sessionIDTemp);
            initEffectFlag_ = true;
        }
    }
    pa_operation_unref(operation);
    return SUCCESS;
}

int32_t PaRendererStreamImpl::Drain()
{
    AUDIO_INFO_LOG("Enter");
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }
    isDrain_ = true;

    pa_operation *operation = nullptr;
    pa_stream_state_t state = pa_stream_get_state(paStream_);
    if (state != PA_STREAM_READY) {
        AUDIO_ERR_LOG("Stream drain failed, state is not ready");
        return ERR_OPERATION_FAILED;
    }
    streamDrainStatus_ = 0;
    operation = pa_stream_drain(paStream_, PAStreamDrainSuccessCb, reinterpret_cast<void *>(this));
    pa_operation_unref(operation);
    return SUCCESS;
}

int32_t PaRendererStreamImpl::Stop()
{
    AUDIO_INFO_LOG("Enter");
    state_ = STOPPING;
    PaLockGuard palock(mainloop_);

    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }

    AudioVolume::GetInstance()->SetFadeoutState(sinkInputIndex_, DO_FADE);
    if (!offloadEnable_) {
        palock.Unlock();
        {
            std::unique_lock<std::mutex> lock(fadingMutex_);
            const int32_t WAIT_TIME_MS = 20;
            fadingCondition_.wait_for(lock, std::chrono::milliseconds(WAIT_TIME_MS));
        }
        palock.Relock();
    }

    pa_operation *operation = pa_stream_cork(paStream_, 1, PaRendererStreamImpl::PAStreamAsyncStopSuccessCb,
        reinterpret_cast<void *>(this));
    CHECK_AND_RETURN_RET_LOG(operation != nullptr, ERR_OPERATION_FAILED, "pa_stream_cork operation is null");
    pa_operation_unref(operation);
    if (effectMode_ == EFFECT_DEFAULT && !IsEffectNone(processConfig_.rendererInfo.streamUsage) &&
        initEffectFlag_ == false) {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        if (audioEffectChainManager == nullptr) {
            AUDIO_INFO_LOG("audioEffectChainManager is null");
        } else {
            std::string sessionIDTemp = std::to_string(streamIndex_);
            audioEffectChainManager->InitEffectBuffer(sessionIDTemp);
            initEffectFlag_ = true;
        }
    }
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume != nullptr) {
        std::string sessionIDTemp = std::to_string(streamIndex_);
        audioEffectVolume->StreamVolumeDelete(sessionIDTemp);
    }

    return SUCCESS;
}

int32_t PaRendererStreamImpl::Release()
{
    AUDIO_INFO_LOG("Enter");

    if (state_ == RUNNING) {
        PaLockGuard lock(mainloop_);
        if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
            return ERR_ILLEGAL_STATE;
        }
        pa_operation *operation = pa_stream_cork(paStream_, 1, nullptr, nullptr);
        CHECK_AND_RETURN_RET_LOG(operation != nullptr, ERR_OPERATION_FAILED, "pa_stream_cork operation is null");
        pa_operation_unref(operation);
    }

    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_RELEASED);
    }
    state_ = RELEASED;
    if (effectMode_ == EFFECT_DEFAULT && !IsEffectNone(processConfig_.rendererInfo.streamUsage) &&
        initEffectFlag_ == false) {
        AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
        if (audioEffectChainManager == nullptr) {
            AUDIO_INFO_LOG("audioEffectChainManager is null");
        } else {
            std::string sessionIDTemp = std::to_string(streamIndex_);
            audioEffectChainManager->InitEffectBuffer(sessionIDTemp);
            initEffectFlag_ = true;
        }
    }
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();
    if (audioEffectVolume != nullptr) {
        std::string sessionIDTemp = std::to_string(streamIndex_);
        audioEffectVolume->StreamVolumeDelete(sessionIDTemp);
    }

    AudioVolume::GetInstance()->RemoveFadeoutState(sinkInputIndex_);

    PaLockGuard lock(mainloop_);
    if (paStream_) {
        pa_stream_set_state_callback(paStream_, nullptr, nullptr);
        pa_stream_set_write_callback(paStream_, nullptr, nullptr);
        pa_stream_set_latency_update_callback(paStream_, nullptr, nullptr);
        pa_stream_set_underflow_callback(paStream_, nullptr, nullptr);
        pa_stream_set_moved_callback(paStream_, nullptr, nullptr);
        pa_stream_set_started_callback(paStream_, nullptr, nullptr);

        pa_stream_disconnect(paStream_);
        releasedFlag_ = true;
    }

    return SUCCESS;
}

int32_t PaRendererStreamImpl::GetStreamFramesWritten(uint64_t &framesWritten)
{
    CHECK_AND_RETURN_RET_LOG(byteSizePerFrame_ != 0, ERR_ILLEGAL_STATE, "Error frame size");
    framesWritten = totalBytesWritten_ / byteSizePerFrame_;
    return SUCCESS;
}

int32_t PaRendererStreamImpl::GetCurrentTimeStamp(uint64_t &timestamp)
{
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }
    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("stream is released, remain current timestamp unchanged");
        return SUCCESS;
    }
    int32_t XcollieFlag = (1 | 2); // flag 1 generate log file, flag 2 die when timeout, restart server
    AudioXCollie audioXCollie("PaRendererStreamImpl::GetCurrentTimeStamp", PA_STREAM_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("pulseAudio timeout");
        }, nullptr, XcollieFlag);

    UpdatePaTimingInfo();

    const pa_timing_info *info = pa_stream_get_timing_info(paStream_);
    if (info == nullptr) {
        AUDIO_ERR_LOG("pa_stream_get_timing_info failed");
        return ERR_OPERATION_FAILED;
    }

    const pa_sample_spec *sampleSpec = pa_stream_get_sample_spec(paStream_);
    timestamp = pa_bytes_to_usec(info->write_index, sampleSpec);
    return SUCCESS;
}

int32_t PaRendererStreamImpl::GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp, uint64_t &latency)
{
    Trace trace("PaRendererStreamImpl::GetCurrentPosition");
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }
    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("stream is released, remain current position unchanged");
        return SUCCESS;
    }
    int32_t XcollieFlag = (1 | 2); // flag 1 generate log file, flag 2 die when timeout, restart server
    AudioXCollie audioXCollie("PaRendererStreamImpl::GetCurrentPosition", PA_STREAM_IMPL_TIMEOUT,
        [](void *) { AUDIO_ERR_LOG("pulseAudio timeout"); }, nullptr, XcollieFlag);

    pa_usec_t curTimeGetLatency = pa_rtclock_now();
    if (curTimeGetLatency - preTimeGetPaLatency_ > AUDIO_CYCLE_TIME_US || firstGetPaLatency_) { // 20000 cycle time
        UpdatePaTimingInfo();
        firstGetPaLatency_ = false;
        preTimeGetPaLatency_ = curTimeGetLatency;
    }

    const pa_timing_info *info = pa_stream_get_timing_info(paStream_);
    CHECK_AND_RETURN_RET_LOG(info != nullptr, ERR_OPERATION_FAILED, "pa_stream_get_timing_info failed");
    const pa_sample_spec *sampleSpec = pa_stream_get_sample_spec(paStream_);
    uint64_t readIndex = pa_bytes_to_usec(info->read_index, sampleSpec);
    framePosition = readIndex * sampleSpec->rate / AUDIO_US_PER_S;
    latency = info->sink_usec * sampleSpec->rate / AUDIO_US_PER_S;
    lock.Unlock();

    // Processing data for algorithmic time delays
    uint32_t algorithmLatency = GetEffectChainLatency();
    if (!offloadEnable_) {
        latency += algorithmLatency * sampleSpec->rate / AUDIO_MS_PER_S;
    }
    // Processing data for a2dpoffload time delays
    uint32_t a2dpOffloadLatency = GetA2dpOffloadLatency();
    latency += a2dpOffloadLatency * sampleSpec->rate / AUDIO_MS_PER_S;

    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    timestamp = static_cast<uint64_t>(tm.tv_sec) * AUDIO_NS_PER_S + static_cast<uint64_t>(tm.tv_nsec);

    AUDIO_DEBUG_LOG("Latency info: framePosition: %{public}" PRIu64 ",readIndex %{public}" PRIu64
        ",timestamp %{public}" PRIu64 ", effect latency: %{public}u ms, a2dp offload latency: %{public}u ms",
        framePosition, readIndex, timestamp, algorithmLatency, a2dpOffloadLatency);
    return SUCCESS;
}

void PaRendererStreamImpl::PAStreamUpdateTimingInfoSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    PaRendererStreamImpl *rendererStreamImpl = (PaRendererStreamImpl *)userdata;
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)rendererStreamImpl->mainloop_;
    pa_threaded_mainloop_signal(mainLoop, 0);
}

int32_t PaRendererStreamImpl::GetLatency(uint64_t &latency)
{
    Trace trace("PaRendererStreamImpl::GetLatency");
    int32_t XcollieFlag = (1 | 2); // flag 1 generate log file, flag 2 die when timeout, restart server
    AudioXCollie audioXCollie("PaRendererStreamImpl::GetLatency", PA_STREAM_IMPL_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("pulseAudio timeout");
        }, nullptr, XcollieFlag);
    pa_usec_t curTimeGetLatency = pa_rtclock_now();
    if (curTimeGetLatency - preTimeGetLatency_ < AUDIO_CYCLE_TIME_US && !firstGetLatency_) { // 20000 cycle time
        latency = preLatency_;
        return SUCCESS;
    }
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }
    if (state_ == RELEASED) {
        AUDIO_WARNING_LOG("stream is released, latency is 0");
        latency = 0;
        return SUCCESS;
    }
    pa_usec_t paLatency {0};

    UpdatePaTimingInfo();
    const pa_timing_info *info = pa_stream_get_timing_info(paStream_);
    CHECK_AND_RETURN_RET_LOG(info != nullptr, ERR_OPERATION_FAILED, "pa_stream_get_timing_info failed");
    const pa_sample_spec *sampleSpec = pa_stream_get_sample_spec(paStream_);
    uint64_t readIndex = pa_bytes_to_usec(info->read_index < 0 ? 0 : info->read_index, sampleSpec);
    uint64_t writeIndex = pa_bytes_to_usec(info->write_index < 0 ? 0 : info->write_index, sampleSpec);
    pa_usec_t usec = readIndex >= info->sink_usec ? readIndex - info->sink_usec : 0;
    paLatency = writeIndex >= usec ? writeIndex - usec : 0;
    lock.Unlock();

    latency = paLatency;
    uint32_t algorithmLatency = GetEffectChainLatency();
    latency += offloadEnable_ ? 0 : algorithmLatency * AUDIO_US_PER_MS;
    uint32_t a2dpOffloadLatency = GetA2dpOffloadLatency();
    latency += a2dpOffloadLatency * AUDIO_US_PER_MS;
    AUDIO_DEBUG_LOG("total latency: %{public}" PRIu64 ", pa latency: %{public}" PRIu64 ", algo latency: %{public}u ms"
        ", a2dp offload latency: %{public}u ms, write: %{public}" PRIu64 ", read: %{public}" PRIu64 ", sink:%{public}"
        PRIu64, latency, paLatency, algorithmLatency, a2dpOffloadLatency, writeIndex, readIndex, info->sink_usec);

    preLatency_ = latency;
    preTimeGetLatency_ = curTimeGetLatency;
    firstGetLatency_ = false;
    return SUCCESS;
}

uint32_t PaRendererStreamImpl::GetEffectChainLatency()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    uint32_t algorithmLatency = 0;
    if (audioEffectChainManager != nullptr) {
        algorithmLatency = audioEffectChainManager->GetLatency(std::to_string(streamIndex_));
    }
    return algorithmLatency;
}

uint32_t PaRendererStreamImpl::GetA2dpOffloadLatency()
{
    Trace trace("PaRendererStreamImpl::GetA2dpOffloadLatency");
    uint32_t a2dpOffloadLatency = 0;
    uint64_t a2dpOffloadSendDataSize = 0;
    uint32_t a2dpOffloadTimestamp = 0;
    auto& handle = PolicyHandler::GetInstance();
    int32_t ret = handle.OffloadGetRenderPosition(a2dpOffloadLatency, a2dpOffloadSendDataSize, a2dpOffloadTimestamp);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("OffloadGetRenderPosition failed");
    }
    return a2dpOffloadLatency;
}

int32_t PaRendererStreamImpl::SetRate(int32_t rate)
{
    AUDIO_INFO_LOG("SetRate in");
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }
    uint32_t currentRate = processConfig_.streamInfo.samplingRate;
    switch (rate) {
        case RENDER_RATE_NORMAL:
            break;
        case RENDER_RATE_DOUBLE:
            currentRate *= DOUBLE_VALUE;
            break;
        case RENDER_RATE_HALF:
            currentRate /= DOUBLE_VALUE;
            break;
        default:
            return ERR_INVALID_PARAM;
    }
    renderRate_ = rate;

    pa_operation *operation = pa_stream_update_sample_rate(paStream_, currentRate, nullptr, nullptr);
    if (operation != nullptr) {
        pa_operation_unref(operation);
    } else {
        AUDIO_ERR_LOG("SetRate: operation is nullptr");
    }
    return SUCCESS;
}

int32_t PaRendererStreamImpl::SetAudioEffectMode(int32_t effectMode)
{
    AUDIO_INFO_LOG("SetAudioEffectMode: %{public}d", effectMode);
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }

    effectMode_ = effectMode;
    const std::string effectModeName = GetEffectModeName(effectMode_);

    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        return ERR_OPERATION_FAILED;
    }

    pa_proplist_sets(propList, "scene.mode", effectModeName.c_str());
    pa_operation *updatePropOperation = pa_stream_proplist_update(paStream_, PA_UPDATE_REPLACE, propList,
        nullptr, nullptr);
    pa_proplist_free(propList);
    CHECK_AND_RETURN_RET_LOG(updatePropOperation != nullptr, ERR_OPERATION_FAILED, "updatePropOperation is nullptr");
    pa_operation_unref(updatePropOperation);

    return SUCCESS;
}

const std::string PaRendererStreamImpl::GetEffectModeName(int32_t effectMode)
{
    std::string name;
    switch (effectMode) {
        case 0: // AudioEffectMode::EFFECT_NONE
            name = "EFFECT_NONE";
            break;
        default:
            name = "EFFECT_DEFAULT";
    }

    const std::string modeName = name;
    return modeName;
}

int32_t PaRendererStreamImpl::GetAudioEffectMode(int32_t &effectMode)
{
    effectMode = effectMode_;
    return SUCCESS;
}

int32_t PaRendererStreamImpl::SetPrivacyType(int32_t privacyType)
{
    AUDIO_DEBUG_LOG("SetInnerCapturerState: %{public}d", privacyType);
    privacyType_ = privacyType;
    return SUCCESS;
}

int32_t PaRendererStreamImpl::GetPrivacyType(int32_t &privacyType)
{
    privacyType_ = privacyType;
    return SUCCESS;
}


void PaRendererStreamImpl::RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterStatusCallback in");
    statusCallback_ = callback;
}

void PaRendererStreamImpl::RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterWriteCallback in");
    writeCallback_ = callback;
}

BufferDesc PaRendererStreamImpl::DequeueBuffer(size_t length)
{
    BufferDesc bufferDesc;
    bufferDesc.bufLength = length;
    // DequeueBuffer is called in PAStreamWriteCb which is running in mainloop, so don't need lock mainloop.
    pa_stream_begin_write(paStream_, reinterpret_cast<void **>(&bufferDesc.buffer), &bufferDesc.bufLength);
    return bufferDesc;
}

int32_t PaRendererStreamImpl::EnqueueBuffer(const BufferDesc &bufferDesc)
{
    Trace trace("PaRendererStreamImpl::EnqueueBuffer " + std::to_string(bufferDesc.bufLength) + " totalBytesWritten" +
        std::to_string(totalBytesWritten_));
    int32_t error = 0;
    if (offloadEnable_) {
        error = OffloadUpdatePolicyInWrite();
        CHECK_AND_RETURN_RET_LOG(error == SUCCESS, error, "OffloadUpdatePolicyInWrite failed");
    }

    // EnqueueBuffer is called in mainloop in most cases and don't need lock.
    PaLockGuard palock(mainloop_, 1);

    if (paStream_ == nullptr) {
        AUDIO_ERR_LOG("paStream is nullptr");
        return ERR_ILLEGAL_STATE;
    }

    error = pa_stream_write(paStream_, static_cast<void*>(bufferDesc.buffer), bufferDesc.bufLength, nullptr,
        0LL, PA_SEEK_RELATIVE);
    if (error < 0) {
        AUDIO_ERR_LOG("Write stream failed");
        pa_stream_cancel_write(paStream_);
    }
    totalBytesWritten_ += bufferDesc.bufLength;
    return SUCCESS;
}

void PaRendererStreamImpl::PAStreamWriteCb(pa_stream *stream, size_t length, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "PAStreamWriteCb: userdata is null");

    std::weak_ptr<PaRendererStreamImpl> paRendererStreamWeakPtr;
    if (rendererStreamInstanceMap_.Find(userdata, paRendererStreamWeakPtr) == false) {
        AUDIO_ERR_LOG("streamImpl is nullptr");
        return;
    }
    auto streamImpl = paRendererStreamWeakPtr.lock();
    CHECK_AND_RETURN_LOG(streamImpl, "PAStreamWriteCb: userdata is null");

    Trace trace("PaRendererStreamImpl::PAStreamWriteCb sink-input:" + std::to_string(streamImpl->sinkInputIndex_) +
        " length:" + std::to_string(length));
    std::shared_ptr<IWriteCallback> writeCallback = streamImpl->writeCallback_.lock();
    if (writeCallback != nullptr) {
        writeCallback->OnWriteData(length);
    } else {
        AUDIO_ERR_LOG("Write callback is nullptr");
    }
}

void PaRendererStreamImpl::PAStreamMovedCb(pa_stream *stream, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "PAStreamMovedCb: userdata is null");

    // get stream informations.
    uint32_t deviceIndex = pa_stream_get_device_index(stream); // pa_context_get_sink_info_by_index
    uint32_t streamIndex = pa_stream_get_index(stream); // get pa_stream index

    // Return 1 if the sink or source this stream is connected to has been suspended.
    // This will return 0 if not, and a negative value on error.
    int res = pa_stream_is_suspended(stream);
    AUDIO_WARNING_LOG("PAstream:[%{public}d] moved to index:[%{public}d] suspended:[%{public}d]",
        streamIndex, deviceIndex, res);
}

void PaRendererStreamImpl::PAStreamUnderFlowCb(pa_stream *stream, void *userdata)
{
    Trace trace("PaRendererStreamImpl::PAStreamUnderFlowCb");
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    std::weak_ptr<PaRendererStreamImpl> paRendererStreamWeakPtr;
    if (rendererStreamInstanceMap_.Find(userdata, paRendererStreamWeakPtr) == false) {
        AUDIO_ERR_LOG("streamImpl is nullptr");
        return;
    }
    auto streamImpl = paRendererStreamWeakPtr.lock();
    CHECK_AND_RETURN_LOG(streamImpl, "PAStreamWriteCb: userdata is null");

    streamImpl->underFlowCount_++;
    std::shared_ptr<IStatusCallback> statusCallback = streamImpl->statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_UNDERRUN);
    }
    AUDIO_WARNING_LOG("PaRendererStreamImpl underrun: %{public}d!", streamImpl->underFlowCount_);
}

void PaRendererStreamImpl::PAStreamUnderFlowCountAddCb(pa_stream *stream, void *userdata)
{
    Trace trace("PaRendererStreamImpl::PAStreamUnderFlowCountAddCb");
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    std::weak_ptr<PaRendererStreamImpl> paRendererStreamWeakPtr;
    if (rendererStreamInstanceMap_.Find(userdata, paRendererStreamWeakPtr) == false) {
        AUDIO_ERR_LOG("streamImpl is nullptr");
        return;
    }
    auto streamImpl = paRendererStreamWeakPtr.lock();
    CHECK_AND_RETURN_LOG(streamImpl, "PAStreamWriteCb: userdata is null");

    std::shared_ptr<IStatusCallback> statusCallback = streamImpl->statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_UNDERFLOW);
    }
}

void PaRendererStreamImpl::PAStreamSetStartedCb(pa_stream *stream, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "PAStreamSetStartedCb: userdata is null");
    AUDIO_PRERELEASE_LOGI("PAStreamSetStartedCb");
    Trace trace("PaRendererStreamImpl::PAStreamSetStartedCb");
}

void PaRendererStreamImpl::PAStreamStartSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    AUDIO_INFO_LOG("PAStreamStartSuccessCb in");
    CHECK_AND_RETURN_LOG(userdata, "PAStreamStartSuccessCb: userdata is null");

    std::weak_ptr<PaRendererStreamImpl> paRendererStreamWeakPtr;
    if (rendererStreamInstanceMap_.Find(userdata, paRendererStreamWeakPtr) == false) {
        AUDIO_ERR_LOG("streamImpl is nullptr");
        return;
    }
    auto streamImpl = paRendererStreamWeakPtr.lock();
    CHECK_AND_RETURN_LOG(streamImpl, "PAStreamWriteCb: userdata is null");

    streamImpl->state_ = RUNNING;
    streamImpl->offloadTsLast_ = 0;
    std::shared_ptr<IStatusCallback> statusCallback = streamImpl->statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_STARTED);
    }
    streamImpl->streamCmdStatus_ = success;
}

void PaRendererStreamImpl::PAStreamPauseSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "PAStreamPauseSuccessCb: userdata is null");

    std::weak_ptr<PaRendererStreamImpl> paRendererStreamWeakPtr;
    if (rendererStreamInstanceMap_.Find(userdata, paRendererStreamWeakPtr) == false) {
        AUDIO_ERR_LOG("streamImpl is nullptr");
        return;
    }
    auto streamImpl = paRendererStreamWeakPtr.lock();
    CHECK_AND_RETURN_LOG(streamImpl, "PAStreamWriteCb: userdata is null");

    streamImpl->state_ = PAUSED;
    if (streamImpl->offloadEnable_ && !streamImpl->isStandbyPause_) {
        streamImpl->offloadTsLast_ = 0;
        streamImpl->ResetOffload();
    }
    std::shared_ptr<IStatusCallback> statusCallback = streamImpl->statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_PAUSED);
    }
    streamImpl->streamCmdStatus_ = success;
}

void PaRendererStreamImpl::PAStreamFlushSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "PAStreamFlushSuccessCb: userdata is null");
    std::weak_ptr<PaRendererStreamImpl> paRendererStreamWeakPtr;
    if (rendererStreamInstanceMap_.Find(userdata, paRendererStreamWeakPtr) == false) {
        AUDIO_ERR_LOG("streamImpl is nullptr");
        return;
    }
    auto streamImpl = paRendererStreamWeakPtr.lock();
    CHECK_AND_RETURN_LOG(streamImpl, "Userdata is null");

    std::shared_ptr<IStatusCallback> statusCallback = streamImpl->statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_FLUSHED);
    }
    streamImpl->streamFlushStatus_ = success;
}

void PaRendererStreamImpl::PAStreamDrainSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "PAStreamDrainSuccessCb: userdata is null");

    std::weak_ptr<PaRendererStreamImpl> paRendererStreamWeakPtr;
    if (rendererStreamInstanceMap_.Find(userdata, paRendererStreamWeakPtr) == false) {
        AUDIO_ERR_LOG("streamImpl is nullptr");
        return;
    }
    auto streamImpl = paRendererStreamWeakPtr.lock();
    CHECK_AND_RETURN_LOG(streamImpl, "PAStreamWriteCb: userdata is null");

    std::shared_ptr<IStatusCallback> statusCallback = streamImpl->statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_DRAINED);
    }
    streamImpl->streamDrainStatus_ = success;
    streamImpl->isDrain_ = false;
}

void PaRendererStreamImpl::PAStreamDrainInStopCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "PAStreamDrainInStopCb: userdata is null");

    PaRendererStreamImpl *streamImpl = static_cast<PaRendererStreamImpl *>(userdata);
    pa_operation *operation = pa_stream_cork(streamImpl->paStream_, 1,
        PaRendererStreamImpl::PAStreamAsyncStopSuccessCb, userdata);

    CHECK_AND_RETURN_LOG(operation != nullptr, "pa_stream_cork operation is null");

    pa_operation_unref(operation);
    streamImpl->streamDrainStatus_ = success;
}

void PaRendererStreamImpl::PAStreamAsyncStopSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    AUDIO_DEBUG_LOG("PAStreamAsyncStopSuccessCb in");
    CHECK_AND_RETURN_LOG(userdata, "PAStreamAsyncStopSuccessCb: userdata is null");
    std::weak_ptr<PaRendererStreamImpl> paRendererStreamWeakPtr;
    if (rendererStreamInstanceMap_.Find(userdata, paRendererStreamWeakPtr) == false) {
        AUDIO_ERR_LOG("streamImpl is nullptr");
        return;
    }
    auto streamImpl = paRendererStreamWeakPtr.lock();
    CHECK_AND_RETURN_LOG(streamImpl, "PAStreamWriteCb: userdata is null");

    streamImpl->state_ = STOPPED;
    std::shared_ptr<IStatusCallback> statusCallback = streamImpl->statusCallback_.lock();

    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_STOPPED);
    }
}

int32_t PaRendererStreamImpl::GetMinimumBufferSize(size_t &minBufferSize) const
{
    minBufferSize = minBufferSize_;
    return SUCCESS;
}

void PaRendererStreamImpl::GetByteSizePerFrame(size_t &byteSizePerFrame) const
{
    byteSizePerFrame = byteSizePerFrame_;
}

void PaRendererStreamImpl::GetSpanSizePerFrame(size_t &spanSizeInFrame) const
{
    spanSizeInFrame = spanSizeInFrame_;
}

void PaRendererStreamImpl::SetStreamIndex(uint32_t index)
{
    AUDIO_INFO_LOG("Using index/sessionId %{public}d", index);
    streamIndex_ = index;
}

uint32_t PaRendererStreamImpl::GetStreamIndex()
{
    return streamIndex_;
}

void PaRendererStreamImpl::AbortCallback(int32_t abortTimes)
{
    abortFlag_ += abortTimes;
}

// offload

size_t PaRendererStreamImpl::GetWritableSize()
{
    PaLockGuard lock(mainloop_, 1);
    if (paStream_ == nullptr) {
        return 0;
    }
    return pa_stream_writable_size(paStream_);
}

int32_t PaRendererStreamImpl::OffloadSetVolume(float volume)
{
    if (!offloadEnable_) {
        return ERR_OPERATION_FAILED;
    }
    IAudioRendererSink *audioRendererSinkInstance = IAudioRendererSink::GetInstance("offload", "");

    if (audioRendererSinkInstance == nullptr) {
        AUDIO_ERR_LOG("Renderer is null.");
        return ERROR;
    }
    return audioRendererSinkInstance->SetVolume(volume, volume);
}

int32_t PaRendererStreamImpl::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }

    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        return ERR_OPERATION_FAILED;
    }

    pa_proplist_sets(propList, "spatialization.enabled", std::to_string(spatializationEnabled).c_str());
    pa_proplist_sets(propList, "headtracking.enabled", std::to_string(headTrackingEnabled).c_str());
    pa_operation *updatePropOperation = pa_stream_proplist_update(paStream_, PA_UPDATE_REPLACE, propList,
        nullptr, nullptr);
    pa_proplist_free(propList);
    CHECK_AND_RETURN_RET_LOG(updatePropOperation != nullptr, ERR_OPERATION_FAILED, "updatePropOperation is nullptr");
    pa_operation_unref(updatePropOperation);

    return SUCCESS;
}

int32_t PaRendererStreamImpl::OffloadGetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    auto *audioRendererSinkInstance = static_cast<IOffloadAudioRendererSink*> (IAudioRendererSink::GetInstance(
        "offload", ""));

    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "Renderer is null.");
    return audioRendererSinkInstance->GetPresentationPosition(frames, timeSec, timeNanoSec);
}

int32_t PaRendererStreamImpl::OffloadSetBufferSize(uint32_t sizeMs)
{
    auto *audioRendererSinkInstance = static_cast<IOffloadAudioRendererSink*> (IAudioRendererSink::GetInstance(
        "offload", ""));

    CHECK_AND_RETURN_RET_LOG(audioRendererSinkInstance != nullptr, ERROR, "Renderer is null.");
    return audioRendererSinkInstance->SetBufferSize(sizeMs);
}

int32_t PaRendererStreamImpl::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    if (!offloadEnable_) {
        return ERR_OPERATION_FAILED;
    }
    PaLockGuard lock(mainloop_);
    if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
        return ERR_ILLEGAL_STATE;
    }

    pa_operation *operation = pa_stream_update_timing_info(paStream_, NULL, NULL);
    if (operation != nullptr) {
        pa_operation_unref(operation);
    } else {
        AUDIO_ERR_LOG("pa_stream_update_timing_info failed");
    }

    const pa_timing_info *info = pa_stream_get_timing_info(paStream_);
    if (info == nullptr) {
        AUDIO_WARNING_LOG("pa_stream_get_timing_info failed");
        return SUCCESS;
    }

    const pa_sample_spec *sampleSpec = pa_stream_get_sample_spec(paStream_);
    uint64_t readIndex = pa_bytes_to_usec(info->read_index, sampleSpec);
    uint64_t writeIndex = pa_bytes_to_usec(info->write_index, sampleSpec);
    timestamp = info->timestamp.tv_sec * AUDIO_US_PER_SECOND + info->timestamp.tv_usec;
    lock.Unlock();

    uint64_t cacheTimeInPulse = writeIndex > readIndex ? writeIndex - readIndex : 0;
    cacheTimePa = cacheTimeInPulse;
    paWriteIndex = writeIndex;

    bool first = offloadTsLast_ == 0;
    offloadTsLast_ = readIndex;

    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    OffloadGetPresentationPosition(frames, timeSec, timeNanoSec);
    int64_t timeDelta = static_cast<int64_t>(timestamp) -
                        static_cast<int64_t>(timeSec * AUDIO_US_PER_SECOND + timeNanoSec / AUDIO_NS_PER_US);
    int64_t framesInt = static_cast<int64_t>(frames) + timeDelta;
    framesInt = framesInt > 0 ? framesInt : 0;
    int64_t readIndexInt = static_cast<int64_t>(readIndex);
    if (framesInt + offloadTsOffset_ < readIndexInt - static_cast<int64_t>(
        (OFFLOAD_HDI_CACHE2 + MAX_LENGTH_OFFLOAD + OFFLOAD_BUFFER) * AUDIO_US_PER_MS) ||
        framesInt + offloadTsOffset_ > readIndexInt || first) {
        offloadTsOffset_ = readIndexInt - framesInt;
    }
    cacheTimeDsp = static_cast<uint64_t>(readIndexInt - (framesInt + offloadTsOffset_));
    return SUCCESS;
}

int32_t PaRendererStreamImpl::OffloadUpdatePolicyInWrite()
{
    int error = 0;
    if ((lastOffloadUpdateFinishTime_ != 0) &&
        (std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) > lastOffloadUpdateFinishTime_)) {
        AUDIO_INFO_LOG("PaWriteStream switching curTime %{public}" PRIu64 ", switchTime %{public}" PRIu64,
            std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), lastOffloadUpdateFinishTime_);
        error = OffloadUpdatePolicy(offloadNextStateTargetPolicy_, true);
    }
    return error;
}

void PaRendererStreamImpl::SyncOffloadMode()
{
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        if (offloadEnable_) {
            statusCallback->OnStatusUpdate(OPERATION_SET_OFFLOAD_ENABLE);
        } else {
            statusCallback->OnStatusUpdate(OPERATION_UNSET_OFFLOAD_ENABLE);
        }
    }
}

void PaRendererStreamImpl::ResetOffload()
{
    offloadEnable_ = false;
    SyncOffloadMode();
    offloadTsOffset_ = 0;
    offloadTsLast_ = 0;
    OffloadUpdatePolicy(OFFLOAD_DEFAULT, true);
}

int32_t PaRendererStreamImpl::OffloadUpdatePolicy(AudioOffloadType statePolicy, bool force)
{
    // if possible turn on the buffer immediately(long buffer -> short buffer), turn it at once.
    if (statePolicy < offloadStatePolicy_ || offloadStatePolicy_ == OFFLOAD_DEFAULT || force) {
        AUDIO_DEBUG_LOG("Update statePolicy immediately: %{public}d -> %{public}d, force(%d)",
            offloadStatePolicy_, statePolicy, force);
        lastOffloadUpdateFinishTime_ = 0;
        PaLockGuard lock(mainloop_, 1);
        if (CheckReturnIfStreamInvalid(paStream_, ERR_ILLEGAL_STATE) < 0) {
            AUDIO_ERR_LOG("Set offload mode: invalid stream state, quit SetStreamOffloadMode due err");
            return ERR_ILLEGAL_STATE;
        }
        pa_proplist *propList = pa_proplist_new();
        CHECK_AND_RETURN_RET_LOG(propList != nullptr, ERR_OPERATION_FAILED, "pa_proplist_new failed");
        if (offloadEnable_) {
            pa_proplist_sets(propList, "stream.offload.enable", "1");
        } else {
            pa_proplist_sets(propList, "stream.offload.enable", "0");
        }
        pa_proplist_sets(propList, "stream.offload.statePolicy", std::to_string(statePolicy).c_str());

        pa_operation *updatePropOperation =
            pa_stream_proplist_update(paStream_, PA_UPDATE_REPLACE, propList, nullptr, nullptr);
        if (updatePropOperation == nullptr) {
            AUDIO_ERR_LOG("pa_stream_proplist_update failed!");
            pa_proplist_free(propList);
            return ERR_OPERATION_FAILED;
        }
        pa_proplist_free(propList);
        pa_operation_unref(updatePropOperation);

        if ((statePolicy != OFFLOAD_DEFAULT && offloadStatePolicy_ != OFFLOAD_DEFAULT) ||
            offloadStatePolicy_ == OFFLOAD_INACTIVE_BACKGROUND) {
            const uint32_t bufLenMs = statePolicy > 1 ? OFFLOAD_HDI_CACHE2 : OFFLOAD_HDI_CACHE1;
            OffloadSetBufferSize(bufLenMs);
        }

        offloadStatePolicy_ = statePolicy;
        offloadNextStateTargetPolicy_ = statePolicy; // Fix here if sometimes can't cut into state 3
    } else {
        // Otherwise, hdi_sink.c's times detects the stateTarget change and switches later
        // this time is checked the PaWriteStream to check if the switch has been made
        AUDIO_DEBUG_LOG("Update statePolicy in 3 seconds: %{public}d -> %{public}d", offloadStatePolicy_, statePolicy);
        lastOffloadUpdateFinishTime_ = std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now() + std::chrono::seconds(3)); // add 3s latency to change offload state
        offloadNextStateTargetPolicy_ = statePolicy;
    }

    return SUCCESS;
}

int32_t PaRendererStreamImpl::SetOffloadMode(int32_t state, bool isAppBack)
{
#ifdef FEATURE_POWER_MANAGER
    static const std::set<PowerMgr::PowerState> screenOffTable = {
        PowerMgr::PowerState::INACTIVE, PowerMgr::PowerState::STAND_BY,
        PowerMgr::PowerState::DOZE, PowerMgr::PowerState::SLEEP,
        PowerMgr::PowerState::HIBERNATE,
    };
    AudioOffloadType statePolicy = OFFLOAD_DEFAULT;
    auto powerState = static_cast<PowerMgr::PowerState>(state);
    if (screenOffTable.count(powerState)) {
        statePolicy = OFFLOAD_INACTIVE_BACKGROUND;
    } else {
        statePolicy = OFFLOAD_ACTIVE_FOREGROUND;
    }

    if (statePolicy == OFFLOAD_DEFAULT) {
        AUDIO_ERR_LOG("impossible INPUT branch error");
        return ERR_OPERATION_FAILED;
    }

    AUDIO_INFO_LOG("calling set stream offloadMode PowerState: %{public}d, isAppBack: %{public}d", state, isAppBack);

    if (offloadNextStateTargetPolicy_ == statePolicy) {
        return SUCCESS;
    }

    if ((offloadStatePolicy_ == offloadNextStateTargetPolicy_) && (offloadStatePolicy_ == statePolicy)) {
        return SUCCESS;
    }

    offloadEnable_ = true;
    SyncOffloadMode();
    if (OffloadUpdatePolicy(statePolicy, false) != SUCCESS) {
        return ERR_OPERATION_FAILED;
    }
#else
    AUDIO_INFO_LOG("SetStreamOffloadMode not available, FEATURE_POWER_MANAGER no define");
#endif
    return SUCCESS;
}

int32_t PaRendererStreamImpl::UnsetOffloadMode()
{
    offloadEnable_ = false;
    SyncOffloadMode();
    return OffloadUpdatePolicy(OFFLOAD_DEFAULT, true);
}

int32_t PaRendererStreamImpl::UpdateMaxLength(uint32_t maxLength)
{
    uint32_t tlength = 4; // 4 is tlength of dup playback
    uint32_t prebuf = 2; // 2 is prebuf of dup playback
    uint32_t maxlength = maxLength;
    AUDIO_INFO_LOG("dup playback stream tlength: %{public}u, maxlength: %{public}u prebuf: %{public}u", tlength,
        maxlength, prebuf);

    PaLockGuard lock(mainloop_);
    const pa_sample_spec *sampleSpec = pa_stream_get_sample_spec(paStream_);
    pa_buffer_attr bufferAttr;
    bufferAttr.fragsize = static_cast<uint32_t>(-1);
    bufferAttr.prebuf = pa_usec_to_bytes(20 * PA_USEC_PER_MSEC * prebuf, sampleSpec); // 20 buf len in ms
    bufferAttr.maxlength = pa_usec_to_bytes(20 * PA_USEC_PER_MSEC * maxlength, sampleSpec); // 20 buf len in ms
    bufferAttr.tlength = pa_usec_to_bytes(20 * PA_USEC_PER_MSEC * tlength, sampleSpec); // 20 buf len in ms
    bufferAttr.minreq = pa_usec_to_bytes(20 * PA_USEC_PER_MSEC, sampleSpec); // 20 buf len in ms

    pa_operation *operation = pa_stream_set_buffer_attr(paStream_, &bufferAttr, nullptr, nullptr);
    if (operation != nullptr) {
        pa_operation_unref(operation);
    }
    return SUCCESS;
}

AudioProcessConfig PaRendererStreamImpl::GetAudioProcessConfig() const noexcept
{
    return processConfig_;
}

int32_t PaRendererStreamImpl::Peek(std::vector<char> *audioBuffer, int32_t &index)
{
    return SUCCESS;
}

int32_t PaRendererStreamImpl::ReturnIndex(int32_t index)
{
    return SUCCESS;
}
// offload end

void PaRendererStreamImpl::BlockStream() noexcept
{
    return;
}

int32_t PaRendererStreamImpl::SetClientVolume(float clientVolume)
{
    if (clientVolume < MIN_FLOAT_VOLUME || clientVolume > MAX_FLOAT_VOLUME) {
        AUDIO_ERR_LOG("SetClientVolume with invalid clientVolume %{public}f", clientVolume);
        return ERR_INVALID_PARAM;
    }

    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    audioEffectChainManager->StreamVolumeUpdate(std::to_string(streamIndex_), clientVolume);
    AUDIO_PRERELEASE_LOGI("set client volume success");

    return SUCCESS;
}

void PaRendererStreamImpl::UpdatePaTimingInfo()
{
    pa_operation *operation = pa_stream_update_timing_info(paStream_, PAStreamUpdateTimingInfoSuccessCb, (void *)this);
    if (operation != nullptr) {
        auto start_time = std::chrono::steady_clock::now();
        while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
            if ((std::chrono::steady_clock::now() - start_time) > std::chrono::seconds(PA_STREAM_IMPL_TIMEOUT + 1)) {
                AUDIO_ERR_LOG("pa_stream_update_timing_info timeout");
                break;
            }
            pa_threaded_mainloop_wait(mainloop_);
        }
        pa_operation_unref(operation);
    } else {
        AUDIO_ERR_LOG("pa_stream_update_timing_info failed");
    }
}
} // namespace AudioStandard
} // namespace OHOS
