/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_EQUALIZER_H
#define AUDIOEDITTESTAPP_EQUALIZER_H

#include <string>
#include "napi/native_api.h"
#include "ohaudiosuite/native_audio_suite_base.h"
#include "../NodeManager.h"
#include "./EffectNode.h"

enum {
    EQ_DEFAULT = 1,
    EQ_BALLADS = 2,
    EQ_CHINESE_STYLE = 3,
    EQ_CLASSICAL = 4,
    EQ_DANCE_MUSIC = 5,
    EQ_JAZZ = 6,
    EQ_POP = 7,
    EQ_RB = 8,
    EQ_ROCK = 9,
};

struct EqBandGainsParams {
    std::string equalizerId;
    std::string inputId;
    std::string selectedNodeId;
};

OH_EqualizerFrequencyBandGains SetEqualizerMode(int32_t equalizerMode);

napi_status GetEqModeParameters(napi_env env, napi_callback_info info, unsigned int &equalizerMode,
    std::string &equalizerId, std::string &inputId);

napi_status GetEqBandGainsParameters(napi_env env, napi_callback_info info,
    OH_EqualizerFrequencyBandGains &frequencyBandGains, EqBandGainsParams &params);

Node GetOrCreateEqualizerNodeByMode(std::string& equalizerId, std::string& inputId);

Node GetOrCreateEqualizerNodeByGains(std::string& equalizerId, std::string& inputId, std::string& selectedNodeId);

#endif // AUDIOEDITTESTAPP_EQUALIZER_H