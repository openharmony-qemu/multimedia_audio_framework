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
#define LOG_TAG "AudioSuiteVoiceBeautifierNode"
#endif

#include "audio_suite_voice_beautifier_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

static constexpr AudioSamplingRate VM_ALGO_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr AudioSampleFormat VM_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel VM_ALGO_CHANNEL_COUNT = STEREO;
static constexpr AudioChannelLayout VM_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
static constexpr uint32_t RESAMPLE_QUALITY = 5;

AudioSuiteVoiceBeautifierNode::AudioSuiteVoiceBeautifierNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_VOICE_BEAUTIFIER,
          AudioFormat{{VM_ALGO_CHANNEL_LAYOUT, VM_ALGO_CHANNEL_COUNT}, VM_ALGO_SAMPLE_FORMAT, VM_ALGO_SAMPLE_RATE}),
      pcmBufferOutput_(VM_ALGO_SAMPLE_RATE, VM_ALGO_CHANNEL_COUNT, VM_ALGO_CHANNEL_LAYOUT),
      channelConvertPcmBuffer_(VM_ALGO_SAMPLE_RATE, VM_ALGO_CHANNEL_COUNT, VM_ALGO_CHANNEL_LAYOUT)
{}

AudioSuiteVoiceBeautifierNode::~AudioSuiteVoiceBeautifierNode()
{
    DeInit();
}

bool AudioSuiteVoiceBeautifierNode::Reset()
{
    if (DeInit() == SUCCESS && Init() == SUCCESS) {
        AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode reset success.");
        return true;
    }
    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode reset fail.");
    return false;
}

int32_t AudioSuiteVoiceBeautifierNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode Init begin");
    algoInterface_ = AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_VOICE_BEAUTIFIER);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create voice beautifier algoInterface");

    int32_t ret = algoInterface_->Init();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to Init voice beautifier algorithm.");
        DeInit();
        return ret;
    }

    uint32_t bufferSize = pcmBufferOutput_.GetFrameLen() * sizeof(short);
    algoInputBuffer_.resize(bufferSize);
    algoOutputBuffer_.resize(bufferSize);

    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode Init end");
    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierNode::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode DeInit begin");

    if (algoInterface_ != nullptr) {
        int32_t ret = algoInterface_->Deinit();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to DeInit voice beautifier algorithm");
        algoInterface_.reset();
    }

    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode DeInit end");
    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierNode::SetOptions(std::string name, std::string value)
{
    if (name != "VoiceBeautifierType") {
        AUDIO_ERR_LOG("wrong options name.");
        return ERROR;
    }
    if (value == std::to_string(static_cast<int32_t>(AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_NORMAL))) {
        SetNodeEnableStatus(NODE_DISABLE);
        return SUCCESS;
    }
    if (algoInterface_ != nullptr && algoInterface_->SetParameter(name, value)) {
        AUDIO_ERR_LOG("SetOptions fail.");
        DeInit();
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierNode::CopyPcmBuffer(
    AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    if (inputPcmBuffer == nullptr || outputPcmBuffer == nullptr) {
        AUDIO_ERR_LOG("copypcmbuffer function get null paras.");
        return ERROR;
    }
    float *inputData = inputPcmBuffer->GetPcmDataBuffer();
    uint32_t inFrameSize = inputPcmBuffer->GetFrameLen() * sizeof(float);
    float *outputData = outputPcmBuffer->GetPcmDataBuffer();
    uint32_t outFrameSize = outputPcmBuffer->GetFrameLen() * sizeof(float);

    int32_t ret = memcpy_s(outputData, outFrameSize, inputData, inFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "AudioSuiteVoiceBeautifierNode PcmBuffer copy failed: %{public}d",
        ret);
    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierNode::DoChannelConvert(
    AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    AudioChannelInfo inChannelInfo = {inputPcmBuffer->GetChannelLayout(), inputPcmBuffer->GetChannelCount()};
    AudioChannelInfo outChannelInfo = {VM_ALGO_CHANNEL_LAYOUT, VM_ALGO_CHANNEL_COUNT};
    SetChannelConvertProcessParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);

    uint32_t frameSize = inputPcmBuffer->GetFrameLen() / inputPcmBuffer->GetChannelCount();
    float *inputData = inputPcmBuffer->GetPcmDataBuffer();
    uint32_t inLen = inputPcmBuffer->GetFrameLen() * sizeof(float);
    float *outputData = outputPcmBuffer->GetPcmDataBuffer();
    uint32_t outLen = outputPcmBuffer->GetFrameLen() * sizeof(float);
    AUDIO_INFO_LOG(
        "DoChannelConvert: frameSize: %{public}u, inLen: %{public}u, outLen: %{public}u",
        frameSize,
        inLen,
        outLen);

    int32_t ret = ChannelConvertProcess(frameSize, inputData, inLen, outputData, outLen);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DoChannelConvert failed: %{public}d", ret);

    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierNode::DoResample(
    AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    uint32_t inRate = inputPcmBuffer->GetSampleRate();
    uint32_t outRate = outputPcmBuffer->GetSampleRate();
    uint32_t channelCount = inputPcmBuffer->GetChannelCount();
    CHECK_AND_RETURN_RET_LOG(channelCount != 0, ERROR, "Invalid ChannelCount: %{public}d", channelCount);

    AUDIO_INFO_LOG(
        "DoResample: inSampleRate: %{public}u, outSampleRate: %{public}u, channelCount: %{public}u",
        inRate,
        outRate,
        channelCount);
    int32_t ret = SetUpResample(inRate, outRate, channelCount, RESAMPLE_QUALITY);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetUpResample failed: %{public}d", ret);

    float *inputData = inputPcmBuffer->GetPcmDataBuffer();
    uint32_t inFrameSize = inputPcmBuffer->GetFrameLen() / channelCount;
    float *outputData = outputPcmBuffer->GetPcmDataBuffer();
    uint32_t outFrameSize = outputPcmBuffer->GetFrameLen() / channelCount;
    ret = DoResampleProcess(inputData, inFrameSize, outputData, outFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DoResampleProcess failed: %{public}d", ret);

    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierNode::ConvertProcess(AudioSuitePcmBuffer *inputPcmBuffer)
{
    uint32_t inChannelCount = inputPcmBuffer->GetChannelCount();
    uint32_t outChannelCount = VM_ALGO_CHANNEL_COUNT;
    uint32_t inSampleRate = inputPcmBuffer->GetSampleRate();
    uint32_t outSampleRate = VM_ALGO_SAMPLE_RATE;
    AUDIO_INFO_LOG("ConvertProcess inChannelCount: %{public}u, outChannelCount: %{public}u,"
                   "inSampleRate: %{public}u, outSampleRate: %{public}u",
        inChannelCount,
        outChannelCount,
        inSampleRate,
        outSampleRate);

    int32_t ret;
    if (inChannelCount == outChannelCount && inSampleRate == outSampleRate) {
        ret = CopyPcmBuffer(inputPcmBuffer, &pcmBufferOutput_);
    } else if (inChannelCount == outChannelCount) {
        ret = DoResample(inputPcmBuffer, &pcmBufferOutput_);
    } else if (inSampleRate == outSampleRate) {
        ret = DoChannelConvert(inputPcmBuffer, &pcmBufferOutput_);
    } else {
        // 采样率和声道数都不同，先做声道转换。
        channelConvertPcmBuffer_.ResetPcmBuffer(inputPcmBuffer->GetSampleRate(),
            VM_ALGO_CHANNEL_COUNT, VM_ALGO_CHANNEL_LAYOUT);
        ret = DoChannelConvert(inputPcmBuffer, &channelConvertPcmBuffer_);
        CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
        ret = DoResample(&channelConvertPcmBuffer_, &pcmBufferOutput_);
    }
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);

    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteVoiceBeautifierNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    if (inputs.empty()) {
        AUDIO_ERR_LOG("SignalProcess inputs list is empty");
        return &pcmBufferOutput_;
    }

    if (inputs[0] == nullptr) {
        AUDIO_ERR_LOG("SignalProcess input data is nullptr");
        return &pcmBufferOutput_;
    }

    // 声道转换及采样率转换
    int32_t ret = ConvertProcess(inputs[0]);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioSuiteVoiceBeautifierNode ConverProcess fail.");
        return &pcmBufferOutput_;
    }

    // 位深转换 float -> SAMPLE_S16LE
    ConvertFromFloat(VM_ALGO_SAMPLE_FORMAT,
        pcmBufferOutput_.GetFrameLen(),
        pcmBufferOutput_.GetPcmDataBuffer(),
        static_cast<void *>(algoInputBuffer_.data()));

    // 调算法
    std::vector<uint8_t *> vmAlgoInputs(1);
    std::vector<uint8_t *> vmAlgoOutputs(1);
    vmAlgoInputs[0] = algoInputBuffer_.data();
    vmAlgoOutputs[0] = algoOutputBuffer_.data();

    AUDIO_DEBUG_LOG("start apply algo.");
    if (algoInterface_ == nullptr) {
        AUDIO_ERR_LOG("AudioSuiteVoiceBeautifierNode algoInterface is null.");
        return &pcmBufferOutput_;
    }
    ret = algoInterface_->Apply(vmAlgoInputs, vmAlgoOutputs);
    if (ret != SUCCESS) {
        return &pcmBufferOutput_;
    }
    AUDIO_DEBUG_LOG("end apply algo.");
    // 位深转换 SAMPLE_S16LE -> float
    ConvertToFloat(VM_ALGO_SAMPLE_FORMAT,
        pcmBufferOutput_.GetFrameLen(),
        static_cast<void *>(algoOutputBuffer_.data()),
        pcmBufferOutput_.GetPcmDataBuffer());
    AUDIO_DEBUG_LOG("signalprocess end.");

    return &pcmBufferOutput_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS