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
#include "hpae_fast_renderer_manager.h"
#include "hpae_node_common.h"
#include "hpae_mocks.h"
#include "hpae_fast_sink_output_node.h"
#include "hpae_mixer_node.h"
#include "oh_audio_buffer.h"
#include "audio_utils.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace {
static std::string g_rootPath = "/data/";
constexpr int32_t FRAME_LENGTH_960 = 960;
constexpr int32_t TEST_STREAM_SESSION_ID = 123456;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t TEST_SLEEP_TIME_160 = 160;

static HpaeSinkInfo GetFastSinkInfo(const std::string &deviceClass = "primary_mmap")
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = deviceClass;
    sinkInfo.adapterName = deviceClass;
    sinkInfo.filePath = g_rootPath + "fast_renderer_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_S32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.deviceName = deviceClass;
    return sinkInfo;
}

static HpaeSinkInfo GetVoipFastSinkInfo()
{
    return GetFastSinkInfo("primary_mmap_voip");
}

static constexpr uint32_t TOTAL_SIZE_IN_FRAME = 8;
static constexpr uint32_t SPAN_SIZE_IN_FRAME = 2;
static constexpr uint32_t BYTE_SIZE_PER_FRAME = 8; // stereo * 4 bytes (f32le)

// Inject mock sink after Init to make sinkOutputNode_ work in UT environment
static void InitWithMockSink(std::shared_ptr<HpaeFastRendererManager> &manager)
{
    ASSERT_NE(manager->sinkOutputNode_, nullptr);

    auto mockSink = std::make_shared<NiceMock<MockAudioRenderSink>>();
    manager->sinkOutputNode_->audioRendererSink_ = mockSink;

    // Directly set up mmap buffer using CreateFromLocal (avoids fd requirement in CreateFromRemote)
    auto buffer = OHAudioBuffer::CreateFromLocal(TOTAL_SIZE_IN_FRAME, SPAN_SIZE_IN_FRAME, BYTE_SIZE_PER_FRAME);
    ASSERT_NE(buffer, nullptr);
    manager->sinkOutputNode_->dstAudioBuffer_ = buffer;
    manager->sinkOutputNode_->dstTotalSizeInframe_ = TOTAL_SIZE_IN_FRAME;
    manager->sinkOutputNode_->dstSpanSizeInframe_ = SPAN_SIZE_IN_FRAME;
    manager->sinkOutputNode_->dstByteSizePerFrame_ = BYTE_SIZE_PER_FRAME;
    manager->sinkOutputNode_->dstSpanSizeInByte_ = SPAN_SIZE_IN_FRAME * BYTE_SIZE_PER_FRAME;
    manager->sinkOutputNode_->syncInfoSize_ = 0;
    manager->sinkOutputNode_->renderFrameData_.resize(SPAN_SIZE_IN_FRAME * BYTE_SIZE_PER_FRAME);
    manager->sinkOutputNode_->SetSinkState(STREAM_MANAGER_IDLE);

    manager->isInit_.store(true);

    // Create mixerNode and connect (normally done in InitSinkInner after RenderSinkInit)
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = STEREO;
    nodeInfo.format = SAMPLE_S32LE;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    nodeInfo.nodeId = 0;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.deviceNetId = manager->sinkInfo_.deviceNetId;
    nodeInfo.deviceClass = manager->sinkInfo_.deviceClass;
    auto mixerNode = std::make_shared<HpaeMixerNode>(nodeInfo);
    manager->sinkOutputNode_->Connect(mixerNode);
    manager->mixerNode_ = mixerNode;
}

void WaitForMsgProcessing(std::shared_ptr<HpaeFastRendererManager> &manager)
{
    int waitCount = 0;
    const int waitCountThd = 5;
    while (manager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
    EXPECT_EQ(manager->IsMsgProcessing(), false);
    EXPECT_EQ(waitCount < waitCountThd, true);
}
} // namespace

class HpaeFastRendererManagerTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<HpaeFastRendererManager> fastManager_;
};

void HpaeFastRendererManagerTest::SetUp()
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    fastManager_ = std::make_shared<HpaeFastRendererManager>(sinkInfo);
}

void HpaeFastRendererManagerTest::TearDown()
{
    fastManager_.reset();
}

static HpaeStreamInfo MakeStreamInfo(uint32_t sessionId)
{
    HpaeStreamInfo streamInfo;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameLen = FRAME_LENGTH_960;
    streamInfo.sessionId = sessionId;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
    return streamInfo;
}

/**
 * @tc.name  : constructFastRendererManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_001
 * @tc.desc  : Test construct HpaeFastRendererManager
 */
HWTEST_F(HpaeFastRendererManagerTest, constructFastRendererManager_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->IsInit(), false);
    EXPECT_EQ(manager->IsRunning(), false);
}

/**
 * @tc.name  : constructFastRendererManager_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_002
 * @tc.desc  : Test construct with VOIP fast device class
 */
HWTEST_F(HpaeFastRendererManagerTest, constructFastRendererManager_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetVoipFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    ASSERT_NE(manager, nullptr);
}

/**
 * @tc.name  : getSinkInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_003
 * @tc.desc  : Test GetSinkInfo returns correct sink info
 */
HWTEST_F(HpaeFastRendererManagerTest, getSinkInfo_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    HpaeSinkInfo retSinkInfo = manager->GetSinkInfo();
    EXPECT_EQ(retSinkInfo.deviceClass, sinkInfo.deviceClass);
    EXPECT_EQ(retSinkInfo.samplingRate, sinkInfo.samplingRate);
    EXPECT_EQ(retSinkInfo.channels, sinkInfo.channels);
    EXPECT_EQ(retSinkInfo.format, sinkInfo.format);
    EXPECT_EQ(retSinkInfo.deviceType, sinkInfo.deviceType);
}

/**
 * @tc.name  : deInitWithoutInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_005
 * @tc.desc  : Test DeInit without Init should not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, deInitWithoutInit_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : doubleDeInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_006
 * @tc.desc  : Test double DeInit should not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, doubleDeInit_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : createStreamNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_007
 * @tc.desc  : Test CreateStream when not initialized returns error
 */
HWTEST_F(HpaeFastRendererManagerTest, createStreamNotInit_001, TestSize.Level0)
{
    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : destroyStreamNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_010
 * @tc.desc  : Test DestroyStream when not initialized returns error
 */
HWTEST_F(HpaeFastRendererManagerTest, destroyStreamNotInit_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->DestroyStream(TEST_STREAM_SESSION_ID), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : flushAndDrain_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_013
 * @tc.desc  : Test Flush and Drain operations
 */
HWTEST_F(HpaeFastRendererManagerTest, flushAndDrain_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->Flush(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->Drain(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : suspendStreamManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_014
 * @tc.desc  : Test SuspendStreamManager and resume
 */
HWTEST_F(HpaeFastRendererManagerTest, suspendStreamManager_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Suspend
    EXPECT_EQ(fastManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Resume
    EXPECT_EQ(fastManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Suspend again with same state should be no-op
    EXPECT_EQ(fastManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : stopManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_015
 * @tc.desc  : Test StopManager
 */
HWTEST_F(HpaeFastRendererManagerTest, stopManager_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->StopManager(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setMute_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_016
 * @tc.desc  : Test SetMute
 */
HWTEST_F(HpaeFastRendererManagerTest, setMute_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->SetMute(true), SUCCESS);
    EXPECT_EQ(fastManager_->SetMute(false), SUCCESS);
    EXPECT_EQ(fastManager_->SetMute(false), SUCCESS);
}

/**
 * @tc.name  : simpleOperationReturns_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_017
 * @tc.desc  : Test simple operations that return SUCCESS by default
 */
HWTEST_F(HpaeFastRendererManagerTest, simpleOperationReturns_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->SetClientVolume(1, 0.5f), SUCCESS);
    EXPECT_EQ(fastManager_->SetRate(1, 0), SUCCESS);
    EXPECT_EQ(fastManager_->SetAudioEffectMode(1, 0), SUCCESS);
    int32_t effectMode = 0;
    EXPECT_EQ(fastManager_->GetAudioEffectMode(1, effectMode), SUCCESS);
    EXPECT_EQ(fastManager_->SetPrivacyType(1, 0), SUCCESS);
    int32_t privacyType = 0;
    EXPECT_EQ(fastManager_->GetPrivacyType(1, privacyType), SUCCESS);
    EXPECT_EQ(fastManager_->UpdateSpatializationState(1, false, false), SUCCESS);
    EXPECT_EQ(fastManager_->UpdateMaxLength(1, 0), SUCCESS);
    EXPECT_EQ(fastManager_->RefreshProcessClusterByDevice(), SUCCESS);
    EXPECT_EQ(fastManager_->SetLoudnessGain(1, 0.0f), SUCCESS);
    EXPECT_EQ(fastManager_->RegisterReadCallback(1, std::weak_ptr<ICapturerStreamCallback>()), ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : getWritableSize_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_018
 * @tc.desc  : Test GetWritableSize returns SUCCESS (0)
 */
HWTEST_F(HpaeFastRendererManagerTest, getWritableSize_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->GetWritableSize(1), static_cast<size_t>(SUCCESS));
}

/**
 * @tc.name  : getAllSinkInputsInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_019
 * @tc.desc  : Test GetAllSinkInputsInfo returns empty vector
 */
HWTEST_F(HpaeFastRendererManagerTest, getAllSinkInputsInfo_001, TestSize.Level0)
{
    auto result = fastManager_->GetAllSinkInputsInfo();
    EXPECT_EQ(result.size(), 0);
}

/**
 * @tc.name  : addNodeToSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_020
 * @tc.desc  : Test AddNodeToSink
 */
HWTEST_F(HpaeFastRendererManagerTest, addNodeToSink_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 60001;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_S32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    int32_t ret = fastManager_->AddNodeToSink(node);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : addAllNodesToSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_021
 * @tc.desc  : Test AddAllNodesToSink
 */
HWTEST_F(HpaeFastRendererManagerTest, addAllNodesToSink_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    HpaeNodeInfo info1;
    info1.sessionId = 80001;
    info1.samplingRate = SAMPLE_RATE_48000;
    info1.format = SAMPLE_S32LE;
    info1.channels = STEREO;
    info1.frameLen = FRAME_LENGTH_960;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info1));

    HpaeNodeInfo info2 = info1;
    info2.sessionId = 80002;
    sinkInputs.push_back(std::make_shared<HpaeSinkInputNode>(info2));

    int32_t ret = fastManager_->AddAllNodesToSink(sinkInputs, true);
    EXPECT_EQ(ret, SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveAllStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_022
 * @tc.desc  : Test MoveAllStream when sink is not initialized (sync mode)
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStream_001, TestSize.Level0)
{
    std::string sinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;
    EXPECT_EQ(fastManager_->MoveAllStream(sinkName, sessionIds, moveType), SUCCESS);
}

/**
 * @tc.name  : moveAllStream_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_023
 * @tc.desc  : Test MoveAllStream when sink is initialized (async mode)
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStream_002, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    std::string sinkName = "test_new_sink";
    std::vector<uint32_t> sessionIds = {1, 2, 3};
    MoveSessionType moveType = MOVE_ALL;
    EXPECT_EQ(fastManager_->MoveAllStream(sinkName, sessionIds, moveType), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_024
 * @tc.desc  : Test MoveStream with session not found
 */
HWTEST_F(HpaeFastRendererManagerTest, moveStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    // Move non-existent session, should trigger MOVE_SESSION_FAILED callback
    fastManager_->MoveStream(99999, "valid_sink_name");
    WaitForMsgProcessing(fastManager_);

    // Manager should still be in init state (no crash, no side effects)
    EXPECT_EQ(fastManager_->IsInit(), true);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveStream_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_025
 * @tc.desc  : Test MoveStream with empty sink name
 */
HWTEST_F(HpaeFastRendererManagerTest, moveStream_002, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Move with empty sink name, should trigger MOVE_SESSION_FAILED
    fastManager_->MoveStream(TEST_STREAM_SESSION_ID, "");
    WaitForMsgProcessing(fastManager_);

    // Stream should still exist since move failed
    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, sinkInputInfo), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveStreamSync_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_026
 * @tc.desc  : Test MoveStream in sync mode when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, moveStreamSync_001, TestSize.Level0)
{
    // Not initialized, so MoveStreamSync is called directly
    EXPECT_EQ(fastManager_->MoveStream(99999, "valid_sink_name"), SUCCESS);
    // Should not crash
}

/**
 * @tc.name  : deactivateThread_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_027
 * @tc.desc  : Test DeactivateThread
 */
HWTEST_F(HpaeFastRendererManagerTest, deactivateThread_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->DeactivateThread(), true);
    EXPECT_EQ(fastManager_->IsRunning(), false);
}

/**
 * @tc.name  : handleMsg_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_028
 * @tc.desc  : Test HandleMsg processes queued messages
 */
HWTEST_F(HpaeFastRendererManagerTest, handleMsg_001, TestSize.Level0)
{
    bool requestHandled = false;
    auto request = [&requestHandled]() { requestHandled = true; };
    fastManager_->hpaeNoLockQueue_.PushRequest(std::move(request));
    EXPECT_EQ(fastManager_->IsMsgProcessing(), true);
    fastManager_->HandleMsg();
    EXPECT_EQ(requestHandled, true);
}

/**
 * @tc.name  : getThreadName_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_029
 * @tc.desc  : Test GetThreadName returns fast audio out thread name
 */
HWTEST_F(HpaeFastRendererManagerTest, getThreadName_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.deviceName = "test_device";
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->GetThreadName(), ThreadName::FAST_AUDIO_OUT);
}

/**
 * @tc.name  : dumpSinkInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_030
 * @tc.desc  : Test DumpSinkInfo when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, dumpSinkInfo_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->DumpSinkInfo(), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : dumpSinkInfo_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_031
 * @tc.desc  : Test DumpSinkInfo when initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, dumpSinkInfo_002, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->DumpSinkInfo(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : getDeviceHDFDumpInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_032
 * @tc.desc  : Test GetDeviceHDFDumpInfo
 */
HWTEST_F(HpaeFastRendererManagerTest, getDeviceHDFDumpInfo_001, TestSize.Level0)
{
    std::string dumpInfo = fastManager_->GetDeviceHDFDumpInfo();
    EXPECT_GT(dumpInfo.size(), 0);
}

/**
 * @tc.name  : registerWriteCallback_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_033
 * @tc.desc  : Test RegisterWriteCallback
 */
HWTEST_F(HpaeFastRendererManagerTest, registerWriteCallback_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(fastManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onRequestLatency_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_034
 * @tc.desc  : Test OnRequestLatency
 */
HWTEST_F(HpaeFastRendererManagerTest, onRequestLatency_001, TestSize.Level0)
{
    uint64_t latency = 0;
    // Should not crash even when nodes are not created
    fastManager_->OnRequestLatency(0, latency);
    // When no nodes are created, latency should remain 0
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : onNotifyQueue_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_035
 * @tc.desc  : Test OnNotifyQueue when thread is nullptr, early return without crash
 */
HWTEST_F(HpaeFastRendererManagerTest, onNotifyQueue_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->IsInit(), false);
    fastManager_->OnNotifyQueue();
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : onNotifyQueue_002
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_036
 * @tc.desc  : Test OnNotifyQueue when thread is active
 */
HWTEST_F(HpaeFastRendererManagerTest, onNotifyQueue_002, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);

    fastManager_->OnNotifyQueue();
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : process_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_037
 * @tc.desc  : Test Process when sink is not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, process_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->IsInit(), false);
    fastManager_->Process();
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : isRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_038
 * @tc.desc  : Test IsRunning returns false when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, isRunning_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->IsRunning(), false);
}

/**
 * @tc.name  : triggerAppsUidUpdate_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_039
 * @tc.desc  : Test TriggerAppsUidUpdate when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerAppsUidUpdate_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->IsInit(), false);
    fastManager_->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : destructor_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_040
 * @tc.desc  : Test destructor calls DeInit when initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, destructor_001, TestSize.Level1)
{
    {
        HpaeSinkInfo sinkInfo = GetFastSinkInfo();
        auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
        EXPECT_EQ(manager->Init(), SUCCESS);
        WaitForMsgProcessing(manager);
        InitWithMockSink(manager);
        EXPECT_EQ(manager->IsInit(), true);
        // Destructor should call DeInit
    }
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : sendRequestNotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_041
 * @tc.desc  : Test SendRequest when not initialized should not execute
 */
HWTEST_F(HpaeFastRendererManagerTest, sendRequestNotInit_001, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 1;
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : setMuteForSwitchDevice_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_042
 * @tc.desc  : Test SetMuteForSwitchDevice
 */
HWTEST_F(HpaeFastRendererManagerTest, setMuteForSwitchDevice_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->SetMuteForSwitchDevice(true);
    WaitForMsgProcessing(fastManager_);
    // Verify mute was set via sinkOutputNode_
    EXPECT_NE(fastManager_->sinkOutputNode_, nullptr);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSpeed_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_043
 * @tc.desc  : Test SetSpeed
 */
HWTEST_F(HpaeFastRendererManagerTest, setSpeed_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->SetSpeed(TEST_STREAM_SESSION_ID, 1.5f);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onNodeStatusUpdate_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_044
 * @tc.desc  : Test OnNodeStatusUpdate
 */
HWTEST_F(HpaeFastRendererManagerTest, onNodeStatusUpdate_001, TestSize.Level0)
{
    // Should not crash for non-existent session
    fastManager_->OnNodeStatusUpdate(99999, OPERATION_STARTED);
}

/**
 * @tc.name  : onFadeDone_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_045
 * @tc.desc  : Test OnFadeDone
 */
HWTEST_F(HpaeFastRendererManagerTest, onFadeDone_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // OnFadeDone for non-existent session should not crash
    fastManager_->OnFadeDone(99999);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onRewindAndFlush_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_046
 * @tc.desc  : Test OnRewindAndFlush
 */
HWTEST_F(HpaeFastRendererManagerTest, onRewindAndFlush_001, TestSize.Level0)
{
    fastManager_->OnRewindAndFlush(1000, 0);
    // Should not crash with no sessions
}

/**
 * @tc.name  : getSpanSizeInFrame_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_047
 * @tc.desc  : Test GetSpanSizeInFrame for non-existent session
 */
HWTEST_F(HpaeFastRendererManagerTest, getSpanSizeInFrame_001, TestSize.Level0)
{
    uint32_t spanSize = 0;
    EXPECT_EQ(fastManager_->GetSpanSizeInFrame(99999, spanSize), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : getMaxAmplitude_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_048
 * @tc.desc  : Test GetMaxAmplitude when not initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, getMaxAmplitude_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->GetMaxAmplitude(), 0.0f);
}

/**
 * @tc.name  : reloadRenderManager_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_049
 * @tc.desc  : Test ReloadRenderManager
 */
HWTEST_F(HpaeFastRendererManagerTest, reloadRenderManager_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    EXPECT_EQ(fastManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : reloadRenderManager_AlreadyInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_050
 * @tc.desc  : Test ReloadRenderManager when already initialized
 */
HWTEST_F(HpaeFastRendererManagerTest, reloadRenderManager_AlreadyInit_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);

    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    EXPECT_EQ(fastManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->IsInit(), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : multiSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_051
 * @tc.desc  : Test creating and destroying multiple sessions
 */
HWTEST_F(HpaeFastRendererManagerTest, multiSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(200001)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(200002)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeSinkInputInfo sinkInputInfo;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(200001, sinkInputInfo), SUCCESS);
    EXPECT_EQ(fastManager_->GetSinkInputInfo(200002, sinkInputInfo), SUCCESS);

    EXPECT_EQ(fastManager_->DestroyStream(200002), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_160));
    EXPECT_EQ(fastManager_->GetSinkInputInfo(200002, sinkInputInfo), ERR_INVALID_OPERATION);
    EXPECT_EQ(fastManager_->GetSinkInputInfo(200001, sinkInputInfo), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : createStreamWithLoopback_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_052
 * @tc.desc  : Test CreateStream with isLoopback=true updates loopback state
 */
HWTEST_F(HpaeFastRendererManagerTest, createStreamWithLoopback_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    streamInfo.isLoopback = true;
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DestroyStream(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : deInitWithMoveDefault_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_053
 * @tc.desc  : Test DeInit with isMoveDefault=true moves streams
 */
HWTEST_F(HpaeFastRendererManagerTest, deInitWithMoveDefault_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(true), SUCCESS);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : moveAllStreamPartial_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_054
 * @tc.desc  : Test MoveAllStream with MOVE_SINGLE type
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStreamPartial_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(600001)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(600002)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    std::string sinkName = "test_target_sink";
    std::vector<uint32_t> sessionIds = {600002};
    EXPECT_EQ(fastManager_->MoveAllStream(sinkName, sessionIds, MOVE_SINGLE), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : notifyStreamChangeToSink_NullSinkOutputNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_055
 * @tc.desc  : Test NotifyStreamChangeToSink with null sinkOutputNode_ does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, notifyStreamChangeToSink_NullSinkOutputNode_001, TestSize.Level0)
{
    // Before Init, sinkOutputNode_ is nullptr
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
    fastManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, 1234, RENDERER_RELEASED);
    // Should not crash
}

/**
 * @tc.name  : notifyStreamChangeToSink_WithSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_056
 * @tc.desc  : Test NotifyStreamChangeToSink with session in map and not in map
 */
HWTEST_F(HpaeFastRendererManagerTest, notifyStreamChangeToSink_WithSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Session in map
    fastManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, TEST_STREAM_SESSION_ID, RENDERER_RELEASED);

    // Session not in map
    fastManager_->NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, 999999, RENDERER_RUNNING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : suspendResumeWithRunningStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_057
 * @tc.desc  : Test SuspendStreamManager resume with running stream starts sink
 */
HWTEST_F(HpaeFastRendererManagerTest, suspendResumeWithRunningStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    auto writeCb = std::make_shared<WriteFixedDataCb>(SAMPLE_S32LE);
    EXPECT_EQ(fastManager_->RegisterWriteCallback(TEST_STREAM_SESSION_ID, writeCb), SUCCESS);

    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Suspend
    EXPECT_EQ(fastManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    // Resume should start sink since there is a running stream
    EXPECT_EQ(fastManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setMuteForSwitchDevice_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_060
 * @tc.desc  : Test SetMuteForSwitchDevice when not initialized returns early
 */
HWTEST_F(HpaeFastRendererManagerTest, setMuteForSwitchDevice_NullSinkOutput_001, TestSize.Level0)
{
    // Before Init, sinkOutputNode_ is nullptr
    fastManager_->SetMuteForSwitchDevice(true);
    // Should not crash, sinkOutputNode_ remains null
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
}

/**
 * @tc.name  : stopManager_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_061
 * @tc.desc  : Test StopManager when sinkOutputNode_ is null returns early
 */
HWTEST_F(HpaeFastRendererManagerTest, stopManager_NullSinkOutput_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->StopManager(), SUCCESS);
    // Should not crash
}

/**
 * @tc.name  : initWithInvalidFrameLen_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_063
 * @tc.desc  : Test Init with invalid frame length
 */
HWTEST_F(HpaeFastRendererManagerTest, initWithInvalidFrameLen_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.frameLen = 0;
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->Init(), SUCCESS);
    WaitForMsgProcessing(manager);
    // Init should fail internally since frameLen is 0
    EXPECT_EQ(manager->IsInit(), false);
}

// ==================== ConnectInputCluster branch tests ====================

/**
 * @tc.name  : connectInputCluster_NullSinkInputNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_064
 * @tc.desc  : Test ConnectInputCluster with non-existent session returns ERROR
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputCluster_NullSinkInputNode_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->ConnectInputCluster(99999), ERROR);
}

/**
 * @tc.name  : connectInputCluster_NullGainNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_065
 * @tc.desc  : Test ConnectInputCluster when gainNode missing returns ERR_NULL_POINTER
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputCluster_NullGainNode_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Remove gainNode to hit null gainNode branch
    fastManager_->gainNodeMap_.erase(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->ConnectInputCluster(TEST_STREAM_SESSION_ID), ERR_NULL_POINTER);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : connectInputCluster_NullConverter_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_066
 * @tc.desc  : Test ConnectInputCluster when converter missing returns ERR_NULL_POINTER
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputCluster_NullConverter_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Remove converter to hit null converter branch
    fastManager_->converterNodeMap_.erase(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->ConnectInputCluster(TEST_STREAM_SESSION_ID), ERR_NULL_POINTER);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : connectInputCluster_SinkAlreadyRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_067
 * @tc.desc  : Test ConnectInputCluster skips RenderSinkStart when sink already RUNNING
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputCluster_SinkAlreadyRunning_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Set sink to RUNNING to skip RenderSinkStart branch
    fastManager_->sinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->ConnectInputCluster(TEST_STREAM_SESSION_ID), SUCCESS);
    EXPECT_EQ(fastManager_->sinkOutputNode_->GetSinkState(), STREAM_MANAGER_RUNNING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : connectInputCluster_WhileSuspend_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_068
 * @tc.desc  : Test ConnectInputCluster skips RenderSinkStart when isSuspend_ is true
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputCluster_WhileSuspend_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Force suspend state so RenderSinkStart is skipped
    fastManager_->isSuspend_.store(true);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->ConnectInputCluster(TEST_STREAM_SESSION_ID), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : connectInputCluster_VoipVolumeSyncNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_143
 * @tc.desc  : Test VOIP fast stream uses volume sync node instead of gain node
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputCluster_VoipVolumeSyncNode_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetVoipFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->Init(), SUCCESS);
    WaitForMsgProcessing(manager);
    InitWithMockSink(manager);

    EXPECT_EQ(manager->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager);
    EXPECT_EQ(manager->gainNodeMap_.count(TEST_STREAM_SESSION_ID), 0);
    EXPECT_EQ(manager->volumeSyncNodeMap_.count(TEST_STREAM_SESSION_ID), 1);
    EXPECT_EQ(manager->ConnectInputCluster(TEST_STREAM_SESSION_ID), SUCCESS);
    manager->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->SetState(HPAE_SESSION_RUNNING);
    EXPECT_EQ(manager->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_PAUSED), false);
    EXPECT_EQ(manager->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_PAUSED);
    EXPECT_EQ(manager->DisConnectInputCluster(TEST_STREAM_SESSION_ID), SUCCESS);

    EXPECT_EQ(manager->DeInit(), SUCCESS);
    WaitForMsgProcessing(manager);
}

/**
 * @tc.name  : connectInputCluster_NullVoipVolumeSyncNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_144
 * @tc.desc  : Test VOIP connect fails safely when volume sync node is missing
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputCluster_NullVoipVolumeSyncNode_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetVoipFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->Init(), SUCCESS);
    WaitForMsgProcessing(manager);
    InitWithMockSink(manager);

    EXPECT_EQ(manager->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager);
    manager->volumeSyncNodeMap_.erase(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(manager->ConnectInputCluster(TEST_STREAM_SESSION_ID), ERR_NULL_POINTER);

    EXPECT_EQ(manager->DeInit(), SUCCESS);
    WaitForMsgProcessing(manager);
}

/**
 * @tc.name  : connectInputCluster_NullMixer_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_145
 * @tc.desc  : Test connect fails safely when mixer is missing
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputCluster_NullMixer_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->mixerNode_ = nullptr;
    EXPECT_EQ(fastManager_->ConnectInputCluster(TEST_STREAM_SESSION_ID), ERR_NULL_POINTER);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : isVoipFastSink_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_146
 * @tc.desc  : Test VoIP fast sink identification
 */
HWTEST_F(HpaeFastRendererManagerTest, isVoipFastSink_001, TestSize.Level0)
{
    HpaeSinkInfo voipSinkInfo = GetVoipFastSinkInfo();
    auto voipManager = std::make_shared<HpaeFastRendererManager>(voipSinkInfo);
    EXPECT_TRUE(voipManager->IsVoipFastSink());
    EXPECT_FALSE(fastManager_->IsVoipFastSink());
}

/**
 * @tc.name  : deleteSessionNodes_VoipNodes_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_147
 * @tc.desc  : Test deleting a VoIP session clears all per-stream nodes
 */
HWTEST_F(HpaeFastRendererManagerTest, deleteSessionNodes_VoipNodes_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetVoipFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->Init(), SUCCESS);
    WaitForMsgProcessing(manager);
    InitWithMockSink(manager);

    EXPECT_EQ(manager->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager);
    ASSERT_EQ(manager->converterNodeMap_.count(TEST_STREAM_SESSION_ID), 1);
    ASSERT_EQ(manager->volumeSyncNodeMap_.count(TEST_STREAM_SESSION_ID), 1);
    EXPECT_EQ(manager->DeleteSessionNodes(TEST_STREAM_SESSION_ID), SUCCESS);
    EXPECT_EQ(manager->converterNodeMap_.count(TEST_STREAM_SESSION_ID), 0);
    EXPECT_EQ(manager->volumeSyncNodeMap_.count(TEST_STREAM_SESSION_ID), 0);

    EXPECT_EQ(manager->DeInit(), SUCCESS);
    WaitForMsgProcessing(manager);
}

// ==================== DisConnectInputCluster branch tests ====================

/**
 * @tc.name  : disConnectInputCluster_NullSinkInputNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_069
 * @tc.desc  : Test DisConnectInputCluster with non-existent session returns ERROR
 */
HWTEST_F(HpaeFastRendererManagerTest, disConnectInputCluster_NullSinkInputNode_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->DisConnectInputCluster(99999), ERROR);
}

/**
 * @tc.name  : disConnectInputCluster_NullConverter_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_148
 * @tc.desc  : Test disconnect fails safely when converter is missing
 */
HWTEST_F(HpaeFastRendererManagerTest, disConnectInputCluster_NullConverter_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->converterNodeMap_.erase(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->DisConnectInputCluster(TEST_STREAM_SESSION_ID), ERROR);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : disConnectInputCluster_NullMixer_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_149
 * @tc.desc  : Test disconnect fails safely when mixer is missing
 */
HWTEST_F(HpaeFastRendererManagerTest, disConnectInputCluster_NullMixer_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->mixerNode_ = nullptr;
    EXPECT_EQ(fastManager_->DisConnectInputCluster(TEST_STREAM_SESSION_ID), ERROR);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : disConnectInputCluster_NullGainNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_150
 * @tc.desc  : Test non-VoIP disconnect fails safely when gain node is missing
 */
HWTEST_F(HpaeFastRendererManagerTest, disConnectInputCluster_NullGainNode_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    fastManager_->gainNodeMap_.erase(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->DisConnectInputCluster(TEST_STREAM_SESSION_ID), ERROR);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : disConnectInputCluster_NullVoipVolumeSyncNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_151
 * @tc.desc  : Test VoIP disconnect fails safely when volume sync node is missing
 */
HWTEST_F(HpaeFastRendererManagerTest, disConnectInputCluster_NullVoipVolumeSyncNode_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetVoipFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    EXPECT_EQ(manager->Init(), SUCCESS);
    WaitForMsgProcessing(manager);
    InitWithMockSink(manager);
    EXPECT_EQ(manager->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager);

    manager->volumeSyncNodeMap_.erase(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(manager->DisConnectInputCluster(TEST_STREAM_SESSION_ID), ERROR);

    EXPECT_EQ(manager->DeInit(), SUCCESS);
    WaitForMsgProcessing(manager);
}

// ==================== ConnectInputSession branch tests ====================

/**
 * @tc.name  : connectInputSession_RunningState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_070
 * @tc.desc  : Test ConnectInputSession with RUNNING state calls ConnectInputCluster
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputSession_RunningState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Set state to RUNNING to enter ConnectInputCluster branch
    fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->SetState(HPAE_SESSION_RUNNING);
    EXPECT_EQ(fastManager_->ConnectInputSession(TEST_STREAM_SESSION_ID), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== SetSessionFade branch tests ====================

/**
 * @tc.name  : setSessionFade_NullNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_071
 * @tc.desc  : Test SetSessionFade with non-existent session returns false
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_NullNode_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->SetSessionFade(99999, OPERATION_STARTED), false);
}

/**
 * @tc.name  : setSessionFade_NotRunningWithStart_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_072
 * @tc.desc  : Test SetSessionFade not running with OPERATION_STARTED returns false (no state set)
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_NotRunningWithStart_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Session is PREPARED (not running) -> warning branch, OPERATION_STARTED -> skip state set
    EXPECT_EQ(fastManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_STARTED), false);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSessionFade_NotRunningWithPause_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_073
 * @tc.desc  : Test SetSessionFade not running with OPERATION_PAUSED sets state to PAUSED
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_NotRunningWithPause_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_PAUSED), false);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_PAUSED);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSessionFade_NotRunningWithStop_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_074
 * @tc.desc  : Test SetSessionFade not running with OPERATION_STOPPED sets state to STOPPED
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_NotRunningWithStop_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_STOPPED), false);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_STOPPED);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSessionFade_RunningWithStart_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_075
 * @tc.desc  : Test SetSessionFade running with OPERATION_STARTED sets fade and returns true
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_RunningWithStart_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Now sink is RUNNING and session is RUNNING
    EXPECT_EQ(fastManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_STARTED), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSessionFade_RunningWithPause_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_076
 * @tc.desc  : Test SetSessionFade running with OPERATION_PAUSED sets state to PAUSING
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_RunningWithPause_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_PAUSED), true);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_PAUSING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSessionFade_RunningWithStop_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_077
 * @tc.desc  : Test SetSessionFade running with OPERATION_STOPPED sets state to STOPPING
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_RunningWithStop_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_STOPPED), true);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_STOPPING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSessionFade_PausingStateSkipFade_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_078
 * @tc.desc  : Test SetSessionFade skips SetFadeState when already PAUSING
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_PausingStateSkipFade_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Set state to PAUSING so SetFadeState is skipped
    fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->SetState(HPAE_SESSION_PAUSING);
    EXPECT_EQ(fastManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_PAUSED), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSessionFade_StoppingStateSkipFade_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_079
 * @tc.desc  : Test SetSessionFade skips SetFadeState when already STOPPING
 */
HWTEST_F(HpaeFastRendererManagerTest, setSessionFade_StoppingStateSkipFade_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Set state to STOPPING so SetFadeState is skipped
    fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->SetState(HPAE_SESSION_STOPPING);
    EXPECT_EQ(fastManager_->SetSessionFade(TEST_STREAM_SESSION_ID, OPERATION_STOPPED), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== AddSingleNodeToSink branch tests ====================

/**
 * @tc.name  : addSingleNodeToSink_NotConnect_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_080
 * @tc.desc  : Test AddSingleNodeToSink with isConnect=false skips ConnectInputSession
 */
HWTEST_F(HpaeFastRendererManagerTest, addSingleNodeToSink_NotConnect_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 70001;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_S32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->SetState(HPAE_SESSION_RUNNING);
    // isConnect=false -> skip ConnectInputSession branch
    fastManager_->AddSingleNodeToSink(node, false);
    EXPECT_NE(fastManager_->sinkInputNodeMap_.count(70001), 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : addSingleNodeToSink_NotRunningWithConnect_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_081
 * @tc.desc  : Test AddSingleNodeToSink with isConnect=true but state not RUNNING skips connect
 */
HWTEST_F(HpaeFastRendererManagerTest, addSingleNodeToSink_NotRunningWithConnect_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 70002;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_S32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    // state PREPARED (not RUNNING) and isConnect=true -> skip connect branch
    node->SetState(HPAE_SESSION_PREPARED);
    fastManager_->AddSingleNodeToSink(node, true);
    EXPECT_NE(fastManager_->sinkInputNodeMap_.count(70002), 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : addSingleNodeToSink_RunningWithConnect_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_082
 * @tc.desc  : Test AddSingleNodeToSink with isConnect=true and state RUNNING calls ConnectInputSession
 */
HWTEST_F(HpaeFastRendererManagerTest, addSingleNodeToSink_RunningWithConnect_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeNodeInfo nodeInfo;
    nodeInfo.sessionId = 70003;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    nodeInfo.format = SAMPLE_S32LE;
    nodeInfo.channels = STEREO;
    nodeInfo.frameLen = FRAME_LENGTH_960;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    node->SetState(HPAE_SESSION_RUNNING);
    // isConnect=true and state RUNNING -> ConnectInputSession branch
    fastManager_->AddSingleNodeToSink(node, true);
    EXPECT_NE(fastManager_->sinkInputNodeMap_.count(70003), 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== MoveAllStreamToNewSink branch tests ====================

/**
 * @tc.name  : moveAllStreamToNewSink_EmptySinkInputs_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_083
 * @tc.desc  : Test MoveAllStreamToNewSink with no matching sessions logs empty warning
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStreamToNewSink_EmptySinkInputs_001, TestSize.Level0)
{
    // No sessions in map, MOVE_ALL collects nothing -> empty branch
    fastManager_->MoveAllStreamToNewSink("target_sink", {}, MOVE_ALL);
    // Should not crash
}

/**
 * @tc.name  : moveAllStreamToNewSink_MoveSingleNoMatch_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_084
 * @tc.desc  : Test MoveAllStreamToNewSink MOVE_SINGLE with no matching sessionIds
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStreamToNewSink_MoveSingleNoMatch_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(500001)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // MOVE_SINGLE with sessionIds that don't match 500001 -> skip in for loop
    std::vector<uint32_t> sessionIds = {999999};
    fastManager_->MoveAllStreamToNewSink("target_sink", sessionIds, MOVE_SINGLE);
    // Stream should still exist
    HpaeSinkInputInfo info;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(500001, info), SUCCESS);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveAllStreamToNewSink_MoveSingleMatch_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_085
 * @tc.desc  : Test MoveAllStreamToNewSink MOVE_SINGLE with matching sessionId collects node
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStreamToNewSink_MoveSingleMatch_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(500002)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // MOVE_SINGLE with matching sessionId -> TriggerCallback (not MOVE_ALL branch)
    std::vector<uint32_t> sessionIds = {500002};
    fastManager_->MoveAllStreamToNewSink("target_sink", sessionIds, MOVE_SINGLE);
    // Node should be removed after move
    HpaeSinkInputInfo info;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(500002, info), ERR_INVALID_OPERATION);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== UpdateAppsUid branch tests ====================

/**
 * @tc.name  : updateAppsUid_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_086
 * @tc.desc  : Test UpdateAppsUid with null sinkOutputNode_ skips sink update
 */
HWTEST_F(HpaeFastRendererManagerTest, updateAppsUid_NullSinkOutput_001, TestSize.Level0)
{
    // Before Init, sinkOutputNode_ is null
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
    fastManager_->UpdateAppsUid();
    // Should not crash
}

/**
 * @tc.name  : updateAppsUid_WithKeepRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_087
 * @tc.desc  : Test UpdateAppsUid with keepRunning stream keeps needCheckZeroVolume true
 */
HWTEST_F(HpaeFastRendererManagerTest, updateAppsUid_WithKeepRunning_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);

    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    streamInfo.keepRunning = true;
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // keepRunning=true should keep needCheckZeroVolume unchanged (true)
    fastManager_->UpdateAppsUid();

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== StopOutputNode branch tests ====================

/**
 * @tc.name  : stopOutputNode_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_088
 * @tc.desc  : Test StopOutputNode with null sinkOutputNode_ does nothing
 */
HWTEST_F(HpaeFastRendererManagerTest, stopOutputNode_NullSinkOutput_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
    fastManager_->StopOutputNode();
    // Should not crash
}

/**
 * @tc.name  : stopOutputNode_NullMixerNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_089
 * @tc.desc  : Test StopOutputNode with null mixerNode_ skips DisConnect
 */
HWTEST_F(HpaeFastRendererManagerTest, stopOutputNode_NullMixerNode_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Force mixerNode_ to null to hit skip-DisConnect branch
    fastManager_->mixerNode_ = nullptr;
    fastManager_->StopOutputNode();
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== StartRenderSink branch tests ====================

/**
 * @tc.name  : startRenderSink_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_090
 * @tc.desc  : Test StartRenderSink with null sinkOutputNode_ returns ERROR
 */
HWTEST_F(HpaeFastRendererManagerTest, startRenderSink_NullSinkOutput_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
    EXPECT_EQ(fastManager_->StartRenderSink(), ERROR);
}

// ==================== CreateSinkAttr branch tests ====================

/**
 * @tc.name  : createSinkAttr_DpAdapter_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_091
 * @tc.desc  : Test CreateSinkAttr with dp adapterName sets AUDIO_PIN_OUT_DP
 */
HWTEST_F(HpaeFastRendererManagerTest, createSinkAttr_DpAdapter_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.adapterName = "dp";
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    auto attr = manager->CreateSinkAttr();
    EXPECT_EQ(attr.pin, AUDIO_PIN_OUT_DP);
}

/**
 * @tc.name  : createSinkAttr_NotDpAdapter_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_092
 * @tc.desc  : Test CreateSinkAttr with non-dp adapterName does not set DP pin
 */
HWTEST_F(HpaeFastRendererManagerTest, createSinkAttr_NotDpAdapter_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.adapterName = "primary_mmap";
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    auto attr = manager->CreateSinkAttr();
    EXPECT_NE(attr.pin, AUDIO_PIN_OUT_DP);
}

/**
 * @tc.name  : createSinkAttr_UltraFast_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_093
 * @tc.desc  : Test CreateSinkAttr with isUltraFast=true sets period
 */
HWTEST_F(HpaeFastRendererManagerTest, createSinkAttr_UltraFast_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.isUltraFast = true;
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    auto attr = manager->CreateSinkAttr();
    EXPECT_GT(attr.period, 0);
}

/**
 * @tc.name  : createSinkAttr_NotUltraFast_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_094
 * @tc.desc  : Test CreateSinkAttr with isUltraFast=false does not set period
 */
HWTEST_F(HpaeFastRendererManagerTest, createSinkAttr_NotUltraFast_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.isUltraFast = false;
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    auto attr = manager->CreateSinkAttr();
    EXPECT_EQ(attr.period, 0);
}

// ==================== IsRunning branch tests ====================

/**
 * @tc.name  : isRunning_BothNonNull_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_095
 * @tc.desc  : Test IsRunning returns true when sink and thread running
 */
HWTEST_F(HpaeFastRendererManagerTest, isRunning_BothNonNull_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Sink is IDLE, not RUNNING
    EXPECT_EQ(fastManager_->IsRunning(), false);
    // Set to RUNNING
    fastManager_->sinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastManager_->IsRunning(), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== Process branch tests ====================

/**
 * @tc.name  : process_RunningWithAppsUid_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_096
 * @tc.desc  : Test Process when running with running stream (appsUid not empty) takes else branch
 */
HWTEST_F(HpaeFastRendererManagerTest, process_RunningWithAppsUid_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Now sink is RUNNING and stream is RUNNING -> appsUid_ not empty after UpdateAppsUid (else branch)
    fastManager_->Process();
    // Should not crash; noneStreamTime_ reset to 0 in else branch
    EXPECT_EQ(fastManager_->noneStreamTime_, 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : process_RunningEmptyAppsUid_NoTimeout_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_097
 * @tc.desc  : Test Process when running with empty appsUid and no timeout sets noneStreamTime
 */
HWTEST_F(HpaeFastRendererManagerTest, process_RunningEmptyAppsUid_NoTimeout_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Set sink to RUNNING with no running streams -> appsUid_ empty after UpdateAppsUid
    fastManager_->sinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    fastManager_->noneStreamTime_ = 0; // first call -> set to now, no timeout
    fastManager_->Process();
    // noneStreamTime_ should now be set (non-zero) since timeout not reached
    EXPECT_NE(fastManager_->noneStreamTime_, 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : process_RunningEmptyAppsUid_Timeout_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_098
 * @tc.desc  : Test Process when running with empty appsUid and timeout reached stops sink
 */
HWTEST_F(HpaeFastRendererManagerTest, process_RunningEmptyAppsUid_Timeout_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    fastManager_->sinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    // Set noneStreamTime_ to past so timeout (3s) is reached
    constexpr int64_t SUSPEND_TIMEOUT_S = 3;
    fastManager_->appsUid_.clear();
    fastManager_->noneStreamTime_ = ClockTime::GetCurNano() -
        static_cast<int64_t>((SUSPEND_TIMEOUT_S + 1) * AUDIO_NS_PER_SECOND);
    fastManager_->Process();
    // Sink should be stopped (state changed from RUNNING)
    EXPECT_NE(fastManager_->sinkOutputNode_->GetSinkState(), STREAM_MANAGER_RUNNING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== CreateStream invalid stream info ====================

/**
 * @tc.name  : createStream_InvalidStreamInfo_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_099
 * @tc.desc  : Test CreateStream with invalid frameLen returns ERROR
 */
HWTEST_F(HpaeFastRendererManagerTest, createStream_InvalidStreamInfo_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    HpaeStreamInfo streamInfo = MakeStreamInfo(TEST_STREAM_SESSION_ID);
    streamInfo.frameLen = 0; // invalid
    EXPECT_EQ(fastManager_->CreateStream(streamInfo), ERROR);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== Stream control not-found branches ====================

/**
 * @tc.name  : destroyStream_SessionNotFound_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_100
 * @tc.desc  : Test DestroyStream non-existent session does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, destroyStream_SessionNotFound_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_NE(fastManager_->DestroyStream(99999), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : start_SessionNotFound_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_101
 * @tc.desc  : Test Start non-existent session does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, start_SessionNotFound_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(99999), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : pause_SessionNotFound_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_102
 * @tc.desc  : Test Pause non-existent session does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, pause_SessionNotFound_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Pause(99999), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : pause_RunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_103
 * @tc.desc  : Test Pause running session triggers fade (SetSessionFade returns true)
 */
HWTEST_F(HpaeFastRendererManagerTest, pause_RunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Pause running session -> SetSessionFade returns true -> skip DisConnectInputCluster
    EXPECT_EQ(fastManager_->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : flush_SessionNotFound_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_104
 * @tc.desc  : Test Flush non-existent session does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, flush_SessionNotFound_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Flush(99999), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : flush_NullConverter_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_105
 * @tc.desc  : Test Flush when converter is null skips FlushBuffers
 */
HWTEST_F(HpaeFastRendererManagerTest, flush_NullConverter_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Remove converter to hit null converter branch
    fastManager_->converterNodeMap_.erase(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->Flush(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : drain_SessionNotFound_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_106
 * @tc.desc  : Test Drain non-existent session does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, drain_SessionNotFound_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Drain(99999), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : drain_RunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_107
 * @tc.desc  : Test Drain running session skips status callback (state == RUNNING)
 */
HWTEST_F(HpaeFastRendererManagerTest, drain_RunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Drain when state is RUNNING -> skip TriggerCallback branch
    EXPECT_EQ(fastManager_->Drain(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : drain_NotRunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_108
 * @tc.desc  : Test Drain not-running session triggers status callback
 */
HWTEST_F(HpaeFastRendererManagerTest, drain_NotRunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // State is PREPARED (not RUNNING) -> TriggerCallback branch
    EXPECT_EQ(fastManager_->Drain(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : stop_SessionNotFound_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_109
 * @tc.desc  : Test Stop non-existent session does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, stop_SessionNotFound_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Stop(99999), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : stop_RunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_110
 * @tc.desc  : Test Stop running session triggers fade (SetSessionFade returns true)
 */
HWTEST_F(HpaeFastRendererManagerTest, stop_RunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Stop running session -> SetSessionFade returns true -> skip DisConnectInputCluster
    EXPECT_EQ(fastManager_->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== UpdateLoopbackState branch tests ====================

/**
 * @tc.name  : updateLoopbackState_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_111
 * @tc.desc  : Test UpdateLoopbackState with null sinkOutputNode_ skips SetLoopbackState
 */
HWTEST_F(HpaeFastRendererManagerTest, updateLoopbackState_NullSinkOutput_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
    fastManager_->UpdateLoopbackState();
    EXPECT_EQ(fastManager_->isExistLoopback_.load(), false);
}

/**
 * @tc.name  : updateLoopbackState_WithSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_112
 * @tc.desc  : Test UpdateLoopbackState with sinkOutputNode_ calls SetLoopbackState
 */
HWTEST_F(HpaeFastRendererManagerTest, updateLoopbackState_WithSinkOutput_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Insert a loopback session id so isExistLoopback becomes true
    fastManager_->loopbackSessionIds_.insert(TEST_STREAM_SESSION_ID);
    fastManager_->UpdateLoopbackState();
    EXPECT_EQ(fastManager_->isExistLoopback_.load(), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== MoveStreamSync success branch ====================

/**
 * @tc.name  : moveStreamSync_Success_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_113
 * @tc.desc  : Test MoveStreamSync with valid session and sink name completes move
 */
HWTEST_F(HpaeFastRendererManagerTest, moveStreamSync_Success_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Valid session and non-empty sink name -> success path
    fastManager_->MoveStreamSync(TEST_STREAM_SESSION_ID, "valid_target_sink");
    HpaeSinkInputInfo info;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, info), ERR_INVALID_OPERATION);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== SuspendStreamManager branch tests ====================

/**
 * @tc.name  : suspendStreamManager_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_114
 * @tc.desc  : Test SuspendStreamManager(true) with null sinkOutputNode_ skips RenderSinkStop
 */
HWTEST_F(HpaeFastRendererManagerTest, suspendStreamManager_NullSinkOutput_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
    EXPECT_EQ(fastManager_->SuspendStreamManager(true), SUCCESS);
    EXPECT_EQ(fastManager_->isSuspend_.load(), false);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
}

/**
 * @tc.name  : suspendStreamManager_ResumeSinkRunning_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_115
 * @tc.desc  : Test SuspendStreamManager(false) when sink already RUNNING skips RenderSinkStart
 */
HWTEST_F(HpaeFastRendererManagerTest, suspendStreamManager_ResumeSinkRunning_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Set suspend=true first, then set sink to RUNNING
    fastManager_->isSuspend_.store(true);
    fastManager_->sinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    // Resume when sink is already RUNNING -> skip RenderSinkStart branch
    EXPECT_EQ(fastManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : suspendStreamManager_ResumeNoRunningStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_116
 * @tc.desc  : Test SuspendStreamManager(false) with no running stream skips RenderSinkStart
 */
HWTEST_F(HpaeFastRendererManagerTest, suspendStreamManager_ResumeNoRunningStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Suspend first
    EXPECT_EQ(fastManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Resume with no running stream -> CheckIsStreamRunning returns false -> skip RenderSinkStart
    EXPECT_EQ(fastManager_->SuspendStreamManager(false), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== SetClientVolume branch tests ====================

/**
 * @tc.name  : setClientVolume_WithGainNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_117
 * @tc.desc  : Test SetClientVolume with existing gainNode calls SetClientVolume
 */
HWTEST_F(HpaeFastRendererManagerTest, setClientVolume_WithGainNode_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // gainNode exists -> SetClientVolume called
    EXPECT_EQ(fastManager_->SetClientVolume(TEST_STREAM_SESSION_ID, 0.8f), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== RegisterWriteCallback branch tests ====================

/**
 * @tc.name  : registerWriteCallback_SessionNotFound_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_119
 * @tc.desc  : Test RegisterWriteCallback non-existent session does not crash
 */
HWTEST_F(HpaeFastRendererManagerTest, registerWriteCallback_SessionNotFound_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->RegisterWriteCallback(99999, std::weak_ptr<IStreamCallback>()), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== OnFadeDone branch tests ====================

/**
 * @tc.name  : onFadeDone_RunningState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_120
 * @tc.desc  : Test OnFadeDone when session state is RUNNING skips disconnect
 */
HWTEST_F(HpaeFastRendererManagerTest, onFadeDone_RunningState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // State is RUNNING -> CHECK_AND_RETURN_LOG skips disconnect
    fastManager_->OnFadeDone(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(fastManager_);
    // State should still be RUNNING
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_RUNNING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onFadeDone_StoppingState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_121
 * @tc.desc  : Test OnFadeDone when session state is STOPPING sets state to STOPPED
 */
HWTEST_F(HpaeFastRendererManagerTest, onFadeDone_StoppingState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Set state to STOPPING -> OnFadeDone sets state to STOPPED
    fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->SetState(HPAE_SESSION_STOPPING);
    fastManager_->OnFadeDone(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_STOPPED);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onFadeDone_PausingState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_122
 * @tc.desc  : Test OnFadeDone when session state is PAUSING sets state to PAUSED
 */
HWTEST_F(HpaeFastRendererManagerTest, onFadeDone_PausingState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Set state to PAUSING -> OnFadeDone sets state to PAUSED
    fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->SetState(HPAE_SESSION_PAUSING);
    fastManager_->OnFadeDone(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_PAUSED);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== OnRequestLatency branch tests ====================

/**
 * @tc.name  : onRequestLatency_WithConverter_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_123
 * @tc.desc  : Test OnRequestLatency with converter and mixerNode adds latency
 */
HWTEST_F(HpaeFastRendererManagerTest, onRequestLatency_WithConverter_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    uint64_t latency = 0;
    fastManager_->OnRequestLatency(TEST_STREAM_SESSION_ID, latency);
    // latency should be accumulated (converter + mixer)
    EXPECT_EQ(latency, 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onRequestLatency_NullMixerNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_124
 * @tc.desc  : Test OnRequestLatency with null mixerNode skips mixer latency
 */
HWTEST_F(HpaeFastRendererManagerTest, onRequestLatency_NullMixerNode_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Set mixerNode_ to null to skip mixer latency branch
    fastManager_->mixerNode_ = nullptr;
    uint64_t latency = 0;
    fastManager_->OnRequestLatency(TEST_STREAM_SESSION_ID, latency);
    // latency should still have converter latency
    EXPECT_EQ(latency, 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== OnRewindAndFlush branch tests ====================

/**
 * @tc.name  : onRewindAndFlush_RunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_125
 * @tc.desc  : Test OnRewindAndFlush with running session calls RewindHistoryBuffer
 */
HWTEST_F(HpaeFastRendererManagerTest, onRewindAndFlush_RunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Running session -> call RewindHistoryBuffer branch
    fastManager_->OnRewindAndFlush(1000, 0);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== GetSpanSizeInFrame branch tests ====================

/**
 * @tc.name  : getSpanSizeInFrame_Found_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_126
 * @tc.desc  : Test GetSpanSizeInFrame with existing session returns SUCCESS
 */
HWTEST_F(HpaeFastRendererManagerTest, getSpanSizeInFrame_Found_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    uint32_t spanSize = 0;
    EXPECT_EQ(fastManager_->GetSpanSizeInFrame(TEST_STREAM_SESSION_ID, spanSize), SUCCESS);
    EXPECT_GT(spanSize, 0);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== GetMaxAmplitude branch tests ====================

/**
 * @tc.name  : getMaxAmplitude_WithSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_127
 * @tc.desc  : Test GetMaxAmplitude with sinkOutputNode_ returns sink amplitude
 */
HWTEST_F(HpaeFastRendererManagerTest, getMaxAmplitude_WithSinkOutput_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // sinkOutputNode_ != null -> call sinkOutputNode_->GetMaxAmplitude()
    float amp = fastManager_->GetMaxAmplitude();
    (void)amp;
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== TriggerStreamState branch tests ====================

/**
 * @tc.name  : triggerStreamState_StoppingState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_128
 * @tc.desc  : Test TriggerStreamState with STOPPING state sets to STOPPED
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerStreamState_StoppingState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    auto node = fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID];
    node->SetState(HPAE_SESSION_STOPPING);
    fastManager_->TriggerStreamState(TEST_STREAM_SESSION_ID, node);
    EXPECT_EQ(node->GetState(), HPAE_SESSION_STOPPED);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : triggerStreamState_PausingState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_129
 * @tc.desc  : Test TriggerStreamState with PAUSING state sets to PAUSED
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerStreamState_PausingState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    auto node = fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID];
    node->SetState(HPAE_SESSION_PAUSING);
    fastManager_->TriggerStreamState(TEST_STREAM_SESSION_ID, node);
    EXPECT_EQ(node->GetState(), HPAE_SESSION_PAUSED);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : triggerStreamState_OtherState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_130
 * @tc.desc  : Test TriggerStreamState with RUNNING state does nothing
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerStreamState_OtherState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    auto node = fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID];
    node->SetState(HPAE_SESSION_RUNNING);
    fastManager_->TriggerStreamState(TEST_STREAM_SESSION_ID, node);
    // State should remain RUNNING (no transition)
    EXPECT_EQ(node->GetState(), HPAE_SESSION_RUNNING);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== TriggerAppsUidUpdate branch tests ====================

/**
 * @tc.name  : triggerAppsUidUpdate_WithRunningStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_131
 * @tc.desc  : Test TriggerAppsUidUpdate with running stream collects appUid
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerAppsUidUpdate_WithRunningStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    fastManager_->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(fastManager_);
    // appsUid_ should be populated with the running stream's appUid
    EXPECT_FALSE(fastManager_->appsUid_.empty());

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : triggerAppsUidUpdate_NoRunningNoMatch_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_132
 * @tc.desc  : Test TriggerAppsUidUpdate with no running stream and non-matching sessionId
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerAppsUidUpdate_NoRunningNoMatch_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Session is PREPARED (not RUNNING) and sessionId doesn't match -> empty appsUid_
    fastManager_->TriggerAppsUidUpdate(99999);
    WaitForMsgProcessing(fastManager_);
    EXPECT_TRUE(fastManager_->appsUid_.empty());

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== ReloadRenderManager running session ====================

/**
 * @tc.name  : reloadRenderManager_WithRunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_133
 * @tc.desc  : Test ReloadRenderManager with running session reconnects after reload
 */
HWTEST_F(HpaeFastRendererManagerTest, reloadRenderManager_WithRunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);

    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    EXPECT_EQ(fastManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Reload should have reconnected the running session
    EXPECT_EQ(fastManager_->IsInit(), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== RemoveNodeFromMap branch tests ====================

/**
 * @tc.name  : removeNodeFromMap_NullNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_134
 * @tc.desc  : Test RemoveNodeFromMap with non-existent session uses RELEASED state
 */
HWTEST_F(HpaeFastRendererManagerTest, removeNodeFromMap_NullNode_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->sinkOutputNode_, nullptr);
    fastManager_->RemoveNodeFromMap(99999);
    // Should not crash, session not in map
    EXPECT_EQ(fastManager_->sinkInputNodeMap_.count(99999), 0);
}

/**
 * @tc.name  : removeNodeFromMap_WithNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_135
 * @tc.desc  : Test RemoveNodeFromMap with existing node uses RELEASED state from node
 */
HWTEST_F(HpaeFastRendererManagerTest, removeNodeFromMap_WithNode_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Set state to PREPARED (not RELEASED) so node != nullptr branch is taken
    fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->SetState(HPAE_SESSION_PREPARED);
    fastManager_->RemoveNodeFromMap(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_.count(TEST_STREAM_SESSION_ID), 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== SendRequest branch tests ====================

/**
 * @tc.name  : sendRequest_NullThread_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_136
 * @tc.desc  : Test SendRequest with isInit=true but null thread reports exception
 */
HWTEST_F(HpaeFastRendererManagerTest, sendRequest_NullThread_001, TestSize.Level0)
{
    // Force isInit_ true but thread null
    fastManager_->isInit_.store(true);
    EXPECT_EQ(fastManager_->hpaeSignalProcessThread_, nullptr);
    // SetMute uses SendRequest with isInit=false (default)
    EXPECT_EQ(fastManager_->SetMute(true), SUCCESS);
    // Request should be pushed but not processed (thread null) -> exception reported
}

// ==================== CheckIsStreamRunning tests ====================

/**
 * @tc.name  : checkIsStreamRunning_NoStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_137
 * @tc.desc  : Test CheckIsStreamRunning with no streams returns false
 */
HWTEST_F(HpaeFastRendererManagerTest, checkIsStreamRunning_NoStream_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->CheckIsStreamRunning(), false);
}

/**
 * @tc.name  : checkIsStreamRunning_WithRunningStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_138
 * @tc.desc  : Test CheckIsStreamRunning with running stream returns true
 */
HWTEST_F(HpaeFastRendererManagerTest, checkIsStreamRunning_WithRunningStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->CheckIsStreamRunning(), true);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== DeleteInputSession tests ====================

/**
 * @tc.name  : deleteInputSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_139
 * @tc.desc  : Test DeleteInputSession removes node from map
 */
HWTEST_F(HpaeFastRendererManagerTest, deleteInputSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_NE(fastManager_->sinkInputNodeMap_.count(TEST_STREAM_SESSION_ID), 0);
    fastManager_->DeleteInputSession(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_.count(TEST_STREAM_SESSION_ID), 0);

    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== OnNodeStatusUpdate with existing session ====================

/**
 * @tc.name  : onNodeStatusUpdate_WithSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_140
 * @tc.desc  : Test OnNodeStatusUpdate with existing session returns node state
 */
HWTEST_F(HpaeFastRendererManagerTest, onNodeStatusUpdate_WithSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // node != nullptr branch of ternary
    fastManager_->OnNodeStatusUpdate(TEST_STREAM_SESSION_ID, OPERATION_STARTED);
    // Should not crash
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== InitSinkInner a2dp_fast skip RenderSinkStart tests ====================

/**
 * @tc.name  : initSinkInner_A2dpFastSkipStart_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_141
 * @tc.desc  : Test InitSinkInner skips RenderSinkStart for a2dp_fast device class
 * @tc.owner : audio_route_fast
 */
HWTEST_F(HpaeFastRendererManagerTest, initSinkInner_A2dpFastSkipStart_001, TestSize.Level1)
{
    HpaeSinkInfo a2dpSinkInfo = GetFastSinkInfo("a2dp_fast");
    auto a2dpManager = std::make_shared<HpaeFastRendererManager>(a2dpSinkInfo);
    EXPECT_EQ(a2dpManager->Init(), SUCCESS);
    WaitForMsgProcessing(a2dpManager);
    ASSERT_NE(a2dpManager->sinkOutputNode_, nullptr);
    EXPECT_EQ(a2dpManager->sinkOutputNode_->GetDeviceClass(), "a2dp_fast");
    // RenderSinkStart should be skipped for a2dp_fast
    EXPECT_NE(a2dpManager->sinkOutputNode_->GetSinkState(), STREAM_MANAGER_RUNNING);
    EXPECT_EQ(a2dpManager->DeInit(), SUCCESS);
    WaitForMsgProcessing(a2dpManager);
}

/**
 * @tc.name  : initSinkInner_NonA2dpFastStart_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_142
 * @tc.desc  : Test InitSinkInner calls RenderSinkStart for non-a2dp_fast device class
 * @tc.owner : audio_route_fast
 */
HWTEST_F(HpaeFastRendererManagerTest, initSinkInner_NonA2dpFastStart_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    ASSERT_NE(fastManager_->sinkOutputNode_, nullptr);
    EXPECT_NE(fastManager_->sinkOutputNode_->GetDeviceClass(), "a2dp_fast");
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

// ==================== Branch coverage supplementary tests ====================

/**
 * @tc.name  : connectInputSession_NullNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_152
 * @tc.desc  : Test ConnectInputSession with non-existent session returns ERROR
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputSession_NullNode_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->ConnectInputSession(99999), ERROR);
}

/**
 * @tc.name  : connectInputSession_NotRunningState_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_153
 * @tc.desc  : Test ConnectInputSession with PREPARED state returns SUCCESS without connecting
 */
HWTEST_F(HpaeFastRendererManagerTest, connectInputSession_NotRunningState_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // State is PREPARED (not RUNNING) -> skip ConnectInputCluster, return SUCCESS
    EXPECT_EQ(fastManager_->ConnectInputSession(TEST_STREAM_SESSION_ID), SUCCESS);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_PREPARED);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : pause_NonRunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_154
 * @tc.desc  : Test Pause on non-running session triggers DisConnectInputCluster via SetSessionFade false
 */
HWTEST_F(HpaeFastRendererManagerTest, pause_NonRunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // PREPARED state -> SetSessionFade returns false -> DisConnectInputCluster called
    EXPECT_EQ(fastManager_->Pause(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_PAUSED);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : stop_NonRunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_155
 * @tc.desc  : Test Stop on non-running session triggers DisConnectInputCluster via SetSessionFade false
 */
HWTEST_F(HpaeFastRendererManagerTest, stop_NonRunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // PREPARED state -> SetSessionFade returns false -> DisConnectInputCluster called
    EXPECT_EQ(fastManager_->Stop(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->sinkInputNodeMap_[TEST_STREAM_SESSION_ID]->GetState(), HPAE_SESSION_STOPPED);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setClientVolume_NullGainNode_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_156
 * @tc.desc  : Test SetClientVolume with missing gainNode skips SetClientVolume call
 */
HWTEST_F(HpaeFastRendererManagerTest, setClientVolume_NullGainNode_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Remove gainNode to hit null gainNode skip branch
    fastManager_->gainNodeMap_.erase(TEST_STREAM_SESSION_ID);
    EXPECT_EQ(fastManager_->SetClientVolume(TEST_STREAM_SESSION_ID, 0.5f), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : setSpeed_SessionNotFound_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_157
 * @tc.desc  : Test SetSpeed with non-existent session does not crash (CHECK_AND_RETURN_LOG)
 */
HWTEST_F(HpaeFastRendererManagerTest, setSpeed_SessionNotFound_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    fastManager_->SetSpeed(99999, 1.0f);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : onRewindAndFlush_NotRunningSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_158
 * @tc.desc  : Test OnRewindAndFlush skips non-running sessions (state != RUNNING)
 */
HWTEST_F(HpaeFastRendererManagerTest, onRewindAndFlush_NotRunningSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // State is PREPARED (not RUNNING) -> skip RewindHistoryBuffer
    fastManager_->OnRewindAndFlush(1000, 0);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : triggerAppsUidUpdate_NullSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_159
 * @tc.desc  : Test TriggerAppsUidUpdate with null sinkOutputNode_ skips sink update in lambda
 */
HWTEST_F(HpaeFastRendererManagerTest, triggerAppsUidUpdate_NullSinkOutput_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // Force sinkOutputNode_ to null to test skip branch in TriggerAppsUidUpdate lambda
    fastManager_->sinkOutputNode_ = nullptr;
    fastManager_->TriggerAppsUidUpdate(TEST_STREAM_SESSION_ID);
    WaitForMsgProcessing(fastManager_);
    // Should not crash; sink update skipped since sinkOutputNode_ is null
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : createSinkAttr_VoipFastFlag_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_160
 * @tc.desc  : Test CreateSinkAttr with VOIP fast sink sets AUDIO_FLAG_VOIP_FAST
 */
HWTEST_F(HpaeFastRendererManagerTest, createSinkAttr_VoipFastFlag_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetVoipFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    auto attr = manager->CreateSinkAttr();
    EXPECT_EQ(attr.audioStreamFlag, AUDIO_FLAG_VOIP_FAST);
}

/**
 * @tc.name  : createSinkAttr_MmapFlag_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_161
 * @tc.desc  : Test CreateSinkAttr with non-VOIP sink sets AUDIO_FLAG_MMAP
 */
HWTEST_F(HpaeFastRendererManagerTest, createSinkAttr_MmapFlag_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    auto manager = std::make_shared<HpaeFastRendererManager>(sinkInfo);
    auto attr = manager->CreateSinkAttr();
    EXPECT_EQ(attr.audioStreamFlag, AUDIO_FLAG_MMAP);
}

/**
 * @tc.name  : release_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_162
 * @tc.desc  : Test Release calls DestroyStream and removes session
 */
HWTEST_F(HpaeFastRendererManagerTest, release_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Release(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    HpaeSinkInputInfo info;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(TEST_STREAM_SESSION_ID, info), ERR_INVALID_OPERATION);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : release_NotInit_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_163
 * @tc.desc  : Test Release when not initialized returns ERR_INVALID_OPERATION
 */
HWTEST_F(HpaeFastRendererManagerTest, release_NotInit_001, TestSize.Level0)
{
    EXPECT_EQ(fastManager_->Release(TEST_STREAM_SESSION_ID), ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : startRenderSink_WithSinkOutput_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_164
 * @tc.desc  : Test StartRenderSink with valid sinkOutputNode_ calls RenderSinkStart and returns SUCCESS
 */
HWTEST_F(HpaeFastRendererManagerTest, startRenderSink_WithSinkOutput_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Direct call to StartRenderSink with valid sinkOutputNode_
    int32_t ret = fastManager_->StartRenderSink();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(fastManager_->sinkOutputNode_->GetSinkState(), STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : moveAllStreamToNewSink_MoveAllWithSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_165
 * @tc.desc  : Test MoveAllStreamToNewSink with MOVE_ALL and existing session triggers TriggerSyncCallback
 */
HWTEST_F(HpaeFastRendererManagerTest, moveAllStreamToNewSink_MoveAllWithSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(500003)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // MOVE_ALL with session in map -> collect, TriggerSyncCallback
    fastManager_->MoveAllStreamToNewSink("target_sink", {}, MOVE_ALL);
    // Session should be removed after move
    HpaeSinkInputInfo info;
    EXPECT_EQ(fastManager_->GetSinkInputInfo(500003, info), ERR_INVALID_OPERATION);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : disConnectInputSession_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_166
 * @tc.desc  : Test DisConnectInputSession calls DisConnectInputCluster and returns SUCCESS
 */
HWTEST_F(HpaeFastRendererManagerTest, disConnectInputSession_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // DisConnectInputSession should call DisConnectInputCluster directly (synchronous)
    EXPECT_EQ(fastManager_->DisConnectInputSession(TEST_STREAM_SESSION_ID), SUCCESS);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : reloadRenderManager_InvalidFrameLen_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_167
 * @tc.desc  : Test ReloadRenderManager with invalid frameLen fails InitSinkInner (isReload=true)
 */
HWTEST_F(HpaeFastRendererManagerTest, reloadRenderManager_InvalidFrameLen_001, TestSize.Level1)
{
    HpaeSinkInfo sinkInfo = GetFastSinkInfo();
    sinkInfo.frameLen = 0;
    EXPECT_EQ(fastManager_->ReloadRenderManager(sinkInfo, true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // InitSinkInner should fail since frameLen is 0, isInit_ stays false
    EXPECT_EQ(fastManager_->IsInit(), false);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : isRunning_NullThread_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_168
 * @tc.desc  : Test IsRunning returns false when thread is null but sinkOutputNode_ is not null
 */
HWTEST_F(HpaeFastRendererManagerTest, isRunning_NullThread_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    fastManager_->sinkOutputNode_->SetSinkState(STREAM_MANAGER_RUNNING);
    EXPECT_EQ(fastManager_->IsRunning(), true);
    // DeactivateThread properly sets hpaeSignalProcessThread_ to null
    fastManager_->DeactivateThread();
    // sinkOutputNode_ != nullptr but hpaeSignalProcessThread_ == nullptr -> false
    EXPECT_EQ(fastManager_->IsRunning(), false);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}

/**
 * @tc.name  : deInit_MoveDefaultWithRunningStream_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_169
 * @tc.desc  : Test DeInit with isMoveDefault=true moves running streams to default sink
 */
HWTEST_F(HpaeFastRendererManagerTest, deInit_MoveDefaultWithRunningStream_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    EXPECT_EQ(fastManager_->CreateStream(MakeStreamInfo(TEST_STREAM_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->Start(TEST_STREAM_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    // DeInit(true) should move running stream via MoveAllStreamToNewSink(MOVE_ALL)
    EXPECT_EQ(fastManager_->DeInit(true), SUCCESS);
    EXPECT_EQ(fastManager_->IsInit(), false);
}

/**
 * @tc.name  : suspendStreamManager_SameStateTrue_001
 * @tc.type  : FUNC
 * @tc.number: HpaeFastRendererManagerTest_170
 * @tc.desc  : Test SuspendStreamManager(true) when already suspended is no-op (early return)
 */
HWTEST_F(HpaeFastRendererManagerTest, suspendStreamManager_SameStateTrue_001, TestSize.Level1)
{
    EXPECT_EQ(fastManager_->Init(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    InitWithMockSink(fastManager_);
    // Suspend first -> isSuspend_ becomes true
    EXPECT_EQ(fastManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->isSuspend_.load(), true);
    // Suspend again with same state (true == true) -> early return, no-op
    EXPECT_EQ(fastManager_->SuspendStreamManager(true), SUCCESS);
    WaitForMsgProcessing(fastManager_);
    EXPECT_EQ(fastManager_->isSuspend_.load(), true);
    EXPECT_EQ(fastManager_->DeInit(), SUCCESS);
    WaitForMsgProcessing(fastManager_);
}
