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

HpaeRemoteOutputCluster::HpaeRemoteOutputCluster(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), HpaeOutputCluster(nodeInfo),
      hpaeSinkOutputNode_(std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo))
{
#ifdef ENABLE_HIDUMP_DFX
    if (nodeInfo.statusCallback.lock()) {
        nodeInfo.nodeName = "hpaeSinkOutputNode";
        nodeInfo.nodeId = nodeInfo.statusCallback.lock()->OnGetNodeId();
        hpaeSinkOutputNode_->SetNodeInfo(nodeInfo);
        nodeInfo.statusCallback.lock()->OnNotifyDfxNodeInfo(true, 0, nodeInfo);
    }
#endif
    frameLenMs_ = nodeInfo.frameLen * MILLISECOND_PER_SECOND / nodeInfo.samplingRate;
    AUDIO_INFO_LOG("HpaeRemoteOutputCluster frameLenMs_:%{public}u ms,"
        "timeoutThdFrames_:%{public}u", frameLenMs_, timeoutThdFrames_);
}

HpaeRemoteOutputCluster::~HpaeRemoteOutputCluster()
{
    Reset();
}

void HpaeRemoteOutputCluster::DoProcess()
{
    Trace trace("HpaeRemoteOutputCluster::DoProcess");
    hpaeSinkOutputNode_->DoProcess();
    if (hpaeSinkOutputNode_->GetPreOutNum() == 0) {
        timeoutStopCount_++;
    } else {
        timeoutStopCount_ = 0;
    }
    if (timeoutStopCount_ > timeoutThdFrames_) {
        int32_t ret = hpaeSinkOutputNode_->RenderSinkStop();
        timeoutStopCount_ = 0;
        AUDIO_INFO_LOG("HpaeRemoteOutputCluster timeout RenderSinkStop ret :%{public}d", ret);
    }
}

bool HpaeRemoteOutputCluster::Reset()
{
    hpaeSinkOutputNode_->Reset();
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
    HpaeNodeInfo &curNodeInfo = GetNodeInfo();
    HpaeProcessorType sceneType = preNodeInfo.sceneType;
    AUDIO_INFO_LOG("HpaeRemoteOutputCluster input sceneType is %{public}u", preNodeInfo.sceneType);
    AUDIO_INFO_LOG("HpaeRemoteOutputCluster input rate is %{public}u, ch is %{public}u",
        preNodeInfo.samplingRate, preNodeInfo.channels);
    AUDIO_INFO_LOG(" HpaeRemoteOutputCluster output rate is %{public}u, ch is %{public}u",
        curNodeInfo.samplingRate, curNodeInfo.channels);
    AUDIO_INFO_LOG(" HpaeRemoteOutputCluster preNode name %{public}s, curNode name is %{public}s",
        preNodeInfo.nodeName.c_str(), curNodeInfo.nodeName.c_str());

#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = hpaeSinkOutputNode_->GetNodeStatusCallback().lock()) {
        curNodeInfo.nodeId = callBack->OnGetNodeId();
        curNodeInfo.nodeName = "HpaeAudioFormatConverterNode";
    }
#endif
    
    if (!SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, curNodeInfo);
    } else {
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = hpaeSinkOutputNode_->GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfo(false, hpaeSinkOutputNode_->GetNodeId(),
                sceneConverterMap_[sceneType]->GetNodeInfo());
        }
#endif
        sceneConverterMap_.erase(sceneType);
        sceneConverterMap_[sceneType] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, curNodeInfo);
    }
    sceneConverterMap_[sceneType]->Connect(preNode);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = hpaeSinkOutputNode_->GetNodeStatusCallback().lock()) {
        AUDIO_INFO_LOG("HpaeRemoteOutputCluster connect curNodeInfo name %{public}s", curNodeInfo.nodeName.c_str());
        AUDIO_INFO_LOG("HpaeRemoteOutputCluster connect preNodeInfo name %{public}s", preNodeInfo.nodeName.c_str());
        callBack->OnNotifyDfxNodeInfo(true, hpaeSinkOutputNode_->GetNodeId(), curNodeInfo);
        callBack->OnNotifyDfxNodeInfo(true, curNodeInfo.nodeId, preNodeInfo);
    }
#endif
    hpaeSinkOutputNode_->Connect(sceneConverterMap_[sceneType]);
    connectedProcessCluster_.insert(sceneType);
}

void HpaeRemoteOutputCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo &preNodeInfo = preNode->GetSharedInstance()->GetNodeInfo();
    HpaeProcessorType sceneType = preNodeInfo.sceneType;
    AUDIO_INFO_LOG("HpaeRemoteOutputCluster input sceneType is %{public}u", preNodeInfo.sceneType);
    if (SafeGetMap(sceneConverterMap_, sceneType)) {
        sceneConverterMap_[sceneType]->DisConnect(preNode);
        hpaeSinkOutputNode_->DisConnect(sceneConverterMap_[sceneType]);
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = hpaeSinkOutputNode_->GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfo(false, hpaeSinkOutputNode_->GetNodeId(),
                sceneConverterMap_[sceneType]->GetNodeInfo());
        }
#endif
        sceneConverterMap_.erase(sceneType);
    } else {
        hpaeSinkOutputNode_->DisConnect(preNode);
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = hpaeSinkOutputNode_->GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfo(false, hpaeSinkOutputNode_->GetNodeId(), preNodeInfo);
        }
#endif
    }
    connectedProcessCluster_.erase(sceneType);
}

int32_t HpaeRemoteOutputCluster::GetConverterNodeCount()
{
    return sceneConverterMap_.size();
}

int32_t HpaeRemoteOutputCluster::GetInstance(std::string deviceClass, std::string deviceNetId)
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
    AUDIO_INFO_LOG(
        "SetTimeoutStopThd: timeoutThdFrames_:%{public}u, timeoutThdMs :%{public}u", timeoutThdFrames_, timeoutThdMs);
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
