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
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr AudioChannelLayout NR_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_MONO;
}

AudioSuiteNrNode::AudioSuiteNrNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_NOISE_REDUCTION)
{}

AudioSuiteNrNode::~AudioSuiteNrNode()
{
    DeInit();
}

int32_t AudioSuiteNrNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteNrNode::Init begin");
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
    }

    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_NOISE_REDUCTION, nodeParameter);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create nr algoInterface");

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to Init nr algorithm");

    SetAudioNodeFormat(AudioFormat{{NR_ALGO_CHANNEL_LAYOUT, nodeParameter.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter.inSampleRate)});
    outPcmBuffer_.ResizePcmBuffer(PcmBufferFormat{static_cast<AudioSamplingRate>(nodeParameter.outSampleRate),
        nodeParameter.outChannels,
        NR_ALGO_CHANNEL_LAYOUT,
        static_cast<AudioSampleFormat>(nodeParameter.outFormat)});
    CHECK_AND_RETURN_RET_LOG(nodeParameter.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = (nodeParameter.frameLen * MILLISECONDS_TO_MICROSECONDS) / nodeParameter.inSampleRate;

    AUDIO_INFO_LOG("AudioSuiteNrNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteNrNode::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteNrNode::DeInit begin");

    if (algoInterface_ != nullptr) {
        algoInterface_->Deinit();
        algoInterface_.reset();
    }

    AUDIO_INFO_LOG("AudioSuiteNrNode::DeInit end");
    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteNrNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "Inputs list is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, nullptr, "Input data is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), nullptr, "Invalid inputs format");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, nullptr, "algoInterface is nullptr, need Init first");

    uint32_t inputDataSize = inputs[0]->GetDataSize();
    uint32_t frameSize = nodeParameter.frameLen * sizeof(int16_t);
    uint32_t frameCount = inputDataSize / frameSize;
    CHECK_AND_RETURN_RET_LOG(inputDataSize % frameSize == 0, nullptr, "Invalid inputPcmBuffer size");

    uint8_t *inDataPtr = inputs[0]->GetPcmData();
    uint8_t *outDataPtr = outPcmBuffer_.GetPcmData();
    // apply algo for every frame
    for (uint32_t i = 0; i < frameCount; i++) {
        algoInputs_[0] = inDataPtr;
        algoOutputs_[0] = outDataPtr;

        int32_t ret = algoInterface_->Apply(algoInputs_, algoOutputs_);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "NR node Apply algo fail");

        inDataPtr += frameSize;
        outDataPtr += frameSize;
    }

    return &outPcmBuffer_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS