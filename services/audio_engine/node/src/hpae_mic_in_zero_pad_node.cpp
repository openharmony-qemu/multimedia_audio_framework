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
#define LOG_TAG "HpaeMicInZeroPadNode"
#endif

#include "hpae_mic_in_zero_pad_node.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#include "stream_dfx_manager.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeMicInZeroPadNode::HpaeMicInZeroPadNode(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
    zeroPadOutput_(pcmBufferInfo_),
    lastUpstreamChannels_(static_cast<uint32_t>(preNodeInfo.channels))
{
    zeroPadOutput_.SetSplitStreamType(preNodeInfo.GetSplitStreamType());
    zeroPadOutput_.SetAudioStreamType(preNodeInfo.streamType);
    zeroPadOutput_.SetAudioStreamUsage(preNodeInfo.effectInfo.streamUsage);
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeMicInZeroPadNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
    AUDIO_INFO_LOG("NodeId %{public}d: micIn-zero-pad node, main %{public}u ch, total %{public}u ch",
        GetNodeId(), static_cast<uint32_t>(preNodeInfo.channels),
        static_cast<uint32_t>(nodeInfo.channels));
}

HpaeMicInZeroPadNode::~HpaeMicInZeroPadNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

HpaePcmBuffer *HpaeMicInZeroPadNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    zeroPadOutput_.Reset();
    if (inputs.empty() || inputs[0] == nullptr || !inputs[0]->IsValid()) {
        return PrepareSilenceBuffer(silenceData_);
    }
    HpaePcmBuffer *input = inputs[0];
    if (lastUpstreamChannels_ != 0 && input->GetChannelCount() != lastUpstreamChannels_) {
        AUDIO_WARNING_LOG("NodeId %{public}d: upstream channels changed %{public}u -> %{public}u, "
            "frozen total width %{public}u may be stale, node rebuild required",
            GetNodeId(), lastUpstreamChannels_, input->GetChannelCount(),
            static_cast<uint32_t>(GetChannelCount()));
    }
    lastUpstreamChannels_ = input->GetChannelCount();

    uint32_t mainCh = input->GetChannelCount();
    uint32_t totalCh = zeroPadOutput_.GetChannelCount();
    if (mainCh > totalCh) {
        AUDIO_ERR_LOG("NodeId %{public}d: upstream %{public}u ch > total %{public}u ch, output silence",
            GetNodeId(), mainCh, totalCh);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM), "upstream channels > total channels", false);
        return PrepareSilenceBuffer(silenceData_);
    }
    if (input->GetFrameLen() != zeroPadOutput_.GetFrameLen()) {
        AUDIO_ERR_LOG("NodeId %{public}d: input frameLen %{public}u != node frameLen %{public}u, output silence",
            GetNodeId(), input->GetFrameLen(), zeroPadOutput_.GetFrameLen());
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM), "frameLen mismatch", false);
        return PrepareSilenceBuffer(silenceData_);
    }
    uint32_t frames = input->GetFrameLen();
    size_t mainSamples = static_cast<size_t>(frames) * mainCh;
    errno_t ret = memcpy_s(zeroPadOutput_.GetPcmDataBuffer(), zeroPadOutput_.Size(),
        input->GetPcmDataBuffer(), mainSamples * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: main block memcpy_s failed", GetNodeId());
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM), "main block memcpy_s failed", false);
        return PrepareSilenceBuffer(silenceData_);
    }
    // trailing micIn block memset 0 (single contiguous fill, not per-frame interleaving)
    size_t totalSamples = static_cast<size_t>(frames) * totalCh;
    errno_t zret = memset_s(zeroPadOutput_.GetPcmDataBuffer() + mainSamples,
        zeroPadOutput_.Size() - mainSamples * sizeof(float), 0,
        (totalSamples - mainSamples) * sizeof(float));
    if (zret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: micIn block memset_s failed", GetNodeId());
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM), "micIn block memset_s failed", false);
        return PrepareSilenceBuffer(silenceData_);
    }
    zeroPadOutput_.SetBufferState(input->GetBufferState());
    return &zeroPadOutput_;
}

void HpaeMicInZeroPadNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    zeroPadOutput_.SetSourceBufferType(nodeInfo.sourceBufferType);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
    }
#endif
}

void HpaeMicInZeroPadNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(false, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
    }
#endif
}

void HpaeMicInZeroPadNode::SetSourceBufferType(HpaeSourceBufferType type)
{
    zeroPadOutput_.SetSourceBufferType(type);
}

uint64_t HpaeMicInZeroPadNode::GetLatency(uint32_t sessionId)
{
    (void)sessionId;
    return 0; // frame-in frame-out, no internal buffering
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
