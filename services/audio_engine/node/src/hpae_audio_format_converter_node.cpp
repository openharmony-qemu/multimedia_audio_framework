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
#include "audio_utils.h"
#include <cinttypes>
#include "audio_effect_log.h"

static constexpr uint32_t FRAME_LEN_20MS = 20;
static constexpr uint32_t MS_IN_SECOND = 1000;
static constexpr uint32_t REASAMPLE_QUAILTY = 1;
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeAudioFormatConverterNode::HpaeAudioFormatConverterNode(HpaeNodeInfo preNodeInfo, HpaeNodeInfo nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
    converterOutput_(pcmBufferInfo_), preNodeInfo_(preNodeInfo), tmpOutBuf_(pcmBufferInfo_)
{
    converterOutput_.SetSplitStreamType(preNodeInfo.GetSplitStreamType());
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
        "input: bitformat %{public}d, frameLen %{public}d, sample rate %{public}d, channels %{public}d,"
        "channelLayout %{public}" PRIu64 ", output: bitformat %{public}d, frameLen %{public}d, sample rate %{public}d,"
        "channels %{public}d, channelLayout %{public}" PRIu64 "", GetNodeId(), GetSessionId(),
        preNodeInfo.format, preNodeInfo.frameLen, preNodeInfo.samplingRate, inChannelInfo.numChannels,
        inChannelInfo.channelLayout, nodeInfo.format, nodeInfo.frameLen, nodeInfo.samplingRate,
        outChannelInfo.numChannels, outChannelInfo.channelLayout);
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeConverterNodeOutput_id_" + std::to_string(GetSessionId()) +
        + "_nodeId_" + std::to_string(GetNodeId()) +
        "_ch_" + std::to_string(GetChannelCount()) + "_rate_" +
        std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
#endif

#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeAudioFormatConverterNode");
#endif
}

HpaeAudioFormatConverterNode::~HpaeAudioFormatConverterNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
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
    if (inputs.empty() || inputs[0] == nullptr) {
        AUDIO_WARNING_LOG("HpaeConverterNode inputs size is empty, SessionId:%{public}d", GetSessionId());
        return &silenceData_;
    }
    if (inputs.size() != 1) {
        AUDIO_WARNING_LOG("error inputs size is not eqaul to 1, SessionId:%{public}d", GetSessionId());
    }
    CHECK_AND_RETURN_RET_LOG(resampler_, &silenceData_, "NodeId %{public}d resampler_ is nullptr", GetNodeId());

    // make sure size of silenceData_, tmpOutput_, and ConverterOutput_ is correct
    CheckAndUpdateInfo(inputs[0]);
    // pass valid tag to next node
    if (!inputs[0]->IsValid()) {
        return &silenceData_;
    }
    float *srcData = (*(inputs[0])).GetPcmDataBuffer();
    converterOutput_.Reset();
    tmpOutBuf_.Reset();

    float *dstData = converterOutput_.GetPcmDataBuffer();
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
        outputPcmDumper_->CheckAndReopenHandle();
        outputPcmDumper_->Dump((int8_t *)dstData,
            converterOutput_.GetFrameLen() * sizeof(float) * channelConverter_.GetOutChannelInfo().numChannels);
    }
#endif
    converterOutput_.SetBufferState(inputs[0]->GetBufferState());
    return &converterOutput_;
}

int32_t HpaeAudioFormatConverterNode::ConverterProcess(float *srcData, float *dstData, float *tmpData,
    HpaePcmBuffer *input)
{
    AudioChannelInfo inChannelInfo = channelConverter_.GetInChannelInfo();
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    uint32_t inRate = resampler_->GetInRate();
    uint32_t outRate = resampler_->GetOutRate();
 
    uint32_t inputFrameLen = preNodeInfo_.frameLen;
    uint32_t outputFrameLen = converterOutput_.GetFrameLen();
    uint32_t inputFrameBytes = inputFrameLen * inChannelInfo.numChannels * sizeof(float);
    uint32_t outputFrameBytes = outputFrameLen * outChannelInfo.numChannels * sizeof(float);
    int32_t ret = EOK;

    if ((inChannelInfo.numChannels == outChannelInfo.numChannels) && (inRate == outRate)) {
        ret = memcpy_s(dstData, outputFrameBytes, srcData, inputFrameBytes);
    } else if (inChannelInfo.numChannels == outChannelInfo.numChannels) {
        ret = resampler_->Process(srcData, inputFrameLen, dstData, outputFrameLen);
    } else if (inRate == outRate) {
        // for 11025, input output rate cannot be the same now
        ret = channelConverter_.Process(inputFrameLen, srcData, (*input).Size(), dstData, converterOutput_.Size());
    } else if (inChannelInfo.numChannels > outChannelInfo.numChannels) { // convert, then resample
        ret = channelConverter_.Process(inputFrameLen, srcData, (*input).Size(), tmpData, tmpOutBuf_.Size());
        ret += resampler_->Process(tmpData, inputFrameLen, dstData, outputFrameLen);
    } else { // output channels larger than input channels, resample, then convert
        ret = resampler_->Process(srcData, inputFrameLen, tmpData, outputFrameLen);
        ret += channelConverter_.Process(outputFrameLen, tmpData, tmpOutBuf_.Size(), dstData, converterOutput_.Size());
    }
    return ret;
}

// return true if output info is updated
bool HpaeAudioFormatConverterNode::CheckUpdateOutInfo()
{
    if (nodeFormatInfoCallback_ == nullptr) {
        return false;
    }
    
    AudioBasicFormat basicFormat;
    basicFormat.rate = preNodeInfo_.samplingRate;
    
    // if there exists an effect node, converter node output is common input of loudness node and effectnode
    // Must check loudness node input before effectnode
    nodeFormatInfoCallback_->GetNodeInputFormatInfo(preNodeInfo_.sessionId, basicFormat);

    uint32_t numChannels = basicFormat.audioChannelInfo.numChannels;
    AudioChannelLayout channelLayout = basicFormat.audioChannelInfo.channelLayout;
    AudioSamplingRate sampleRate = basicFormat.rate;
    if (numChannels == 0) {
        // set to node info, which is device output info
        AUDIO_WARNING_LOG("Fail to check format info from down stream nodes");
        numChannels = GetChannelCount();
        channelLayout = GetChannelLayout();
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
    // special case for 11025, frameLen is 441, 0, 441, 0... alternating
    // do not influence isInfoUpdated flag, which is used for update tmp data length
    if (preNodeInfo_.samplingRate == SAMPLE_RATE_11025) {
        preNodeInfo_.frameLen = input->GetFrameLen();
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

    if (preNodeInfo_.samplingRate == SAMPLE_RATE_11025) {
        // for 11025, fix out frameLen based on output sample rate and fixed frameLen 20ms
        outPcmBufferInfo.frameLen = resampler_->GetOutRate() * FRAME_LEN_20MS / MS_IN_SECOND;
    }

    AUDIO_INFO_LOG("NodeId %{public}d: output or input format info is changed, update tmp PCM buffer info!",
        GetNodeId());
    UpdateTmpOutPcmBufferInfo(outPcmBufferInfo);

    if (isOutInfoUpdated) {
        AUDIO_INFO_LOG("NodeId %{public}d: output format info is changed, update output PCM buffer info!", GetNodeId());
        converterOutput_.ReConfig(outPcmBufferInfo);
        silenceData_.ReConfig(outPcmBufferInfo);
        // reconfig need reset valid
        silenceData_.SetBufferValid(false);
        silenceData_.SetBufferSilence(true);
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfoChanged(GetNodeId(), GetNodeInfo());
        }
#endif
// update PCM dumper
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeConverterNodeOutput_id_" + std::to_string(GetSessionId()) +
        + "_nodeId_" + std::to_string(GetNodeId()) +
        "_ch_" + std::to_string(GetChannelCount()) + "_rate_" +
        std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
#endif
    }
}

void HpaeAudioFormatConverterNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    converterOutput_.SetSourceBufferType(nodeInfo.sourceBufferType);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, preNode->GetSharedInstance()->GetNodeId(), GetNodeInfo());
    }
#endif
}
void HpaeAudioFormatConverterNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(false, GetNodeId(), GetNodeInfo());
    }
#endif
}
} // Hpae
} // AudioStandard
} // OHOS