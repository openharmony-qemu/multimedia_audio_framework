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
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_renderer_manager.h"
#include "hpae_offload_renderer_manager.h"
#include "hpae_inner_capturer_manager.h"
#include "hpae_co_buffer_node.h"
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
namespace {
static std::string g_rootPath = "/data/";
constexpr int32_t FRAME_LENGTH_882 = 882;
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr uint32_t INVALID_ID = 99999;
constexpr uint32_t LOUDNESS_GAIN = 1.0f;
class HpaeRendererManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeRendererManagerTest::SetUp()
{}

void HpaeRendererManagerTest::TearDown()
{}

static void TestCheckSinkInputInfo(HpaeSinkInputInfo &sinkInputInfo, const HpaeStreamInfo &streamInfo)
{
    EXPECT_EQ(sinkInputInfo.nodeInfo.channels == streamInfo.channels, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.format == streamInfo.format, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.frameLen == streamInfo.frameLen, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.sessionId == streamInfo.sessionId, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.samplingRate == streamInfo.samplingRate, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.streamType == streamInfo.streamType, true);
}

template <class RenderManagerType>
static void WaitForMsgProcessing(std::shared_ptr<RenderManagerType> &hpaeRendererManager)
{
    int waitCount = 0;
    const int waitCountThd = 5;
    while (hpaeRendererManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
    EXPECT_EQ(hpaeRendererManager->IsMsgProcessing(), false);
    EXPECT_EQ(waitCount < waitCountThd, true);
}

template <class RenderManagerType>
void TestIRendererManagerConstruct()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    HpaeSinkInfo dstSinkInfo = hpaeRendererManager->GetSinkInfo();
    EXPECT_EQ(dstSinkInfo.deviceNetId == sinkInfo.deviceNetId, true);
    EXPECT_EQ(dstSinkInfo.deviceClass == sinkInfo.deviceClass, true);
    EXPECT_EQ(dstSinkInfo.adapterName == sinkInfo.adapterName, true);
    EXPECT_EQ(dstSinkInfo.frameLen == sinkInfo.frameLen, true);
    EXPECT_EQ(dstSinkInfo.samplingRate == sinkInfo.samplingRate, true);
    EXPECT_EQ(dstSinkInfo.format == sinkInfo.format, true);
    EXPECT_EQ(dstSinkInfo.channels == sinkInfo.channels, true);
    EXPECT_EQ(dstSinkInfo.deviceType == sinkInfo.deviceType, true);
}

template <class RenderManagerType>
void TestIRendererManagerInit()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

template <class RenderManagerType>
void TestRendererManagerCreateStream(
    std::shared_ptr<RenderManagerType> &hpaeRendererManager, HpaeStreamInfo &streamInfo)
{
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH_882;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager.use_count() == 1, true);
    HpaeSinkInputInfo sinkInputInfo;
    int32_t ret = hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo);
    EXPECT_EQ(ret == SUCCESS, true);
    TestCheckSinkInputInfo(sinkInputInfo, streamInfo);
}

template <class RenderManagerType>
void TestIRendererManagerCreateDestoryStream()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    int32_t ret = hpaeRendererManager->DestroyStream(streamInfo.sessionId);
    EXPECT_EQ(ret == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeSinkInputInfo sinkInputInfo;
    ret = hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo);
    EXPECT_EQ(ret == ERR_INVALID_OPERATION, true);
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager.use_count() == 1, true);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    TestCheckSinkInputInfo(sinkInputInfo, streamInfo);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PREPARED);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    ret = hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo);
    EXPECT_EQ(ret == ERR_INVALID_OPERATION, true);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

template <class RenderManagerType>
static void TestIRendererManagerStartPuaseStream()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    HpaeSinkInputInfo sinkInputInfo;
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeRendererManager->RegisterWriteCallback(streamInfo.sessionId, writeIncDataCb), SUCCESS);
    EXPECT_EQ(writeIncDataCb.use_count() == 1, true);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    // offload need enable after start
    hpaeRendererManager->SetOffloadPolicy(streamInfo.sessionId, 0);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    // offload need enable after start
    hpaeRendererManager->SetOffloadPolicy(streamInfo.sessionId, 0);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Stop(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(
        hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == ERR_INVALID_OPERATION, true);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

template <class RenderManagerType>
static void TestIRendererManagerSetLoudnessGain(uint32_t sessionId, float loundessGain)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceName = "test_device";
    sinkInfo.deviceClass = "test_class";
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);

    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    // test SetLoundessGain when session is created but not connected
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    hpaeRendererManager->sinkInputNodeMap_[TEST_STREAM_SESSION_ID] = std::make_shared<RenderManagerType>(streamInfo);

    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    // test set loundess gain before start
    EXPECT_EQ(hpaeRendererManager->SetLoudnessGain(TEST_STREAM_SESSION_ID, LOUDNESS_GAIN) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);

    // test set loudness gain after start
    EXPECT_EQ(hpaeRendererManager->SetLoudnessGain(TEST_STREAM_SESSION_ID, LOUDNESS_GAIN) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

TEST_F(HpaeRendererManagerTest, constructHpaeRendererManagerTest)
{
    TestIRendererManagerConstruct<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerConstruct<HpaeOffloadRendererManager>();
}

TEST_F(HpaeRendererManagerTest, HpaeRendererManagerInitTest)
{
    TestIRendererManagerInit<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerInit<HpaeOffloadRendererManager>();
}

TEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateDestoryStreamTest)
{
    TestIRendererManagerCreateDestoryStream<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerCreateDestoryStream<HpaeOffloadRendererManager>();
}

TEST_F(HpaeRendererManagerTest, HpaeRendererManagerStartPuaseStreamTest)
{
    TestIRendererManagerStartPuaseStream<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerStartPuaseStream<HpaeOffloadRendererManager>();
}

template <class RenderManagerType>
static void HpaeRendererManagerCreateStream(
    std::shared_ptr<RenderManagerType> &hpaeRendererManager, HpaeStreamInfo &streamInfo)
{
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH_882;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager.use_count() == 1, true);
    HpaeSinkInputInfo sinkInputInfo;
    int32_t ret = hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo);
    EXPECT_EQ(ret == SUCCESS, true);
    TestCheckSinkInputInfo(sinkInputInfo, streamInfo);
}

TEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateStreamTest_001)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.deviceName = "MCH_Speaker";
    sinkInfo.lib = "libmodule-split-stream-sink.z.so";
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);

    EXPECT_EQ(hpaeRendererManager->DestroyStream(INVALID_ID) == SUCCESS, false);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 1;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    EXPECT_EQ(hpaeRendererManager->DestroyStream(INVALID_ID) == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

TEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateStreamTest_002)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.deviceName = "MCH_Speaker";
    sinkInfo.lib = "libmodule-split-stream-sink.z.so";
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);

    EXPECT_EQ(hpaeRendererManager->DestroyStream(INVALID_ID) == SUCCESS, false);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);

    std::shared_ptr<IHpaeRendererManager> hpaeRendererManagerNew = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManagerNew->DestroyStream(INVALID_ID) == SUCCESS, false);
    EXPECT_EQ(hpaeRendererManagerNew->Init() == SUCCESS, true);

    WaitForMsgProcessing(hpaeRendererManager);
    WaitForMsgProcessing(hpaeRendererManagerNew);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    EXPECT_EQ(hpaeRendererManagerNew->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 1;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    HpaeRendererManagerCreateStream(hpaeRendererManagerNew, streamInfo);

    EXPECT_EQ(hpaeRendererManager->DestroyStream(INVALID_ID) == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManagerNew->DestroyStream(INVALID_ID) == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManagerNew->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManagerNew);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManagerNew->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManagerNew);
}


TEST_F(HpaeRendererManagerTest, HpaeRendererManagerTransStreamUsage)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.lib = "libmodule-split-stream-sink.z.so";
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);

    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 1;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    HpaeSinkInputInfo sinkInputInfo;
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeRendererManager->RegisterWriteCallback(streamInfo.sessionId, writeIncDataCb), SUCCESS);
    EXPECT_EQ(writeIncDataCb.use_count() == 1, true);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    // offload need enable after start
    hpaeRendererManager->SetOffloadPolicy(streamInfo.sessionId, 0);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    // offload need enable after start
    hpaeRendererManager->SetOffloadPolicy(streamInfo.sessionId, 0);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Stop(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(
        hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == ERR_INVALID_OPERATION, true);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

static void GetBtSpeakerSinkInfo(HpaeSinkInfo &sinkInfo)
{
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    sinkInfo.deviceName = "Bt_Speaker";
}
/**
 * @tc.name  : Test UpdateCollaborativeState
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_001
 * @tc.desc  : Test UpdateCollaborativeState before stream is created.
 */
TEST_F(HpaeRendererManagerTest, UpdateCollaborativeState_001)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(true), SUCCESS);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 1;
    streamInfo.effectInfo.effectScene = SCENE_MUSIC;
    streamInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeRendererManager->RegisterWriteCallback(streamInfo.sessionId, writeIncDataCb), SUCCESS);
    EXPECT_EQ(writeIncDataCb.use_count() == 1, true);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Stop(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(hpaeRendererManager->Release(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), ERR_INVALID_OPERATION);
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(false), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name  : Test UpdateCollaborativeState
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_002
 * @tc.desc  : Test UpdateCollaborativeState after stream is created.
 */
TEST_F(HpaeRendererManagerTest, UpdateCollaborativeState_002)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 1;
    streamInfo.effectInfo.effectScene = SCENE_MUSIC;
    streamInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeRendererManager->RegisterWriteCallback(streamInfo.sessionId, writeIncDataCb), SUCCESS);
    EXPECT_EQ(writeIncDataCb.use_count() == 1, true);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(true), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(false), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Stop(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Release(streamInfo.sessionId), ERROR);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), ERR_INVALID_OPERATION);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name  : Test ConnectCoBufferNode
 * @tc.type  : FUNC
 * @tc.number: ConnectCoBufferNode_001
 * @tc.desc  : Test ConnectCoBufferNode when config in vaild.
 */
TEST_F(HpaeRendererManagerTest, ConnectCoBufferNode_001)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeNodeInfo nodeInfo;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
    std::shared_ptr<HpaeCoBufferNode> coBufferNode = std::make_shared<HpaeCoBufferNode>();
    EXPECT_NE(coBufferNode, nullptr);
    coBufferNode->SetNodeInfo(nodeInfo);
    int32_t ret = hpaeRendererManager->ConnectCoBufferNode(coBufferNode);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    ret = hpaeRendererManager->DisConnectCoBufferNode(coBufferNode);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name  : Test MoveAllStream
 * @tc.type  : FUNC
 * @tc.number: MoveAllStream_001
 * @tc.desc  : Test MoveAllStream when sink is initialized.
 */
TEST_F(HpaeRendererManagerTest, MoveAllStream_001)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    std::string newSinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;

    int32_t ret = hpaeRendererManager->MoveAllStream(newSinkName, sessionIds, moveType);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name  : Test MoveAllStream
 * @tc.type  : FUNC
 * @tc.number: MoveAllStream_002
 * @tc.desc  : Test MoveAllStream when sink is not initialized.
 */
TEST_F(HpaeRendererManagerTest, MoveAllStream_002)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);

    std::string newSinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {4, 5, 6};
    MoveSessionType moveType = MOVE_ALL;

    int32_t ret = hpaeRendererManager->MoveAllStream(newSinkName, sessionIds, moveType);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name  : Test MoveStreamSync
 * @tc.type  : FUNC
 * @tc.number: MoveStreamSync_001
 * @tc.desc  : Test MoveStreamSync when sessionId doesn't exist in sinkInputNodeMap_.
 */
TEST_F(HpaeRendererManagerTest, MoveStreamSync_001)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    uint32_t invalidSessionId = 999; // Assuming this ID doesn't exist
    std::string sinkName = "valid_sink_name";
    hpaeRendererManager->MoveStreamSync(invalidSessionId, sinkName);
}

/**
 * @tc.name  : Test MoveStreamSync
 * @tc.type  : FUNC
 * @tc.number: MoveStreamSync_002
 * @tc.desc  : Test MoveStreamSync when sinkName is empty.
 */
TEST_F(HpaeRendererManagerTest, MoveStreamSync_002)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    std::string emptySinkName;
    hpaeRendererManager->MoveStreamSync(streamInfo.sessionId, emptySinkName);
}

/**
 * @tc.name  : Test MoveStreamSync
 * @tc.type  : FUNC
 * @tc.number: MoveStreamSync_003
 * @tc.desc  : Test MoveStreamSync when session is in HPAE_SESSION_STOPPING state.
 */
TEST_F(HpaeRendererManagerTest, MoveStreamSync_003)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    
    EXPECT_EQ(hpaeRendererManager->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    std::string sinkName = "valid_sink_name";
    hpaeRendererManager->MoveStreamSync(streamInfo.sessionId, sinkName);
}

/**
 * @tc.name  : Test MoveStreamSync
 * @tc.type  : FUNC
 * @tc.number: MoveStreamSync_004
 * @tc.desc  : Test MoveStreamSync when session is in HPAE_SESSION_PAUSING state.
 */
TEST_F(HpaeRendererManagerTest, MoveStreamSync_004)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    EXPECT_EQ(hpaeRendererManager->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    std::string sinkName = "valid_sink_name";
    hpaeRendererManager->MoveStreamSync(streamInfo.sessionId, sinkName);
}

/**
 * @tc.name  : Test CreateDefaultProcessCluster
 * @tc.type  : FUNC
 * @tc.number: CreateDefaultProcessCluster_001
 * @tc.desc  : Verify function creates new default cluster when none exists.
 */
TEST_F(HpaeRendererManagerTest, CreateDefaultProcessCluster_001)
{
    HpaeSinkInfo sinkInfo;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_NE(hpaeRendererManager, nullptr);

    HpaeNodeInfo nodeInfo;
    hpaeRendererManager->CreateDefaultProcessCluster(nodeInfo);
}

/**
 * @tc.name  : Test CreateDefaultProcessCluster
 * @tc.type  : FUNC
 * @tc.number: CreateDefaultProcessCluster_002
 * @tc.desc  : Verify function reuses existing default cluster.
 */
TEST_F(HpaeRendererManagerTest, CreateDefaultProcessCluster_002)
{
    HpaeSinkInfo sinkInfo;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_NE(hpaeRendererManager, nullptr);

    HpaeNodeInfo defaultNodeInfo;
    HpaeNodeInfo nodeInfo;
    
    hpaeRendererManager->CreateDefaultProcessCluster(defaultNodeInfo);
    hpaeRendererManager->CreateDefaultProcessCluster(nodeInfo);
}

/**
 * @tc.name: ReloadRenderManager
 * @tc.type: FUNC
 * @tc.number: ReloadRenderManager_001
 * @tc.desc: Test basic reload functionality
 */
TEST_F(HpaeRendererManagerTest, ReloadRenderManager_001)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceName = "test_device";
    sinkInfo.deviceClass = "test_class";
    
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    
    int32_t ret = hpaeRendererManager->ReloadRenderManager(sinkInfo);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(HpaeRendererManagerTest, HpaeRendererSetLoudnessGain_001)
{
    std::cout << "test renderer manager" << std::endl;
    TestIRendererManagerSetLoudnessGain<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerSetLoudnessGain<HpaeOffloadRendererManager>();
    std::cout << "test innercapture manager" << std::endl;
    TestIRendererManagerSetLoudnessGain<HpaeInnerCapturerManager>();
}
}  // namespace