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
#ifndef HPAE_AUDIOFORMAT_CONVERTER_H
#define HPAE_AUDIOFORMAT_CONVERTER_H
#include "audio_stream_info.h"
#include "hpae_plugin_node.h"
#include "channel_converter.h"
#include "audio_proresampler.h"
#ifdef ENABLE_HOOK_PCM
    #include "hpae_pcm_dumper.h"
#endif
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
// Process mode is decided at construction and format-change events only, never per frame.
enum class ConverterProcessMode {
    PASSTHROUGH,     // same rate/channels/frameLen: zero-copy forwarding, no intermediate buffers
    DIRECT_PROCESS,  // standard rate with exact frame ratio: per-frame convert, no dual buffers
    ACCUMULATE,      // non-standard rate or inexact ratio: legacy dual-buffer staging
};
class HpaeAudioFormatConverterNode : public HpaePluginNode {
public:
    HpaeAudioFormatConverterNode(HpaeNodeInfo preNodeInfo, HpaeNodeInfo nodeInfo);
    virtual ~HpaeAudioFormatConverterNode();
    // Register once before streaming; callback is read on the process thread
    void RegisterCallback(INodeFormatInfoCallback *callback);
    void DoProcess() override;
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode, HpaeNodeInfo &nodeInfo) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    uint64_t GetLatency(uint32_t sessionId = 0) override;
    void SetDownmixNormalization(bool normalizing);
    void SetUpmixCoef(float coef);
    void FlushBuffers();
    void SetSourceBufferType(HpaeSourceBufferType type);
protected:
    HpaePcmBuffer* SignalProcess(const std::vector<HpaePcmBuffer*>& inputs) override;
    // factory hook for lazy resampler creation; overridable for failure injection in tests.
    // NOTE: calls from the base constructor dispatch to this base impl, not to a derived override
    virtual std::unique_ptr<Resampler> CreateResampler(uint32_t inRate, uint32_t outRate, uint32_t channels);
private:
    bool CheckUpdateInInfo(HpaePcmBuffer *input);
    bool UpdateInChannelInfo(uint32_t numChannels, uint64_t channelLayout);
    bool CheckUpdateOutInfo();
    void CheckAndUpdateInfo(HpaePcmBuffer *input);
    // mode state machine: decide at ctor/format events, acquire/release mode resources
    bool DecideProcessMode();
    ConverterProcessMode ComputeProcessMode() const;
    bool IsExactDirectRatio() const;
    void ApplyModeResources();
    void ReleaseModeResources();
    void ConfigureDirectTmpBuffer();
    PcmBufferInfo BuildOutPcmBufferInfo() const;
    uint32_t ComputeOutFrameLen() const;
    bool NeedsResampler() const;
    uint32_t ResamplerWorkChannels() const;
    HpaePcmBuffer* ProcessByMode(const std::vector<HpaePcmBuffer *> &inputs);
    HpaePcmBuffer* ProcessPassthrough(HpaePcmBuffer *input);
    HpaePcmBuffer* ProcessDirectFrame(HpaePcmBuffer *input);
    void ReportConverterFault(const char *reason);
    void DumpConverterOutput(HpaePcmBuffer *output);
    void NotifyOutFormatChangedDfx();

    // --- Dual-buffer mechanism for non-standard sample rate handling ---
    bool AppendToUnprocessedBuffer(const float *data, uint32_t frameLen, uint32_t channels);
    bool AppendToProcessedBuffer(const float *data, uint32_t frameLen, uint32_t channels);
    void PullAheadUntilThresholdOrFailure(uint32_t inputFrameLen);
    void ProcessResampleLoop();
    bool ExtractOutputFromProcessedBuffer();
    bool RemoveConsumedInputFrames(uint32_t consumedFrames, uint32_t channels);
    void CompactProcessedBuffer(uint32_t channels);
    void MoveUnprocessedToProcessed(uint32_t channels);
    bool TryPullOneFrame();
    uint32_t CalculateResamplerInputThreshold() const;
    void InitDualBuffer();
    int32_t ResampleWithChannelConversion(float *srcData, uint32_t inFrameLen,
        float *dstData, uint32_t outFrameLen);

    // --- Refactoring helpers (sub-50-line extraction targets) ---
    void ProcessSameRateChannelConversion(uint32_t inCh, uint32_t outCh);
    void ProcessDiffRateResample(uint32_t inCh, uint32_t outCh, uint32_t inRate, uint32_t outRate);
    void AccumulateInputAndPullAhead(HpaePcmBuffer *input);
    HpaePcmBuffer* ProcessAndExtractOutput(HpaePcmBuffer *input);
    void ReconfigTmpOutForChannelConversion(uint32_t inCh, uint32_t outCh,
        uint32_t inFrameLen, uint32_t maxOutFrames);
    bool UpdateOutChannelInfo(uint32_t numChannels, AudioChannelLayout channelLayout);
    void ConsumeProcessedFrames(uint32_t frames, uint32_t stride);
    bool DrainProcessedBufferToOutput(const std::vector<HpaePcmBuffer *> &preOutputs, bool hasPreOutput);
    void ResolveDownstreamOutFormat(uint32_t &numChannels, AudioChannelLayout &channelLayout,
        AudioSamplingRate &sampleRate);
    void UpdateNodeInfoOnOutChange(uint32_t numChannels, AudioChannelLayout channelLayout, uint32_t oldOutRate);
    void ResizeDualBuffers(uint32_t inCh, uint32_t outCh, uint32_t maxOutFrames);
    void OnTraceInfoChanged() override;
    void RebuildTraceName();

    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer converterOutput_;
    HpaeNodeInfo preNodeInfo_;
    // rate/frameLen truth source independent of lazy resampler_; updated at ctor and format events
    uint32_t inRate_ = 0;
    uint32_t outRate_ = 0;
    uint32_t inputFrameLen_ = 0;
    ConverterProcessMode processMode_ = ConverterProcessMode::ACCUMULATE;
    // tag recorded from SetSourceBufferType/ConnectWithInfo; passthrough stamps it on the
    // upstream buffer per cycle
    HpaeSourceBufferType configuredSourceBufferType_ = HPAE_SOURCE_BUFFER_TYPE_DEFAULT;
    // DFX fault event is reported once per degraded episode, cleared on successful (re)acquire
    bool converterFaultReported_ = false;
    std::unique_ptr<Resampler> resampler_ = nullptr;
    ChannelConverter channelConverter_;
    HpaePcmBuffer tmpOutBuf_; // cache between resample and converter
    // downstream target of the output format query; must outlive this node
    INodeFormatInfoCallback *nodeFormatInfoCallback_ = nullptr;

    // Unprocessed input accumulation buffer (interleaved float, input sample rate)
    std::vector<float> unprocessedBuffer_;
    uint32_t unprocessedFrames_ = 0;

    // Processed output buffer (interleaved float, output sample rate)
    std::vector<float> processedBuffer_;
    uint32_t processedFrames_ = 0;
    uint32_t processedReadPos_ = 0;

    // Threshold: minimum input frames to trigger a resampler call
    // 441 for 11025Hz, 801 for 8010Hz, 960 for 48000Hz
    uint32_t resamplerInputThreshold_ = 0;

    // 20ms output frame count at output sample rate (e.g., 960 for 48000Hz)
    uint32_t outputFrameLen20ms_ = 0;

    // Temporary buffer for resampler output (sized per call, reused)
    std::vector<float> resamplerOutBuf_;

    // Pull-ahead credit counter: caps upstream pulls to prevent buffer bloat.
    // Decremented on each successful pull, incremented when output is consumed downstream.
    uint32_t pullAheadCredit_ = 0;
    uint32_t pullAheadMaxCredit_ = 0;

    float frameLenInMs_ = 0.0f;

    // Cached DoProcess trace name; rebuilt in ctor and whenever node info changes
    std::string processTraceName_;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
#endif
};

} // HPAE
} // AudioStandard
} // OHOS
#endif
