/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOG_TAG
#define LOG_TAG "HpaeDfxMapTree"
#endif
#include "hpae_dfx_map_tree.h"
#include <queue>
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
DfxMapTreeNode::DfxMapTreeNode(const HpaeDfxNodeInfo &info) : nodeInfo_(info)
{}

const HpaeDfxNodeInfo &DfxMapTreeNode::GetNodeInfo() const
{
    return nodeInfo_;
}

uint32_t DfxMapTreeNode::GetNodeId() const
{
    return nodeInfo_.nodeId;
}

const std::string &DfxMapTreeNode::GetNodeName() const
{
    return nodeInfo_.nodeName;
}

void DfxMapTreeNode::SetNodeInfo(const HpaeDfxNodeInfo &info)
{
    nodeInfo_ = info;
}

const std::set<uint32_t> &DfxMapTreeNode::GetParentIds() const
{
    return parentIds_;
}

const std::set<uint32_t> &DfxMapTreeNode::GetChildrenIds() const
{
    return childrenIds_;
}

bool DfxMapTreeNode::IsRoot() const
{
    return parentIds_.empty();
}

void DfxMapTreeNode::AddParent(uint32_t parentId)
{
    parentIds_.insert(parentId);
}

bool DfxMapTreeNode::RemoveParent(uint32_t parentId)
{
    return parentIds_.erase(parentId) > 0;
}

size_t DfxMapTreeNode::GetParentCount() const
{
    return parentIds_.size();
}

bool DfxMapTreeNode::IsLeaf() const
{
    return childrenIds_.empty();
}

void DfxMapTreeNode::AddChild(uint32_t childId)
{
    childrenIds_.insert(childId);
}

bool DfxMapTreeNode::RemoveChild(uint32_t childId)
{
    return childrenIds_.erase(childId) > 0;
}

size_t DfxMapTreeNode::GetChildrenCount() const
{
    return childrenIds_.size();
}

// DFX tree interface
std::shared_ptr<DfxMapTreeNode> HpaeDfxMapTree::FindDfxNode(uint32_t nodeId)
{
    auto it = nodeMap_.find(nodeId);
    return it != nodeMap_.end() ? it->second : nullptr;
}

std::shared_ptr<const DfxMapTreeNode> HpaeDfxMapTree::FindDfxNode(uint32_t nodeId) const
{
    auto it = nodeMap_.find(nodeId);
    return it != nodeMap_.end() ? it->second : nullptr;
}

bool HpaeDfxMapTree::AddNode(const HpaeDfxNodeInfo &nodeInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    uint32_t nodeId = nodeInfo.nodeId;
    if (FindDfxNode(nodeId) != nullptr) {
        AUDIO_WARNING_LOG("Node already exists, nodeName:%{public}s nodeId:%{public}u",
            nodeInfo.nodeName.c_str(), nodeId);
        return false;
    }
    
    auto node = std::make_shared<DfxMapTreeNode>(nodeInfo);
    nodeMap_.emplace(nodeId, node);
    AUDIO_INFO_LOG("Add node success, nodeName:%{public}s nodeId:%{public}u",
        nodeInfo.nodeName.c_str(), nodeId);
    return true;
}

bool HpaeDfxMapTree::RemoveNode(uint32_t nodeId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto node = FindDfxNode(nodeId);
    if (node == nullptr) {
        AUDIO_WARNING_LOG("Node %{public}u not found", nodeId);
        return false;
    }

    auto parentIds = node->GetParentIds();
    for (uint32_t parentId : parentIds) {
        auto parent = FindDfxNode(parentId);
        CHECK_AND_CONTINUE(parent != nullptr);
        parent->RemoveChild(nodeId);
    }

    auto childrenIds = node->GetChildrenIds();
    for (uint32_t childId : childrenIds) {
        auto child = FindDfxNode(childId);
        CHECK_AND_CONTINUE(child != nullptr);
        child->RemoveParent(nodeId);
    }

    nodeMap_.erase(nodeId);
    AUDIO_INFO_LOG("Remove node %{public}u success", nodeId);
    return true;
}

bool HpaeDfxMapTree::ConnectNodes(uint32_t parentId, uint32_t childId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (parentId == childId) {
        AUDIO_WARNING_LOG("Cannot connect node to itself, nodeId:%{public}u", parentId);
        return false;
    }

    if (CheckCycleConnection(parentId, childId)) {
        AUDIO_WARNING_LOG("May cause cycle, parent %{public}u, child %{public}u", parentId, childId);
        return false;
    }

    auto parent = FindDfxNode(parentId);
    auto child = FindDfxNode(childId);
    if (parent == nullptr || child == nullptr) {
        AUDIO_WARNING_LOG("Parent %{public}u or child %{public}u not found", parentId, childId);
        return false;
    }

    parent->AddChild(childId);
    child->AddParent(parentId);
    AUDIO_INFO_LOG("Connection established, parent name:%{public}s id:%{public}u, child name:%{public}s id:%{public}u",
        parent->GetNodeName().c_str(), parentId, child->GetNodeName().c_str(), childId);
    return true;
}

bool HpaeDfxMapTree::DisConnectNodes(uint32_t parentId, uint32_t childId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto parent = FindDfxNode(parentId);
    auto child = FindDfxNode(childId);
    if (parent == nullptr || child == nullptr) {
        AUDIO_WARNING_LOG("Parent %{public}u or child %{public}u not found", parentId, childId);
        return false;
    }

    bool parentRemoved = parent->RemoveChild(childId);
    bool childRemoved = child->RemoveParent(parentId);
    if (!parentRemoved || !childRemoved) {
        AUDIO_WARNING_LOG("Connect not found, parent %{public}u, child %{public}u", parentId, childId);
        return false;
    }
    
    AUDIO_INFO_LOG("Disconnect success, parent name:%{public}s id:%{public}u, child name:%{public}s id:%{public}u",
        parent->GetNodeName().c_str(), parentId, child->GetNodeName().c_str(), childId);
    return true;
}

void HpaeDfxMapTree::UpdateNodeInfo(uint32_t nodeId, const HpaeDfxNodeInfo &nodeInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto node = FindDfxNode(nodeId);
    CHECK_AND_RETURN_LOG(node != nullptr, "Node %{public}u not found", nodeId);
    node->SetNodeInfo(nodeInfo);
    AUDIO_INFO_LOG("Update node %{public}u success", nodeId);
}

bool HpaeDfxMapTree::CheckCycleConnection(uint32_t parentId, uint32_t childId) const
{
    std::queue<uint32_t> q;
    q.push(childId);

    while (!q.empty()) {
        uint32_t currentId = q.front();
        q.pop();

        CHECK_AND_RETURN_RET(currentId != parentId, true); // cycle connect

        auto current = FindDfxNode(currentId);
        if (current) {
            const auto &childrenSet = current->GetChildrenIds();
            for (uint32_t grandChildId : childrenSet) {
                q.push(grandChildId);
            }
        }
    }
    return false;
}

std::vector<uint32_t> HpaeDfxMapTree::GetRoots() const
{
    std::vector<uint32_t> roots;
    for (const auto &pair : nodeMap_) {
        if (pair.second->IsRoot()) {
            roots.push_back(pair.first);
        }
    }
    return roots;
}

void HpaeDfxMapTree::PrintTree(std::string &outStr) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (nodeMap_.empty()) {
        outStr += "Graph is empty\n";
        return;
    }
    auto roots = GetRoots();
    outStr += "Nodes Connect Graph Structure (Total nodes: " + std::to_string(nodeMap_.size()) +
        ", Total Roots: " + std::to_string(roots.size()) + ")\n";

    for (size_t i = 0; i < roots.size(); ++i) {
        auto root = FindDfxNode(roots[i]);
        if (root) {
            outStr += "\nTree " + std::to_string(i + 1) + " (Root: " + root->GetNodeInfo().nodeName + "):\n";
            PrintNodeInfo(outStr, root->GetNodeInfo(), root->GetParentCount());

            const auto &childrenSet =  root->GetChildrenIds();
            uint32_t childId = 0;
            bool childIsLast = false;
            for (auto it = childrenSet.begin(); it != childrenSet.end();) {
                childId = *it;
                childIsLast = (++it == childrenSet.end());
                PrintSubTree(childId, "", childIsLast, outStr);
            }
            outStr += "\n";
        }
    }
}

void HpaeDfxMapTree::PrintSubTree(uint32_t nodeId, const std::string &prefix, bool isLastChild,
    std::string &outStr) const
{
    auto node = FindDfxNode(nodeId);
    CHECK_AND_RETURN(node != nullptr);

    outStr += prefix;
    outStr += (isLastChild ? "|___ " : "|--- ");
    PrintNodeInfo(outStr, node->GetNodeInfo(), node->GetParentCount());
    std::string newPrefix = prefix + (isLastChild ? "     " : "|    ");
    const auto &childrenSet = node->GetChildrenIds();
    uint32_t childId = 0;
    bool childIsLast = false;
    for (auto it = childrenSet.begin(); it != childrenSet.end();) {
        childId = *it;
        childIsLast = (++it == childrenSet.end());
        PrintSubTree(childId, newPrefix, childIsLast, outStr);
    }
}

void HpaeDfxMapTree::PrintNodeInfo(std::string &outStr, const HpaeDfxNodeInfo &nodeInfo,
    size_t parentCount)
{
    outStr = outStr + nodeInfo.nodeName + ": " + "sessionId[" + std::to_string(nodeInfo.sessionId) + "],";
    outStr = outStr + "nodeId[" + std::to_string(nodeInfo.nodeId) + "],";
    outStr = outStr + "rate[" + std::to_string(nodeInfo.samplingRate) + "],";
    outStr = outStr + "ch[" + std::to_string(nodeInfo.channels) + "],";
    outStr = outStr + "bw[" + std::to_string(nodeInfo.format) + "],";
    outStr = outStr + "len[" + std::to_string(nodeInfo.frameLen) + "],";
    outStr = outStr + "scene[" + std::to_string(nodeInfo.sceneType) + "],";
    outStr = outStr + "streamType[" + std::to_string(nodeInfo.streamType) + "]";

    if (parentCount > 0) {
        // parent num, record node parent may > 1, mic、ec、micref
        outStr += ", Parents[" + std::to_string(parentCount) + "]";
    }
    outStr += "\n";
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS