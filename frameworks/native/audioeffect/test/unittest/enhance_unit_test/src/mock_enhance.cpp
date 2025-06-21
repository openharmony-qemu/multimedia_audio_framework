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

#include "mock_enhance.h"

#include <map>

#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
namespace {
std::map<uint32_t, int32_t> COMMAND_RET_MAP;
} // namespace

void ClearCommandRetMap(void)
{
    COMMAND_RET_MAP.clear();
}

void SetCommandRet(uint32_t cmdCode, int32_t cmdRet)
{
    COMMAND_RET_MAP.insert_or_assign(cmdCode, cmdRet);
}

int32_t ProcessTest(AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer)
{
    static_cast<void>(self);
    if (inBuffer->s16 == nullptr || outBuffer->s16 == nullptr || outBuffer->frameLength > inBuffer->frameLength) {
        return ERROR;
    }
    const int16_t factor = 10;
    for (uint32_t i = 0; i < outBuffer->frameLength / sizeof(int16_t); ++i) {
        outBuffer->s16[i] = inBuffer->s16[i] * factor;
    }
    return SUCCESS;
}

int32_t CommandTest(AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
    AudioEffectTransInfo *replyInfo)
{
    static_cast<void>(self);
    static_cast<void>(cmdInfo);
    static_cast<void>(replyInfo);

    if (auto iter = COMMAND_RET_MAP.find(cmdCode); iter != COMMAND_RET_MAP.end()) {
        return iter->second;
    }

    return SUCCESS;
}

bool CheckEffectTest(const AudioEffectDescriptor descriptor)
{
    static_cast<void>(descriptor);
    return true;
}

int32_t CreateEffectTestSucc(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle)
{
    static_cast<void>(descriptor);
    if (handle == nullptr) {
        return ERROR;
    }
    auto context = new (std::nothrow) EffectContextTest();
    if (context == nullptr) {
        return ERROR;
    }
    static AudioEffectInterface interface = { ProcessTest, CommandTest };
    context->interface = &interface;
    *handle = reinterpret_cast<AudioEffectHandle>(context);
    return SUCCESS;
}

int32_t CreateEffectTestFail(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle)
{
    static_cast<void>(descriptor);
    if (handle != nullptr) {
        *handle = nullptr;
        return ERROR;
    }
    return ERROR;
}

int32_t ReleaseEffectTest(AudioEffectHandle handle)
{
    if (handle == nullptr) {
        return ERROR;
    }
    auto context = reinterpret_cast<EffectContextTest *>(handle);
    delete context;
    return SUCCESS;
}
} // AudioStandard
} // OHOS