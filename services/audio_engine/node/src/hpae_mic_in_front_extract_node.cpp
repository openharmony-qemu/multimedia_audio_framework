/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "HpaeMicInFrontExtractNode"
#endif

#include "hpae_mic_in_front_extract_node.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#include "stream_dfx_manager.h"
#include "securec.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeMicInFrontExtractNode::HpaeMicInFrontExtractNode(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
    extractOutput_(pcmBufferInfo_),
    lastUpstreamChannels_(static_cast<uint32_t>(preNodeInfo.channels))
{
    extractOutput_.SetSplitStreamType(preNodeInfo.GetSplitStreamType());
    extractOutput_.SetAudioStreamType(preNodeInfo.streamType);
    extractOutput_.SetAudioStreamUsage(preNodeInfo.effectInfo.streamUsage);
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeMicInFrontExtractNode");
    auto callback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN(callback);
    callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
#endif
    AUDIO_INFO_LOG("NodeId %{public}d: front-extract node, raw in %{public}u ch -> main %{public}u ch",
        GetNodeId(), static_cast<uint32_t>(preNodeInfo.channels),
        static_cast<uint32_t>(nodeInfo.channels));
}

HpaeMicInFrontExtractNode::~HpaeMicInFrontExtractNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    auto callback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN(callback);
    callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
#endif
}

HpaePcmBuffer *HpaeMicInFrontExtractNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    extractOutput_.Reset();
    if (inputs.empty() || inputs[0] == nullptr || !inputs[0]->IsValid()) {
        return PrepareSilenceBuffer(silenceData_);
    }
    HpaePcmBuffer *input = inputs[0];
    if (lastUpstreamChannels_ != 0 && input->GetChannelCount() != lastUpstreamChannels_) {
        AUDIO_WARNING_LOG("NodeId %{public}d: upstream channels changed %{public}u -> %{public}u, "
            "frozen main width %{public}u may be stale, node rebuild required",
            GetNodeId(), lastUpstreamChannels_, input->GetChannelCount(), static_cast<uint32_t>(GetChannelCount()));
    }
    lastUpstreamChannels_ = input->GetChannelCount();

    uint32_t mainCh = extractOutput_.GetChannelCount();
    if (input->GetChannelCount() < mainCh) {
        AUDIO_ERR_LOG("NodeId %{public}d: upstream %{public}u ch < main %{public}u ch, output silence",
            GetNodeId(), input->GetChannelCount(), mainCh);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM), "upstream channels < main channels", false);
        return PrepareSilenceBuffer(silenceData_);
    }
    if (input->GetFrameLen() != extractOutput_.GetFrameLen()) {
        AUDIO_ERR_LOG("NodeId %{public}d: input frameLen %{public}u != node frameLen %{public}u, output silence",
            GetNodeId(), input->GetFrameLen(), extractOutput_.GetFrameLen());
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM), "frameLen mismatch", false);
        return PrepareSilenceBuffer(silenceData_);
    }
    // Non-interleaved partition [main block][micIn block]: the leading main block is contiguous.
    uint32_t frames = input->GetFrameLen();
    size_t mainSamples = static_cast<size_t>(frames) * static_cast<size_t>(mainCh);
    size_t mainBytes = mainSamples * sizeof(float);
    // explicit bound before the copy: guards the main block against the output buffer
    // (Size() also carries 64-byte alignment slack, so this stays conservative)
    if (mainBytes > extractOutput_.Size()) {
        AUDIO_ERR_LOG("NodeId %{public}d: main block %{public}zu bytes exceeds buffer %{public}zu",
            GetNodeId(), mainBytes, extractOutput_.Size());
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM), "main block exceeds buffer size", false);
        return PrepareSilenceBuffer(silenceData_);
    }
    errno_t ret = memcpy_s(extractOutput_.GetPcmDataBuffer(), extractOutput_.Size(),
        input->GetPcmDataBuffer(), mainBytes);
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: main block memcpy_s failed", GetNodeId());
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM), "main block memcpy_s failed", false);
        return PrepareSilenceBuffer(silenceData_);
    }
    extractOutput_.SetBufferState(input->GetBufferState());
    return &extractOutput_;
}

void HpaeMicInFrontExtractNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    extractOutput_.SetSourceBufferType(nodeInfo.sourceBufferType);
#ifdef ENABLE_HIDUMP_DFX
    auto callback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN(callback);
    callback->OnNotifyDfxNodeInfo(true, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
#endif
}

void HpaeMicInFrontExtractNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
#ifdef ENABLE_HIDUMP_DFX
    auto callback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN(callback);
    callback->OnNotifyDfxNodeInfo(false, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
#endif
}

void HpaeMicInFrontExtractNode::SetSourceBufferType(HpaeSourceBufferType type)
{
    extractOutput_.SetSourceBufferType(type);
}

uint64_t HpaeMicInFrontExtractNode::GetLatency(uint32_t sessionId)
{
    (void)sessionId;
    return 0; // frame-in frame-out, no internal buffering
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
