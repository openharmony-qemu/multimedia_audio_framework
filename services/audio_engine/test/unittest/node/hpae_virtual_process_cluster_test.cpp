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
#include "audio_errors.h"
#include "hpae_virtual_process_cluster.h"
#include "hpae_mixer_node.h"
#include "hpae_gain_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_node_common.h"
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static constexpr uint32_t DEFAULT_SESSION_ID01 = 100000;
static constexpr uint32_t DEFAULT_SESSION_ID02 = 100001;
class HpaeVirtualProcessClusterTest : public testing::Test {
public:
    void SetUp() override {}

    void TearDown() override {}
};

static HpaeNodeInfo GetNodeInfo()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = 960; // 960 for framelen
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sessionId = DEFAULT_SESSION_ID01;
    return nodeInfo;
}

HWTEST_F(HpaeVirtualProcessClusterTest, ConstructHpaeVirtualProcessCluster, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    
    EXPECT_NE(cluster->GetSharedInstance(), nullptr);
    std::shared_ptr<HpaeNode> mixerInstance = cluster->GetSharedInstance();
    EXPECT_NE(mixerInstance, nullptr);
    OutputPort<HpaePcmBuffer*>* port = cluster->GetOutputPort();
    EXPECT_NE(port, nullptr);
}

HWTEST_F(HpaeVirtualProcessClusterTest, DestructorCleansUpResources, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    HpaeNodeInfo nodeInfo1 = nodeInfo;
    nodeInfo1.samplingRate = SAMPLE_RATE_44100;
    nodeInfo1.frameLen = CalculateFrameLenBySampleRate(nodeInfo1.samplingRate);
    std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(nodeInfo1);
    cluster->Connect(preNode);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 1);
}

HWTEST_F(HpaeVirtualProcessClusterTest, DoProcessDelegatesToMixerNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    EXPECT_NO_FATAL_FAILURE(cluster->DoProcess());
}

HWTEST_F(HpaeVirtualProcessClusterTest, ResetClearsMixerNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    cluster->Connect(preNode);
    EXPECT_EQ(preNode.use_count(), 2);
    EXPECT_EQ(cluster->Reset(), true);
    EXPECT_EQ(preNode.use_count(), 2);
}

HWTEST_F(HpaeVirtualProcessClusterTest, ResetAllClearsAllConnections, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    
    HpaeNodeInfo info1 = GetNodeInfo();;
    info1.sessionId = DEFAULT_SESSION_ID01;
    HpaeNodeInfo info2 = GetNodeInfo();;
    info2.sessionId = DEFAULT_SESSION_ID02;
    
    std::shared_ptr<HpaeSinkInputNode> preNode1 = std::make_shared<HpaeSinkInputNode>(info1);
    std::shared_ptr<HpaeSinkInputNode> preNode2 = std::make_shared<HpaeSinkInputNode>(info2);
    cluster->Connect(preNode1);
    cluster->Connect(preNode2);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 2);
    cluster->DisConnect(preNode2);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 1);
    EXPECT_EQ(cluster->ResetAll(), true);
}

HWTEST_F(HpaeVirtualProcessClusterTest, GetSharedInstanceReturnsMixerNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    EXPECT_EQ(cluster->GetSharedInstance()!= nullptr, true);
    EXPECT_EQ(cluster->GetSharedInstance()->GetChannelCount(), nodeInfo.channels);
}

HWTEST_F(HpaeVirtualProcessClusterTest, GetOutputPortReturnsMixerPort, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    EXPECT_EQ(cluster->GetOutputPort() != nullptr, true);
}

HWTEST_F(HpaeVirtualProcessClusterTest, ConnectCreatesGainAndConverterNodes, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    HpaeNodeInfo preNodeInfo = GetNodeInfo();
    preNodeInfo.sessionId = DEFAULT_SESSION_ID01;
    std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(preNodeInfo);
    cluster->Connect(preNode);
    std::shared_ptr<HpaeGainNode> gainNode = cluster->GetGainNodeById(preNodeInfo.sessionId);
    EXPECT_NE(gainNode, nullptr);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 1);
}

HWTEST_F(HpaeVirtualProcessClusterTest, ConnectSameSessionIdNoDuplicates, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    HpaeNodeInfo preNodeInfo = GetNodeInfo();
    preNodeInfo.sessionId = DEFAULT_SESSION_ID01;
    std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(preNodeInfo);
    cluster->Connect(preNode);
    size_t initialCount = cluster->GetConnectSinkInputNum();
    cluster->Connect(preNode);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), initialCount);
}

HWTEST_F(HpaeVirtualProcessClusterTest, DisConnectRemovesNodesFromMaps, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    
    HpaeNodeInfo preNodeInfo = GetNodeInfo();
    preNodeInfo.sessionId = DEFAULT_SESSION_ID01;
    std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(preNodeInfo);
    cluster->Connect(preNode);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 1);
    std::shared_ptr<HpaeGainNode> gainNodeBefore = cluster->GetGainNodeById(preNodeInfo.sessionId);
    EXPECT_NE(gainNodeBefore, nullptr);
    cluster->DisConnect(preNode);
    std::shared_ptr<HpaeGainNode> gainNodeAfter = cluster->GetGainNodeById(preNodeInfo.sessionId);
    EXPECT_EQ(gainNodeAfter, nullptr);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 0);
}

HWTEST_F(HpaeVirtualProcessClusterTest, DisConnectNonExistentSession, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    HpaeNodeInfo preNodeInfo = GetNodeInfo();
    preNodeInfo.sessionId = DEFAULT_SESSION_ID01;
    std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(preNodeInfo);
    EXPECT_NO_FATAL_FAILURE(cluster->DisConnect(preNode));
}

HWTEST_F(HpaeVirtualProcessClusterTest, SetupAudioLimiterReturnsSuccess, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    
    int32_t result = cluster->SetupAudioLimiter();
    EXPECT_EQ(result, SUCCESS);
}

HWTEST_F(HpaeVirtualProcessClusterTest, GetConnectSinkInputNumReturnsCorrectCount, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 0);
    
    HpaeNodeInfo info1 = GetNodeInfo();
    info1.sessionId = DEFAULT_SESSION_ID01;
    HpaeNodeInfo info2 = GetNodeInfo();
    info2.sessionId = DEFAULT_SESSION_ID02;
    
    std::shared_ptr<HpaeSinkInputNode> preNode1 = std::make_shared<HpaeSinkInputNode>(info1);
    std::shared_ptr<HpaeSinkInputNode> preNode2 = std::make_shared<HpaeSinkInputNode>(info2);
    
    cluster->Connect(preNode1);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 1);
    
    cluster->Connect(preNode2);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 2);
    
    cluster->DisConnect(preNode1);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 1);
    
    cluster->DisConnect(preNode2);
    EXPECT_EQ(cluster->GetConnectSinkInputNum(), 0);
}

HWTEST_F(HpaeVirtualProcessClusterTest, GetGainNodeByIdReturnsCorrectNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    
    HpaeNodeInfo preNodeInfo = GetNodeInfo();
    preNodeInfo.sessionId = DEFAULT_SESSION_ID01;
    std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(preNodeInfo);
    
    cluster->Connect(preNode);
    
    std::shared_ptr<HpaeGainNode> gainNode = cluster->GetGainNodeById(preNodeInfo.sessionId);
    EXPECT_NE(gainNode, nullptr);
    EXPECT_EQ(gainNode->GetSessionId(), preNodeInfo.sessionId);
}

HWTEST_F(HpaeVirtualProcessClusterTest, GetGainNodeByIdNonExistentReturnsNull, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    
    std::shared_ptr<HpaeGainNode> gainNode = cluster->GetGainNodeById(9999);
    EXPECT_EQ(gainNode, nullptr);
}

HWTEST_F(HpaeVirtualProcessClusterTest, MultipleConnectDisconnectOperations, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetNodeInfo();
    std::shared_ptr<HpaeVirtualProcessCluster> cluster = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    const int32_t numOperations = 5;
    HpaeNodeInfo preNodeInfo = GetNodeInfo();
    
    for (int32_t i = 0; i < numOperations; i++) {
        preNodeInfo.sessionId = DEFAULT_SESSION_ID01 + i;
        std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(preNodeInfo);
        cluster->Connect(preNode);
        EXPECT_EQ(cluster->GetConnectSinkInputNum(), i + 1);
        std::shared_ptr<HpaeGainNode> gainNode = cluster->GetGainNodeById(preNodeInfo.sessionId);
        EXPECT_NE(gainNode, nullptr);
    }
    
    for (int32_t i = 0; i < numOperations; i++) {
        preNodeInfo.sessionId = DEFAULT_SESSION_ID01 + i;
        std::shared_ptr<HpaeSinkInputNode> preNode = std::make_shared<HpaeSinkInputNode>(preNodeInfo);
        
        cluster->DisConnect(preNode);
        EXPECT_EQ(cluster->GetConnectSinkInputNum(), numOperations - i - 1);
        std::shared_ptr<HpaeGainNode> gainNode = cluster->GetGainNodeById(preNodeInfo.sessionId);
        EXPECT_EQ(gainNode, nullptr);
    }
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
