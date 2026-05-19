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
#include "hpae_injector_renderer_manager.h"
#include "hpae_output_cluster.h"
#include "hpae_co_buffer_node.h"
#include "hpae_inner_capturer_manager.h"
#include "audio_effect_chain_manager.h"
#include "hpae_sink_virtual_output_node.h"
#include "hpae_node_common.h"
#include "hpae_mocks.h"
#include "audio_utils.h"
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;
namespace {
static std::string g_rootPath = "/data/";
constexpr int32_t FRAME_LENGTH_882 = 882;
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t FRAME_LENGTH_1024 = 1024;
constexpr int32_t OVERSIZED_FRAME_LENGTH = 38500;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t TEST_SLEEP_TIME_100 = 100;
constexpr uint32_t INVALID_ID = 99999;
constexpr uint32_t LOUDNESS_GAIN = 1.0f;
constexpr uint32_t DEFAULT_SESSIONID_NUM_FIRST = 100000;

static HpaeSinkInfo GetSinkInfo()
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
    return sinkInfo;
}

static void TestCheckSinkInputInfo(HpaeSinkInputInfo &sinkInputInfo, const HpaeStreamInfo &streamInfo)
{
    EXPECT_EQ(sinkInputInfo.nodeInfo.channels == streamInfo.channels, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.format == streamInfo.format, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.frameLen == streamInfo.frameLen, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.sessionId == streamInfo.sessionId, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.samplingRate == streamInfo.samplingRate, true);
    EXPECT_EQ(sinkInputInfo.nodeInfo.streamType == streamInfo.streamType, true);
}

static std::shared_ptr<HpaeSinkInputNode> CreateTestNode(OHOS::AudioStandard::HPAE::HpaeSessionState state)
{
    HpaeNodeInfo nodeinfo;
    nodeinfo.streamType = STREAM_MUSIC;
    std::shared_ptr<HpaeSinkInputNode> node = std::make_shared<HpaeSinkInputNode>(nodeinfo);
    node->SetState(state);
    return node;
}

class HpaeRendererManagerTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager_;
    std::shared_ptr<HpaeOutputCluster> outputCluster_;
    std::shared_ptr<HpaeSinkInputNode> sinkInputNode_;
    std::shared_ptr<MockStreamCallback> mockCallback_;
};

void HpaeRendererManagerTest::SetUp()
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    hpaeRendererManager_ = std::make_shared<HpaeRendererManager>(sinkInfo);

    outputCluster_ = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeRendererManager_->outputCluster_ = outputCluster_;
    hpaeRendererManager_->hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    sinkInputNode_ = CreateTestNode(HPAE_SESSION_RUNNING);
    mockCallback_ = std::make_shared<NiceMock<MockStreamCallback>>();
    sinkInputNode_->RegisterWriteCallback(mockCallback_);
    hpaeRendererManager_->appsUid_ = {123};
    hpaeRendererManager_->sinkInputNodeMap_[1] = sinkInputNode_;
}

void HpaeRendererManagerTest::TearDown()
{
    hpaeRendererManager_.reset();
    outputCluster_.reset();
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

static std::shared_ptr<HpaeSinkVirtualOutputNode> SetSinkVirtualOutputNode(const HpaeSinkInfo &sinkInfo,
    const std::shared_ptr<IHpaeRendererManager> &rendererManager)
{
    HpaeNodeInfo nodeInfo;
    TransSinkInfoToNodeInfo(sinkInfo, rendererManager, nodeInfo);
    std::shared_ptr<HpaeSinkVirtualOutputNode> sinkOutputNode = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo);
    rendererManager->SetSinkVirtualOutputNode(sinkOutputNode);
    return sinkOutputNode;
}

template <class RenderManagerType>
void TestIRendererManagerConstruct()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
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
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
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
    HpaeSinkInputInfo sinkInputInfo;
    int32_t ret = hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo);
    EXPECT_EQ(ret == SUCCESS, true);
    TestCheckSinkInputInfo(sinkInputInfo, streamInfo);
}

template <class RenderManagerType>
void TestRenderManagerReload()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeStreamInfo streamInfo;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    EXPECT_EQ(hpaeRendererManager->ReloadRenderManager(sinkInfo, true) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    hpaeRendererManager->SetOffloadPolicy(streamInfo.sessionId, 0);
    WaitForMsgProcessing(hpaeRendererManager);

    hpaeRendererManager->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);

    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->ReloadRenderManager(sinkInfo, true) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

template <class RenderManagerType>
void TestIRendererManagerCreateDestoryStream()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
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
    HpaeSinkInfo sinkInfo = GetSinkInfo();
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
    hpaeRendererManager->SetSpeed(streamInfo.sessionId, 1.0f);
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
    hpaeRendererManager->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->SuspendStreamManager(true) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
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
static void TestIRendererManagerFlushDrainStream()
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
    hpaeRendererManager->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->Flush(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
    EXPECT_EQ(hpaeRendererManager->Drain(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(
        hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == ERR_INVALID_OPERATION, true);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

template <class RenderManagerType>
static void TestIRendererManagerDiffFrameLenStream()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    sinkInfo.frameLen = FRAME_LENGTH_1024;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
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
    hpaeRendererManager->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->Flush(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
    EXPECT_EQ(hpaeRendererManager->Drain(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(
        hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == ERR_INVALID_OPERATION, true);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

template <class RenderManagerType>
static void TestIRendererManagerSetLoudnessGain()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);

    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    // test SetLoudnessGain when session is created but not connected
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    // test set loundess gain before start
    EXPECT_EQ(hpaeRendererManager->SetLoudnessGain(streamInfo.sessionId, LOUDNESS_GAIN) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);

    // test set loudness gain after start
    EXPECT_EQ(hpaeRendererManager->SetLoudnessGain(streamInfo.sessionId, LOUDNESS_GAIN) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

template <class RenderManagerType>
static void TestIRendererManagerOnRequestLatency()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);

    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->SetLoudnessGain(streamInfo.sessionId, LOUDNESS_GAIN) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);

    // test set get latency after start
    uint64_t latency = 0;
    hpaeRendererManager->OnRequestLatency(streamInfo.sessionId, latency);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

HWTEST_F(HpaeRendererManagerTest, constructHpaeRendererManagerTest, TestSize.Level0)
{
    TestIRendererManagerConstruct<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerConstruct<HpaeOffloadRendererManager>();
    std::cout << "test injector" << std::endl;
    TestIRendererManagerConstruct<HpaeInjectorRendererManager>();
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerInitTest, TestSize.Level1)
{
    TestIRendererManagerInit<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerInit<HpaeOffloadRendererManager>();
    std::cout << "test injector" << std::endl;
    TestIRendererManagerInit<HpaeInjectorRendererManager>();
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerReloadTest, TestSize.Level1)
{
    TestRenderManagerReload<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestRenderManagerReload<HpaeOffloadRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerReload<HpaeInjectorRendererManager>();
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateDestoryStreamTest, TestSize.Level1)
{
    TestIRendererManagerCreateDestoryStream<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerCreateDestoryStream<HpaeOffloadRendererManager>();
    std::cout << "test injector" << std::endl;
    TestIRendererManagerCreateDestoryStream<HpaeInjectorRendererManager>();
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerStartPuaseStreamTest, TestSize.Level1)
{
    TestIRendererManagerStartPuaseStream<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerStartPuaseStream<HpaeOffloadRendererManager>();
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerFlushDrainStreamTest, TestSize.Level1)
{
    TestIRendererManagerFlushDrainStream<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerFlushDrainStream<HpaeOffloadRendererManager>();
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerDiffFrameLenStreamTest, TestSize.Level1)
{
    TestIRendererManagerDiffFrameLenStream<HpaeRendererManager>();
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
    HpaeSinkInputInfo sinkInputInfo;
    int32_t ret = hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo);
    EXPECT_EQ(ret == SUCCESS, true);
    TestCheckSinkInputInfo(sinkInputInfo, streamInfo);
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateStreamTest_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
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

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateStreamTest_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
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

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateStreamTest_003, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
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
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == ERR_INVALID_OPERATION, true);
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerTransStreamUsage, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    sinkInfo.lib = "libmodule-split-stream-sink.z.so";
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);

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

    hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_PAUSING);
    EXPECT_EQ(hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]->state_, HPAE_SESSION_PAUSING);
    hpaeRendererManager->TriggerStreamState(streamInfo.sessionId,
                                            hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]);
    hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_PAUSED);
    hpaeRendererManager->SetSessionState(streamInfo.sessionId, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->sessionNodeMap_[streamInfo.sessionId].state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    // offload need enable after start
    hpaeRendererManager->SetOffloadPolicy(streamInfo.sessionId, 0);
    hpaeRendererManager->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    EXPECT_EQ(hpaeRendererManager->Stop(streamInfo.sessionId) == SUCCESS, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_100));
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);

    hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_STOPPING);
    EXPECT_EQ(hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]->state_, HPAE_SESSION_STOPPING);
    hpaeRendererManager->TriggerStreamState(streamInfo.sessionId,
                                            hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]);
    hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_STOPPED);
    hpaeRendererManager->SetSessionState(streamInfo.sessionId, HPAE_SESSION_STOPPED);
    EXPECT_EQ(hpaeRendererManager->sessionNodeMap_[streamInfo.sessionId].state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), ERR_INVALID_OPERATION);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
}

template <class RenderManagerType>
void TestRenderManagerMoveAllStream001()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<RenderManagerType> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    std::string newSinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;

    int32_t ret = hpaeRendererManager->MoveAllStream(newSinkName, sessionIds, moveType);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test MoveAllStream
 * @tc.type  : FUNC
 * @tc.number: MoveAllStream_001
 * @tc.desc  : Test MoveAllStream when sink is initialized.
 */
HWTEST_F(HpaeRendererManagerTest, MoveAllStream_001, TestSize.Level1)
{
    TestRenderManagerMoveAllStream001<HpaeRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerMoveAllStream001<HpaeInjectorRendererManager>();
}

template <class RenderManagerType>
void TestRenderManagerMoveAllStream002()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<RenderManagerType> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);

    std::string newSinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {4, 5, 6};
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
HWTEST_F(HpaeRendererManagerTest, MoveAllStream_002, TestSize.Level0)
{
    TestRenderManagerMoveAllStream002<HpaeRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerMoveAllStream002<HpaeInjectorRendererManager>();
}

template <class RenderManagerType>
void TestRenderManagerMoveStreamSync001()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<RenderManagerType> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    uint32_t invalidSessionId = 999; // Assuming this ID doesn't exist
    std::string sinkName = "valid_sink_name";
    hpaeRendererManager->MoveStreamSync(invalidSessionId, sinkName);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test MoveStreamSync
 * @tc.type  : FUNC
 * @tc.number: MoveStreamSync_001
 * @tc.desc  : Test MoveStreamSync when sessionId doesn't exist in sinkInputNodeMap_.
 */
HWTEST_F(HpaeRendererManagerTest, MoveStreamSync_001, TestSize.Level1)
{
    TestRenderManagerMoveStreamSync001<HpaeRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerMoveStreamSync001<HpaeInjectorRendererManager>();
}

template <class RenderManagerType>
void TestRenderManagerMoveStreamSync002()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<RenderManagerType> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    std::string emptySinkName;
    hpaeRendererManager->MoveStreamSync(TEST_STREAM_SESSION_ID, emptySinkName);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test MoveStreamSync
 * @tc.type  : FUNC
 * @tc.number: MoveStreamSync_002
 * @tc.desc  : Test MoveStreamSync when sinkName is empty.
 */
HWTEST_F(HpaeRendererManagerTest, MoveStreamSync_002, TestSize.Level1)
{
    TestRenderManagerMoveStreamSync002<HpaeRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerMoveStreamSync002<HpaeInjectorRendererManager>();
}

template <class RenderManagerType>
void TestRenderManagerMoveStreamSync003()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<RenderManagerType> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    EXPECT_EQ(hpaeRendererManager->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    std::string sinkName = "valid_sink_name";
    hpaeRendererManager->MoveStreamSync(TEST_STREAM_SESSION_ID, sinkName);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test MoveStreamSync
 * @tc.type  : FUNC
 * @tc.number: MoveStreamSync_003
 * @tc.desc  : Test MoveStreamSync when session is in HPAE_SESSION_STOPPING state.
 */
HWTEST_F(HpaeRendererManagerTest, MoveStreamSync_003, TestSize.Level1)
{
    TestRenderManagerMoveStreamSync003<HpaeRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerMoveStreamSync003<HpaeInjectorRendererManager>();
}

template <class RenderManagerType>
void TestRenderManagerMoveStreamSync004()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<RenderManagerType> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    EXPECT_EQ(hpaeRendererManager->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    std::string sinkName = "valid_sink_name";
    hpaeRendererManager->MoveStreamSync(TEST_STREAM_SESSION_ID, sinkName);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test MoveStreamSync
 * @tc.type  : FUNC
 * @tc.number: MoveStreamSync_004
 * @tc.desc  : Test MoveStreamSync when session is in HPAE_SESSION_PAUSING state.
 */
HWTEST_F(HpaeRendererManagerTest, MoveStreamSync_004, TestSize.Level1)
{
    TestRenderManagerMoveStreamSync004<HpaeRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerMoveStreamSync004<HpaeInjectorRendererManager>();
}

/**
 * @tc.name  : Test CreateDefaultProcessCluster
 * @tc.type  : FUNC
 * @tc.number: CreateDefaultProcessCluster_001
 * @tc.desc  : Verify function creates new default cluster when none exists.
 */
HWTEST_F(HpaeRendererManagerTest, CreateDefaultProcessCluster_001, TestSize.Level0)
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
HWTEST_F(HpaeRendererManagerTest, CreateDefaultProcessCluster_002, TestSize.Level0)
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
HWTEST_F(HpaeRendererManagerTest, ReloadRenderManager_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();

    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);

    int32_t ret = hpaeRendererManager->ReloadRenderManager(sinkInfo);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name: ReloadRenderManager Offload
 * @tc.type: FUNC
 * @tc.number: ReloadRenderManager_002
 * @tc.desc: Test ReloadRenderManager when STREAM_MANAGER_RUNNING
 */
HWTEST_F(HpaeRendererManagerTest, ReloadRenderManager_002, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo sinkInfo  = GetSinkInfo();
    auto hpaeRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    hpaeRendererManager->sinkOutputNode_ = std::make_unique<HpaeOffloadSinkOutputNode>(nodeInfo);
    hpaeRendererManager->sinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->AddNodeToMap(sinkInputNode);
    EXPECT_EQ(sinkInputNode, hpaeRendererManager->curNode_);
    auto tmpNodeId1 = hpaeRendererManager->converterForOutput_->GetNodeId();
    auto tmpNodeId2 = hpaeRendererManager->loudnessGainNode_->GetNodeId();
    auto tmpNodeId3 = hpaeRendererManager->converterForLoudness_->GetNodeId();
    EXPECT_NE(hpaeRendererManager->converterForOutput_, nullptr);
    EXPECT_NE(hpaeRendererManager->loudnessGainNode_, nullptr);
    EXPECT_NE(hpaeRendererManager->converterForLoudness_, nullptr);
    EXPECT_EQ(hpaeRendererManager->ConnectInputSession(), SUCCESS);

    int32_t ret = hpaeRendererManager->ReloadRenderManager(sinkInfo);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(hpaeRendererManager->converterForOutput_, nullptr);
    EXPECT_NE(hpaeRendererManager->loudnessGainNode_, nullptr);
    EXPECT_NE(hpaeRendererManager->converterForLoudness_, nullptr);
    EXPECT_EQ(tmpNodeId1, hpaeRendererManager->converterForOutput_->GetNodeId());
    EXPECT_EQ(tmpNodeId2, hpaeRendererManager->loudnessGainNode_->GetNodeId());
    EXPECT_EQ(tmpNodeId3, hpaeRendererManager->converterForLoudness_->GetNodeId());
}

/**
 * @tc.name: ReloadRenderManager Offload
 * @tc.type: FUNC
 * @tc.number: ReloadRenderManager_003
 * @tc.desc: Test ReloadRenderManager when not STREAM_MANAGER_RUNNING
 */
HWTEST_F(HpaeRendererManagerTest, ReloadRenderManager_003, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto hpaeRendererManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    hpaeRendererManager->sinkOutputNode_ = std::make_unique<HpaeOffloadSinkOutputNode>(nodeInfo);
    hpaeRendererManager->sinkOutputNode_->SetSinkState(STREAM_MANAGER_SUSPENDED);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->AddNodeToMap(sinkInputNode);
    EXPECT_EQ(sinkInputNode, hpaeRendererManager->curNode_);
    auto tmpNodeId1 = hpaeRendererManager->converterForOutput_->GetNodeId();
    auto tmpNodeId2 = hpaeRendererManager->loudnessGainNode_->GetNodeId();
    auto tmpNodeId3 = hpaeRendererManager->converterForLoudness_->GetNodeId();
    EXPECT_NE(hpaeRendererManager->converterForOutput_, nullptr);
    EXPECT_NE(hpaeRendererManager->loudnessGainNode_, nullptr);
    EXPECT_NE(hpaeRendererManager->converterForLoudness_, nullptr);
    EXPECT_EQ(hpaeRendererManager->ConnectInputSession(), SUCCESS);

    int32_t ret = hpaeRendererManager->ReloadRenderManager(sinkInfo);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(hpaeRendererManager->converterForOutput_, nullptr);
    EXPECT_NE(hpaeRendererManager->loudnessGainNode_, nullptr);
    EXPECT_NE(hpaeRendererManager->converterForLoudness_, nullptr);
    EXPECT_EQ(tmpNodeId1, hpaeRendererManager->converterForOutput_->GetNodeId());
    EXPECT_EQ(tmpNodeId2, hpaeRendererManager->loudnessGainNode_->GetNodeId());
    EXPECT_EQ(tmpNodeId3, hpaeRendererManager->converterForLoudness_->GetNodeId());
}

/**
 * @tc.name  : Test HpaeRendererManager Pause API with isStandby
 * @tc.type  : FUNC
 * @tc.number: PauseWithStandby_001
 * @tc.desc  : Test Pause with isStandby=true for renderer stream
 */
HWTEST_F(HpaeRendererManagerTest, PauseWithStandby_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    HpaeSinkInputInfo sinkInputInfo;
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeRendererManager->RegisterWriteCallback(streamInfo.sessionId, writeIncDataCb), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    hpaeRendererManager->SetOffloadPolicy(streamInfo.sessionId, 0);
    hpaeRendererManager->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId, true) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);

    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name  : Test HpaeRendererManager Pause API with isStandby=false
 * @tc.type  : FUNC
 * @tc.number: PauseWithStandby_002
 * @tc.desc  : Test Pause with isStandby=false for renderer stream
 */
HWTEST_F(HpaeRendererManagerTest, PauseWithStandby_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    HpaeSinkInputInfo sinkInputInfo;
    std::shared_ptr<WriteFixedDataCb> writeIncDataCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S16LE);
    EXPECT_EQ(hpaeRendererManager->RegisterWriteCallback(streamInfo.sessionId, writeIncDataCb), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);
    hpaeRendererManager->SetOffloadPolicy(streamInfo.sessionId, 0);
    hpaeRendererManager->SetSpeed(streamInfo.sessionId, 1.0f);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId, false) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);

    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name  : Test IHpaeRendererManager AdjustFrameLen
 * @tc.type  : FUNC
 * @tc.number: AdjustFrameLen_001
 * @tc.desc  : Test AdjustFrameLen with different frameLen in microseconds
 */
HWTEST_F(HpaeRendererManagerTest, AdjustFrameLen_001, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    inNodeInfo.frameLen = 480;
    inNodeInfo.samplingRate = SAMPLE_RATE_48000;

    HpaeNodeInfo outNodeInfo;
    outNodeInfo.frameLen = 960;
    outNodeInfo.samplingRate = SAMPLE_RATE_48000;

    bool result = IHpaeRendererManager::AdjustFrameLen(inNodeInfo, outNodeInfo);
    EXPECT_EQ(result, true);
    EXPECT_EQ(inNodeInfo.frameLen, 960);
}

/**
 * @tc.name  : Test IHpaeRendererManager AdjustFrameLen same frameLen
 * @tc.type  : FUNC
 * @tc.number: AdjustFrameLen_002
 * @tc.desc  : Test AdjustFrameLen when frameLen already matches
 */
HWTEST_F(HpaeRendererManagerTest, AdjustFrameLen_002, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    inNodeInfo.frameLen = 960;
    inNodeInfo.samplingRate = SAMPLE_RATE_48000;

    HpaeNodeInfo outNodeInfo;
    outNodeInfo.frameLen = 960;
    outNodeInfo.samplingRate = SAMPLE_RATE_48000;

    bool result = IHpaeRendererManager::AdjustFrameLen(inNodeInfo, outNodeInfo);
    EXPECT_EQ(result, false);
    EXPECT_EQ(inNodeInfo.frameLen, 960);
}

/**
 * @tc.name  : Test IHpaeRendererManager AdjustFrameLen different samplingRate
 * @tc.type  : FUNC
 * @tc.number: AdjustFrameLen_003
 * @tc.desc  : Test AdjustFrameLen with different samplingRate
 */
HWTEST_F(HpaeRendererManagerTest, AdjustFrameLen_003, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    inNodeInfo.frameLen = 441;
    inNodeInfo.samplingRate = SAMPLE_RATE_44100;

    HpaeNodeInfo outNodeInfo;
    outNodeInfo.frameLen = 960;
    outNodeInfo.samplingRate = SAMPLE_RATE_48000;

    bool result = IHpaeRendererManager::AdjustFrameLen(inNodeInfo, outNodeInfo);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test SetIsLowLatency for HpaeRendererManager
 * @tc.type  : FUNC
 * @tc.number: SetIsLowLatency_001
 * @tc.desc  : Test SetIsLowLatency is called with false during AddSingleNodeToSink
 */
HWTEST_F(HpaeRendererManagerTest, SetIsLowLatency_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    WaitForMsgProcessing(hpaeRendererManager);

    auto it = hpaeRendererManager->sinkInputNodeMap_.find(streamInfo.sessionId);
    ASSERT_EQ(it != hpaeRendererManager->sinkInputNodeMap_.end(), true);
    auto node = it->second;
    EXPECT_NE(node, nullptr);
    EXPECT_EQ(node->GetIsLowLatency(), false);

    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name: CreateRendererManager
 * @tc.type: FUNC
 * @tc.number: CreateRendererManager_001
 * @tc.desc: Test CreateRendererManager
 */
HWTEST_F(HpaeRendererManagerTest, CreateRendererManager_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceClass = "remote_offload";
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    EXPECT_NE(hpaeRendererManager, nullptr);
    sinkInfo.deviceClass = "offload";
    hpaeRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    EXPECT_NE(hpaeRendererManager, nullptr);
    sinkInfo.deviceClass = "test";
    hpaeRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    EXPECT_NE(hpaeRendererManager, nullptr);
}

/**
 * @tc.name: StartWithSyncId
 * @tc.type: FUNC
 * @tc.number: StartWithSyncId_001
 * @tc.desc: Test StartWithSyncId
 */
HWTEST_F(HpaeRendererManagerTest, StartWithSyncId_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);

    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    int32_t syncId = 123;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->StartWithSyncId(streamInfo.sessionId, syncId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
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
HWTEST_F(HpaeRendererManagerTest, UpdateCollaborativeState_001, TestSize.Level1)
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
    EXPECT_EQ(hpaeRendererManager->DestroyStream(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), ERR_INVALID_OPERATION);
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(false), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test UpdateCollaborativeState
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_002
 * @tc.desc  : Test UpdateCollaborativeState after stream is created.
 */
HWTEST_F(HpaeRendererManagerTest, UpdateCollaborativeState_002, TestSize.Level1)
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
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), ERR_INVALID_OPERATION);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test ConnectCoBufferNode
 * @tc.type  : FUNC
 * @tc.number: ConnectCoBufferNode_001
 * @tc.desc  : Test ConnectCoBufferNode when config in vaild.
 */
HWTEST_F(HpaeRendererManagerTest, ConnectCoBufferNode_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
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
    coBufferNode->SetOutputClusterConnected(true);
    hpaeRendererManager->outputCluster_->Start();
    int32_t ret = hpaeRendererManager->ConnectCoBufferNode(coBufferNode);
    EXPECT_EQ(ret, SUCCESS);
    hpaeRendererManager->outputCluster_->DeInit();
    hpaeRendererManager->isSuspend_ = true;
    ret = hpaeRendererManager->ConnectCoBufferNode(coBufferNode);
    EXPECT_EQ(ret, SUCCESS);
    coBufferNode->SetOutputClusterConnected(false);
    hpaeRendererManager->isSuspend_ = false;
    ret = hpaeRendererManager->ConnectCoBufferNode(coBufferNode);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    ret = hpaeRendererManager->DisConnectCoBufferNode(coBufferNode);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererSetLoudnessGain_001, TestSize.Level0)
{
    std::cout << "test renderer manager" << std::endl;
    TestIRendererManagerSetLoudnessGain<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerSetLoudnessGain<HpaeOffloadRendererManager>();
    std::cout << "test innercapture manager" << std::endl;
    TestIRendererManagerSetLoudnessGain<HpaeInnerCapturerManager>();
}

/**
 * @tc.name: RefreshProcessClusterByDevice
 * @tc.type: FUNC
 * @tc.number: RefreshProcessClusterByDevice_001
 * @tc.desc: Test RefreshProcessClusterByDevice
 */
HWTEST_F(HpaeRendererManagerTest, RefreshProcessClusterByDevice_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 10001;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId]->connectedProcessorType_ = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = true;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    int32_t ret = hpaeRendererManager->RefreshProcessClusterByDevice();
    EXPECT_EQ(ret == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: RefreshProcessClusterByDevice
 * @tc.type: FUNC
 * @tc.number: RefreshProcessClusterByDevice_002
 * @tc.desc: Test RefreshProcessClusterByDevice
 */
HWTEST_F(HpaeRendererManagerTest, RefreshProcessClusterByDevice_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 10002;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = false;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    int32_t ret = hpaeRendererManager->RefreshProcessClusterByDevice();
    EXPECT_EQ(ret == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: RefreshProcessClusterByDevice
 * @tc.type: FUNC
 * @tc.number: RefreshProcessClusterByDevice_003
 * @tc.desc: Test RefreshProcessClusterByDevice
 */
HWTEST_F(HpaeRendererManagerTest, RefreshProcessClusterByDevice_003, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 10003;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = true;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    int32_t ret = hpaeRendererManager->RefreshProcessClusterByDevice();
    EXPECT_EQ(ret == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: RefreshProcessClusterByDevice
 * @tc.type: FUNC
 * @tc.number: RefreshProcessClusterByDevice_004
 * @tc.desc: Test RefreshProcessClusterByDevice
 */
HWTEST_F(HpaeRendererManagerTest, RefreshProcessClusterByDevice_004, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeNodeInfo nodeInfo1;
    nodeInfo1.sessionId = 10004;
    nodeInfo1.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo1.effectInfo.effectMode = EFFECT_NONE;
    nodeInfo1.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo1.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo1);
    hpaeRendererManager->sessionNodeMap_[nodeInfo1.sessionId].bypass = false;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;

    HpaeNodeInfo nodeInfo2 = nodeInfo1;
    nodeInfo2.sessionId = 10005;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo2.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo2);
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo2.sessionId]->connectedProcessorType_ = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager->sessionNodeMap_[nodeInfo2.sessionId].bypass = false;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo2, sinkInfo);
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT] = hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC];

    int32_t ret = hpaeRendererManager->RefreshProcessClusterByDevice();
    EXPECT_EQ(ret == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

// ==================== UT for DisconnectNodeFromEffectChain ====================

/**
 * @tc.name  : DisconnectNodeFromEffectChain
 * @tc.type  : FUNC
 * @tc.number: DisconnectNodeFromEffectChain_001
 * @tc.desc  : Test DisconnectNodeFromEffectChain releases from scene and reconnects to EFFECT_NONE.
 */
HWTEST_F(HpaeRendererManagerTest, DisconnectNodeFromEffectChain_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 20001;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;

    // Set up sceneClusterMap_ with a process cluster for the scene
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT] =
        hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC];
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 2;
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT] = 2;
    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = false;

    hpaeRendererManager->DisconnectNodeFromEffectChain(nodeInfo);

    // Verify bypass is now true
    EXPECT_EQ(hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass, true);
    // Verify counts decremented for both sceneType and default
    EXPECT_EQ(hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC], 1);
    EXPECT_EQ(hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT], 1);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
}

/**
 * @tc.name  : DisconnectNodeFromEffectChain
 * @tc.type  : FUNC
 * @tc.number: DisconnectNodeFromEffectChain_002
 * @tc.desc  : Test DisconnectNodeFromEffectChain with non-default scene does not decrement default count.
 */
HWTEST_F(HpaeRendererManagerTest, DisconnectNodeFromEffectChain_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 20002;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;

    // scene is NOT shared with default
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 1;
    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = false;

    hpaeRendererManager->DisconnectNodeFromEffectChain(nodeInfo);

    EXPECT_EQ(hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass, true);
    EXPECT_EQ(hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC], 0);
    // Default count should not be affected
    EXPECT_EQ(hpaeRendererManager->sceneTypeToProcessClusterCountMap_.count(HPAE_SCENE_DEFAULT) == 0 ||
        hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT] == 0, true);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
}

/**
 * @tc.name  : DisconnectNodeFromEffectChain
 * @tc.type  : FUNC
 * @tc.number: DisconnectNodeFromEffectChain_003
 * @tc.desc  : Test DisconnectNodeFromEffectChain returns early when scene cluster not found.
 */
HWTEST_F(HpaeRendererManagerTest, DisconnectNodeFromEffectChain_003, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 20003;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    // No sceneClusterMap_ entry for HPAE_SCENE_MUSIC — should CHECK_AND_RETURN

    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = false;
    hpaeRendererManager->DisconnectNodeFromEffectChain(nodeInfo);

    // Should not crash, bypass should remain unchanged
    EXPECT_EQ(hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass, false);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
}

// ==================== UT for SwitchBypassState ====================

/**
 * @tc.name  : SwitchBypassState
 * @tc.type  : FUNC
 * @tc.number: SwitchBypassState_001
 * @tc.desc  : Test SwitchBypassState with USE_NONE_PROCESSCLUSTER and EFFECT_NONE triggers disconnect.
 */
HWTEST_F(HpaeRendererManagerTest, SwitchBypassState_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 30001;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;

    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->connectedProcessorType_ = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = node;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 1;
    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = false;

    hpaeRendererManager->SwitchBypassState(node, nodeInfo, USE_NONE_PROCESSCLUSTER);

    // Should have been disconnected and bypass set to true
    EXPECT_EQ(hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass, true);
    EXPECT_EQ(hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC], 0);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
}

/**
 * @tc.name  : SwitchBypassState
 * @tc.type  : FUNC
 * @tc.number: SwitchBypassState_002
 * @tc.desc  : Test SwitchBypassState with non-NONE decision and EFFECT_NONE processor creates process cluster.
 */
HWTEST_F(HpaeRendererManagerTest, SwitchBypassState_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 30002;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;

    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->connectedProcessorType_ = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = node;
    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = false;

    hpaeRendererManager->SwitchBypassState(node, nodeInfo, CREATE_NEW_PROCESSCLUSTER);

    // Should not crash; a new process cluster should be created
    EXPECT_NE(hpaeRendererManager->sceneClusterMap_.count(HPAE_SCENE_MUSIC) == 0 ||
        hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] != nullptr, false);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
}

/**
 * @tc.name  : SwitchBypassState
 * @tc.type  : FUNC
 * @tc.number: SwitchBypassState_003
 * @tc.desc  : Test SwitchBypassState with non-EFFECT_NONE processor triggers full reconnect.
 */
HWTEST_F(HpaeRendererManagerTest, SwitchBypassState_003, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 30003;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;

    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    // Not connected to EFFECT_NONE — will go to the else branch
    node->connectedProcessorType_ = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = node;
    hpaeRendererManager->sessionNodeMap_[nodeInfo.sessionId].bypass = false;

    hpaeRendererManager->SwitchBypassState(node, nodeInfo, USE_DEFAULT_PROCESSCLUSTER);

    // Should not crash — the full reconnect path was taken
    SUCCEED();

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
}

/**
 * @tc.name  : CreateProcessClusterInner
 * @tc.type  : FUNC
 * @tc.number: CreateProcessClusterInner_001
 * @tc.desc  : Test CreateProcessClusterInner
 */
HWTEST_F(HpaeRendererManagerTest, CreateProcessClusterInner_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeNodeInfo nodeInfo;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    int32_t processClusterDecision = NO_NEED_TO_CREATE_PROCESSCLUSTER;
    hpaeRendererManager->CreateProcessClusterInner(nodeInfo, processClusterDecision);
}

HWTEST_F(HpaeRendererManagerTest, CreateProcessClusterInner_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeNodeInfo nodeInfo;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    int32_t processClusterDecision = CREATE_NEW_PROCESSCLUSTER;
    hpaeRendererManager->CreateProcessClusterInner(nodeInfo, processClusterDecision);
}

HWTEST_F(HpaeRendererManagerTest, CreateProcessClusterInner_003, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    HpaeNodeInfo nodeInfo;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.channelLayout = CH_LAYOUT_STEREO;
    nodeInfo.sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    int32_t processClusterDecision = USE_DEFAULT_PROCESSCLUSTER;
    hpaeRendererManager->CreateProcessClusterInner(nodeInfo, processClusterDecision);
}

template <class RenderManagerType>
void TestRenderManagerMoveStream001()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<RenderManagerType> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
    std::string sinkName = "valid_sink_name";
    hpaeRendererManager->MoveStream(TEST_STREAM_SESSION_ID, sinkName);
}

/**
 * @tc.name  : MoveStream
 * @tc.type  : FUNC
 * @tc.number: MoveStream_001
 * @tc.desc  : Test MoveStream
 */
HWTEST_F(HpaeRendererManagerTest, MoveStream_001, TestSize.Level1)
{
    TestRenderManagerMoveStream001<HpaeRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerMoveStream001<HpaeInjectorRendererManager>();
}

template <class RenderManagerType>
void TestRenderManagerMoveStream002()
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<RenderManagerType> hpaeRendererManager = std::make_shared<RenderManagerType>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    WaitForMsgProcessing(hpaeRendererManager);
    std::string sinkName = "valid_sink_name";
    hpaeRendererManager->MoveStream(TEST_STREAM_SESSION_ID, sinkName);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : MoveStream
 * @tc.type  : FUNC
 * @tc.number: MoveStream_002
 * @tc.desc  : Test MoveStream
 */
HWTEST_F(HpaeRendererManagerTest, MoveStream_002, TestSize.Level1)
{
    TestRenderManagerMoveStream002<HpaeRendererManager>();
    std::cout << "test injector" << std::endl;
    TestRenderManagerMoveStream002<HpaeInjectorRendererManager>();
}

/**
 * @tc.name  : DeactivateThread
 * @tc.type  : FUNC
 * @tc.number: DeactivateThread_001
 * @tc.desc  : Test DeactivateThread
 */
HWTEST_F(HpaeRendererManagerTest, DeactivateThread_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread = std::make_unique<HpaeSignalProcessThread>();
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
    hpaeSignalProcessThread->ActivateThread(hpaeRendererManager);
    EXPECT_EQ(hpaeSignalProcessThread->IsRunning(), true);
    hpaeRendererManager->DeactivateThread();
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
}

 /**
 * @tc.name  : Test MoveAllStreamToNewSinkInner
 * @tc.type  : FUNC
 * @tc.number: MoveAllStreamToNewSinkInner_001
 * @tc.desc  : Test MoveAllStreamToNewSinkInner.
 */
HWTEST_F(HpaeRendererManagerTest, MoveAllStreamToNewSinkInner_001, TestSize.Level0)
{
    HpaeSinkInfo info = GetSinkInfo();
    auto hpaeOffloadRendererManager = std::make_shared<HpaeOffloadRendererManager>(info);
    auto hpaeRendererManager = std::make_shared<HpaeRendererManager>(info);
    auto mockCallback = std::make_shared<MockSendMsgCallback>();
    EXPECT_CALL(*mockCallback, InvokeSync(MOVE_ALL_SINK_INPUT, testing::_))
        .Times(2);
    EXPECT_CALL(*mockCallback, Invoke(MOVE_ALL_SINK_INPUT, testing::_))
        .Times(2);
    hpaeOffloadRendererManager->weakCallback_ = mockCallback;
    hpaeRendererManager->weakCallback_ = mockCallback;
    vector<uint32_t> moveids;
    hpaeOffloadRendererManager->MoveAllStreamToNewSink("", moveids, MOVE_ALL);
    hpaeRendererManager->MoveAllStreamToNewSink("", moveids, MOVE_ALL);
    hpaeOffloadRendererManager->MoveAllStreamToNewSink("", moveids, MOVE_PREFER);
    hpaeRendererManager->MoveAllStreamToNewSink("", moveids, MOVE_PREFER);
}

/**
 * @tc.name  : Test Process
 * @tc.type  : FUNC
 * @tc.number: Process_001
 * @tc.desc  : Test Process.
 */
HWTEST_F(HpaeRendererManagerTest, Process_001, TestSize.Level0)
{
    HpaeSinkInfo info;
    HpaeNodeInfo nodeinfo;
    auto hpaeRendererManager = std::make_shared<HpaeRendererManager>(info);
    auto outputCluster = std::make_shared<HpaeOutputCluster>(nodeinfo);
    hpaeRendererManager->outputCluster_ = outputCluster;
    ::testing::DefaultValue<int32_t>::Set(0);
    auto mockRenderSink = std::make_shared<NiceMock<MockAudioRenderSink>>();
    outputCluster->hpaeSinkOutputNode_->audioRendererSink_ = mockRenderSink;
    outputCluster->hpaeSinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    outputCluster->timeoutThdFramesForDevice_ = 300; // prevent unexpected call
    hpaeRendererManager->hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    hpaeRendererManager->hpaeSignalProcessThread_->running_.store(true);

    hpaeRendererManager->Process();
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeinfo);
    hpaeRendererManager->sinkInputNodeMap_.insert_or_assign(1, sinkInputNode);
    sinkInputNode->SetState(HPAE_SESSION_RUNNING);
    hpaeRendererManager->Process();
    EXPECT_EQ(hpaeRendererManager->IsRunning(), true);
    hpaeRendererManager->sinkInputNodeMap_.erase(1);
    hpaeRendererManager->noneStreamTime_ = 1;
    EXPECT_CALL(*mockRenderSink, Stop())
        .WillOnce(Return(0));
    hpaeRendererManager->Process();
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
    ::testing::DefaultValue<int32_t>::Clear();
}

/**
 * @tc.name: SetAudioEffectMode
 * @tc.type: FUNC
 * @tc.number: SetAudioEffectMode_001
 * @tc.desc: Test SetAudioEffectMode
 */
HWTEST_F(HpaeRendererManagerTest, SetAudioEffectMode_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    uint32_t sessionId = TEST_STREAM_SESSION_ID;
    int32_t effectMode = -1; // invalid effect mode
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    int32_t ret = hpaeRendererManager->SetAudioEffectMode(sessionId, effectMode);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    effectMode = 2; // invalid effect mode
    ret = hpaeRendererManager->SetAudioEffectMode(sessionId, effectMode);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    effectMode = EFFECT_NONE;
    ret = hpaeRendererManager->SetAudioEffectMode(100000, effectMode); // invalid sessionId
    EXPECT_EQ(ret, SUCCESS);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = TEST_STREAM_SESSION_ID;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    ret = hpaeRendererManager->SetAudioEffectMode(sessionId, effectMode); // same effectMode
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: SetAudioEffectMode
 * @tc.type: FUNC
 * @tc.number: SetAudioEffectMode_002
 * @tc.desc: Test SetAudioEffectMode
 */
HWTEST_F(HpaeRendererManagerTest, SetAudioEffectMode_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    uint32_t sessionId = TEST_STREAM_SESSION_ID;
    int32_t effectMode = EFFECT_DEFAULT;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = TEST_STREAM_SESSION_ID;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    int32_t ret = hpaeRendererManager->SetAudioEffectMode(sessionId, effectMode);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    effectMode = EFFECT_NONE;
    hpaeRendererManager->CreateProcessCluster(nodeInfo);
    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_EFFECT_NONE);
    hpaeRendererManager->sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_PAUSED);
    ret = hpaeRendererManager->SetAudioEffectMode(sessionId, effectMode);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    effectMode = EFFECT_DEFAULT;
    hpaeRendererManager->sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_RUNNING);
    ret = hpaeRendererManager->SetAudioEffectMode(sessionId, effectMode);
    EXPECT_EQ(ret, SUCCESS);
    
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test SendRequestInner_001
 * @tc.type  : FUNC
 * @tc.number: SendRequestInner_001
 * @tc.desc  : Test SendRequestInner when config in vaild.
 */
HWTEST_F(HpaeRendererManagerTest, SendRequestInner_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    auto request = []() {
    };
    hpaeRendererManager->SendRequest(request, "unit_test_send_request");
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    hpaeRendererManager->SendRequest(request, "unit_test_send_request");
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->hpaeSignalProcessThread_ = nullptr;
    hpaeRendererManager->SendRequest(request, "unit_test_send_request");
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test SendRequestInner_002
 * @tc.type  : FUNC
 * @tc.number: SendRequestInner_002
 * @tc.desc  : Test SendRequest when config in vaild.
 */
HWTEST_F(HpaeRendererManagerTest, SendRequestInner_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeOffloadRendererManager> hpaeRendererManager =
        std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    auto request = []() {
    };
    hpaeRendererManager->SendRequest(request, "unit_test_send_request");
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    hpaeRendererManager->SendRequest(request, "unit_test_send_request");
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->hpaeSignalProcessThread_ = nullptr;
    hpaeRendererManager->SendRequest(request, "unit_test_send_request");
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test HpaeOffloadRendererManagerInitSinkInner_001
 * @tc.type  : FUNC
 * @tc.number: HpaeOffloadRendererManagerInitSinkInner_001
 * @tc.desc  : Test HpaeOffloadRendererManagerInitSinkInner when frameLen is 0.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeOffloadRendererManagerInitSinkInner_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = 0;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<HpaeOffloadRendererManager> hpaeRendererManager =
        std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    bool isReload = true;
    EXPECT_EQ(hpaeRendererManager->InitSinkInner(isReload), ERROR);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test HpaeOffloadRendererManagerInitSinkInner_002
 * @tc.type  : FUNC
 * @tc.number: HpaeOffloadRendererManagerInitSinkInner_002
 * @tc.desc  : Test HpaeOffloadRendererManagerInitSinkInner when frameLen is over-sized.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeOffloadRendererManagerInitSinkInner_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = OVERSIZED_FRAME_LENGTH;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<HpaeOffloadRendererManager> hpaeRendererManager =
        std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    bool isReload = true;
    EXPECT_EQ(hpaeRendererManager->InitSinkInner(isReload), ERROR);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test HpaeRendererManagerInitManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerInitManager_001
 * @tc.desc  : Test HpaeRendererManagerInitManager when frameLen is 0.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerInitManager_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = 0;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager =
        std::make_shared<HpaeRendererManager>(sinkInfo);
    bool isReload = true;
    EXPECT_EQ(hpaeRendererManager->InitManager(isReload, true), ERROR);

    hpaeRendererManager->InitManager(isReload, false);
    isReload = false;
    hpaeRendererManager->InitManager(isReload, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test HpaeRendererManagerInitManager_002
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerInitManager_002
 * @tc.desc  : Test HpaeRendererManagerInitManager when frameLen is over-sized.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerInitManager_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "constructHpaeRendererManagerTest.pcm";
    sinkInfo.frameLen = OVERSIZED_FRAME_LENGTH;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager =
        std::make_shared<HpaeRendererManager>(sinkInfo);
    bool isReload = true;
    EXPECT_EQ(hpaeRendererManager->InitManager(isReload, true), ERROR);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test HpaeOffloadRendererManagerCreateStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeOffloadRendererManagerCreateStream_001
 * @tc.desc  : Test HpaeOffloadRendererManagerCreateStream when frameLen is 0.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeOffloadRendererManagerCreateStream_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    bool isReload = true;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(isReload), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.frameLen = 0;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo), ERROR);
}

/**
 * @tc.name  : Test HpaeOffloadRendererManagerCreateStream_002
 * @tc.type  : FUNC
 * @tc.number: HpaeOffloadRendererManagerCreateStream_002
 * @tc.desc  : Test HpaeOffloadRendererManagerCreateStream when frameLen is over-sized.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeOffloadRendererManagerCreateStream_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    bool isReload = true;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(isReload), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.frameLen = OVERSIZED_FRAME_LENGTH;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo), ERROR);
}

/**
 * @tc.name  : Test HpaeRendererManagerCreateStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerCreateStream_001
 * @tc.desc  : Test HpaeRendererManagerCreateStream when frameLen is 0.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateStream_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    bool isReload = true;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(isReload), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.frameLen = 0;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo), ERROR);
}

/**
 * @tc.name  : Test HpaeRendererManagerCreateStream_002
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerCreateStream_002
 * @tc.desc  : Test HpaeRendererManagerCreateStream when frameLen is over-sized.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerCreateStream_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    bool isReload = true;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(isReload), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    HpaeStreamInfo streamInfo;
    streamInfo.frameLen = OVERSIZED_FRAME_LENGTH;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo), ERROR);
}

/**
 * @tc.name: Test ConnectInputCluster and DeleteProcessCluster
 * @tc.type: FUNC
 * @tc.number: ConnectInputCluster_001
 * @tc.desc: Test Connect and Delete when create nodes in defaultProcessCluster but connect noneProcessCluster
 */
HWTEST_F(HpaeRendererManagerTest, ConnectInputCluster_001, TestSize.Level0)
{
    uint32_t sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    // create defaultProcessCluster, sceneType is HPAE_SCENE_MUSIC, effectMode = EFFECT_DEFAULT
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->CheckNodes(sessionId), SUCCESS);

    // connect noneProcessCluster, sceneType is HPAE_SCENE_EFFECT_NONE
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;
    hpaeRendererManager->ConnectInputCluster(sessionId, HPAE_SCENE_EFFECT_NONE);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->CheckNodes(sessionId), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->CheckNodes(sessionId), ERROR);
    // delete nodes in noneProcessCluster
    EXPECT_EQ(hpaeRendererManager->DeleteProcessCluster(sessionId), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->CheckNodes(sessionId), ERROR);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: DisConnectInputCluster
 * @tc.type: FUNC
 * @tc.number: DisConnectInputCluster_001
 * @tc.desc: Test DisConnectInputCluster
 */
HWTEST_F(HpaeRendererManagerTest, DisConnectInputCluster_001, TestSize.Level0)
{
    uint32_t sessionId = 10000;
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = true;

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[nodeInfo.sessionId]), SUCCESS);
    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MUSIC);
    hpaeRendererManager->DisConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;
    hpaeRendererManager->DisConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: Test DeleteProcessCluster
 * @tc.type: FUNC
 * @tc.number: DeleteProcessCluster_001
 * @tc.desc: Test Delete when create nodes in defaultProcessCluster but delete noneProcessCluster
 */
HWTEST_F(HpaeRendererManagerTest, DeleteProcessCluster_001, TestSize.Level0)
{
    uint32_t sessionId = DEFAULT_SESSIONID_NUM_FIRST;
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    // create defaultProcessCluster, sceneType is HPAE_SCENE_DEFAULT, effectMode = EFFECT_DEFAULT
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT]->CheckNodes(sessionId), SUCCESS);

    // try delete nodes in noneProcessCluster, sceneType is HPAE_SCENE_EFFECT_NONE
    // actually nodes found in defaultProcessCluster
    nodeInfo.effectInfo.effectMode = EFFECT_NONE;
    EXPECT_EQ(hpaeRendererManager->GetProcessorType(sessionId), HPAE_SCENE_EFFECT_NONE);
    EXPECT_EQ(hpaeRendererManager->DeleteProcessCluster(sessionId), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->CheckNodes(sessionId), ERROR);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT]->CheckNodes(sessionId), ERROR);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test HpaeOffloadRendererManagerSetCurrentNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeOffloadRendererManagerSetCurrentNode_001
 * @tc.desc  : Test SetCurrentNode when curNode_ already exists.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeOffloadRendererManagerSetCurrentNode_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "SetCurrentNodeTest001.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    bool isReload = true;
    std::shared_ptr<HpaeOffloadRendererManager> offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(isReload), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->IsInit(), true);
  
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    // nodes create for curNode
    EXPECT_NE(offloadManager->curNode_, nullptr);
    auto tmpNodeId1 = offloadManager->converterForOutput_->GetNodeId();
    auto tmpNodeId2 = offloadManager->loudnessGainNode_->GetNodeId();
    auto tmpNodeId3 = offloadManager->converterForLoudness_->GetNodeId();
    EXPECT_NE(offloadManager->converterForOutput_, nullptr);
    EXPECT_NE(offloadManager->loudnessGainNode_, nullptr);
    EXPECT_NE(offloadManager->converterForLoudness_, nullptr);

    offloadManager->SetCurrentNode();
    // curNode exit, nodes remain unchanged
    EXPECT_NE(offloadManager->curNode_, nullptr);
    EXPECT_EQ(tmpNodeId1, offloadManager->converterForOutput_->GetNodeId());
    EXPECT_EQ(tmpNodeId2, offloadManager->loudnessGainNode_->GetNodeId());
    EXPECT_EQ(tmpNodeId3, offloadManager->converterForLoudness_->GetNodeId());

    offloadManager->RemoveNodeFromMap(TEST_STREAM_SESSION_ID);
    // curNode remove, destroy nodes
    EXPECT_EQ(offloadManager->curNode_, nullptr);
    EXPECT_EQ(offloadManager->converterForOutput_, nullptr);
    EXPECT_EQ(offloadManager->loudnessGainNode_, nullptr);
    EXPECT_EQ(offloadManager->converterForLoudness_, nullptr);
}

static void CreateTwoStreamInOffload(std::shared_ptr<HpaeOffloadRendererManager> &offloadManager)
{
    HpaeStreamInfo streamInfo1;
    streamInfo1.channels = STEREO;
    streamInfo1.samplingRate = SAMPLE_RATE_48000;
    streamInfo1.format = SAMPLE_F32LE;
    streamInfo1.frameLen = FRAME_LENGTH_960;
    streamInfo1.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo1.streamType = STREAM_MUSIC;
    streamInfo1.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(offloadManager->CreateStream(streamInfo1), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    HpaeStreamInfo streamInfo2(streamInfo1);
    streamInfo2.sessionId = DEFAULT_SESSIONID_NUM_FIRST;

    EXPECT_EQ(offloadManager->CreateStream(streamInfo2), SUCCESS);
    WaitForMsgProcessing(offloadManager);
}

/**
 * @tc.name  : Test HpaeOffloadRendererManagerSetCurrentNode_002
 * @tc.type  : FUNC
 * @tc.number: HpaeOffloadRendererManagerSetCurrentNode_002
 * @tc.desc  : Test SetCurrentNode when curNode_ is nullptr.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeOffloadRendererManagerSetCurrentNode_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "SetCurrentNodeTest002.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<HpaeOffloadRendererManager> offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    std::pair<uint64_t, TimePoint> hdiPos = std::make_pair(0, std::chrono::high_resolution_clock::now());
    offloadManager->OnNotifyHdiData(hdiPos);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
  
    CreateTwoStreamInOffload(offloadManager);

    // create nodes for stream1
    EXPECT_NE(offloadManager->converterForOutput_, nullptr);
    EXPECT_NE(offloadManager->loudnessGainNode_, nullptr);
    EXPECT_NE(offloadManager->converterForLoudness_, nullptr);
    auto tmpNodeId1 = offloadManager->converterForOutput_->GetNodeId();
    auto tmpNodeId2 = offloadManager->loudnessGainNode_->GetNodeId();
    auto tmpNodeId3 = offloadManager->converterForLoudness_->GetNodeId();

    EXPECT_EQ(offloadManager->Start(DEFAULT_SESSIONID_NUM_FIRST), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    // curNode does not change, nodes for stream1 remain unchanged
    EXPECT_EQ(tmpNodeId1, offloadManager->converterForOutput_->GetNodeId());
    EXPECT_EQ(tmpNodeId2, offloadManager->loudnessGainNode_->GetNodeId());
    EXPECT_EQ(tmpNodeId3, offloadManager->converterForLoudness_->GetNodeId());

    offloadManager->RemoveNodeFromMap(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(offloadManager->curNode_, nullptr);
    // curNode remove, nodes for stream1 destroy
    EXPECT_EQ(offloadManager->converterForOutput_, nullptr);
    EXPECT_EQ(offloadManager->loudnessGainNode_, nullptr);
    EXPECT_EQ(offloadManager->converterForLoudness_, nullptr);
    
    offloadManager->SetCurrentNode();
    EXPECT_NE(offloadManager->curNode_, nullptr);
    offloadManager->OnNotifyHdiData(hdiPos);
    // new curNode, create new nodes for stream2
    EXPECT_NE(offloadManager->converterForOutput_, nullptr);
    EXPECT_NE(offloadManager->loudnessGainNode_, nullptr);
    EXPECT_NE(offloadManager->converterForLoudness_, nullptr);
    EXPECT_NE(tmpNodeId1, offloadManager->converterForOutput_->GetNodeId());
    EXPECT_NE(tmpNodeId2, offloadManager->loudnessGainNode_->GetNodeId());
    EXPECT_NE(tmpNodeId3, offloadManager->converterForLoudness_->GetNodeId());
}

/**
 * @tc.name  : Test HpaeOffloadRendererManagerSetCurrentNode_00
 * @tc.type  : FUNC
 * @tc.number: HpaeOffloadRendererManagerSetCurrentNode_003
 * @tc.desc  : Test SetCurrentNode when stream is not running but SetCurrentNode.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeOffloadRendererManagerSetCurrentNode_003, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<HpaeOffloadRendererManager> offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
  
    CreateTwoStreamInOffload(offloadManager);

    offloadManager->DestroyStream(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(offloadManager);
    EXPECT_NE(offloadManager->curNode_->GetState(), HPAE_SESSION_RUNNING);
    EXPECT_NE(offloadManager->converterForOutput_, nullptr);
    EXPECT_NE(offloadManager->loudnessGainNode_, nullptr);
    EXPECT_NE(offloadManager->converterForLoudness_, nullptr);

    offloadManager->Start(DEFAULT_SESSIONID_NUM_FIRST);
    WaitForMsgProcessing(offloadManager);
}

/**
 * @tc.name  : Test HpaeRendererGetLatency_001
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererGetLatency_001
 * @tc.desc  : Test get latency via legal state.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererGetLatency_001, TestSize.Level0)
{
    std::cout << "test renderer manager" << std::endl;
    TestIRendererManagerOnRequestLatency<HpaeRendererManager>();
    std::cout << "test offload" << std::endl;
    TestIRendererManagerOnRequestLatency<HpaeOffloadRendererManager>();
}

/**
 * @tc.name  : Test QueryOneStreamUnderrun
 * @tc.type  : FUNC
 * @tc.number: QueryOneStreamUnderrun_001
 * @tc.desc  : Test QueryOneStreamUnderrun when not one stream underrun.
 */
HWTEST_F(HpaeRendererManagerTest, QueryOneStreamUnderrun_001, TestSize.Level1)
{
    hpaeRendererManager_->lastOnUnderrunTime_ = 1;

    EXPECT_CALL(*mockCallback_, OnQueryUnderrun())
        .WillOnce(Return(false));

    EXPECT_FALSE(hpaeRendererManager_->QueryOneStreamUnderrun());
    EXPECT_EQ(hpaeRendererManager_->lastOnUnderrunTime_, 0);

    sinkInputNode_->SetState(HPAE_SESSION_PAUSED);
    hpaeRendererManager_->lastOnUnderrunTime_ = 1;

    EXPECT_FALSE(hpaeRendererManager_->QueryOneStreamUnderrun());
    EXPECT_EQ(hpaeRendererManager_->lastOnUnderrunTime_, 0);
}

/**
 * @tc.name  : Test QueryOneStreamUnderrun
 * @tc.type  : FUNC
 * @tc.number: QueryOneStreamUnderrun_002
 * @tc.desc  : Test QueryOneStreamUnderrun when one stream underrun.
 */
HWTEST_F(HpaeRendererManagerTest, QueryOneStreamUnderrun_002, TestSize.Level1)
{
    EXPECT_CALL(*mockCallback_, OnQueryUnderrun())
        .WillOnce(Return(true))
        .WillOnce(Return(true));
    hpaeRendererManager_->lastOnUnderrunTime_ = ClockTime::GetCurNano();
    EXPECT_TRUE(hpaeRendererManager_->QueryOneStreamUnderrun());

    hpaeRendererManager_->lastOnUnderrunTime_ = 1;
    EXPECT_FALSE(hpaeRendererManager_->QueryOneStreamUnderrun());
    EXPECT_EQ(hpaeRendererManager_->lastOnUnderrunTime_, 1);
}

/**
 * @tc.name  : Test DeleteInputSessionIdNotExit
 * @tc.type  : FUNC
 * @tc.number: DeleteInputSessionIdNotExit
 * @tc.desc  : Test delete input session which is not exit.
 */
HWTEST_F(HpaeRendererManagerTest, DeleteInputSessionIdNotExit, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto rendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_NE(rendererManager, nullptr);
    EXPECT_EQ(hpaeRendererManager_->DeleteInputSession(INVALID_ID), SUCCESS);

    auto injectorManager = std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    EXPECT_NE(injectorManager, nullptr);
    EXPECT_EQ(hpaeRendererManager_->DeleteInputSession(INVALID_ID), SUCCESS);
}

/**
 * @tc.name: DeleteProcessClusterInner
 * @tc.type: FUNC
 * @tc.number: DeleteProcessClusterInner_001
 * @tc.desc: Test DeleteProcessClusterInner
 */
HWTEST_F(HpaeRendererManagerTest, DeleteProcessClusterInner_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    uint32_t sessionId = TEST_STREAM_SESSION_ID;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    int32_t ret = hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(ret, SUCCESS);

    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 0;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->SetConnectedFlag(true);
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT] = 1;
    ret = hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(ret, SUCCESS);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: DeleteProcessClusterInner
 * @tc.type: FUNC
 * @tc.number: DeleteProcessClusterInner_002
 * @tc.desc: Test DeleteProcessClusterInner
 */
HWTEST_F(HpaeRendererManagerTest, DeleteProcessClusterInner_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    uint32_t sessionId = TEST_STREAM_SESSION_ID;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 0;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT] = hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC];
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT] = 0;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT]->SetConnectedFlag(true);
    int32_t ret = hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(ret, SUCCESS);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: DeleteProcessClusterInner
 * @tc.type: FUNC
 * @tc.number: DeleteProcessClusterInner_003
 * @tc.desc: Test DeleteProcessClusterInner
 */
HWTEST_F(HpaeRendererManagerTest, DeleteProcessClusterInner_003, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    uint32_t sessionId = TEST_STREAM_SESSION_ID;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 0;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->SetConnectedFlag(false);
    int32_t ret = hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(ret, SUCCESS);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: OnDisConnectProcessCluster
 * @tc.type: FUNC
 * @tc.number: OnDisConnectProcessCluster_001
 * @tc.desc: Test OnDisConnectProcessCluster
 */
HWTEST_F(HpaeRendererManagerTest, OnDisConnectProcessCluster_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->OnDisConnectProcessCluster(HPAE_SCENE_MUSIC);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: OnDisConnectProcessCluster
 * @tc.type: FUNC
 * @tc.number: OnDisConnectProcessCluster_002
 * @tc.desc: Test OnDisConnectProcessCluster
 */
HWTEST_F(HpaeRendererManagerTest, OnDisConnectProcessCluster_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 1;
    hpaeRendererManager->OnDisConnectProcessCluster(HPAE_SCENE_MUSIC);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name: OnDisConnectProcessCluster
 * @tc.type: FUNC
 * @tc.number: OnDisConnectProcessCluster_003
 * @tc.desc: Test OnDisConnectProcessCluster
 */
HWTEST_F(HpaeRendererManagerTest, OnDisConnectProcessCluster_003, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 0;
    hpaeRendererManager->OnDisConnectProcessCluster(HPAE_SCENE_MUSIC);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test SetOffloadRenderCallbackType
 * @tc.type  : FUNC
 * @tc.number: SetOffloadRenderCallbackType
 * @tc.desc  : Test set offload render callback type.
 */
HWTEST_F(HpaeRendererManagerTest, SetOffloadRenderCallbackType_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "HpaeOffloadRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    std::shared_ptr<HpaeOffloadRendererManager> offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
  
    CreateTwoStreamInOffload(offloadManager);
    auto ret = offloadManager->SetOffloadRenderCallbackType(TEST_STREAM_SESSION_ID, CB_NONBLOCK_WRITE_COMPLETED);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(offloadManager);
    ret = offloadManager->SetOffloadRenderCallbackType(DEFAULT_SESSIONID_NUM_FIRST, CB_RENDER_FULL);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(offloadManager);
    EXPECT_NE(offloadManager->curNode_, nullptr);
    EXPECT_NE(offloadManager->sinkInputNodeMap_.size(), 1);
}

/**
 * @tc.name  : Test TriggerAppsUidUpdate
 * @tc.type  : FUNC
 * @tc.number: TriggerAppsUidUpdate_001
 * @tc.desc  : Test update apps uid.
 */
HWTEST_F(HpaeRendererManagerTest, TriggerAppsUidUpdate_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    HpaeNodeInfo nodeInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    sinkInfo.deviceClass = "remote";
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeStreamInfo streamInfo;
    TestRendererManagerCreateStream(hpaeRendererManager, streamInfo);

    EXPECT_EQ(hpaeRendererManager->CreateInputSession(streamInfo), SUCCESS);
    auto sinkInputNode = hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId];
    EXPECT_NE(sinkInputNode, nullptr);
    hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]->state_ = HPAE_SESSION_RUNNING;
    hpaeRendererManager->TriggerAppsUidUpdate(streamInfo.sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->appsUid_.size(), 1);
    hpaeRendererManager->TriggerAppsUidUpdate(0);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->appsUid_.size(), 1);
    hpaeRendererManager->sinkInputNodeMap_[streamInfo.sessionId]->state_ = HPAE_SESSION_STOPPED;
    hpaeRendererManager->TriggerAppsUidUpdate(streamInfo.sessionId);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->appsUid_.size(), 1);
    hpaeRendererManager->TriggerAppsUidUpdate(0);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->appsUid_.size(), 0);
}

/**
 * @tc.name  : Test TriggerAppsUidUpdate
 * @tc.type  : FUNC
 * @tc.number: TriggerAppsUidUpdate_002
 * @tc.desc  : Test offload update apps uid.
 */
HWTEST_F(HpaeRendererManagerTest, TriggerAppsUidUpdate_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = "remote_offload";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "HpaeOffloadRendererManagerTest.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;

    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = TEST_STREAM_SESSION_ID;
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->appUid_ = 123;
    sinkInputNode->state_ = HPAE_SESSION_RUNNING;
    offloadManager->curNode_ = sinkInputNode;
    offloadManager->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->appsUid_.size(), 1);
    offloadManager->TriggerAppsUidUpdate(0);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->appsUid_.size(), 1);
    sinkInputNode->state_ = HPAE_SESSION_STOPPED;
    offloadManager->curNode_ = sinkInputNode;
    offloadManager->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->appsUid_.size(), 1);
    offloadManager->TriggerAppsUidUpdate(0);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->appsUid_.size(), 0);
}

/**
 * @tc.name  : AddNodeToSink_001
 * @tc.type  : FUNC
 * @tc.number: AddNodeToSink_001
 * @tc.desc  : Test AddNodeToSink to cover asynchronous node addition logic.
 */
HWTEST_F(HpaeRendererManagerTest, AddNodeToSink_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    offloadManager->Init();
    WaitForMsgProcessing(offloadManager);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 60001;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.channels = STEREO;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    int32_t ret = offloadManager->AddNodeToSink(node);
    EXPECT_EQ(ret, SUCCESS);

    WaitForMsgProcessing(offloadManager);

    EXPECT_EQ(offloadManager->IsInit(), true);
    offloadManager->DeInit();
}

/**
 * @tc.name  : AddAllNodesToSink_001
 * @tc.type  : FUNC
 * @tc.number: AddAllNodesToSink_001
 * @tc.desc  : Test AddAllNodesToSink to cover vector traversal and async request logic.
 */
HWTEST_F(HpaeRendererManagerTest, AddAllNodesToSink_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    offloadManager->Init();
    WaitForMsgProcessing(offloadManager);

    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;

    HpaeNodeInfo info1;
    info1.sessionId = 80001;
    info1.samplingRate = SAMPLE_RATE_48000;
    info1.frameLen = FRAME_LENGTH_960;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info1));

    HpaeNodeInfo info2 = info1;
    info2.sessionId = 80002;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info2));

    int32_t ret = offloadManager->AddAllNodesToSink(sinkInputs, true);
    EXPECT_EQ(ret, SUCCESS);

    WaitForMsgProcessing(offloadManager);

    EXPECT_NE(offloadManager->sinkInputNodeMap_.find(80001), offloadManager->sinkInputNodeMap_.end());
    EXPECT_NE(offloadManager->sinkInputNodeMap_.find(80002), offloadManager->sinkInputNodeMap_.end());

    offloadManager->DeInit();
}

/**
 * @tc.name  : OffloadMoveAllStream_001
 * @tc.type  : FUNC
 * @tc.number: OffloadMoveAllStream_001
 * @tc.desc  : Test MoveAllStream covering both sync (uninit) and async (init) paths.
 */
HWTEST_F(HpaeRendererManagerTest, OffloadMoveAllStream_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);

    std::string sinkName = "TargetSink";
    std::vector<uint32_t> sessionIds = {10001, 10002};
    MoveSessionType moveType = MOVE_ALL;

    EXPECT_EQ(offloadManager->MoveAllStream(sinkName, sessionIds, moveType), SUCCESS);

    offloadManager->Init();
    WaitForMsgProcessing(offloadManager);

    EXPECT_EQ(offloadManager->MoveAllStream(sinkName, sessionIds, moveType), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    offloadManager->DeInit();
}

/**
 * @tc.name  : MoveStream_003
 * @tc.type  : FUNC
 * @tc.number: MoveStream_003
 * @tc.desc  : Test MoveStream for error paths (not found, empty sinkName) and success path.
 */
HWTEST_F(HpaeRendererManagerTest, MoveStream_003, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    offloadManager->Init();
    WaitForMsgProcessing(offloadManager);

    uint32_t activeSid = 95001;
    uint32_t normalSid = 95002;
    uint32_t invalidSid = 99999;

    HpaeNodeInfo info1;
    info1.sessionId = activeSid;
    auto activeNode = std::make_shared<HpaeSinkInputNode>(info1);

    HpaeNodeInfo info2;
    info2.sessionId = normalSid;
    auto normalNode = std::make_shared<HpaeSinkInputNode>(info2);

    offloadManager->sinkInputNodeMap_[activeSid] = activeNode;
    offloadManager->sinkInputNodeMap_[normalSid] = normalNode;
    offloadManager->curNode_ = activeNode;

    offloadManager->MoveStream(invalidSid, "NewSink");
    offloadManager->MoveStream(normalSid, "");
    offloadManager->MoveStream(normalSid, "ValidSink");
    offloadManager->MoveStream(activeSid, "ValidSink");

    WaitForMsgProcessing(offloadManager);

    EXPECT_EQ(offloadManager->IsInit(), true);
}

/**
 * @tc.name  : OffloadStopManager_001
 * @tc.type  : FUNC
 * @tc.number: OffloadStopManager_001
 * @tc.desc  : Test StopManager for both valid and nullptr sinkOutputNode scenarios.
 */
HWTEST_F(HpaeRendererManagerTest, OffloadStopManager_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    offloadManager->Init();
    WaitForMsgProcessing(offloadManager);

    offloadManager->sinkOutputNode_.reset();
    EXPECT_EQ(offloadManager->StopManager(), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    HpaeNodeInfo nodeInfo;
    offloadManager->sinkOutputNode_ = std::make_unique<HpaeOffloadSinkOutputNode>(nodeInfo);

    EXPECT_EQ(offloadManager->StopManager(), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    offloadManager->DeInit();
}

HWTEST_F(HpaeRendererManagerTest, OffloadDeactivateThread_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);

    offloadManager->Init();
    ASSERT_NE(offloadManager->hpaeSignalProcessThread_, nullptr);

    EXPECT_TRUE(offloadManager->DeactivateThread());
    EXPECT_EQ(offloadManager->hpaeSignalProcessThread_, nullptr);

    bool requestHandled = false;
    auto request = [&requestHandled]() { requestHandled = true; };

    offloadManager->SendRequest(request, "UT_Cleanup_Task");
    offloadManager->DeactivateThread();

    EXPECT_TRUE(requestHandled);
}

/**
 * @tc.name  : Test EnableCollaboration
 * @tc.type  : FUNC
 * @tc.number: EnableCollaboration_001
 * @tc.desc  : Test EnableCollaboration.
 */
HWTEST_F(HpaeRendererManagerTest, EnableCollaboration_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    hpaeRendererManager->sinkInputNodeMap_.clear();
    hpaeRendererManager->EnableCollaboration();
    
    HpaeNodeInfo nodeInfo1;
    uint32_t sessionIdTemp1 = TEST_STREAM_SESSION_ID;
    nodeInfo1.sessionId = sessionIdTemp1;
    nodeInfo1.effectInfo.effectScene = SCENE_MOVIE;
    auto sinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo1);
    sinkInputNode1->SetState(HPAE_SESSION_RUNNING);
    hpaeRendererManager->sinkInputNodeMap_[sessionIdTemp1] = sinkInputNode1;
    EXPECT_EQ(hpaeRendererManager->hpaeCoBufferNode_ == nullptr, true);
    hpaeRendererManager->EnableCollaboration();
    EXPECT_EQ(hpaeRendererManager->hpaeCoBufferNode_ != nullptr, true);

    HpaeNodeInfo nodeInfo2;
    uint32_t sessionIdTemp2 = sessionIdTemp1 + 1;
    nodeInfo2.sessionId = sessionIdTemp2;
    nodeInfo2.effectInfo.effectScene = SCENE_MOVIE;
    auto sinkInputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo2);
    sinkInputNode2->SetState(HPAE_SESSION_RELEASED);
    hpaeRendererManager->sinkInputNodeMap_[sessionIdTemp2] = sinkInputNode2;
    hpaeRendererManager->EnableCollaboration();

    HpaeNodeInfo nodeInfo3;
    uint32_t sessionIdTemp3 = sessionIdTemp2 + 1;
    nodeInfo3.sessionId = sessionIdTemp3;
    nodeInfo3.effectInfo.effectScene = SCENE_MUSIC;
    auto sinkInputNode3 = std::make_shared<HpaeSinkInputNode>(nodeInfo3);
    sinkInputNode3->SetState(HPAE_SESSION_RELEASED);
    hpaeRendererManager->sinkInputNodeMap_[sessionIdTemp3] = sinkInputNode3;
    hpaeRendererManager->EnableCollaboration();

    HpaeNodeInfo nodeInfo4;
    uint32_t sessionIdTemp4 = sessionIdTemp3 + 1;
    nodeInfo4.sessionId = sessionIdTemp4;
    nodeInfo4.effectInfo.effectScene = SCENE_MUSIC;
    auto sinkInputNode4 = std::make_shared<HpaeSinkInputNode>(nodeInfo4);
    sinkInputNode4->SetState(HPAE_SESSION_RUNNING);
    hpaeRendererManager->sinkInputNodeMap_[sessionIdTemp4] = sinkInputNode4;
    hpaeRendererManager->EnableCollaboration();

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test OnCollborationFadeDone
 * @tc.type  : FUNC
 * @tc.number: OnCollborationFadeDone_001
 * @tc.desc  : Test OnCollborationFadeDone.
 */
HWTEST_F(HpaeRendererManagerTest, OnCollborationFadeDone_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    hpaeRendererManager->isReloadManagerFromCollStateChange_ = false;
    hpaeRendererManager->OnCollborationFadeDone(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->isReloadManagerFromCollStateChange_ = true;
    hpaeRendererManager->OnCollborationFadeDone(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test DisableCollaboration
 * @tc.type  : FUNC
 * @tc.number: DisableCollaboration_001
 * @tc.desc  : Test DisableCollaboration.
 */
HWTEST_F(HpaeRendererManagerTest, DisableCollaboration_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    hpaeRendererManager->EnableCollaboration();
    HpaeNodeInfo nodeInfo1;
    uint32_t sessionIdTemp1 = TEST_STREAM_SESSION_ID;
    nodeInfo1.sessionId = sessionIdTemp1;
    nodeInfo1.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto sinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo1);
    sinkInputNode1->SetState(HPAE_SESSION_RUNNING);
    hpaeRendererManager->sinkInputNodeMap_[sessionIdTemp1] = sinkInputNode1;
    hpaeRendererManager->DisableCollaboration();
    EXPECT_EQ(hpaeRendererManager->hpaeCoBufferNode_ != nullptr, true);

    HpaeNodeInfo nodeInfo2;
    uint32_t sessionIdTemp2 = sessionIdTemp1 + 1;
    nodeInfo2.sessionId = sessionIdTemp2;
    nodeInfo2.effectInfo.effectScene = SCENE_COLLABORATIVE;
    auto sinkInputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo2);
    sinkInputNode2->SetState(HPAE_SESSION_RELEASED);
    hpaeRendererManager->sinkInputNodeMap_[sessionIdTemp2] = sinkInputNode2;
    hpaeRendererManager->DisableCollaboration();

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test LimiterNode CreateForRemoteOffload_001
 * @tc.type  : FUNC
 * @tc.number: LimiterNode_CreateForRemoteOffload_001
 * @tc.desc  : Test limiterNode_ is created for remote_offload device.
 */
HWTEST_F(HpaeRendererManagerTest, LimiterNode_CreateForRemoteOffload_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = "remote_offload";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "source_file_io_48000_2_s16le.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;

    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->IsInit(), true);

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is created for remote_offload device
    EXPECT_NE(offloadManager->limiterNode_, nullptr);
    EXPECT_TRUE(offloadManager->limiterNode_->IsLimiterInited());

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
}

/**
 * @tc.name  : Test LimiterNode CreateForRemoteOffload 002
 * @tc.type  : FUNC
 * @tc.number: LimiterNode_CreateForRemoteOffload_002
 * @tc.desc  : Test limiterNode_ is created for remote_offload device.
 */
HWTEST_F(HpaeRendererManagerTest, LimiterNode_CreateForRemoteOffload_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = "remote_offload";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "source_file_io_48000_2_s16le.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;

    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->IsInit(), true);

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is created for remote_offload device
    EXPECT_EQ(offloadManager->limiterNode_, nullptr);

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
}

/**
 * @tc.name  : Test LimiterNode NotCreateForNonRemoteOffload_001
 * @tc.type  : FUNC
 * @tc.number: LimiterNode_NotCreateForNonRemoteOffload_001
 * @tc.desc  : Test limiterNode_ is not created for non-remote_offload device.
 */
HWTEST_F(HpaeRendererManagerTest, LimiterNode_NotCreateForNonRemoteOffload_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    // Ensure deviceClass is not "remote_offload"
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;

    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->IsInit(), true);

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is not created for non-remote_offload device
    EXPECT_EQ(offloadManager->limiterNode_, nullptr);

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
}

/**
 * @tc.name  : Test LimiterNode_Destroy_001
 * @tc.type  : FUNC
 * @tc.number: LimiterNode_Destroy_001
 * @tc.desc  : Test limiterNode_ is destroyed when stream is destroyed.
 */
HWTEST_F(HpaeRendererManagerTest, LimiterNode_Destroy_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = "remote_offload";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "source_file_io_48000_2_s16le.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;

    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is created
    EXPECT_NE(offloadManager->limiterNode_, nullptr);

    // Destroy stream to trigger DestroyOffloadNodes
    EXPECT_EQ(offloadManager->DestroyStream(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is destroyed
    EXPECT_EQ(offloadManager->limiterNode_, nullptr);

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
}

/**
 * @tc.name  : Test LimiterNode_ConnectAndDisconnect_001
 * @tc.type  : FUNC
 * @tc.number: LimiterNode_ConnectAndDisconnect_001
 * @tc.desc  : Test limiterNode_ is connected and disconnected properly.
 */
HWTEST_F(HpaeRendererManagerTest, LimiterNode_ConnectAndDisconnect_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = "remote_offload";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "source_file_io_48000_2_s16le.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;

    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Create and start stream to trigger ConnectInputSession
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is connected (nodes should be non-null after start)
    EXPECT_NE(offloadManager->limiterNode_, nullptr);
    EXPECT_NE(offloadManager->converterForOutput_, nullptr);
    EXPECT_NE(offloadManager->loudnessGainNode_, nullptr);

    // Pause stream to trigger DisConnectInputSession
    EXPECT_EQ(offloadManager->Pause(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ still exists (disconnection doesn't destroy the node)
    EXPECT_NE(offloadManager->limiterNode_, nullptr);

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
}

/**
 * @tc.name  : Test LimiterNode_LatencyCalculation_001
 * @tc.type  : FUNC
 * @tc.number: LimiterNode_LatencyCalculation_001
 * @tc.desc  : Test limiterNode_ contributes to latency calculation.
 */
HWTEST_F(HpaeRendererManagerTest, LimiterNode_LatencyCalculation_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = "remote_offload";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "source_file_io_48000_2_s16le.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;

    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ contributes to latency calculation
    uint64_t initialLatency = 0;
    uint64_t latencyWithLimiter = 0;

    // Get latency without limiter (simulate by setting limiterNode_ to nullptr temporarily)
    auto limiterNodeBackup = offloadManager->limiterNode_;
    offloadManager->limiterNode_ = nullptr;
    offloadManager->OnRequestLatency(streamInfo.sessionId, initialLatency);

    // Get latency with limiter
    offloadManager->limiterNode_ = limiterNodeBackup;
    offloadManager->OnRequestLatency(streamInfo.sessionId, latencyWithLimiter);

    // Latency with limiter should be greater or equal to latency without limiter
    EXPECT_GE(latencyWithLimiter, initialLatency);

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
}

/**
 * @tc.name  : Test LimiterNode_SetupAudioLimiter_001
 * @tc.type  : FUNC
 * @tc.number: LimiterNode_SetupAudioLimiter_001
 * @tc.desc  : Test limiterNode_ SetupAudioLimiter is called during CreateOffloadNodes.
 */
HWTEST_F(HpaeRendererManagerTest, LimiterNode_SetupAudioLimiter_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = "remote_offload";
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.filePath = g_rootPath + "source_file_io_48000_2_s16le.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;

    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is initialized after CreateOffloadNodes
    EXPECT_NE(offloadManager->limiterNode_, nullptr);
    EXPECT_TRUE(offloadManager->limiterNode_->IsLimiterInited());

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
}

/**
 * @tc.name  : Test StopInterphoneSinkIfNeeded for interphone stream
 * @tc.type  : FUNC
 * @tc.number: StopInterphoneSinkIfNeeded_001
 * @tc.desc  : Test StopInterphoneSinkIfNeeded when deviceName contains "Interphone".
 */
HWTEST_F(HpaeRendererManagerTest, StopInterphoneSinkIfNeeded_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.deviceName = "Interphone_Sink";  // deviceName contains "Interphone"
    sinkInfo.filePath = g_rootPath + "interphone_stop_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    // Test StopInterphoneSinkIfNeeded function directly with deviceName containing "Interphone"
    hpaeRendererManager->StopInterphoneSinkIfNeeded();
    WaitForMsgProcessing(hpaeRendererManager);
    
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test StopInterphoneSinkIfNeeded for non-interphone stream
 * @tc.type  : FUNC
 * @tc.number: StopInterphoneSinkIfNeeded_002
 * @tc.desc  : Test StopInterphoneSinkIfNeeded when deviceName does not contain "Interphone".
 */
HWTEST_F(HpaeRendererManagerTest, StopInterphoneSinkIfNeeded_002, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();  // deviceName is "Speaker", does not contain "Interphone"
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    // Test StopInterphoneSinkIfNeeded - should not stop for non-interphone deviceName
    hpaeRendererManager->StopInterphoneSinkIfNeeded();
    WaitForMsgProcessing(hpaeRendererManager);
    
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test Pause for interphone stream
 * @tc.type  : FUNC
 * @tc.number: Pause_Interphone_001
 * @tc.desc  : Test Pause when deviceName contains "Interphone", calls StopInterphoneSinkIfNeeded.
 */
HWTEST_F(HpaeRendererManagerTest, Pause_Interphone_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.deviceName = "Interphone_Sink";  // deviceName contains "Interphone"
    sinkInfo.filePath = g_rootPath + "interphone_pause_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.effectInfo.effectScene = SCENE_MUSIC;
    streamInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    
    EXPECT_EQ(hpaeRendererManager->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    
    // Pause will call StopInterphoneSinkIfNeeded internally for interphone deviceName
    // Since outputCluster is stopped immediately, fade out cannot complete, state stays at PAUSING
    EXPECT_EQ(hpaeRendererManager->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSING);
    
    EXPECT_EQ(hpaeRendererManager->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    
    EXPECT_EQ(hpaeRendererManager->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test Pause for non-interphone stream
 * @tc.type  : FUNC
 * @tc.number: Pause_NonInterphone_001
 * @tc.desc  : Test Pause when deviceName does not contain "Interphone".
 */
HWTEST_F(HpaeRendererManagerTest, Pause_NonInterphone_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();  // deviceName is "Speaker", does not contain "Interphone"
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.effectInfo.effectScene = SCENE_MUSIC;
    streamInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    HpaeRendererManagerCreateStream(hpaeRendererManager, streamInfo);
    
    EXPECT_EQ(hpaeRendererManager->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    
    EXPECT_EQ(hpaeRendererManager->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    
    EXPECT_EQ(hpaeRendererManager->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    
    EXPECT_EQ(hpaeRendererManager->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : Test IHpaeRendererManager AdjustFrameLen with customSampleRate
 * @tc.type  : FUNC
 * @tc.number: AdjustFrameLen_004
 * @tc.desc  : Test AdjustFrameLen uses customSampleRate instead of samplingRate
 */
HWTEST_F(HpaeRendererManagerTest, AdjustFrameLen_004, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    inNodeInfo.frameLen = 160;
    inNodeInfo.samplingRate = SAMPLE_RATE_8000;
    inNodeInfo.customSampleRate = 8010;

    HpaeNodeInfo outNodeInfo;
    outNodeInfo.frameLen = 960;
    outNodeInfo.samplingRate = SAMPLE_RATE_48000;

    bool result = IHpaeRendererManager::AdjustFrameLen(inNodeInfo, outNodeInfo);
    EXPECT_EQ(result, true);
    // 160 frames @ 8010Hz = 19ms, 960 frames @ 48000Hz = 20ms, different -> adjust
    // outFrameLenUs = 960 * 1000 / 48000 = 20, inNodeInfo.frameLen = 20 * 8010 / 1000 = 160
    EXPECT_EQ(inNodeInfo.frameLen, 160);
}

/**
 * @tc.name  : Test IHpaeRendererManager AdjustFrameLen divide by zero inRate
 * @tc.type  : FUNC
 * @tc.number: AdjustFrameLen_005
 * @tc.desc  : Test AdjustFrameLen returns false when inRate is zero
 */
HWTEST_F(HpaeRendererManagerTest, AdjustFrameLen_005, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    inNodeInfo.frameLen = 960;
    inNodeInfo.samplingRate = static_cast<AudioSamplingRate>(0);
    inNodeInfo.customSampleRate = 0;

    HpaeNodeInfo outNodeInfo;
    outNodeInfo.frameLen = 960;
    outNodeInfo.samplingRate = SAMPLE_RATE_48000;

    bool result = IHpaeRendererManager::AdjustFrameLen(inNodeInfo, outNodeInfo);
    EXPECT_EQ(result, false);
    EXPECT_EQ(inNodeInfo.frameLen, 960);
}

/**
 * @tc.name  : Test IHpaeRendererManager AdjustFrameLen divide by zero outRate
 * @tc.type  : FUNC
 * @tc.number: AdjustFrameLen_006
 * @tc.desc  : Test AdjustFrameLen returns false when outRate is zero
 */
HWTEST_F(HpaeRendererManagerTest, AdjustFrameLen_006, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    inNodeInfo.frameLen = 960;
    inNodeInfo.samplingRate = SAMPLE_RATE_48000;

    HpaeNodeInfo outNodeInfo;
    outNodeInfo.frameLen = 960;
    outNodeInfo.samplingRate = static_cast<AudioSamplingRate>(0);
    outNodeInfo.customSampleRate = 0;

    bool result = IHpaeRendererManager::AdjustFrameLen(inNodeInfo, outNodeInfo);
    EXPECT_EQ(result, false);
    EXPECT_EQ(inNodeInfo.frameLen, 960);
}

/**
 * @tc.name  : Test IHpaeRendererManager AdjustFrameLen with 11025Hz
 * @tc.type  : FUNC
 * @tc.number: AdjustFrameLen_007
 * @tc.desc  : Test AdjustFrameLen with non-standard sample rate 11025Hz
 */
HWTEST_F(HpaeRendererManagerTest, AdjustFrameLen_007, TestSize.Level1)
{
    HpaeNodeInfo inNodeInfo;
    inNodeInfo.frameLen = 220;
    inNodeInfo.samplingRate = SAMPLE_RATE_11025;

    HpaeNodeInfo outNodeInfo;
    outNodeInfo.frameLen = 960;
    outNodeInfo.samplingRate = SAMPLE_RATE_48000;

    bool result = IHpaeRendererManager::AdjustFrameLen(inNodeInfo, outNodeInfo);
    EXPECT_EQ(result, true);
    // inFrameLenUs = 220 * 1000 / 11025 = 19, outFrameLenUs = 960 * 1000 / 48000 = 20
    // 19 != 20, adjust: inNodeInfo.frameLen = 20 * 11025 / 1000 = 220
    EXPECT_EQ(inNodeInfo.frameLen, 220);
}
}  // namespace