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
#define LOG_TAG "AudioSuiteAissNode"
#endif

#include "audio_suite_aiss_node.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

constexpr uint16_t DEFAULT_CHANNEL_COUNT = 2;
constexpr uint16_t DEFAULT_CHANNEL_COUNT_OUT = 4;

AudioSuiteAissNode::AudioSuiteAissNode()
    : AudioSuiteProcessNode(NODE_TYPE_AUDIO_SEPARATION, AudioFormat{{CH_LAYOUT_STEREO,
        DEFAULT_CHANNEL_COUNT}, SAMPLE_F32LE, SAMPLE_RATE_48000}),
    tmpOutput_(PcmBufferFormat(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT_OUT, CH_LAYOUT_QUAD, SAMPLE_F32LE)),
    tmpHumanSoundOutput_(PcmBufferFormat(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT, CH_LAYOUT_STEREO, SAMPLE_F32LE)),
    tmpBkgSoundOutput_(PcmBufferFormat(SAMPLE_RATE_48000, DEFAULT_CHANNEL_COUNT, CH_LAYOUT_STEREO, SAMPLE_F32LE))
{
    AUDIO_INFO_LOG("AudioSuiteAissNode create success");
}

AudioSuiteAissNode::~AudioSuiteAissNode()
{
    DeInit();
}

int32_t AudioSuiteAissNode::DoProcess()
{
    CHECK_AND_RETURN_RET_LOG(GetAudioNodeDataFinishedFlag() != true, SUCCESS, "AudioSuiteProcessNode"
        "DoProcess:Current node type = %{public}d does not have more data to process.", GetNodeType());
    CHECK_AND_RETURN_RET_LOG(Init() == SUCCESS, ERROR, "AudioSuiteAissNode init failed");
    CHECK_AND_RETURN_RET_LOG(inputStream_ != nullptr, ERR_INVALID_PARAM,
        "node type = %{public}d inputstream is null!", GetNodeType());
    if (!outputStream_) {
        outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
    }

    AudioSuitePcmBuffer* tempOut = nullptr;
    std::vector<AudioSuitePcmBuffer*>& preOutputs = ReadProcessNodePreOutputData();
    if ((GetNodeBypassStatus() == false) && !preOutputs.empty()) {
        AUDIO_DEBUG_LOG("AudioSuiteProcessNode::DoProcess: node type = %{public}d need "
            "do SignalProcess.", GetNodeType());
        Trace trace("AudioSuiteAissNode::SignalProcess Start");
        tempOut = SignalProcess(preOutputs);
        trace.End();
        if (tempOut == nullptr) {
            AUDIO_ERR_LOG("AudioSuiteProcessNode::DoProcess: node %{public}d do SignalProcess failed, "
                "return a nullptr", GetNodeType());
            return ERR_OPERATION_FAILED;
        }
        tmpHumanSoundOutput_.SetIsFinished(GetAudioNodeDataFinishedFlag());
        tmpBkgSoundOutput_.SetIsFinished(GetAudioNodeDataFinishedFlag());
        outputStream_->WriteDataToOutput(&tmpHumanSoundOutput_);
        outputStream_->WriteDataToOutput(&tmpBkgSoundOutput_);
    } else if (!preOutputs.empty()) {
        AUDIO_DEBUG_LOG("AudioSuiteProcessNode::DoProcess: node type = %{public}d signalProcess "
            "is not enabled.", GetNodeType());
        tempOut = preOutputs[0];
        if (tempOut == nullptr) {
            AUDIO_ERR_LOG("AudioSuiteProcessNode::DoProcess: node %{public}d get a null pcmbuffer "
                "from prenode", GetNodeType());
            return ERR_INVALID_READ;
        }
        tempOut->SetIsFinished(GetAudioNodeDataFinishedFlag());
        tmpHumanSoundOutput_ = *tempOut;
        tmpBkgSoundOutput_ = *tempOut;
        outputStream_->WriteDataToOutput(tempOut);
        outputStream_->WriteDataToOutput(tempOut);
    } else {
        AUDIO_ERR_LOG("AudioSuiteProcessNode::DoProcess: node %{public}d can't get "
            "pcmbuffer from prenodes", GetNodeType());
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioSuiteAissNode::Init()
{
    if (isInit_ == true) {
        AUDIO_DEBUG_LOG("AudioSuiteAissNode has inited");
        return SUCCESS;
    }
    if (!aissAlgo_) {
        aissAlgo_ =
            AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_AUDIO_SEPARATION, nodeCapability);
    }
    
    if (!outputStream_) {
        outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
    }

    if (aissAlgo_->Init() != SUCCESS) {
        AUDIO_ERR_LOG("InitAlgorithm failed");
        return ERROR;
    }
    isInit_ = true;
    AUDIO_DEBUG_LOG("AudioSuiteAissNode Init success");
    return SUCCESS;
}

int32_t AudioSuiteAissNode::DeInit()
{
    isInit_ = false;
    if (aissAlgo_ != nullptr) {
        aissAlgo_->Deinit();
    }
    aissAlgo_ = nullptr;
    
    AUDIO_DEBUG_LOG("AudioSuiteAissNode DeInit success");
    return SUCCESS;
}

AudioSuitePcmBuffer* AudioSuiteAissNode::SignalProcess(const std::vector<AudioSuitePcmBuffer*>& inputs)
{
    CHECK_AND_RETURN_RET_LOG(aissAlgo_ != nullptr, nullptr, "aissAlgo_ is nullptr, need Init first");
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "Inputs list is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, nullptr, "Input data is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputs[0]->IsSameFormat(GetAudioNodeInPcmFormat()), nullptr, "Invalid input format");
    uint32_t background = 2;
    uint32_t humanVoice = 1;

    tmpin_[0] = inputs[0]->GetPcmData();
    tmpout_[0] = tmpOutput_.GetPcmData();
    tmpout_[humanVoice] = tmpHumanSoundOutput_.GetPcmData();
    tmpout_[background] = tmpBkgSoundOutput_.GetPcmData();

    int32_t ret = aissAlgo_->Apply(tmpin_, tmpout_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "AudioSuiteAissNode SignalProcess Apply failed");

    AUDIO_DEBUG_LOG("AudioSuiteAissNode SignalProcess success");
    return &tmpOutput_;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS