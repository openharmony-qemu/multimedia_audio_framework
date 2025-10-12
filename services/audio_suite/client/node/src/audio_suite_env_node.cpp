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
#define LOG_TAG "AudioSuiteEnvNode"
#endif

#include <vector>
#include <memory>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_env_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
AudioSuiteEnvNode::AudioSuiteEnvNode()
    : AudioSuiteProcessNode(
          NODE_TYPE_EQUALIZER, AudioFormat{{CH_LAYOUT_STEREO, ALGO_CHANNEL_NUM}, SAMPLE_S16LE, SAMPLE_RATE_48000}),
      outPcmBuffer_(SAMPLE_RATE_48000, ALGO_CHANNEL_NUM, CH_LAYOUT_STEREO)
{}

AudioSuiteEnvNode::~AudioSuiteEnvNode()
{
    if (isInit_) {
        DeInit();
    }
}

int32_t AudioSuiteEnvNode::Init()
{
    if (isInit_) {
        AUDIO_ERR_LOG("AudioSuiteEnvNode::Init failed, already inited");
        return ERROR;
    }
    envAlgoInterfaceImpl_ = std::make_shared<AudioSuiteEnvAlgoInterfaceImpl>();
    int32_t ret = envAlgoInterfaceImpl_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "envAlgoInterfaceImpl Init failed");
    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteEnvNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEnvNode::DeInit()
{
    tmpin_.resize(0);
    tmpout_.resize(0);
    if (envAlgoInterfaceImpl_ != nullptr) {
        envAlgoInterfaceImpl_->Deinit();
        envAlgoInterfaceImpl_ = nullptr;
    }

    if (isInit_) {
        isInit_ = false;
        AUDIO_INFO_LOG("AudioSuiteEnvNode::DeInit end");
        return SUCCESS;
    }
    return ERROR;
}

bool AudioSuiteEnvNode::Reset()
{
    return true;
}

int32_t AudioSuiteEnvNode::preProcess(AudioSuitePcmBuffer *inputPcmBuffer)
{
    uint32_t inChannelCount = inputPcmBuffer->GetChannelCount();
    uint32_t inSampleRate = inputPcmBuffer->GetSampleRate();

    int32_t ret;
    if (inChannelCount == ALGO_CHANNEL_NUM && inSampleRate == outPcmBuffer_.GetSampleRate()) {
        ret = CopyBuffer(inputPcmBuffer, &outPcmBuffer_);
    } else if (inChannelCount == ALGO_CHANNEL_NUM) {
        ret = DoResample(inputPcmBuffer, &outPcmBuffer_);
    } else {
        AUDIO_ERR_LOG("Don't support channel convert now");
        return ERROR;
    }
    CHECK_AND_RETURN_RET(ret == SUCCESS, ret);
    return SUCCESS;
}

int32_t AudioSuiteEnvNode::CopyBuffer(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    float *inputData = (*inputPcmBuffer).GetPcmDataBuffer();
    uint32_t inFrameSize = (*inputPcmBuffer).GetFrameLen() * sizeof(float);
    float *outputData = (*outputPcmBuffer).GetPcmDataBuffer();
    uint32_t outFrameSize = (*outputPcmBuffer).GetFrameLen() * sizeof(float);
    int32_t ret = memcpy_s(outputData, outFrameSize, inputData, inFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CopyBuffer failed.");
    return SUCCESS;
}

int32_t AudioSuiteEnvNode::DoResample(AudioSuitePcmBuffer *inputPcmBuffer, AudioSuitePcmBuffer *outputPcmBuffer)
{
    uint32_t inRate = (*inputPcmBuffer).GetSampleRate();
    uint32_t outRate = SAMPLE_RATE_48000;
    uint32_t channelCount = (*inputPcmBuffer).GetChannelCount();
    if (channelCount == 0) {
        AUDIO_ERR_LOG("InputPcmBuffer ChannelCount is zero!");
        return ERROR;
    }
    int32_t ret = SetUpResample(inRate, outRate, channelCount, RESAMPLE_QUALITY);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetUpResample failed with error code %{public}d", ret);

    float *inputData = (*inputPcmBuffer).GetPcmDataBuffer();
    uint32_t inFrameSize = (*inputPcmBuffer).GetFrameLen() / channelCount;
    float *outputData = (*outputPcmBuffer).GetPcmDataBuffer();
    uint32_t outFrameSize = (*outputPcmBuffer).GetFrameLen() / channelCount;
    ret = DoResampleProcess(inputData, inFrameSize, outputData, outFrameSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DoResampleProcess failed with error code %{public}d", ret);
    AUDIO_DEBUG_LOG("AudioSuiteEnvNode::DoReample finished");
    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteEnvNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    if (inputs.empty()) {
        AUDIO_ERR_LOG("AudioSuiteEnvNode SignalProcess inputs is empty");
        return nullptr;
    } else {
        AUDIO_DEBUG_LOG("AudioSuiteEnvNode SignalProcess inputs frameLen:%{public}d", inputs[0]->GetFrameLen());
    }

    inputDataBuffer_.resize(outPcmBuffer_.GetFrameLen() * ALGO_BYTE_NUM);
    outputDataBuffer_.resize(outPcmBuffer_.GetFrameLen() * ALGO_BYTE_NUM);
    int32_t ret = preProcess(inputs[0]);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, &outPcmBuffer_, "AudioSuiteEnvNode SignalProcess preProCess failed");

    ConvertFromFloat(SAMPLE_S16LE,
        outPcmBuffer_.GetFrameLen(),
        outPcmBuffer_.GetPcmDataBuffer(),
        static_cast<void *>(inputDataBuffer_.data()));

    tmpin_.resize(1);
    tmpout_.resize(1);
    uint8_t *inputPointer = inputDataBuffer_.data();
    uint8_t *outputPointer = outputDataBuffer_.data();

    tmpin_[0] = inputPointer;
    tmpout_[0] = outputPointer;
    ret = envAlgoInterfaceImpl_->Apply(tmpin_, tmpout_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, &outPcmBuffer_, "AudioSuiteEnvNode SignalProcess Apply failed");

    ConvertToFloat(
        SAMPLE_S16LE, outPcmBuffer_.GetFrameLen(), outputDataBuffer_.data(), outPcmBuffer_.GetPcmDataBuffer());
    return &outPcmBuffer_;
}

int32_t AudioSuiteEnvNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("AudioSuiteEnvNode::SetOptions Enter");
    if (name == "EnvironmentType") {
        envAlgoInterfaceImpl_->SetParameter(value, value);
        AUDIO_INFO_LOG("SetOptions SUCCESS");
        return SUCCESS;
    } else {
        AUDIO_ERR_LOG("SetOptions Unknow Type %{public}s", name.c_str());
        return ERROR;
    }
}
}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS