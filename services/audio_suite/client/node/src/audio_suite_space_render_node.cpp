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
#define LOG_TAG "AudioSpaceRenderNode"
#endif
 
#include "audio_suite_space_render_node.h"
#include <fstream>
#include "audio_utils.h"
#include "audio_suite_log.h"
 
namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
 
namespace {
static constexpr AudioChannelLayout SPACE_RENDER_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
}  // namespace

AudioSuiteSpaceRenderNode::AudioSuiteSpaceRenderNode()
    : AudioSuiteProcessNode(NODE_TYPE_SPACE_RENDER)
{
}

AudioSuiteSpaceRenderNode::~AudioSuiteSpaceRenderNode()
{
    if (isInit_) {
        DeInit();
    }
}

int32_t AudioSuiteSpaceRenderNode::Init()
{
    if (isInit_) {
        AUDIO_ERR_LOG("AudioSuiteSpaceRenderNode::Init failed, already inited");
        return ERROR;
    }
    
    if (!isOutputPortInit_) {
        CHECK_AND_RETURN_RET_LOG(InitOutputStream() == SUCCESS, ERROR, "Init OutPutStream error");
        isOutputPortInit_ = true;
    }

    algoInterface_ = AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_SPACE_RENDER,
        nodeParameter);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algoInterface_ CreateAlgoInterface failed");

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "algoInterface_ Init failed");
    SetAudioNodeFormat(AudioFormat{{SPACE_RENDER_ALGO_CHANNEL_LAYOUT, nodeParameter.inChannels},
        static_cast<AudioSampleFormat>(nodeParameter.inFormat),
        static_cast<AudioSamplingRate>(nodeParameter.inSampleRate)});

    outPcmBuffer_.ResizePcmBuffer(PcmBufferFormat{static_cast<AudioSamplingRate>(nodeParameter.outSampleRate),
        nodeParameter.outChannels,
        SPACE_RENDER_ALGO_CHANNEL_LAYOUT,
        static_cast<AudioSampleFormat>(nodeParameter.outFormat)});
    CHECK_AND_RETURN_RET_LOG(nodeParameter.inSampleRate != 0, ERROR, "Invalid input SampleRate");
    pcmDurationMs_ = (nodeParameter.frameLen * MILLISECONDS_TO_MICROSECONDS) / nodeParameter.inSampleRate;

    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuiteSpaceRenderNode::Init end");
    return SUCCESS;
}

int32_t AudioSuiteSpaceRenderNode::DeInit()
{
    if (algoInterface_ != nullptr) {
        algoInterface_->Deinit();
        algoInterface_ = nullptr;
    }
 
    if (isInit_) {
        isInit_ = false;
        AUDIO_INFO_LOG("AudioSuiteSpaceRenderNode::DeInit end");
        return SUCCESS;
    }
 
    return ERROR;
}

AudioSuitePcmBuffer *AudioSuiteSpaceRenderNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    int32_t ret;
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "AudioSuiteSpaceRenderNode SignalProcess inputs is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, nullptr,
        "AudioSuiteSpaceRenderNode SignalProcess inputs[0] is nullptr");
    CHECK_AND_RETURN_RET_LOG(outPcmBuffer_.GetPcmData() != nullptr, nullptr, "outPcmBuffer_ GetPcmData is nullptr");

    std::vector<uint8_t *> dataInPcm = {inputs[0]->GetPcmData()};
    std::vector<uint8_t *> dataOutPcm = {outPcmBuffer_.GetPcmData()};
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, nullptr, "algoInterface_ is nullptr");

    ret = algoInterface_->Apply(dataInPcm, dataOutPcm);

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "AudioSuiteSpaceRenderNode SignalProcess Apply failed");

    return &outPcmBuffer_;
}

int32_t AudioSuiteSpaceRenderNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("AudioSuiteSpaceRenderNode::SetOptions Enter");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algoInterface_ is nullptr");

    paraName_ = name;
    paraValue_ = value;

    int32_t ret = algoInterface_->SetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetParameter failed ret: %{public}d", ret);
    AUDIO_INFO_LOG("SetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuiteSpaceRenderNode::GetOptions(std::string name, std::string &value)
{
    AUDIO_INFO_LOG("AudioSuiteSpaceRenderNode::GetOptions Enter");
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algoInterface_ is nullptr");
    
    int32_t ret = algoInterface_->GetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "GetParameter failed");
    AUDIO_INFO_LOG("GetOptions SUCCESS");
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS