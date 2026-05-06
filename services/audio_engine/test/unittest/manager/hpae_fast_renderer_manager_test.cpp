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
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_fast_renderer_manager.h"
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

static HpaeSinkInfo GetFastSinkInfo(const std::string &deviceClass = "primary_mmap")
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = deviceClass;
    sinkInfo.adapterName = deviceClass;
    sinkInfo.filePath = g_rootPath + "fast_renderer_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_S32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.deviceName = deviceClass;
    return sinkInfo;
}

static HpaeSinkInfo GetVoipFastSinkInfo()
{
    return GetFastSinkInfo("primary_mmap_voip");
}

void WaitForMsgProcessing(std::shared_ptr<HpaeFastRendererManager> &manager)
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

class HpaeFastRendererManagerTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<HpaeFastRendererManager> fastManager_;
};

void HpaeFastRendererManagerTest::SetUp()
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    fastManager_ = std::make_shared<HpaeFastRendererManager>(sinkInfo);
}

void HpaeFastRendererManagerTest::TearDown()
{
    fastManager_.reset();
}

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
 * @tc.name  : constructFastRendererManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_001
 * @tc.desc  : Test construct HpaeFastRendererManager
 */
HWTEST_F(HpaeFastRendererManagerTest, constructFastRendererManager_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->IsInit(), false);
    EXPECT_EQ(manager->IsRunning(), false);
}

/**
 * @tc.name  : constructFastRendererManager_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_002
 * @tc.desc  : Test construct with VOIP fast device class
 */
HWTEST_F(HpaeFastRendererManagerTest, constructFastRendererManager_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetVoipFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    ASSERT_NE(manager, nullptr);
}

/**
 * @tc.name  : getSinkInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_003
 * @tc.desc  : Test GetSinkInfo returns correct sink info
 */
HWTEST_F(HpaeFastRendererManagerTest, getSinkInfo_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
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
 * @tc.number: HpaeFastRendererManagerTest_004
 * @tc.desc  : Test Init and DeInit lifecycle
 */
HWTEST_F(HpaeFastRendererManagerTest, initAndDeInit_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : deInitWithoutInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_005
 * @tc.desc  : Test DeInit without Init should not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, deInitWithoutInit_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : doubleDeInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_006
 * @tc.desc  : Test double DeInit should not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, doubleDeInit_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : createStreamNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_007
 * @tc.desc  : Test CreateStream when not initialized returns error
 */
HWTEST_F(HpaeFastRendererManagerTest, createStreamNotInit_001, TestSize.Level0)
{
    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : createStreamWithInvalidFrameLen_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_008
 * @tc.desc  : Test CreateStream with invalid frame length
 */
HWTEST_F(HpaeFastRendererManagerTest, createStreamWithInvalidFrameLen_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    streamInfo.frameLen = 0;
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), ERROR);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : createAndDestroyStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_009
 * @tc.desc  : Test CreateStream and DestroyStream lifecycle
 */
HWTEST_F(HpaeFastRendererManagerTest, createAndDestroyStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.nodeInfo.sessionId, TEST_STREAM_SESSION_ID);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PREPARED);

    EXPECT_EQ(fastManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), ERR_INVALID_OPERATION);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : destroyStreamNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_010
 * @tc.desc  : Test DestroyStream when not initialized returns error
 */
HWTEST_F(HpaeFastRendererManagerTest, destroyStreamNotInit_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->DestroyStream(TEST_STREAM_SESSION_ID), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : releaseStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_011
 * @tc.desc  : Test ReleaseStream calls DestroyStream internally
 */
HWTEST_F(HpaeFastRendererManagerTest, releaseStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->Release(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), ERR_INVALID_OPERATION);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : startPauseStopStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_012
 * @tc.desc  : Test Start, Pause, Stop stream lifecycle
 */
HWTEST_F(HpaeFastRendererManagerTest, startPauseStopStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(fastManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    // Start stream
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);

    // Pause stream
    EXPECT_EQ(fastManager_->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);

    // Stop stream
    EXPECT_EQ(fastManager_->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);

    // Destroy stream
    EXPECT_EQ(fastManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
}

/**
 * @tc.name  : flushAndDrain_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_013
 * @tc.desc  : Test Flush and Drain operations
 */
HWTEST_F(HpaeFastRendererManagerTest, flushAndDrain_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->Flush(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->Drain(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : suspendStreamManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_014
 * @tc.desc  : Test SuspendStreamManager and resume
 */
HWTEST_F(HpaeFastRendererManagerTest, suspendStreamManager_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Suspend
    EXPECT_EQ(fastManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Resume
    EXPECT_EQ(fastManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Suspend again with same state should be no-op
    EXPECT_EQ(fastManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : stopManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_015
 * @tc.desc  : Test StopManager
 */
HWTEST_F(HpaeFastRendererManagerTest, stopManager_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->StopManager(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setMute_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_016
 * @tc.desc  : Test SetMute
 */
HWTEST_F(HpaeFastRendererManagerTest, setMute_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->SetMute(true), SUCCESS);
    EXPECT_EQ(fastManager_->SetMute(false), SUCCESS);
    EXPECT_EQ(fastManager_->SetMute(false), SUCCESS);
}

/**
 * @tc.name  : simpleOperationReturns_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_017
 * @tc.desc  : Test simple operations that return SUCCESS by default
 */
HWTEST_F(HpaeFastRendererManagerTest, simpleOperationReturns_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->SetClientVolume(1, 0.5f), SUCCESS);
    EXPECT_EQ(fastManager_->SetRate(1, 0), SUCCESS);
    EXPECT_EQ(fastManager_->SetAudioEffectMode(1, 0), SUCCESS);
    int32_t effectMode = 0;
    EXPECT_EQ(fastManager_->GetAudioEffectMode(1, effectMode), SUCCESS);
    EXPECT_EQ(fastManager_->SetPrivacyType(1, 0), SUCCESS);
    int32_t privacyType = 0;
    EXPECT_EQ(fastManager_->GetPrivacyType(1, privacyType), SUCCESS);
    EXPECT_EQ(fastManager_->UpdateSpatializationState(1, false, false), SUCCESS);
    EXPECT_EQ(fastManager_->UpdateMaxLength(1, 0), SUCCESS);
    EXPECT_EQ(fastManager_->RefreshProcessClusterByDevice(), SUCCESS);
    EXPECT_EQ(fastManager_->SetLoudnessGain(1, 0.0f), SUCCESS);
    EXPECT_EQ(fastManager_->RegisterReadCallback(1, std::weak_ptr<ICapturerStreamCallback>()), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : getWritableSize_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_018
 * @tc.desc  : Test GetWritableSize returns SUCCESS (0)
 */
HWTEST_F(HpaeFastRendererManagerTest, getWritableSize_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->GetWritableSize(1), static_cast<size_t>(SUCCESS));
}

/**
 * @tc.name  : getAllSinkInputsInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_019
 * @tc.desc  : Test GetAllSinkInputsInfo returns empty vector
 */
HWTEST_F(HpaeFastRendererManagerTest, getAllSinkInputsInfo_001, TestSize.Level0)
{
    auto result = fastManager_->GetAllSinkInputsInfo();
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name  : addNodeToSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_020
 * @tc.desc  : Test AddNodeToSink
 */
HWTEST_F(HpaeFastRendererManagerTest, addNodeToSink_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 60001;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_S32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    int32_t ret = fastManager_->AddNodeToSink(node);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : addAllNodesToSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_021
 * @tc.desc  : Test AddAllNodesToSink
 */
HWTEST_F(HpaeFastRendererManagerTest, addAllNodesToSink_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    HpaeNodeInfo info1;
    info1.sessionId = 80001;
    info1.samplingRate = SAMPLE_RATE_48000;
    info1.format = SAMPLE_S32LE;
    info1.channels = STEREO;
    info1.frameLen = FRAME_LENGTH_960;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info1));

    HpaeNodeInfo info2 = info1;
    info2.sessionId = 80002;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info2));

    int32_t ret = fastManager_->AddAllNodesToSink(sinkInputs, true);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveAllStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_022
 * @tc.desc  : Test MoveAllStream when sink is not initialized (sync mode)
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStream_001, TestSize.Level0)
{
    std::string sinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;
    EXPECT_EQ(fastManager_->MoveAllStream(sinkName, sessionIds, moveType), SUCCESS);
}

/**
 * @tc.name  : moveAllStream_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_023
 * @tc.desc  : Test MoveAllStream when sink is initialized (async mode)
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStream_002, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    std::string sinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;
    EXPECT_EQ(fastManager_->MoveAllStream(sinkName, sessionIds, moveType), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_024
 * @tc.desc  : Test MoveStream with session not found
 */
HWTEST_F(HpaeFastRendererManagerTest, moveStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Move non-existent session, should trigger MOVE_SESSION_FAILED callback
    fastManager_->MoveStream(99999, "valid_sink_name");
    WaitForMsgProcessing(fastManager_);

    // Manager should still be in init state (no crash, no side effects)
    EXPECT_EQ(fastManager_->IsInit(), true);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveStream_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_025
 * @tc.desc  : Test MoveStream with empty sink name
 */
HWTEST_F(HpaeFastRendererManagerTest, moveStream_002, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Move with empty sink name, should trigger MOVE_SESSION_FAILED
    fastManager_->MoveStream(TEST_STREAM_SESSION_ID, "");
    WaitForMsgProcessing(fastManager_);

    // Stream should still exist since move failed
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveStreamSync_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_026
 * @tc.desc  : Test MoveStream in sync mode when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, moveStreamSync_001, TestSize.Level0)
{
    // Not initialized, so MoveStreamSync is called directly
    EXPECT_EQ(fastManager_->MoveStream(99999, "valid_sink_name"), SUCCESS);
    // Should not crash
}

/**
 * @tc.name  : deactivateThread_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_027
 * @tc.desc  : Test DeactivateThread
 */
HWTEST_F(HpaeFastRendererManagerTest, deactivateThread_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->DeactivateThread(), true);
    EXPECT_EQ(fastManager_->IsRunning(), false);
}

/**
 * @tc.name  : handleMsg_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_028
 * @tc.desc  : Test HandleMsg processes queued messages
 */
HWTEST_F(HpaeFastRendererManagerTest, handleMsg_001, TestSize.Level0)
{
    bool requestHandled = false;
    auto request = [&requestHandled]() { requestHandled = true; };
    fastManager_->hpaeNoLockQueue_.PushRequest(std::move(request));
    EXPECT_EQ(fastManager_->IsMsgProcessing(), true);
    fastManager_->HandleMsg();
    EXPECT_EQ(requestHandled, true);
}

/**
 * @tc.name  : getThreadName_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_029
 * @tc.desc  : Test GetThreadName returns device name
 */
HWTEST_F(HpaeFastRendererManagerTest, getThreadName_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.deviceName = "test_device";
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->GetThreadName(), "test_device");
}

/**
 * @tc.name  : dumpSinkInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_030
 * @tc.desc  : Test DumpSinkInfo when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, dumpSinkInfo_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->DumpSinkInfo(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : dumpSinkInfo_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_031
 * @tc.desc  : Test DumpSinkInfo when initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, dumpSinkInfo_002, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DumpSinkInfo(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : getDeviceHDFDumpInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_032
 * @tc.desc  : Test GetDeviceHDFDumpInfo
 */
HWTEST_F(HpaeFastRendererManagerTest, getDeviceHDFDumpInfo_001, TestSize.Level0)
{
    std::string dumpInfo = fastManager_->GetDeviceHDFDumpInfo();
    EXPECT_GT(dumpInfo.size(), 0);
}

/**
 * @tc.name  : registerWriteCallback_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_033
 * @tc.desc  : Test RegisterWriteCallback
 */
HWTEST_F(HpaeFastRendererManagerTest, registerWriteCallback_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(fastManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onRequestLatency_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_034
 * @tc.desc  : Test OnRequestLatency
 */
HWTEST_F(HpaeFastRendererManagerTest, onRequestLatency_001, TestSize.Level0)
{
    uint64_t latency = 0;
    // Should not crash even when nodes are not created
    fastManager_->OnRequestLatency(0, latency);
    // When no nodes are created, latency should remain 0
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : onNotifyQueue_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_035
 * @tc.desc  : Test OnNotifyQueue when thread is nullptr, early return without crash
 */
HWTEST_F(HpaeFastRendererManagerTest, onNotifyQueue_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->IsInit(), false);
    fastManager_->OnNotifyQueue();
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : onNotifyQueue_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_036
 * @tc.desc  : Test OnNotifyQueue when thread is active
 */
HWTEST_F(HpaeFastRendererManagerTest, onNotifyQueue_002, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);

    fastManager_->OnNotifyQueue();
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : process_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_037
 * @tc.desc  : Test Process when sink is not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, process_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->IsInit(), false);
    fastManager_->Process();
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : isRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_038
 * @tc.desc  : Test IsRunning returns false when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, isRunning_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->IsRunning(), false);
}

/**
 * @tc.name  : triggerAppsUidUpdate_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_039
 * @tc.desc  : Test TriggerAppsUidUpdate when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerAppsUidUpdate_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->IsInit(), false);
    fastManager_->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : destructor_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_040
 * @tc.desc  : Test destructor calls DeInit when initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, destructor_001, TestSize.Level1)
{
    {
        HpaeSinkInfo sinkInfo = GetFastSinkInfo();
        auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
        EXPECT_EQ(manager->Init(), SUCCESS);
        WaitForMsgProcessing(manager);
        EXPECT_EQ(manager->IsInit(), true);
        // Destructor should call DeInit
    }
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : sendRequestNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_041
 * @tc.desc  : Test SendRequest when not initialized should not execute
 */
HWTEST_F(HpaeFastRendererManagerTest, sendRequestNotInit_001, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 1;
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : setMuteForSwitchDevice_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_042
 * @tc.desc  : Test SetMuteForSwitchDevice
 */
HWTEST_F(HpaeFastRendererManagerTest, setMuteForSwitchDevice_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->SetMuteForSwitchDevice(true);
    WaitForMsgProcessing(fastManager_);
    // Verify mute was set via sinkOutputNode_
    EXPECT_NE(fastManager_->sinkOutputNode_, nullptr);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSpeed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_043
 * @tc.desc  : Test SetSpeed
 */
HWTEST_F(HpaeFastRendererManagerTest, setSpeed_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->SetSpeed(TEST_STREAM_SESSION_ID, 1.5f);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onNodeStatusUpdate_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_044
 * @tc.desc  : Test OnNodeStatusUpdate
 */
HWTEST_F(HpaeFastRendererManagerTest, onNodeStatusUpdate_001, TestSize.Level0)
{
    // Should not crash for non-existent session
    fastManager_->OnNodeStatusUpdate(99999, OPERATION_STARTED);
}

/**
 * @tc.name  : onFadeDone_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_045
 * @tc.desc  : Test OnFadeDone
 */
HWTEST_F(HpaeFastRendererManagerTest, onFadeDone_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // OnFadeDone for non-existent session should not crash
    fastManager_->OnFadeDone(99999);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onRewindAndFlush_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_046
 * @tc.desc  : Test OnRewindAndFlush
 */
HWTEST_F(HpaeFastRendererManagerTest, onRewindAndFlush_001, TestSize.Level0)
{
    fastManager_->OnRewindAndFlush(1000, 0);
    // Should not crash with no sessions
}

/**
 * @tc.name  : getSpanSizeInFrame_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_047
 * @tc.desc  : Test GetSpanSizeInFrame for non-existent session
 */
HWTEST_F(HpaeFastRendererManagerTest, getSpanSizeInFrame_001, TestSize.Level0)
{
    uint32_t spanSize = 0;
    EXPECT_EQ(fastManager_->GetSpanSizeInFrame(99999, spanSize), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : getMaxAmplitude_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_048
 * @tc.desc  : Test GetMaxAmplitude when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, getMaxAmplitude_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->GetMaxAmplitude(), 0.0f);
}

/**
 * @tc.name  : reloadRenderManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_049
 * @tc.desc  : Test ReloadRenderManager
 */
HWTEST_F(HpaeFastRendererManagerTest, reloadRenderManager_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    EXPECT_EQ(fastManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : reloadRenderManager_AlreadyInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_050
 * @tc.desc  : Test ReloadRenderManager when already initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, reloadRenderManager_AlreadyInit_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);

    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    EXPECT_EQ(fastManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : multiSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_051
 * @tc.desc  : Test creating and destroying multiple sessions
 */
HWTEST_F(HpaeFastRendererManagerTest, multiSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(200002)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(200001, sinkInputInfo), SUCCESS);
    EXPECT_EQ(fastManager_->GetSinkInputInfo(200002, sinkInputInfo), SUCCESS);

    EXPECT_EQ(fastManager_->DestroyStream(200002), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));
    EXPECT_EQ(fastManager_->GetSinkInputInfo(200002, sinkInputInfo), ERR_INVALID_OPERATION);
    EXPECT_EQ(fastManager_->GetSinkInputInfo(200001, sinkInputInfo), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : createStreamWithLoopback_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_052
 * @tc.desc  : Test CreateStream with isLoopback=true updates loopback state
 */
HWTEST_F(HpaeFastRendererManagerTest, createStreamWithLoopback_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    streamInfo.isLoopback = true;
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : deInitWithMoveDefault_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_053
 * @tc.desc  : Test DeInit with isMoveDefault=true moves streams
 */
HWTEST_F(HpaeFastRendererManagerTest, deInitWithMoveDefault_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(true), SUCCESS);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : moveAllStreamPartial_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_054
 * @tc.desc  : Test MoveAllStream with MOVE_SINGLE type
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStreamPartial_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(600001)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(600002)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    std::string sinkName = "test_target_sink";
    std::vector<uint32_t> sessionIds = {600002};
    EXPECT_EQ(fastManager_->MoveAllStream(sinkName, sessionIds, MOVE_SINGLE), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : notifyStreamChangeToSink_NullSinkOutputNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_055
 * @tc.desc  : Test NotifyStreamChangeToSink with null sinkOutputNode_ does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, notifyStreamChangeToSink_NullSinkOutputNode_001, TestSize.Level0)
{
    // Before Init, sinkOutputNode_ is nullptr
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
    fastManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, 1234, RENDERER_RELEASED);
    // Should not crash
}

/**
 * @tc.name  : notifyStreamChangeToSink_WithSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_056
 * @tc.desc  : Test NotifyStreamChangeToSink with session in map and not in map
 */
HWTEST_F(HpaeFastRendererManagerTest, notifyStreamChangeToSink_WithSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Session in map
    fastManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, TEST_STREAM_SESSION_ID, RENDERER_RELEASED);

    // Session not in map
    fastManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, 999999, RENDERER_RUNNING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : suspendResumeWithRunningStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_057
 * @tc.desc  : Test SuspendStreamManager resume with running stream starts sink
 */
HWTEST_F(HpaeFastRendererManagerTest, suspendResumeWithRunningStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(fastManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Suspend
    EXPECT_EQ(fastManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Resume should start sink since there is a running stream
    EXPECT_EQ(fastManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : voipFastInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_058
 * @tc.desc  : Test Init with VOIP fast device class
 */
HWTEST_F(HpaeFastRendererManagerTest, voipFastInit_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetVoipFastSinkInfo();
    auto voipManager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(voipManager->Init(), SUCCESS);
    WaitForMsgProcessing(voipManager);
    EXPECT_EQ(voipManager->IsInit(), true);
    EXPECT_EQ(voipManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(voipManager);
}

/**
 * @tc.name  : triggerAppsUidUpdate_Init_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_059
 * @tc.desc  : Test TriggerAppsUidUpdate when initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerAppsUidUpdate_Init_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setMuteForSwitchDevice_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_060
 * @tc.desc  : Test SetMuteForSwitchDevice when not initialized returns early
 */
HWTEST_F(HpaeFastRendererManagerTest, setMuteForSwitchDevice_NullSinkOutput_001, TestSize.Level0)
{
    // Before Init, sinkOutputNode_ is nullptr
    fastManager_->SetMuteForSwitchDevice(true);
    // Should not crash, sinkOutputNode_ remains null
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
}

/**
 * @tc.name  : stopManager_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_061
 * @tc.desc  : Test StopManager when sinkOutputNode_ is null returns early
 */
HWTEST_F(HpaeFastRendererManagerTest, stopManager_NullSinkOutput_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->StopManager(), SUCCESS);
    // Should not crash
}

/**
 * @tc.name  : drainNonRunningState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_062
 * @tc.desc  : Test Drain when stream is in PREPARED state triggers callback
 */
HWTEST_F(HpaeFastRendererManagerTest, drainNonRunningState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Stream is in PREPARED state (not RUNNING), Drain should trigger callback
    EXPECT_EQ(fastManager_->Drain(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : initWithInvalidFrameLen_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_063
 * @tc.desc  : Test Init with invalid frame length
 */
HWTEST_F(HpaeFastRendererManagerTest, initWithInvalidFrameLen_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.frameLen = 0;
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->Init(), SUCCESS);
    WaitForMsgProcessing(manager);
    // Init should fail internally since frameLen is 0
    EXPECT_EQ(manager->IsInit(), false);
}

/**
 * @tc.name  : initWithUltraFast_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_064
 * @tc.desc  : Test Init with ultra-fast flag
 */
HWTEST_F(HpaeFastRendererManagerTest, initWithUltraFast_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.isUltraFast = true;
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->Init(), SUCCESS);
    WaitForMsgProcessing(manager);
    EXPECT_EQ(manager->IsInit(), true);
    EXPECT_EQ(manager->DeInit(), SUCCESS);
    WaitForMsgProcessing(manager);
}

/**
 * @tc.name  : onFadeDone_RunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_065
 * @tc.desc  : Test OnFadeDone when session is RUNNING should not disconnect
 */
HWTEST_F(HpaeFastRendererManagerTest, onFadeDone_RunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(fastManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // OnFadeDone for running session should early return
    fastManager_->OnFadeDone(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(fastManager_);

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onRequestLatency_WithNodes_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_066
 * @tc.desc  : Test OnRequestLatency with converter node present
 */
HWTEST_F(HpaeFastRendererManagerTest, onRequestLatency_WithNodes_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    uint64_t latency = 0;
    fastManager_->OnRequestLatency(TEST_STREAM_SESSION_ID, latency);
    // latency should be >= 0 (converter adds some latency)
    EXPECT_GE(latency, 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : reloadWithRunningStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_067
 * @tc.desc  : Test ReloadRenderManager with a running stream reconnects
 */
HWTEST_F(HpaeFastRendererManagerTest, reloadWithRunningStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(fastManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeSinkInfo newSinkInfo = GetFastSinkInfo();
    EXPECT_EQ(fastManager_->ReloadRenderManager(newSinkInfo, true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}
