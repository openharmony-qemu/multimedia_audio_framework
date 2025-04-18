/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */
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
#include "audio_engine_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr int REASAMPLE_QUAILTY = 5;
HpaeResampleNode::HpaeResampleNode(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo, ResamplerType type)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate),
    resampleOuput_(pcmBufferInfo_), preNodeInfo_(preNodeInfo), tempOuput_(preNodeInfo.channels * nodeInfo.frameLen)
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
    resampleOuput_(pcmBufferInfo_), preNodeInfo_(preNodeInfo), tempOuput_(preNodeInfo.channels * nodeInfo.frameLen)
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
        AUDIO_WARNING_LOG("HpaeResampleNode inputs size is empty, SessionId:%{public}d", GetSessionId());
        return nullptr;
    }
    if (inputs.size() != 1) {
        AUDIO_WARNING_LOG("error inputs size is not eqaul to 1, SessionId:%{public}d", GetSessionId());
    }
    if (resampler_ == nullptr) {
        return &silenceData_;
    }
    resampleOuput_.Reset();
    uint32_t inputFrameLen = preNodeInfo_.frameLen;
    uint32_t outputFrameLen = GetFrameLen();
    float *srcData = (*(inputs[0])).GetPcmDataBuffer();
    float *dstData = tempOuput_.data();
    if (preNodeInfo_.channels == GetChannelCount()) {
        dstData = resampleOuput_.GetPcmDataBuffer();
    }
#ifdef ENABLE_HOOK_PCM
    inputPcmDumper_->CheckAndReopenHandlde();
    if (inputPcmDumper_ != nullptr) {
        inputPcmDumper_->Dump((int8_t *)(srcData), (inputFrameLen * sizeof(float) * preNodeInfo_.channels));
    }
#endif
    ResampleProcess(srcData, inputFrameLen, dstData, outputFrameLen);
    return &resampleOuput_;
}

void HpaeResampleNode::ResampleProcess(float *srcData, uint32_t inputFrameLen, float *dstData, uint32_t outputFrameLen)
{
    resampler_->Process(srcData, &inputFrameLen, dstData, &outputFrameLen);
    int32_t addZeroLen = GetFrameLen() - outputFrameLen > 0 ? GetFrameLen() - outputFrameLen : 0;

    if (preNodeInfo_.channels == GetChannelCount()) {
#ifdef ENABLE_HOOK_PCM
        outputPcmDumper_->CheckAndReopenHandlde();
        if (outputPcmDumper_ != nullptr) {
            outputPcmDumper_->Dump(
                (int8_t *)(resampleOuput_.GetPcmDataBuffer()), GetFrameLen() * sizeof(float) * GetChannelCount());
        }
#endif
        return;
    }
    
    float *targetData = resampleOuput_.GetPcmDataBuffer();
    size_t targetChannels = GetChannelCount();
    for (int32_t i = 0; i < (int32_t)outputFrameLen; ++i) {
        for (int32_t ch = 0; ch < (int32_t)targetChannels; ++ch) {
            size_t leftChIndex = std::min(ch, (preNodeInfo_.channels - 1));
            if (i < addZeroLen) {
                targetData[i * targetChannels + ch] = 0;
            } else {
                targetData[i * targetChannels + ch] =
                    dstData[(i - addZeroLen) * preNodeInfo_.channels + leftChIndex];
            }
        }
    }

#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_->CheckAndReopenHandlde();
    if (outputPcmDumper_ != nullptr) {
        outputPcmDumper_->Dump(
            (int8_t *)(resampleOuput_.GetPcmDataBuffer()), GetFrameLen() * sizeof(float) * GetChannelCount());
    }
#endif
}

void HpaeResampleNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    resampleOuput_.SetSourceBufferType(preNode->GetOutputPortBufferType(nodeInfo));
}

void HpaeResampleNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS