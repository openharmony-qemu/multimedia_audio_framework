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
#define LOG_TAG "AudioSuiteNrNode"
#endif

#include "audio_suite_nr_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr AudioSamplingRate NR_ALGO_SAMPLE_RATE = SAMPLE_RATE_16000;
static constexpr AudioSampleFormat NR_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel NR_ALGO_CHANNEL_COUNT = MONO;
static constexpr AudioChannelLayout NR_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_MONO;
static constexpr uint32_t NR_ALGO_FRAME_LENGTH = 160;
static constexpr uint32_t NR_ALGO_FRAME_SIZE = NR_ALGO_FRAME_LENGTH * 2;
static constexpr uint32_t RESAMPLE_QUALITY = 5;

AudioSuiteNrNode::AudioSuiteNrNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_NOISE_REDUCTION,
          AudioFormat{{NR_ALGO_CHANNEL_LAYOUT, NR_ALGO_CHANNEL_COUNT}, NR_ALGO_SAMPLE_FORMAT, NR_ALGO_SAMPLE_RATE}),
      pcmBufferOutput_(NR_ALGO_SAMPLE_RATE, NR_ALGO_CHANNEL_COUNT, NR_ALGO_CHANNEL_LAYOUT)
{}

AudioSuiteNrNode::~AudioSuiteNrNode()
{}

int32_t AudioSuiteNrNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteNrNode::Init begin");
    algoInterface_ = AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_NOISE_REDUCTION);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create nr algoInterface");

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to Init nr algorithm");

    uint32_t bufferSize = pcmBufferOutput_.GetFrameLen() * sizeof(short);
    algoInputBuffer_.resize(bufferSize);
    algoOutputBuffer_.resize(bufferSize);

    AUDIO_INFO_LOG("AudioSuiteNrNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteNrNode::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteNrNode::DeInit begin");

    if (algoInterface_ != nullptr) {
        int32_t ret = algoInterface_->Deinit();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to DeInit nr algorithm");
        algoInterface_.reset();
    }

    AUDIO_INFO_LOG("AudioSuiteNrNode::DeInit end");
    return SUCCESS;
}

int32_t AudioSuiteNrNode::CopyPcmBuffer(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    float *inputData = inputPcmBuffer->GetPcmDataBuffer();
    uint32_t inFrameSize = inputPcmBuffer->GetFrameLen() * sizeof(float);
    float *outputData = outputPcmBuffer->GetPcmDataBuffer();
    uint32_t outFrameSize = outputPcmBuffer->GetFrameLen() * sizeof(float);

    int32_t ret = memcpy_s(outputData, outFrameSize, inputData, inFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "AudioSuiteNrNode PcmBuffer copy failed: %{public}d", ret);
    return SUCCESS;
}

int32_t AudioSuiteNrNode::DoChannelConvert(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    AudioChannelInfo inChannelInfo = {inputPcmBuffer->GetChannelLayout(), inputPcmBuffer->GetChannelCount()};
    AudioChannelInfo outChannelInfo = {NR_ALGO_CHANNEL_LAYOUT, NR_ALGO_CHANNEL_COUNT};
    SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);

    uint32_t frameSize = inputPcmBuffer->GetFrameLen() / inputPcmBuffer->GetChannelCount();
    float *inputData = inputPcmBuffer->GetPcmDataBuffer();
    uint32_t inLen = inputPcmBuffer->GetFrameLen() * sizeof(float);
    float *outputData = outputPcmBuffer->GetPcmDataBuffer();
    uint32_t outLen = outputPcmBuffer->GetFrameLen() * sizeof(float);
    AUDIO_INFO_LOG("AudioSuiteNrNode::DoChannelConvert: frameSize: %{public}u, inLen: %{public}u, outLen: %{public}u",
        frameSize, inLen, outLen);

    int32_t ret = ChannelConvertProcess(frameSize, inputData, inLen, outputData, outLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "AudioSuiteNrNode DoChannelConvert failed: %{public}d", ret);

    return SUCCESS;
}

int32_t AudioSuiteNrNode::DoResample(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    uint32_t inRate = inputPcmBuffer->GetSampleRate();
    uint32_t outRate = NR_ALGO_SAMPLE_RATE;
    uint32_t channelCount = inputPcmBuffer->GetChannelCount();
    CHECK_AND_RETURN_RET_LOG(channelCount != 0, ERROR, "Invalid ChannelCount: %{public}d", channelCount);

    AUDIO_INFO_LOG(
        "AudioSuiteNrNode::DoResample: inSampleRate: %{public}u, outSampleRate: %{public}u, channelCount: %{public}u",
        inRate, outRate, channelCount);
    int32_t ret = SetUpResample(inRate, outRate, channelCount, RESAMPLE_QUALITY);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "AudioSuiteNrNode SetUpResample failed: %{public}d", ret);

    float *inputData = inputPcmBuffer->GetPcmDataBuffer();
    uint32_t inFrameSize = inputPcmBuffer->GetFrameLen() / channelCount;
    float *outputData = outputPcmBuffer->GetPcmDataBuffer();
    uint32_t outFrameSize = outputPcmBuffer->GetFrameLen() / channelCount;
    ret = DoResampleProcess(inputData, inFrameSize, outputData, outFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "AudioSuiteNrNode DoResampleProcess failed: %{public}d", ret);

    return SUCCESS;
}

int32_t AudioSuiteNrNode::ConvertProcess(AudioSuitePcmBuffer *inputPcmBuffer)
{
    uint32_t inChannelCount = inputPcmBuffer->GetChannelCount();
    uint32_t outChannelCount = NR_ALGO_CHANNEL_COUNT;
    uint32_t inSampleRate = inputPcmBuffer->GetSampleRate();
    uint32_t outSampleRate = NR_ALGO_SAMPLE_RATE;
    AUDIO_INFO_LOG("AudioSuiteNrNode::ConvertProcess inChannelCount: %{public}u, outChannelCount: %{public}u,"
                   "inSampleRate: %{public}u, outSampleRate: %{public}u",
        inChannelCount, outChannelCount, inSampleRate, outSampleRate);
    
    int32_t ret;
    if (inChannelCount == outChannelCount && inSampleRate == outSampleRate) {
        ret = CopyPcmBuffer(inputPcmBuffer, &pcmBufferOutput_);
    } else if (inChannelCount == outChannelCount) {
        ret = DoResample(inputPcmBuffer, &pcmBufferOutput_);
    } else if (inSampleRate == outSampleRate) {
        ret = DoChannelConvert(inputPcmBuffer, &pcmBufferOutput_);
    } else if (inChannelCount > outChannelCount) {
        // 采样率和声道数都不同，先做声道转换。
        AudioSuitePcmBuffer channelConvertPcmBuffer(
            inputPcmBuffer->GetSampleRate(), NR_ALGO_CHANNEL_COUNT, NR_ALGO_CHANNEL_LAYOUT);
        ret = DoChannelConvert(inputPcmBuffer, &channelConvertPcmBuffer);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
        ret = DoResample(&channelConvertPcmBuffer, &pcmBufferOutput_);
    } else {
        AUDIO_ERR_LOG("AudioSuiteNrNode::ConvertProcess input channel count less than output");
        return ERROR;
    }
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteNrNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    if (inputs.empty()) {
        AUDIO_ERR_LOG("AudioSuiteNrNode::SignalProcess inputs list is empty");
        return &pcmBufferOutput_;
    }

    if (inputs[0] == nullptr) {
        AUDIO_ERR_LOG("AudioSuiteNrNode::SignalProcess input data is nullptr");
        return &pcmBufferOutput_;
    }

    // 声道转换及采样率转换
    int32_t ret = ConvertProcess(inputs[0]);
    if (ret != SUCCESS) {
        return &pcmBufferOutput_;
    }

    // 位深转换 float -> SAMPLE_S16LE
    ConvertFromFloat(NR_ALGO_SAMPLE_FORMAT,
        pcmBufferOutput_.GetFrameLen(),
        pcmBufferOutput_.GetPcmDataBuffer(),
        static_cast<void *>(algoInputBuffer_.data()));

    // 调降噪算法
    std::vector<uint8_t *> nrAlgoInputs(1);
    std::vector<uint8_t *> nrAlgoOutputs(1);
    uint8_t *frameInputPtr = algoInputBuffer_.data();
    uint8_t *frameOutputPtr = algoOutputBuffer_.data();
    for (int32_t i = 0; i + NR_ALGO_FRAME_SIZE <= algoInputBuffer_.size(); i += NR_ALGO_FRAME_SIZE) {
        nrAlgoInputs[0] = frameInputPtr;
        nrAlgoOutputs[0] = frameOutputPtr;

        ret = algoInterface_->Apply(nrAlgoInputs, nrAlgoOutputs);
        if (ret != SUCCESS) {
            return &pcmBufferOutput_;
        }

        frameInputPtr += NR_ALGO_FRAME_SIZE;
        frameOutputPtr += NR_ALGO_FRAME_SIZE;
    }

    // 位深转换 SAMPLE_S16LE -> float
    ConvertToFloat(NR_ALGO_SAMPLE_FORMAT,
        pcmBufferOutput_.GetFrameLen(),
        static_cast<void *>(algoOutputBuffer_.data()),
        pcmBufferOutput_.GetPcmDataBuffer());

    return &pcmBufferOutput_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS