/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_EQUAILIZER_H
#define AUDIOEDITTESTAPP_EQUAILIZER_H

#include <string>
#include "napi/native_api.h"
#include "ohaudio/native_audio_suite_base.h"
#include "../NodeManager.h"
#include "./EffectNode.h"

struct EqBandGainsParams {
    std::string equailizerId;
    std::string inputId;
    std::string selectedNodeId;
};

OH_EqualizerFrequencyBandGains SetEqualizerMode(int32_t equailizerMode);

napi_status getEqModeParameters(napi_env env, napi_value *argv, unsigned int &equailizerMode, std::string &equailizerId, std::string &inputId);

napi_status getEqBandGainsParameters(napi_env env, napi_value *argv, OH_EqualizerFrequencyBandGains &frequencyBandGains, EqBandGainsParams &params);

Node getOrCreateEqualizerNodeByMode(std::string& equailizerId, std::string& inputId);

Node getOrCreateEqualizerNodeByGains(std::string& equailizerId, std::string& inputId, std::string& selectedNodeId);

#endif //AUDIOEDITTESTAPP_EQUAILIZER_H