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

napi_value addAudioSeparation(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, AISS_TAG, "addAudioSeparation---IN");
    size_t argc = 5;
    napi_value argv[5] = {nullptr};
    napi_value ret = nullptr;
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_create_int64(env, AUDIOSUITE_ERROR_SYSTEM, &ret);

    std::string uuidStr;
    std::string inputIdStr;
    std::string selectedNodeId;
    bool isAllSep = false;
    if (ParseNapiString(env, argv[ARG_1], uuidStr) != napi_ok ||
        ParseNapiString(env, argv[ARG_2], inputIdStr) != napi_ok ||
        ParseNapiString(env, argv[ARG_3], selectedNodeId) != napi_ok ||
        napi_get_value_bool(env, argv[ARG_4], &isAllSep) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "Parse params failed");
        return ret;
    }

    Node node = CreateNodeByType(uuidStr, EFFECT_MULTII_OUTPUT_NODE_TYPE_AUDIO_SEPARATION);
    if (!node.physicalNode) return ret;

    OH_AudioSuite_Result result = AUDIOSUITE_ERROR_SYSTEM;
    if (isAllSep) {
        auto outputNodes = g_nodeManager->getNodesByType(OUTPUT_NODE_TYPE_DEFAULT);
        if (outputNodes.empty()) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "No output node found");
            return ret;
        }
        result = g_nodeManager->insertNode(uuidStr, outputNodes[0].id, BEFORE);
    } else {
        result = selectedNodeId.empty() ?
            (OH_AudioSuite_Result)AddEffectNodeToNodeManager(inputIdStr, uuidStr) :
            g_nodeManager->insertNode(uuidStr, selectedNodeId, LATER);
    }

    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, AISS_TAG, "insertNode failed: %d", (int)result);
        return ret;
    }

    g_multiRenderFrameFlag = true;
    if (g_threadPipelineManager) g_threadPipelineManager->multiRenderFrameFlag = true;
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