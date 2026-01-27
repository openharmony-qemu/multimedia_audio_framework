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
#define LOG_TAG "AudioSuiteChannel"
#endif

#include "audio_suite_channel.h"
#include "audio_suite_log.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

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
std::vector<T>& InputPort<T>::ReadPreOutputData(PcmBufferFormat outFormat, bool needConvert, uint32_t needDataLength)
{
    inputData_.clear();
    std::vector<T> outputData;
    for (auto &o : outputPorts_) {
        if (o.first == nullptr) {
            continue;
        }

        outputData = o.first->PullOutputData(outFormat, needConvert, needDataLength);
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
std::vector<T> OutputPort<T>::PullOutputData(PcmBufferFormat outFormat, bool needConvert, uint32_t needDataLengthMs)
{
    CHECK_AND_RETURN_RET_LOG(audioNode_ != nullptr, std::vector<T>(), "audionode is nullptr.");
    audioNode_->DoProcess(needDataLengthMs);

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
            AudioSuitePcmBuffer *convertData = convert_[idx]->Process(data, outFormat, needDataLengthMs);
            CHECK_AND_RETURN_RET_LOG(convertData != nullptr, std::vector<T>(), "convertData is nullptr.");
            convertData->SetIsFinished(data->GetIsFinished());
            outData.push_back(convertData);
        }
    }

    outputData_.clear();
    return outData;
}

template <class T>
int32_t OutputPort<T>::ResetResampleCfg()
{
    CHECK_AND_RETURN_RET_LOG(!convert_.empty(), ERROR, "convert_ is empty.");
    for (auto &tmpConvert : convert_) {
        CHECK_AND_RETURN_RET_LOG(tmpConvert != nullptr, ERROR, "tmpConvert is nullptr.");
        tmpConvert->Reset();
    }
    return SUCCESS;
}

template <class T>
int32_t OutputPort<T>::SetOutputPort(std::shared_ptr<AudioNode> node)
{
    CHECK_AND_RETURN_RET_LOG(audioNode_ == nullptr, SUCCESS, "audio node port already inited");
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERROR, "node is nullptr, SetOutputPort failed");
    uint32_t sourceSepara = 2;
    convert_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
    tmpData_.resize(1);
    if (node && node->GetNodeType() == NODE_TYPE_AUDIO_SEPARATION) {
        convert_.emplace_back(std::make_unique<AudioSuiteFormatConversion>());
        tmpData_.resize(sourceSepara);
    }
    audioNode_ = node;
    AUDIO_INFO_LOG("SetOutputPort SUCCESS");
    return SUCCESS;
}

template class InputPort<AudioSuitePcmBuffer*>;
template class OutputPort<AudioSuitePcmBuffer*>;

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS