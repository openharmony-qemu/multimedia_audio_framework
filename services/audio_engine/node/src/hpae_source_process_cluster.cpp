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
HpaeSourceProcessCluster::HpaeSourceProcessCluster(HpaeNodeInfo& nodeInfo)
    : HpaeNode(nodeInfo), captureEffectNode_(std::make_shared<HpaeCaptureEffectNode>(nodeInfo))
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
    captureEffectNode_->Reset();
    for (auto fmtConverterNode : fmtConverterNodeMap_) {
        fmtConverterNode.second->Reset();
    }
    return true;
}

bool HpaeSourceProcessCluster::ResetAll()
{
    return captureEffectNode_->ResetAll();
}

std::shared_ptr<HpaeNode> HpaeSourceProcessCluster::GetSharedInstance()
{
    return captureEffectNode_;
}

OutputPort<HpaePcmBuffer *> *HpaeSourceProcessCluster::GetOutputPort()
{
    return captureEffectNode_->GetOutputPort();
}

std::shared_ptr<HpaeNode> HpaeSourceProcessCluster::GetSharedInstance(HpaeNodeInfo &nodeInfo)
{
    std::string sourceOutputNodeKey = TransNodeInfoToStringKey(nodeInfo);
    HpaeNodeInfo effectNodeInfo;
    captureEffectNode_->GetCapturerEffectConfig(effectNodeInfo);
    std::string effectNodeKey = TransNodeInfoToStringKey(effectNodeInfo);
    AUDIO_INFO_LOG("sourceOutput:[%{public}s] effectNode:[%{public}s]",
        sourceOutputNodeKey.c_str(), effectNodeKey.c_str());
    if (CheckHpaeNodeInfoIsSame(nodeInfo, effectNodeInfo)) {
        AUDIO_INFO_LOG("Specification of sourceOutputNode is same with capture effect");
        return captureEffectNode_;
    }
    if (!SafeGetMap(fmtConverterNodeMap_, sourceOutputNodeKey)) {
        fmtConverterNodeMap_[sourceOutputNodeKey] =
            std::make_shared<HpaeAudioFormatConverterNode>(effectNodeInfo, nodeInfo);
        fmtConverterNodeMap_[sourceOutputNodeKey]->SetSourceNode(true);
    }
    fmtConverterNodeMap_[sourceOutputNodeKey]->Connect(captureEffectNode_);
    return fmtConverterNodeMap_[sourceOutputNodeKey];
}

OutputPort<HpaePcmBuffer *> *HpaeSourceProcessCluster::GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect)
{
    std::string sourceOutputNodeKey = TransNodeInfoToStringKey(nodeInfo);
    HpaeNodeInfo effectNodeInfo;
    captureEffectNode_->GetCapturerEffectConfig(effectNodeInfo);
    std::string effectNodeKey = TransNodeInfoToStringKey(effectNodeInfo);
    AUDIO_INFO_LOG("sourceOutput:[%{public}s] effectNode:[%{public}s]",
        sourceOutputNodeKey.c_str(), effectNodeKey.c_str());
    if (CheckHpaeNodeInfoIsSame(nodeInfo, effectNodeInfo)) {
        AUDIO_INFO_LOG("Specification of sourceOutputNode is same with capture effect");
        return captureEffectNode_->GetOutputPort();
    }
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(fmtConverterNodeMap_, sourceOutputNodeKey),
        captureEffectNode_->GetOutputPort(),
        "not find the sourceOutputNodeKey = %{public}s", sourceOutputNodeKey.c_str());
    if (isDisConnect && fmtConverterNodeMap_[sourceOutputNodeKey]->GetOutputPortNum() <= 1) {
        // disconnect fmtConverterNode->upEffectNode
        AUDIO_INFO_LOG("disconnect fmtConverterNode between effectnode[[%{public}s] and sourceoutputnode[%{public}s]",
            effectNodeKey.c_str(), sourceOutputNodeKey.c_str());
        fmtConverterNodeMap_[sourceOutputNodeKey]->DisConnect(captureEffectNode_);
    }
    return fmtConverterNodeMap_[sourceOutputNodeKey]->GetOutputPort();
}

void HpaeSourceProcessCluster::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo effectNodeInfo;
    captureEffectNode_->GetCapturerEffectConfig(effectNodeInfo);
    captureEffectNode_->ConnectWithInfo(preNode, effectNodeInfo);
}

void HpaeSourceProcessCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo effectNodeInfo;
    captureEffectNode_->GetCapturerEffectConfig(effectNodeInfo);
    captureEffectNode_->DisConnectWithInfo(preNode, effectNodeInfo);
}

void HpaeSourceProcessCluster::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode,
    HpaeNodeInfo &nodeInfo)
{
    captureEffectNode_->ConnectWithInfo(preNode, nodeInfo);
}

void HpaeSourceProcessCluster::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode,
    HpaeNodeInfo &nodeInfo)
{
    captureEffectNode_->DisConnectWithInfo(preNode, nodeInfo);
}

bool HpaeSourceProcessCluster::GetCapturerEffectConfig(HpaeNodeInfo &nodeInfo, HpaeSourceBufferType type)
{
    return captureEffectNode_->GetCapturerEffectConfig(nodeInfo, type);
}

size_t HpaeSourceProcessCluster::GetOutputPortNum()
{
    return captureEffectNode_->GetOutputPortNum();
}

int32_t HpaeSourceProcessCluster::CaptureEffectCreate(uint64_t sceneKeyCode, CaptureEffectAttr attr)
{
    CHECK_AND_RETURN_RET_LOG(captureEffectNode_, ERROR_ILLEGAL_STATE, "captureEffectNode_ is nullptr");
    return captureEffectNode_->CaptureEffectCreate(sceneKeyCode, attr);
}

int32_t HpaeSourceProcessCluster::CaptureEffectRelease(uint64_t sceneKeyCode)
{
    CHECK_AND_RETURN_RET_LOG(captureEffectNode_, ERROR_ILLEGAL_STATE, "captureEffectNode_ is nullptr");
    return captureEffectNode_->CaptureEffectRelease(sceneKeyCode);
}

// for ut test
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