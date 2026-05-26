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

#ifndef EFFECT_INSTANCE_H
#define EFFECT_INSTANCE_H

#include <string>
#include <cstdint>
#include "hpae_define.h"
#include "audio_effect.h"
#include "i_effect_instance.h"
#include "se_process_stats.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class EffectInstance : public IEffectInstance {
public:
    enum class EffectState { IDLE, ACTIVE, PENDING_RELEASE, ERROR };

    EffectInstance();
    ~EffectInstance();

    // Lifecycle
    int32_t Init(const std::string &effectName, const AudioEffectConfig &config);
    // Phase 3A-1: Init with injected AudioEffectLibrary
    int32_t Init(const std::string &effectName, const AudioEffectConfig &config,
                 const AudioEffectLibrary *lib);
    int32_t Release() override;

    // State
    EffectState GetState() const;
    void SetState(EffectState state);
    void MarkPendingRelease();

    // Binding info
    uint32_t GetBoundSessionId() const;
    void SetBoundSessionId(uint32_t sessionId);
    const std::string &GetEffectName() const override;
    const AudioEffectConfig &GetAudioConfig() const override;

    // Lazy Release control
    uint32_t GetReleaseGeneration() const;
    uint64_t GetPendingTaskId() const;
    void SetPendingTaskId(uint64_t taskId);

    // Phase 1 stubs (Phase 3: real algorithm integration)
    int32_t FeedInput(const uint8_t *data, size_t len) override;
    int32_t ReadOutput(uint8_t *buf, size_t len, uint32_t *skipCount = nullptr) override;
    size_t GetInputLevel() const override;
    void ClearBuffers() override;
    int32_t StartEffect() override;
    int32_t StopEffect() override;
    int32_t TriggerProcess() override;
    int32_t Flush() override;
    int32_t SetParameter(int32_t type, float value) override;
    uint32_t GetPreheatFrames() override;
    const SeProcessStats &GetStatsForTest() const { return stats_; }
    void SetStatsReportThresholdForTest(int64_t t) { statsReportThreshold_ = t; }

private:
    bool SkipStaleFrames(float *buf, size_t floatLen, uint32_t *skipCount);

    EffectState state_ = EffectState::IDLE;
    uint32_t boundSessionId_ = 0;
    std::string effectName_;
    AudioEffectConfig audioConfig_;
    uint32_t releaseGeneration_ = 0;
    uint64_t pendingTaskId_ = 0;
    AudioEffectHandle effectHandle_ = nullptr;
    AudioEffectLibrary effectLib_ = {};
    bool hasHandle_ = false;
    uint32_t preheatFrames_ = 0;
    bool preheatFramesCached_ = false;
    bool isEffectStarted_ = false;
    size_t fedBytes_ = 0;
    SeProcessStats stats_;
    int64_t statsReportThreshold_ = 48000;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // EFFECT_INSTANCE_H
