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
#ifndef HPAE_DFX_MAP_TREE_H
#define HPAE_DFX_MAP_TREE_H
#include "hpae_define.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class DfxMapTreeNode {
public:
    explicit DfxMapTreeNode(const HpaeDfxNodeInfo &info);
    ~DfxMapTreeNode() = default;
    const HpaeDfxNodeInfo &GetNodeInfo() const;
    uint32_t GetNodeId() const;
    const std::string &GetNodeName() const;
    void SetNodeInfo(const HpaeDfxNodeInfo &info);
    const std::set<uint32_t> &GetParentIds() const;
    const std::set<uint32_t> &GetChildrenIds() const;

    bool IsRoot() const;
    void AddParent(uint32_t parentId);
    bool RemoveParent(uint32_t parentId);
    size_t GetParentCount() const;

    bool IsLeaf() const;
    void AddChild(uint32_t childId);
    bool RemoveChild(uint32_t childId);
    size_t GetChildrenCount() const;
private:
    HpaeDfxNodeInfo nodeInfo_;
    std::set<uint32_t> parentIds_;
    std::set<uint32_t> childrenIds_;
};

class HpaeDfxMapTree {
public:
    HpaeDfxMapTree() = default;
    ~HpaeDfxMapTree() = default;

    bool AddNode(const HpaeDfxNodeInfo &nodeInfo);
    bool RemoveNode(uint32_t nodeId);
    void UpdateNodeInfo(uint32_t nodeId, const HpaeDfxNodeInfo &nodeInfo);

    bool ConnectNodes(uint32_t parentId, uint32_t childId);
    bool DisConnectNodes(uint32_t parentId, uint32_t childId);

    void PrintTree(std::string &outStr) const;
private:
    std::shared_ptr<DfxMapTreeNode> FindDfxNode(uint32_t nodeId);
    std::shared_ptr<const DfxMapTreeNode> FindDfxNode(uint32_t nodeId) const;
    bool CheckCycleConnection(uint32_t parentId, uint32_t childId) const;
    std::vector<uint32_t> GetRoots() const;
    void PrintSubTree(uint32_t nodeId, const std::string &prefix, bool isLastChild, std::string &outStr) const;
    static void PrintNodeInfo(std::string &outStr, const HpaeDfxNodeInfo &nodeInfo,
        size_t parentCount = 0);

private:
    std::unordered_map<uint32_t, std::shared_ptr<DfxMapTreeNode>> nodeMap_;
    mutable std::mutex mutex_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif