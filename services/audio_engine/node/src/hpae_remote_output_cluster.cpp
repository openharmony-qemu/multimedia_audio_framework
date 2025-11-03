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
#define LOG_TAG "HpaeRemoteOutputCluster"
#endif

#include <sstream>
#include "hpae_remote_output_cluster.h"
#include "hpae_node_common.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeRemoteOutputCluster::HpaeRemoteOutputCluster(HpaeNodeInfo &nodeInfo, HpaeSinkInfo &sinkInfo)
    : HpaeNode(nodeInfo), hpaeSinkOutputNode_(std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo))
{
    frameLenMs_ = nodeInfo.frameLen * MILLISECOND_PER_SECOND / nodeInfo.samplingRate;
    AUDIO_INFO_LOG("frameLenMs_:%{public}u ms, timeoutThdFrames_:%{public}u", frameLenMs_, timeoutThdFrames_);
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeRemoteOutputCluster");
#endif
}

HpaeRemoteOutputCluster::~HpaeRemoteOutputCluster()
{
    Reset();
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeRemoteOutputCluster::DoProcess()
{
    Trace trace("HpaeRemoteOutputCluster::DoProcess");
    hpaeSinkOutputNode_->DoProcess();
    
    for (auto mixerNodeIt = sceneMixerMap_.begin(); mixerNodeIt != sceneMixerMap_.end();) {
        if (mixerNodeIt->second->GetPreOutNum() != 0) {
            ++mixerNodeIt;
            stopCount_ = 0;
            continue;
        }
        if (sceneMixerMap_.size() == 1) {
            ++stopCount_;
            break;
        }
        hpaeSinkOutputNode_->DisConnect(mixerNodeIt->second);
        mixerNodeIt = sceneMixerMap_.erase(mixerNodeIt);
    }
    
    if (stopCount_ > timeoutThdFrames_) {
        if (!sceneMixerMap_.empty()) {
            hpaeSinkOutputNode_->DisConnect(sceneMixerMap_.begin()->second);
        }
        sceneMixerMap_.clear();
        int32_t ret = hpaeSinkOutputNode_->RenderSinkStop();
        stopCount_ = 0;
        AUDIO_INFO_LOG("timeout RenderSinkStop ret :%{public}d", ret);
    }
}

bool HpaeRemoteOutputCluster::Reset()
{
    hpaeSinkOutputNode_->Reset();
    for (auto &mixerNode : sceneMixerMap_) {
        mixerNode.second->Reset();
    }
    for (auto converterNode : sceneConverterMap_) {
        converterNode.second->Reset();
    }
    return true;
}

bool HpaeRemoteOutputCluster::ResetAll()
{
    return hpaeSinkOutputNode_->ResetAll();  // Complete the code here
}

void HpaeRemoteOutputCluster::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo &preNodeInfo = preNode->GetSharedInstance()->GetNodeInfo();
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    HpaeProcessorType sceneType = preNodeInfo.sceneType;
    AUDIO_INFO_LOG("input sceneType is %{public}u input rate is %{public}u, ch is %{public}u"
        "output rate is %{public}u, ch is %{public}u preNode name %{public}s, curNode name is %{public}s",
        sceneType, preNodeInfo.samplingRate, preNodeInfo.channels, nodeInfo.samplingRate, nodeInfo.channels,
        preNodeInfo.nodeName.c_str(), nodeInfo.nodeName.c_str());
    nodeInfo.sceneType = sceneType;
    nodeInfo.streamType = preNodeInfo.streamType;
    nodeInfo.effectInfo.streamUsage = preNodeInfo.effectInfo.streamUsage;
    if (!SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, nodeInfo);
    }
    if (!SafeGetMap(sceneMixerMap_, sceneType)) {
        sceneMixerMap_[sceneType] = std::make_shared<HpaeMixerNode>(nodeInfo);
        hpaeSinkOutputNode_->Connect(sceneMixerMap_[sceneType]);
    }
    sceneMixerMap_[sceneType]->Connect(sceneConverterMap_[sceneType]);
    sceneConverterMap_[sceneType]->Connect(preNode);
    UpdateStreamInfo(preNode);
    connectedProcessCluster_.insert(sceneType);
}

void HpaeRemoteOutputCluster::UpdateStreamInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> preNode)
{
    CHECK_AND_RETURN_LOG(preNode != nullptr, "the param proNode is nullptr");
    const HpaeNodeInfo &preNodeInfo = preNode->GetSharedInstance()->GetNodeInfo();
    const HpaeProcessorType sceneType = preNodeInfo.sceneType;
    
    // update mixed node streamType and streamUsage
    auto mixerNode = SafeGetMap(sceneMixerMap_, sceneType);
    CHECK_AND_RETURN_LOG(mixerNode != nullptr, "the sceneType<%{public}d> is disconnect<Mixer>", sceneType);
    HpaeNodeInfo mixerNodeInfo = mixerNode->GetNodeInfo();
    mixerNodeInfo.streamType = preNodeInfo.streamType;
    mixerNodeInfo.effectInfo.streamUsage = preNodeInfo.effectInfo.streamUsage;
    mixerNode->SetNodeInfo(mixerNodeInfo);

    // update convert node streamType and streamUsage
    auto convertNode = SafeGetMap(sceneConverterMap_, sceneType);
    CHECK_AND_RETURN_LOG(convertNode != nullptr, "the sceneType<%{public}d> is disconnect<Converter>", sceneType);
    HpaeNodeInfo converterNodeInfo = convertNode->GetNodeInfo();
    converterNodeInfo.streamType = preNodeInfo.streamType;
    converterNodeInfo.effectInfo.streamUsage = preNodeInfo.effectInfo.streamUsage;
    convertNode->SetNodeInfo(converterNodeInfo);

    const HpaeNodeInfo &printNode = sceneMixerMap_[sceneType]->GetNodeInfo();
    AUDIO_INFO_LOG("update stream info %{public}d type %{public}d usage %{public}d", printNode.nodeId,
        printNode.streamType, printNode.effectInfo.streamUsage);
}

void HpaeRemoteOutputCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo &preNodeInfo = preNode->GetSharedInstance()->GetNodeInfo();
    HpaeProcessorType sceneType = preNodeInfo.sceneType;
    AUDIO_INFO_LOG("input sceneType is %{public}u", preNodeInfo.sceneType);
    if (SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType]->DisConnect(preNode);
        sceneMixerMap_[sceneType]->DisConnect(sceneConverterMap_[sceneType]);
        sceneConverterMap_.erase(sceneType);
    }
    connectedProcessCluster_.erase(sceneType);
}

int32_t HpaeRemoteOutputCluster::GetConverterNodeCount()
{
    return sceneConverterMap_.size();
}

int32_t HpaeRemoteOutputCluster::GetInstance(const std::string &deviceClass, const std::string &deviceNetId)
{
    return hpaeSinkOutputNode_->GetRenderSinkInstance(deviceClass, deviceNetId);
}

int32_t HpaeRemoteOutputCluster::Init(IAudioSinkAttr &attr)
{
    return hpaeSinkOutputNode_->RenderSinkInit(attr);
}

int32_t HpaeRemoteOutputCluster::DeInit()
{
    return hpaeSinkOutputNode_->RenderSinkDeInit();
}

int32_t HpaeRemoteOutputCluster::Flush(void)
{
    return hpaeSinkOutputNode_->RenderSinkFlush();
}

int32_t HpaeRemoteOutputCluster::Pause(void)
{
    return hpaeSinkOutputNode_->RenderSinkPause();
}

int32_t HpaeRemoteOutputCluster::ResetRender(void)
{
    return hpaeSinkOutputNode_->RenderSinkReset();
}

int32_t HpaeRemoteOutputCluster::Resume(void)
{
    return hpaeSinkOutputNode_->RenderSinkResume();
}

int32_t HpaeRemoteOutputCluster::Start(void)
{
    return hpaeSinkOutputNode_->RenderSinkStart();
}

int32_t HpaeRemoteOutputCluster::Stop(void)
{
    return hpaeSinkOutputNode_->RenderSinkStop();
}

const char *HpaeRemoteOutputCluster::GetFrameData(void)
{
    return hpaeSinkOutputNode_->GetRenderFrameData();
}

StreamManagerState HpaeRemoteOutputCluster::GetState(void)
{
    return hpaeSinkOutputNode_->GetSinkState();
}

int32_t HpaeRemoteOutputCluster::GetPreOutNum()
{
    return hpaeSinkOutputNode_->GetPreOutNum();
}

int32_t HpaeRemoteOutputCluster::SetTimeoutStopThd(uint32_t timeoutThdMs)
{
    if (frameLenMs_ != 0) {
        timeoutThdFrames_ = timeoutThdMs / frameLenMs_;
    }
    AUDIO_INFO_LOG("timeoutThdFrames_:%{public}u, timeoutThdMs :%{public}u", timeoutThdFrames_, timeoutThdMs);
    return SUCCESS;
}

bool HpaeRemoteOutputCluster::IsProcessClusterConnected(HpaeProcessorType sceneType)
{
    return connectedProcessCluster_.find(sceneType) != connectedProcessCluster_.end();
}

HpaeProcessorType TransStreamUsageToSplitSceneType(StreamUsage streamUsage, const std::string &splitMode)
{
    static constexpr int splitOneStream = 1;
    static constexpr int splitTwoStream = 2;
    static constexpr int splitThreeStream = 3;
    static constexpr int maxParts = 3;
    AUDIO_INFO_LOG("streamUsage is: %{public}d, splitMode is: %{public}s",
        static_cast<int>(streamUsage), splitMode.c_str());
    int splitNums = 0;
    if (splitMode.empty()) {
        AUDIO_ERR_LOG("input SPLIT_MODE is empty");
        return HPAE_SCENE_DEFAULT;
    }
    std::istringstream iss(splitMode);
    std::string token;
    while (splitNums < maxParts && std::getline(iss, token, ':')) {
        ++splitNums;
    }
    const auto getSceneType = [streamUsage](size_t splitNums) -> HpaeProcessorType {
        return
            (splitNums == splitOneStream) ? HPAE_SCENE_SPLIT_MEDIA :
            (splitNums == splitTwoStream) ? (streamUsage == STREAM_USAGE_NAVIGATION ?
                HPAE_SCENE_SPLIT_NAVIGATION : HPAE_SCENE_SPLIT_MEDIA) :
            (splitNums == splitThreeStream) ? (
                (streamUsage == STREAM_USAGE_NAVIGATION) ? HPAE_SCENE_SPLIT_NAVIGATION :
                (streamUsage == STREAM_USAGE_VOICE_COMMUNICATION || streamUsage == STREAM_USAGE_VIDEO_COMMUNICATION)
                    ? HPAE_SCENE_SPLIT_COMMUNICATION
                    : HPAE_SCENE_SPLIT_MEDIA
            ) : HPAE_SCENE_DEFAULT;
    };
    return getSceneType(splitNums);
}

int32_t HpaeRemoteOutputCluster::UpdateAppsUid(const std::vector<int32_t> &appsUid)
{
    return hpaeSinkOutputNode_->UpdateAppsUid(appsUid);
}

uint64_t HpaeRemoteOutputCluster::GetLatency(HpaeProcessorType sceneType)
{
    uint64_t latency = 0;

    latency += SafeGetMap(sceneConverterMap_, sceneType) ? sceneConverterMap_[sceneType]->GetLatency() : 0;

    latency += SafeGetMap(sceneMixerMap_, sceneType) ? sceneMixerMap_[sceneType]->GetLatency() : 0;

    return latency;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
