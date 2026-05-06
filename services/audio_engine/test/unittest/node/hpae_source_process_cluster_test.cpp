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
#include <cmath>
#include <memory>
#include "gmock/gmock.h"
#include "hpae_source_process_cluster.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_source_output_node.h"
#include "hpae_source_input_cluster.h"
#include "hpae_source_input_node.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const uint32_t DEFAULT_FRAME_LENGTH = 960;
constexpr uint64_t INVALID_SCENE_KEY_CODE = 66;

class HpaeSourceProcessClusterTest : public ::testing::Test {
public:
    void SetUp();
    void TearDown();
};

class MockHpaeCaptureEffectNode : public HpaeCaptureEffectNode {
public:
    explicit MockHpaeCaptureEffectNode(HpaeNodeInfo &nodeInfo) : HpaeCaptureEffectNode(nodeInfo) {}
    MOCK_METHOD(int32_t, CaptureEffectCreate, (uint64_t sceneKeyCode, CaptureEffectAttr attr), (override));
    MOCK_METHOD(bool, GetCapturerEffectConfig, (HpaeNodeInfo & nodeInfo, HpaeSourceBufferType type), (override));
    MOCK_METHOD(int32_t, SetAppsEnhanceMuteState, (bool isMute), (override));
};

void HpaeSourceProcessClusterTest::SetUp()
{}

void HpaeSourceProcessClusterTest::TearDown()
{}

HWTEST_F(HpaeSourceProcessClusterTest, constructHpaeSourceProcessClusterNode, TestSize.Level0)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_VOIP_UP;
    nodeInfo.statusCallback = testStatuscallback;
    std::shared_ptr<HpaeSourceProcessCluster> hpaeSourceProcessCluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeSourceProcessCluster->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeSourceProcessCluster->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeSourceProcessCluster->GetBitWidth(), nodeInfo.format);
    EXPECT_EQ(hpaeSourceProcessCluster->GetMixerNodeUseCount(), 1);

    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode1 = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode1->ConnectWithInfo(hpaeSourceProcessCluster, nodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetMixerNodeUseCount(), 1 + 1);
    EXPECT_EQ(hpaeSourceProcessCluster->GetConverterNodeCount(), 0);

    nodeInfo.samplingRate = SAMPLE_RATE_16000;
    std::shared_ptr<HpaeSourceOutputNode> hpaeSourceOutputNode2 = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    hpaeSourceOutputNode2->ConnectWithInfo(hpaeSourceProcessCluster, nodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetMixerNodeUseCount(), 1 + 1 + 1);
    EXPECT_EQ(hpaeSourceProcessCluster->GetConverterNodeCount(), 1);

    hpaeSourceOutputNode2->DisConnectWithInfo(hpaeSourceProcessCluster, nodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetMixerNodeUseCount(), 1 + 1);

    HpaeNodeInfo sourceInputNodeInfo;
    sourceInputNodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInputNodeInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInputNodeInfo.channels = STEREO;
    sourceInputNodeInfo.format = SAMPLE_F32LE;
    sourceInputNodeInfo.statusCallback = testStatuscallback;
    std::shared_ptr<HpaeSourceInputCluster> hpaeSourceInputCluster =
        std::make_shared<HpaeSourceInputCluster>(sourceInputNodeInfo);
    EXPECT_EQ(hpaeSourceProcessCluster->GetPreOutNum(), 0);
    hpaeSourceProcessCluster->ConnectWithInfo(hpaeSourceInputCluster, hpaeSourceProcessCluster->GetNodeInfo());
    EXPECT_EQ(hpaeSourceProcessCluster->GetPreOutNum(), 1);
    hpaeSourceProcessCluster->DisConnectWithInfo(hpaeSourceInputCluster, hpaeSourceProcessCluster->GetNodeInfo());
    EXPECT_EQ(hpaeSourceProcessCluster->GetPreOutNum(), 0);
}

HWTEST_F(HpaeSourceProcessClusterTest, testInterfaces, TestSize.Level0)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_VOIP_UP;
    nodeInfo.statusCallback = testStatuscallback;
    std::shared_ptr<HpaeSourceProcessCluster> hpaeSourceProcessCluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<HpaeSourceInputNode> inputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    std::shared_ptr<HpaeNode> hpaeNode = hpaeSourceProcessCluster->GetSharedInstance();
    ASSERT_NE(hpaeNode, nullptr);
    EXPECT_NE(hpaeSourceProcessCluster->GetSharedInstance(hpaeNode->GetNodeInfo()), nullptr);
    EXPECT_NE(hpaeSourceProcessCluster->GetOutputPort(), nullptr);
    EXPECT_NE(hpaeSourceProcessCluster->GetOutputPort(hpaeNode->GetNodeInfo(), true), nullptr);
    EXPECT_NE(hpaeSourceProcessCluster->GetOutputPort(hpaeNode->GetNodeInfo(), false), nullptr);
    EXPECT_NE(hpaeSourceProcessCluster->GetCapturerEffectConfig(nodeInfo, HPAE_SOURCE_BUFFER_TYPE_MIC), true);
    EXPECT_EQ(hpaeSourceProcessCluster->GetOutputPortNum(), 0);
    hpaeSourceProcessCluster->DoProcess();
    uint64_t sceneKeyCode = INVALID_SCENE_KEY_CODE;
    CaptureEffectAttr attr;
    EXPECT_EQ(hpaeSourceProcessCluster->CaptureEffectRelease(sceneKeyCode), 0);
    EXPECT_NE(hpaeSourceProcessCluster->CaptureEffectCreate(sceneKeyCode, attr), 0);
    EXPECT_NE(hpaeSourceProcessCluster->CaptureEffectRelease(sceneKeyCode), 0);
    hpaeSourceProcessCluster->Connect(inputNode);
    hpaeSourceProcessCluster->DisConnect(inputNode);
    EXPECT_EQ(hpaeSourceProcessCluster->ResetAll(), true);
}

HWTEST_F(HpaeSourceProcessClusterTest, EffectNodeNotNullTest, TestSize.Level1)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_VOIP_UP;
    nodeInfo.statusCallback = testStatuscallback;
    std::shared_ptr<HpaeSourceProcessCluster> hpaeSourceProcessCluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> inputCluster = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    EXPECT_EQ(inputCluster->fmtConverterNodeMap_.size(), 0);
    hpaeSourceProcessCluster->Connect(inputCluster);
    EXPECT_EQ(inputCluster->fmtConverterNodeMap_.size(), 1);
    hpaeSourceProcessCluster->DisConnect(inputCluster);
    EXPECT_EQ(inputCluster->fmtConverterNodeMap_.size(), 1);
}

HWTEST_F(HpaeSourceProcessClusterTest, HpaeSourceProcessClusterCreateEffectTest, TestSize.Level1)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_VOIP_UP;
    nodeInfo.statusCallback = testStatuscallback;
    std::shared_ptr<HpaeSourceProcessCluster> hpaeSourceProcessCluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<MockHpaeCaptureEffectNode> effectNode =
        std::make_shared<NiceMock<MockHpaeCaptureEffectNode>>(nodeInfo);
    hpaeSourceProcessCluster->captureEffectNode_ = effectNode;
    CaptureEffectAttr testAttr;
    EXPECT_CALL(*effectNode, CaptureEffectCreate(_, _))
        .WillOnce([]() { return SUCCESS; }); // Success
    
    // Mock GetCapturerEffectConfig to succeed and modify nodeInfo
    HpaeNodeInfo modifiedNodeInfo = nodeInfo;
    modifiedNodeInfo.frameLen = 1024; // Modified value
    EXPECT_CALL(*effectNode, GetCapturerEffectConfig(_, _))
        .WillOnce([]() { return true; }); // Success with modified nodeInfo
    
    auto result = hpaeSourceProcessCluster->CaptureEffectCreate(12345, testAttr);
    
    EXPECT_EQ(result, 0); // Success
    EXPECT_NE(hpaeSourceProcessCluster->mixerNode_, nullptr); // Mixer node should be created
    EXPECT_EQ(hpaeSourceProcessCluster->captureEffectNode_, effectNode); // Should not be reset
}

HWTEST_F(HpaeSourceProcessClusterTest, HpaeSourceProcessClusterInjectTest, TestSize.Level1)
{
    std::shared_ptr<NodeStatusCallback> testStatuscallback = std::make_shared<NodeStatusCallback>();
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_VOIP_UP;
    nodeInfo.statusCallback = testStatuscallback;
    std::shared_ptr<HpaeSourceProcessCluster> hpaeSourceProcessCluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);

    nodeInfo.channels = MONO;
    std::shared_ptr<HpaeSourceInputNode> inputNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    hpaeSourceProcessCluster->ConnectInjector(inputNode);
    EXPECT_EQ(hpaeSourceProcessCluster->injectorFmtConverterNodeMap_.size() == 1, true);
    hpaeSourceProcessCluster->DisConnectInjector(inputNode);
    EXPECT_EQ(hpaeSourceProcessCluster->injectorFmtConverterNodeMap_.size() == 0, true);
}

static HpaeNodeInfo GetTestNodeInfo()
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sceneType = HPAE_SCENE_VOIP_UP;
    return nodeInfo;
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_001
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_001
 * @tc.desc: Test HpaeSourceProcessCluster constructor and destructor
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    EXPECT_NE(cluster, nullptr);
    
    // Test that internal nodes are created
    EXPECT_TRUE(cluster->IsEffectNodeValid());
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_002
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_002
 * @tc.desc: Test HpaeSourceProcessCluster GetSharedInstance with same node info
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = MONO;
    nodeInfo.format = SAMPLE_S16LE;
    
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    // Request shared instance with same node info
    std::shared_ptr<HpaeNode> sharedInstance = cluster->GetSharedInstance(nodeInfo);
    EXPECT_NE(sharedInstance, nullptr);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_003
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_003
 * @tc.desc: Test HpaeSourceProcessCluster GetSharedInstance with different node info
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_003, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = MONO;
    nodeInfo.format = SAMPLE_S16LE;
    
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    // Request shared instance with different node info
    HpaeNodeInfo differentNodeInfo = nodeInfo;
    differentNodeInfo.samplingRate = SAMPLE_RATE_16000;
    
    std::shared_ptr<HpaeNode> sharedInstance = cluster->GetSharedInstance(differentNodeInfo);
    EXPECT_NE(sharedInstance, nullptr);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_004
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_004
 * @tc.desc: Test HpaeSourceProcessCluster GetOutputPort with same node info
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_004, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    // Get output port with same node info
    OutputPort<HpaePcmBuffer*>* outputPort = cluster->GetOutputPort(nodeInfo, false);
    EXPECT_NE(outputPort, nullptr);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_005
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_005
 * @tc.desc: Test HpaeSourceProcessCluster GetOutputPort with different node info
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_005, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    // First create converter node by getting shared instance
    HpaeNodeInfo differentNodeInfo = nodeInfo;
    differentNodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeNode> sharedInstance = cluster->GetSharedInstance(differentNodeInfo);
    
    // Then get output port
    OutputPort<HpaePcmBuffer*>* outputPort = cluster->GetOutputPort(differentNodeInfo, false);
    EXPECT_NE(outputPort, nullptr);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_006
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_006
 * @tc.desc: Test HpaeSourceProcessCluster Reset functionality
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_006, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    // Create some converter nodes
    HpaeNodeInfo differentNodeInfo1 = nodeInfo;
    differentNodeInfo1.samplingRate = SAMPLE_RATE_16000;
    cluster->GetSharedInstance(differentNodeInfo1);
    
    HpaeNodeInfo differentNodeInfo2 = nodeInfo;
    differentNodeInfo2.channels = MONO;
    cluster->GetSharedInstance(differentNodeInfo2);
    
    // Perform reset
    bool result = cluster->Reset();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_007
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_007
 * @tc.desc: Test HpaeSourceProcessCluster ResetAll functionality
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_007, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    bool result = cluster->ResetAll();
    EXPECT_TRUE(result);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_008
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_008
 * @tc.desc: Test HpaeSourceProcessCluster GetOutputPortNum
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_008, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    size_t portNum = cluster->GetOutputPortNum();
    EXPECT_GE(portNum, 0U);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_009
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_009
 * @tc.desc: Test HpaeSourceProcessCluster IsEffectNodeValid
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_009, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    bool isValid = cluster->IsEffectNodeValid();
    EXPECT_TRUE(isValid);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_010
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_010
 * @tc.desc: Test HpaeSourceProcessCluster GetCapturerEffectConfig
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_010, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    cluster->captureEffectNode_ = nullptr;
    
    HpaeNodeInfo effectConfig;
    bool result = cluster->GetCapturerEffectConfig(effectConfig);
    
    EXPECT_TRUE(result);
    // Should return valid node info
    EXPECT_EQ(effectConfig.samplingRate, nodeInfo.samplingRate);
    EXPECT_EQ(effectConfig.channels, nodeInfo.channels);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_011
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_011
 * @tc.desc: Test HpaeSourceProcessCluster CaptureEffectCreate with mock success
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_011, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    
    std::shared_ptr<HpaeSourceProcessCluster> cluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<MockHpaeCaptureEffectNode> mockEffectNode =
        std::make_shared<NiceMock<MockHpaeCaptureEffectNode>>(nodeInfo);
    cluster->captureEffectNode_ = mockEffectNode;
    EXPECT_CALL(*mockEffectNode, CaptureEffectCreate(_, _))
        .WillOnce(Return(0));
    EXPECT_CALL(*mockEffectNode, GetCapturerEffectConfig(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(nodeInfo), Return(true)));

    int32_t result = cluster->CaptureEffectCreate(12345, CaptureEffectAttr{});
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_012
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_012
 * @tc.desc: Test HpaeSourceProcessCluster CaptureEffectCreate with mock failure
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_012, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    
    std::shared_ptr<HpaeSourceProcessCluster> cluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<MockHpaeCaptureEffectNode> mockEffectNode =
        std::make_shared<NiceMock<MockHpaeCaptureEffectNode>>(nodeInfo);
    cluster->captureEffectNode_ = mockEffectNode;
    EXPECT_CALL(*mockEffectNode, CaptureEffectCreate(_, _))
        .WillOnce(Return(-1));

    int32_t result = cluster->CaptureEffectCreate(12345, CaptureEffectAttr{});
    
    EXPECT_NE(result, 0);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_013
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_013
 * @tc.desc: Test HpaeSourceProcessCluster GetCapturerEffectConfig with mock
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_013, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    HpaeNodeInfo expectedConfig = GetTestNodeInfo();
    expectedConfig.samplingRate = SAMPLE_RATE_16000;
    expectedConfig.channels = MONO;
    
    std::shared_ptr<HpaeSourceProcessCluster> cluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<MockHpaeCaptureEffectNode> mockEffectNode =
        std::make_shared<NiceMock<MockHpaeCaptureEffectNode>>(nodeInfo);
    cluster->captureEffectNode_ = mockEffectNode;
    EXPECT_CALL(*mockEffectNode, GetCapturerEffectConfig(_, _))
        .WillOnce(DoAll(SetArgReferee<0>(expectedConfig), Return(true)));

    HpaeNodeInfo actualConfig;
    bool result = cluster->GetCapturerEffectConfig(actualConfig);

    EXPECT_TRUE(result);
    EXPECT_EQ(actualConfig.samplingRate, expectedConfig.samplingRate);
    EXPECT_EQ(actualConfig.channels, expectedConfig.channels);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_014
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_014
 * @tc.desc: Test HpaeSourceProcessCluster with different scene types
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_014, TestSize.Level0)
{
    HpaeProcessorType sceneTypes[] = {
        HPAE_SCENE_RECORD,
        HPAE_SCENE_VOIP_UP,
        HPAE_SCENE_PRE_ENHANCE,
        HPAE_SCENE_VOICE_MESSAGE,
        HPAE_SCENE_RECOGNITION
    };
    
    for (auto sceneType : sceneTypes) {
        HpaeNodeInfo nodeInfo = GetTestNodeInfo();
        nodeInfo.sceneType = sceneType;
        
        std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
        EXPECT_NE(cluster, nullptr);
        
        // Test basic functionality with different scene types
        bool isValid = cluster->IsEffectNodeValid();
        size_t portNum = cluster->GetOutputPortNum();
        
        EXPECT_TRUE(isValid);
        EXPECT_GE(portNum, 0U);
    }
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_015
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_015
 * @tc.desc: Test HpaeSourceProcessCluster multiple format converter management
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_015, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);
    
    // Create multiple converter nodes with different node info
    HpaeNodeInfo config1 = nodeInfo;
    config1.samplingRate = SAMPLE_RATE_16000;
    
    HpaeNodeInfo config2 = nodeInfo;
    config2.channels = MONO;
    
    HpaeNodeInfo config3 = nodeInfo;
    config3.format = SAMPLE_F32LE;
    
    std::shared_ptr<HpaeNode> instance1 = cluster->GetSharedInstance(config1);
    std::shared_ptr<HpaeNode> instance2 = cluster->GetSharedInstance(config2);
    std::shared_ptr<HpaeNode> instance3 = cluster->GetSharedInstance(config3);
    
    EXPECT_NE(instance1, nullptr);
    EXPECT_NE(instance2, nullptr);
    EXPECT_NE(instance3, nullptr);
    
    // Reset should clean up all connections
    bool resetResult = cluster->Reset();
    EXPECT_TRUE(resetResult);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_016
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_016
 * @tc.desc: Test HpaeSourceProcessCluster SetAppsEnhanceMuteState with null effect node
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_016, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    std::unique_ptr<HpaeSourceProcessCluster> cluster = std::make_unique<HpaeSourceProcessCluster>(nodeInfo);

    // Set captureEffectNode_ to nullptr to test error handling
    cluster->captureEffectNode_ = nullptr;

    // Should return ERROR_ILLEGAL_STATE when captureEffectNode_ is nullptr
    int32_t result = cluster->SetAppsEnhanceMuteState(true);
    EXPECT_EQ(result, ERROR_ILLEGAL_STATE);

    result = cluster->SetAppsEnhanceMuteState(false);
    EXPECT_EQ(result, ERROR_ILLEGAL_STATE);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_017
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_017
 * @tc.desc: Test HpaeSourceProcessCluster SetAppsEnhanceMuteState with mock effect node
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_017, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();

    std::shared_ptr<HpaeSourceProcessCluster> cluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<MockHpaeCaptureEffectNode> mockEffectNode =
        std::make_shared<NiceMock<MockHpaeCaptureEffectNode>>(nodeInfo);
    cluster->captureEffectNode_ = mockEffectNode;

    // Mock SetAppsEnhanceMuteState to return SUCCESS
    EXPECT_CALL(*mockEffectNode, SetAppsEnhanceMuteState(true))
        .WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockEffectNode, SetAppsEnhanceMuteState(false))
        .WillOnce(Return(SUCCESS));

    int32_t result = cluster->SetAppsEnhanceMuteState(true);
    EXPECT_EQ(result, SUCCESS);

    result = cluster->SetAppsEnhanceMuteState(false);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name: TestHpaeSourceProcessCluster_018
 * @tc.type: FUNC
 * @tc.number: TestHpaeSourceProcessCluster_018
 * @tc.desc: Test HpaeSourceProcessCluster SetAppsEnhanceMuteState with different mute states
 */
HWTEST_F(HpaeSourceProcessClusterTest, TestHpaeSourceProcessCluster_018, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();

    std::shared_ptr<HpaeSourceProcessCluster> cluster =
        std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    std::shared_ptr<MockHpaeCaptureEffectNode> mockEffectNode =
        std::make_shared<NiceMock<MockHpaeCaptureEffectNode>>(nodeInfo);
    cluster->captureEffectNode_ = mockEffectNode;

    // Test multiple calls with different states
    EXPECT_CALL(*mockEffectNode, SetAppsEnhanceMuteState(_))
        .Times(4)
        .WillRepeatedly(Return(SUCCESS));

    EXPECT_EQ(cluster->SetAppsEnhanceMuteState(true), SUCCESS);
    EXPECT_EQ(cluster->SetAppsEnhanceMuteState(false), SUCCESS);
    EXPECT_EQ(cluster->SetAppsEnhanceMuteState(true), SUCCESS);
    EXPECT_EQ(cluster->SetAppsEnhanceMuteState(false), SUCCESS);
}

// FaultCode test constants
static constexpr uint64_t TEST_SCENE_KEY_CODE = 12345;

/**
 * @tc.name  : Test CaptureEffectCreate fault code
 * @tc.type  : FUNC
 * @tc.number: FaultCode_CaptureEffectCreate_001
 * @tc.desc  : Test CaptureEffectCreate returns ERROR_ILLEGAL_STATE when captureEffectNode_ is null
 */
HWTEST_F(HpaeSourceProcessClusterTest, FaultCode_CaptureEffectCreate_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    cluster->captureEffectNode_ = nullptr;
    CaptureEffectAttr attr;
    EXPECT_EQ(cluster->CaptureEffectCreate(TEST_SCENE_KEY_CODE, attr), ERROR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test CaptureEffectRelease fault code
 * @tc.type  : FUNC
 * @tc.number: FaultCode_CaptureEffectRelease_001
 * @tc.desc  : Test CaptureEffectRelease returns ERROR_ILLEGAL_STATE when captureEffectNode_ is null
 */
HWTEST_F(HpaeSourceProcessClusterTest, FaultCode_CaptureEffectRelease_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    cluster->captureEffectNode_ = nullptr;
    EXPECT_EQ(cluster->CaptureEffectRelease(TEST_SCENE_KEY_CODE), ERROR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test GetPreOutNum fault code
 * @tc.type  : FUNC
 * @tc.number: FaultCode_GetPreOutNum_001
 * @tc.desc  : Test GetPreOutNum returns 0 when captureEffectNode_ is null
 */
HWTEST_F(HpaeSourceProcessClusterTest, FaultCode_GetPreOutNum_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    cluster->captureEffectNode_ = nullptr;
    EXPECT_EQ(cluster->GetPreOutNum(), 0);
}

/**
 * @tc.name  : Test GetOutputPort fallback when fmtConverterNodeMap_ key not found
 * @tc.type  : FUNC
 * @tc.number: FaultCode_GetOutputPort_001
 * @tc.desc  : Test GetOutputPort returns mixerNode_ port when fmtConverterNodeMap_ key not found
 */
HWTEST_F(HpaeSourceProcessClusterTest, FaultCode_GetOutputPort_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo = GetTestNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    auto cluster = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
    HpaeNodeInfo differentNodeInfo = nodeInfo;
    differentNodeInfo.samplingRate = SAMPLE_RATE_16000;
    OutputPort<HpaePcmBuffer *> *port = cluster->GetOutputPort(differentNodeInfo, false);
    EXPECT_NE(port, nullptr);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
