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

#include "audio_speed.h"
#include "audio_common_log.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "sonic.h"

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
    if (sonicStream_ != nullptr) {
        sonicDestroyStream(sonicStream_);
        sonicStream_ = nullptr;
        AUDIO_INFO_LOG("Sonic stream destroy");
    }
}

int32_t AudioSpeed::OutputFrameCapacity() const
{
    // Sonic's read limit is frames; maxSpeedBufferSize_ is destination bytes.
    if (maxSpeedBufferSize_ <= 0 || formatSize_ == 0 || channels_ == 0 ||
        channels_ > static_cast<size_t>(maxSpeedBufferSize_) / formatSize_) {
        return 0;
    }
    return maxSpeedBufferSize_ / static_cast<int32_t>(formatSize_ * channels_);
}

int32_t AudioSpeed::Init()
{
    finished_ = false;
    LoadChangeSpeedFunc();
    CHECK_AND_RETURN_RET_LOG(OutputFrameCapacity() > 0, ERROR, "invalid speed output capacity");
    sonicStream_ = sonicCreateStream(rate_, channels_);
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr, ERROR, "sonicCreateStream failed.");
    finished_ = true; // A newly created stream has no input to finish.

    return SUCCESS;
}

int32_t AudioSpeed::LoadChangeSpeedFunc()
{
    switch (format_) {
        case SAMPLE_U8:
            formatSize_ = 1; // size is 1
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor8Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
            break;
        case SAMPLE_S16LE:
            formatSize_ = 2; // size is 2
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor16Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
            break;
        case SAMPLE_S24LE:
            formatSize_ = 3; // size is 3
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor24Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
            break;
        case SAMPLE_S32LE:
            formatSize_ = 4; // size is 4
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor32Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
            break;
        case SAMPLE_F32LE:
            formatSize_ = 4; // size is 4
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedForFloat(reinterpret_cast<float*>(buffer), bufferSize,
                        reinterpret_cast<float*>(outBuffer.get()), outBufferSize);
                };
            break;
        default:
            formatSize_ = 2; // size is 2
            ChangeSpeedFunc = [this] (uint8_t *buffer, int32_t bufferSize,
                std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)-> int32_t {
                    return this->ChangeSpeedFor16Bit(buffer, bufferSize, outBuffer, outBufferSize);
                };
    }
    AUDIO_INFO_LOG("load change speed func for format %{public}zu", format_);
    return SUCCESS;
}

int32_t AudioSpeed::SetSpeed(float speed)
{
    AUDIO_INFO_LOG("SetSpeed %{public}f", speed);
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr, ERROR, "speed stream is not initialized");
    speed_ = speed;
    sonicSetSpeed(sonicStream_, speed_);
    return SUCCESS;
}

int32_t AudioSpeed::SetPitch(float pitch)
{
    AUDIO_INFO_LOG("SetPitch %{public}f", pitch);
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr, ERROR, "speed stream is not initialized");
    sonicSetPitch(sonicStream_, pitch);
    sonicSetRate(sonicStream_, 1.0f);
    return SUCCESS;
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

float AudioSpeed::GetSpeed()
{
    return speed_;
}

int32_t AudioSpeed::ChangeSpeedFor8Bit(uint8_t *buffer, int32_t bufferSize,
    std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)
{
    outBufferSize = 0;
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr && OutputFrameCapacity() > 0 && buffer != nullptr &&
        bufferSize > 0 && bufferSize % (formatSize_ * channels_) == 0, ERROR, "invalid speed input");
    int32_t frames = bufferSize / static_cast<int32_t>(formatSize_ * channels_);
    CHECK_AND_RETURN_RET_LOG(sonicWriteUnsignedCharToStream(sonicStream_, buffer, frames) == 1,
        ERROR, "sonic write failed");
    finished_ = false;
    int32_t ret = ReadOutput(outBuffer, outBufferSize);
    return ret == SUCCESS ? bufferSize : ret;
}

int32_t AudioSpeed::ChangeSpeedFor16Bit(uint8_t *buffer, int32_t bufferSize,
    std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)
{
    outBufferSize = 0;
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr && OutputFrameCapacity() > 0 && buffer != nullptr &&
        bufferSize > 0 && bufferSize % (formatSize_ * channels_) == 0, ERROR, "invalid speed input");
    int32_t frames = bufferSize / static_cast<int32_t>(formatSize_ * channels_);
    CHECK_AND_RETURN_RET_LOG(sonicWriteShortToStream(sonicStream_, reinterpret_cast<short*>(buffer), frames) == 1,
        ERROR, "sonic write failed");
    finished_ = false;
    int32_t ret = ReadOutput(outBuffer, outBufferSize);
    return ret == SUCCESS ? bufferSize : ret;
}

int32_t AudioSpeed::ChangeSpeedFor24Bit(uint8_t *buffer, int32_t bufferSize,
    std::unique_ptr<uint8_t []> &outBuffer, int32_t &outBufferSize)
{
    Trace trace("AudioSpeed::ChangeSpeedFor24Bit");
    outBufferSize = 0;
    if (bufferSize <= 0 || channels_ == 0 || bufferSize % (formatSize_ * channels_) != 0) {
        AUDIO_ERR_LOG("BufferSize is illegal:%{public}d", bufferSize);
        return ERR_MEMORY_ALLOC_FAILED;
    }
    float *bitTofloat = new (std::nothrow) float[bufferSize / formatSize_];
    if (bitTofloat == nullptr) {
        AUDIO_ERR_LOG("bitTofloat nullptr, No memory");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    ConvertFrom24BitToFloat(bufferSize / formatSize_, buffer, bitTofloat);

    float *speedBuf = new (std::nothrow) float[static_cast<size_t>(OutputFrameCapacity()) * channels_];
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
    outBufferSize = 0;
    if (bufferSize <= 0 || channels_ == 0 || bufferSize % (formatSize_ * channels_) != 0) {
        AUDIO_ERR_LOG("BufferSize is illegal:%{public}d", bufferSize);
        return ERR_MEMORY_ALLOC_FAILED;
    }
    float *bitTofloat = new (std::nothrow) float[bufferSize / formatSize_];
    if (bitTofloat == nullptr) {
        AUDIO_ERR_LOG("bitTofloat nullptr, No memory");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    ConvertFrom32BitToFloat(bufferSize / formatSize_, reinterpret_cast<int32_t *>(buffer), bitTofloat);

    float *speedBuf = new (std::nothrow) float[static_cast<size_t>(OutputFrameCapacity()) * channels_];
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
    outBufferSize = 0;
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr && OutputFrameCapacity() > 0 && buffer != nullptr &&
        outBuffer != nullptr && bufferSize > 0 && bufferSize % (formatSize_ * channels_) == 0,
        ERROR, "invalid speed float input");
    int32_t frames = bufferSize / static_cast<int32_t>(formatSize_ * channels_);
    CHECK_AND_RETURN_RET_LOG(sonicWriteFloatToStream(sonicStream_, buffer, frames) == 1,
        ERROR, "sonic write float failed");
    finished_ = false;
    int32_t outputFrames = sonicReadFloatFromStream(sonicStream_, outBuffer, OutputFrameCapacity());
    CHECK_AND_RETURN_RET_LOG(outputFrames >= 0, ERROR, "sonic read float failed");
    outBufferSize = outputFrames * static_cast<int32_t>(formatSize_ * channels_);
    return bufferSize;
}

int32_t AudioSpeed::ReadOutput(std::unique_ptr<uint8_t[]> &outBuffer, int32_t &outBufferSize)
{
    outBufferSize = 0;
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr && OutputFrameCapacity() > 0 && outBuffer != nullptr,
        ERROR, "invalid speed output");
    if (sonicSamplesAvailable(sonicStream_) == 0) {
        return SUCCESS;
    }
    int32_t frames = 0;
    switch (format_) {
        case SAMPLE_U8:
            frames = sonicReadUnsignedCharFromStream(sonicStream_, outBuffer.get(), OutputFrameCapacity());
            break;
        case SAMPLE_S16LE:
            frames = sonicReadShortFromStream(sonicStream_, reinterpret_cast<short*>(outBuffer.get()),
                OutputFrameCapacity());
            break;
        case SAMPLE_S24LE:
        case SAMPLE_S32LE: {
            const size_t samples = static_cast<size_t>(OutputFrameCapacity()) * channels_;
            auto floats = std::unique_ptr<float[]>(new (std::nothrow) float[samples]);
            CHECK_AND_RETURN_RET_LOG(floats != nullptr, ERROR, "speed output allocation failed");
            frames = sonicReadFloatFromStream(sonicStream_, floats.get(), OutputFrameCapacity());
            if (format_ == SAMPLE_S24LE) {
                ConvertFromFloatTo24Bit(static_cast<size_t>(frames) * channels_, floats.get(), outBuffer.get());
            } else {
                ConvertFromFloatTo32Bit(static_cast<size_t>(frames) * channels_, floats.get(),
                    reinterpret_cast<int32_t*>(outBuffer.get()));
            }
            break;
        }
        case SAMPLE_F32LE:
            frames = sonicReadFloatFromStream(sonicStream_, reinterpret_cast<float*>(outBuffer.get()),
                OutputFrameCapacity());
            break;
        default:
            return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(frames > 0 && frames <= OutputFrameCapacity(), ERROR, "sonic output made no progress");
    outBufferSize = frames * static_cast<int32_t>(formatSize_ * channels_);
    return SUCCESS;
}

int32_t AudioSpeed::Finish()
{
    if (finished_) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(sonicStream_ != nullptr && sonicFlushStream(sonicStream_) == 1,
        ERROR, "sonic flush failed");
    finished_ = true;
    return SUCCESS;
}

int32_t AudioSpeed::Reconfigure(size_t rate, size_t format, size_t channels)
{
    const float pitch = sonicStream_ == nullptr ? SPEED_NORMAL : sonicGetPitch(sonicStream_);
    if (sonicStream_ != nullptr) {
        sonicDestroyStream(sonicStream_);
        sonicStream_ = nullptr;
    }
    rate_ = rate;
    format_ = format;
    channels_ = channels;
    CHECK_AND_RETURN_RET_LOG(Init() == SUCCESS, ERROR, "speed stream recreation failed");
    SetSpeed(speed_);
    return SetPitch(pitch);
}

int32_t AudioSpeed::Flush()
{
    return Reconfigure(rate_, format_, channels_);
}
} // namespace AudioStandard
} // namespace OHOS
