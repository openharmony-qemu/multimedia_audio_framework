/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "napi/native_api.h"
#include "VoiceChange.h"
#include "hilog/log.h"
#include "./utils/Utils.h"
#include "./EffectNode.h"
#include "./utils/Constant.h"

const int GLOBAL_RESMGR = 0xFF00;
const char *CHANGE_TAG = "[AudioEditTestApp_VoiceChange_cpp]";
OH_AudioSuite_GeneralVoiceChangeType getVoiceChangeTypeByMode(int mode)
{
    OH_AudioSuite_GeneralVoiceChangeType type;
    switch (mode) {
        case UINT_1:
            type = GENERAL_VOICE_CHANGE_TYPE_CUTE;
            break;
        case UINT_2:
            type = GENERAL_VOICE_CHANGE_TYPE_CYBERPUNK;
            break;
        case UINT_3:
            type = GENERAL_VOICE_CHANGE_TYPE_FEMALE;
            break;
        case UINT_4:
            type = GENERAL_VOICE_CHANGE_TYPE_MALE;
            break;
        case UINT_5:
            type = GENERAL_VOICE_CHANGE_TYPE_MIX;
            break;
        case UINT_6:
            type = GENERAL_VOICE_CHANGE_TYPE_MONSTER;
            break;
        case UINT_7:
            type = GENERAL_VOICE_CHANGE_TYPE_SEASONED;
            break;
        case UINT_8:
            type = GENERAL_VOICE_CHANGE_TYPE_SYNTH;
            break;
        case UINT_9:
            type = GENERAL_VOICE_CHANGE_TYPE_TRILL;
            break;
        case UINT_10:
            type = GENERAL_VOICE_CHANGE_TYPE_WAR;
            break;
    }
    return type;
}

OH_AudioSuite_PureVoiceChangeOption getPureVoiceChangeOptionByMode(int gender, int vocalPart, int optionType)
{
    OH_AudioSuite_PureVoiceChangeOption option;
    switch (optionType) {
        case UINT_1:
            option.optionType = PURE_VOICE_CHANGE_TYPE_CARTOON;
            break;
        case UINT_2:
            option.optionType = PURE_VOICE_CHANGE_TYPE_CUTE;
            break;
        case UINT_3:
            option.optionType = PURE_VOICE_CHANGE_TYPE_FEMALE;
            break;
        case UINT_4:
            option.optionType = PURE_VOICE_CHANGE_TYPE_MALE;
            break;
        case UINT_5:
            option.optionType = PURE_VOICE_CHANGE_TYPE_MONSTER;
            break;
        case UINT_6:
            option.optionType = PURE_VOICE_CHANGE_TYPE_ROBOTS;
            break;
        case UINT_7:
            option.optionType = PURE_VOICE_CHANGE_TYPE_SEASONED;
            break;
    }

    switch (gender) {
        case UINT_1:
            option.optionGender = PURE_VOICE_CHANGE_FEMALE;
            break;
        case UINT_2:
            option.optionGender = PURE_VOICE_CHANGE_MALE;
            break;
    }
    return option;
}

napi_status getvoiceChangeModeParameters(napi_env env, napi_value *argv, int &voiceChangeMode,
                                         std::string &VoiceChangeId, std::string &inputId)
{
    napi_status status = napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_0], &voiceChangeMode);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_1], VoiceChangeId);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_2], inputId);
    OH_LOG_Print(
        LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG,
        "audioEditTest SetEquailizerMode equailizerMode: %{public}d, equailizerId: %{public}s, inputId: %{public}s",
        voiceChangeMode, VoiceChangeId.c_str(), inputId.c_str());
    return status;
}

OH_AudioSuite_Result createGeneralVCNodeAndSetType(std::string uuidStr, unsigned int mode, Node &node)
{
    OH_AudioSuite_GeneralVoiceChangeType type = getVoiceChangeTypeByMode(mode);
    node = CreateNodeByType(uuidStr, OH_AudioNode_Type::EFFECT_NODE_TYPE_GENERAL_VOICE_CHANGE);
    if (node.physicalNode == nullptr) {
        return AUDIOSUITE_ERROR_SYSTEM;
    }
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    result = OH_AudioSuiteEngine_SetGeneralVoiceChangeType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG,
                     "OH_AudioSuiteEngine_SetGeneralVoiceChangeType ERROR:%{public}d", result);
    }
    return result;
}

OH_AudioSuite_Result createPureVCNodeAndSetType(std::string uuidStr, OH_AudioSuite_PureVoiceChangeOption option,
                                                Node &node)
{
    node = CreateNodeByType(uuidStr, OH_AudioNode_Type::EFFECT_NODE_TYPE_PURE_VOICE_CHANGE);
    if (node.physicalNode == nullptr) {
        return AUDIOSUITE_ERROR_SYSTEM;
    }
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    result = OH_AudioSuiteEngine_SetPureVoiceChangeOption(node.physicalNode, option);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG,
                     "OH_AudioSuiteEngine_SetGeneralVoiceChangeType ERROR:%{public}d", result);
    }
    return result;
}

napi_value StartGeneralVoiceChange(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "audioEditTest---startGeneralVoiceChangeEffect---IN");
    size_t argc = UINT_4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    std::string inputId;
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], inputId);
    unsigned int mode = 0;
    napi_get_value_uint32(env, argv[NAPI_ARGV_INDEX_1], &mode);
        std::string effectNodeId;
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_2], effectNodeId);
    std::string selectedNodeId;
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_3], selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "inputId:%{public}s, uuid:%{public}s, mode:%{public}d, "
                  "selectedNodeId:%{public}s", inputId.c_str(), effectNodeId.c_str(), mode, selectedNodeId.c_str());

    Node node;
    napi_value ret;
    OH_AudioSuite_Result result = createGeneralVCNodeAndSetType(effectNodeId, mode, node);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG, "StartGeneralVoiceChange ERROR!");
        napi_create_int64(env, result, &ret);
        return ret;
    }

    if (selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(inputId, effectNodeId);
        if (insertRes == (int)AudioSuiteResult::NODE_MANAGER_OPERATION_ERROR) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG, "addEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, insertRes, &ret);
            return ret;
        }
    } else {
        result = g_nodeManager->insertNode(effectNodeId, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG, "StartGeneralVoiceChange insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "StartGeneralVoiceChange: operation success");
    return ret;
}

napi_value ResetGeneralVoiceChange(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "audioEditTest---resetGeneralVoiceChangeEffect---IN");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    std::string inputId;
    ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], inputId);
    unsigned int mode = 0;
    napi_get_value_uint32(env, argv[NAPI_ARGV_INDEX_1], &mode);
    std::string effectNodeId;
    ParseNapiString(env, argv[NAPI_ARGV_INDEX_2], effectNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "inputId:%{public}s, uuid:%{public}s, mode:%{public}d",
                 inputId.c_str(), effectNodeId.c_str(), mode);

    OH_AudioSuite_GeneralVoiceChangeType type = getVoiceChangeTypeByMode(mode);
    napi_value ret;
    Node node = g_nodeManager->GetNodeById(effectNodeId);
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    result = OH_AudioSuiteEngine_SetGeneralVoiceChangeType(node.physicalNode, type);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG,
                     "SetGeneralVoiceChangeType ERROR---%{public}d", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "resetGeneralVoiceChangeEffect: operation success");
    return ret;
}

void parsePureVoiceChangeArguments(napi_callback_info info, napi_env env, PureVoiceChangeParam &params)
{
    size_t argc = 6;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], params.inputId);
    ParseNapiString(env, argv[NAPI_ARGV_INDEX_1], params.effectNodeId);
    napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_2], &params.gender);
    double pitchDouble = 0;
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_3], &pitchDouble);
    const int precision = 10;
    params.pitch = std::round(pitchDouble * precision) / static_cast<double>(precision);
    napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_4], &params.optionType);
    ParseNapiString(env, argv[NAPI_ARGV_INDEX_5], params.selectedNodeId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "inputId:%{public}s, uuid:%{public}s, gender:%{public}d,"
                 "pitch:%{public}f, optionType:%{public}d, selectedNodeId:%{public}s",
                 params.inputId.c_str(), params.effectNodeId.c_str(), params.gender,
                 params.pitch, params.optionType, params.selectedNodeId.c_str());
}

napi_value StartPureVoiceChange(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "audioEditTest---startPureVoiceChangeEffect---IN");
    PureVoiceChangeParam params;
    parsePureVoiceChangeArguments(info,  env, params);

    Node node;
    napi_value ret;
    OH_AudioSuite_PureVoiceChangeOption option =
        getPureVoiceChangeOptionByMode(params.gender, params.pitch, params.optionType);
    OH_AudioSuite_Result result = createPureVCNodeAndSetType(params.effectNodeId, option, node);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG, "createPureVCNodeAndSetType ERROR!");
        napi_create_int64(env, result, &ret);
        return ret;
    }

    if (params.selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(params.inputId, params.effectNodeId);
        if (insertRes == -1) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG, "addEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, insertRes, &ret);
            return ret;
        }
    } else {
        result = g_nodeManager->insertNode(params.effectNodeId, params.selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG, "StartPureVoiceChange insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "StartPureVoiceChange: operation success");
    return ret;
}

napi_value ResetPureVoiceChange(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "audioEditTest---resetPureVoiceChangeEffect---IN");
    PureVoiceChangeParam params;
    parsePureVoiceChangeArguments(info,  env, params);

    OH_AudioSuite_PureVoiceChangeOption option =
        getPureVoiceChangeOptionByMode(params.gender, params.pitch,  params.optionType);
    napi_value ret;
    Node node = g_nodeManager->GetNodeById(params.effectNodeId);
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    result = OH_AudioSuiteEngine_SetPureVoiceChangeOption(node.physicalNode, option);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, CHANGE_TAG,
                     "SetPureVoiceChangeOption ERROR---%{public}d", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, CHANGE_TAG, "ResetPureVoiceChange: operation success");
    return ret;
}