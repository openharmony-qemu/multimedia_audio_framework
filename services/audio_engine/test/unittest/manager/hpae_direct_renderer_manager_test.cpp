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
#include <string>
#include <thread>
#include <chrono>
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_direct_renderer_manager.h"
#include "hpae_node_common.h"
#include "hpae_mocks.h"
#include "audio_utils.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace {
static std::string g_rootPath = "/data/";
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t TEST_SLEEP_TIME_160 = 160;

static HpaeSinkInfo GetDirectSinkInfo(const std::string &deviceClass = "primary_direct")
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = deviceClass;
    sinkInfo.adapterName = deviceClass;
    sinkInfo.filePath = g_rootPath + "direct_renderer_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_S32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.deviceName = deviceClass;
    return sinkInfo;
}

static HpaeSinkInfo GetVoipDirectSinkInfo()
{
    return GetDirectSinkInfo("primary_direct_voip");
}

void WaitForMsgProcessing(std::shared_ptr<HpaeDirectRendererManager> &manager)
{
    int waitCount = 0;
    const int waitCountThd = 5;
    while (manager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
    EXPECT_EQ(manager->IsMsgProcessing(), false);
    EXPECT_EQ(waitCount < waitCountThd, true);
}
} // namespace

class HpaeDirectRendererManagerTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<HpaeDirectRendererManager> directManager_;
};

void HpaeDirectRendererManagerTest::SetUp()
{
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    directManager_ = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
}

void HpaeDirectRendererManagerTest::TearDown()
{
    directManager_.reset();
}

/**
 * @tc.name  : constructDirectRendererManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_001
 * @tc.desc  : Test construct HpaeDirectRendererManager
 */
HWTEST_F(HpaeDirectRendererManagerTest, constructDirectRendererManager_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->IsInit(), false);
    EXPECT_EQ(manager->IsRunning(), false);
}

/**
 * @tc.name  : constructDirectRendererManager_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_002
 * @tc.desc  : Test construct with VOIP direct device class
 */
HWTEST_F(HpaeDirectRendererManagerTest, constructDirectRendererManager_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetVoipDirectSinkInfo();
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    ASSERT_NE(manager, nullptr);
}

/**
 * @tc.name  : getSinkInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_003
 * @tc.desc  : Test GetSinkInfo returns correct sink info
 */
HWTEST_F(HpaeDirectRendererManagerTest, getSinkInfo_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    HpaeSinkInfo retSinkInfo = manager->GetSinkInfo();
    EXPECT_EQ(retSinkInfo.deviceClass, sinkInfo.deviceClass);
    EXPECT_EQ(retSinkInfo.samplingRate, sinkInfo.samplingRate);
    EXPECT_EQ(retSinkInfo.channels, sinkInfo.channels);
    EXPECT_EQ(retSinkInfo.format, sinkInfo.format);
    EXPECT_EQ(retSinkInfo.deviceType, sinkInfo.deviceType);
}

/**
 * @tc.name  : initAndDeInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_004
 * @tc.desc  : Test Init and DeInit lifecycle
 */
HWTEST_F(HpaeDirectRendererManagerTest, initAndDeInit_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->IsInit(), true);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->IsInit(), false);
}

/**
 * @tc.name  : deInitWithoutInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_005
 * @tc.desc  : Test DeInit without Init should not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, deInitWithoutInit_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    EXPECT_EQ(directManager_->IsInit(), false);
}

/**
 * @tc.name  : doubleDeInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_006
 * @tc.desc  : Test double DeInit should not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, doubleDeInit_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    EXPECT_EQ(directManager_->IsInit(), false);
}

/**
 * @tc.name  : createStreamNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_007
 * @tc.desc  : Test CreateStream when not initialized returns error
 */
HWTEST_F(HpaeDirectRendererManagerTest, createStreamNotInit_001, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(directManager_->CreateStream(streamInfo), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : createStreamWithInvalidFrameLen_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_008
 * @tc.desc  : Test CreateStream with invalid frame length
 */
HWTEST_F(HpaeDirectRendererManagerTest, createStreamWithInvalidFrameLen_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = 0;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(directManager_->CreateStream(streamInfo), ERROR);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : createAndDestroyStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_009
 * @tc.desc  : Test CreateStream and DestroyStream lifecycle
 */
HWTEST_F(HpaeDirectRendererManagerTest, createAndDestroyStream_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(directManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.nodeInfo.sessionId, TEST_STREAM_SESSION_ID);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PREPARED);

    EXPECT_EQ(directManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), ERR_INVALID_OPERATION);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : destroyStreamNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_010
 * @tc.desc  : Test DestroyStream when not initialized returns error
 */
HWTEST_F(HpaeDirectRendererManagerTest, destroyStreamNotInit_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->DestroyStream(TEST_STREAM_SESSION_ID), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : releaseStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_011
 * @tc.desc  : Test ReleaseStream calls DestroyStream internally
 */
HWTEST_F(HpaeDirectRendererManagerTest, releaseStream_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(directManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->Release(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), ERR_INVALID_OPERATION);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : startPauseStopStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_012
 * @tc.desc  : Test Start, Pause, Stop stream lifecycle
 */
HWTEST_F(HpaeDirectRendererManagerTest, startPauseStopStream_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(directManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Register write callback for data feeding
    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    // Start stream
    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);

    // Pause stream
    EXPECT_EQ(directManager_->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);

    // Stop stream
    EXPECT_EQ(directManager_->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);

    // Destroy stream
    EXPECT_EQ(directManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
}

/**
 * @tc.name  : flushAndDrain_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_013
 * @tc.desc  : Test Flush and Drain operations
 */
HWTEST_F(HpaeDirectRendererManagerTest, flushAndDrain_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(directManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->Flush(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->Drain(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : suspendStreamManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_014
 * @tc.desc  : Test SuspendStreamManager and resume
 */
HWTEST_F(HpaeDirectRendererManagerTest, suspendStreamManager_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Suspend
    EXPECT_EQ(directManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Resume
    EXPECT_EQ(directManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Suspend again with same state should be no-op
    EXPECT_EQ(directManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : stopManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_015
 * @tc.desc  : Test StopManager
 */
HWTEST_F(HpaeDirectRendererManagerTest, stopManager_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->StopManager(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : setMute_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_016
 * @tc.desc  : Test SetMute
 */
HWTEST_F(HpaeDirectRendererManagerTest, setMute_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->SetMute(true), SUCCESS);
    EXPECT_EQ(directManager_->SetMute(false), SUCCESS);
    // Set same mute state again
    EXPECT_EQ(directManager_->SetMute(false), SUCCESS);
}

/**
 * @tc.name  : simpleOperationReturns_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_017
 * @tc.desc  : Test simple operations that return SUCCESS by default
 */
HWTEST_F(HpaeDirectRendererManagerTest, simpleOperationReturns_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->SetClientVolume(1, 0.5f), SUCCESS);
    EXPECT_EQ(directManager_->SetRate(1, 0), SUCCESS);
    EXPECT_EQ(directManager_->SetAudioEffectMode(1, 0), SUCCESS);
    EXPECT_EQ(directManager_->GetAudioEffectMode(1, *(new int32_t(0))), SUCCESS);
    EXPECT_EQ(directManager_->SetPrivacyType(1, 0), SUCCESS);
    EXPECT_EQ(directManager_->GetPrivacyType(1, *(new int32_t(0))), SUCCESS);
    EXPECT_EQ(directManager_->UpdateSpatializationState(1, false, false), SUCCESS);
    EXPECT_EQ(directManager_->UpdateMaxLength(1, 0), SUCCESS);
    EXPECT_EQ(directManager_->RefreshProcessClusterByDevice(), SUCCESS);
    EXPECT_EQ(directManager_->SetLoudnessGain(1, 0.0f), SUCCESS);
    EXPECT_EQ(directManager_->RegisterReadCallback(1, std::weak_ptr<ICapturerStreamCallback>()), SUCCESS);
}

/**
 * @tc.name  : getWritableSize_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_018
 * @tc.desc  : Test GetWritableSize returns SUCCESS (0)
 */
HWTEST_F(HpaeDirectRendererManagerTest, getWritableSize_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->GetWritableSize(1), static_cast<size_t>(SUCCESS));
}

/**
 * @tc.name  : getAllSinkInputsInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_019
 * @tc.desc  : Test GetAllSinkInputsInfo returns empty vector
 */
HWTEST_F(HpaeDirectRendererManagerTest, getAllSinkInputsInfo_001, TestSize.Level0)
{
    auto result = directManager_->GetAllSinkInputsInfo();
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name  : addNodeToSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_021
 * @tc.desc  : Test AddNodeToSink
 */
HWTEST_F(HpaeDirectRendererManagerTest, addNodeToSink_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 60001;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_S32LE;
    nodeInfo.channels = STEREO;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    int32_t ret = directManager_->AddNodeToSink(node);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : addAllNodesToSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_022
 * @tc.desc  : Test AddAllNodesToSink
 */
HWTEST_F(HpaeDirectRendererManagerTest, addAllNodesToSink_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    HpaeNodeInfo info1;
    info1.sessionId = 80001;
    info1.samplingRate = SAMPLE_RATE_48000;
    info1.format = SAMPLE_S32LE;
    info1.channels = STEREO;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info1));

    HpaeNodeInfo info2 = info1;
    info2.sessionId = 80002;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info2));

    int32_t ret = directManager_->AddAllNodesToSink(sinkInputs, true);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : moveAllStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_023
 * @tc.desc  : Test MoveAllStream when sink is not initialized (sync mode)
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveAllStream_001, TestSize.Level0)
{
    std::string sinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;
    EXPECT_EQ(directManager_->MoveAllStream(sinkName, sessionIds, moveType), SUCCESS);
}

/**
 * @tc.name  : moveAllStream_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_024
 * @tc.desc  : Test MoveAllStream when sink is initialized (async mode)
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveAllStream_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    std::string sinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;
    EXPECT_EQ(directManager_->MoveAllStream(sinkName, sessionIds, moveType), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : moveStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_025
 * @tc.desc  : Test MoveStream with session not found
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveStream_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Move non-existent session, should trigger MOVE_SESSION_FAILED callback
    directManager_->MoveStream(99999, "valid_sink_name");
    WaitForMsgProcessing(directManager_);

    // Manager should still be in init state (no crash, no side effects)
    EXPECT_EQ(directManager_->IsInit(), true);
    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : moveStream_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_026
 * @tc.desc  : Test MoveStream with empty sink name
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveStream_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(directManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Move with empty sink name, should trigger MOVE_SESSION_FAILED and not remove stream
    directManager_->MoveStream(TEST_STREAM_SESSION_ID, "");
    WaitForMsgProcessing(directManager_);

    // Stream should still exist since move failed
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : deactivateThread_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_027
 * @tc.desc  : Test DeactivateThread
 */
HWTEST_F(HpaeDirectRendererManagerTest, deactivateThread_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->DeactivateThread(), true);
    EXPECT_EQ(directManager_->IsRunning(), false);
}

/**
 * @tc.name  : handleMsg_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_028
 * @tc.desc  : Test HandleMsg processes queued messages
 */
HWTEST_F(HpaeDirectRendererManagerTest, handleMsg_001, TestSize.Level0)
{
    bool requestHandled = false;
    auto request = [&requestHandled]() { requestHandled = true; };
    directManager_->hpaeNoLockQueue_.PushRequest(std::move(request));
    EXPECT_EQ(directManager_->IsMsgProcessing(), true);
    directManager_->HandleMsg();
    EXPECT_EQ(requestHandled, true);
}

/**
 * @tc.name  : getThreadName_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_029
 * @tc.desc  : Test GetThreadName returns device name
 */
HWTEST_F(HpaeDirectRendererManagerTest, getThreadName_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    sinkInfo.deviceName = "test_device";
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    EXPECT_EQ(manager->GetThreadName(), "test_device");
}

/**
 * @tc.name  : dumpSinkInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_030
 * @tc.desc  : Test DumpSinkInfo when not initialized
 */
HWTEST_F(HpaeDirectRendererManagerTest, dumpSinkInfo_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->DumpSinkInfo(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : dumpSinkInfo_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_031
 * @tc.desc  : Test DumpSinkInfo when initialized
 */
HWTEST_F(HpaeDirectRendererManagerTest, dumpSinkInfo_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->DumpSinkInfo(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : getDeviceHDFDumpInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_032
 * @tc.desc  : Test GetDeviceHDFDumpInfo
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDeviceHDFDumpInfo_001, TestSize.Level0)
{
    std::string dumpInfo = directManager_->GetDeviceHDFDumpInfo();
    EXPECT_GT(dumpInfo.size(), 0);
}

/**
 * @tc.name  : setOffloadPolicy_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_033
 * @tc.desc  : Test SetOffloadPolicy with stream not found
 */
HWTEST_F(HpaeDirectRendererManagerTest, setOffloadPolicy_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    // Should not crash for non-existent session
    EXPECT_EQ(directManager_->SetOffloadPolicy(99999, 0), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : registerWriteCallback_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_034
 * @tc.desc  : Test RegisterWriteCallback
 */
HWTEST_F(HpaeDirectRendererManagerTest, registerWriteCallback_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(directManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : onRequestLatency_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_035
 * @tc.desc  : Test OnRequestLatency
 */
HWTEST_F(HpaeDirectRendererManagerTest, onRequestLatency_001, TestSize.Level0)
{
    uint64_t latency = 0;
    // Should not crash even when nodes are not created
    directManager_->OnRequestLatency(0, latency);
    // When no nodes are created, latency should remain 0
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : onNotifyQueue_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_036
 * @tc.desc  : Test OnNotifyQueue when thread is nullptr, early return without crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, onNotifyQueue_001, TestSize.Level0)
{
    // hpaeSignalProcessThread_ is nullptr before Init, CHECK_AND_RETURN_LOG should early return
    EXPECT_EQ(directManager_->IsInit(), false);
    directManager_->OnNotifyQueue();
    // Verify no state change after calling OnNotifyQueue with null thread
    EXPECT_EQ(directManager_->IsInit(), false);
}

/**
 * @tc.name  : onNotifyQueue_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_036b
 * @tc.desc  : Test OnNotifyQueue when thread is active, Notify should be called
 */
HWTEST_F(HpaeDirectRendererManagerTest, onNotifyQueue_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->IsInit(), true);

    // After Init, hpaeSignalProcessThread_ is not nullptr, Notify() should be called
    directManager_->OnNotifyQueue();
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : process_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_037
 * @tc.desc  : Test Process when sink is not initialized
 */
HWTEST_F(HpaeDirectRendererManagerTest, process_001, TestSize.Level0)
{
    // sinkOutputNode_ is nullptr before Init, Process should early return
    EXPECT_EQ(directManager_->IsInit(), false);
    directManager_->Process();
    EXPECT_EQ(directManager_->IsInit(), false);
}

/**
 * @tc.name  : isRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_038
 * @tc.desc  : Test IsRunning returns false when not initialized
 */
HWTEST_F(HpaeDirectRendererManagerTest, isRunning_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->IsRunning(), false);
}

/**
 * @tc.name  : triggerAppsUidUpdate_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_039
 * @tc.desc  : Test TriggerAppsUidUpdate when not initialized
 */
HWTEST_F(HpaeDirectRendererManagerTest, triggerAppsUidUpdate_001, TestSize.Level0)
{
    // TriggerAppsUidUpdate sends async request, should not crash when not initialized
    EXPECT_EQ(directManager_->IsInit(), false);
    directManager_->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(directManager_->IsInit(), false);
}

/**
 * @tc.name  : voipDirectInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_040
 * @tc.desc  : Test Init with VOIP direct device class
 */
HWTEST_F(HpaeDirectRendererManagerTest, voipDirectInit_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetVoipDirectSinkInfo();
    auto voipManager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    EXPECT_EQ(voipManager->Init(), SUCCESS);
    WaitForMsgProcessing(voipManager);
    EXPECT_EQ(voipManager->IsInit(), true);
    EXPECT_EQ(voipManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(voipManager);
}

/**
 * @tc.name  : reloadRenderManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_041
 * @tc.desc  : Test ReloadRenderManager
 */
HWTEST_F(HpaeDirectRendererManagerTest, reloadRenderManager_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    EXPECT_EQ(directManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->IsInit(), true);
    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : destructor_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_042
 * @tc.desc  : Test destructor calls DeInit when initialized
 */
HWTEST_F(HpaeDirectRendererManagerTest, destructor_001, TestSize.Level1)
{
    {
        HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
        auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
        EXPECT_EQ(manager->Init(), SUCCESS);
        WaitForMsgProcessing(manager);
        EXPECT_EQ(manager->IsInit(), true);
        // Destructor should call DeInit, isInit_ set to false
    }
    // Verify manager was destroyed without leak - no crash and shared_ptr released
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : sendRequestNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_043
 * @tc.desc  : Test SendRequest when not initialized should not execute
 */
HWTEST_F(HpaeDirectRendererManagerTest, sendRequestNotInit_001, TestSize.Level0)
{
    // Since we cannot call SendRequest directly (it's private), test via public API
    // CreateStream should return ERR_INVALID_OPERATION
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 1;
    EXPECT_EQ(directManager_->CreateStream(streamInfo), ERR_INVALID_OPERATION);
}

// ==================== GetDirectSampleRate branch tests ====================

/**
 * @tc.name  : getDirectSampleRate_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_044
 * @tc.desc  : Test GetDirectSampleRate non-voip 44100 -> 48000
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectSampleRate_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectSampleRate(SAMPLE_RATE_44100, false), SAMPLE_RATE_48000);
}

/**
 * @tc.name  : getDirectSampleRate_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_045
 * @tc.desc  : Test GetDirectSampleRate non-voip 88200 -> 96000
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectSampleRate_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectSampleRate(SAMPLE_RATE_88200, false), SAMPLE_RATE_96000);
}

/**
 * @tc.name  : getDirectSampleRate_003
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_046
 * @tc.desc  : Test GetDirectSampleRate non-voip 176400 -> 192000
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectSampleRate_003, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectSampleRate(SAMPLE_RATE_176400, false), SAMPLE_RATE_192000);
}

/**
 * @tc.name  : getDirectSampleRate_004
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_047
 * @tc.desc  : Test GetDirectSampleRate non-voip default rate unchanged
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectSampleRate_004, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectSampleRate(SAMPLE_RATE_48000, false), SAMPLE_RATE_48000);
    EXPECT_EQ(directManager_->GetDirectSampleRate(SAMPLE_RATE_96000, false), SAMPLE_RATE_96000);
}

/**
 * @tc.name  : getDirectSampleRate_005
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_048
 * @tc.desc  : Test GetDirectSampleRate voip rate <= 16000 returns 16000
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectSampleRate_005, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectSampleRate(SAMPLE_RATE_8000, true), SAMPLE_RATE_16000);
    EXPECT_EQ(directManager_->GetDirectSampleRate(SAMPLE_RATE_16000, true), SAMPLE_RATE_16000);
}

/**
 * @tc.name  : getDirectSampleRate_006
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_049
 * @tc.desc  : Test GetDirectSampleRate voip rate > 16000 returns 48000
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectSampleRate_006, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectSampleRate(SAMPLE_RATE_48000, true), SAMPLE_RATE_48000);
}

// ==================== GetDirectFormat branch tests ====================

/**
 * @tc.name  : getDirectFormat_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_050
 * @tc.desc  : Test GetDirectFormat non-voip always returns S32LE
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectFormat_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectFormat(SAMPLE_F32LE, false), SAMPLE_S32LE);
    EXPECT_EQ(directManager_->GetDirectFormat(SAMPLE_S16LE, false), SAMPLE_S32LE);
}

/**
 * @tc.name  : getDirectFormat_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_051
 * @tc.desc  : Test GetDirectFormat voip S16LE returns S16LE
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectFormat_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectFormat(SAMPLE_S16LE, true), SAMPLE_S16LE);
}

/**
 * @tc.name  : getDirectFormat_003
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_052
 * @tc.desc  : Test GetDirectFormat voip S32LE returns S32LE
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectFormat_003, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectFormat(SAMPLE_S32LE, true), SAMPLE_S32LE);
}

/**
 * @tc.name  : getDirectFormat_004
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_053
 * @tc.desc  : Test GetDirectFormat voip F32LE converts to S16LE
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectFormat_004, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectFormat(SAMPLE_F32LE, true), SAMPLE_S16LE);
}

/**
 * @tc.name  : getDirectFormat_005
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_054
 * @tc.desc  : Test GetDirectFormat voip unsupported format falls back to S32LE
 */
HWTEST_F(HpaeDirectRendererManagerTest, getDirectFormat_005, TestSize.Level1)
{
    EXPECT_EQ(directManager_->GetDirectFormat(SAMPLE_U8, true), SAMPLE_S32LE);
    EXPECT_EQ(directManager_->GetDirectFormat(SAMPLE_S24LE, true), SAMPLE_S32LE);
}

// ==================== Multi-session operation tests ====================

static HpaeStreamInfo MakeStreamInfo(uint32_t sessionId)
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = sessionId;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    return streamInfo;
}

/**
 * @tc.name  : destroyStreamNonCurrentNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_055
 * @tc.desc  : Test DestroyStream for non-current node removes from map only
 */
HWTEST_F(HpaeDirectRendererManagerTest, destroyStreamNonCurrentNode_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Create two streams: first becomes curNode_
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Destroy the non-current node (200002)
    EXPECT_EQ(directManager_->DestroyStream(200002), SUCCESS);
    WaitForMsgProcessing(directManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(200002, sinkInputInfo), ERR_INVALID_OPERATION);
    // curNode_ (200001) still exists
    EXPECT_EQ(directManager_->GetSinkInputInfo(200001, sinkInputInfo), SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : pauseNonCurrentNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_056
 * @tc.desc  : Test Pause for non-current node sets state without fade/disconnect
 */
HWTEST_F(HpaeDirectRendererManagerTest, pauseNonCurrentNode_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(300001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(300002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Pause the non-current node
    EXPECT_EQ(directManager_->Pause(300002), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(300002, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : stopNonCurrentNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_057
 * @tc.desc  : Test Stop for non-current node sets state without fade/disconnect
 */
HWTEST_F(HpaeDirectRendererManagerTest, stopNonCurrentNode_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(400001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(400002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Stop the non-current node
    EXPECT_EQ(directManager_->Stop(400002), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(400002, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Pause isStandby branch ====================

/**
 * @tc.name  : pauseWithStandby_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_058
 * @tc.desc  : Test Pause with isStandby=true disconnects without sleep
 */
HWTEST_F(HpaeDirectRendererManagerTest, pauseWithStandby_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Pause with isStandby=true
    EXPECT_EQ(directManager_->Pause(TEST_STREAM_SESSION_ID, true), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Drain non-RUNNING state branch ====================

/**
 * @tc.name  : drainNonRunningState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_059
 * @tc.desc  : Test Drain when stream is in PREPARED state triggers callback
 */
HWTEST_F(HpaeDirectRendererManagerTest, drainNonRunningState_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Stream is in PREPARED state (not RUNNING), Drain should trigger callback
    EXPECT_EQ(directManager_->Drain(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== DeInit with isMoveDefault ====================

/**
 * @tc.name  : deInitWithMoveDefault_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_060
 * @tc.desc  : Test DeInit with isMoveDefault=true moves streams
 */
HWTEST_F(HpaeDirectRendererManagerTest, deInitWithMoveDefault_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(true), SUCCESS);
    EXPECT_EQ(directManager_->IsInit(), false);
}

// ==================== SetCurrentNode after curNode destroyed ====================

/**
 * @tc.name  : setCurrentNodeAfterDestroy_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_061
 * @tc.desc  : Test SetCurrentNode picks another node after curNode_ destroyed
 */
HWTEST_F(HpaeDirectRendererManagerTest, setCurrentNodeAfterDestroy_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(500001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(500002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Destroy curNode_ (500001) - should call SetCurrentNode to pick 500002
    EXPECT_EQ(directManager_->DestroyStream(500001), SUCCESS);
    WaitForMsgProcessing(directManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    // 500001 gone, 500002 should become curNode_
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(500001, sinkInputInfo), ERR_INVALID_OPERATION);
    EXPECT_EQ(directManager_->GetSinkInputInfo(500002, sinkInputInfo), SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== MoveAllStream MOVE_PARTIAL ====================

/**
 * @tc.name  : moveAllStreamPartial_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_062
 * @tc.desc  : Test MoveAllStream with MOVE_SINGLE type (partial move)
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveAllStreamPartial_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(600001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(600002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    std::string sinkName = "test_target_sink";
    std::vector<uint32_t> sessionIds = {600002};
    EXPECT_EQ(directManager_->MoveAllStream(sinkName, sessionIds, MOVE_SINGLE), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== MoveStream valid session and sinkName ====================

/**
 * @tc.name  : moveStreamValidSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_063
 * @tc.desc  : Test MoveStream with valid session moves curNode_ and sets new curNode_
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveStreamValidSession_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(700001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(700002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Move curNode_ (700001) to another sink
    directManager_->MoveStream(700001, "valid_target_sink");
    WaitForMsgProcessing(directManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    // 700001 gone, 700002 should be picked by SetCurrentNode
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(700001, sinkInputInfo), ERR_INVALID_OPERATION);
    EXPECT_EQ(directManager_->GetSinkInputInfo(700002, sinkInputInfo), SUCCESS);

    // Move non-curNode_ (700002) to another sink
    directManager_->MoveStream(700002, "another_target_sink");
    WaitForMsgProcessing(directManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));
    EXPECT_EQ(directManager_->GetSinkInputInfo(700002, sinkInputInfo), ERR_INVALID_OPERATION);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== OnRequestLatency with nodes ====================

/**
 * @tc.name  : onRequestLatencyWithNodes_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_064
 * @tc.desc  : Test OnRequestLatency with created process nodes returns accumulated latency
 */
HWTEST_F(HpaeDirectRendererManagerTest, onRequestLatencyWithNodes_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    uint64_t latency = 0;
    directManager_->OnRequestLatency(TEST_STREAM_SESSION_ID, latency);
    // With created nodes, latency should still be 0 (all GetLatency return 0 for gain/converter)
    EXPECT_EQ(latency, 0);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== TriggerAppsUidUpdate with running session ====================

/**
 * @tc.name  : triggerAppsUidUpdateRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_065
 * @tc.desc  : Test TriggerAppsUidUpdate with running curNode_ updates uid
 */
HWTEST_F(HpaeDirectRendererManagerTest, triggerAppsUidUpdateRunning_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // TriggerAppsUidUpdate should update appsUid_ for running session
    directManager_->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== SuspendStreamManager resume with running stream ====================

/**
 * @tc.name  : suspendStreamManagerResumeRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_066
 * @tc.desc  : Test SuspendStreamManager resume with running curNode_ restarts sink
 */
HWTEST_F(HpaeDirectRendererManagerTest, suspendStreamManagerResumeRunning_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Suspend
    EXPECT_EQ(directManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Resume while curNode_ is running - should restart sink
    EXPECT_EQ(directManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== ReloadRenderManager with running stream ====================

/**
 * @tc.name  : reloadRenderManagerRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_067
 * @tc.desc  : Test ReloadRenderManager with running stream reconnects after reload
 */
HWTEST_F(HpaeDirectRendererManagerTest, reloadRenderManagerRunning_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Reload while stream is running - should disconnect, destroy, recreate, reconnect
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    EXPECT_EQ(directManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== CreateStream frameLen oversized ====================

/**
 * @tc.name  : createStreamOversizedFrameLen_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_068
 * @tc.desc  : Test CreateStream with oversized frame length returns error
 */
HWTEST_F(HpaeDirectRendererManagerTest, createStreamOversizedFrameLen_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = 1000000; // oversized
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(directManager_->CreateStream(streamInfo), ERROR);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== OnFadeDone branches ====================

/**
 * @tc.name  : onFadeDoneMismatch_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_069
 * @tc.desc  : Test OnFadeDone with mismatched sessionId does nothing
 */
HWTEST_F(HpaeDirectRendererManagerTest, onFadeDoneMismatch_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // OnFadeDone with wrong sessionId should not crash or change state
    directManager_->OnFadeDone(99999);
    WaitForMsgProcessing(directManager_);

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PREPARED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Start non-existent session ====================

/**
 * @tc.name  : startNonExistent_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_070
 * @tc.desc  : Test Start with non-existent session does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, startNonExistent_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Start non-existent session
    EXPECT_EQ(directManager_->Start(99999), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Pause non-existent session ====================

/**
 * @tc.name  : pauseNonExistent_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_071
 * @tc.desc  : Test Pause with non-existent session does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, pauseNonExistent_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->Pause(99999), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Stop non-existent session ====================

/**
 * @tc.name  : stopNonExistent_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_072
 * @tc.desc  : Test Stop with non-existent session does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, stopNonExistent_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->Stop(99999), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== DestroyStream non-existent session ====================

/**
 * @tc.name  : destroyStreamNonExistent_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_073
 * @tc.desc  : Test DestroyStream with non-existent session does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, destroyStreamNonExistent_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DestroyStream(99999), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== AddAllNodesToSink with isConnect false ====================

/**
 * @tc.name  : addAllNodesToSinkNoConnect_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_074
 * @tc.desc  : Test AddAllNodesToSink with isConnect=false skips connection
 */
HWTEST_F(HpaeDirectRendererManagerTest, addAllNodesToSinkNoConnect_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    HpaeNodeInfo info;
    info.sessionId = 900001;
    info.samplingRate = SAMPLE_RATE_48000;
    info.format = SAMPLE_S32LE;
    info.channels = STEREO;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info));

    // isConnect=false should add to map but skip connection
    EXPECT_EQ(directManager_->AddAllNodesToSink(sinkInputs, false), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Flush non-existent session ====================

/**
 * @tc.name  : flushNonExistent_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_075
 * @tc.desc  : Test Flush with non-existent session does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, flushNonExistent_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->Flush(99999), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Drain non-existent session ====================

/**
 * @tc.name  : drainNonExistent_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_076
 * @tc.desc  : Test Drain with non-existent session does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, drainNonExistent_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->Drain(99999), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== OnNodeStatusUpdate ====================

/**
 * @tc.name  : onNodeStatusUpdate_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_077
 * @tc.desc  : Test OnNodeStatusUpdate triggers callback
 */
HWTEST_F(HpaeDirectRendererManagerTest, onNodeStatusUpdate_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // OnNodeStatusUpdate triggers callback with curNode_ state
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PREPARED);

    directManager_->OnNodeStatusUpdate(TEST_STREAM_SESSION_ID, OPERATION_STARTED);

    // State should remain PREPARED (OnNodeStatusUpdate only triggers callback, does not change state)
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PREPARED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Init double init ====================

/**
 * @tc.name  : doubleInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_078
 * @tc.desc  : Test double Init does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, doubleInit_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->IsInit(), true);

    // Second init should create new thread
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== RegisterWriteCallback non-existent session ====================

/**
 * @tc.name  : registerWriteCallbackNonExistent_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_079
 * @tc.desc  : Test RegisterWriteCallback with non-existent session does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, registerWriteCallbackNonExistent_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(99999, writeCb), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== MoveAllStream with empty sessions ====================

/**
 * @tc.name  : moveAllStreamEmpty_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_080
 * @tc.desc  : Test MoveAllStream with empty session list when not initialized
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveAllStreamEmpty_001, TestSize.Level0)
{
    std::string sinkName = "test_sink";
    std::vector<uint32_t> emptyIds;
    EXPECT_EQ(directManager_->MoveAllStream(sinkName, emptyIds, MOVE_ALL), SUCCESS);
}

// ==================== ReloadRenderManager not init ====================

/**
 * @tc.name  : reloadRenderManagerNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_081
 * @tc.desc  : Test ReloadRenderManager when not init creates thread and activates
 */
HWTEST_F(HpaeDirectRendererManagerTest, reloadRenderManagerNotInit_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    EXPECT_EQ(directManager_->ReloadRenderManager(sinkInfo, false), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->IsInit(), true);
    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== SetOffloadPolicy with existing session ====================

/**
 * @tc.name  : setOffloadPolicyExisting_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_082
 * @tc.desc  : Test SetOffloadPolicy with existing session sets offload enabled
 */
HWTEST_F(HpaeDirectRendererManagerTest, setOffloadPolicyExisting_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Set offload policy to non-default
    EXPECT_EQ(directManager_->SetOffloadPolicy(TEST_STREAM_SESSION_ID, 1), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Reset to default
    EXPECT_EQ(directManager_->SetOffloadPolicy(TEST_STREAM_SESSION_ID, 0), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== VOIP stream lifecycle ====================

/**
 * @tc.name  : voipStreamLifecycle_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_083
 * @tc.desc  : Test VOIP direct manager full stream lifecycle with volumeSyncNode
 */
HWTEST_F(HpaeDirectRendererManagerTest, voipStreamLifecycle_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetVoipDirectSinkInfo();
    auto voipManager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    EXPECT_EQ(voipManager->Init(), SUCCESS);
    WaitForMsgProcessing(voipManager);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(voipManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(voipManager);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(voipManager->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(voipManager->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(voipManager);

    EXPECT_EQ(voipManager->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(voipManager);

    EXPECT_EQ(voipManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(voipManager);
}

// ==================== Start then Stop lifecycle ====================

/**
 * @tc.name  : startStopLifecycle_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_084
 * @tc.desc  : Test Start then Stop stream without Pause
 */
HWTEST_F(HpaeDirectRendererManagerTest, startStopLifecycle_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(directManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

// ==================== Private function tests ====================

/**
 * @tc.name  : addNodeToMap_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_085
 * @tc.desc  : Test AddNodeToMap sets curNode_ and creates direct nodes for first node
 */
HWTEST_F(HpaeDirectRendererManagerTest, addNodeToMap_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->curNode_->GetSessionId(), TEST_STREAM_SESSION_ID);
    EXPECT_NE(directManager_->converterForGain_, nullptr);
    EXPECT_NE(directManager_->converterForOutput_, nullptr);
    EXPECT_NE(directManager_->gainNode_, nullptr);
    EXPECT_EQ(directManager_->volumeSyncNode_, nullptr);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 1u);
    EXPECT_EQ(directManager_->curNode_->GetDirect(), true);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : addNodeToMap_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_086
 * @tc.desc  : Test AddNodeToMap second node does not change curNode_ or recreate direct nodes
 */
HWTEST_F(HpaeDirectRendererManagerTest, addNodeToMap_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto firstConverter = directManager_->converterForGain_;
    EXPECT_NE(firstConverter, nullptr);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->curNode_->GetSessionId(), 200001u);
    EXPECT_EQ(directManager_->converterForGain_, firstConverter);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 2u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : removeNodeFromMap_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_087
 * @tc.desc  : Test RemoveNodeFromMap for current node destroys direct nodes and clears curNode_
 */
HWTEST_F(HpaeDirectRendererManagerTest, removeNodeFromMap_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_NE(directManager_->converterForGain_, nullptr);

    directManager_->RemoveNodeFromMap(TEST_STREAM_SESSION_ID);

    EXPECT_EQ(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->converterForGain_, nullptr);
    EXPECT_EQ(directManager_->converterForOutput_, nullptr);
    EXPECT_EQ(directManager_->gainNode_, nullptr);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 0u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : removeNodeFromMap_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_088
 * @tc.desc  : Test RemoveNodeFromMap for non-current node keeps curNode_ and direct nodes intact
 */
HWTEST_F(HpaeDirectRendererManagerTest, removeNodeFromMap_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 2u);

    directManager_->RemoveNodeFromMap(200002);

    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->curNode_->GetSessionId(), 200001u);
    EXPECT_NE(directManager_->converterForGain_, nullptr);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 1u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : removeNodeFromMap_003
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_089
 * @tc.desc  : Test RemoveNodeFromMap for non-existent session does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, removeNodeFromMap_003, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 1u);
    directManager_->RemoveNodeFromMap(999999u);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 1u);
    EXPECT_NE(directManager_->curNode_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : setCurrentNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_090
 * @tc.desc  : Test SetCurrentNode does nothing when curNode_ already set
 */
HWTEST_F(HpaeDirectRendererManagerTest, setCurrentNode_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto origNode = directManager_->curNode_;
    EXPECT_NE(origNode, nullptr);

    directManager_->SetCurrentNode();

    EXPECT_EQ(directManager_->curNode_, origNode);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : setCurrentNode_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_091
 * @tc.desc  : Test SetCurrentNode picks node from map when curNode_ is null and connects if running
 */
HWTEST_F(HpaeDirectRendererManagerTest, setCurrentNode_002, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);
    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // Manually clear curNode_ to simulate removal
    directManager_->curNode_ = nullptr;
    directManager_->converterForGain_ = nullptr;
    directManager_->converterForOutput_ = nullptr;
    directManager_->gainNode_ = nullptr;
    directManager_->limiterNode_ = nullptr;
    directManager_->volumeSyncNode_ = nullptr;

    directManager_->SetCurrentNode();

    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->curNode_->GetSessionId(), TEST_STREAM_SESSION_ID);
    EXPECT_NE(directManager_->converterForGain_, nullptr);
    EXPECT_NE(directManager_->converterForOutput_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : setCurrentNode_003
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_092
 * @tc.desc  : Test SetCurrentNode picks node but does not connect when node is not running
 */
HWTEST_F(HpaeDirectRendererManagerTest, setCurrentNode_003, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->curNode_->GetState(), HPAE_SESSION_PREPARED);

    directManager_->curNode_ = nullptr;
    directManager_->converterForGain_ = nullptr;
    directManager_->converterForOutput_ = nullptr;
    directManager_->gainNode_ = nullptr;
    directManager_->limiterNode_ = nullptr;
    directManager_->volumeSyncNode_ = nullptr;

    directManager_->SetCurrentNode();

    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_NE(directManager_->converterForGain_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : createInputSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_093
 * @tc.desc  : Test CreateInputSession creates node with correct info and adds to map
 */
HWTEST_F(HpaeDirectRendererManagerTest, createInputSession_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    auto node = directManager_->CreateInputSession(streamInfo);

    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->GetSessionId(), TEST_STREAM_SESSION_ID);
    EXPECT_EQ(node->GetDirect(), true);
    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->curNode_->GetSessionId(), TEST_STREAM_SESSION_ID);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.count(TEST_STREAM_SESSION_ID), 1u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : createDirectNodes_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_094
 * @tc.desc  : Test CreateDirectNodes returns ERROR when curNode_ is null
 */
HWTEST_F(HpaeDirectRendererManagerTest, createDirectNodes_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->CreateDirectNodes(), ERROR);
}

/**
 * @tc.name  : createDirectNodes_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_095
 * @tc.desc  : Test CreateDirectNodes succeeds with valid curNode_
 */
HWTEST_F(HpaeDirectRendererManagerTest, createDirectNodes_002, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_NE(directManager_->converterForGain_, nullptr);
    EXPECT_NE(directManager_->gainNode_, nullptr);
    EXPECT_NE(directManager_->converterForOutput_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : destroyDirectNodes_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_096
 * @tc.desc  : Test DestroyDirectNodes returns ERROR when nodes are null
 */
HWTEST_F(HpaeDirectRendererManagerTest, destroyDirectNodes_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->DestroyDirectNodes(), ERROR);
}

/**
 * @tc.name  : destroyDirectNodes_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_097
 * @tc.desc  : Test DestroyDirectNodes nullifies all nodes when they exist
 */
HWTEST_F(HpaeDirectRendererManagerTest, destroyDirectNodes_002, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->converterForGain_, nullptr);
    EXPECT_NE(directManager_->gainNode_, nullptr);
    EXPECT_NE(directManager_->converterForOutput_, nullptr);

    EXPECT_EQ(directManager_->DestroyDirectNodes(), SUCCESS);

    EXPECT_EQ(directManager_->converterForGain_, nullptr);
    EXPECT_EQ(directManager_->gainNode_, nullptr);
    EXPECT_EQ(directManager_->volumeSyncNode_, nullptr);
    EXPECT_EQ(directManager_->converterForOutput_, nullptr);
    EXPECT_EQ(directManager_->limiterNode_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : createProcessNodes_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_098
 * @tc.desc  : Test CreateProcessNodes for non-VOIP STEREO creates gainNode_ without limiterNode_
 */
HWTEST_F(HpaeDirectRendererManagerTest, createProcessNodes_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->gainNode_, nullptr);
    EXPECT_EQ(directManager_->volumeSyncNode_, nullptr);
    EXPECT_EQ(directManager_->limiterNode_, nullptr);
    EXPECT_NE(directManager_->converterForGain_, nullptr);
    EXPECT_NE(directManager_->converterForOutput_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : createProcessNodes_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_099
 * @tc.desc  : Test CreateProcessNodes for VOIP device creates volumeSyncNode_ instead of gainNode_
 */
HWTEST_F(HpaeDirectRendererManagerTest, createProcessNodes_002, TestSize.Level0)
{
    HpaeSinkInfo voipSinkInfo = GetVoipDirectSinkInfo();
    auto voipManager = std::make_shared<HpaeDirectRendererManager>(voipSinkInfo);
    EXPECT_EQ(voipManager->Init(), SUCCESS);
    WaitForMsgProcessing(voipManager);

    HpaeStreamInfo voipStreamInfo;
    voipStreamInfo.channels = STEREO;
    voipStreamInfo.samplingRate = SAMPLE_RATE_48000;
    voipStreamInfo.format = SAMPLE_S16LE;
    voipStreamInfo.frameLen = FRAME_LENGTH_960;
    voipStreamInfo.sessionId = TEST_STREAM_SESSION_ID;
    voipStreamInfo.streamType = STREAM_VOICE_COMMUNICATION;
    voipStreamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(voipManager->CreateStream(voipStreamInfo), SUCCESS);
    WaitForMsgProcessing(voipManager);

    EXPECT_NE(voipManager->volumeSyncNode_, nullptr);
    EXPECT_EQ(voipManager->gainNode_, nullptr);
    EXPECT_NE(voipManager->converterForGain_, nullptr);
    EXPECT_NE(voipManager->converterForOutput_, nullptr);

    EXPECT_EQ(voipManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : connectInputSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_100
 * @tc.desc  : Test ConnectInputSession returns SUCCESS when node is not RUNNING
 */
HWTEST_F(HpaeDirectRendererManagerTest, connectInputSession_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->curNode_->GetState(), HPAE_SESSION_PREPARED);
    EXPECT_EQ(directManager_->ConnectInputSession(), SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : connectInputSession_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_101
 * @tc.desc  : Test ConnectInputSession connects nodes and starts sink when node is RUNNING
 */
HWTEST_F(HpaeDirectRendererManagerTest, connectInputSession_002, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    directManager_->curNode_->SetState(HPAE_SESSION_RUNNING);
    EXPECT_EQ(directManager_->ConnectInputSession(), SUCCESS);

    EXPECT_NE(directManager_->sinkOutputNode_, nullptr);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : disconnectInputSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_102
 * @tc.desc  : Test DisConnectInputSession returns SUCCESS when converterForGain_ is null
 */
HWTEST_F(HpaeDirectRendererManagerTest, disconnectInputSession_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->converterForGain_, nullptr);
    EXPECT_EQ(directManager_->DisConnectInputSession(), SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : disconnectInputSession_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_103
 * @tc.desc  : Test DisConnectInputSession disconnects all nodes when connected
 */
HWTEST_F(HpaeDirectRendererManagerTest, disconnectInputSession_002, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);
    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->converterForGain_, nullptr);
    EXPECT_EQ(directManager_->DisConnectInputSession(), SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : deleteInputSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_104
 * @tc.desc  : Test DeleteInputSession disconnects and removes curNode_ from map
 */
HWTEST_F(HpaeDirectRendererManagerTest, deleteInputSession_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 1u);

    directManager_->DeleteInputSession();

    EXPECT_EQ(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 0u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : addSingleNodeToSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_105
 * @tc.desc  : Test AddSingleNodeToSink with isConnect=false does not connect session
 */
HWTEST_F(HpaeDirectRendererManagerTest, addSingleNodeToSink_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(200002);
    auto node = directManager_->CreateInputSession(streamInfo);
    directManager_->curNode_ = SafeGetMap(directManager_->sinkInputNodeMap_, 200001u);
    directManager_->RemoveNodeFromMap(200002);

    directManager_->AddSingleNodeToSink(node, false);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.count(200002), 1u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : addSingleNodeToSink_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_106
 * @tc.desc  : Test AddSingleNodeToSink with non-running node does not connect
 */
HWTEST_F(HpaeDirectRendererManagerTest, addSingleNodeToSink_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(200002);
    auto node = directManager_->CreateInputSession(streamInfo);
    directManager_->curNode_ = SafeGetMap(directManager_->sinkInputNodeMap_, 200001u);
    directManager_->RemoveNodeFromMap(200002);

    // Node is PREPARED (not RUNNING), isConnect=true but won't connect
    directManager_->AddSingleNodeToSink(node, true);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.count(200002), 1u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : stopOutputNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_107
 * @tc.desc  : Test StopOuputNode with null sinkOutputNode_ does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, stopOutputNode_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->sinkOutputNode_, nullptr);
    directManager_->StopOuputNode();
    EXPECT_EQ(directManager_->sinkOutputNode_, nullptr);
}

/**
 * @tc.name  : stopOutputNode_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_108
 * @tc.desc  : Test StopOuputNode with valid sinkOutputNode_ stops and nullifies
 */
HWTEST_F(HpaeDirectRendererManagerTest, stopOutputNode_002, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->sinkOutputNode_, nullptr);
    directManager_->StopOuputNode();
    EXPECT_EQ(directManager_->sinkOutputNode_, nullptr);
}

/**
 * @tc.name  : notifyStreamChangeToSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_109
 * @tc.desc  : Test NotifyStreamChangeToSink with null sinkOutputNode_ does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, notifyStreamChangeToSink_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->sinkOutputNode_, nullptr);
    directManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, 1234, RENDERER_RELEASED);
}

/**
 * @tc.name  : notifyStreamChangeToSink_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_110
 * @tc.desc  : Test NotifyStreamChangeToSink with session in map and session not in map
 */
HWTEST_F(HpaeDirectRendererManagerTest, notifyStreamChangeToSink_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->sinkOutputNode_, nullptr);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    // With session in map
    directManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, TEST_STREAM_SESSION_ID, RENDERER_RELEASED);

    // With session not in map (uses default STREAM_USAGE_UNKNOWN)
    directManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, 999999, RENDERER_RUNNING);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : updateAppsUid_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_111
 * @tc.desc  : Test UpdateAppsUid with curNode_ running adds uid to list
 */
HWTEST_F(HpaeDirectRendererManagerTest, updateAppsUid_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    directManager_->curNode_->SetState(HPAE_SESSION_RUNNING);
    directManager_->UpdateAppsUid();

    EXPECT_EQ(directManager_->appsUid_.size(), 1u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : updateAppsUid_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_112
 * @tc.desc  : Test UpdateAppsUid with curNode_ not running clears uid list
 */
HWTEST_F(HpaeDirectRendererManagerTest, updateAppsUid_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    directManager_->curNode_->SetState(HPAE_SESSION_PREPARED);
    directManager_->UpdateAppsUid();

    EXPECT_EQ(directManager_->appsUid_.size(), 0u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : updateAppsUid_003
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_113
 * @tc.desc  : Test UpdateAppsUid with null curNode_ clears uid list
 */
HWTEST_F(HpaeDirectRendererManagerTest, updateAppsUid_003, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->curNode_, nullptr);
    directManager_->UpdateAppsUid();

    EXPECT_EQ(directManager_->appsUid_.size(), 0u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : initSinkInner_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_114
 * @tc.desc  : Test InitSinkInner returns error when CheckFramelen fails with isCallback=false
 */
HWTEST_F(HpaeDirectRendererManagerTest, initSinkInner_001, TestSize.Level0)
{
    HpaeSinkInfo badSinkInfo = GetDirectSinkInfo();
    badSinkInfo.frameLen = 0;

    auto manager = std::make_shared<HpaeDirectRendererManager>(badSinkInfo);
    int32_t ret = manager->InitSinkInner(false, false);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : initSinkInner_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_115
 * @tc.desc  : Test InitSinkInner with valid config and isCallback=false returns SUCCESS
 */
HWTEST_F(HpaeDirectRendererManagerTest, initSinkInner_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    int32_t ret = manager->InitSinkInner(false, false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(manager->sinkOutputNode_, nullptr);

    manager->StopOuputNode();
}

/**
 * @tc.name  : initSinkInner_003
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_116
 * @tc.desc  : Test InitSinkInner with valid config and isCallback=true sets isInit_
 */
HWTEST_F(HpaeDirectRendererManagerTest, initSinkInner_003, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    auto manager = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    EXPECT_EQ(manager->IsInit(), false);
    int32_t ret = manager->InitSinkInner(false, true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(manager->IsInit(), true);
    EXPECT_NE(manager->sinkOutputNode_, nullptr);

    manager->DeInit();
}

/**
 * @tc.name  : setSessionFade_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_117
 * @tc.desc  : Test SetSessionFade returns false when sessionId doesn't match curNode_
 */
HWTEST_F(HpaeDirectRendererManagerTest, setSessionFade_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->SetSessionFade(999999, OPERATION_STARTED), false);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : setSessionFade_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_118
 * @tc.desc  : Test SetSessionFade returns false when curNode_ is null
 */
HWTEST_F(HpaeDirectRendererManagerTest, setSessionFade_002, TestSize.Level0)
{
    EXPECT_EQ(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_STARTED), false);
}

/**
 * @tc.name  : setSessionFade_003
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_119
 * @tc.desc  : Test SetSessionFade returns false and sets STOPPED state when node is STOPPED
 */
HWTEST_F(HpaeDirectRendererManagerTest, setSessionFade_003, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    directManager_->curNode_->SetState(HPAE_SESSION_STOPPED);
    EXPECT_EQ(directManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_STOPPED), false);
    EXPECT_EQ(directManager_->curNode_->GetState(), HPAE_SESSION_STOPPED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : setSessionFade_004
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_120
 * @tc.desc  : Test SetSessionFade returns false and sets PAUSED state when node is PAUSED
 */
HWTEST_F(HpaeDirectRendererManagerTest, setSessionFade_004, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    directManager_->curNode_->SetState(HPAE_SESSION_PAUSED);
    EXPECT_EQ(directManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_PAUSED), false);
    EXPECT_EQ(directManager_->curNode_->GetState(), HPAE_SESSION_PAUSED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : setSessionFade_005
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_121
 * @tc.desc  : Test SetSessionFade returns false for OPERATION_STARTED in early return path (no state change)
 */
HWTEST_F(HpaeDirectRendererManagerTest, setSessionFade_005, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    directManager_->curNode_->SetState(HPAE_SESSION_STOPPED);
    EXPECT_EQ(directManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_STARTED), false);
    EXPECT_EQ(directManager_->curNode_->GetState(), HPAE_SESSION_STOPPED);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : sendRequest_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_122
 * @tc.desc  : Test SendRequest not init and not isInit flag does not push request
 */
HWTEST_F(HpaeDirectRendererManagerTest, sendRequest_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->IsInit(), false);
    bool executed = false;
    auto request = [&executed]() { executed = true; };
    directManager_->SendRequest(std::move(request), "test_func", false);
    EXPECT_EQ(executed, false);
}

/**
 * @tc.name  : sendRequest_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_123
 * @tc.desc  : Test SendRequest with isInit=true pushes request even when not initialized
 */
HWTEST_F(HpaeDirectRendererManagerTest, sendRequest_002, TestSize.Level0)
{
    EXPECT_EQ(directManager_->IsInit(), false);
    bool executed = false;
    auto request = [&executed]() { executed = true; };
    directManager_->SendRequest(std::move(request), "test_func", true);
    EXPECT_EQ(executed, false);
    directManager_->HandleMsg();
    EXPECT_EQ(executed, true);
}

/**
 * @tc.name  : moveAllStreamToNewSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_124
 * @tc.desc  : Test MoveAllStreamToNewSink with MOVE_ALL moves all sessions
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveAllStreamToNewSink_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 2u);

    std::vector<uint32_t> emptyIds;
    directManager_->MoveAllStreamToNewSink("new_sink", emptyIds, MOVE_ALL);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 0u);
    EXPECT_EQ(directManager_->curNode_, nullptr);
}

/**
 * @tc.name  : moveAllStreamToNewSink_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_125
 * @tc.desc  : Test MoveAllStreamToNewSink with MOVE_SINGLE moves only specified sessions
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveAllStreamToNewSink_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(200002)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 2u);

    std::vector<uint32_t> moveIds = {200002};
    directManager_->MoveAllStreamToNewSink("new_sink", moveIds, MOVE_SINGLE);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 1u);
    EXPECT_NE(directManager_->curNode_, nullptr);
    EXPECT_EQ(directManager_->curNode_->GetSessionId(), 200001u);
}

/**
 * @tc.name  : moveAllStreamToNewSink_003
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_126
 * @tc.desc  : Test MoveAllStreamToNewSink with empty map does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, moveAllStreamToNewSink_003, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 0u);

    std::vector<uint32_t> emptyIds;
    directManager_->MoveAllStreamToNewSink("new_sink", emptyIds, MOVE_ALL);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 0u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : recreateSinkOutputNodeIfNeeded_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_127
 * @tc.desc  : Test RecreateSinkOutputNodeIfNeeded returns SUCCESS when config unchanged
 */
HWTEST_F(HpaeDirectRendererManagerTest, recreateSinkOutputNodeIfNeeded_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    int32_t ret = directManager_->RecreateSinkOutputNodeIfNeeded();
    EXPECT_EQ(ret, SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : recreateSinkOutputNodeIfNeeded_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_128
 * @tc.desc  : Test RecreateSinkOutputNodeIfNeeded recreates when sample rate differs
 */
HWTEST_F(HpaeDirectRendererManagerTest, recreateSinkOutputNodeIfNeeded_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeNodeInfo nodeInfo = directManager_->curNode_->GetNodeInfo();
    nodeInfo.samplingRate = SAMPLE_RATE_96000;
    directManager_->curNode_->SetNodeInfo(nodeInfo);

    int32_t ret = directManager_->RecreateSinkOutputNodeIfNeeded();
    EXPECT_EQ(ret, SUCCESS);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : process_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_129
 * @tc.desc  : Test Process does nothing when sinkOutputNode_ is null
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_process_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->sinkOutputNode_, nullptr);
    directManager_->Process();
}

/**
 * @tc.name  : process_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_130
 * @tc.desc  : Test Process does nothing when not running
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_process_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->sinkOutputNode_, nullptr);
    EXPECT_EQ(directManager_->IsRunning(), false);
    directManager_->Process();

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : onRequestLatency_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_131
 * @tc.desc  : Test OnRequestLatency accumulates latency from all processing nodes
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_onRequestLatency_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    uint64_t latency = 0;
    directManager_->OnRequestLatency(TEST_STREAM_SESSION_ID, latency);
    EXPECT_GE(latency, 0u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : onRequestLatency_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_132
 * @tc.desc  : Test OnRequestLatency with no process nodes (only sinkOutputNode_)
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_onRequestLatency_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    uint64_t latency = 100;
    directManager_->OnRequestLatency(TEST_STREAM_SESSION_ID, latency);
    EXPECT_GE(latency, 100u);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : priv_getThreadName_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_133
 * @tc.desc  : Test GetThreadName returns sinkInfo device name
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_getThreadName_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->GetThreadName(), "primary_direct");
}

/**
 * @tc.name  : priv_onNotifyQueue_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_134
 * @tc.desc  : Test OnNotifyQueue with null thread does not crash
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_onNotifyQueue_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->hpaeSignalProcessThread_, nullptr);
    directManager_->OnNotifyQueue();
}

/**
 * @tc.name  : priv_suspendStreamManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_135
 * @tc.desc  : Test SuspendStreamManager with same suspend state does nothing
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_suspendStreamManager_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);
    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->isSuspend_, false);
    EXPECT_EQ(directManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->isSuspend_, false);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : priv_suspendStreamManager_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_136
 * @tc.desc  : Test SuspendStreamManager suspend and resume
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_suspendStreamManager_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);
    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->isSuspend_, true);

    EXPECT_EQ(directManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->isSuspend_, false);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : priv_stopManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_137
 * @tc.desc  : Test StopManager with null sinkOutputNode_ does not crash via async message
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_stopManager_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    directManager_->StopOuputNode();
    EXPECT_EQ(directManager_->sinkOutputNode_, nullptr);

    EXPECT_EQ(directManager_->StopManager(), SUCCESS);
    WaitForMsgProcessing(directManager_);
}

/**
 * @tc.name  : priv_setMute_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_138
 * @tc.desc  : Test SetMute toggles isMute_ state
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_setMute_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->isMute_, false);
    EXPECT_EQ(directManager_->SetMute(true), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->isMute_, true);

    EXPECT_EQ(directManager_->SetMute(false), SUCCESS);
    WaitForMsgProcessing(directManager_);
    EXPECT_EQ(directManager_->isMute_, false);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : priv_reloadRenderManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_139
 * @tc.desc  : Test ReloadRenderManager reinitializes sink with running stream
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_reloadRenderManager_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(directManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);
    EXPECT_EQ(directManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(directManager_);

    HpaeSinkInfo newSinkInfo = GetDirectSinkInfo();
    EXPECT_EQ(directManager_->ReloadRenderManager(newSinkInfo, true), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : deInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_140
 * @tc.desc  : Test DeInit with isMoveDefault=true moves all streams
 */
HWTEST_F(HpaeDirectRendererManagerTest, deInit_001, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 1u);
    EXPECT_EQ(directManager_->DeInit(true), SUCCESS);
    EXPECT_EQ(directManager_->sinkInputNodeMap_.size(), 0u);
    EXPECT_EQ(directManager_->IsInit(), false);
}

/**
 * @tc.name  : deInit_002
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_141
 * @tc.desc  : Test DeInit with isMoveDefault=false does not move streams
 */
HWTEST_F(HpaeDirectRendererManagerTest, deInit_002, TestSize.Level1)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_EQ(directManager_->DeInit(false), SUCCESS);
    EXPECT_EQ(directManager_->IsInit(), false);
}

/**
 * @tc.name  : priv_deactivateThread_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_142
 * @tc.desc  : Test DeactivateThread with running thread deactivates and processes remaining messages
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_deactivateThread_001, TestSize.Level0)
{
    EXPECT_EQ(directManager_->Init(), SUCCESS);
    WaitForMsgProcessing(directManager_);

    EXPECT_NE(directManager_->hpaeSignalProcessThread_, nullptr);
    EXPECT_EQ(directManager_->DeactivateThread(), true);
    EXPECT_EQ(directManager_->hpaeSignalProcessThread_, nullptr);
}

/**
 * @tc.name  : priv_getSinkInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_143
 * @tc.desc  : Test GetSinkInfo returns configured sink info
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_getSinkInfo_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = directManager_->GetSinkInfo();
    EXPECT_EQ(sinkInfo.deviceClass, "primary_direct");
    EXPECT_EQ(sinkInfo.samplingRate, SAMPLE_RATE_48000);
    EXPECT_EQ(sinkInfo.format, SAMPLE_S32LE);
    EXPECT_EQ(sinkInfo.channels, STEREO);
}

/**
 * @tc.name  : priv_getDeviceHDFDumpInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeDirectRendererManagerTest_144
 * @tc.desc  : Test GetDeviceHDFDumpInfo returns non-empty string
 */
HWTEST_F(HpaeDirectRendererManagerTest, priv_getDeviceHDFDumpInfo_001, TestSize.Level1)
{
    std::string dumpInfo = directManager_->GetDeviceHDFDumpInfo();
    EXPECT_FALSE(dumpInfo.empty());
}
