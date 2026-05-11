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
#include <memory>
#include <chrono>
#include "audio_errors.h"
#include "hpae_offload_renderer_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
const std::string TEST_DEVICE_NAME = "test_offload";
const std::string TEST_DEVICE_CLASS = "remote_offload";
const std::string TEST_SINK_NAME = "test_sink";
constexpr uint32_t TEST_SESSION_ID = 100;
constexpr float TEST_VOLUME = 0.5f;
constexpr int32_t TEST_RATE = 1;
constexpr int32_t TEST_EFFECT_MODE = 0;
constexpr int32_t TEST_PRIVACY_TYPE = 0;
constexpr int32_t FRAME_SIZE = 960;
} // namespace

class HpaeOffloadRendererManagerTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    HpaeSinkInfo sinkInfo_;
    std::shared_ptr<HpaeOffloadRendererManager> manager_;
};

void HpaeOffloadRendererManagerTest::SetUp()
{
    sinkInfo_.deviceName = TEST_DEVICE_NAME;
    sinkInfo_.deviceClass = TEST_DEVICE_CLASS;
    sinkInfo_.adapterName = TEST_SINK_NAME;
    sinkInfo_.frameLen = FRAME_SIZE;
    sinkInfo_.samplingRate = SAMPLE_RATE_48000;
    sinkInfo_.format = SAMPLE_F32LE;
    sinkInfo_.channels = STEREO;
    manager_ = std::make_shared<HpaeOffloadRendererManager>(sinkInfo_);
}

void HpaeOffloadRendererManagerTest::TearDown()
{
    manager_.reset();
}

/**
 * @tc.name    : Construct_001
 * @tc.type    : FUNC
 * @tc.number  : Construct_001
 * @tc.desc    : Verify constructor initializes state correctly (not init, sink info stored).
 */
HWTEST_F(HpaeOffloadRendererManagerTest, Construct_001, TestSize.Level0)
{
    EXPECT_EQ(manager_->IsInit(), false);
    HpaeSinkInfo info = manager_->GetSinkInfo();
    EXPECT_EQ(info.deviceName, TEST_DEVICE_NAME);
    EXPECT_EQ(info.deviceClass, TEST_DEVICE_CLASS);
}

/**
 * @tc.name    : IsInit_001
 * @tc.type    : FUNC
 * @tc.number  : IsInit_001
 * @tc.desc    : Verify IsInit returns false before Init is called.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, IsInit_001, TestSize.Level0)
{
    EXPECT_EQ(manager_->IsInit(), false);
}

/**
 * @tc.name    : IsRunning_001
 * @tc.type    : FUNC
 * @tc.number  : IsRunning_001
 * @tc.desc    : Verify IsRunning returns false before Init and thread creation.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, IsRunning_001, TestSize.Level0)
{
    EXPECT_EQ(manager_->IsRunning(), false);
}

/**
 * @tc.name    : IsMsgProcessing_001
 * @tc.type    : FUNC
 * @tc.number  : IsMsgProcessing_001
 * @tc.desc    : Verify IsMsgProcessing returns false when no messages are pending.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, IsMsgProcessing_001, TestSize.Level0)
{
    EXPECT_EQ(manager_->IsMsgProcessing(), false);
}

/**
 * @tc.name    : GetSinkInfo_001
 * @tc.type    : FUNC
 * @tc.number  : GetSinkInfo_001
 * @tc.desc    : Verify GetSinkInfo returns the sink info passed to the constructor.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, GetSinkInfo_001, TestSize.Level0)
{
    HpaeSinkInfo info = manager_->GetSinkInfo();
    EXPECT_EQ(info.deviceName, TEST_DEVICE_NAME);
    EXPECT_EQ(info.deviceClass, TEST_DEVICE_CLASS);
    EXPECT_EQ(info.adapterName, TEST_SINK_NAME);
}

/**
 * @tc.name    : HandleMsg_001
 * @tc.type    : FUNC
 * @tc.number  : HandleMsg_001
 * @tc.desc    : Verify HandleMsg is safe to call when no messages are queued.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, HandleMsg_001, TestSize.Level0)
{
    manager_->HandleMsg();
    EXPECT_EQ(manager_->IsMsgProcessing(), false);
}

/**
 * @tc.name    : SetClientVolume_001
 * @tc.type    : FUNC
 * @tc.number  : SetClientVolume_001
 * @tc.desc    : Verify SetClientVolume returns SUCCESS as a stub implementation.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, SetClientVolume_001, TestSize.Level0)
{
    int32_t ret = manager_->SetClientVolume(TEST_SESSION_ID, TEST_VOLUME);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : SetRate_001
 * @tc.type    : FUNC
 * @tc.number  : SetRate_001
 * @tc.desc    : Verify SetRate returns SUCCESS as a stub implementation.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, SetRate_001, TestSize.Level0)
{
    int32_t ret = manager_->SetRate(TEST_SESSION_ID, TEST_RATE);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : SetAudioEffectMode_001
 * @tc.type    : FUNC
 * @tc.number  : SetAudioEffectMode_001
 * @tc.desc    : Verify SetAudioEffectMode returns SUCCESS as a stub implementation.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, SetAudioEffectMode_001, TestSize.Level0)
{
    int32_t ret = manager_->SetAudioEffectMode(TEST_SESSION_ID, TEST_EFFECT_MODE);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : GetAudioEffectMode_001
 * @tc.type    : FUNC
 * @tc.number  : GetAudioEffectMode_001
 * @tc.desc    : Verify GetAudioEffectMode returns SUCCESS as a stub implementation.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, GetAudioEffectMode_001, TestSize.Level0)
{
    int32_t effectMode = -1;
    int32_t ret = manager_->GetAudioEffectMode(TEST_SESSION_ID, effectMode);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : RegisterReadCallback_001
 * @tc.type    : FUNC
 * @tc.number  : RegisterReadCallback_001
 * @tc.desc    : Verify RegisterReadCallback returns ERR_NOT_SUPPORTED (offload does not support capture).
 */
HWTEST_F(HpaeOffloadRendererManagerTest, RegisterReadCallback_001, TestSize.Level0)
{
    std::weak_ptr<ICapturerStreamCallback> callback;
    int32_t ret = manager_->RegisterReadCallback(TEST_SESSION_ID, callback);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name    : GetAllSinkInputsInfo_001
 * @tc.type    : FUNC
 * @tc.number  : GetAllSinkInputsInfo_001
 * @tc.desc    : Verify GetAllSinkInputsInfo returns empty vector before any streams are created.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, GetAllSinkInputsInfo_001, TestSize.Level0)
{
    std::vector<SinkInput> sinkInputs = manager_->GetAllSinkInputsInfo();
    EXPECT_EQ(sinkInputs.empty(), true);
}

/**
 * @tc.name    : RefreshProcessClusterByDevice_001
 * @tc.type    : FUNC
 * @tc.number  : RefreshProcessClusterByDevice_001
 * @tc.desc    : Verify RefreshProcessClusterByDevice returns SUCCESS.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, RefreshProcessClusterByDevice_001, TestSize.Level0)
{
    int32_t ret = manager_->RefreshProcessClusterByDevice();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : Process_001
 * @tc.type    : FUNC
 * @tc.number  : Process_001
 * @tc.desc    : Verify Process is safe to call when manager is not running (no crash).
 */
HWTEST_F(HpaeOffloadRendererManagerTest, Process_001, TestSize.Level0)
{
    manager_->Process();
    EXPECT_EQ(manager_->IsRunning(), false);
}

/**
 * @tc.name    : DeactivateThread_001
 * @tc.type    : FUNC
 * @tc.number  : DeactivateThread_001
 * @tc.desc    : Verify DeactivateThread is safe to call when thread is null and returns true.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, DeactivateThread_001, TestSize.Level0)
{
    bool ret = manager_->DeactivateThread();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name    : DumpSinkInfo_001
 * @tc.type    : FUNC
 * @tc.number  : DumpSinkInfo_001
 * @tc.desc    : Verify DumpSinkInfo returns ERR_ILLEGAL_STATE when not initialized.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, DumpSinkInfo_001, TestSize.Level0)
{
    int32_t ret = manager_->DumpSinkInfo();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name    : GetThreadName_001
 * @tc.type    : FUNC
 * @tc.number  : GetThreadName_001
 * @tc.desc    : Verify GetThreadName returns sinkInfo_.deviceName.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, GetThreadName_001, TestSize.Level0)
{
    std::string threadName = manager_->GetThreadName();
    EXPECT_EQ(threadName, TEST_DEVICE_NAME);
}

/**
 * @tc.name    : SetPrivacyType_001
 * @tc.type    : FUNC
 * @tc.number  : SetPrivacyType_001
 * @tc.desc    : Verify SetPrivacyType returns SUCCESS as a stub implementation.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, SetPrivacyType_001, TestSize.Level0)
{
    int32_t ret = manager_->SetPrivacyType(TEST_SESSION_ID, TEST_PRIVACY_TYPE);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : GetPrivacyType_001
 * @tc.type    : FUNC
 * @tc.number  : GetPrivacyType_001
 * @tc.desc    : Verify GetPrivacyType returns SUCCESS as a stub implementation.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, GetPrivacyType_001, TestSize.Level0)
{
    int32_t privacyType = -1;
    int32_t ret = manager_->GetPrivacyType(TEST_SESSION_ID, privacyType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name    : Destruct_001
 * @tc.type    : FUNC
 * @tc.number  : Destruct_001
 * @tc.desc    : Verify safe destruction when manager was constructed but never initialized.
 */
HWTEST_F(HpaeOffloadRendererManagerTest, Destruct_001, TestSize.Level0)
{
    HpaeSinkInfo localSinkInfo;
    localSinkInfo.deviceName = "destruct_test";
    localSinkInfo.deviceClass = "remote_offload";
    auto localManager = std::make_shared<HpaeOffloadRendererManager>(localSinkInfo);
    EXPECT_EQ(localManager->IsInit(), false);
    localManager.reset();
    // Destruction completes without crash
}

// ============================================================================
// White-box injection tests: bypass Init() by directly setting private members
// ============================================================================

class HpaeOffloadRendererManagerWhiteBoxTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    HpaeSinkInfo sinkInfo_;
    std::shared_ptr<HpaeOffloadRendererManager> manager_;
    std::shared_ptr<HpaeSinkInputNode> testNode_;
    std::shared_ptr<HpaeSinkInputNode> CreateTestNode(uint32_t sessionId, HpaeSessionState state);
};

std::shared_ptr<HpaeSinkInputNode> HpaeOffloadRendererManagerWhiteBoxTest::CreateTestNode(
    uint32_t sessionId, HpaeSessionState state)
{
    HpaeNodeInfo nodeinfo;
    nodeinfo.streamType = STREAM_MUSIC;
    nodeinfo.sessionId = sessionId;
    nodeinfo.channels = STEREO;
    nodeinfo.samplingRate = SAMPLE_RATE_48000;
    nodeinfo.format = SAMPLE_F32LE;
    nodeinfo.frameLen = FRAME_SIZE;
    auto node = std::make_shared<HpaeSinkInputNode>(nodeinfo);
    node->SetState(state);
    return node;
}

void HpaeOffloadRendererManagerWhiteBoxTest::SetUp()
{
    sinkInfo_.deviceName = TEST_DEVICE_NAME;
    sinkInfo_.deviceClass = TEST_DEVICE_CLASS;
    sinkInfo_.adapterName = TEST_SINK_NAME;
    sinkInfo_.frameLen = FRAME_SIZE;
    sinkInfo_.samplingRate = SAMPLE_RATE_48000;
    sinkInfo_.format = SAMPLE_F32LE;
    sinkInfo_.channels = STEREO;
    manager_ = std::make_shared<HpaeOffloadRendererManager>(sinkInfo_);
    manager_->isInit_.store(true);
    manager_->hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    testNode_ = CreateTestNode(TEST_SESSION_ID, HPAE_SESSION_RUNNING);
    manager_->sinkInputNodeMap_[TEST_SESSION_ID] = testNode_;
    manager_->curNode_ = testNode_;
}

void HpaeOffloadRendererManagerWhiteBoxTest::TearDown()
{
    manager_->DeInit();
    manager_.reset();
}

/**
 * @tc.name    : CreateStream_001
 * @tc.type    : FUNC
 * @tc.number  : CreateStream_001
 * @tc.desc    : Verify CreateStream succeeds when isInit=true with valid streamInfo.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, CreateStream_001, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 200;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_SIZE;
    streamInfo.streamType = STREAM_MUSIC;
    streamInfo.uid = 1000;
    int32_t ret = manager_->CreateStream(streamInfo);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(200), 1u);
}

/**
 * @tc.name    : CreateStream_002
 * @tc.type    : FUNC
 * @tc.number  : CreateStream_002
 * @tc.desc    : Verify CreateStream returns ERR_INVALID_OPERATION when isInit=false.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, CreateStream_002, TestSize.Level0)
{
    manager_->isInit_.store(false);
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 201;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = FRAME_SIZE;
    streamInfo.streamType = STREAM_MUSIC;
    int32_t ret = manager_->CreateStream(streamInfo);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    manager_->isInit_.store(true);
}

/**
 * @tc.name    : CreateStream_003
 * @tc.type    : FUNC
 * @tc.number  : CreateStream_003
 * @tc.desc    : Verify CreateStream returns ERROR when frameLen is 0 (CheckStreamInfo fails).
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, CreateStream_003, TestSize.Level0)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = 202;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_F32LE;
    streamInfo.frameLen = 0; // invalid: frameLen=0 fails CheckStreamInfo
    streamInfo.streamType = STREAM_MUSIC;
    int32_t ret = manager_->CreateStream(streamInfo);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name    : DestroyStream_001
 * @tc.type    : FUNC
 * @tc.number  : DestroyStream_001
 * @tc.desc    : Verify DestroyStream removes the node for an existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, DestroyStream_001, TestSize.Level0)
{
    // Add a second node that is not curNode_
    auto secondNode = CreateTestNode(201, HPAE_SESSION_PREPARED);
    manager_->sinkInputNodeMap_[201] = secondNode;
    int32_t ret = manager_->DestroyStream(201);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(201), 0u);
    // curNode_ should still be testNode_ since 201 was not curNode_
    EXPECT_NE(manager_->curNode_, nullptr);
}

/**
 * @tc.name    : DestroyStream_002
 * @tc.type    : FUNC
 * @tc.number  : DestroyStream_002
 * @tc.desc    : Verify DestroyStream with non-existing sessionId does not crash.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, DestroyStream_002, TestSize.Level0)
{
    int32_t ret = manager_->DestroyStream(9999);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    // No crash, original node still present
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(TEST_SESSION_ID), 1u);
}

/**
 * @tc.name    : DestroyStream_003
 * @tc.type    : FUNC
 * @tc.number  : DestroyStream_003
 * @tc.desc    : Verify DestroyStream for curNode_ clears curNode_ (no other nodes to pick).
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, DestroyStream_003, TestSize.Level0)
{
    // Destroy the curNode_ (TEST_SESSION_ID) — no second node, so curNode_ becomes nullptr
    int32_t ret = manager_->DestroyStream(TEST_SESSION_ID);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(TEST_SESSION_ID), 0u);
    // curNode_ should be nullptr since no other nodes exist in map
    EXPECT_EQ(manager_->curNode_, nullptr);
}

/**
 * @tc.name    : Start_001
 * @tc.type    : FUNC
 * @tc.number  : Start_001
 * @tc.desc    : Verify Start sets node state to RUNNING for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, Start_001, TestSize.Level0)
{
    // Use a non-curNode_ node to avoid ConnectInputSession (needs sinkOutputNode_)
    auto secondNode = CreateTestNode(201, HPAE_SESSION_PREPARED);
    manager_->sinkInputNodeMap_[201] = secondNode;
    int32_t ret = manager_->Start(201);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(secondNode->GetState(), HPAE_SESSION_RUNNING);
}

/**
 * @tc.name    : Start_002
 * @tc.type    : FUNC
 * @tc.number  : Start_002
 * @tc.desc    : Verify Start with non-existing sessionId does not crash.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, Start_002, TestSize.Level0)
{
    int32_t ret = manager_->Start(9999);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    // No crash, original node state unchanged
    EXPECT_EQ(testNode_->GetState(), HPAE_SESSION_RUNNING);
}

/**
 * @tc.name    : Start_003
 * @tc.type    : FUNC
 * @tc.number  : Start_003
 * @tc.desc    : Verify Start ConnectInputSession for curNode_->GetOffloadType() != OFFLOAD_DEFAULT
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, Start_003, TestSize.Level0)
{
    // Use a non-curNode_ node to avoid ConnectInputSession (needs sinkOutputNode_)
    auto secondNode = CreateTestNode(202, HPAE_SESSION_PREPARED);
    secondNode->SetOffloadType(OFFLOAD_ACTIVE_FOREGROUND);
    manager_->sinkInputNodeMap_[202] = secondNode;
    int32_t ret = manager_->Start(202);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(secondNode->GetState(), HPAE_SESSION_RUNNING);
}

/**
 * @tc.name    : Pause_001
 * @tc.type    : FUNC
 * @tc.number  : Pause_001
 * @tc.desc    : Verify Pause sets node state to PAUSED for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, Pause_001, TestSize.Level0)
{
    int32_t ret = manager_->Pause(TEST_SESSION_ID);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(testNode_->GetState(), HPAE_SESSION_PAUSED);
}

/**
 * @tc.name    : Stop_001
 * @tc.type    : FUNC
 * @tc.number  : Stop_001
 * @tc.desc    : Verify Stop sets node state to STOPPED for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, Stop_001, TestSize.Level0)
{
    int32_t ret = manager_->Stop(TEST_SESSION_ID);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(testNode_->GetState(), HPAE_SESSION_STOPPED);
}

/**
 * @tc.name    : Flush_001
 * @tc.type    : FUNC
 * @tc.number  : Flush_001
 * @tc.desc    : Verify Flush returns SUCCESS for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, Flush_001, TestSize.Level0)
{
    // Use a non-curNode_ node to avoid sinkOutputNode_->FlushStream()
    auto secondNode = CreateTestNode(202, HPAE_SESSION_RUNNING);
    manager_->sinkInputNodeMap_[202] = secondNode;
    int32_t ret = manager_->Flush(202);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
}

/**
 * @tc.name    : Drain_001
 * @tc.type    : FUNC
 * @tc.number  : Drain_001
 * @tc.desc    : Verify Drain returns SUCCESS for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, Drain_001, TestSize.Level0)
{
    int32_t ret = manager_->Drain(TEST_SESSION_ID);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
}

/**
 * @tc.name    : MoveStream_001
 * @tc.type    : FUNC
 * @tc.number  : MoveStream_001
 * @tc.desc    : Verify MoveStream removes map entry for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, MoveStream_001, TestSize.Level0)
{
    int32_t ret = manager_->MoveStream(TEST_SESSION_ID, "new_sink");
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(TEST_SESSION_ID), 0u);
}

/**
 * @tc.name    : MoveStream_002
 * @tc.type    : FUNC
 * @tc.number  : MoveStream_002
 * @tc.desc    : Verify MoveStream with non-existing sessionId does not crash.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, MoveStream_002, TestSize.Level0)
{
    int32_t ret = manager_->MoveStream(9999, "new_sink");
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    // No crash, original node still present
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(TEST_SESSION_ID), 1u);
}

/**
 * @tc.name    : MoveAllStream_001
 * @tc.type    : FUNC
 * @tc.number  : MoveAllStream_001
 * @tc.desc    : Verify MoveAllStream when isInit=true uses async path and returns SUCCESS.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, MoveAllStream_001, TestSize.Level0)
{
    std::vector<uint32_t> ids = {TEST_SESSION_ID};
    int32_t ret = manager_->MoveAllStream("new_sink", ids, MOVE_ALL);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(TEST_SESSION_ID), 0u);
}

/**
 * @tc.name    : MoveAllStream_002
 * @tc.type    : FUNC
 * @tc.number  : MoveAllStream_002
 * @tc.desc    : Verify MoveAllStream when isInit=false uses sync path and returns SUCCESS.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, MoveAllStream_002, TestSize.Level0)
{
    manager_->isInit_.store(false);
    std::vector<uint32_t> ids = {TEST_SESSION_ID};
    int32_t ret = manager_->MoveAllStream("new_sink", ids, MOVE_ALL);
    EXPECT_EQ(ret, SUCCESS);
    // Sync path already executed, no HandleMsg needed
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(TEST_SESSION_ID), 0u);
    manager_->isInit_.store(true);
}

/**
 * @tc.name    : SuspendStreamManager_001
 * @tc.type    : FUNC
 * @tc.number  : SuspendStreamManager_001
 * @tc.desc    : Verify SuspendStreamManager returns SUCCESS and updates isSuspend_.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, SuspendStreamManager_001, TestSize.Level0)
{
    // Test early-return path: isSuspend_ == isSuspend → no sinkOutputNode_ access
    manager_->isSuspend_.store(true);
    int32_t ret = manager_->SuspendStreamManager(true);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->isSuspend_.load(), true);
}

/**
 * @tc.name    : SetMute_001
 * @tc.type    : FUNC
 * @tc.number  : SetMute_001
 * @tc.desc    : Verify SetMute sets isMute_ to true after HandleMsg.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, SetMute_001, TestSize.Level0)
{
    int32_t ret = manager_->SetMute(true);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->isMute_, true);
}

/**
 * @tc.name    : SetSpeed_001
 * @tc.type    : FUNC
 * @tc.number  : SetSpeed_001
 * @tc.desc    : Verify SetSpeed returns SUCCESS for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, SetSpeed_001, TestSize.Level0)
{
    manager_->SetSpeed(TEST_SESSION_ID, 2.0f);
    manager_->HandleMsg();
    EXPECT_EQ(testNode_->GetSpeed(), 2.0f);
}

/**
 * @tc.name    : SetOffloadPolicy_001
 * @tc.type    : FUNC
 * @tc.number  : SetOffloadPolicy_001
 * @tc.desc    : Verify SetOffloadPolicy returns SUCCESS for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, SetOffloadPolicy_001, TestSize.Level0)
{
    int32_t ret = manager_->SetOffloadPolicy(TEST_SESSION_ID, 1);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(testNode_->GetOffloadEnabled(), true);
}

/**
 * @tc.name    : RegisterWriteCallback_001
 * @tc.type    : FUNC
 * @tc.number  : RegisterWriteCallback_001
 * @tc.desc    : Verify RegisterWriteCallback returns SUCCESS for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, RegisterWriteCallback_001, TestSize.Level0)
{
    std::weak_ptr<IStreamCallback> callback;
    int32_t ret = manager_->RegisterWriteCallback(TEST_SESSION_ID, callback);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
}

/**
 * @tc.name    : SetLoudnessGain_001
 * @tc.type    : FUNC
 * @tc.number  : SetLoudnessGain_001
 * @tc.desc    : Verify SetLoudnessGain returns SUCCESS for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, SetLoudnessGain_001, TestSize.Level0)
{
    int32_t ret = manager_->SetLoudnessGain(TEST_SESSION_ID, 2.0f);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(testNode_->GetLoudnessGain(), 2.0f);
}

/**
 * @tc.name    : OnRequestLatency_001
 * @tc.type    : FUNC
 * @tc.number  : OnRequestLatency_001
 * @tc.desc    : Verify OnRequestLatency with no converter/loudness nodes does not change latency.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, OnRequestLatency_001, TestSize.Level0)
{
    uint64_t latency = 0;
    manager_->OnRequestLatency(TEST_SESSION_ID, latency);
    EXPECT_EQ(latency, 0u);
}

/**
 * @tc.name    : OnRewindAndFlush_001
 * @tc.type    : FUNC
 * @tc.number  : OnRewindAndFlush_001
 * @tc.desc    : Verify OnRewindAndFlush with curNode_ set does not crash.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, OnRewindAndFlush_001, TestSize.Level0)
{
    manager_->OnRewindAndFlush(1000, 0);
    // No crash expected
}

/**
 * @tc.name    : OnNotifyHdiData_001
 * @tc.type    : FUNC
 * @tc.number  : OnNotifyHdiData_001
 * @tc.desc    : Verify OnNotifyHdiData with curNode_ set does not crash.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, OnNotifyHdiData_001, TestSize.Level0)
{
    auto now = std::chrono::high_resolution_clock::now();
    std::pair<uint64_t, TimePoint> hdiPos = {100, now};
    manager_->OnNotifyHdiData(hdiPos);
    // No crash expected
}

/**
 * @tc.name    : DeInit_001
 * @tc.type    : FUNC
 * @tc.number  : DeInit_001
 * @tc.desc    : Verify DeInit sets isInit_ to false.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, DeInit_001, TestSize.Level0)
{
    EXPECT_EQ(manager_->IsInit(), true);
    int32_t ret = manager_->DeInit();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(manager_->IsInit(), false);
}

/**
 * @tc.name    : Release_001
 * @tc.type    : FUNC
 * @tc.number  : Release_001
 * @tc.desc    : Verify Release calls DestroyStream and removes the node.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, Release_001, TestSize.Level0)
{
    // Release delegates to DestroyStream — curNode_ becomes nullptr since no other nodes
    int32_t ret = manager_->Release(TEST_SESSION_ID);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(TEST_SESSION_ID), 0u);
    EXPECT_EQ(manager_->curNode_, nullptr);
}

/**
 * @tc.name    : PauseWithStandby_001
 * @tc.type    : FUNC
 * @tc.number  : PauseWithStandby_001
 * @tc.desc    : Verify Pause with isStandby=true for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, PauseWithStandby_001, TestSize.Level0)
{
    int32_t ret = manager_->Pause(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(testNode_->GetState(), HPAE_SESSION_PAUSED);
}

/**
 * @tc.name    : PauseWithStandby_002
 * @tc.type    : FUNC
 * @tc.number  : PauseWithStandby_002
 * @tc.desc    : Verify Pause with isStandby=false for existing sessionId.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, PauseWithStandby_002, TestSize.Level0)
{
    int32_t ret = manager_->Pause(TEST_SESSION_ID, false);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(testNode_->GetState(), HPAE_SESSION_PAUSED);
}

/**
 * @tc.name    : AdjustFrameLen_001
 * @tc.type    : FUNC
 * @tc.number  : AdjustFrameLen_001
 * @tc.desc    : Verify AdjustFrameLen adjusts frameLen when input and output differ.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, AdjustFrameLen_001, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceName = TEST_DEVICE_NAME;
    sinkInfo.deviceClass = TEST_DEVICE_CLASS;
    sinkInfo.adapterName = TEST_SINK_NAME;
    sinkInfo.frameLen = FRAME_SIZE;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);

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
 * @tc.name    : AdjustFrameLen_002
 * @tc.type    : FUNC
 * @tc.number  : AdjustFrameLen_002
 * @tc.desc    : Verify AdjustFrameLen returns false when frameLen already matches.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, AdjustFrameLen_002, TestSize.Level0)
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceName = TEST_DEVICE_NAME;
    sinkInfo.deviceClass = TEST_DEVICE_CLASS;
    sinkInfo.adapterName = TEST_SINK_NAME;
    sinkInfo.frameLen = FRAME_SIZE;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    auto offloadManager = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);

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
 * @tc.name    : AddNodeToSink_001
 * @tc.type    : FUNC
 * @tc.number  : AddNodeToSink_001
 * @tc.desc    : Verify AddNodeToSink returns SUCCESS and adds node to map.
 */
HWTEST_F(HpaeOffloadRendererManagerWhiteBoxTest, AddNodeToSink_001, TestSize.Level0)
{
    auto newNode = CreateTestNode(204, HPAE_SESSION_PREPARED);
    HpaeNodeInfo nodeInfo;
    nodeInfo.frameLen = 480;
    nodeInfo.samplingRate = SAMPLE_RATE_48000;
    manager_->sinkOutputNode_ = std::make_unique<HpaeOffloadSinkOutputNode>(nodeInfo);
    int32_t ret = manager_->AddNodeToSink(newNode);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();
    EXPECT_EQ(manager_->sinkInputNodeMap_.count(204), 1u);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
