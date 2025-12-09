/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
 
#include "SoundSpeedTone.h"
#include "./utils/Utils.h"
#include "hilog/log.h"
 
const int GLOBAL_RESMGR = 0xFF00;
const char *SOUND_SPEED_TONE_TAG = "[AudioEditTestApp_SoundSpeedTone_cpp]";
 
napi_status GetSoundSpeedToneParameters(napi_env env, napi_value *argv, SoundSpeedToneParams &params)
{
    napi_status status = ParseNapiString(env, argv[ARG_0], params.inputId);
    status = ParseNapiString(env, argv[ARG_1], params.soundSpeedToneId);
    status = napi_get_value_double(env, argv[ARG_2], &params.soundSpeed);
    status = napi_get_value_double(env, argv[ARG_3], &params.soundTone);
    napi_valuetype valueType;
    napi_typeof(env, argv[ARG_4], &valueType);
    if (valueType != napi_null && valueType != napi_undefined) {
        status = ParseNapiString(env, argv[ARG_4], params.selectedNodeId);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SOUND_SPEED_TONE_TAG,
        "soundSpeedToneId: %{public}s, soundSpeed: %{public}f, soundTone: %{public}f",
        params.soundSpeedToneId.c_str(), params.soundSpeed, params.soundTone);
    return status;
}
 
Node GetOrCreateSpeedToneNode(std::string& soundSpeedToneId, std::string& inputId, std::string selectedNodeId)
{
    Node eqNode = g_nodeManager->GetNodeById(soundSpeedToneId);
    if (!eqNode.physicalNode) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SOUND_SPEED_TONE_TAG,
            "audioEditTest GetOrCreateSpeedToneNode create node");
        eqNode.id = soundSpeedToneId;
        eqNode.type = OH_AudioNode_Type::EFFECT_NODE_TYPE_TEMPO_PITCH;
        g_nodeManager->createNode(soundSpeedToneId, OH_AudioNode_Type::EFFECT_NODE_TYPE_TEMPO_PITCH);
        eqNode = g_nodeManager->GetNodeById(soundSpeedToneId);
        
        if (selectedNodeId.empty()) {
            int32_t resultInt = AddEffectNodeToNodeManager(inputId, soundSpeedToneId);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SOUND_SPEED_TONE_TAG,
                         "audioEditTest addEffectNodeToNodeManager result: %{public}d", resultInt);
            if (resultInt != 0) {
                eqNode.physicalNode = nullptr; // Marked as failed
            }
        } else {
            int resultInt = g_nodeManager->insertNode(soundSpeedToneId, selectedNodeId, Direction::LATER);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SOUND_SPEED_TONE_TAG,
                         "audioEditTest insertNode insertNode result: %{public}d", resultInt);
            if (resultInt != 0) {
                eqNode.physicalNode = nullptr; // Marked as failed
            }
        }
    }
    return eqNode;
}