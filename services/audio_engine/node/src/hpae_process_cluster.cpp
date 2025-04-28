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
#define LOG_TAG "HpaeProcessCluster"
#endif

#include <cinttypes>
 
#include "audio_errors.h"
#include "hpae_process_cluster.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeProcessCluster::HpaeProcessCluster(HpaeNodeInfo nodeInfo, HpaeSinkInfo &sinkInfo)
    : HpaeNode(nodeInfo), mixerNode_(std::make_shared<HpaeMixerNode>(nodeInfo)), sinkInfo_(sinkInfo)
{
    if (TransProcessorTypeToSceneType(nodeInfo.sceneType) != "SCENE_EXTRA" && nodeInfo.deviceClass != "remote") {
        renderEffectNode_ = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    } else {
        renderEffectNode_ = nullptr;
    }
#ifdef ENABLE_HIDUMP_DFX
    if (nodeInfo.statusCallback.lock()) {
        nodeInfo.nodeName = "HpaeMixerNode";
        nodeInfo.sessionId = 0;
        nodeInfo.nodeId = nodeInfo.statusCallback.lock()->OnGetNodeId();
        AUDIO_INFO_LOG("HpaeProcessCluster, HpaeMixerNode id %{public}u ", nodeInfo.nodeId);
        mixerNode_->SetNodeInfo(nodeInfo);
        if (renderEffectNode_) {
            nodeInfo.nodeName = "HpaeRenderEffectNode";
            nodeInfo.nodeId = nodeInfo.statusCallback.lock()->OnGetNodeId();
            nodeInfo.sessionId = 0;
            renderEffectNode_->SetNodeInfo(nodeInfo);
            AUDIO_INFO_LOG("HpaeProcessCluster, HpaeRenderEffectNode id %{public}u ", nodeInfo.nodeId);
        }
    }
#endif
}

HpaeProcessCluster::~HpaeProcessCluster()
{
    AUDIO_INFO_LOG("process cluster destroyed, processor scene type is %{public}d", GetSceneType());
    Reset();
}

void HpaeProcessCluster::DoProcess()
{
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->DoProcess();
        return;
    }
    mixerNode_->DoProcess();
}

bool HpaeProcessCluster::Reset()
{
    mixerNode_->Reset();
    for (auto converterNode : idConverterMap_) {
        converterNode.second->Reset();
    }
    for (auto gainNode : idGainMap_) {
        gainNode.second->Reset();
    }
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->Reset();
        renderEffectNode_ = nullptr;
    }
    return true;
}

bool HpaeProcessCluster::ResetAll()
{
    return renderEffectNode_ != nullptr ? renderEffectNode_->ResetAll() : mixerNode_->ResetAll();
}

std::shared_ptr<HpaeNode> HpaeProcessCluster::GetSharedInstance()
{
    if (renderEffectNode_ != nullptr) {
        AUDIO_INFO_LOG("HpaeProcessCluster, GetSharedInstance renderEffectNode_ name %{public}s id: %{public}u ",
            renderEffectNode_->GetNodeName().c_str(),
            renderEffectNode_->GetNodeId());
        return renderEffectNode_;
    }
    AUDIO_INFO_LOG("HpaeProcessCluster, GetSharedInstance mixerNode_ name %{public}s  id: %{public}u  ",
        mixerNode_->GetNodeName().c_str(),
        mixerNode_->GetNodeId());
    return mixerNode_;
}

OutputPort<HpaePcmBuffer *> *HpaeProcessCluster::GetOutputPort()
{
    return renderEffectNode_ != nullptr ? renderEffectNode_->GetOutputPort() : mixerNode_->GetOutputPort();
}

int32_t HpaeProcessCluster::GetGainNodeCount()
{
    return idGainMap_.size();
}

int32_t HpaeProcessCluster::GetConverterNodeCount()
{
    return idConverterMap_.size();
}

int32_t HpaeProcessCluster::GetPreOutNum()
{
    return mixerNode_->GetPreOutNum();
}

void HpaeProcessCluster::ConnectMixerNode()
{
    if (renderEffectNode_ != nullptr && renderEffectNode_->GetPreOutNum() == 0) {
        renderEffectNode_->Connect(mixerNode_);
        AUDIO_INFO_LOG("Process Connect mixerNode_");
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = renderEffectNode_->GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfo(true, renderEffectNode_->GetNodeId(), mixerNode_->GetNodeInfo());
        }
#endif
    }
    return;
}

void HpaeProcessCluster::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo &preNodeInfo = preNode->GetNodeInfo();
    uint32_t sessionId = preNodeInfo.sessionId;
    AUDIO_INFO_LOG("HpaeProcessCluster sessionId is %{public}u, streamType is %{public}d, "
        "HpaeProcessCluster rate is %{public}u, ch is %{public}u, "
        "HpaeProcessCluster preNodeId %{public}u, preNodeName is %{public}s",
        preNodeInfo.sessionId, preNodeInfo.streamType, preNodeInfo.samplingRate, preNodeInfo.channels,
        preNodeInfo.nodeId, preNodeInfo.nodeName.c_str());
    ConnectMixerNode();
    if (!SafeGetMap(idGainMap_, sessionId)) {
        HpaeNodeInfo gainNodeInfo = preNodeInfo;
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = mixerNode_->GetNodeStatusCallback().lock()) {
            gainNodeInfo.nodeName = "HpaeGainNode";
            gainNodeInfo.nodeId = callBack->OnGetNodeId();
        }
#endif
        idGainMap_[sessionId] = std::make_shared<HpaeGainNode>(gainNodeInfo);
    }
    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->GetExpectedInputChannelInfo(channels, channelLayout);
    }
    HpaeNodeInfo effectNodeInfo = preNodeInfo;
    effectNodeInfo.frameLen = sinkInfo_.frameLen;
    effectNodeInfo.samplingRate = sinkInfo_.samplingRate;
    effectNodeInfo.format = sinkInfo_.format;
    effectNodeInfo.channels = channels == 0 ? sinkInfo_.channels : static_cast<AudioChannel>(channels);
    effectNodeInfo.channelLayout = channelLayout == 0 ? static_cast<AudioChannelLayout>(sinkInfo_.channelLayout) :
        static_cast<AudioChannelLayout>(channelLayout);
#ifdef ENABLE_HIDUMP_DFX
    effectNodeInfo.nodeName = "HpaeAudioFormatConverterNode";
    if (auto callBack = mixerNode_->GetNodeStatusCallback().lock()) {
        effectNodeInfo.nodeId = callBack->OnGetNodeId();
    }
#endif
    idConverterMap_[sessionId] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, effectNodeInfo);
    if (renderEffectNode_ != nullptr) {
        idConverterMap_[sessionId]->RegisterCallback(this);
    }
    idGainMap_[sessionId]->Connect(preNode);
    idConverterMap_[sessionId]->Connect(idGainMap_[sessionId]);
    mixerNode_->Connect(idConverterMap_[sessionId]);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = mixerNode_->GetNodeStatusCallback().lock()) {
        callBack->OnNotifyDfxNodeInfo(true, mixerNode_->GetNodeId(), idConverterMap_[sessionId]->GetNodeInfo());
        callBack->OnNotifyDfxNodeInfo(
            true, idConverterMap_[sessionId]->GetNodeId(), idGainMap_[sessionId]->GetNodeInfo());
        callBack->OnNotifyDfxNodeInfo(true, idGainMap_[sessionId]->GetNodeId(), preNodeInfo);
    }
#endif
}

void HpaeProcessCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    uint32_t sessionId = preNode->GetNodeInfo().sessionId;
    AUDIO_INFO_LOG(
        "Process DisConnect sessionId is %{public}u, streamType is %{public}d",
        sessionId, preNode->GetNodeInfo().streamType);
#ifdef ENABLE_HIDUMP_DFX
    auto callBack = mixerNode_->GetNodeStatusCallback().lock();
    if (callBack != nullptr && SafeGetMap(idConverterMap_, sessionId)) {
        callBack->OnNotifyDfxNodeInfo(false, idConverterMap_[sessionId]->GetNodeId(),
            idConverterMap_[sessionId]->GetNodeInfo());
    }
#endif
    if (SafeGetMap(idConverterMap_, sessionId)) {
        idGainMap_[sessionId]->DisConnect(preNode);
        idConverterMap_[sessionId]->DisConnect(idGainMap_[sessionId]);
        mixerNode_->DisConnect(idConverterMap_[sessionId]);
        idConverterMap_.erase(sessionId);
        idGainMap_.erase(sessionId);
        AUDIO_INFO_LOG("Process DisConnect Exist converterNode preOutNum is %{public}zu", mixerNode_->GetPreOutNum());
    }
    if (renderEffectNode_ != nullptr && mixerNode_->GetPreOutNum() == 0) {
        renderEffectNode_->DisConnect(mixerNode_);
        AUDIO_INFO_LOG("Process DisConnect mixerNode_");
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = renderEffectNode_->GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfo(false, renderEffectNode_->GetNodeId(), mixerNode_->GetNodeInfo());
        }
#endif
    }
}

int32_t HpaeProcessCluster::GetEffectNodeInputChannelInfo(uint32_t &channels, uint64_t &channelLayout)
{
    if (renderEffectNode_ == nullptr) {
        return ERR_READ_FAILED;
    }
    int32_t ret = renderEffectNode_->GetExpectedInputChannelInfo(channels, channelLayout);
    return ret;
}

int32_t HpaeProcessCluster::AudioRendererCreate(HpaeNodeInfo &nodeInfo)
{
    if (renderEffectNode_ == nullptr) {
        return 0;
    }
    return renderEffectNode_->AudioRendererCreate(nodeInfo);
}

int32_t HpaeProcessCluster::AudioRendererStart(HpaeNodeInfo &nodeInfo)
{
    if (renderEffectNode_ == nullptr) {
        return 0;
    }
    return renderEffectNode_->AudioRendererStart(nodeInfo);
}

int32_t HpaeProcessCluster::AudioRendererStop(HpaeNodeInfo &nodeInfo)
{
    if (renderEffectNode_ == nullptr) {
        return 0;
    }
    return renderEffectNode_->AudioRendererStop(nodeInfo);
}

int32_t HpaeProcessCluster::AudioRendererRelease(HpaeNodeInfo &nodeInfo)
{
    if (renderEffectNode_ == nullptr) {
        return 0;
    }
    return renderEffectNode_->AudioRendererRelease(nodeInfo);
}

std::shared_ptr<HpaeGainNode> HpaeProcessCluster::GetGainNodeById(uint32_t sessionId) const
{
    return SafeGetMap(idGainMap_, sessionId);
}

std::shared_ptr<HpaeAudioFormatConverterNode> HpaeProcessCluster::GetConverterNodeById(uint32_t sessionId) const
{
    return SafeGetMap(idConverterMap_, sessionId);
}

void HpaeProcessCluster::SetConnectedFlag(bool flag)
{
    isConnectedToOutputCluster = flag;
}

bool HpaeProcessCluster::GetConnectedFlag() const
{
    return isConnectedToOutputCluster;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS