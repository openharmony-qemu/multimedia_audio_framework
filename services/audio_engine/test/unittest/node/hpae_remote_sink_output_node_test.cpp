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
#include "hpae_remote_sink_output_node.h"
#include "hpae_mixer_node.h"
#include "hpae_mocks.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {


class HpaeRemoteSinkOutputNodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeRemoteSinkOutputNodeTest::SetUp()
{}

void HpaeRemoteSinkOutputNodeTest::TearDown()
{}

HWTEST_F(HpaeRemoteSinkOutputNodeTest, constructNode_01, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo1;
    nodeInfo1.nodeId = 1001; // 1001: node id
    nodeInfo1.frameLen = 960; // 960: frameLen
    nodeInfo1.sessionId = 123456; // 123456: sessionId
    nodeInfo1.samplingRate = SAMPLE_RATE_48000;
    nodeInfo1.channels = STEREO;
    nodeInfo1.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRemoteSinkOutputNode> hpaeRemoteSinkOutputNode =
        std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo1, sinkInfo);

    HpaeNodeInfo nodeInfo2;
    nodeInfo2.nodeId = 1002; // 1002: nodeId
    nodeInfo2.frameLen = 960; // 960: frameLen
    nodeInfo2.samplingRate = SAMPLE_RATE_48000;
    nodeInfo2.channels = STEREO;
    nodeInfo2.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeMixerNode> hpaeMixerNode = std::make_shared<HpaeMixerNode>(nodeInfo2);

    hpaeRemoteSinkOutputNode->Connect(hpaeMixerNode);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->GetRenderSinkInstance("remote", "LocalDevice"), 0);
    hpaeRemoteSinkOutputNode->DoProcess();
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkDeInit(), SUCCESS);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkFlush(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkPause(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkReset(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkResume(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkStart(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->RenderSinkStop(), ERROR);
    EXPECT_EQ(hpaeRemoteSinkOutputNode->GetPreOutNum(), 1);
    
    hpaeRemoteSinkOutputNode->DisConnect(hpaeMixerNode);
}

/**
 * @tc.name  : HandlePcmDumping_001
 * @tc.type  : FUNC
 * @tc.desc  : Test PCM dumping logic with correct Dumper construction.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, HandlePcmDumping_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    size_t dataSize = 1024;
    std::vector<char> mockData(dataSize, 0x5A);
    char* dataPtr = mockData.data();

    remoteNode->outputMediaPcmDumper_ = nullptr;
    remoteNode->HandlePcmDumping(SplitStreamType::STREAM_TYPE_MEDIA, dataPtr, dataSize);

    remoteNode->outputMediaPcmDumper_ = std::make_unique<HpaePcmDumper>("ut_media_dump.pcm");
    remoteNode->HandlePcmDumping(SplitStreamType::STREAM_TYPE_MEDIA, dataPtr, dataSize);

    remoteNode->outputNavigationPcmDumper_ = std::make_unique<HpaePcmDumper>("ut_nav_dump.pcm");
    remoteNode->HandlePcmDumping(SplitStreamType::STREAM_TYPE_NAVIGATION, dataPtr, dataSize);

    remoteNode->outputCommunicationPcmDumper_ = std::make_unique<HpaePcmDumper>("ut_comm_dump.pcm");
    remoteNode->HandlePcmDumping(SplitStreamType::STREAM_TYPE_COMMUNICATION, dataPtr, dataSize);
}

/**
 * @tc.name  : ResetAll_001
 * @tc.type  : FUNC
 * @tc.number: ResetAll_001
 * @tc.desc  : Test ResetAll to cover node traversal and conditional disconnection.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, ResetAll_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    HpaeNodeInfo infoA;
    infoA.nodeId = 3001;
    auto nodeA = std::make_shared<HpaeMixerNode>(infoA);
    remoteNode->Connect(nodeA);

    HpaeNodeInfo infoB;
    infoB.nodeId = 3002;
    auto nodeB = std::make_shared<HpaeMixerNode>(infoB);
    remoteNode->Connect(nodeB);

    EXPECT_EQ(remoteNode->GetPreOutNum(), 2);

    bool result = remoteNode->ResetAll();

    EXPECT_TRUE(result);

    EXPECT_EQ(remoteNode->GetPreOutNum(), 0);
}

// FaultCode test constants
static constexpr int32_t TEST_APP_UID = 1001;

/**
 * @tc.name  : FaultCode_UpdateAppsUid_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAppsUid returns ERROR when audioRendererSink_ is nullptr,
 *             covering PLAY_SEND_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, FaultCode_UpdateAppsUid_NullSink, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // audioRendererSink_ is nullptr by default
    std::vector<int32_t> appsUid = {TEST_APP_UID};
    int32_t result = remoteNode->UpdateAppsUid(appsUid);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : FaultCode_RenderSinkInit_NullSink
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkInit returns ERROR when audioRendererSink_ is nullptr,
 *             covering PLAY_CREATE_STREAM_FAIL fault code path.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, FaultCode_RenderSinkInit_NullSink, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // audioRendererSink_ is nullptr by default
    IAudioSinkAttr attr;
    attr.adapterName = "remote";
    int32_t result = remoteNode->RenderSinkInit(attr);
    EXPECT_EQ(result, ERROR);
}

/**
 * @tc.name  : RenderSinkInit_InitFailedReleasesSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkInit releases audioRendererSink_ and renderId_ when Init fails.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, RenderSinkInit_InitFailedReleasesSink_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    auto mockSink = std::make_shared<MockAudioRenderSink>();
    remoteNode->audioRendererSink_ = mockSink;
    remoteNode->renderId_ = 1;

    EXPECT_CALL(*mockSink, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink, Init(_)).WillOnce(Return(ERROR));

    IAudioSinkAttr attr;
    attr.adapterName = "remote";
    int32_t result = remoteNode->RenderSinkInit(attr);
    EXPECT_EQ(result, ERROR);
    EXPECT_EQ(remoteNode->audioRendererSink_, nullptr);
    EXPECT_EQ(remoteNode->renderId_, HDI_INVALID_ID);
    EXPECT_NE(remoteNode->RenderSinkDeInit(), SUCCESS);
}

// ==================== New UT for GetRenderId and GetCurrentActiveDevice ====================

/**
 * @tc.name  : GetRenderId_Default_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetRenderId returns HDI_INVALID_ID by default.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, GetRenderId_Default_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);
    EXPECT_EQ(remoteNode->GetRenderId(), HDI_INVALID_ID);
}

/**
 * @tc.name  : GetRenderId_SetValue_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetRenderId returns correct value after setting private member.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, GetRenderId_SetValue_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);
    remoteNode->renderId_ = 500;
    EXPECT_EQ(remoteNode->GetRenderId(), 500u);
}

/**
 * @tc.name  : GetCurrentActiveDevice_NullSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetCurrentActiveDevice returns DEVICE_TYPE_NONE when sink is null.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, GetCurrentActiveDevice_NullSink_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);
    // audioRendererSink_ is nullptr by default
    EXPECT_EQ(remoteNode->GetCurrentOutputDevice(), DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : GetRenderSinkInstance_EmptyDeviceNetId_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetRenderSinkInstance with empty deviceNetId uses HDI_ID_INFO_DEFAULT.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, GetRenderSinkInstance_EmptyDeviceNetId_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // Test with empty deviceNetId (should use HDI_ID_INFO_DEFAULT)
    // In real environment, HdiAdapterManager may successfully get sink
    int32_t result = remoteNode->GetRenderSinkInstance("remote", "");
    // Verify renderId is set (not HDI_INVALID_ID) when successful
    if (result == SUCCESS) {
        EXPECT_NE(remoteNode->GetRenderId(), HDI_INVALID_ID);
    }
}

/**
 * @tc.name  : RenderSinkInit_NullSink_002
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkInit returns ERROR when audioRendererSink_ is nullptr.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, RenderSinkInit_NullSink_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // Use SetSinkState to simulate initialization
    remoteNode->SetSinkState(STREAM_MANAGER_RUNNING);

    IAudioSinkAttr attr;
    attr.adapterName = "remote";

    // Since audioRendererSink_ is nullptr, returns ERROR
    EXPECT_EQ(remoteNode->RenderSinkInit(attr), ERROR);
}

/**
 * @tc.name  : RenderSinkStart_Failure_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkStart returns ERROR when sink Start() fails.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, RenderSinkStart_Failure_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // audioRendererSink_ is nullptr by default
    EXPECT_EQ(remoteNode->RenderSinkStart(), ERROR);
}

/**
 * @tc.name  : RenderSinkResume_Failure_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkResume returns error code when sink Resume() fails.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, RenderSinkResume_Failure_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // audioRendererSink_ is nullptr by default
    EXPECT_EQ(remoteNode->RenderSinkResume(), ERROR);
}

/**
 * @tc.name  : RenderSinkStop_Failure_001
 * @tc.type  : FUNC
 * @tc.desc  : Test RenderSinkStop returns error code when sink Stop() fails.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, RenderSinkStop_Failure_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // audioRendererSink_ is nullptr by default
    EXPECT_EQ(remoteNode->RenderSinkStop(), ERROR);
}

/**
 * @tc.name  : SetGetSinkState_001
 * @tc.type  : FUNC
 * @tc.desc  : Test SetSinkState and GetSinkState functionality.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, SetGetSinkState_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // Test setting different states
    EXPECT_EQ(remoteNode->SetSinkState(STREAM_MANAGER_IDLE), SUCCESS);
    EXPECT_EQ(remoteNode->GetSinkState(), STREAM_MANAGER_IDLE);

    EXPECT_EQ(remoteNode->SetSinkState(STREAM_MANAGER_RUNNING), SUCCESS);
    EXPECT_EQ(remoteNode->GetSinkState(), STREAM_MANAGER_RUNNING);

    EXPECT_EQ(remoteNode->SetSinkState(STREAM_MANAGER_SUSPENDED), SUCCESS);
    EXPECT_EQ(remoteNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    EXPECT_EQ(remoteNode->SetSinkState(STREAM_MANAGER_RELEASED), SUCCESS);
    EXPECT_EQ(remoteNode->GetSinkState(), STREAM_MANAGER_RELEASED);
}

/**
 * @tc.name  : UpdateAppsUid_MultipleUids_001
 * @tc.type  : FUNC
 * @tc.desc  : Test UpdateAppsUid with multiple UIDs returns ERROR when sink is nullptr.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, UpdateAppsUid_MultipleUids_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    std::vector<int32_t> appsUid = {TEST_APP_UID, TEST_APP_UID + 1, TEST_APP_UID + 2};
    // audioRendererSink_ is nullptr, returns ERROR
    EXPECT_EQ(remoteNode->UpdateAppsUid(appsUid), ERROR);
}

/**
 * @tc.name  : HandlePcmDumping_DefaultCase_001
 * @tc.type  : FUNC
 * @tc.desc  : Test HandlePcmDumping default case with unknown stream type.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, HandlePcmDumping_DefaultCase_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    auto remoteNode = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    size_t dataSize = 512;
    std::vector<char> mockData(dataSize, 0xAB);
    char* dataPtr = mockData.data();

    // Test with default case (non-standard stream type)
    remoteNode->outputCommunicationPcmDumper_ = std::make_unique<HpaePcmDumper>("ut_default_dump.pcm");

    // Use an invalid stream type to trigger default case
    remoteNode->HandlePcmDumping(static_cast<SplitStreamType>(999), dataPtr, dataSize);
}

// ==================== Delegate-arm coverage (audioRendererSink_ non-null) ====================
// Every Render* / GetCurrentOutputDevice has a `if (audioRendererSink_ == nullptr) return ERROR;`
// guard. Existing cases cover only the null arm. To drive the delegate arm we obtain a REAL sink
// via GetRenderSinkInstance (the same path constructNode_01 relies on; the test env yields a
// RemoteAudioRenderSink whose vtable is complete). A NiceMock<MockAudioRenderSink> is NOT used:
// hpae_mocks.h's MockAudioRenderSink is stale vs the current IAudioRenderSink pipe API (it omits
// GetOutputPipeInfo/InitPipeInfo/ChangePipeStatus/ChangePipeDevice/ChangePipeStream/DeinitPipeInfo
// and transitively pulls in SinkCallbackWrapper), leaving 6 virtuals un-overridden -> undefined
// vtable at link time. Real-sink method returns are not asserted to fixed values; the goal is to
// execute the delegate arms, so those calls are intentionally soft.

constexpr uint32_t TEST_REMOTE_NODE_ID = 1001;   // 1001: node id
constexpr uint32_t TEST_REMOTE_FRAME_LEN = 960;  // 960: frameLen

/**
 * @tc.name  : RenderSinkDelegates_RealSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Cover all Render* / GetCurrentOutputDevice delegate arms with a real sink obtained
 *             via GetRenderSinkInstance (cpp:288/296/306/314/216 FALSE -> cpp:291/299-301/309/
 *             317-322/219). Hard-asserts the sink is obtained; method returns are soft (real sink).
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, RenderSinkDelegates_RealSink_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_REMOTE_NODE_ID;
    nodeInfo.frameLen = TEST_REMOTE_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // Real sink path (links cleanly; constructNode_01 proves this returns SUCCESS in the test env).
    ASSERT_EQ(node->GetRenderSinkInstance("remote", "LocalDevice"), SUCCESS);
    ASSERT_NE(node->audioRendererSink_, nullptr);

    // Each call now skips its null guard and delegates to the real sink.
    (void)node->RenderSinkFlush();        // cpp:291
    (void)node->RenderSinkPause();        // cpp:299-301 (Pause + SetSinkState SUSPENDED + SUCCESS)
    (void)node->RenderSinkReset();        // cpp:309
    (void)node->RenderSinkResume();       // cpp:317-322 (one arm of cpp:318 on the real Resume() ret)
    (void)node->GetCurrentOutputDevice(); // cpp:219

    node->RenderSinkDeInit(); // cleanup: nulls audioRendererSink_ + ReleaseId
    SUCCEED();
}

/**
 * @tc.name  : GetRenderSinkInstance_NullSinkPath_001
 * @tc.type  : FUNC
 * @tc.desc  : Cover GetRenderSinkInstance null path (cpp:203-210): a bogus device class yields no
 *             adapter -> GetRenderSink returns null -> ReleaseId + ERROR. Non-empty deviceNetId
 *             also exercises the cpp:200 ternary's non-empty arm.
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, GetRenderSinkInstance_NullSinkPath_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = TEST_REMOTE_NODE_ID;
    nodeInfo.frameLen = TEST_REMOTE_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);

    // Bogus device class -> no adapter -> null sink -> cpp:203-210 (ReleaseId + ERROR).
    int32_t result = node->GetRenderSinkInstance("nonexistent_remote_class", "fakeNetworkId123");
    EXPECT_EQ(result, ERROR);
    EXPECT_EQ(node->audioRendererSink_, nullptr); // confirms the null path ran
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
/**
 * @tc.name  : doProcess sink-null hits edge gate first and sustained
 * @tc.type  : FUNC
 * @tc.number: HpaeRemoteSinkOutputNodeTest_EdgeLog_001
 */
HWTEST_F(HpaeRemoteSinkOutputNodeTest, doProcessSinkNullEdgeLog, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = 1001; // 1001: node id
    nodeInfo.frameLen = 960; // 960: frameLen
    nodeInfo.sessionId = 123456; // 123456: sessionId
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    std::shared_ptr<HpaeRemoteSinkOutputNode> hpaeRemoteSinkOutputNode =
        std::make_shared<HpaeRemoteSinkOutputNode>(nodeInfo, sinkInfo);
    hpaeRemoteSinkOutputNode->DoProcess(); // first gate hit: sink not created yet
    hpaeRemoteSinkOutputNode->DoProcess(); // sustained hit
}
