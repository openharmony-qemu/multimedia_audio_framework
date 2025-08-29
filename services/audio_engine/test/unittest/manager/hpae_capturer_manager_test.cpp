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
#include "hpae_capturer_manager.h"
#include "hpae_node_common.h"

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
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS