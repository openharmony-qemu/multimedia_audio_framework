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
#ifndef HPAE_DFX_TREE_H
#define HPAE_DFX_TREE_H
#include "hpae_define.h"
#include <vector>
#include <memory>
#include <queue>
#include <string>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class DfxTreeNode {
public:
    explicit DfxTreeNode(HpaeDfxNodeInfo val) : nodeInfo_(val)
    {}
    ~DfxTreeNode()
    {
        for (auto child : children_) {
            delete child;
        }
    }
    HpaeDfxNodeInfo nodeInfo_;
    std::vector<DfxTreeNode *> children_;
};

class HpaeDfxTree {
public:
    HpaeDfxTree() : root_(nullptr)
    {}
    ~HpaeDfxTree()
    {
        delete root_;
    }
    std::vector<std::vector<HpaeDfxNodeInfo>> LevelOrderTraversal();
    bool Insert(const uint32_t parentNodeId, const HpaeDfxNodeInfo &info);
    bool Remove(const uint32_t nodeId);
    void PrintTree(std::string &outStr);
    void UpdateNodeInfo(uint32_t nodeId, const HpaeDfxNodeInfo &nodeInfo);
    DfxTreeNode *GetRoot() const { return root_; }
private:
    DfxTreeNode *FindDfxNode(DfxTreeNode *currentNode, const uint32_t nodeId);
    DfxTreeNode *FindDfxParent(DfxTreeNode *target);
    void PrintSubTree(DfxTreeNode *node, const std::string &prefix, bool isLastChild, std::string &outStr);
    void PrintNodeInfo(std::string &outStr, HpaeDfxNodeInfo &nodeInfo);
    DfxTreeNode *root_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif