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

#ifndef HPAE_NODE_H
#define HPAE_NODE_H
#include <memory>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <set>
#include <sstream>
#include "hpae_pcm_buffer.h"
#include "hpae_define.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint32_t MIN_START_NODE_ID = 100;

class HpaeNode : public std::enable_shared_from_this<HpaeNode> {
public:
    HpaeNode()
    {
#ifdef ENABLE_HIDUMP_DFX
        nodeInfo_.nodeId = GenerateHpaeNodeId();
#endif
    }

    virtual ~HpaeNode() {};

    HpaeNode(HpaeNodeInfo& nodeInfo) : nodeInfo_(nodeInfo)
    {
#ifdef ENABLE_HIDUMP_DFX
        nodeInfo_.nodeId = GenerateHpaeNodeId();
#endif
    }

    virtual void DoProcess() = 0;
    virtual void Enqueue(HpaePcmBuffer* buffer) {};
    // for process node
    virtual bool Reset() = 0;
    virtual bool ResetAll() = 0;

    virtual HpaeNodeInfo& GetNodeInfo()
    {
        return nodeInfo_;
    }

    virtual void SetNodeInfo(HpaeNodeInfo& nodeInfo)
    {
        nodeInfo_ = nodeInfo;
    }

    virtual void SetNodeId(uint32_t nodeId)
    {
        nodeInfo_.nodeId = nodeId;
    }

    virtual void SetNodeName(std::string nodeName)
    {
        nodeInfo_.nodeName = nodeName;
    }

    virtual AudioSamplingRate GetSampleRate()
    {
        return nodeInfo_.samplingRate;
    }

    virtual AudioSampleFormat GetBitWidth()
    {
        return nodeInfo_.format;
    }
    
    virtual AudioChannel GetChannelCount()
    {
        return nodeInfo_.channels;
    }

    virtual AudioChannelLayout GetChannelLayout()
    {
        return nodeInfo_.channelLayout;
    }

    virtual size_t GetFrameLen()
    {
        return nodeInfo_.frameLen;
    }

    virtual uint32_t GetNodeId()
    {
        return nodeInfo_.nodeId;
    }

    virtual uint32_t GetSessionId()
    {
        return nodeInfo_.sessionId;
    }

    virtual AudioStreamType GetStreamType()
    {
        return nodeInfo_.streamType;
    }

    virtual HpaeProcessorType GetSceneType()
    {
        return nodeInfo_.sceneType;
    }

    virtual std::string GetDeviceClass()
    {
        return nodeInfo_.deviceClass;
    }

    virtual std::string GetDeviceNetId()
    {
        return nodeInfo_.deviceNetId;
    }

    virtual std::string GetNodeName()
    {
        return nodeInfo_.nodeName;
    }
    
    virtual std::weak_ptr<INodeCallback> GetNodeStatusCallback()
    {
        return nodeInfo_.statusCallback;
    }

    virtual std::string GetTraceInfo()
    {
        std::ostringstream oss;
        oss << "rate[" << nodeInfo_.samplingRate << "]_"
            << "ch[" << static_cast<int32_t>(nodeInfo_.channels) << "]_"
            << "len[" << nodeInfo_.frameLen << "]_"
            << "bit[" << static_cast<int32_t>(nodeInfo_.format) << "]";
        return oss.str();
    }
private:
    static uint32_t GenerateHpaeNodeId()
    {
        std::lock_guard<std::mutex> lock(nodeIdCounterMutex_);
        if (nodeIdCounter_ == std::numeric_limits<uint32_t>::max()) {
            nodeIdCounter_ = MIN_START_NODE_ID;
        } else {
            ++nodeIdCounter_;
        }
        return nodeIdCounter_;
    }

private:
    HpaeNodeInfo nodeInfo_;
    inline static std::mutex nodeIdCounterMutex_;
    inline static uint32_t nodeIdCounter_ = MIN_START_NODE_ID;
};

template <typename T>
class InputPort;

template <typename T>
class OutputPort {
public:
    explicit OutputPort(HpaeNode *node) : hpaeNode_(node)
    {}
    void WriteDataToOutput(T data, HpaeBufferType bufferType = HPAE_BUFFER_TYPE_DEFAULT);
    OutputPort(const OutputPort &that) = delete;
    T PullOutputData();
    void AddInput(InputPort<T> *input);
    void AddInput(InputPort<T> *input, const std::shared_ptr<HpaeNode> &node);
    bool RemoveInput(InputPort<T> *input, HpaeBufferType bufferType = HPAE_BUFFER_TYPE_DEFAULT);
    size_t GetInputNum() const;
private:
    std::set<InputPort<T>*> inputPortSet_;
    std::vector<T> outputData_;
    HpaeNode *hpaeNode_;
    std::unordered_map<InputPort<T>*, std::shared_ptr<HpaeNode>> coInputPorts_;
};

template <typename OutputType>
class OutputNode : virtual public HpaeNode {
public:
    virtual ~OutputNode()
    {}
    virtual std::shared_ptr<HpaeNode> GetSharedInstance() = 0;
    virtual std::shared_ptr<HpaeNode> GetSharedInstance(HpaeNodeInfo &nodeInfo) { return nullptr; }
    virtual OutputPort<OutputType>* GetOutputPort() = 0;
    virtual OutputPort<OutputType>* GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect = false) { return nullptr; }
    virtual HpaeSourceBufferType GetOutputPortBufferType(HpaeNodeInfo &nodeInfo)
        { return HPAE_SOURCE_BUFFER_TYPE_DEFAULT; }
};

template <typename InputType>
class InputNode : virtual public HpaeNode {
public:
    virtual ~InputNode()
    {}
    virtual void Connect(const std::shared_ptr<OutputNode<InputType>> &preNode) = 0;
    virtual void ConnectWithInfo(const std::shared_ptr<OutputNode<InputType>> &preNode, HpaeNodeInfo &nodeInfo) {}
    virtual void DisConnect(const std::shared_ptr<OutputNode<InputType>> &preNode) = 0;
    virtual void DisConnectWithInfo(const std::shared_ptr<OutputNode<InputType>> &preNode, HpaeNodeInfo &nodeInfo) {}
};

template <typename T>
class InputPort {
public:
    InputPort()
    {}
    ~InputPort();
    std::vector<T>& ReadPreOutputData();

    void Connect(const std::shared_ptr<HpaeNode>& node, OutputPort<T>* output,
        HpaeBufferType bufferType = HPAE_BUFFER_TYPE_DEFAULT);

    void DisConnect(OutputPort<T>* output, HpaeBufferType bufferType = HPAE_BUFFER_TYPE_DEFAULT);

    size_t GetPreOutputNum() const;

    const std::unordered_map<OutputPort<T> *, std::shared_ptr<HpaeNode>>& GetPreOutputMap();

    bool CheckIfDisConnected(OutputPort<T>* output);

    InputPort(const InputPort &that) = delete;

    void AddPreOutput(const std::shared_ptr<HpaeNode> &node, OutputPort<T>* output);
    void RemovePreOutput(OutputPort<T>* output);
private:
    std::unordered_map<OutputPort<T>*, std::shared_ptr<HpaeNode>> outputPorts_;
    std::vector<T> inputData_;
};

template <class T>
InputPort<T>::~InputPort()
{
    for (auto &o : outputPorts_) {
        o.first->RemoveInput(this);
    }
}

template <class T>
std::vector<T>& InputPort<T>::ReadPreOutputData()
{
    inputData_.clear();
    for (auto &o : outputPorts_) {
        T pcmData = o.first->PullOutputData();
        if (pcmData != nullptr) {
            inputData_.emplace_back(std::move(pcmData));
        }
    }
    return inputData_;
}

template <class T>
void InputPort<T>::Connect(const std::shared_ptr<HpaeNode> &node, OutputPort<T>* output, HpaeBufferType bufferType)
{
    // for default type
    if (bufferType == HPAE_BUFFER_TYPE_DEFAULT) {
        if (output) {
            output->AddInput(this);
        }
        AddPreOutput(node, output);
        return;
    }
    // for cobuffer type
    if (bufferType == HPAE_BUFFER_TYPE_COBUFFER) {
        if (output) {
            output->AddInput(this, node);
        }
        AddPreOutput(node, output);
        return;
    }
    return;
}

template <class T>
void InputPort<T>::DisConnect(OutputPort<T>* output, HpaeBufferType bufferType)
{
    if (output) {
        output->RemoveInput(this, bufferType);
    }
    RemovePreOutput(output);
}

template <class T>
size_t InputPort<T>::GetPreOutputNum() const
{
    return outputPorts_.size();
}

template <class T>
const std::unordered_map<OutputPort<T> *, std::shared_ptr<HpaeNode>>& InputPort<T>::GetPreOutputMap()
{
    return outputPorts_;
}

template <class T>
bool InputPort<T>::CheckIfDisConnected(OutputPort<T>* output)
{
    return outputPorts_.find(output) == outputPorts_.end();
}

template <class T>
void InputPort<T>::AddPreOutput(const std::shared_ptr<HpaeNode> &node, OutputPort<T> *output)
{
    outputPorts_[output] = node;
}

template <class T>
void InputPort<T>::RemovePreOutput(OutputPort<T> *output)
{
    outputPorts_.erase(output);
}

template <class T>
T OutputPort<T>::PullOutputData()
{
    if (outputData_.empty()) {
        hpaeNode_->DoProcess();
    }
    if (!outputData_.empty()) {
        T retValue = std::move(outputData_.back());
        outputData_.pop_back();
        return retValue;
    } else {
        return nullptr;
    }
}

template <class T>
void OutputPort<T>::WriteDataToOutput(T data, HpaeBufferType bufferType)
{
    // for default type
    if (bufferType == HPAE_BUFFER_TYPE_DEFAULT) {
        outputData_.clear();
        outputData_.emplace_back(std::move(data));

        for (size_t i = 1; i < inputPortSet_.size(); i++) {
            outputData_.push_back(outputData_[0]);
        }
        return;
    }
    // for cobuffer type
    if (bufferType == HPAE_BUFFER_TYPE_COBUFFER) {
        for (auto &i : coInputPorts_) {
            i.second->Enqueue(data);
        }
        return;
    }
    return;
}

template <class T>
void OutputPort<T>::AddInput(InputPort<T> *input)
{
    inputPortSet_.insert(input);
}
template <class T>
void OutputPort<T>::AddInput(InputPort<T> *input, const std::shared_ptr<HpaeNode> &node)
{
    coInputPorts_[input] = node;
}
template <class T>
size_t OutputPort<T>::GetInputNum() const
{
    return inputPortSet_.size();
}
template <class T>
bool OutputPort<T>::RemoveInput(InputPort<T> *input, HpaeBufferType bufferType)
{
    // for default type
    if (bufferType == HPAE_BUFFER_TYPE_DEFAULT) {
        auto it = inputPortSet_.find(input);
        if (it == inputPortSet_.end()) {
            return false;
        }
        inputPortSet_.erase(it);
        return true;
    }
    // for cobuffer type
    if (bufferType == HPAE_BUFFER_TYPE_COBUFFER) {
        auto it = coInputPorts_.find(input);
        if (it == coInputPorts_.end()) {
            return false;
        }
        coInputPorts_.erase(it);
        return true;
    }
    return true;
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif