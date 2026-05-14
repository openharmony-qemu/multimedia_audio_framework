/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "AissEffect.h"
#include "NoiseReduction.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include "napi/native_api.h"
#include "hilog/log.h"
#include "ohaudiosuite/native_audio_suite_base.h"
#include "ohaudiosuite/native_audio_suite_engine.h"
#include "NodeManager.h"
#include "callback/RegisterCallback.h"
#include "audioSuiteError/AudioSuiteError.h"
#include "audioEffectNode/Equalizer.h"
#include "audioEffectNode/EffectNode.h"
#include "audioEffectNode/Input.h"
#include "audioEffectNode/Output.h"
#include "realTimePlay/RealTimePlaying.h"
#include "multiPipelineEdit/MultiPipelineEdit.h"
#include "utils/Utils.h"
#include "./EffectNode.h"
#include "/utils/Constant.h"

const int GLOBAL_RESMGR = 0xFF00;
const char *AISS_TAG = "[AudioEditTestApp_AISS_cpp]";

// Argument counts
const int32_t ARG_COUNT_5 = 5;
const int32_t ARG_COUNT_1 = 1;
const int32_t MIN_ARGS_3 = 3;
const int32_t MIN_ARGS_4 = 4;
const int32_t MIN_ARGS_5 = 5;

// Default values
const bool DEFAULT_IS_ALL_SEP = false;

// Array indices
const int32_t FIRST_OUTPUT_NODE_INDEX = 0;

struct AudioSeparationParams {
    std::string uuid;
    std::string inputId;
    std::string selectedNodeId;
    bool isAllSep;
};

static bool ParseAudioSeparationParams(napi_env env, napi_callback_info info, AudioSeparationParams &params)
{
    size_t argc = ARG_COUNT_5;
    napi_value argv[ARG_COUNT_5] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, AISS_TAG, "addAudioSeparation argc: %{public}d", (int)argc);

    params.isAllSep = false;

    if (argc < MIN_ARGS_3) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "Invalid argc: %{public}d", (int)argc);
        return false;
    }

    if (ParseNapiString(env, argv[ARG_1], params.uuid) != napi_ok ||
        ParseNapiString(env, argv[ARG_2], params.inputId) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "Parse params failed");
        return false;
    }

    if (argc >= MIN_ARGS_4) {
        if (ParseNapiString(env, argv[ARG_3], params.selectedNodeId) != napi_ok) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "Parse selectedNodeId failed");
            return false;
        }
    }

    if (argc >= MIN_ARGS_5) {
        if (napi_get_value_bool(env, argv[ARG_4], &params.isAllSep) != napi_ok) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "Parse isAllSep failed");
            return false;
        }
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, AISS_TAG,
                 "uuid:%{public}s, inputId:%{public}s,"
                 "selectedNodeId:%{public}s, isAllSep:%{public}d",
                 params.uuid.c_str(), params.inputId.c_str(), params.selectedNodeId.c_str(), params.isAllSep);
    return true;
}

static OH_AudioSuite_Result InsertAudioSeparationNode(const AudioSeparationParams &params)
{
    if (params.isAllSep) {
        auto outputNodes = g_nodeManager->getNodesByType(OUTPUT_NODE_TYPE_DEFAULT);
        if (outputNodes.empty()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "No output node found");
            return AUDIOSUITE_ERROR_SYSTEM;
        }
        return g_nodeManager->insertNode(params.uuid, outputNodes[FIRST_OUTPUT_NODE_INDEX].id, BEFORE);
    }

    if (params.selectedNodeId.empty()) {
        return (OH_AudioSuite_Result)AddEffectNodeToNodeManager(params.inputId, params.uuid);
    }
    return g_nodeManager->insertNode(params.uuid, params.selectedNodeId, LATER);
}

napi_value addAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, AISS_TAG, "addAudioSeparation---IN");

    AudioSeparationParams params;
    if (!ParseAudioSeparationParams(env, info, params)) {
        napi_value ret;
        napi_create_int64(env, AUDIOSUITE_ERROR_SYSTEM, &ret);
        return ret;
    }

    Node node = CreateNodeByType(params.uuid, EFFECT_MULTII_OUTPUT_NODE_TYPE_AUDIO_SEPARATION);
    if (!node.physicalNode) {
        napi_value ret;
        napi_create_int64(env, AUDIOSUITE_ERROR_SYSTEM, &ret);
        return ret;
    }

    OH_AudioSuite_Result result = InsertAudioSeparationNode(params);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "insertNode failed: %d", (int)result);
        napi_value ret;
        napi_create_int64(env, AUDIOSUITE_ERROR_SYSTEM, &ret);
        return ret;
    }

    g_multiRenderFrameFlag = true;
    if (g_threadPipelineManager) g_threadPipelineManager->multiRenderFrameFlag = true;

    napi_value ret;
    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, AISS_TAG, "addAudioSeparation: success");
    return ret;
}

napi_value deleteAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, AISS_TAG, "deleteAudioSeparation IN");
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    // get uuid
    std::string uuidStr;
    ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], uuidStr);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, AISS_TAG, "uuid==%{public}s", uuidStr.c_str());

    OH_AudioSuite_Result result;
    napi_value napiValue = nullptr;
    result = g_nodeManager->removeNode(uuidStr);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "audioEditTest removeNode ERROR:%{public}d", result);
    }
    napi_create_int64(env, result, &napiValue);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, AISS_TAG, "deleteAudioSeparation: operation success");
    delete[] argv;
    return napiValue;
}