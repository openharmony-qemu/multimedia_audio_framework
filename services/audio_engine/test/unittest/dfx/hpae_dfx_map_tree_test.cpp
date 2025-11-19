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
#include "hpae_dfx_map_tree.h"
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t FRAME_LENGTH_882 = 882;
constexpr uint32_t ID_1001 = 1001;
constexpr uint32_t ID_1002 = 1002;
constexpr uint32_t ID_1003 = 1003;
constexpr uint32_t ID_1004 = 1004;
constexpr uint32_t NOT_EXIT_ID_9998 = 9998;
constexpr uint32_t NOT_EXIT_ID_9999 = 9999;
class HpaeDfxMapTreeTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    HpaeDfxNodeInfo CreateNodeInfo(uint32_t nodeId, const string &name, uint32_t sessionId = ID_1001);
    
    std::unique_ptr<HpaeDfxMapTree> dfxTree_;
    HpaeDfxNodeInfo node1_;
    HpaeDfxNodeInfo node2_;
    HpaeDfxNodeInfo node3_;
    HpaeDfxNodeInfo node4_;
};

void HpaeDfxMapTreeTest::SetUpTestCase(void) {}

void HpaeDfxMapTreeTest::TearDownTestCase(void) {}

void HpaeDfxMapTreeTest::SetUp()
{
    dfxTree_ = std::make_unique<HpaeDfxMapTree>();
    
    node1_ = CreateNodeInfo(ID_1001, "Node1", ID_1001);
    node2_ = CreateNodeInfo(ID_1002, "Node2", ID_1002);
    node3_ = CreateNodeInfo(ID_1003, "Node3", ID_1003);
    node4_ = CreateNodeInfo(ID_1004, "Node4", ID_1004);
}

void HpaeDfxMapTreeTest::TearDown()
{
    dfxTree_.reset();
}

HpaeDfxNodeInfo HpaeDfxMapTreeTest::CreateNodeInfo(uint32_t nodeId, const string &name, uint32_t sessionId)
{
    HpaeDfxNodeInfo info;
    info.nodeId = nodeId;
    info.nodeName = name;
    info.sessionId = sessionId;
    info.frameLen = FRAME_LENGTH_960;
    info.samplingRate = SAMPLE_RATE_48000;
    info.format = SAMPLE_S16LE;
    info.channels = STEREO;
    info.channelLayout = CH_LAYOUT_STEREO;
    info.fadeType = NONE_FADE;
    info.streamType = STREAM_MUSIC;
    info.sceneType = HPAE_SCENE_DEFAULT;
    info.deviceClass = "SPEAKER";
    info.deviceNetId = "local";
    info.deviceName = "Speaker";
    info.sourceType = SOURCE_TYPE_INVALID;
    return info;
}

/**
 * @tc.name  : Test Add Node Success
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_001
 * @tc.desc  : test add node successfully
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_001, TestSize.Level1)
{
    bool result = dfxTree_->AddNode(node1_);
    EXPECT_TRUE(result);
    
    auto node = dfxTree_->FindDfxNode(node1_.nodeId);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->GetNodeId(), node1_.nodeId);
    EXPECT_EQ(node->GetNodeInfo().nodeName, "Node1");
}

/**
 * @tc.name  : Test Add Duplicate Node
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_002
 * @tc.desc  : test add duplicate node should fail
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_002, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    
    bool result = dfxTree_->AddNode(node1_);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test Remove Node Success
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_003
 * @tc.desc  : test remove node successfully
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_003, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    
    bool result = dfxTree_->RemoveNode(node1_.nodeId);
    EXPECT_TRUE(result);
    
    auto node = dfxTree_->FindDfxNode(node1_.nodeId);
    EXPECT_EQ(node, nullptr);
}

/**
 * @tc.name  : Test Remove Non-existent Node
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_004
 * @tc.desc  : test remove non-existent node should fail
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_004, TestSize.Level1)
{
    bool result = dfxTree_->RemoveNode(NOT_EXIT_ID_9999);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test Remove Node With Connections
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_005
 * @tc.desc  : test remove node with parent and child connections
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_005, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    dfxTree_->AddNode(node3_);
    dfxTree_->ConnectNodes(node1_.nodeId, node2_.nodeId);
    dfxTree_->ConnectNodes(node2_.nodeId, node3_.nodeId);
    
    bool result = dfxTree_->RemoveNode(node2_.nodeId);
    EXPECT_TRUE(result);
    
    auto node1 = dfxTree_->FindDfxNode(node1_.nodeId);
    auto node2 = dfxTree_->FindDfxNode(node2_.nodeId);
    auto node3 = dfxTree_->FindDfxNode(node3_.nodeId);
    
    EXPECT_NE(node1, nullptr);
    EXPECT_EQ(node2, nullptr);
    EXPECT_NE(node3, nullptr);
    EXPECT_EQ(node1->GetChildrenCount(), 0);
    EXPECT_EQ(node3->GetParentCount(), 0);
}

/**
 * @tc.name  : Test Connect Nodes Success
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_006
 * @tc.desc  : test connect nodes successfully
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_006, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    
    bool result = dfxTree_->ConnectNodes(node1_.nodeId, node2_.nodeId);
    EXPECT_TRUE(result);
    
    auto parentNode = dfxTree_->FindDfxNode(node1_.nodeId);
    auto childNode = dfxTree_->FindDfxNode(node2_.nodeId);
    
    EXPECT_EQ(parentNode->GetChildrenCount(), 1);
    EXPECT_EQ(childNode->GetParentCount(), 1);
    EXPECT_TRUE(parentNode->GetChildrenIds().count(node2_.nodeId) > 0);
    EXPECT_TRUE(childNode->GetParentIds().count(node1_.nodeId) > 0);
}

/**
 * @tc.name  : Test Connect Self Node
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_007
 * @tc.desc  : test connect node to itself should fail
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_007, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    
    bool result = dfxTree_->ConnectNodes(node1_.nodeId, node1_.nodeId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test Connect Non-existent Nodes
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_008
 * @tc.desc  : test connect non-existent nodes should fail
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_008, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    
    bool result1 = dfxTree_->ConnectNodes(node1_.nodeId, NOT_EXIT_ID_9999);
    bool result2 = dfxTree_->ConnectNodes(NOT_EXIT_ID_9999, node1_.nodeId);
    
    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name  : Test Cycle Connection Detection
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_009
 * @tc.desc  : test cycle connection should be detected and prevented
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_009, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    dfxTree_->AddNode(node3_);
    
    dfxTree_->ConnectNodes(node1_.nodeId, node2_.nodeId);
    dfxTree_->ConnectNodes(node2_.nodeId, node3_.nodeId);
    
    bool result = dfxTree_->ConnectNodes(node3_.nodeId, node1_.nodeId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test Disconnect Nodes Success
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_010
 * @tc.desc  : test disconnect nodes successfully
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_010, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    dfxTree_->ConnectNodes(node1_.nodeId, node2_.nodeId);
    
    bool result = dfxTree_->DisConnectNodes(node1_.nodeId, node2_.nodeId);
    EXPECT_TRUE(result);
    
    auto parentNode = dfxTree_->FindDfxNode(node1_.nodeId);
    auto childNode = dfxTree_->FindDfxNode(node2_.nodeId);
    
    EXPECT_EQ(parentNode->GetChildrenCount(), 0);
    EXPECT_EQ(childNode->GetParentCount(), 0);
}

/**
 * @tc.name  : Test Disconnect Non-existent Connection
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_011
 * @tc.desc  : test disconnect non-existent connection should fail
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_011, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    
    bool result = dfxTree_->DisConnectNodes(node1_.nodeId, node2_.nodeId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test Disconnect Non-existent Nodes
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_012
 * @tc.desc  : test disconnect non-existent nodes should fail
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_012, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    
    bool result1 = dfxTree_->DisConnectNodes(node1_.nodeId, NOT_EXIT_ID_9999);
    bool result2 = dfxTree_->DisConnectNodes(NOT_EXIT_ID_9999, node1_.nodeId);
    
    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
}

/**
 * @tc.name  : Test Find Node Const Version
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_013
 * @tc.desc  : test find node const version
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_013, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    
    const auto& constTree = *dfxTree_;
    auto node = constTree.FindDfxNode(node1_.nodeId);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->GetNodeId(), node1_.nodeId);
}

/**
 * @tc.name  : Test Update Node Info
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_014
 * @tc.desc  : test update nodeinfo
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_014, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    dfxTree_->ConnectNodes(node1_.nodeId, node2_.nodeId);
    uint32_t testSession = 2002; // 2002 for test sessionId
    HpaeDfxNodeInfo nodeInfo = CreateNodeInfo(ID_1002, "TestUpdate", testSession);
    nodeInfo.frameLen = FRAME_LENGTH_882;
    nodeInfo.samplingRate = SAMPLE_RATE_44100;
    nodeInfo.format = SAMPLE_S24LE;
    nodeInfo.channels = MONO;
    nodeInfo.channelLayout = CH_LAYOUT_MONO;
    nodeInfo.deviceClass = "BT";
    nodeInfo.sourceType = SOURCE_TYPE_INVALID;
    
    dfxTree_->UpdateNodeInfo(node2_.nodeId, nodeInfo);

    auto node = dfxTree_->FindDfxNode(node2_.nodeId);
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->GetNodeInfo().nodeName, "TestUpdate");
    EXPECT_EQ(node->GetNodeInfo().sessionId, testSession);
    EXPECT_EQ(node->GetNodeInfo().frameLen, FRAME_LENGTH_882);
    EXPECT_EQ(node->GetNodeInfo().samplingRate, SAMPLE_RATE_44100);
    EXPECT_EQ(node->GetNodeInfo().format, SAMPLE_S24LE);
    EXPECT_EQ(node->GetNodeInfo().channels, MONO);
    
    EXPECT_EQ(node->GetParentCount(), 1);
    EXPECT_TRUE(node->GetParentIds().count(node1_.nodeId) > 0);
}

/**
 * @tc.name  : Test Update Non-existent Node
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_015
 * @tc.desc  : test update nodeinfo which not exit
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_015, TestSize.Level1)
{
    HpaeDfxNodeInfo nodeInfo = CreateNodeInfo(NOT_EXIT_ID_9999, "NotExit", NOT_EXIT_ID_9999);
    
    dfxTree_->UpdateNodeInfo(NOT_EXIT_ID_9999, nodeInfo);
    
    auto node = dfxTree_->FindDfxNode(NOT_EXIT_ID_9999);
    EXPECT_EQ(node, nullptr);
}

/**
 * @tc.name  : Test Get Roots
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_016
 * @tc.desc  : test get roots from tree
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_016, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    dfxTree_->AddNode(node3_);
    dfxTree_->ConnectNodes(node1_.nodeId, node2_.nodeId);
    dfxTree_->ConnectNodes(node2_.nodeId, node3_.nodeId);
    
    auto roots = dfxTree_->GetRoots();
    EXPECT_EQ(roots.size(), 1);
    EXPECT_EQ(roots[0], node1_.nodeId);
}

/**
 * @tc.name  : Test Get Multiple Roots
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_017
 * @tc.desc  : test get multiple roots from disconnected graph
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_017, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    dfxTree_->AddNode(node4_);
    
    auto roots = dfxTree_->GetRoots();
    EXPECT_EQ(roots.size(), 3); // 3 for size test
}

/**
 * @tc.name  : Test Print Empty Tree
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_018
 * @tc.desc  : test print empty tree
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_018, TestSize.Level1)
{
    string output;
    dfxTree_->PrintTree(output);
    
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("Graph is empty"), string::npos);
}

/**
 * @tc.name  : Test Print Tree With Nodes
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_019
 * @tc.desc  : test print tree with nodes
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_019, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    dfxTree_->AddNode(node3_);
    dfxTree_->ConnectNodes(node1_.nodeId, node2_.nodeId);
    dfxTree_->ConnectNodes(node2_.nodeId, node3_.nodeId);
    
    string output;
    dfxTree_->PrintTree(output);
    
    EXPECT_FALSE(output.empty());
    EXPECT_NE(output.find("Node1"), string::npos);
    EXPECT_NE(output.find("Node2"), string::npos);
    EXPECT_NE(output.find("Node3"), string::npos);
    EXPECT_NE(output.find("Total nodes: 3"), string::npos);
}

/**
 * @tc.name  : Test DfxMapTreeNode Methods
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_020
 * @tc.desc  : test DfxMapTreeNode all public methods
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_020, TestSize.Level1)
{
    DfxMapTreeNode node(node1_);
    
    EXPECT_EQ(node.GetNodeId(), node1_.nodeId);
    EXPECT_EQ(node.GetNodeInfo().nodeName, "Node1");
    EXPECT_TRUE(node.IsRoot());
    EXPECT_TRUE(node.IsLeaf());
    EXPECT_EQ(node.GetParentCount(), 0);
    EXPECT_EQ(node.GetChildrenCount(), 0);
    
    node.AddParent(NOT_EXIT_ID_9998);
    node.AddChild(NOT_EXIT_ID_9999);
    
    EXPECT_FALSE(node.IsRoot());
    EXPECT_FALSE(node.IsLeaf());
    EXPECT_EQ(node.GetParentCount(), 1);
    EXPECT_EQ(node.GetChildrenCount(), 1);
    
    bool removeParent = node.RemoveParent(NOT_EXIT_ID_9998);
    bool removeChild = node.RemoveChild(NOT_EXIT_ID_9999);
    
    EXPECT_TRUE(removeParent);
    EXPECT_TRUE(removeChild);
    EXPECT_TRUE(node.IsRoot());
    EXPECT_TRUE(node.IsLeaf());
}

/**
 * @tc.name  : Test Complex Multi-level Connection
 * @tc.type  : FUNC
 * @tc.number: HpaeDfxMapTree_021
 * @tc.desc  : test complex multi-level connection scenario
 */
HWTEST_F(HpaeDfxMapTreeTest, HpaeDfxMapTree_021, TestSize.Level1)
{
    dfxTree_->AddNode(node1_);
    dfxTree_->AddNode(node2_);
    dfxTree_->AddNode(node3_);
    dfxTree_->AddNode(node4_);
    
    dfxTree_->ConnectNodes(node1_.nodeId, node2_.nodeId);
    dfxTree_->ConnectNodes(node2_.nodeId, node3_.nodeId);
    dfxTree_->ConnectNodes(node1_.nodeId, node4_.nodeId);
    
    auto node1 = dfxTree_->FindDfxNode(node1_.nodeId);
    auto node2 = dfxTree_->FindDfxNode(node2_.nodeId);
    auto node3 = dfxTree_->FindDfxNode(node3_.nodeId);
    auto node4 = dfxTree_->FindDfxNode(node4_.nodeId);
    
    EXPECT_EQ(node1->GetChildrenCount(), 2); // 2 for test count
    EXPECT_EQ(node2->GetParentCount(), 1);
    EXPECT_EQ(node2->GetChildrenCount(), 1);
    EXPECT_EQ(node3->GetParentCount(), 1);
    EXPECT_EQ(node4->GetParentCount(), 1);
}

} // HPAE
} // AudioStandard
} // OHOS