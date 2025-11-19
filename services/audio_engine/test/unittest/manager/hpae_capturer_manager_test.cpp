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
    capturerManager->Process();
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
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS