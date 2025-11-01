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
#define LOG_TAG "HpaeSourceProcessCluster"
#endif

#include "hpae_source_process_cluster.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeSourceProcessCluster::HpaeSourceProcessCluster(HpaeNodeInfo& nodeInfo) // nodeInfo maybe sourceinputnode info
    : HpaeNode(nodeInfo), captureEffectNode_(std::make_shared<HpaeCaptureEffectNode>(nodeInfo)),
      mixerNode_(std::make_shared<HpaeMixerNode>(nodeInfo))
{
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeSourceProcessCluster");
#endif
    AUDIO_INFO_LOG("sceneType = %{public}u", nodeInfo.sceneType);
}

HpaeSourceProcessCluster::~HpaeSourceProcessCluster()
{
    Reset();
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeSourceProcessCluster::DoProcess()
{
}

bool HpaeSourceProcessCluster::Reset()
{
    if (captureEffectNode_ != nullptr) {
        captureEffectNode_->Reset();
        mixerNode_->DisConnectWithInfo(captureEffectNode_, GetNodeInfo()); // useless nodeinfo
    }
    mixerNode_->Reset();
    for (auto fmtConverterNode : fmtConverterNodeMap_) {
        fmtConverterNode.second->Reset();
        fmtConverterNode.second->DisConnect(mixerNode_);
    }
    for (auto fmtConverterNode : injectorFmtConverterNodeMap_) {
        fmtConverterNode.second->Reset();
        mixerNode_->DisConnect(fmtConverterNode.second);
    }
    return true;
}

bool HpaeSourceProcessCluster::ResetAll()
{
    return captureEffectNode_ != nullptr ? captureEffectNode_->ResetAll() : mixerNode_->ResetAll();
}

std::shared_ptr<HpaeNode> HpaeSourceProcessCluster::GetSharedInstance()
{
    return mixerNode_;
}

OutputPort<HpaePcmBuffer *> *HpaeSourceProcessCluster::GetOutputPort()
{
    return mixerNode_->GetOutputPort();
}

std::shared_ptr<HpaeNode> HpaeSourceProcessCluster::GetSharedInstance(HpaeNodeInfo &nodeInfo)
{
    std::string sourceOutputNodeKey = TransNodeInfoToStringKey(nodeInfo);
    HpaeNodeInfo effectNodeInfo = mixerNode_->GetNodeInfo();
    std::string effectNodeKey = TransNodeInfoToStringKey(effectNodeInfo);
    AUDIO_INFO_LOG("sourceOutput:[%{public}s] mixerNode:[%{public}s]",
        sourceOutputNodeKey.c_str(), effectNodeKey.c_str());
    if (CheckHpaeNodeInfoIsSame(nodeInfo, effectNodeInfo)) {
        AUDIO_INFO_LOG("Config of sourceOutputNode is same with capture mixerNode");
        return mixerNode_;
    }
    if (!SafeGetMap(fmtConverterNodeMap_, sourceOutputNodeKey)) {
        fmtConverterNodeMap_[sourceOutputNodeKey] =
            std::make_shared<HpaeAudioFormatConverterNode>(effectNodeInfo, nodeInfo);
        fmtConverterNodeMap_[sourceOutputNodeKey]->SetSourceNode(true);
    }
    fmtConverterNodeMap_[sourceOutputNodeKey]->Connect(mixerNode_);
    return fmtConverterNodeMap_[sourceOutputNodeKey];
}

OutputPort<HpaePcmBuffer *> *HpaeSourceProcessCluster::GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect)
{
    std::string sourceOutputNodeKey = TransNodeInfoToStringKey(nodeInfo);
    HpaeNodeInfo effectNodeInfo = mixerNode_->GetNodeInfo();
    std::string effectNodeKey = TransNodeInfoToStringKey(effectNodeInfo);
    AUDIO_INFO_LOG("sourceOutput:[%{public}s] mixerNode:[%{public}s]",
        sourceOutputNodeKey.c_str(), effectNodeKey.c_str());
    if (CheckHpaeNodeInfoIsSame(nodeInfo, effectNodeInfo)) {
        AUDIO_INFO_LOG("Config of sourceOutputNode is same with capture mixerNode");
        return mixerNode_->GetOutputPort();
    }
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(fmtConverterNodeMap_, sourceOutputNodeKey),
        mixerNode_->GetOutputPort(),
        "not find the sourceOutputNodeKey = %{public}s", sourceOutputNodeKey.c_str());
    if (isDisConnect && fmtConverterNodeMap_[sourceOutputNodeKey]->GetOutputPortNum() <= 1) {
        // disconnect fmtConverterNode->upEffectNode
        AUDIO_INFO_LOG("disconnect fmtConverterNode between mixerNode[[%{public}s] and sourceoutputnode[%{public}s]",
            effectNodeKey.c_str(), sourceOutputNodeKey.c_str());
        fmtConverterNodeMap_[sourceOutputNodeKey]->DisConnect(mixerNode_);
    }
    return fmtConverterNodeMap_[sourceOutputNodeKey]->GetOutputPort();
}

void HpaeSourceProcessCluster::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo effectNodeInfo;
    GetCapturerEffectConfig(effectNodeInfo);
    if (captureEffectNode_ != nullptr) {
        captureEffectNode_->ConnectWithInfo(preNode, effectNodeInfo);
    } else {
        mixerNode_->ConnectWithInfo(preNode, effectNodeInfo);
    }
}

void HpaeSourceProcessCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo effectNodeInfo;
    GetCapturerEffectConfig(effectNodeInfo);
    if (captureEffectNode_ != nullptr) {
        captureEffectNode_->DisConnectWithInfo(preNode, effectNodeInfo);
    } else {
        mixerNode_->DisConnectWithInfo(preNode, effectNodeInfo);
    }
}

void HpaeSourceProcessCluster::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode,
    HpaeNodeInfo &nodeInfo)
{
    if (captureEffectNode_) {
        captureEffectNode_->ConnectWithInfo(preNode, nodeInfo);
    } else {
        mixerNode_->ConnectWithInfo(preNode, nodeInfo);
    }
}

void HpaeSourceProcessCluster::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode,
    HpaeNodeInfo &nodeInfo)
{
    if (captureEffectNode_) {
        captureEffectNode_->DisConnectWithInfo(preNode, nodeInfo);
    } else {
        mixerNode_->DisConnectWithInfo(preNode, nodeInfo);
    }
}

void HpaeSourceProcessCluster::ConnectInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    AUDIO_INFO_LOG("connect injector sinkOutputNode in processcluster");
    CHECK_AND_RETURN_LOG(preNode != nullptr, "pre sinkOutputNode is nullptr");
    HpaeNodeInfo sinkNodeInfo = preNode->GetNodeInfo();
    HpaeNodeInfo mixerNodeInfo = mixerNode_->GetNodeInfo();
    if (CheckHpaeNodeInfoIsSame(sinkNodeInfo, mixerNodeInfo)) {
        AUDIO_INFO_LOG("Specification of sinkOutputNode is same with mixerNode");
        mixerNode_->Connect(preNode);
    } else {
        injectorFmtConverterNodeMap_[preNode] =
            std::make_shared<HpaeAudioFormatConverterNode>(sinkNodeInfo, mixerNodeInfo);
        mixerNode_->Connect(injectorFmtConverterNodeMap_[preNode]);
        injectorFmtConverterNodeMap_[preNode]->Connect(preNode);
    }
}

void HpaeSourceProcessCluster::DisConnectInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode)
{
    AUDIO_INFO_LOG("disconnect injector sinkOutputNode in processcluster");
    CHECK_AND_RETURN_LOG(preNode != nullptr, "pre sinkOutputNode is nullptr");
    HpaeNodeInfo sinkNodeInfo = preNode->GetNodeInfo();
    HpaeNodeInfo mixerNodeInfo = mixerNode_->GetNodeInfo();
    if (CheckHpaeNodeInfoIsSame(sinkNodeInfo, mixerNodeInfo)) {
        AUDIO_INFO_LOG("Specification of sinkOutputNode is same with mixerNode");
        mixerNode_->DisConnect(preNode);
    } else if (auto injectorConvert = SafeGetMap(injectorFmtConverterNodeMap_, preNode)) {
        injectorConvert->DisConnect(preNode);
        mixerNode_->DisConnect(injectorConvert);
        injectorFmtConverterNodeMap_.erase(preNode);
    }
}

bool HpaeSourceProcessCluster::GetCapturerEffectConfig(HpaeNodeInfo &nodeInfo, HpaeSourceBufferType type)
{
    if (captureEffectNode_ == nullptr) {
        nodeInfo = mixerNode_->GetNodeInfo();
        return true;
    }
    return captureEffectNode_->GetCapturerEffectConfig(nodeInfo, type);
}

size_t HpaeSourceProcessCluster::GetOutputPortNum()
{
    return mixerNode_->GetOutputPortNum();
}

int32_t HpaeSourceProcessCluster::CaptureEffectCreate(uint64_t sceneKeyCode, CaptureEffectAttr attr)
{
    CHECK_AND_RETURN_RET_LOG(captureEffectNode_, ERROR_ILLEGAL_STATE, "captureEffectNode_ is nullptr");
    HpaeNodeInfo nodeInfo;
    if (captureEffectNode_->CaptureEffectCreate(sceneKeyCode, attr) != 0 || !GetCapturerEffectConfig(nodeInfo)) {
        captureEffectNode_ = nullptr;
        return ERROR_ILLEGAL_STATE;
    }
    // create captureEffectNode, updata mixerNode info by effectnode info
    mixerNode_ = std::make_shared<HpaeMixerNode>(nodeInfo);
    mixerNode_->ConnectWithInfo(captureEffectNode_, nodeInfo);
    return 0;
}

int32_t HpaeSourceProcessCluster::CaptureEffectRelease(uint64_t sceneKeyCode)
{
    CHECK_AND_RETURN_RET_LOG(captureEffectNode_, ERROR_ILLEGAL_STATE, "captureEffectNode_ is nullptr");
    mixerNode_->DisConnectWithInfo(captureEffectNode_, GetNodeInfo());
    return captureEffectNode_->CaptureEffectRelease(sceneKeyCode);
}

bool HpaeSourceProcessCluster::IsEffectNodeValid()
{
    return captureEffectNode_ != nullptr;
}

// for ut test
uint32_t HpaeSourceProcessCluster::GetMixerNodeUseCount()
{
    return mixerNode_.use_count();
}

uint32_t HpaeSourceProcessCluster::GetCapturerEffectNodeUseCount()
{
    return captureEffectNode_.use_count();
}

uint32_t HpaeSourceProcessCluster::GetConverterNodeCount()
{
    return fmtConverterNodeMap_.size();
}

size_t HpaeSourceProcessCluster::GetPreOutNum()
{
    CHECK_AND_RETURN_RET_LOG(captureEffectNode_, 0, "captureEffectNode_ is nullptr");
    return captureEffectNode_->GetPreOutNum();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS