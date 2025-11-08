/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "Equalizer.h"

// 封装入参 OH_EqualizerMode
OH_EqualizerFrequencyBandGains SetEqualizerMode(int32_t equailizerMode)
{
    OH_EqualizerFrequencyBandGains eqMode;
    switch (equailizerMode) {
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_DEFAULT):
            eqMode = OH_EQUALIZER_PARAM_DEFAULT;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_BALLADS):
            eqMode = OH_EQUALIZER_PARAM_BALLADS;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_CHINESE_STYLE):
            eqMode = OH_EQUALIZER_PARAM_CHINESE_STYLE;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_CLASSICAL):
            eqMode = OH_EQUALIZER_PARAM_CLASSICAL;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_DANCE_MUSIC):
            eqMode = OH_EQUALIZER_PARAM_DANCE_MUSIC;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_JAZZ):
            eqMode = OH_EQUALIZER_PARAM_JAZZ;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_POP):
            eqMode = OH_EQUALIZER_PARAM_POP;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_RB):
            eqMode = OH_EQUALIZER_PARAM_RB;
            break;
        case static_cast<int>(EqualizerFrequencyBandGains::EQUALIZER_PARAM_ROCK):
            eqMode = OH_EQUALIZER_PARAM_ROCK;
            break;
        default:
            eqMode = OH_EQUALIZER_PARAM_DEFAULT;
            break;
    }
    return eqMode;
}