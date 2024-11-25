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
constexpr float SCALE = 1 << (AUDIO_SAMPLE_16BIT_LENGTH - 1);

inline float GetVolumeStep(const BufferBaseInfo &bufferInfo)
{
    if (bufferInfo.frameSize <= 0) {
        return 0.f;
    }
    float volStep = (bufferInfo.volumeEd - bufferInfo.volumeBg) * bufferInfo.channelCount / bufferInfo.frameSize;
    return volStep;
}

inline float GetVolume(float volStep, int32_t frameIndex, float volumeBg)
{
    float vol = volStep * frameIndex + volumeBg;
    return vol;
}

inline void CopyFromU8ToS32(const uint8_t *buffer, int32_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    dst += bufferInfo.frameSize;
    buffer += bufferInfo.frameSize;
    for (; frameCount > 0; --frameCount) {
        float vol = GetVolume(volStep, frameCount, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            *--dst = (((int32_t)(*--buffer) - 0x80) << AUDIO_SAMPLE_24BIT_LENGTH) * vol;
        }
    }
}

inline void CopyFromS16ToS32(const int16_t *buffer, int32_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    dst += bufferInfo.frameSize;
    buffer += bufferInfo.frameSize;
    for (; frameCount > 0; --frameCount) {
        float vol = GetVolume(volStep, frameCount, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            *--dst = (((int32_t) * --buffer) << AUDIO_SAMPLE_16BIT_LENGTH) * vol;
        }
    }
}

static void CopyFrom24ToS32(const uint8_t *buffer, int32_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    dst += bufferInfo.frameSize;
    buffer += bufferInfo.frameSize * AUDIO_24BIT_LENGTH;
    for (; frameCount > 0; --frameCount) {
        float vol = GetVolume(volStep, frameCount, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            buffer -= AUDIO_24BIT_LENGTH;
            *--dst = ((buffer[0] << BYTES_ALIGNMENT_SIZE) | (buffer[1] << AUDIO_SAMPLE_16BIT_LENGTH) |
                (buffer[AUDIO_NUMBER_2] << AUDIO_SAMPLE_24BIT_LENGTH)) * vol;
        }
    }
}

inline void CopyFromS32ToS32(const int32_t *buffer, int32_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    dst += bufferInfo.frameSize;
    buffer += bufferInfo.frameSize;
    for (; frameCount > 0; --frameCount) {
        float vol = GetVolume(volStep, frameCount, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            *--dst = (*--buffer * vol);
        }
    }
}

inline void CopyFromF32ToS32(const float *buffer, int32_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    for (int32_t j = 0; j < frameCount; j++) {
        float vol = GetVolume(volStep, j + 1, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            *dst++ = *buffer++ * vol * AUDIO_SAMPLE_32BIT_VALUE;
        }
    }
}

void AudioCommonConverter::ConvertBufferTo32Bit(const BufferBaseInfo &srcBuffer, std::vector<char> &dstBuffer)
{
    if (srcBuffer.frameSize != (dstBuffer.size() / sizeof(int32_t))) {
        return;
    }
    uint8_t *buffer = srcBuffer.buffer;
    float volumeStep = GetVolumeStep(srcBuffer);
    int32_t *dst = reinterpret_cast<int32_t *>(dstBuffer.data());
    switch (srcBuffer.format) {
        case AUDIO_SAMPLE_FORMAT_8BIT:
            CopyFromU8ToS32(buffer, dst, volumeStep, srcBuffer);
            break;
        case AUDIO_SAMPLE_FORMAT_16BIT: {
            const int16_t *src = reinterpret_cast<const int16_t *>(buffer);
            CopyFromS16ToS32(src, dst, volumeStep, srcBuffer);
            break;
        }
        case AUDIO_SAMPLE_FORMAT_24BIT:
            CopyFrom24ToS32(buffer, dst, volumeStep, srcBuffer);
            break;
        case AUDIO_SAMPLE_FORMAT_32BIT: {
            const int32_t *src = reinterpret_cast<const int32_t *>(buffer);
            CopyFromS32ToS32(src, dst, volumeStep, srcBuffer);
            break;
        }
        case AUDIO_SAMPLE_FORMAT_32F_BIT: {
            const float *src = reinterpret_cast<const float *>(buffer);
            CopyFromF32ToS32(src, dst, volumeStep, srcBuffer);
            break;
        }
        default:
            break;
    }
}

inline void CopyFromU8ToS16(const uint8_t *buffer, int16_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    dst += bufferInfo.frameSize;
    buffer += bufferInfo.frameSize;
    for (; frameCount > 0; --frameCount) {
        float vol = GetVolume(volStep, frameCount, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            *--dst = (((int16_t)(*--buffer) - 0x80) << BYTES_ALIGNMENT_SIZE) * vol;
        }
    }
}

inline void CopyFromS16ToS16(const int16_t *buffer, int16_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    dst += bufferInfo.frameSize;
    buffer += bufferInfo.frameSize;
    for (; frameCount > 0; --frameCount) {
        float vol = GetVolume(volStep, frameCount, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            *--dst = (*--buffer * vol);
        }
    }
}

inline void CopyFrom24ToS16(const uint8_t *buffer, int16_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    dst += bufferInfo.frameSize;
    buffer += bufferInfo.frameSize * AUDIO_24BIT_LENGTH;
    for (; frameCount > 0; --frameCount) {
        float vol = GetVolume(volStep, frameCount, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            buffer -= AUDIO_24BIT_LENGTH;
            *--dst = ((buffer[1]) | (buffer[AUDIO_NUMBER_2] << BYTES_ALIGNMENT_SIZE)) * vol;
        }
    }
}

inline void CopyFromS32ToS16(const int32_t *buffer, int16_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    dst += bufferInfo.frameSize;
    buffer += bufferInfo.frameSize;
    for (; frameCount > 0; --frameCount) {
        float vol = GetVolume(volStep, frameCount, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            *--dst = ((*--buffer >> AUDIO_SAMPLE_16BIT_LENGTH) * vol);
        }
    }
}

inline void CopyFromF32ToS16(const float *buffer, int16_t *dst, float volStep, const BufferBaseInfo &bufferInfo)
{
    int32_t frameCount = bufferInfo.frameSize / bufferInfo.channelCount;
    for (int32_t j = 0; j < frameCount; j++) {
        float vol = GetVolume(volStep, j + 1, bufferInfo.volumeBg);
        for (uint32_t i = 0; i < bufferInfo.channelCount; i++) {
            *dst++ = *buffer++ * SCALE * vol;
        }
    }
}

void AudioCommonConverter::ConvertBufferTo16Bit(const BufferBaseInfo &srcBuffer, std::vector<char> &dstBuffer)
{
    if (srcBuffer.frameSize != (dstBuffer.size() / sizeof(int16_t))) {
        return;
    }
    int16_t *dst = reinterpret_cast<int16_t *>(dstBuffer.data());
    uint8_t *buffer = srcBuffer.buffer;
    float volumeStep = GetVolumeStep(srcBuffer);
    switch (srcBuffer.format) {
        case AUDIO_SAMPLE_FORMAT_8BIT:
            CopyFromU8ToS16(buffer, dst, volumeStep, srcBuffer);
            break;
        case AUDIO_SAMPLE_FORMAT_16BIT: {
            const int16_t *src = reinterpret_cast<const int16_t *>(buffer);
            CopyFromS16ToS16(src, dst, volumeStep, srcBuffer);
            break;
        }
        case AUDIO_SAMPLE_FORMAT_24BIT:
            CopyFrom24ToS16(buffer, dst, volumeStep, srcBuffer);
            break;
        case AUDIO_SAMPLE_FORMAT_32BIT: {
            const int32_t *src = reinterpret_cast<const int32_t *>(buffer);
            CopyFromS32ToS16(src, dst, volumeStep, srcBuffer);
            break;
        }
        case AUDIO_SAMPLE_FORMAT_32F_BIT: {
            const float *src = reinterpret_cast<const float *>(buffer);
            CopyFromF32ToS16(src, dst, volumeStep, srcBuffer);
            break;
        }
        default:
            break;
    }
}

inline int32_t ConvertS24ToFloat(const uint8_t *buffer, int32_t index, int32_t format)
{
    int32_t sampleValue = 0;
    sampleValue = ((buffer[index * format + AUDIO_NUMBER_2] & 0xff) << AUDIO_SAMPLE_24BIT_LENGTH) |
        ((buffer[index * format + 1] & 0xff) << AUDIO_SAMPLE_16BIT_LENGTH) |
        ((buffer[index * format] & 0xff) << BYTES_ALIGNMENT_SIZE);
    return sampleValue;
}

void AudioCommonConverter::ConvertFloatToFloatWithVolume(const BufferBaseInfo &srcBuffer,
                                                         std::vector<float> &floatBuffer)
{
    if (srcBuffer.frameSize != floatBuffer.size()) {
        return;
    }
    float *buffer = reinterpret_cast<float *>(srcBuffer.buffer);
    float volumeStep = GetVolumeStep(srcBuffer);
    uint32_t frameCount = floatBuffer.size() / srcBuffer.channelCount;
    for (uint32_t i = 0; i < frameCount; i++) {
        float volume = GetVolume(volumeStep, i + 1, srcBuffer.volumeBg);
        for (uint32_t j = 0; j < srcBuffer.channelCount; j++) {
            uint32_t index = i * srcBuffer.channelCount + j;
            floatBuffer[index] = buffer[index] * volume;
        }
    }
}

void AudioCommonConverter::ConvertBufferToFloat(const BufferBaseInfo &srcBuffer, std::vector<float> &floatBuffer)
{
    if (srcBuffer.frameSize != floatBuffer.size()) {
        return;
    }
    uint8_t *buffer = srcBuffer.buffer;
    float volumeStep = GetVolumeStep(srcBuffer);
    uint32_t convertValue = srcBuffer.samplePerFrame * BYTES_ALIGNMENT_SIZE - 1;
    uint32_t frameCount = floatBuffer.size() / srcBuffer.channelCount;
    for (uint32_t i = 0; i < frameCount; i++) {
        float volume = GetVolume(volumeStep, i + 1, srcBuffer.volumeBg);
        for (uint32_t j = 0; j < srcBuffer.channelCount; j++) {
            int32_t sampleValue = 0;
            uint32_t index = i * srcBuffer.channelCount + j;
            if (srcBuffer.samplePerFrame == AUDIO_24BIT_LENGTH) {
                sampleValue = ConvertS24ToFloat(buffer, static_cast<int32_t>(index), srcBuffer.samplePerFrame);
                floatBuffer[index] = sampleValue * volume * (1.0f / AUDIO_SAMPLE_32BIT_VALUE);
                continue;
            }
            for (uint32_t k = 0; k < srcBuffer.samplePerFrame; k++) {
                sampleValue |= (buffer[index * srcBuffer.samplePerFrame + k] & 0xff) << (k * BYTES_ALIGNMENT_SIZE);
            }
            floatBuffer[index] = sampleValue * volume * (1.0f / (1U << convertValue));
        }
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
