/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "HpaeRendererStreamImpl"
#endif

#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#endif

#include "hpae_renderer_stream_impl.h"
#include "sink/i_audio_render_sink.h"
#include "manager/hdi_adapter_manager.h"
#include <chrono>
#include "safe_map.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "i_hpae_manager.h"
#include "audio_stream_info.h"
#include "audio_effect_map.h"
#include "down_mixer.h"

using namespace OHOS::AudioStandard::HPAE;
namespace OHOS {
namespace AudioStandard {

const int32_t MIN_BUFFER_SIZE = 2;
const int32_t FRAME_LEN_10MS = 2;
const int32_t TENMS_PER_SEC = 100;
static const std::string DEVICE_CLASS_OFFLOAD = "offload";
static std::shared_ptr<IAudioRenderSink> GetRenderSinkInstance(std::string deviceClass, std::string deviceNetId);
HpaeRendererStreamImpl::HpaeRendererStreamImpl(AudioProcessConfig processConfig, bool isMoveAble, bool isCallbackMode)
{
    processConfig_ = processConfig;
    spanSizeInFrame_ = FRAME_LEN_10MS * (processConfig.streamInfo.samplingRate / TENMS_PER_SEC);
    byteSizePerFrame_ = (processConfig.streamInfo.channels *
        static_cast<size_t>((processConfig.streamInfo.format)));
    minBufferSize_ = MIN_BUFFER_SIZE * byteSizePerFrame_ * spanSizeInFrame_;
    isCallbackMode_ = isCallbackMode;
    isMoveAble_ = isMoveAble;
    if (!isCallbackMode_) {
        InitRingBuffer();
    }
}
HpaeRendererStreamImpl::~HpaeRendererStreamImpl()
{
    AUDIO_DEBUG_LOG("~HpaeRendererStreamImpl");
    if (dumpEnqueueIn_ != nullptr) {
        DumpFileUtil::CloseDumpFile(&dumpEnqueueIn_);
    }
}

int32_t HpaeRendererStreamImpl::InitParams(const std::string &deviceName)
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = processConfig_.streamInfo.channels;
    streamInfo.samplingRate = processConfig_.streamInfo.samplingRate;
    streamInfo.format = processConfig_.streamInfo.format;
    streamInfo.channelLayout = processConfig_.streamInfo.channelLayout;
    if (streamInfo.channelLayout == CH_LAYOUT_UNKNOWN) {
        streamInfo.channelLayout = DownMixer::SetDefaultChannelLayout((AudioChannel)streamInfo.channels);
    }
    streamInfo.frameLen = spanSizeInFrame_;
    streamInfo.sessionId = processConfig_.originalSessionId;
    streamInfo.streamType = processConfig_.streamType;
    streamInfo.fadeType = FadeType::DEFAULT_FADE; // to be passed from processConfig
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.uid = processConfig_.appInfo.appUid;
    streamInfo.pid = processConfig_.appInfo.appPid;
    effectMode_ = processConfig_.rendererInfo.effectMode;
    streamInfo.effectInfo.effectMode = (effectMode_ != EFFECT_DEFAULT && effectMode_ != EFFECT_NONE) ? EFFECT_DEFAULT :
        static_cast<AudioEffectMode>(effectMode_);
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    streamInfo.effectInfo.effectScene = static_cast<AudioEffectScene>(GetKeyFromValue(
        audioSupportedSceneTypes, processConfig_.rendererInfo.sceneType));
    streamInfo.effectInfo.volumeType = STREAM_MUSIC;
    streamInfo.effectInfo.streamUsage = processConfig_.rendererInfo.streamUsage;
    streamInfo.sourceType = processConfig_.isInnerCapturer == true ? SOURCE_TYPE_PLAYBACK_CAPTURE : SOURCE_TYPE_INVALID;
    streamInfo.deviceName = deviceName;
    streamInfo.isMoveAble = isMoveAble_;
    AUDIO_INFO_LOG("InitParams channels %{public}u  end", streamInfo.channels);
    AUDIO_INFO_LOG("InitParams channelLayout %{public}" PRIu64 " end", streamInfo.channelLayout);
    AUDIO_INFO_LOG("InitParams samplingRate %{public}u  end", streamInfo.samplingRate);
    AUDIO_INFO_LOG("InitParams format %{public}u  end", streamInfo.format);
    AUDIO_INFO_LOG("InitParams frameLen %{public}zu  end", streamInfo.frameLen);
    AUDIO_INFO_LOG("InitParams streamType %{public}u  end", streamInfo.streamType);
    AUDIO_INFO_LOG("InitParams sessionId %{public}u  end", streamInfo.sessionId);
    AUDIO_INFO_LOG("InitParams streamClassType %{public}u  end", streamInfo.streamClassType);
    AUDIO_INFO_LOG("InitParams sourceType %{public}d  end", streamInfo.sourceType);
    auto &hpaeManager = IHpaeManager::GetHpaeManager();
    int32_t ret = hpaeManager.CreateStream(streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "CreateStream is error");

    // Register Callback
    ret = hpaeManager.RegisterStatusCallback(HPAE_STREAM_CLASS_TYPE_PLAY, streamInfo.sessionId, shared_from_this());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "RegisterStatusCallback is error");
    ret = hpaeManager.RegisterWriteCallback(streamInfo.sessionId, shared_from_this());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "RegisterWriteCallback is error");
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Start()
{
    AUDIO_INFO_LOG("Start");
    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    timestamp_ = static_cast<uint64_t>(tm.tv_sec) * AUDIO_NS_PER_SECOND + static_cast<uint64_t>(tm.tv_nsec);
    int32_t ret = IHpaeManager::GetHpaeManager().Start(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Start is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Pause(bool isStandby)
{
    AUDIO_INFO_LOG("Pause");
    int32_t ret = IHpaeManager::GetHpaeManager().Pause(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Pause is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Flush()
{
    AUDIO_PRERELEASE_LOGI("Flush Enter");
    int32_t ret = IHpaeManager::GetHpaeManager().Flush(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Flush is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Drain(bool stopFlag)
{
    AUDIO_INFO_LOG("Drain Enter %{public}d", stopFlag);
    int32_t ret = IHpaeManager::GetHpaeManager().Drain(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Drain is error");
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Stop()
{
    AUDIO_INFO_LOG("Stop Enter");
    int32_t ret = IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Stop is error");
        return ERR_INVALID_PARAM;
    }
    state_ = STOPPING;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::Release()
{
    if (state_ == RUNNING) {
        AUDIO_ERR_LOG("%{public}u Release state_ is RUNNING", processConfig_.originalSessionId);
        IHpaeManager::GetHpaeManager().Stop(HPAE_STREAM_CLASS_TYPE_PLAY, processConfig_.originalSessionId);
    }
    AUDIO_INFO_LOG("Release Enter");
    int32_t ret = IHpaeManager::GetHpaeManager().DestroyStream(HPAE_STREAM_CLASS_TYPE_PLAY,
        processConfig_.originalSessionId);
    if (ret != 0) {
        AUDIO_ERR_LOG("Release is error");
        return ERR_INVALID_PARAM;
    }
    state_ = RELEASED;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetStreamFramesWritten(uint64_t &framesWritten)
{
    framesWritten = framesWritten_;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetCurrentTimeStamp(uint64_t &timestamp)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    timestamp = timestamp_;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp, uint64_t &latency)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    framePosition = framePosition_;
    timestamp = timestamp_;
    latency = latency_;
    if (deviceClass_ != DEVICE_CLASS_OFFLOAD) {
        uint32_t SinkLatency = 0;
        std::shared_ptr<IAudioRenderSink> audioRendererSink = GetRenderSinkInstance(deviceClass_, deviceNetId_);
        if (audioRendererSink) {
            audioRendererSink->GetLatency(SinkLatency);
        }
        latency = SinkLatency + latency_;
    }
    return SUCCESS;
}


int32_t HpaeRendererStreamImpl::GetLatency(uint64_t &latency)
{
    std::shared_lock<std::shared_mutex> lock(latencyMutex_);
    if (deviceClass_ != DEVICE_CLASS_OFFLOAD) {
        uint32_t SinkLatency = 0;
        std::shared_ptr<IAudioRenderSink> audioRendererSink = GetRenderSinkInstance(deviceClass_, deviceNetId_);
        if (audioRendererSink) {
            audioRendererSink->GetLatency(SinkLatency);
        }
        latency = SinkLatency + latency_;
        return SUCCESS;
    }
    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    auto timestamp = static_cast<uint64_t>(tm.tv_sec) * 1000000000ll + static_cast<uint64_t>(tm.tv_nsec);
    auto interval = (timestamp - timestamp_) / 1000;
    latency = latency_ > interval ? latency_ - interval : 0;
    AUDIO_DEBUG_LOG("HpaeRendererStreamImpl::GetLatency latency_ %{public}" PRIu64 ", \
        interval %{public}llu latency %{public}" PRIu64, latency_, interval, latency);
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetRate(int32_t rate)
{
    AUDIO_INFO_LOG("SetRate in");
    renderRate_ = rate;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetAudioEffectMode(int32_t effectMode)
{
    AUDIO_INFO_LOG("SetAudioEffectMode: %d", effectMode);
    int32_t ret = IHpaeManager::GetHpaeManager().SetAudioEffectMode(processConfig_.originalSessionId, effectMode);
    if (ret != 0) {
        AUDIO_ERR_LOG("SetAudioEffectMode is error");
        return ERR_INVALID_PARAM;
    }
    effectMode_ = effectMode;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetAudioEffectMode(int32_t &effectMode)
{
    effectMode = effectMode_;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::SetPrivacyType(int32_t privacyType)
{
    AUDIO_DEBUG_LOG("SetInnerCapturerState: %d", privacyType);
    privacyType_ = privacyType;
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetPrivacyType(int32_t &privacyType)
{
    privacyType_ = privacyType;
    return SUCCESS;
}

void HpaeRendererStreamImpl::RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterStatusCallback in");
    statusCallback_ = callback;
}

void HpaeRendererStreamImpl::RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback)
{
    AUDIO_DEBUG_LOG("RegisterWriteCallback in");
    writeCallback_ = callback;
}

int32_t HpaeRendererStreamImpl::OnStreamData(AudioCallBackStreamInfo &callBackStreamInfo)
{
    {
        std::unique_lock<std::shared_mutex> lock(latencyMutex_);
        framePosition_ = callBackStreamInfo.framePosition;
        timestamp_ = callBackStreamInfo.timestamp;
        latency_ = callBackStreamInfo.latency;
        framesWritten_ = callBackStreamInfo.framesWritten;
        deviceClass_ = callBackStreamInfo.deviceClass;
        deviceNetId_ = callBackStreamInfo.deviceNetId;
    }
    if (isCallbackMode_) { // callback buffer
        auto writeCallback = writeCallback_.lock();
        if (callBackStreamInfo.needData && writeCallback) {
            return writeCallback->OnWriteData(callBackStreamInfo.inputData, callBackStreamInfo.requestDataLen);
        }
    } else { // write buffer
        return WriteDataFromRingBuffer(callBackStreamInfo.inputData, callBackStreamInfo.requestDataLen);
    }
    return SUCCESS;
}

BufferDesc HpaeRendererStreamImpl::DequeueBuffer(size_t length)
{
    BufferDesc bufferDesc;
    return bufferDesc;
}

int32_t HpaeRendererStreamImpl::EnqueueBuffer(const BufferDesc &bufferDesc)
{
    CHECK_AND_RETURN_RET_LOG(!isCallbackMode_, ERROR, "Not write buffer mode");
    CHECK_AND_RETURN_RET_LOG(ringBuffer_ != nullptr, ERROR, "RingBuffer is nullptr");

    size_t targetSize = bufferDesc.bufLength;
    OptResult result = ringBuffer_->GetWritableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
        "Get writable size failed, ret:%{public}d size:%{public}zu", result.ret, result.size);

    size_t writableSize = result.size;
    if (targetSize > writableSize) {
        AUDIO_ERR_LOG("Enqueue buffer overflow, targetSize: %{public}zu, writableSize: %{public}zu",
            targetSize, writableSize);
    }

    size_t writeSize = std::min(writableSize, targetSize);
    BufferWrap bufferWrap = {bufferDesc.buffer, writeSize};
    result = ringBuffer_->Enqueue(bufferWrap);
    if (result.ret != OPERATION_SUCCESS) {
        AUDIO_ERR_LOG("Enqueue buffer failed, ret:%{public}d size:%{public}zu", result.ret, result.size);
        return ERROR;
    }
    DumpFileUtil::WriteDumpFile(dumpEnqueueIn_, bufferDesc.buffer, writeSize);
    return writeSize; // success return written in length
}

int32_t HpaeRendererStreamImpl::GetMinimumBufferSize(size_t &minBufferSize) const
{
    minBufferSize = minBufferSize_;
    return SUCCESS;
}

void HpaeRendererStreamImpl::GetByteSizePerFrame(size_t &byteSizePerFrame) const
{
    byteSizePerFrame = byteSizePerFrame_;
}

void HpaeRendererStreamImpl::GetSpanSizePerFrame(size_t &spanSizeInFrame) const
{
    spanSizeInFrame = spanSizeInFrame_;
}

void HpaeRendererStreamImpl::SetStreamIndex(uint32_t index)
{
    AUDIO_INFO_LOG("Using index/sessionId %{public}d", index);
    streamIndex_ = index;
}

uint32_t HpaeRendererStreamImpl::GetStreamIndex()
{
    return streamIndex_;
}

void HpaeRendererStreamImpl::AbortCallback(int32_t abortTimes)
{
    abortFlag_ += abortTimes;
}

// offload

size_t HpaeRendererStreamImpl::GetWritableSize()
{
    return 0;
}

int32_t HpaeRendererStreamImpl::OffloadSetVolume(float volume)
{
    std::shared_ptr<IAudioRenderSink> audioRendererSinkInstance = GetRenderSinkInstance(DEVICE_CLASS_OFFLOAD, "");
    if (audioRendererSinkInstance == nullptr) {
        AUDIO_ERR_LOG("Renderer is null.");
        return ERROR;
    }
    return audioRendererSinkInstance->SetVolume(volume, volume);
}

int32_t HpaeRendererStreamImpl::SetOffloadDataCallbackState(int32_t state)
{
    AUDIO_INFO_LOG("SetOffloadDataCallbackState state: %{public}d", state);
    if (!offloadEnable_) {
        return ERR_OPERATION_FAILED;
    }
    return IHpaeManager::GetHpaeManager().SetOffloadRenderCallbackType(processConfig_.originalSessionId, state);
}

int32_t HpaeRendererStreamImpl::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    return SUCCESS;
}

void HpaeRendererStreamImpl::SyncOffloadMode()
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

int32_t HpaeRendererStreamImpl::SetOffloadMode(int32_t state, bool isAppBack)
{
#ifdef FEATURE_POWER_MANAGER
    static const std::set<PowerMgr::PowerState> screenOffTable = {
        PowerMgr::PowerState::INACTIVE, PowerMgr::PowerState::STAND_BY,
        PowerMgr::PowerState::DOZE, PowerMgr::PowerState::SLEEP,
        PowerMgr::PowerState::HIBERNATE,
    };
    AudioOffloadType statePolicy = OFFLOAD_DEFAULT;
    statePolicy = screenOffTable.count(static_cast<PowerMgr::PowerState>(state)) ?
        OFFLOAD_INACTIVE_BACKGROUND : OFFLOAD_ACTIVE_FOREGROUND;

    AUDIO_INFO_LOG("calling set stream offloadMode PowerState: %{public}d, isAppBack: %{public}d", state, isAppBack);

    if (offloadStatePolicy_.load() == statePolicy && offloadEnable_) {
        return SUCCESS;
    }

    offloadEnable_ = true;
    SyncOffloadMode();
    auto ret = IHpaeManager::GetHpaeManager().SetOffloadPolicy(processConfig_.originalSessionId, statePolicy);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        "SetOffloadPolicy failed, errcode is %{public}d", ret);
    offloadStatePolicy_.store(statePolicy);
#else
    AUDIO_INFO_LOG("SetStreamOffloadMode not available, FEATURE_POWER_MANAGER no define");
#endif
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::UnsetOffloadMode()
{
    offloadEnable_ = false;
    SyncOffloadMode();
    IHpaeManager::GetHpaeManager().SetOffloadPolicy(processConfig_.originalSessionId, OFFLOAD_DEFAULT);
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::UpdateMaxLength(uint32_t maxLength)
{
    size_t bufferSize = maxLength * spanSizeInFrame_ * byteSizePerFrame_;
    AUDIO_INFO_LOG("bufferSize: %{public}zu, spanSizeInFrame: %{public}zu, byteSizePerFrame: %{public}zu,"
        "maxLength:%{public}u", bufferSize, spanSizeInFrame_, byteSizePerFrame_, maxLength);
    if (ringBuffer_ != nullptr) {
        ringBuffer_->ReConfig(bufferSize, false);
    } else {
        AUDIO_ERR_LOG("ring buffer is nullptr!");
    }
    return SUCCESS;
}

AudioProcessConfig HpaeRendererStreamImpl::GetAudioProcessConfig() const noexcept
{
    return processConfig_;
}

int32_t HpaeRendererStreamImpl::Peek(std::vector<char> *audioBuffer, int32_t &index)
{
    return SUCCESS;
}

int32_t HpaeRendererStreamImpl::ReturnIndex(int32_t index)
{
    return SUCCESS;
}

void HpaeRendererStreamImpl::BlockStream() noexcept
{
    return;
}
// offload end

int32_t HpaeRendererStreamImpl::SetClientVolume(float clientVolume)
{
    AUDIO_PRERELEASE_LOGI("set client volume success");
    int32_t ret = IHpaeManager::GetHpaeManager().SetClientVolume(processConfig_.originalSessionId, clientVolume);
    if (ret != 0) {
        AUDIO_ERR_LOG("SetClientVolume is error");
        return ERR_INVALID_PARAM;
    }
    clientVolume_ = clientVolume;
    return SUCCESS;
}

void HpaeRendererStreamImpl::InitRingBuffer()
{
    uint32_t maxLength = 20; // 20 for dup and dual play, only for enqueue buffer
    size_t bufferSize = maxLength * spanSizeInFrame_ * byteSizePerFrame_;
    AUDIO_INFO_LOG("bufferSize: %{public}zu, spanSizeInFrame: %{public}zu, byteSizePerFrame: %{public}zu,"
        "maxLength:%{public}u", bufferSize, spanSizeInFrame_, byteSizePerFrame_, maxLength);
    // create ring buffer
    ringBuffer_ = AudioRingCache::Create(bufferSize);
    if (ringBuffer_ == nullptr) {
        AUDIO_ERR_LOG("Create ring buffer failed!");
    }

    std::string dumpEnqueueInFileName = std::to_string(processConfig_.originalSessionId) + "_dual_in_" +
        std::to_string(processConfig_.streamInfo.samplingRate) + "_" +
        std::to_string(processConfig_.streamInfo.channels) + "_" +
        std::to_string(processConfig_.streamInfo.format) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpEnqueueInFileName, &dumpEnqueueIn_);
}

int32_t HpaeRendererStreamImpl::WriteDataFromRingBuffer(int8_t *inputData, size_t requestDataLen)
{
    CHECK_AND_RETURN_RET_LOG(ringBuffer_ != nullptr, ERROR, "RingBuffer is nullptr");
    OptResult result = ringBuffer_->GetReadableSize();
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR,
        "RingBuffer get readable size failed, size is:%{public}zu", result.size);
    CHECK_AND_RETURN_RET_LOG((result.size != 0) && (result.size >= requestDataLen), ERROR,
        "Readable size is invalid, result.size:%{public}zu, requestDataLen:%{public}zu, buffer underflow.",
        result.size, requestDataLen);
    AUDIO_DEBUG_LOG("requestDataLen is:%{public}zu readSize is:%{public}zu", requestDataLen, result.size);
    result = ringBuffer_->Dequeue({reinterpret_cast<uint8_t *>(inputData), requestDataLen});
    CHECK_AND_RETURN_RET_LOG(result.ret == OPERATION_SUCCESS, ERROR, "RingBuffer dequeue failed");
    return SUCCESS;
}

void HpaeRendererStreamImpl::OnStatusUpdate(IOperation operation)
{
    auto statusCallback = statusCallback_.lock();
    if (statusCallback) {
        statusCallback->OnStatusUpdate(operation);
    }
}

static std::shared_ptr<IAudioRenderSink> GetRenderSinkInstance(std::string deviceClass, std::string deviceNetId)
{
    uint32_t renderId = HDI_INVALID_ID;
    renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(deviceClass,
        deviceNetId.empty() ? HDI_ID_INFO_DEFAULT : deviceNetId, false);
    return HdiAdapterManager::GetInstance().GetRenderSink(renderId, true);
}
} // namespace AudioStandard
} // namespace OHOS
