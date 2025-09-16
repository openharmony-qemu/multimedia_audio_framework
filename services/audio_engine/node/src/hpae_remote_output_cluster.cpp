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
    AUDIO_INFO_LOG("frameLenMs_:%{public}u ms,"
        "timeoutThdFrames_:%{public}u", frameLenMs_, timeoutThdFrames_);
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
    std::vector<HpaeProcessorType> needErased;
    for (auto &mixerNode : sceneMixerMap_) {
        if (mixerNode.second->GetPreOutNum() == 0) {
            ++sceneStopCountMap_[mixerNode.first];
        } else {
            sceneStopCountMap_[mixerNode.first] = 0;
        }
        if (sceneStopCountMap_[mixerNode.first] > timeoutThdFrames_) {
            needErased.emplace_back(mixerNode.first);
            hpaeSinkOutputNode_->DisConnect(mixerNode.second);
        }
    }
    for (auto sceneType : needErased) {
        sceneMixerMap_.erase(sceneType);
    }
    if (hpaeSinkOutputNode_->GetPreOutNum() == 0) {
        int32_t ret = hpaeSinkOutputNode_->RenderSinkStop();
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
#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = hpaeSinkOutputNode_->GetNodeStatusCallback().lock()) {
        callBack->OnNotifyDfxNodeInfo(false, hpaeSinkOutputNode_->GetNodeId(), hpaeSinkOutputNode_->GetNodeInfo());
    }
#endif
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
    AUDIO_INFO_LOG("input sceneType is %{public}u", sceneType);
    AUDIO_INFO_LOG("input rate is %{public}u, ch is %{public}u",
        preNodeInfo.samplingRate, preNodeInfo.channels);
    AUDIO_INFO_LOG("output rate is %{public}u, ch is %{public}u",
        nodeInfo.samplingRate, nodeInfo.channels);
    AUDIO_INFO_LOG("preNode name %{public}s, curNode name is %{public}s",
        preNodeInfo.nodeName.c_str(), nodeInfo.nodeName.c_str());
    nodeInfo.sceneType = sceneType;
    if (!SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, nodeInfo);
    }
    if (!SafeGetMap(sceneMixerMap_, sceneType)) {
        sceneMixerMap_[sceneType] = std::make_shared<HpaeMixerNode>(nodeInfo);
        sceneStopCountMap_[sceneType] = 0;
        hpaeSinkOutputNode_->Connect(sceneMixerMap_[sceneType]);
    }
    sceneMixerMap_[sceneType]->Connect(sceneConverterMap_[sceneType]);
    sceneConverterMap_[sceneType]->Connect(preNode);
    connectedProcessCluster_.insert(sceneType);
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
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
