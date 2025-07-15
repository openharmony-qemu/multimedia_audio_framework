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
#define LOG_TAG "ProRendererStream"
#endif

#include "pro_renderer_stream_impl.h"
#include "audio_errors.h"
#include "audio_renderer_log.h"
#include "audio_utils.h"
#include "securec.h"
#include "policy_handler.h"
#include "audio_volume.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint64_t AUDIO_NS_PER_S = 1000000000;
constexpr int32_t SECOND_TO_MILLISECOND = 1000;
constexpr int32_t DEFAULT_BUFFER_MILLISECOND = 20;
constexpr int32_t DEFAULT_BUFFER_MICROSECOND = 20000000;
constexpr uint32_t DOUBLE_VALUE = 2;
constexpr int32_t DEFAULT_RESAMPLE_QUANTITY = 2;
constexpr int32_t STEREO_CHANNEL_COUNT = 2;
constexpr int32_t DEFAULT_TOTAL_SPAN_COUNT = 2;
constexpr int32_t DRAIN_WAIT_TIMEOUT_TIME = 100;
constexpr int32_t FIRST_FRAME_TIMEOUT_TIME = 500;
const std::string DUMP_DIRECT_STREAM_FILE = "dump_direct_audio_stream.pcm";
const std::string DEVICE_NAME = "primary";

ProRendererStreamImpl::ProRendererStreamImpl(AudioProcessConfig processConfig, bool isDirect)
    : isDirect_(isDirect),
      isNeedResample_(false),
      isNeedMcr_(false),
      isBlock_(true),
      isDrain_(false),
      isFirstFrame_(true),
      privacyType_(0),
      renderRate_(0),
      streamIndex_(static_cast<uint32_t>(-1)),
      currentRate_(1),
      desSamplingRate_(0),
      desFormat_(AudioSampleFormat::SAMPLE_S32LE),
      byteSizePerFrame_(0),
      spanSizeInFrame_(0),
      totalBytesWritten_(0),
      sinkBytesWritten_(0),
      minBufferSize_(0),
      status_(I_STATUS_INVALID),
      resample_(nullptr),
      processConfig_(processConfig),
      downMixer_(nullptr),
      dumpFile_(nullptr)
{
    AUDIO_DEBUG_LOG("constructor");
}

ProRendererStreamImpl::~ProRendererStreamImpl()
{
    AUDIO_DEBUG_LOG("deconstructor");
    status_ = I_STATUS_INVALID;
    DumpFileUtil::CloseDumpFile(&dumpFile_);
}

AudioSamplingRate ProRendererStreamImpl::GetDirectSampleRate(AudioSamplingRate sampleRate) const noexcept
{
    if (processConfig_.streamType == STREAM_VOICE_CALL || processConfig_.streamType == STREAM_VOICE_COMMUNICATION) {
        // VoIP stream type. Return the special sample rate of direct VoIP mode.
        if (sampleRate <= AudioSamplingRate::SAMPLE_RATE_16000) {
            return AudioSamplingRate::SAMPLE_RATE_16000;
        } else {
            return AudioSamplingRate::SAMPLE_RATE_48000;
        }
    }
    // High resolution for music
    AudioSamplingRate result = sampleRate;
    switch (sampleRate) {
        case AudioSamplingRate::SAMPLE_RATE_44100:
            result = AudioSamplingRate::SAMPLE_RATE_48000;
            break;
        case AudioSamplingRate::SAMPLE_RATE_88200:
            result = AudioSamplingRate::SAMPLE_RATE_96000;
            break;
        case AudioSamplingRate::SAMPLE_RATE_176400:
            result = AudioSamplingRate::SAMPLE_RATE_192000;
            break;
        default:
            break;
    }
    return result;
}

AudioSampleFormat ProRendererStreamImpl::GetDirectFormat(AudioSampleFormat format) const noexcept
{
    if (isDirect_) {
        // Only SAMPLE_S32LE is supported for high resolution stream.
        return AudioSampleFormat::SAMPLE_S32LE;
    }

    // Both SAMPLE_S16LE and SAMPLE_S32LE are supported for direct VoIP stream.
    if (format == SAMPLE_S16LE || format == SAMPLE_S32LE) {
        return format;
    } else if (format == SAMPLE_F32LE) {
        // Direct VoIP not support SAMPLE_F32LE format.It needs to be converted to S16.
        return AudioSampleFormat::SAMPLE_S16LE;
    } else {
        AUDIO_WARNING_LOG("The format %{public}u is unsupported for direct VoIP. Use 32Bit.", format);
        return AudioSampleFormat::SAMPLE_S32LE;
    }
}

int32_t ProRendererStreamImpl::InitParams()
{
    Trace trace("ProRendererStreamImpl::InitParams");
    if (status_ != I_STATUS_INVALID) {
        return ERR_ILLEGAL_STATE;
    }
    AudioStreamInfo streamInfo = processConfig_.streamInfo;
    AUDIO_INFO_LOG("sampleSpec: channels: %{public}u, formats: %{public}d, rate: %{public}d", streamInfo.channels,
        streamInfo.format, streamInfo.samplingRate);
    InitBasicInfo(streamInfo);
    size_t frameSize = spanSizeInFrame_ * streamInfo.channels;
    uint32_t desChannels = streamInfo.channels >= STEREO_CHANNEL_COUNT ? STEREO_CHANNEL_COUNT : 1;
    uint32_t desSpanSize = (desSamplingRate_ * DEFAULT_BUFFER_MILLISECOND) / SECOND_TO_MILLISECOND;
    if (streamInfo.samplingRate != desSamplingRate_) {
        Trace::Count("ProRendererStreamImpl::InitParams", streamInfo.samplingRate);
        AUDIO_INFO_LOG("stream need resample, dest:%{public}d", desSamplingRate_);
        isNeedResample_ = true;
        resample_ = std::make_shared<AudioResample>(desChannels, streamInfo.samplingRate, desSamplingRate_,
            DEFAULT_RESAMPLE_QUANTITY);
        if (!resample_->IsResampleInit()) {
            AUDIO_ERR_LOG("resample not supported!");
            return ERR_INVALID_PARAM;
        }
        resampleSrcBuffer.resize(frameSize, 0.f);
        resampleDesBuffer.resize(desSpanSize * desChannels, 0.f);
        resample_->ProcessFloatResample(resampleSrcBuffer, resampleDesBuffer);
    }
    if (streamInfo.channels > STEREO_CHANNEL_COUNT) {
        Trace::Count("ProRendererStreamImpl::InitParams", streamInfo.channels);
        isNeedMcr_ = true;
        if (!isNeedResample_) {
            resampleSrcBuffer.resize(frameSize, 0.f);
            resampleDesBuffer.resize(desSpanSize * desChannels, 0.f);
        }
        downMixer_ = std::make_unique<AudioDownMixStereo>();
        int32_t ret = downMixer_->InitMixer(streamInfo.channelLayout, streamInfo.channels);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("down mixer not supported!");
            return ret;
        }
    }
    uint32_t bufferSize = Util::GetSamplePerFrame(desFormat_) * desSpanSize * desChannels;
    sinkBuffer_.resize(DEFAULT_TOTAL_SPAN_COUNT, std::vector<char>(bufferSize, 0));
    for (int32_t i = 0; i < DEFAULT_TOTAL_SPAN_COUNT; i++) {
        writeQueue_.emplace(i);
    }
    SetOffloadDisable();
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, DUMP_DIRECT_STREAM_FILE, &dumpFile_);
    status_ = I_STATUS_IDLE;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::Start()
{
    Trace trace("ProRendererStreamImpl::Start::" + std::to_string(streamIndex_));
    isBlock_ = false;
    AUDIO_INFO_LOG("Enter");
    if (status_ == I_STATUS_INVALID) {
        return ERR_ILLEGAL_STATE;
    }
    if (status_ == I_STATUS_STARTED) {
        return SUCCESS;
    }
    status_ = I_STATUS_STARTED;
    isFirstFrame_ = true;
    isFirstNoUnderrunFrame_ = false;
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_STARTED);
    }
    return SUCCESS;
}

int32_t ProRendererStreamImpl::Pause(bool isStandby)
{
    Trace trace("ProRendererStreamImpl::Pause::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("Enter");
    if (status_ == I_STATUS_STARTED) {
        status_ = I_STATUS_PAUSED;
    }
    if (isFirstFrame_) {
        firstFrameSync_.notify_all();
    }
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_PAUSED);
    }
    return SUCCESS;
}

int32_t ProRendererStreamImpl::Flush()
{
    Trace trace("ProRendererStreamImpl::Flush::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("reset total bytes");
    {
        std::lock_guard lock(enqueueMutex);
        while (!readQueue_.empty()) {
            int32_t index = readQueue_.front();
            readQueue_.pop();
            writeQueue_.emplace(index);
        }
        if (isDrain_) {
            drainSync_.notify_all();
        }
    }
    for (auto &buffer : sinkBuffer_) {
        memset_s(buffer.data(), buffer.size(), 0, buffer.size());
    }
    sinkBytesWritten_ = 0;
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_FLUSHED);
    }
    return SUCCESS;
}

int32_t ProRendererStreamImpl::Drain(bool stopFlag)
{
    Trace trace("ProRendererStreamImpl::Drain::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("Enter");
    isDrain_ = true;
    if (!readQueue_.empty()) {
        std::unique_lock lock(enqueueMutex);
        drainSync_.wait_for(lock, std::chrono::milliseconds(DRAIN_WAIT_TIMEOUT_TIME),
            [this] { return readQueue_.empty(); });
    }
    isDrain_ = false;
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_DRAINED);
    }
    status_ = I_STATUS_DRAINED;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::Stop()
{
    Trace trace("ProRendererStreamImpl::Stop::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("Enter");
    status_ = I_STATUS_STOPPED;
    if (isFirstFrame_) {
        firstFrameSync_.notify_all();
    }
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_STOPPED);
    }
    return SUCCESS;
}

int32_t ProRendererStreamImpl::Release()
{
    Trace trace("ProRendererStreamImpl::Release::" + std::to_string(streamIndex_));
    AUDIO_INFO_LOG("Enter");
    status_ = I_STATUS_INVALID;
    isBlock_ = true;
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_RELEASED);
    }
    return SUCCESS;
}

int32_t ProRendererStreamImpl::GetStreamFramesWritten(uint64_t &framesWritten)
{
    CHECK_AND_RETURN_RET_LOG(byteSizePerFrame_ != 0, ERR_ILLEGAL_STATE, "Error frame size");
    framesWritten = totalBytesWritten_ / byteSizePerFrame_;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::GetCurrentTimeStamp(uint64_t &timestamp)
{
    int64_t timeSec = 0;
    int64_t timeNsec = 0;
    uint64_t framePosition;
    bool ret = GetAudioTime(framePosition, timeSec, timeNsec);
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "GetAudioTime error");
    timestamp = static_cast<uint64_t>(timeSec * AUDIO_NS_PER_S + timeNsec);
    return SUCCESS;
}

int32_t ProRendererStreamImpl::GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp, uint64_t &latency,
    int32_t base)
{
    int64_t timeSec = 0;
    int64_t timeNsec = 0;
    bool ret = GetAudioTime(framePosition, timeSec, timeNsec);
    CHECK_AND_RETURN_RET_LOG(ret, ERROR, "GetAudioTime error");
    int64_t stamp = 0;
    stamp = base == Timestamp::BOOTTIME ? ClockTime::GetBootNano() : ClockTime::GetCurNano();
    timestamp = stamp >= 0 ? stamp : 0;
    latency = 0;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::GetLatency(uint64_t &latency)
{
    CHECK_AND_RETURN_RET_LOG(byteSizePerFrame_ != 0, ERR_ILLEGAL_STATE, "Error frame size");
    uint64_t framePos = sinkBytesWritten_ / byteSizePerFrame_;
    latency = ((framePos / byteSizePerFrame_) * AUDIO_US_PER_S) / processConfig_.streamInfo.samplingRate;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::SetRate(int32_t rate)
{
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
    (void)currentRate;
    renderRate_ = rate;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::SetAudioEffectMode(int32_t effectMode)
{
    return SUCCESS;
}

int32_t ProRendererStreamImpl::GetAudioEffectMode(int32_t &effectMode)
{
    return SUCCESS;
}

int32_t ProRendererStreamImpl::SetPrivacyType(int32_t privacyType)
{
    privacyType_ = privacyType;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::GetPrivacyType(int32_t &privacyType)
{
    privacyType = privacyType_;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::SetSpeed(float speed)
{
    AUDIO_WARNING_LOG("not support");
    return ERR_NOT_SUPPORTED;
}

void ProRendererStreamImpl::RegisterStatusCallback(const std::weak_ptr<IStatusCallback> &callback)
{
    AUDIO_DEBUG_LOG("enter in");
    statusCallback_ = callback;
}

void ProRendererStreamImpl::RegisterWriteCallback(const std::weak_ptr<IWriteCallback> &callback)
{
    AUDIO_DEBUG_LOG("enter in");
    writeCallback_ = callback;
}

BufferDesc ProRendererStreamImpl::DequeueBuffer(size_t length)
{
    Trace trace("ProRendererStreamImpl::DequeueBuffer");
    BufferDesc bufferDesc = {nullptr, 0, 0};
    if (status_ != I_STATUS_STARTED) {
        return bufferDesc;
    }
    bufferDesc.buffer = reinterpret_cast<uint8_t *>(sinkBuffer_[0].data());
    bufferDesc.bufLength = sinkBuffer_[0].size();
    return bufferDesc;
}

int32_t ProRendererStreamImpl::EnqueueBuffer(const BufferDesc &bufferDesc)
{
    Trace trace("ProRendererStreamImpl::EnqueueBuffer::" + std::to_string(streamIndex_));
    int32_t writeIndex = PopWriteBufferIndex();
    CHECK_AND_RETURN_RET_LOG(writeIndex >= 0, ERR_WRITE_BUFFER, "write index is empty.");
    std::lock_guard lock(peekMutex);
    GetStreamVolume();
    if (processConfig_.streamInfo.encoding == ENCODING_EAC3) {
        auto error = memcpy_s(sinkBuffer_[writeIndex].data(), sinkBuffer_[writeIndex].size(), bufferDesc.buffer,
            bufferDesc.bufLength);
        if (error != EOK) {
            AUDIO_ERR_LOG("copy failed!");
        }
    } else {
        if (isNeedMcr_ && !isNeedResample_) {
            ConvertSrcToFloat(bufferDesc);
            downMixer_->Apply(spanSizeInFrame_, resampleSrcBuffer.data(), resampleDesBuffer.data());
            ConvertFloatToDes(writeIndex);
        } else if (isNeedMcr_ && isNeedResample_) {
            ConvertSrcToFloat(bufferDesc);
            downMixer_->Apply(spanSizeInFrame_, resampleSrcBuffer.data(), resampleSrcBuffer.data());
        }
        if (isNeedResample_) {
            if (!isNeedMcr_) {
                ConvertSrcToFloat(bufferDesc);
            }
            resample_->ProcessFloatResample(resampleSrcBuffer, resampleDesBuffer);
            DumpFileUtil::WriteDumpFile(dumpFile_, resampleDesBuffer.data(), resampleDesBuffer.size() * sizeof(float));
            ConvertFloatToDes(writeIndex);
        } else if (!isNeedMcr_) {
            bufferInfo_.bufLength = bufferDesc.bufLength;
            bufferInfo_.frameSize = bufferDesc.bufLength / bufferInfo_.samplePerFrame;
            bufferInfo_.buffer = bufferDesc.buffer;
            if (desFormat_ == AudioSampleFormat::SAMPLE_S16LE) {
                AudioCommonConverter::ConvertBufferTo16Bit(bufferInfo_, sinkBuffer_[writeIndex]);
            } else {
                AudioCommonConverter::ConvertBufferTo32Bit(bufferInfo_, sinkBuffer_[writeIndex]);
            }
        }
    }
    readQueue_.emplace(writeIndex);
    if (isFirstFrame_) {
        firstFrameSync_.notify_all();
    }
    AUDIO_DEBUG_LOG("buffer length:%{public}zu ,sink buffer length:%{public}zu", bufferDesc.bufLength,
        sinkBuffer_[0].size());
    totalBytesWritten_ += bufferDesc.bufLength;
    sinkBytesWritten_ += bufferDesc.bufLength;
    return SUCCESS;
}

int32_t ProRendererStreamImpl::GetMinimumBufferSize(size_t &minBufferSize) const
{
    minBufferSize = minBufferSize_;
    return SUCCESS;
}

void ProRendererStreamImpl::GetByteSizePerFrame(size_t &byteSizePerFrame) const
{
    byteSizePerFrame = byteSizePerFrame_;
}

void ProRendererStreamImpl::GetSpanSizePerFrame(size_t &spanSizeInFrame) const
{
    spanSizeInFrame = spanSizeInFrame_;
}

void ProRendererStreamImpl::SetStreamIndex(uint32_t index)
{
    AUDIO_INFO_LOG("Using index/sessionId %{public}d", index);
    streamIndex_ = index;
}

uint32_t ProRendererStreamImpl::GetStreamIndex()
{
    return streamIndex_;
}

// offload
int32_t ProRendererStreamImpl::SetOffloadMode(int32_t state, bool isAppBack)
{
    SetOffloadDisable();
    return SUCCESS;
}

int32_t ProRendererStreamImpl::UnsetOffloadMode()
{
    SetOffloadDisable();
    return SUCCESS;
}

int32_t ProRendererStreamImpl::GetOffloadApproximatelyCacheTime(uint64_t &timestamp, uint64_t &paWriteIndex,
    uint64_t &cacheTimeDsp, uint64_t &cacheTimePa)
{
    return SUCCESS;
}

int32_t ProRendererStreamImpl::OffloadSetVolume(float volume)
{
    return SUCCESS;
}

int32_t ProRendererStreamImpl::SetOffloadDataCallbackState(int32_t state)
{
    AUDIO_WARNING_LOG("SetOffloadDataCallbackState not support");
    return ERR_NOT_SUPPORTED;
}

size_t ProRendererStreamImpl::GetWritableSize()
{
    return writeQueue_.size() * minBufferSize_;
}
// offload end
int32_t ProRendererStreamImpl::UpdateSpatializationState(bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

AudioProcessConfig ProRendererStreamImpl::GetAudioProcessConfig() const noexcept
{
    return processConfig_;
}

bool ProRendererStreamImpl::GetAudioTime(uint64_t &framePos, int64_t &sec, int64_t &nanoSec)
{
    GetStreamFramesWritten(framePos);
    int64_t time = handleTimeModel_.GetTimeOfPos(framePos);
    int64_t deltaTime = DEFAULT_BUFFER_MICROSECOND; // note: 20ms
    time += deltaTime;
    sec = time / AUDIO_NS_PER_S;
    nanoSec = time % AUDIO_NS_PER_S;
    return true;
}

int32_t ProRendererStreamImpl::Peek(std::vector<char> *audioBuffer, int32_t &index)
{
    Trace trace("ProRendererStreamImpl::Peek::" + std::to_string(streamIndex_));
    int32_t result = SUCCESS;
    if (isBlock_) {
        return ERR_WRITE_BUFFER;
    }
    if (!readQueue_.empty()) {
        PopSinkBuffer(audioBuffer, index);
        return result;
    }

    std::shared_ptr<IWriteCallback> writeCallback = writeCallback_.lock();
    if (writeCallback != nullptr) {
        result = writeCallback->OnWriteData(minBufferSize_);
        switch (result) {
            // As a low-risk change, temporarily keep the previous behavior
            // and avoid enterring the err logic on underrun.
            case ERR_RENDERER_IN_SERVER_UNDERRUN: {
                auto statusCallback = statusCallback_.lock();
                if (statusCallback != nullptr && isFirstNoUnderrunFrame_) {
                    statusCallback->OnStatusUpdate(OPERATION_UNDERFLOW);
                }
                [[fallthrough]];
            }
            case SUCCESS: {
                PopSinkBuffer(audioBuffer, index);
                if (result != ERR_RENDERER_IN_SERVER_UNDERRUN) {
                    isFirstNoUnderrunFrame_ = true;
                    result = SUCCESS;
                }
                break;
            }
            default: {
                AUDIO_ERR_LOG("Write callback failed,result:%{public}d", result);
                return result;
            }
        }
    } else {
        AUDIO_ERR_LOG("Write callback is nullptr!");
        result = ERR_WRITE_BUFFER;
    }
    return result;
}

int32_t ProRendererStreamImpl::ReturnIndex(int32_t index)
{
    if (index < 0) {
        return SUCCESS;
    }
    std::lock_guard lock(enqueueMutex);
    writeQueue_.emplace(index);
    return SUCCESS;
}

int32_t ProRendererStreamImpl::SetClientVolume(float clientVolume)
{
    AUDIO_INFO_LOG("clientVolume: %{public}f", clientVolume);
    return SUCCESS;
}

int32_t ProRendererStreamImpl::SetLoudnessGain(float loudnessGain)
{
    AUDIO_WARNING_LOG("SetLoudnessGain only for hpae renderer stream");
    return ERR_PRO_STREAM_NOT_SUPPORTED;
}

int32_t ProRendererStreamImpl::UpdateMaxLength(uint32_t maxLength)
{
    return SUCCESS;
}

int32_t ProRendererStreamImpl::PopWriteBufferIndex()
{
    std::lock_guard lock(enqueueMutex);
    int32_t writeIndex = -1;
    if (!writeQueue_.empty()) {
        writeIndex = writeQueue_.front();
        writeQueue_.pop();
    }
    return writeIndex;
}

void ProRendererStreamImpl::PopSinkBuffer(std::vector<char> *audioBuffer, int32_t &index)
{
    if (readQueue_.empty() && isFirstFrame_) {
        std::unique_lock firstFrameLock(firstFrameMutex);
        firstFrameSync_.wait_for(firstFrameLock, std::chrono::milliseconds(FIRST_FRAME_TIMEOUT_TIME),
            [this] { return (!readQueue_.empty() || isBlock_); });
        if (!readQueue_.empty()) {
            isFirstFrame_ = false;
        }
    }
    std::lock_guard lock(enqueueMutex);
    if (!readQueue_.empty()) {
        index = readQueue_.front();
        readQueue_.pop();
        *audioBuffer = sinkBuffer_[index];
    }
    if (readQueue_.empty() && isDrain_) {
        drainSync_.notify_all();
    }
}

void ProRendererStreamImpl::SetOffloadDisable()
{
    std::shared_ptr<IStatusCallback> statusCallback = statusCallback_.lock();
    if (statusCallback != nullptr) {
        statusCallback->OnStatusUpdate(OPERATION_UNSET_OFFLOAD_ENABLE);
    }
}

void ProRendererStreamImpl::ConvertSrcToFloat(const BufferDesc &bufferDesc)
{
    bufferInfo_.buffer = bufferDesc.buffer;
    bufferInfo_.bufLength = bufferDesc.bufLength;
    bufferInfo_.frameSize = bufferDesc.bufLength / bufferInfo_.samplePerFrame;

    if (bufferInfo_.format == AudioSampleFormat::SAMPLE_F32LE) {
        AudioCommonConverter::ConvertFloatToFloatWithVolume(bufferInfo_, resampleSrcBuffer);
        return;
    }
    AUDIO_DEBUG_LOG("ConvertSrcToFloat resample buffer,samplePerFrame:%{public}d,size:%{public}zu",
        bufferInfo_.samplePerFrame, resampleSrcBuffer.size());
    AudioCommonConverter::ConvertBufferToFloat(bufferInfo_, resampleSrcBuffer);
}

void ProRendererStreamImpl::ConvertFloatToDes(int32_t writeIndex)
{
    uint32_t samplePerFrame = Util::GetSamplePerFrame(desFormat_);
    if (desFormat_ == AudioSampleFormat::SAMPLE_F32LE) {
        auto error = memcpy_s(sinkBuffer_[writeIndex].data(), sinkBuffer_[writeIndex].size(), resampleDesBuffer.data(),
            resampleDesBuffer.size() * samplePerFrame);
        if (error != EOK) {
            AUDIO_ERR_LOG("copy failed");
        }
        return;
    }
    AudioCommonConverter::ConvertFloatToAudioBuffer(resampleDesBuffer,
        reinterpret_cast<uint8_t *>(sinkBuffer_[writeIndex].data()), samplePerFrame);
}

void ProRendererStreamImpl::GetStreamVolume()
{
    if (processConfig_.streamType == STREAM_VOICE_COMMUNICATION) {
        bufferInfo_.volumeBg = 1;
        bufferInfo_.volumeEd = 1;
        return;
    }
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    bufferInfo_.volumeEd = AudioVolume::GetInstance()->GetVolume(streamIndex_, processConfig_.streamType,
        DEVICE_NAME, &volumes);
    bufferInfo_.volumeBg = volumes.volumeHistory;
    if (bufferInfo_.volumeBg != bufferInfo_.volumeEd) {
        AudioVolume::GetInstance()->SetHistoryVolume(streamIndex_, bufferInfo_.volumeEd);
        AudioVolume::GetInstance()->Monitor(streamIndex_, true);
        AUDIO_INFO_LOG("audio volume begin:%{public}f,end:%{public}f", bufferInfo_.volumeBg, bufferInfo_.volumeEd);
    }
}

void ProRendererStreamImpl::InitBasicInfo(const AudioStreamInfo &streamInfo)
{
    currentRate_ = streamInfo.samplingRate;
    desSamplingRate_ = GetDirectSampleRate(streamInfo.samplingRate);
    desFormat_ = GetDirectFormat(streamInfo.format);
    spanSizeInFrame_ = (streamInfo.samplingRate * DEFAULT_BUFFER_MILLISECOND) / SECOND_TO_MILLISECOND;
    byteSizePerFrame_ = Util::GetSamplePerFrame(streamInfo.format) * streamInfo.channels;
    minBufferSize_ = spanSizeInFrame_ * byteSizePerFrame_;
    handleTimeModel_.ConfigSampleRate(currentRate_);
    bufferInfo_.channelCount = streamInfo.channels;
    bufferInfo_.format = streamInfo.format;
    bufferInfo_.samplePerFrame = Util::GetSamplePerFrame(streamInfo.format);
    bufferInfo_.frameSize = spanSizeInFrame_;
}

void ProRendererStreamImpl::BlockStream() noexcept
{
    isBlock_ = true;
    AudioVolume::GetInstance()->SetHistoryVolume(streamIndex_, 0.f);
}
} // namespace AudioStandard
} // namespace OHOS
