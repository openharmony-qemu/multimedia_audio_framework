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
#define LOG_TAG "HpaeMixerNode"
#endif
#include <iostream>
#include "hpae_mixer_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_utils.h"
#include "cinttypes"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeMixerNode::HpaeMixerNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
    mixedOutput_(pcmBufferInfo_)
{
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ =  std::make_unique<HpaePcmDumper>("HpaeMixerNodeOut_ch_" +
        std::to_string(nodeInfo.channels) + "_scenType_" +
        std::to_string(GetSceneType()) + "_rate_" + std::to_string(GetSampleRate()) + ".pcm");
    AUDIO_INFO_LOG("HpaeMixerNode scene type is %{public}d", GetSceneType());
#endif
    mixedOutput_.SetSplitStreamType(nodeInfo.GetSplitStreamType());
}

bool HpaeMixerNode::Reset()
{
    return HpaePluginNode::Reset();
}

HpaePcmBuffer *HpaeMixerNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    Trace trace("HpaeMixerNode::SignalProcess");
    CHECK_AND_RETURN_RET_LOG(!inputs.empty(), nullptr, "inputs is empty");

    mixedOutput_.Reset();

    bool isPCMBufferInfoUpdated = false;
    if (pcmBufferInfo_.ch != inputs[0]->GetChannelCount()) {
        AUDIO_INFO_LOG("Update channel count: %{public}d -> %{public}d",
            pcmBufferInfo_.ch, inputs[0]->GetChannelCount());
        pcmBufferInfo_.ch = inputs[0]->GetChannelCount();
        isPCMBufferInfoUpdated = true;
    }
    if (pcmBufferInfo_.frameLen != inputs[0]->GetFrameLen()) {
        AUDIO_INFO_LOG("Update frame len %{public}d -> %{public}d",
            pcmBufferInfo_.frameLen, inputs[0]->GetFrameLen());
        pcmBufferInfo_.frameLen = inputs[0]->GetFrameLen();
        isPCMBufferInfoUpdated = true;
    }
    if (pcmBufferInfo_.rate != inputs[0]->GetSampleRate()) {
        AUDIO_INFO_LOG("Update sample rate %{public}d -> %{public}d",
            pcmBufferInfo_.rate, inputs[0]->GetSampleRate());
        pcmBufferInfo_.rate = inputs[0]->GetSampleRate();
        isPCMBufferInfoUpdated = true;
    }
    if (pcmBufferInfo_.channelLayout != inputs[0]->GetChannelLayout()) {
        AUDIO_INFO_LOG("Update channel layout %{public}" PRIu64 " -> %{public}" PRIu64 "",
            pcmBufferInfo_.channelLayout, inputs[0]->GetChannelLayout());
        pcmBufferInfo_.channelLayout = inputs[0]->GetChannelLayout();
        isPCMBufferInfoUpdated = true;
    }
    // if other bitwidth is supported, add check here

    if (isPCMBufferInfoUpdated) {
        mixedOutput_.ReConfig(pcmBufferInfo_);
    }

    for (auto input : inputs) {
        mixedOutput_ += *input;
    }
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_->CheckAndReopenHandlde();
    outputPcmDumper_->Dump((int8_t *)(mixedOutput_.GetPcmDataBuffer()),
        mixedOutput_.GetChannelCount() * sizeof(float) * mixedOutput_.GetFrameLen());
#endif
    return &mixedOutput_;
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS