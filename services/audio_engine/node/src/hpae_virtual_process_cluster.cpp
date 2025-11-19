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
#define LOG_TAG "HpaeVirtualProcessCluster"
#endif

#include "hpae_virtual_process_cluster.h"

#include <cinttypes>
#include "audio_engine_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "hpae_node_common.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeVirtualProcessCluster::HpaeVirtualProcessCluster(HpaeNodeInfo nodeInfo)
    : HpaeNode(nodeInfo), mixerNode_(std::make_shared<HpaeMixerNode>(nodeInfo))
{
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeVirtualProcessCluster");
#endif
}

HpaeVirtualProcessCluster::~HpaeVirtualProcessCluster()
{
    AUDIO_INFO_LOG("Virtual process cluster destroyed, processor scene type is %{public}d", GetSceneType());
    Reset();
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeVirtualProcessCluster::DoProcess()
{
    mixerNode_->DoProcess();
}

bool HpaeVirtualProcessCluster::Reset()
{
    mixerNode_->Reset();
    return true;
}

bool HpaeVirtualProcessCluster::ResetAll()
{
    return mixerNode_->ResetAll();
}

std::shared_ptr<HpaeNode> HpaeVirtualProcessCluster::GetSharedInstance()
{
    return mixerNode_;
}

OutputPort<HpaePcmBuffer *> *HpaeVirtualProcessCluster::GetOutputPort()
{
    return mixerNode_->GetOutputPort();
}

void HpaeVirtualProcessCluster::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo sinkInputNodeInfo = preNode->GetNodeInfo();
    uint32_t sessionId = sinkInputNodeInfo.sessionId;
    CreateGainNode(sessionId, sinkInputNodeInfo);
    CreateConverterNode(sessionId, sinkInputNodeInfo);

    mixerNode_->Connect(idGainMap_[sessionId]);
    idGainMap_[sessionId]->Connect(idConverterMap_[sessionId]);
    idConverterMap_[sessionId]->Connect(preNode);

    mixerNode_->EnableProcess(true);
}

void HpaeVirtualProcessCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo sinkInputNodeInfo = preNode->GetNodeInfo();
    uint32_t sessionId = sinkInputNodeInfo.sessionId;
    if (SafeGetMap(idConverterMap_, sessionId)) {
        idConverterMap_[sessionId]->DisConnect(preNode);
        idGainMap_[sessionId]->DisConnect(idConverterMap_[sessionId]);
        mixerNode_->DisConnect(idGainMap_[sessionId]);

        idConverterMap_.erase(sessionId);
        idGainMap_.erase(sessionId);
        AUDIO_INFO_LOG("Process DisConnect Exist converterNode preOutNum is %{public}zu", mixerNode_->GetPreOutNum());
    }
    if (mixerNode_->GetPreOutNum() == 0) { // maybe not nessary
        mixerNode_->EnableProcess(false);
        AUDIO_DEBUG_LOG("Set mixerNode EnableProcess false");
    }
}

int32_t HpaeVirtualProcessCluster::SetupAudioLimiter()
{
    return SUCCESS;
}

void HpaeVirtualProcessCluster::CreateGainNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo)
{
    CHECK_AND_RETURN(!SafeGetMap(idGainMap_, sessionId));
    HpaeNodeInfo gainNodeInfo = preNodeInfo;
    idGainMap_[sessionId] = std::make_shared<HpaeGainNode>(gainNodeInfo);
}

void HpaeVirtualProcessCluster::CreateConverterNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo)
{
    CHECK_AND_RETURN(!SafeGetMap(idConverterMap_, sessionId));
    HpaeNodeInfo gainNodeInfo = preNodeInfo;
    idConverterMap_[sessionId] = std::make_shared<HpaeAudioFormatConverterNode>(gainNodeInfo, GetNodeInfo());
}

size_t HpaeVirtualProcessCluster::GetConnectSinkInputNum()
{
    return idConverterMap_.size(); // todo: not be converter num, should be mixernode input stream num
}

std::shared_ptr<HpaeGainNode> HpaeVirtualProcessCluster::GetGainNodeById(const uint32_t &sessionId)
{
    return SafeGetMap(idGainMap_, sessionId);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS