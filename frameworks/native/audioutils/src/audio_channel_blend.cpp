/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioBlend"
#endif

#include <cinttypes>
#include "audio_common_log.h"
#include "audio_channel_blend.h"

namespace OHOS {
namespace AudioStandard {
AudioBlend::AudioBlend()
{
    blendMode_ = MODE_DEFAULT;
    format_ = 0;
    channels_ = 0;
}

AudioBlend::AudioBlend(ChannelBlendMode blendMode, uint8_t format, uint8_t channel)
    :blendMode_(blendMode), format_(format), channels_(channel)
{
}

void AudioBlend::SetParams(ChannelBlendMode blendMode, uint8_t format, uint8_t channel)
{
    AUDIO_DEBUG_LOG("SetParams blendMode_:%{public}d format_:%{public}d channels_:%{public}d",
        blendMode_, format_, channels_);
    blendMode_ = blendMode;
    format_ = format;
    channels_ = channel;
}

void AudioBlend::Process(uint8_t *buffer, size_t bufferSize)
{
    switch (format_) {
        case AudioSampleFormat::SAMPLE_U8:
            ProcessWithBlendMode<uint8_t>(reinterpret_cast<uint8_t*>(buffer), bufferSize);
            break;
        case AudioSampleFormat::SAMPLE_S16LE:
            ProcessWithBlendMode<int16_t>(reinterpret_cast<int16_t*>(buffer), bufferSize);
            break;
        case AudioSampleFormat::SAMPLE_S24LE:
            ProcessWithBlendMode<int24_t>(reinterpret_cast<int24_t*>(buffer), bufferSize);
            break;
        case AudioSampleFormat::SAMPLE_S32LE:
            ProcessWithBlendMode<int32_t>(reinterpret_cast<int32_t*>(buffer), bufferSize);
            break;
        default:
            break;
    }
}

template<typename T>
void AudioBlend::ProcessWithBlendMode(T *buffer, size_t bufferSize)
{
    if (channels_ == 0) {
        return;
    }

    uint32_t frameCount = 0;
    frameCount = bufferSize / (channels_ * (format_ + 1));
    switch (blendMode_) {
        case MODE_BLEND_LR:
            ProcessBlendLRModeWithFormat<T>(buffer, frameCount, (AudioChannel)channels_);
            break;
        case MODE_ALL_LEFT:
            ProcessAllLeftModeWithFormat<T>(buffer, frameCount, (AudioChannel)channels_);
            break;
        case MODE_ALL_RIGHT:
            ProcessAllRightModeWithFormat<T>(buffer, frameCount, (AudioChannel)channels_);
            break;
        default:
            break;
    }
}

template <typename T>
void AudioBlend::BlendLR(T& left, T& right)
{
    left = left / 2 + right / 2;
    right = left;
}

template <>
void AudioBlend::BlendLR(int24_t& left, int24_t& right)
{
    left.value[0] = left.value[0] / 2 + right.value[0] / 2;
    right.value[0] = left.value[0];
    left.value[1] = left.value[1] / 2 + right.value[1] / 2;
    right.value[0] = left.value[0];
    left.value[2] = left.value[2] / 2 + right.value[2] / 2;
    right.value[2] = left.value[2];
}

template <typename T>
void AudioBlend::ProcessBlendLRModeWithFormat(T *buffer, size_t count, AudioChannel channel)
{
    for (uint32_t i = count; i > 0; i--) {
        switch (channel) {
            case CHANNEL_8:
                BlendLR(buffer[CHANNEL_SEVEN], buffer[CHANNEL_EIGHT]);
                [[fallthrough]];
            case CHANNEL_7:
            case CHANNEL_6:
                BlendLR(buffer[CHANNEL_FIVE], buffer[CHANNEL_SIX]);
                BlendLR(buffer[CHANNEL_ONE], buffer[CHANNEL_TWO]);
                break;
            case CHANNEL_5:
            case CHANNEL_4:
                BlendLR(buffer[CHANNEL_THREE], buffer[CHANNEL_FOUR]);
                [[fallthrough]];
            case CHANNEL_3:
            case STEREO:
                BlendLR(buffer[CHANNEL_ONE], buffer[CHANNEL_TWO]);
                break;
            default:
                break;
        }
        buffer += (int8_t)channel;
    }
}

template <typename T>
void AudioBlend::ProcessAllLeftModeWithFormat(T *buffer, size_t count, AudioChannel channel)
{
    for (uint32_t i = count; i > 0; i--) {
        switch (channel) {
            case CHANNEL_8:
                buffer[CHANNEL_EIGHT] = buffer[CHANNEL_SEVEN];
                [[fallthrough]];
            case CHANNEL_7:
            case CHANNEL_6:
                buffer[CHANNEL_SIX] = buffer[CHANNEL_FIVE];
                buffer[CHANNEL_TWO] = buffer[CHANNEL_ONE];
                break;
            case CHANNEL_5:
            case CHANNEL_4:
                buffer[CHANNEL_FOUR] = buffer[CHANNEL_THREE];
                [[fallthrough]];
            case CHANNEL_3:
            case STEREO:
                buffer[CHANNEL_TWO] = buffer[CHANNEL_ONE];
                break;
            default:
                break;
        }
        buffer += (int8_t)channel;
    }
}

template <typename T>
void AudioBlend::ProcessAllRightModeWithFormat(T *buffer, size_t count, AudioChannel channel)
{
    for (uint32_t i = count; i > 0; i--) {
        switch (channel) {
            case CHANNEL_8:
                buffer[CHANNEL_SEVEN] = buffer[CHANNEL_EIGHT];
                [[fallthrough]];
            case CHANNEL_7:
            case CHANNEL_6:
                buffer[CHANNEL_FIVE] = buffer[CHANNEL_SIX];
                buffer[CHANNEL_ONE] = buffer[CHANNEL_TWO];
                break;
            case CHANNEL_5:
            case CHANNEL_4:
                buffer[CHANNEL_THREE] = buffer[CHANNEL_FOUR];
                [[fallthrough]];
            case CHANNEL_3:
            case STEREO:
                buffer[CHANNEL_ONE] = buffer[CHANNEL_TWO];
                break;
            default:
                break;
        }
        buffer += (int8_t)channel;
    }
}

template void AudioBlend::ProcessBlendLRModeWithFormat(uint8_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessBlendLRModeWithFormat(int16_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessBlendLRModeWithFormat(int24_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessBlendLRModeWithFormat(int32_t *buffer, size_t count, AudioChannel channel);

template void AudioBlend::ProcessAllLeftModeWithFormat(uint8_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessAllLeftModeWithFormat(int16_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessAllLeftModeWithFormat(int24_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessAllLeftModeWithFormat(int32_t *buffer, size_t count, AudioChannel channel);

template void AudioBlend::ProcessAllRightModeWithFormat(uint8_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessAllRightModeWithFormat(int16_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessAllRightModeWithFormat(int24_t *buffer, size_t count, AudioChannel channel);
template void AudioBlend::ProcessAllRightModeWithFormat(int32_t *buffer, size_t count, AudioChannel channel);
} // namespace AudioStandard
} // namespace OHOS
