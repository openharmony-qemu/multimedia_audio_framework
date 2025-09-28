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
#include "hpae_virtual_capturer_manager.h"
#include "hpae_node_common.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static std::string g_rootCapturerPath = "";
const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_SESSION_ID = 123456;
const std::string DEFAULT_SOURCE_NAME = "Built_in_mic";
const std::string DEFAULT_THREAD_NAME = "Virtual_capture";

class HpaeVirtualCapturerManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeVirtualCapturerManagerTest::SetUp()
{}

void HpaeVirtualCapturerManagerTest::TearDown()
{}

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

static void InitStreamInfo(HpaeStreamInfo &streamInfo)
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

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerCreateStream
 * tc.desc   : Test HpaeVirtualCapturerManagerCreateStream
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerCreateStream, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    int32_t ret = capturerManager->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerDestroyStream
 * tc.desc   : Test HpaeVirtualCapturerManagerDestroyStream
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerDestroyStream, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    uint32_t sessionId = DEFAULT_SESSION_ID;
    int32_t ret = capturerManager->DestroyStream(sessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerStart_001
 * tc.desc   : Test HpaeVirtualCapturerManagerStart_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerStart_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    int32_t ret = capturerManager->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t sessionId = streamInfo.sessionId;
    ret = capturerManager->Start(sessionId);
    HpaeCaptureMoveInfo stream = capturerManager->captureStream_[sessionId];
    EXPECT_EQ(stream.sessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(stream.sourceOutputNode->GetState(), HPAE_SESSION_RUNNING);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerPause_001
 * tc.desc   : Test HpaeVirtualCapturerManagerPause_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerPause_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    int32_t ret = capturerManager->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t sessionId = streamInfo.sessionId;
    ret = capturerManager->Pause(sessionId);
    EXPECT_EQ(ret, SUCCESS);
    HpaeCaptureMoveInfo stream = capturerManager->captureStream_[sessionId];
    EXPECT_EQ(stream.sessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(stream.sourceOutputNode->GetState(), HPAE_SESSION_PAUSED);
}


/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerFlush_001
 * tc.desc   : Test HpaeVirtualCapturerManagerFlush_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerFlush_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    int32_t ret = capturerManager->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t sessionId = streamInfo.sessionId;
    ret = capturerManager->Flush(sessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerDrain_001
 * tc.desc   : Test HpaeVirtualCapturerManagerDrain_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerDrain_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    int32_t ret = capturerManager->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t sessionId = streamInfo.sessionId;
    ret = capturerManager->Drain(sessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerStop_001
 * tc.desc   : Test HpaeVirtualCapturerManagerStop_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerStop_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    int32_t ret = capturerManager->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t sessionId = streamInfo.sessionId;
    ret = capturerManager->Stop(sessionId);
    EXPECT_EQ(ret, SUCCESS);
    HpaeCaptureMoveInfo stream = capturerManager->captureStream_[sessionId];
    EXPECT_EQ(stream.sessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(stream.sourceOutputNode->GetState(), HPAE_SESSION_STOPPED);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerRelease_001
 * tc.desc   : Test HpaeVirtualCapturerManagerRelease_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerRelease_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    int32_t ret = capturerManager->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t sessionId = streamInfo.sessionId;
    ret = capturerManager->Release(sessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerSetStreamMute_001
 * tc.desc   : Test HpaeVirtualCapturerManagerSetStreamMute_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerSetStreamMute_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeStreamInfo streamInfo;
    InitStreamInfo(streamInfo);
    int32_t ret = capturerManager->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);

    uint32_t sessionId = streamInfo.sessionId;
    ret = capturerManager->SetStreamMute(sessionId, true);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerSetMute_001
 * tc.desc   : Test HpaeVirtualCapturerManagerSetMute_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerSetMute_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    int32_t ret = capturerManager->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerMoveStream_001
 * tc.desc   : Test HpaeVirtualCapturerManagerMoveStream_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerMoveStream_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    uint32_t sessionId = DEFAULT_SESSION_ID;
    std::string sourceName = DEFAULT_SOURCE_NAME;
    int32_t ret = capturerManager->MoveStream(sessionId, sourceName);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerMoveAllStream_001
 * tc.desc   : Test HpaeVirtualCapturerManagerMoveAllStream_001
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerMoveAllStream_001, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    std::string sourceName = DEFAULT_SOURCE_NAME;
    std::vector<uint32_t> moveIds {};
    MoveSessionType moveType = MOVE_SINGLE;
    int32_t ret = capturerManager->MoveAllStream(sourceName, moveIds, moveType);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerMoveAllStream_002
 * tc.desc   : Test HpaeVirtualCapturerManagerMoveAllStream_002
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerMoveAllStream_002, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    std::string sourceName = DEFAULT_SOURCE_NAME;
    std::vector<uint32_t> moveIds {};
    MoveSessionType moveType = MOVE_ALL;
    int32_t ret = capturerManager->MoveAllStream(sourceName, moveIds, moveType);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerMoveAllStream_003
 * tc.desc   : Test HpaeVirtualCapturerManagerMoveAllStream_003
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerMoveAllStream_003, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    uint32_t sessionId = DEFAULT_SESSION_ID;
    HpaeCaptureMoveInfo moveInfo {};
    capturerManager->captureStream_.insert_or_assign(sessionId, moveInfo);

    std::string sourceName = DEFAULT_SOURCE_NAME;
    std::vector<uint32_t> moveIds {};
    moveIds.push_back(sessionId);
    MoveSessionType moveType = MOVE_SINGLE;
    int32_t ret = capturerManager->MoveAllStream(sourceName, moveIds, moveType);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerMoveAllStream_004
 * tc.desc   : Test HpaeVirtualCapturerManagerMoveAllStream_004
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerMoveAllStream_004, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    uint32_t sessionId = DEFAULT_SESSION_ID;
    HpaeCaptureMoveInfo moveInfo {};
    capturerManager->captureStream_.insert_or_assign(sessionId, moveInfo);

    std::string sourceName = DEFAULT_SOURCE_NAME;
    std::vector<uint32_t> moveIds {};
    moveIds.push_back(sessionId);
    MoveSessionType moveType = MOVE_ALL;
    int32_t ret = capturerManager->MoveAllStream(sourceName, moveIds, moveType);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerProcess
 * tc.desc   : Test HpaeVirtualCapturerManagerProcess
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerProcess, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    capturerManager->Process();
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerHandleMsg
 * tc.desc   : Test HpaeVirtualCapturerManagerHandleMsg
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerHandleMsg, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    capturerManager->HandleMsg();
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerInit
 * tc.desc   : Test HpaeVirtualCapturerManagerInit
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerInit, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    int32_t ret = capturerManager->Init(true);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerDeInit
 * tc.desc   : Test HpaeVirtualCapturerManagerDeInit
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerDeInit, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    int32_t ret = capturerManager->DeInit(true);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerIsInit
 * tc.desc   : Test HpaeVirtualCapturerManagerIsInit
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerIsInit, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    bool ret = capturerManager->IsInit();
    EXPECT_EQ(ret, true);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerIsRunning
 * tc.desc   : Test HpaeVirtualCapturerManagerIsRunning
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerIsRunning, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    bool ret = capturerManager->IsRunning();
    EXPECT_EQ(ret, true);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerIsMsgProcessing
 * tc.desc   : Test HpaeVirtualCapturerManagerIsMsgProcessing
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerIsMsgProcessing, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    bool ret = capturerManager->IsMsgProcessing();
    EXPECT_EQ(ret, true);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerDeactivateThread
 * tc.desc   : Test HpaeVirtualCapturerManagerDeactivateThread
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerDeactivateThread, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    bool ret = capturerManager->DeactivateThread();
    EXPECT_EQ(ret, true);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerRegisterReadCallback
 * tc.desc   : Test HpaeVirtualCapturerManagerRegisterReadCallback
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerRegisterReadCallback, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    uint32_t sessionId = DEFAULT_SESSION_ID;
    std::shared_ptr<ReadDataCb> readDataCb =
        std::make_shared<ReadDataCb>(g_rootCapturerPath);
    int32_t ret = capturerManager->RegisterReadCallback(sessionId, readDataCb);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerGetSourceOutputInfo
 * tc.desc   : Test HpaeVirtualCapturerManagerGetSourceOutputInfo
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerGetSourceOutputInfo, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    uint32_t sessionId = DEFAULT_SESSION_ID;
    HpaeSourceOutputInfo info {};
    int32_t ret = capturerManager->GetSourceOutputInfo(sessionId, info);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerGetSourceInfo
 * tc.desc   : Test HpaeVirtualCapturerManagerGetSourceInfo
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerGetSourceInfo, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeSourceInfo ret = capturerManager->GetSourceInfo();
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerGetAllSourceOutputsInfo
 * tc.desc   : Test HpaeVirtualCapturerManagerGetAllSourceOutputsInfo
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerGetAllSourceOutputsInfo, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    std::vector<SourceOutput> ret = capturerManager->GetAllSourceOutputsInfo();
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerOnNodeStatusUpdate
 * tc.desc   : Test HpaeVirtualCapturerManagerOnNodeStatusUpdate
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerOnNodeStatusUpdate, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    uint32_t sessionId = DEFAULT_SESSION_ID;
    IOperation operation = OPERATION_STARTED;
    capturerManager->OnNodeStatusUpdate(sessionId, operation);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerOnNotifyQueue
 * tc.desc   : Test HpaeVirtualCapturerManagerOnNotifyQueue
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerOnNotifyQueue, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    capturerManager->OnNotifyQueue();
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerOnRequestLatency
 * tc.desc   : Test HpaeVirtualCapturerManagerOnRequestLatency
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerOnRequestLatency, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    uint32_t sessionId = DEFAULT_SESSION_ID;
    uint64_t latency = 0;
    capturerManager->OnRequestLatency(sessionId, latency);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerAddNodeToSource
 * tc.desc   : Test HpaeVirtualCapturerManagerAddNodeToSource
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerAddNodeToSource, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeCaptureMoveInfo moveInfo {};
    int32_t ret = capturerManager->AddNodeToSource(moveInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerAddAllNodesToSource
 * tc.desc   : Test HpaeVirtualCapturerManagerAddAllNodesToSource
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerAddAllNodesToSource, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    std::vector<HpaeCaptureMoveInfo> moveInfos {};
    int32_t ret = capturerManager->AddAllNodesToSource(moveInfos, true);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerGetThreadName
 * tc.desc   : Test HpaeVirtualCapturerManagerGetThreadName
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerGetThreadName, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    std::string threadName = DEFAULT_THREAD_NAME;
    std::string ret = capturerManager->GetThreadName();
    EXPECT_EQ(ret, threadName);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerReloadCaptureManager
 * tc.desc   : Test HpaeVirtualCapturerManagerReloadCaptureManager
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerReloadCaptureManager, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    HpaeSourceInfo sourceInfo;
    InitSourceInfo(sourceInfo);
    int32_t ret = capturerManager->ReloadCaptureManager(sourceInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerDumpSourceInfo
 * tc.desc   : Test HpaeVirtualCapturerManagerDumpSourceInfo
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerDumpSourceInfo, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    int32_t ret = capturerManager->DumpSourceInfo();
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * tc.name   : Test HpaeVirtualCapturerManager API
 * tc.type   : FUNC
 * tc.number : HpaeVirtualCapturerManagerGetDeviceHDFDumpInfo
 * tc.desc   : Test HpaeVirtualCapturerManagerGetDeviceHDFDumpInfo
 */
HWTEST_F(HpaeVirtualCapturerManagerTest, HpaeVirtualCapturerManagerGetDeviceHDFDumpInfo, TestSize.Level0)
{
    auto capturerManager = std::make_shared<HpaeVirtualCapturerManager>();
    EXPECT_NE(capturerManager, nullptr);

    std::string info = "";
    std::string ret = capturerManager->GetDeviceHDFDumpInfo();
    EXPECT_EQ(ret, info);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS