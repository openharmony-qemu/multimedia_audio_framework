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
#include "audio_pipe_types.h"
#include "hpae_capturer_manager.h"
#include "hpae_fast_capturer_manager.h"
#include "i_hpae_capturer_manager.h"
#include "hpae_node_common.h"
#include "hpae_capture_move_info.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";
const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t OVERSIZED_FRAME_LENGTH = 38500;
const uint32_t DEFAULT_SESSION_ID = 123456;
const uint32_t DEFAULT_NODE_ID = 1243;
const std::string DEFAULT_SOURCE_NAME = "Built_in_mic";

class HpaeCapturerManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeCapturerManagerTest::SetUp()
{}

void HpaeCapturerManagerTest::TearDown()
{}

static void TestCheckSourceOutputInfo(HpaeSourceOutputInfo& sourceOutputInfo, const HpaeStreamInfo& streamInfo)
{
    EXPECT_EQ(sourceOutputInfo.nodeInfo.channels == streamInfo.channels, true);
    EXPECT_EQ(sourceOutputInfo.nodeInfo.format == streamInfo.format, true);
    EXPECT_EQ(sourceOutputInfo.nodeInfo.frameLen == streamInfo.frameLen, true);
    EXPECT_EQ(sourceOutputInfo.nodeInfo.sessionId == streamInfo.sessionId, true);
    EXPECT_EQ(sourceOutputInfo.nodeInfo.samplingRate == streamInfo.samplingRate, true);
    EXPECT_EQ(sourceOutputInfo.nodeInfo.streamType == streamInfo.streamType, true);
}

static void InitSourceInfo(HpaeSourceInfo &sourceInfo)
{
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.filePath = g_rootCapturerPath;

    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    sourceInfo.micRef = HPAE_REF_OFF;
}

static void InitReloadStreamInfo(HpaeStreamInfo &streamInfo)
{
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.deviceName = "Built_in_mic";
}

static void WaitForMsgProcessing(std::shared_ptr<HpaeCapturerManager> &capturerManager)
{
    int waitCount = 0;
    const int waitCountThd = 5;
    while (capturerManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));  // 20 for sleep
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(40));  // 40 for sleep
    EXPECT_EQ(capturerManager->IsMsgProcessing(), false);
    EXPECT_EQ(waitCount < waitCountThd, true);
}

static void InitNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.nodeId = DEFAULT_NODE_ID;
    nodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S16LE;
    nodeInfo.sceneType = HPAE_SCENE_RECORD;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
}

/*
 * tc.name   : Test IHpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : AdjustFrameLen_001
 * tc.desc   : Test AdjustFrameLen recalculates output frame length by effective sample rate.
 */
HWTEST_F(HpaeCapturerManagerTest, AdjustFrameLen_001, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    HpaeNodeInfo outNodeInfo;
    inNodeInfo.frameLen = 480;
    inNodeInfo.samplingRate = SAMPLE_RATE_48000;
    outNodeInfo.frameLen = 0;
    outNodeInfo.samplingRate = SAMPLE_RATE_96000;

    bool ret = IHpaeCapturerManager::AdjustFrameLen(outNodeInfo, inNodeInfo);

    EXPECT_TRUE(ret);
    EXPECT_EQ(outNodeInfo.frameLen, 960);
}

/*
 * tc.name   : Test IHpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : AdjustFrameLen_002
 * tc.desc   : Test AdjustFrameLen returns false when input effective sample rate is zero.
 */
HWTEST_F(HpaeCapturerManagerTest, AdjustFrameLen_002, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    HpaeNodeInfo outNodeInfo;
    inNodeInfo.frameLen = 480;
    inNodeInfo.samplingRate = static_cast<AudioSamplingRate>(0);
    outNodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    outNodeInfo.samplingRate = SAMPLE_RATE_48000;

    bool ret = IHpaeCapturerManager::AdjustFrameLen(outNodeInfo, inNodeInfo);

    EXPECT_FALSE(ret);
    EXPECT_EQ(outNodeInfo.frameLen, DEFAULT_FRAME_LENGTH);
}

/*
 * tc.name   : Test IHpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : AdjustFrameLen_003
 * tc.desc   : Test AdjustFrameLen returns false when output effective sample rate is zero.
 */
HWTEST_F(HpaeCapturerManagerTest, AdjustFrameLen_003, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    HpaeNodeInfo outNodeInfo;
    inNodeInfo.frameLen = 480;
    inNodeInfo.samplingRate = SAMPLE_RATE_48000;
    outNodeInfo.frameLen = DEFAULT_FRAME_LENGTH;
    outNodeInfo.samplingRate = static_cast<AudioSamplingRate>(0);

    bool ret = IHpaeCapturerManager::AdjustFrameLen(outNodeInfo, inNodeInfo);

    EXPECT_FALSE(ret);
    EXPECT_EQ(outNodeInfo.frameLen, DEFAULT_FRAME_LENGTH);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest_001
 * tc.desc   : Test HpaeCapturerManagerConstructTest
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerConstructTest, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    HpaeSourceInfo dstSourceInfo = capturerManager->GetSourceInfo();
    EXPECT_EQ(dstSourceInfo.deviceNetId == sourceInfo.deviceNetId, true);
    EXPECT_EQ(dstSourceInfo.deviceClass == sourceInfo.deviceClass, true);
    EXPECT_EQ(dstSourceInfo.frameLen == sourceInfo.frameLen, true);
    EXPECT_EQ(dstSourceInfo.samplingRate == sourceInfo.samplingRate, true);
    EXPECT_EQ(dstSourceInfo.format == sourceInfo.format, true);
    EXPECT_EQ(dstSourceInfo.channels == sourceInfo.channels, true);
    EXPECT_EQ(dstSourceInfo.ecType == sourceInfo.ecType, true);
    EXPECT_EQ(dstSourceInfo.micRef == sourceInfo.micRef, true);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest_002
 * tc.desc   : Test HpaeCapturerManagerInitTest
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerInitTest, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest_003
 * tc.desc   : Test HpaeCapturerManagerCreateDestoryStreamTest
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerCreateDestoryStreamTest, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    TestCheckSourceOutputInfo(sourceOutputInfo, streamInfo);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PREPARED);
    EXPECT_EQ(capturerManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(
        capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == ERR_INVALID_OPERATION, true);
    EXPECT_EQ(capturerManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

static void StateControlTest(std::shared_ptr<HpaeCapturerManager> &capturerManager, HpaeStreamInfo &streamInfo,
    HpaeSourceOutputInfo &sourceOutputInfo)
{
    EXPECT_EQ(capturerManager->Start(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(capturerManager->IsRunning(), true);
    
    EXPECT_EQ(capturerManager->Pause(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(capturerManager->IsRunning(), false);

    EXPECT_EQ(capturerManager->Start(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(capturerManager->IsRunning(), true);

    EXPECT_EQ(capturerManager->Stop(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(capturerManager->IsRunning(), false);

    EXPECT_EQ(capturerManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(
        capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == ERR_INVALID_OPERATION, true);
    EXPECT_EQ(capturerManager->IsRunning(), false);
}

/**
 * @tc.name  : Test HpaeCapturerManager Pause API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerManagerPauseWithStandby_001
 * @tc.desc  : Test Pause with isStandby=true for normal session
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerPauseWithStandby_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    std::shared_ptr<ReadDataCb> readDataCb =
        std::make_shared<ReadDataCb>(g_rootCapturerPath);
    EXPECT_EQ(capturerManager->RegisterReadCallback(streamInfo.sessionId, readDataCb), SUCCESS);

    EXPECT_EQ(capturerManager->Start(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(capturerManager->IsRunning(), true);

    EXPECT_EQ(capturerManager->Pause(streamInfo.sessionId, true) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(capturerManager->IsRunning(), false);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test HpaeCapturerManager Pause API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerManagerPauseWithStandby_002
 * @tc.desc  : Test Pause with isStandby=false for normal session
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerPauseWithStandby_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    std::shared_ptr<ReadDataCb> readDataCb =
        std::make_shared<ReadDataCb>(g_rootCapturerPath);
    EXPECT_EQ(capturerManager->RegisterReadCallback(streamInfo.sessionId, readDataCb), SUCCESS);

    EXPECT_EQ(capturerManager->Start(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_RUNNING);

    EXPECT_EQ(capturerManager->Pause(streamInfo.sessionId, false) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PAUSED);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest_004
 * tc.desc   : Test HpaeCapturerManagerStartStopTest
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerStartStopTest, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    TestCheckSourceOutputInfo(sourceOutputInfo, streamInfo);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PREPARED);
    EXPECT_EQ(capturerManager->IsRunning(), false);

    std::shared_ptr<ReadDataCb> readDataCb =
        std::make_shared<ReadDataCb>(g_rootCapturerPath);
    EXPECT_EQ(capturerManager->RegisterReadCallback(streamInfo.sessionId, readDataCb), SUCCESS);
    EXPECT_EQ(readDataCb.use_count() == 1, true);

    StateControlTest(capturerManager, streamInfo, sourceOutputInfo);
    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

static void InitReloadSourceInfo(HpaeSourceInfo &sourceInfo, HpaeSourceInfo &newSourceInfo)
{
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.filePath = g_rootCapturerPath;

    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    sourceInfo.micRef = HPAE_REF_OFF;

    newSourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    newSourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    newSourceInfo.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;
    newSourceInfo.filePath = g_rootCapturerPath;

    newSourceInfo.samplingRate = SAMPLE_RATE_48000;
    newSourceInfo.channels = STEREO;
    newSourceInfo.format = SAMPLE_S16LE;
    newSourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    newSourceInfo.ecType = HPAE_EC_TYPE_SAME_ADAPTER;
    newSourceInfo.micRef = HPAE_REF_OFF;
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest_005
 * tc.desc   : Test HpaeCapturerManagerReloadTest
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerReloadTest, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    HpaeSourceInfo newSourceInfo;
    InitReloadSourceInfo(sourceInfo, newSourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    TestCheckSourceOutputInfo(sourceOutputInfo, streamInfo);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PREPARED);
    EXPECT_EQ(capturerManager->ReloadCaptureManager(newSourceInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->ReloadCaptureManager(newSourceInfo, true) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test CreateOutputSession_001
 */
HWTEST_F(HpaeCapturerManagerTest, CreateOutputSession_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_MIC;

    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test CreateOutputSession_002
 */
HWTEST_F(HpaeCapturerManagerTest, CreateOutputSession_002, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_WAKEUP;

    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test CreateOutputSession_003
 */
HWTEST_F(HpaeCapturerManagerTest, CreateOutputSession_003, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_OFFLOAD_CAPTURE;

    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
    auto sourceOutputNode = capturerManager->sourceOutputNodeMap_[streamInfo.sessionId];
    EXPECT_NE(sourceOutputNode, nullptr);
    EXPECT_EQ(sourceOutputNode->GetNodeInfo().sourceBufferType, HPAE_SOURCE_BUFFER_TYPE_EC);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test DisConnectSceneClusterFromSourceInputCluster_001
 */
HWTEST_F(HpaeCapturerManagerTest, DisConnectSceneClusterFromSourceInputCluster_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeProcessorType sceneType = HPAE_SCENE_VOIP_UP;
    capturerManager->DisConnectSceneClusterFromSourceInputCluster(sceneType);
    EXPECT_NE(capturerManager, nullptr);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test DeleteOutputSession_001
 */
HWTEST_F(HpaeCapturerManagerTest, DeleteOutputSession_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->DeleteOutputSession(DEFAULT_SESSION_ID), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test CreateStream_001
 */
HWTEST_F(HpaeCapturerManagerTest, CreateStream_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo info;
    EXPECT_EQ(capturerManager->CreateStream(info), ERR_INVALID_OPERATION);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : CreateStream_UnprocessedVaOverLimit_001
 * tc.desc   : Test CreateStream rejects unprocessed VA stream whose concat total channels exceed max.
 */
HWTEST_F(HpaeCapturerManagerTest, CreateStream_UnprocessedVaOverLimit_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT;
    // 18: concat total over max 16 (CHANNEL_16 + 2), must fail synchronously.
    streamInfo.channels = static_cast<AudioChannel>(static_cast<uint32_t>(CHANNEL_16) + 2);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo), ERROR);
    // Synchronous rejection: no output node may be created for the rejected session.
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.count(streamInfo.sessionId), static_cast<size_t>(0));
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : CreateOutputSession_004
 * tc.desc   : Test CreateOutputSession defense gate rejects over-limit unprocessed VA without node insert.
 */
HWTEST_F(HpaeCapturerManagerTest, CreateOutputSession_004, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT;
    // 18: concat total over max 16 (CHANNEL_16 + 2).
    streamInfo.channels = static_cast<AudioChannel>(static_cast<uint32_t>(CHANNEL_16) + 2);

    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), ERROR);
    // A null map entry here is exactly the async lambda null-deref source; gate must insert nothing.
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.count(streamInfo.sessionId), static_cast<size_t>(0));
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : CreateOutputSession_005
 * tc.desc   : Test CreateOutputSession allows unprocessed VA with concat total channels exactly at max.
 */
HWTEST_F(HpaeCapturerManagerTest, CreateOutputSession_005, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT;
    // 16: concat total exactly at CHANNEL_16, allowed.
    streamInfo.channels = CHANNEL_16;

    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
    EXPECT_NE(capturerManager->sourceOutputNodeMap_[streamInfo.sessionId], nullptr);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test DestroyStream_001
 */
HWTEST_F(HpaeCapturerManagerTest, DestroyStream_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->DestroyStream(DEFAULT_SESSION_ID), ERR_INVALID_OPERATION);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test Flush_001
 */
HWTEST_F(HpaeCapturerManagerTest, Flush_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->Flush(DEFAULT_SESSION_ID), ERR_INVALID_OPERATION);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test Flush_002
 */
HWTEST_F(HpaeCapturerManagerTest, Flush_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->Flush(DEFAULT_SESSION_ID), SUCCESS);
    EXPECT_EQ(capturerManager->DeInit(DEFAULT_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(capturerManager);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test Drain_001
 */
HWTEST_F(HpaeCapturerManagerTest, Drain_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->Drain(DEFAULT_SESSION_ID), ERR_INVALID_OPERATION);
}
 
/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test Drain_002
 */
HWTEST_F(HpaeCapturerManagerTest, Drain_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->Drain(DEFAULT_SESSION_ID), SUCCESS);
    EXPECT_EQ(capturerManager->DeInit(DEFAULT_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(capturerManager);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test Release_001
 */
HWTEST_F(HpaeCapturerManagerTest, Release_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->Release(DEFAULT_SESSION_ID), ERR_INVALID_OPERATION);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test SetMute_001
 */
HWTEST_F(HpaeCapturerManagerTest, SetMute_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->SetMute(false), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test SetMute_002
 */
HWTEST_F(HpaeCapturerManagerTest, SetMute_002, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->SetMute(true), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test PrepareCapturerEc_001
 */
HWTEST_F(HpaeCapturerManagerTest, PrepareCapturerEc_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.ecType = HPAE_EC_TYPE_DIFF_ADAPTER;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    EXPECT_EQ(capturerManager->PrepareCapturerEc(nodeInfo), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test PrepareCapturerMicRef_001
 */
HWTEST_F(HpaeCapturerManagerTest, PrepareCapturerMicRef_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.micRef = HPAE_REF_ON;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    EXPECT_EQ(capturerManager->PrepareCapturerMicRef(nodeInfo), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test ReloadCaptureManager_001
 */
HWTEST_F(HpaeCapturerManagerTest, ReloadCaptureManager_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->ReloadCaptureManager(sourceInfo), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test ReloadCaptureManager_002
 */
HWTEST_F(HpaeCapturerManagerTest, ReloadCaptureManager_002, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    capturerManager->isInit_ = true;
    EXPECT_EQ(capturerManager->ReloadCaptureManager(sourceInfo), SUCCESS);
    EXPECT_EQ(capturerManager->ReloadCaptureManager(sourceInfo, true), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test DeInit_001
 */
HWTEST_F(HpaeCapturerManagerTest, DeInit_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->DeInit(true), ERR_INVALID_OPERATION);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test DeactivateThread_001
 */
HWTEST_F(HpaeCapturerManagerTest, DeactivateThread_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    EXPECT_EQ(capturerManager->DeactivateThread(), true);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test RegisterReadCallback_001
 */
HWTEST_F(HpaeCapturerManagerTest, RegisterReadCallback_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    std::shared_ptr<ReadDataCb> readDataCb =
        std::make_shared<ReadDataCb>(g_rootCapturerPath);
    EXPECT_EQ(capturerManager->RegisterReadCallback(DEFAULT_SESSION_ID, readDataCb), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test GetSourceOutputInfo_001
 */
HWTEST_F(HpaeCapturerManagerTest, GetSourceOutputInfo_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(DEFAULT_SESSION_ID, sourceOutputInfo), ERR_INVALID_OPERATION);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test GetAllSourceOutputsInfo_001
 */
HWTEST_F(HpaeCapturerManagerTest, GetAllSourceOutputsInfo_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    std::vector<SourceOutput> sourceOutputInfos = capturerManager->GetAllSourceOutputsInfo();
    EXPECT_EQ(sourceOutputInfos.size() == 0, true);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test OnNodeStatusUpdate_001
 */
HWTEST_F(HpaeCapturerManagerTest, OnNodeStatusUpdate_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    capturerManager->OnNodeStatusUpdate(DEFAULT_SESSION_ID, OPERATION_STOPPED);
    EXPECT_NE(capturerManager, nullptr);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test AddAllNodesToSource_001
 */
HWTEST_F(HpaeCapturerManagerTest, AddAllNodesToSource_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    std::vector<HpaeCaptureMoveInfo> moveInfos;
    EXPECT_EQ(capturerManager->AddAllNodesToSource(moveInfos, true), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test MoveAllStream_001
 */
HWTEST_F(HpaeCapturerManagerTest, MoveAllStream_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    std::vector<uint32_t> sessionIds;
    EXPECT_EQ(capturerManager->MoveAllStream(DEFAULT_SOURCE_NAME, sessionIds, MOVE_ALL), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test MoveAllStream_002
 */
HWTEST_F(HpaeCapturerManagerTest, MoveAllStream_002, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    capturerManager->isInit_ = true;
    std::vector<uint32_t> sessionIds;
    EXPECT_EQ(capturerManager->MoveAllStream(DEFAULT_SOURCE_NAME, sessionIds, MOVE_ALL), SUCCESS);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test MoveAllStreamToNewSource_001
 */
HWTEST_F(HpaeCapturerManagerTest, MoveAllStreamToNewSource_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    std::vector<uint32_t> moveIds;
    capturerManager->MoveAllStreamToNewSource(DEFAULT_SOURCE_NAME, moveIds, MOVE_SINGLE);
    EXPECT_NE(capturerManager, nullptr);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test MoveAllStreamToNewSource_002
 */
HWTEST_F(HpaeCapturerManagerTest, MoveAllStreamToNewSource_002, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    std::vector<uint32_t> moveIds;
    capturerManager->MoveAllStreamToNewSource(DEFAULT_SOURCE_NAME, moveIds, MOVE_ALL);
    EXPECT_NE(capturerManager, nullptr);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test OnRequestLatency_001
 */
HWTEST_F(HpaeCapturerManagerTest, OnRequestLatency_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    uint64_t latency = 0;
    capturerManager->OnRequestLatency(DEFAULT_SESSION_ID, latency);
    EXPECT_NE(capturerManager, nullptr);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test DumpSourceInfo_001
 */
HWTEST_F(HpaeCapturerManagerTest, DumpSourceInfo_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    capturerManager->DumpSourceInfo();
    EXPECT_NE(capturerManager, nullptr);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test GetDeviceHDFDumpInfo_001
 */
HWTEST_F(HpaeCapturerManagerTest, GetDeviceHDFDumpInfo_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    std::string config = capturerManager->GetDeviceHDFDumpInfo();
    std::string info;
    TransDeviceInfoToString(sourceInfo, info);
    EXPECT_EQ(config == info, true);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test CheckEcAndMicRefCondition_001
 */
HWTEST_F(HpaeCapturerManagerTest, CheckEcAndMicRefCondition_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.ecType = HPAE_EC_TYPE_SAME_ADAPTER;
    sourceInfo.micRef = HPAE_REF_ON;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeProcessorType sceneType = HPAE_SCENE_VOIP_UP;
    HpaeNodeInfo ecNodeInfo;
    HpaeSourceInputNodeType ecNodeType = HPAE_SOURCE_DEFAULT;
    EXPECT_EQ(capturerManager->CheckEcCondition(sceneType, ecNodeInfo, ecNodeType), false);

    HpaeNodeInfo micRefNodeInfo;
    EXPECT_EQ(capturerManager->CheckMicRefCondition(sceneType, micRefNodeInfo), false);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test InitCaptureManager_001
 */
HWTEST_F(HpaeCapturerManagerTest, InitCaptureManager_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    EXPECT_EQ(capturerManager->InitCapturerManager() == SUCCESS, true);

    auto sourceInputCluster = capturerManager->sourceInputClusterMap_[HPAE_SOURCE_MIC];
    EXPECT_NE(sourceInputCluster, nullptr);
    EXPECT_NE(sourceInputCluster->GetSourceInputNodeType(), HPAE_SOURCE_OFFLOAD);
}

/**
 * @tc.name  : Test SendRequestInner_001
 * @tc.type  : FUNC
 * @tc.number: SendRequestInner_001
 * @tc.desc  : Test SendRequestInner when config in vaild.
 */
HWTEST_F(HpaeCapturerManagerTest, SendRequestInner_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.ecType = HPAE_EC_TYPE_SAME_ADAPTER;
    sourceInfo.micRef = HPAE_REF_ON;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    auto request = []() {
    };
    capturerManager->SendRequest(request, "unit_test_send_request");
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    capturerManager->SendRequest(request, "unit_test_send_request");
    WaitForMsgProcessing(capturerManager);
    capturerManager->hpaeSignalProcessThread_ = nullptr;
    capturerManager->SendRequest(request, "unit_test_send_request");
    EXPECT_EQ(capturerManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test InitCapturerManager_001
 * @tc.type  : FUNC
 * @tc.number: InitCapturerManager_001
 * @tc.desc  : Test InitCapturerManager when frameLen is 0.
 */
HWTEST_F(HpaeCapturerManagerTest, InitCapturerManager_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.frameLen = 0;
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->InitCapturerManager(), ERROR);
}

/**
 * @tc.name  : Test InitCapturerManager_002
 * @tc.type  : FUNC
 * @tc.number: InitCapturerManager_002
 * @tc.desc  : Test InitCapturerManager when frameLen is over-sized.
 */
HWTEST_F(HpaeCapturerManagerTest, InitCapturerManager_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.frameLen = OVERSIZED_FRAME_LENGTH;
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->InitCapturerManager(), ERROR);
}

/**
 * @tc.name  : Test CreateStream_002
 * @tc.type  : FUNC
 * @tc.number: CreateStream_002
 * @tc.desc  : Test CreateStream when frameLen is 0.
 */
HWTEST_F(HpaeCapturerManagerTest, CreateStream_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.frameLen = 0;
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo), ERROR);
}

/**
 * @tc.name  : Test InitCapturerManager_003
 * @tc.type  : FUNC
 * @tc.number: InitCapturerManager_003
 * @tc.desc  : Test InitCapturerManager when frameLen is over-sized.
 */
HWTEST_F(HpaeCapturerManagerTest, CreateStream_003, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.frameLen = OVERSIZED_FRAME_LENGTH;
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo), ERROR);
}

/**
 * @tc.name  : Test AddRemoveCaptureInjectorTest
 * @tc.type  : FUNC
 * @tc.number: AddRemoveCaptureInjectorTest
 * @tc.desc  : Test AddCapturerInjector and RemoveCapturerInjector func
 */
HWTEST_F(HpaeCapturerManagerTest, AddRemoveCaptureInjectorTest, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_MIC;
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    
    HpaeProcessorType sceneType = TransSourceTypeToSceneType(streamInfo.sourceType);
    auto it = capturerManager->sceneClusterMap_.find(sceneType);
    ASSERT_EQ(it != capturerManager->sceneClusterMap_.end(), true);
    auto sceneCluster = it->second;
    ASSERT_EQ(sceneCluster != nullptr, true);
    HpaeNodeInfo nodeInfo;
    nodeInfo.deviceClass = sourceInfo.deviceClass;
    nodeInfo.channels = sourceInfo.channels;
    nodeInfo.format = sourceInfo.format;
    nodeInfo.frameLen = sourceInfo.frameLen;
    nodeInfo.samplingRate = sourceInfo.samplingRate;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.statusCallback = capturerManager;
    std::shared_ptr<HpaeSourceInputNode> preNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(capturerManager->AddCaptureInjector(preNode, streamInfo.sourceType), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(preNode.use_count(), 2);
    EXPECT_EQ(capturerManager->RemoveCaptureInjector(preNode, streamInfo.sourceType), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(preNode.use_count(), 1);
}

/**
 * @tc.name  : Test AddRemoveCaptureInjectorTest
 * @tc.type  : FUNC
 * @tc.number: AddRemoveCaptureInjectorTest_002
 * @tc.desc  : Test AddCapturerInjector sourceType not exit
 */
HWTEST_F(HpaeCapturerManagerTest, AddRemoveCaptureInjectorTest_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.deviceClass = sourceInfo.deviceClass;
    nodeInfo.channels = sourceInfo.channels;
    nodeInfo.format = sourceInfo.format;
    nodeInfo.frameLen = sourceInfo.frameLen;
    nodeInfo.samplingRate = sourceInfo.samplingRate;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.statusCallback = capturerManager;
    std::shared_ptr<HpaeSourceInputNode> preNode = std::make_shared<HpaeSourceInputNode>(nodeInfo);
    EXPECT_EQ(capturerManager->AddCaptureInjector(preNode, SOURCE_TYPE_VOICE_MESSAGE), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(preNode.use_count(), 1);
    EXPECT_EQ(capturerManager->RemoveCaptureInjector(preNode, SOURCE_TYPE_VOICE_MESSAGE), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(preNode.use_count(), 1);
}

/**
 * @tc.name  : Test Process
 * @tc.type  : FUNC
 * @tc.number: Process_001
 * @tc.desc  : Test Process.
 */
HWTEST_F(HpaeCapturerManagerTest, Process_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_MIC;
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->Start(DEFAULT_SESSION_ID) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsRunning(), true);
    EXPECT_EQ(capturerManager->Stop(DEFAULT_SESSION_ID) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsRunning(), false);
    capturerManager->sourceInputClusterMap_[capturerManager->mainMicType_]->CapturerSourceStart();
    capturerManager->Process();
    EXPECT_EQ(capturerManager->IsRunning(), false);
}

/*
 * tc.name   : Test HpaeCapturerManager SetStreamMute API
 * tc.type   : FUNC
 * tc.number : SetStreamMuteValidParamsTest
 * tc.desc   : Test SetStreamMute functionality
 */
HWTEST_F(HpaeCapturerManagerTest, SetStreamMuteValidParamsTest, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    
    int32_t result = capturerManager->SetStreamMute(streamInfo.sessionId, true);
    EXPECT_EQ(result == SUCCESS, true);

    result = capturerManager->SetStreamMute(streamInfo.sessionId, false);
    EXPECT_EQ(result == SUCCESS, true);
    
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
}

/*
 * tc.name   : Test HpaeCapturerManager SetMute API true
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerSetMute_001
 * tc.desc   : Test SetMute functionality
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerSetMute_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    
    EXPECT_EQ(capturerManager->SetMute(true), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->isMute_, true);
}

/*
 * tc.name   : Test HpaeCapturerManager SetMute API false
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerSetMute_002
 * tc.desc   : Test SetMute functionality
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerSetMute_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    
    EXPECT_EQ(capturerManager->SetMute(false), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->isMute_, false);
}

/*
 * tc.name   : Test HpaeCapturerManager PrepareCapturerEc API
 * tc.type   : FUNC
 * tc.number : PrepareCapturerEc_DiffAdapter_Test
 * tc.desc   : Test PrepareCapturerEc with HPAE_EC_TYPE_DIFF_ADAPTER
 */
HWTEST_F(HpaeCapturerManagerTest, PrepareCapturerEc_DiffAdapter_Test, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.ecType = HPAE_EC_TYPE_DIFF_ADAPTER;
    sourceInfo.ecFrameLen = 1024; // 1024 for test framelen
    sourceInfo.ecChannels = MONO;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    
    HpaeNodeInfo ecNodeInfo;
    int32_t result = capturerManager->PrepareCapturerEc(ecNodeInfo); // ecNodeInfo will change
    
    EXPECT_EQ(result == SUCCESS, true);
    EXPECT_EQ(ecNodeInfo.frameLen == sourceInfo.ecFrameLen, true);
    EXPECT_EQ(ecNodeInfo.channels == sourceInfo.ecChannels, true);
    EXPECT_EQ(ecNodeInfo.format == sourceInfo.ecFormat, true);
    EXPECT_EQ(ecNodeInfo.samplingRate == sourceInfo.ecSamplingRate, true);
    EXPECT_EQ(ecNodeInfo.sourceBufferType == HPAE_SOURCE_BUFFER_TYPE_EC, true);
    EXPECT_EQ(ecNodeInfo.sourceInputNodeType == HPAE_SOURCE_EC, true);
}

/*
 * tc.name   : Test HpaeCapturerManager PrepareCapturerEc API
 * tc.type   : FUNC
 * tc.number : PrepareCapturerEc_NoEc_Test
 * tc.desc   : Test PrepareCapturerEc with HPAE_EC_TYPE_NONE
 */
HWTEST_F(HpaeCapturerManagerTest, PrepareCapturerEc_NoEc_Test, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    
    HpaeNodeInfo ecNodeInfo;
    ecNodeInfo.frameLen = 512; // 512 for test framelen
    ecNodeInfo.channels = STEREO;
    ecNodeInfo.format = SAMPLE_S32LE;
    ecNodeInfo.samplingRate = SAMPLE_RATE_48000;
    
    int32_t result = capturerManager->PrepareCapturerEc(ecNodeInfo); // ecNodeInfo will not change
    
    EXPECT_EQ(result == SUCCESS, true);
    // Verify node info is not modified when ecType is NONE
    EXPECT_EQ(ecNodeInfo.frameLen == 512, true);
    EXPECT_EQ(ecNodeInfo.channels == STEREO, true);
    EXPECT_EQ(ecNodeInfo.format == SAMPLE_S32LE, true);
    EXPECT_EQ(ecNodeInfo.samplingRate == SAMPLE_RATE_48000, true);
}

/*
 * tc.name   : Test HpaeCapturerManager PrepareCapturerEc API
 * tc.type   : FUNC
 * tc.number : PrepareCapturerEc_SameAdapter_Test
 * tc.desc   : Test PrepareCapturerEc with HPAE_EC_TYPE_SAME_ADAPTER
 */
HWTEST_F(HpaeCapturerManagerTest, PrepareCapturerEc_SameAdapter_Test, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.ecType = HPAE_EC_TYPE_SAME_ADAPTER;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    
    HpaeNodeInfo ecNodeInfo;
    ecNodeInfo.frameLen = 512; // 512 for test framelen
    ecNodeInfo.channels = STEREO;
    
    int32_t result = capturerManager->PrepareCapturerEc(ecNodeInfo); // ecNodeInfo will not change
    
    EXPECT_EQ(result == SUCCESS, true);
    // Verify node info is not modified when ecType is SAME_ADAPTER
    EXPECT_EQ(ecNodeInfo.frameLen == 512, true); // 512 for test framelen
    EXPECT_EQ(ecNodeInfo.channels == STEREO, true);
}

/*
 * tc.name   : Test HpaeCapturerManager PrepareCapturerMicRef API
 * tc.type   : FUNC
 * tc.number : PrepareCapturerMicRef_RefOn_Test
 * tc.desc   : Test PrepareCapturerMicRef with HPAE_REF_ON
 */
HWTEST_F(HpaeCapturerManagerTest, PrepareCapturerMicRef_RefOn_Test, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.micRef = HPAE_REF_ON;
    sourceInfo.micRefFrameLen = 512; // 512 for test framelen
    sourceInfo.micRefChannels = MONO;
    sourceInfo.micRefFormat = SAMPLE_S16LE;
    sourceInfo.micRefSamplingRate = SAMPLE_RATE_16000;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    
    HpaeNodeInfo micRefNodeInfo;
    int32_t result = capturerManager->PrepareCapturerMicRef(micRefNodeInfo); // micRefNodeInfo will change
    
    EXPECT_EQ(result == SUCCESS, true);
    EXPECT_EQ(micRefNodeInfo.frameLen == sourceInfo.micRefFrameLen, true);
    EXPECT_EQ(micRefNodeInfo.channels == sourceInfo.micRefChannels, true);
    EXPECT_EQ(micRefNodeInfo.format == sourceInfo.micRefFormat, true);
    EXPECT_EQ(micRefNodeInfo.samplingRate == sourceInfo.micRefSamplingRate, true);
    EXPECT_EQ(micRefNodeInfo.sourceBufferType == HPAE_SOURCE_BUFFER_TYPE_MICREF, true);
    EXPECT_EQ(micRefNodeInfo.sourceInputNodeType == HPAE_SOURCE_MICREF, true);
}

/*
 * tc.name   : Test HpaeCapturerManager PrepareCapturerMicRef API
 * tc.type   : FUNC
 * tc.number : PrepareCapturerMicRef_RefOff_Test
 * tc.desc   : Test PrepareCapturerMicRef with HPAE_REF_OFF
 */
HWTEST_F(HpaeCapturerManagerTest, PrepareCapturerMicRef_RefOff_Test, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.micRef = HPAE_REF_OFF;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    
    HpaeNodeInfo micRefNodeInfo;
    micRefNodeInfo.frameLen = 1024; // 1024 for test framelen
    micRefNodeInfo.channels = STEREO;
    micRefNodeInfo.format = SAMPLE_S32LE;
    micRefNodeInfo.samplingRate = SAMPLE_RATE_48000;
    
    int32_t result = capturerManager->PrepareCapturerMicRef(micRefNodeInfo); // micRefNodeInfo will not change
    
    EXPECT_EQ(result == SUCCESS, true);
    // Verify node info is not modified when micRef is OFF
    EXPECT_EQ(micRefNodeInfo.frameLen == 1024, true); // 1024 for test framelen
    EXPECT_EQ(micRefNodeInfo.channels == STEREO, true);
    EXPECT_EQ(micRefNodeInfo.format == SAMPLE_S32LE, true);
    EXPECT_EQ(micRefNodeInfo.samplingRate == SAMPLE_RATE_48000, true);
}

/*
 * tc.name   : Test HpaeCapturerManager CreateSourceAttr API
 * tc.type   : FUNC
 * tc.number : CreateSourceAttr_BasicInfo_Test
 * tc.desc   : Test CreateSourceAttr with basic source info
 */
HWTEST_F(HpaeCapturerManagerTest, CreateSourceAttr_BasicInfo_Test, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.adapterName = "primary";
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.channelLayout = 0;
    sourceInfo.deviceType = DEVICE_TYPE_MIC;
    sourceInfo.volume = 0.f;
    sourceInfo.deviceNetId = "network123";
    sourceInfo.filePath = "/data/test.pcm";
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.openMicSpeaker = true;
    sourceInfo.routeFlag = AUDIO_INPUT_FLAG_LIVE;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    
    IAudioSourceAttr attr;
    capturerManager->CreateSourceAttr(attr);

    EXPECT_EQ(attr.adapterName == sourceInfo.adapterName, true);
    EXPECT_EQ(attr.sampleRate == sourceInfo.samplingRate, true);
    EXPECT_EQ(attr.channel == sourceInfo.channels, true);
    EXPECT_EQ(attr.format == sourceInfo.format, true);
    EXPECT_EQ(attr.channelLayout == sourceInfo.channelLayout, true);
    EXPECT_EQ(attr.deviceType == sourceInfo.deviceType, true);
    EXPECT_EQ(attr.volume == sourceInfo.volume, true);
    EXPECT_EQ(std::string(attr.deviceNetworkId) == sourceInfo.deviceNetId, true);
    EXPECT_EQ(std::string(attr.filePath) == sourceInfo.filePath, true);
    EXPECT_EQ(attr.isBigEndian == false, true);
    EXPECT_EQ(attr.sourceType == static_cast<int32_t>(sourceInfo.sourceType), true);
    EXPECT_EQ(attr.openMicSpeaker == sourceInfo.openMicSpeaker, true);
    EXPECT_EQ(attr.routeFlag == sourceInfo.routeFlag, true);
}

/**
 * @tc.name  : Test HpaeCapturerManager DeleteOutputSession API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerDeleteOutputSession_001
 * @tc.desc  : Test HpaeCapturerManager DeleteOutputSession
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerDeleteOutputSession_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 1);

    capturerManager->DeleteOutputSession(streamInfo.sessionId);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 0);
}

/**
 * @tc.name  : Test HpaeCapturerManager DeleteOutputSession API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerDeleteOutputSession_002
 * @tc.desc  : Test HpaeCapturerManager DeleteOutputSession not exit
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerDeleteOutputSession_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 1);

    capturerManager->DeleteOutputSession(streamInfo.sessionId + 1);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 1);
}

/**
 * @tc.name  : Test HpaeCapturerManager DeleteOutputSession API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerDeleteOutputSession_003
 * @tc.desc  : Test HpaeCapturerManager DeleteOutputSession not delete all
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerDeleteOutputSession_003, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    size_t size = 10; // 10 for test size
    HpaeStreamInfo streamInfo;
    for (size_t i = 0; i < size; i++) {
        streamInfo.sessionId = i;
        EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
    }
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), size);
    size_t deleteSize = 5; // 5 for delete size
    for (size_t i = 0; i < deleteSize; i++) {
        capturerManager->DeleteOutputSession(i);
    }
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), size - deleteSize);
}

/**
 * @tc.name  : Test HpaeCapturerManager GetThreadName API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerGetThreadNameTest
 * @tc.desc  : Test HpaeCapturerManager GetThreadName
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerGetThreadNameTest, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    std::string threadName = capturerManager->GetThreadName();
    EXPECT_EQ(threadName, sourceInfo.deviceName);
}

/**
 * @tc.name  : Test HpaeCapturerManager CheckIfAnyStreamRunning API
 * @tc.type  : FUNC
 * @tc.number: HpaeCaptureCheckIfAnyStreamRunningTest_001
 * @tc.desc  : Test CheckIfAnyStreamRunning with no sessions
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCaptureCheckIfAnyStreamRunningTest_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    capturerManager->CheckIfAnyStreamRunning();
    EXPECT_EQ(capturerManager->IsRunning(), false);
    EXPECT_EQ(capturerManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerManager CheckIfAnyStreamRunning API
 * @tc.type  : FUNC
 * @tc.number: HpaeCaptureCheckIfAnyStreamRunningTest_002
 * @tc.desc  : Test CheckIfAnyStreamRunning with running session
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCaptureCheckIfAnyStreamRunningTest_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsRunning(), false);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateOutputSession(streamInfo);
    auto sourceOutput = capturerManager->sourceOutputNodeMap_.find(streamInfo.sessionId);
    ASSERT_EQ(sourceOutput != capturerManager->sourceOutputNodeMap_.end(), true);
    auto sourceOutputNode = sourceOutput->second;
    ASSERT_EQ(sourceOutputNode != nullptr, true); // make sure sessionId exit in sourceOutputMap
    capturerManager->SetSessionState(streamInfo.sessionId, HPAE_SESSION_RUNNING);

    EXPECT_EQ(capturerManager->IsRunning(), false);
    capturerManager->CheckIfAnyStreamRunning();
    EXPECT_EQ(capturerManager->IsRunning(), true);
    EXPECT_EQ(capturerManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerManager CheckIfAnyStreamRunning API
 * @tc.type  : FUNC
 * @tc.number: HpaeCaptureCheckIfAnyStreamRunningTest_003
 * @tc.desc  : Test CheckIfAnyStreamRunning with no running session
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCaptureCheckIfAnyStreamRunningTest_003, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsRunning(), false);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    capturerManager->CreateOutputSession(streamInfo);

    EXPECT_EQ(capturerManager->IsRunning(), false);
    capturerManager->CheckIfAnyStreamRunning();
    EXPECT_EQ(capturerManager->IsRunning(), false);
    EXPECT_EQ(capturerManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerManager IsRunning API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerIsRunningTest_001
 * @tc.desc  : Test HpaeCapturerManager IsRunning with sourceinputnode is null
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerIsRunningTest_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->sourceInputClusterMap_.size(), 0); // no sourceInputNode
    EXPECT_EQ(capturerManager->IsRunning(), false);
}

/**
 * @tc.name  : Test HpaeCapturerManager IsRunning API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerIsRunningTest_002
 * @tc.desc  : Test HpaeCapturerManager IsRunning with processThread is null
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerIsRunningTest_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_NE(capturerManager->sourceInputClusterMap_.size(), 0); // sourceIn is not null
    capturerManager->hpaeSignalProcessThread_ = nullptr;
    EXPECT_EQ(capturerManager->IsRunning(), false);
}

/**
 * @tc.name  : Test HpaeCapturerManager IsRunning API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerIsRunningTest_003
 * @tc.desc  : Test HpaeCapturerManager IsRunning with processThread is not run
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerIsRunningTest_003, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_NE(capturerManager->sourceInputClusterMap_.size(), 0); // sourceIn is not null
    EXPECT_EQ(capturerManager->hpaeSignalProcessThread_ != nullptr, true);
    capturerManager->hpaeSignalProcessThread_->DeactivateThread();
    EXPECT_EQ(capturerManager->hpaeSignalProcessThread_->IsRunning(), false);
    EXPECT_EQ(capturerManager->IsRunning(), false);
}

/**
 * @tc.name  : Test HpaeCapturerManager IsRunning API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerIsRunningTest_004
 * @tc.desc  : Test HpaeCapturerManager IsRunning with sourceInput is not run
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerIsRunningTest_004, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_NE(capturerManager->sourceInputClusterMap_.size(), 0); // sourceIn is not null
    EXPECT_EQ(capturerManager->hpaeSignalProcessThread_ != nullptr, true);
    EXPECT_EQ(capturerManager->hpaeSignalProcessThread_->IsRunning(), true);

    auto micType = capturerManager->mainMicType_;
    EXPECT_EQ(micType, HPAE_SOURCE_MIC);
    auto sourceInput = capturerManager->sourceInputClusterMap_.find(micType);
    EXPECT_EQ(sourceInput != capturerManager->sourceInputClusterMap_.end(), true);
    auto sourceInputCluster = sourceInput->second;
    EXPECT_EQ(sourceInputCluster != nullptr, true);
    EXPECT_NE(sourceInputCluster->GetSourceState(), STREAM_MANAGER_RUNNING); // sourceIn not run
    EXPECT_EQ(capturerManager->IsRunning(), false);
}

/**
 * @tc.name  : Test HpaeCapturerManager IsRunning API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerIsRunningTest_005
 * @tc.desc  : Test HpaeCapturerManager IsRunning true
 */
HWTEST_F(HpaeCapturerManagerTest, HpaeCapturerIsRunningTest_005, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_NE(capturerManager->sourceInputClusterMap_.size(), 0); // sourceIn is not null
    EXPECT_EQ(capturerManager->hpaeSignalProcessThread_ != nullptr, true);
    EXPECT_EQ(capturerManager->hpaeSignalProcessThread_->IsRunning(), true);

    auto micType = capturerManager->mainMicType_;
    EXPECT_EQ(micType, HPAE_SOURCE_MIC);
    auto sourceInput = capturerManager->sourceInputClusterMap_.find(micType);
    ASSERT_EQ(sourceInput != capturerManager->sourceInputClusterMap_.end(), true);
    auto sourceInputCluster = sourceInput->second;
    ASSERT_EQ(sourceInputCluster != nullptr, true);
    EXPECT_EQ(sourceInputCluster->CapturerSourceStart(), SUCCESS);
    EXPECT_EQ(sourceInputCluster->GetSourceState(), STREAM_MANAGER_RUNNING); // sourceIn is run
    EXPECT_EQ(capturerManager->IsRunning(), true);
}

/**
 * @tc.name  : Test HpaeCapturerManager NotifyStreamChangeToSource API
 * @tc.type  : FUNC
 * @tc.number: NotifyStreamChangeToSource_001
 * @tc.desc  : Test HpaeCapturerManager NotifyStreamChangeToSource() cases
 */
HWTEST_F(HpaeCapturerManagerTest, NotifyStreamChangeToSource_001, TestSize.Level2)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(SUCCESS, capturerManager->Init());
    WaitForMsgProcessing(capturerManager);

    capturerManager->NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, DEFAULT_SESSION_ID, CAPTURER_PREPARED);
    EXPECT_EQ(0, capturerManager->sourceOutputNodeMap_.size());

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 1);

    capturerManager->NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, streamInfo.sessionId, CAPTURER_PREPARED);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 1);
}

/**
* @tc.name  : Test HpaeCapturerManager TriggerAppsUidUpdate API
* @tc.type  : FUNC
* @tc.number: TriggerAppsUidUpdate_001
* @tc.desc  : Test HpaeCapturerManager TriggerAppsUidUpdate() cases
*/
HWTEST_F(HpaeCapturerManagerTest, TriggerAppsUidUpdate_001, TestSize.Level2)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.deviceClass = "remote";
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(SUCCESS, capturerManager->Init());
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);

    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
    auto sourceOutputNode = capturerManager->sourceOutputNodeMap_[streamInfo.sessionId];
    EXPECT_NE(sourceOutputNode, nullptr);
    capturerManager->sourceOutputNodeMap_[streamInfo.sessionId]->state_ = HPAE_SESSION_RUNNING;
    capturerManager->TriggerAppsUidUpdate(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->appsUid_.size(), 1);
    capturerManager->TriggerAppsUidUpdate(0);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->appsUid_.size(), 1);
    capturerManager->sourceOutputNodeMap_[streamInfo.sessionId]->state_ = HPAE_SESSION_STOPPED;
    capturerManager->TriggerAppsUidUpdate(streamInfo.sessionId);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->appsUid_.size(), 1);
    capturerManager->TriggerAppsUidUpdate(0);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->appsUid_.size(), 0);
}

/**
 * @tc.name  : Test HpaeCapturerManager SetAppsEnhanceMuteState API
 * @tc.type  : FUNC
 * @tc.number: HandleAppsEnhanceMuteState_001
 * @tc.desc  : Test HpaeCapturerManager SetAppsEnhanceMuteState with invalid session
 */
HWTEST_F(HpaeCapturerManagerTest, HandleAppsEnhanceMuteState_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(SUCCESS, capturerManager->Init());
    WaitForMsgProcessing(capturerManager);

    // Test with invalid session id
    uint32_t invalidSessionId = DEFAULT_SESSION_ID;
    EXPECT_EQ(SUCCESS, capturerManager->SetAppsEnhanceMuteState(invalidSessionId, true));
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(SUCCESS, capturerManager->SetAppsEnhanceMuteState(invalidSessionId, false));
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test HpaeCapturerManager SetAppsEnhanceMuteState API
 * @tc.type  : FUNC
 * @tc.number: HandleAppsEnhanceMuteState_002
 * @tc.desc  : Test HpaeCapturerManager SetAppsEnhanceMuteState with valid session
 */
HWTEST_F(HpaeCapturerManagerTest, HandleAppsEnhanceMuteState_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(SUCCESS, capturerManager->Init());
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    // Test with valid session id
    EXPECT_EQ(SUCCESS, capturerManager->SetAppsEnhanceMuteState(streamInfo.sessionId, true));
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(SUCCESS, capturerManager->SetAppsEnhanceMuteState(streamInfo.sessionId, false));
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test HpaeCapturerManager SetAppsEnhanceMuteState API
 * @tc.type  : FUNC
 * @tc.number: HandleAppsEnhanceMuteState_003
 * @tc.desc  : Test HpaeCapturerManager SetAppsEnhanceMuteState with multiple mute states
 */
HWTEST_F(HpaeCapturerManagerTest, HandleAppsEnhanceMuteState_003, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(SUCCESS, capturerManager->Init());
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    // Test multiple calls with different states
    EXPECT_EQ(SUCCESS, capturerManager->SetAppsEnhanceMuteState(streamInfo.sessionId, true));
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(SUCCESS, capturerManager->SetAppsEnhanceMuteState(streamInfo.sessionId, false));
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(SUCCESS, capturerManager->SetAppsEnhanceMuteState(streamInfo.sessionId, true));
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(SUCCESS, capturerManager->SetAppsEnhanceMuteState(streamInfo.sessionId, false));
    WaitForMsgProcessing(capturerManager);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test SourceTypeVoiceRecognitionConstruct_001
 */
HWTEST_F(HpaeCapturerManagerTest, SourceTypeVoiceRecognitionConstruct_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    EXPECT_EQ(capturerManager->sourceInfo_.micInSamplingRate, SAMPLE_RATE_16000);
    EXPECT_EQ(capturerManager->sourceInfo_.micInFormat, SAMPLE_S16LE);
    EXPECT_EQ(capturerManager->sourceInfo_.micInChannels, CHANNEL_4);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test SourceTypeVoiceRecognitionConstruct_002
 */
HWTEST_F(HpaeCapturerManagerTest, SourceTypeVoiceRecognitionConstruct_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    EXPECT_NE(capturerManager->sourceInfo_.micInSamplingRate, SAMPLE_RATE_16000);
    EXPECT_NE(capturerManager->sourceInfo_.micInFormat, SAMPLE_S16LE);
    EXPECT_NE(capturerManager->sourceInfo_.micInChannels, CHANNEL_4);
}

/*
 * tc.name   : Test HpaeCapturerManager UpdateSourceAttr API
 * tc.type   : FUNC
 * tc.number : UpdateSourceAttr_001
 * tc.desc   : Test UpdateSourceAttr with voice recognition source type
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceAttr_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    
    IAudioSourceAttr attr;
    capturerManager->CreateSourceAttr(attr);
    capturerManager->UpdateSourceAttr(attr);

    EXPECT_EQ(attr.formatMicIn == sourceInfo.micInFormat, true);
    EXPECT_EQ(attr.sampleRateMicIn == sourceInfo.micInSamplingRate, true);
    EXPECT_EQ(attr.channelMicIn == sourceInfo.micInChannels, true);
}

/*
 * tc.name   : Test HpaeCapturerManager UpdateSourceAttr API
 * tc.type   : FUNC
 * tc.number : UpdateSourceAttr_002
 * tc.desc   : Test UpdateSourceAttr without voice recognition source type
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceAttr_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    
    IAudioSourceAttr attr;
    capturerManager->CreateSourceAttr(attr);
    capturerManager->UpdateSourceAttr(attr);

    EXPECT_EQ(attr.sampleRate == sourceInfo.samplingRate, true);
    EXPECT_EQ(attr.channel == sourceInfo.channels, true);
    EXPECT_EQ(attr.format == sourceInfo.format, true);
}

/*
 * tc.name   : Test HpaeCapturerManager UpdateSourceAttr API
 * tc.type   : FUNC
 * tc.number : UpdateSourceAttr_003
 * tc.desc   : Test UpdateSourceAttr with hasEcConfig
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceAttr_003, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecChannels = CHANNEL_4;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->mainMicType_ = HPAE_SOURCE_MIC_EC;
    
    IAudioSourceAttr attr;
    capturerManager->CreateSourceAttr(attr);
    capturerManager->UpdateSourceAttr(attr);

    EXPECT_EQ(attr.formatEc == sourceInfo.ecFormat, true);
    EXPECT_EQ(attr.sampleRateEc == sourceInfo.ecSamplingRate, true);
    EXPECT_EQ(attr.channelEc == sourceInfo.ecChannels, true);
}

/*
 * tc.name   : Test HpaeCapturerManager UpdateSourceAttr API
 * tc.type   : FUNC
 * tc.number : UpdateSourceAttr_004
 * tc.desc   : Test UpdateSourceAttr with hasEcConfig
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceAttr_004, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_MIC;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->mainMicType_ = HPAE_SOURCE_MIC;
    
    IAudioSourceAttr attr;
    capturerManager->CreateSourceAttr(attr);
    capturerManager->UpdateSourceAttr(attr);

    EXPECT_EQ(attr.format == sourceInfo.format, true);
    EXPECT_EQ(attr.sampleRate == sourceInfo.samplingRate, true);
    EXPECT_EQ(attr.channel == sourceInfo.channels, true);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test UpdateSourceInputNodeInfoForRecognition_001
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceInputNodeInfoForRecognition_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    nodeInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    capturerManager->UpdateSourceInputNodeInfoForRecognition(nodeInfo);

    EXPECT_EQ((uint8_t)nodeInfo.channels, (uint8_t)sourceInfo.micInChannels + (uint8_t)sourceInfo.channels);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test UpdateSourceInputNodeInfoForRecognition_002
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceInputNodeInfoForRecognition_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    capturerManager->UpdateSourceInputNodeInfoForRecognition(nodeInfo);

    EXPECT_EQ(nodeInfo.channels == STEREO, true);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test UpdateSourceInputNodeInfoForRecognition_003
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceInputNodeInfoForRecognition_003, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_16;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    capturerManager->UpdateSourceInputNodeInfoForRecognition(nodeInfo);

    EXPECT_EQ(nodeInfo.channels == (AudioChannel)((uint8_t)sourceInfo.micInChannels + (uint8_t)sourceInfo.channels),
        false);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test UpdateClusterNodeInfoForRecognition_001
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateClusterNodeInfoForRecognition_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    nodeInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    capturerManager->UpdateClusterNodeInfoForRecognition(nodeInfo);

    uint8_t totalChannels = static_cast<uint8_t>(sourceInfo.micInChannels) +
        static_cast<uint8_t>(sourceInfo.channels) + static_cast<uint8_t>(sourceInfo.ecChannels);
    EXPECT_EQ((uint8_t)nodeInfo.channels, totalChannels);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test UpdateClusterNodeInfoForRecognition_002
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateClusterNodeInfoForRecognition_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    capturerManager->UpdateClusterNodeInfoForRecognition(nodeInfo);

    EXPECT_EQ(nodeInfo.channels == STEREO, true);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeCapturerManagerTest
 * tc.desc   : Test UpdateClusterNodeInfoForRecognition_003
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateClusterNodeInfoForRecognition_003, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_16;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecChannels = CHANNEL_16;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    capturerManager->UpdateClusterNodeInfoForRecognition(nodeInfo);

    uint8_t totalChannels = static_cast<uint8_t>(sourceInfo.micInChannels) +
        static_cast<uint8_t>(sourceInfo.channels) + static_cast<uint8_t>(sourceInfo.ecChannels);
    EXPECT_EQ(nodeInfo.channels == (AudioChannel)totalChannels, false);
}

/**
 * @tc.name  : Test StopInterphoneSourceIfNeeded for interphone capturer
 * @tc.type  : FUNC
 * @tc.number: StopInterphoneSourceIfNeeded_001
 * @tc.desc  : Test StopInterphoneSourceIfNeeded when deviceName contains "Interphone".
 */
HWTEST_F(HpaeCapturerManagerTest, StopInterphoneSourceIfNeeded_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.deviceName = "Interphone_Source";  // deviceName contains "Interphone"
    sourceInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    sourceInfo.filePath = g_rootCapturerPath;
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    sourceInfo.micRef = HPAE_REF_OFF;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    
    // Test StopInterphoneSourceIfNeeded function directly with deviceName containing "Interphone"
    capturerManager->StopInterphoneSourceIfNeeded();
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), false);
}

/**
 * @tc.name  : Test StopInterphoneSourceIfNeeded for non-interphone capturer
 * @tc.type  : FUNC
 * @tc.number: StopInterphoneSourceIfNeeded_002
 * @tc.desc  : Test StopInterphoneSourceIfNeeded when deviceName does not contain "Interphone".
 */
HWTEST_F(HpaeCapturerManagerTest, StopInterphoneSourceIfNeeded_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);  // deviceName is "Built_in_mic", does not contain "Interphone"
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    
    // Test StopInterphoneSourceIfNeeded - should not stop for non-interphone deviceName
    capturerManager->StopInterphoneSourceIfNeeded();
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), false);
}

/**
 * @tc.name  : Test Pause for interphone capturer stream
 * @tc.type  : FUNC
 * @tc.number: Pause_Interphone_001
 * @tc.desc  : Test Pause when deviceName contains "Interphone", calls StopInterphoneSourceIfNeeded.
 */
HWTEST_F(HpaeCapturerManagerTest, Pause_Interphone_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.deviceName = "Interphone_Source";  // deviceName contains "Interphone"
    sourceInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    sourceInfo.filePath = g_rootCapturerPath;
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    sourceInfo.micRef = HPAE_REF_OFF;
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    EXPECT_EQ(capturerManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo), SUCCESS);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_RUNNING);
    
    // Pause will call StopInterphoneSourceIfNeeded internally for interphone deviceName
    EXPECT_EQ(capturerManager->Pause(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo), SUCCESS);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PAUSED);
    
    EXPECT_EQ(capturerManager->Stop(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->DestroyStream(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), false);
}

/**
 * @tc.name  : Test Pause for non-interphone capturer stream
 * @tc.type  : FUNC
 * @tc.number: Pause_NonInterphone_001
 * @tc.desc  : Test Pause when deviceName does not contain "Interphone".
 */
HWTEST_F(HpaeCapturerManagerTest, Pause_NonInterphone_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);  // deviceName is "Built_in_mic", does not contain "Interphone"
    
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    EXPECT_EQ(capturerManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo), SUCCESS);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_RUNNING);
    
    EXPECT_EQ(capturerManager->Pause(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo), SUCCESS);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PAUSED);
    
    EXPECT_EQ(capturerManager->Stop(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->DestroyStream(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    
    EXPECT_EQ(capturerManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), false);
}

/**
 * @tc.name  : FaultCode_CaptureEffectCreate_InvalidSceneType
 * @tc.type  : FUNC
 * @tc.desc  : Test CaptureEffectCreate returns ERROR when sceneType is not supported,
 *             covering FAULT_NO_SOUND/RECORD/CREATE/ERR_INVALID_PARAM fault code path.
 */
HWTEST_F(HpaeCapturerManagerTest, FaultCode_CaptureEffectCreate_InvalidSceneType,
    TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    auto capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);

    static constexpr AudioEnhanceScene INVALID_ENHANCE_SCENE = static_cast<AudioEnhanceScene>(9999);
    int32_t ret = capturerManager->CaptureEffectCreate(
        HPAE_SCENE_DEFAULT, INVALID_ENHANCE_SCENE);
    EXPECT_EQ(ret, ERROR);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : SourceTypeCamcorderConstruct_001
 * tc.desc   : Test SourceTypeCamcorderConstruct_001
 */
HWTEST_F(HpaeCapturerManagerTest, SourceTypeCamcorderConstruct_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    EXPECT_EQ(capturerManager->sourceInfo_.micInSamplingRate, SAMPLE_RATE_16000);
    EXPECT_EQ(capturerManager->sourceInfo_.micInFormat, SAMPLE_S16LE);
    EXPECT_EQ(capturerManager->sourceInfo_.micInChannels, CHANNEL_4);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : SourceTypeCamcorderConstruct_002
 * tc.desc   : Test SourceTypeCamcorderConstruct_002
 */
HWTEST_F(HpaeCapturerManagerTest, SourceTypeCamcorderConstruct_002, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);
    EXPECT_EQ(capturerManager->sourceInfo_.sourceType, SOURCE_TYPE_CAMCORDER);
}

/*
 * tc.name   : Test HpaeCapturerManager UpdateSourceAttr API
 * tc.type   : FUNC
 * tc.number : UpdateSourceAttr_Camcorder_001
 * tc.desc   : Test UpdateSourceAttr with camcorder source type
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceAttr_Camcorder_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);

    IAudioSourceAttr attr;
    capturerManager->CreateSourceAttr(attr);
    capturerManager->UpdateSourceAttr(attr);

    EXPECT_EQ(attr.formatMicIn == sourceInfo.micInFormat, true);
    EXPECT_EQ(attr.sampleRateMicIn == sourceInfo.micInSamplingRate, true);
    EXPECT_EQ(attr.channelMicIn == sourceInfo.micInChannels, true);
}

/*
 * tc.name   : Test HpaeCapturerManager UpdateSourceAttr API
 * tc.type   : FUNC
 * tc.number : UpdateSourceAttr_Camcorder_002
 * tc.desc   : Test UpdateSourceAttr with camcorder and ec config
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceAttr_Camcorder_002, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    capturerManager->mainMicType_ = HPAE_SOURCE_MIC_EC;

    IAudioSourceAttr attr;
    capturerManager->CreateSourceAttr(attr);
    capturerManager->UpdateSourceAttr(attr);

    EXPECT_EQ(attr.formatMicIn == sourceInfo.micInFormat, true);
    EXPECT_EQ(attr.sampleRateMicIn == sourceInfo.micInSamplingRate, true);
    EXPECT_EQ(attr.channelMicIn == sourceInfo.micInChannels, true);
    EXPECT_EQ(attr.formatEc == sourceInfo.ecFormat, true);
    EXPECT_EQ(attr.sampleRateEc == sourceInfo.ecSamplingRate, true);
    EXPECT_EQ(attr.channelEc == sourceInfo.ecChannels, true);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : UpdateSourceInputNodeInfoForCamcorder_001
 * tc.desc   : Test UpdateSourceInputNodeInfoForCamcorder_001
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceInputNodeInfoForCamcorder_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    nodeInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    capturerManager->UpdateSourceInputNodeInfoForRecognition(nodeInfo);

    EXPECT_EQ(static_cast<uint8_t>(nodeInfo.channels), static_cast<uint8_t>(sourceInfo.micInChannels) +
        static_cast<uint8_t>(sourceInfo.channels));
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : UpdateSourceInputNodeInfoForCamcorder_NonCamcorder_001
 * tc.desc   : Test UpdateSourceInputNodeInfoForCamcorder_NonCamcorder_001
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateSourceInputNodeInfoForCamcorder_NonCamcorder_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_16;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    capturerManager->UpdateSourceInputNodeInfoForRecognition(nodeInfo);

    EXPECT_EQ(nodeInfo.channels == static_cast<AudioChannel>(static_cast<uint8_t>(sourceInfo.micInChannels) +
        static_cast<uint8_t>(sourceInfo.channels)), false);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : UpdateClusterNodeInfoForCamcorder_001
 * tc.desc   : Test UpdateClusterNodeInfoForCamcorder_001
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateClusterNodeInfoForCamcorder_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_4;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecChannels = CHANNEL_4;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    nodeInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    capturerManager->UpdateClusterNodeInfoForRecognition(nodeInfo);

    uint8_t totalChannels = static_cast<uint8_t>(sourceInfo.micInChannels) +
        static_cast<uint8_t>(sourceInfo.channels) + static_cast<uint8_t>(sourceInfo.ecChannels);
    EXPECT_EQ(static_cast<uint8_t>(nodeInfo.channels), totalChannels);
}

/*
 * tc.name   : Test HpaeCapturerManager API
 * tc.type   : FUNC
 * tc.number : UpdateClusterNodeInfoForCamcorder_NonCamcorder_001
 * tc.desc   : Test UpdateClusterNodeInfoForCamcorder_NonCamcorder_001
 */
HWTEST_F(HpaeCapturerManagerTest, UpdateClusterNodeInfoForCamcorder_NonCamcorder_001, TestSize.Level0)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.sourceType = SOURCE_TYPE_CAMCORDER;
    sourceInfo.micInSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.micInFormat = SAMPLE_S16LE;
    sourceInfo.micInChannels = CHANNEL_16;
    sourceInfo.ecSamplingRate = SAMPLE_RATE_16000;
    sourceInfo.ecFormat = SAMPLE_S16LE;
    sourceInfo.ecChannels = CHANNEL_16;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    capturerManager->UpdateClusterNodeInfoForRecognition(nodeInfo);

    uint8_t totalChannels = static_cast<uint8_t>(sourceInfo.micInChannels) +
        static_cast<uint8_t>(sourceInfo.channels) + static_cast<uint8_t>(sourceInfo.ecChannels);
    EXPECT_EQ(nodeInfo.channels == static_cast<AudioChannel>(totalChannels), false);
}

/**
 * @tc.name  : Test DeleteOutputSession with keepSessionMaps parameter
 * @tc.type  : FUNC
 * @tc.number: DeleteOutputSession_KeepSessionMaps_001
 * @tc.desc  : Test DeleteOutputSession with keepSessionMaps=true keeps session maps
 */
HWTEST_F(HpaeCapturerManagerTest, DeleteOutputSession_KeepSessionMaps_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 1);

    // Delete with keepSessionMaps=true - should keep both maps
    capturerManager->DeleteOutputSession(streamInfo.sessionId, true);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 1);
    EXPECT_EQ(capturerManager->sessionNodeMap_.size(), 1);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test DeleteOutputSession with keepSessionMaps=false
 * @tc.type  : FUNC
 * @tc.number: DeleteOutputSession_KeepSessionMaps_002
 * @tc.desc  : Test DeleteOutputSession with keepSessionMaps=false removes session maps
 */
HWTEST_F(HpaeCapturerManagerTest, DeleteOutputSession_KeepSessionMaps_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 1);

    // Delete with keepSessionMaps=false (default) - should remove sessionNodeMap
    capturerManager->DeleteOutputSession(streamInfo.sessionId, false);
    EXPECT_EQ(capturerManager->sourceOutputNodeMap_.size(), 0);
    EXPECT_EQ(capturerManager->sessionNodeMap_.size(), 0);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test DeInit with isMoveDefault=true
 * @tc.type  : FUNC
 * @tc.number: DeInit_MoveDefault_001
 * @tc.desc  : Test DeInit with isMoveDefault=true triggers stream move
 */
HWTEST_F(HpaeCapturerManagerTest, DeInit_MoveDefault_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    // DeInit with isMoveDefault=true - should trigger move to default
    EXPECT_EQ(capturerManager->DeInit(true) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), false);
}

/**
 * @tc.name  : Test DeInit with isMoveDefault=false (default)
 * @tc.type  : FUNC
 * @tc.number: DeInit_MoveDefault_002
 * @tc.desc  : Test DeInit with isMoveDefault=false does not trigger stream move
 */
HWTEST_F(HpaeCapturerManagerTest, DeInit_MoveDefault_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    // DeInit with isMoveDefault=false (default) - should not trigger move
    EXPECT_EQ(capturerManager->DeInit(false) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), false);
}

/**
 * @tc.name  : Test MoveStream with empty sourceName
 * @tc.type  : FUNC
 * @tc.number: MoveStream_EmptySourceName_001
 * @tc.desc  : Test MoveStream with empty sourceName handles gracefully
 */
HWTEST_F(HpaeCapturerManagerTest, MoveStream_EmptySourceName_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    // MoveStream with empty sourceName - should return SUCCESS
    EXPECT_EQ(capturerManager->MoveStream(streamInfo.sessionId, "") == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test StopManager
 * @tc.type  : FUNC
 * @tc.number: StopManager_001
 * @tc.desc  : Test StopManager stops the capture source
 */
HWTEST_F(HpaeCapturerManagerTest, StopManager_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->Start(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    // StopManager should stop the source
    EXPECT_EQ(capturerManager->StopManager() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test HandleMsg
 * @tc.type  : FUNC
 * @tc.number: HandleMsg_001
 * @tc.desc  : Test HandleMsg processes message queue
 */
HWTEST_F(HpaeCapturerManagerTest, HandleMsg_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    // HandleMsg should process queued messages
    capturerManager->HandleMsg();
    EXPECT_EQ(capturerManager->IsMsgProcessing(), false);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test Release equals DestroyStream
 * @tc.type  : FUNC
 * @tc.number: Release_DestroyStream_001
 * @tc.desc  : Test Release calls DestroyStream internally
 */
HWTEST_F(HpaeCapturerManagerTest, Release_DestroyStream_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    // Release should destroy stream
    EXPECT_EQ(capturerManager->Release(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test Init with isReload parameter
 * @tc.type  : FUNC
 * @tc.number: Init_ReloadParam_001
 * @tc.desc  : Test Init with isReload=false (default)
 */
HWTEST_F(HpaeCapturerManagerTest, Init_ReloadParam_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);

    // Init with isReload=false (default)
    EXPECT_EQ(capturerManager->Init(false) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test Init with isReload=true
 * @tc.type  : FUNC
 * @tc.number: Init_ReloadParam_002
 * @tc.desc  : Test Init with isReload=true
 */
HWTEST_F(HpaeCapturerManagerTest, Init_ReloadParam_002, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);

    // Init with isReload=true
    EXPECT_EQ(capturerManager->Init(true) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test OnNotifyQueue callback
 * @tc.type  : FUNC
 * @tc.number: OnNotifyQueue_001
 * @tc.desc  : Test OnNotifyQueue notifies signal process thread
 */
HWTEST_F(HpaeCapturerManagerTest, OnNotifyQueue_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    // OnNotifyQueue should notify the signal process thread
    capturerManager->OnNotifyQueue();
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test StopInterphoneSourceIfNeeded direct call
 * @tc.type  : FUNC
 * @tc.number: StopInterphoneSourceIfNeeded_Direct_001
 * @tc.desc  : Test direct call to StopInterphoneSourceIfNeeded for interphone device
 */
HWTEST_F(HpaeCapturerManagerTest, StopInterphoneSourceIfNeeded_Direct_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    sourceInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sourceInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sourceInfo.deviceName = "interphone_test_device";
    sourceInfo.sourceType = SOURCE_TYPE_INTERPHONE;
    sourceInfo.filePath = g_rootCapturerPath;
    sourceInfo.samplingRate = SAMPLE_RATE_48000;
    sourceInfo.channels = STEREO;
    sourceInfo.format = SAMPLE_S16LE;
    sourceInfo.frameLen = DEFAULT_FRAME_LENGTH;
    sourceInfo.ecType = HPAE_EC_TYPE_NONE;
    sourceInfo.micRef = HPAE_REF_OFF;

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    // Direct call to test interphone-specific behavior
    capturerManager->StopInterphoneSourceIfNeeded();
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test CreateOutputSession with VOICE_MESSAGE source type
 * @tc.type  : FUNC
 * @tc.number: CreateOutputSession_VoiceMessage_001
 * @tc.desc  : Test CreateOutputSession with SOURCE_TYPE_VOICE_MESSAGE
 */
HWTEST_F(HpaeCapturerManagerTest, CreateOutputSession_VoiceMessage_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_VOICE_MESSAGE;

    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
}

/**
 * @tc.name  : Test CreateOutputSession with VOICE_TRANSCRIPTION source type
 * @tc.type  : FUNC
 * @tc.number: CreateOutputSession_VoiceTranscription_001
 * @tc.desc  : Test CreateOutputSession with SOURCE_TYPE_VOICE_TRANSCRIPTION
 */
HWTEST_F(HpaeCapturerManagerTest, CreateOutputSession_VoiceTranscription_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);

    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    streamInfo.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;

    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
}

/**
 * @tc.name  : Test MoveAllStream with MOVE_SINGLE type
 * @tc.type  : FUNC
 * @tc.number: MoveAllStream_MoveSingle_001
 * @tc.desc  : Test MoveAllStream with MOVE_SINGLE move type
 */
HWTEST_F(HpaeCapturerManagerTest, MoveAllStream_MoveSingle_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    std::vector<uint32_t> sessionIds;
    EXPECT_EQ(capturerManager->MoveAllStream(DEFAULT_SOURCE_NAME, sessionIds, MOVE_SINGLE), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/*
 * tc.name   : Test scene cluster survives while effect ref count > 0
 * tc.type   : FUNC
 * tc.number : SceneClusterEffectRefCount_001
 * tc.desc   : DeleteOutputSession must not release/erase the shared scene cluster while the effect
 *             ref count is > 0, and must erase it once the count drops to 0.
 */
HWTEST_F(HpaeCapturerManagerTest, SceneClusterEffectRefCount_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init(), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    // two recording streams sharing the same scene cluster
    HpaeStreamInfo streamInfoA;
    InitReloadStreamInfo(streamInfoA);
    streamInfoA.sourceType = SOURCE_TYPE_MIC;
    EXPECT_EQ(capturerManager->CreateStream(streamInfoA) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfoB;
    InitReloadStreamInfo(streamInfoB);
    streamInfoB.sourceType = SOURCE_TYPE_MIC;
    streamInfoB.sessionId = DEFAULT_SESSION_ID + 1; // 1 for another session id
    EXPECT_EQ(capturerManager->CreateStream(streamInfoB) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeProcessorType sceneType = TransSourceTypeToSceneType(streamInfoA.sourceType);
    ASSERT_EQ(capturerManager->sceneClusterMap_.count(sceneType), 1);
    auto sceneCluster = capturerManager->sceneClusterMap_[sceneType];
    ASSERT_NE(sceneCluster, nullptr);

    // UT env cannot instantiate the real algo chain: effect node is invalid and ref count is 0,
    // so IncSceneClusterEffectRef is a no-op here. Simulate two alive streams depending on the
    // effect (what the runtime would produce with a valid effect), then exercise the release path.
    EXPECT_EQ(sceneCluster->IsEffectNodeValid(), false);
    EXPECT_EQ(capturerManager->GetSceneClusterEffectRefCount(sceneType), 0);
    sceneCluster->IncEffectRefCount(); // stream A
    sceneCluster->IncEffectRefCount(); // stream B
    EXPECT_EQ(capturerManager->GetSceneClusterEffectRefCount(sceneType), 2);

    // release stream B: ref 2 -> 1, the shared cluster/effect must survive (the bug being fixed)
    capturerManager->DeleteOutputSession(streamInfoB.sessionId);
    EXPECT_EQ(capturerManager->sceneClusterMap_.count(sceneType), 1);
    EXPECT_EQ(capturerManager->GetSceneClusterEffectRefCount(sceneType), 1);

    // release stream A (last dependent): ref 1 -> 0, cluster is released and erased
    capturerManager->DeleteOutputSession(streamInfoA.sessionId);
    EXPECT_EQ(capturerManager->sceneClusterMap_.count(sceneType), 0);
    EXPECT_EQ(capturerManager->GetSceneClusterEffectRefCount(sceneType), 0);
}

/**
 * @tc.name  : Test AddNodeToSource for accessory device with 40ms frame len
 * @tc.type  : FUNC
 * @tc.number: AddNodeToSource_Accessory_40ms_001
 * @tc.desc  : Test AddNodeToSource for accessory device with 40ms frame len (halved span size branch)
 */
HWTEST_F(HpaeCapturerManagerTest, AddNodeToSource_Accessory_40ms_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    sourceInfo.deviceType = DEVICE_TYPE_ACCESSORY;
    sourceInfo.frameLen = 1920;
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);

    HpaeNodeInfo nodeInfo;
    InitNodeInfo(nodeInfo);
    nodeInfo.sessionId = streamInfo.sessionId;

    auto sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    EXPECT_NE(sourceOutputNode, nullptr);

    HpaeCaptureMoveInfo moveInfo;
    moveInfo.sessionId = streamInfo.sessionId;
    moveInfo.sourceOutputNode = sourceOutputNode;
    moveInfo.sessionInfo.state = HPAE_SESSION_PREPARED;

    EXPECT_EQ(capturerManager->AddNodeToSource(moveInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test MoveStream for normal stream
 * @tc.type  : FUNC
 * @tc.number: MoveStream_IsLowLatencyStream_001
 * @tc.desc  : Test MoveStream for normal stream
 */
HWTEST_F(HpaeCapturerManagerTest, MoveStream_IsLowLatencyStream_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->MoveStream(streamInfo.sessionId, DEFAULT_SOURCE_NAME), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

/**
 * @tc.name  : Test MoveAllStreamToNewSource for normal stream
 * @tc.type  : FUNC
 * @tc.number: MoveAllStreamToNewSource_IsLowLatencyStream_001
 * @tc.desc  : Test MoveAllStreamToNewSource for normal stream
 */
HWTEST_F(HpaeCapturerManagerTest, MoveAllStreamToNewSource_IsLowLatencyStream_001, TestSize.Level1)
{
    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    std::shared_ptr<HpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);

    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateOutputSession(streamInfo), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    std::vector<uint32_t> sessionIds = {streamInfo.sessionId};
    EXPECT_EQ(capturerManager->MoveAllStream(DEFAULT_SOURCE_NAME, sessionIds, MOVE_ALL), SUCCESS);
    WaitForMsgProcessing(capturerManager);

    EXPECT_EQ(capturerManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
}

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
