/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef AUDIO_COMMON_CONVERTER_H
#define AUDIO_COMMON_CONVERTER_H
#include <cinttypes>
#include <vector>

namespace OHOS {
namespace AudioStandard {
class AudioCommonConverter {
public:
    static void ConvertBufferToFloat(const uint8_t *buffer, uint32_t samplePerFrame, std::vector<float> &floatBuffer,
                                     float volume);
    static void ConvertBufferTo32Bit(const uint8_t *buffer, int32_t format, int32_t *dst, size_t count, float volume);
    static void ConvertBufferTo16Bit(const uint8_t *buffer, int32_t format, int16_t *dst, size_t count, float volume);
    static void ConvertFloatToAudioBuffer(const std::vector<float> &floatBuffer, uint8_t *buffer,
                                          uint32_t samplePerFrame);
};
} // namespace AudioStandard
} // namespace OHOS
#endif
