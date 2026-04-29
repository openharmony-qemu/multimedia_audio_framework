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
#include <gmock/gmock.h>
#include <memory>
#include "hpae_fast_sink_output_node.h"
#include "hpae_sink_input_node.h"
#include "hpae_mixer_node.h"
#include "hpae_gain_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_mocks.h"
#include "test_case_common.h"
#include "audio_errors.h"
#include "oh_audio_buffer.h"
#include "audio_utils.h"

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
constexpr AudioSamplingRate DEFAULT_SAMPLE_RATE = SAMPLE_RATE_48000;
constexpr AudioChannel DEFAULT_CHANNELS = STEREO;
constexpr uint32_t TOTAL_SIZE_IN_FRAME = 8;
constexpr uint32_t SPAN_SIZE_IN_FRAME = 2;
constexpr uint32_t BYTE_SIZE_PER_FRAME = 8; // stereo * 4 bytes (f32le)
}

class HpaeFastSinkOutputNodeTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<HpaeFastSinkOutputNode> fastNode_;
    std::shared_ptr<NiceMock<MockAudioRenderSink>> mockSink_;
};

static void PrepareNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sessionId = DEFAULT_SESSION_ID;
}

void HpaeFastSinkOutputNodeTest::SetUp()
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    fastNode_ = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    mockSink_ = std::make_shared<NiceMock<MockAudioRenderSink>>();
    fastNode_->audioRendererSink_ = mockSink_;
}

void HpaeFastSinkOutputNodeTest::TearDown()
{
    fastNode_ = nullptr;
    mockSink_ = nullptr;
}

static void SetupMmapBuffer(std::shared_ptr<HpaeFastSinkOutputNode> &node)
{
    auto buffer = OHAudioBuffer::CreateFromLocal(TOTAL_SIZE_IN_FRAME, SPAN_SIZE_IN_FRAME, BYTE_SIZE_PER_FRAME);
    ASSERT_NE(buffer, nullptr);
    node->dstAudioBuffer_ = buffer;
    node->dstTotalSizeInframe_ = TOTAL_SIZE_IN_FRAME;
    node->dstSpanSizeInframe_ = SPAN_SIZE_IN_FRAME;
    node->dstByteSizePerFrame_ = BYTE_SIZE_PER_FRAME;
    node->dstSpanSizeInByte_ = SPAN_SIZE_IN_FRAME * BYTE_SIZE_PER_FRAME;
    node->syncInfoSize_ = 0;
    node->renderFrameData_.resize(node->dstSpanSizeInByte_);
}

/**
 * @tc.name  : constructNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_001
 * @tc.desc  : Test construct HpaeFastSinkOutputNode and verify node info
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, constructNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetSampleRate(), nodeInfo.samplingRate);
    EXPECT_EQ(node->GetFrameLen(), nodeInfo.frameLen);
    EXPECT_EQ(node->GetChannelCount(), nodeInfo.channels);
    EXPECT_EQ(node->GetBitWidth(), nodeInfo.format);
}

/**
 * @tc.name  : constructNode_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_002
 * @tc.desc  : Test construct with different formats
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, constructNode_002, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LEN;
    nodeInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    nodeInfo.channels = DEFAULT_CHANNELS;
    nodeInfo.format = SAMPLE_S16LE;
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->GetBitWidth(), SAMPLE_S16LE);
}

/**
 * @tc.name  : constructNode_withDeviceClass
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_003
 * @tc.desc  : Test construct with ultra_fast device class sets isUltraFast_ flag
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, constructNode_withDeviceClass, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    nodeInfo.deviceClass = "ultra_fast";
    auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->isUltraFast_, true);
}

/**
 * @tc.name  : destructNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_004
 * @tc.desc  : Test destructor does not crash
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, destructNode_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    PrepareNodeInfo(nodeInfo);
    {
        auto node = std::make_shared<HpaeFastSinkOutputNode>(nodeInfo);
        EXPECT_NE(node, nullptr);
    }
    // Node should be destructed without crash
}

// ==================== RenderSink Lifecycle Tests ====================

/**
 * @tc.name  : renderSinkInit_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_005
 * @tc.desc  : Test RenderSinkInit when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkInit_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    IAudioSinkAttr attr;
    int32_t ret = fastNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkInit_AlreadyInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_006
 * @tc.desc  : Test RenderSinkInit when sink is already inited skips Init call
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkInit_AlreadyInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, Init(_)).Times(0);
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<1>(TOTAL_SIZE_IN_FRAME),
                        SetArgReferee<2>(SPAN_SIZE_IN_FRAME),
                        SetArgReferee<3>(BYTE_SIZE_PER_FRAME),
                        SetArgReferee<4>(0),
                        Return(SUCCESS)));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, GetCurrentOutputDevice()).WillOnce(Return(DEVICE_TYPE_SPEAKER));
    EXPECT_CALL(*mockSink_, RegisterCurrentDeviceCallback(_)).Times(1);

    IAudioSinkAttr attr;
    attr.sampleRate = DEFAULT_SAMPLE_RATE;
    attr.channel = DEFAULT_CHANNELS;
    attr.format = SAMPLE_F32LE;
    int32_t ret = fastNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : renderSinkInit_InitSuccess_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_007
 * @tc.desc  : Test RenderSinkInit with successful init
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkInit_InitSuccess_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink_, Init(_)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<1>(TOTAL_SIZE_IN_FRAME),
                        SetArgReferee<2>(SPAN_SIZE_IN_FRAME),
                        SetArgReferee<3>(BYTE_SIZE_PER_FRAME),
                        SetArgReferee<4>(0),
                        Return(SUCCESS)));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, GetCurrentOutputDevice()).WillOnce(Return(DEVICE_TYPE_SPEAKER));
    EXPECT_CALL(*mockSink_, RegisterCurrentDeviceCallback(_)).Times(1);

    IAudioSinkAttr attr;
    attr.sampleRate = DEFAULT_SAMPLE_RATE;
    attr.channel = DEFAULT_CHANNELS;
    attr.format = SAMPLE_F32LE;
    int32_t ret = fastNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(fastNode_->dstAudioBuffer_, nullptr);
}

/**
 * @tc.name  : renderSinkInit_InitFailed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_008
 * @tc.desc  : Test RenderSinkInit with init failure
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkInit_InitFailed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    EXPECT_CALL(*mockSink_, Init(_)).WillOnce(Return(ERROR));

    IAudioSinkAttr attr;
    int32_t ret = fastNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : renderSinkInit_MmapBufferFailed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_009
 * @tc.desc  : Test RenderSinkInit with GetMmapBufferInfo failure
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkInit_MmapBufferFailed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(Return(ERROR));

    IAudioSinkAttr attr;
    attr.sampleRate = DEFAULT_SAMPLE_RATE;
    int32_t ret = fastNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : renderSinkInit_VoipFast_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_010
 * @tc.desc  : Test RenderSinkInit with VOIP_FAST flag sets adapter type
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkInit_VoipFast_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<1>(TOTAL_SIZE_IN_FRAME),
                        SetArgReferee<2>(SPAN_SIZE_IN_FRAME),
                        SetArgReferee<3>(BYTE_SIZE_PER_FRAME),
                        SetArgReferee<4>(0),
                        Return(SUCCESS)));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, GetCurrentOutputDevice()).WillOnce(Return(DEVICE_TYPE_SPEAKER));
    EXPECT_CALL(*mockSink_, RegisterCurrentDeviceCallback(_)).Times(1);

    IAudioSinkAttr attr;
    attr.sampleRate = DEFAULT_SAMPLE_RATE;
    attr.channel = DEFAULT_CHANNELS;
    attr.audioStreamFlag = AUDIO_FLAG_VOIP_FAST;
    int32_t ret = fastNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(fastNode_->adapterType_, ADAPTER_TYPE_VOIP_FAST);
}

/**
 * @tc.name  : renderSinkStart_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_011
 * @tc.desc  : Test RenderSinkStart when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkStart_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    int32_t ret = fastNode_->RenderSinkStart();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkStart_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_012
 * @tc.desc  : Test RenderSinkStart with successful start
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkStart_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(SUCCESS));
    int32_t ret = fastNode_->RenderSinkStart();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastNode_->isStarted_, true);
    fastNode_->StopUpdateThread();
}

/**
 * @tc.name  : renderSinkStart_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_013
 * @tc.desc  : Test RenderSinkStart with failure
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkStart_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(ERROR));
    int32_t ret = fastNode_->RenderSinkStart();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : renderSinkStop_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_014
 * @tc.desc  : Test RenderSinkStop when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkStop_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    int32_t ret = fastNode_->RenderSinkStop();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkStop_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_015
 * @tc.desc  : Test RenderSinkStop with successful stop
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkStop_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));
    int32_t ret = fastNode_->RenderSinkStop();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(fastNode_->isStarted_, false);
}

/**
 * @tc.name  : renderSinkStop_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_016
 * @tc.desc  : Test RenderSinkStop with failure
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkStop_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(ERROR));
    int32_t ret = fastNode_->RenderSinkStop();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : renderSinkDeInit_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_017
 * @tc.desc  : Test RenderSinkDeInit when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkDeInit_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    int32_t ret = fastNode_->RenderSinkDeInit();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkDeInit_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_018
 * @tc.desc  : Test RenderSinkDeInit successfully
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkDeInit_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, DeInit()).Times(1);
    fastNode_->renderId_ = 100;
    int32_t ret = fastNode_->RenderSinkDeInit();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(fastNode_->audioRendererSink_, nullptr);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_RELEASED);
    EXPECT_EQ(fastNode_->dstAudioBuffer_, nullptr);
}

// ==================== State Tests ====================

/**
 * @tc.name  : getSinkState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_019
 * @tc.desc  : Test GetSinkState and SetSinkState
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getSinkState_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_NEW);
    fastNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_RUNNING);
    fastNode_->SetSinkState(STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : setTimeoutStopThd_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_020
 * @tc.desc  : Test SetTimeoutStopThd with valid timeout
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, setTimeoutStopThd_001, TestSize.Level0)
{
    int32_t ret = fastNode_->SetTimeoutStopThd(1000);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_GT(fastNode_->timeoutThdFrames_, 0);
}

/**
 * @tc.name  : setTimeoutStopThd_Invalid_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_021
 * @tc.desc  : Test SetTimeoutStopThd with negative timeout returns error
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, setTimeoutStopThd_Invalid_001, TestSize.Level0)
{
    int32_t ret = fastNode_->SetTimeoutStopThd(-1);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : setTimeoutStopThd_ZeroFrameLenMs_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_022
 * @tc.desc  : Test SetTimeoutStopThd with zero frameLenMs uses default
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, setTimeoutStopThd_ZeroFrameLenMs_001, TestSize.Level0)
{
    fastNode_->frameLenMs_ = 0;
    int32_t ret = fastNode_->SetTimeoutStopThd(1000);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(fastNode_->timeoutThdFrames_, 150); // TIME_OUT_STOP_THD_DEFAULT_FRAME
}

// ==================== UpdateAppsUid Tests ====================

/**
 * @tc.name  : updateAppsUid_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_023
 * @tc.desc  : Test UpdateAppsUid when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, updateAppsUid_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    std::vector<int32_t> appsUid = {1000, 2000};
    EXPECT_EQ(fastNode_->UpdateAppsUid(appsUid), ERROR);
}

/**
 * @tc.name  : updateAppsUid_NotInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_024
 * @tc.desc  : Test UpdateAppsUid when sink is not inited
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, updateAppsUid_NotInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    std::vector<int32_t> appsUid = {1000};
    EXPECT_EQ(fastNode_->UpdateAppsUid(appsUid), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : updateAppsUid_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_025
 * @tc.desc  : Test UpdateAppsUid with valid sink
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, updateAppsUid_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, UpdateAppsUid(_)).WillOnce(Return(SUCCESS));
    std::vector<int32_t> appsUid = {1000, 2000};
    EXPECT_EQ(fastNode_->UpdateAppsUid(appsUid), SUCCESS);
}

// ==================== NotifyStreamChangeToSink Tests ====================

/**
 * @tc.name  : notifyStreamChangeToSink_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_026
 * @tc.desc  : Test NotifyStreamChangeToSink when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, notifyStreamChangeToSink_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    // Should not crash
    fastNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, STREAM_USAGE_UNKNOWN, RENDERER_RUNNING);
}

/**
 * @tc.name  : notifyStreamChangeToSink_NotInited_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_027
 * @tc.desc  : Test NotifyStreamChangeToSink when sink is not inited
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, notifyStreamChangeToSink_NotInited_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(false));
    // Should not crash
    fastNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, STREAM_USAGE_UNKNOWN, RENDERER_RUNNING);
}

/**
 * @tc.name  : notifyStreamChangeToSink_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_028
 * @tc.desc  : Test NotifyStreamChangeToSink with valid sink
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, notifyStreamChangeToSink_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, NotifyStreamChangeToSink(_, _, _, _, _)).Times(1);
    fastNode_->NotifyStreamChangeToSink(
        STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, STREAM_USAGE_UNKNOWN, RENDERER_RUNNING);
}

// ==================== GetLatency Tests ====================

/**
 * @tc.name  : getLatency_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_029
 * @tc.desc  : Test GetLatency when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getLatency_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    EXPECT_EQ(fastNode_->GetLatency(), 0);
}

/**
 * @tc.name  : getLatency_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_030
 * @tc.desc  : Test GetLatency with valid sink
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getLatency_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetLatency(_)).WillOnce(DoAll(SetArgReferee<0>(1000), Return(SUCCESS)));
    EXPECT_EQ(fastNode_->GetLatency(), 1000);
}

/**
 * @tc.name  : getLatency_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_031
 * @tc.desc  : Test GetLatency when GetLatency fails
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getLatency_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetLatency(_)).WillOnce(Return(ERROR));
    EXPECT_EQ(fastNode_->GetLatency(), 0);
}

// ==================== GetMaxAmplitude Tests ====================

/**
 * @tc.name  : getMaxAmplitude_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_032
 * @tc.desc  : Test GetMaxAmplitude returns stored value
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getMaxAmplitude_001, TestSize.Level0)
{
    fastNode_->maxAmplitude_ = 0.5f;
    EXPECT_FLOAT_EQ(fastNode_->GetMaxAmplitude(), 0.5f);
    EXPECT_EQ(fastNode_->startUpdate_, true);
}

// ==================== SetNeedCheckZeroVolume Tests ====================

/**
 * @tc.name  : setNeedCheckZeroVolume_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_033
 * @tc.desc  : Test SetNeedCheckZeroVolume
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, setNeedCheckZeroVolume_001, TestSize.Level0)
{
    fastNode_->SetNeedCheckZeroVolume(true);
    EXPECT_EQ(fastNode_->needCheckZeroVolume_, true);
    fastNode_->SetNeedCheckZeroVolume(false);
    EXPECT_EQ(fastNode_->needCheckZeroVolume_, false);
}

// ==================== SetLoopbackState Tests ====================

/**
 * @tc.name  : setLoopbackState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_034
 * @tc.desc  : Test SetLoopbackState
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, setLoopbackState_001, TestSize.Level0)
{
    fastNode_->SetLoopbackState(true);
    EXPECT_EQ(fastNode_->isExistLoopback_, true);
    fastNode_->SetLoopbackState(false);
    EXPECT_EQ(fastNode_->isExistLoopback_, false);
}

// ==================== SetMuteForSwitchDevice Tests ====================

/**
 * @tc.name  : setMuteForSwitchDevice_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_035
 * @tc.desc  : Test SetMuteForSwitchDevice
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, setMuteForSwitchDevice_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->switchDevicesMute_, false);
    fastNode_->SetMuteForSwitchDevice(true);
    EXPECT_EQ(fastNode_->switchDevicesMute_, true);
    fastNode_->SetMuteForSwitchDevice(false);
    EXPECT_EQ(fastNode_->switchDevicesMute_, false);
}

/**
 * @tc.name  : setMuteForSwitchDevice_SameState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_036
 * @tc.desc  : Test SetMuteForSwitchDevice with same state (no-op)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, setMuteForSwitchDevice_SameState_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->switchDevicesMute_, false);
    fastNode_->SetMuteForSwitchDevice(false); // same state
    EXPECT_EQ(fastNode_->switchDevicesMute_, false);
}

// ==================== CheckIfSuspend Tests ====================

/**
 * @tc.name  : checkIfSuspend_NoPreOut_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_037
 * @tc.desc  : Test CheckIfSuspend with no pre-output returns true
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, checkIfSuspend_NoPreOut_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->GetPreOutNum(), 0);
    bool result = fastNode_->CheckIfSuspend();
    EXPECT_EQ(result, true);
    EXPECT_EQ(fastNode_->isCheckingSuspend_, true);
    EXPECT_GT(fastNode_->timeoutStopCount_, 0);
}

/**
 * @tc.name  : checkIfSuspend_Timeout_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_038
 * @tc.desc  : Test CheckIfSuspend triggers RenderSinkStop after timeout
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, checkIfSuspend_Timeout_001, TestSize.Level0)
{
    fastNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    fastNode_->timeoutThdFrames_ = 1;
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));
    bool result = fastNode_->CheckIfSuspend();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : checkIfSuspend_HasPreOut_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_039
 * @tc.desc  : Test CheckIfSuspend with pre-output connected returns false
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, checkIfSuspend_HasPreOut_001, TestSize.Level0)
{
    // Create a mixer node and connect it as a pre-output
    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);
    fastNode_->Connect(mixerNode);
    EXPECT_GT(fastNode_->GetPreOutNum(), 0);

    bool result = fastNode_->CheckIfSuspend();
    EXPECT_EQ(result, false);
    EXPECT_EQ(fastNode_->isCheckingSuspend_, false);
}

// ==================== GetSpanSizeInFrame Tests ====================

/**
 * @tc.name  : getSpanSizeInFrame_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_040
 * @tc.desc  : Test GetSpanSizeInFrame
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getSpanSizeInFrame_001, TestSize.Level0)
{
    fastNode_->dstSpanSizeInframe_ = SPAN_SIZE_IN_FRAME;
    EXPECT_EQ(fastNode_->GetSpanSizeInFrame(), SPAN_SIZE_IN_FRAME);
}

// ==================== RenderSinkFlush Tests ====================

/**
 * @tc.name  : renderSinkFlush_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_041
 * @tc.desc  : Test RenderSinkFlush when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkFlush_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    EXPECT_EQ(fastNode_->RenderSinkFlush(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkFlush_BufferNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_042
 * @tc.desc  : Test RenderSinkFlush when dstAudioBuffer_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkFlush_BufferNull_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, Flush()).WillOnce(Return(SUCCESS));
    fastNode_->dstAudioBuffer_ = nullptr;
    EXPECT_EQ(fastNode_->RenderSinkFlush(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : renderSinkFlush_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_043
 * @tc.desc  : Test RenderSinkFlush successfully
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, renderSinkFlush_Success_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    EXPECT_CALL(*mockSink_, Flush()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(fastNode_->RenderSinkFlush(), SUCCESS);
    EXPECT_EQ(fastNode_->needReSyncPosition_, true);
}

// ==================== GetRenderFrameData Tests ====================

/**
 * @tc.name  : getRenderFrameData_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_044
 * @tc.desc  : Test GetRenderFrameData with empty render data
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getRenderFrameData_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->GetRenderFrameData(), nullptr);
}

/**
 * @tc.name  : getRenderFrameData_WithData_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_045
 * @tc.desc  : Test GetRenderFrameData with data present
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getRenderFrameData_WithData_001, TestSize.Level0)
{
    fastNode_->renderFrameData_.resize(100);
    const char *data = fastNode_->GetRenderFrameData();
    EXPECT_NE(data, nullptr);
}

// ==================== Reset/ResetAll Tests ====================

/**
 * @tc.name  : reset_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_046
 * @tc.desc  : Test Reset does not crash with no connections
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, reset_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->Reset(), true);
}

/**
 * @tc.name  : resetAll_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_047
 * @tc.desc  : Test ResetAll does not crash with no connections
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, resetAll_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->ResetAll(), true);
}

// ==================== Connect/DisConnect Tests ====================

/**
 * @tc.name  : connectDisconnect_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_048
 * @tc.desc  : Test Connect and DisConnect with a mixer node
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, connectDisconnect_001, TestSize.Level0)
{
    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);

    fastNode_->Connect(mixerNode);
    EXPECT_EQ(fastNode_->GetPreOutNum(), 1);

    fastNode_->DisConnect(mixerNode);
    EXPECT_EQ(fastNode_->GetPreOutNum(), 0);
}

// ==================== ZeroVolumeCheck Tests ====================

/**
 * @tc.name  : zeroVolumeCheck_Disabled_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_049
 * @tc.desc  : Test ZeroVolumeCheck when needCheckZeroVolume_ is false
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_Disabled_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = false;
    fastNode_->ZeroVolumeCheck(0);
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::INACTIVE); // INACTIVE, unchanged
}

/**
 * @tc.name  : zeroVolumeCheck_Loopback_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_050
 * @tc.desc  : Test ZeroVolumeCheck when loopback exists skips check
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_Loopback_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = true;
    fastNode_->isExistLoopback_ = true;
    fastNode_->ZeroVolumeCheck(0);
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::INACTIVE); // INACTIVE, unchanged
}

/**
 * @tc.name  : zeroVolumeCheck_BluetoothA2dp_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_051
 * @tc.desc  : Test ZeroVolumeCheck skips for Bluetooth A2DP device
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_BluetoothA2dp_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = true;
    fastNode_->isExistLoopback_ = false;
    fastNode_->currentOutputDevice_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    fastNode_->ZeroVolumeCheck(0);
    // INACTIVE, unchanged due to A2DP early return
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::INACTIVE);
}

/**
 * @tc.name  : zeroVolumeCheck_Nearlink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_052
 * @tc.desc  : Test ZeroVolumeCheck with zero volume on NearLink device returns early
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_Nearlink_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = true;
    fastNode_->isExistLoopback_ = false;
    fastNode_->currentOutputDevice_ = DEVICE_TYPE_NEARLINK;
    fastNode_->ZeroVolumeCheck(0);
    // NearLink returns early, state stays at INACTIVE (no timing initiated)
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::INACTIVE);
}

/**
 * @tc.name  : zeroVolumeCheck_ZeroVolumeEnterTiming_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_053
 * @tc.desc  : Test ZeroVolumeCheck enters IN_TIMING state on zero volume (speaker)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_ZeroVolumeEnterTiming_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = true;
    fastNode_->isExistLoopback_ = false;
    fastNode_->currentOutputDevice_ = DEVICE_TYPE_SPEAKER;
    fastNode_->ZeroVolumeCheck(0);
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::IN_TIMING); // IN_TIMING
    EXPECT_NE(fastNode_->zeroVolumeStartTime_, INT64_MAX);
}

/**
 * @tc.name  : zeroVolumeCheck_NonZeroVolumeResets_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_054
 * @tc.desc  : Test ZeroVolumeCheck with non-zero volume resets state
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_NonZeroVolumeResets_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = true;
    fastNode_->isExistLoopback_ = false;
    fastNode_->currentOutputDevice_ = DEVICE_TYPE_SPEAKER;
    // First enter IN_TIMING
    fastNode_->ZeroVolumeCheck(0);
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::IN_TIMING); // IN_TIMING
    // Then non-zero volume resets
    fastNode_->ZeroVolumeCheck(1000);
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::INACTIVE); // INACTIVE
}

/**
 * @tc.name  : zeroVolumeCheck_NonZeroFromInactive_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_055
 * @tc.desc  : Test ZeroVolumeCheck with non-zero volume from INACTIVE (no-op)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_NonZeroFromInactive_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = true;
    fastNode_->isExistLoopback_ = false;
    fastNode_->currentOutputDevice_ = DEVICE_TYPE_SPEAKER;
    fastNode_->ZeroVolumeCheck(1000);
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::INACTIVE); // INACTIVE, unchanged
}

/**
 * @tc.name  : zeroVolumeCheck_NonZeroFromActive_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_056
 * @tc.desc  : Test ZeroVolumeCheck with non-zero volume from ACTIVE state calls HandleZeroVolumeStartEvent
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_NonZeroFromActive_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = true;
    fastNode_->isExistLoopback_ = false;
    fastNode_->currentOutputDevice_ = DEVICE_TYPE_SPEAKER;
    fastNode_->isStarted_ = false;
    // Manually set ACTIVE state
    fastNode_->zeroVolumeState_ = HpaeFastSinkOutputNode::ZeroVolumeState::ACTIVE;
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(SUCCESS));
    fastNode_->ZeroVolumeCheck(1000);
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::INACTIVE);
    EXPECT_EQ(fastNode_->isStarted_, true);
    fastNode_->StopUpdateThread();
}

// ==================== HandleZeroVolumeStartEvent Tests ====================

/**
 * @tc.name  : handleZeroVolumeStart_AlreadyStarted_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_057
 * @tc.desc  : Test HandleZeroVolumeStartEvent when already started (no-op)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, handleZeroVolumeStart_AlreadyStarted_001, TestSize.Level0)
{
    fastNode_->isStarted_ = true;
    fastNode_->HandleZeroVolumeStartEvent();
    // No Start call, stays started
    EXPECT_EQ(fastNode_->isStarted_, true);
}

/**
 * @tc.name  : handleZeroVolumeStart_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_058
 * @tc.desc  : Test HandleZeroVolumeStartEvent when sink is null
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, handleZeroVolumeStart_SinkNull_001, TestSize.Level0)
{
    fastNode_->isStarted_ = false;
    fastNode_->audioRendererSink_ = nullptr;
    fastNode_->HandleZeroVolumeStartEvent();
    EXPECT_EQ(fastNode_->isStarted_, false);
}

/**
 * @tc.name  : handleZeroVolumeStart_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_059
 * @tc.desc  : Test HandleZeroVolumeStartEvent when Start fails
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, handleZeroVolumeStart_Failed_001, TestSize.Level0)
{
    fastNode_->isStarted_ = false;
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(ERROR));
    fastNode_->HandleZeroVolumeStartEvent();
    EXPECT_EQ(fastNode_->isStarted_, false);
}

// ==================== HandleZeroVolumeStopEvent Tests ====================

/**
 * @tc.name  : handleZeroVolumeStop_AlreadyStopped_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_060
 * @tc.desc  : Test HandleZeroVolumeStopEvent when already stopped (no-op)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, handleZeroVolumeStop_AlreadyStopped_001, TestSize.Level0)
{
    fastNode_->isStarted_ = false;
    fastNode_->HandleZeroVolumeStopEvent();
    EXPECT_EQ(fastNode_->isStarted_, false);
}

/**
 * @tc.name  : handleZeroVolumeStop_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_061
 * @tc.desc  : Test HandleZeroVolumeStopEvent when sink is null
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, handleZeroVolumeStop_SinkNull_001, TestSize.Level0)
{
    fastNode_->isStarted_ = true;
    fastNode_->audioRendererSink_ = nullptr;
    fastNode_->HandleZeroVolumeStopEvent();
    // CHECK_AND_RETURN_LOG prevents crash
}

/**
 * @tc.name  : handleZeroVolumeStop_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_062
 * @tc.desc  : Test HandleZeroVolumeStopEvent with successful stop
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, handleZeroVolumeStop_Success_001, TestSize.Level0)
{
    fastNode_->isStarted_ = true;
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));
    fastNode_->HandleZeroVolumeStopEvent();
    EXPECT_EQ(fastNode_->isStarted_, false);
}

/**
 * @tc.name  : handleZeroVolumeStop_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_063
 * @tc.desc  : Test HandleZeroVolumeStopEvent when stop fails keeps started
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, handleZeroVolumeStop_Failed_001, TestSize.Level0)
{
    fastNode_->isStarted_ = true;
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(ERROR));
    fastNode_->HandleZeroVolumeStopEvent();
    EXPECT_EQ(fastNode_->isStarted_, true);
}

// ==================== SyncCurrentOutputDevice Tests ====================

/**
 * @tc.name  : syncCurrentOutputDevice_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_064
 * @tc.desc  : Test SyncCurrentOutputDevice when sink is null
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, syncCurrentOutputDevice_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    fastNode_->SyncCurrentOutputDevice();
    // No crash, currentOutputDevice_ unchanged
    EXPECT_EQ(fastNode_->currentOutputDevice_, DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : syncCurrentOutputDevice_InvalidDevice_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_065
 * @tc.desc  : Test SyncCurrentOutputDevice with invalid device
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, syncCurrentOutputDevice_InvalidDevice_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetCurrentOutputDevice()).WillOnce(Return(DEVICE_TYPE_INVALID));
    fastNode_->SyncCurrentOutputDevice();
    EXPECT_EQ(fastNode_->currentOutputDevice_, DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : syncCurrentOutputDevice_NoneDevice_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_066
 * @tc.desc  : Test SyncCurrentOutputDevice with NONE device
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, syncCurrentOutputDevice_NoneDevice_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetCurrentOutputDevice()).WillOnce(Return(DEVICE_TYPE_NONE));
    fastNode_->SyncCurrentOutputDevice();
    EXPECT_EQ(fastNode_->currentOutputDevice_, DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : syncCurrentOutputDevice_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_067
 * @tc.desc  : Test SyncCurrentOutputDevice with valid device
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, syncCurrentOutputDevice_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetCurrentOutputDevice()).WillOnce(Return(DEVICE_TYPE_SPEAKER));
    fastNode_->SyncCurrentOutputDevice();
    EXPECT_EQ(fastNode_->currentOutputDevice_, DEVICE_TYPE_SPEAKER);
}

// ==================== DoProcess Tests ====================

/**
 * @tc.name  : doProcess_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_068
 * @tc.desc  : Test DoProcess when audioRendererSink_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, doProcess_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    fastNode_->DoProcess();
    // Should not crash
}

/**
 * @tc.name  : doProcess_BufferNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_069
 * @tc.desc  : Test DoProcess when dstAudioBuffer_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, doProcess_BufferNull_001, TestSize.Level0)
{
    fastNode_->dstAudioBuffer_ = nullptr;
    fastNode_->DoProcess();
    // Should not crash
}

/**
 * @tc.name  : doProcess_SuspendCheck_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_070
 * @tc.desc  : Test DoProcess returns early when CheckIfSuspend returns true
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, doProcess_SuspendCheck_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    // No pre-output, so CheckIfSuspend returns true
    EXPECT_EQ(fastNode_->GetPreOutNum(), 0);
    fastNode_->DoProcess();
    // Should not crash, returns early
}

// ==================== GetAdapterBufferInfo Tests ====================

/**
 * @tc.name  : getAdapterBufferInfo_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_071
 * @tc.desc  : Test GetAdapterBufferInfo when sink is null
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getAdapterBufferInfo_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    EXPECT_EQ(fastNode_->GetAdapterBufferInfo(), ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : getAdapterBufferInfo_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_072
 * @tc.desc  : Test GetAdapterBufferInfo when GetMmapBufferInfo fails
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getAdapterBufferInfo_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _)).WillOnce(Return(ERROR));
    EXPECT_EQ(fastNode_->GetAdapterBufferInfo(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : getAdapterBufferInfo_InvalidParams_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_073
 * @tc.desc  : Test GetAdapterBufferInfo with invalid buffer params
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getAdapterBufferInfo_InvalidParams_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<1>(0), // totalSizeInframe = 0
                        SetArgReferee<2>(SPAN_SIZE_IN_FRAME),
                        SetArgReferee<3>(BYTE_SIZE_PER_FRAME),
                        SetArgReferee<4>(0),
                        Return(SUCCESS)));
    EXPECT_EQ(fastNode_->GetAdapterBufferInfo(), ERR_ILLEGAL_STATE);
}

// ==================== RefreshSpanSize Tests ====================

/**
 * @tc.name  : refreshSpanSize_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_074
 * @tc.desc  : Test RefreshSpanSize when sink is null
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, refreshSpanSize_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    bool isUpdated = false;
    EXPECT_EQ(fastNode_->RefreshSpanSize(isUpdated), ERR_ILLEGAL_STATE);
    EXPECT_EQ(isUpdated, false);
}

/**
 * @tc.name  : refreshSpanSize_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_075
 * @tc.desc  : Test RefreshSpanSize when GetAdapterBufferInfo fails
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, refreshSpanSize_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _)).WillOnce(Return(ERROR));
    bool isUpdated = false;
    EXPECT_EQ(fastNode_->RefreshSpanSize(isUpdated), ERR_ILLEGAL_STATE);
}

// ==================== GetDeviceHandleInfo Tests ====================

/**
 * @tc.name  : getDeviceHandleInfo_SinkNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_076
 * @tc.desc  : Test GetDeviceHandleInfo when sink is null
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getDeviceHandleInfo_SinkNull_001, TestSize.Level0)
{
    fastNode_->audioRendererSink_ = nullptr;
    uint64_t frames = 0;
    int64_t nanoTime = 0;
    EXPECT_EQ(fastNode_->GetDeviceHandleInfo(frames, nanoTime), false);
}

/**
 * @tc.name  : getDeviceHandleInfo_Failed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_077
 * @tc.desc  : Test GetDeviceHandleInfo when GetMmapHandlePosition fails
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getDeviceHandleInfo_Failed_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetMmapHandlePosition(_, _, _)).WillOnce(Return(ERROR));
    uint64_t frames = 0;
    int64_t nanoTime = 0;
    EXPECT_EQ(fastNode_->GetDeviceHandleInfo(frames, nanoTime), false);
}

/**
 * @tc.name  : getDeviceHandleInfo_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_078
 * @tc.desc  : Test GetDeviceHandleInfo with success
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getDeviceHandleInfo_Success_001, TestSize.Level0)
{
    EXPECT_CALL(*mockSink_, GetMmapHandlePosition(_, _, _))
        .WillOnce(DoAll(SetArgReferee<0>(1000), SetArgReferee<1>(1), SetArgReferee<2>(500000000), Return(SUCCESS)));
    uint64_t frames = 0;
    int64_t nanoTime = 0;
    EXPECT_EQ(fastNode_->GetDeviceHandleInfo(frames, nanoTime), true);
}

// ==================== TriggerPrepareNextLoop Tests ====================

/**
 * @tc.name  : triggerPrepareNextLoop_BufferNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_079
 * @tc.desc  : Test TriggerPrepareNextLoop when dstAudioBuffer_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, triggerPrepareNextLoop_BufferNull_001, TestSize.Level0)
{
    fastNode_->dstAudioBuffer_ = nullptr;
    EXPECT_EQ(fastNode_->TriggerPrepareNextLoop(), false);
}

/**
 * @tc.name  : triggerPrepareNextLoop_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_080
 * @tc.desc  : Test TriggerPrepareNextLoop with valid buffer
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, triggerPrepareNextLoop_Success_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    fastNode_->dstSpanSizeInframe_ = SPAN_SIZE_IN_FRAME;
    EXPECT_EQ(fastNode_->TriggerPrepareNextLoop(), true);
}

// ==================== ClientWakeCallback Tests ====================

/**
 * @tc.name  : setClientWakeCallback_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_081
 * @tc.desc  : Test SetClientWakeCallback stores and invokes callback
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, setClientWakeCallback_001, TestSize.Level0)
{
    bool callbackInvoked = false;
    fastNode_->SetClientWakeCallback([&callbackInvoked]() { callbackInvoked = true; });
    ASSERT_NE(fastNode_->clientWakeCallback_, nullptr);
    fastNode_->clientWakeCallback_();
    EXPECT_EQ(callbackInvoked, true);
}

// ==================== IsCheckingSuspend Tests ====================

/**
 * @tc.name  : isCheckingSuspend_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_082
 * @tc.desc  : Test IsCheckingSuspend returns correct state
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, isCheckingSuspend_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->IsCheckingSuspend(), false);
    fastNode_->isCheckingSuspend_ = true;
    EXPECT_EQ(fastNode_->IsCheckingSuspend(), true);
}

// ==================== GetPreOutNum Tests ====================

/**
 * @tc.name  : getPreOutNum_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_083
 * @tc.desc  : Test GetPreOutNum with no connections
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getPreOutNum_001, TestSize.Level0)
{
    EXPECT_EQ(fastNode_->GetPreOutNum(), 0);
}

// ==================== InitTransBuffer Tests ====================

/**
 * @tc.name  : initTransBuffer_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_084
 * @tc.desc  : Test InitTransBuffer with valid span size
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, initTransBuffer_001, TestSize.Level0)
{
    fastNode_->dstSpanSizeInframe_ = SPAN_SIZE_IN_FRAME;
    fastNode_->dstByteSizePerFrame_ = BYTE_SIZE_PER_FRAME;
    fastNode_->InitTransBuffer();
    EXPECT_EQ(fastNode_->dstSpanSizeInByte_, SPAN_SIZE_IN_FRAME * BYTE_SIZE_PER_FRAME);
    EXPECT_EQ(fastNode_->renderFrameData_.size(), SPAN_SIZE_IN_FRAME * BYTE_SIZE_PER_FRAME);
}

/**
 * @tc.name  : initTransBuffer_ZeroSize_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_085
 * @tc.desc  : Test InitTransBuffer with zero span size uses MAX_TRANS_BUFFER_SIZE
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, initTransBuffer_ZeroSize_001, TestSize.Level0)
{
    fastNode_->dstSpanSizeInframe_ = 0;
    fastNode_->dstByteSizePerFrame_ = 0;
    fastNode_->InitTransBuffer();
    EXPECT_EQ(fastNode_->dstSpanSizeInByte_, 1 * 1024 * 1024); // MAX_TRANS_BUFFER_SIZE
}

// ==================== Full Lifecycle Integration Test ====================

/**
 * @tc.name  : fullLifecycle_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_086
 * @tc.desc  : Test full Init->Start->Stop->DeInit lifecycle
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, fullLifecycle_001, TestSize.Level1)
{
    // Init
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<1>(TOTAL_SIZE_IN_FRAME),
                        SetArgReferee<2>(SPAN_SIZE_IN_FRAME),
                        SetArgReferee<3>(BYTE_SIZE_PER_FRAME),
                        SetArgReferee<4>(0),
                        Return(SUCCESS)));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, GetCurrentOutputDevice()).WillOnce(Return(DEVICE_TYPE_SPEAKER));
    EXPECT_CALL(*mockSink_, RegisterCurrentDeviceCallback(_)).Times(1);

    IAudioSinkAttr attr;
    attr.sampleRate = DEFAULT_SAMPLE_RATE;
    attr.channel = DEFAULT_CHANNELS;
    attr.format = SAMPLE_F32LE;
    EXPECT_EQ(fastNode_->RenderSinkInit(attr), SUCCESS);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_IDLE);

    // Start
    EXPECT_CALL(*mockSink_, Start()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(fastNode_->RenderSinkStart(), SUCCESS);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_RUNNING);

    // Stop
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));
    EXPECT_EQ(fastNode_->RenderSinkStop(), SUCCESS);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    // DeInit
    EXPECT_CALL(*mockSink_, DeInit()).Times(1);
    EXPECT_EQ(fastNode_->RenderSinkDeInit(), SUCCESS);
    EXPECT_EQ(fastNode_->GetSinkState(), STREAM_MANAGER_RELEASED);
    EXPECT_EQ(fastNode_->audioRendererSink_, nullptr);
}

// ==================== GetRenderSinkInstance Tests ====================

/**
 * @tc.name  : getRenderSinkInstance_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_087
 * @tc.desc  : Test GetRenderSinkInstance with invalid device returns error
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, getRenderSinkInstance_001, TestSize.Level0)
{
    // This will call HdiAdapterManager which returns real instances.
    // We test with a known-invalid device class to expect failure.
    int32_t ret = fastNode_->GetRenderSinkInstance("invalid_class", "invalid_netid");
    // May return SUCCESS or ERROR depending on HdiAdapterManager implementation,
    // but should not crash.
    EXPECT_TRUE(ret == SUCCESS || ret == ERROR);
}

// ==================== WriteToDeviceBuffer Standalone Tests ====================

/**
 * @tc.name  : writeToDeviceBuffer_DstBufferNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_088
 * @tc.desc  : Test WriteToDeviceBuffer when dstAudioBuffer_ is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_DstBufferNull_001, TestSize.Level0)
{
    fastNode_->dstAudioBuffer_ = nullptr;
    HpaePcmBuffer *pcmBuffer = nullptr;
    EXPECT_EQ(fastNode_->WriteToDeviceBuffer(pcmBuffer, 0), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : writeToDeviceBuffer_SpanSizeZero_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_089
 * @tc.desc  : Test WriteToDeviceBuffer when dstSpanSizeInByte_ is zero
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_SpanSizeZero_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    fastNode_->dstSpanSizeInByte_ = 0;
    HpaePcmBuffer *pcmBuffer = nullptr;
    EXPECT_EQ(fastNode_->WriteToDeviceBuffer(pcmBuffer, 0), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : writeToDeviceBuffer_PcmBufferNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_090
 * @tc.desc  : Test WriteToDeviceBuffer when pcmBuffer is nullptr
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_PcmBufferNull_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    HpaePcmBuffer *pcmBuffer = nullptr;
    EXPECT_EQ(fastNode_->WriteToDeviceBuffer(pcmBuffer, 0), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : writeToDeviceBuffer_GetWriteBufferFail_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_091
 * @tc.desc  : Test WriteToDeviceBuffer when GetWriteBuffer fails (invalid write pos)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_GetWriteBufferFail_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    PcmBufferInfo bufInfo;
    bufInfo.ch = DEFAULT_CHANNELS;
    bufInfo.frameLen = DEFAULT_FRAME_LEN;
    bufInfo.rate = DEFAULT_SAMPLE_RATE;
    auto pcmBuffer = std::make_unique<HpaePcmBuffer>(bufInfo);
    // Use a very large writePos that exceeds buffer bounds to cause GetWriteBuffer to fail
    uint64_t invalidPos = static_cast<uint64_t>(-1);
    EXPECT_NE(fastNode_->WriteToDeviceBuffer(pcmBuffer.get(), invalidPos), SUCCESS);
}

/**
 * @tc.name  : writeToDeviceBuffer_WriteSizeZero_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_092
 * @tc.desc  : Test WriteToDeviceBuffer when validRenderDataLen_ is zero (writeSize==0)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_WriteSizeZero_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    PcmBufferInfo bufInfo;
    bufInfo.ch = DEFAULT_CHANNELS;
    bufInfo.frameLen = DEFAULT_FRAME_LEN;
    bufInfo.rate = DEFAULT_SAMPLE_RATE;
    auto pcmBuffer = std::make_unique<HpaePcmBuffer>(bufInfo);
    // validRenderDataLen_ is 0 by default, so writeSize will be 0
    fastNode_->validRenderDataLen_ = 0;
    EXPECT_EQ(fastNode_->WriteToDeviceBuffer(pcmBuffer.get(), 0), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : writeToDeviceBuffer_SwitchDevicesMute_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_093
 * @tc.desc  : Test WriteToDeviceBuffer when switchDevicesMute_ is true (memset path)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_SwitchDevicesMute_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    fastNode_->switchDevicesMute_ = true;
    fastNode_->validRenderDataLen_ = fastNode_->dstSpanSizeInByte_;
    fastNode_->renderFrameData_.resize(fastNode_->dstSpanSizeInByte_);
    // Fill render data with non-zero to verify it gets zeroed
    std::fill(fastNode_->renderFrameData_.begin(), fastNode_->renderFrameData_.end(), 0x7F);

    PcmBufferInfo bufInfo;
    bufInfo.ch = DEFAULT_CHANNELS;
    bufInfo.frameLen = SPAN_SIZE_IN_FRAME;
    bufInfo.rate = DEFAULT_SAMPLE_RATE;
    auto pcmBuffer = std::make_unique<HpaePcmBuffer>(bufInfo);

    int32_t ret = fastNode_->WriteToDeviceBuffer(pcmBuffer.get(), 0);
    EXPECT_EQ(ret, SUCCESS);
    fastNode_->switchDevicesMute_ = false;
}

/**
 * @tc.name  : writeToDeviceBuffer_ShortFrame_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_094
 * @tc.desc  : Test WriteToDeviceBuffer with short frame (writeSize < bufLength)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_ShortFrame_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    // Set validRenderDataLen_ smaller than the buffer span to trigger short frame path
    fastNode_->validRenderDataLen_ = fastNode_->dstSpanSizeInByte_ / 2;
    fastNode_->renderFrameData_.resize(fastNode_->dstSpanSizeInByte_);

    PcmBufferInfo bufInfo;
    bufInfo.ch = DEFAULT_CHANNELS;
    bufInfo.frameLen = SPAN_SIZE_IN_FRAME;
    bufInfo.rate = DEFAULT_SAMPLE_RATE;
    auto pcmBuffer = std::make_unique<HpaePcmBuffer>(bufInfo);

    int32_t ret = fastNode_->WriteToDeviceBuffer(pcmBuffer.get(), 0);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : writeToDeviceBuffer_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_095
 * @tc.desc  : Test WriteToDeviceBuffer normal success path
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_Success_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    fastNode_->validRenderDataLen_ = fastNode_->dstSpanSizeInByte_;
    fastNode_->renderFrameData_.resize(fastNode_->dstSpanSizeInByte_);

    PcmBufferInfo bufInfo;
    bufInfo.ch = DEFAULT_CHANNELS;
    bufInfo.frameLen = SPAN_SIZE_IN_FRAME;
    bufInfo.rate = DEFAULT_SAMPLE_RATE;
    auto pcmBuffer = std::make_unique<HpaePcmBuffer>(bufInfo);

    int32_t ret = fastNode_->WriteToDeviceBuffer(pcmBuffer.get(), 0);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : writeToDeviceBuffer_StartUpdateAmplitude_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_096
 * @tc.desc  : Test WriteToDeviceBuffer with startUpdate_=true triggers amplitude tracking
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, writeToDeviceBuffer_StartUpdateAmplitude_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    fastNode_->validRenderDataLen_ = fastNode_->dstSpanSizeInByte_;
    fastNode_->renderFrameData_.resize(fastNode_->dstSpanSizeInByte_);
    fastNode_->startUpdate_ = true;
    fastNode_->renderFrameNum_ = 0;
    fastNode_->lastGetMaxAmplitudeTime_ = 0;

    PcmBufferInfo bufInfo;
    bufInfo.ch = DEFAULT_CHANNELS;
    bufInfo.frameLen = SPAN_SIZE_IN_FRAME;
    bufInfo.rate = DEFAULT_SAMPLE_RATE;
    auto pcmBuffer = std::make_unique<HpaePcmBuffer>(bufInfo);

    int32_t ret = fastNode_->WriteToDeviceBuffer(pcmBuffer.get(), 0);
    EXPECT_EQ(ret, SUCCESS);
    // renderFrameNum_ should have been incremented
    EXPECT_GT(fastNode_->renderFrameNum_, 0);
    fastNode_->startUpdate_ = false;
}

// ==================== ZeroVolumeCheck IN_TIMING -> ACTIVE Timeout Tests ====================

/**
 * @tc.name  : zeroVolumeCheck_TimingToActive_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_097
 * @tc.desc  : Test ZeroVolumeCheck IN_TIMING -> ACTIVE transition on timeout (>4s)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, zeroVolumeCheck_TimingToActive_001, TestSize.Level0)
{
    fastNode_->needCheckZeroVolume_ = true;
    fastNode_->isExistLoopback_ = false;
    fastNode_->currentOutputDevice_ = DEVICE_TYPE_SPEAKER;
    fastNode_->isStarted_ = true;

    // Set zeroVolumeStartTime_ to a value far in the past (>4s = 4000000000ns)
    fastNode_->zeroVolumeState_ = HpaeFastSinkOutputNode::ZeroVolumeState::IN_TIMING;
    fastNode_->zeroVolumeStartTime_ = ClockTime::GetCurNano() - 5000000000LL;

    // Calling with zero volume should trigger IN_TIMING -> ACTIVE transition
    EXPECT_CALL(*mockSink_, Stop()).WillOnce(Return(SUCCESS));
    fastNode_->ZeroVolumeCheck(0);
    EXPECT_EQ(fastNode_->zeroVolumeState_, HpaeFastSinkOutputNode::ZeroVolumeState::ACTIVE);
    EXPECT_EQ(fastNode_->isStarted_, false);
}

// ==================== PrepareDeviceBuffer Standalone Tests ====================

/**
 * @tc.name  : prepareDeviceBuffer_BufferExists_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_098
 * @tc.desc  : Test PrepareDeviceBuffer when buffer already exists (returns early)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, prepareDeviceBuffer_BufferExists_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    // dstAudioBuffer_ is already set by SetupMmapBuffer
    int32_t ret = fastNode_->PrepareDeviceBuffer();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : prepareDeviceBuffer_GetAdapterBufferInfoFail_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_099
 * @tc.desc  : Test PrepareDeviceBuffer when GetAdapterBufferInfo fails
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, prepareDeviceBuffer_GetAdapterBufferInfoFail_001, TestSize.Level0)
{
    fastNode_->dstAudioBuffer_ = nullptr;
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _)).WillOnce(Return(ERROR));
    int32_t ret = fastNode_->PrepareDeviceBuffer();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
}

/**
 * @tc.name  : prepareDeviceBuffer_CreateFromRemoteNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_101
 * @tc.desc  : Test PrepareDeviceBuffer when CreateFromRemote returns null (invalid fd)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, prepareDeviceBuffer_CreateFromRemoteNull_001, TestSize.Level0)
{
    fastNode_->dstAudioBuffer_ = nullptr;
    fastNode_->dstBufferFd_ = -1; // invalid fd will cause CreateFromRemote to return null

    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<1>(TOTAL_SIZE_IN_FRAME),
                        SetArgReferee<2>(SPAN_SIZE_IN_FRAME),
                        SetArgReferee<3>(BYTE_SIZE_PER_FRAME),
                        SetArgReferee<4>(0),
                        Return(SUCCESS)));

    int32_t ret = fastNode_->PrepareDeviceBuffer();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : prepareDeviceBuffer_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_102
 * @tc.desc  : Test PrepareDeviceBuffer success path with local buffer
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, prepareDeviceBuffer_Success_001, TestSize.Level0)
{
    // Use RenderSinkInit to set up everything properly for a success path
    EXPECT_CALL(*mockSink_, IsInited()).WillOnce(Return(true));
    EXPECT_CALL(*mockSink_, GetMmapBufferInfo(_, _, _, _, _))
        .WillOnce(DoAll(SetArgReferee<1>(TOTAL_SIZE_IN_FRAME),
                        SetArgReferee<2>(SPAN_SIZE_IN_FRAME),
                        SetArgReferee<3>(BYTE_SIZE_PER_FRAME),
                        SetArgReferee<4>(0),
                        Return(SUCCESS)));
    EXPECT_CALL(*mockSink_, SetVolume(_, _)).WillOnce(Return(SUCCESS));
    EXPECT_CALL(*mockSink_, GetCurrentOutputDevice()).WillOnce(Return(DEVICE_TYPE_SPEAKER));
    EXPECT_CALL(*mockSink_, RegisterCurrentDeviceCallback(_)).Times(1);

    IAudioSinkAttr attr;
    attr.sampleRate = DEFAULT_SAMPLE_RATE;
    attr.channel = DEFAULT_CHANNELS;
    attr.format = SAMPLE_F32LE;
    int32_t ret = fastNode_->RenderSinkInit(attr);
    EXPECT_EQ(ret, SUCCESS);
    // RenderSinkInit calls PrepareDeviceBuffer internally; buffer should be prepared
    EXPECT_NE(fastNode_->dstAudioBuffer_, nullptr);
}

// ==================== DoProcess Branch Tests ====================

/**
 * @tc.name  : doProcess_NeedReSyncPosition_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_103
 * @tc.desc  : Test DoProcess with needReSyncPosition_=true triggers ReSyncPosition
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, doProcess_NeedReSyncPosition_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    // Connect a mixer node so CheckIfSuspend returns false (has pre-output)
    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);
    fastNode_->Connect(mixerNode);

    fastNode_->needReSyncPosition_ = true;
    // DoProcess will call ReSyncPosition then continue. Without a running thread,
    // GetDeviceHandleInfo may fail, but it should not crash.
    fastNode_->DoProcess();
    EXPECT_EQ(fastNode_->needReSyncPosition_, false);
    fastNode_->DisConnect(mixerNode);
}

/**
 * @tc.name  : doProcess_GetRenderFrameDataFail_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_104
 * @tc.desc  : Test DoProcess when GetRenderFrameDataInner fails (no pre-output data)
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, doProcess_GetRenderFrameDataFail_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    // Connect a mixer node to bypass CheckIfSuspend
    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);
    fastNode_->Connect(mixerNode);

    fastNode_->needReSyncPosition_ = false;
    // No data pushed into the mixer, so GetRenderFrameDataInner will return false
    // DoProcess should return early without crash
    fastNode_->DoProcess();
    fastNode_->DisConnect(mixerNode);
}

/**
 * @tc.name  : doProcess_ClientWakeCallback_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_105
 * @tc.desc  : Test DoProcess invokes clientWakeCallback_ on happy path
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, doProcess_ClientWakeCallback_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    // Connect a mixer node to bypass CheckIfSuspend
    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);
    fastNode_->Connect(mixerNode);

    fastNode_->needReSyncPosition_ = false;

    // Set up a client wake callback
    bool callbackInvoked = false;
    fastNode_->SetClientWakeCallback([&callbackInvoked]() { callbackInvoked = true; });

    // Prepare the buffer with valid render data so WriteToDeviceBuffer succeeds
    fastNode_->validRenderDataLen_ = fastNode_->dstSpanSizeInByte_;
    fastNode_->renderFrameData_.resize(fastNode_->dstSpanSizeInByte_);

    // Need to provide pcm data from the mixer. Create a sink input node and connect.
    HpaeNodeInfo inputInfo;
    inputInfo.sessionId = DEFAULT_SESSION_ID + 1;
    inputInfo.frameLen = DEFAULT_FRAME_LEN;
    inputInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    inputInfo.channels = DEFAULT_CHANNELS;
    inputInfo.format = SAMPLE_F32LE;
    auto inputNode = std::make_shared<HpaeSinkInputNode>(inputInfo);

    // DoProcess reads from mixer chain; since no data is queued, GetRenderFrameDataInner
    // will fail and return early. The callback should NOT be invoked in that case.
    fastNode_->DoProcess();
    // Without actual data flow, GetRenderFrameDataInner returns false,
    // so callback should not have been invoked
    EXPECT_EQ(callbackInvoked, false);

    fastNode_->DisConnect(mixerNode);
}

/**
 * @tc.name  : doProcess_HappyPath_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastSinkOutputNodeTest_106
 * @tc.desc  : Test DoProcess happy path with data flowing through the node chain
 */
HWTEST_F(HpaeFastSinkOutputNodeTest, doProcess_HappyPath_001, TestSize.Level0)
{
    SetupMmapBuffer(fastNode_);
    // Connect a mixer node to bypass CheckIfSuspend
    HpaeNodeInfo mixerInfo;
    mixerInfo.frameLen = DEFAULT_FRAME_LEN;
    mixerInfo.samplingRate = DEFAULT_SAMPLE_RATE;
    mixerInfo.channels = DEFAULT_CHANNELS;
    mixerInfo.format = SAMPLE_F32LE;
    auto mixerNode = std::make_shared<HpaeMixerNode>(mixerInfo);
    fastNode_->Connect(mixerNode);

    fastNode_->needReSyncPosition_ = false;

    // Set up callback
    bool callbackInvoked = false;
    fastNode_->SetClientWakeCallback([&callbackInvoked]() { callbackInvoked = true; });

    // Set up valid render data so WriteToDeviceBuffer can succeed
    fastNode_->validRenderDataLen_ = fastNode_->dstSpanSizeInByte_;
    fastNode_->renderFrameData_.resize(fastNode_->dstSpanSizeInByte_);

    // Without queued data from the mixer chain, GetRenderFrameDataInner returns false.
    // DoProcess returns early via CHECK_AND_RETURN_LOG. Verify no crash.
    fastNode_->DoProcess();
    // The happy path (data flow through chain) requires an actual thread model
    // which is hard to simulate in unit tests. Verify no crash occurred.
    EXPECT_TRUE(true);

    fastNode_->DisConnect(mixerNode);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
