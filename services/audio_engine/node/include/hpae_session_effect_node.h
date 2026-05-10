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

    explicit HpaeSessionEffectNode(HpaeNodeInfo &nodeInfo);
    ~HpaeSessionEffectNode() override;

    // Effect lifecycle — node receives pre-created instances from manager layer
    int32_t CreateEffectWithInstance(const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config);
    int32_t ReleaseEffect();
    int32_t DropEffect();  // Release node reference without destroying algorithm instance

    // Query
    std::shared_ptr<IEffectInstance> GetEffectInstance() const;
    std::string GetEffectName() const;
    AudioEffectConfig GetAudioConfig() const;

    // Control
    int32_t StartEffect();
    int32_t StopEffect();
    int32_t FlushEffect();
    int32_t SetParameter(int32_t type, float value);

    // Close SE support
    void StartCloseCrossfade();
    bool IsCloseCrossfadePending() const;

    // Pause: preserves caches, does NOT send DISABLE to algorithm
    int32_t PauseEffect();
    // Prepare for MoveStream: RewindBuffer + clear caches (manager handles node deletion)
    void PrepareMoveStream();

    // HpaePluginNode override
    uint64_t GetLatency(uint32_t sessionId = 0) override;

    // DoProcess override for WARMING_UP disconnect
    void DoProcess() override;
    STATUS GetStatus() const { return status_; }

protected:
    virtual void StartPreheatThread();
    void SetPrebufFilledForTest(bool filled) { prebufFilled_.store(filled); }
    bool GetPrebufFilled() const { return prebufFilled_.load(); }
    void SetStatusForTest(STATUS s) { status_ = s; }
    size_t GetPrebufTargetBytes() const;  // TEMP diag
    bool IsPaused() const { return isPaused_; }
    uint32_t GetSilenceFrameCount() const { return silenceFrameCount_; }
    size_t GetTotalBytesIn() const { return totalBytesIn_; }
    size_t GetTotalBytesOut() const { return totalBytesOut_; }
    const SeProcessStats &GetStatsForTest() const { return stats_; }
    void SetStatsReportThresholdForTest(int64_t threshold) { statsReportThreshold_ = threshold; }

private:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;

    STATUS status_ = STATUS::IDLE; // Phase 3B: three-stage state machine
    bool closeCrossfadePending_ = false;
    std::shared_ptr<IEffectInstance> effectInstance_;
    std::string effectName_;
    AudioEffectConfig audioConfig_;

    void PreheatThreadFunc();
    void StopPreheatThread();
    HpaePcmBuffer *ProcessIntercepting(const std::vector<HpaePcmBuffer *> &inputs);
    HpaePcmBuffer *ProcessActive(const std::vector<HpaePcmBuffer *> &inputs);
    void FeedInputFromBuffer(HpaePcmBuffer *buf);
    HpaePcmBuffer *ReadAlgoOutput(HpaePcmBuffer *inputBuf, size_t byteLen, size_t channels);

    HpaePcmBuffer *ProcessCloseCrossfade(const std::vector<HpaePcmBuffer *> &inputs);

    // Close SE core logic (RewindBuffer + clear + bypass)
    void CloseSessionEffectCore();

    // --- Phase 3C: Crossfade ---
    void ApplyInterceptingFadeIn(float *algoOutput, size_t sampleCount);
    void ApplyCrossfadeMix(float *algoOutput, const float *rawInput, size_t sampleCount);

    // --- Phase 3B: Three-stage state machine ---
    // --- Phase 3B-2: Operation matrix ---
    bool isPaused_ = false;  // true = data flow suspended, status_ preserved
    std::thread preheatThread_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> prebufFilled_{false};
    uint32_t silenceFrameCount_ = 0; // Dynamic count of silence frames during WARMING_UP + INTERCEPTING
    // --- Phase 3C: Latency tracking ---
    size_t totalBytesIn_ = 0;
    size_t totalBytesOut_ = 0;
    size_t prebufTargetBytes_ = 0;
    SeProcessStats stats_;
    int64_t statsReportThreshold_ = 48000; // 1s@48kHz
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // HPAE_SESSION_EFFECT_NODE_H
