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
#include "hpae_injector_renderer_manager.h"
#include "hpae_sink_virtual_output_node.h"
#include "hpae_node_common.h"
#include "hpae_mocks.h"
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
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;

class HpaeInjectorRendererManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaeInjectorRendererManagerTest::SetUp()
{}

void HpaeInjectorRendererManagerTest::TearDown()
{}

static void WaitForMsgProcessing(std::shared_ptr<HpaeInjectorRendererManager> &hpaeRendererManager)
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

static std::shared_ptr<HpaeSinkVirtualOutputNode> SetSinkVirtualOutputNode(const HpaeSinkInfo &sinkInfo,
    const std::shared_ptr<IHpaeRendererManager> &rendererManager)
{
    HpaeNodeInfo nodeInfo;
    TransSinkInfoToNodeInfo(sinkInfo, rendererManager, nodeInfo);
    std::shared_ptr<HpaeSinkVirtualOutputNode> sinkOutputNode = std::make_shared<HpaeSinkVirtualOutputNode>(nodeInfo);
    rendererManager->SetSinkVirtualOutputNode(sinkOutputNode);
    return sinkOutputNode;
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

void TestRendererManagerCreateStream(
    std::shared_ptr<HpaeInjectorRendererManager> &hpaeRendererManager, HpaeStreamInfo &streamInfo)
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

/**
 * @tc.name  : Test HpaeInjectorRendererManagerStartPauseTest_001
 * @tc.type  : FUNC
 * @tc.number: HpaeInjectorRendererManagerStartPauseTest_001
 * @tc.desc  : Test HpaeInjectorRendererManager start pause stop with
 */
HWTEST_F(HpaeInjectorRendererManagerTest, HpaeInjectorRendererManagerStartPauseTest_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto hpaeRendererManager = std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    auto sinkOutputNode = SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
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

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->CheckIsStreamRunning(), true);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
    EXPECT_EQ(sinkOutputNode->GetIsReadFinished(), false);
    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSING);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId) == SUCCESS, true);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_RUNNING);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
    EXPECT_EQ(sinkOutputNode->GetIsReadFinished(), false);
    EXPECT_EQ(hpaeRendererManager->Stop(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    auto it = hpaeRendererManager->sinkInputNodeMap_.find(streamInfo.sessionId);
    ASSERT_EQ(it != hpaeRendererManager->sinkInputNodeMap_.end(), true);
    auto sinkInputNode = it->second;
    ASSERT_EQ(sinkInputNode != nullptr, true);
    hpaeRendererManager->TriggerStreamState(streamInfo.sessionId, sinkInputNode);

    EXPECT_EQ(hpaeRendererManager->Drain(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Flush(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == SUCCESS, true);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_STOPPED);
    EXPECT_EQ(hpaeRendererManager->Release(streamInfo.sessionId) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(
        hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo) == ERR_INVALID_OPERATION, true);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
}

/**
 * @tc.name  : Test HpaeInjectorRendererManagerTest
 * @tc.type  : FUNC
 * @tc.number: HpaeInjectorRendererManagerTest
 * @tc.desc  : Test HpaeInjectorRendererManager invalid state and func
 */
HWTEST_F(HpaeInjectorRendererManagerTest, HpaeInjectorRendererManagerTest, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto hpaeRendererManager = std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    auto sinkOutputNode = SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->SetMute(false), SUCCESS);
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH_882;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, false);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->OnNodeStatusUpdate(streamInfo.sessionId, OPERATION_STOPPED);
    EXPECT_NE(hpaeRendererManager->DestroyStream(streamInfo.sessionId) == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->DeactivateThread(), true);
    EXPECT_EQ(hpaeRendererManager->SetClientVolume(streamInfo.sessionId, 0.f), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->SetRate(streamInfo.sessionId, 0), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->SetAudioEffectMode(streamInfo.sessionId, 0), SUCCESS);
    int32_t mode = 0;
    EXPECT_EQ(hpaeRendererManager->GetAudioEffectMode(streamInfo.sessionId, mode), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->SetPrivacyType(streamInfo.sessionId, 0), SUCCESS);
    int32_t type = 0;
    EXPECT_EQ(hpaeRendererManager->GetPrivacyType(streamInfo.sessionId, type), SUCCESS);
    std::shared_ptr<ICapturerStreamCallback> callback = nullptr;
    EXPECT_EQ(hpaeRendererManager->RegisterReadCallback(streamInfo.sessionId, callback), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->GetWritableSize(streamInfo.sessionId), 0);
    EXPECT_EQ(hpaeRendererManager->UpdateSpatializationState(streamInfo.sessionId, false, false), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->UpdateMaxLength(streamInfo.sessionId, 0), SUCCESS);
    std::vector<SinkInput> vct = hpaeRendererManager->GetAllSinkInputsInfo();
    EXPECT_EQ(vct.size(), 0);
    EXPECT_EQ(hpaeRendererManager->RefreshProcessClusterByDevice(), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->DumpSinkInfo(), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->GetDeviceHDFDumpInfo() == "", true);
    EXPECT_EQ(hpaeRendererManager->SetLoudnessGain(streamInfo.sessionId, 0.f), SUCCESS);

    EXPECT_EQ(hpaeRendererManager->Init() == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    EXPECT_EQ(hpaeRendererManager->DeactivateThread(), true);
    hpaeRendererManager = nullptr;
}

/**
 * @tc.name  : Test HpaeInjectorRendererManagerReloadTest
 * @tc.type  : FUNC
 * @tc.number: HpaeInjectorRendererManagerReloadTest
 * @tc.desc  : Test HpaeInjectorRendererManager Reload
 */
HWTEST_F(HpaeInjectorRendererManagerTest, HpaeInjectorRendererManagerReloadTest, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto hpaeRendererManager = std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    EXPECT_NE(hpaeRendererManager->Init(), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
    EXPECT_NE(hpaeRendererManager->ReloadRenderManager(sinkInfo), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);

    auto sinkOutputNode = SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->ReloadRenderManager(sinkInfo), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_44100;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.frameLen = FRAME_LENGTH_882;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;

    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(hpaeRendererManager);

    streamInfo.sessionId += 1;
    EXPECT_EQ(hpaeRendererManager->CreateStream(streamInfo) == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    hpaeRendererManager->Start(streamInfo.sessionId);
    WaitForMsgProcessing(hpaeRendererManager);

    hpaeRendererManager->Stop(streamInfo.sessionId);
    WaitForMsgProcessing(hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->ReloadRenderManager(sinkInfo), SUCCESS);
}

/**
 * @tc.name  : Test HpaeInjectorAddNodeToSinkTest
 * @tc.type  : FUNC
 * @tc.number: HpaeInjectorAddNodeToSinkTest
 * @tc.desc  : Test HpaeInjectorRendererManager addNodeToSink
 */
HWTEST_F(HpaeInjectorRendererManagerTest, HpaeInjectorAddNodeToSinkTest, TestSize.Level1)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;
    nodeInfo.sessionId = TEST_STREAM_SESSION_ID;
    nodeInfo.historyFrameCount = 0;
    std::vector<std::shared_ptr<HpaeSinkInputNode>> vec;
    auto sinkInputNode1 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    vec.emplace_back(sinkInputNode1);
    nodeInfo.sessionId += 1;
    auto sinkInputNode2 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode2->SetState(HPAE_SESSION_RUNNING);
    vec.emplace_back(sinkInputNode2);
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto hpaeRendererManager = std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    auto sinkOutputNode = SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    EXPECT_EQ(hpaeRendererManager->AddAllNodesToSink(vec, false), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    nodeInfo.sessionId += 1;
    auto sinkInputNode3 = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    EXPECT_EQ(hpaeRendererManager->AddNodeToSink(sinkInputNode3), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->sinkInputNodeMap_.size(), 3); // 3 for size
    hpaeRendererManager->sinkOutputNode_ = nullptr;
    EXPECT_EQ(hpaeRendererManager->DeInit(true), SUCCESS);
}

/**
 * @tc.name  : Test HpaeInjectorRenderManagerSuspendTest
 * @tc.type  : FUNC
 * @tc.number: HpaeInjectorRenderManagerSuspendTest
 * @tc.desc  : Test HpaeInjectorRendererManager suspend
 */
HWTEST_F(HpaeInjectorRendererManagerTest, HpaeInjectorRenderManagerSuspendTest, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto hpaeRendererManager = std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    auto sinkOutputNode = SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    hpaeRendererManager->isSuspend_ = true;
    bool isSuspend = true;
    EXPECT_EQ(hpaeRendererManager->SuspendStreamManager(isSuspend), SUCCESS);

    hpaeRendererManager->isSuspend_ = true;
    EXPECT_EQ(hpaeRendererManager->SuspendStreamManager(isSuspend), SUCCESS);

    isSuspend = false;
    EXPECT_EQ(hpaeRendererManager->SuspendStreamManager(isSuspend), SUCCESS);

    isSuspend = true;
    hpaeRendererManager->sinkOutputNode_ = nullptr;
    EXPECT_EQ(hpaeRendererManager->SuspendStreamManager(isSuspend), SUCCESS);
}

/**
 * @tc.name  : Test HpaeInjectorRenderManagerDeinitWithMove
 * @tc.type  : FUNC
 * @tc.number: HpaeInjectorRenderManagerSuspendTest
 * @tc.desc  : Test HpaeInjectorRendererManager suspend
 */
HWTEST_F(HpaeInjectorRendererManagerTest, HpaeInjectorRenderManagerDeinitWithMoveTest, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto hpaeRendererManager = std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    auto sinkOutputNode = SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    EXPECT_EQ(hpaeRendererManager->DeInit(true), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->hpaeSignalProcessThread_ == nullptr, true);
}
}  // namespace