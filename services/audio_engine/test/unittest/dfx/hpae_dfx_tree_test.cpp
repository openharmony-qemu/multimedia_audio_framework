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
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "hpae_dfx_tree.h"
#include <locale>
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

constexpr uint32_t NUM_TWO = 2;
constexpr uint32_t NUM_THREE = 3;
constexpr uint32_t NUM_FOUR = 4;
constexpr uint32_t NODE_ID_100 = 100;
constexpr uint32_t NODE_ID_101 = 101;
constexpr uint32_t NODE_ID_102 = 102;
constexpr uint32_t NODE_ID_103 = 103;
constexpr uint32_t NODE_ID_123 = 123;
constexpr uint32_t NODE_ID_999 = 999;

class HpaeDfxTreeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    HpaeDfxTree CreateSampleTree();
    DfxTreeNode *FindNode(HpaeDfxTree &tree, uint32_t nodeId)
    {
        return tree.FindDfxNode(tree.GetRoot(), nodeId);
    }
};

void HpaeDfxTreeTest::SetUp()
{
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());
}

void HpaeDfxTreeTest::TearDown()
{}

HpaeDfxTree HpaeDfxTreeTest::CreateSampleTree()
{
    HpaeDfxTree hpaeDfxTree;
    HpaeDfxNodeInfo info;
    uint32_t nodeId = NODE_ID_100;
    uint32_t sessionId = 1000; // 1000: session id

    info.nodeId = nodeId;
    info.sessionId = sessionId;
    hpaeDfxTree.Insert(0, info);
    
    info.nodeId = nodeId + 1;
    info.sessionId = sessionId + 1;
    hpaeDfxTree.Insert(nodeId, info);
    
    info.nodeId = nodeId + NUM_TWO;
    info.sessionId = sessionId + NUM_TWO;
    hpaeDfxTree.Insert(nodeId + 1, info);
    
    info.nodeId = nodeId + NUM_THREE;
    info.sessionId = sessionId + NUM_THREE;
    hpaeDfxTree.Insert(nodeId + 1, info);
    
    info.nodeId = nodeId + NUM_FOUR;
    info.sessionId = sessionId + NUM_FOUR;
    hpaeDfxTree.Insert(nodeId, info);
    
    return hpaeDfxTree;
}

HWTEST_F(HpaeDfxTreeTest, constructHpaeDfxTreeTest, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree;
    HpaeNodeInfo info;
    uint32_t nodeId = NODE_ID_123;
    uint32_t sessionId = 12345; // 12345: session id
    size_t frameLen = 960; // 960: frameLen
    uint32_t preNodeId = 0;
    int32_t testNum = 10; // 10: testNum
    for (int32_t i = 0; i < testNum; i++) {
        preNodeId = info.nodeId;
        info.nodeId = nodeId + i;
        info.sessionId = sessionId + i;
        info.nodeName = "testNode1";
        info.frameLen = frameLen;
        info.channels = STEREO;
        info.samplingRate = SAMPLE_RATE_48000;
        info.format = SAMPLE_F32LE;
        info.sceneType = HPAE_SCENE_DEFAULT;
        EXPECT_EQ(hpaeDfxTree.Insert(preNodeId, info), true);
    }
    std::vector<std::vector<HpaeDfxNodeInfo>> result = hpaeDfxTree.LevelOrderTraversal();
    std::string outStr;
    hpaeDfxTree.PrintTree(outStr);
    std::cout << outStr.c_str() << std::endl;
    int32_t index = 0;
    for (int32_t i = 0; i < result.size(); i++) {
        for (int32_t j = 0; j < result[i].size(); j++) {
            EXPECT_EQ(result[i][j].nodeId, index + nodeId);
            EXPECT_EQ(result[i][j].sessionId, index + sessionId);
            EXPECT_EQ(result[i][j].frameLen, frameLen);
            EXPECT_EQ(result[i][j].samplingRate, SAMPLE_RATE_48000);
            EXPECT_EQ(result[i][j].channels, STEREO);
            EXPECT_EQ(result[i][j].format, SAMPLE_F32LE);
            index++;
        }
    }
}

HWTEST_F(HpaeDfxTreeTest, removeDfxTreeTest, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree;
    HpaeNodeInfo info;
    uint32_t nodeId = NODE_ID_123;
    uint32_t sessionId = 12345; // 12345: session id
    size_t frameLen = 960; // 960: frameLen
    uint32_t preNodeId = 0;
    int32_t testNum = 10; // 10: testLen
    for (int32_t i = 0; i < testNum; i++) {
        preNodeId = info.nodeId;
        info.nodeId = nodeId + i;
        info.sessionId = sessionId + i;
        info.nodeName = "testNode2";
        info.frameLen = frameLen;
        info.channels = MONO;
        info.samplingRate = SAMPLE_RATE_16000;
        info.format = SAMPLE_F32LE;
        info.sceneType = HPAE_SCENE_MUSIC;
        EXPECT_EQ(hpaeDfxTree.Insert(preNodeId, info), true);
    }
    std::vector<std::vector<HpaeDfxNodeInfo>> result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_EQ(result.size(), testNum);
    uint32_t removeNodeIndex = NUM_THREE;
    EXPECT_EQ(hpaeDfxTree.Remove(nodeId + removeNodeIndex), true);
    std::string outStr;
    hpaeDfxTree.PrintTree(outStr);
    std::cout << outStr.c_str() << std::endl;
    result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_EQ(result.size(), removeNodeIndex);
}

HWTEST_F(HpaeDfxTreeTest, constructHpaeDfxTreeTest_002, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree;
    HpaeDfxNodeInfo info;
    uint32_t nodeId = NODE_ID_123;
    uint32_t sessionId = 12345; // 12345: session id
    size_t frameLen = 960; // 960: frameLen
    uint32_t preNodeId = 0;
    int32_t testNum = 10; // 10: testLen
    for (int32_t i = 0; i < testNum; i++) {
        preNodeId = info.nodeId;
        info.nodeId = nodeId + i;
        info.sessionId = sessionId + i;
        info.nodeName = "testNode1";
        info.frameLen = frameLen;
        info.channels = STEREO;
        info.samplingRate = SAMPLE_RATE_48000;
        info.format = SAMPLE_F32LE;
        info.sceneType = HPAE_SCENE_DEFAULT;
        EXPECT_EQ(hpaeDfxTree.Insert(preNodeId, info), true);
    }
    std::vector<std::vector<HpaeDfxNodeInfo>> result = hpaeDfxTree.LevelOrderTraversal();
    std::string outStr;
    hpaeDfxTree.PrintTree(outStr);
    std::cout << outStr.c_str() << std::endl;
    int32_t index = 0;
    for (int32_t i = 0; i < result.size(); i++) {
        for (int32_t j = 0; j < result[i].size(); j++) {
            EXPECT_EQ(result[i][j].nodeId, index + nodeId);
            EXPECT_EQ(result[i][j].sessionId, index + sessionId);
            EXPECT_EQ(result[i][j].frameLen, frameLen);
            EXPECT_EQ(result[i][j].samplingRate, SAMPLE_RATE_48000);
            EXPECT_EQ(result[i][j].channels, STEREO);
            EXPECT_EQ(result[i][j].format, SAMPLE_F32LE);
            index++;
        }
    }
}

HWTEST_F(HpaeDfxTreeTest, removeDfxTreeTest_002, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree = CreateSampleTree();

    auto result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_EQ(result.size(), NUM_THREE);
    EXPECT_EQ(result[0].size(), 1);
    EXPECT_EQ(result[1].size(), NUM_TWO);
    EXPECT_EQ(result[NUM_TWO].size(), NUM_TWO);

    EXPECT_EQ(hpaeDfxTree.Remove(NODE_ID_102), true);
    result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_EQ(result.size(), NUM_THREE);
    EXPECT_EQ(result[NUM_TWO].size(), 1);

    EXPECT_EQ(hpaeDfxTree.Remove(NODE_ID_101), true);
    result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_EQ(result.size(), NUM_TWO);

    EXPECT_EQ(hpaeDfxTree.Remove(NODE_ID_999), false);
}

HWTEST_F(HpaeDfxTreeTest, emptyTreeOperations, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree;

    EXPECT_EQ(hpaeDfxTree.Remove(NODE_ID_100), false);

    auto result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_TRUE(result.empty());

    std::string outStr;
    hpaeDfxTree.PrintTree(outStr);
    EXPECT_TRUE(outStr.empty());
}

HWTEST_F(HpaeDfxTreeTest, invalidInsertionTest, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree;
    HpaeDfxNodeInfo info;

    info.nodeId = NODE_ID_100;
    EXPECT_EQ(hpaeDfxTree.Insert(NODE_ID_999, info), true);

    EXPECT_EQ(hpaeDfxTree.Insert(0, info), false);

    EXPECT_EQ(hpaeDfxTree.Insert(NODE_ID_100, info), true);
}

HWTEST_F(HpaeDfxTreeTest, updateNodeInfoTest, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree = CreateSampleTree();

    HpaeDfxNodeInfo newInfo;
    newInfo.nodeId = NODE_ID_102;
    newInfo.sessionId = 2000; // 2000: session id
    newInfo.nodeName = "UpdatedNode";
    newInfo.frameLen = 512; // 512: frame length
    newInfo.channels = MONO;
    newInfo.samplingRate = SAMPLE_RATE_44100;
    newInfo.format = SAMPLE_S16LE;
    newInfo.sceneType = HPAE_SCENE_MOVIE;
    
    hpaeDfxTree.UpdateNodeInfo(NODE_ID_102, newInfo);

    auto result = hpaeDfxTree.LevelOrderTraversal();
    bool found = false;
    for (const auto& level : result) {
        for (const auto& node : level) {
            if (node.nodeId == NODE_ID_102) {
                found = true;
                EXPECT_EQ(node.sessionId, 2000); // 2000: session id
                EXPECT_EQ(node.frameLen, 512); // 512: frame length
                EXPECT_EQ(node.channels, MONO);
                EXPECT_EQ(node.samplingRate, SAMPLE_RATE_44100);
                EXPECT_EQ(node.format, SAMPLE_S16LE);
                EXPECT_EQ(node.sceneType, HPAE_SCENE_MOVIE);
            }
        }
    }
    EXPECT_TRUE(found);

    hpaeDfxTree.UpdateNodeInfo(NODE_ID_999, newInfo);
}

HWTEST_F(HpaeDfxTreeTest, rootNodeOperations, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree;
    HpaeDfxNodeInfo info;
    info.nodeId = NODE_ID_100;

    EXPECT_EQ(hpaeDfxTree.Insert(0, info), true);

    EXPECT_EQ(hpaeDfxTree.Remove(NODE_ID_100), true);

    auto result = hpaeDfxTree.LevelOrderTraversal();
    EXPECT_TRUE(result.empty());

    EXPECT_EQ(hpaeDfxTree.Remove(NODE_ID_100), false);
}

HWTEST_F(HpaeDfxTreeTest, treeTraversalBoundaryTest, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree;

    HpaeDfxNodeInfo info;
    info.nodeId = NODE_ID_100;
    hpaeDfxTree.Insert(0, info);
    
    auto result = hpaeDfxTree.LevelOrderTraversal();
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0].size(), 1);
    EXPECT_EQ(result[0][0].nodeId, NODE_ID_100);

    info.nodeId = NODE_ID_101;
    hpaeDfxTree.Insert(NODE_ID_100, info);
    
    result = hpaeDfxTree.LevelOrderTraversal();
    ASSERT_EQ(result.size(), NUM_TWO);
    EXPECT_EQ(result[0].size(), 1);
    EXPECT_EQ(result[1].size(), 1);
}

HWTEST_F(HpaeDfxTreeTest, findParentNodeTest, TestSize.Level0)
{
    HpaeDfxTree hpaeDfxTree = CreateSampleTree();
    DfxTreeNode *root = hpaeDfxTree.GetRoot();
    ASSERT_NE(root, nullptr);
    EXPECT_EQ(root->nodeInfo_.nodeId, NODE_ID_100);

    DfxTreeNode *node101 = hpaeDfxTree.FindDfxNode(root, NODE_ID_101);
    ASSERT_NE(node101, nullptr);
    EXPECT_EQ(node101->nodeInfo_.nodeId, NODE_ID_101);

    DfxTreeNode *node102 = hpaeDfxTree.FindDfxNode(root, NODE_ID_102);
    ASSERT_NE(node102, nullptr);
    EXPECT_EQ(node102->nodeInfo_.nodeId, NODE_ID_102);

    DfxTreeNode *parent = hpaeDfxTree.FindDfxParent(node102);
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->nodeInfo_.nodeId, NODE_ID_101);

    DfxTreeNode *node103 = hpaeDfxTree.FindDfxNode(root, NODE_ID_103);
    ASSERT_NE(node103, nullptr);
    parent = hpaeDfxTree.FindDfxParent(node103);
    ASSERT_NE(parent, nullptr);
    EXPECT_EQ(parent->nodeInfo_.nodeId, NODE_ID_101);

    parent = hpaeDfxTree.FindDfxParent(root);
    EXPECT_EQ(parent, nullptr);

    HpaeDfxNodeInfo dummyInfo;
    dummyInfo.nodeId = NODE_ID_999;
    dummyInfo.nodeName = "dummy";
    dummyInfo.sessionId = 9999; // 9999: session id
    DfxTreeNode dummyNode(dummyInfo);

    parent = hpaeDfxTree.FindDfxParent(&dummyNode);
    EXPECT_EQ(parent, nullptr);
    ASSERT_TRUE(hpaeDfxTree.Remove(NODE_ID_102));
    EXPECT_EQ(hpaeDfxTree.FindDfxNode(root, NODE_ID_102), nullptr);
}
} // HPAE
} // AudioStandard
} // OHOS