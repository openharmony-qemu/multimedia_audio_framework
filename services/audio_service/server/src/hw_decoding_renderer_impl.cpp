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
#define LOG_TAG "HWDecodingRendererStream"
#endif

#include "hw_decoding_renderer_impl.h"

#include "securec.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_volume.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/i_audio_render_sink.h"

// placed at the end to take effect
#include "audio_renderer_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint64_t MOCK_LATENCY_US = 100000; // 100ms
const std::string HW_DECODING_SINK = "hw_decoding";
}
HWDecodingRendererStream::HWDecodingRendererStream(AudioProcessConfig &processConfig)
    : processConfig_(processConfig)
{
    AUDIO_INFO_LOG("ctor");
}

HWDecodingRendererStream::~HWDecodingRendererStream()
{
    AUDIO_INFO_LOG("destor");
}

int32_t HWDecodingRendererStream::Init()
{
    Trace trace("HWDecodingRendererStream::InitParams");

    AudioStreamInfo streamInfo = processConfig_.streamInfo;
    AUDIO_INFO_LOG("encoding:%{public}s rate:%{public}d channels:%{public}u, formats:%{public}d",
        EncodingTypeStr(streamInfo.encoding).c_str(), streamInfo.samplingRate, streamInfo.channels, streamInfo.format);
    int32_t ret = InitSink(streamInfo);
    return ret;
}

int32_t HWDecodingRendererStream::InitSink(AudioStreamInfo streamInfo)
{
    std::string sinkName = HW_DECODING_SINK;
    renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_HWDECODE, sinkName, true);
    std::shared_ptr<IAudioRenderSink> sink = HdiAdapterManager::GetInstance().GetRenderSink(renderId_, true);
    if (sink == nullptr) {
        AUDIO_ERR_LOG("get render fail, sinkName: %{public}s", sinkName.c_str());
        HdiAdapterManager::GetInstance().ReleaseId(renderId_);
        return ERR_INVALID_HANDLE;
    }
    IAudioSinkAttr attr = {};
    attr.adapterName = "dp";
    attr.encodingType = streamInfo.encoding; // used for HW decoding
    attr.sampleRate = static_cast<uint32_t>(streamInfo.samplingRate);
    attr.channel = static_cast<uint32_t>(streamInfo.channels);
    attr.format = streamInfo.format;
    attr.channelLayout = static_cast<uint64_t>(streamInfo.channelLayout);
    attr.deviceType = DEVICE_TYPE_DP; // in plan
    attr.volume = 1.0f;
    attr.openMicSpeaker = 1;
    AUDIO_INFO_LOG("sinkName:%{public}s,device:%{public}d,sample rate:%{public}d,format:%{public}d,channel:%{public}d",
        sinkName.c_str(), attr.deviceType, attr.sampleRate, attr.format, attr.channel);
    int32_t ret = sink->Init(attr);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "init sink fail, sinkName: %{public}s", sinkName.c_str());

    float volume = 1.0f;
    sink->SetVolume(volume, volume);

    return ret;
}

int32_t HWDecodingRendererStream::Start()
{
    Trace trace("HWDecodingRendererStream::Start::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("in %{public}d", streamIndex_);

    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_STARTED);
    }
    return SUCCESS;
}

int32_t HWDecodingRendererStream::Pause(bool isStandby)
{
    Trace trace("HWDecodingRendererStream::Pause::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("in %{public}d", streamIndex_);

    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_PAUSED);
    }
    return SUCCESS;
}

int32_t HWDecodingRendererStream::Flush()
{
    Trace trace("HWDecodingRendererStream::Flush::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("in %{public}d", streamIndex_);

    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_FLUSHED);
    }
    return SUCCESS;
}

int32_t HWDecodingRendererStream::Drain(bool stopFlag)
{
    Trace trace("HWDecodingRendererStream::Drain::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("in %{public}d", streamIndex_);

    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_DRAINED);
    }
    return SUCCESS;
}

int32_t HWDecodingRendererStream::Stop()
{
    Trace trace("HWDecodingRendererStream::Stop::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("in %{public}d", streamIndex_);

    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_STOPPED);
    }
    return SUCCESS;
}

int32_t HWDecodingRendererStream::Release()
{
    Trace trace("HWDecodingRendererStream::Release::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("in %{public}d", streamIndex_);

    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_RELEASED);
    }
    return SUCCESS;
}

int32_t HWDecodingRendererStream::GetStreamFramesWritten(uint64_t &framesWritten)
{
    framesWritten = writtenFrameCount_;
    return SUCCESS;
}

int32_t HWDecodingRendererStream::GetCurrentTimeStamp(uint64_t &timestamp)
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp, uint64_t &latency,
    int32_t base)
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::GetLatency(uint64_t &latency)
{
    latency = MOCK_LATENCY_US;
    return SUCCESS;
}

int32_t HWDecodingRendererStream::SetRate(int32_t rate)
{
    AUDIO_WARNING_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

int32_t HWDecodingRendererStream::SetAudioEffectMode(int32_t effectMode)
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::GetAudioEffectMode(int32_t &effectMode)
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::SetPrivacyType(int32_t privacyType)
{
    privacyType_ = privacyType;
    return SUCCESS;
}

int32_t HWDecodingRendererStream::GetPrivacyType(int32_t &privacyType)
{
    privacyType = privacyType_;
    return SUCCESS;
}

int32_t HWDecodingRendererStream::SetSpeed(float speed)
{
    AUDIO_INFO_LOG("set speed to %{public}f", speed);
    return SUCCESS;
}

void HWDecodingRendererStream::RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback)
{
    statusCallback_ = callback;
}

void HWDecodingRendererStream::RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback)
{
    writeCallback_ = callback;
}

BufferDesc HWDecodingRendererStream::DequeueBuffer(size_t length)
{
    Trace trace("HWDecodingRendererStream::DequeueBuffer");
    BufferDesc bufferDesc = {nullptr, 0, 0};
    return bufferDesc;
}

int32_t HWDecodingRendererStream::EnqueueBuffer(const BufferDesc &bufferDesc)
{
    Trace trace("HWDecodingRendererStream::EnqueueBuffer::" + std::to_string(streamIndex_));
    writtenFrameCount_++;
    // in plan
    return SUCCESS;
}

int32_t HWDecodingRendererStream::GetMinimumBufferSize(size_t &minBufferSize) const
{
    return SUCCESS;
}

void HWDecodingRendererStream::GetByteSizePerFrame(size_t &byteSizePerFrame) const
{
    AudioStreamInfo streamInfo = processConfig_.streamInfo;
    byteSizePerFrame = Util::GetSamplePerFrame(streamInfo.format) * streamInfo.channels;
}

void HWDecodingRendererStream::GetSpanSizePerFrame(size_t &spanSizeInFrame) const
{
    spanSizeInFrame = 1; // mock value
}

void HWDecodingRendererStream::SetStreamIndex(uint32_t index)
{
    AUDIO_INFO_LOG("Using sessionId %{public}d", index);
    streamIndex_ = index;
}

uint32_t HWDecodingRendererStream::GetStreamIndex()
{
    return streamIndex_;
}

// offload
int32_t HWDecodingRendererStream::SetOffloadMode(int32_t state, bool isAppBack)
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::UnsetOffloadMode()
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::OffloadSetVolume()
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::SetOffloadDataCallbackState(int32_t state)
{
    AUDIO_WARNING_LOG("SetOffloadDataCallbackState not support");
    return ERR_NOT_SUPPORTED;
}

size_t HWDecodingRendererStream::GetWritableSize()
{
    // in plan
    return 1;
}
// offload end
int32_t HWDecodingRendererStream::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

AudioProcessConfig HWDecodingRendererStream::GetAudioProcessConfig() const noexcept
{
    return processConfig_;
}

int32_t HWDecodingRendererStream::Peek(std::vector<char> *audioBuffer, int32_t &index)
{
    // do nothing, peek is useless for HW decoding.
    return SUCCESS;
}

int32_t HWDecodingRendererStream::ReturnIndex(int32_t index)
{
    return SUCCESS;
}

int32_t HWDecodingRendererStream::SetClientVolume(float clientVolume)
{
    AUDIO_INFO_LOG("clientVolume: %{public}f", clientVolume);
    return SUCCESS;
}

int32_t HWDecodingRendererStream::SetLoudnessGain(float loudnessGain)
{
    AUDIO_WARNING_LOG("SetLoudnessGain only for hpae renderer stream");
    return ERR_PRO_STREAM_NOT_SUPPORTED;
}

int32_t HWDecodingRendererStream::UpdateMaxLength(uint32_t maxLength)
{
    return SUCCESS;
}

void HWDecodingRendererStream::BlockStream() noexcept
{
}

int32_t HWDecodingRendererStream::GetLatencyWithFlag(uint64_t &latency, LatencyFlag flag)
{
    latency = 0;
    bool needHardware = (flag & LATENCY_FLAG_HARDWARE) != 0;
    CHECK_AND_RETURN_RET(needHardware, SUCCESS);
    std::function<int32_t (uint32_t &)> fetcher;
    {
        std::lock_guard<std::mutex> lock(sinkLatencyFetcherMutex_);
        fetcher = sinkLatencyFetcher_;
    }
    CHECK_AND_RETURN_RET_LOG(fetcher, ERR_OPERATION_FAILED, "sinkLatencyFetcher is null");
    uint32_t latencyMs = 0;
    int32_t ret = fetcher(latencyMs);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "fetcher get latency failed %{public}d", ret);
    latency = static_cast<uint64_t>(latencyMs * AUDIO_US_PER_MS);
    return SUCCESS;
}

int32_t HWDecodingRendererStream::RegisterSinkLatencyFetcher(
    const std::function<int32_t (uint32_t &)> &fetcher)
{
    CHECK_AND_RETURN_RET_LOG(fetcher, ERR_INVALID_PARAM, "fetcher is null");
    std::lock_guard<std::mutex> lock(sinkLatencyFetcherMutex_);
    sinkLatencyFetcher_ = fetcher;
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
