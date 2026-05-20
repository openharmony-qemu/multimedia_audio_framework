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

#ifndef I_EFFECT_INSTANCE_H
#define I_EFFECT_INSTANCE_H

#include <cstdint>
#include <string>
#include "audio_effect.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

// Abstract interface for effect algorithm instances.
// Node layer depends on this interface; concrete EffectInstance lives in manager layer.
class IEffectInstance {
public:
    virtual ~IEffectInstance() = default;

    // Lifecycle
    virtual int32_t Release() = 0;

    // Query
    virtual const std::string &GetEffectName() const = 0;
    virtual const AudioEffectConfig &GetAudioConfig() const = 0;

    // Buffer operations (Phase 2: interface alignment; Phase 3: real implementation)
    virtual int32_t FeedInput(const uint8_t *data, size_t len) = 0;
    virtual int32_t ReadOutput(uint8_t *buf, size_t len, uint32_t *skipCount = nullptr) = 0;
    virtual size_t GetInputLevel() const = 0;
    virtual void ClearBuffers() = 0;

    // Effect control (Phase 3A-1: sends ENABLE/DISABLE to algorithm)
    virtual int32_t StartEffect() = 0;
    virtual int32_t StopEffect() = 0;

    // Control
    virtual int32_t TriggerProcess() = 0;
    virtual int32_t Flush() = 0;
    virtual int32_t SetParameter(int32_t type, float value) = 0;

    // Preheat frames — 查询算法预热帧数（单位：DoProcess帧，1帧=20ms@48kHz）
    virtual uint32_t GetPreheatFrames() = 0;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // I_EFFECT_INSTANCE_H
