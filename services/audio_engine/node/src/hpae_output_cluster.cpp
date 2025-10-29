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
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeOutputCluster::HpaeOutputCluster(HpaeNodeInfo nodeInfo)
    : HpaeNode(nodeInfo), hpaeSinkOutputNode_(std::make_shared<HpaeSinkOutputNode>(nodeInfo))
{
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeOutputCluster");
#endif
    nodeInfo.frameLen = nodeInfo.samplingRate * FRAME_LEN_20MS / MILLISECOND_PER_SECOND;
    SetNodeInfo(nodeInfo);
    mixerNode_ = std::make_shared<HpaeMixerNode>(nodeInfo);
    if (mixerNode_->SetupAudioLimiter() != SUCCESS) {
        AUDIO_INFO_LOG("mixerNode SetupAudioLimiter failed!");
    }
    hpaeSinkOutputNode_->Connect(mixerNode_);
    frameLenMs_ = hpaeSinkOutputNode_->GetFrameLen() * MILLISECOND_PER_SECOND / hpaeSinkOutputNode_->GetSampleRate();
    AUDIO_INFO_LOG("frameLenMs_:%{public}u ms, timeoutThdFrames_:%{public}u", frameLenMs_, timeoutThdFrames_);
}

HpaeOutputCluster::~HpaeOutputCluster()
{
    Reset();
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeOutputCluster::RegisterCurrentDeviceCallback()
{
    std::function<void(bool)> callback = [=](bool isA2dp) {
        if (isA2dp) {
            timeoutThdFramesForDevice_ = TIME_OUT_STOP_THD_DEFAULT_FRAME;
        } else {
            timeoutThdFramesForDevice_ = timeoutThdFrames_;
        }
    };
    hpaeSinkOutputNode_->RegisterCurrentDeviceCallback(callback);
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
    if (timeoutStopCount_ > timeoutThdFramesForDevice_) {
        int32_t ret = hpaeSinkOutputNode_->RenderSinkStop();
        timeoutStopCount_ = 0;
        AUDIO_INFO_LOG("timeout RenderSinkStop ret :%{public}d", ret);
    }
}

bool HpaeOutputCluster::Reset()
{
    mixerNode_->Reset();
    for (auto converterNode : sceneConverterMap_) {
        converterNode.second->Reset();
    }
    hpaeSinkOutputNode_->DisConnect(mixerNode_);
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
    AUDIO_INFO_LOG("input sceneType is %{public}u, input:[%{public}u_%{public}u], output:[%{public}u_%{public}u], "
        "preNode name %{public}s, curNode name %{public}s, "
        "mixer id %{public}u, SinkOut id %{public}u", preNodeInfo.sceneType,
        preNodeInfo.samplingRate, preNodeInfo.channels,
        curNodeInfo.samplingRate, curNodeInfo.channels,
        preNodeInfo.nodeName.c_str(), curNodeInfo.nodeName.c_str(),
        mixerNode_->GetNodeId(), hpaeSinkOutputNode_->GetNodeId());

    if (!SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, curNodeInfo);
        // disable downmix normalization in output cluster because mixer node here enables limiter
        sceneConverterMap_[sceneType]->SetDownmixNormalization(false);
    } else {
        sceneConverterMap_.erase(sceneType);
        sceneConverterMap_[sceneType] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, curNodeInfo);
        sceneConverterMap_[sceneType]->SetDownmixNormalization(false);
    }
    mixerNode_->Connect(sceneConverterMap_[sceneType]);
    sceneConverterMap_[sceneType]->Connect(preNode);
    connectedProcessCluster_.insert(sceneType);
}

void HpaeOutputCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo &preNodeInfo = preNode->GetSharedInstance()->GetNodeInfo();
    HpaeProcessorType sceneType = preNodeInfo.sceneType;
    AUDIO_INFO_LOG("input sceneType is %{public}u", preNodeInfo.sceneType);
    if (SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType]->DisConnect(preNode);
        mixerNode_->DisConnect(sceneConverterMap_[sceneType]);
        sceneConverterMap_.erase(sceneType);
    } else {
        mixerNode_->DisConnect(preNode);
    }

    if (GetPreOutNum() == 0) {
        mixerNode_->InitAudioLimiter();
    }
    connectedProcessCluster_.erase(sceneType);
}

int32_t HpaeOutputCluster::GetConverterNodeCount()
{
    return sceneConverterMap_.size();
}

int32_t HpaeOutputCluster::GetInstance(const std::string &deviceClass, const std::string &deviceNetId)
{
    return hpaeSinkOutputNode_->GetRenderSinkInstance(deviceClass, deviceNetId);
}

int32_t HpaeOutputCluster::Init(IAudioSinkAttr &attr)
{
    RegisterCurrentDeviceCallback();
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
    timeoutThdFramesForDevice_ = timeoutThdFrames_;
    AUDIO_INFO_LOG("timeoutThdFrames_:%{public}u, timeoutThdMs :%{public}u", timeoutThdFrames_, timeoutThdMs);
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

uint32_t HpaeOutputCluster::GetHdiLatency()
{
    return hpaeSinkOutputNode_->GetLatency();
}

uint64_t HpaeOutputCluster::GetLatency(HpaeProcessorType sceneType)
{
    uint64_t latency = 0;

    latency += SafeGetMap(sceneConverterMap_, sceneType) ? sceneConverterMap_[sceneType]->GetLatency() : 0;

    latency += mixerNode_ ? mixerNode_->GetLatency() : 0;

    return latency;
}

int32_t HpaeOutputCluster::SetSyncId(int32_t syncId)
{
    return hpaeSinkOutputNode_->RenderSinkSetSyncId(syncId);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS