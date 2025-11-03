/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef TEST1_AUDIOCONFIGPARAM_H
#define TEST1_AUDIOCONFIGPARAM_H

#include <cstdint>
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"

enum Class SampleFormat {
    AUDIO_SAMPLE_U8 = 8,
    AUDIO_SAMPLE_S16LE = 16,
    AUDIO_SAMPLE_S24LE = 24,
    AUDIO_SAMPLE_S32LE = 64,
    AUDIO_SAMPLE_F32LE = 32
};

enum Class AudioChannelLayout {
    CH_LAYOUT_MONO = 1,
    CH_LAYOUT_STEREO = 2,
    CH_LAYOUT_STEREO_DOWNMIX = 3
};

class AudioConfigParam {
};

OH_Audio_SampleRate SetSamplingRate(int32_t sampleRate);

OH_AudioChannelLayout SetChannelLayout(int32_t channels);

OH_Audio_SampleFormat SetSampleFormat(int32_t bitsPerSample);

#endif //TEST1_AUDIOCONFIGPARAM_H