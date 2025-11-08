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

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr AudioSamplingRate DEFAULT_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr AudioSampleFormat DEFAULT_SAMPLE_FORMAT = SAMPLE_F32LE;
static constexpr AudioChannel DEFAULT_CHANNEL_COUNT = STEREO;
static constexpr AudioChannelLayout DEFAULT_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
}

AudioSuiteMixerNode::AudioSuiteMixerNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_AUDIO_MIXER,
          AudioFormat{{DEFAULT_CHANNEL_LAYOUT, DEFAULT_CHANNEL_COUNT}, DEFAULT_SAMPLE_FORMAT, DEFAULT_SAMPLE_RATE}),
      tmpOutput_(
          PcmBufferFormat{DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL_COUNT, DEFAULT_CHANNEL_LAYOUT, DEFAULT_SAMPLE_FORMAT}),
      mixerOutput_(
          PcmBufferFormat{DEFAULT_SAMPLE_RATE, DEFAULT_CHANNEL_COUNT, DEFAULT_CHANNEL_LAYOUT, DEFAULT_SAMPLE_FORMAT})
{}

AudioSuiteMixerNode::~AudioSuiteMixerNode()
{
    DeInit();
}

void AudioSuiteMixerNode::SetAudioNodeFormat(AudioFormat audioFormat)
{
    AudioFormat currentFormat = GetAudioNodeFormat();
    currentFormat.rate = audioFormat.rate;
    AudioNode::SetAudioNodeFormat(currentFormat);
    AUDIO_INFO_LOG("numChannels:%{public}u, sampleFormat:%{public}u, sampleRate:%{public}d",
        currentFormat.audioChannelInfo.numChannels, currentFormat.format, currentFormat.rate);

    PcmBufferFormat newPcmFormat = GetAudioNodeInPcmFormat();
    tmpOutput_.ResizePcmBuffer(newPcmFormat);
    mixerOutput_.ResizePcmBuffer(newPcmFormat);

    int32_t ret = InitAudioLimiter();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "Failed to Init Mixer node");
}

int32_t AudioSuiteMixerNode::InitAudioLimiter()
{
    AUDIO_INFO_LOG("AudioSuiteMixerNode::InitAudioLimiter");
    if (limiter_ == nullptr) {
        limiter_ = std::make_unique<AudioLimiter>(GetAudioNodeId());
    }
    int32_t ret = limiter_->SetConfig(
        tmpOutput_.GetDataSize(), sizeof(float), tmpOutput_.GetSampleRate(), tmpOutput_.GetChannelCount());
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "InitAudioLimiter fail, ret: %{public}d", ret);

    return SUCCESS;
}

int32_t AudioSuiteMixerNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteMixerNode::Init begin");

    int32_t ret = InitAudioLimiter();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to Init Mixer node");

    AUDIO_INFO_LOG("AudioSuiteMixerNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteMixerNode::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteMixerNode::DeInit begin");

    limiter_.reset();

    AUDIO_INFO_LOG("AudioSuiteMixerNode::DeInit end");
    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteMixerNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(limiter_ != nullptr, nullptr, "limiter_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "AudioSuitePcmBuffer inputs is nullptr");

    tmpOutput_.Reset();
    float *outData = reinterpret_cast<float *>(tmpOutput_.GetPcmData());
    float *inData = nullptr;
    for (auto input : inputs) {
        CHECK_AND_RETURN_RET_LOG(input != nullptr, nullptr, "Input pcm buffer is nullptr");
        CHECK_AND_RETURN_RET_LOG(input->IsSameFormat(tmpOutput_), nullptr, "Invalid inputPcmBuffer format");
        CHECK_AND_RETURN_RET_LOG(input->GetSampleCount() == tmpOutput_.GetSampleCount(),
            nullptr, "Invalid inputPcmBuffer data");
        inData = reinterpret_cast<float *>(input->GetPcmData());
        CHECK_AND_RETURN_RET_LOG(inData != nullptr, nullptr, "Input data is nullptr");
        for (size_t idx = 0; idx < tmpOutput_.GetSampleCount(); ++idx) {
            outData[idx] += inData[idx];
        }
    }

    limiter_->Process(tmpOutput_.GetSampleCount(),
        reinterpret_cast<float *>(tmpOutput_.GetPcmData()),
        reinterpret_cast<float *>(mixerOutput_.GetPcmData()));

    return &mixerOutput_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS