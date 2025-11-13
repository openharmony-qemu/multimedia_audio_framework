/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#include "EffectNode.h"
#include "hilog/log.h"

const int GLOBAL_RESMGR = 0xFF00;
const char *EFFECT_NODE_TAG = "[AudioEditTestApp_EffectNode_cpp]";

std::shared_ptr<NodeManager> g_nodeManager = nullptr;

// 创建效果节点后调用该方法将效果节点加入到nodeManager中
int32_t AddEffectNodeToNodeManager(std::string &inputNodeId, std::string &effectNodeId)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG,
        "audioEditTest AddEffectNodeToNodeManager start and inputNodeId is: %{public}s, effectNodeId is %{public}s",
        inputNodeId.c_str(), effectNodeId.c_str());
    // 添加效果节点，检查是否有混音节点，没有混音节点就将效果节点添加到output节点之前；有混音节点，获取到对应input节点id，按序插入到混音节点之前
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
        const std::vector<Node> outPutNodes = g_nodeManager->getNodesByType(OH_AudioNode_Type::OUT_NODE_TYPE_DEFAULT);
        result = g_nodeManager->insertNode(effectNodeId, outPutNodes[0].id, Direction::BEFORE);
    }

    OH_LOG_Print(LOG_APP, LOG_INFO, GLOBAL_RESMGR, EFFECT_NODE_TAG,
        "audioEditTest AddEffectNodeToNodeManager end and result is: %{public}d", static_cast<int>(result));

    return result;
}

Node CreateNodeByType(std::string uuid, OH_AudioNode_Type nodeType)
{
    OH_AudioSuite_Result result = nodeManager->createNode(uuid, nodeType);
    if (result != AUDIOSUITE_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, GLOBAL_RESMGR, EFFECT_NODE_TAG, "audioEditTest---create Node Failed");
    }
    Node node = nodeManager->GetNodeById(uuid);
    return node;
}