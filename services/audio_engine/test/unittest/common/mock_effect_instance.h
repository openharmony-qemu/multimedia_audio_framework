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

#ifndef MOCK_EFFECT_INSTANCE_H
#define MOCK_EFFECT_INSTANCE_H

#include "gmock/gmock.h"
#include "i_effect_instance.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class MockEffectInstance : public IEffectInstance {
public:
    MOCK_METHOD(int32_t, Release, (), (override));
    MOCK_METHOD(const std::string &, GetEffectName, (), (const, override));
    MOCK_METHOD(const AudioEffectConfig &, GetAudioConfig, (), (const, override));
    MOCK_METHOD(int32_t, FeedInput, (const uint8_t *data, size_t len), (override));
    MOCK_METHOD(int32_t, ReadOutput, (uint8_t *buf, size_t len, uint32_t *skipCount), (override));
    MOCK_METHOD(size_t, GetInputLevel, (), (const, override));
    MOCK_METHOD(void, ClearBuffers, (), (override));
    MOCK_METHOD(int32_t, StartEffect, (), (override));
    MOCK_METHOD(int32_t, StopEffect, (), (override));
    MOCK_METHOD(int32_t, TriggerProcess, (), (override));
    MOCK_METHOD(int32_t, Flush, (), (override));
    MOCK_METHOD(int32_t, SetParameter, (int32_t type, float value), (override));
    MOCK_METHOD(uint32_t, GetPreheatFrames, (), (override));
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif  // MOCK_EFFECT_INSTANCE_H
