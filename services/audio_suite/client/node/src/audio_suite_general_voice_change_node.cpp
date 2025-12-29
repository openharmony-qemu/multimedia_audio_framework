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
#define LOG_TAG "AudioSuiteGeneralVoiceChangeNode"
#endif

#include "audio_suite_general_voice_change_node.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioChannelLayout VM_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
const std::string setVoiceChangeMode = "AudioGeneralVoiceChangeType";
}

AudioSuiteGeneralVoiceChangeNode::AudioSuiteGeneralVoiceChangeNode()
    : AudioSuiteProcessNode(NODE_TYPE_GENERAL_VOICE_CHANGE)
{}

AudioSuiteGeneralVoiceChangeNode::~AudioSuiteGeneralVoiceChangeNode()
{
    DeInit();
}

int32_t AudioSuiteGeneralVoiceChangeNode::Init()
{
    AUDIO_INFO_LOG("AudioSuiteGeneralVoiceChangeNode Init begin");
    algoInterface_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_GENERAL_VOICE_CHANGE, nodeCapability);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "Failed to create General Voice Change algoInterface");
    CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to Init General Voice Change Algo");

    InitAudioFormat(AudioFormat{{VM_ALGO_CHANNEL_LAYOUT, nodeCapability.inChannels},
        static_cast<AudioSampleFormat>(nodeCapability.inFormat),
        static_cast<AudioSamplingRate>(nodeCapability.inSampleRate)});
    
    pcmBufferOutput_ = AudioSuitePcmBuffer(PcmBufferFormat{static_cast<AudioSamplingRate>(nodeCapability.outSampleRate),
        nodeCapability.outChannels,
        VM_ALGO_CHANNEL_LAYOUT,
        static_cast<AudioSampleFormat>(nodeCapability.outFormat)});
    CHECK_AND_RETURN_RET_LOG(nodeCapability.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = nodeCapability.frameLen / nodeCapability.inSampleRate * MILLISECONDS_TO_MICROSECONDS;

    AUDIO_INFO_LOG("AudioSuiteGeneralVoiceChangeNode Init end");
    return SUCCESS;
}

int32_t AudioSuiteGeneralVoiceChangeNode::DeInit()
{
    AUDIO_INFO_LOG("AudioSuiteGeneralVoiceChangeNode DeInit begin");
    if (algoInterface_ != nullptr) {
        int32_t ret = algoInterface_->Deinit();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "Failed to DeInit voice beautifier algorithm");
        algoInterface_ = nullptr;
    }

    AUDIO_INFO_LOG("AudioSuiteGeneralVoiceChangeNode DeInit end");
    return SUCCESS;
}

int32_t AudioSuiteGeneralVoiceChangeNode::SetOptions(std::string name, std::string value)
{
    CHECK_AND_RETURN_RET_LOG(name == setVoiceChangeMode, ERROR, "SetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algoInterfaceImpl_ is nullptr");

    paraName_ = name;
    paraValue_ = value;

    int32_t ret = algoInterface_->SetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetParameter failed %{public}d", ret);
    AUDIO_INFO_LOG("SetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteGeneralVoiceChangeNode::GetOptions(std::string name, std::string &value)
{
    CHECK_AND_RETURN_RET_LOG(name == setVoiceChangeMode, ERROR, "GetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(!paraValue_.empty(), ERROR, "paraValue_ is empty");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algo interface is null, need Init first");
    value = paraValue_;
    AUDIO_INFO_LOG("GetOptions SUCCESS");
    return SUCCESS;
}

AudioSuitePcmBuffer *AudioSuiteGeneralVoiceChangeNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    Trace trace("AudioSuiteGeneralVoiceChangeNode::SignalProcess Start");
    CHECK_AND_RETURN_RET_LOG(
        !inputs.empty(), nullptr, "AudioSuiteGeneralVoiceChangeNode SignalProcess inputs is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr && inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()),
        nullptr,
        "AudioSuiteGeneralVoiceChangeNode SignalProcess inputs[0] is nullptr");

    tmpin_[0] = inputs[0]->GetPcmData();
    tmpout_[0] = pcmBufferOutput_.GetPcmData();
    CHECK_AND_RETURN_RET_LOG(tmpout_[0] != nullptr && tmpin_[0] != nullptr, nullptr, "tmpin or tempout is nullptr");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, nullptr, "algoInterfaceImpl_ is nullptr");
    int32_t ret = algoInterface_->Apply(tmpin_, tmpout_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "AudioSuiteGeneralVoiceChangeNode SignalProcess Apply failed");
    trace.End();
    return &pcmBufferOutput_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS