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
 
namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
 
namespace {
static constexpr AudioSamplingRate SPACE_RENDER_ALGO_SAMPLE_RATE = SAMPLE_RATE_48000;
static constexpr AudioSampleFormat SPACE_RENDER_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel SPACE_RENDER_ALGO_CHANNEL_COUNT = STEREO;
static constexpr AudioChannelLayout SPACE_RENDER_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
}  // namespace

AudioSuiteSpaceRenderNode::AudioSuiteSpaceRenderNode()
    : AudioSuiteProcessNode(NODE_TYPE_SPACE_RENDER,
          AudioFormat{{SPACE_RENDER_ALGO_CHANNEL_LAYOUT, SPACE_RENDER_ALGO_CHANNEL_COUNT},
              SPACE_RENDER_ALGO_SAMPLE_FORMAT, SPACE_RENDER_ALGO_SAMPLE_RATE}),
    doubleDataBuffer_(PcmBufferFormat{
        SPACE_RENDER_ALGO_SAMPLE_RATE, SPACE_RENDER_ALGO_CHANNEL_COUNT,
          SPACE_RENDER_ALGO_CHANNEL_LAYOUT, SPACE_RENDER_ALGO_SAMPLE_FORMAT}, PCM_DATA_DURATION_40_MS),
    outTmpPcmBuffer_(PcmBufferFormat{
        SPACE_RENDER_ALGO_SAMPLE_RATE, SPACE_RENDER_ALGO_CHANNEL_COUNT,
          SPACE_RENDER_ALGO_CHANNEL_LAYOUT, SPACE_RENDER_ALGO_SAMPLE_FORMAT}, PCM_DATA_DURATION_40_MS),
    outPcmBuffer_(PcmBufferFormat{
        SPACE_RENDER_ALGO_SAMPLE_RATE, SPACE_RENDER_ALGO_CHANNEL_COUNT,
          SPACE_RENDER_ALGO_CHANNEL_LAYOUT, SPACE_RENDER_ALGO_SAMPLE_FORMAT})
{
    readDataVector_.push_back(doubleDataBuffer_);
    emptyVector_.clear();
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
 
    algoInterface_ = AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_SPACE_RENDER,
        nodeCapability);
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, ERROR, "algoInterface_ CreateAlgoInterface failed");

    int32_t ret = algoInterface_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "algoInterface_ Init failed");
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
 
    std::vector<uint8_t *> dataInPcm = {inputs[0]->GetPcmData()};
    std::vector<uint8_t *> dataOutPcm = {outTmpPcmBuffer_.GetPcmData()};
    CHECK_AND_RETURN_RET_LOG(algoInterface_ != nullptr, nullptr, "algoInterface_ is nullptr");

    if (isSecondEnterFlag_) {
        ret = memcpy_s(outPcmBuffer_.GetPcmData(), outPcmBuffer_.GetDataSize(),
            outTmpPcmBuffer_.GetPcmData() + outPcmBuffer_.GetDataSize(),
            outTmpPcmBuffer_.GetDataSize() - outPcmBuffer_.GetDataSize());
        CHECK_AND_RETURN_RET_LOG(ret == EOK, emptyPcmBuffer_, "memcpy failed, ret is %{public}d.\n", ret);
        isSecondEnterFlag_ = false;
 
        return &outPcmBuffer_;
    }
    isSecondEnterFlag_ = true;
    ret = algoInterface_->Apply(dataInPcm, dataOutPcm);
 
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "AudioSuiteSpaceRenderNode SignalProcess Apply failed");
 
    ret = memcpy_s(outPcmBuffer_.GetPcmData(), outPcmBuffer_.GetDataSize(),
        outTmpPcmBuffer_.GetPcmData(), outPcmBuffer_.GetDataSize());
    CHECK_AND_RETURN_RET_LOG(ret == EOK, emptyPcmBuffer_, "memcpy failed, ret is %{public}d.\n", ret);
    
    return &outPcmBuffer_;
}

AudioSuitePcmBuffer *AudioSuiteSpaceRenderNode::BypassSignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    int32_t ret;
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "AudioSuiteSpaceRenderNode BypassSignalProcess inputs is empty");
    CHECK_AND_RETURN_RET_LOG(inputs[0] != nullptr, nullptr,
        "AudioSuiteSpaceRenderNode BypassSignalProcess inputs[0] is nullptr");

    if (isSecondEnterFlag_) {
        ret = memcpy_s(outPcmBuffer_.GetPcmData(), outPcmBuffer_.GetDataSize(),
            inputs[0]->GetPcmData() + outPcmBuffer_.GetDataSize(),
            inputs[0]->GetDataSize() - outPcmBuffer_.GetDataSize());
        CHECK_AND_RETURN_RET_LOG(ret == EOK, emptyPcmBuffer_, "memcpy failed, ret is %{public}d.\n", ret);
        isSecondEnterFlag_ = false;
 
        return &outPcmBuffer_;
    }
    isSecondEnterFlag_ = true;
 
    ret = memcpy_s(outPcmBuffer_.GetPcmData(), outPcmBuffer_.GetDataSize(),
        inputs[0]->GetPcmData(), outPcmBuffer_.GetDataSize());
    CHECK_AND_RETURN_RET_LOG(ret == EOK, emptyPcmBuffer_, "memcpy failed, ret is %{public}d.\n", ret);
    
    return &outPcmBuffer_;
}

std::vector<AudioSuitePcmBuffer*>& AudioSuiteSpaceRenderNode::ReadDoubleProcessNodePreOutputData()
{
    CHECK_AND_RETURN_RET_LOG(!readDataVector_.empty(), emptyVector_,
        "readDataVector_ is empty");

    if (isSecondEnterFlag_) {
        if (finishFlag_) {
            AUDIO_ERR_LOG("Data read finished.");
            SetAudioNodeDataFinishedFlag(finishFlag_);
        }
        return tmpDataPointers_;
    }

    readDataVector_[0].Reset();

    std::vector<AudioSuitePcmBuffer*>& preOutputsFirst = ReadProcessNodePreOutputData();
    CHECK_AND_RETURN_RET_LOG((!preOutputsFirst.empty() && (preOutputsFirst[0] != nullptr)), emptyVector_,
        "node %{public}d can't get pcmbuffer from prenodes", GetNodeType());

    errno_t ret = memcpy_s(readDataVector_[0].GetPcmData(), readDataVector_[0].GetDataSize(),
        preOutputsFirst[0]->GetPcmData(), preOutputsFirst[0]->GetDataSize());
    CHECK_AND_RETURN_RET_LOG(ret == EOK, emptyVector_, "memcpy failed, ret is %{public}d.\n", ret);

    if (preOutputsFirst[0]->GetIsFinished()) {
        AUDIO_ERR_LOG("Data is less than 40ms");
        ret = memset_s(readDataVector_[0].GetPcmData() + preOutputsFirst[0]->GetDataSize(),
            readDataVector_[0].GetDataSize() - preOutputsFirst[0]->GetDataSize(), 0, preOutputsFirst[0]->GetDataSize());
        CHECK_AND_RETURN_RET_LOG(ret == EOK, emptyVector_, "memset failed, ret is %{public}d.\n", ret);
    } else {
        std::vector<AudioSuitePcmBuffer*>& preOutputsSecond = ReadProcessNodePreOutputData();
        CHECK_AND_RETURN_RET_LOG((!preOutputsSecond.empty() && (preOutputsSecond[0] != nullptr)), emptyVector_,
            "node %{public}d can't get pcmbuffer from prenodes", GetNodeType());
        if (preOutputsSecond[0]->GetIsFinished()) {
            SetAudioNodeDataFinishedFlag(finishFlag_);
            finishFlag_ = true;
        }

        ret = memcpy_s(readDataVector_[0].GetPcmData() + preOutputsSecond[0]->GetDataSize(),
            readDataVector_[0].GetDataSize() - preOutputsSecond[0]->GetDataSize(), preOutputsSecond[0]->GetPcmData(),
            preOutputsSecond[0]->GetDataSize());
        CHECK_AND_RETURN_RET_LOG(ret == EOK, emptyVector_, "memcpy failed, ret is %{public}d.\n", ret);
    }

    tmpDataPointers_.clear();
    for (AudioSuitePcmBuffer& buffer : readDataVector_) {
        tmpDataPointers_.push_back(&buffer);
    }
    return tmpDataPointers_;
}

int32_t AudioSuiteSpaceRenderNode::DoProcess()
{
    if (GetAudioNodeDataFinishedFlag()) {
        AUDIO_ERR_LOG("Current node type = %{public}d does not have more data to process.", GetNodeType());
        return SUCCESS;
    }
    if (!outputStream_) {
        outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
    }
    if (!inputStream_) {
        AUDIO_ERR_LOG("node type = %{public}d inputstream is null!", GetNodeType());
        return ERR_INVALID_PARAM;
    }
    AudioSuitePcmBuffer* tempOut = nullptr;
    std::vector<AudioSuitePcmBuffer*>& preOutputs = ReadDoubleProcessNodePreOutputData();
    if ((GetNodeBypassStatus() == false) && !preOutputs.empty()) {
        AUDIO_DEBUG_LOG("node type = %{public}d need do SignalProcess.", GetNodeType());
        Trace trace("AudioSuiteSpaceRenderNode::SignalProcess Start");
        tempOut = SignalProcess(preOutputs);
        trace.End();
        CHECK_AND_RETURN_RET_LOG(tempOut != nullptr, ERR_OPERATION_FAILED,
            "node %{public}d do SignalProcess failed, return a nullptr", GetNodeType());
    } else if (!preOutputs.empty()) {
        AUDIO_DEBUG_LOG("node type = %{public}d signalProcess is not enabled.", GetNodeType());
        tempOut = BypassSignalProcess(preOutputs);
        CHECK_AND_RETURN_RET_LOG(tempOut != nullptr, ERR_INVALID_READ,
            "node %{public}d get a null pcmbuffer from prenode", GetNodeType());
    } else {
        AUDIO_ERR_LOG("node %{public}d can't get pcmbuffer from prenodes", GetNodeType());
        return ERROR;
    }
    AUDIO_DEBUG_LOG("node type = %{public}d set "
        "pcmbuffer IsFinished: %{public}d.", GetNodeType(), GetAudioNodeDataFinishedFlag());
    tempOut->SetIsFinished(GetAudioNodeDataFinishedFlag());
    outputStream_->WriteDataToOutput(tempOut);
    return SUCCESS;
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