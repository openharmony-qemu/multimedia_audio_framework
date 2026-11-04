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
constexpr uint32_t PROCESSED_BUFFER_CAPACITY_MULTIPLIER = 2;
constexpr uint32_t PROCESSED_BUFFER_COMPACTION_DIVISOR = 2;

// zero-capacity info for lazy buffers; frames=0 keeps InitPcmProcess allocation empty
// non-const ref: HpaePcmBuffer ctor takes PcmBufferInfo& (never modified through this handle)
PcmBufferInfo &ZeroPcmBufferInfo()
{
    static PcmBufferInfo zeroInfo(0, 0, 0, 0, 0);
    return zeroInfo;
}
}
HpaeAudioFormatConverterNode::HpaeAudioFormatConverterNode(HpaeNodeInfo preNodeInfo, HpaeNodeInfo nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    pcmBufferInfo_(nodeInfo.channels, nodeInfo.frameLen, nodeInfo.samplingRate, nodeInfo.channelLayout),
    converterOutput_(ZeroPcmBufferInfo()), preNodeInfo_(preNodeInfo), tmpOutBuf_(ZeroPcmBufferInfo())
{
    frameLenInMs_ = static_cast<float>(nodeInfo.frameLen * MILLISECOND_PER_SECOND) /
        static_cast<float>(nodeInfo.samplingRate);
    inRate_ = preNodeInfo.customSampleRate == 0 ? static_cast<uint32_t>(preNodeInfo.samplingRate) :
        static_cast<uint32_t>(preNodeInfo.customSampleRate);
    outRate_ = static_cast<uint32_t>(nodeInfo.samplingRate);
    inputFrameLen_ = static_cast<uint32_t>(preNodeInfo.frameLen);
    converterOutput_.SetSplitStreamType(preNodeInfo.GetSplitStreamType());

    uint32_t inCh = static_cast<uint32_t>(preNodeInfo.channels);
    uint32_t outCh = static_cast<uint32_t>(nodeInfo.channels);
    AudioChannelInfo inChannelInfo = {
        .channelLayout = preNodeInfo.channelLayout,
        .numChannels = inCh,
    };
    AudioChannelInfo outChannelInfo = {
        .channelLayout = nodeInfo.channelLayout,
        .numChannels = outCh,
    };

    // for now, work at float32le by default
    channelConverter_.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, true);
    // lazy resources: decide the initial mode, then acquire only what that mode needs
    processMode_ = ComputeProcessMode();
    ApplyModeResources();

    AUDIO_INFO_LOG("node id %{public}d, sessionid %{public}d, "
        "input: bitformat %{public}d, frameLen %{public}d, sample rate %{public}d, channels %{public}d,"
        "channelLayout %{public}" PRIu64 ", output: bitformat %{public}d, frameLen %{public}d, sample rate %{public}d,"
        "channels %{public}d, channelLayout %{public}" PRIu64 ", mode %{public}d", GetNodeId(), GetSessionId(),
        preNodeInfo.format, preNodeInfo.frameLen, preNodeInfo.customSampleRate == 0 ? preNodeInfo.samplingRate :
        preNodeInfo.customSampleRate, inChannelInfo.numChannels,
        inChannelInfo.channelLayout, nodeInfo.format, nodeInfo.frameLen, nodeInfo.samplingRate,
        outChannelInfo.numChannels, outChannelInfo.channelLayout, static_cast<int32_t>(processMode_));

#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeAudioFormatConverterNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
    RebuildTraceName();
}

void HpaeAudioFormatConverterNode::OnTraceInfoChanged()
{
    HpaePluginNode::OnTraceInfoChanged();
    RebuildTraceName();
}

void HpaeAudioFormatConverterNode::RebuildTraceName()
{
    auto rate = "rate[" + std::to_string(GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(GetFrameLen()) + "]";
    processTraceName_ = "[" + std::to_string(GetSessionId()) + "]HpaeAudioFormatConverterNode::DoProcess "
        + rate + ch + len;
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
    Trace trace(processTraceName_);
    std::vector<HpaePcmBuffer *> &preOutputs = inputStream_.ReadPreOutputData();
    bool hasPreOutput = !preOutputs.empty() && preOutputs[0] != nullptr;
    bool hasValidInput = hasPreOutput && preOutputs[0]->IsValid();
    AUDIO_DEBUG_LOG("NodeId %{public}d: hasValidInput=%{public}d, enableProcess=%{public}d, mode=%{public}d, "
        "processedFrames=%{public}u, unprocessedFrames=%{public}u, credit=%{public}u/%{public}u",
        GetNodeId(), hasValidInput, enableProcess_, static_cast<int32_t>(processMode_), processedFrames_,
        unprocessedFrames_, pullAheadCredit_, pullAheadMaxCredit_);
    if (hasPreOutput) {
        // make sure size of silenceData_, tmpOutput_, and ConverterOutput_ is correct
        CheckAndUpdateInfo(preOutputs[0]);
    }
    if (NeedsResampler() && resampler_ == nullptr) {
        ReportConverterFault("resampler_ is nullptr");
        return;
    }
    if (hasValidInput && enableProcess_) {
        HpaePcmBuffer *tempOut = ProcessByMode(preOutputs);
        outputStream_.WriteDataToOutput(tempOut);
        return;
    }
    if (processMode_ == ConverterProcessMode::ACCUMULATE &&
        DrainProcessedBufferToOutput(preOutputs, hasPreOutput)) {
        return;
    }
    AUDIO_DEBUG_LOG("NodeId %{public}d: output silence, "
        "hasValidInput=%{public}d, enableProcess=%{public}d, processedFrames=%{public}u",
        GetNodeId(), hasValidInput, enableProcess_, processedFrames_);
    outputStream_.WriteDataToOutput(PrepareSilenceBuffer(silenceData_));
}

// inputs is guaranteed non-empty with a valid first element by the DoProcess guards
HpaePcmBuffer *HpaeAudioFormatConverterNode::ProcessByMode(const std::vector<HpaePcmBuffer *> &inputs)
{
    if (inputs.size() != 1) {
        AUDIO_WARNING_LOG("error inputs size is not eqaul to 1, SessionId:%{public}d", GetSessionId());
    }
    HpaePcmBuffer *output = nullptr;
    switch (processMode_) {
        case ConverterProcessMode::PASSTHROUGH:
            output = ProcessPassthrough(inputs[0]);
            break;
        case ConverterProcessMode::DIRECT_PROCESS:
            output = ProcessDirectFrame(inputs[0]);
            break;
        default:
            output = SignalProcess(inputs);
            break;
    }
    DumpConverterOutput(output);
    return output;
}

// ENABLE_HOOK_PCM: unified output dump shared by passthrough/direct/accumulate paths
void HpaeAudioFormatConverterNode::DumpConverterOutput(HpaePcmBuffer *output)
{
#ifdef ENABLE_HOOK_PCM
    if (output == nullptr) {
        return;
    }
    if (outputPcmDumper_ == nullptr) {
        outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
            "HpaeConverterNodeOutput_id_" + std::to_string(GetSessionId()) +
            "_nodeId_" + std::to_string(GetNodeId()) + "_ch_" + std::to_string(GetChannelCount()) +
            "_rate_" + std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
    }
    outputPcmDumper_->CheckAndReopenHandle();
    outputPcmDumper_->Dump((int8_t *)output->GetPcmDataBuffer(),
        output->GetFrameLen() * sizeof(float) * output->GetChannelCount());
#else
    (void)output;
#endif
}

HpaePcmBuffer *HpaeAudioFormatConverterNode::ProcessPassthrough(HpaePcmBuffer *input)
{
    // CONTRACT: passthrough forwards the upstream carrier itself, so downstream consumers
    // must NOT write the received buffer in place (same rule as session effect node forwarding)
    if (input->GetFrameLen() == 0) {
        return PrepareSilenceBuffer(silenceData_);  // degenerate upstream frame
    }
    // unconditional store keeps tag semantics identical to converterOutput_-based output
    input->SetSourceBufferType(configuredSourceBufferType_);
    return input;
}

HpaePcmBuffer *HpaeAudioFormatConverterNode::ProcessDirectFrame(HpaePcmBuffer *input)
{
    if (input->GetFrameLen() == 0) {
        return PrepareSilenceBuffer(silenceData_);  // degenerate upstream frame
    }
    int32_t ret = ResampleWithChannelConversion(input->GetPcmDataBuffer(), input->GetFrameLen(),
        converterOutput_.GetPcmDataBuffer(), converterOutput_.GetFrameLen());
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: direct convert failed, code %{public}d", GetNodeId(), ret);
        return PrepareSilenceBuffer(silenceData_);
    }
    converterOutput_.SetBufferState(input->GetBufferState());
    return &converterOutput_;
}

void HpaeAudioFormatConverterNode::ReportConverterFault(const char *reason)
{
    // report the event once per degraded episode; silence keeps flowing every cycle
    if (!converterFaultReported_) {
        converterFaultReported_ = true;
        AUDIO_ERR_LOG("NodeId %{public}d %{public}s", GetNodeId(), reason);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::GENERAL,
                BusinessScenario::QUERY, ERR_NULL_POINTER), reason, false);
    }
    outputStream_.WriteDataToOutput(PrepareSilenceBuffer(silenceData_));
}

// drain processedBuffer_ with buffer state propagation; return true if output was written
bool HpaeAudioFormatConverterNode::DrainProcessedBufferToOutput(const std::vector<HpaePcmBuffer *> &preOutputs,
    bool hasPreOutput)
{
    if (processedFrames_ < outputFrameLen20ms_ || !enableProcess_) {
        return false;
    }
    converterOutput_.Reset();
    if (!ExtractOutputFromProcessedBuffer()) {
        return false;
    }
    if (hasPreOutput) {
        converterOutput_.SetBufferState(preOutputs[0]->GetBufferState());
        converterOutput_.SetBufferValid(true);
    }
    AUDIO_DEBUG_LOG("NodeId %{public}d: drain processedBuffer_, "
        "remaining %{public}u frames", GetNodeId(), processedFrames_);
    outputStream_.WriteDataToOutput(&converterOutput_);
    return true;
}

void HpaeAudioFormatConverterNode::AccumulateInputAndPullAhead(HpaePcmBuffer *input)
{
    if (resampler_ == nullptr) {
        // defensive: accumulate mode is guaranteed a resampler by the DoProcess gate
        AUDIO_ERR_LOG("NodeId %{public}d resampler_ is nullptr", GetNodeId());
        return;
    }
    float *srcData = input->GetPcmDataBuffer();
    uint32_t inputFrameLen = input->GetFrameLen();
    uint32_t inputChannels = input->GetChannelCount();

    // Step 1: Append input data to unprocessedBuffer_
    CHECK_AND_RETURN_LOG(AppendToUnprocessedBuffer(srcData, inputFrameLen, inputChannels),
        "NodeId %{public}d: AppendToUnprocessedBuffer failed", GetNodeId());

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
    if (pullAheadCredit_ > 0 && unprocessedFrames_ < resamplerInputThreshold_ && needsAccumulation) {
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
        return PrepareSilenceBuffer(silenceData_);
    }

    AUDIO_DEBUG_LOG("NodeId %{public}d: output success, "
        "extracted %{public}u frames, remaining processedFrames=%{public}u",
        GetNodeId(), outputFrameLen20ms_, processedFrames_);

    converterOutput_.SetBufferState(input->GetBufferState());
    return &converterOutput_;
}

HpaePcmBuffer *HpaeAudioFormatConverterNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
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
    uint32_t resampleChannels = ResamplerWorkChannels();
    if (resampler_ != nullptr && resampleChannels != resampler_->GetChannels()) {
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

    uint32_t numChannels = 0;
    AudioChannelLayout channelLayout = AudioChannelLayout::CH_LAYOUT_UNKNOWN;
    AudioSamplingRate sampleRate = SAMPLE_RATE_8000;
    ResolveDownstreamOutFormat(numChannels, channelLayout, sampleRate);

    AudioChannelInfo curOutChannelInfo = channelConverter_.GetOutChannelInfo();
    if ((curOutChannelInfo.numChannels == numChannels) && (curOutChannelInfo.channelLayout == channelLayout) &&
        (sampleRate == outRate_)) {
        return false;
    }
    // update channel info
    if (curOutChannelInfo.numChannels != numChannels || curOutChannelInfo.channelLayout != channelLayout) {
        if (!UpdateOutChannelInfo(numChannels, channelLayout)) {
            return false;
        }
    }
    // update sample rate
    uint32_t oldOutRate = outRate_;
    if (oldOutRate != sampleRate) {
        AUDIO_INFO_LOG("NodeId: %{public}d, update output sample rate: "
            "%{public}d -> %{public}d", GetNodeId(), oldOutRate, sampleRate);
        outRate_ = sampleRate;
        if (resampler_ != nullptr) {
            resampler_->UpdateRates(resampler_->GetInRate(), sampleRate);
        }
    }

    UpdateNodeInfoOnOutChange(numChannels, channelLayout, oldOutRate);
    return true;
}

// resolve the desired output format from the format query (algo-side or mixer-side), with the
// empty-format fallback
void HpaeAudioFormatConverterNode::ResolveDownstreamOutFormat(uint32_t &numChannels,
    AudioChannelLayout &channelLayout, AudioSamplingRate &sampleRate)
{
    AudioBasicFormat basicFormat;
    basicFormat.rate = preNodeInfo_.samplingRate;

    // the registered callback decides what output format this position needs
    nodeFormatInfoCallback_->GetNodeInputFormatInfo(preNodeInfo_.sessionId, basicFormat);

    numChannels = basicFormat.audioChannelInfo.numChannels;
    channelLayout = basicFormat.audioChannelInfo.channelLayout;
    sampleRate = basicFormat.rate;
    if (numChannels == 0) {
        // set to node info, which is device output info
        AUDIO_WARNING_LOG("Fail to check format info from down stream nodes");
        numChannels = GetChannelCount();
        channelLayout = GetChannelLayout();
        sampleRate = GetSampleRate();
    }
}

// rebuild node-level info after an output format change
void HpaeAudioFormatConverterNode::UpdateNodeInfoOnOutChange(uint32_t numChannels,
    AudioChannelLayout channelLayout, uint32_t oldOutRate)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    nodeInfo.channels = static_cast<AudioChannel>(numChannels);
    nodeInfo.channelLayout = (AudioChannelLayout)channelLayout;
    uint32_t newOutRate = outRate_;
    nodeInfo.samplingRate = (AudioSamplingRate)newOutRate;
    // Proportionally update frameLen when output rate changes to preserve time duration
    if (oldOutRate != 0 && oldOutRate != newOutRate) {
        nodeInfo.frameLen = static_cast<uint32_t>(
            static_cast<uint64_t>(nodeInfo.frameLen) * newOutRate / oldOutRate);
    }
    SetNodeInfo(nodeInfo);
}

// update channel info from processCluster. For now sample rate will not change
bool HpaeAudioFormatConverterNode::CheckUpdateInInfo(HpaePcmBuffer *input)
{
    uint32_t numChannels = input->GetChannelCount();
    uint64_t channelLayout = input->GetChannelLayout();
    uint32_t sampleRate = input->GetSampleRate();
    uint32_t inputFrameLen = input->GetFrameLen();
    bool isInfoUpdated = UpdateInChannelInfo(numChannels, channelLayout);
    // update sample rate
    if (sampleRate != inRate_) {
        AUDIO_INFO_LOG("NodeId %{public}d: Update resampler input sample rate: "
            "%{public}d -> %{public}d", GetNodeId(), inRate_, sampleRate);
        preNodeInfo_.samplingRate = (AudioSamplingRate)sampleRate;
        preNodeInfo_.frameLen = CalculateFrameLenBySampleRate(sampleRate);
        inRate_ = sampleRate;
        if (resampler_ != nullptr) {
            resampler_->UpdateRates(sampleRate, resampler_->GetOutRate());
        }
        isInfoUpdated = true;
    }
    // track the actual upstream frame length; zero-length buffers keep the last valid value
    if (inputFrameLen != 0 && inputFrameLen != inputFrameLen_) {
        AUDIO_INFO_LOG("NodeId %{public}d: Update input frameLen: "
            "%{public}d -> %{public}d", GetNodeId(), inputFrameLen_, inputFrameLen);
        inputFrameLen_ = inputFrameLen;
        isInfoUpdated = true;
    }
    return isInfoUpdated;
}

bool HpaeAudioFormatConverterNode::UpdateInChannelInfo(uint32_t numChannels, uint64_t channelLayout)
{
    AudioChannelInfo curInChannelInfo = channelConverter_.GetInChannelInfo();
    if ((curInChannelInfo.numChannels == numChannels) && (curInChannelInfo.channelLayout == channelLayout)) {
        return false;
    }
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

    uint32_t resampleChannels = ResamplerWorkChannels();
    if (resampler_ != nullptr && resampleChannels != resampler_->GetChannels()) {
        AUDIO_INFO_LOG("NodeId: %{public}d, Update resampler work channel from effectNode!", GetNodeId());
        resampler_->UpdateChannels(resampleChannels);
    }
    return true;
}

void HpaeAudioFormatConverterNode::CheckAndUpdateInfo(HpaePcmBuffer *input)
{
    bool isInfoUpdated = CheckUpdateInInfo(input);
    bool isOutInfoUpdated = CheckUpdateOutInfo();
    if ((!isInfoUpdated) && (!isOutInfoUpdated)) {
        return;
    }
    AUDIO_INFO_LOG("NodeId %{public}d: output or input format info is changed, re-decide process mode!",
        GetNodeId());
    if (!DecideProcessMode()) {
        ApplyModeResources();  // same-mode format change: resize resources only
    }
    if (isOutInfoUpdated) {
        NotifyOutFormatChangedDfx();
    }
}

// hidump notify and pcm dumper refresh after an output format change
void HpaeAudioFormatConverterNode::NotifyOutFormatChangedDfx()
{
    AUDIO_INFO_LOG("NodeId %{public}d: output format info is changed, update output PCM buffer info!", GetNodeId());
#ifdef ENABLE_HIDUMP_DFX
    if (auto callBack = GetNodeStatusCallback().lock()) {
        callBack->OnNotifyDfxNodeInfoChanged(GetNodeId(), GetNodeInfo());
    }
#endif
// update PCM dumper
#ifdef ENABLE_HOOK_PCM
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeConverterNodeOutput_id_" + std::to_string(GetSessionId()) +
        "_nodeId_" + std::to_string(GetNodeId()) +
        "_ch_" + std::to_string(GetChannelCount()) + "_rate_" +
        std::to_string(GetSampleRate()) + "_" + GetTime() + ".pcm");
#endif
}

// ---- process mode state machine: decisions happen only at ctor and format events ----

// returns true when the mode changed and resources were already re-applied
bool HpaeAudioFormatConverterNode::DecideProcessMode()
{
    ConverterProcessMode newMode = ComputeProcessMode();
    if (newMode == processMode_) {
        return false;
    }
    AUDIO_INFO_LOG("NodeId %{public}d: process mode %{public}d -> %{public}d", GetNodeId(),
        static_cast<int32_t>(processMode_), static_cast<int32_t>(newMode));
    ReleaseModeResources();
    processMode_ = newMode;
    ApplyModeResources();
    return true;
}

ConverterProcessMode HpaeAudioFormatConverterNode::ComputeProcessMode() const
{
    uint32_t inCh = channelConverter_.GetInChannelInfo().numChannels;
    uint32_t outCh = channelConverter_.GetOutChannelInfo().numChannels;
    if (inRate_ == outRate_ && inCh == outCh && inputFrameLen_ == GetFrameLen()) {
        return ConverterProcessMode::PASSTHROUGH;
    }
    bool standardRate = inRate_ != 0 && inRate_ % CUSTOM_SAMPLE_RATE_MULTIPLES == 0;
    if (standardRate && outRate_ != 0 && GetFrameLen() != 0 && IsExactDirectRatio()) {
        return ConverterProcessMode::DIRECT_PROCESS;
    }
    if (standardRate && inRate_ != outRate_) {
        AUDIO_INFO_LOG("NodeId %{public}d: frame ratio not exact (in %{public}u -> out %{public}u, "
            "inframeLen %{public}u, outframeLen %{public}zu), fall back to accumulate",
            GetNodeId(), inRate_, outRate_, inputFrameLen_, GetFrameLen());
    }
    return ConverterProcessMode::ACCUMULATE;
}

bool HpaeAudioFormatConverterNode::IsExactDirectRatio() const
{
    uint64_t outSamples = static_cast<uint64_t>(inputFrameLen_) * outRate_;
    if (outSamples % inRate_ != 0) {
        return false;
    }
    return static_cast<uint32_t>(outSamples / inRate_) == GetFrameLen();
}

bool HpaeAudioFormatConverterNode::NeedsResampler() const
{
    if (processMode_ == ConverterProcessMode::ACCUMULATE) {
        return true;
    }
    return processMode_ == ConverterProcessMode::DIRECT_PROCESS && inRate_ != outRate_;
}

uint32_t HpaeAudioFormatConverterNode::ResamplerWorkChannels() const
{
    return std::min(channelConverter_.GetInChannelInfo().numChannels,
        channelConverter_.GetOutChannelInfo().numChannels);
}

std::unique_ptr<Resampler> HpaeAudioFormatConverterNode::CreateResampler(uint32_t inRate, uint32_t outRate,
    uint32_t channels)
{
    return std::make_unique<ProResampler>(inRate, outRate, channels, REASAMPLE_QUAILTY);
}

// acquire or resize the resources required by the current mode; event-time only and idempotent
void HpaeAudioFormatConverterNode::ApplyModeResources()
{
    if (processMode_ == ConverterProcessMode::PASSTHROUGH) {
        // silence frames must still match the node output format after a format round-trip
        silenceData_.ReConfig(BuildOutPcmBufferInfo());
        silenceData_.SetBufferValid(false);
        silenceData_.SetBufferSilence(true);
        return;
    }
    if (NeedsResampler() && resampler_ == nullptr) {
        resampler_ = CreateResampler(inRate_, outRate_, ResamplerWorkChannels());
        if (resampler_ == nullptr) {
            // degraded: silence frames must still carry the negotiated output format
            AUDIO_ERR_LOG("NodeId %{public}d: create resampler failed, degraded until next format event",
                GetNodeId());
            silenceData_.ReConfig(BuildOutPcmBufferInfo());
            silenceData_.SetBufferValid(false);
            silenceData_.SetBufferSilence(true);
            return;
        }
        converterFaultReported_ = false;
    }
    PcmBufferInfo outPcmBufferInfo = BuildOutPcmBufferInfo();
    converterOutput_.ReConfig(outPcmBufferInfo);
    silenceData_.ReConfig(outPcmBufferInfo);
    // reconfig need reset valid
    silenceData_.SetBufferValid(false);
    silenceData_.SetBufferSilence(true);
    if (processMode_ == ConverterProcessMode::DIRECT_PROCESS) {
        ConfigureDirectTmpBuffer();
        return;
    }
    InitDualBuffer();
}

// release all intermediate resources; silenceData_ stays for the silence path
void HpaeAudioFormatConverterNode::ReleaseModeResources()
{
    resampler_.reset();
    converterFaultReported_ = false;
    std::vector<float>().swap(unprocessedBuffer_);
    std::vector<float>().swap(processedBuffer_);
    std::vector<float>().swap(resamplerOutBuf_);
    converterOutput_.ReConfig(ZeroPcmBufferInfo());
    tmpOutBuf_.ReConfig(ZeroPcmBufferInfo());
    unprocessedFrames_ = 0;
    processedFrames_ = 0;
    processedReadPos_ = 0;
    pullAheadCredit_ = 0;
    pullAheadMaxCredit_ = 0;
    resamplerInputThreshold_ = 0;
    outputFrameLen20ms_ = 0;
}

// tmp staging is only needed for the combined two-stage conversion (rate and channels differ)
void HpaeAudioFormatConverterNode::ConfigureDirectTmpBuffer()
{
    uint32_t inCh = channelConverter_.GetInChannelInfo().numChannels;
    uint32_t outCh = channelConverter_.GetOutChannelInfo().numChannels;
    if (inCh == outCh || inRate_ == outRate_) {
        return;
    }
    ReconfigTmpOutForChannelConversion(inCh, outCh, inputFrameLen_, GetFrameLen());
}

PcmBufferInfo HpaeAudioFormatConverterNode::BuildOutPcmBufferInfo() const
{
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    // isMultiFrames_ and frame_ are inheritated from sinkInputNode
    PcmBufferInfo outPcmBufferInfo = pcmBufferInfo_;
    outPcmBufferInfo.ch = outChannelInfo.numChannels;
    outPcmBufferInfo.rate = outRate_;
    outPcmBufferInfo.frameLen = ComputeOutFrameLen();
    outPcmBufferInfo.channelLayout = outChannelInfo.channelLayout;
    return outPcmBufferInfo;
}

// exact output frame length: integer math for DIRECT (eligibility guarantees it equals GetFrameLen()),
// legacy float duration math for ACCUMULATE to keep the historical sizing unchanged
uint32_t HpaeAudioFormatConverterNode::ComputeOutFrameLen() const
{
    if (inRate_ == outRate_) {
        return GetFrameLen();
    }
    if (processMode_ == ConverterProcessMode::DIRECT_PROCESS) {
        return static_cast<uint32_t>(static_cast<uint64_t>(inputFrameLen_) * outRate_ / inRate_);
    }
    return static_cast<uint32_t>(outRate_ * frameLenInMs_ / MILLISECOND_PER_SECOND);
}

void HpaeAudioFormatConverterNode::ReconfigTmpOutForChannelConversion(uint32_t inCh, uint32_t outCh,
    uint32_t inFrameLen, uint32_t maxOutFrames)
{
    PcmBufferInfo tmpOutPcmBufferInfo;
    if (inCh > outCh) {
        // Downmix first, then resample: tmpOutBuf_ holds downmixed input at inRate
        tmpOutPcmBufferInfo.ch = inCh;
        tmpOutPcmBufferInfo.rate = inRate_;
        tmpOutPcmBufferInfo.frameLen = inFrameLen;
    } else {
        // Resample first, then upmix: tmpOutBuf_ holds resampled output at outRate
        tmpOutPcmBufferInfo.ch = inCh;
        tmpOutPcmBufferInfo.rate = outRate_;
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
        static_cast<uint32_t>(outRate * frameLenInMs_ / MILLISECOND_PER_SECOND);

    AudioChannelInfo inChannelInfo = channelConverter_.GetInChannelInfo();
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    uint32_t inCh = inChannelInfo.numChannels;
    uint32_t outCh = outChannelInfo.numChannels;

    // processedBuffer_ and resamplerOutBuf_ store data at outCh per frame
    uint32_t maxOutFrames = static_cast<uint32_t>(
        static_cast<uint64_t>(resamplerInputThreshold_) * resampler_->GetOutRate() / resampler_->GetInRate());
    ResizeDualBuffers(inCh, outCh, maxOutFrames);

    // Resize tmpOutBuf_ for channel conversion paths
    if (inCh != outCh) {
        ReconfigTmpOutForChannelConversion(inCh, outCh, resamplerInputThreshold_, maxOutFrames);
    }

    AUDIO_INFO_LOG("NodeId %{public}d: InitDualBuffer threshold=%{public}u, outputFrameLen20ms=%{public}u, "
        "maxOutFrames=%{public}u, outCh=%{public}u, inCh=%{public}u", GetNodeId(), resamplerInputThreshold_,
        outputFrameLen20ms_, maxOutFrames, outCh, inCh);
}

// size the dual buffers (unprocessed at inCh/frame, processed/resamplerOut at outCh/frame)
// and reset the pull-ahead credit
void HpaeAudioFormatConverterNode::ResizeDualBuffers(uint32_t inCh, uint32_t outCh, uint32_t maxOutFrames)
{
    // unprocessedBuffer_ stores data at inCh per frame
    size_t unprocessedCapacity = static_cast<size_t>(resamplerInputThreshold_) *
        UNPROCESSED_BUFFER_CAPACITY_MULTIPLIER * inCh;
    unprocessedBuffer_.resize(unprocessedCapacity, 0.0f);
    unprocessedFrames_ = 0;

    size_t processedCapacity = static_cast<size_t>(maxOutFrames) *
        PROCESSED_BUFFER_CAPACITY_MULTIPLIER * outCh;
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
}

uint32_t HpaeAudioFormatConverterNode::CalculateResamplerInputThreshold() const
{
    if (resampler_ == nullptr) {
        return 0;
    }
    uint32_t inRate = resampler_->GetInRate();
    uint32_t outRate = resampler_->GetOutRate();
    if (inRate == outRate) {
        return static_cast<uint32_t>(inRate * frameLenInMs_ / MILLISECOND_PER_SECOND);
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
        return static_cast<uint32_t>(inRate * frameLenInMs_ / MILLISECOND_PER_SECOND); // standard 20ms
    }
}

bool HpaeAudioFormatConverterNode::AppendToUnprocessedBuffer(const float *data, uint32_t frameLen,
    uint32_t channels)
{
    if (frameLen == 0 || data == nullptr) {
        return false;
    }
    size_t samplesToAdd = static_cast<size_t>(frameLen) * channels;
    size_t requiredSize = static_cast<size_t>(unprocessedFrames_ + frameLen) * channels;
    if (unprocessedBuffer_.size() < requiredSize) {
        AUDIO_ERR_LOG("NodeId %{public}d: AppendToUnprocessedBuffer buffer overflow, "
            "size=%{public}zu, required=%{public}zu", GetNodeId(), unprocessedBuffer_.size(), requiredSize);
        return false;
    }
    errno_t ret = memcpy_s(unprocessedBuffer_.data() + static_cast<size_t>(unprocessedFrames_) * channels,
        (unprocessedBuffer_.size() - static_cast<size_t>(unprocessedFrames_) * channels) * sizeof(float),
        data, samplesToAdd * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: AppendToUnprocessedBuffer memcpy_s failed", GetNodeId());
        return false;
    }
    unprocessedFrames_ += frameLen;
    if (pullAheadCredit_ > 0) {
        pullAheadCredit_--;
    }
    AUDIO_DEBUG_LOG("NodeId %{public}d: AppendToUnprocessedBuffer added %{public}u frames @%{public}u ch, "
        "total unprocessedFrames=%{public}u", GetNodeId(), frameLen, channels, unprocessedFrames_);
    return true;
}

bool HpaeAudioFormatConverterNode::AppendToProcessedBuffer(const float *data, uint32_t frameLen, uint32_t channels)
{
    size_t writeOffset = static_cast<size_t>(processedReadPos_ + processedFrames_) * channels;
    size_t samplesToAdd = static_cast<size_t>(frameLen) * channels;
    if (processedBuffer_.size() < writeOffset + samplesToAdd) {
        AUDIO_ERR_LOG("NodeId %{public}d: AppendToProcessedBuffer buffer overflow, "
            "size=%{public}zu, required=%{public}zu", GetNodeId(), processedBuffer_.size(),
            writeOffset + samplesToAdd);
        return false;
    }
    errno_t ret = memcpy_s(processedBuffer_.data() + writeOffset,
        (processedBuffer_.size() - writeOffset) * sizeof(float),
        data, samplesToAdd * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: AppendToProcessedBuffer memcpy_s failed", GetNodeId());
        return false;
    }
    processedFrames_ += frameLen;
    return true;
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
    return AppendToUnprocessedBuffer(data, frameLen, channels);
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
        AUDIO_DEBUG_LOG("NodeId %{public}d: PullAhead pulled %{public}u extra frames, "
            "unprocessedFrames=%{public}u, remaining credit=%{public}u",
            GetNodeId(), pullCount, unprocessedFrames_, pullAheadCredit_);
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
            inPtr, inFrameLen, resamplerOutBuf_.data(), outFrameLen);
        if (ret != EOK) {
            AUDIO_ERR_LOG("NodeId %{public}d: Same-rate channel conversion "
                "failed, code %{public}d", GetNodeId(), ret);
            break;
        }
        // Remove at inCh per frame
        CHECK_AND_BREAK_LOG(RemoveConsumedInputFrames(inFrameLen, inCh),
            "NodeId %{public}d: RemoveConsumedInputFrames failed", GetNodeId());
        CHECK_AND_BREAK_LOG(AppendToProcessedBuffer(resamplerOutBuf_.data(), outFrameLen, outCh),
            "NodeId %{public}d: AppendToProcessedBuffer failed", GetNodeId());
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
            inPtr, inFrameLen, resamplerOutBuf_.data(), outFrameLen);
        if (ret != EOK) {
            AUDIO_ERR_LOG("NodeId %{public}d: Resample failed with code %{public}d",
                GetNodeId(), ret);
            break;
        }

        CHECK_AND_BREAK_LOG(RemoveConsumedInputFrames(inFrameLen, inCh),
            "NodeId %{public}d: RemoveConsumedInputFrames failed", GetNodeId());
        CHECK_AND_BREAK_LOG(AppendToProcessedBuffer(resamplerOutBuf_.data(), outFrameLen, outCh),
            "NodeId %{public}d: AppendToProcessedBuffer failed", GetNodeId());
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
    float *dstData, uint32_t outFrameLen)
{
    AudioChannelInfo inChannelInfo = channelConverter_.GetInChannelInfo();
    AudioChannelInfo outChannelInfo = channelConverter_.GetOutChannelInfo();
    uint32_t inRate = inRate_;
    uint32_t outRate = outRate_;
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
    uint32_t frames = outputFrameLen20ms_;
    float *dstData = converterOutput_.GetPcmDataBuffer();

    uint32_t channels = converterOutput_.GetChannelCount();
    uint32_t copySamples = frames * channels;
    errno_t ret = memcpy_s(dstData, converterOutput_.Size(),
        processedBuffer_.data() + static_cast<size_t>(processedReadPos_) * channels,
        copySamples * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: ExtractOutput memcpy_s failed", GetNodeId());
        return false;
    }
    ConsumeProcessedFrames(frames, channels);
    AUDIO_DEBUG_LOG("NodeId %{public}d: ExtractOutput success, remaining processedFrames=%{public}u",
        GetNodeId(), processedFrames_);
    return true;
}

void HpaeAudioFormatConverterNode::ConsumeProcessedFrames(uint32_t frames, uint32_t stride)
{
    processedReadPos_ += frames;
    processedFrames_ -= frames;
    if (processedFrames_ == 0) {
        processedReadPos_ = 0;
    } else if (processedReadPos_ >
        processedBuffer_.size() / (stride * PROCESSED_BUFFER_COMPACTION_DIVISOR)) {
        CompactProcessedBuffer(stride);
    }
    // Return one credit: data has been consumed downstream
    if (pullAheadCredit_ < pullAheadMaxCredit_) {
        pullAheadCredit_++;
    }
}

bool HpaeAudioFormatConverterNode::RemoveConsumedInputFrames(uint32_t consumedFrames, uint32_t channels)
{
    uint32_t remainingFrames = unprocessedFrames_ - consumedFrames;
    if (remainingFrames == 0) {
        unprocessedFrames_ = 0;
        return true;
    }
    size_t srcOffset = static_cast<size_t>(consumedFrames) * channels;
    size_t remainingSamples = static_cast<size_t>(remainingFrames) * channels;
    errno_t ret = memmove_s(unprocessedBuffer_.data(), unprocessedBuffer_.size() * sizeof(float),
        unprocessedBuffer_.data() + srcOffset, remainingSamples * sizeof(float));
    if (ret != EOK) {
        AUDIO_ERR_LOG("NodeId %{public}d: RemoveConsumedInputFrames memmove_s failed", GetNodeId());
        return false;
    }
    unprocessedFrames_ = remainingFrames;
    return true;
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
        AUDIO_ERR_LOG("NodeId %{public}d: MoveUnprocessedToProcessed buffer overflow, "
            "size=%{public}zu, required=%{public}zu", GetNodeId(), processedBuffer_.size(),
            writeOffset + samples);
        return;
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

void HpaeAudioFormatConverterNode::SetSourceBufferType(HpaeSourceBufferType type)
{
    configuredSourceBufferType_ = type;
    converterOutput_.SetSourceBufferType(type);
}

void HpaeAudioFormatConverterNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    inputStream_.Connect(preNode->GetSharedInstance(), preNode->GetOutputPort(nodeInfo));
    // keep the passthrough tag in sync with the connect-time tag (MIC/EC/MICREF chains)
    configuredSourceBufferType_ = nodeInfo.sourceBufferType;
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
    if (processMode_ != ConverterProcessMode::ACCUMULATE) {
        return 0; // passthrough and direct paths never stage frames
    }
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

void HpaeAudioFormatConverterNode::SetUpmixCoef(float coef)
{
    channelConverter_.SetUpmixCoef(coef);
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
    if (resampler_ != nullptr) {
        resampler_->Reset();
    }
}
} // Hpae
} // AudioStandard
} // OHOS