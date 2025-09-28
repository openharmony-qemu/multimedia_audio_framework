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
#define LOG_TAG "AudioSuiteMixerNode"
#endif

#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_mixer_node.h"
#include <fstream>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

constexpr int REASAMPLE_QUAILTY = 5;
constexpr int FRAME_TIME = 20;
constexpr int CONVERSION = 1000;
constexpr uint32_t CHANNEL_TWO = 2;

AudioSuiteMixerNode::AudioSuiteMixerNode()
    :AudioSuiteProcessNode(NODE_TYPE_AUDIO_MIXER, AudioFormat{{CH_LAYOUT_STEREO, CHANNEL_TWO},
        SAMPLE_F32LE, SAMPLE_RATE_96000}),
    rate_(SAMPLE_RATE_192000),
    mixerOutput_(rate_, STEREO, CH_LAYOUT_STEREO),
    tmpOutput_(rate_, STEREO, CH_LAYOUT_STEREO),
    channelOutput_(rate_, STEREO, CH_LAYOUT_STEREO),
    rateOutput_(rate_, STEREO, CH_LAYOUT_STEREO),
    frameLen_(FRAME_TIME * rate_ / CONVERSION)
{
    int32_t ret = InitAudioLimiter();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "InitAudioLimiter fail");
    rate_ = SAMPLE_RATE_8000;
    AUDIO_INFO_LOG("AudioSuiteMixerNode Create SUCCESS.");
}

AudioSuiteMixerNode::~AudioSuiteMixerNode()
{
}

AudioSuitePcmBuffer *AudioSuiteMixerNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "AudioSuitePcmBuffer inputs is nullptr");

    for (auto input: inputs) {
        CHECK_AND_RETURN_RET_LOG(input != nullptr, nullptr, "Input buffer is nullptr");
        mixrate_ = (mixrate_ > static_cast<AudioSamplingRate>(input->GetSampleRate())) ?
            mixrate_ : static_cast<AudioSamplingRate>(input->GetSampleRate());
    }

    if (rate_ != mixrate_) {
        rate_ = mixrate_;
        mixerOutput_.ResizePcmBuffer(rate_, STEREO);
        tmpOutput_.ResizePcmBuffer(rate_, STEREO);
        frameLen_ = FRAME_TIME * rate_ / CONVERSION;

        int32_t ret = InitAudioLimiter();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "ReInitAudioLimiter fail");
        AUDIO_INFO_LOG("ReInitAudioLimiter rate_: %{public}d frameLen_: %{public}d", rate_, frameLen_);
    }

    CHECK_AND_RETURN_RET_LOG(limiter_ != nullptr, nullptr, "limiter_ is nullptr");

    mixerOutput_.Reset();
    tmpOutput_.Reset();

    for (auto input: inputs) {
        CHECK_AND_RETURN_RET_LOG(input != nullptr, nullptr, "Input buffer is nullptr");
        rateOutput_.Reset();
        channelOutput_.Reset();

        if (!preProcess(input)) {
            AUDIO_ERR_LOG("SignalProcess Pre Process failed.");
            return nullptr;
        }

        if (input->GetChannelCount() != STEREO) {
            tmpOutput_ += channelOutput_;
        } else if (input->GetSampleRate() != rate_) {
            tmpOutput_ += rateOutput_;
        } else {
            tmpOutput_ += *input;
        }
    }

    limiter_->Process(frameLen_ * STEREO,
        tmpOutput_.GetPcmDataBuffer(), mixerOutput_.GetPcmDataBuffer());
    return &mixerOutput_;
}

int32_t AudioSuiteMixerNode::InitAudioLimiter()
{
    if (limiter_ == nullptr) {
        limiter_ = std::make_unique<AudioLimiter>(GetAudioNodeId());
    }

    int32_t ret = limiter_->SetConfig(frameLen_ * STEREO * sizeof(float), sizeof(float), rate_, STEREO);
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("NodeId: %{public}d,rate_: %{public}d,frameLen_: %{public}d,limiter init success!",
            GetAudioNodeId(), rate_, frameLen_);
    } else {
        limiter_ = nullptr;
        AUDIO_INFO_LOG("NodeId: %{public}d,rate_: %{public}d,frameLen_: %{public}d,limiter init fail!!",
            GetAudioNodeId(), rate_, frameLen_);
    }
    return ret;
}

AudioSuitePcmBuffer *AudioSuiteMixerNode::preProcess(AudioSuitePcmBuffer *input)
{
    AUDIO_INFO_LOG("Channel: %{public}d, SampleRate: %{public}d",
        input->GetChannelCount(), input->GetSampleRate());

    if (input->GetSampleRate() != rate_) {
        CHECK_AND_RETURN_RET_LOG(preRateProcess(input, rate_, input->GetChannelCount()),
            nullptr, "preRateProcess failed.");

        if (input->GetChannelCount() != STEREO) {
            CHECK_AND_RETURN_RET_LOG(preChannelProcess(&rateOutput_, rateOutput_.GetSampleRate(),
                STEREO), nullptr, "preRateProcess failed.");
            return &channelOutput_;
        }
        return &rateOutput_;
    }

    if (input->GetChannelCount() != STEREO) {
        CHECK_AND_RETURN_RET_LOG(preChannelProcess(input, input->GetSampleRate(),
            STEREO), nullptr, "preRateProcess failed.");
        return &channelOutput_;
    }

    return input;
}

AudioSuitePcmBuffer *AudioSuiteMixerNode::preChannelProcess(AudioSuitePcmBuffer *input,
    uint32_t sampleRate, uint32_t channelCount)
{
    if (sampleRate != channel_sampleRate_ || channelCount != channel_channelCount_) {
        AUDIO_INFO_LOG("channel Rate: %{public}d Rate_: %{public}d channel: %{public}d channel_: %{public}d",
            sampleRate, rate_sampleRate_, channelCount, rate_channelCount_);
        channel_sampleRate_ = sampleRate;
        channel_channelCount_ = channelCount;
        channelOutput_.ResizePcmBuffer(sampleRate, channelCount);
    }

    AudioChannelInfo inChannelInfo = {input->GetChannelLayout(), input->GetChannelCount()};
    AudioChannelInfo outChannelInfo = {CH_LAYOUT_STEREO, STEREO};
    int ret = SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);
    CHECK_AND_RETURN_RET_LOG(ret == HPAE::MIX_ERR_SUCCESS, nullptr,
        "Set channel convert processParam failed with error code %{public}d", ret);

    uint32_t formatChannelSrcBufSize = input->GetFrameLen() * SAMPLE_F32LE;
    uint32_t channelConvertOutputBytes = channelOutput_.GetFrameLen() * SAMPLE_F32LE;
    ret = ChannelConvertProcess(input->GetFrameLen() / input->GetChannelCount(),
        input->GetPcmDataBuffer(), formatChannelSrcBufSize, channelOutput_.GetPcmDataBuffer(),
        channelConvertOutputBytes);
    CHECK_AND_RETURN_RET_LOG(ret == HPAE::MIX_ERR_SUCCESS, nullptr,
        "Channel convert process failed with error code %{public}d", ret);

    return &channelOutput_;
}

AudioSuitePcmBuffer *AudioSuiteMixerNode::preRateProcess(AudioSuitePcmBuffer *input,
    uint32_t sampleRate, uint32_t channelCount)
{
    int ret;
    if (sampleRate != rate_sampleRate_ || channelCount != rate_channelCount_) {
        AUDIO_INFO_LOG("rate Rate: %{public}d Rate_: %{public}d channel: %{public}d channel_: %{public}d",
            sampleRate, rate_sampleRate_, channelCount, rate_channelCount_);
        rate_sampleRate_ = sampleRate;
        rate_channelCount_ = channelCount;
        rateOutput_.ResizePcmBuffer(sampleRate, channelCount);
    }

    ret = SetUpResample(input->GetSampleRate(), rate_, input->GetChannelCount(), REASAMPLE_QUAILTY);
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, nullptr,
        "setup resample failed with error code %{public}d", ret);

    uint32_t inFrameSize = input->GetFrameLen() / input->GetChannelCount();
    uint32_t outFrameSize = frameLen_;
    ret = DoResampleProcess(input->GetPcmDataBuffer(), inFrameSize, rateOutput_.GetPcmDataBuffer(), outFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == RESAMPLER_ERR_SUCCESS, nullptr,
        "Do resample process failed with error code %{public}d", ret);

    return &rateOutput_;
}

bool AudioSuiteMixerNode::Reset()
{
    int32_t ret = InitAudioLimiter();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "InitAudioLimiter fail");
    return true;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS