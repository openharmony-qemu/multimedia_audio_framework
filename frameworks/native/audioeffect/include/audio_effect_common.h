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

#ifndef AUDIO_EFFECT_COMMON_H
#define AUDIO_EFFECT_COMMON_H

#include <cstdint>

namespace OHOS {
namespace AudioStandard {
struct AudioEnhanceDeviceAttr {
    uint32_t micRate { 0 };
    uint32_t micChannels { 0 };
    uint32_t micFormat { 0 };
    bool needEc { false };
    uint32_t ecRate { 0 };
    uint32_t ecChannels { 0 };
    uint32_t ecFormat { 0 };
    bool needMicRef { false };
    uint32_t micRefRate { 0 };
    uint32_t micRefChannels { 0 };
    uint32_t micRefFormat { 0 };
};

struct EnhanceTransBuffer {
    void *ecData { nullptr };
    void *micData { nullptr };
    void *micRefData { nullptr };
    size_t ecDataLen { 0 };
    size_t micDataLen { 0 };
    size_t micRefDataLen { 0 };
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_EFFECT_COMMON_H