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
#include "hpae_plugin_node.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaePluginNode::HpaePluginNode(HpaeNodeInfo& nodeInfo)
    : HpaeNode(nodeInfo),  pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate),
    outputStream_(this), enableProcess_(true), silenceData_(pcmBufferInfo_)
      
{
    silenceData_.Reset();
    silenceData_.SetBufferValid(false);
    silenceData_.SetBufferSilence(true);
}

void HpaePluginNode::DoProcess()
{
    HpaePcmBuffer *tempOut = nullptr;
    std::vector<HpaePcmBuffer *>& preOutputs = inputStream_.ReadPreOutputData();
    // if buffer is not valid, write silence data(invalid) to output
    if (enableProcess_ && !preOutputs.empty()) {
        tempOut = SignalProcess(preOutputs);
        outputStream_.WriteDataToOutput(tempOut);
    } else if (!preOutputs.empty()) {
        outputStream_.WriteDataToOutput(preOutputs[0]);
    } else if (!enableProcess_ && preOutputs.empty()) {
        // use to drain data when disconnecting, now use for mixerNode of processCluster
        tempOut = SignalProcess(preOutputs);
        outputStream_.WriteDataToOutput(tempOut);
    } else {
        outputStream_.WriteDataToOutput(&silenceData_);
    }
}

bool HpaePluginNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaePluginNode::ResetAll()
{
    const auto preOutputMap = inputStream_.GetPreOutputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        std::shared_ptr<HpaeNode> hpaeNode = preOutput.second;
        if (hpaeNode->ResetAll()) {
            inputStream_.DisConnect(output);
        }
    }
    return true;
}

int32_t HpaePluginNode::EnableProcess(bool enable)
{
    enableProcess_ = enable;
    return SUCCESS;
}

bool HpaePluginNode::IsEnableProcess()
{
    return enableProcess_;
}

std::shared_ptr<HpaeNode> HpaePluginNode::GetSharedInstance()
{
    return shared_from_this();
}

OutputPort<HpaePcmBuffer*>* HpaePluginNode::GetOutputPort()
{
    return &outputStream_;
}

void HpaePluginNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
}

void HpaePluginNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    inputStream_.DisConnect(preNode->GetOutputPort());
}

size_t HpaePluginNode::GetPreOutNum()
{
    return inputStream_.GetPreOutputNum();
}

size_t HpaePluginNode::GetOutputPortNum()
{
    return outputStream_.GetInputNum();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS