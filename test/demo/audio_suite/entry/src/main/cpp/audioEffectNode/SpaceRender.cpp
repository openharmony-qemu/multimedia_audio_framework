/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "SpaceRender.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include "napi/native_api.h"
#include "hilog/log.h"
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"
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
const char *SP_TAG = "[AudioEditTestApp_SPATIALRENDER_cpp]";

napi_value startFixedPositionEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "audioEditTest---startFixedPositionEffect---IN");
    size_t argc = 6;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    double x = 0;
    double y = 0;
    double z = 0;
    std::string effectNodeId;
    std::string inputId;
    std::string selectedNodeId;

    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_0], &x);
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_1], &y);
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_2], &z);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_3], effectNodeId);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_4], inputId);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_5], selectedNodeId);

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "x:%{public}lf, y:%{public}lf, z:%{public}lf, ", x, y, z);

    Node node;
    napi_value ret;
    OH_AudioSuite_SpaceRenderPositionParams positionPara = {static_cast<float>(x), static_cast<float>(y),
                                                            static_cast<float>(z)};
    OH_AudioSuite_Result result = createRenderNodeAndSetPosition(positionPara, effectNodeId, node);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "createRenderNodeAndSetType ERROR!");
        napi_create_int64(env, result, &ret);
        return ret;
    }

    if (selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(inputId, effectNodeId);
        if (insertRes == -1) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "AddEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, insertRes, &ret);
            return ret;
        }
    } else {
        result = g_nodeManager->insertNode(effectNodeId, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "startFixedPositionEffect insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "startFixedPositionEffect: operation success");
    return ret;
}

napi_value resetFixedPositionEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "audioEditTest---resetFixedPositionEffect---IN");
    size_t argc = 4;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    double x = 0;
    double y = 0;
    double z = 0;
    std::string effectNodeId;

    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_0], &x);
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_1], &y);
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_2], &z);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_3], effectNodeId);

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "x:%{public}lf, y:%{public}lf, z:%{public}lf, ", x, y, z);

    napi_value ret;
    OH_AudioSuite_Result result;

    Node node = g_nodeManager->GetNodeById(effectNodeId);
    OH_AudioSuite_SpaceRenderPositionParams positionPara = {static_cast<float>(x), static_cast<float>(y),
                                                            static_cast<float>(z)};

    result = OH_AudioSuiteEngine_SetSpaceRenderPositionParams(node.physicalNode, positionPara);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG,
                     "OH_AudioSuiteEngine_SetSpaceRenderPositionParams ERROR---%{public}d", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "resetFixedPositionEffect: operation success");
    return ret;
}

void parseDynamicRenderParams(napi_env env, napi_value* argv, DynamicRenderParams& params)
{
    napi_status status;

    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_0], &params.x);
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_1], &params.y);
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_2], &params.z);
    napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_3], &params.surroundTime);
    napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_4], &params.surroundDirection);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_5], params.effectNodeId);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_6], params.inputId);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_7], params.selectedNodeId);

    switch (params.surroundDirection) {
        case 0:
            params.surroundDirectionType = OH_AudioSuite_SurroundDirection::SPACE_RENDER_CCW;
            break;
        case 1:
            params.surroundDirectionType = OH_AudioSuite_SurroundDirection::SPACE_RENDER_CW;
            break;
        default:
            params.surroundDirectionType = OH_AudioSuite_SurroundDirection::SPACE_RENDER_CCW;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG,
                 "x:%{public}lf, y:%{public}lf, z:%{public}lf, surroundTime:%{public}d surroundDirection:%{public}d",
                 params.x, params.y, params.z, params.surroundTime, params.surroundDirection);
}

napi_value startDynamicRenderEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "audioEditTest---startDynamicRenderEffect---IN");
    size_t argc = 8;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;

    DynamicRenderParams params;
    parseDynamicRenderParams(env, argv, params);

    double x = params.x;
    double y = params.y;
    double z = params.z;
    int surroundTime = params.surroundTime;
    OH_AudioSuite_SurroundDirection surroundDirectionType = params.surroundDirectionType;
    std::string effectNodeId = params.effectNodeId;
    std::string inputId = params.inputId;
    std::string selectedNodeId = params.selectedNodeId;

    Node node;
    napi_value ret;
    OH_AudioSuite_SpaceRenderRotationParams rotationPara = {static_cast<float>(x), static_cast<float>(y),
                                                            static_cast<float>(z), surroundTime, surroundDirectionType};
    OH_AudioSuite_Result result = createRenderNodeAndSetRenderPara(rotationPara, effectNodeId, node);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "createRenderNodeAndSetRenderPara ERROR!");
        napi_create_int64(env, result, &ret);
        return ret;
    }

    if (selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(inputId, effectNodeId);
        if (insertRes == -1) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "AddEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, insertRes, &ret);
            return ret;
        }
    } else {
        result = g_nodeManager->insertNode(effectNodeId, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "startDynamicRenderEffect insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "startDynamicRenderEffect: operation success");
    return ret;
}

napi_value resetDynamicRenderEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "audioEditTest---resetDynamicRenderEffect---IN");
    size_t argc = 6;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    double x = 0;
    double y = 0;
    double z = 0;
    int surroundTime = 2;
    int surroundDirection = 0;
    std::string effectNodeId;

    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_0], &x);
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_1], &y);
    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_2], &z);
    napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_3], &surroundTime);
    napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_4], &surroundDirection);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_5], effectNodeId);

    OH_AudioSuite_SurroundDirection surroundDirectionType;
    switch (surroundDirection) {
        case 0:
            surroundDirectionType = OH_AudioSuite_SurroundDirection::SPACE_RENDER_CCW;
            break;
        case 1:
            surroundDirectionType = OH_AudioSuite_SurroundDirection::SPACE_RENDER_CW;
            break;
        default:
            surroundDirectionType = OH_AudioSuite_SurroundDirection::SPACE_RENDER_CCW;
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "x:%{public}lf, y:%{public}lf, z:%{public}lf, ", x, y, z);

    napi_value ret;
    OH_AudioSuite_Result result;

    Node node = g_nodeManager->GetNodeById(effectNodeId);
    OH_AudioSuite_SpaceRenderRotationParams rotationPara = {static_cast<float>(x), static_cast<float>(y),
                                                            static_cast<float>(z), surroundTime, surroundDirectionType};

    result = OH_AudioSuiteEngine_SetSpaceRenderRotationParams(node.physicalNode, rotationPara);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG,
                     "OH_AudioSuiteEngine_SetSpaceRenderRotationParams ERROR---%{public}d", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "resetDynamicRenderEffect: operation success");
    return ret;
}

napi_value startExpandEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "audioEditTest---startExpandEffect---IN");
    size_t argc = 5;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    double extRadius = 1;
    int extAngle = 1;
    std::string effectNodeId;
    std::string inputId;
    std::string selectedNodeId;

    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_0], &extRadius);
    napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_1], &extAngle);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_2], effectNodeId);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_3], inputId);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_4], selectedNodeId);

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "extRadius:%{public}lf, extAngle:%{public}d, ", extRadius,
                 extAngle);

    Node node;
    napi_value ret;
    OH_AudioSuite_SpaceRenderExtensionParams extensionPara = {static_cast<float>(extRadius), extAngle};
    OH_AudioSuite_Result result = createRenderNodeAndSetExtension(extensionPara, effectNodeId, node);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "createRenderNodeAndSetExtension ERROR!");
        napi_create_int64(env, result, &ret);
        return ret;
    }

    if (selectedNodeId.empty()) {
        int insertRes = AddEffectNodeToNodeManager(inputId, effectNodeId);
        if (insertRes == -1) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "AddEffectNodeToNodeManager ERROR!");
            napi_create_int64(env, insertRes, &ret);
            return ret;
        }
    } else {
        result = g_nodeManager->insertNode(effectNodeId, selectedNodeId, Direction::LATER);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "startExpandEffect insertNode ERROR!");
            napi_create_int64(env, result, &ret);
            return ret;
        }
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "startExpandEffect: operation success");
    return ret;
}

napi_value resetExpandEffect(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "audioEditTest---resetExpandEffect---IN");
    size_t argc = 3;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    double extRadius = 1;
    int extAngle = 1;
    std::string effectNodeId;

    napi_get_value_double(env, argv[NAPI_ARGV_INDEX_0], &extRadius);
    napi_get_value_int32(env, argv[NAPI_ARGV_INDEX_1], &extAngle);
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_2], effectNodeId);

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "extRadius:%{public}lf, extAngle:%{public}d, ", extRadius,
                 extAngle);

    napi_value ret;
    OH_AudioSuite_Result result;
    OH_AudioSuite_SpaceRenderExtensionParams extensionPara = {static_cast<float>(extRadius), extAngle};
    Node node = g_nodeManager->GetNodeById(effectNodeId);

    result = OH_AudioSuiteEngine_SetSpaceRenderExtensionParams(node.physicalNode, extensionPara);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG,
                     "OH_AudioSuiteEngine_SetSpaceRenderExtensionParams ERROR---%{public}d", result);
        napi_create_int64(env, result, &ret);
        return ret;
    }

    napi_create_int64(env, AUDIOSUITE_SUCCESS, &ret);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "resetExpandEffect: operation success");
    return ret;
}

napi_value getFixedPositionParams(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "audioEditTest---getFixedPositionParams---IN");
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    std::string effectNodeId;
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], effectNodeId);

    Node node = g_nodeManager->GetNodeById(effectNodeId);
    OH_AudioSuite_SpaceRenderPositionParams positionPara;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_GetSpaceRenderPositionParams(node.physicalNode, &positionPara);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "audioEditTest---getFixedPositionParams Failed");
        return nullptr;
    }
    napi_value resultObj;
    status = napi_create_object(env, &resultObj);
    // 创建属性键（字符串）
    napi_value xKey;
    napi_value yKey;
    napi_value zKey;
    napi_create_string_utf8(env, "x", NAPI_AUTO_LENGTH, &xKey);
    napi_create_string_utf8(env, "y", NAPI_AUTO_LENGTH, &yKey);
    napi_create_string_utf8(env, "z", NAPI_AUTO_LENGTH, &zKey);

    // 创建属性值（double 类型 napi_value）
    napi_value xValue;
    napi_value yValue;
    napi_value zValue;
    napi_create_double(env, positionPara.x, &xValue);
    napi_create_double(env, positionPara.y, &yValue);
    napi_create_double(env, positionPara.z, &zValue);

    // 设置对象属性
    napi_set_property(env, resultObj, xKey, xValue);
    napi_set_property(env, resultObj, yKey, yValue);
    napi_set_property(env, resultObj, zKey, zValue);

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, SP_TAG, "audioEditTest---getFixedPositionParams---END");

    return resultObj;
}

napi_value getDynamicRenderParams(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    std::string effectNodeId;
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], effectNodeId);

    Node node = g_nodeManager->GetNodeById(effectNodeId);
    OH_AudioSuite_SpaceRenderRotationParams rotationPara;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_GetSpaceRenderRotationParams(node.physicalNode, &rotationPara);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "audioEditTest---getDynamicRenderParams Failed");
        return nullptr;
    }
    napi_value resultObj;
    status = napi_create_object(env, &resultObj);

    // 创建属性键（字符串）
    napi_value xKey;
    napi_value yKey;
    napi_value zKey;
    napi_value surroundTimeKey;
    napi_value surroundDirectionKey;
    napi_create_string_utf8(env, "x", NAPI_AUTO_LENGTH, &xKey);
    napi_create_string_utf8(env, "y", NAPI_AUTO_LENGTH, &yKey);
    napi_create_string_utf8(env, "z", NAPI_AUTO_LENGTH, &zKey);
    napi_create_string_utf8(env, "surroundTime", NAPI_AUTO_LENGTH, &surroundTimeKey);
    napi_create_string_utf8(env, "surroundDirection", NAPI_AUTO_LENGTH, &surroundDirectionKey);

    // 创建属性值（double 类型 napi_value）
    napi_value xValue;
    napi_value yValue;
    napi_value zValue;
    napi_value surroundTimeValue;
    napi_value surroundDirectionValue;
    napi_create_double(env, rotationPara.x, &xValue);
    napi_create_double(env, rotationPara.y, &yValue);
    napi_create_double(env, rotationPara.z, &zValue);
    napi_create_int32(env, rotationPara.surroundTime, &surroundTimeValue);
    napi_create_int32(env, static_cast<int>(rotationPara.surroundDirection), &surroundDirectionValue);

    // 设置对象属性
    napi_set_property(env, resultObj, xKey, xValue);
    napi_set_property(env, resultObj, yKey, yValue);
    napi_set_property(env, resultObj, zKey, zValue);
    napi_set_property(env, resultObj, surroundTimeKey, surroundTimeValue);
    napi_set_property(env, resultObj, surroundDirectionKey, surroundDirectionValue);

    return resultObj;
}

napi_value getExpandParams(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value *argv = new napi_value[argc];
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_status status;
    std::string effectNodeId;
    status = ParseNapiString(env, argv[NAPI_ARGV_INDEX_0], effectNodeId);

    Node node = g_nodeManager->GetNodeById(effectNodeId);
    OH_AudioSuite_SpaceRenderExtensionParams expandPara;
    OH_AudioSuite_Result result = OH_AudioSuiteEngine_GetSpaceRenderExtensionParams(node.physicalNode, &expandPara);
    if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG, "audioEditTest---getFixedPositionParams Failed");
        return nullptr;
    }
    napi_value resultObj;
    status = napi_create_object(env, &resultObj);

    // 创建属性键（字符串）
    napi_value extRadiusKey;
    napi_value extAngleKey;
    napi_create_string_utf8(env, "extRadius", NAPI_AUTO_LENGTH, &extRadiusKey);
    napi_create_string_utf8(env, "extAngle", NAPI_AUTO_LENGTH, &extAngleKey);

    // 创建属性值（double 类型 napi_value）
    napi_value extRadiusValue;
    napi_value extAngleValue;
    napi_create_double(env, expandPara.extRadius, &extRadiusValue);
    napi_create_double(env, expandPara.extAngle, &extAngleValue);

    // 设置对象属性
    napi_set_property(env, resultObj, extRadiusKey, extRadiusValue);
    napi_set_property(env, resultObj, extAngleKey, extAngleValue);

    napi_value extRadius;
    napi_value extAngle;
    status = napi_create_double(env, expandPara.extRadius, &extRadius);
    status = napi_set_element(env, resultObj, 0, extRadius);
    status = napi_create_int32(env, expandPara.extAngle, &extAngle);
    status = napi_set_element(env, resultObj, 1, extAngle);

    return resultObj;
}

OH_AudioSuite_Result createRenderNodeAndSetPosition(OH_AudioSuite_SpaceRenderPositionParams &positionPara,
    std::string effectNodeId, Node &node)
{
    node = CreateNodeByType(effectNodeId, OH_AudioNode_Type::EFFECT_NODE_TYPE_SPACE_RENDER);
    if (node.physicalNode == nullptr) {
        return AUDIOSUITE_ERROR_SYSTEM;
    }
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    result = OH_AudioSuiteEngine_SetSpaceRenderPositionParams(node.physicalNode, positionPara);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG,
                     "audioEditTest---OH_AudioSuiteEngine_SetSpaceRenderPositionParams ERROR---%{public}d", result);
    }
    return result;
}

OH_AudioSuite_Result createRenderNodeAndSetRenderPara(OH_AudioSuite_SpaceRenderRotationParams &rotationPara,
    std::string effectNodeId, Node &node)
{
    node = CreateNodeByType(effectNodeId, OH_AudioNode_Type::EFFECT_NODE_TYPE_SPACE_RENDER);
    if (node.physicalNode == nullptr) {
        return AUDIOSUITE_ERROR_SYSTEM;
    }
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    result = OH_AudioSuiteEngine_SetSpaceRenderRotationParams(node.physicalNode, rotationPara);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG,
                     "audioEditTest---OH_AudioSuiteEngine_SetSpaceRenderRotationParams ERROR---%{public}d", result);
    }
    return result;
}

OH_AudioSuite_Result createRenderNodeAndSetExtension(OH_AudioSuite_SpaceRenderExtensionParams &extensionPara,
    std::string effectNodeId, Node &node)
{
    node = CreateNodeByType(effectNodeId, OH_AudioNode_Type::EFFECT_NODE_TYPE_SPACE_RENDER);
    if (node.physicalNode == nullptr) {
        return AUDIOSUITE_ERROR_SYSTEM;
    }
    OH_AudioSuite_Result result = AUDIOSUITE_SUCCESS;
    result = OH_AudioSuiteEngine_SetSpaceRenderExtensionParams(node.physicalNode, extensionPara);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, SP_TAG,
                     "audioEditTest---OH_AudioSuiteEngine_SetSpaceRenderExtensionParams ERROR---%{public}d", result);
    }
    return result;
}