/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
 
#ifndef AUDIOEDITTESTAPP_VOICEBEAUTIFIER_H
#define AUDIOEDITTESTAPP_VOICEBEAUTIFIER_H
 
#include <string>
#include "napi/native_api.h"
 
class VoiceBeautifier {};
 
int AddVBEffectNode(std::string inputId, int mode, std::string voiceBeautifierId, std::string selectNodeId);
 
int ModifyVBEffectNode(std::string inputId, int mode, std::string voiceBeautifierId);
 
napi_status getStartVBParameters(napi_env env, napi_value *argv, std::string &inputId, int &mode,
                                 std::string &voiceBeautifierId, std::string &selectNodeId);
 
napi_status getResetVBParameters(napi_env env, napi_value *argv, std::string &inputId, int &mode,
                                 std::string &voiceBeautifierId);
#endif // AUDIOEDITTESTAPP_VOICEBEAUTIFIER_H