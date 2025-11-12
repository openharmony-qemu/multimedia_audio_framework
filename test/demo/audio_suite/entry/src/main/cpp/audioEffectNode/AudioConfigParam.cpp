/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "AudioConfigParam.h"

// 设置采样率
OH_Audio_SampleRate SetSamplingRate(int32_t sampleRate)
{
    switch (sampleRate) {
        case OH_Audio_SampleRate::SAMPLE_RATE_8000:
            return OH_Audio_SampleRate::SAMPLE_RATE_8000;
        case OH_Audio_SampleRate::SAMPLE_RATE_11025:
            return OH_Audio_SampleRate::SAMPLE_RATE_11025;
        case OH_Audio_SampleRate::SAMPLE_RATE_12000:
            return OH_Audio_SampleRate::SAMPLE_RATE_12000;
        case OH_Audio_SampleRate::SAMPLE_RATE_16000:
            return OH_Audio_SampleRate::SAMPLE_RATE_16000;
        case OH_Audio_SampleRate::SAMPLE_RATE_22050:
            return OH_Audio_SampleRate::SAMPLE_RATE_22050;
        case OH_Audio_SampleRate::SAMPLE_RATE_24000:
            return OH_Audio_SampleRate::SAMPLE_RATE_24000;
        case OH_Audio_SampleRate::SAMPLE_RATE_32000:
            return OH_Audio_SampleRate::SAMPLE_RATE_32000;
        case OH_Audio_SampleRate::SAMPLE_RATE_44100:
            return OH_Audio_SampleRate::SAMPLE_RATE_44100;
        case OH_Audio_SampleRate::SAMPLE_RATE_48000:
            return OH_Audio_SampleRate::SAMPLE_RATE_48000;
        case OH_Audio_SampleRate::SAMPLE_RATE_64000:
            return OH_Audio_SampleRate::SAMPLE_RATE_64000;
        case OH_Audio_SampleRate::SAMPLE_RATE_88200:
            return OH_Audio_SampleRate::SAMPLE_RATE_88200;
        case OH_Audio_SampleRate::SAMPLE_RATE_96000:
            return OH_Audio_SampleRate::SAMPLE_RATE_96000;
        case OH_Audio_SampleRate::SAMPLE_RATE_176400:
            return OH_Audio_SampleRate::SAMPLE_RATE_176400;
        case OH_Audio_SampleRate::SAMPLE_RATE_192000:
            return OH_Audio_SampleRate::SAMPLE_RATE_192000;
        default:
            return OH_Audio_SampleRate::SAMPLE_RATE_48000;
    }
}

// 设置声道
OH_AudioChannelLayout SetChannelLayout(int32_t channels)
{
    OH_AudioChannelLayout audioChannelLayout;
    switch (channels) {
        case static_cast<int>(AudioChannelLayout::CH_LAYOUT_MONO):
            audioChannelLayout = CH_LAYOUT_MONO;
            break;
        case static_cast<int>(AudioChannelLayout::CH_LAYOUT_STEREO):
            audioChannelLayout = CH_LAYOUT_STEREO;
            break;
        default:
            audioChannelLayout = CH_LAYOUT_STEREO_DOWNMIX;
            break;
    }
    return audioChannelLayout;
}

// 设置位深
OH_Audio_SampleFormat SetSampleFormat(int32_t bitsPerSample)
{
    OH_Audio_SampleFormat audioSampleFormat;
    switch (bitsPerSample) {
        case static_cast<int>(SampleFormat::AUDIO_SAMPLE_U8):
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_U8;
            break;
        case static_cast<int>(SampleFormat::AUDIO_SAMPLE_S16LE):
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
            break;
        case static_cast<int>(SampleFormat::AUDIO_SAMPLE_S24LE):
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE;
            break;
        case static_cast<int>(SampleFormat::AUDIO_SAMPLE_F32LE):
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;
            break;
        default:
            audioSampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE;
            break;
    }
    return audioSampleFormat;
}