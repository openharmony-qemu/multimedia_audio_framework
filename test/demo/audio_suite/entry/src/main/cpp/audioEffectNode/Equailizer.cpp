/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "hilog/log.h" 
#include "Equailizer.h"
#include "./utils/Utils.h"
#include "./EffectNode.h"

const int GLOBAL_RESMGR = 0xFF00;
const char *EQUAILIZER_TAG = "[AudioEditTestApp_Equailizer_cpp]";

// 封装入参 OH_EqualizerMode
OH_EqualizerFrequencyBandGains SetEqualizerMode(int32_t equailizerMode)
{
    OH_EqualizerFrequencyBandGains eqMode;
    switch (equailizerMode) {
        case 1:
            eqMode = OH_EQUALIZER_PARAM_DEFAULT;
            break;
        case 2:
            eqMode = OH_EQUALIZER_PARAM_BALLADS;
            break;
        case 3:
            eqMode = OH_EQUALIZER_PARAM_CHINESE_STYLE;
            break;
        case 4:
            eqMode = OH_EQUALIZER_PARAM_CLASSICAL;
            break;
        case 5:
            eqMode = OH_EQUALIZER_PARAM_DANCE_MUSIC;
            break;
        case 6:
            eqMode = OH_EQUALIZER_PARAM_JAZZ;
            break;
        case 7:
            eqMode = OH_EQUALIZER_PARAM_POP;
            break;
        case 8:
            eqMode = OH_EQUALIZER_PARAM_RB;
            break;
        case 9:
            eqMode = OH_EQUALIZER_PARAM_ROCK;
            break;
        default:
            eqMode = OH_EQUALIZER_PARAM_DEFAULT;
            break;
    }
    return eqMode;
}

napi_status getEqModeParameters(napi_env env, napi_value *argv, unsigned int &equailizerMode, std::string &equailizerId, std::string &inputId)
{
    napi_status status = napi_get_value_uint32(env, argv[0], &equailizerMode);
    status = parseNapiString(env, argv[1], equailizerId);
    status = parseNapistring(env, argv[2], inputId);
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EQUAILIZER_TAG, "audioEditTest getEqModeParameters equailizerMode is %{public}d, equailizerId: %{public}s, inputId: %{public}s",
        equailizerMode, equailizerId.c_str(), inputId.c_str());
    return status;
}

napi_status getEqBandGainsParameters(napi_env, env, napi_value *argv, OH_EqualizerFrequencyBandGains &frequencyBandGains, EqBandGainsParams &params)
{
    // 遍历数组并打印每个元素
    for (uint32_t i = 0; i < length; ++i) {
        napi_value element;
        napi_get_element(env, argv[0], i, &element);
        unsigned int value;
        napi_get_value_uint32(env, element, &value);
        frequencyBandGains.gains[i] = value;
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EQUAILIZER_TAG, "audioEditTest getEqBandGainsParamters"
            " element at index %{public}d is %{public}d", i, frequencyBandGains.gains[i]);
        napi_status status = parseNapiString(env, argv[1], params.equailizerId);
        status = parseNapiString(env, argv[2], params.inputId);
        status = parseNapiString(env, argv[3], params.selectedNodeId);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EQUAILIZER_TAG, "audioEditTest getEqBandGainsParamters equailizerId: %{public}s, inputId: %{public}s, selectedNodeId: %{public}s",
            params.equailizerId.c_str(), params.inputId.c_str(), params.selectedNodeId.c_str());
        return status;
    }
}

Node getOrCreateEqualizerNodeByMode(std::string& equailizerId, std::string& inputId)
{
    Node eqNode = nodeManager->getNodeById(equailizerId);
    if (!eqNode.physicalNode) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EQUAILIZER_TAG, "audioEditTest getOrCreateEqualizerNodeByMode create");
        eqNode.id = equailizerId;
        eqNode.type = OH_AudioNode_Type::EFFECT_NODE_TYPE_EQUALIZER;
        nodeManager->createNode(equailizerId, OH_AudioNode_Type::EFFECT_NODE_TYPE_EQUALIZER);
        eqNode = nodeManager->getNodeById(equailizerId);
        int32_t result = addEffectNodeManager(inputId, equailizerId);
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EQUAILIZER_TAG,
            "audioEditTest addEffectNodeManager result: %{public}d", result);
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            eqNode.physicalNode = nullptr; // 标记为失败
        }
    }
    return eqNode;
}

Node getOrCreateEqualizerNodeByGains(std::string& equailizerId, std::string& inputId, std::string& selectedNodeId)
{
    Node eqNode = nodeManager->getNodeById(equailizerId);
    if (!eqNode.physicalNode) {
        // 创建均衡器节点
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EQUAILIZER_TAG,
            "audioEditTest getOrCreateEqualizerNodeByGains create");
        eqNode.id = equailizerId;
        eqNode.type = OH_AudioNode_Type::EFFECT_NODE_TYPE_EQUALIZER;
        nodeManager->createNode(equailizerId, OH_AudioNode_Type::EFFECT_NODE_TYPE_EQUALIZER);
        // 获取效果节点
        eqNode = nodeManager->getNodeById(equailizerId);
        if (selectedNodeId.empty()) {
            int result = addEffectNodeToNodeManager(inputId, equailizerId);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EQUAILIZER_TAG,
                "audioEditTest addEffectNodeToNodeManager addEffectNodeToNodeManager result: %{public}d",
                result);
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                eqNode.physicalNode = nullptr; // 标记为失败
            }
        } else {
            int result = nodeManager->insertNode(equailizerId, selectedNodeId, Direction::LATER);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EQUAILIZER_TAG,
                "audioEditTest addEffectNodeToNodeManager insertNode result: %{public}d", result);
            if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
                eqNode.physicalNode = nullptr; // 标记为失败
            }
        }
    }
    return eqNode;
}