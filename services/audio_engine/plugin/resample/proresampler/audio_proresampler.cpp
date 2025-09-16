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
constexpr uint32_t BUFFER_EXPAND_SIZE = 2;
constexpr uint32_t SAMPLE_RATE_11025 = 11025;
constexpr uint32_t FRAME_LEN_20MS = 20;
constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t ADD_SIZE = 100;
// for now ProResampler accept input 20ms for other sample rates, 40ms input for 11025hz
// output 20ms for all sample rates
ProResampler::ProResampler(uint32_t inRate, uint32_t outRate, uint32_t channels, uint32_t quality)
    : inRate_(inRate), outRate_(outRate), channels_(channels), quality_(quality),
    expectedOutFrameLen_(outRate_ * FRAME_LEN_20MS / MS_PER_SECOND)
{
    if (inRate_ == SAMPLE_RATE_11025) { // for 11025, process input 40ms per time and output 20ms per time
        buf11025_.reserve(expectedOutFrameLen_ * channels_ * BUFFER_EXPAND_SIZE + ADD_SIZE);
        AUDIO_INFO_LOG("input 11025hz, output resample rate %{public}d, buf11025_ size %{public}d",
            outRate_, expectedOutFrameLen_ * channels_ * BUFFER_EXPAND_SIZE + ADD_SIZE);
        expectedInFrameLen_ = inRate_ * FRAME_LEN_20MS * BUFFER_EXPAND_SIZE / MS_PER_SECOND;
    } else {
        expectedInFrameLen_ = inRate_ * FRAME_LEN_20MS / MS_PER_SECOND;
    }
    int32_t errRet;
    state_ = SingleStagePolyphaseResamplerInit(channels_, inRate_, outRate_, quality_, &errRet);
    CHECK_AND_RETURN_LOG(state_, "Init failed! failed with error %{public}s.",
        ErrCodeToString(errRet).c_str());
    
    SingleStagePolyphaseResamplerSkipHalfTaps(state_);
    AUDIO_INFO_LOG("Init success inRate: %{public}d, outRate: %{public}d, channels: %{public}d, "
        "quality: %{public}d.", inRate_, outRate_, channels_, quality_);
}

int32_t ProResampler::Process(const float *inBuffer, uint32_t inFrameSize, float *outBuffer,
    uint32_t outFrameSize)
{
    CHECK_AND_RETURN_RET_LOG(state_ != nullptr, RESAMPLER_ERR_ALLOC_FAILED,
        "Process: resampler is %{public}s", ErrCodeToString(RESAMPLER_ERR_ALLOC_FAILED).c_str());
    if (inRate_ == SAMPLE_RATE_11025) {
        return Process11025SampleRate(inBuffer, inFrameSize, outBuffer, outFrameSize);
    } else {
        return ProcessOtherSampleRate(inBuffer, inFrameSize, outBuffer, outFrameSize);
    }
}

int32_t ProResampler::ProcessOtherSampleRate(const float *inBuffer, uint32_t inFrameSize, float *outBuffer,
    uint32_t outFrameSize)
{
    uint32_t expectedOutFrameSize = outFrameSize;
    std::vector<float> tmpOutBuf(expectedOutFrameSize * channels_, 0.0f);
    int32_t ret =
        SingleStagePolyphaseResamplerProcess(state_, inBuffer, &inFrameSize, tmpOutBuf.data(), &outFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "process failed with error %{public}s",
        ErrCodeToString(ret).c_str());
    
    uint32_t fillSize = expectedOutFrameSize - outFrameSize > 0 ? expectedOutFrameSize - outFrameSize : 0;
    ret = memset_s(outBuffer, fillSize * channels_ * sizeof(float), 0, fillSize * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memset_s failed with error %{public}d", ret);

    ret = memcpy_s(outBuffer + fillSize * channels_,
        (expectedOutFrameSize - fillSize) * channels_ * sizeof(float),
        tmpOutBuf.data(), outFrameSize * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memset_s failed with error %{public}d", ret);
    return ret;
}

int32_t ProResampler::Process11025SampleRate(const float *inBuffer, uint32_t inFrameSize, float *outBuffer,
    uint32_t outFrameSize)
{
    CHECK_AND_RETURN_RET_LOG(outFrameSize >= expectedOutFrameLen_, RESAMPLER_ERR_INVALID_ARG,
        "output frame size %{public}d is not valid", outFrameSize);
    CHECK_AND_RETURN_RET_LOG(((inFrameSize == 0) || (inFrameSize == expectedInFrameLen_)), RESAMPLER_ERR_INVALID_ARG,
        "input frame size %{public}d is not valid", inFrameSize);
    if (inFrameSize == 0) {
        int32_t ret = RESAMPLER_ERR_SUCCESS;
        if (buf11025Index_ > 0) { // output second half of 11025 buffer
            ret = memcpy_s(outBuffer, outFrameSize * channels_ * sizeof(float),
                buf11025_.data() + buf11025Index_,  expectedOutFrameLen_ * channels_ * sizeof(float));
            CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memcpy_s failed with error %{public}d", ret);
            
            ret = memset_s(buf11025_.data(), buf11025_.capacity() * sizeof(float), 0,
                buf11025_.capacity() * sizeof(float));
            buf11025Index_ = 0;
        } else { // no data left in buffer, the only thing can be done is to return 0s
            ret = memset_s(outBuffer, outFrameSize * channels_ * sizeof(float), 0,
                outFrameSize * channels_ * sizeof(float));
        }
        return ret;
    }
    std::vector<float> tmpOutBuf(expectedOutFrameLen_ * channels_ * BUFFER_EXPAND_SIZE, 0.0f);
    uint32_t tmpOutFrameLen = expectedOutFrameLen_ * BUFFER_EXPAND_SIZE;
    uint32_t reserveOutFrameLen = tmpOutFrameLen;
    int32_t ret =
        SingleStagePolyphaseResamplerProcess(state_, inBuffer, &inFrameSize, tmpOutBuf.data(), &tmpOutFrameLen);
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret, "Process failed with error %{public}s",
        ErrCodeToString(ret).c_str());

    uint32_t fillSize = reserveOutFrameLen - tmpOutFrameLen > 0 ? reserveOutFrameLen - tmpOutFrameLen : 0;
    ret = memset_s(buf11025_.data(), fillSize * channels_ * sizeof(float), 0, fillSize * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memset_s failed with error %{public}d", ret);

    ret = memcpy_s(buf11025_.data() + fillSize * channels_,
        (reserveOutFrameLen - fillSize) * channels_ * sizeof(float),
        tmpOutBuf.data(), tmpOutFrameLen * channels_ * sizeof(float));
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "memcpy_s failed with error %{public}d", ret);

    // output first half of data
    ret = memcpy_s(outBuffer, outFrameSize * channels_ * sizeof(float),
        buf11025_.data(), expectedOutFrameLen_ * channels_ * sizeof(float));
    buf11025Index_ = expectedOutFrameLen_ * channels_;
    return ret;
}

int32_t ProResampler::UpdateRates(uint32_t inRate, uint32_t outRate)
{
    inRate_ = inRate;
    outRate_ = outRate;
    expectedOutFrameLen_ = outRate_ * FRAME_LEN_20MS / MS_PER_SECOND;
    expectedInFrameLen_ = inRate_ * FRAME_LEN_20MS / MS_PER_SECOND;
    if (inRate_ == SAMPLE_RATE_11025) {
        expectedInFrameLen_ = inRate_ * FRAME_LEN_20MS * BUFFER_EXPAND_SIZE / MS_PER_SECOND;
    }
    CHECK_AND_RETURN_RET_LOG(state_ != nullptr, RESAMPLER_ERR_ALLOC_FAILED, "resampler is null");
    
    int32_t ret = SingleStagePolyphaseResamplerSetRate(state_, inRate_, outRate_);
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, ret,
        "update rate failed with error code %{public}s", ErrCodeToString(ret).c_str());
    return ret;
}

int32_t ProResampler::UpdateChannels(uint32_t channels)
{
    uint32_t oldChannels = channels_;
    channels_ = channels;
    CHECK_AND_RETURN_RET_LOG(state_ != nullptr, RESAMPLER_ERR_ALLOC_FAILED, "resampler is null");
    SingleStagePolyphaseResamplerFree(state_);

    int32_t errRet = RESAMPLER_ERR_SUCCESS;
    state_ = SingleStagePolyphaseResamplerInit(channels_, inRate_, outRate_, quality_, &errRet);
    CHECK_AND_RETURN_RET_LOG(state_ && (errRet == RESAMPLER_ERR_SUCCESS), errRet,
        "update work channels failed with error %{public}s.", ErrCodeToString(errRet).c_str());

    AUDIO_INFO_LOG("update work channel success old channels: %{public}d, new channels: %{public}d",
        oldChannels, channels_);

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
        if (state_ != nullptr) {
            SingleStagePolyphaseResamplerFree(state_);
        }
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
    if (state_ != nullptr) {
        SingleStagePolyphaseResamplerFree(state_);
        state_ = nullptr;
    }
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

} // HPAE
} // AudioStandard
} // OHOS