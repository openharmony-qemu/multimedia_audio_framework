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
#define LOG_TAG "AudioSuitePureVoiceChangeNode"
#endif

#include "audio_suite_pure_voice_change_node.h"
#include <fstream>
#include "audio_utils.h"


namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static constexpr AudioSamplingRate VMPH_ALGO_SAMPLE_RATE = SAMPLE_RATE_16000;
static constexpr AudioSampleFormat VMPH_ALGO_SAMPLE_FORMAT = SAMPLE_S16LE;
static constexpr AudioChannel VMPH_ALGO_CHANNEL_COUNT = STEREO;
static constexpr AudioChannelLayout VMPH_ALGO_CHANNEL_LAYOUT = CH_LAYOUT_STEREO;
const std::string PURE_VOICE_CHANGE_MODE = "AudioPureVoiceChangeOption";
}  // namespace

AudioSuitePureVoiceChangeNode::AudioSuitePureVoiceChangeNode()
    : AudioSuiteProcessNode(AudioNodeType::NODE_TYPE_PURE_VOICE_CHANGE,
          AudioFormat{
              {VMPH_ALGO_CHANNEL_LAYOUT, VMPH_ALGO_CHANNEL_COUNT}, VMPH_ALGO_SAMPLE_FORMAT, VMPH_ALGO_SAMPLE_RATE}),
      outPcmBuffer_(PcmBufferFormat{
          VMPH_ALGO_SAMPLE_RATE, VMPH_ALGO_CHANNEL_COUNT, VMPH_ALGO_CHANNEL_LAYOUT, VMPH_ALGO_SAMPLE_FORMAT}),
      outTmpPcmBuffer_(
          PcmBufferFormat{
              VMPH_ALGO_SAMPLE_RATE, VMPH_ALGO_CHANNEL_COUNT, VMPH_ALGO_CHANNEL_LAYOUT, VMPH_ALGO_SAMPLE_FORMAT},
          PCM_DATA_DURATION_40_MS),
      tmpDataBuffer_(
          PcmBufferFormat{
              VMPH_ALGO_SAMPLE_RATE, VMPH_ALGO_CHANNEL_COUNT, VMPH_ALGO_CHANNEL_LAYOUT, VMPH_ALGO_SAMPLE_FORMAT},
          PCM_DATA_DURATION_40_MS)

{
    tmpData_.push_back(tmpDataBuffer_);
}

AudioSuitePureVoiceChangeNode::~AudioSuitePureVoiceChangeNode()
{
    if (isInit_) {
        DeInit();
    }
}

int32_t AudioSuitePureVoiceChangeNode::Init()
{
    if (isInit_) {
        AUDIO_ERR_LOG("AudioSuitePureVoiceChangeNode::Init failed, already inited");
        return ERROR;
    }
    algoInterfaceImpl_ =
        AudioSuiteAlgoInterface::CreateAlgoInterface(AlgoType::AUDIO_NODE_TYPE_PURE_VOICE_CHANGE, nodeCapability);
    int32_t ret = algoInterfaceImpl_->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "algoInterfaceImpl_ Init failed");
    isInit_ = true;
    AUDIO_INFO_LOG("AudioSuitePureVoiceChangeNode::Init end");
    return SUCCESS;
}

int32_t AudioSuitePureVoiceChangeNode::DeInit()
{
    tmpin_.resize(0);
    tmpout_.resize(0);
    if (algoInterfaceImpl_ != nullptr) {
        algoInterfaceImpl_->Deinit();
        algoInterfaceImpl_ = nullptr;
    }

    if (isInit_) {
        isInit_ = false;
        AUDIO_INFO_LOG("AudioSuitePureVoiceChangeNode::DeInit end");
        return SUCCESS;
    }
    return ERROR;
}

AudioSuitePcmBuffer *AudioSuitePureVoiceChangeNode::SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs)
{
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "AudioSuitePureVoiceChangeNode SignalProcess inputs is empty");
    CHECK_AND_RETURN_RET_LOG(
        inputs[0] != nullptr, nullptr, "AudioSuitePureVoiceChangeNode SignalProcess inputs[0] is nullptr");
    AUDIO_DEBUG_LOG(
        "AudioSuitePureVoiceChangeNode SignalProcess inputs frameLen:%{public}d", inputs[0]->GetSampleCount());

    tmpin_.resize(1);
    tmpout_.resize(1);

    tmpin_[0] = inputs[0]->GetPcmData();
    tmpout_[0] = outTmpPcmBuffer_.GetPcmData();

    if (isSecondFlag) {
        int32_t ret = memcpy_s(outPcmBuffer_.GetPcmData(),
            outPcmBuffer_.GetDataSize(),  // Copy the second frame 20ms data
            outTmpPcmBuffer_.GetPcmData() + outPcmBuffer_.GetDataSize(),
            outPcmBuffer_.GetDataSize());
        CHECK_AND_RETURN_RET_LOG(ret == EOK, nullptr, "memcpy failed, ret is %{public}d.", ret);
        isSecondFlag = false;
        return &outPcmBuffer_;
    }
    isSecondFlag = true;

    CHECK_AND_RETURN_RET_LOG(algoInterfaceImpl_ != nullptr, nullptr, "algoInterfaceImpl_ is nullptr");
    Trace trace("AudioSuitePureVoiceChangeNode::SignalProcess Start");
    int32_t ret = algoInterfaceImpl_->Apply(tmpin_, tmpout_);
    trace.End();

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "AudioSuitePureVoiceChangeNode SignalProcess Apply failed");

    ret = memcpy_s(outPcmBuffer_.GetPcmData(),
        outPcmBuffer_.GetDataSize(),  // Copy the first frame 20ms data
        outTmpPcmBuffer_.GetPcmData(),
        outPcmBuffer_.GetDataSize());
    CHECK_AND_RETURN_RET_LOG(ret == EOK, nullptr, "memcpy failed, ret is %{public}d.", ret);

    return &outPcmBuffer_;
}

std::vector<AudioSuitePcmBuffer*>& AudioSuitePureVoiceChangeNode::ReadDoubleProcessNodePreOutputData()
{
    if (isSecondFlag) {
        return tmpDataPointers_;
    }
    tmpData_[0].Reset();  // Clear the 40ms of data required by the algorithm before storing it.
    std::vector<AudioSuitePcmBuffer*>& preOutputsFirst = ReadProcessNodePreOutputData(); // Need data for the first time
    
    int32_t ret = memcpy_s(tmpData_[0].GetPcmData(), tmpData_[0].GetDataSize(), // Copy the first frame 20ms data
        preOutputsFirst[0]->GetPcmData(), preOutputsFirst[0]->GetDataSize());
    CHECK_AND_RETURN_RET_LOG(ret == EOK, tmpDataPointers_, "memcpy failed, ret is %{public}d.", ret);
    if (GetAudioNodeDataFinishedFlag()) {
        ret = memset_s(tmpData_[0].GetPcmData() + preOutputsFirst[0]->GetDataSize(), // Copy the second frame 20ms data
                       tmpData_[0].GetDataSize() - preOutputsFirst[0]->GetDataSize(),
                       0,
                       preOutputsFirst[0]->GetDataSize());
        CHECK_AND_RETURN_RET_LOG(ret == EOK, tmpDataPointers_, "memset failed, ret is %{public}d.", ret);
    } else {
        std::vector<AudioSuitePcmBuffer*>& preOutputsSecond = ReadProcessNodePreOutputData();  // Need second data
        ret = memcpy_s(tmpData_[0].GetPcmData() + preOutputsFirst[0]->GetDataSize(), // Copy the second frame 20ms data
                       tmpData_[0].GetDataSize() - preOutputsFirst[0]->GetDataSize(),
                       preOutputsSecond[0]->GetPcmData(), preOutputsSecond[0]->GetDataSize());
        CHECK_AND_RETURN_RET_LOG(ret == EOK, tmpDataPointers_, "memcpy failed, ret is %{public}d.", ret);
    }

    for (AudioSuitePcmBuffer& buffer : tmpData_) {
        tmpDataPointers_.push_back(&buffer);
    }
    return tmpDataPointers_;
}

int32_t AudioSuitePureVoiceChangeNode::DoProcess()
{
    if (GetAudioNodeDataFinishedFlag()) {
        AUDIO_DEBUG_LOG("Current node type = %{public}d does not have more data to process.", GetNodeType());
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
    std::vector<AudioSuitePcmBuffer*>& preOutputs = ReadDoubleProcessNodePreOutputData();  // Returns 40ms PCM buffer
    if ((GetNodeBypassStatus() == false) && !preOutputs.empty()) {
        AUDIO_DEBUG_LOG("node type = %{public}d need do SignalProcess.", GetNodeType());
        tempOut = SignalProcess(preOutputs);
        CHECK_AND_RETURN_RET_LOG(tempOut != nullptr,
            ERR_OPERATION_FAILED,
            "node %{public}d do SignalProcess failed, return a nullptr",
            GetNodeType());
    } else if (!preOutputs.empty()) {
        AUDIO_DEBUG_LOG("node type = %{public}d signalProcess is not enabled.", GetNodeType());
        tempOut = preOutputs[0];
        CHECK_AND_RETURN_RET_LOG(
            tempOut != nullptr, ERR_INVALID_READ, "node %{public}d get a null pcmbuffer from prenode", GetNodeType());
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

int32_t AudioSuitePureVoiceChangeNode::SetOptions(std::string name, std::string value)
{
    AUDIO_INFO_LOG("AudioSuitePureVoiceChangeNode::SetOptions Enter");
    CHECK_AND_RETURN_RET_LOG(name == PURE_VOICE_CHANGE_MODE, ERROR, "SetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(algoInterfaceImpl_ != nullptr, ERROR, "algoInterfaceImpl_ is nullptr");
    
    paraName_ = name;
    paraValue_ = value;
    int32_t ret = algoInterfaceImpl_->SetParameter(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetParameter failed");
    AUDIO_INFO_LOG("SetOptions SUCCESS");
    return SUCCESS;
}

int32_t AudioSuitePureVoiceChangeNode::GetOptions(std::string name, std::string &value)
{
    CHECK_AND_RETURN_RET_LOG(!paraValue_.empty(), ERROR, "voicePureType is empty.");
    CHECK_AND_RETURN_RET_LOG(name == PURE_VOICE_CHANGE_MODE, ERROR, "GetOptions Unknow Type %{public}s", name.c_str());
    CHECK_AND_RETURN_RET_LOG(algoInterfaceImpl_ != nullptr, ERROR, "algoInterfaceImpl_ is nullptr");
    
    value = paraValue_;
    return SUCCESS;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS