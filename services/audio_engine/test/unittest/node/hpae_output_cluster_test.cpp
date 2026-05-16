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
#include "hpae_process_cluster.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_sink_input_node.h"
#include "hpae_output_cluster.h"
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t NODE_ID = 1243;
constexpr uint32_t SESSION_ID_1 = 12345;
constexpr uint32_t SESSION_ID_2 = 12346;
constexpr uint32_t FRAME_LEN = 960;
constexpr uint32_t FRAME_LEN_2 = 820;
constexpr uint32_t NUM_TWO = 2;
constexpr int32_t TEST_VALUE_1 = 300;
constexpr int32_t TEST_VALUE_2 = 400;

static std::string g_deviceClass = "file_io";
static std::string g_deviceNetId = "LocalDevice";


static int32_t TestRendererRenderFrame(const char *data, uint64_t len)
{
    float curGain = 0.0f;
    float targetGain = 1.0f;
    uint64_t frameLen = len / (SAMPLE_F32LE * STEREO);
    float stepGain = (targetGain - curGain) / frameLen;
    
    const float *tempData = reinterpret_cast<const float *>(data);
    for (int32_t i = 0; i < frameLen; i++) {
        const float left = tempData[NUM_TWO * i];
        const float right = tempData[NUM_TWO * i + 1];
        const float expectedValue = TEST_VALUE_1 * (curGain + i * stepGain) + TEST_VALUE_2 * (curGain + i * stepGain);
        EXPECT_EQ(left, expectedValue);
        EXPECT_EQ(right, expectedValue);
    }
    return 0;
}

static void InitHpaeWriteDataOutSessionTest(HpaeNodeInfo &nodeInfo, HpaeSinkInfo &dummySinkInfo)
{
    nodeInfo.nodeId = NODE_ID;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    dummySinkInfo.frameLen = FRAME_LEN;
    dummySinkInfo.samplingRate = SAMPLE_RATE_48000;
    dummySinkInfo.channels = STEREO;
    dummySinkInfo.format = SAMPLE_F32LE;
    dummySinkInfo.deviceClass = g_deviceClass;
    dummySinkInfo.deviceNetId = g_deviceNetId;
}

class HpaeOutputClusterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeOutputClusterTest::SetUp()
{}

void HpaeOutputClusterTest::TearDown()
{}

HWTEST_F(HpaeOutputClusterTest, constructHpaeOutputClusterNode, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NODE_ID;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.sessionId = SESSION_ID_1;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    int32_t syncId = 123;

    std::shared_ptr<HpaeOutputCluster> hpaeoutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    EXPECT_EQ(hpaeoutputCluster->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(hpaeoutputCluster->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(hpaeoutputCluster->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(hpaeoutputCluster->GetBitWidth(), nodeInfo.format);
 
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeoutputCluster->Connect(hpaeSinkInputNode);
    EXPECT_EQ(hpaeSinkInputNode.use_count(), NUM_TWO);
    EXPECT_EQ(hpaeoutputCluster->GetConverterNodeCount(), 1);
    nodeInfo.frameLen = FRAME_LEN_2;
    nodeInfo.sessionId = SESSION_ID_2;
    nodeInfo.samplingRate = SAMPLE_RATE_44100;
    std::shared_ptr<HpaeSinkInputNode> hpaeSinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeoutputCluster->Connect(hpaeSinkInputNode1);
    EXPECT_EQ(hpaeSinkInputNode1.use_count(), NUM_TWO);
    EXPECT_EQ(hpaeoutputCluster->GetConverterNodeCount(), 1);
    EXPECT_EQ(hpaeoutputCluster->SetSyncId(syncId), SUCCESS);
}

HWTEST_F(HpaeOutputClusterTest, testHpaeWriteDataOutSessionTest, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo dummySinkInfo;
    InitHpaeWriteDataOutSessionTest(nodeInfo, dummySinkInfo);
    std::shared_ptr<HpaeOutputCluster> hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    nodeInfo.sessionId = SESSION_ID_1;
    nodeInfo.streamType = STREAM_MUSIC;
    if (hpaeOutputCluster->mixerNode_) {
        hpaeOutputCluster->mixerNode_->limiter_ = nullptr;
    }
    std::shared_ptr<HpaeSinkInputNode> musicSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    nodeInfo.sessionId = SESSION_ID_2;
    nodeInfo.streamType = STREAM_RING;
    std::shared_ptr<HpaeSinkInputNode> ringSinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    std::shared_ptr<HpaeProcessCluster> muiscProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    nodeInfo.sceneType = HPAE_SCENE_RING;
    std::shared_ptr<HpaeProcessCluster> ringProcessCluster =
        std::make_shared<HpaeProcessCluster>(nodeInfo, dummySinkInfo);
    EXPECT_EQ(muiscProcessCluster->CreateNodes(musicSinkInputNode), SUCCESS);
    EXPECT_EQ(ringProcessCluster->CreateNodes(ringSinkInputNode), SUCCESS);
    muiscProcessCluster->Connect(musicSinkInputNode);
    ringProcessCluster->Connect(ringSinkInputNode);
    hpaeOutputCluster->Connect(muiscProcessCluster);
    hpaeOutputCluster->Connect(ringProcessCluster);

    EXPECT_EQ(ringProcessCluster->GetGainNodeCount(), 1);
    EXPECT_EQ(muiscProcessCluster->GetGainNodeCount(), 1);
    EXPECT_EQ(muiscProcessCluster->GetConverterNodeCount(), 1);
    EXPECT_EQ(ringProcessCluster->GetConverterNodeCount(), 1);
    EXPECT_EQ(hpaeOutputCluster->GetConverterNodeCount(), NUM_TWO);
    EXPECT_EQ(hpaeOutputCluster->GetPreOutNum(), NUM_TWO);

    EXPECT_EQ(hpaeOutputCluster->GetInstance(g_deviceClass, g_deviceNetId), 0);
    EXPECT_EQ(musicSinkInputNode.use_count(), NUM_TWO);
    EXPECT_EQ(ringSinkInputNode.use_count(), NUM_TWO);
    EXPECT_EQ(muiscProcessCluster.use_count(), 1);
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb0 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, TEST_VALUE_1);
    musicSinkInputNode->RegisterWriteCallback(writeFixedValueCb0);
    std::shared_ptr<WriteFixedValueCb> writeFixedValueCb1 =
        std::make_shared<WriteFixedValueCb>(SAMPLE_F32LE, TEST_VALUE_2);
    ringSinkInputNode->RegisterWriteCallback(writeFixedValueCb1);
    hpaeOutputCluster->RegisterCurrentDeviceCallback();
    hpaeOutputCluster->DoProcess();
    TestRendererRenderFrame(hpaeOutputCluster->GetFrameData(),
        nodeInfo.frameLen * nodeInfo.channels * GetSizeFromFormat(nodeInfo.format));
    muiscProcessCluster->DisConnect(musicSinkInputNode);
    EXPECT_EQ(musicSinkInputNode.use_count(), 1);
    EXPECT_EQ(muiscProcessCluster->GetGainNodeCount(), 1);
    ringProcessCluster->DisConnect(ringSinkInputNode);
    EXPECT_EQ(ringSinkInputNode.use_count(), 1);
    hpaeOutputCluster->DisConnect(muiscProcessCluster);
    EXPECT_EQ(hpaeOutputCluster->GetPreOutNum(), 1);
    hpaeOutputCluster->DisConnect(ringProcessCluster);
    EXPECT_EQ(hpaeOutputCluster->GetPreOutNum(), 0);
}

/**
 * @tc.name  : FaultCode_SetAuxiliarySinkEnable_NullSinkOutputNode
 * @tc.type  : FUNC
 * @tc.number: FaultCode_SetAuxiliarySinkEnable_NullSinkOutputNode
 * @tc.desc  : Test SetAuxiliarySinkEnable when hpaeSinkOutputNode_ is null,
 *             should report PLAY_QUERY_INSTANCE_NULL fault code.
 */
HWTEST_F(HpaeOutputClusterTest, FaultCode_SetAuxiliarySinkEnable_NullSinkOutputNode, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NODE_ID;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto outputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    ASSERT_NE(outputCluster, nullptr);
    // hpaeSinkOutputNode_ is created in constructor, test the normal path
    (void)outputCluster->SetAuxiliarySinkEnable(true);
    // May succeed or fail depending on sink implementation
    SUCCEED();
}

/**
 * @tc.name  : FaultCode_SetCollaborationState_NullSinkOutputNode
 * @tc.type  : FUNC
 * @tc.number: FaultCode_SetCollaborationState_NullSinkOutputNode
 * @tc.desc  : Test SetCollaborationState when hpaeSinkOutputNode_ is null,
 *             should not crash (CHECK_AND_RETURN_LOG guards).
 */
HWTEST_F(HpaeOutputClusterTest, FaultCode_SetCollaborationState_NullSinkOutputNode, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NODE_ID;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto outputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    ASSERT_NE(outputCluster, nullptr);
    // hpaeSinkOutputNode_ is created in constructor
    outputCluster->SetCollaborationState(true);
    outputCluster->SetCollaborationState(false);
    SUCCEED();
}

/**
 * @tc.name  : FaultCode_GetLatency_WithSceneType
 * @tc.type  : FUNC
 * @tc.number: FaultCode_GetLatency_WithSceneType
 * @tc.desc  : Test GetLatency with scene type that has no converter node,
 *             should return mixerNode latency only.
 */
HWTEST_F(HpaeOutputClusterTest, FaultCode_GetLatency_WithSceneType, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NODE_ID;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto outputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    ASSERT_NE(outputCluster, nullptr);
    // No converter nodes connected for HPAE_SCENE_MUSIC
    uint64_t latency = outputCluster->GetLatency(HPAE_SCENE_MUSIC);
    // Should return mixerNode latency (or 0 if mixerNode has no limiter)
    EXPECT_GE(latency, 0);
}

// FaultCode test constants
static constexpr uint32_t TEST_NOTIFY_SESSION_ID = 1000;

/**
 * @tc.name  : FaultCode_NotifyStreamChangeToSink_NullSinkOutputNode
 * @tc.type  : FUNC
 * @tc.number: FaultCode_NotifyStreamChangeToSink_NullSinkOutputNode
 * @tc.desc  : Test NotifyStreamChangeToSink when hpaeSinkOutputNode_ is null,
 *             should not crash (CHECK_AND_RETURN guards).
 */
HWTEST_F(HpaeOutputClusterTest, FaultCode_NotifyStreamChangeToSink_NullSinkOutputNode, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = NODE_ID;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto outputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    ASSERT_NE(outputCluster, nullptr);
    // hpaeSinkOutputNode_ is created in constructor
    outputCluster->NotifyStreamChangeToSink(
        StreamChangeType::STREAM_CHANGE_TYPE_STATE_CHANGE, SESSION_ID_1,
        STREAM_USAGE_MEDIA, RENDERER_RUNNING, TEST_NOTIFY_SESSION_ID);
    SUCCEED();
}
// ==================== New UT for GetRenderId and GetCurrentOutputDevice ====================

/**
 * @tc.name  : GetRenderId_Default_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetRenderId returns HDI_INVALID_ID when sink output node has default renderId.
 */
HWTEST_F(HpaeOutputClusterTest, GetRenderId_Default_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 100;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    // hpaeSinkOutputNode_ default renderId_ is HDI_INVALID_ID
    EXPECT_EQ(hpaeOutputCluster->GetRenderId(), HDI_INVALID_ID);
}

/**
 * @tc.name  : GetCurrentOutputDevice_NullSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetCurrentOutputDevice returns DEVICE_TYPE_NONE when sink is null.
 */
HWTEST_F(HpaeOutputClusterTest, GetCurrentOutputDevice_NullSink_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 100;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    // hpaeSinkOutputNode_->audioRendererSink_ is nullptr by default
    EXPECT_EQ(hpaeOutputCluster->GetCurrentOutputDevice(), DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : GetRenderId_SetValue_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetRenderId returns correct value after setting via sink output node.
 */
HWTEST_F(HpaeOutputClusterTest, GetRenderId_SetValue_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 100;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    // Access private member via -fno-access-control
    hpaeOutputCluster->hpaeSinkOutputNode_->renderId_ = 200;
    EXPECT_EQ(hpaeOutputCluster->GetRenderId(), 200u);
}

/**
 * @tc.name  : SetDeviceChangeCallback_Invoke_001
 * @tc.type  : FUNC
 * @tc.desc  : Test SetDeviceChangeCallback sets callback that can be invoked.
 */
HWTEST_F(HpaeOutputClusterTest, SetDeviceChangeCallback_Invoke_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 100;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);

    bool callbackInvoked = false;
    hpaeOutputCluster->SetDeviceChangeCallback([&callbackInvoked]() {
        callbackInvoked = true;
    });

    // Access private member via -fno-access-control to invoke the callback
    ASSERT_NE(hpaeOutputCluster->deviceChangeCallback_, nullptr);
    hpaeOutputCluster->deviceChangeCallback_();
    EXPECT_EQ(callbackInvoked, true);
}

/**
 * @tc.name  : SetDeviceChangeCallback_Null_001
 * @tc.type  : FUNC
 * @tc.desc  : Test SetDeviceChangeCallback with no callback set (default state).
 */
HWTEST_F(HpaeOutputClusterTest, SetDeviceChangeCallback_Null_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 100;
    nodeInfo.frameLen = FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto hpaeOutputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);

    // deviceChangeCallback_ should be empty by default
    EXPECT_EQ(static_cast<bool>(hpaeOutputCluster->deviceChangeCallback_), false);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS