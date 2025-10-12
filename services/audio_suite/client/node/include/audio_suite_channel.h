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

#include "audio_suite_node.h"
#include "audio_suite_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

template <typename T>
class InputPort;

template <typename T>
class OutputPort {
public:
    explicit OutputPort(std::shared_ptr<AudioNode> node) : audioNode_(node)
    {}
    void WriteDataToOutput(T data);
    OutputPort(const OutputPort &that) = delete;
    std::vector<T> PullOutputData();
    void AddInput(InputPort<T>* input);
    void RemoveInput(InputPort<T>* input);
    size_t GetInputNum() const;
    void SetPortType(AudioNodePortType type) {portType_ = type;}
    AudioNodePortType GetPortType() {return portType_;}
private:
    std::set<InputPort<T>*> inputPortSet_;
    std::vector<T> outputData_;
    std::shared_ptr<AudioNode> audioNode_;
    AudioNodePortType portType_ = AudioNodePortType::AUDIO_NODE_DEFAULT_OUTPORT_TYPE;
};

template <typename T>
class InputPort {
public:
    InputPort()
    {}
    ~InputPort();
    std::vector<T>& ReadPreOutputData();

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
std::vector<T>& InputPort<T>::ReadPreOutputData()
{
    inputData_.clear();
    for (auto &o : outputPorts_) {
        if (o.first == nullptr) {
            continue;
        }
        std::vector<T> outputData = o.first->PullOutputData();
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
std::vector<T> OutputPort<T>::PullOutputData()
{
    if (audioNode_ == nullptr) {
        AUDIO_ERR_LOG("OutputPort audioNode_ is empty");
        return std::vector<T>();
    }
    audioNode_->DoProcess();
    if (outputData_.empty()) {
        AUDIO_ERR_LOG("OutputPort outputData_ is empty");
        return std::vector<T>();
    }

    std::vector<T> retValue = outputData_;
    outputData_.clear();
    return retValue;
}

}
}
}
#endif