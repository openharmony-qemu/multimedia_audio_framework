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
#define LOG_TAG "AudioSpeed"
#endif

#include <cmath>
#include "audio_speed.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "audio_errors.h"
#ifndef AUDIO_SPEED_ALGO_ENABLE
#include "sonic.h"
#endif

namespace OHOS {
namespace AudioStandard {

static constexpr float SLOW_PLAY_1_8_SPEED = 0.125f;

AudioSpeed::AudioSpeed(size_t rate, size_t format, size_t channels, int32_t maxSpeedBufferSize)
    :rate_(rate), format_(format), channels_(channels),  maxSpeedBufferSize_(maxSpeedBufferSize)
{
    AUDIO_INFO_LOG("AudioSpeed construct");
    Init();
    streamParam_ = {};
}

AudioSpeed::~AudioSpeed()
{
    AUDIO_INFO_LOG("~AudioSpeed destroy");
#ifdef AUDIO_SPEED_ALGO_ENABLE
    if (timeStretch_ != nullptr) {
        timeStretch_.reset();
    }
#else
    if (sonicStream_ != nullptr) {
        sonicDestroyStream(sonicStream_);
        sonicStream_ = nullptr;
        AUDIO_INFO_LOG("Sonic stream destroy");
    }
#endif
}

int32_t AudioSpeed::Init()
{
#ifdef AUDIO_SPEED_ALGO_ENABLE
    if (AudioSuiteTimeStretch::IsTimeStretchSupported()) {
        timeStretch_ = AudioSuiteTimeStretch::Create(rate_, channels_,
            static_cast<AudioSampleFormat>(format_));
        if (timeStretch_ != nullptr) {
            LoadChangeSpeedFunc();
            AUDIO_INFO_LOG("AudioSpeed Init: using AudioSuiteTimeStretch");
            return SUCCESS;
        }
    }
    AUDIO_ERR_LOG("AudioSpeed Init: algo unavailable, no fallback");
    return ERROR;
#else
    sonicStream_ = sonicCreateStream(rate_, channels_);
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr, ERROR, "sonicCreateStream failed.");
    LoadChangeSpeedFunc();

    return SUCCESS;
#endif
}

#ifdef AUDIO_SPEED_ALGO_ENABLE
bool AudioSpeed::LoadAlgoChangeSpeedFunc()
{
    if (timeStretch_ == nullptr) {
        return false;
    }
    formatSize_ = PcmFormatToBits(static_cast<AudioSampleFormat>(format_));
    ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
        std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
        int32_t outSize = 0;
        int32_t ret = timeStretch_->Process(buffer, bufferSize,
            outBuffer.get(), maxSpeedBufferSize_, outSize);
        if (ret != SUCCESS) {
            outBufferSize = 0;
            return 0;
        }
        outBufferSize = outSize;
        return bufferSize;
    };
    AUDIO_INFO_LOG("load algo change speed func for format %{public}zu", format_);
    return true;
}
#endif

int32_t AudioSpeed::LoadChangeSpeedFunc()
{
#ifdef AUDIO_SPEED_ALGO_ENABLE
    if (LoadAlgoChangeSpeedFunc()) {
        return SUCCESS;
    }
    AUDIO_ERR_LOG("LoadChangeSpeedFunc: algo unavailable, no fallback");
    return ERROR;
#else
    return LoadSonicChangeSpeedFunc();
#endif
}

#ifndef AUDIO_SPEED_ALGO_ENABLE
int32_t AudioSpeed::LoadSonicChangeSpeedFunc()
{
    formatSize_ = PcmFormatToBits(static_cast<AudioSampleFormat>(format_));
    switch (format_) {
        case SAMPLE_U8:
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor8Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
            break;
        case SAMPLE_S16LE:
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor16Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
            break;
        case SAMPLE_S24LE:
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor24Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
            break;
        case SAMPLE_S32LE:
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor32Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
            break;
        case SAMPLE_F32LE:
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedForFloat(reinterpret_cast<float*>(buffer), bufferSize,
                        reinterpret_cast<float*>(outBuffer.get()), outBufferSize);
                };
            break;
        default:
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor16Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
    }
    AUDIO_INFO_LOG("load change speed func for format %{public}zu", format_);
    return SUCCESS;
}
#endif

int32_t AudioSpeed::SetSpeed(float speed)
{
    AUDIO_INFO_LOG("SetSpeed %{public}f", speed);
    speed_ = speed;
#ifdef AUDIO_SPEED_ALGO_ENABLE
    if (timeStretch_ != nullptr) {
        return timeStretch_->SetSpeed(speed);
    }
    AUDIO_WARNING_LOG("SetSpeed noop: timeStretch_ is null");
    return SUCCESS;
#else
    sonicSetSpeed(sonicStream_, speed_);
    return SUCCESS;
#endif
}

int32_t AudioSpeed::SetPitch(float pitch)
{
    AUDIO_INFO_LOG("SetPitch %{public}f", pitch);
#ifdef AUDIO_SPEED_ALGO_ENABLE
    if (timeStretch_ != nullptr) {
        return timeStretch_->SetPitch(pitch);
    }
    AUDIO_WARNING_LOG("SetPitch noop: timeStretch_ is null");
    return SUCCESS;
#else
    sonicSetPitch(sonicStream_, pitch);
    sonicSetRate(sonicStream_, 1.0f);
    return SUCCESS;
#endif
}

float AudioSpeed::GetPitchForSpeed(float speed)
{
    float noPitchPoint = 0.5f;
    float pitch = SPEED_NORMAL;
    if (speed > noPitchPoint) {
        pitch = SPEED_NORMAL;
    } else {
        pitch = (speed - SLOW_PLAY_1_8_SPEED) * (SPEED_NORMAL - SLOW_PLAY_1_8_SPEED) /
            (noPitchPoint - SLOW_PLAY_1_8_SPEED) + SLOW_PLAY_1_8_SPEED;
    }
    AUDIO_INFO_LOG("final pitch is %{public}f for speed %{public}f", pitch, speed);
    return pitch;
}

int32_t AudioSpeed::CalculateMaxSpeedBufferSize(size_t rate, size_t format, size_t channels, float speed)
{
    uint32_t bytesPerSample = PcmFormatToBits(static_cast<AudioSampleFormat>(format));
    if (bytesPerSample == 0 || rate == 0 || channels == 0) {
        AUDIO_INFO_LOG("CalculateMaxSpeedBufferSize invalid params, fallback: rate=%{public}zu, "
            "channels=%{public}zu, speed=%{public}f", rate, channels, speed);
        return MAX_SPEED_BUFFER_SIZE;
    }
    // cacheBufferSize: bytes of MAX_CBBUF_IN_USEC (100ms) duration for the actual stream format,
    // consistent with RendererInClientInner::SetPreferredFrameSize.
    int64_t sizePerFrame = static_cast<int64_t>(channels) * static_cast<int64_t>(bytesPerSample);
    int64_t cacheBufferSize = static_cast<int64_t>(MAX_CBBUF_IN_USEC) * static_cast<int64_t>(rate)
        * sizePerFrame / static_cast<int64_t>(AUDIO_US_PER_S);
    int32_t factor = 1;
    if (speed > 0.0f && speed < SPEED_NORMAL) {
        factor = static_cast<int32_t>(std::ceil(1.0f / speed)) + 1;
    }
    int64_t total = cacheBufferSize * static_cast<int64_t>(factor);
    if (total <= 0 || total > static_cast<int64_t>(INT32_MAX)) {
        AUDIO_INFO_LOG("CalculateMaxSpeedBufferSize overflow, fallback: rate=%{public}zu, "
            "channels=%{public}zu, speed=%{public}f, factor=%{public}d", rate, channels, speed, factor);
        return MAX_SPEED_BUFFER_SIZE;
    }
    AUDIO_INFO_LOG("CalculateMaxSpeedBufferSize: rate=%{public}zu, channels=%{public}zu, "
        "speed=%{public}f, factor=%{public}d, size=%{public}d", rate, channels, speed, factor,
        static_cast<int32_t>(total));
    return static_cast<int32_t>(total);
}

float AudioSpeed::GetSpeed()
{
    return speed_;
}

#ifndef AUDIO_SPEED_ALGO_ENABLE
int32_t AudioSpeed::ChangeSpeedFor8Bit(uint8_t *buffer, int32_t bufferSize,
    std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)
{
    Trace trace("AudioSpeed::ChangeSpeedFor8Bit");
    int32_t numSamples = bufferSize / static_cast<int32_t>(formatSize_ * channels_);
    int32_t res = sonicWriteUnsignedCharToStream(sonicStream_, static_cast<unsigned char*>(buffer), numSamples);
    CHECK_AND_RETURN_RET_LOG(res == 1, 0, "sonic write unsigned char to stream failed.");

    int32_t maxSamples = maxSpeedBufferSize_ / static_cast<int32_t>(formatSize_ * channels_);
    int32_t outSamples = sonicReadUnsignedCharFromStream(sonicStream_,
        static_cast<unsigned char*>(outBuffer.get()), maxSamples);
    CHECK_AND_RETURN_RET_LOG(outSamples != 0, bufferSize, "sonic stream is not full continue to write.");

    outBufferSize = outSamples * static_cast<int32_t>(formatSize_ * channels_);
    return bufferSize;
}

int32_t AudioSpeed::ChangeSpeedFor16Bit(uint8_t *buffer, int32_t bufferSize,
    std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)
{
    Trace trace("AudioSpeed::ChangeSpeedFor16Bit");
    int32_t numSamples = bufferSize / static_cast<int32_t>(formatSize_ * channels_);
    int32_t res = sonicWriteShortToStream(sonicStream_, reinterpret_cast<short*>(buffer), numSamples);
    CHECK_AND_RETURN_RET_LOG(res == 1, 0, "sonic write short to stream failed.");

    int32_t maxSamples = maxSpeedBufferSize_ / static_cast<int32_t>(formatSize_ * channels_);
    int32_t outSamples = sonicReadShortFromStream(sonicStream_, reinterpret_cast<short*>(outBuffer.get()),
        maxSamples);
    CHECK_AND_RETURN_RET_LOG(outSamples != 0, bufferSize, "sonic stream is not full continue to write.");

    outBufferSize = outSamples * static_cast<int32_t>(formatSize_ * channels_);
    return bufferSize;
}

int32_t AudioSpeed::ChangeSpeedFor24Bit(uint8_t *buffer, int32_t bufferSize,
    std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)
{
    Trace trace("AudioSpeed::ChangeSpeedFor24Bit");
    if (bufferSize <= 0 || bufferSize > maxSpeedBufferSize_) {
        AUDIO_ERR_LOG("BufferSize is illegal:%{public}d", bufferSize);
        return ERR_MEMORY_ALLOC_FAILED;
    }
    float *bitTofloat = new (std::nothrow) float[bufferSize];
    if (bitTofloat == nullptr) {
        AUDIO_ERR_LOG("bitTofloat nullptr, No memory");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    ConvertFrom24BitToFloat(bufferSize / formatSize_, buffer, bitTofloat);

    float *speedBuf = new (std::nothrow) float[maxSpeedBufferSize_];
    if (speedBuf == nullptr) {
        AUDIO_ERR_LOG("speedBuf nullptr, No memory");
        delete [] bitTofloat;
        return ERR_MEMORY_ALLOC_FAILED;
    }
    int32_t ret = ChangeSpeedForFloat(bitTofloat, bufferSize, speedBuf, outBufferSize);

    ConvertFromFloatTo24Bit(outBufferSize / formatSize_, speedBuf, outBuffer.get());

    delete [] bitTofloat;
    delete [] speedBuf;
    return ret;
}

int32_t AudioSpeed::ChangeSpeedFor32Bit(uint8_t *buffer, int32_t bufferSize,
    std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)
{
    Trace trace("AudioSpeed::ChangeSpeedFor32Bit");
    if (bufferSize <= 0 || bufferSize > maxSpeedBufferSize_) {
        AUDIO_ERR_LOG("BufferSize is illegal:%{public}d", bufferSize);
        return ERR_MEMORY_ALLOC_FAILED;
    }
    float *bitTofloat = new (std::nothrow) float[bufferSize];
    if (bitTofloat == nullptr) {
        AUDIO_ERR_LOG("bitTofloat nullptr, No memory");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    ConvertFrom32BitToFloat(bufferSize / formatSize_, reinterpret_cast<int32_t *>(buffer), bitTofloat);

    float *speedBuf = new (std::nothrow) float[maxSpeedBufferSize_];
    if (speedBuf == nullptr) {
        AUDIO_ERR_LOG("speedBuf nullptr, No memory");
        delete [] bitTofloat;
        return ERR_MEMORY_ALLOC_FAILED;
    }
    int32_t ret = ChangeSpeedForFloat(bitTofloat, bufferSize, speedBuf, outBufferSize);

    ConvertFromFloatTo32Bit(outBufferSize / formatSize_, speedBuf, reinterpret_cast<int32_t *>(outBuffer.get()));

    delete [] bitTofloat;
    delete [] speedBuf;
    return ret;
}

int32_t AudioSpeed::ChangeSpeedForFloat(float *buffer, int32_t bufferSize,
    float* outBuffer, int32_t &outBufferSize)
{
    Trace trace("AudioSpeed::ChangeSpeedForFloat");
    int32_t numSamples = bufferSize / static_cast<int32_t>(formatSize_ * channels_);
    int32_t res = static_cast<int32_t>(sonicWriteFloatToStream(sonicStream_, buffer, numSamples));
    CHECK_AND_RETURN_RET_LOG(res == 1, 0, "sonic write float to stream failed.");
    int32_t maxSamples = maxSpeedBufferSize_ / static_cast<int32_t>(channels_ * formatSize_);
    int32_t outSamples = sonicReadFloatFromStream(sonicStream_, outBuffer, maxSamples);
    outBufferSize = outSamples * static_cast<int32_t>(formatSize_ * channels_);
    return bufferSize;
}
#endif // AUDIO_SPEED_ALGO_ENABLE

int32_t AudioSpeed::Flush()
{
    Trace trace("AudioSpeed::Flush");
#ifdef AUDIO_SPEED_ALGO_ENABLE
    if (timeStretch_ != nullptr) {
        std::unique_ptr<uint8_t[]> flushBuffer = std::make_unique<uint8_t[]>(maxSpeedBufferSize_);
        CHECK_AND_RETURN_RET_LOG(flushBuffer != nullptr, ERR_MEMORY_ALLOC_FAILED, "flushBuffer alloc failed");
        int32_t outSize = 0;
        int32_t residualFrames = 0;
        return timeStretch_->Flush(flushBuffer.get(), maxSpeedBufferSize_, outSize, residualFrames);
    }
    AUDIO_WARNING_LOG("Flush noop: timeStretch_ is null");
    return SUCCESS;
#else
    sonicFlushStream(sonicStream_);
    std::unique_ptr<uint8_t[]> tmpBuffer = std::make_unique<uint8_t[]>(maxSpeedBufferSize_);

    int samplesWritten = 0;
    const size_t channelMultiplier = static_cast<size_t>(channels_);
    do {
        switch (format_) {
            case SAMPLE_U8:
                samplesWritten = sonicReadUnsignedCharFromStream(sonicStream_,
                    reinterpret_cast<uint8_t*>(tmpBuffer.get()),
                    maxSpeedBufferSize_ / channelMultiplier);
                break;
            case SAMPLE_S24LE:
            case SAMPLE_S32LE:
            case SAMPLE_F32LE:
                samplesWritten = sonicReadFloatFromStream(sonicStream_,
                    reinterpret_cast<float*>(tmpBuffer.get()),
                    maxSpeedBufferSize_ / (channelMultiplier * sizeof(float)));
                break;
            case SAMPLE_S16LE:
                samplesWritten = sonicReadShortFromStream(sonicStream_,
                    reinterpret_cast<short*>(tmpBuffer.get()),
                    maxSpeedBufferSize_ / (channelMultiplier * sizeof(short)));
                break;
            default:
                AUDIO_ERR_LOG("invalid format_");
                samplesWritten = 0;
                break;
        }
    } while (samplesWritten > 0);
    return SUCCESS;
#endif
}

int32_t AudioSpeed::FlushSpeedBuffer(uint8_t *outBuffer, int32_t outCapacity, int32_t &outSize)
{
    Trace trace("AudioSpeed::FlushSpeedBuffer");
    outSize = 0;
    CHECK_AND_RETURN_RET_LOG(outBuffer != nullptr && outCapacity > 0, ERR_INVALID_PARAM,
        "FlushSpeedBuffer invalid params");
#ifdef AUDIO_SPEED_ALGO_ENABLE
    if (timeStretch_ != nullptr) {
        int32_t residualFrames = 0;
        return timeStretch_->Flush(outBuffer, outCapacity, outSize, residualFrames);
    }
    AUDIO_WARNING_LOG("FlushSpeedBuffer noop: timeStretch_ is null");
    return SUCCESS;
#else
    return FlushSonicSpeedBuffer(outBuffer, outCapacity, outSize);
#endif
}

#ifndef AUDIO_SPEED_ALGO_ENABLE
int32_t AudioSpeed::FlushSonicSpeedBuffer(uint8_t *outBuffer, int32_t outCapacity, int32_t &outSize)
{
    sonicFlushStream(sonicStream_);
    int32_t frameSize = static_cast<int32_t>(formatSize_ * channels_);
    CHECK_AND_RETURN_RET_LOG(frameSize > 0, ERR_ILLEGAL_STATE, "FlushSpeedBuffer invalid frameSize");
    int32_t offset = 0;
    while (offset < outCapacity) {
        int32_t remainingBytes = outCapacity - offset;
        int32_t maxSamples = remainingBytes / frameSize;
        if (maxSamples <= 0) {
            break;
        }
        int32_t samplesRead = 0;
        switch (format_) {
            case SAMPLE_U8:
                samplesRead = sonicReadUnsignedCharFromStream(sonicStream_,
                    outBuffer + offset, maxSamples);
                break;
            case SAMPLE_S16LE:
                samplesRead = sonicReadShortFromStream(sonicStream_,
                    reinterpret_cast<short*>(outBuffer + offset), maxSamples);
                break;
            case SAMPLE_S24LE:
            case SAMPLE_S32LE:
            case SAMPLE_F32LE:
                samplesRead = sonicReadFloatFromStream(sonicStream_,
                    reinterpret_cast<float*>(outBuffer + offset), maxSamples);
                break;
            default:
                AUDIO_ERR_LOG("invalid format_");
                samplesRead = 0;
                break;
        }
        if (samplesRead <= 0) {
            break;
        }
        offset += samplesRead * frameSize;
    }
    outSize = offset;
    return SUCCESS;
}
#endif
} // namespace AudioStandard
} // namespace OHOS
