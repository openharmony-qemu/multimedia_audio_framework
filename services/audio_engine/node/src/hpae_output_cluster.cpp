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
#define LOG_TAG "HpaeOutputCluster"
#endif

#include "hpae_output_cluster.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeOutputCluster::HpaeOutputCluster(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), mixerNode_(std::make_shared<HpaeMixerNode>(nodeInfo)),
      hpaeSinkOutputNode_(std::make_shared<HpaeSinkOutputNode>(nodeInfo))
{
#ifdef ENABLE_HIDUMP_DFX
    if (nodeInfo.statusCallback.lock()) {
        nodeInfo.nodeName = "hpaeSinkOutputNode";
        nodeInfo.nodeId = nodeInfo.statusCallback.lock()->OnGetNodeId();
        hpaeSinkOutputNode_->SetNodeInfo(nodeInfo);
        nodeInfo.statusCallback.lock()->OnNotifyDfxNodeInfo(true, 0, nodeInfo);
        nodeInfo.nodeName = "HpaeMixerNode";
        nodeInfo.nodeId = nodeInfo.statusCallback.lock()->OnGetNodeId();
        mixerNode_->SetNodeInfo(nodeInfo);
        nodeInfo.statusCallback.lock()->OnNotifyDfxNodeInfo(true, hpaeSinkOutputNode_->GetNodeId(), nodeInfo);
    }
#endif
    if (mixerNode_->SetupAudioLimiter() != SUCCESS) {
        AUDIO_INFO_LOG("HpaeOutputCluster mixerNode SetupAudioLimiter failed!");
    }
    hpaeSinkOutputNode_->Connect(mixerNode_);
    frameLenMs_ = nodeInfo.frameLen * MILLISECOND_PER_SECOND / nodeInfo.samplingRate;
    AUDIO_INFO_LOG(
        "HpaeOutputCluster frameLenMs_:%{public}u ms, timeoutThdFrames_:%{public}u", frameLenMs_, timeoutThdFrames_);
}

HpaeOutputCluster::~HpaeOutputCluster()
{
    Reset();
}

void HpaeOutputCluster::DoProcess()
{
    Trace trace("HpaeOutputCluster::DoProcess");
    hpaeSinkOutputNode_->DoProcess();
    if (mixerNode_->GetPreOutNum() == 0) {
        timeoutStopCount_++;
    } else {
        timeoutStopCount_ = 0;
    }
    if (timeoutStopCount_ > timeoutThdFrames_) {
        int32_t ret = hpaeSinkOutputNode_->RenderSinkStop();
        timeoutStopCount_ = 0;
        AUDIO_INFO_LOG("HpaeOutputCluster timeout RenderSinkStop ret :%{public}d", ret);
    }
}

bool HpaeOutputCluster::Reset()
{
    mixerNode_->Reset();
    for (auto converterNode : sceneConverterMap_) {
        converterNode.second->Reset();
    }
    hpaeSinkOutputNode_->DisConnect(mixerNode_);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = hpaeSinkOutputNode_->GetNodeStatusCallback().lock()) {
        callBack->OnNotifyDfxNodeInfo(false, hpaeSinkOutputNode_->GetNodeId(), hpaeSinkOutputNode_->GetNodeInfo());
    }
#endif
    return true;
}

bool HpaeOutputCluster::ResetAll()
{
    return hpaeSinkOutputNode_->ResetAll();  // Complete the code here
}

void HpaeOutputCluster::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo &preNodeInfo = preNode->GetSharedInstance()->GetNodeInfo();
    HpaeNodeInfo &curNodeInfo = GetNodeInfo();
    HpaeProcessorType sceneType = preNodeInfo.sceneType;
    AUDIO_INFO_LOG("HpaeOutputCluster input sceneType is %{public}u, "
        "input:[%{public}u_%{public}u], output:[%{public}u_%{public}u], "
        "preNode name %{public}s, curNode name %{public}s, "
        "mixer id %{public}u, SinkOut id %{public}u", preNodeInfo.sceneType,
        preNodeInfo.samplingRate, preNodeInfo.channels,
        curNodeInfo.samplingRate, curNodeInfo.channels,
        preNodeInfo.nodeName.c_str(), curNodeInfo.nodeName.c_str(),
        mixerNode_->GetNodeId(), hpaeSinkOutputNode_->GetNodeId());

#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = mixerNode_->GetNodeStatusCallback().lock()) {
        curNodeInfo.nodeId = callBack->OnGetNodeId();
        curNodeInfo.nodeName = "HpaeAudioFormatConverterNode";
    }
#endif
    
    if (!SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, curNodeInfo);
    } else {
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = mixerNode_->GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfo(false, mixerNode_->GetNodeId(), sceneConverterMap_[sceneType]->GetNodeInfo());
        }
#endif
        sceneConverterMap_.erase(sceneType);
        sceneConverterMap_[sceneType] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, curNodeInfo);
    }
    sceneConverterMap_[sceneType]->Connect(preNode);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = mixerNode_->GetNodeStatusCallback().lock()) {
        AUDIO_INFO_LOG("HpaeOutputCluster connect curNodeInfo name %{public}s", curNodeInfo.nodeName.c_str());
        AUDIO_INFO_LOG("HpaeOutputCluster connect preNodeInfo name %{public}s", preNodeInfo.nodeName.c_str());
        callBack->OnNotifyDfxNodeInfo(true, mixerNode_->GetNodeId(), curNodeInfo);
        callBack->OnNotifyDfxNodeInfo(true, curNodeInfo.nodeId, preNodeInfo);
    }
#endif
    mixerNode_->Connect(sceneConverterMap_[sceneType]);
    connectedProcessCluster_.insert(sceneType);
}

void HpaeOutputCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo &preNodeInfo = preNode->GetSharedInstance()->GetNodeInfo();
    HpaeProcessorType sceneType = preNodeInfo.sceneType;
    AUDIO_INFO_LOG("HpaeOutputCluster input sceneType is %{public}u", preNodeInfo.sceneType);
    if (SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType]->DisConnect(preNode);
        mixerNode_->DisConnect(sceneConverterMap_[sceneType]);
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = mixerNode_->GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfo(false, mixerNode_->GetNodeId(), sceneConverterMap_[sceneType]->GetNodeInfo());
        }
#endif
        sceneConverterMap_.erase(sceneType);
    } else {
        mixerNode_->DisConnect(preNode);
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = mixerNode_->GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfo(false, mixerNode_->GetNodeId(), preNodeInfo);
        }
#endif
    }
    connectedProcessCluster_.erase(sceneType);
}

int32_t HpaeOutputCluster::GetConverterNodeCount()
{
    return sceneConverterMap_.size();
}

int32_t HpaeOutputCluster::GetInstance(std::string deviceClass, std::string deviceNetId)
{
    return hpaeSinkOutputNode_->GetRenderSinkInstance(deviceClass, deviceNetId);
}

int32_t HpaeOutputCluster::Init(IAudioSinkAttr &attr)
{
    return hpaeSinkOutputNode_->RenderSinkInit(attr);
}

int32_t HpaeOutputCluster::DeInit()
{
    return hpaeSinkOutputNode_->RenderSinkDeInit();
}

int32_t HpaeOutputCluster::Flush(void)
{
    return hpaeSinkOutputNode_->RenderSinkFlush();
}

int32_t HpaeOutputCluster::Pause(void)
{
    return hpaeSinkOutputNode_->RenderSinkPause();
}

int32_t HpaeOutputCluster::ResetRender(void)
{
    return hpaeSinkOutputNode_->RenderSinkReset();
}

int32_t HpaeOutputCluster::Resume(void)
{
    return hpaeSinkOutputNode_->RenderSinkResume();
}

int32_t HpaeOutputCluster::Start(void)
{
    return hpaeSinkOutputNode_->RenderSinkStart();
}

int32_t HpaeOutputCluster::Stop(void)
{
    return hpaeSinkOutputNode_->RenderSinkStop();
}

const char *HpaeOutputCluster::GetFrameData(void)
{
    return hpaeSinkOutputNode_->GetRenderFrameData();
}

StreamManagerState HpaeOutputCluster::GetState(void)
{
    return hpaeSinkOutputNode_->GetSinkState();
}

int32_t HpaeOutputCluster::GetPreOutNum()
{
    return mixerNode_->GetPreOutNum();
}

int32_t HpaeOutputCluster::SetTimeoutStopThd(uint32_t timeoutThdMs)
{
    if (frameLenMs_ != 0) {
        timeoutThdFrames_ = timeoutThdMs / frameLenMs_;
    }
    AUDIO_INFO_LOG(
        "SetTimeoutStopThd: timeoutThdFrames_:%{public}u, timeoutThdMs :%{public}u", timeoutThdFrames_, timeoutThdMs);
    return SUCCESS;
}

bool HpaeOutputCluster::IsProcessClusterConnected(HpaeProcessorType sceneType)
{
    return connectedProcessCluster_.find(sceneType) != connectedProcessCluster_.end();
}

int32_t HpaeOutputCluster::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return hpaeSinkOutputNode_->UpdateAppsUid(appsUid);
}

int32_t HpaeOutputCluster::SetPriPaPower(void)
{
    return hpaeSinkOutputNode_->RenderSinkSetPriPaPower();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS