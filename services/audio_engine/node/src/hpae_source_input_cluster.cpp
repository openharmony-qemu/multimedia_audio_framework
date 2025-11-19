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
#define LOG_TAG "HpaeSourceInputCluster"
#endif

#include "hpae_source_input_cluster.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeSourceInputCluster::HpaeSourceInputCluster(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), sourceInputNode_(std::make_shared<HpaeSourceInputNode>(nodeInfo))
{
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeSourceInputCluster");
#endif
}

HpaeSourceInputCluster::HpaeSourceInputCluster(std::vector<HpaeNodeInfo> &nodeInfos)
{
    CHECK_AND_RETURN_LOG(!nodeInfos.empty(), "nodeInfos vector is empty!");
    auto nodeInfo = *nodeInfos.begin();
    SetNodeInfo(nodeInfo);
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeSourceInputCluster");
#endif
    sourceInputNode_ = std::make_shared<HpaeSourceInputNode>(nodeInfos);
}

HpaeSourceInputCluster::~HpaeSourceInputCluster()
{
    Reset();
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeSourceInputCluster::DoProcess()
{
}

bool HpaeSourceInputCluster::Reset()
{
    for (auto fmtConverterNode : fmtConverterNodeMap_) {
        fmtConverterNode.second->DisConnectWithInfo(sourceInputNode_, fmtConverterNode.second->GetNodeInfo());
        fmtConverterNode.second->Reset();
    }
    sourceInputNode_->Reset();
    return true;
}

bool HpaeSourceInputCluster::ResetAll()
{
    for (auto fmtConverterNode : fmtConverterNodeMap_) {
        fmtConverterNode.second->ResetAll();
    }
    sourceInputNode_->ResetAll();
    return true;
}

std::shared_ptr<HpaeNode> HpaeSourceInputCluster::GetSharedInstance()
{
    return sourceInputNode_;
}

std::shared_ptr<HpaeNode> HpaeSourceInputCluster::GetSharedInstance(HpaeNodeInfo &nodeInfo)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, nullptr, "sourceInputNode_ is nullptr");
    std::string preNodeKey = TransNodeInfoToStringKey(nodeInfo);
    std::string inputNodeKey = TransNodeInfoToStringKey(GetNodeInfoWithInfo(nodeInfo.sourceBufferType));
    AUDIO_INFO_LOG("sourceInput:[%{public}s] preNode:[%{public}s]",
        inputNodeKey.c_str(), preNodeKey.c_str());
    if (CheckHpaeNodeInfoIsSame(nodeInfo, GetNodeInfoWithInfo(nodeInfo.sourceBufferType))) {
        AUDIO_INFO_LOG("Specification of sourceInputNode is same with preNode");
        return sourceInputNode_;
    }
    if (!SafeGetMap(fmtConverterNodeMap_, preNodeKey)) {
        fmtConverterNodeMap_[preNodeKey] =
            std::make_shared<HpaeAudioFormatConverterNode>(GetNodeInfoWithInfo(nodeInfo.sourceBufferType), nodeInfo);
        fmtConverterNodeMap_[preNodeKey]->SetSourceNode(true);
    }
    fmtConverterNodeMap_[preNodeKey]->ConnectWithInfo(
        sourceInputNode_, fmtConverterNodeMap_[preNodeKey]->GetNodeInfo());
    return fmtConverterNodeMap_[preNodeKey];
}

OutputPort<HpaePcmBuffer *> *HpaeSourceInputCluster::GetOutputPort()
{
    return sourceInputNode_->GetOutputPort();
}

OutputPort<HpaePcmBuffer *> *HpaeSourceInputCluster::GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, nullptr, "sourceInputNode_ is nullptr");
    std::string preNodeKey = TransNodeInfoToStringKey(nodeInfo);
    std::string inputNodeKey = TransNodeInfoToStringKey(GetNodeInfoWithInfo(nodeInfo.sourceBufferType));
    AUDIO_INFO_LOG("sourceinput:[%{public}s] preNodeKey:[%{public}s]",
        inputNodeKey.c_str(), preNodeKey.c_str());
    if (CheckHpaeNodeInfoIsSame(nodeInfo, GetNodeInfoWithInfo(nodeInfo.sourceBufferType))) {
        AUDIO_INFO_LOG("sourceInputNode is same as preNode");
        return sourceInputNode_->GetOutputPort(nodeInfo);
    }
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(fmtConverterNodeMap_, preNodeKey),
        sourceInputNode_->GetOutputPort(nodeInfo),
        "not find the preNodeKey = %{public}s", preNodeKey.c_str());
    if (isDisConnect && fmtConverterNodeMap_[preNodeKey]->GetOutputPortNum() <= 1) {
        AUDIO_INFO_LOG("disconnect fmtConverterNode between preNode[%{public}s] and sourceInputNode[%{public}s]",
            preNodeKey.c_str(), inputNodeKey.c_str());
        fmtConverterNodeMap_[preNodeKey]->DisConnectWithInfo(
            sourceInputNode_, fmtConverterNodeMap_[preNodeKey]->GetNodeInfo());
    }
    return fmtConverterNodeMap_[preNodeKey]->GetOutputPort();
}

int32_t HpaeSourceInputCluster::GetCapturerSourceInstance(const std::string &deviceClass,
    const std::string &deviceNetId, const SourceType &sourceType, const std::string &sourceName)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->GetCapturerSourceInstance(deviceClass, deviceNetId, sourceType, sourceName);
}

int32_t HpaeSourceInputCluster::CapturerSourceInit(IAudioSourceAttr &attr)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->CapturerSourceInit(attr);
}

int32_t HpaeSourceInputCluster::CapturerSourceDeInit()
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->CapturerSourceDeInit();
}

int32_t HpaeSourceInputCluster::CapturerSourceFlush(void)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->CapturerSourceFlush();
}

int32_t HpaeSourceInputCluster::CapturerSourcePause(void)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->CapturerSourcePause();
}

int32_t HpaeSourceInputCluster::CapturerSourceReset(void)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->CapturerSourceReset();
}

int32_t HpaeSourceInputCluster::CapturerSourceResume(void)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->CapturerSourceResume();
}

int32_t HpaeSourceInputCluster::CapturerSourceStart(void)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->CapturerSourceStart();
}

int32_t HpaeSourceInputCluster::CapturerSourceStop(void)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    return sourceInputNode_->CapturerSourceStop();
}

StreamManagerState HpaeSourceInputCluster::GetSourceState(void)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, STREAM_MANAGER_INVALID, "sourceInputNode_ is nullptr");
    return sourceInputNode_->GetSourceState();
}

size_t HpaeSourceInputCluster::GetOutputPortNum()
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, 0, "sourceInputNode_ is nullptr");
    return sourceInputNode_->GetOutputPortNum();
}

size_t HpaeSourceInputCluster::GetOutputPortNum(HpaeNodeInfo &nodeInfo)
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, 0, "sourceInputNode_ is nullptr");
    return sourceInputNode_->GetOutputPortNum(nodeInfo);
}

HpaeSourceInputNodeType HpaeSourceInputCluster::GetSourceInputNodeType()
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_, HPAE_SOURCE_DEFAULT, "sourceInputNode_ is nullptr");
    return sourceInputNode_->GetSourceInputNodeType();
}

void HpaeSourceInputCluster::SetSourceInputNodeType(HpaeSourceInputNodeType type)
{
    CHECK_AND_RETURN_LOG(sourceInputNode_, "sourceInputNode_ is nullptr");
    sourceInputNode_->SetSourceInputNodeType(type);
}
 
HpaeNodeInfo &HpaeSourceInputCluster::GetNodeInfoWithInfo(HpaeSourceBufferType &type)
{
    return sourceInputNode_->GetNodeInfoWithInfo(type);
}

void HpaeSourceInputCluster::UpdateAppsUidAndSessionId(std::vector<int32_t> &appsUid, std::vector<int32_t> &sessionsId)
{
    sourceInputNode_->UpdateAppsUidAndSessionId(appsUid, sessionsId);
}

// for test
uint32_t HpaeSourceInputCluster::GetConverterNodeCount()
{
    return fmtConverterNodeMap_.size();
}

uint32_t HpaeSourceInputCluster::GetSourceInputNodeUseCount()
{
    return sourceInputNode_.use_count();
}

uint32_t HpaeSourceInputCluster::GetCaptureId()
{
    return sourceInputNode_->GetCaptureId();
}

void HpaeSourceInputCluster::SetInjectState(bool isInjecting)
{
    sourceInputNode_->SetInjectState(isInjecting);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS