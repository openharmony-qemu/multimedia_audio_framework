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

#ifndef MOCK_ENHANCE_H
#define MOCK_ENHANCE_H

#include "audio_enhance_chain.h"

namespace OHOS {
namespace AudioStandard {
struct EffectContextTest {
    AudioEffectInterface *interface { nullptr };
    uint64_t frameIdx { 0 };
};

void ClearCommandRetMap(void);

void SetCommandRet(uint32_t cmdCode, int32_t cmdRet);

int32_t ProcessTest(AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer);

int32_t CommandTest(AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
    AudioEffectTransInfo *replyInfo);

bool CheckEffectTest(const AudioEffectDescriptor descriptor);

int32_t CreateEffectTestSucc(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle);

int32_t CreateEffectTestFail(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle);

int32_t ReleaseEffectTest(AudioEffectHandle handle);
} // AudioStandard
} // OHOS

#endif // MOCK_ENHANCE_H