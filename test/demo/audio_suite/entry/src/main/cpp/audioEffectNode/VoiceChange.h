/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_VOICE_CHANGE_H
#define AUDIOEDITTESTAPP_VOICE_CHANGE_H
#include "./utils/Utils.h"
#include "../NodeManager.h"

class VoiceChange {
};

class PureVoiceChangeParam {
public:
    std::string inputId;
    std::string effectNodeId;
    std::string selectedNodeId;
    int gender;
    float pitch;
    int optionType;
};

napi_value StartGeneralVoiceChange(napi_env env, napi_callback_info info);

napi_value ResetGeneralVoiceChange(napi_env env, napi_callback_info info);

napi_value StartPureVoiceChange(napi_env env, napi_callback_info info);

napi_value ResetPureVoiceChange(napi_env env, napi_callback_info info);

#endif //AUDIOEDITTESTAPP_VoiceChange_H
