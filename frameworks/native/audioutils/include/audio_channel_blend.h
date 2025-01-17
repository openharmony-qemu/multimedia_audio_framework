/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_CHANNEL_BLEND_H
#define AUDIO_CHANNEL_BLEND_H

#include <cstdint>
#include <string>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

typedef struct {
    int8_t value[3];
} __attribute__((__packed__)) int24_t;

class AudioBlend {
public:
    AudioBlend();
    AudioBlend(ChannelBlendMode blendMode, uint8_t format, uint8_t channel);
    void Process(uint8_t *buffer, size_t bufferSize);
    void SetParams(ChannelBlendMode blendMode, uint8_t format, uint8_t channel);

private:
    static constexpr int32_t CHANNEL_ONE = 0;
    static constexpr int32_t CHANNEL_TWO = 1;
    static constexpr int32_t CHANNEL_THREE = 2;
    static constexpr int32_t CHANNEL_FOUR = 3;
    static constexpr int32_t CHANNEL_FIVE = 4;
    static constexpr int32_t CHANNEL_SIX = 5;
    static constexpr int32_t CHANNEL_SEVEN = 6;
    static constexpr int32_t CHANNEL_EIGHT = 7;
    template <typename T>
    void BlendLR(T& left, T& right);
    template <>
    void BlendLR(int24_t& left, int24_t& right);
    template <typename T>
    void ProcessBlendLRModeWithFormat(T *buffer, size_t count, AudioChannel channel);
    template <typename T>
    void ProcessAllLeftModeWithFormat(T *buffer, size_t count, AudioChannel channel);
    template <typename T>
    void ProcessAllRightModeWithFormat(T *buffer, size_t count, AudioChannel channel);
    template<typename T>
    void ProcessWithBlendMode(T *buffer, size_t bufferSize);

    ChannelBlendMode blendMode_;
    uint8_t format_;
    uint8_t channels_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_CHANNEL_BLEND_H
