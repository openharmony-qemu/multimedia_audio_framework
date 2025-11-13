/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */
 
#include "VoiceBeautifier.h"
#include <string>
#include "hilog/log.h"
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"
#include "NodeManager.h"
#include "audioSuiteError/AudioSuiteError.h"
#include "audioEffectNode/Equailizer.h"
#include "audioEffectNode/EffectNode.h"
#include "audioEffectNode/Input.h"
#include "audioEffectNode/Output.h"
#include "realTimePlay/RealTimePlaying.h"
#include "multiPipelineEdit/MultiPipelineEdit.h"
#include "utils/Utils.h"
 
const int GLOBAL_RESMGR = 0xFF00;
const char *VB_NODE_TAG = "[AudioEditTestApp_VoiceBeautifierNode_cpp]";

enum {
    ARG_0 = 0,
    ARG_1 = 1,
    ARG_2 = 2,
    ARG_3 = 3,
    ARG_4 = 4,
    ARG_5 = 5,
    ARG_6 = 6,
    ARG_7 = 7,
    ARG_8 = 8
};
 
int AddVBEffectNode(std::string inputId, int mode, std::string voiceBeautifierId, std::string selectNodeId)
{
    OH_VoiceBeautifierType type;
    switch (mode) {
        case 1:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR;
            break;
        case 2:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_THEATRE;
            break;
        case 3:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CD;
            break;
        case 4:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_RECORDING_STUDIO;
            break;
        default:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR;
            break;
    }
 
 
    Node node = createNodeByType(voiceBeautifierId, OH_AudioNode_Type::EFFECT_NODE_TYPE_VOICE_BEAUTIFIER);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetVoiceBeautifierType(node.physicalNode, type);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, VB_NODE_TAG,
                     "audioEditTest---startVBEffect OH_AudioSuiteEngine_SetVoiceBeautifierType ERROR!");
        return result;
    }
    int res = -1;
    if (selectNodeId.empty()) {
        res = addEffectNodeToNodeManager(inputId, voiceBeautifierId);
    } else {
        res = nodeManager->insertNode(voiceBeautifierId, selectNodeId, Direction::LATER);
    }
    if (res != 0) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, VB_NODE_TAG,
                     "audioEditTest---startVBEffect addEffectNodeToNodeManager ERROR!");
        return res;
    }
 
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, VB_NODE_TAG, "audioEditTest---startVBEffect: operation success");
    return result;
}
 
int ModifyVBEffectNode(std::string inputId, int mode, std::string voiceBeautifierId)
{
    OH_VoiceBeautifierType type;
    switch (mode) {
        case 1:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR;
            break;
        case 2:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_THEATRE;
            break;
        case 3:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CD;
            break;
        case 4:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_RECORDING_STUDIO;
            break;
        default:
            type = OH_VoiceBeautifierType::VOICE_BEAUTIFIER_TYPE_CLEAR;
            break;
    }
 
    Node node = nodeManager->GetNodeById(voiceBeautifierId);
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_SetVoiceBeautifierType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, VB_NODE_TAG,
                     "audioEditTest---OH_AudioSuiteEngine_SetVoiceBeautifierType ERROR---%{public}zd", result);
        return result;
    }
 
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, VB_NODE_TAG, "audioEditTest---resetVBEffect: operation success");
    return result;
}
 
napi_status getStartVBParameters(napi_env env, napi_value *argv, std::string &inputId, int &mode,
                                 std::string &voiceBeautifierId, std::string &selectNodeId)
{
    napi_status status = parseNapiString(env, argv[ARG_0], inputId);
    status = napi_get_value_int32(env, argv[ARG_1], &mode);
    status = parseNapiString(env, argv[ARG_2], voiceBeautifierId);
    if (!selectNodeId.empty()) {
        status = parseNapiString(env, argv[ARG_3], selectNodeId);
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, VB_NODE_TAG,
                 "audioEditTest startVBEffect inputId: %{public}s, mode: %{public}d, voiceBeautifierId: %{public}s, "
                 "selectNodeId:%{public}s",
                 inputId.c_str(), mode, voiceBeautifierId.c_str(), selectNodeId.c_str());
    return status;
}
 
napi_status getResetVBParameters(napi_env env, napi_value *argv, std::string &inputId, int &mode,
                                 std::string &voiceBeautifierId)
{
    napi_status status = parseNapiString(env, argv[ARG_0], inputId);
    status = napi_get_value_int32(env, argv[ARG_1], &mode);
    status = parseNapiString(env, argv[ARG_2], voiceBeautifierId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, VB_NODE_TAG,
                 "audioEditTest resetVBEffect inputId: %{public}s, mode: %{public}d, voiceBeautifierId: %{public}s",
                 inputId.c_str(), mode, voiceBeautifierId.c_str());
    return status;
}