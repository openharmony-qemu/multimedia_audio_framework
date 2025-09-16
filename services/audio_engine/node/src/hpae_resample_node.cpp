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
#define LOG_TAG "HpaeResampleNode"
#endif

#include <iostream>
#include <algorithm>
#include <memory>
#include "hpae_resample_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_utils.h"
#include "audio_effect_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr int REASAMPLE_QUAILTY = 5;
static inline uint32_t Min(const uint32_t a, const uint32_t b) {return a > b ? b : a;}
HpaeResampleNode::HpaeResampleNode(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo, ResamplerType type)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate),
    resampleOutput_(pcmBufferInfo_), preNodeInfo_(preNodeInfo), tempOutput_(preNodeInfo.channels * nodeInfo.frameLen)
{
    if (type == ResamplerType::PRORESAMPLER) {
        resampler_ = std::make_unique<ProResampler>(preNodeInfo_.samplingRate, nodeInfo.samplingRate,
            preNodeInfo_.channels, REASAMPLE_QUAILTY);
    }
#ifdef ENABLE_HOOK_PCM
    inputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeResampleNodeInput1_id_" +
        std::to_string(GetSessionId()) + "_ch_" + std::to_string(preNodeInfo_.channels) +
        "_rate_" + std::to_string(preNodeInfo_.samplingRate) + "_scene_" +
        std::to_string(HpaeNode::GetSceneType()) + "_" + GetTime() + ".pcm");
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeResampleNodeOutput1_id_" +
        std::to_string(GetSessionId()) + "_ch_" + std::to_string(GetChannelCount()) +
        "_rate_" + std::to_string(GetSampleRate()) + "_scene_" +
        std::to_string(HpaeNode::GetSceneType())+ "_" + GetTime() + ".pcm");
#endif
}

HpaeResampleNode::HpaeResampleNode(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate),
    resampleOutput_(pcmBufferInfo_), preNodeInfo_(preNodeInfo), tempOutput_(preNodeInfo.channels * nodeInfo.frameLen)
{   // use ProResampler as default
    resampler_ = std::make_unique<ProResampler>(preNodeInfo_.samplingRate, nodeInfo.samplingRate,
        preNodeInfo_.channels, REASAMPLE_QUAILTY);

#ifdef ENABLE_HOOK_PCM
    inputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeResampleNodeInput1_id_" +
        std::to_string(HpaeNode::GetSessionId()) + "_ch_" + std::to_string(preNodeInfo_.channels) + "_rate_" +
        std::to_string(preNodeInfo_.samplingRate) + "_scene_" + std::to_string(HpaeNode::GetSceneType())+".pcm");

    outputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeResampleNodeOutput1_id_" +
        std::to_string(HpaeNode::GetSessionId()) + "_ch_" + std::to_string(HpaeNode::GetChannelCount()) +
        "_rate_" + std::to_string(HpaeNode::GetSampleRate()) +
        "_scene_"+ std::to_string(HpaeNode::GetSceneType())+".pcm");
#endif
    AUDIO_INFO_LOG("input rate %{public}u, output rate %{public}u", preNodeInfo_.samplingRate, nodeInfo.samplingRate);
    AUDIO_INFO_LOG("input SessionId %{public}u, output streamType %{public}u", HpaeNode::GetSessionId(),
        nodeInfo.streamType);
    AUDIO_INFO_LOG("input ch %{public}u, output ch %{public}u", preNodeInfo_.channels,  HpaeNode::GetChannelCount());
}

bool HpaeResampleNode::Reset()
{
    if (resampler_ == nullptr) {
        AUDIO_WARNING_LOG("resampler_ is nullptr, SessionId:%{public}d", GetSessionId());
        return false;
    }
    resampler_->Reset();
    return true;
}

HpaePcmBuffer *HpaeResampleNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    Trace trace("[" + std::to_string(GetSessionId()) + "]HpaeResampleNode::SignalProcess");
    if (inputs.empty()) {
        AUDIO_WARNING_LOG("inputs size is empty, SessionId:%{public}d", GetSessionId());
        return nullptr;
    }
    if (inputs.size() != 1) {
        AUDIO_WARNING_LOG("error inputs size is not eqaul to 1, SessionId:%{public}d", GetSessionId());
    }
    if (resampler_ == nullptr) {
        return &silenceData_;
    }
    resampleOutput_.Reset();
    uint32_t inputFrameLen = preNodeInfo_.frameLen;
    uint32_t outputFrameLen = GetFrameLen();
    float *srcData = (*(inputs[0])).GetPcmDataBuffer();
    float *dstData = tempOutput_.data();
    if (preNodeInfo_.channels == GetChannelCount()) {
        dstData = resampleOutput_.GetPcmDataBuffer();
    }
#ifdef ENABLE_HOOK_PCM
    if (inputPcmDumper_ != nullptr) {
        inputPcmDumper_->CheckAndReopenHandle();
        inputPcmDumper_->Dump((int8_t *)(srcData), (inputFrameLen * sizeof(float) * preNodeInfo_.channels));
    }
#endif
    ResampleProcess(srcData, inputFrameLen, dstData, outputFrameLen);
    return &resampleOutput_;
}

void HpaeResampleNode::ResampleProcess(float *srcData, uint32_t inputFrameLen, float *dstData, uint32_t outputFrameLen)
{
    resampler_->Process(srcData, inputFrameLen, dstData, outputFrameLen);

    if (preNodeInfo_.channels == GetChannelCount()) {
#ifdef ENABLE_HOOK_PCM
        if (outputPcmDumper_ != nullptr) {
            outputPcmDumper_->CheckAndReopenHandle();
            outputPcmDumper_->Dump(
                (int8_t *)(resampleOutput_.GetPcmDataBuffer()), GetFrameLen() * sizeof(float) * GetChannelCount());
        }
#endif
        return;
    }
    
    float *targetData = resampleOutput_.GetPcmDataBuffer();
    uint32_t targetChannels = GetChannelCount();
    for (uint32_t i = 0; i < outputFrameLen; ++i) {
        for (uint32_t ch = 0; ch < targetChannels; ++ch) {
            uint32_t leftChIndex = Min(ch, (preNodeInfo_.channels - 1));
            targetData[i * targetChannels + ch] =
                dstData[i * preNodeInfo_.channels + leftChIndex];
        }
    }

#ifdef ENABLE_HOOK_PCM
    if (outputPcmDumper_ != nullptr) {
        outputPcmDumper_->CheckAndReopenHandle();
        outputPcmDumper_->Dump(
            (int8_t *)(resampleOutput_.GetPcmDataBuffer()), GetFrameLen() * sizeof(float) * GetChannelCount());
    }
#endif
}

void HpaeResampleNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    resampleOutput_.SetSourceBufferType(preNode->GetOutputPortBufferType(nodeInfo));
}

void HpaeResampleNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS