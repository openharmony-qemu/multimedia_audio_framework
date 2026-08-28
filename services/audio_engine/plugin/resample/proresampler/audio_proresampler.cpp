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
#define LOG_TAG "AudioProResampler"
#endif
#include "audio_proresampler.h"
#include "audio_stream_info.h"
#include "securec.h"
#include "audio_engine_log.h"
#include <cinttypes>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t FRAME_LEN_20MS = (20 * SOURCE_PERIOD_SIZE);

constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t MAX_CHANNELS = 16;
constexpr uint32_t MAX_SAMPLE_RATE = 5644800; // plugin capability for audio editing; framework API stays 384000
constexpr uint32_t MIN_SAMPLE_RATE = SAMPLE_RATE_8000;
constexpr uint32_t MAX_FRAME_LEN = SAMPLE_RATE_384000 * 10; // frame length guard, covers 100ms at 5644800
constexpr uint32_t MAX_QUALITY = 10;
}
// Unified Process: accepts arbitrary input frame counts, outputs resampled data in one call.
// The caller (converter node) handles input accumulation and output splitting.
ProResampler::ProResampler(uint32_t inRate, uint32_t outRate, uint32_t channels, uint32_t quality)
    : inRate_(inRate), outRate_(outRate), channels_(channels), quality_(quality),
    expectedOutFrameLen_(outRate_ * FRAME_LEN_20MS / MS_PER_SECOND)
{
    CHECK_AND_RETURN(inRate != outRate);
    
    CHECK_AND_RETURN_LOG((inRate_ >= MIN_SAMPLE_RATE) && (inRate_ <= MAX_SAMPLE_RATE) &&
        (outRate_ >= MIN_SAMPLE_RATE) && (outRate_ <= MAX_SAMPLE_RATE),
        "resampler input and output sample rate should be within [8000, 5644800]. "
        "inRate_ %{public}d, outRate_ %{public}d is not valid", inRate_, outRate_);
    
    CHECK_AND_RETURN_LOG((channels_ > 0) && (channels_ <= MAX_CHANNELS), "invalid channel number: %{public}d, "
        "channel number should within [1, 16]", channels_);
    
    CHECK_AND_RETURN_LOG(quality <= MAX_QUALITY, "invalid quality level: %{public}d", quality);

    int32_t errRet = ConfigBufferSizeAndExpectedInFrameLen();
    CHECK_AND_RETURN_LOG(errRet == RESAMPLER_ERR_SUCCESS,
        "ProResampler construct reserve buff error code %{public}s", ErrCodeToString(errRet).c_str());

    state_ = SingleStagePolyphaseResamplerInit(channels_, inRate_, outRate_, quality_, &errRet);
    CHECK_AND_RETURN_LOG(state_, "Init failed! failed with error %{public}s.",
        ErrCodeToString(errRet).c_str());
    
    SingleStagePolyphaseResamplerSkipHalfTaps(state_);
    AUDIO_INFO_LOG("Init success inRate: %{public}d, outRate: %{public}d, channels: %{public}d, "
        "quality: %{public}d, tmpOutBuf size: %{public}zu.", inRate_, outRate_, channels_, quality_,
        tmpOutBuf_.size());
}

int32_t ProResampler::Process(const float *inBuffer, uint32_t inFrameLen, float *outBuffer,
    uint32_t outFrameLen)
{
    CHECK_AND_RETURN_RET_LOG(outBuffer != nullptr, RESAMPLER_ERR_INVALID_ARG, "out buffer ptr is nullptr");
    CHECK_AND_RETURN_RET_LOG(state_ != nullptr, RESAMPLER_ERR_ALLOC_FAILED, "resampler state is invalid");
    CHECK_AND_RETURN_RET_LOG(inBuffer != nullptr, RESAMPLER_ERR_INVALID_ARG, "in buffer ptr is nullptr");
    CHECK_AND_RETURN_RET_LOG((inFrameLen <= MAX_FRAME_LEN) && (outFrameLen <= MAX_FRAME_LEN),
        RESAMPLER_ERR_ALLOC_FAILED,
        "inFrameLen %{public}d or outFrameLen %{public}d out of valid range",
        inFrameLen,
        outFrameLen);

    AUDIO_DEBUG_LOG("Process unified: inRate=%{public}u->outRate=%{public}u, channels=%{public}u, "
        "inFrames=%{public}u, outFrames=%{public}u", inRate_, outRate_, channels_, inFrameLen, outFrameLen);

    uint32_t expectedOutFrameLen = outFrameLen;

    if (tmpOutBuf_.size() < static_cast<size_t>(expectedOutFrameLen) * channels_) {
        tmpOutBuf_.resize(static_cast<size_t>(expectedOutFrameLen) * channels_, 0.0f);
    }

    int32_t ret =
        SingleStagePolyphaseResamplerProcess(state_, inBuffer, &inFrameLen, tmpOutBuf_.data(), &outFrameLen);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "process failed with error %{public}s",
        ErrCodeToString(ret).c_str());

    uint32_t fillSize = expectedOutFrameLen > outFrameLen ? expectedOutFrameLen - outFrameLen : 0;
    lastDelayFrames_ = fillSize;
    ret = memset_s(outBuffer, fillSize * channels_ * sizeof(float), 0, fillSize * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memset_s failed with error %{public}d", ret);

    ret = memcpy_s(outBuffer + fillSize * channels_, (expectedOutFrameLen - fillSize) * channels_ * sizeof(float),
        tmpOutBuf_.data(), outFrameLen * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memcpy_s failed with error %{public}d", ret);
    return ret;
}

int32_t ProResampler::CheckRateAndChannelParam()
{
    // resampler change from valid state to invalid state
    if ((inRate_ < MIN_SAMPLE_RATE) || (inRate_ > MAX_SAMPLE_RATE) || (outRate_ < MIN_SAMPLE_RATE) ||
        (outRate_ > MAX_SAMPLE_RATE) || (inRate_ == outRate_) || (channels_ <= 0) || (channels_ > MAX_CHANNELS)) {
        AUDIO_ERR_LOG("resampler set to invalid state, because inRate %{public}u Hz and outRate %{public}u Hz "
            "should be within [8000, 5644800] and be different, or channel number %{public}u should be within [1, 16]",
            inRate_, outRate_, channels_);
        CHECK_AND_RETURN_RET(state_ != nullptr, RESAMPLER_ERR_INVALID_ARG);
        SingleStagePolyphaseResamplerFree(state_);
        state_ = nullptr;
        tmpOutBuf_.resize(0);
        return RESAMPLER_ERR_INVALID_ARG;
    }
    return RESAMPLER_ERR_SUCCESS;
}

int32_t ProResampler::UpdateRates(uint32_t inRate, uint32_t outRate)
{
    // log only when a rate really changes; same-value calls are no-ops
    if (inRate != inRate_ || outRate != outRate_) {
        AUDIO_INFO_LOG("ProResampler inRate update: %{public}d -> %{public}d, outRate update: %{public}d "
            "-> %{public}d", inRate_, inRate, outRate_, outRate);
    }
    inRate_ = inRate;
    outRate_ = outRate;

    int32_t ret = CheckRateAndChannelParam();
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret,
        "ProResampler updateRates error code %{public}s", ErrCodeToString(ret).c_str());

    expectedOutFrameLen_ = outRate_ * FRAME_LEN_20MS / MS_PER_SECOND;
    ret = ConfigBufferSizeAndExpectedInFrameLen();
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret,
        "ProResampler updateRates reserve buff error code %{public}s", ErrCodeToString(ret).c_str());
    if (state_ == nullptr) { // resampler can be updated from an invalid state to valid state
        int32_t errRet = RESAMPLER_ERR_SUCCESS;
        state_ = SingleStagePolyphaseResamplerInit(channels_, inRate_, outRate_, quality_, &errRet);
        CHECK_AND_RETURN_RET_LOG(state_ && (errRet == RESAMPLER_ERR_SUCCESS), errRet,
            "error code %{public}s", ErrCodeToString(errRet).c_str());
        return SingleStagePolyphaseResamplerSkipHalfTaps(state_);
    }
    ret = SingleStagePolyphaseResamplerSetRate(state_, inRate_, outRate_);
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret, "error code %{public}s", ErrCodeToString(ret).c_str());
    return ret;
}

int32_t ProResampler::UpdateChannels(uint32_t channels)
{
    // if update channel, the only way to update SingleStagePolyphaseResampler is to create a new one
    AUDIO_INFO_LOG("update work channel %{public}d -> %{public}d", channels_, channels);
    channels_ = channels;

    int32_t errRet = CheckRateAndChannelParam();
    CHECK_AND_RETURN_RET(errRet == RESAMPLER_ERR_SUCCESS, errRet,
        "ProResampler updateChannels error code %{public}s", ErrCodeToString(errRet).c_str());

    if (state_ != nullptr) {
        SingleStagePolyphaseResamplerFree(state_);
    }
    state_ = SingleStagePolyphaseResamplerInit(channels_, inRate_, outRate_, quality_, &errRet);
    CHECK_AND_RETURN_RET_LOG(state_ && (errRet == RESAMPLER_ERR_SUCCESS), errRet,
        "error code %{public}s", ErrCodeToString(errRet).c_str());
    errRet = ConfigBufferSizeAndExpectedInFrameLen();
    CHECK_AND_RETURN_RET_LOG(errRet == RESAMPLER_ERR_SUCCESS, errRet,
        "ProResampler updateChannels reserve buff error code %{public}s", ErrCodeToString(errRet).c_str());
    return SingleStagePolyphaseResamplerSkipHalfTaps(state_);
}

ProResampler::ProResampler(ProResampler &&other) noexcept
    : inRate_(other.inRate_), outRate_(other.outRate_), channels_(other.channels_),
    quality_(other.quality_), expectedOutFrameLen_(other.expectedOutFrameLen_),
    expectedInFrameLen_(other.expectedInFrameLen_), state_(other.state_),
    tmpOutBuf_(std::move(other.tmpOutBuf_))
{
    other.state_ = nullptr;
}

ProResampler &ProResampler::operator=(ProResampler &&other) noexcept
{
    if (this != &other) {
        SingleStagePolyphaseResamplerFree(state_);
        inRate_ = other.inRate_;
        outRate_ = other.outRate_;
        channels_ = other.channels_;
        quality_ = other.quality_;
        state_ = other.state_;
        expectedOutFrameLen_ = other.expectedOutFrameLen_;
        expectedInFrameLen_ = other.expectedInFrameLen_;
        tmpOutBuf_ = std::move(other.tmpOutBuf_);
        other.state_ = nullptr;
    }
    return *this;
}

void ProResampler::Reset()
{
    CHECK_AND_RETURN_LOG(state_ != nullptr, "resampler is null");
    SingleStagePolyphaseResamplerResetMem(state_);
    SingleStagePolyphaseResamplerSkipHalfTaps(state_);
    lastDelayFrames_ = 0;
}

uint32_t ProResampler::GetInRate() const
{
    return inRate_;
}

uint32_t ProResampler::GetOutRate() const
{
    return outRate_;
}

uint32_t ProResampler::GetChannels() const
{
    return channels_;
}

uint32_t ProResampler::GetQuality() const
{
    return quality_;
}

uint32_t ProResampler::GetDelayFrames() const
{
    return lastDelayFrames_;
}

ProResampler::~ProResampler()
{
    CHECK_AND_RETURN(state_ != nullptr);
    SingleStagePolyphaseResamplerFree(state_);
}

std::string ProResampler::ErrCodeToString(int32_t errCode)
{
    switch (errCode) {
        case RESAMPLER_ERR_SUCCESS: {
            return "RESAMPLER_ERR_SUCCESS";
            break;
        }
        case RESAMPLER_ERR_ALLOC_FAILED: {
            return "RESAMPLER_ERR_ALLOC_FAILED";
            break;
        }
        case RESAMPLER_ERR_INVALID_ARG: {
            return "RESAMPLER_ERR_INVALID_ARG";
            break;
        }
        case RESAMPLER_ERR_OVERFLOW: {
            return "RESAMPLER_ERR_OVERFLOW";
            break;
        }
        default: {
            return "Unknown Error Code";
        }
    }
}

int32_t ProResampler::ConfigBufferSizeAndExpectedInFrameLen()
{
    // tmpOutBuf_ sized for one resampler call output.
    // The caller is responsible for providing appropriate outFrameLen.
    size_t sizeNeed = static_cast<size_t>(expectedOutFrameLen_) * channels_;
    expectedInFrameLen_ = inRate_ * FRAME_LEN_20MS / MS_PER_SECOND;
    tmpOutBuf_.resize(sizeNeed, 0.0f);
    CHECK_AND_RETURN_RET_LOG(tmpOutBuf_.size() >= sizeNeed, RESAMPLER_ERR_ALLOC_FAILED,
        "tmpOutBuf_ size error, should be above %{public}zu, actually %{public}zu", sizeNeed, tmpOutBuf_.size());
    AUDIO_INFO_LOG("input %{public}u Hz, output %{public}u Hz, channels: %{public}u, tmpOutBuf size %{public}zu",
        inRate_, outRate_, channels_, tmpOutBuf_.size());
    return RESAMPLER_ERR_SUCCESS;
}

} // HPAE
} // AudioStandard
} // OHOS

// PCM <-> float conversion and memory length helpers for direct callers,
// declared in audio_proresampler_process.h.
namespace {
constexpr float PCM_FLOAT_EPS = 1e-6f;
constexpr int PCM_OFFSET_BIT_24 = 3;
constexpr int PCM_BIT_8 = 8;
constexpr int PCM_BIT_16 = 16;
constexpr int PCM_BIT_32 = 32;
constexpr int PCM_BYTES_S16 = 2;
constexpr int PCM_BYTES_S24 = 3;
constexpr int PCM_BYTES_S32 = 4;
// byte indexes of a little-endian 24-bit sample: LSB first, MSB last
constexpr int PCM_S24_BYTE_LSB = 0;
constexpr int PCM_S24_BYTE_MID = 1;
constexpr int PCM_S24_BYTE_MSB = 2;

uint32_t Read24Bit(const void *pcm)
{
    const uint8_t *p = static_cast<const uint8_t *>(pcm);
    return (static_cast<uint32_t>(p[PCM_S24_BYTE_MSB]) << PCM_BIT_16) |
        (static_cast<uint32_t>(p[PCM_S24_BYTE_MID]) << PCM_BIT_8) |
        static_cast<uint32_t>(p[PCM_S24_BYTE_LSB]);
}

void Write24Bit(void *pcm, uint32_t u)
{
    uint8_t *p = static_cast<uint8_t *>(pcm);
    p[PCM_S24_BYTE_MSB] = static_cast<uint8_t>(u >> PCM_BIT_16);
    p[PCM_S24_BYTE_MID] = static_cast<uint8_t>(u >> PCM_BIT_8);
    p[PCM_S24_BYTE_LSB] = static_cast<uint8_t>(u);
}

float CapMax(float v)
{
    if (v < -1.0f + PCM_FLOAT_EPS) {
        return -1.0f + PCM_FLOAT_EPS;
    }
    if (v > 1.0f - PCM_FLOAT_EPS) {
        return 1.0f - PCM_FLOAT_EPS;
    }
    return v;
}

void CopyFloat(int32_t n, const float *src, float *dst)
{
    errno_t ret = memcpy_s(dst, static_cast<size_t>(n) * sizeof(float), src, static_cast<size_t>(n) * sizeof(float));
    if (ret != 0) {
        for (int32_t i = 0; i < n; i++) {
            dst[i] = src[i];
        }
    }
}

int32_t GetPcmFormatBytes(AudioPcmFormat format)
{
    switch (format) {
        case AUDIO_PCM_S16_LE:
            return PCM_BYTES_S16;
        case AUDIO_PCM_S24_LE:
            return PCM_BYTES_S24;
        case AUDIO_PCM_S32_LE:
        case AUDIO_PCM_F32_LE:
            return PCM_BYTES_S32;
        default:
            return 0;
    }
}
} // namespace

extern "C" int32_t ConvertPcmToFloat(AudioPcmFormat format, const void *pcmData, int32_t sampleCount, float *outData)
{
    if (pcmData == nullptr || outData == nullptr || sampleCount <= 0) {
        AUDIO_ERR_LOG("ConvertPcmToFloat invalid params, sampleCount: %{public}d", sampleCount);
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }

    switch (format) {
        case AUDIO_PCM_S16_LE: {
            const int16_t *in = static_cast<const int16_t *>(pcmData);
            for (int32_t i = 0; i < sampleCount; i++) {
                outData[i] = in[i] * (1.0f / (1 << (PCM_BIT_16 - 1)));
            }
            return AUDIO_PCM_ALGO_SUCCESS;
        }
        case AUDIO_PCM_S24_LE: {
            const uint8_t *in = static_cast<const uint8_t *>(pcmData);
            for (int32_t i = 0; i < sampleCount; i++) {
                // shift in the unsigned domain: a signed left shift that overflows int32_t
                // is undefined behavior; the cast keeps the same 32-bit pattern
                int32_t s = static_cast<int32_t>(Read24Bit(in + i * PCM_OFFSET_BIT_24) << PCM_BIT_8);
                outData[i] = s * (1.0f / (1U << (PCM_BIT_32 - 1)));
            }
            return AUDIO_PCM_ALGO_SUCCESS;
        }
        case AUDIO_PCM_S32_LE: {
            const int32_t *in = static_cast<const int32_t *>(pcmData);
            for (int32_t i = 0; i < sampleCount; i++) {
                outData[i] = in[i] * (1.0f / (1U << (PCM_BIT_32 - 1)));
            }
            return AUDIO_PCM_ALGO_SUCCESS;
        }
        case AUDIO_PCM_F32_LE:
            CopyFloat(sampleCount, static_cast<const float *>(pcmData), outData);
            return AUDIO_PCM_ALGO_SUCCESS;
        default:
            AUDIO_ERR_LOG("ConvertPcmToFloat unsupported format: %{public}d", static_cast<int32_t>(format));
            return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
}

extern "C" int32_t ConvertFloatToPcm(AudioPcmFormat format, const float *inData, int32_t sampleCount, void *outPcm)
{
    if (inData == nullptr || outPcm == nullptr || sampleCount <= 0) {
        AUDIO_ERR_LOG("ConvertFloatToPcm invalid params, sampleCount: %{public}d", sampleCount);
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }

    switch (format) {
        case AUDIO_PCM_S16_LE: {
            int16_t *out = static_cast<int16_t *>(outPcm);
            for (int32_t i = 0; i < sampleCount; i++) {
                out[i] = static_cast<int16_t>(CapMax(inData[i]) * (1 << (PCM_BIT_16 - 1)));
            }
            return AUDIO_PCM_ALGO_SUCCESS;
        }
        case AUDIO_PCM_S24_LE: {
            uint8_t *out = static_cast<uint8_t *>(outPcm);
            for (int32_t i = 0; i < sampleCount; i++) {
                float v = CapMax(inData[i]) * (1U << (PCM_BIT_32 - 1));
                Write24Bit(out + i * PCM_OFFSET_BIT_24, static_cast<uint32_t>(static_cast<int32_t>(v)) >> PCM_BIT_8);
            }
            return AUDIO_PCM_ALGO_SUCCESS;
        }
        case AUDIO_PCM_S32_LE: {
            int32_t *out = static_cast<int32_t *>(outPcm);
            for (int32_t i = 0; i < sampleCount; i++) {
                out[i] = static_cast<int32_t>(CapMax(inData[i]) * (1U << (PCM_BIT_32 - 1)));
            }
            return AUDIO_PCM_ALGO_SUCCESS;
        }
        case AUDIO_PCM_F32_LE:
            CopyFloat(sampleCount, inData, static_cast<float *>(outPcm));
            return AUDIO_PCM_ALGO_SUCCESS;
        default:
            AUDIO_ERR_LOG("ConvertFloatToPcm unsupported format: %{public}d", static_cast<int32_t>(format));
            return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
}

extern "C" int32_t AudioFormatCalcMemLength(AudioPcmFormat format, int32_t channels, int32_t sampleCount)
{
    if (channels <= 0 || sampleCount < 0) {
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
    int32_t bytesPerSample = GetPcmFormatBytes(format);
    if (bytesPerSample == 0) {
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
    int64_t length = static_cast<int64_t>(sampleCount) * channels * bytesPerSample;
    if (length > INT32_MAX) {
        AUDIO_ERR_LOG("AudioFormatCalcMemLength overflow, length: %{public}" PRId64, length);
        return AUDIO_PCM_ALGO_ERR_INVALID_PARAM;
    }
    return static_cast<int32_t>(length);
}