/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
 
#ifndef AUDIOEDITTESTAPP_VOICEBEAUTIFIER_H
#define AUDIOEDITTESTAPP_VOICEBEAUTIFIER_H
 
#include <string>
#include "napi/native_api.h"
#include "./utils/Utils.h"
 
class VoiceBeautifier {};

struct StartVBParameters {
    std::string inputId;
    int mode = -1;
    std::string voiceBeautifierId;
    std::string selectNodeId; // Optional parameter; leaving it empty means no node is specified
};
 
constexpr OH_VoiceBeautifierType TYPE_MAP[] = {
    OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR, OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_THEATRE,
    OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CD, OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_RECORDING_STUDIO};

int AddVBEffectNode(std::string& inputId, int mode, std::string& voiceBeautifierId, std::string& selectNodeId);
 
int ModifyVBEffectNode(std::string inputId, int mode, std::string voiceBeautifierId);
 
napi_status getResetVBParameters(napi_env env, napi_callback_info info, std::string &inputId, int &mode,
                                 std::string &voiceBeautifierId);
#endif // AUDIOEDITTESTAPP_VOICEBEAUTIFIER_H