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
#define LOG_TAG "HpaeSourceOutputNode"
#endif

#include <hpae_source_output_node.h>
#include "audio_engine_log.h"
#include "hpae_format_convert.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeSourceOutputNode::HpaeSourceOutputNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo),
      sourceOuputData_(nodeInfo.frameLen * nodeInfo.channels * GET_SIZE_FROM_FORMAT(nodeInfo.format)),
      interleveData_(nodeInfo.frameLen * nodeInfo.channels)
{
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeSourceOutputNode_id_" + std::to_string(GetSessionId()) + "_ch_" + std::to_string(GetChannelCount()) +
        "_rate_" + std::to_string(GetSampleRate()) + "_bit_" + std::to_string(GetBitWidth()) + ".pcm");
#endif
}

void HpaeSourceOutputNode::DoProcess()
{
    auto rate = "rate[" + std::to_string(GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(GetFrameLen()) + "]_";
    auto format = "bit[" + std::to_string(GetBitWidth()) + "]";
    Trace trace("[" + std::to_string(GetSessionId()) + "]HpaeSourceOutputNode::DoProcess " +
       rate + ch + len + format);
    if (readCallback_.lock() == nullptr) {
        AUDIO_WARNING_LOG("HpaeSourceOutputNode readCallback_ is nullptr");
        return;
    }
    std::vector<HpaePcmBuffer *> &outputVec = inputStream_.ReadPreOutputData();
    if (outputVec.empty()) {
        return;
    }
    HpaePcmBuffer *outputData = outputVec.front();
    ConvertFromFloat(
        GetBitWidth(), GetChannelCount() * GetFrameLen(), outputData->GetPcmDataBuffer(), sourceOuputData_.data());
#ifdef ENABLE_HOOK_PCM
    if (outputPcmDumper_) {
        outputPcmDumper_->Dump(
            (int8_t *)sourceOuputData_.data(), GetChannelCount() * GetFrameLen() * GET_SIZE_FROM_FORMAT(GetBitWidth()));
    }
#endif
    int32_t ret = readCallback_.lock()->OnReadData(sourceOuputData_, sourceOuputData_.size());
    if (ret != 0) {
        AUDIO_WARNING_LOG("sessionId %{public}u, readCallback_ write read data error", GetSessionId());
    }
    return;
}

bool HpaeSourceOutputNode::Reset()
{
    const auto preOutputMap = inputStream_.GetPreOuputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        inputStream_.DisConnect(output);
    }
    return true;
}

bool HpaeSourceOutputNode::ResetAll()
{
    const auto preOutputMap = inputStream_.GetPreOuputMap();
    for (const auto &preOutput : preOutputMap) {
        OutputPort<HpaePcmBuffer *> *output = preOutput.first;
        std::shared_ptr<HpaeNode> hpaeNode = preOutput.second;
        if (hpaeNode->ResetAll()) {
            inputStream_.DisConnect(output);
        }
    }
    return true;
}

bool HpaeSourceOutputNode::RegisterReadCallback(const std::weak_ptr<IReadCallback> &callback)
{
    if (callback.lock() == nullptr) {
        return false;
    }
    readCallback_ = callback;
    return true;
}

void HpaeSourceOutputNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort());
}

void HpaeSourceOutputNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    std::shared_ptr<HpaeNode> realPreNode = preNode->GetSharedInstance(nodeInfo);
    inputStream_.Connect(realPreNode, preNode->GetOutputPort(nodeInfo));
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeInfo().statusCallback.lock()) {
        callback->OnNotifyDfxNodeInfo(
            true, realPreNode->GetNodeId(), GetNodeInfo());
    }
#endif
}

void HpaeSourceOutputNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    inputStream_.DisConnect(preNode->GetOutputPort());
}

void HpaeSourceOutputNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
}


}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
