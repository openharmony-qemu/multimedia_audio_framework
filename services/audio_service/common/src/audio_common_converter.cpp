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
#include "audio_common_converter.h"
#include <cmath>

namespace OHOS {
namespace AudioStandard {
constexpr float AUDIO_SAMPLE_32BIT_VALUE = 2147483647.f;
constexpr int32_t BYTES_ALIGNMENT_SIZE = 8;
constexpr int32_t AUDIO_24BIT_LENGTH = 3;
constexpr int32_t AUDIO_SAMPLE_FORMAT_8BIT = 0;
constexpr int32_t AUDIO_SAMPLE_FORMAT_16BIT = 1;
constexpr int32_t AUDIO_SAMPLE_FORMAT_24BIT = 2;
constexpr int32_t AUDIO_SAMPLE_FORMAT_32BIT = 3;
constexpr int32_t AUDIO_SAMPLE_FORMAT_32F_BIT = 4;
constexpr int32_t AUDIO_SAMPLE_24BIT_LENGTH = 24;
constexpr int32_t AUDIO_SAMPLE_16BIT_LENGTH = 16;
constexpr int32_t AUDIO_NUMBER_2 = 2;

void AudioCommonConverter::ConvertBufferTo32Bit(const uint8_t *buffer, int32_t format, int32_t *dst, size_t count,
                                                float volume)
{
    switch (format) {
        case AUDIO_SAMPLE_FORMAT_8BIT:
            dst += count;
            buffer += count;
            for (; count > 0; --count) {
                *--dst = ((int32_t)(*--buffer) - 0x80) << AUDIO_SAMPLE_24BIT_LENGTH;
            }
            break;
        case AUDIO_SAMPLE_FORMAT_16BIT: {
            const int16_t *src = reinterpret_cast<const int16_t *>(buffer);
            dst += count;
            src += count;
            for (; count > 0; --count) {
                *--dst = (int32_t)((*--src << AUDIO_SAMPLE_16BIT_LENGTH) * volume);
            }
            break;
        }
        case AUDIO_SAMPLE_FORMAT_24BIT:
            dst += count;
            buffer += count * AUDIO_24BIT_LENGTH;
            for (; count > 0; --count) {
                buffer -= AUDIO_24BIT_LENGTH;
                *--dst = ((buffer[0] << BYTES_ALIGNMENT_SIZE) | (buffer[1] << AUDIO_SAMPLE_16BIT_LENGTH) |
                          (buffer[AUDIO_NUMBER_2] << AUDIO_SAMPLE_24BIT_LENGTH)) *
                         volume;
            }
            break;
        case AUDIO_SAMPLE_FORMAT_32BIT: {
            const int32_t *src = reinterpret_cast<const int32_t *>(buffer);
            dst += count;
            src += count;
            for (; count > 0; --count) {
                *--dst = (*--src * volume);
            }
            break;
        }
        case AUDIO_SAMPLE_FORMAT_32F_BIT: {
            const float *src = reinterpret_cast<const float *>(buffer);
            for (; count > 0; --count) {
                *dst++ = *src++ * volume * AUDIO_SAMPLE_32BIT_VALUE;
            }
            break;
        }
        default:
            break;
    }
}

void AudioCommonConverter::ConvertBufferTo16Bit(const uint8_t *buffer, int32_t format, int16_t *dst, size_t count,
                                                float volume)
{
    switch (format) {
        case AUDIO_SAMPLE_FORMAT_8BIT:
            dst += count;
            buffer += count;
            for (; count > 0; --count) {
                *--dst = ((int16_t)(*--buffer) - 0x80) << BYTES_ALIGNMENT_SIZE;
            }
            break;
        case AUDIO_SAMPLE_FORMAT_16BIT: {
            const int16_t *src = reinterpret_cast<const int16_t *>(buffer);
            dst += count;
            src += count;
            for (; count > 0; --count) {
                *--dst = (*--src * volume);
            }
            break;
        }
        case AUDIO_SAMPLE_FORMAT_24BIT:
            dst += count;
            buffer += count * AUDIO_24BIT_LENGTH;
            for (; count > 0; --count) {
                *dst++ = ((buffer[1]) | (buffer[AUDIO_NUMBER_2] << BYTES_ALIGNMENT_SIZE)) * volume;
                buffer += AUDIO_24BIT_LENGTH;
            }
            break;
        case AUDIO_SAMPLE_FORMAT_32BIT: {
            const int32_t *src = reinterpret_cast<const int32_t *>(buffer);
            dst += count;
            src += count;
            for (; count > 0; --count) {
                *--dst = ((*--src >> AUDIO_SAMPLE_16BIT_LENGTH) * volume);
            }
            break;
        }
        case AUDIO_SAMPLE_FORMAT_32F_BIT: {
            const float *src = reinterpret_cast<const float *>(buffer);
            static const float scale = 1 << (AUDIO_SAMPLE_16BIT_LENGTH - 1);
            for (; count > 0; --count) {
                *dst++ = *src++ * scale * volume;
            }
            break;
        }
        default:
            break;
    }
}

void AudioCommonConverter::ConvertBufferToFloat(const uint8_t *buffer, uint32_t samplePerFrame,
                                                std::vector<float> &floatBuffer, float volume)
{
    uint32_t convertValue = samplePerFrame * BYTES_ALIGNMENT_SIZE - 1;
    for (uint32_t i = 0; i < floatBuffer.size(); i++) {
        int32_t sampleValue = 0;
        if (samplePerFrame == AUDIO_24BIT_LENGTH) {
            sampleValue = ((buffer[i * samplePerFrame + AUDIO_NUMBER_2] & 0xff) << AUDIO_SAMPLE_24BIT_LENGTH) |
                          ((buffer[i * samplePerFrame + 1] & 0xff) << AUDIO_SAMPLE_16BIT_LENGTH) |
                          ((buffer[i * samplePerFrame] & 0xff) << BYTES_ALIGNMENT_SIZE);
            floatBuffer[i] = sampleValue * volume * (1.0f / AUDIO_SAMPLE_32BIT_VALUE);
            continue;
        }
        for (uint32_t j = 0; j < samplePerFrame; j++) {
            sampleValue |= (buffer[i * samplePerFrame + j] & 0xff) << (j * BYTES_ALIGNMENT_SIZE);
        }
        floatBuffer[i] = sampleValue * volume * (1.0f / (1U << convertValue));
    }
}

void AudioCommonConverter::ConvertFloatToAudioBuffer(const std::vector<float> &floatBuffer, uint8_t *buffer,
                                                     uint32_t samplePerFrame)
{
    uint32_t convertValue = samplePerFrame * BYTES_ALIGNMENT_SIZE - 1;
    for (uint32_t i = 0; i < floatBuffer.size(); i++) {
        int32_t sampleValue = static_cast<int32_t>(floatBuffer[i] * std::pow(AUDIO_NUMBER_2, convertValue));
        for (uint32_t j = 0; j < samplePerFrame; j++) {
            uint8_t tempValue = (sampleValue >> (BYTES_ALIGNMENT_SIZE * j)) & 0xff;
            buffer[samplePerFrame * i + j] = tempValue;
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS
