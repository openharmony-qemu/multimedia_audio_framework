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

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";
const uint32_t DEFAULT_FRAME_LENGTH = 960;
const uint32_t DEFAULT_SESSION_ID = 123456;

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

static void WaitForMsgProcessing(std::shared_ptr<IHpaeCapturerManager> &capturerManager)
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

TEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerConstructTest)
{
    HpaeSourceInfo sourceInfo;
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
    
    std::shared_ptr<IHpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
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

TEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerInitTest)
{
    HpaeSourceInfo sourceInfo;
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
    
    std::shared_ptr<IHpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
}

TEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerCreateDestoryStreamTest)
{
    HpaeSourceInfo sourceInfo;
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
    
    std::shared_ptr<IHpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.deviceName = "Built_in_mic";
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager.use_count() == 1, true);
    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    TestCheckSourceOutputInfo(sourceOutputInfo, streamInfo);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PREPARED);
    EXPECT_EQ(capturerManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(
        capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == ERR_INVALID_OPERATION, true);
}

static void StateControlTest(std::shared_ptr<IHpaeCapturerManager> &capturerManager, HpaeStreamInfo &streamInfo,
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

TEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerStartStopTest)
{
    HpaeSourceInfo sourceInfo;
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
    
    std::shared_ptr<IHpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = DEFAULT_FRAME_LENGTH;
    streamInfo.sessionId = DEFAULT_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_RECORD;
    streamInfo.deviceName = "Built_in_mic";
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager.use_count() == 1, true);

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

TEST_F(HpaeCapturerManagerTest, HpaeCapturerManagerReloadTest)
{
    HpaeSourceInfo sourceInfo;
    HpaeSourceInfo newSourceInfo;
    InitReloadSourceInfo(sourceInfo, newSourceInfo);

    std::shared_ptr<IHpaeCapturerManager> capturerManager = std::make_shared<HpaeCapturerManager>(sourceInfo);
    EXPECT_EQ(capturerManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    InitReloadStreamInfo(streamInfo);
    EXPECT_EQ(capturerManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager.use_count() == 1, true);
    HpaeSourceOutputInfo sourceOutputInfo;
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
    TestCheckSourceOutputInfo(sourceOutputInfo, streamInfo);
    EXPECT_EQ(sourceOutputInfo.capturerSessionInfo.state, HPAE_SESSION_PREPARED);
    EXPECT_EQ(capturerManager->ReloadCaptureManager(newSourceInfo) == SUCCESS, true);
    WaitForMsgProcessing(capturerManager);
    EXPECT_EQ(capturerManager->GetSourceOutputInfo(streamInfo.sessionId, sourceOutputInfo) == SUCCESS, true);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS