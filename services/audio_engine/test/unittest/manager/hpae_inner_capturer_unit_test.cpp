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

#include "hpae_inner_capturer_manager.h"
#include "hpae_mocks.h"
#include "test_case_common.h"
#include <string>
#include "audio_errors.h"
#include <thread>
#include <chrono>

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const uint32_t DEFAULT_SESSION_ID = 123456;
const uint32_t OVERSIZED_FRAME_LENGTH = 38500;
const float FRAME_LENGTH_IN_SECOND = 0.02;
const uint32_t TEST_SESSION_ID_1 = 10999;
const uint32_t TEST_SESSION_ID_2 = 11000;
// Constants for InnerCapSinkNode tests
const uint32_t TEST_SESSION_ID_SINK_NODE = 70001;
const uint32_t TEST_SESSION_ID_PRE_NODE_BASE = 70100;
const uint32_t TEST_CONNECTION_COUNT = 5;
std::string g_rootPath = "/data/";

static HpaeSinkInfo GetInCapSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeInnerCapturerManagerTest.pcm";
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.frameLen = SAMPLE_RATE_48000 * FRAME_LENGTH_IN_SECOND;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    return sinkInfo;
}

class HpaeInnerCapturerManagerUnitTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    std::shared_ptr<HpaeInnerCapturerManager> hpaeInnerCapturerManager_ = nullptr;
};

void HpaeInnerCapturerManagerUnitTest::SetUp(void)
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    hpaeInnerCapturerManager_ = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
}

void HpaeInnerCapturerManagerUnitTest::TearDown(void)
{
    hpaeInnerCapturerManager_->DeInit();
    hpaeInnerCapturerManager_ = nullptr;
}

static HpaeStreamInfo GetInCapPlayStreamInfo()
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.frameLen = SAMPLE_RATE_44100 * FRAME_LENGTH_IN_SECOND;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.sessionId = DEFAULT_SESSION_ID + 1;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    return streamInfo;
}

static HpaeStreamInfo GetInCapRecordStreamInfo()
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.frameLen = SAMPLE_RATE_44100 * FRAME_LENGTH_IN_SECOND;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    return streamInfo;
}

static void WaitForMsgProcessing(std::shared_ptr<HpaeInnerCapturerManager>& hpaeInnerCapturerManager)
{
    int waitCount = 0;
    const int32_t waitCountThd = 5;  // 5ms
    while (hpaeInnerCapturerManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 20ms frameLen, need optimize
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));  // 40ms wait time, need optimize
    EXPECT_EQ(hpaeInnerCapturerManager->IsMsgProcessing(), false);
    EXPECT_EQ(waitCount < waitCountThd, true);
}

/**
 * @tc.name  : Test Construct
 * @tc.type  : FUNC
 * @tc.number: Construct_001
 * @tc.desc  : Test Construct when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, Construct_001, TestSize.Level1)
{
    EXPECT_NE(hpaeInnerCapturerManager_, nullptr);
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    HpaeSinkInfo dstSinkInfo = hpaeInnerCapturerManager_->GetSinkInfo();
    EXPECT_EQ(dstSinkInfo.deviceNetId == sinkInfo.deviceNetId, true);
    EXPECT_EQ(dstSinkInfo.deviceClass == sinkInfo.deviceClass, true);
    EXPECT_EQ(dstSinkInfo.adapterName == sinkInfo.adapterName, true);
    EXPECT_EQ(dstSinkInfo.frameLen == sinkInfo.frameLen, true);
    EXPECT_EQ(dstSinkInfo.samplingRate == sinkInfo.samplingRate, true);
    EXPECT_EQ(dstSinkInfo.format == sinkInfo.format, true);
    EXPECT_EQ(dstSinkInfo.channels == sinkInfo.channels, true);
    EXPECT_EQ(dstSinkInfo.deviceType == sinkInfo.deviceType, true);
}

/**
 * @tc.name  : Test Init
 * @tc.type  : FUNC
 * @tc.number: Init_001
 * @tc.desc  : Test Init.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, Init_001, TestSize.Level1)
{
    EXPECT_NE(hpaeInnerCapturerManager_, nullptr);
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
}

/**
 * @tc.name  : Test DeInit
 * @tc.type  : FUNC
 * @tc.number: DeInit_001
 * @tc.desc  : Test DeInit.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, DeInit_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), false);
    EXPECT_EQ(hpaeInnerCapturerManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), false);
}

/**
 * @tc.name  : Test CreateStream
 * @tc.type  : FUNC
 * @tc.number: CreateStream_001
 * @tc.desc  : Test CreateRendererStream when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, CreateStream_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
}

/**
 * @tc.name  : Test CreateStream
 * @tc.type  : FUNC
 * @tc.number: CreateStream_002
 * @tc.desc  : Test CreateCapturerStream when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, CreateStream_002, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
    HpaeStreamInfo streamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSourceOutputInfo sourceOutoputInfo;
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSourceOutputInfo(streamInfo.sessionId, sourceOutoputInfo), SUCCESS);
}

/**
 * @tc.name  : Test DestroyStream
 * @tc.type  : FUNC
 * @tc.number: DestroyStream_001
 * @tc.desc  : Test DestroyRendererStream when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, DestroyStream_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSinkInputInfo(streamInfo.sessionId,
        sinkInputInfo) == ERR_INVALID_OPERATION, true);
}

/**
 * @tc.name  : Test DestroyStream
 * @tc.type  : FUNC
 * @tc.number: DestroyStream_002
 * @tc.desc  : Test DestroyCapturerStream when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, DestroyStream_002, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo streamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSourceOutputInfo sourceOutoputInfo;
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSourceOutputInfo(streamInfo.sessionId, sourceOutoputInfo)
        == ERR_INVALID_OPERATION, SUCCESS);
}

/**
 * @tc.name  : Test StreamStartPauseFlushChange_001
 * @tc.type  : FUNC
 * @tc.number: StreamStartPauseFlushChange_001
 * @tc.desc  : Test StreamStartPauseFlushChange when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, StreamStartPauseFlushChange_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeInnerCapturerManager_->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetOffloadPolicy(playStreamInfo.sessionId, 0), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    hpaeInnerCapturerManager_->SetSpeed(playStreamInfo.sessionId, 2.0f); // 2.0f test
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInputInfo sinkInputInfo;

    EXPECT_EQ(hpaeInnerCapturerManager_->IsRunning(), true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Pause(recordStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsRunning(), false);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Pause(playStreamInfo.sessionId) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->Pause(playStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsRunning(), false);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Flush(recordStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Flush(playStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSinkInputInfo(playStreamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSourceOutputInfo(recordStreamInfo.sessionId, sourceOutoputInfo), SUCCESS);
    EXPECT_EQ(sourceOutoputInfo.capturerSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(recordStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playStreamInfo.sessionId) == SUCCESS, true);
}

/**
 * @tc.name  : Test StreamStartStopDrainChange_001
 * @tc.type  : FUNC
 * @tc.number: StreamStartStopDrainChange_001
 * @tc.desc  : Test StreamStartStopDrainChange when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, StreamStartStopDrainChange_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo;
    recordStreamInfo.channels = STEREO;
    recordStreamInfo.samplingRate = SAMPLE_RATE_44100;
    recordStreamInfo.frameLen = SAMPLE_RATE_44100 * FRAME_LENGTH_IN_SECOND;
    recordStreamInfo.format = SAMPLE_S16LE;
    recordStreamInfo.sessionId = DEFAULT_SESSION_ID;
    recordStreamInfo.streamType = STREAM_MUSIC;
    recordStreamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    recordStreamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeInnerCapturerManager_->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInputInfo sinkInputInfo;

    EXPECT_EQ(hpaeInnerCapturerManager_->Drain(recordStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Drain(playStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsRunning(), true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Stop(recordStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Stop(playStreamInfo.sessionId) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->Stop(playStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsRunning(), false);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSinkInputInfo(playStreamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSourceOutputInfo(recordStreamInfo.sessionId, sourceOutoputInfo), SUCCESS);
    EXPECT_EQ(sourceOutoputInfo.capturerSessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(recordStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playStreamInfo.sessionId) == SUCCESS, true);
}

/**
 * @tc.name  : Test StreamStartStopDump_001
 * @tc.type  : FUNC
 * @tc.number: StreamStartStopDump_001
 * @tc.desc  : Test StreamStartStop when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, StreamStartStopDump_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo;
    recordStreamInfo.channels = STEREO;
    recordStreamInfo.samplingRate = SAMPLE_RATE_44100;
    recordStreamInfo.frameLen = SAMPLE_RATE_44100 * FRAME_LENGTH_IN_SECOND;
    recordStreamInfo.format = SAMPLE_S16LE;
    recordStreamInfo.sessionId = DEFAULT_SESSION_ID;
    recordStreamInfo.streamType = STREAM_MUSIC;
    recordStreamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    recordStreamInfo.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSourceOutputInfo sourceOutoputInfo;

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeInnerCapturerManager_->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(hpaeInnerCapturerManager_->IsRunning(), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->Stop(playStreamInfo.sessionId) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->DumpSinkInfo() == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->Stop(recordStreamInfo.sessionId) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(recordStreamInfo.sessionId) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playStreamInfo.sessionId) == SUCCESS, true);
}

/**
 * @tc.name  : Test AddNodeToSink_001
 * @tc.type  : FUNC
 * @tc.number: AddNodeToSink_001
 * @tc.desc  : Test AddNodeToSink when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, AddNodeToSink_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo playSencondStreamInfo = GetInCapPlayStreamInfo();
    ++playSencondStreamInfo.sessionId;
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playSencondStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playSencondStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeNodeInfo playSencondNodeInfo;
    playSencondNodeInfo.sessionId = playSencondStreamInfo.sessionId + 1;
    playSencondNodeInfo.channels = STEREO;
    playSencondNodeInfo.format = SAMPLE_S16LE;
    playSencondNodeInfo.frameLen = SAMPLE_RATE_44100 * FRAME_LENGTH_IN_SECOND;
    playSencondNodeInfo.samplingRate = SAMPLE_RATE_44100;
    playSencondNodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    playSencondNodeInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    playSencondNodeInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    std::shared_ptr<HpaeSinkInputNode> HpaeSinkInputSencondNode =
        std::make_shared<HpaeSinkInputNode>(playSencondNodeInfo);
    EXPECT_EQ(HpaeSinkInputSencondNode != nullptr, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    hpaeInnerCapturerManager_->AddSingleNodeToSinkInner(HpaeSinkInputSencondNode, false);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->AddNodeToSink(HpaeSinkInputSencondNode), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playSencondNodeInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playSencondStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(recordStreamInfo.sessionId) == SUCCESS, true);
}

/**
 * @tc.name  : Test SetMute_001
 * @tc.type  : FUNC
 * @tc.number: SetMute_001
 * @tc.desc  : Test SetMute when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, SetMute_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->SetMute(true), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    EXPECT_EQ(hpaeInnerCapturerManager_->SetMute(true), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetMute(false), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : Test OnFadeDone_001
 * @tc.type  : FUNC
 * @tc.number: OnFadeDone_001
 * @tc.desc  : Test OnFadeDone when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, OnFadeDone_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeInnerCapturerManager_->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    hpaeInnerCapturerManager_->OnFadeDone(playStreamInfo.sessionId);
}

/**
 * @tc.name  : Test GetThreadName_001
 * @tc.type  : FUNC
 * @tc.number: GetThreadName_001
 * @tc.desc  : Test GetThreadName
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, GetThreadName_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    sinkInfo.deviceName = "InnerCap1";
    hpaeInnerCapturerManager_ = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    std::string threadName = hpaeInnerCapturerManager_->GetThreadName();
    EXPECT_EQ(threadName, "InnerCap1");

    sinkInfo.deviceName = "InnerCap";
    hpaeInnerCapturerManager_ = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    threadName = hpaeInnerCapturerManager_->GetThreadName();
    EXPECT_EQ(threadName, "InnerCap");

    sinkInfo.deviceName = "RemoteCastInnerCapturer";
    hpaeInnerCapturerManager_ = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    threadName = hpaeInnerCapturerManager_->GetThreadName();
    EXPECT_EQ(threadName, "RemoteCast");
}

/**
 * @tc.name  : Test SendRequestInner_001
 * @tc.type  : FUNC
 * @tc.number: SendRequestInner_001
 * @tc.desc  : Test SendRequestInner when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, SendRequestInner_001, TestSize.Level1)
{
    auto request = []() {
    };
    hpaeInnerCapturerManager_->SendRequestInner(request, "unit_test_send_request");
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    hpaeInnerCapturerManager_->SendRequestInner(request, "unit_test_send_request");
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    hpaeInnerCapturerManager_->hpaeSignalProcessThread_ = nullptr;
    hpaeInnerCapturerManager_->SendRequestInner(request, "unit_test_send_request");
    EXPECT_EQ(hpaeInnerCapturerManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test Other_001
 * @tc.type  : FUNC
 * @tc.number: Other_001
 * @tc.desc  : Test Other when config in vaild.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, Other_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeInnerCapturerManager_->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    
    std::vector<SinkInput> sinkInputs;
    sinkInputs = hpaeInnerCapturerManager_->GetAllSinkInputsInfo();
    std::vector<SourceOutput> sourceOutputs;
    sourceOutputs = hpaeInnerCapturerManager_->GetAllSourceOutputsInfo();
    std::string config = hpaeInnerCapturerManager_->GetDeviceHDFDumpInfo();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    int32_t testVariable = 1;
    EXPECT_EQ(hpaeInnerCapturerManager_->SetClientVolume(playStreamInfo.sessionId, 1.0f), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetRate(playStreamInfo.sessionId, testVariable), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetAudioEffectMode(playStreamInfo.sessionId, testVariable), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetAudioEffectMode(playStreamInfo.sessionId, testVariable), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetPrivacyType(playStreamInfo.sessionId, testVariable), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetPrivacyType(playStreamInfo.sessionId, testVariable), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetWritableSize(playStreamInfo.sessionId), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->UpdateSpatializationState(playStreamInfo.sessionId, true, true), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->UpdateMaxLength(playStreamInfo.sessionId, testVariable), SUCCESS);
}

/**
 * @tc.name  : Test ReloadRenderManager_001
 * @tc.type  : FUNC
 * @tc.number: ReloadRenderManager_001
 * @tc.desc  : Test ReloadRenderManager.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, ReloadRenderManager_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    ++playStreamInfo.sessionId;
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->ReloadRenderManager(sinkInfo, false), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DeInit(), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test MoveAllStreamToNewSinkInner
 * @tc.type  : FUNC
 * @tc.number: MoveAllStreamToNewSinkInner_001
 * @tc.desc  : Test MoveAllStreamToNewSinkInner.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, MoveAllStreamToNewSinkInner_001, TestSize.Level0)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    ++playStreamInfo.sessionId;
    auto mockCallback = std::make_shared<MockSendMsgCallback>();
    EXPECT_CALL(*mockCallback, InvokeSync(MOVE_ALL_SINK_INPUT, testing::_))
        .Times(1);
    EXPECT_CALL(*mockCallback, Invoke(MOVE_ALL_SINK_INPUT, testing::_))
        .Times(1);
    hpaeInnerCapturerManager_->weakCallback_ = mockCallback;
    vector<uint32_t> moveids;
    hpaeInnerCapturerManager_->MoveAllStreamToNewSinkInner("", moveids, MOVE_ALL);
    hpaeInnerCapturerManager_->MoveAllStreamToNewSinkInner("", moveids, MOVE_PREFER);
    EXPECT_EQ(hpaeInnerCapturerManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test MoveAllStreamToNewSinkInner with created stream
 * @tc.type  : FUNC
 * @tc.number: MoveAllStreamToNewSinkInner_003
 * @tc.desc  : Test MoveAllStreamToNewSinkInner with created streams.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, MoveAllStreamToNewSinkInner_003, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    // Create stream first to populate sinkInputNodeMap_
    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    auto mockCallback = std::make_shared<MockSendMsgCallback>();
    EXPECT_CALL(*mockCallback, InvokeSync(MOVE_ALL_SINK_INPUT, testing::_))
        .Times(1);
    EXPECT_CALL(*mockCallback, Invoke(MOVE_ALL_SINK_INPUT, testing::_))
        .Times(1);
    hpaeInnerCapturerManager_->weakCallback_ = mockCallback;

    vector<uint32_t> moveids;
    hpaeInnerCapturerManager_->MoveAllStreamToNewSinkInner("", moveids, MOVE_ALL);
    hpaeInnerCapturerManager_->MoveAllStreamToNewSinkInner("", moveids, MOVE_PREFER);
    EXPECT_EQ(hpaeInnerCapturerManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test InitSinkInner
 * @tc.type  : FUNC
 * @tc.number: InitSinkInner_001
 * @tc.desc  : Test InitSinkInner
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InitSinkInner_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    sinkInfo.frameLen = 0;
    bool isReload = true;
    hpaeInnerCapturerManager_ = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    EXPECT_EQ(hpaeInnerCapturerManager_->InitSinkInner(isReload), ERROR);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), false);
}

/**
 * @tc.name  : Test InitSinkInner
 * @tc.type  : FUNC
 * @tc.number: InitSinkInner_002
 * @tc.desc  : Test InitSinkInner
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InitSinkInner_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetInCapSinkInfo();
    sinkInfo.frameLen = OVERSIZED_FRAME_LENGTH;
    bool isReload = true;
    hpaeInnerCapturerManager_ = std::make_shared<HPAE::HpaeInnerCapturerManager>(sinkInfo);
    EXPECT_EQ(hpaeInnerCapturerManager_->InitSinkInner(isReload), ERROR);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), false);
}

/**
 * @tc.name  : Test CreateStream
 * @tc.type  : FUNC
 * @tc.number: CreateStream_003
 * @tc.desc  : Test CreateStream
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, CreateStream_003, TestSize.Level0)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    streamInfo.frameLen = 0;
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), ERROR);
}

/**
 * @tc.name  : Test CreateStream
 * @tc.type  : FUNC
 * @tc.number: CreateStream_004
 * @tc.desc  : Test CreateStream
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, CreateStream_004, TestSize.Level0)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    streamInfo.frameLen = OVERSIZED_FRAME_LENGTH;
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), ERROR);
}

/**
 * @tc.name  : Test SetSessionFade
 * @tc.type  : FUNC
 * @tc.number: SetSessionFade_001
 * @tc.desc  : Test SetSessionFade
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, SetSessionFade_001, TestSize.Level0)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_STARTED), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_PAUSED), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->Stop(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_STARTED), false);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_PAUSED), false);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    hpaeInnerCapturerManager_->rendererSceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->idGainMap_.clear();
    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_STARTED), false);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_PAUSED), false);
}

/**
 * @tc.name  : Test SetSessionFade
 * @tc.type  : FUNC
 * @tc.number: SetSessionFade_002
 * @tc.desc  : Test SetSessionFade
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, SetSessionFade_002, TestSize.Level0)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_STOPPED), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_PAUSED), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->Pause(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Stop(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : Test TriggerStreamState
 * @tc.type  : FUNC
 * @tc.number: TriggerStreamState_001
 * @tc.desc  : Test TriggerStreamState
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, TriggerStreamState_001, TestSize.Level0)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_STARTED), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->sinkInputNodeMap_[streamInfo.sessionId]->GetState(), HPAE_SESSION_RUNNING);
    hpaeInnerCapturerManager_->TriggerStreamState(streamInfo.sessionId,
        hpaeInnerCapturerManager_->sinkInputNodeMap_[streamInfo.sessionId]);

    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_STOPPED), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->sinkInputNodeMap_[streamInfo.sessionId]->GetState(), HPAE_SESSION_STOPPING);
    hpaeInnerCapturerManager_->TriggerStreamState(streamInfo.sessionId,
        hpaeInnerCapturerManager_->sinkInputNodeMap_[streamInfo.sessionId]);
    EXPECT_EQ(hpaeInnerCapturerManager_->sinkInputNodeMap_[streamInfo.sessionId]->GetState(), HPAE_SESSION_STOPPED);

    EXPECT_EQ(hpaeInnerCapturerManager_->SetSessionFade(streamInfo.sessionId, OPERATION_PAUSED), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->sinkInputNodeMap_[streamInfo.sessionId]->GetState(), HPAE_SESSION_PAUSING);
    hpaeInnerCapturerManager_->TriggerStreamState(streamInfo.sessionId,
        hpaeInnerCapturerManager_->sinkInputNodeMap_[streamInfo.sessionId]);
    EXPECT_EQ(hpaeInnerCapturerManager_->sinkInputNodeMap_[streamInfo.sessionId]->GetState(), HPAE_SESSION_PAUSED);
}

/**
 * @tc.name  : AddAllNodesToSink_Coverage_Only
 * @tc.type  : FUNC
 * @tc.number: AddAllNodes_001
 * @tc.desc  : Coverage-focused test for MoveAllStreamToNewSinkInner loop and isMoveAble logic.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, AddAllNodesToSink_Coverage_Only, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    HpaeNodeInfo info;
    info.sessionId = 3001;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info));
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info));

    int32_t ret = hpaeInnerCapturerManager_->AddAllNodesToSink(sinkInputs, true);

    // 4. 断言及等待
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : MoveAllStreamToNewSinkInner_002
 * @tc.type  : FUNC
 * @tc.number: MoveAllStream_002
 * @tc.desc  : Coverage-focused test for MoveAllStreamToNewSinkInner loop and isMoveAble logic.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, MoveAllStreamToNewSinkInner_002, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo streamA = GetInCapPlayStreamInfo();
    streamA.sessionId = 6001;
    hpaeInnerCapturerManager_->CreateStream(streamA);

    HpaeStreamInfo streamB = GetInCapPlayStreamInfo();
    streamB.sessionId = 6002;
    hpaeInnerCapturerManager_->CreateStream(streamB);

    hpaeInnerCapturerManager_->rendererSessionNodeMap_[6002].isMoveAble = false;

    hpaeInnerCapturerManager_->MoveAllStreamToNewSinkInner("TargetSink", {}, MOVE_ALL);

    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
}

/**
 * @tc.name  : MoveAllStream_001
 * @tc.type  : FUNC
 * @tc.number: MoveAllStream_001
 * @tc.desc  : Test MoveAllStream sync and async paths for code coverage.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, MoveAllStream_001, TestSize.Level1)
{
    std::string sinkName = "TestSink";
    std::vector<uint32_t> sessionIds = {7001};

    hpaeInnerCapturerManager_->MoveAllStream(sinkName, sessionIds, MOVE_ALL);

    hpaeInnerCapturerManager_->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    int32_t result = hpaeInnerCapturerManager_->MoveAllStream(sinkName, sessionIds, MOVE_PREFER);

    EXPECT_EQ(result, SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : MoveStream_001
 * @tc.type  : FUNC
 * @tc.number: MoveStream_001
 * @tc.desc  : Test MoveStream for full branch coverage including error paths.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, MoveStream_001, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    uint32_t validSessionId = 8001;
    uint32_t invalidSessionId = 9999;

    hpaeInnerCapturerManager_->MoveStream(invalidSessionId, "TargetSink");
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    streamInfo.sessionId = validSessionId;
    hpaeInnerCapturerManager_->CreateStream(streamInfo);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    hpaeInnerCapturerManager_->MoveStream(validSessionId, "");
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    hpaeInnerCapturerManager_->MoveStream(validSessionId, "TargetSink");
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
}

/**
 * @tc.name  : MoveAllStream not init
 * @tc.type  : FUNC
 * @tc.number: MoveAllStream_002
 * @tc.desc  : Test MoveAllStream when not initialized (sync mode).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, MoveAllStream_002, TestSize.Level1)
{
    std::string sinkName = "TestSink";
    std::vector<uint32_t> sessionIds = {9001};

    // Test MoveAllStream when not initialized - should use sync mode
    hpaeInnerCapturerManager_->MoveAllStream(sinkName, sessionIds, MOVE_ALL);

    // Now initialize and test async mode
    hpaeInnerCapturerManager_->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    int32_t result = hpaeInnerCapturerManager_->MoveAllStream(sinkName, sessionIds, MOVE_PREFER);

    EXPECT_EQ(result, SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : MoveAllStream with created streams
 * @tc.type  : FUNC
 * @tc.number: MoveAllStream_003
 * @tc.desc  : Test MoveAllStream with initialized manager and created streams.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, MoveAllStream_003, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    // Create stream to populate sinkInputNodeMap_
    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    std::string sinkName = "TargetSink";
    std::vector<uint32_t> sessionIds = {streamInfo.sessionId};

    // Test MoveAllStream in async mode (after Init)
    int32_t result = hpaeInnerCapturerManager_->MoveAllStream(sinkName, sessionIds, MOVE_ALL);

    EXPECT_EQ(result, SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : MoveStream with empty sink name
 * @tc.type  : FUNC
 * @tc.number: MoveStream_002
 * @tc.desc  : Test MoveStream with empty sink name.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, MoveStream_002, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    // Create stream
    HpaeStreamInfo streamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    // Test MoveStream with empty sink name - should fail gracefully
    hpaeInnerCapturerManager_->MoveStream(streamInfo.sessionId, "");

    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);
}

/**
 * @tc.name  : DeactivateThread_001
 * @tc.type  : FUNC
 * @tc.number: DeactivateThread_001
 * @tc.desc  : Test DeactivateThread to cover thread cleanup and request handling.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, DeactivateThread_001, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();

    bool result1 = hpaeInnerCapturerManager_->DeactivateThread();
    EXPECT_TRUE(result1);

    bool result2 = hpaeInnerCapturerManager_->DeactivateThread();
    EXPECT_TRUE(result2);
}

/**
 * @tc.name  : StopManager_001
 * @tc.type  : FUNC
 * @tc.number: StopManager_001
 * @tc.desc  : Test StopManager to cover internal sink node stop logic.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, StopManager_001, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();
    hpaeInnerCapturerManager_->hpaeInnerCapSinkNode_ = nullptr;

    hpaeInnerCapturerManager_->StopManager();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeNodeInfo info;
    info.deviceName = "InnerCapSink";
    auto mockSinkNode = std::make_shared<HpaeInnerCapSinkNode>(info);
    hpaeInnerCapturerManager_->hpaeInnerCapSinkNode_ = mockSinkNode;

    int32_t result = hpaeInnerCapturerManager_->StopManager();

    EXPECT_EQ(result, SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : InnerCapSinkNode_Reset_001
 * @tc.type  : FUNC
 * @tc.desc  : Test Reset logic by accessing the node through InnerCapturerManager.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Reset_001, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 60001;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);
    hpaeInnerCapturerManager_->hpaeInnerCapSinkNode_ = sinkNode;

    HpaeNodeInfo preInfo;
    preInfo.sessionId = 70001;
    auto preNode = std::make_shared<HpaeSinkInputNode>(preInfo);

    sinkNode->Connect(preNode);
    ASSERT_EQ(sinkNode->GetPreOutNum(), 1);

    bool result = sinkNode->Reset();

    EXPECT_TRUE(result);
    EXPECT_EQ(sinkNode->GetPreOutNum(), 0);
}

/**
 * @tc.name  : InnerCapSinkNode_Control_001
 * @tc.type  : FUNC
 * @tc.desc  : Test Flush, Pause, Reset, and Resume through the manager's node.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Control_001, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);
    hpaeInnerCapturerManager_->hpaeInnerCapSinkNode_ = sinkNode;

    EXPECT_EQ(sinkNode->InnerCapturerSinkFlush(), SUCCESS);

    EXPECT_EQ(sinkNode->InnerCapturerSinkPause(), SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    EXPECT_EQ(sinkNode->InnerCapturerSinkReset(), SUCCESS);

    EXPECT_EQ(sinkNode->InnerCapturerSinkResume(), SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : DeactivateThread_002
 * @tc.type  : FUNC
 * @tc.number: DeactivateThread_002
 * @tc.desc  : Test DeactivateThread after Init to verify thread deactivation.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, DeactivateThread_002, TestSize.Level1)
{
    hpaeInnerCapturerManager_->Init();
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    // Verify thread is initialized and active
    EXPECT_EQ(hpaeInnerCapturerManager_->IsInit(), true);

    // Test DeactivateThread
    bool result = hpaeInnerCapturerManager_->DeactivateThread();
    EXPECT_TRUE(result);

    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : Test Pause with isStandby for InnerCapturerManager
 * @tc.type  : FUNC
 * @tc.number: PauseWithStandby_001
 * @tc.desc  : Test Pause with isStandby=true for renderer stream in InnerCapturerManager
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, PauseWithStandby_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeInnerCapturerManager_->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInputInfo sinkInputInfo;

    EXPECT_EQ(hpaeInnerCapturerManager_->IsRunning(), true);
    EXPECT_EQ(hpaeInnerCapturerManager_->Pause(playStreamInfo.sessionId, true) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSinkInputInfo(playStreamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);

    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(recordStreamInfo.sessionId) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : Test Pause with isStandby for InnerCapturerManager
 * @tc.type  : FUNC
 * @tc.number: PauseWithStandby_002
 * @tc.desc  : Test Pause with isStandby=false for renderer stream in InnerCapturerManager
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, PauseWithStandby_002, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo recordStreamInfo = GetInCapRecordStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(recordStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(recordStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    std::shared_ptr<WriteFixedDataCb> writeInPlayDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeInnerCapturerManager_->RegisterWriteCallback(playStreamInfo.sessionId, writeInPlayDataCb), SUCCESS);
    EXPECT_EQ(hpaeInnerCapturerManager_->Start(playStreamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeSinkInputInfo sinkInputInfo;

    EXPECT_EQ(hpaeInnerCapturerManager_->Pause(playStreamInfo.sessionId, false) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    EXPECT_EQ(hpaeInnerCapturerManager_->GetSinkInputInfo(playStreamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);

    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(recordStreamInfo.sessionId) == SUCCESS, true);
    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : Test SetIsLowLatency for InnerCapturerManager
 * @tc.type  : FUNC
 * @tc.number: SetIsLowLatency_001
 * @tc.desc  : Test SetIsLowLatency is called with false during AddSingleNodeToSinkInner
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, SetIsLowLatency_001, TestSize.Level1)
{
    EXPECT_EQ(hpaeInnerCapturerManager_->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
    HpaeStreamInfo playStreamInfo = GetInCapPlayStreamInfo();
    EXPECT_EQ(hpaeInnerCapturerManager_->CreateStream(playStreamInfo), SUCCESS);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);

    auto it = hpaeInnerCapturerManager_->sinkInputNodeMap_.find(playStreamInfo.sessionId);
    ASSERT_EQ(it != hpaeInnerCapturerManager_->sinkInputNodeMap_.end(), true);
    auto node = it->second;
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->GetIsLowLatency(), false);

    EXPECT_EQ(hpaeInnerCapturerManager_->DestroyStream(playStreamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeInnerCapturerManager_);
}

/**
 * @tc.name  : InnerCapSinkNode_Reset_Empty_001
 * @tc.type  : FUNC
 * @tc.desc  : Test Reset with no connections (Branch 1: preOutputMap is empty).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Reset_Empty_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Verify no connections initially
    EXPECT_EQ(sinkNode->GetPreOutNum(), 0);

    // Call Reset with empty map - loop should not execute
    bool result = sinkNode->Reset();

    EXPECT_TRUE(result);
    EXPECT_EQ(sinkNode->GetPreOutNum(), 0);
}

/**
 * @tc.name  : InnerCapSinkNode_Reset_Multiple_001
 * @tc.type  : FUNC
 * @tc.desc  : Test Reset with multiple connections (Branch 2: loop executes for each).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Reset_Multiple_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Create multiple pre-nodes and connect them
    HpaeNodeInfo preInfo1;
    preInfo1.sessionId = TEST_SESSION_ID_PRE_NODE_BASE;
    auto preNode1 = std::make_shared<HpaeSinkInputNode>(preInfo1);

    HpaeNodeInfo preInfo2;
    preInfo2.sessionId = TEST_SESSION_ID_1;
    auto preNode2 = std::make_shared<HpaeSinkInputNode>(preInfo2);

    HpaeNodeInfo preInfo3;
    preInfo3.sessionId = TEST_SESSION_ID_2;
    auto preNode3 = std::make_shared<HpaeSinkInputNode>(preInfo3);

    sinkNode->Connect(preNode1);
    sinkNode->Connect(preNode2);
    sinkNode->Connect(preNode3);

    // Verify multiple connections
    EXPECT_EQ(sinkNode->GetPreOutNum(), 3);

    // Call Reset - loop should execute for all three connections
    bool result = sinkNode->Reset();

    EXPECT_TRUE(result);
    EXPECT_EQ(sinkNode->GetPreOutNum(), 0);
}

/**
 * @tc.name  : InnerCapSinkNode_Flush_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkFlush always returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Flush_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Flush should always return SUCCESS
    int32_t result = sinkNode->InnerCapturerSinkFlush();

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : InnerCapSinkNode_Pause_FromNew_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkPause from NEW state (Branch 1: NEW -> SUSPENDED).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Pause_FromNew_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Initial state should be NEW
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_NEW);

    // Pause from NEW state
    int32_t result = sinkNode->InnerCapturerSinkPause();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : InnerCapSinkNode_Pause_FromRunning_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkPause from RUNNING state (Branch 2: RUNNING -> SUSPENDED).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Pause_FromRunning_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Set state to RUNNING
    sinkNode->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RUNNING);

    // Pause from RUNNING state
    int32_t result = sinkNode->InnerCapturerSinkPause();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : InnerCapSinkNode_Pause_FromSuspended_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkPause from SUSPENDED state (Branch 3: duplicate pause).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Pause_FromSuspended_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Set state to SUSPENDED
    sinkNode->SetSinkState(STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    // Pause from SUSPENDED state (duplicate)
    int32_t result = sinkNode->InnerCapturerSinkPause();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : InnerCapSinkNode_Pause_FromReleased_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkPause from RELEASED state (Branch 4: RELEASED -> SUSPENDED).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Pause_FromReleased_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Set state to RELEASED
    sinkNode->SetSinkState(STREAM_MANAGER_RELEASED);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RELEASED);

    // Pause from RELEASED state
    int32_t result = sinkNode->InnerCapturerSinkPause();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : InnerCapSinkNode_ResetSink_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkReset always returns SUCCESS.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_ResetSink_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Reset should always return SUCCESS
    int32_t result = sinkNode->InnerCapturerSinkReset();

    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : InnerCapSinkNode_Resume_FromNew_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkResume from NEW state (Branch 1: NEW -> RUNNING).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Resume_FromNew_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Initial state should be NEW
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_NEW);

    // Resume from NEW state
    int32_t result = sinkNode->InnerCapturerSinkResume();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : InnerCapSinkNode_Resume_FromSuspended_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkResume from SUSPENDED state (Branch 2: SUSPENDED -> RUNNING).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Resume_FromSuspended_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Set state to SUSPENDED
    sinkNode->SetSinkState(STREAM_MANAGER_SUSPENDED);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);

    // Resume from SUSPENDED state
    int32_t result = sinkNode->InnerCapturerSinkResume();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : InnerCapSinkNode_Resume_FromRunning_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkResume from RUNNING state (Branch 3: duplicate resume).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Resume_FromRunning_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Set state to RUNNING
    sinkNode->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RUNNING);

    // Resume from RUNNING state (duplicate)
    int32_t result = sinkNode->InnerCapturerSinkResume();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : InnerCapSinkNode_Resume_FromReleased_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkResume from RELEASED state (Branch 4: RELEASED -> RUNNING).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Resume_FromReleased_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Set state to RELEASED
    sinkNode->SetSinkState(STREAM_MANAGER_RELEASED);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RELEASED);

    // Resume from RELEASED state
    int32_t result = sinkNode->InnerCapturerSinkResume();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RUNNING);
}

/**
 * @tc.name  : InnerCapSinkNode_GetPreOutNum_NoConnections_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetPreOutNum with no connections (Branch 1: return 0).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_GetPreOutNum_NoConnections_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // No connections - should return 0
    size_t numConnections = sinkNode->GetPreOutNum();

    EXPECT_EQ(numConnections, 0);
}

/**
 * @tc.name  : InnerCapSinkNode_GetPreOutNum_SingleConnection_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetPreOutNum with single connection (Branch 2: return 1).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_GetPreOutNum_SingleConnection_001, TestSize.Level0)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    HpaeNodeInfo preInfo;
    preInfo.sessionId = TEST_SESSION_ID_PRE_NODE_BASE;
    auto preNode = std::make_shared<HpaeSinkInputNode>(preInfo);

    sinkNode->Connect(preNode);

    // Single connection - should return 1
    size_t numConnections = sinkNode->GetPreOutNum();

    EXPECT_EQ(numConnections, 1);
}

/**
 * @tc.name  : InnerCapSinkNode_GetPreOutNum_MultipleConnections_001
 * @tc.type  : FUNC
 * @tc.desc  : Test GetPreOutNum with multiple connections (Branch 3: return N).
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_GetPreOutNum_MultipleConnections_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Create 5 pre-nodes and connect them
    const int numNodes = TEST_CONNECTION_COUNT;
    std::vector<std::shared_ptr<HpaeSinkInputNode>> preNodes;
    for (int i = 0; i < numNodes; i++) {
        HpaeNodeInfo preInfo;
        preInfo.sessionId = TEST_SESSION_ID_PRE_NODE_BASE + i;
        auto preNode = std::make_shared<HpaeSinkInputNode>(preInfo);
        preNodes.push_back(preNode);
        sinkNode->Connect(preNode);
    }

    // Multiple connections - should return numNodes
    size_t numConnections = sinkNode->GetPreOutNum();

    EXPECT_EQ(numConnections, static_cast<size_t>(numNodes));
}


/**
 * @tc.name  : InnerCapSinkNode_Pause_FromIdle_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkPause from IDLE state.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Pause_FromIdle_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = TEST_SESSION_ID_SINK_NODE;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Set state to IDLE
    sinkNode->SetSinkState(STREAM_MANAGER_IDLE);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_IDLE);

    int32_t result = sinkNode->InnerCapturerSinkPause();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_SUSPENDED);
}

/**
 * @tc.name  : InnerCapSinkNode_Resume_FromIdle_001
 * @tc.type  : FUNC
 * @tc.desc  : Test InnerCapturerSinkResume from IDLE state.
 */
HWTEST_F(HpaeInnerCapturerManagerUnitTest, InnerCapSinkNode_Resume_FromIdle_001, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = TEST_SESSION_ID_SINK_NODE;
    auto sinkNode = std::make_shared<HpaeInnerCapSinkNode>(nodeInfo);

    // Set state to IDLE
    sinkNode->SetSinkState(STREAM_MANAGER_IDLE);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_IDLE);

    int32_t result = sinkNode->InnerCapturerSinkResume();

    EXPECT_EQ(result, SUCCESS);
    EXPECT_EQ(sinkNode->GetSinkState(), STREAM_MANAGER_RUNNING);
}

}  // namespace HPAE
}  // namespace OHOS::AudioStandard
}  // namespace OHOS