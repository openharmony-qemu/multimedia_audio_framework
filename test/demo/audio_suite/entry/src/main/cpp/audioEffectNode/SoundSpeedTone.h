/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
 
#ifndef AUDIOEDITTESTAPP_SOUNDSPEEDTONE_H
#define AUDIOEDITTESTAPP_SOUNDSPEEDTONE_H
 
#include <string>
#include "napi/native_api.h"
#include "./EffectNode.h"
 
class SoundSpeedTone {
 
};
 
struct SoundSpeedToneParams {
    std::string inputId;
    std::string soundSpeedToneId;
    double soundSpeed;
    double soundTone;
    std::string selectedNodeId;
};
 
napi_status getSoundSpeedToneParameters(napi_env env, napi_value *argv, SoundSpeedToneParams &params);
 
Node getOrCreateSpeedToneNode(std::string& soundSpeedToneId, std::string& inputId, std::string selectedNodeId);
 
#endif //AUDIOEDITTESTAPP_SOUNDSPEEDTONE_H