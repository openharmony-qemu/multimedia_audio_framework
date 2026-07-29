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

#ifndef HPAE_SESSION_EFFECT_NODE_H
#define HPAE_SESSION_EFFECT_NODE_H

#include "hpae_plugin_node.h"
#include "i_effect_instance.h"
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include "se_process_stats.h"
#include <chrono>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeSessionEffectNode : public HpaePluginNode {
public:
    enum class STATUS { IDLE, WARMING_UP, INTERCEPTING, ACTIVE, STOPPING };
    static constexpr uint32_t CROSSFADE_DURATION_MS = 10; // 48kHz: 480 samples
    static constexpr uint32_t ACTIVE_NO_DATA_PADDING_DELAY_MS = 1000;
    static constexpr uint32_t ACTIVE_TAIL_PADDING_MAX_FRAMES = 200;

    explicit HpaeSessionEffectNode(HpaeNodeInfo &nodeInfo);
    ~HpaeSessionEffectNode() override;

    // Effect lifecycle — node receives pre-created instances from manager layer
    int32_t CreateEffectWithInstance(const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config);
    int32_t DropEffect();  // Release node reference without destroying algorithm instance
    int32_t DetachEffectForReload();

    // Query
    std::shared_ptr<IEffectInstance> GetEffectInstance() const;
    std::string GetEffectName() const;
    AudioEffectConfig GetAudioConfig() const;

    // Control
    int32_t StartEffect();
    int32_t FlushEffect();
    int32_t SetParameter(int32_t type, float value);

    // Close SE support
    void StartCloseCrossfade();
    bool IsCloseCrossfadePending() const;
    void StopPreheatThread();

    // Pause: preserves caches, does NOT send DISABLE to algorithm
    int32_t PauseEffect();
    // Prepare for MoveStream: RewindBuffer + clear caches (manager handles node deletion)
    void PrepareMoveStream();

    // Graph-switch preheat suspend/resume (idempotent, same-node only).
    // Suspend: stops preheat thread if WARMING_UP, marks resume expected.
    void SuspendSessionEffectPreheatForGraphSwitch();
    // Resume: restarts preheat thread if previously suspended and still WARMING_UP.
    void ResumeSessionEffectPreheatForGraphSwitch();

    // HpaePluginNode override
    uint64_t GetLatency(uint32_t sessionId = 0) override;

    // DoProcess override for WARMING_UP disconnect
    void DoProcess() override;
    STATUS GetStatus() const { return status_; }

protected:
    virtual void StartPreheatThread();
    void SetPrebufFilledForTest(bool filled) { prebufFilled_.store(filled); }
    bool GetPrebufFilled() const { return prebufFilled_.load(); }
    void RunPreheatThreadForTest() { PreheatThreadFunc(); }
    void SetPrebufTargetBytesForTest(size_t bytes) { prebufTargetBytes_ = bytes; }
    void SetStatusForTest(STATUS s) { status_ = s; }
    void ResetTimestampsForTest()
    {
        warmupStartTime_ = std::chrono::steady_clock::now();
        pauseAccumulatedUs_ = 0;
        pauseStartTime_ = std::chrono::steady_clock::time_point{};
    }
    void SetWarmupStartTimeForTest(std::chrono::steady_clock::time_point t) { warmupStartTime_ = t; }
    size_t GetPrebufTargetBytes() const;  // TEMP diag
    bool IsPaused() const { return isPaused_; }
    size_t GetTotalBytesIn() const { return totalBytesIn_; }
    size_t GetTotalBytesOut() const { return totalBytesOut_; }
    size_t GetPaddingBytesInForTest() const { return paddingBytesIn_; }
    const SeProcessStats &GetStatsForTest() const { return stats_; }
    void SetStatsReportThresholdForTest(int64_t threshold) { statsReportThreshold_ = threshold; }
    void SetActiveNoDataStartForTest(std::chrono::steady_clock::time_point start) { activeNoDataStart_ = start; }
    HpaePcmBuffer *ProcessActiveNoInputForTest() { return ProcessActiveNoInput(); }
    void UpdatePrebufTargetBytesForTest() { UpdatePrebufTargetBytes(); }
    bool HandleNoDataTimeoutForTest(size_t inputLevel) { return HandleNoDataTimeout(inputLevel); }
    void SetPreheatNoDataStartTimeForTest(std::chrono::steady_clock::time_point t) { preheatNoDataStartTime_ = t; }
    void SetStopRequestedForTest(bool v) { stopRequested_ = v; }
    size_t FeedInputFromBuffer(HpaePcmBuffer *buf);
    int32_t FeedPaddingInput(size_t byteLen);
    void AccountOutput(size_t byteLen);

private:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;

    STATUS status_ = STATUS::IDLE; // Phase 3B: three-stage state machine
    bool closeCrossfadePending_ = false;
    bool preheatSuspendedForGraphSwitch_ = false;
    std::shared_ptr<IEffectInstance> effectInstance_;
    std::string effectName_;
    AudioEffectConfig audioConfig_;

    void PreheatThreadFunc();
    void OnNoDataReceived(bool &noDataTimerActive,
        std::chrono::steady_clock::time_point &noDataStartTime, uint32_t &backoffMs);
    bool HandleNoDataTimeout(size_t inputLevel);
    HpaePcmBuffer *ProcessIntercepting(const std::vector<HpaePcmBuffer *> &inputs);
    HpaePcmBuffer *ProcessActive(const std::vector<HpaePcmBuffer *> &inputs);
    HpaePcmBuffer *ProcessActiveNoInput();
    HpaePcmBuffer *ReadAlgoOutput(HpaePcmBuffer *inputBuf, size_t byteLen);
    uint64_t BytesToLatencyUs(size_t bytes) const;
    void UpdatePrebufTargetBytes();
    void RestartPreheat();
    void ResetAccounting();
    void ResetTailPaddingState();
    void ReconfigOutputBuffers(const AudioEffectConfig &config);
    void ReconfigRawBypassBuffers(const AudioEffectConfig &config);
    void ClearRawBypassBuffer();
    bool WaitRawBypassBufferWritable();
    bool PushRawBypassBuffer(HpaePcmBuffer *buf);
    HpaePcmBuffer *PopRawBypassBuffer();
    bool WriteRawBypassOutput();
    bool HandleWarmingUpRawBypass();
    HpaePcmBuffer *ProcessRawBypassOutput(HpaePcmBuffer *rawBuf);
    HpaePcmBuffer *ProcessInterceptingRaw(HpaePcmBuffer *inputBuf, bool feedInput);
    void ConsumePendingRawBypassBlocks(size_t skipRequest, uint32_t skipRemaining);

    HpaePcmBuffer *ProcessCloseCrossfade(const std::vector<HpaePcmBuffer *> &inputs);

    // Close SE core logic (RewindBuffer + clear + bypass)
    void CloseSessionEffectCore();

    // --- Phase 3C: Crossfade ---
    void ApplyInterceptingFadeIn(float *algoOutput, const float *rawInput, size_t sampleCount);
    // Close crossfade: per-frame wet→dry mix (same coefficient for all channels in a frame).
    void ApplyCrossfadeMix(float *dryOutput, const float *wetInput, size_t frameLen, size_t channels);

    // --- Phase 3B: Three-stage state machine ---
    // --- Phase 3B-2: Operation matrix ---
    bool isPaused_ = false;  // true = data flow suspended, status_ preserved
    std::thread preheatThread_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> prebufFilled_{false};
    // Warmup lifecycle timestamps retained for pause/resume bookkeeping.
    std::chrono::steady_clock::time_point warmupStartTime_;
    uint64_t pauseAccumulatedUs_ = 0;
    std::chrono::steady_clock::time_point pauseStartTime_;
    std::chrono::steady_clock::time_point preheatNoDataStartTime_;
    // --- Phase 3C: Latency tracking ---
    size_t totalBytesIn_ = 0;
    size_t totalBytesOut_ = 0;
    size_t paddingBytesIn_ = 0;
    std::optional<std::chrono::steady_clock::time_point> activeNoDataStart_;
    uint32_t activeTailPaddingFrames_ = 0;
    PcmBufferInfo tailOutputBufferInfo_;
    HpaePcmBuffer tailOutputData_;
    PcmBufferInfo rawBypassBufferInfo_;
    HpaePcmBuffer rawBypassBuffer_;
    PcmBufferInfo rawBypassOutputBufferInfo_;
    HpaePcmBuffer rawBypassOutputData_;
    bool rawBypassNeeded_ = false;
    std::vector<float> interceptingRawData_;
    std::mutex rawBypassMutex_;
    std::condition_variable rawBypassCv_;
    size_t prebufTargetBytes_ = 0;
    std::atomic<size_t> pendingRawBypassBlocks_{0};
    SeProcessStats stats_;
    int64_t statsReportThreshold_ = 48000; // 1s@48kHz
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // HPAE_SESSION_EFFECT_NODE_H
