/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <gmock/gmock.h>
#include <memory>
#include "hpae_direct_sinkoutput_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_gain_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_mocks.h"
#include "test_case_common.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace testing;
using ::testing::_;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

namespace {
constexpr uint32_t DEFAULT_NODE_ID = 1243;
constexpr uint32_t DEFAULT_FRAME_LEN = 960;
constexpr uint32_t DEFAULT_SESSION_ID = 100;
static constexpr int32_t AUDIO_US_PER_MS = 1000;
static constexpr int32_t AUDIO_FRAME_WORK_LATENCY_US = 40000;
static constexpr int32_t AUDIO_DEFAULT_LATENCY_US = 160000;
}

class HpaeDirectSinkOutputNodeTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<HpaeDirectSinkOutputNode> directNode_;
    std::shared_ptr<NiceMock<MockAudioRenderSink>> mockSink_;
};

static void PrepareNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
}

void HpaeDirectSinkOutputNodeTest::SetUp()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    directNode_ = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);
    mockSink_ = std::make_shared<NiceMock<MockAudioRenderSink>>();
    directNode_->audioRendererSink_ = mockSink_;
}

void HpaeDirectSinkOutputNodeTest::TearDown()
{
    directNode_ = nullptr;
    mockSink_ = nullptr;
}

/**
 * @tc.name  : constructNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_001
 * @tc.desc  : Test construct HpaeDirectSinkOutputNode and verify node info
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, constructNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);

    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(node->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(node->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(node->GetBitWidth(), nodeInfo.format);
}

/**
 * @tc.name  : constructNode_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_002
 * @tc.desc  : Test construct HpaeDirectSinkOutputNode with different formats
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, constructNode_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;

    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetBitWidth(), SAMPLE_S16LE);
}

/**
 * @tc.name  : destructNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_003
 * @tc.desc  : Test destructor does not crash
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, destructNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    {
        auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);
        EXPECT_NE(node, nullptr);
    }
    // Node should be destructed without crash
}

/**
 * @tc.name  : doProcess_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_004
 * @tc.desc  : Test DoProcess when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, doProcess_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    size_t prevSize = directNode_->currentSize_;
    directNode_->DoProcess();
    // currentSize_ should be unchanged since DoProcess returns early
    EXPECT_EQ(directNode_->currentSize_, prevSize);
}

/**
 * @tc.name  : renderSinkInit_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_005
 * @tc.desc  : Test RenderSinkInit when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    IAudioSinkAttr attr;
    int32_t ret = directNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkInit_AlreadyInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_006
 * @tc.desc  : Test RenderSinkInit when sink is already inited
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_AlreadyInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));

    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    int32_t ret = directNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : renderSinkInit_InitSuccess_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_007
 * @tc.desc  : Test RenderSinkInit with successful init
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_InitSuccess_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink_, Init(_)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));

    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    int32_t ret = directNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : renderSinkInit_InitFailed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_008
 * @tc.desc  : Test RenderSinkInit with init failure
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_InitFailed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink_, Init(_)).WillOnce(Return(ERROR));

    IAudioSinkAttr attr;
    int32_t ret = directNode_->RenderSinkInit(attr);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : renderSinkDeInit_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_009
 * @tc.desc  : Test RenderSinkDeInit when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkDeInit_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    int32_t ret = directNode_->RenderSinkDeInit();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkDeInit_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_010
 * @tc.desc  : Test RenderSinkDeInit successfully deinitializes
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkDeInit_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, DeInit()).Times(1);

    int32_t ret = directNode_->RenderSinkDeInit();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(directNode_->audioRendererSink_, nullptr);
}

/**
 * @tc.name  : renderSinkFlush_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_011
 * @tc.desc  : Test RenderSinkFlush when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkFlush_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    int32_t ret = directNode_->RenderSinkFlush();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkFlush_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_012
 * @tc.desc  : Test RenderSinkFlush successfully flushes
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkFlush_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Flush()).WillOnce(Return(SUCCESS));
    int32_t ret = directNode_->RenderSinkFlush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : renderSinkStart_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_013
 * @tc.desc  : Test RenderSinkStart when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStart_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    int32_t ret = directNode_->RenderSinkStart();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkStart_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_014
 * @tc.desc  : Test RenderSinkStart with successful start
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStart_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(SUCCESS));
    int32_t ret = directNode_->RenderSinkStart();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : renderSinkStart_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_015
 * @tc.desc  : Test RenderSinkStart with start failure
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStart_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(ERROR));
    int32_t ret = directNode_->RenderSinkStart();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : renderSinkStop_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_016
 * @tc.desc  : Test RenderSinkStop when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStop_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    int32_t ret = directNode_->RenderSinkStop();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkStop_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_017
 * @tc.desc  : Test RenderSinkStop with successful stop
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStop_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));
    int32_t ret = directNode_->RenderSinkStop();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : renderSinkStop_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_018
 * @tc.desc  : Test RenderSinkStop with stop failure
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStop_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(ERROR));
    int32_t ret = directNode_->RenderSinkStop();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : getSetSinkState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_019
 * @tc.desc  : Test SetSinkState and GetSinkState
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getSetSinkState_001, TestSize.Level0)
{
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_NEW);

    int32_t ret = directNode_->SetSinkState(STREAM_MANAGER_IDLE);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_IDLE);

    ret = directNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_RUNNING);

    ret = directNode_->SetSinkState(STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    ret = directNode_->SetSinkState(STREAM_MANAGER_RELEASED);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_RELEASED);
}

/**
 * @tc.name  : getRenderFrameData_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_020
 * @tc.desc  : Test GetRenderFrameData returns non-null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getRenderFrameData_001, TestSize.Level0)
{
    const char *data = directNode_->GetRenderFrameData();
    EXPECT_NE(data, nullptr);
}

/**
 * @tc.name  : getLatency_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_021
 * @tc.desc  : Test GetLatency when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    uint64_t latency = directNode_->GetLatency();
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : getLatency_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_022
 * @tc.desc  : Test GetLatency returns correct value from sink
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_Success_001, TestSize.Level0)
{
    uint32_t hdiLatency = 10;
    EXPECT_CALL(*mockSink_, GetLatency(_))
        .WillOnce(DoAll(SetArgReferee<0>(hdiLatency), Return(SUCCESS)));

    uint64_t latency = directNode_->GetLatency();
    uint64_t expectedLatency = hdiLatency * AUDIO_US_PER_MS + AUDIO_FRAME_WORK_LATENCY_US;
    EXPECT_EQ(latency, expectedLatency);
}

/**
 * @tc.name  : getLatency_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_023
 * @tc.desc  : Test GetLatency uses default when sink GetLatency fails
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetLatency(_)).WillOnce(Return(ERROR));

    uint64_t latency = directNode_->GetLatency();
    EXPECT_EQ(latency, AUDIO_DEFAULT_LATENCY_US);
}

/**
 * @tc.name  : getLatency_Cached_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_024
 * @tc.desc  : Test GetLatency returns cached value on subsequent calls
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_Cached_001, TestSize.Level0)
{
    uint32_t hdiLatency = 10;
    EXPECT_CALL(*mockSink_, GetLatency(_))
        .WillOnce(DoAll(SetArgReferee<0>(hdiLatency), Return(SUCCESS)));

    uint64_t latency1 = directNode_->GetLatency();
    uint64_t expectedLatency = hdiLatency * AUDIO_US_PER_MS + AUDIO_FRAME_WORK_LATENCY_US;
    EXPECT_EQ(latency1, expectedLatency);

    // Second call should return cached value without calling sink again
    uint64_t latency2 = directNode_->GetLatency();
    EXPECT_EQ(latency2, expectedLatency);
}

/**
 * @tc.name  : updateAppsUid_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_025
 * @tc.desc  : Test UpdateAppsUid when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, updateAppsUid_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    std::vector<int32_t> appsUid = {1000, 2000};
    int32_t ret = directNode_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : updateAppsUid_NotInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_026
 * @tc.desc  : Test UpdateAppsUid when sink is not inited
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, updateAppsUid_NotInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    std::vector<int32_t> appsUid = {1000, 2000};
    int32_t ret = directNode_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : updateAppsUid_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_027
 * @tc.desc  : Test UpdateAppsUid with valid sink and apps uid
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, updateAppsUid_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, UpdateAppsUid(_)).WillOnce(Return(SUCCESS));

    std::vector<int32_t> appsUid = {1000, 2000};
    int32_t ret = directNode_->UpdateAppsUid(appsUid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : notifyStreamChange_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_028
 * @tc.desc  : Test NotifyStreamChangeToSink when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, notifyStreamChange_SinkNull_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    StreamManagerState prevState = directNode_->GetSinkState();
    directNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, STREAM_USAGE_MUSIC, RENDERER_RUNNING, 1000);
    // State should remain unchanged
    EXPECT_EQ(directNode_->GetSinkState(), prevState);
}

/**
 * @tc.name  : notifyStreamChange_NotInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_029
 * @tc.desc  : Test NotifyStreamChangeToSink when sink is not inited
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, notifyStreamChange_NotInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    StreamManagerState prevState = directNode_->GetSinkState();
    directNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, STREAM_USAGE_MUSIC, RENDERER_RUNNING, 1000);
    // State should remain unchanged since sink not inited
    EXPECT_EQ(directNode_->GetSinkState(), prevState);
}

/**
 * @tc.name  : notifyStreamChange_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_030
 * @tc.desc  : Test NotifyStreamChangeToSink with valid sink does not crash
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, notifyStreamChange_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    // Verify sink is not null before call
    EXPECT_NE(directNode_->audioRendererSink_, nullptr);
    directNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, STREAM_USAGE_MUSIC, RENDERER_RUNNING, 1000);
    // Verify sink state unchanged after call
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_NEW);
}

/**
 * @tc.name  : reset_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_031
 * @tc.desc  : Test Reset returns true
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, reset_001, TestSize.Level0)
{
    bool ret = directNode_->Reset();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : resetAll_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_032
 * @tc.desc  : Test ResetAll returns true
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, resetAll_001, TestSize.Level0)
{
    bool ret = directNode_->ResetAll();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : differentChannels_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_033
 * @tc.desc  : Test construct with MONO channel
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, differentChannels_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = MONO;
    nodeInfo.format = SAMPLE_F32LE;

    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetChannelCount(), MONO);
}

/**
 * @tc.name  : differentSampleRates_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_034
 * @tc.desc  : Test construct with different sample rate
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, differentSampleRates_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_96000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetSampleRate(), SAMPLE_RATE_96000);
}

/**
 * @tc.name  : sinkStateTransition_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_035
 * @tc.desc  : Test full state transition: NEW -> IDLE -> RUNNING -> SUSPENDED -> RELEASED
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, sinkStateTransition_001, TestSize.Level0)
{
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_NEW);

    directNode_->SetSinkState(STREAM_MANAGER_IDLE);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_IDLE);

    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(SUCCESS));
    directNode_->RenderSinkStart();
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_RUNNING);

    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));
    directNode_->RenderSinkStop();
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    EXPECT_CALL(*mockSink_, DeInit()).Times(1);
    directNode_->RenderSinkDeInit();
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_RELEASED);
}

/**
 * @tc.name  : doProcess_ReadDataFail_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_036
 * @tc.desc  : Test DoProcess when ReadDataAndConvertFormat fails (no input connected)
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, doProcess_ReadDataFail_001, TestSize.Level0)
{
    // No input connected, ReadDataAndConvertFormat will fail and return early
    // RenderFrame should NOT be called
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).Times(0);
    directNode_->DoProcess();
}

// ==================== DoProcess with data flow ====================

/**
 * @tc.name  : doProcess_WithDataFlow_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_037
 * @tc.desc  : Test DoProcess with connected input providing valid data
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, doProcess_WithDataFlow_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, nodeInfo);
    converterNode->SetDownmixNormalization(false);

    // Connect chain: sinkInputNode -> converterNode -> directNode_
    converterNode->Connect(sinkInputNode);
    directNode_->Connect(converterNode);

    // Register write callback to provide data
    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    sinkInputNode->RegisterWriteCallback(writeCb);

    // GetRenderSinkInstance to set file_io sink
    EXPECT_EQ(directNode_->GetRenderSinkInstance("file_io", "LocalDevice"), SUCCESS);

    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    attr.sampleRate = SAMPLE_RATE_48000;
    attr.channel = STEREO;
    attr.format = SAMPLE_F32LE;
    EXPECT_EQ(directNode_->RenderSinkInit(attr), SUCCESS);

    // DoProcess should read data and call RenderFrame
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).WillOnce(Return(SUCCESS));
    directNode_->DoProcess();

    directNode_->DisConnect(converterNode);
    converterNode->DisConnect(sinkInputNode);
}

/**
 * @tc.name  : doProcess_RenderFrameFailed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_038
 * @tc.desc  : Test DoProcess when RenderFrame returns error
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, doProcess_RenderFrameFailed_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, nodeInfo);
    converterNode->SetDownmixNormalization(false);

    converterNode->Connect(sinkInputNode);
    directNode_->Connect(converterNode);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    sinkInputNode->RegisterWriteCallback(writeCb);

    EXPECT_EQ(directNode_->GetRenderSinkInstance("file_io", "LocalDevice"), SUCCESS);
    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    EXPECT_EQ(directNode_->RenderSinkInit(attr), SUCCESS);

    // RenderFrame returns error - should trigger usleep path
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).WillOnce(Return(ERROR));
    directNode_->DoProcess();
    // Should not crash, verify currentSize_ was adjusted
    EXPECT_EQ(directNode_->currentSize_, directNode_->currentSize_);

    directNode_->DisConnect(converterNode);
    converterNode->DisConnect(sinkInputNode);
}

/**
 * @tc.name  : doProcess_PartialWrite_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_039
 * @tc.desc  : Test DoProcess when RenderFrame returns partial write (writeLen != renderSize_)
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, doProcess_PartialWrite_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, nodeInfo);
    converterNode->SetDownmixNormalization(false);

    converterNode->Connect(sinkInputNode);
    directNode_->Connect(converterNode);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    sinkInputNode->RegisterWriteCallback(writeCb);

    EXPECT_EQ(directNode_->GetRenderSinkInstance("file_io", "LocalDevice"), SUCCESS);
    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    EXPECT_EQ(directNode_->RenderSinkInit(attr), SUCCESS);

    // RenderFrame succeeds but writeLen is less than renderSize_
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _))
        .WillOnce(DoAll(SetArgReferee<2>(100), Return(SUCCESS)));
    directNode_->DoProcess();
    // Partial write path should trigger usleep
    EXPECT_EQ(directNode_->currentSize_, directNode_->currentSize_);

    directNode_->DisConnect(converterNode);
    converterNode->DisConnect(sinkInputNode);
}

// ==================== Connect/DisConnect ====================

/**
 * @tc.name  : connectDisConnect_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_040
 * @tc.desc  : Test Connect and DisConnect with real nodes
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, connectDisConnect_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, nodeInfo);
    converterNode->SetDownmixNormalization(false);

    converterNode->Connect(sinkInputNode);

    // Connect should not crash
    directNode_->Connect(converterNode);
    // Verify connected by doing DoProcess (ReadDataAndConvertFormat should at least get outputVec)
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).Times(0);
    directNode_->DoProcess();

    // DisConnect should not crash
    directNode_->DisConnect(converterNode);
    converterNode->DisConnect(sinkInputNode);
}

// ==================== Reset/ResetAll with connected nodes ====================

/**
 * @tc.name  : resetWithConnectedNodes_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_041
 * @tc.desc  : Test Reset with connected upstream nodes
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, resetWithConnectedNodes_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, nodeInfo);
    converterNode->SetDownmixNormalization(false);

    converterNode->Connect(sinkInputNode);
    directNode_->Connect(converterNode);

    // Reset should disconnect all inputs
    bool ret = directNode_->Reset();
    EXPECT_EQ(ret, true);

    // After reset, DoProcess should fail (no input)
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).Times(0);
    directNode_->DoProcess();

    converterNode->DisConnect(sinkInputNode);
}

/**
 * @tc.name  : resetAllWithConnectedNodes_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_042
 * @tc.desc  : Test ResetAll with connected upstream nodes
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, resetAllWithConnectedNodes_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, nodeInfo);
    converterNode->SetDownmixNormalization(false);

    converterNode->Connect(sinkInputNode);
    directNode_->Connect(converterNode);

    // ResetAll should recursively reset and disconnect all
    bool ret = directNode_->ResetAll();
    EXPECT_EQ(ret, true);

    // After ResetAll, DoProcess should fail (no input)
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).Times(0);
    directNode_->DoProcess();
}

// ==================== ReadDataAndConvertFormat branches ====================

/**
 * @tc.name  : readDataInvalidBuffer_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_043
 * @tc.desc  : Test DoProcess when input buffer is not valid (IsValid returns false)
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, readDataInvalidBuffer_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, nodeInfo);
    converterNode->SetDownmixNormalization(false);

    converterNode->Connect(sinkInputNode);
    directNode_->Connect(converterNode);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    sinkInputNode->RegisterWriteCallback(writeCb);

    EXPECT_EQ(directNode_->GetRenderSinkInstance("file_io", "LocalDevice"), SUCCESS);
    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    EXPECT_EQ(directNode_->RenderSinkInit(attr), SUCCESS);

    // First DoProcess reads data successfully
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).WillOnce(Return(SUCCESS));
    directNode_->DoProcess();

    // Set buffer invalid to trigger the !outputData->IsValid() branch
    sinkInputNode->inputAudioBuffer_.SetBufferValid(false);
    // DoProcess should return early (ReadDataAndConvertFormat returns false)
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).Times(0);
    directNode_->DoProcess();

    directNode_->DisConnect(converterNode);
    converterNode->DisConnect(sinkInputNode);
}

// ==================== RenderSinkInit SetVolume failure branch ====================

/**
 * @tc.name  : renderSinkInit_InitSuccess_SetVolumeFailed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_044
 * @tc.desc  : Test RenderSinkInit with successful init but SetVolume failure returns error
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_InitSuccess_SetVolumeFailed_001, TestSize.Level1)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink_, Init(_)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(ERROR));

    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    int32_t ret = directNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : renderSinkInit_AlreadyInited_SetsIdle_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_045
 * @tc.desc  : Test RenderSinkInit already inited path sets state to IDLE
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_AlreadyInited_SetsIdle_001, TestSize.Level1)
{
    directNode_->SetSinkState(STREAM_MANAGER_NEW);
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));

    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    EXPECT_EQ(directNode_->RenderSinkInit(attr), SUCCESS);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_IDLE);
}

// ==================== RenderSinkStop resets currentSize_ ====================

/**
 * @tc.name  : renderSinkStop_ResetsCurrentSize_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_046
 * @tc.desc  : Test RenderSinkStop resets currentSize_ to 0
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStop_ResetsCurrentSize_001, TestSize.Level1)
{
    // Set currentSize_ to non-zero
    directNode_->currentSize_ = 1024;
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));

    int32_t ret = directNode_->RenderSinkStop();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(directNode_->currentSize_, 0);
}

// ==================== Constructor with different sample rates ====================

/**
 * @tc.name  : constructDifferentFormats_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_047
 * @tc.desc  : Test construct with SAMPLE_S32LE format
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, constructDifferentFormats_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S32LE;

    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetBitWidth(), SAMPLE_S32LE);
    // Verify renderSize_ matches expected buffer size
    size_t expectedSize = DEFAULT_FRAME_LEN * STEREO * GetSizeFromFormat(SAMPLE_S32LE);
    EXPECT_EQ(node->renderSize_, expectedSize);
}

/**
 * @tc.name  : constructDifferentFormats_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_048
 * @tc.desc  : Test construct with SAMPLE_S16LE format
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, constructDifferentFormats_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;

    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    size_t expectedSize = DEFAULT_FRAME_LEN * STEREO * GetSizeFromFormat(SAMPLE_S16LE);
    EXPECT_EQ(node->renderSize_, expectedSize);
}

// ==================== RenderSinkStart/Stop state verification ====================

/**
 * @tc.name  : renderSinkStart_SetsRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_049
 * @tc.desc  : Test RenderSinkStart sets state to RUNNING even after SUSPENDED
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStart_SetsRunning_001, TestSize.Level1)
{
    // First set to SUSPENDED
    directNode_->SetSinkState(STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(directNode_->RenderSinkStart(), SUCCESS);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_RUNNING);
}

// ==================== GetLatency with zero HDI latency ====================

/**
 * @tc.name  : getLatency_ZeroHdiLatency_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_050
 * @tc.desc  : Test GetLatency with zero HDI latency returns framework latency only
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_ZeroHdiLatency_001, TestSize.Level0)
{
    uint32_t hdiLatency = 0;
    EXPECT_CALL(*mockSink_, GetLatency(_))
        .WillOnce(DoAll(SetArgReferee<0>(hdiLatency), Return(SUCCESS)));

    uint64_t latency = directNode_->GetLatency();
    EXPECT_EQ(latency, static_cast<uint64_t>(AUDIO_FRAME_WORK_LATENCY_US));
}

// ==================== Multiple DoProcess with data accumulation ====================

/**
 * @tc.name  : doProcess_MultipleCalls_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_051
 * @tc.desc  : Test multiple DoProcess calls with data flow
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, doProcess_MultipleCalls_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, nodeInfo);
    converterNode->SetDownmixNormalization(false);

    converterNode->Connect(sinkInputNode);
    directNode_->Connect(converterNode);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    sinkInputNode->RegisterWriteCallback(writeCb);

    EXPECT_EQ(directNode_->GetRenderSinkInstance("file_io", "LocalDevice"), SUCCESS);
    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    EXPECT_EQ(directNode_->RenderSinkInit(attr), SUCCESS);

    // Multiple DoProcess calls should succeed
    EXPECT_CALL(*mockSink_, RenderFrame(_, _, _)).Times(3).WillRepeatedly(Return(SUCCESS));
    directNode_->DoProcess();
    directNode_->DoProcess();
    directNode_->DoProcess();

    directNode_->DisConnect(converterNode);
    converterNode->DisConnect(sinkInputNode);
}

// ==================== renderSinkInit_AttrStored_001 ====================

/**
 * @tc.name  : renderSinkInit_AttrStored_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_052
 * @tc.desc  : Test RenderSinkInit stores attr in sinkOutAttr_
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_AttrStored_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink_, Init(_)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));

    IAudioSinkAttr attr;
    attr.adapterName = "my_test_adapter";
    attr.sampleRate = SAMPLE_RATE_48000;
    attr.channel = STEREO;
    EXPECT_EQ(directNode_->RenderSinkInit(attr), SUCCESS);
    EXPECT_EQ(std::string(directNode_->sinkOutAttr_.adapterName), "my_test_adapter");
}

// ==================== UpdateAppsUid with empty vector ====================

/**
 * @tc.name  : updateAppsUid_EmptyVector_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_053
 * @tc.desc  : Test UpdateAppsUid with empty uid vector
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, updateAppsUid_EmptyVector_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, UpdateAppsUid(_)).WillOnce(Return(SUCCESS));

    std::vector<int32_t> emptyUid;
    int32_t ret = directNode_->UpdateAppsUid(emptyUid);
    EXPECT_EQ(ret, SUCCESS);
}

// ==================== NotifyStreamChangeToSink with REMOVE type ====================

/**
 * @tc.name  : notifyStreamChange_RemoveType_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_054
 * @tc.desc  : Test NotifyStreamChangeToSink with REMOVE change type
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, notifyStreamChange_RemoveType_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_NE(directNode_->audioRendererSink_, nullptr);
    directNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_REMOVE, DEFAULT_SESSION_ID, STREAM_USAGE_MUSIC, RENDERER_RELEASED, 1000);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_NEW);
}

// ==================== Private function tests ====================

/**
 * @tc.name  : readDataAndConvertFormat_noInputData_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_055
 * @tc.desc  : Test ReadDataAndConvertFormat returns false when no input data (not connected)
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, readDataAndConvertFormat_noInputData_001, TestSize.Level0)
{
    // inputStream_ not connected → ReadPreOutputData returns empty vector
    EXPECT_EQ(directNode_->ReadDataAndConvertFormat(), false);
}

/**
 * @tc.name  : readDataAndConvertFormat_invalidBuffer_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_057
 * @tc.desc  : Test ReadDataAndConvertFormat returns false when input buffer is invalid
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, readDataAndConvertFormat_invalidBuffer_001, TestSize.Level0)
{
    HpaeNodeInfo inputNodeInfo;
    PrepareNodeInfo(inputNodeInfo);
    inputNodeInfo.streamType = STREAM_MUSIC;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(inputNodeInfo);
    auto converterNode = std::make_shared<HpaeAudioFormatConverterNode>(inputNodeInfo, inputNodeInfo);
    converterNode->Connect(sinkInputNode);
    directNode_->Connect(converterNode);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_F32LE);
    sinkInputNode->RegisterWriteCallback(writeCb);

    // Make the input buffer invalid
    sinkInputNode->inputAudioBuffer_.SetBufferValid(false);

    // DoProcess will call ReadDataAndConvertFormat, which returns false → DoProcess returns early
    directNode_->DoProcess();
    // currentSize_ should remain 0 since ReadDataAndConvertFormat returned false
    EXPECT_EQ(directNode_->currentSize_, 0u);
}

/**
 * @tc.name  : privateMembers_initialState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_058
 * @tc.desc  : Test private members initial state after construction
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, privateMembers_initialState_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);

    EXPECT_EQ(node->state_, STREAM_MANAGER_NEW);
    EXPECT_EQ(node->currentSize_, 0u);
    EXPECT_GT(node->renderSize_, 0u);
    EXPECT_EQ(node->outputSize_, node->renderSize_);
    EXPECT_EQ(node->latency_, 0u);
    EXPECT_EQ(node->audioRendererSink_, nullptr);
    EXPECT_NE(node->renderFrameData_.size(), 0u);
}

/**
 * @tc.name  : privateMembers_renderFrameDataSize_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_059
 * @tc.desc  : Test renderFrameData_ size matches frameLen * channels * formatSize
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, privateMembers_renderFrameDataSize_001, TestSize.Level0)
{
    size_t expectedSize = DEFAULT_FRAME_LEN * STEREO * GetSizeFromFormat(SAMPLE_F32LE);
    EXPECT_EQ(directNode_->renderFrameData_.size(), expectedSize);
    EXPECT_EQ(directNode_->renderSize_, expectedSize);
}

/**
 * @tc.name  : setSinkState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_060
 * @tc.desc  : Test SetSinkState transitions through all states
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, setSinkState_001, TestSize.Level0)
{
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_NEW);

    directNode_->SetSinkState(STREAM_MANAGER_IDLE);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_IDLE);

    directNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_RUNNING);

    directNode_->SetSinkState(STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_SUSPENDED);

    directNode_->SetSinkState(STREAM_MANAGER_RELEASED);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_RELEASED);
}

/**
 * @tc.name  : renderSinkFlush_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_061
 * @tc.desc  : Test RenderSinkFlush returns ERR_ILLEGAL_STATE when audioRendererSink_ is null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkFlush_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    EXPECT_EQ(directNode_->RenderSinkFlush(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkFlush_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_062
 * @tc.desc  : Test RenderSinkFlush calls sink Flush when sink is valid
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkFlush_002, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Flush()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(directNode_->RenderSinkFlush(), SUCCESS);
}

/**
 * @tc.name  : getLatency_nullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_063
 * @tc.desc  : Test GetLatency returns 0 when audioRendererSink_ is null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_nullSink_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    EXPECT_EQ(directNode_->GetLatency(), 0u);
}

/**
 * @tc.name  : getLatency_getLatencySuccess_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_064
 * @tc.desc  : Test GetLatency with successful GetLatency from sink
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_getLatencySuccess_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetLatency(_)).WillOnce([](uint32_t &latency) {
        latency = 10;
        return SUCCESS;
    });

    uint64_t lat = directNode_->GetLatency();
    EXPECT_EQ(lat, static_cast<uint64_t>(10 * AUDIO_US_PER_MS + AUDIO_FRAME_WORK_LATENCY_US));
    EXPECT_EQ(directNode_->latency_, lat);
}

/**
 * @tc.name  : getLatency_getLatencyFail_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_065
 * @tc.desc  : Test GetLatency uses default when sink GetLatency fails
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_getLatencyFail_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetLatency(_)).WillOnce(Return(ERROR));

    uint64_t lat = directNode_->GetLatency();
    EXPECT_EQ(lat, static_cast<uint64_t>(AUDIO_DEFAULT_LATENCY_US));
    EXPECT_EQ(directNode_->latency_, lat);
}

/**
 * @tc.name  : getLatency_cached_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_066
 * @tc.desc  : Test GetLatency returns cached value on second call
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getLatency_cached_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetLatency(_)).WillOnce([](uint32_t &latency) {
        latency = 10;
        return SUCCESS;
    });

    uint64_t lat1 = directNode_->GetLatency();
    // Second call should use cached value, no additional mock call expected
    uint64_t lat2 = directNode_->GetLatency();
    EXPECT_EQ(lat1, lat2);
}

/**
 * @tc.name  : renderSinkStart_nullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_067
 * @tc.desc  : Test RenderSinkStart returns ERR_ILLEGAL_STATE when sink is null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStart_nullSink_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    EXPECT_EQ(directNode_->RenderSinkStart(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkStart_fail_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_068
 * @tc.desc  : Test RenderSinkStart returns error when sink Start fails
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStart_fail_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(ERROR));
    EXPECT_EQ(directNode_->RenderSinkStart(), ERROR);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_NEW);
}

/**
 * @tc.name  : renderSinkStart_success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_069
 * @tc.desc  : Test RenderSinkStart sets state to RUNNING on success
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStart_success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(directNode_->RenderSinkStart(), SUCCESS);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : renderSinkStop_nullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_070
 * @tc.desc  : Test RenderSinkStop returns ERR_ILLEGAL_STATE when sink is null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStop_nullSink_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    EXPECT_EQ(directNode_->RenderSinkStop(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkStop_fail_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_071
 * @tc.desc  : Test RenderSinkStop returns error when sink Stop fails
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStop_fail_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(ERROR));
    EXPECT_EQ(directNode_->RenderSinkStop(), ERROR);
    // State should be set to SUSPENDED before Stop call
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : renderSinkStop_success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_072
 * @tc.desc  : Test RenderSinkStop resets currentSize_ and sets state to SUSPENDED
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkStop_success_001, TestSize.Level0)
{
    directNode_->currentSize_ = 100;
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(directNode_->RenderSinkStop(), SUCCESS);
    EXPECT_EQ(directNode_->currentSize_, 0u);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : renderSinkDeInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_073
 * @tc.desc  : Test RenderSinkDeInit with null sink returns ERR_ILLEGAL_STATE
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkDeInit_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    EXPECT_EQ(directNode_->RenderSinkDeInit(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkDeInit_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_074
 * @tc.desc  : Test RenderSinkDeInit sets state to RELEASED and nullifies sink
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkDeInit_002, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, DeInit()).Times(1);
    EXPECT_EQ(directNode_->RenderSinkDeInit(), SUCCESS);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_RELEASED);
    EXPECT_EQ(directNode_->audioRendererSink_, nullptr);
}

/**
 * @tc.name  : updateAppsUid_nullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_075
 * @tc.desc  : Test UpdateAppsUid returns ERROR when sink is null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, updateAppsUid_nullSink_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    std::vector<int32_t> uids = {100, 200};
    EXPECT_EQ(directNode_->UpdateAppsUid(uids), ERROR);
}

/**
 * @tc.name  : updateAppsUid_notInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_076
 * @tc.desc  : Test UpdateAppsUid returns ERR_ILLEGAL_STATE when sink not inited
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, updateAppsUid_notInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    std::vector<int32_t> uids = {100, 200};
    EXPECT_EQ(directNode_->UpdateAppsUid(uids), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : notifyStreamChange_nullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_077
 * @tc.desc  : Test NotifyStreamChangeToSink does not crash when sink is null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, notifyStreamChange_nullSink_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    StreamManagerState prevState = directNode_->GetSinkState();
    directNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, STREAM_USAGE_MUSIC, RENDERER_RUNNING);
    EXPECT_EQ(directNode_->GetSinkState(), prevState);
}

/**
 * @tc.name  : notifyStreamChange_notInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_078
 * @tc.desc  : Test NotifyStreamChangeToSink does not call sink when not inited
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, notifyStreamChange_notInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    StreamManagerState prevState = directNode_->GetSinkState();
    directNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, STREAM_USAGE_MUSIC, RENDERER_RUNNING);
    EXPECT_EQ(directNode_->GetSinkState(), prevState);
}

/**
 * @tc.name  : renderSinkInit_nullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_079
 * @tc.desc  : Test RenderSinkInit returns ERR_ILLEGAL_STATE when sink is null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_nullSink_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    IAudioSinkAttr attr;
    EXPECT_EQ(directNode_->RenderSinkInit(attr), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkInit_initFail_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_080
 * @tc.desc  : Test RenderSinkInit returns error when sink Init fails
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_initFail_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink_, Init(_)).WillOnce(Return(ERROR));

    IAudioSinkAttr attr;
    EXPECT_EQ(directNode_->RenderSinkInit(attr), ERROR);
}

/**
 * @tc.name  : doProcess_nullSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_081
 * @tc.desc  : Test DoProcess returns early when audioRendererSink_ is null
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, doProcess_nullSink_001, TestSize.Level0)
{
    directNode_->audioRendererSink_ = nullptr;
    size_t prevSize = directNode_->currentSize_;
    directNode_->DoProcess();
    EXPECT_EQ(directNode_->currentSize_, prevSize);
}

/**
 * @tc.name  : renderSinkInit_alreadyInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_084
 * @tc.desc  : Test RenderSinkInit with already initialized sink sets IDLE state
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, renderSinkInit_alreadyInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));

    IAudioSinkAttr attr;
    attr.adapterName = "test_adapter";
    EXPECT_EQ(directNode_->RenderSinkInit(attr), SUCCESS);
    EXPECT_EQ(directNode_->state_, STREAM_MANAGER_IDLE);
}

/**
 * @tc.name  : priv_getRenderFrameData_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_085
 * @tc.desc  : Test GetRenderFrameData returns pointer to renderFrameData_
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, priv_getRenderFrameData_001, TestSize.Level0)
{
    const char *data = directNode_->GetRenderFrameData();
    EXPECT_NE(data, nullptr);
}

/**
 * @tc.name  : getSinkState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_086
 * @tc.desc  : Test GetSinkState returns current state
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, getSinkState_001, TestSize.Level0)
{
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_NEW);
    directNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(directNode_->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : constructNode_withS32LE_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_087
 * @tc.desc  : Test construct with SAMPLE_S32LE format sets correct renderFrameData size
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, constructNode_withS32LE_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.format = SAMPLE_S32LE;
    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);

    size_t expectedSize = DEFAULT_FRAME_LEN * STEREO * GetSizeFromFormat(SAMPLE_S32LE);
    EXPECT_EQ(node->renderFrameData_.size(), expectedSize);
    EXPECT_EQ(node->renderSize_, expectedSize);
}

/**
 * @tc.name  : constructNode_withMono_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectSinkOutputNodeTest_088
 * @tc.desc  : Test construct with MONO channel sets correct renderFrameData size
 */
HWTEST_F(HpaeDirectSinkOutputNodeTest, constructNode_withMono_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.channels = MONO;
    auto node = std::make_shared<HpaeDirectSinkOutputNode>(nodeInfo);

    size_t expectedSize = DEFAULT_FRAME_LEN * MONO * GetSizeFromFormat(SAMPLE_F32LE);
    EXPECT_EQ(node->renderFrameData_.size(), expectedSize);
    EXPECT_EQ(node->renderSize_, expectedSize);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
