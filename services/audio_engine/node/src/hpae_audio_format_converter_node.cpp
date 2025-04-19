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
#define LOG_TAG "HpaeAudioFormatConverterNode"
#endif
#include "hpae_audio_format_converter_node.h"
#include "audio_engine_log.h"
#include "audio_utils.h"
#include "cinttypes"

static constexpr uint32_t DEFAULT_EFFECT_RATE = 48000;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr int REASAMPLE_QUAILTY = 5;
HpaeAudioFormatConverterNode::HpaeAudioFormatConverterNode(HpaeNodeInfo preNodeInfo, HpaeNodeInfo nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
    converterOuput_(pcmBufferInfo_), preNodeInfo_(preNodeInfo), tmpOutBuf_(pcmBufferInfo_)
{
    UpdateTmpOutPcmBufferInfo(pcmBufferInfo_);
    // use ProResamppler as default
    resampler_ = std::make_unique<ProResampler>(preNodeInfo_.samplingRate, nodeInfo.samplingRate,
        std::min(preNodeInfo_.channels, nodeInfo.channels), REASAMPLE_QUAILTY);
    
    AudioChannelInfo inChannelInfo = {
        .channelLayout = preNodeInfo.channelLayout,
        .numChannels = preNodeInfo.channels,
    };
    AudioChannelInfo outChannelInfo = {
        .channelLayout = nodeInfo.channelLayout,
        .numChannels = nodeInfo.channels,
    };

    // for now, work at float32le by default
    channelConverter_.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);
    AUDIO_INFO_LOG("node id %{public}d, sessionid %{public}d, "
        "input: bitformat %{public}d, sample rate %{public}d, channels %{public}d,"
        "channelLayout %{public}" PRIu64 ", output: bitformat %{public}d, sample rate %{public}d,"
        "channels %{public}d, channelLayout %{public}" PRIu64 "", GetNodeId(), GetSessionId(),
        preNodeInfo.format, preNodeInfo.samplingRate, inChannelInfo.numChannels,
        inChannelInfo.channelLayout, nodeInfo.format, nodeInfo.samplingRate,
        outChannelInfo.numChannels, outChannelInfo.channelLayout);
#ifdef ENABLE_HOOK_PCM
    inputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeConverterNodeInput_id_" + std::to_string(GetSessionId()) +
                                                      "_ch_" + std::to_string(preNodeInfo_.channels) + "_rate_" +
                                                      std::to_string(preNodeInfo_.samplingRate) + "_" + GetTime() + ".pcm");
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>("HpaeConverterNodeOutput_id_" + std::to_string(GetSessionId()) +
                                                       "_ch_" + std::to_string(GetChannelCount()) + "_rate_" +
                                                       std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
#endif
}

void HpaeAudioFormatConverterNode::RegisterCallback(INodeFormatInfoCallback *callback)
{
    nodeFormatInfoCallback_ = callback;
}

HpaePcmBuffer *HpaeAudioFormatConverterNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    auto rate = "rate[" + std::to_string(GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(GetFrameLen()) + "]";
    Trace trace("[" + std::to_string(GetSessionId()) + "]HpaeAudioFormatConverterNode::SignalProcess "
     + rate + ch + len);
    if (inputs.empty()) {
        AUDIO_WARNING_LOG("HpaeConverterNode inputs size is empty, SessionId:%{public}d", GetSessionId());
        return nullptr;
    }
    if (inputs.size() != 1) {
        AUDIO_WARNING_LOG("error inputs size is not eqaul to 1, SessionId:%{public}d", GetSessionId());
    }
    float *srcData = (*(inputs[0])).GetPcmDataBuffer();
#ifdef ENABLE_HOOK_PCM
    if (inputPcmDumper_ != nullptr) {
        inputPcmDumper_->Dump((int8_t *)(srcData),
            inputs[0]->GetFrameLen() * inputs[0]->GetChannelCount() * sizeof(float));
    }
#endif
    converterOuput_.Reset();
    tmpOutBuf_.Reset();

    CheckAndUpdateInfo(inputs[0]);

    float *dstData = converterOuput_.GetPcmDataBuffer();
    float *tmpData = tmpOutBuf_.GetPcmDataBuffer();

    if (resampler_ == nullptr) {
        return &silenceData_;
    }
    int32_t ret = ConverterProcess(srcData, dstData, tmpData, inputs[0]);
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d, sessionId %{public}d, Format Converter fail to process!",
            GetNodeId(), GetSessionId());
        return &silenceData_;
    }

#ifdef ENABLE_HOOK_PCM
    if (outputPcmDumper_ != nullptr) {
        outputPcmDumper_->Dump((int8_t *)dstData,
            converterOuput_.GetFrameLen() * sizeof(float) * channelConverter_.GetOutChannelInfo().numChannels);
    }
#endif
    AUDIO_DEBUG_LOG("NodeId %{public}d, buffer valid %{public}d", GetSessionId(), converterOuput_.IsValid());
    return &converterOuput_;
}

int32_t HpaeAudioFormatConverterNode::ConverterProcess(float *srcData, float *dstData, float *tmpData,
    HpaePcmBuffer *input)
{
    AudioChannelInfo inChannelInfo = channelConverter_.GetInChannelInfo();
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    uint32_t inRate = resampler_->GetInRate();
    uint32_t outRate = resampler_->GetOutRate();
 
    uint32_t inputFrameLen = preNodeInfo_.frameLen;
    uint32_t outputFrameLen = converterOuput_.GetFrameLen();
    uint32_t inputFrameBytes = inputFrameLen * inChannelInfo.numChannels * sizeof(float);
    uint32_t outputFrameBytes = outputFrameLen * outChannelInfo.numChannels * sizeof(float);
    int32_t ret = EOK;

    if ((inChannelInfo.numChannels == outChannelInfo.numChannels) && (inRate == outRate)) {
        ret = memcpy_s(dstData, outputFrameBytes, srcData, inputFrameBytes);
    } else if (inChannelInfo.numChannels == outChannelInfo.numChannels) {
        ret = resampler_->Process(srcData, &inputFrameLen, dstData, &outputFrameLen);
    } else if (inRate == outRate) {
        ret = channelConverter_.Process(inputFrameLen, srcData, (*input).Size(), dstData, converterOuput_.Size());
    } else if (inChannelInfo.numChannels > outChannelInfo.numChannels) { // convert, then resample
        ret = channelConverter_.Process(inputFrameLen, srcData, (*input).Size(), tmpData, tmpOutBuf_.Size());
        ret += resampler_->Process(tmpData, &inputFrameLen, dstData, &outputFrameLen);
    } else { // output channels larger than input channels, resample, then convert
        ret = resampler_->Process(srcData, &inputFrameLen, tmpData, &outputFrameLen);
        ret += channelConverter_.Process(outputFrameLen, tmpData, tmpOutBuf_.Size(), dstData, converterOuput_.Size());
    }
    return ret;
}

// return true if output info is updated
bool HpaeAudioFormatConverterNode::CheckUpdateOutInfo()
{
    // update channelLayout and numChannels
    if (nodeFormatInfoCallback_ == nullptr) {
        return false;
    }
    
    uint32_t numChannels = 0;
    uint64_t channelLayout = CH_LAYOUT_UNKNOWN;
    // effectnode input is 48k by default now
    uint32_t sampleRate = DEFAULT_EFFECT_RATE;
    
    // if there exists an effect node, converter node output is effect node input
    // update channels and channelLayout
    
    nodeFormatInfoCallback_->GetEffectNodeInputChannelInfo(numChannels, channelLayout);
    
    if (numChannels == 0 ||  channelLayout == CH_LAYOUT_UNKNOWN) {
        // set to node info, which is device output info
        AUDIO_INFO_LOG("Fail to check format into from effect node");
        numChannels = GetChannelCount();
        channelLayout = (uint64_t)GetChannelLayout();
        sampleRate = GetSampleRate();
    }

    AudioChannelInfo curOutChannelInfo = channelConverter_.GetOutChannelInfo();
    if ((curOutChannelInfo.numChannels == numChannels) && (curOutChannelInfo.channelLayout == channelLayout) &&
        (sampleRate == resampler_->GetOutRate())) {
        return false;
    }
    // update channel info
    if (curOutChannelInfo.numChannels != numChannels || curOutChannelInfo.channelLayout != channelLayout) {
        AudioChannelInfo newOutChannelInfo = {
            .channelLayout = (AudioChannelLayout)channelLayout,
            .numChannels = numChannels,
        };
        AUDIO_INFO_LOG("NodeId %{public}d, update out channels and channelLayout: channels %{public}d -> %{public}d",
            GetNodeId(), curOutChannelInfo.numChannels, numChannels);
        CHECK_AND_RETURN_RET_LOG(channelConverter_.SetOutChannelInfo(newOutChannelInfo) == DMIX_ERR_SUCCESS, false,
            "NodeId: %{public}d, Fail to set output channel info from effectNode!", GetNodeId());
 
        uint32_t resampleChannels = std::min(channelConverter_.GetInChannelInfo().numChannels, numChannels);
        if (resampleChannels != resampler_->GetChannels()) {
            AUDIO_INFO_LOG("NodeId: %{public}d, Update resampler work channel from effectNode!", GetNodeId());
            resampler_->UpdateChannels(resampleChannels);
        }
    }
    // update sample rate
    if (resampler_->GetOutRate() != sampleRate) {
        AUDIO_INFO_LOG("NodeId: %{public}d, update output sample rate: %{public}d -> %{public}d",
            GetNodeId(), resampler_->GetOutRate(), sampleRate);
        resampler_->UpdateRates(preNodeInfo_.samplingRate, sampleRate);
    }

    HpaeNodeInfo nodeInfo = GetNodeInfo();
    nodeInfo.channels = (AudioChannel)numChannels;
    nodeInfo.channelLayout = (AudioChannelLayout)channelLayout;
    nodeInfo.samplingRate = (AudioSamplingRate)resampler_->GetOutRate();
    SetNodeInfo(nodeInfo);
    return true;
}

// update channel info from processCluster. For now sample rate will not change
bool HpaeAudioFormatConverterNode::CheckUpdateInInfo(HpaePcmBuffer *input)
{
    uint32_t numChannels = input->GetChannelCount();
    uint64_t channelLayout = input->GetChannelLayout();
    uint32_t sampleRate = input->GetSampleRate();
    AudioChannelInfo curInChannelInfo = channelConverter_.GetInChannelInfo();
    bool isInfoUpdated = false;
    // update channels and channelLayout
    if ((curInChannelInfo.numChannels != numChannels) || (curInChannelInfo.channelLayout != channelLayout)) {
        AUDIO_INFO_LOG("NodeId %{public}d: Update innput channel info from pcmBufferInfo, "
            "channels: %{public}d -> %{public}d, channellayout: %{public}" PRIu64 " -> %{public}" PRIu64 ".",
            GetNodeId(), curInChannelInfo.numChannels, numChannels, curInChannelInfo.channelLayout, channelLayout);
 
        AudioChannelInfo newInChannelInfo = {
            .channelLayout = (AudioChannelLayout)channelLayout,
            .numChannels = numChannels,
        };
        channelConverter_.SetInChannelInfo(newInChannelInfo);
        preNodeInfo_.channelLayout = (AudioChannelLayout)channelLayout;
        preNodeInfo_.channels = (AudioChannel)numChannels;

        uint32_t resampleChannels = std::min(numChannels, channelConverter_.GetOutChannelInfo().numChannels);
        if (resampleChannels != resampler_->GetChannels()) {
            AUDIO_INFO_LOG("NodeId %{public}d: Update resampler work channel from effectNode!", GetNodeId());
            resampler_->UpdateChannels(resampleChannels);
        }
        isInfoUpdated = true;
    }
    // update sample rate
    if (sampleRate != resampler_->GetInRate()) {
        AUDIO_INFO_LOG("NodeId %{public}d: Update resampler input sample rate: %{public}d -> %{public}d",
            GetNodeId(), resampler_->GetInRate(), sampleRate);
        preNodeInfo_.frameLen = input->GetFrameLen();
        preNodeInfo_.samplingRate = (AudioSamplingRate)sampleRate;
        resampler_->UpdateRates(sampleRate, resampler_->GetOutRate());
        isInfoUpdated = true;
    }
    return isInfoUpdated;
}

void HpaeAudioFormatConverterNode::UpdateTmpOutPcmBufferInfo(const PcmBufferInfo &outPcmBufferInfo)
{
    if (outPcmBufferInfo.ch == preNodeInfo_.channels || outPcmBufferInfo.rate == preNodeInfo_.samplingRate) {
        // do not need tmpOutput Buffer
        return;
    }
    PcmBufferInfo tmpOutPcmBufferInfo = outPcmBufferInfo;
    if (outPcmBufferInfo.ch < preNodeInfo_.channels) { // downmix, then resample
        tmpOutPcmBufferInfo.rate = preNodeInfo_.samplingRate;
        tmpOutPcmBufferInfo.frameLen = preNodeInfo_.frameLen;
    } else { // resample, then upmix
        tmpOutPcmBufferInfo.ch = preNodeInfo_.channels;
    }
    AUDIO_INFO_LOG("NodeId: %{public}d, updated tmp buffer rate %{public}d, frameLen %{public}d, channels %{public}d",
        GetNodeId(), tmpOutPcmBufferInfo.rate, tmpOutPcmBufferInfo.frameLen, tmpOutPcmBufferInfo.ch);
    tmpOutBuf_.ReConfig(tmpOutPcmBufferInfo);
}


void HpaeAudioFormatConverterNode::CheckAndUpdateInfo(HpaePcmBuffer *input)
{
    bool isInfoUpdated = CheckUpdateInInfo(input);
    bool isOutInfoUpdated = CheckUpdateOutInfo();
    if ((!isInfoUpdated) && (!isOutInfoUpdated)) {
        return;
    }

    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    PcmBufferInfo outPcmBufferInfo = pcmBufferInfo_; // isMultiFrames_ and frame_ are inheritated from sinkInputNode
    outPcmBufferInfo.ch = outChannelInfo.numChannels;
    outPcmBufferInfo.rate = resampler_->GetOutRate();
    outPcmBufferInfo.frameLen = preNodeInfo_.frameLen * resampler_->GetOutRate() / resampler_->GetInRate();
    outPcmBufferInfo.channelLayout = outChannelInfo.channelLayout;

    AUDIO_INFO_LOG("NodeId %{public}d: output or input format info is changed, update tmp PCM buffer info!",
        GetNodeId());
    UpdateTmpOutPcmBufferInfo(outPcmBufferInfo);

    if (isOutInfoUpdated) {
        AUDIO_INFO_LOG("NodeId %{public}d: output format info is changed, update output PCM buffer info!", GetNodeId());
        converterOuput_.ReConfig(outPcmBufferInfo);
        silenceData_.ReConfig(outPcmBufferInfo);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = GetNodeStatusCallback().lock()) {
        callBack->OnNotifyDfxNodeInfoChanged(GetNodeId(), GetNodeInfo());
    }
#endif
    }
}

void HpaeAudioFormatConverterNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    converterOuput_.SetSourceBufferType(nodeInfo.sourceBufferType);
}
void HpaeAudioFormatConverterNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
}

} // Hpae
} // AudioStandard
} // OHOS