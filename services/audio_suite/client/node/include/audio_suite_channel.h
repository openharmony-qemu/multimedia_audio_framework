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

#ifndef AUDIO_SUITE_CHANNEL_H
#define AUDIO_SUITE_CHANNEL_H

#include <vector>
#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_node.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_suite_format_conversion.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

template <typename T>
class InputPort;

template <typename T>
class OutputPort {
public:
    explicit OutputPort(std::shared_ptr<AudioNode> node) : audioNode_(node)
    {
        uint32_t sourceSepara = 2;
        convert_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
        tmpData_.resize(1);
        if (audioNode_->GetNodeType() == NODE_TYPE_AUDIO_SEPARATION) {
            convert_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
            tmpData_.resize(sourceSepara);
        }
    }
    void WriteDataToOutput(T data);
    OutputPort(const OutputPort &that) = delete;
    std::vector<T> PullOutputData(PcmBufferFormat outFormat, bool needConvert);
    int32_t PullOutputDataForDoubleFrame();
    void AddInput(InputPort<T>* input);
    void RemoveInput(InputPort<T>* input);
    size_t GetInputNum() const;
    void SetPortType(AudioNodePortType type) {portType_ = type;}
    AudioNodePortType GetPortType() {return portType_;}
    int32_t resetResampleCfg();
private:
    std::set<InputPort<T>*> inputPortSet_;
    std::vector<T> outputData_;
    std::shared_ptr<AudioNode> audioNode_;
    AudioNodePortType portType_ = AudioNodePortType::AUDIO_NODE_DEFAULT_OUTPORT_TYPE;

    std::vector<std::unique_ptr<AudioSuiteFormatConversion>> convert_;
    std::vector<AudioSuitePcmBuffer> tmpData_;
};

template <typename T>
class InputPort {
public:
    InputPort()
    {}
    ~InputPort();
    std::vector<T>& ReadPreOutputData(PcmBufferFormat outFormat, bool needConvert);

    void Connect(const std::shared_ptr<AudioNode>& node, OutputPort<T>* output);

    void DisConnect(const std::shared_ptr<AudioNode>& node);

    size_t GetPreOutputNum() const;

    const std::unordered_map<OutputPort<T>*, std::shared_ptr<AudioNode>>& GetPreOutputMap();

    bool CheckIfDisConnected(OutputPort<T>* output);

    InputPort(const InputPort &that) = delete;

    void deInit();

    void AddPreOutput(const std::shared_ptr<AudioNode>& node, OutputPort<T>* output);
    void RemovePreOutput(OutputPort<T>* output);
    std::vector<T> getInputData();
    std::vector<T>& getInputDataRef();
private:
    std::unordered_map<OutputPort<T>*, std::shared_ptr<AudioNode>> outputPorts_;
    std::vector<T> inputData_;
};

template <class T>
InputPort<T>::~InputPort()
{
}

template <class T>
std::vector<T> InputPort<T>::getInputData()
{
    return inputData_;
}

template <class T>
std::vector<T>& InputPort<T>::getInputDataRef()
{
    return inputData_;
}

template <class T>
void InputPort<T>::deInit()
{
    for (auto &o : outputPorts_) {
        if (o.first != nullptr) {
            o.first->RemoveInput(this);
        }
    }
    outputPorts_.clear();
}

template <class T>
std::vector<T>& InputPort<T>::ReadPreOutputData(PcmBufferFormat outFormat, bool needConvert)
{
    inputData_.clear();
    for (auto &o : outputPorts_) {
        if (o.first == nullptr) {
            continue;
        }

        std::vector<T> outputData = o.first->PullOutputData(outFormat, needConvert);
        inputData_.insert(inputData_.end(), outputData.begin(), outputData.end());
    }
    return inputData_;
}

template <class T>
void InputPort<T>::Connect(const std::shared_ptr<AudioNode>& node, OutputPort<T>* output)
{
    // for default type
    if (output) {
        output->AddInput(this);
    }
    AddPreOutput(node, output);
    return;
}

template <class T>
void InputPort<T>::DisConnect(const std::shared_ptr<AudioNode>& preNode)
{
    OutputPort<T>* port = nullptr;
    for (auto outputNode : outputPorts_) {
        if (outputNode.second == preNode) {
            port = outputNode.first;
            break;
        }
    }
    if (port != nullptr) {
        port->RemoveInput(this);
        RemovePreOutput(port);
    }
}

template <class T>
size_t InputPort<T>::GetPreOutputNum() const
{
    return outputPorts_.size();
}

template <class T>
const std::unordered_map<OutputPort<T>*, std::shared_ptr<AudioNode>>& InputPort<T>::GetPreOutputMap()
{
    return outputPorts_;
}

template <class T>
bool InputPort<T>::CheckIfDisConnected(OutputPort<T>* output)
{
    return outputPorts_.find(output) == outputPorts_.end();
}

template <class T>
void InputPort<T>::AddPreOutput(const std::shared_ptr<AudioNode>& node, OutputPort<T>* output)
{
    outputPorts_[output] = node;
}

template <class T>
void InputPort<T>::RemovePreOutput(OutputPort<T>* output)
{
    outputPorts_.erase(output);
}

template <class T>
void OutputPort<T>::AddInput(InputPort<T>* input)
{
    inputPortSet_.insert(input);
}

template <class T>
void OutputPort<T>::RemoveInput(InputPort<T>* input)
{
    inputPortSet_.erase(input);
}

template <class T>
size_t OutputPort<T>::GetInputNum() const
{
    return inputPortSet_.size();
}

template <class T>
void OutputPort<T>::WriteDataToOutput(T data)
{
    outputData_.emplace_back(std::move(data));
    return;
}

template <class T>
int32_t OutputPort<T>::PullOutputDataForDoubleFrame()
{
    uint32_t doubleFrame = 2;
    CHECK_AND_RETURN_RET_LOG(audioNode_ != nullptr, ERROR, "audionode is nullptr.");
    for (uint32_t frame = 0; frame < doubleFrame; frame++) {
        outputData_.clear();
        audioNode_->DoProcess();
        CHECK_AND_RETURN_RET_LOG(!outputData_.empty(), ERROR, "outputData is empty.");
        CHECK_AND_RETURN_RET_LOG(outputData_.size() == tmpData_.size(), ERROR, "input data num err.");
        for (size_t idx = 0; idx < tmpData_.size(); idx++) {
            T in = outputData_[idx];
            CHECK_AND_RETURN_RET_LOG(in != nullptr, ERROR, "outputData is nullptr.");

            if (frame == 0) {
                tmpData_[idx].ResizePcmBuffer(in->GetPcmBufferFormat(), PCM_DATA_DURATION_40_MS);
                tmpData_[idx].Reset();
            }
            int32_t ret = memcpy_s(tmpData_[idx].GetPcmData() + frame * in->GetDataSize(),
                tmpData_[idx].GetDataSize() - frame * in->GetDataSize(), in->GetPcmData(), in->GetDataSize());
            CHECK_AND_RETURN_RET_LOG(ret == EOK, ERROR, "memecpy failed, ret is %{public}d.", ret);
            tmpData_[idx].SetIsFinished(in->GetIsFinished());
        }

        CHECK_AND_RETURN_RET_LOG(outputData_[0] != nullptr, ERROR, "outputData is nullptr.");
        if (outputData_[0]->GetIsFinished()) {
            break;
        }
    }

    outputData_.clear();
    for (size_t idx = 0; idx < tmpData_.size(); idx++) {
        outputData_.push_back(&tmpData_[idx]);
    }
    return SUCCESS;
}

template <class T>
std::vector<T> OutputPort<T>::PullOutputData(PcmBufferFormat outFormat, bool needConvert)
{
    if (outFormat.sampleRate == SAMPLE_RATE_11025) {
        int32_t ret = PullOutputDataForDoubleFrame();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, std::vector<T>(), "Get double frame data fail.");
    } else {
        CHECK_AND_RETURN_RET_LOG(audioNode_ != nullptr, std::vector<T>(), "audionode is nullptr.");
        audioNode_->DoProcess();
    }
    CHECK_AND_RETURN_RET_LOG(!outputData_.empty(), std::vector<T>(), "outputData is empty.");
    CHECK_AND_RETURN_RET_LOG(outputData_.size() == convert_.size(), std::vector<T>(), "input data num err.");

    std::vector<T> outData;
    for (size_t idx = 0; idx < outputData_.size(); idx++) {
        T data = outputData_[idx];
        CHECK_AND_RETURN_RET_LOG(data != nullptr, std::vector<T>(), "outputData is nullptr.");
        CHECK_AND_RETURN_RET_LOG(convert_[idx] != nullptr, std::vector<T>(), "convert is nullptr.");

        if (!needConvert || data->IsSameFormat(outFormat)) {
            outData.push_back(data);
        } else {
            AudioSuitePcmBuffer *convertData = convert_[idx]->Process(data, outFormat);
            convertData->SetIsFinished(data->GetIsFinished());
            outData.push_back(convertData);
        }
    }

    outputData_.clear();
    return outData;
}

template <class T>
int32_t OutputPort<T>::resetResampleCfg()
{
    CHECK_AND_RETURN_RET_LOG(!convert_.empty(), ERROR, "convert_ is empty.");
    for (auto &tmpConvert : convert_) {
        tmpConvert->Reset();
    }
    return SUCCESS;
}

}
}
}
#endif