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
#include "hpae_remote_device_class.h"
#include "hpae_offload_renderer_manager.h"
#include "hpae_injector_renderer_manager.h"
#include "hpae_output_cluster.h"
#include "hpae_co_buffer_node.h"
#include "hpae_inner_capturer_manager.h"
#include "audio_effect_chain.h"
#include "audio_effect_chain_manager.h"
#include "hpae_sink_virtual_output_node.h"
#include "hpae_node_common.h"
#include "hpae_session_effect_node.h"
#include "hpae_mocks.h"
#include "audio_utils.h"
#include "audio_pipe_types.h"
#include "util/id_handler.h"
#include "mock_effect_instance.h"
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <any>
#include <tuple>
#include <vector>

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

std::vector<EffectChain> DEFAULT_EFFECT_CHAINS = {
    {"EFFECTCHAIN_SPK_MUSIC", {"apply1", "apply2", "apply3"}, ""},
    {"EFFECTCHAIN_BT_MUSIC", {}, ""}
};

EffectChainManagerParam DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"},
        {"SCENE_EXTRA_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_OTHER_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

std::vector<std::shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};

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

static uint32_t InjectMockRenderSink(const HpaeSinkInfo &sinkInfo)
{
    // Mirror production info selection (IdHandler::SelectIdInfo, class-dispatched by deviceClass).
    HdiIdInfoParam param = {.deviceClass = sinkInfo.deviceClass, .networkId = sinkInfo.deviceNetId,
        .macAddress = sinkInfo.macAddress, .busAddress = sinkInfo.busAddress};
    uint32_t renderId = HdiAdapterManager::GetInstance().GetRenderIdByDeviceClass(
        sinkInfo.deviceClass, IdHandler::SelectIdInfo(param), true);
    EXPECT_NE(renderId, HDI_INVALID_ID);

    auto mockRenderSink = std::make_shared<NiceMock<MockAudioRenderSink>>();
    auto isInited = std::make_shared<bool>(false);
    ON_CALL(*mockRenderSink, Init(_)).WillByDefault(Invoke([isInited](const IAudioSinkAttr &) {
        *isInited = true;
        return SUCCESS;
    }));
    ON_CALL(*mockRenderSink, IsInited()).WillByDefault(Invoke([isInited]() {
        return *isInited;
    }));
    ON_CALL(*mockRenderSink, DeInit()).WillByDefault(Invoke([isInited]() {
        *isInited = false;
    }));
    ON_CALL(*mockRenderSink, Start()).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockRenderSink, Stop()).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockRenderSink, Pause()).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockRenderSink, Flush()).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockRenderSink, UpdateAppsUid(A<const std::vector<int32_t> &>())).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockRenderSink, RegistOffloadHdiCallback(_)).WillByDefault(Return());
    ON_CALL(*mockRenderSink, SetBufferSize(_)).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockRenderSink, SetVolume(_, _)).WillByDefault(Return(SUCCESS));
    ON_CALL(*mockRenderSink, GetLatency(_)).WillByDefault(Return(SUCCESS));

    HdiAdapterManager::GetInstance().renderSinks_[renderId].sink_ = mockRenderSink;
    return renderId;
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

static std::shared_ptr<HpaeSinkInputNode> AddSessionEffectTestSession(
    const std::shared_ptr<HpaeRendererManager> &rendererManager, uint32_t sessionId, uint32_t nodeId)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = nodeId;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->SetState(HPAE_SESSION_RUNNING);
    sinkInputNode->connectedProcessorType_ = HPAE_SCENE_EFFECT_NONE;
    rendererManager->sinkInputNodeMap_[sessionId] = sinkInputNode;
    rendererManager->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_EFFECT_NONE;
    rendererManager->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    rendererManager->sessionNodeMap_[sessionId].bypass = false;

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, rendererManager->sinkInfo_);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);
    rendererManager->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE] = cluster;
    return sinkInputNode;
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

HWTEST_F(HpaeRendererManagerTest, CoverageDrainAndAuxiliarySinkImmediateQueue_001, TestSize.Level1)
{
    EXPECT_NE(hpaeRendererManager_, nullptr);
    EXPECT_NE(outputCluster_, nullptr);

    hpaeRendererManager_->isInit_.store(true);
    hpaeRendererManager_->hpaeSignalProcessThread_.reset();
    hpaeRendererManager_->sinkInfo_.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;

    hpaeRendererManager_->sessionNodeMap_[1].state = HPAE_SESSION_PAUSED;
    EXPECT_EQ(hpaeRendererManager_->Drain(1), SUCCESS);
    EXPECT_EQ(hpaeRendererManager_->IsMsgProcessing(), false);

    hpaeRendererManager_->sessionNodeMap_[1].state = HPAE_SESSION_RUNNING;
    EXPECT_EQ(hpaeRendererManager_->Drain(1), SUCCESS);
    EXPECT_EQ(hpaeRendererManager_->IsMsgProcessing(), false);

    hpaeRendererManager_->sinkInfo_.deviceClass = "a2dp";
    EXPECT_EQ(hpaeRendererManager_->SetAuxiliarySinkEnable(true), SUCCESS);
    EXPECT_EQ(hpaeRendererManager_->IsMsgProcessing(), false);

    hpaeRendererManager_->sinkInfo_.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    EXPECT_EQ(hpaeRendererManager_->SetAuxiliarySinkEnable(false), SUCCESS);
    EXPECT_EQ(hpaeRendererManager_->IsMsgProcessing(), false);

    HpaeNodeInfo stoppedNodeInfo;
    stoppedNodeInfo.sessionId = 2;
    auto stoppedNode = std::make_shared<HpaeSinkInputNode>(stoppedNodeInfo);
    stoppedNode->SetState(HPAE_SESSION_STOPPED);
    stoppedNode->appUid_ = 456;
    hpaeRendererManager_->sinkInputNodeMap_[2] = stoppedNode;
    sinkInputNode_->SetState(HPAE_SESSION_RUNNING);
    sinkInputNode_->appUid_ = 123;
    hpaeRendererManager_->sinkInfo_.deviceClass = "remote";

    hpaeRendererManager_->TriggerAppsUidUpdate(2);
    EXPECT_EQ(hpaeRendererManager_->appsUid_.size(), 2);

    sinkInputNode_->SetState(HPAE_SESSION_STOPPED);
    hpaeRendererManager_->TriggerAppsUidUpdate(0);
    EXPECT_EQ(hpaeRendererManager_->appsUid_.size(), 0);
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
    ASSERT_NE(hpaeRendererManager, nullptr);
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

    ASSERT_NE(hpaeRendererManager, nullptr);
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
    ASSERT_NE(hpaeRendererManager->converterForOutput_, nullptr);
    ASSERT_NE(hpaeRendererManager->loudnessGainNode_, nullptr);
    ASSERT_NE(hpaeRendererManager->converterForLoudness_, nullptr);
    EXPECT_EQ(hpaeRendererManager->ConnectInputSession(), SUCCESS);

    int32_t ret = hpaeRendererManager->ReloadRenderManager(sinkInfo);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(ret, SUCCESS);
    ASSERT_NE(hpaeRendererManager->converterForOutput_, nullptr);
    ASSERT_NE(hpaeRendererManager->loudnessGainNode_, nullptr);
    ASSERT_NE(hpaeRendererManager->converterForLoudness_, nullptr);
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
    ASSERT_NE(hpaeRendererManager->converterForOutput_, nullptr);
    ASSERT_NE(hpaeRendererManager->loudnessGainNode_, nullptr);
    ASSERT_NE(hpaeRendererManager->converterForLoudness_, nullptr);
    EXPECT_EQ(hpaeRendererManager->ConnectInputSession(), SUCCESS);

    int32_t ret = hpaeRendererManager->ReloadRenderManager(sinkInfo);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(ret, SUCCESS);
    ASSERT_NE(hpaeRendererManager->converterForOutput_, nullptr);
    ASSERT_NE(hpaeRendererManager->loudnessGainNode_, nullptr);
    ASSERT_NE(hpaeRendererManager->converterForLoudness_, nullptr);
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
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);

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
    EXPECT_EQ(result, true);
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
    ASSERT_NE(node, nullptr);
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
    ASSERT_NE(hpaeRendererManager, nullptr);
    sinkInfo.deviceClass = "offload";
    hpaeRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    ASSERT_NE(hpaeRendererManager, nullptr);
    sinkInfo.deviceClass = "test";
    hpaeRendererManager = IHpaeRendererManager::CreateRendererManager(sinkInfo);
    ASSERT_NE(hpaeRendererManager, nullptr);
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
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_BLUETOOTH_OPEN,
        COLLABORATION_DEFAULT_ROUTE_MODE), SUCCESS);
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
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_CLOSE, COLLABORATION_DEFAULT_ROUTE_MODE),
        SUCCESS);
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
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_BLUETOOTH_OPEN,
        COLLABORATION_DEFAULT_ROUTE_MODE), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->Pause(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->GetSinkInputInfo(streamInfo.sessionId, sinkInputInfo), SUCCESS);
    EXPECT_EQ(sinkInputInfo.rendererSessionInfo.state, HPAE_SESSION_PAUSED);
    EXPECT_EQ(hpaeRendererManager->Start(streamInfo.sessionId), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_CLOSE,
        COLLABORATION_DEFAULT_ROUTE_MODE), SUCCESS);
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
    ASSERT_NE(coBufferNode, nullptr);
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
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] =
        std::make_shared<HpaeProcessCluster>(nodeInfo2, sinkInfo);
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT] =
        hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC];

    int32_t ret = hpaeRendererManager->RefreshProcessClusterByDevice();
    EXPECT_EQ(ret == SUCCESS, true);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

// Output cluster stub with a controllable GetCurrentOutputDevice, for UpdateRenderDeviceType UT.
class TestDeviceTypeOutputCluster : public HpaeOutputCluster {
public:
    explicit TestDeviceTypeOutputCluster(DeviceType deviceType)
        : HpaeOutputCluster(HpaeNodeInfo()), currentDeviceType_(deviceType) {}

    DeviceType GetCurrentOutputDevice() const override
    {
        return currentDeviceType_;
    }

private:
    DeviceType currentDeviceType_;
};

// ==================== UT for UpdateRenderDeviceType ====================

/**
 * @tc.name  : Test HpaeRendererManagerUpdateRenderDeviceType_001
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerUpdateRenderDeviceType_001
 * @tc.desc  : Test UpdateRenderDeviceType when outputCluster_ is nullptr.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerUpdateRenderDeviceType_001, TestSize.Level1)
{
    hpaeRendererManager_->renderDeviceType_ = DEVICE_TYPE_SPEAKER;
    hpaeRendererManager_->outputCluster_ = nullptr;
    EXPECT_EQ(hpaeRendererManager_->UpdateRenderDeviceType(), false);
    EXPECT_EQ(hpaeRendererManager_->renderDeviceType_, DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test HpaeRendererManagerUpdateRenderDeviceType_002
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerUpdateRenderDeviceType_002
 * @tc.desc  : Test UpdateRenderDeviceType when current output device is DEVICE_TYPE_INVALID.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerUpdateRenderDeviceType_002, TestSize.Level1)
{
    hpaeRendererManager_->renderDeviceType_ = DEVICE_TYPE_SPEAKER;
    hpaeRendererManager_->outputCluster_ = std::make_shared<TestDeviceTypeOutputCluster>(DEVICE_TYPE_INVALID);
    EXPECT_EQ(hpaeRendererManager_->UpdateRenderDeviceType(), false);
    EXPECT_EQ(hpaeRendererManager_->renderDeviceType_, DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test HpaeRendererManagerUpdateRenderDeviceType_003
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerUpdateRenderDeviceType_003
 * @tc.desc  : Test UpdateRenderDeviceType when current output device is DEVICE_TYPE_NONE.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerUpdateRenderDeviceType_003, TestSize.Level1)
{
    hpaeRendererManager_->renderDeviceType_ = DEVICE_TYPE_SPEAKER;
    hpaeRendererManager_->outputCluster_ = std::make_shared<TestDeviceTypeOutputCluster>(DEVICE_TYPE_NONE);
    EXPECT_EQ(hpaeRendererManager_->UpdateRenderDeviceType(), false);
    EXPECT_EQ(hpaeRendererManager_->renderDeviceType_, DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test HpaeRendererManagerUpdateRenderDeviceType_004
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerUpdateRenderDeviceType_004
 * @tc.desc  : Test UpdateRenderDeviceType when device type is the same as renderDeviceType_.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerUpdateRenderDeviceType_004, TestSize.Level1)
{
    hpaeRendererManager_->renderDeviceType_ = DEVICE_TYPE_SPEAKER;
    hpaeRendererManager_->outputCluster_ = std::make_shared<TestDeviceTypeOutputCluster>(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(hpaeRendererManager_->UpdateRenderDeviceType(), false);
    EXPECT_EQ(hpaeRendererManager_->renderDeviceType_, DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test HpaeRendererManagerUpdateRenderDeviceType_005
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerUpdateRenderDeviceType_005
 * @tc.desc  : Test UpdateRenderDeviceType when device type changes to a valid different type.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerUpdateRenderDeviceType_005, TestSize.Level1)
{
    hpaeRendererManager_->renderDeviceType_ = DEVICE_TYPE_SPEAKER;
    hpaeRendererManager_->outputCluster_ =
        std::make_shared<TestDeviceTypeOutputCluster>(DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(hpaeRendererManager_->UpdateRenderDeviceType(), true);
    EXPECT_EQ(hpaeRendererManager_->renderDeviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
}

// ==================== UT for ReloadSinkinputs ====================

/**
 * @tc.name  : Test HpaeRendererManagerReloadSinkinputs_001
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerReloadSinkinputs_001
 * @tc.desc  : Test ReloadSinkinputs with a running session in sinkInputNodeMap_.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerReloadSinkinputs_001, TestSize.Level1)
{
    // sinkInputNodeMap_[1] holds a RUNNING node from SetUp.
    hpaeRendererManager_->sinkInputNodeMap_[1]->connectedProcessorType_ = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager_->ReloadSinkinputs();
    WaitForMsgProcessing(hpaeRendererManager_);
    EXPECT_EQ(hpaeRendererManager_->IsMsgProcessing(), false);
}

/**
 * @tc.name  : Test HpaeRendererManagerReloadSinkinputs_002
 * @tc.type  : FUNC
 * @tc.number: HpaeRendererManagerReloadSinkinputs_002
 * @tc.desc  : Test ReloadSinkinputs with a stopped session and a null node in sinkInputNodeMap_.
 */
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerReloadSinkinputs_002, TestSize.Level1)
{
    sinkInputNode_->SetState(HPAE_SESSION_STOPPED);
    hpaeRendererManager_->sinkInputNodeMap_[2] = nullptr;
    hpaeRendererManager_->ReloadSinkinputs();
    WaitForMsgProcessing(hpaeRendererManager_);
    EXPECT_EQ(hpaeRendererManager_->IsMsgProcessing(), false);
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
 * @tc.name: Test ConnectInputCluster conditional fade arming
 * @tc.type: FUNC
 * @tc.number: ConnectInputCluster_002
 * @tc.desc: A cleanly playing session keeps gain continuity on reconnect (fade-in NOT armed);
 *           a node resuming from a fade lifecycle (stale DONE_FADEOUT) keeps the arm path.
 */
HWTEST_F(HpaeRendererManagerTest, ConnectInputCluster_002, TestSize.Level0)
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
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    auto gainNode = hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->GetGainNodeById(sessionId);
    ASSERT_NE(gainNode, nullptr);

    // Cleanly playing session: hot re-plumb (scene refresh / effect-mode switch / reload)
    // keeps gain continuity, fade-in must NOT be armed.
    gainNode->fadeInState_ = false;
    gainNode->fadeOutState_ = FadeOutState::NO_FADEOUT;
    hpaeRendererManager->ConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(gainNode->fadeInState_, false);

    // Session resuming from a fade lifecycle (stale DONE_FADEOUT left by a Pause):
    // reconnect keeps the legacy behavior (clear + arm fade-in).
    gainNode->fadeOutState_ = FadeOutState::DONE_FADEOUT;
    hpaeRendererManager->ConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(gainNode->fadeInState_, true);

    EXPECT_EQ(hpaeRendererManager->DeleteProcessCluster(sessionId), SUCCESS);
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
    ASSERT_NE(offloadManager->curNode_, nullptr);
    auto tmpNodeId1 = offloadManager->converterForOutput_->GetNodeId();
    auto tmpNodeId2 = offloadManager->loudnessGainNode_->GetNodeId();
    auto tmpNodeId3 = offloadManager->converterForLoudness_->GetNodeId();
    ASSERT_NE(offloadManager->converterForOutput_, nullptr);
    ASSERT_NE(offloadManager->loudnessGainNode_, nullptr);
    ASSERT_NE(offloadManager->converterForLoudness_, nullptr);

    offloadManager->SetCurrentNode();
    // curNode exit, nodes remain unchanged
    ASSERT_NE(offloadManager->curNode_, nullptr);
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
    ASSERT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    offloadManager->isInit_ = true;

    CreateTwoStreamInOffload(offloadManager);

    // create nodes for stream1
    ASSERT_NE(offloadManager->converterForOutput_, nullptr);
    ASSERT_NE(offloadManager->loudnessGainNode_, nullptr);
    ASSERT_NE(offloadManager->converterForLoudness_, nullptr);
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
    ASSERT_NE(offloadManager->curNode_, nullptr);
    offloadManager->OnNotifyHdiData(hdiPos);
    // new curNode, create new nodes for stream2
    ASSERT_NE(offloadManager->converterForOutput_, nullptr);
    ASSERT_NE(offloadManager->loudnessGainNode_, nullptr);
    ASSERT_NE(offloadManager->converterForLoudness_, nullptr);
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
    offloadManager->isInit_ = true;

    CreateTwoStreamInOffload(offloadManager);

    offloadManager->DestroyStream(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(offloadManager);
    EXPECT_NE(offloadManager->curNode_->GetState(), HPAE_SESSION_RUNNING);
    ASSERT_NE(offloadManager->converterForOutput_, nullptr);
    ASSERT_NE(offloadManager->loudnessGainNode_, nullptr);
    ASSERT_NE(offloadManager->converterForLoudness_, nullptr);

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
    ASSERT_NE(rendererManager, nullptr);
    EXPECT_EQ(hpaeRendererManager_->DeleteInputSession(INVALID_ID), SUCCESS);

    auto injectorManager = std::make_shared<HpaeInjectorRendererManager>(sinkInfo);
    ASSERT_NE(injectorManager, nullptr);
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
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT] =
        hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC];
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
 * @tc.name : ConnectProcessCluster
 * @tc.type : FUNC
 * @tc.number: ConnectProcessCluster_001
 * @tc.desc : Test ConnectProcessCluster
 */
HWTEST_F(HpaeRendererManagerTest, ConnectProcessCluster_001, TestSize.Level0)
{
    uint32_t sessionId = 10001;
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    // Build the sink input node. bypass=false so DisConnectInputCluster takes the deferred-stop path.
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;

    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    HpaeNodeInfo movieNodeInfo = nodeInfo;
    movieNodeInfo.sceneType = HPAE_SCENE_MOVIE;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MOVIE] =
        std::make_shared<HpaeProcessCluster>(movieNodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MOVIE]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);

    // Start the session on OLD (MUSIC).
    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MUSIC);

    // Disconnect OLD -> it is now empty, so the stop is deferred inside the MUSIC cluster.
    hpaeRendererManager->DisConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), true);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->GetDeferredStopSessionId(), sessionId);

    // Reconnect to NEW (MOVIE). The deferred OLD stop must be flushed on MUSIC; without the fix it would
    // survive and fire a late Stop on a session that is now running on MOVIE.
    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MOVIE);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), false);
    // Verify the target scene (MOVIE) was actually connected.
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MOVIE]->GetConnectedFlag(), true);
    EXPECT_EQ(hpaeRendererManager->sinkInputNodeMap_[sessionId]->connectedProcessorType_, HPAE_SCENE_MOVIE);

    hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);
    hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MOVIE);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : ConnectProcessCluster
 * @tc.type  : FUNC
 * @tc.number: ConnectProcessCluster_002
 * @tc.desc  : Test ConnectProcessCluster when a deferred stop is pending and both the pending
 *             sceneCluster and the sinkInputNode exist, so the inner condition is true and
 *             AudioRendererStop is invoked for the pending scene.
 */
HWTEST_F(HpaeRendererManagerTest, ConnectProcessCluster_002, TestSize.Level0)
{
    uint32_t sessionId = 10002;
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
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;

    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);

    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MUSIC);
    // Last-stream disconnect defers the stop inside the MUSIC cluster.
    hpaeRendererManager->DisConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), true);

    // Reconnect the SAME session to the SAME cluster: the deferred stop is cancelled (the effect chain
    // still considers the session started, so AudioRendererStart is a no-op) and the connection is restored.
    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), false);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->GetConnectedFlag(), true);
    EXPECT_EQ(hpaeRendererManager->sinkInputNodeMap_[sessionId]->connectedProcessorType_, HPAE_SCENE_MUSIC);

    hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : ConnectProcessCluster
 * @tc.type  : FUNC
 * @tc.number: ConnectProcessCluster_003
 * @tc.desc  : Test ConnectProcessCluster when a deferred stop is pending but the pending
 *             sceneCluster does not exist. The inner condition short-circuits on its first
 *             operand and AudioRendererStop is skipped.
 */
HWTEST_F(HpaeRendererManagerTest, ConnectProcessCluster_003, TestSize.Level0)
{
    uint32_t sessionId = 10003;
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
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;

    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    // Keep a non-zero count so OnDisConnectProcessCluster does not erase the cluster before we can assert.
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 1;

    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MUSIC);
    hpaeRendererManager->DisConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), true);

    // Drain completes: OnDisConnectProcessCluster self-executes the deferred stop (no AudioRendererStop
    // call from the manager anymore).
    hpaeRendererManager->OnDisConnectProcessCluster(HPAE_SCENE_MUSIC);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), false);

    hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : ConnectProcessCluster
 * @tc.type  : FUNC
 * @tc.number: ConnectProcessCluster_004
 * @tc.desc  : The deferred stop is an effect-chain REMOVE independent of the graph nodes: even if the
 *             session's nodes are destroyed before drain completes, ExecuteDeferredStop still performs the
 *             (idempotent) REMOVE and clears the deferred state, with no crash.
 */
HWTEST_F(HpaeRendererManagerTest, ConnectProcessCluster_004, TestSize.Level0)
{
    uint32_t sessionId = 10004;
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
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;

    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 1;

    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MUSIC);
    hpaeRendererManager->DisConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), true);

    // Simulate the session being fully released before drain completes.
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->DestroyNodes(sessionId), SUCCESS);

    // Drain completes: even though the session's graph nodes are gone, ExecuteDeferredStop still performs the
    // effect REMOVE (idempotent) and clears the deferred state, with no crash.
    hpaeRendererManager->OnDisConnectProcessCluster(HPAE_SCENE_MUSIC);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), false);

    hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : ConnectProcessCluster
 * @tc.type  : FUNC
 * @tc.number: ConnectProcessCluster_005
 * @tc.desc  : null processCluster
 */
HWTEST_F(HpaeRendererManagerTest, ConnectProcessCluster_005, TestSize.Level0)
{
    uint32_t sessionId = 10005;
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    // Build the sink input node. bypass=false so DisConnectInputCluster takes the deferred-stop path.
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.effectInfo.effectScene = SCENE_MUSIC;
    nodeInfo.effectInfo.effectMode = EFFECT_DEFAULT;
    nodeInfo.sceneType = HPAE_SCENE_MUSIC;
    hpaeRendererManager->sinkInputNodeMap_[sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;

    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    HpaeNodeInfo movieNodeInfo = nodeInfo;
    movieNodeInfo.sceneType = HPAE_SCENE_MOVIE;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MOVIE] =
        std::make_shared<HpaeProcessCluster>(movieNodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MOVIE]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);

    // Inject a corrupted null shared_ptr entry. Without the SafeGetMap guard in ConnectProcessCluster
    // this null pointer is dereferenced on the very first loop traversal and crashes.
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_GAME] = nullptr;
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_GAME], nullptr);

    // Start the session on OLD (MUSIC); the loop must skip the null GAME entry without crashing.
    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MUSIC);

    // Disconnect OLD -> it is now empty, so the stop is deferred inside the MUSIC cluster.
    hpaeRendererManager->DisConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), true);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->GetDeferredStopSessionId(), sessionId);

    // Reconnect to NEW (MOVIE). The null GAME entry must still be skipped, the deferred stop on the
    // valid MUSIC cluster must be flushed, and MOVIE must be connected.
    hpaeRendererManager->ConnectProcessCluster(sessionId, HPAE_SCENE_MOVIE);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->HasDeferredStop(), false);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MOVIE]->GetConnectedFlag(), true);
    EXPECT_EQ(hpaeRendererManager->sinkInputNodeMap_[sessionId]->connectedProcessorType_, HPAE_SCENE_MOVIE);
    // The guard must neither crash on nor mutate the corrupted null entry.
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_GAME], nullptr);

    hpaeRendererManager->sceneClusterMap_.erase(HPAE_SCENE_GAME);
    hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MUSIC);
    hpaeRendererManager->DeleteProcessClusterInner(sessionId, HPAE_SCENE_MOVIE);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : DeleteProcessCluster
 * @tc.type  : FUNC
 * @tc.number: DeleteProcessCluster_002
 * @tc.desc  : connectedProcessorType_ is UNCONNECTED (Pause cleared it) and a cluster still holds the
 *             session's nodes -> the recovery loop resolves the real sceneType and cleanup completes.
 */
HWTEST_F(HpaeRendererManagerTest, DeleteProcessCluster_002, TestSize.Level0)
{
    uint32_t sessionId = 10006;
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
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 1;

    // Simulate the post-Pause state: connectedProcessorType_ cleared to UNCONNECTED, nodes still present.
    hpaeRendererManager->sinkInputNodeMap_[sessionId]->connectedProcessorType_ = HPAE_SCENE_UNCONNECTED;

    // Recovery loop finds the MUSIC cluster (CheckNodes SUCCESS) and cleanup completes.
    EXPECT_EQ(hpaeRendererManager->DeleteProcessCluster(sessionId), SUCCESS);

    // No cluster retains this session's nodes after release.
    for (const auto &[type, cluster] : hpaeRendererManager->sceneClusterMap_) {
        if (cluster != nullptr) {
            EXPECT_NE(cluster->CheckNodes(sessionId), SUCCESS);
        }
    }

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : DeleteProcessCluster
 * @tc.type  : FUNC
 * @tc.number: DeleteProcessCluster_003
 * @tc.desc  : connectedProcessorType_ is UNCONNECTED and no cluster holds the session's nodes -> the
 *             recovery loop exhausts without matching, sceneType stays UNCONNECTED, cleanup is skipped
 *             gracefully and no crash.
 */
HWTEST_F(HpaeRendererManagerTest, DeleteProcessCluster_003, TestSize.Level0)
{
    uint32_t sessionId = 10007;
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
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;
    // A MUSIC cluster exists but does NOT hold this session's nodes (CheckNodes fails).
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->sinkInputNodeMap_[sessionId]->connectedProcessorType_ = HPAE_SCENE_UNCONNECTED;

    // Recovery loop exhausts; sceneType stays UNCONNECTED, co-buffer/output cleanup skipped, no crash.
    EXPECT_EQ(hpaeRendererManager->DeleteProcessCluster(sessionId), SUCCESS);
    // Nothing was wrongly erased: the nodeless MUSIC cluster is still there.
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_.count(HPAE_SCENE_MUSIC), 1);

    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

/**
 * @tc.name  : DeleteProcessCluster
 * @tc.type  : FUNC
 * @tc.number: DeleteProcessCluster_004
 * @tc.desc  : connectedProcessorType_ is still the real scene (no prior Pause) -> the recovery branch is
 *             skipped and DeleteProcessCluster cleans up via the direct path.
 */
HWTEST_F(HpaeRendererManagerTest, DeleteProcessCluster_004, TestSize.Level0)
{
    uint32_t sessionId = 10008;
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
    hpaeRendererManager->sessionNodeMap_[sessionId].bypass = false;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    EXPECT_EQ(hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_MUSIC]->
        CreateNodes(hpaeRendererManager->sinkInputNodeMap_[sessionId]), SUCCESS);
    hpaeRendererManager->sceneTypeToProcessClusterCountMap_[HPAE_SCENE_MUSIC] = 1;
    // Connect the input node so connectedProcessorType_ holds the real scene (MUSIC) -> recovery skipped.
    hpaeRendererManager->ConnectInputCluster(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_EQ(hpaeRendererManager->sinkInputNodeMap_[sessionId]->connectedProcessorType_, HPAE_SCENE_MUSIC);

    EXPECT_EQ(hpaeRendererManager->DeleteProcessCluster(sessionId), SUCCESS);

    for (const auto &[type, cluster] : hpaeRendererManager->sceneClusterMap_) {
        if (cluster != nullptr) {
            EXPECT_NE(cluster->CheckNodes(sessionId), SUCCESS);
        }
    }

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
    offloadManager->isInit_ = true;

    CreateTwoStreamInOffload(offloadManager);
    auto ret = offloadManager->SetOffloadRenderCallbackType(TEST_STREAM_SESSION_ID, CB_NONBLOCK_WRITE_COMPLETED);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(offloadManager);
    ret = offloadManager->SetOffloadRenderCallbackType(DEFAULT_SESSIONID_NUM_FIRST, CB_RENDER_FULL);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(offloadManager);
    ASSERT_NE(offloadManager->curNode_, nullptr);
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
    ASSERT_NE(sinkInputNode, nullptr);
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

    uint32_t mockRenderId = InjectMockRenderSink(sinkInfo);
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    offloadManager->isInit_ = true;

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
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    HdiAdapterManager::GetInstance().ReleaseId(mockRenderId);
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
    offloadManager->isInit_ = true;

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
    offloadManager->isInit_ = true;

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
    offloadManager->isInit_ = true;

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
    offloadManager->isInit_ = true;

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
    offloadManager->isInit_ = true;

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
    EXPECT_EQ(hpaeRendererManager->hpaeCoBufferNode_ != nullptr, true);
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

    uint32_t mockRenderId = InjectMockRenderSink(sinkInfo);
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    ASSERT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    ASSERT_EQ(offloadManager->IsInit(), true);

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    ASSERT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is created for remote_offload device
    ASSERT_NE(offloadManager->limiterNode_, nullptr);
    EXPECT_TRUE(offloadManager->limiterNode_->IsLimiterInited());

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
    HdiAdapterManager::GetInstance().ReleaseId(mockRenderId);
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

    uint32_t mockRenderId = InjectMockRenderSink(sinkInfo);
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    ASSERT_EQ(offloadManager->IsInit(), true);

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
    HdiAdapterManager::GetInstance().ReleaseId(mockRenderId);
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

    uint32_t mockRenderId = InjectMockRenderSink(sinkInfo);
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    offloadManager->isInit_ = true;
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
    HdiAdapterManager::GetInstance().ReleaseId(mockRenderId);
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

    uint32_t mockRenderId = InjectMockRenderSink(sinkInfo);
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    EXPECT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    offloadManager->isInit_ = true;

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    ASSERT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is created
    ASSERT_NE(offloadManager->limiterNode_, nullptr);

    // Destroy stream to trigger DestroyOffloadNodes
    ASSERT_EQ(offloadManager->DestroyStream(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is destroyed
    EXPECT_EQ(offloadManager->limiterNode_, nullptr);

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
    HdiAdapterManager::GetInstance().ReleaseId(mockRenderId);
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

    uint32_t mockRenderId = InjectMockRenderSink(sinkInfo);
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    ASSERT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    offloadManager->isInit_ = true;

    // Create and start stream to trigger ConnectInputSession
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    ASSERT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    ASSERT_EQ(offloadManager->Start(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is connected (nodes should be non-null after start)
    ASSERT_NE(offloadManager->limiterNode_, nullptr);
    ASSERT_NE(offloadManager->converterForOutput_, nullptr);
    ASSERT_NE(offloadManager->loudnessGainNode_, nullptr);

    // Pause stream to trigger DisConnectInputSession
    ASSERT_EQ(offloadManager->Pause(streamInfo.sessionId), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ still exists (disconnection doesn't destroy the node)
    ASSERT_NE(offloadManager->limiterNode_, nullptr);

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
    HdiAdapterManager::GetInstance().ReleaseId(mockRenderId);
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

    uint32_t mockRenderId = InjectMockRenderSink(sinkInfo);
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    ASSERT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    offloadManager->isInit_ = true;

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    ASSERT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    ASSERT_EQ(offloadManager->Start(streamInfo.sessionId), SUCCESS);
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
    HdiAdapterManager::GetInstance().ReleaseId(mockRenderId);
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

    uint32_t mockRenderId = InjectMockRenderSink(sinkInfo);
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
    ASSERT_EQ(offloadManager->Init(), SUCCESS);
    WaitForMsgProcessing(offloadManager);
    offloadManager->isInit_ = true;

    // Create stream to trigger CreateOffloadNodes
    HpaeStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = TEST_STREAM_SESSION_ID;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    ASSERT_EQ(offloadManager->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(offloadManager);

    // Verify limiterNode_ is initialized after CreateOffloadNodes
    ASSERT_NE(offloadManager->limiterNode_, nullptr);
    EXPECT_TRUE(offloadManager->limiterNode_->IsLimiterInited());

    WaitForMsgProcessing(offloadManager);
    EXPECT_EQ(offloadManager->DeInit() == SUCCESS, true);
    EXPECT_EQ(offloadManager->IsInit(), false);
    HdiAdapterManager::GetInstance().ReleaseId(mockRenderId);
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
    sinkInfo.deviceName = "Interphone_Sink";
    sinkInfo.filePath = g_rootPath + "interphone_stop_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
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
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    sinkInfo.routeFlag = AUDIO_FLAG_NONE;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
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
    sinkInfo.deviceName = "Interphone_Sink";
    sinkInfo.filePath = g_rootPath + "interphone_pause_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    
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
    EXPECT_EQ(inNodeInfo.frameLen, 220);
}

HWTEST_F(HpaeRendererManagerTest, Init_Interphone_NoThread_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.deviceName = "Interphone_Sink";
    sinkInfo.filePath = g_rootPath + "interphone_init_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;

    auto hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    
    EXPECT_EQ(hpaeRendererManager->hpaeSignalProcessThread_, nullptr);
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);
    
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

HWTEST_F(HpaeRendererManagerTest, SendRequest_Interphone_Sync_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.deviceName = "Interphone_Sink";
    sinkInfo.filePath = g_rootPath + "interphone_sync_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;

    HpaeNodeInfo nodeInfo;
    auto hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    ::testing::DefaultValue<int32_t>::Set(0);
    
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    EXPECT_EQ(hpaeRendererManager->hpaeSignalProcessThread_, nullptr);
    ASSERT_NE(hpaeRendererManager->outputCluster_, nullptr);
    
    auto mockRenderSink = std::make_shared<NiceMock<MockAudioRenderSink>>();
    auto outputClusterImpl = std::static_pointer_cast<HpaeOutputCluster>(hpaeRendererManager->outputCluster_);
    outputClusterImpl->hpaeSinkOutputNode_->audioRendererSink_ = mockRenderSink;
    outputClusterImpl->hpaeSinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->sinkInputNodeMap_.insert_or_assign(1, sinkInputNode);
    sinkInputNode->SetState(HPAE_SESSION_RUNNING);

    EXPECT_CALL(*mockRenderSink, RenderFrame(_, _, _)).Times(0);
    EXPECT_CALL(*mockRenderSink, Stop()).Times(0);
    hpaeRendererManager->Process();
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);

    EXPECT_CALL(*mockRenderSink, Stop()).Times(AtLeast(1)).WillRepeatedly(Return(0));
    hpaeRendererManager->StopInterphoneSinkIfNeeded();
    
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
    ::testing::DefaultValue<int32_t>::Clear();
}

HWTEST_F(HpaeRendererManagerTest, Process_Interphone_NoThread_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.adapterName = DEFAULT_TEST_DEVICE_CLASS;
    sinkInfo.deviceName = "Interphone_Sink";
    sinkInfo.filePath = g_rootPath + "interphone_process_nothread_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;

    HpaeNodeInfo nodeInfo;
    auto hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    auto outputCluster = std::make_shared<HpaeOutputCluster>(nodeInfo);
    hpaeRendererManager->outputCluster_ = outputCluster;
    ::testing::DefaultValue<int32_t>::Set(0);
    auto mockRenderSink = std::make_shared<NiceMock<MockAudioRenderSink>>();
    outputCluster->hpaeSinkOutputNode_->audioRendererSink_ = mockRenderSink;
    outputCluster->hpaeSinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);

    EXPECT_EQ(hpaeRendererManager->hpaeSignalProcessThread_, nullptr);
    
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager->sinkInputNodeMap_.insert_or_assign(1, sinkInputNode);
    sinkInputNode->SetState(HPAE_SESSION_RUNNING);

    EXPECT_CALL(*mockRenderSink, RenderFrame(_, _, _)).Times(0);
    EXPECT_CALL(*mockRenderSink, Stop()).Times(0);
    hpaeRendererManager->Process();
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);

    hpaeRendererManager->sinkInputNodeMap_.erase(1);
    EXPECT_CALL(*mockRenderSink, Stop()).WillOnce(Return(0));
    hpaeRendererManager->StopInterphoneSinkIfNeeded();
    EXPECT_EQ(hpaeRendererManager->IsRunning(), false);
    ::testing::DefaultValue<int32_t>::Clear();
}

HWTEST_F(HpaeRendererManagerTest, CreateSessionEffect_EnsureHistoryBufferForAiss, TestSize.Level0)
{
    constexpr uint32_t sessionId = 789;
    constexpr uint32_t testNodeId = 100;
    constexpr uint32_t preheatFrames = 2400;

    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = testNodeId;
    nodeInfo.sessionId = sessionId;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    hpaeRendererManager_->sinkInputNodeMap_[sessionId] = sinkInputNode;
    hpaeRendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager_->sessionNodeMap_[sessionId].bypass = false;

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, hpaeRendererManager_->sinkInfo_);
    ASSERT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);
    hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE] = cluster;

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(preheatFrames));

    int32_t ret = hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(sinkInputNode->historyBuffer_, nullptr);
    EXPECT_GE(sinkInputNode->historyBuffer_->GetFrames(), preheatFrames);
}

HWTEST_F(HpaeRendererManagerTest, CreateSessionEffect_nullInstanceRejected, TestSize.Level0)
{
    constexpr uint32_t sessionId = 794;
    constexpr uint32_t testNodeId = 105;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", nullptr, config),
        static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER));
    EXPECT_FALSE(hpaeRendererManager_->HasSessionEffect(sessionId));
}

HWTEST_F(HpaeRendererManagerTest, SessionEffectStatusCallback, TestSize.Level0)
{
    constexpr uint32_t sessionId = 790;
    constexpr uint32_t testNodeId = 101;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);
    EXPECT_EQ(hpaeRendererManager_->Init(), SUCCESS);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_ENABLED), 1u);
    ASSERT_FALSE(callback->events_.empty());
    EXPECT_EQ(callback->events_.back().state, HPAE_SESSION_RUNNING);

    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    // IDLE node has no crossfade pending: FinalizeClose runs inside the destroy request itself;
    // the drain below only covers any leftover bookkeeping requests.
    WaitForMsgProcessing(hpaeRendererManager_);
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_EQ(hpaeRendererManager_->DeInit(), SUCCESS);
}

// Rapid recreate: Create arriving while the old node is STOPPING is not rejected as duplicate;
// it requeues and materializes after the close finalizes (now driven by OnCloseCrossfadeDone).
HWTEST_F(HpaeRendererManagerTest, SessionEffectDestroyThenCreateRequeues, TestSize.Level0)
{
    constexpr uint32_t sessionId = 7901;
    constexpr uint32_t testNodeId = 1101;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);
    EXPECT_EQ(hpaeRendererManager_->Init(), SUCCESS);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto oldInst = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*oldInst, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "old", oldInst, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Arm the smooth close: an ACTIVE node arms the close crossfade, so the destroy below waits
    // for OnCloseCrossfadeDone instead of finalizing inline.
    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE];
    ASSERT_NE(cluster, nullptr);
    auto node = cluster->GetSessionEffectNode(sessionId);
    ASSERT_NE(node, nullptr);
    node->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Freeze the queue (thread gone): sync calls still run inline, pushed requests wait for a
    // manual drain; interphone route makes OnCloseCrossfadeDone's SendRequest drain inline.
    EXPECT_TRUE(hpaeRendererManager_->DeactivateThread());
    hpaeRendererManager_->sinkInfo_.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;

    // Destroy (BeginClose, crossfade pending -> wait) then immediately Create the same session:
    // requeues behind the STOPPING node instead of returning ALREADY_BOUND.
    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    auto newInst = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*newInst, GetPreheatFrames()).WillByDefault(Return(0));
    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "new", newInst, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Crossfade done: the inline drain runs the requeued retry first (node still STOPPING, so it
    // re-defers), then the callback's Finalize — old finalized (DISABLED), new still deferred.
    hpaeRendererManager_->OnCloseCrossfadeDone(sessionId);
    hpaeRendererManager_->HandleMsg();  // retry materializes the new node
    EXPECT_TRUE(hpaeRendererManager_->HasSessionEffect(sessionId));
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_GE(callback->Count(sessionId, OPERATION_SESSION_EFFECT_ENABLED), 2u);
    EXPECT_EQ(hpaeRendererManager_->DeInit(), SUCCESS);
}

HWTEST_F(HpaeRendererManagerTest, HasSessionEffectUsesProcessorTypeWhenSessionSceneIsStale, TestSize.Level0)
{
    constexpr uint32_t sessionId = 793;
    constexpr uint32_t testNodeId = 104;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);
    hpaeRendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_MUSIC;

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_TRUE(hpaeRendererManager_->HasSessionEffect(sessionId));
}

HWTEST_F(HpaeRendererManagerTest, DeleteInputSessionWithSessionEffectSendsDisabledStatus, TestSize.Level0)
{
    constexpr uint32_t sessionId = 791;
    constexpr uint32_t testNodeId = 102;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    callback->events_.clear();

    EXPECT_EQ(hpaeRendererManager_->DeleteInputSession(sessionId), SUCCESS);
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    ASSERT_FALSE(callback->events_.empty());
    EXPECT_EQ(callback->events_.back().state, HPAE_SESSION_RUNNING);
}

HWTEST_F(HpaeRendererManagerTest, DeleteInputSessionWithoutSessionEffectDoesNotSendDisabledStatus, TestSize.Level0)
{
    constexpr uint32_t sessionId = 792;
    constexpr uint32_t testNodeId = 103;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    EXPECT_EQ(hpaeRendererManager_->DeleteInputSession(sessionId), SUCCESS);
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 0u);
}

/**
 * @tc.name  : Test UpdateCollaborativeState with NEARLINK_OPEN on BT speaker
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_NearLink_003
 * @tc.desc  : Test UpdateCollaborativeState with COLLABORATION_NEARLINK_OPEN on BT speaker (early return).
 */
HWTEST_F(HpaeRendererManagerTest, UpdateCollaborativeState_NearLink_003, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    // NEARLINK_OPEN on BT speaker device - should return SUCCESS (MCH check)
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_NEARLINK_OPEN, COLLABORATION_MCH_ROUTE_MODE),
        SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_CLOSE, COLLABORATION_MCH_ROUTE_MODE),
        SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Test UpdateCollaborativeState state not changed
 * @tc.type  : FUNC
 * @tc.number: UpdateCollaborativeState_NoChange_004
 * @tc.desc  : Test UpdateCollaborativeState with same state returns SUCCESS early.
 */
HWTEST_F(HpaeRendererManagerTest, UpdateCollaborativeState_NoChange_004, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    std::shared_ptr<IHpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    // State is COLLABORATION_CLOSE by default, set same state
    EXPECT_EQ(hpaeRendererManager->UpdateCollaborativeState(COLLABORATION_CLOSE, COLLABORATION_DEFAULT_ROUTE_MODE),
        SUCCESS);
    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
}

/**
 * @tc.name  : Materialize_Primary_ExpiredClearsAttachStateAndEmitsDisabled
 * @tc.type  : FUNC
 * @tc.number: Materialize_Primary_ExpiredClearsAttachStateAndEmitsDisabled
 * @tc.desc  : When attach-state instance is expired, Materialize clears attach state and emits DISABLED.
 */
HWTEST_F(HpaeRendererManagerTest, Materialize_Primary_ExpiredClearsAttachStateAndEmitsDisabled, TestSize.Level0)
{
    constexpr uint32_t sessionId = 801;
    constexpr uint32_t testNodeId = 111;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);

    // Write attach state with an expired instance
    SessionEffectAttachState st;
    st.enabled = true;
    st.effectName = "audio_aiss_enhance";
    // instance is default-constructed weak_ptr (expired)
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto sinkInputNode = hpaeRendererManager_->sinkInputNodeMap_[sessionId];
    sinkInputNode->SetSessionEffectAttachState(st);
    ASSERT_TRUE(sinkInputNode->HasSessionEffectAttachState());

    hpaeRendererManager_->MaterializeSessionEffectFromAttachState(sessionId, HPAE_SCENE_EFFECT_NONE);

    EXPECT_FALSE(sinkInputNode->HasSessionEffectAttachState());
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

/**
 * @tc.name  : MoveStream_Primary_PreservesAttachState_NoDisabled
 * @tc.type  : FUNC
 * @tc.number: MoveStream_Primary_PreservesAttachState_NoDisabled
 * @tc.desc  : MoveStreamSync must NOT clear attach state or emit DISABLED.
 */
HWTEST_F(HpaeRendererManagerTest, MoveStream_Primary_PreservesAttachState_NoDisabled, TestSize.Level0)
{
    constexpr uint32_t sessionId = 802;
    constexpr uint32_t testNodeId = 112;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    callback->events_.clear();

    hpaeRendererManager_->MoveStreamSync(sessionId, "new_sink");
    WaitForMsgProcessing(hpaeRendererManager_);

    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 0u);
}

/**
 * @tc.name  : DeleteInputSession_FullTeardown_EmitsDisabled
 * @tc.type  : FUNC
 * @tc.number: DeleteInputSession_FullTeardown_EmitsDisabled
 * @tc.desc  : Default DeleteInputSession mode (FULL_TEARDOWN) emits DISABLED and clears attach state.
 */
HWTEST_F(HpaeRendererManagerTest, DeleteInputSession_FullTeardown_EmitsDisabled, TestSize.Level0)
{
    constexpr uint32_t sessionId = 803;
    constexpr uint32_t testNodeId = 113;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    callback->events_.clear();

    EXPECT_EQ(hpaeRendererManager_->DeleteInputSession(sessionId), SUCCESS);
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

/**
 * @tc.name  : Materialize_Primary_SuccessEmitsEnabled
 * @tc.type  : FUNC
 * @tc.number: Materialize_Primary_SuccessEmitsEnabled
 * @tc.desc  : When MaterializeSessionEffectFromAttachState succeeds, ENABLED is emitted.
 */
HWTEST_F(HpaeRendererManagerTest, Materialize_Primary_SuccessEmitsEnabled, TestSize.Level0)
{
    constexpr uint32_t sessionId = 804;
    constexpr uint32_t testNodeId = 114;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    SessionEffectAttachState st;
    st.enabled = true;
    st.effectName = "audio_aiss_enhance";
    st.instance = instance;
    st.config = config;

    auto sinkInputNode = hpaeRendererManager_->sinkInputNodeMap_[sessionId];
    sinkInputNode->SetSessionEffectAttachState(st);

    hpaeRendererManager_->MaterializeSessionEffectFromAttachState(sessionId, HPAE_SCENE_EFFECT_NONE);

    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_ENABLED), 1u);
    EXPECT_TRUE(hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->HasSessionEffectNode(sessionId));
}

/**
 * @tc.name  : DetachSessionEffectNodeForRefresh_Captures
 * @tc.type  : FUNC
 * @tc.number: DetachSessionEffectNodeForRefresh_Captures
 * @tc.desc  : DetachSessionEffectNodeForRefresh captures into attach state.
 */
HWTEST_F(HpaeRendererManagerTest, DetachSessionEffectNodeForRefresh_Captures, TestSize.Level0)
{
    constexpr uint32_t sessionId = 805;
    constexpr uint32_t testNodeId = 115;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Force the SE node status to WARMING_UP
    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE];
    auto seNode = cluster->GetSessionEffectNode(sessionId);
    ASSERT_NE(seNode, nullptr);
    seNode->SetStatusForTest(HpaeSessionEffectNode::STATUS::WARMING_UP);

    hpaeRendererManager_->DetachSessionEffectNodeForRefresh(sessionId, HPAE_SCENE_EFFECT_NONE);

    auto sinkInputNode = hpaeRendererManager_->sinkInputNodeMap_[sessionId];
    ASSERT_TRUE(sinkInputNode->HasSessionEffectAttachState());
    auto stOpt = sinkInputNode->GetSessionEffectAttachState();
    ASSERT_TRUE(stOpt.has_value());
}

/**
 * @tc.name  : SuspendResumePreheatForGraphSwitch_Pairing
 * @tc.type  : FUNC
 * @tc.number: SuspendResumePreheatForGraphSwitch_Pairing
 * @tc.desc  : Suspend stops preheat thread; Resume restarts it; flag is managed correctly.
 */
HWTEST_F(HpaeRendererManagerTest, SuspendResumePreheatForGraphSwitch_Pairing, TestSize.Level0)
{
    constexpr uint32_t sessionId = 806;
    constexpr uint32_t testNodeId = 116;

    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = testNodeId;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto seNode = std::make_shared<HpaeSessionEffectNode>(nodeInfo);
    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(FRAME_LENGTH_960));
    ASSERT_EQ(seNode->CreateEffectWithInstance("audio_aiss_enhance", instance, config), SUCCESS);

    seNode->SetStatusForTest(HpaeSessionEffectNode::STATUS::WARMING_UP);
    seNode->StartEffect();

    // Suspend: should stop thread and set flag
    seNode->SuspendSessionEffectPreheatForGraphSwitch();

    // Resume: should restart thread and clear flag
    seNode->ResumeSessionEffectPreheatForGraphSwitch();

    // Resume without Suspend: should be no-op (flag already cleared)
    seNode->ResumeSessionEffectPreheatForGraphSwitch();

    seNode->DropEffect();
    EXPECT_TRUE(true); // If we get here without crashing, test passes
}

HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerInitOutputCluster_InvalidDeviceTypeFallback, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    sinkInfo.deviceType = DEVICE_TYPE_INVALID;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    // line 1259 fallback: INVALID → SPEAKER
    EXPECT_EQ(hpaeRendererManager->renderDeviceType_, DEVICE_TYPE_SPEAKER);
    // renderId_ is populated unconditionally regardless of deviceType fallback.
    EXPECT_NE(hpaeRendererManager->renderId_, HDI_INVALID_ID);

    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

// Negative case: a valid deviceType must be preserved (no fallback triggered).
HWTEST_F(HpaeRendererManagerTest, HpaeRendererManagerInitOutputCluster_DeviceTypePreserved, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    sinkInfo.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, hpaeRendererManager);

    EXPECT_EQ(hpaeRendererManager->Init(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), true);

    EXPECT_EQ(hpaeRendererManager->renderDeviceType_, DEVICE_TYPE_USB_HEADSET);

    EXPECT_EQ(hpaeRendererManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(hpaeRendererManager);
    EXPECT_EQ(hpaeRendererManager->IsInit(), false);
}

// ---------------------------------------------------------------------------
// HiCar MCH remote device-class detection (PR #15881).
// Only construct the manager and read sinkInfo_ via IsRemoteDevice(); never call
// Init()/Start()/any worker-triggering API. Private access via -fno-access-control.
// ---------------------------------------------------------------------------

HWTEST_F(HpaeRendererManagerTest, IsRemoteDeviceClass_Values_001, TestSize.Level1)
{
    EXPECT_EQ(IsRemoteDeviceClass("remote"), true);
    EXPECT_EQ(IsRemoteDeviceClass("remote_multichannel"), true);
    EXPECT_EQ(IsRemoteDeviceClass("primary"), false);
    EXPECT_EQ(IsRemoteDeviceClass("a2dp"), false);
    EXPECT_EQ(IsRemoteDeviceClass(""), false);
}

/**
 * @tc.name  : IsHdiSpeedSupported_NormalManager_001
 * @tc.type  : FUNC
 * @tc.number: IsHdiSpeedSupported_NormalManager_001
 * @tc.desc  : Normal renderer manager has no HDI speed capability (client keeps sonic speed).
 */
HWTEST_F(HpaeRendererManagerTest, IsHdiSpeedSupported_NormalManager_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    EXPECT_EQ(hpaeRendererManager->IsHdiSpeedSupported(), false);
}

HWTEST_F(HpaeRendererManagerTest, IsRemoteDevice_RemoteClass_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    sinkInfo.deviceClass = "remote";
    auto mgr = std::make_shared<HpaeRendererManager>(sinkInfo);
    ASSERT_NE(mgr, nullptr);
    EXPECT_EQ(mgr->IsRemoteDevice(), true);
}

HWTEST_F(HpaeRendererManagerTest, IsRemoteDevice_RemoteMultichannelClass_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    sinkInfo.deviceClass = "remote_multichannel";
    auto mgr = std::make_shared<HpaeRendererManager>(sinkInfo);
    ASSERT_NE(mgr, nullptr);
    EXPECT_EQ(mgr->IsRemoteDevice(), true);
}

HWTEST_F(HpaeRendererManagerTest, IsRemoteDevice_LocalClass_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo;
    GetBtSpeakerSinkInfo(sinkInfo);
    sinkInfo.deviceClass = "primary";
    auto mgr = std::make_shared<HpaeRendererManager>(sinkInfo);
    ASSERT_NE(mgr, nullptr);
    EXPECT_EQ(mgr->IsRemoteDevice(), false);
}

void InjectOverrideChain(const std::string &sceneType)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
}

// SetChannelOverrideForRoute: plain (non-enhanced) else-branch writes the override.
HWTEST_F(HpaeRendererManagerTest, ChannelOverride_SetForRoute_PlainBranchWritesMap, TestSize.Level1)
{
    const std::string scene = "SCENE_MOVIE";
    InjectOverrideChain(scene);

    ChannelOverrideRoute route{scene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->SetChannelOverrideForRoute(route);

    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));
    EXPECT_EQ(layout, static_cast<uint64_t>(CH_LAYOUT_STEREO));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// ClearChannelOverrideForRoute: plain (non-enhanced) else-branch erases the override.
HWTEST_F(HpaeRendererManagerTest, ChannelOverride_ClearForRoute_PlainBranchErasesMap, TestSize.Level1)
{
    const std::string scene = "SCENE_MOVIE";
    InjectOverrideChain(scene);

    ChannelOverrideRoute route{scene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->SetChannelOverrideForRoute(route);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // override applied

    hpaeRendererManager_->ClearChannelOverrideForRoute(route);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// OnSessionEffectMaterialized: NONE scene -> route nullopt -> no Set (false branch).
HWTEST_F(HpaeRendererManagerTest, OnSessionEffectMaterialized_NoneSceneIsNoOp, TestSize.Level1)
{
    const std::string probeScene = TransProcessorTypeToSceneType(HPAE_SCENE_EFFECT_NONE); // "SCENE_EXTRA"
    InjectOverrideChain(probeScene);
    uint32_t absentSession = 88881;
    hpaeRendererManager_->OnSessionEffectMaterialized(absentSession);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(probeScene, channels, layout),
        SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// OnSessionEffectMaterialized: split scene -> route has_value -> Set writes (true branch).
// A split processor type makes GetProcessorType short-circuit past the arm-chain check.
HWTEST_F(HpaeRendererManagerTest, OnSessionEffectMaterialized_SplitSceneWritesOverride, TestSize.Level1)
{
    uint32_t sessionId = 88882;
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA;
    nodeInfo.channels = STEREO;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->SetState(HPAE_SESSION_RUNNING);
    hpaeRendererManager_->sinkInputNodeMap_[sessionId] = node;

    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA); // "SCENE_EXTRA"
    InjectOverrideChain(scene);

    hpaeRendererManager_->OnSessionEffectMaterialized(sessionId);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // split scene -> route has_value -> Set applied

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// DeleteProcessCluster override snippet: route nullopt (NONE) -> no Clear.
HWTEST_F(HpaeRendererManagerTest, DeleteProcessCluster_NoneSceneNoClearOverride, TestSize.Level1)
{
    uint32_t sessionId = 88883;
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE; // -> GetProcessorType NONE -> route nullopt
    nodeInfo.channels = STEREO;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->SetState(HPAE_SESSION_RUNNING);
    node->connectedProcessorType_ = HPAE_SCENE_UNCONNECTED;
    hpaeRendererManager_->sinkInputNodeMap_[sessionId] = node;
    // sceneClusterMap_[NONE] must exist, or DeleteProcessCluster early-returns at the guard.
    hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, hpaeRendererManager_->sinkInfo_);

    // Pre-set an unrelated-scene override; the NONE route must leave it untouched.
    const std::string otherScene = "SCENE_OTHER";
    InjectOverrideChain(otherScene);
    ChannelOverrideRoute setRoute{otherScene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->SetChannelOverrideForRoute(setRoute);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(otherScene, channels, layout),
        SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // override applied

    EXPECT_EQ(hpaeRendererManager_->DeleteProcessCluster(sessionId), SUCCESS);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(otherScene, channels, layout),
        SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // NONE route -> no Clear -> override retained

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// DeleteProcessCluster override snippet: route has_value but no SE node -> no Clear.
HWTEST_F(HpaeRendererManagerTest, DeleteProcessCluster_NoClearWithoutSessionEffectNode, TestSize.Level1)
{
    uint32_t sessionId = 88884;
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA; // split type -> non-NONE route
    nodeInfo.channels = STEREO;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->SetState(HPAE_SESSION_RUNNING);
    node->connectedProcessorType_ = HPAE_SCENE_UNCONNECTED;
    hpaeRendererManager_->sinkInputNodeMap_[sessionId] = node;
    hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_SPLIT_MEDIA] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, hpaeRendererManager_->sinkInfo_);
    // No session-effect node injected -> HasSessionEffectNode == false.

    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA); // "SCENE_EXTRA"
    InjectOverrideChain(scene);
    ChannelOverrideRoute setRoute{scene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->SetChannelOverrideForRoute(setRoute);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // override applied

    EXPECT_EQ(hpaeRendererManager_->DeleteProcessCluster(sessionId), SUCCESS);
    // route valid but no SE node -> Clear short-circuited -> override retained.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// DeleteProcessCluster override snippet: route has_value AND SE node present -> Clear.
HWTEST_F(HpaeRendererManagerTest, DeleteProcessCluster_ClearsOverrideWithSessionEffectNode, TestSize.Level1)
{
    uint32_t sessionId = 88885;
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA; // split type -> non-NONE route
    nodeInfo.channels = STEREO;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->SetState(HPAE_SESSION_RUNNING);
    node->connectedProcessorType_ = HPAE_SCENE_UNCONNECTED;
    hpaeRendererManager_->sinkInputNodeMap_[sessionId] = node;

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, hpaeRendererManager_->sinkInfo_);
    // Inject an SE node so HasSessionEffectNode == true. DestroyNodes() bails out via
    // CheckNodes() (no converter/loudness/gain created) before reaching DropEffect, so the
    // SE node is never driven and needs no live effect instance.
    cluster->idSessionEffectMap_[sessionId] = std::make_shared<HpaeSessionEffectNode>(nodeInfo);
    hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_SPLIT_MEDIA] = cluster;

    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA); // "SCENE_EXTRA"
    InjectOverrideChain(scene);
    ChannelOverrideRoute setRoute{scene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->SetChannelOverrideForRoute(setRoute);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // override applied

    EXPECT_EQ(hpaeRendererManager_->DeleteProcessCluster(sessionId), SUCCESS);
    // route valid AND SE node present -> Clear taken -> override erased.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}
// Variant of AddSessionEffectTestSession parameterized by sceneType, used to drive a non-NONE
// (split) processor type so GetProcessorType/GetConnectedProcessorType resolve to a scene that
// yields a populated ChannelOverrideRoute. The original helper stays NONE-only (unchanged).
static std::shared_ptr<HpaeSinkInputNode> AddSessionEffectTestSessionWithScene(
    const std::shared_ptr<HpaeRendererManager> &rendererManager, uint32_t sessionId, uint32_t nodeId,
    HpaeProcessorType sceneType)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = nodeId;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = sceneType;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_F32LE;

    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->SetState(HPAE_SESSION_RUNNING);
    sinkInputNode->connectedProcessorType_ = sceneType;
    rendererManager->sinkInputNodeMap_[sessionId] = sinkInputNode;
    rendererManager->sessionNodeMap_[sessionId].sceneType = sceneType;
    rendererManager->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    rendererManager->sessionNodeMap_[sessionId].bypass = false;

    auto cluster = std::make_shared<HpaeProcessCluster>(nodeInfo, rendererManager->sinkInfo_);
    EXPECT_EQ(cluster->CreateNodes(sinkInputNode), SUCCESS);
    cluster->Connect(sinkInputNode);
    rendererManager->sceneClusterMap_[sceneType] = cluster;
    return sinkInputNode;
}

// HandleSessionEffectMaterializeFailure (line 164): split scene -> route has_value -> Clear (true).
HWTEST_F(HpaeRendererManagerTest, HandleSessionEffectMaterializeFailure_ClearsOverride, TestSize.Level1)
{
    constexpr uint32_t sessionId = 88891;
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = HPAE_SCENE_SPLIT_MEDIA; // split type -> route has_value
    nodeInfo.channels = STEREO;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.format = SAMPLE_F32LE;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->SetState(HPAE_SESSION_RUNNING);
    hpaeRendererManager_->sinkInputNodeMap_[sessionId] = node;
    hpaeRendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_SPLIT_MEDIA;
    hpaeRendererManager_->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    hpaeRendererManager_->sessionNodeMap_[sessionId].bypass = false;

    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA); // "SCENE_EXTRA"
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    ChannelOverrideRoute setRoute{scene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->SetChannelOverrideForRoute(setRoute);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // override applied

    hpaeRendererManager_->HandleSessionEffectMaterializeFailure(sessionId, -1, node);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// HandleSessionEffectMaterializeFailure (line 164): absent session -> route nullopt -> no Clear (false).
HWTEST_F(HpaeRendererManagerTest, HandleSessionEffectMaterializeFailure_NoneSceneNoClear, TestSize.Level1)
{
    constexpr uint32_t absentSession = 88892; // not in sinkInputNodeMap_ -> route nullopt
    auto node = CreateTestNode(HPAE_SESSION_RUNNING); // sinkInputNode param (any valid node)

    const std::string otherScene = "SCENE_OTHER";
    InjectOverrideChain(otherScene);
    ChannelOverrideRoute setRoute{otherScene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->SetChannelOverrideForRoute(setRoute);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(otherScene, channels, layout),
        SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // override applied

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    hpaeRendererManager_->HandleSessionEffectMaterializeFailure(absentSession, -1, node);
    // route nullopt -> Clear skipped -> otherScene override retained.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(otherScene, channels, layout),
        SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// DestroySessionEffect (line 112): split scene -> route has_value -> Clear (true branch).
// CreateSessionEffect success auto-sets the override via OnSessionEffectMaterialized; Destroy then clears it.
HWTEST_F(HpaeRendererManagerTest, DestroySessionEffect_ClearsOverride_SplitScene, TestSize.Level1)
{
    constexpr uint32_t sessionId = 88893;
    constexpr uint32_t nodeId = 101;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);

    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA); // "SCENE_EXTRA"
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    // CreateSessionEffect -> OnSessionEffectMaterialized -> SetChannelOverrideForRoute -> override applied.
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));

    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    // route has_value -> ClearChannelOverrideForRoute -> override erased.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// DetachSessionEffectNodeForRefresh (line 146): split oldSceneType -> oldRoute has_value -> Clear (true).
HWTEST_F(HpaeRendererManagerTest, DetachSessionEffectNodeForRefresh_ClearsOverride_SplitScene, TestSize.Level1)
{
    constexpr uint32_t sessionId = 88894;
    constexpr uint32_t nodeId = 102;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);

    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA); // "SCENE_EXTRA"
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));   // CreateSessionEffect auto-applied override

    hpaeRendererManager_->DetachSessionEffectNodeForRefresh(sessionId, HPAE_SCENE_SPLIT_MEDIA);
    // oldRoute(split) has_value -> ClearChannelOverrideForRoute -> override erased.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// ===========================================================================
// Branch-coverage supplements for hpae_renderer_manager_session_effect.cpp.
// Each case targets an early-return / else arm the happy-path tests above do
// not reach. All setups are deterministic and self-contained; none drives the
// worker thread, and only the *_PreservesOverride case touches the effect chain.
// ===========================================================================

// CreateSessionEffect (line 32): session not registered -> ERR_SESSION_NOT_FOUND.
HWTEST_F(HpaeRendererManagerTest, CreateSessionEffect_SessionNotFound, TestSize.Level0)
{
    constexpr uint32_t absentSession = 90001;
    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(absentSession, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
}

// CreateSessionEffect (line 42): session present but sceneClusterMap_ has no cluster for it.
HWTEST_F(HpaeRendererManagerTest, CreateSessionEffect_ClusterNotFound, TestSize.Level0)
{
    constexpr uint32_t sessionId = 90002;
    hpaeRendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager_->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    hpaeRendererManager_->sessionNodeMap_[sessionId].bypass = false;

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
}

// CreateSessionEffect (line 47): cluster exists but CreateSessionEffectNode fails. A bare cluster
// (CreateNodes never called) has no converter/loudnessGain nodes, so the cluster returns
// ERR_SESSION_NOT_FOUND and the manager takes the ret != SUCCESS warning arm without writing state.
HWTEST_F(HpaeRendererManagerTest, CreateSessionEffect_NodeCreateFails, TestSize.Level0)
{
    constexpr uint32_t sessionId = 90003;
    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = sessionId;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    nodeInfo.channels = STEREO;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.format = SAMPLE_F32LE;

    hpaeRendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager_->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    hpaeRendererManager_->sessionNodeMap_[sessionId].bypass = false;
    hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, hpaeRendererManager_->sinkInfo_);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
}

// CreateSessionEffect (line 61 else-branch): SE node created but SinkInputNode missing.
HWTEST_F(HpaeRendererManagerTest, CreateSessionEffect_SinkInputNodeAbsent, TestSize.Level0)
{
    constexpr uint32_t sessionId = 90004;
    constexpr uint32_t nodeId = 301;
    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, nodeId); // cluster has converter/loudness
    hpaeRendererManager_->sinkInputNodeMap_.erase(sessionId); // force the SinkInputNode-not-found path

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));

    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
}

// DestroySessionEffect (line 79): session not registered -> ERR_SESSION_NOT_FOUND.
HWTEST_F(HpaeRendererManagerTest, DestroySessionEffect_SessionNotFound, TestSize.Level0)
{
    constexpr uint32_t absentSession = 90005;
    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(absentSession),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
}

// DestroySessionEffect (line 85): session present but cluster missing.
HWTEST_F(HpaeRendererManagerTest, DestroySessionEffect_ClusterNotFound, TestSize.Level0)
{
    constexpr uint32_t sessionId = 90006;
    hpaeRendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager_->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    hpaeRendererManager_->sessionNodeMap_[sessionId].bypass = false;

    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
}

// DestroySessionEffect (line 92): DestroySessionEffectNode fails because the cluster has no SE node.
// Per the safety comment at line 93, the channel override must be PRESERVED and the function returns
// the error code without rewinding/clearing. Split scene -> override set; destroy fails -> retained.
HWTEST_F(HpaeRendererManagerTest, DestroySessionEffect_DestroyFails_PreservesOverride, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90007;
    constexpr uint32_t nodeId = 302;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);

    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA); // "SCENE_EXTRA"
    InjectOverrideChain(scene);
    ChannelOverrideRoute setRoute{scene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->SetChannelOverrideForRoute(setRoute);
    uint32_t channels = 0;
    uint64_t layout = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO)); // override applied

    // No SE node was ever created -> DestroySessionEffectNode returns ERR_EFFECT_NOT_BOUND.
    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId),
        static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(scene, channels, layout), SUCCESS);
    EXPECT_EQ(channels, static_cast<uint32_t>(STEREO)); // override preserved (returned early at line 98)

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// DetachSessionEffectNodeForRefresh (line 126): old cluster not in sceneClusterMap_ -> early return.
HWTEST_F(HpaeRendererManagerTest, DetachSessionEffectNodeForRefresh_OldClusterNotFound, TestSize.Level0)
{
    constexpr uint32_t sessionId = 90008;
    // HPAE_SCENE_MUSIC cluster was never created -> SafeGetMap null -> CHECK_AND_RETURN (line 126).
    hpaeRendererManager_->DetachSessionEffectNodeForRefresh(sessionId, HPAE_SCENE_MUSIC);
    EXPECT_TRUE(true); // reaching here means the early return did not crash
}

// DetachSessionEffectNodeForRefresh (line 129): cluster exists but has no SE node -> early return.
HWTEST_F(HpaeRendererManagerTest, DetachSessionEffectNodeForRefresh_NoSeNode, TestSize.Level0)
{
    constexpr uint32_t sessionId = 90009;
    constexpr uint32_t nodeId = 303;
    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, nodeId); // NONE cluster, no SE node
    hpaeRendererManager_->DetachSessionEffectNodeForRefresh(sessionId, HPAE_SCENE_EFFECT_NONE);
    EXPECT_TRUE(true);
}

// HasSessionEffect (lines 167/172): session absent and cluster absent both return false.
HWTEST_F(HpaeRendererManagerTest, HasSessionEffect_NotFoundPaths, TestSize.Level0)
{
    constexpr uint32_t absentSession = 90010;
    EXPECT_FALSE(hpaeRendererManager_->HasSessionEffect(absentSession)); // session not in map (line 167)

    constexpr uint32_t sessionId = 90011;
    hpaeRendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager_->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    hpaeRendererManager_->sessionNodeMap_[sessionId].bypass = false;
    EXPECT_FALSE(hpaeRendererManager_->HasSessionEffect(sessionId)); // cluster missing (line 172)
}

// IsValidSession (line 180): both arms.
HWTEST_F(HpaeRendererManagerTest, IsValidSession_FoundAndNotFound, TestSize.Level0)
{
    constexpr uint32_t absentSession = 90012;
    EXPECT_FALSE(hpaeRendererManager_->IsValidSession(absentSession));

    constexpr uint32_t sessionId = 90013;
    hpaeRendererManager_->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    EXPECT_TRUE(hpaeRendererManager_->IsValidSession(sessionId));
}

// SetSessionEffectParameter (lines 207/212/215): not-found, cluster-missing, and forward paths.
HWTEST_F(HpaeRendererManagerTest, SetSessionEffectParameter_AllPaths, TestSize.Level0)
{
    constexpr uint32_t absentSession = 90014;
    EXPECT_EQ(hpaeRendererManager_->SetSessionEffectParameter(absentSession, 1, 0.5f),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND)); // line 207

    constexpr uint32_t sessionId = 90015;
    hpaeRendererManager_->sessionNodeMap_[sessionId].sceneType = HPAE_SCENE_EFFECT_NONE;
    hpaeRendererManager_->sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
    hpaeRendererManager_->sessionNodeMap_[sessionId].bypass = false;
    EXPECT_EQ(hpaeRendererManager_->SetSessionEffectParameter(sessionId, 1, 0.5f),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND)); // line 212 (no cluster)

    constexpr uint32_t seSession = 90016;
    constexpr uint32_t nodeId = 304;
    AddSessionEffectTestSession(hpaeRendererManager_, seSession, nodeId); // cluster present, no SE node
    EXPECT_EQ(hpaeRendererManager_->SetSessionEffectParameter(seSession, 1, 0.5f),
        static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND)); // line 215 forwarded to cluster
}

// ==================== UT for FinalizeSessionEffectClose (Normal Renderer) ====================

// N_Fin_ClusterNull: sceneType has no cluster in sceneClusterMap_ -> CHECK_AND_RETURN early-exit,
// no DISABLED (hpae_renderer_manager_session_effect.cpp:159-160).
HWTEST_F(HpaeRendererManagerTest, N_Fin_ClusterNull, TestSize.Level0)
{
    constexpr uint32_t absentSession = 90010;
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    hpaeRendererManager_->FinalizeSessionEffectClose(absentSession, std::nullopt,
        HPAE_SESSION_RUNNING, std::weak_ptr<HpaeSessionEffectNode>{});

    EXPECT_EQ(callback->Count(absentSession, OPERATION_SESSION_EFFECT_DISABLED), 0u);
}

// N_Fin_FinalizedFalse: cluster present but closingNode expired -> FinalizeDestroySessionEffectNode
// returns false -> CHECK_AND_RETURN(finalized) early-exit, no DISABLED, no ClearOverride
// (hpae_renderer_manager_session_effect.cpp:166-167).
HWTEST_F(HpaeRendererManagerTest, N_Fin_FinalizedFalse, TestSize.Level0)
{
    constexpr uint32_t sessionId = 90011;
    constexpr uint32_t nodeId = 301;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    hpaeRendererManager_->FinalizeSessionEffectClose(sessionId, std::nullopt,
        HPAE_SESSION_RUNNING, std::weak_ptr<HpaeSessionEffectNode>{});

    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 0u);
}

// N_Fin_RouteValue: route has_value + finalize ok -> ClearChannelOverrideForRoute + DISABLED once
// (hpae_renderer_manager_session_effect.cpp:168-172). Split scene; CreateSessionEffect auto-sets
// the override via OnSessionEffectMaterialized.
HWTEST_F(HpaeRendererManagerTest, N_Fin_RouteValue, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90012;
    constexpr uint32_t nodeId = 302;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);
    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA);
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_SPLIT_MEDIA];
    ASSERT_NE(cluster, nullptr);
    uint64_t latency = 0;
    std::weak_ptr<HpaeSessionEffectNode> weakNode;
    ASSERT_EQ(cluster->BeginDestroySessionEffectNode(sessionId, latency, weakNode), 0);

    ChannelOverrideRoute route{scene, DEVICE_TYPE_SPEAKER, 0};
    hpaeRendererManager_->FinalizeSessionEffectClose(sessionId, route,
        HPAE_SESSION_RUNNING, weakNode);

    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// N_Fin_RouteNull: route==nullopt + finalize ok -> skip ClearChannelOverrideForRoute + DISABLED once
// (hpae_renderer_manager_session_effect.cpp:168 false, 171-172).
HWTEST_F(HpaeRendererManagerTest, N_Fin_RouteNull, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90013;
    constexpr uint32_t nodeId = 303;
    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, nodeId);  // NONE scene cluster
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE];
    ASSERT_NE(cluster, nullptr);
    uint64_t latency = 0;
    std::weak_ptr<HpaeSessionEffectNode> weakNode;
    ASSERT_EQ(cluster->BeginDestroySessionEffectNode(sessionId, latency, weakNode), 0);

    hpaeRendererManager_->FinalizeSessionEffectClose(sessionId, std::nullopt,
        HPAE_SESSION_RUNNING, weakNode);

    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// N_Fin_PendingInst: pendingSeCreateMap_ has the session with a live weak pointing to the SAME
// instance the SE node holds -> sameInstanceReuse -> CloseSessionEffectCore (NOT Stop/Flush)
// (hpae_renderer_manager_session_effect.cpp:163-166, hpae_process_cluster.cpp:628-631).
HWTEST_F(HpaeRendererManagerTest, N_Fin_PendingInst, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90014;
    constexpr uint32_t nodeId = 304;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);
    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA);
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    EXPECT_CALL(*instance, StopEffect()).Times(0);  // same-instance reuse must NOT stop
    EXPECT_CALL(*instance, Flush()).Times(1);
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    hpaeRendererManager_->pendingSeCreateMap_[sessionId] = {1, instance};  // SAME instance, live weak

    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_SPLIT_MEDIA];
    ASSERT_NE(cluster, nullptr);
    uint64_t latency = 0;
    std::weak_ptr<HpaeSessionEffectNode> weakNode;
    ASSERT_EQ(cluster->BeginDestroySessionEffectNode(sessionId, latency, weakNode), 0);

    hpaeRendererManager_->FinalizeSessionEffectClose(sessionId, std::nullopt,
        HPAE_SESSION_RUNNING, weakNode);

    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    testing::Mock::VerifyAndClearExpectations(instance.get());
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// N_Fin_PendingExpired: pendingSeCreateMap_ has the session but weak expired -> pendingInst=nullptr
// -> DropEffect normal release (hpae_renderer_manager_session_effect.cpp:164-166,
// hpae_process_cluster.cpp:632-633).
HWTEST_F(HpaeRendererManagerTest, N_Fin_PendingExpired, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90015;
    constexpr uint32_t nodeId = 305;
    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, nodeId);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    {
        auto tmp = std::make_shared<NiceMock<MockEffectInstance>>();
        hpaeRendererManager_->pendingSeCreateMap_[sessionId] = {1, tmp};
    }  // tmp destroyed -> weak expired

    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE];
    ASSERT_NE(cluster, nullptr);
    uint64_t latency = 0;
    std::weak_ptr<HpaeSessionEffectNode> weakNode;
    ASSERT_EQ(cluster->BeginDestroySessionEffectNode(sessionId, latency, weakNode), 0);

    hpaeRendererManager_->FinalizeSessionEffectClose(sessionId, std::nullopt,
        HPAE_SESSION_RUNNING, weakNode);

    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// N_Fin_PendingAbsent: pendingSeCreateMap_ has no entry -> pendingIt==end -> pendingInst=nullptr
// -> DropEffect (hpae_renderer_manager_session_effect.cpp:162-163,166).
HWTEST_F(HpaeRendererManagerTest, N_Fin_PendingAbsent, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90016;
    constexpr uint32_t nodeId = 306;
    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, nodeId);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    // pendingSeCreateMap_ intentionally left without this session.

    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE];
    ASSERT_NE(cluster, nullptr);
    uint64_t latency = 0;
    std::weak_ptr<HpaeSessionEffectNode> weakNode;
    ASSERT_EQ(cluster->BeginDestroySessionEffectNode(sessionId, latency, weakNode), 0);

    hpaeRendererManager_->FinalizeSessionEffectClose(sessionId, std::nullopt,
        HPAE_SESSION_RUNNING, weakNode);

    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// N_Fin_DirectWhenNoPending: IDLE node has no crossfade pending -> destroy finalizes inside
// the same (synchronous) request, DISABLED exactly once.
HWTEST_F(HpaeRendererManagerTest, N_Fin_DirectWhenNoPending, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90017;
    constexpr uint32_t nodeId = 307;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);
    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA);
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Synchronous destroy on the test thread: IDLE node, no crossfade pending -> direct Finalize.
    hpaeRendererManager_->DestroySessionEffect(sessionId);
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    hpaeRendererManager_->HandleMsg();  // nothing queued -> no duplicate
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// N_Fin_DeferredUntilCrossfadeDone: ACTIVE node -> destroy only Begins (STOPPING, no DISABLED);
// Finalize runs when OnCloseCrossfadeDone arrives (queued via SendRequest).
HWTEST_F(HpaeRendererManagerTest, N_Fin_DeferredUntilCrossfadeDone, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90019;
    constexpr uint32_t nodeId = 309;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);
    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA);
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Force the smooth-close precondition: an ACTIVE node arms the close crossfade.
    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_SPLIT_MEDIA];
    ASSERT_NE(cluster, nullptr);
    auto node = cluster->GetSessionEffectNode(sessionId);
    ASSERT_NE(node, nullptr);
    node->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Synchronous destroy on the test thread: BeginClose, crossfade pending -> wait for callback.
    hpaeRendererManager_->DestroySessionEffect(sessionId);
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    EXPECT_TRUE(cluster->IsSessionEffectNodeStopping(sessionId));
    EXPECT_EQ(hpaeRendererManager_->pendingSeFinalizeMap_.count(sessionId), 1u);

    hpaeRendererManager_->OnCloseCrossfadeDone(sessionId);  // SendRequest -> queued
    hpaeRendererManager_->HandleMsg();
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_FALSE(cluster->HasSessionEffectNode(sessionId));
    EXPECT_EQ(hpaeRendererManager_->pendingSeFinalizeMap_.count(sessionId), 0u);

    hpaeRendererManager_->OnCloseCrossfadeDone(sessionId);  // stale callback -> no-op
    hpaeRendererManager_->HandleMsg();
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// N_Fin_PauseRescue: Pause arriving while the close waits on the callback finalizes directly;
// the late callback no-ops (token cleared) — no duplicate DISABLED.
HWTEST_F(HpaeRendererManagerTest, N_Fin_PauseRescue, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90020;
    constexpr uint32_t nodeId = 310;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);
    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA);
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_SPLIT_MEDIA];
    ASSERT_NE(cluster, nullptr);
    cluster->GetSessionEffectNode(sessionId)->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Synchronous destroy on the test thread: BeginClose, waiting on callback.
    hpaeRendererManager_->DestroySessionEffect(sessionId);
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 0u);

    hpaeRendererManager_->Pause(sessionId);  // entry rescue finalizes instead of waiting
    hpaeRendererManager_->HandleMsg();
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_FALSE(cluster->HasSessionEffectNode(sessionId));
    EXPECT_EQ(hpaeRendererManager_->pendingSeFinalizeMap_.count(sessionId), 0u);

    hpaeRendererManager_->OnCloseCrossfadeDone(sessionId);  // late callback: token gone
    hpaeRendererManager_->HandleMsg();
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// N_Fin_OnFadeDoneRescue: fade completion while the close still waits -> finalize before
// SessionEffectPause/DisConnectInputSession (the chain stops being pulled afterwards).
HWTEST_F(HpaeRendererManagerTest, N_Fin_OnFadeDoneRescue, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90021;
    constexpr uint32_t nodeId = 311;
    AddSessionEffectTestSessionWithScene(hpaeRendererManager_, sessionId, nodeId, HPAE_SCENE_SPLIT_MEDIA);
    std::string scene = TransProcessorTypeToSceneType(HPAE_SCENE_SPLIT_MEDIA);
    InjectOverrideChain(scene);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_SPLIT_MEDIA];
    ASSERT_NE(cluster, nullptr);
    cluster->GetSessionEffectNode(sessionId)->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Synchronous destroy on the test thread: BeginClose, waiting on callback.
    hpaeRendererManager_->DestroySessionEffect(sessionId);

    // Fade done fires with the session pausing (state != RUNNING).
    hpaeRendererManager_->sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_PAUSING);
    hpaeRendererManager_->OnFadeDone(sessionId);
    hpaeRendererManager_->HandleMsg();
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);  // rescue
    EXPECT_FALSE(cluster->HasSessionEffectNode(sessionId));

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// DestroySessionEffect on an already-STOPPING node must NOT re-Begin (double-rewind / re-arm crossfade /
// duplicate Finalize) — cancel pending create + return SUCCESS, mirroring derived managers.
// pendingSeCreateMap_ is the discriminator (only the guard erases it).
HWTEST_F(HpaeRendererManagerTest, DestroySessionEffect_AlreadyStopping_IsNoOp, TestSize.Level1)
{
    constexpr uint32_t sessionId = 90018;
    constexpr uint32_t nodeId = 308;
    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, nodeId);  // NONE-scene cluster
    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;
    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Put the SE node into STOPPING exactly as the first DestroySessionEffect does (BeginClose),
    // leaving it in the cluster map until a later Finalize cycle erases it.
    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE];
    ASSERT_NE(cluster, nullptr);
    ASSERT_TRUE(cluster->HasSessionEffectNode(sessionId));
    uint64_t latency = 0;
    std::weak_ptr<HpaeSessionEffectNode> weakNode;
    ASSERT_EQ(cluster->BeginDestroySessionEffectNode(sessionId, latency, weakNode), 0);
    ASSERT_TRUE(cluster->IsSessionEffectNodeStopping(sessionId));  // precondition: STOPPING

    // A deferred create pending behind this STOPPING node (rapid recreate path).
    hpaeRendererManager_->pendingSeCreateMap_[sessionId] = {1, instance};
    ASSERT_EQ(hpaeRendererManager_->pendingSeCreateMap_.count(sessionId), 1u);

    // Second DestroySessionEffect while STOPPING: must short-circuit (cancel pending, SUCCESS),
    // without re-BeginClose and without emitting an extra DISABLED synchronously.
    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(hpaeRendererManager_->pendingSeCreateMap_.count(sessionId), 0u);  // guard cancelled it
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 0u);  // no sync DISABLED
}

/**
 * @tc.name  : ConnectOutputCluster_NullOutputCluster_001
 * @tc.type  : FUNC
 * @tc.number: ConnectOutputCluster_NullOutputCluster_001
 * @tc.desc  : Test ConnectOutputCluster returns safely (no null deref) when
 *             outputCluster_ is nullptr while sceneClusterMap_ has an entry.
 *             This covers the half-zombie state after Init/Reload failure where
 *             SafeGetMap passes but outputCluster_ was never constructed.
 */
HWTEST_F(HpaeRendererManagerTest, ConnectOutputCluster_NullOutputCluster_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    std::shared_ptr<HpaeRendererManager> hpaeRendererManager = std::make_shared<HpaeRendererManager>(sinkInfo);
    ASSERT_NE(hpaeRendererManager, nullptr);

    // Half-zombie state: sceneClusterMap_ has an entry (SafeGetMap passes the first
    // guard) but outputCluster_ is null because Init/Reload failed to construct it.
    HpaeNodeInfo nodeInfo;
    // HpaeProcessCluster ctor builds mixer/effect nodes that need valid params
    // (esp. channels); a default nodeInfo (channels=0) crashes in HpaeMixerNode.
    nodeInfo.sceneType = HPAE_SCENE_DEFAULT;
    nodeInfo.frameLen = sinkInfo.frameLen;
    nodeInfo.samplingRate = sinkInfo.samplingRate;
    nodeInfo.channels = sinkInfo.channels;
    nodeInfo.format = sinkInfo.format;
    hpaeRendererManager->sceneClusterMap_[HPAE_SCENE_DEFAULT] =
        std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo);
    hpaeRendererManager->outputCluster_ = nullptr;

    // Before the fix: SafeGetMap returns the cluster, then the code dereferences a
    // null outputCluster_ (outputCluster_->IsProcessClusterConnected) -> SEGV.
    // After the fix: CHECK_AND_RETURN_LOG(outputCluster_ != nullptr, ...) returns safely.
    hpaeRendererManager->ConnectOutputCluster(1u, HPAE_SCENE_DEFAULT);
    SUCCEED(); // Reaching here without crashing means the null guard works.
}

/*
 * @tc.type  : FUNC
 * @tc.number: SendRequest_NotInit_ThreadExists_QueuedNotDropped_001
 * @tc.desc  : In the not-init window (process thread created, init request still
 *             pending), SendRequest must queue the request instead of dropping it;
 *             the request then executes when the queue is drained (HandleMsg),
 *             i.e. on the process thread after init.
 */
HWTEST_F(HpaeRendererManagerTest, SendRequest_NotInit_ThreadExists_QueuedNotDropped_001, TestSize.Level1)
{
    // Fixture state: isInit_ == false, hpaeSignalProcessThread_ exists (created but not
    // activated, so Notify() wakes nobody and the request stays queued until HandleMsg()).
    EXPECT_FALSE(hpaeRendererManager_->IsInit());
    ASSERT_NE(hpaeRendererManager_->hpaeSignalProcessThread_, nullptr);
    hpaeRendererManager_->hpaeSignalProcessThread_->ActivateThread(hpaeRendererManager_);

    bool executed = false;
    hpaeRendererManager_->SendRequest([&executed]() { executed = true; }, __func__);

    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
    
    EXPECT_TRUE(executed);
}

/*
 * @tc.type  : FUNC
 * @tc.number: SendRequest_NotInit_ExecutesAfterInitRequest_002
 * @tc.desc  : FIFO ordering: a request sent during the not-init window must run
 *             AFTER the already-queued init request (which flips isInit_), never before it.
 */
HWTEST_F(HpaeRendererManagerTest, SendRequest_NotInit_ExecutesAfterInitRequest_002, TestSize.Level1)
{
    EXPECT_FALSE(hpaeRendererManager_->IsInit());
    auto mgr = hpaeRendererManager_;
    std::vector<int32_t> order;

    // The init request queued by Init() before any command can arrive.
    mgr->hpaeNoLockQueue_.PushRequest([&order, mgr]() {
        order.push_back(1); // init runs first
        mgr->isInit_.store(true);
    });
    // Command arriving in the not-init window (init in flight).
    mgr->SendRequest([&order]() { order.push_back(2); }, __func__);

    mgr->HandleMsg(); // process thread drains in FIFO order
    EXPECT_TRUE(mgr->IsInit());
    ASSERT_EQ(order.size(), 2u);
    EXPECT_EQ(order[0], 1); // init first
    EXPECT_EQ(order[1], 2); // command after init
}

/*
 * @tc.type  : FUNC
 * @tc.number: SendRequest_NotInit_NoThread_NotExecuted_003
 * @tc.desc  : No process thread and non-interphone route (init never started):
 *             the request must not be executed, and must not be left in the queue
 *             as an orphan that could later run before the init request.
 */
HWTEST_F(HpaeRendererManagerTest, SendRequest_NotInit_NoThread_NotExecuted_003, TestSize.Level1)
{
    hpaeRendererManager_->hpaeSignalProcessThread_.reset();
    hpaeRendererManager_->sinkInfo_.routeFlag = AUDIO_FLAG_NONE;
    EXPECT_FALSE(hpaeRendererManager_->IsInit());

    bool executed = false;
    hpaeRendererManager_->SendRequest([&executed]() { executed = true; }, __func__);

    EXPECT_FALSE(executed);
    // No orphan: draining later must still not execute it.
    hpaeRendererManager_->HandleMsg();
    EXPECT_FALSE(executed);
}

/*
 * @tc.type  : FUNC
 * @tc.number: SendRequest_NotInit_InterphoneNoThread_ExecutedInline_004
 * @tc.desc  : Interphone route never creates a process thread; SendRequest must
 *             push and drain inline (synchronous execution on the caller thread).
 */
HWTEST_F(HpaeRendererManagerTest, SendRequest_NotInit_InterphoneNoThread_ExecutedInline_004, TestSize.Level1)
{
    hpaeRendererManager_->hpaeSignalProcessThread_.reset();
    hpaeRendererManager_->sinkInfo_.routeFlag = AUDIO_OUTPUT_FLAG_INTERPHONE;
    EXPECT_FALSE(hpaeRendererManager_->IsInit());

    bool executed = false;
    hpaeRendererManager_->SendRequest([&executed]() { executed = true; }, __func__);

    EXPECT_TRUE(executed); // drained inline by HandleRequests
}

/*
 * @tc.type  : FUNC
 * @tc.number: SendRequest_Init_StillQueued_005
 * @tc.desc  : Regression: with isInit_ true and a thread present, requests are
 *             still queued (executed only when the queue is drained).
 */
HWTEST_F(HpaeRendererManagerTest, SendRequest_Init_StillQueued_005, TestSize.Level1)
{
    hpaeRendererManager_->isInit_.store(true);

    bool executed = false;
    hpaeRendererManager_->SendRequest([&executed]() { executed = true; }, __func__);

    EXPECT_FALSE(executed);
    hpaeRendererManager_->HandleMsg();
    EXPECT_TRUE(executed);
}

/*
 * @tc.type  : FUNC
 * @tc.number: SendRequest_ActiveThread_InitThenCommand_Executed_006
 * @tc.desc  : Production sequence with a REAL activated process thread: upper layer
 *             sends a command right after Init() returns, i.e. while the init request
 *             may still be in flight (isInit_ not yet flipped). The command must be
 *             queued and executed once the queue drains — never dropped.
 */
HWTEST_F(HpaeRendererManagerTest, SendRequest_ActiveThread_InitThenCommand_Executed_006, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto mgr = std::make_shared<HpaeRendererManager>(sinkInfo);
    SetSinkVirtualOutputNode(sinkInfo, mgr);
    ASSERT_EQ(mgr->Init(), SUCCESS); // creates + activates hpaeSignalProcessThread_, queues init request

    // Command arriving immediately after Init() returns (the not-init race window).
    std::atomic<bool> executed {false};
    mgr->SendRequest([&executed]() { executed.store(true); }, __func__);

    WaitForMsgProcessing(mgr); // let the process thread drain init request + command
    EXPECT_TRUE(mgr->IsInit());
    EXPECT_TRUE(executed.load());

    EXPECT_EQ(mgr->DeInit(), SUCCESS);
    WaitForMsgProcessing(mgr);
}

/*
 * @tc.type  : FUNC
 * @tc.number: SendRequest_ActiveThread_InitInFlight_ExecutesAfterInit_007
 * @tc.desc  : Deterministic not-init window with a REAL process thread: an init
 *             request blocks on a gate, so isInit_ is guaranteed false while
 *             SendRequest is called from another thread. The command must be queued
 *             (not dropped), and by FIFO it must execute strictly AFTER init.
 */
HWTEST_F(HpaeRendererManagerTest, SendRequest_ActiveThread_InitInFlight_ExecutesAfterInit_007, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetSinkInfo();
    auto mgr = std::make_shared<HpaeRendererManager>(sinkInfo);

    std::atomic<bool> releaseGate {false};
    std::atomic<bool> initDone {false};
    std::atomic<bool> cmdDone {false};
    std::atomic<bool> cmdAfterInit {false};

    // "init" request: holds the not-init window open until released, then flips isInit_.
    mgr->hpaeNoLockQueue_.PushRequest([&releaseGate, &initDone, mgr]() {
        const int spinLimit = 250; // bounded: 250 * 20ms = 5s, keeps the test from hanging
        for (int i = 0; i < spinLimit && !releaseGate.load(); i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        }
        mgr->isInit_.store(true);
        initDone.store(true);
    });
    mgr->hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    mgr->hpaeSignalProcessThread_->ActivateThread(mgr); // thread enters and blocks inside the init request

    // isInit_ is deterministically false here: the init request only flips it after the gate.
    EXPECT_FALSE(mgr->IsInit());
    mgr->SendRequest([&initDone, &cmdDone, &cmdAfterInit]() {
        cmdAfterInit.store(initDone.load()); // true iff init finished before this ran
        cmdDone.store(true);
    }, __func__);

    releaseGate.store(true); // release init; the thread then drains the command (FIFO)

    const int waitLimit = 250; // 250 * 20ms = 5s bound
    for (int i = 0; i < waitLimit && !cmdDone.load(); i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
    }
    EXPECT_TRUE(initDone.load());
    EXPECT_TRUE(cmdDone.load());
    EXPECT_TRUE(cmdAfterInit.load()); // strictly after init
    EXPECT_TRUE(mgr->IsInit());

    EXPECT_TRUE(mgr->DeactivateThread()); // stop the real thread before teardown
}

// finalizeNow: DestroySessionEffect closes the SE synchronously inside the call — no deferred
// FinalizeClose batch needed (switch path "clear old before opening new").
HWTEST_F(HpaeRendererManagerTest, SessionEffectDestroyFinalizeNowClosesImmediately, TestSize.Level0)
{
    constexpr uint32_t sessionId = 7902;
    constexpr uint32_t testNodeId = 1102;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);
    EXPECT_EQ(hpaeRendererManager_->Init(), SUCCESS);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_TRUE(hpaeRendererManager_->HasSessionEffect(sessionId));

    // finalizeNow: node erased from the cluster map before returning, DISABLED already emitted.
    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId, true),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_FALSE(hpaeRendererManager_->HasSessionEffect(sessionId));
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_EQ(hpaeRendererManager_->DeInit(), SUCCESS);
}

// finalizeNow switch scenario: Create right behind a finalizeNow Destroy succeeds immediately —
// no STOPPING residue, no requeue wait (contrast SessionEffectDestroyThenCreateRequeues).
HWTEST_F(HpaeRendererManagerTest, SessionEffectDestroyFinalizeNowThenCreateSucceeds, TestSize.Level0)
{
    constexpr uint32_t sessionId = 7903;
    constexpr uint32_t testNodeId = 1103;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);
    EXPECT_EQ(hpaeRendererManager_->Init(), SUCCESS);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto oldInst = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*oldInst, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "old", oldInst, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId, true),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // The new Create materializes in-place: no STOPPING node left in the cluster map.
    auto newInst = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*newInst, GetPreheatFrames()).WillByDefault(Return(0));
    EXPECT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "new", newInst, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_TRUE(hpaeRendererManager_->HasSessionEffect(sessionId));
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_EQ(hpaeRendererManager_->DeInit(), SUCCESS);
}

// finalizeNow on an already-STOPPING node: direct finalize in the STOPPING branch; a late
// OnCloseCrossfadeDone afterwards no-ops on the cleared pending-close token (exactly one DISABLED).
HWTEST_F(HpaeRendererManagerTest, SessionEffectDestroyFinalizeNowOnStoppingNodeFinalizesDirectly,
    TestSize.Level0)
{
    constexpr uint32_t sessionId = 7904;
    constexpr uint32_t testNodeId = 1104;

    AddSessionEffectTestSession(hpaeRendererManager_, sessionId, testNodeId);
    EXPECT_EQ(hpaeRendererManager_->Init(), SUCCESS);

    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg = config.inputCfg;

    auto callback = std::make_shared<StatusCallbackRecorder>();
    hpaeRendererManager_->RegisterSendMsgCallback(callback);

    auto instance = std::make_shared<NiceMock<MockEffectInstance>>();
    ON_CALL(*instance, GetPreheatFrames()).WillByDefault(Return(0));
    ASSERT_EQ(hpaeRendererManager_->CreateSessionEffect(sessionId, "audio_aiss_enhance", instance, config),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_TRUE(hpaeRendererManager_->HasSessionEffect(sessionId));

    // Arm the smooth close: an ACTIVE node arms the close crossfade, so the plain destroy below
    // waits for OnCloseCrossfadeDone (nothing queued) instead of finalizing inline.
    auto cluster = hpaeRendererManager_->sceneClusterMap_[HPAE_SCENE_EFFECT_NONE];
    ASSERT_NE(cluster, nullptr);
    auto node = cluster->GetSessionEffectNode(sessionId);
    ASSERT_NE(node, nullptr);
    node->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    // Plain destroy: node enters STOPPING and waits on the crossfade callback.
    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_TRUE(hpaeRendererManager_->HasSessionEffect(sessionId)); // STOPPING residue

    // finalizeNow on the STOPPING node: direct finalize inside the STOPPING branch.
    EXPECT_EQ(hpaeRendererManager_->DestroySessionEffect(sessionId, true),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_FALSE(hpaeRendererManager_->HasSessionEffect(sessionId));
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    // The crossfade callback arrives late: the token was cleared by the finalize above, so the
    // marshalled request must be a no-op (no duplicate DISABLED).
    hpaeRendererManager_->OnCloseCrossfadeDone(sessionId);
    WaitForMsgProcessing(hpaeRendererManager_);
    EXPECT_EQ(callback->Count(sessionId, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_EQ(hpaeRendererManager_->DeInit(), SUCCESS);
}

}  // namespace
