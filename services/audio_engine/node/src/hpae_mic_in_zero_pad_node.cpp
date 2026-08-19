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
    auto callback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN(callback);
    callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
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
    auto callback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN(callback);
    callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
#endif
}

HpaePcmBuffer *HpaeMicInZeroPadNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    zeroPadOutput_.Reset();
    if (inputs.empty() || inputs[0] == nullptr || !inputs[0]->IsValid()) {
        return PrepareSilenceBuffer(silenceData_);
    }
    HpaePcmBuffer *input = inputs[0];
    // mainCh deliberately follows the LIVE input width (unlike the front-extract node's frozen
    // width): a narrower upstream warns but still maps onto the total partition, while an
    // overrun degrades to silence in CheckUpstreamFormat.
    uint32_t mainCh = input->GetChannelCount();
    uint32_t totalCh = zeroPadOutput_.GetChannelCount();
    if (!CheckUpstreamFormat(input) || !WritePartitionedOutput(input, mainCh, totalCh)) {
        return PrepareSilenceBuffer(silenceData_);
    }
    zeroPadOutput_.SetBufferState(input->GetBufferState());
    return &zeroPadOutput_;
}

// report a hard format error via the shared capture-path DFX idiom (no rate limiting)
void HpaeMicInZeroPadNode::ReportInvalidParamEvent(const std::string &reason)
{
    StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
        BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
            BusinessScenario::QUERY, ERR_INVALID_PARAM), reason, false);
}

// guard the frozen-width invariants: warn on mid-stream upstream reconfig, hard-fail on
// width overrun or frameLen mismatch (the cluster builds them equal at construction)
bool HpaeMicInZeroPadNode::CheckUpstreamFormat(HpaePcmBuffer *input)
{
    if (lastUpstreamChannels_ != 0 && input->GetChannelCount() != lastUpstreamChannels_) {
        AUDIO_WARNING_LOG("NodeId %{public}d: upstream channels changed %{public}u -> %{public}u, "
            "frozen total width %{public}u may be stale, node rebuild required",
            GetNodeId(), lastUpstreamChannels_, input->GetChannelCount(),
            static_cast<uint32_t>(GetChannelCount()));
    }
    lastUpstreamChannels_ = input->GetChannelCount();

    uint32_t mainCh = input->GetChannelCount();
    uint32_t totalCh = zeroPadOutput_.GetChannelCount();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(mainCh <= totalCh, false,
        ReportInvalidParamEvent("upstream channels > total channels"),
        "NodeId %{public}d: upstream %{public}u ch > total %{public}u ch, output silence",
        GetNodeId(), mainCh, totalCh);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(input->GetFrameLen() == zeroPadOutput_.GetFrameLen(), false,
        ReportInvalidParamEvent("frameLen mismatch"),
        "NodeId %{public}d: input frameLen %{public}u != node frameLen %{public}u, output silence",
        GetNodeId(), input->GetFrameLen(), zeroPadOutput_.GetFrameLen());
    return true;
}

// write the partitioned non-interleaved output: [main block copy][contiguous micIn zero fill]
bool HpaeMicInZeroPadNode::WritePartitionedOutput(HpaePcmBuffer *input, uint32_t mainCh, uint32_t totalCh)
{
    uint32_t frames = input->GetFrameLen();
    size_t mainSamples = static_cast<size_t>(frames) * static_cast<size_t>(mainCh);
    size_t totalSamples = static_cast<size_t>(frames) * static_cast<size_t>(totalCh);
    size_t mainBytes = mainSamples * sizeof(float);
    size_t totalBytes = totalSamples * sizeof(float);
    // explicit bound before any copy: guards the full partition against the output buffer
    // (Size() also carries 64-byte alignment slack, so this stays conservative)
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(totalBytes <= zeroPadOutput_.Size() && mainBytes <= totalBytes, false,
        ReportInvalidParamEvent("partitioned output exceeds buffer size"),
        "NodeId %{public}d: partitioned output %{public}zu bytes exceeds buffer %{public}zu",
        GetNodeId(), totalBytes, zeroPadOutput_.Size());
    errno_t ret = memcpy_s(zeroPadOutput_.GetPcmDataBuffer(), zeroPadOutput_.Size(),
        input->GetPcmDataBuffer(), mainBytes);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == EOK, false,
        ReportInvalidParamEvent("main block memcpy_s failed"),
        "NodeId %{public}d: main block memcpy_s failed", GetNodeId());
    // trailing micIn block memset 0 (single contiguous fill, not per-frame interleaving)
    errno_t zret = memset_s(zeroPadOutput_.GetPcmDataBuffer() + mainSamples,
        zeroPadOutput_.Size() - mainBytes, 0, totalBytes - mainBytes);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(zret == EOK, false,
        ReportInvalidParamEvent("micIn block memset_s failed"),
        "NodeId %{public}d: micIn block memset_s failed", GetNodeId());
    return true;
}

void HpaeMicInZeroPadNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    zeroPadOutput_.SetSourceBufferType(nodeInfo.sourceBufferType);
#ifdef ENABLE_HIDUMP_DFX
    auto callback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN(callback);
    callback->OnNotifyDfxNodeInfo(true, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
#endif
}

void HpaeMicInZeroPadNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
#ifdef ENABLE_HIDUMP_DFX
    auto callback = GetNodeStatusCallback().lock();
    CHECK_AND_RETURN(callback);
    callback->OnNotifyDfxNodeInfo(false, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
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
