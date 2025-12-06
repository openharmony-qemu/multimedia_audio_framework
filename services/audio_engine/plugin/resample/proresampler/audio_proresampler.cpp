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

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t BUFFER_EXPAND_SIZE_2 = 2;
constexpr uint32_t BUFFER_EXPAND_SIZE_5 = 5;
constexpr uint32_t SAMPLE_RATE_11025 = 11025;
constexpr uint32_t FRAME_LEN_20MS = 20;
constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t ADD_SIZE = 100;
static constexpr uint32_t CUSTOM_SAMPLE_RATE_MULTIPLES = 50;
constexpr uint32_t MAX_CHANNELS = 16;
constexpr uint32_t MAX_SAMPLE_RATE = SAMPLE_RATE_384000;
constexpr uint32_t MIN_SAMPLE_RATE = SAMPLE_RATE_8000;
constexpr uint32_t MAX_FRAME_LEN = SAMPLE_RATE_384000 * 10; // max frame size, max sample rate, 10s duration
constexpr uint32_t MAX_QUALITY = 10;
// for now ProResampler accept input 20ms for other sample rates, 40ms input for 11025hz
// 100ms input for 10Hz resolution rates that are not multiples of 50, eg. 8010, 8020, 8030, 8040...
// however 8050, 8100, 8150... are for 20ms
// output 20ms for all sample rates
ProResampler::ProResampler(uint32_t inRate, uint32_t outRate, uint32_t channels, uint32_t quality)
    : inRate_(inRate), outRate_(outRate), channels_(channels), quality_(quality),
    expectedOutFrameLen_(outRate_ * FRAME_LEN_20MS / MS_PER_SECOND)
{
    CHECK_AND_RETURN_LOG(inRate != outRate,
        "input and output rate of ProResampler should be different! Same Rate: %{public}d", inRate);
    
    CHECK_AND_RETURN_LOG((inRate_ >= MIN_SAMPLE_RATE) && (inRate_ <= MAX_SAMPLE_RATE) &&
        (outRate_ >= MIN_SAMPLE_RATE) && (outRate_ <= MAX_SAMPLE_RATE),
        "resampler input and output sample rate should be within [8000, 384000]. "
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
        "quality: %{public}d.", inRate_, outRate_, channels_, quality_);
}

int32_t ProResampler::Process(const float *inBuffer, uint32_t inFrameLen, float *outBuffer,
    uint32_t outFrameLen)
{
    CHECK_AND_RETURN_RET_LOG(outBuffer != nullptr, RESAMPLER_ERR_INVALID_ARG, "out buffer ptr is nullptr");
    CHECK_AND_RETURN_RET_LOG(state_ != nullptr, RESAMPLER_ERR_ALLOC_FAILED, "resampler state is invalid");
    CHECK_AND_RETURN_RET_LOG((inFrameLen <= MAX_FRAME_LEN) && (outFrameLen <= MAX_FRAME_LEN),
        RESAMPLER_ERR_ALLOC_FAILED,
        "inFrameLen %{public}d or outFrameLen %{public}d out of valid range",
        inFrameLen,
        outFrameLen);
        
    if (inRate_ == SAMPLE_RATE_11025) {
        return Process11025SampleRate(inBuffer, inFrameLen, outBuffer, outFrameLen);
    } else if (inRate_ % CUSTOM_SAMPLE_RATE_MULTIPLES != 0) {
        return Process10HzSampleRate(inBuffer, inFrameLen, outBuffer, outFrameLen);
    } else {
        return ProcessOtherSampleRate(inBuffer, inFrameLen, outBuffer, outFrameLen);
    }
}

int32_t ProResampler::ProcessOtherSampleRate(const float *inBuffer, uint32_t inFrameLen, float *outBuffer,
    uint32_t outFrameLen)
{
    CHECK_AND_RETURN_RET_LOG(inBuffer != nullptr, RESAMPLER_ERR_INVALID_ARG, "in buffer ptr is nullptr");
    uint32_t expectedOutFrameLen = outFrameLen;
    std::vector<float> tmpOutBuf(expectedOutFrameLen * channels_, 0.0f);
    int32_t ret =
        SingleStagePolyphaseResamplerProcess(state_, inBuffer, &inFrameLen, tmpOutBuf.data(), &outFrameLen);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "process failed with error %{public}s",
        ErrCodeToString(ret).c_str());
    
    uint32_t fillSize = expectedOutFrameLen > outFrameLen ? expectedOutFrameLen - outFrameLen : 0;
    ret = memset_s(outBuffer, fillSize * channels_ * sizeof(float), 0, fillSize * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memset_s failed with error %{public}d", ret);

    ret = memcpy_s(outBuffer + fillSize * channels_,
        (expectedOutFrameLen - fillSize) * channels_ * sizeof(float),
        tmpOutBuf.data(), outFrameLen * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memcpy_s failed with error %{public}d", ret);
    return ret;
}

int32_t ProResampler::Process11025SampleRate(const float *inBuffer, uint32_t inFrameLen, float *outBuffer,
    uint32_t outFrameLen)
{
    CHECK_AND_RETURN_RET_LOG(outFrameLen >= expectedOutFrameLen_, RESAMPLER_ERR_INVALID_ARG,
        "output frame size %{public}d is not valid", outFrameLen);
    CHECK_AND_RETURN_RET_LOG(((inFrameLen == 0) || (inFrameLen == expectedInFrameLen_)), RESAMPLER_ERR_INVALID_ARG,
        "input frame size %{public}d is not valid", inFrameLen);
    int32_t ret = RESAMPLER_ERR_SUCCESS;
    if (inFrameLen == 0) {
        if (buf11025Index_ > 0) { // output second half of 11025 buffer
            CHECK_AND_RETURN_RET_LOG(buf11025Index_ <= buf11025_.capacity() - expectedOutFrameLen_ * channels_,
                RESAMPLER_ERR_OVERFLOW, "buf11025 overflow detected, required %{public}u, available %{public}zu",
                buf11025Index_ + expectedOutFrameLen_ * channels_, buf11025_.capacity());
            ret = memcpy_s(outBuffer, outFrameLen * channels_ * sizeof(float),
                buf11025_.data() + buf11025Index_,  expectedOutFrameLen_ * channels_ * sizeof(float));
            CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memcpy_s failed with error %{public}d", ret);
            
            ret = memset_s(buf11025_.data(), buf11025_.capacity() * sizeof(float), 0,
                buf11025_.capacity() * sizeof(float));
            buf11025Index_ = 0;
        } else { // no data left in buffer, the only thing can be done is to return 0s
            ret = memset_s(outBuffer, outFrameLen * channels_ * sizeof(float), 0,
                outFrameLen * channels_ * sizeof(float));
        }
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(inBuffer != nullptr, RESAMPLER_ERR_INVALID_ARG, "in buffer ptr is nullptr");

    std::vector<float> tmpOutBuf(expectedOutFrameLen_ * channels_ * BUFFER_EXPAND_SIZE_2, 0.0f);
    uint32_t tmpOutFrameLen = expectedOutFrameLen_ * BUFFER_EXPAND_SIZE_2;
    uint32_t reserveOutFrameLen = tmpOutFrameLen;
    ret = SingleStagePolyphaseResamplerProcess(state_, inBuffer, &inFrameLen, tmpOutBuf.data(), &tmpOutFrameLen);
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret, "Process failed with error %{public}s",
        ErrCodeToString(ret).c_str());
    uint32_t fillSize = reserveOutFrameLen > tmpOutFrameLen ? reserveOutFrameLen - tmpOutFrameLen : 0;
    ret = memset_s(buf11025_.data(), fillSize * channels_ * sizeof(float), 0, fillSize * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memset_s failed with error %{public}d", ret);

    CHECK_AND_RETURN_RET_LOG(static_cast<uint64_t>(tmpOutFrameLen) * channels_ <= buf11025_.capacity(),
        RESAMPLER_ERR_OVERFLOW, "buf11025 overflow detected, required %{public}u, available %{public}zu",
        tmpOutFrameLen * channels_, buf11025_.capacity());
    ret = memcpy_s(buf11025_.data() + fillSize * channels_,
        (reserveOutFrameLen - fillSize) * channels_ * sizeof(float),
        tmpOutBuf.data(), tmpOutFrameLen * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memcpy_s failed with error %{public}d", ret);

    // output first half of data
    ret = memcpy_s(outBuffer, outFrameLen * channels_ * sizeof(float),
        buf11025_.data(), expectedOutFrameLen_ * channels_ * sizeof(float));
    buf11025Index_ = expectedOutFrameLen_ * channels_;
    return ret;
}

// Process 10Hz resolution custom sample rate that is not multiples of 50, 100ms input
int32_t ProResampler::Process10HzSampleRate(const float *inBuffer, uint32_t inFrameLen, float *outBuffer,
    uint32_t outFrameLen)
{
    CHECK_AND_RETURN_RET_LOG(outFrameLen >= expectedOutFrameLen_, RESAMPLER_ERR_INVALID_ARG,
        "output frame size %{public}d is not valid", outFrameLen);
    CHECK_AND_RETURN_RET_LOG(((inFrameLen == 0) || (inFrameLen == expectedInFrameLen_)), RESAMPLER_ERR_INVALID_ARG,
        "input frame size %{public}d is not valid", inFrameLen);
    int32_t ret = RESAMPLER_ERR_SUCCESS;
    if (inFrameLen == 0) {
        if (bufFor100msIndex_ > 0) { // output 2nd, 3rd, 4th, 5th part of 100ms buffer
            CHECK_AND_RETURN_RET_LOG(bufFor100msIndex_ <= bufFor100ms_.capacity() - expectedOutFrameLen_ * channels_,
                RESAMPLER_ERR_OVERFLOW, "bufFor100ms overflow detected, required %{public}u, available %{public}zu",
                bufFor100msIndex_ + expectedOutFrameLen_ * channels_, bufFor100ms_.capacity());
            ret = memcpy_s(outBuffer, outFrameLen * channels_ * sizeof(float),
                bufFor100ms_.data() + bufFor100msIndex_,  expectedOutFrameLen_ * channels_ * sizeof(float));
            CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memcpy_s failed with error %{public}d", ret);
            bufFor100msIndex_ += expectedOutFrameLen_ * channels_;
            if (bufFor100msIndex_ >= BUFFER_EXPAND_SIZE_5 * expectedOutFrameLen_ * channels_) {
                bufFor100msIndex_ = 0;
                ret = memset_s(bufFor100ms_.data(), bufFor100ms_.capacity() * sizeof(float), 0,
                    bufFor100ms_.capacity() * sizeof(float));
            }
        } else { // no data left in buffer, the only thing can be done is to return 0s
            ret = memset_s(outBuffer, outFrameLen * channels_ * sizeof(float), 0,
                outFrameLen * channels_ * sizeof(float));
        }
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(inBuffer != nullptr, RESAMPLER_ERR_INVALID_ARG, "in buffer ptr is nullptr");

    std::vector<float> tmpOutBuf(expectedOutFrameLen_ * channels_ * BUFFER_EXPAND_SIZE_5, 0.0f);
    uint32_t tmpOutFrameLen = expectedOutFrameLen_ * BUFFER_EXPAND_SIZE_5;
    uint32_t reserveOutFrameLen = tmpOutFrameLen;
    ret = SingleStagePolyphaseResamplerProcess(state_, inBuffer, &inFrameLen, tmpOutBuf.data(), &tmpOutFrameLen);
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret, "Process failed with error %{public}s",
        ErrCodeToString(ret).c_str());

    uint32_t fillSize = reserveOutFrameLen > tmpOutFrameLen ? reserveOutFrameLen - tmpOutFrameLen : 0;
    ret = memset_s(bufFor100ms_.data(), fillSize * channels_ * sizeof(float), 0, fillSize * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memset_s failed with error %{public}d", ret);

    CHECK_AND_RETURN_RET_LOG(static_cast<uint64_t>(tmpOutFrameLen) * channels_ <= bufFor100ms_.capacity(),
        RESAMPLER_ERR_OVERFLOW, "bufFor100ms overflow detected, required %{public}u, available %{public}zu",
        tmpOutFrameLen * channels_, bufFor100ms_.capacity());
    ret = memcpy_s(bufFor100ms_.data() + fillSize * channels_,
        (reserveOutFrameLen - fillSize) * channels_ * sizeof(float),
        tmpOutBuf.data(), tmpOutFrameLen * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memcpy_s failed with error %{public}d", ret);

    // output 1st part of data
    ret = memcpy_s(outBuffer, outFrameLen * channels_ * sizeof(float),
        bufFor100ms_.data(), expectedOutFrameLen_ * channels_ * sizeof(float));
    bufFor100msIndex_ = expectedOutFrameLen_ * channels_;
    return ret;
}

int32_t ProResampler::UpdateRates(uint32_t inRate, uint32_t outRate)
{
    AUDIO_INFO_LOG("ProResampler inRate update: %{public}d -> %{public}d, outRate update: %{public}d -> %{public}d",
        inRate_, inRate, outRate_, outRate);
    inRate_ = inRate;
    outRate_ = outRate;
    // resampler change from valid state to invalid state
    if ((inRate < MIN_SAMPLE_RATE) || (inRate > MAX_SAMPLE_RATE) || (outRate < MIN_SAMPLE_RATE) ||
        (outRate > MAX_SAMPLE_RATE) || (inRate_ == outRate_)) {
        AUDIO_ERR_LOG("resampler set to invalid state, input and output sample rate should be within [8000, 384000]"
            "and be different");
        CHECK_AND_RETURN_RET(state_ != nullptr, RESAMPLER_ERR_INVALID_ARG);
        SingleStagePolyphaseResamplerFree(state_);
        state_ = nullptr;
        return RESAMPLER_ERR_INVALID_ARG;
    }
    expectedOutFrameLen_ = outRate_ * FRAME_LEN_20MS / MS_PER_SECOND;
    int32_t ret = ConfigBufferSizeAndExpectedInFrameLen();
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
    if ((channels_ <= 0) || (channels_ > MAX_CHANNELS)) {
        AUDIO_ERR_LOG("resampler set to invalid state, channel number should within [1, 16]");
        CHECK_AND_RETURN_RET(state_ != nullptr, RESAMPLER_ERR_INVALID_ARG);
        SingleStagePolyphaseResamplerFree(state_);
        state_ = nullptr;
        return RESAMPLER_ERR_INVALID_ARG;
    }
    if (state_ != nullptr) {
        SingleStagePolyphaseResamplerFree(state_);
    }
    int32_t errRet = RESAMPLER_ERR_SUCCESS;
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
    expectedInFrameLen_(other.expectedInFrameLen_), state_(other.state_)
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
        other.state_ = nullptr;
    }
    return *this;
}

void ProResampler::Reset()
{
    CHECK_AND_RETURN_LOG(state_ != nullptr, "resampler is null");
    SingleStagePolyphaseResamplerResetMem(state_);
    SingleStagePolyphaseResamplerSkipHalfTaps(state_);
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
    if (inRate_ == SAMPLE_RATE_11025) { // for 11025, process input 40ms per time and output 20ms per time
        size_t capacityNeed = static_cast<size_t>(expectedOutFrameLen_) * channels_ * BUFFER_EXPAND_SIZE_2 + ADD_SIZE;
        buf11025_.reserve(capacityNeed);
        AUDIO_INFO_LOG("input 11025hz, output resample rate %{public}u, buf11025_ size %{public}zu",
            outRate_, buf11025_.capacity());
        CHECK_AND_RETURN_RET_LOG(buf11025_.capacity() >= capacityNeed, RESAMPLER_ERR_ALLOC_FAILED,
            "buf11025_ size error, should be above %{public}zu, actually %{public}zu",
            capacityNeed, buf11025_.capacity());
        expectedInFrameLen_ = inRate_ * FRAME_LEN_20MS * BUFFER_EXPAND_SIZE_2 / MS_PER_SECOND;
    } else if (inRate_ % CUSTOM_SAMPLE_RATE_MULTIPLES != 0) {   // not multiples of 50
        size_t capacityNeed = static_cast<size_t>(expectedOutFrameLen_) * channels_ * BUFFER_EXPAND_SIZE_5 + ADD_SIZE;
        bufFor100ms_.reserve(capacityNeed);
        AUDIO_INFO_LOG("input %{public}u, output resample rate %{public}u, bufFor100ms_ size %{public}zu",
            inRate_, outRate_, bufFor100ms_.capacity());
        CHECK_AND_RETURN_RET_LOG(bufFor100ms_.capacity() >= capacityNeed, RESAMPLER_ERR_ALLOC_FAILED,
            "bufFor100ms_ size error, should be above %{public}zu, actually %{public}zu",
            capacityNeed, bufFor100ms_.capacity());
        expectedInFrameLen_ = inRate_ * FRAME_LEN_20MS * BUFFER_EXPAND_SIZE_5 / MS_PER_SECOND;
    } else {
        expectedInFrameLen_ = inRate_ * FRAME_LEN_20MS / MS_PER_SECOND;
    }
    return RESAMPLER_ERR_SUCCESS;
}

} // HPAE
} // AudioStandard
} // OHOS