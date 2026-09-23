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
#define LOG_TAG "HpaeAudioFormatConverterCluster"
#endif

#include "hpae_audio_format_converter_cluster.h"
#include "hpae_node_common.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
enum MicInScenario {
    MICIN_SCENARIO_A,   // app wants micIn AND HAL has it: full-channel path
    MICIN_SCENARIO_B,   // HAL split layout, app wants main only: [extract] -> [converter]
    MICIN_SCENARIO_C,   // app wants micIn, HAL has none: [converter] -> [zeropad]
    MICIN_SCENARIO_D,   // plain stream (no camcorder micIn info): bare converter
};

// Single classification source for a camcorder split-layout source (main + micIn):
// it decides BOTH which chain to build AND the main width (mainCh) that chain uses, so the
// branch choice and the width can never drift apart.
//  - Scenario B: HAL has a split layout (micInChannels>0) but the app only wants main
//    -> main = pre - halMicIn.
//  - Scenario C: app wants micIn (appMicInChannelCount>0) but the HAL has none
//    (micInChannels==0) -> main = total - appMicIn.
// A/D (and malformed combos) fall back to the full-channel path with main = preChannels.
MicInScenario ClassifyMicInScenario(const HpaeNodeInfo &preNodeInfo, const HpaeNodeInfo &nodeInfo,
    uint32_t &mainCh)
{
    uint32_t preChannels = static_cast<uint32_t>(preNodeInfo.channels);
    mainCh = preChannels;
    if (nodeInfo.sourceType != SOURCE_TYPE_CAMCORDER) {
        return MICIN_SCENARIO_D;
    }
    uint32_t halMicIn = static_cast<uint32_t>(nodeInfo.micInChannels);
    uint32_t appMicIn = static_cast<uint32_t>(nodeInfo.appMicInChannelCount);
    if (halMicIn > 0 && halMicIn < preChannels && appMicIn == 0) {           // Scenario B
        mainCh = preChannels - halMicIn;
        return MICIN_SCENARIO_B;
    }
    uint32_t totalChannels = static_cast<uint32_t>(nodeInfo.channels);
    if (halMicIn == 0 && appMicIn > 0 && appMicIn < totalChannels) {          // Scenario C
        mainCh = totalChannels - appMicIn;
        return MICIN_SCENARIO_C;
    }
    return MICIN_SCENARIO_A;
}

// sanitized info for the inner chain: main width + micIn fields cleared, so the vanilla
// converter (and the zero-pad node's preInfo) never re-engage mode detection
HpaeNodeInfo SanitizedMainInfo(const HpaeNodeInfo &nodeInfo, uint32_t mainCh)
{
    HpaeNodeInfo sanitized = nodeInfo;
    sanitized.channels = static_cast<AudioChannel>(mainCh);
    sanitized.micInChannels = CHANNEL_UNKNOW;
    sanitized.appMicInChannelCount = 0;
    return sanitized;
}
} // namespace

HpaeAudioFormatConverterCluster::HpaeAudioFormatConverterCluster(HpaeNodeInfo preNodeInfo,
    HpaeNodeInfo nodeInfo) : HpaeNode(nodeInfo)
{
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeAudioFormatConverterCluster");
#endif
    uint32_t preChannels = static_cast<uint32_t>(preNodeInfo.channels);
    uint32_t totalChannels = static_cast<uint32_t>(nodeInfo.channels);
    uint32_t mainCh = 0;
    MicInScenario scenario = ClassifyMicInScenario(preNodeInfo, nodeInfo, mainCh);

    if (scenario == MICIN_SCENARIO_B) {
        BuildFrontExtractChain(preNodeInfo, nodeInfo, mainCh, preChannels);
    } else if (scenario == MICIN_SCENARIO_C) {
        BuildZeroPadChain(preNodeInfo, nodeInfo, mainCh, totalChannels);
    } else {
        converterNode_ = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, nodeInfo);
        head_ = converterNode_;
        tail_ = converterNode_;
    }
}

// Scenario B (HAL split layout, app wants main only): [MicInFrontExtract] -> [Converter]
void HpaeAudioFormatConverterCluster::BuildFrontExtractChain(HpaeNodeInfo &preNodeInfo,
    HpaeNodeInfo &nodeInfo, uint32_t mainCh, uint32_t preChannels)
{
    HpaeNodeInfo mainInfo = preNodeInfo;   // extract output: pre fields at main width
    mainInfo.channels = static_cast<AudioChannel>(mainCh);
    extractNode_ = std::make_shared<HpaeMicInFrontExtractNode>(preNodeInfo, mainInfo);
    converterNode_ = std::make_shared<HpaeAudioFormatConverterNode>(mainInfo, nodeInfo);
    converterNode_->ConnectWithInfo(extractNode_, nodeInfo); // extract is the converter's upstream
    head_ = extractNode_;
    tail_ = converterNode_;
    AUDIO_INFO_LOG("scenario B chain: extract(%{public}u->%{public}u ch) -> converter",
        preChannels, mainCh);
}

// Scenario C (app wants micIn, HAL has none): [Converter] -> [MicInZeroPad]
void HpaeAudioFormatConverterCluster::BuildZeroPadChain(HpaeNodeInfo &preNodeInfo,
    HpaeNodeInfo &nodeInfo, uint32_t mainCh, uint32_t totalChannels)
{
    // mainInfo lives at the APP output rate/frameLen (the converter already resampled
    // when the zero-pad node sees the data)
    HpaeNodeInfo mainInfo = SanitizedMainInfo(nodeInfo, mainCh);
    converterNode_ = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, mainInfo);
    zeroPadNode_ = std::make_shared<HpaeMicInZeroPadNode>(mainInfo, nodeInfo);
    // ConnectWithInfo (not plain Connect) so the DFX edge is reported pre->self (converter
    // as parent); it also tags zeroPadOutput_ with nodeInfo.sourceBufferType (app mic/ec).
    zeroPadNode_->ConnectWithInfo(converterNode_, nodeInfo); // converter is the zero-pad node's upstream
    head_ = converterNode_;
    tail_ = zeroPadNode_;
    AUDIO_INFO_LOG("scenario C chain: converter -> zero-pad(%{public}u->%{public}u ch)",
        mainCh, totalChannels);
}

HpaeAudioFormatConverterCluster::~HpaeAudioFormatConverterCluster()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeAudioFormatConverterCluster::DoProcess()
{
    // data is pulled through the chain via ports (tail pull -> head pull -> upstream)
}

bool HpaeAudioFormatConverterCluster::Reset()
{
    // mirror the bare node: unlink only the EXTERNAL input so a later re-Connect restores
    // the chain. The internal link carries no per-stream state; full teardown is ResetAll's job.
    return head_ != nullptr ? head_->Reset() : true;
}

bool HpaeAudioFormatConverterCluster::ResetAll()
{
    // ResetAll from the tail walks the whole upstream chain (tail -> head -> external)
    return tail_ != nullptr ? tail_->ResetAll() : true;
}

// downstream sees the tail (the port owner); the cluster shell never appears in the runtime graph
std::shared_ptr<HpaeNode> HpaeAudioFormatConverterCluster::GetSharedInstance()
{
    return tail_ != nullptr ? tail_->GetSharedInstance() : shared_from_this();
}

std::shared_ptr<HpaeNode> HpaeAudioFormatConverterCluster::GetSharedInstance(HpaeNodeInfo &nodeInfo)
{
    (void)nodeInfo;
    return GetSharedInstance();
}

OutputPort<HpaePcmBuffer *> *HpaeAudioFormatConverterCluster::GetOutputPort()
{
    return tail_ != nullptr ? tail_->GetOutputPort() : nullptr;
}

OutputPort<HpaePcmBuffer *> *HpaeAudioFormatConverterCluster::GetOutputPort(HpaeNodeInfo &nodeInfo,
    bool isDisConnect)
{
    return tail_ != nullptr ? tail_->GetOutputPort(nodeInfo, isDisConnect) : nullptr;
}

void HpaeAudioFormatConverterCluster::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    CHECK_AND_RETURN(head_ != nullptr);
    head_->Connect(preNode);
}

void HpaeAudioFormatConverterCluster::ConnectWithInfo(
    const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode, HpaeNodeInfo &nodeInfo)
{
    CHECK_AND_RETURN(head_ != nullptr);
    head_->ConnectWithInfo(preNode, nodeInfo);
}

void HpaeAudioFormatConverterCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    CHECK_AND_RETURN(head_ != nullptr);
    head_->DisConnect(preNode);
}

void HpaeAudioFormatConverterCluster::DisConnectWithInfo(
    const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode, HpaeNodeInfo &nodeInfo)
{
    CHECK_AND_RETURN(head_ != nullptr);
    head_->DisConnectWithInfo(preNode, nodeInfo);
}

void HpaeAudioFormatConverterCluster::SetSourceNode(bool isSourceNode)
{
    CHECK_AND_RETURN(head_ != nullptr);
    head_->SetSourceNode(isSourceNode);
}

void HpaeAudioFormatConverterCluster::SetSourceBufferType(HpaeSourceBufferType type)
{
    // HpaePluginNode declares no SetSourceBufferType (each concrete node owns one), so tag the
    // concrete tail: zero-pad node in scenario C, converter node otherwise.
    if (zeroPadNode_ != nullptr) {
        zeroPadNode_->SetSourceBufferType(type); // the tag rides the final output buffer
    } else if (converterNode_ != nullptr) {
        converterNode_->SetSourceBufferType(type);
    }
}

size_t HpaeAudioFormatConverterCluster::GetOutputPortNum()
{
    return tail_ != nullptr ? tail_->GetOutputPortNum() : 0;
}

uint64_t HpaeAudioFormatConverterCluster::GetLatency(uint32_t sessionId)
{
    // extract/zero-pad nodes are frame-in-frame-out and add no latency
    return converterNode_ != nullptr ? converterNode_->GetLatency(sessionId) : 0;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
