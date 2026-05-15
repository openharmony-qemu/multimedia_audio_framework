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
#include "hpae_node_common.h"
#include "stream_dfx_manager.h"
#include "securec.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t REASAMPLE_QUAILTY = 1;
constexpr uint32_t CUSTOM_SAMPLE_RATE_MULTIPLES = 50;
constexpr uint32_t SAMPLE_RATE_11025 = 11025;
constexpr uint32_t FRAME_LEN_40MS = 40;
constexpr uint32_t FRAME_LEN_100MS = 100;
constexpr uint32_t UNPROCESSED_BUFFER_CAPACITY_MULTIPLIER = 2;
constexpr uint32_t PROCESSED_BUFFER_COMPACTION_DIVISOR = 2;
}
HpaeAudioFormatConverterNode::HpaeAudioFormatConverterNode(HpaeNodeInfo preNodeInfo, HpaeNodeInfo nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
    converterOutput_(pcmBufferInfo_), preNodeInfo_(preNodeInfo), tmpOutBuf_(pcmBufferInfo_)
{
    int32_t enableNewRoute = 0;
    GetSysPara("persist.multimedia.enable_new_route", enableNewRoute);
    frameLenInMs_ =
        enableNewRoute == 1 ? nodeInfo.frameLen * MILLISECOND_PER_SECOND / nodeInfo.samplingRate : FRAME_LEN_20MS;
    converterOutput_.SetSplitStreamType(preNodeInfo.GetSplitStreamType());
    // use ProResamppler as default
    resampler_ = std::make_unique<ProResampler>(preNodeInfo.customSampleRate == 0 ? preNodeInfo.samplingRate :
        preNodeInfo.customSampleRate, nodeInfo.samplingRate,
        std::min(preNodeInfo.channels, nodeInfo.channels), REASAMPLE_QUAILTY);

    UpdateTmpOutPcmBufferInfo(pcmBufferInfo_);
    
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
    InitDualBuffer();
    AUDIO_INFO_LOG("node id %{public}d, sessionid %{public}d, "
        "input: bitformat %{public}d, frameLen %{public}d, sample rate %{public}d, channels %{public}d,"
        "channelLayout %{public}" PRIu64 ", output: bitformat %{public}d, frameLen %{public}d, sample rate %{public}d,"
        "channels %{public}d, channelLayout %{public}" PRIu64 "", GetNodeId(), GetSessionId(),
        preNodeInfo.format, preNodeInfo.frameLen, preNodeInfo.customSampleRate == 0 ? preNodeInfo.samplingRate :
        preNodeInfo.customSampleRate, inChannelInfo.numChannels,
        inChannelInfo.channelLayout, nodeInfo.format, nodeInfo.frameLen, nodeInfo.samplingRate,
        outChannelInfo.numChannels, outChannelInfo.channelLayout);

#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeAudioFormatConverterNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
}

HpaeAudioFormatConverterNode::~HpaeAudioFormatConverterNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

void HpaeAudioFormatConverterNode::RegisterCallback(INodeFormatInfoCallback *callback)
{
    nodeFormatInfoCallback_ = callback;
}

void HpaeAudioFormatConverterNode::DoProcess()
{
    std::vector<HpaePcmBuffer *> &preOutputs = inputStream_.ReadPreOutputData();
    bool hasValidInput = !preOutputs.empty() && preOutputs[0] != nullptr && preOutputs[0]->IsValid();
    AUDIO_DEBUG_LOG("NodeId %{public}d: hasValidInput=%{public}d, enableProcess=%{public}d, "
        "processedFrames=%{public}u, outputFrameLen20ms=%{public}u, unprocessedFrames=%{public}u, "
        "credit=%{public}u/%{public}u, threshold=%{public}u",
        GetNodeId(), hasValidInput, enableProcess_, processedFrames_, outputFrameLen20ms_,
        unprocessedFrames_, pullAheadCredit_, pullAheadMaxCredit_, resamplerInputThreshold_);

    if (hasValidInput && enableProcess_) {
        HpaePcmBuffer *tempOut = SignalProcess(preOutputs);
        outputStream_.WriteDataToOutput(tempOut);
        return;
    }
    // No valid input or process disabled -- still try to drain processedBuffer_
    if (processedFrames_ >= outputFrameLen20ms_ && enableProcess_) {
        converterOutput_.Reset();
        if (ExtractOutputFromProcessedBuffer()) {
            AUDIO_DEBUG_LOG("NodeId %{public}d: drain processedBuffer_, "
                "remaining %{public}u frames", GetNodeId(), processedFrames_);
            outputStream_.WriteDataToOutput(&converterOutput_);
            return;
        }
    }
    AUDIO_DEBUG_LOG("NodeId %{public}d: output silence, "
        "hasValidInput=%{public}d, enableProcess=%{public}d, processedFrames=%{public}u",
        GetNodeId(), hasValidInput, enableProcess_, processedFrames_);
    outputStream_.WriteDataToOutput(&silenceData_);
}

void HpaeAudioFormatConverterNode::AccumulateInputAndPullAhead(HpaePcmBuffer *input)
{
    float *srcData = input->GetPcmDataBuffer();
    uint32_t inputFrameLen = input->GetFrameLen();
    uint32_t inputChannels = input->GetChannelCount();

    // Step 1: Append input data to unprocessedBuffer_
    AppendToUnprocessedBuffer(srcData, inputFrameLen, inputChannels);

    // Step 1b: Pull-Ahead -- if not enough data accumulated and credit available, actively pull from upstream
    AudioChannelInfo inChannelInfo = channelConverter_.GetInChannelInfo();
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    bool needsAccumulation = (resampler_->GetInRate() != resampler_->GetOutRate());
    AUDIO_DEBUG_LOG("NodeId %{public}d: inRate=%{public}u->outRate=%{public}u, "
        "inCh=%{public}u->outCh=%{public}u, inputFrameLen=%{public}u, unprocessedFrames=%{public}u, "
        "threshold=%{public}u, credit=%{public}u/%{public}u, needsAccumulation=%{public}d",
        GetNodeId(), resampler_->GetInRate(), resampler_->GetOutRate(),
        inChannelInfo.numChannels, outChannelInfo.numChannels, inputFrameLen,
        unprocessedFrames_, resamplerInputThreshold_, pullAheadCredit_, pullAheadMaxCredit_,
        needsAccumulation);
    if (resampler_ != nullptr && pullAheadCredit_ > 0 &&
        unprocessedFrames_ < resamplerInputThreshold_ && needsAccumulation) {
        PullAheadUntilThresholdOrFailure(inputFrameLen);
    }

    // If credit exhausted and no output available, pull one more uncredited frame
    // to prevent deadlock where neither pull nor output can make progress
    if ((pullAheadCredit_ == 0) && (unprocessedFrames_ < resamplerInputThreshold_) && (processedFrames_ == 0)) {
        TryPullOneFrame();
    }
}

HpaePcmBuffer* HpaeAudioFormatConverterNode::ProcessAndExtractOutput(HpaePcmBuffer *input)
{
    // Step 2: Conditionally resample -- drain unprocessedBuffer_ into processedBuffer_
    ProcessResampleLoop();

    // Step 3: Extract 20ms output from processedBuffer_ into converterOutput_
    converterOutput_.Reset();
    if (!ExtractOutputFromProcessedBuffer()) {
        AUDIO_DEBUG_LOG("NodeId %{public}d: insufficient output, "
            "processedFrames=%{public}u < outputFrameLen20ms=%{public}u, returning silence",
            GetNodeId(), processedFrames_, outputFrameLen20ms_);
        return &silenceData_;
    }

    AUDIO_DEBUG_LOG("NodeId %{public}d: output success, "
        "extracted %{public}u frames, remaining processedFrames=%{public}u",
        GetNodeId(), outputFrameLen20ms_, processedFrames_);

#ifdef ENABLE_HOOK_PCM
    if (outputPcmDumper_ != nullptr) {
        outputPcmDumper_->CheckAndReopenHandle();
        outputPcmDumper_->Dump((int8_t *)converterOutput_.GetPcmDataBuffer(),
            converterOutput_.GetFrameLen() * sizeof(float) * channelConverter_.GetOutChannelInfo().numChannels);
    }
#endif
    converterOutput_.SetBufferState(input->GetBufferState());
    return &converterOutput_;
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
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(resampler_, &silenceData_,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::GENERAL,
                BusinessScenario::QUERY, ERR_NULL_POINTER), "resampler_ is nullptr", false),
        "NodeId %{public}d resampler_ is nullptr", GetNodeId());

    // make sure size of silenceData_, tmpOutput_, and ConverterOutput_ is correct
    CheckAndUpdateInfo(inputs[0]);

#ifdef ENABLE_HOOK_PCM
    if (!outputPcmDumper_) {
        outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
            "HpaeConverterNodeOutput_id_" + std::to_string(GetSessionId()) + "_nodeId_" + std::to_string(GetNodeId()) +
            "_ch_" + std::to_string(GetChannelCount()) +
            "_rate_" + std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
    }
#endif

    AccumulateInputAndPullAhead(inputs[0]);
    return ProcessAndExtractOutput(inputs[0]);
}

bool HpaeAudioFormatConverterNode::UpdateOutChannelInfo(uint32_t numChannels, AudioChannelLayout channelLayout)
{
    AudioChannelInfo curOutChannelInfo = channelConverter_.GetOutChannelInfo();
    AudioChannelInfo newOutChannelInfo = {
        .channelLayout = (AudioChannelLayout)channelLayout,
        .numChannels = numChannels,
    };
    HILOG_COMM_INFO("[CheckUpdateOutInfo]NodeId %{public}d, update out channels and channelLayout: "
        "channels %{public}d -> %{public}d", GetNodeId(), curOutChannelInfo.numChannels, numChannels);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(channelConverter_.SetOutChannelInfo(newOutChannelInfo) == MIX_ERR_SUCCESS, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::GENERAL, BusinessScenario::SEND_DATA,
                ERR_ILLEGAL_STATE), "Fail to set output channel info", false),
        "NodeId: %{public}d, Fail to set output channel info from effectNode!", GetNodeId());
    uint32_t resampleChannels = std::min(channelConverter_.GetInChannelInfo().numChannels, numChannels);
    if (resampleChannels != resampler_->GetChannels()) {
        AUDIO_INFO_LOG("NodeId: %{public}d, Update resampler work channel from effectNode!", GetNodeId());
        resampler_->UpdateChannels(resampleChannels);
    }
    return true;
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
        if (!UpdateOutChannelInfo(numChannels, channelLayout)) {
            return false;
        }
    }
    // update sample rate
    uint32_t oldOutRate = resampler_->GetOutRate();
    if (oldOutRate != sampleRate) {
        HILOG_COMM_INFO("[CheckUpdateOutInfo]NodeId: %{public}d, update output sample rate: "
            "%{public}d -> %{public}d", GetNodeId(), oldOutRate, sampleRate);
        resampler_->UpdateRates(resampler_->GetInRate(), sampleRate);
    }

    HpaeNodeInfo nodeInfo = GetNodeInfo();
    nodeInfo.channels = (AudioChannel)numChannels;
    nodeInfo.channelLayout = (AudioChannelLayout)channelLayout;
    uint32_t newOutRate = resampler_->GetOutRate();
    nodeInfo.samplingRate = (AudioSamplingRate)newOutRate;
    // Proportionally update frameLen when output rate changes to preserve time duration
    if (oldOutRate != 0 && oldOutRate != newOutRate) {
        nodeInfo.frameLen = static_cast<uint32_t>(
            static_cast<uint64_t>(nodeInfo.frameLen) * newOutRate / oldOutRate);
    }
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
        HILOG_COMM_INFO("[CheckUpdateInInfo]NodeId %{public}d: Update innput channel info from pcmBufferInfo, "
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
        HILOG_COMM_INFO("[CheckUpdateInInfo]NodeId %{public}d: Update resampler input sample rate: "
            "%{public}d -> %{public}d", GetNodeId(), resampler_->GetInRate(), sampleRate);
        preNodeInfo_.samplingRate = (AudioSamplingRate)sampleRate;
        preNodeInfo_.frameLen = CalculateFrameLenBySampleRate(sampleRate);
        resampler_->UpdateRates(sampleRate, resampler_->GetOutRate());
        isInfoUpdated = true;
    }
    return isInfoUpdated;
}

void HpaeAudioFormatConverterNode::UpdateTmpOutPcmBufferInfo(const PcmBufferInfo &outPcmBufferInfo)
{
    if (outPcmBufferInfo.ch == preNodeInfo_.channels || outPcmBufferInfo.rate == resampler_->GetInRate()) {
        // do not need tmpOutput Buffer
        return;
    }
    PcmBufferInfo tmpOutPcmBufferInfo = outPcmBufferInfo;
    if (outPcmBufferInfo.ch < preNodeInfo_.channels) { // downmix, then resample
        tmpOutPcmBufferInfo.rate = resampler_->GetInRate();
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
    {
        uint32_t curInRate = resampler_->GetInRate();
        uint32_t curOutRate = resampler_->GetOutRate();
        outPcmBufferInfo.frameLen = (curInRate == curOutRate) ? GetFrameLen() :
            curOutRate * frameLenInMs_ / MILLISECOND_PER_SECOND;
    }
    outPcmBufferInfo.channelLayout = outChannelInfo.channelLayout;

    AUDIO_INFO_LOG("NodeId %{public}d: output or input format info is changed, update tmp PCM buffer info!",
        GetNodeId());
    UpdateTmpOutPcmBufferInfo(outPcmBufferInfo);
    InitDualBuffer();

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

void HpaeAudioFormatConverterNode::ReconfigTmpOutForChannelConversion(uint32_t inCh, uint32_t outCh,
    uint32_t maxOutFrames)
{
    PcmBufferInfo tmpOutPcmBufferInfo;
    if (inCh > outCh) {
        // Downmix first, then resample: tmpOutBuf_ holds downmixed input at inRate
        tmpOutPcmBufferInfo.ch = inCh;
        tmpOutPcmBufferInfo.rate = resampler_->GetInRate();
        tmpOutPcmBufferInfo.frameLen = resamplerInputThreshold_;
    } else {
        // Resample first, then upmix: tmpOutBuf_ holds resampled output at outRate
        tmpOutPcmBufferInfo.ch = inCh;
        tmpOutPcmBufferInfo.rate = resampler_->GetOutRate();
        tmpOutPcmBufferInfo.frameLen = maxOutFrames;
    }
    tmpOutPcmBufferInfo.channelLayout = static_cast<uint64_t>(channelConverter_.GetInChannelInfo().channelLayout);
    tmpOutBuf_.ReConfig(tmpOutPcmBufferInfo);
}

void HpaeAudioFormatConverterNode::InitDualBuffer()
{
    if (resampler_ == nullptr) {
        return;
    }
    CHECK_AND_RETURN_LOG(resampler_->GetInRate() != 0, "NodeId %{public}d: inRate is zero", GetNodeId());
    resamplerInputThreshold_ = CalculateResamplerInputThreshold();
    uint32_t outRate = resampler_->GetOutRate();
    uint32_t inRate = resampler_->GetInRate();
    // Same-rate path (e.g., low-latency 5ms): use node's actual output frameLen to match converterOutput_ buffer
    outputFrameLen20ms_ = (inRate == outRate) ? GetFrameLen() :
        outRate * frameLenInMs_ / MILLISECOND_PER_SECOND;

    AudioChannelInfo inChannelInfo = channelConverter_.GetInChannelInfo();
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    uint32_t inCh = inChannelInfo.numChannels;
    uint32_t outCh = outChannelInfo.numChannels;

    // unprocessedBuffer_ stores data at inCh per frame
    size_t unprocessedCapacity = static_cast<size_t>(resamplerInputThreshold_) *
        UNPROCESSED_BUFFER_CAPACITY_MULTIPLIER * inCh;
    unprocessedBuffer_.resize(unprocessedCapacity, 0.0f);
    unprocessedFrames_ = 0;

    // processedBuffer_ and resamplerOutBuf_ store data at outCh per frame
    uint32_t maxOutFrames = static_cast<uint32_t>(
        static_cast<uint64_t>(resamplerInputThreshold_) * resampler_->GetOutRate() / resampler_->GetInRate());
    size_t processedCapacity = static_cast<size_t>(maxOutFrames) * outCh;
    processedBuffer_.resize(processedCapacity, 0.0f);
    processedFrames_ = 0;
    processedReadPos_ = 0;

    size_t resamplerOutCapacity = static_cast<size_t>(maxOutFrames) * outCh;
    resamplerOutBuf_.resize(resamplerOutCapacity, 0.0f);

    // Initialize pull-ahead credit: max 20ms output chunks one threshold input produces
    if (outputFrameLen20ms_ > 0) {
        pullAheadMaxCredit_ = maxOutFrames / outputFrameLen20ms_;
    } else {
        pullAheadMaxCredit_ = 0;
    }
    pullAheadCredit_ = pullAheadMaxCredit_;

    // Resize tmpOutBuf_ for channel conversion paths
    if (inCh != outCh) {
        ReconfigTmpOutForChannelConversion(inCh, outCh, maxOutFrames);
    }

    AUDIO_INFO_LOG("NodeId %{public}d: InitDualBuffer threshold=%{public}u, outputFrameLen20ms=%{public}u, "
        "maxOutFrames=%{public}u, outCh=%{public}u, inCh=%{public}u", GetNodeId(), resamplerInputThreshold_,
        outputFrameLen20ms_, maxOutFrames, outCh, inCh);
}

uint32_t HpaeAudioFormatConverterNode::CalculateResamplerInputThreshold() const
{
    if (resampler_ == nullptr) {
        return 0;
    }
    uint32_t inRate = resampler_->GetInRate();
    uint32_t outRate = resampler_->GetOutRate();
    if (inRate == outRate) {
        return inRate * frameLenInMs_ / MILLISECOND_PER_SECOND;
    }
    // Find the minimum span that produces exact output at outRate.
    // This avoids rounding errors in the resampler output.
    // 11025Hz: 441 * 48000 / 11025 = 1920 (exact)
    // 8010Hz:  801 * 48000 / 8010 = 4800 (exact)
    if (inRate == SAMPLE_RATE_11025) {
        return inRate * FRAME_LEN_40MS / MILLISECOND_PER_SECOND; // 441
    } else if (inRate % CUSTOM_SAMPLE_RATE_MULTIPLES != 0) {
        return inRate * FRAME_LEN_100MS / MILLISECOND_PER_SECOND; // e.g., 801 for 8010Hz
    } else {
        return inRate * frameLenInMs_ / MILLISECOND_PER_SECOND; // standard 20ms
    }
}

void HpaeAudioFormatConverterNode::AppendToUnprocessedBuffer(const float *data, uint32_t frameLen,
    uint32_t channels)
{
    if (frameLen == 0 || data == nullptr) {
        return;
    }
    size_t samplesToAdd = static_cast<size_t>(frameLen) * channels;
    size_t requiredSize = static_cast<size_t>(unprocessedFrames_ + frameLen) * channels;
    if (unprocessedBuffer_.size() < requiredSize) {
        unprocessedBuffer_.resize(requiredSize, 0.0f);
    }
    errno_t ret = memcpy_s(unprocessedBuffer_.data() + static_cast<size_t>(unprocessedFrames_) * channels,
        (unprocessedBuffer_.size() - static_cast<size_t>(unprocessedFrames_) * channels) * sizeof(float),
        data, samplesToAdd * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: AppendToUnprocessedBuffer memcpy_s failed", GetNodeId());
        return;
    }
    unprocessedFrames_ += frameLen;
    if (pullAheadCredit_ > 0) {
        pullAheadCredit_--;
    }
    AUDIO_DEBUG_LOG("NodeId %{public}d: AppendToUnprocessedBuffer added %{public}u frames @%{public}u ch, "
        "total unprocessedFrames=%{public}u", GetNodeId(), frameLen, channels, unprocessedFrames_);
}

void HpaeAudioFormatConverterNode::AppendToProcessedBuffer(const float *data, uint32_t frameLen, uint32_t channels)
{
    size_t writeOffset = static_cast<size_t>(processedReadPos_ + processedFrames_) * channels;
    size_t samplesToAdd = static_cast<size_t>(frameLen) * channels;
    if (processedBuffer_.size() < writeOffset + samplesToAdd) {
        processedBuffer_.resize(writeOffset + samplesToAdd, 0.0f);
    }
    errno_t ret = memcpy_s(processedBuffer_.data() + writeOffset,
        (processedBuffer_.size() - writeOffset) * sizeof(float),
        data, samplesToAdd * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: AppendToProcessedBuffer memcpy_s failed", GetNodeId());
        return;
    }
    processedFrames_ += frameLen;
}

bool HpaeAudioFormatConverterNode::TryPullOneFrame()
{
    auto &newInputs = inputStream_.ReadPreOutputData();
    if (newInputs.empty() || newInputs[0] == nullptr || !newInputs[0]->IsValid()) {
        return false;
    }
    float *data = newInputs[0]->GetPcmDataBuffer();
    uint32_t frameLen = newInputs[0]->GetFrameLen();
    uint32_t channels = newInputs[0]->GetChannelCount();
    AppendToUnprocessedBuffer(data, frameLen, channels);
    return true;
}

void HpaeAudioFormatConverterNode::PullAheadUntilThresholdOrFailure(uint32_t inputFrameLen)
{
    if (inputFrameLen == 0 || pullAheadCredit_ == 0) {
        return;
    }

    uint32_t pullCount = 0;

    AUDIO_DEBUG_LOG("NodeId %{public}d: PullAhead start, unprocessedFrames=%{public}u, threshold=%{public}u, "
        "credit=%{public}u", GetNodeId(), unprocessedFrames_, resamplerInputThreshold_, pullAheadCredit_);

    while (pullAheadCredit_ > 0 && unprocessedFrames_ < resamplerInputThreshold_) {
        if (!TryPullOneFrame()) {
            AUDIO_DEBUG_LOG("NodeId %{public}d: PullAhead stopped, upstream underflow after %{public}u pulls",
                GetNodeId(), pullCount);
            break;
        }
        pullCount++;
    }

    if (pullCount > 0) {
        AUDIO_DEBUG_LOG("NodeId %{public}d: PullAhead pulled %{public}u frames, unprocessedFrames=%{public}u, "
            "credit=%{public}u", GetNodeId(), pullCount, unprocessedFrames_, pullAheadCredit_);
    }
}

void HpaeAudioFormatConverterNode::ProcessSameRateChannelConversion(uint32_t inCh, uint32_t outCh)
{
    while (unprocessedFrames_ >= resamplerInputThreshold_) {
        float *inPtr = unprocessedBuffer_.data();
        uint32_t inFrameLen = resamplerInputThreshold_;
        // Same rate => outFrameLen == inFrameLen
        uint32_t outFrameLen = inFrameLen;

        size_t outSamplesNeeded = static_cast<size_t>(outFrameLen) * outCh;
        if (resamplerOutBuf_.size() < outSamplesNeeded) {
            resamplerOutBuf_.resize(outSamplesNeeded, 0.0f);
        }

        int32_t ret = ResampleWithChannelConversion(
            inPtr, inFrameLen, resamplerOutBuf_.data(), outFrameLen,
            resampler_->GetChannels());
        if (ret != EOK) {
            AUDIO_ERR_LOG("NodeId %{public}d: Same-rate channel conversion "
                "failed, code %{public}d", GetNodeId(), ret);
            break;
        }

        AppendToProcessedBuffer(resamplerOutBuf_.data(), outFrameLen, outCh);
        // Remove at inCh per frame
        RemoveConsumedInputFrames(inFrameLen, inCh);
    }
}

void HpaeAudioFormatConverterNode::ProcessDiffRateResample(uint32_t inCh, uint32_t outCh,
    uint32_t inRate, uint32_t outRate)
{
    CHECK_AND_RETURN_LOG(inRate != 0, "NodeId %{public}d: inRate is zero, skip resample", GetNodeId());
    while (unprocessedFrames_ >= resamplerInputThreshold_) {
        float *inPtr = unprocessedBuffer_.data();
        uint32_t inFrameLen = resamplerInputThreshold_;
        uint32_t outFrameLen = static_cast<uint32_t>(
            static_cast<uint64_t>(inFrameLen) * outRate / inRate);

        AUDIO_DEBUG_LOG("NodeId %{public}d: inRate=%{public}u->outRate=%{public}u, "
            "inFrames=%{public}u, outFrames=%{public}u, inCh=%{public}u, outCh=%{public}u, "
            "resamplerChannels=%{public}u, unprocessedBefore=%{public}u",
            GetNodeId(), inRate, outRate, inFrameLen, outFrameLen, inCh, outCh,
            resampler_->GetChannels(), unprocessedFrames_);

        size_t outSamplesNeeded = static_cast<size_t>(outFrameLen) * outCh;
        if (resamplerOutBuf_.size() < outSamplesNeeded) {
            resamplerOutBuf_.resize(outSamplesNeeded, 0.0f);
        }

        int32_t ret = ResampleWithChannelConversion(
            inPtr, inFrameLen, resamplerOutBuf_.data(), outFrameLen,
            resampler_->GetChannels());
        if (ret != EOK) {
            AUDIO_ERR_LOG("NodeId %{public}d: Resample failed with code %{public}d",
                GetNodeId(), ret);
            break;
        }

        AppendToProcessedBuffer(resamplerOutBuf_.data(), outFrameLen, outCh);
        RemoveConsumedInputFrames(inFrameLen, inCh);
    }
}

void HpaeAudioFormatConverterNode::ProcessResampleLoop()
{
    if (resampler_ == nullptr) {
        return;
    }

    AudioChannelInfo inChannelInfo = channelConverter_.GetInChannelInfo();
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    uint32_t inCh = inChannelInfo.numChannels;
    uint32_t outCh = outChannelInfo.numChannels;
    uint32_t inRate = resampler_->GetInRate();
    uint32_t outRate = resampler_->GetOutRate();

    AUDIO_DEBUG_LOG("NodeId %{public}d: inRate=%{public}u, outRate=%{public}u, "
        "inCh=%{public}u, outCh=%{public}u, unprocessedFrames=%{public}u, threshold=%{public}u, "
        "processedFrames=%{public}u",
        GetNodeId(), inRate, outRate, inCh, outCh, unprocessedFrames_,
        resamplerInputThreshold_, processedFrames_);

    // Same-rate path: handle channel conversion
    if (inRate == outRate) {
        if (inCh == outCh) {
            // Same rate, same channels -- pure passthrough
            if (unprocessedFrames_ > 0) {
                AUDIO_DEBUG_LOG("NodeId %{public}d: passthrough "
                    "%{public}u frames", GetNodeId(), unprocessedFrames_);
            }
            MoveUnprocessedToProcessed(outCh);  // outCh == inCh here
        } else {
            // Same rate, different channels -- need channel conversion only
            ProcessSameRateChannelConversion(inCh, outCh);
        }
        return;
    }

    // Different rates -- resample (possibly with channel conversion)
    ProcessDiffRateResample(inCh, outCh, inRate, outRate);
}

int32_t HpaeAudioFormatConverterNode::ResampleWithChannelConversion(float *srcData, uint32_t inFrameLen,
    float *dstData, uint32_t outFrameLen, uint32_t resamplerChannels)
{
    AudioChannelInfo inChannelInfo = channelConverter_.GetInChannelInfo();
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    uint32_t inRate = resampler_->GetInRate();
    uint32_t outRate = resampler_->GetOutRate();
    int32_t ret = EOK;

    const char *pathDesc = "unknown";
    if (inChannelInfo.numChannels == outChannelInfo.numChannels && inRate == outRate) {
        // Same rate, same channels -- passthrough
        pathDesc = "passthrough";
        uint32_t copyBytes = inFrameLen * inChannelInfo.numChannels * sizeof(float);
        ret = memcpy_s(dstData, copyBytes, srcData, copyBytes);
    } else if (inChannelInfo.numChannels == outChannelInfo.numChannels) {
        // Resample only (same channels)
        pathDesc = "resample_only";
        ret = resampler_->Process(srcData, inFrameLen, dstData, outFrameLen);
    } else if (inRate == outRate) {
        // Channel conversion only (same rate)
        pathDesc = "channel_convert_only";
        ret = channelConverter_.Process(inFrameLen, srcData,
            inFrameLen * inChannelInfo.numChannels * sizeof(float),
            dstData, outFrameLen * outChannelInfo.numChannels * sizeof(float));
    } else if (inChannelInfo.numChannels > outChannelInfo.numChannels) {
        // Downmix first, then resample
        pathDesc = "downmix_then_resample";
        ret = channelConverter_.Process(inFrameLen, srcData,
            inFrameLen * inChannelInfo.numChannels * sizeof(float),
            tmpOutBuf_.GetPcmDataBuffer(), tmpOutBuf_.Size());
        ret += resampler_->Process(tmpOutBuf_.GetPcmDataBuffer(), inFrameLen, dstData, outFrameLen);
    } else {
        // Resample first, then upmix
        pathDesc = "resample_then_upmix";
        ret = resampler_->Process(srcData, inFrameLen, tmpOutBuf_.GetPcmDataBuffer(), outFrameLen);
        ret += channelConverter_.Process(outFrameLen, tmpOutBuf_.GetPcmDataBuffer(),
            tmpOutBuf_.Size(), dstData, outFrameLen * outChannelInfo.numChannels * sizeof(float));
    }
    AUDIO_DEBUG_LOG("NodeId %{public}d: path=%{public}s, "
        "inRate=%{public}u->outRate=%{public}u, inCh=%{public}u->outCh=%{public}u, "
        "inFrames=%{public}u, outFrames=%{public}u, ret=%{public}d",
        GetNodeId(), pathDesc, inRate, outRate, inChannelInfo.numChannels, outChannelInfo.numChannels,
        inFrameLen, outFrameLen, ret);
    return ret;
}

bool HpaeAudioFormatConverterNode::ExtractOutputFromProcessedBuffer()
{
    if (processedFrames_ < outputFrameLen20ms_) {
        AUDIO_DEBUG_LOG("NodeId %{public}d: ExtractOutput insufficient data, have=%{public}u, need=%{public}u",
            GetNodeId(), processedFrames_, outputFrameLen20ms_);
        return false;
    }
    uint32_t channels = converterOutput_.GetChannelCount();
    float *dstData = converterOutput_.GetPcmDataBuffer();
    uint32_t copySamples = outputFrameLen20ms_ * channels;
    errno_t ret = memcpy_s(dstData, converterOutput_.Size(),
        processedBuffer_.data() + static_cast<size_t>(processedReadPos_) * channels,
        copySamples * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: ExtractOutput memcpy_s failed", GetNodeId());
        return false;
    }
    processedReadPos_ += outputFrameLen20ms_;
    processedFrames_ -= outputFrameLen20ms_;
    if (processedFrames_ == 0) {
        processedReadPos_ = 0;
    } else if (processedReadPos_ > processedBuffer_.size() / (channels * PROCESSED_BUFFER_COMPACTION_DIVISOR)) {
        CompactProcessedBuffer(channels);
    }
    AUDIO_DEBUG_LOG("NodeId %{public}d: ExtractOutput success, remaining processedFrames=%{public}u",
        GetNodeId(), processedFrames_);
    // Return one credit: data has been consumed downstream
    if (pullAheadCredit_ < pullAheadMaxCredit_) {
        pullAheadCredit_++;
    }
    return true;
}

void HpaeAudioFormatConverterNode::RemoveConsumedInputFrames(uint32_t consumedFrames, uint32_t channels)
{
    uint32_t remainingFrames = unprocessedFrames_ - consumedFrames;
    if (remainingFrames == 0) {
        unprocessedFrames_ = 0;
        return;
    }
    size_t srcOffset = static_cast<size_t>(consumedFrames) * channels;
    size_t remainingSamples = static_cast<size_t>(remainingFrames) * channels;
    errno_t ret = memmove_s(unprocessedBuffer_.data(), unprocessedBuffer_.size() * sizeof(float),
        unprocessedBuffer_.data() + srcOffset, remainingSamples * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: RemoveConsumedInputFrames memmove_s failed", GetNodeId());
    }
    unprocessedFrames_ = remainingFrames;
}

void HpaeAudioFormatConverterNode::CompactProcessedBuffer(uint32_t channels)
{
    size_t remainingSamples = static_cast<size_t>(processedFrames_) * channels;
    size_t srcOffset = static_cast<size_t>(processedReadPos_) * channels;
    errno_t ret = memmove_s(processedBuffer_.data(), processedBuffer_.size() * sizeof(float),
        processedBuffer_.data() + srcOffset, remainingSamples * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: CompactProcessedBuffer memmove_s failed", GetNodeId());
    }
    processedReadPos_ = 0;
}

void HpaeAudioFormatConverterNode::MoveUnprocessedToProcessed(uint32_t channels)
{
    size_t samples = static_cast<size_t>(unprocessedFrames_) * channels;
    size_t writeOffset = static_cast<size_t>(processedReadPos_ + processedFrames_) * channels;
    if (processedBuffer_.size() < writeOffset + samples) {
        processedBuffer_.resize(writeOffset + samples, 0.0f);
    }
    errno_t ret = memcpy_s(processedBuffer_.data() + writeOffset,
        (processedBuffer_.size() - writeOffset) * sizeof(float),
        unprocessedBuffer_.data(), samples * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: MoveUnprocessedToProcessed memcpy_s failed", GetNodeId());
        return;
    }
    processedFrames_ += unprocessedFrames_;
    AUDIO_DEBUG_LOG("NodeId %{public}d: MoveUnprocessedToProcessed moved %{public}u frames, "
        "total processedFrames=%{public}u", GetNodeId(), unprocessedFrames_, processedFrames_);
    unprocessedFrames_ = 0;
}

void HpaeAudioFormatConverterNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    converterOutput_.SetSourceBufferType(nodeInfo.sourceBufferType);
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
    }
#endif
}
void HpaeAudioFormatConverterNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.DisConnect(preNode->GetOutputPort(nodeInfo, true));
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(false, preNode->GetSharedInstance()->GetNodeId(), GetNodeId());
    }
#endif
}

uint64_t HpaeAudioFormatConverterNode::GetLatency(uint32_t sessionId)
{
    if (resampler_ == nullptr) {
        return 0;
    }
    // Dual-buffer latency: unprocessed input at input rate + processed output at output rate
    uint32_t inRate = resampler_->GetInRate();
    uint32_t outRate = resampler_->GetOutRate();
    uint64_t latencyUs = 0;
    if (inRate != 0) {
        latencyUs += static_cast<uint64_t>(unprocessedFrames_) * AUDIO_US_PER_SECOND / inRate;
    }
    if (outRate != 0) {
        latencyUs += static_cast<uint64_t>(processedFrames_) * AUDIO_US_PER_SECOND / outRate;
    }
    return latencyUs;
}

void HpaeAudioFormatConverterNode::SetDownmixNormalization(bool normalizing)
{
    channelConverter_.SetDownmixNormalization(normalizing);
}

void HpaeAudioFormatConverterNode::FlushBuffers()
{
    AUDIO_INFO_LOG("NodeId %{public}d: FlushBuffers, discarding "
        "unprocessedFrames=%{public}u, processedFrames=%{public}u",
        GetNodeId(), unprocessedFrames_, processedFrames_);
    unprocessedFrames_ = 0;
    processedFrames_ = 0;
    processedReadPos_ = 0;
    pullAheadCredit_ = pullAheadMaxCredit_;
}
} // Hpae
} // AudioStandard
} // OHOS