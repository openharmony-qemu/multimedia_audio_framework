/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "EffectNode.h"
#include "hilog/log.h"
#include "../audioSuiteError/AudioSuiteError.h"
#include "napi/native_api.h"

const int GLOBAL_RESMGR = 0xFF00;
const char *EFFECT_NODE_TAG = "[AudioEditTestApp_EffectNode_cpp]";

std::shared_ptr<NodeManager> g_nodeManager = nullptr;
std::shared_ptr<NodeManager> g_singlePipelineNodeManager = nullptr;

// After creating an effect node, call this method to add the effect node to the nodeManager
int32_t AddEffectNodeToNodeManager(std::string &inputNodeId, std::string &effectNodeId)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG,
        "audioEditTest AddEffectNodeToNodeManager start and inputNodeId is: %{public}s, effectNodeId is %{public}s",
        inputNodeId.c_str(), effectNodeId.c_str());
    // Add effect nodes, check if there is a mixer node. 
    // If there is no mixer node, add the effect nodes before the output node; 
    // If there is a mixer node, 
    // obtain the corresponding input node ID and insert the effect nodes sequentially before the mixer node
    const std::vector<Node> mixerNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER);
    OH_AudioSuite_Result result;
    Node currentNode  = g_nodeManager->GetNodeById(effectNodeId);
    if (currentNode .id.empty()) {
        return static_cast<int32_t>(AudioSuiteResult::DEMO_ERROR_FAILD);
    }

    if (mixerNodes.size() > 0) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG,
            "audioEditTest AddEffectNodeToNodeManager has mixerNodes");
        Node node = g_nodeManager->GetNodeById(inputNodeId);
        if (node.nextNodeId.empty()) {
            return static_cast<int32_t>(AudioSuiteResult::DEMO_ERROR_FAILD);
        }
        while (g_nodeManager->GetNodeById(node.nextNodeId).type !=  OH_AudioNode_Type::EFFECT_NODE_TYPE_AUDIO_MIXER) {
            node = g_nodeManager->GetNodeById(node.nextNodeId);
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG,
                "audioEditTest AddEffectNodeToNodeManager has mixerNodes and nextNode : %{public}s",
                node.id.c_str());
        }
        result = g_nodeManager->insertNode(effectNodeId, node.id, Direction::LATER);
    } else {
        const std::vector<Node> outPutNodes =
            g_nodeManager->getNodesByType(OH_AudioNode_Type::OUTPUT_NODE_TYPE_DEFAULT);
        if (outPutNodes.size() > 0) {
            result = g_nodeManager->insertNode(effectNodeId, outPutNodes[0].id, Direction::BEFORE);
        } else {
            result = AUDIOSUITE_ERROR_NODE_NOT_EXIST;
        }
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG,
        "audioEditTest AddEffectNodeToNodeManager end and result is: %{public}d", static_cast<int>(result));

    return result;
}

Node CreateNodeByType(std::string uuid, OH_AudioNode_Type nodeType)
{
    OH_AudioSuite_Result result = g_nodeManager->createNode(uuid, nodeType);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, EFFECT_NODE_TAG, "audioEditTest---create Node Failed");
    }
    Node node = g_nodeManager->GetNodeById(uuid);
    return node;
}

/**
 * @brief Traverse all OH_AudioNode_Type enumeration values to check for support.
 * @return napi_value All supported node types.
 */
napi_value GetSupportedAudioNodeTypes(napi_env env)
{
    const int minNodeType = 201;
    const int mixerNodeType = 207;
    const int maxNodeType = 211;
 
    std::vector<OH_AudioNode_Type> supportedTypes;
 
    for (int value = minNodeType; value <= maxNodeType; ++value) {
        if (value == mixerNodeType) {
            continue;
        }
        OH_AudioNode_Type nodeType = static_cast<OH_AudioNode_Type>(value);
        bool isSupported = false;
        OH_AudioSuite_Result result = OH_AudioSuiteEngine_IsNodeTypeSupported(nodeType, &isSupported);
        // Invoke interface to check support status
        if (result != OH_AudioSuite_Result::AUDIOSUITE_SUCCESS) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, EFFECT_NODE_TAG,
                         "audioEditTest---OH_AudioSuiteEngine_IsNodeTypeSupported Failed");
        } else {
            if (isSupported) {
                supportedTypes.push_back(nodeType);
            }
        }
    }
    napi_value typeArray;
    napi_status status = napi_create_array(env, &typeArray);
    if (status != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG, "Failed to create type array");
        return nullptr;
    }
    // Add each supported value to the JS array
    for (size_t i = 0; i < supportedTypes.size(); ++i) {
        napi_value jsValue;
        status = napi_create_int32(env, supportedTypes[i], &jsValue);
        if (status != napi_ok) {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG, "Failed to napi_create_int32");
            return nullptr;
        }
 
        status = napi_set_element(env, typeArray, i, jsValue);
        if (status != napi_ok) {
            OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG, "Failed to napi_set_element");
            return nullptr;
        }
    }
    return typeArray;
}