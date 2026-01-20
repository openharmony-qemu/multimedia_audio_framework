/*
 * Copyright (c) 2025 Huawei Device Co., Ltd. 2025-2025. ALL rights reserved.
 */

#ifndef AUDIOEDITTESTAPP_NODEMANAGER_H
#define AUDIOEDITTESTAPP_NODEMANAGER_H

#include <string>
#include <unordered_map>
#include <vector>
#include "ohaudio/native_audio_suite_base.h"
#include "ohaudio/native_audio_suite_engine.h"

enum NodeType { INPUT_NODE, OUTPUT_NODE, NOISE_REDUCTION_NODE, AUDIO_SEPARATION_NODE, SOUND_FIELD_NODE, MIXER_NODE };

enum Direction { BEFORE, LATER };

struct Node {
    std::string id;
    OH_AudioNode_Type type;
    std::vector<std::string> preNodeIds = {};  // List of previous node IDs
    std::string nextNodeId = "";               // List of subsequent node IDs
    OH_AudioNode *physicalNode = nullptr;      // Real node
};

class NodeManager {
private:
    std::unordered_map<std::string, Node> nodes;  // Node storage, with the key being the node ID
    OH_AudioSuitePipeline *audioSuitePipeLine;    // One NodeManager, one pipeline

    // Get Pipeline Connection Details
    void GetPipeLineDetail();

public:
    explicit NodeManager(OH_AudioSuitePipeline *audioSuitePipeLine);
    ~NodeManager();

    // Create Node: Create a real node, construct the Node, and then store it in nodes
    OH_AudioSuite_Result createNode(
        const std::string &nodeId, const OH_AudioNode_Type NodeType, OH_AudioNodeBuilder *builder = nullptr);

    // Delete Node
    OH_AudioSuite_Result removeNode(const std::string &nodeId);

    // Reset Effect Node
    OH_AudioSuite_Result resetNode(const std::string &nodeId, const OH_AudioNode_Type type);

    // Connecting two nodes
    OH_AudioSuite_Result connect(const std::string &fromId, const std::string &toId);

    // Disconnecting Two Nodes
    OH_AudioSuite_Result disconnect(const std::string &fromId, const std::string &toId);

    // Disconnect all connections to the specified node
    void DisconnectAll(const std::string &nodeId);

    // Insert Node
    OH_AudioSuite_Result insertNode(
        const std::string &sourceNodeId, const std::string &targetNodeId, Direction direction);

    // mobile node
    OH_AudioSuite_Result moveNode(
        const std::string &sourceNodeId, const std::string &targetNodeId, Direction direction);

    // Get All Nodes
    const std::unordered_map<std::string, Node> &getAllNodes() const;

    // Get node by id
    const Node &GetNodeById(const std::string &nodeId) const;

    // Get node based on node type
    const std::vector<Node> getNodesByType(const OH_AudioNode_Type type) const;

    // Stop pipeline status
    OH_AudioSuite_Result stopPipelineState();

    bool IsValidNode(const std::string &nodeId) const;

    OH_AudioSuite_Result insertBefore(
        const std::string &sourceNodeId, const std::string &targetNodeId, const Node &targetNode);

    OH_AudioSuite_Result insertAfter(
        const std::string &sourceNodeId, const std::string &targetNodeId, const Node &targetNode);
    
    // Obtain node effect parameters based on different effect types
    std::string GetOptionsByType(const Node& node);

    char *getNodeTypeName(OH_AudioNode_Type type);
};

#endif