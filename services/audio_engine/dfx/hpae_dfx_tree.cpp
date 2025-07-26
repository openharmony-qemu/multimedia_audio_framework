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
#define LOG_TAG "HpaeDfxTree"
#endif
#include "hpae_dfx_tree.h"
#include "audio_engine_log.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

DfxTreeNode *HpaeDfxTree::FindDfxNode(DfxTreeNode *currentNode, const uint32_t nodeId)
{
    if (!currentNode) {
        return nullptr;
    }
    std::queue<DfxTreeNode *> q;
    q.push(currentNode);
    while (!q.empty()) {
        DfxTreeNode *node = q.front();
        q.pop();
        if (node->nodeInfo_.nodeId == nodeId) {
            return node;
        }
        for (auto &child : node->children_) {
            q.push(child);
        }
    }
    return nullptr;
}

DfxTreeNode *HpaeDfxTree::FindDfxParent(DfxTreeNode *target)
{
    if (!root_ || target == root_) {
        return nullptr;
    }
    std::queue<DfxTreeNode *> q;
    q.push(root_);
    while (!q.empty()) {
        DfxTreeNode *node = q.front();
        q.pop();
        for (auto &child : node->children_) {
            if (child->nodeInfo_.nodeId == target->nodeInfo_.nodeId) {
                return node;
            }
            q.push(child);
        }
    }
    return nullptr;
}

bool HpaeDfxTree::Insert(const uint32_t parentNodeId, const HpaeDfxNodeInfo &info)
{
    if (!root_) {
        AUDIO_INFO_LOG("Insert Root is null");
        root_ = new DfxTreeNode(info);
        return true;
    }
    DfxTreeNode *parent = FindDfxNode(root_, parentNodeId);
    if (!parent) {
        AUDIO_INFO_LOG("Insert can not find correct parent");
        return false;
    }
    auto it = find_if(parent->children_.begin(), parent->children_.end(),
        [&info](DfxTreeNode *node) -> bool { return node->nodeInfo_.nodeId == info.nodeId; });
    if (it == parent->children_.end()) {
        parent->children_.push_back(new DfxTreeNode(info));
    }
    return true;
}

bool HpaeDfxTree::Remove(const uint32_t nodeId)
{
    if (!root_) {
        AUDIO_INFO_LOG("Remove Root is null");
        return false;
    }
    DfxTreeNode *nodeToRemove = FindDfxNode(root_, nodeId);
    if (!nodeToRemove) {
        return false;
    }

    if (nodeToRemove == root_) {
        delete root_;
        root_ = nullptr;
        return true;
    }

    DfxTreeNode *parent = FindDfxParent(nodeToRemove);
    if (!parent) {
        return false;
    }
    // Remove from parent's children
    auto &children = parent->children_;
    auto it = find(children.begin(), children.end(), nodeToRemove);
    if (it != children.end()) {
        children.erase(it);
        delete nodeToRemove;
        return true;
    }
    return false;
}

std::vector<std::vector<HpaeDfxNodeInfo>> HpaeDfxTree::LevelOrderTraversal()
{
    std::vector<std::vector<HpaeDfxNodeInfo>> result;
    if (!root_) {
        return result;
    }
    std::queue<DfxTreeNode *> q;
    q.push(root_);
    while (!q.empty()) {
        size_t levelSize = q.size();
        std::vector<HpaeDfxNodeInfo> curLevelResult;
        for (size_t i = 0; i < levelSize; ++i) {
            DfxTreeNode *node = q.front();
            q.pop();
            curLevelResult.push_back(node->nodeInfo_);
            for (auto &child : node->children_) {
                q.push(child);
            }
        }
        result.push_back(curLevelResult);
    }
    return result;
}

void HpaeDfxTree::PrintNodeInfo(std::string &outStr, HpaeDfxNodeInfo &nodeInfo)
{
    outStr = outStr + nodeInfo.nodeName + ": " + "sessionId[" + std::to_string(nodeInfo.sessionId) + "],";
    outStr = outStr + "nodeId[" + std::to_string(nodeInfo.nodeId) + "],";
    outStr = outStr + "rate[" + std::to_string(nodeInfo.samplingRate) + "],";
    outStr = outStr + "ch[" + std::to_string(nodeInfo.channels) + "],";
    outStr = outStr + "bw[" + std::to_string(nodeInfo.format) + "],";
    outStr = outStr + "len[" + std::to_string(nodeInfo.frameLen) + "],";
    outStr = outStr + "scene[" + std::to_string(nodeInfo.sceneType) + "] \n";
}

void HpaeDfxTree::PrintSubTree(DfxTreeNode *node, const std::string &prefix, bool isLastChild, std::string &outStr)
{
    if (!node) {
        return;
    }

    outStr = outStr + prefix;
    outStr = outStr + (isLastChild ? "|___ " : "|--- ");
    PrintNodeInfo(outStr, node->nodeInfo_);
    std::string newPrefix = prefix + (isLastChild ? "    " : "|   ");
    for (size_t i = 0; i < node->children_.size(); ++i) {
        bool childIsLast = (i == node->children_.size() - 1);
        PrintSubTree(node->children_[i], newPrefix, childIsLast, outStr);
    }
}

void HpaeDfxTree::PrintTree(std::string &outStr)
{
    if (!root_) {
        return;
    }
    PrintNodeInfo(outStr, root_->nodeInfo_);
    for (size_t i = 0; i < root_->children_.size(); ++i) {
        bool isLast = (i == root_->children_.size() - 1);
        PrintSubTree(root_->children_[i], "", isLast, outStr);
    }
}

void HpaeDfxTree::UpdateNodeInfo(uint32_t nodeId, const HpaeDfxNodeInfo &nodeInfo)
{
    if (root_ == nullptr) {
        AUDIO_WARNING_LOG("Hidumper dfx tree is empty!");
        return;
    }
    DfxTreeNode *target = FindDfxNode(root_, nodeId);
    if (target == nullptr) {
        AUDIO_WARNING_LOG("Cannot find Node Id: %{public}d", nodeId);
        return;
    }
    target->nodeInfo_ = nodeInfo;
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS