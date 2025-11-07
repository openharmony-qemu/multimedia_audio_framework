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

AudioSuiteVoiceBeautifierNode::AudioSuiteVoiceBeautifierNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_VOICE_BEAUTIFIER,
          AudioFormat{{VM_ALGO_CHANNEL_LAYOUT, VM_ALGO_CHANNEL_COUNT}, VM_ALGO_SAMPLE_FORMAT, VM_ALGO_SAMPLE_RATE}),
      outPcmBuffer_(
          PcmBufferFormat{VM_ALGO_SAMPLE_RATE, VM_ALGO_CHANNEL_COUNT, VM_ALGO_CHANNEL_LAYOUT, VM_ALGO_SAMPLE_FORMAT})
{}

AudioSuiteVoiceBeautifierNode::~AudioSuiteVoiceBeautifierNode()
{
    DeInit();
}

int32_t AudioSuiteVoiceBeautifierNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteVoiceBeautifierNode Init begin");
    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_VOICE_BEAUTIFIER, nodeCapability);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create voice beautifier algoInterface");

    int32_t ret = algoInterface_->Init();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to Init voice beautifier algorithm.");
        DeInit();
        return ret;
    }

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
    CHECK_AND_RETURN_RET_LOG(name == "VoiceBeautifierType", ERROR, "wrong options name.");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algo interface is null, need Init first");

    if (algoInterface_->SetParameter(name, value)) {
        AUDIO_ERR_LOG("SetOptions fail.");
        DeInit();
        return ERROR;
    }
    paraName_ = name;
    paraValue_ = value;
    return SUCCESS;
}

int32_t AudioSuiteVoiceBeautifierNode::GetOptions(std::string name, std::string &value)
{
    CHECK_AND_RETURN_RET_LOG(!paraValue_.empty(), ERROR, "voiceBeautifierType is empty.");
    CHECK_AND_RETURN_RET_LOG(name == "VoiceBeautifierType", ERROR, "wrong options name.");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algo interface is null, need Init first");

    value = paraValue_;
    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteVoiceBeautifierNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, nullptr, "algoInterface is nullptr, need Init first");
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "Inputs list is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, inputs[0], "Input data is nullptr");
    CHECK_AND_RETURN_RET_LOG(!paraValue_.empty(), nullptr, "voiceBeautifierType is empty, skip signalProccess.");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), nullptr, "Invalid input format");

    // 调算法
    AUDIO_DEBUG_LOG("start apply algo.");
    vmAlgoInputs_[0] = inputs[0]->GetPcmData();
    vmAlgoOutputs_[0] = outPcmBuffer_.GetPcmData();
    int32_t ret = algoInterface_->Apply(vmAlgoInputs_, vmAlgoOutputs_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "voiceBeautifierType node Apply algo fail");
    AUDIO_DEBUG_LOG("end apply algo.");

    return &outPcmBuffer_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS