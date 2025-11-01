/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef TEST1_EQUALIZER_H
#define TEST1_EQUALIZER_H

#include <cstdint>
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"

enum Class EqualizerFrequencyBandGains {
    EQUALIZER_PARAM_DEFAULT = 1,
    EQUALIZER_PARAM_BALLADS = 2,
    EQUALIZER_PARAM_CHINESE_STYLE = 3,
    EQUALIZER_PARAM_CLASSICAL = 4,
    EQUALIZER_PARAM_DANCE_MUSIC = 5,
    EQUALIZER_PARAM_JAZZ = 6,
    EQUALIZER_PARAM_POP = 7,
    EQUALIZER_PARAM_RB = 8,
    EQUALIZER_PARAM_ROCK = 9
};

class Equalizer {
};

OH_EqualizerFrequencyBandGains SetEqualizerMode(int32_t equailizerMode);

#endif //TEST1_EQUALIZER_H