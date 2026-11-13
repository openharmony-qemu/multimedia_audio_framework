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
#include <limits>
#include "audio_stream_info.h"
#include "securec.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t FRAME_LEN_20MS = (20 * SOURCE_PERIOD_SIZE);

constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t MAX_CHANNELS = 16;
constexpr uint32_t MAX_SAMPLE_RATE = SAMPLE_RATE_384000;
constexpr uint32_t MIN_SAMPLE_RATE = SAMPLE_RATE_8000;
constexpr uint32_t MAX_FRAME_LEN = SAMPLE_RATE_384000 * 10; // max frame size, max sample rate, 10s duration
constexpr uint32_t MAX_QUALITY = 10;
}
// Unified Process: accepts arbitrary input frame counts, outputs resampled data in one call.
// The caller (converter node) handles input accumulation and output splitting.
ProResampler::ProResampler(uint32_t inRate, uint32_t outRate, uint32_t channels, uint32_t quality)
    : inRate_(inRate), outRate_(outRate), channels_(channels), quality_(quality),
    expectedOutFrameLen_(outRate_ * FRAME_LEN_20MS / MS_PER_SECOND)
{
    
    CHECK_AND_RETURN_LOG((inRate_ >= MIN_SAMPLE_RATE) && (inRate_ <= MAX_SAMPLE_RATE) &&
        (outRate_ >= MIN_SAMPLE_RATE) && (outRate_ <= MAX_SAMPLE_RATE),
        "resampler input and output sample rate should be within [8000, 384000]. "
        "inRate_ %{public}d, outRate_ %{public}d is not valid", inRate_, outRate_);
    
    CHECK_AND_RETURN_LOG((channels_ > 0) && (channels_ <= MAX_CHANNELS), "invalid channel number: %{public}d, "
        "channel number should within [1, 16]", channels_);
    
    CHECK_AND_RETURN_LOG(quality <= MAX_QUALITY, "invalid quality level: %{public}d", quality);

    if (inRate_ == outRate_) {
        configurationValid_ = true;
        return;
    }

    int32_t errRet = ConfigBufferSizeAndExpectedInFrameLen();
    CHECK_AND_RETURN_LOG(errRet == RESAMPLER_ERR_SUCCESS,
        "ProResampler construct reserve buff error code %{public}s", ErrCodeToString(errRet).c_str());

    state_ = SingleStagePolyphaseResamplerInit(channels_, inRate_, outRate_, quality_, &errRet);
    CHECK_AND_RETURN_LOG(state_, "Init failed! failed with error %{public}s.",
        ErrCodeToString(errRet).c_str());
    
    configurationValid_ = errRet == EOK && SingleStagePolyphaseResamplerSkipHalfTaps(state_) == EOK;
    AUDIO_INFO_LOG("Init success inRate: %{public}d, outRate: %{public}d, channels: %{public}d, "
        "quality: %{public}d.", inRate_, outRate_, channels_, quality_);
}

int32_t ProResampler::Process(const float *inBuffer, uint32_t inFrameLen, float *outBuffer,
    uint32_t outFrameLen)
{
    ProcessResult result;
    return ProcessWithCounts(inBuffer, inFrameLen, outBuffer, outFrameLen, result);
}

int32_t ProResampler::ProcessWithCounts(const float *inBuffer, uint32_t inFrameLen, float *outBuffer,
    uint32_t outFrameLen, ProcessResult &result)
{
    result = {};
    CHECK_AND_RETURN_RET_LOG(inBuffer != nullptr && outBuffer != nullptr,
        RESAMPLER_ERR_INVALID_ARG, "invalid input/output buffer");
    CHECK_AND_RETURN_RET_LOG(configurationValid_ && state_ != nullptr,
        RESAMPLER_ERR_ALLOC_FAILED, "invalid resampler configuration");
    CHECK_AND_RETURN_RET_LOG(inFrameLen <= MAX_FRAME_LEN && outFrameLen <= MAX_FRAME_LEN &&
        channels_ > 0 && channels_ <= MAX_CHANNELS &&
        inFrameLen <= std::numeric_limits<size_t>::max() / channels_ / sizeof(float) &&
        outFrameLen <= std::numeric_limits<size_t>::max() / channels_ / sizeof(float),
        RESAMPLER_ERR_OVERFLOW, "invalid frame/sample count");
    const uint32_t requestedIn = inFrameLen;
    const uint32_t requestedOut = outFrameLen;
    const size_t outputSamples = static_cast<size_t>(outFrameLen) * channels_;
    CHECK_AND_RETURN_RET_LOG(outputSamples <= tmpOutBuf_.max_size(),
        RESAMPLER_ERR_OVERFLOW, "output allocation overflow");
    if (tmpOutBuf_.size() < outputSamples) {
        tmpOutBuf_.resize(outputSamples, 0.0f);
    }
    int32_t ret = SingleStagePolyphaseResamplerProcess(state_, inBuffer, &inFrameLen,
        tmpOutBuf_.data(), &outFrameLen);
    result.consumedFrames = inFrameLen;
    result.producedFrames = outFrameLen;
    CHECK_AND_RETURN_RET_LOG(ret == EOK && inFrameLen <= requestedIn && outFrameLen <= requestedOut,
        ret == EOK ? RESAMPLER_ERR_OVERFLOW : ret, "invalid/failed actual resampler counts");
    result.paddingFrames = requestedOut - outFrameLen;
    lastDelayFrames_ = result.paddingFrames;
    const size_t paddingSamples = static_cast<size_t>(result.paddingFrames) * channels_;
    if (paddingSamples != 0) {
        ret = memset_s(outBuffer, outputSamples * sizeof(float), 0, paddingSamples * sizeof(float));
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "padding copy failed after resampler state advanced");
    }
    if (outFrameLen != 0) {
        ret = memcpy_s(outBuffer + paddingSamples, (outputSamples - paddingSamples) * sizeof(float),
            tmpOutBuf_.data(), static_cast<size_t>(outFrameLen) * channels_ * sizeof(float));
        CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "output copy failed after resampler state advanced");
    }
    result.outputFrames = requestedOut; // leading padding plus actual produced frames
    result.valid = true;
    return EOK;
}

int32_t ProResampler::CheckRateAndChannelParam()
{
    // resampler change from valid state to invalid state
    if ((inRate_ < MIN_SAMPLE_RATE) || (inRate_ > MAX_SAMPLE_RATE) || (outRate_ < MIN_SAMPLE_RATE) ||
        (outRate_ > MAX_SAMPLE_RATE) || (channels_ <= 0) || (channels_ > MAX_CHANNELS)) {
        AUDIO_ERR_LOG("resampler set to invalid state, because inRate %{public}u Hz and outRate %{public}u Hz "
            "should be within [8000, 384000] or channel number %{public}u should be within [1, 16]",
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
    configurationValid_ = false;
    inRate_ = inRate;
    outRate_ = outRate;
    int32_t ret = CheckRateAndChannelParam();
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "invalid updated rates");
    if (inRate_ == outRate_) {
        SingleStagePolyphaseResamplerFree(state_);
        state_ = nullptr;
        tmpOutBuf_.clear();
        lastDelayFrames_ = 0;
        configurationValid_ = true; // converter's same-rate path does not resample
        return EOK;
    }
    expectedOutFrameLen_ = outRate_ * FRAME_LEN_20MS / MS_PER_SECOND;
    ret = ConfigBufferSizeAndExpectedInFrameLen();
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "updated buffer configuration failed");
    if (state_ == nullptr) {
        state_ = SingleStagePolyphaseResamplerInit(channels_, inRate_, outRate_, quality_, &ret);
        CHECK_AND_RETURN_RET_LOG(state_ != nullptr && ret == EOK,
            ret == EOK ? RESAMPLER_ERR_ALLOC_FAILED : ret, "updated initialization failed");
        ret = SingleStagePolyphaseResamplerSkipHalfTaps(state_);
    } else {
        ret = SingleStagePolyphaseResamplerSetRate(state_, inRate_, outRate_);
    }
    configurationValid_ = (ret == EOK);
    return ret;
}

int32_t ProResampler::UpdateChannels(uint32_t channels)
{
    configurationValid_ = false;
    channels_ = channels;
    int32_t ret = CheckRateAndChannelParam();
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "invalid updated channels");
    SingleStagePolyphaseResamplerFree(state_);
    state_ = nullptr;
    if (inRate_ == outRate_) {
        tmpOutBuf_.clear();
        lastDelayFrames_ = 0;
        configurationValid_ = true;
        return EOK;
    }
    state_ = SingleStagePolyphaseResamplerInit(channels_, inRate_, outRate_, quality_, &ret);
    CHECK_AND_RETURN_RET_LOG(state_ != nullptr && ret == EOK,
        ret == EOK ? RESAMPLER_ERR_ALLOC_FAILED : ret, "channel initialization failed");
    ret = ConfigBufferSizeAndExpectedInFrameLen();
    CHECK_AND_RETURN_RET_LOG(ret == EOK, ret, "channel buffer configuration failed");
    ret = SingleStagePolyphaseResamplerSkipHalfTaps(state_);
    configurationValid_ = (ret == EOK);
    return ret;
}

ProResampler::ProResampler(ProResampler &&other) noexcept
    : inRate_(other.inRate_), outRate_(other.outRate_), channels_(other.channels_),
    quality_(other.quality_), expectedOutFrameLen_(other.expectedOutFrameLen_),
    expectedInFrameLen_(other.expectedInFrameLen_), configurationValid_(other.configurationValid_), state_(other.state_),
    tmpOutBuf_(std::move(other.tmpOutBuf_))
{
    other.state_ = nullptr;
    other.configurationValid_ = false;
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
        configurationValid_ = other.configurationValid_;
        expectedOutFrameLen_ = other.expectedOutFrameLen_;
        expectedInFrameLen_ = other.expectedInFrameLen_;
        tmpOutBuf_ = std::move(other.tmpOutBuf_);
        other.state_ = nullptr;
    other.configurationValid_ = false;
    }
    return *this;
}

void ProResampler::Reset()
{
    (void)ResetWithStatus();
}

int32_t ProResampler::ResetWithStatus()
{
    CHECK_AND_RETURN_RET_LOG(configurationValid_, RESAMPLER_ERR_INVALID_ARG,
        "cannot reset a failed configuration");
    if (inRate_ == outRate_) {
        lastDelayFrames_ = 0;
        return EOK;
    }
    CHECK_AND_RETURN_RET_LOG(state_ != nullptr, RESAMPLER_ERR_ALLOC_FAILED, "resampler is null");
    int32_t ret = SingleStagePolyphaseResamplerResetMem(state_);
    if (ret == EOK) {
        ret = SingleStagePolyphaseResamplerSkipHalfTaps(state_);
    }
    configurationValid_ = (ret == EOK);
    if (configurationValid_) {
        lastDelayFrames_ = 0;
    }
    return ret;
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