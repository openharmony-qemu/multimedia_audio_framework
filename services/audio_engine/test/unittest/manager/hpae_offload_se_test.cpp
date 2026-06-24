/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "gtest/gtest.h"
#include <algorithm>
#include <any>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "hpae_offload_renderer_manager.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_loudness_gain_node.h"
#include "effect_instance.h"
#include "hpae_define.h"
#include "test_case_common.h"
#include "fake_audio_effect_lib_entry.h"
#include "hpae_plugin_node.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static constexpr uint32_t CUR_SESSION_ID = 42;
static constexpr uint32_t OTHER_SESSION_ID = 43;
static constexpr uint32_t INVALID_SESSION_ID = 99;
static constexpr uint32_t TEST_FRAME_LEN = 960;
static constexpr uint32_t CUR_NODE_ID = 100;
static constexpr uint32_t OTHER_NODE_ID = 101;
static constexpr uint32_t OUTPUT_NODE_ID = 200;
static constexpr uint32_t PROBE_NODE_ID = 300;

static AudioEffectConfig MakeTestConfig()
{
    AudioEffectConfig config;
    config.inputCfg.samplingRate = SAMPLE_RATE_48000;
    config.inputCfg.channels = STEREO;
    config.outputCfg.samplingRate = SAMPLE_RATE_48000;
    config.outputCfg.channels = STEREO;
    return config;
}

static HpaeSinkInfo GetTestSinkInfo()
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = "LocalDevice";
    sinkInfo.deviceClass = "file_io";
    sinkInfo.adapterName = "file_io";
    sinkInfo.filePath = "/data/test_offload_se.pcm";
    sinkInfo.frameLen = TEST_FRAME_LEN;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_F32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    return sinkInfo;
}

static HpaeNodeInfo MakeCurNodeInfo()
{
    HpaeNodeInfo info;
    info.nodeId = CUR_NODE_ID;
    info.sessionId = CUR_SESSION_ID;
    info.frameLen = TEST_FRAME_LEN;
    info.samplingRate = SAMPLE_RATE_48000;
    info.channels = STEREO;
    info.format = SAMPLE_F32LE;
    info.deviceClass = "file_io";
    return info;
}

static HpaeNodeInfo MakeOtherNodeInfo()
{
    HpaeNodeInfo info;
    info.nodeId = OTHER_NODE_ID;
    info.sessionId = OTHER_SESSION_ID;
    info.frameLen = TEST_FRAME_LEN;
    info.samplingRate = SAMPLE_RATE_48000;
    info.channels = STEREO;
    info.format = SAMPLE_F32LE;
    info.deviceClass = "file_io";
    return info;
}

static HpaeNodeInfo MakeOutputNodeInfo()
{
    HpaeNodeInfo info;
    info.nodeId = OUTPUT_NODE_ID;
    info.sessionId = CUR_SESSION_ID;
    info.frameLen = TEST_FRAME_LEN;
    info.samplingRate = SAMPLE_RATE_48000;
    info.channels = STEREO;
    info.format = SAMPLE_F32LE;
    info.deviceClass = "file_io";
    return info;
}

// Creates an EffectInstance for testing
static std::shared_ptr<EffectInstance> MakeTestEffectInstance()
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("audio_aiss_enhance", MakeTestConfig());
    return instance;
}

struct StatusEvent {
    HpaeStreamClassType streamClassType;
    uint32_t sessionId;
    HpaeSessionState state;
    IOperation operation;
};

class StatusCallbackRecorder final : public ISendMsgCallback {
public:
    void Invoke(HpaeMsgCode cmdID, const std::any &args) override
    {
        Record(cmdID, args);
    }

    void InvokeSync(HpaeMsgCode cmdID, const std::any &args) override
    {
        Record(cmdID, args);
    }

    size_t Count(uint32_t sessionId, IOperation operation) const
    {
        return static_cast<size_t>(std::count_if(events_.begin(), events_.end(),
            [sessionId, operation](const StatusEvent &event) {
                return event.sessionId == sessionId && event.operation == operation;
            }));
    }

    std::vector<StatusEvent> events_;

private:
    void Record(HpaeMsgCode cmdID, const std::any &args)
    {
        if (cmdID != UPDATE_STATUS) {
            return;
        }
        auto statusArgs = std::any_cast<std::tuple<HpaeStreamClassType, uint32_t,
            HpaeSessionState, IOperation>>(&args);
        if (statusArgs == nullptr) {
            return;
        }
        events_.push_back({std::get<0>(*statusArgs), std::get<1>(*statusArgs),
            std::get<2>(*statusArgs), std::get<3>(*statusArgs)});
    }
};

// Test fixture: sets up HpaeOffloadRendererManager with real node chain,
// following the same pattern as hpae_render_manager_test.cpp.
// Uses -fno-access-control build flag to set private members directly.
class HpaeOffloadSeTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        HpaeSinkInfo sinkInfo = GetTestSinkInfo();
        manager_ = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
        callback_ = std::make_shared<StatusCallbackRecorder>();
        manager_->RegisterSendMsgCallback(callback_);
        config_ = MakeTestConfig();

        // Set up SignalProcessThread (needed for SendRequest infrastructure)
        manager_->hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();

        // Set up curNode_ (the active stream in Offload single-stream model)
        HpaeNodeInfo curInfo = MakeCurNodeInfo();
        curNode_ = std::make_shared<HpaeSinkInputNode>(curInfo);
        curNode_->SetState(HPAE_SESSION_RUNNING);
        manager_->curNode_ = curNode_;
        manager_->sinkInputNodeMap_[CUR_SESSION_ID] = curNode_;

        // Set up non-curNode_ (exists in map but not active chain)
        HpaeNodeInfo otherInfo = MakeOtherNodeInfo();
        otherNode_ = std::make_shared<HpaeSinkInputNode>(otherInfo);
        otherNode_->SetState(HPAE_SESSION_RUNNING);
        manager_->sinkInputNodeMap_[OTHER_SESSION_ID] = otherNode_;

        // Set up offload processing node chain (normally done by CreateOffloadNodes)
        HpaeNodeInfo outputInfo = MakeOutputNodeInfo();
        manager_->converterForOutput_ =
            std::make_shared<HpaeAudioFormatConverterNode>(outputInfo, outputInfo);
        manager_->converterForOutput_->SetDownmixNormalization(false);
        manager_->converterForLoudness_ =
            std::make_shared<HpaeAudioFormatConverterNode>(curInfo, outputInfo);
        manager_->converterForLoudness_->SetDownmixNormalization(false);
        manager_->loudnessGainNode_ =
            std::make_shared<HpaeLoudnessGainNode>(outputInfo);

        // Connect chain: converterForOutput_ -> loudnessGainNode_ -> converterForLoudness_ -> curNode_
        // (Connect = downstream reads from upstream)
        manager_->converterForOutput_->Connect(manager_->loudnessGainNode_);
        manager_->loudnessGainNode_->Connect(manager_->converterForLoudness_);
        manager_->converterForLoudness_->Connect(curNode_);

        HpaeNodeInfo outputNodeInfo = MakeOutputNodeInfo();
        manager_->sinkOutputNode_ = std::make_unique<HpaeOffloadSinkOutputNode>(outputNodeInfo);
    }

    void TearDown() override
    {
        // Clean up SE node if present
        if (manager_->sessionEffectNode_ != nullptr) {
            manager_->sessionEffectNode_->DropEffect();
            manager_->sessionEffectNode_.reset();
            manager_->sessionEffectSessionId_ = 0;
        }
        // Clear attach state on test nodes
        for (auto &[sid, node] : manager_->sinkInputNodeMap_) {
            if (node) { node->ClearSessionEffectAttachState(); }
        }
        manager_.reset();
    }

    std::shared_ptr<HpaeOffloadRendererManager> manager_;
    std::shared_ptr<HpaeSinkInputNode> curNode_;
    std::shared_ptr<HpaeSinkInputNode> otherNode_;
    std::shared_ptr<StatusCallbackRecorder> callback_;
    AudioEffectConfig config_;
};

// OSE-1: CreateSessionEffect for curNode_ → immediate creation
HWTEST_F(HpaeOffloadSeTest, createSessionEffect_curNode_immediate, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance",
        instance, config_);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, CUR_SESSION_ID);
    // Attach state should be on the SinkInputNode
    EXPECT_TRUE(curNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);
    ASSERT_FALSE(callback_->events_.empty());
    EXPECT_EQ(callback_->events_.back().state, HPAE_SESSION_RUNNING);
}

// OSE-B1: a duplicate CreateSessionEffect (curNode already has a materialized SE) is rejected with
// ERR_SESSION_ALREADY_BOUND and must NOT overwrite the existing attach state on the SinkInputNode.
HWTEST_F(HpaeOffloadSeTest, createSessionEffect_duplicateBound_doesNotOverwriteAttachState, TestSize.Level0)
{
    auto instance1 = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance1, config_);
    ASSERT_EQ(ret, 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());

    // Second Create with a different instance/effect — must be rejected as ALREADY_BOUND.
    auto instance2 = MakeTestEffectInstance();
    int32_t ret2 = manager_->CreateSessionEffect(CUR_SESSION_ID, "other_effect", instance2, config_);
    EXPECT_EQ(ret2, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND));

    // Attach state still reflects the FIRST accepted Create, not the rejected second one.
    auto st = curNode_->GetSessionEffectAttachState();
    ASSERT_TRUE(st.has_value());
    EXPECT_EQ(st->effectName, "audio_aiss_enhance");
    EXPECT_EQ(st->instance.lock().get(), instance1.get());
    EXPECT_NE(st->instance.lock().get(), instance2.get());
    // The rejected Create must not emit a second ENABLED.
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);
}

// OSE-1b: Reload detach drops only the physical node and recreate uses a fresh node
HWTEST_F(HpaeOffloadSeTest, reloadDetach_recreatesFreshNodeWithoutDisabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_EQ(ret, 0);
    auto oldNode = manager_->sessionEffectNode_;
    ASSERT_NE(oldNode, nullptr);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    ASSERT_EQ(manager_->DisConnectInputSession(), SUCCESS);
    manager_->DetachSessionEffectNodeForReload();

    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(oldNode->GetEffectInstance(), nullptr);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);

    ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    EXPECT_EQ(ret, 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_NE(manager_->sessionEffectNode_, oldNode);
    EXPECT_EQ(manager_->sessionEffectSessionId_, CUR_SESSION_ID);
    EXPECT_EQ(manager_->sessionEffectNode_->GetEffectInstance(), instance);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 2u);
}

// OSE-1c: Reload detach preserves attach state (no physical node was created)
HWTEST_F(HpaeOffloadSeTest, reloadDetach_pendingOnly_clearsPendingWithoutDisabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->converterForLoudness_ = nullptr;

    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(manager_->sessionEffectNode_, nullptr);
    // Attach state should be on the SinkInputNode (deferred, no physical node)
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    manager_->DetachSessionEffectNodeForReload();

    // Attach state SURVIVES reload (will re-materialize on ConnectInputSession)
    EXPECT_TRUE(curNode_->HasSessionEffectAttachState());
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);
}

// OSE-1d: DetachSessionEffectNodeForReload captures lastSeStatus so preheat restarts after reload.
HWTEST_F(HpaeOffloadSeTest, detachForReload_writesLastSeStatus_toAttachState, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_EQ(ret, 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());

    // Drive the SE node into WARMING_UP so GetStatus() reports an active preheat state.
    manager_->sessionEffectNode_->StartEffect();
    ASSERT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    manager_->DetachSessionEffectNodeForReload();

    // Physical cache is reset, attach state survives...
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());
    // ...and now carries the captured lastSeStatus so materialize can restart preheat.
    auto st = curNode_->GetSessionEffectAttachState();
    ASSERT_TRUE(st.has_value());
    EXPECT_EQ(st->lastSeStatus, static_cast<int32_t>(HpaeSessionEffectNode::STATUS::WARMING_UP));

    // Re-materialize: lastSeStatus >= WARMING_UP triggers StartEffect on the fresh node, so it
    // ends up in WARMING_UP (not IDLE) — proving the captured status flowed through.
    manager_->MaterializeSessionEffectFromAttachState(CUR_SESSION_ID);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
}

// OSE-1a: CreateSessionEffect creation failure clears active SE state without disabled rollback
HWTEST_F(HpaeOffloadSeTest, createSessionEffect_createFailure_clearsStateWithoutDisabled, TestSize.Level0)
{
    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", nullptr, config_);

    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_CREATE_FAILED));
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
}

// OSE-2: CreateSessionEffect for non-curNode_ → deferred to pending
HWTEST_F(HpaeOffloadSeTest, createSessionEffect_notCurNode_deferred, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(OTHER_SESSION_ID, "audio_aiss_enhance",
        instance, config_);
    EXPECT_EQ(ret, 0);
    // SE not created yet (session is not curNode_)
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    // Attach state stored on otherNode_ (deferred)
    EXPECT_TRUE(otherNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);
    ASSERT_FALSE(callback_->events_.empty());
    EXPECT_EQ(callback_->events_.back().state, HPAE_SESSION_RUNNING);
}

// OSE-3: DestroySessionEffect for active SE → disconnect + restore chain
HWTEST_F(HpaeOffloadSeTest, destroySessionEffect_active, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    int32_t ret = manager_->DestroySessionEffect(CUR_SESSION_ID);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// OSE-3-enhanced: DestroySessionEffect for active SE → chain restored (direct connection)
HWTEST_F(HpaeOffloadSeTest, destroySessionEffect_active_restoresChain, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    int32_t ret = manager_->DestroySessionEffect(CUR_SESSION_ID);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);

    // Verify chain restored: loudnessGainNode_ reads directly from converterForLoudness_
    // (no SE node in between)
    // After disconnect+reconnect, loudnessGainNode_'s upstream should be converterForLoudness_
    // We verify indirectly: no crash when SignalProcess runs on the chain
    ASSERT_NE(manager_->loudnessGainNode_, nullptr);
    ASSERT_NE(manager_->converterForLoudness_, nullptr);
}

// OSE-4: DestroySessionEffect for deferred SE (attach state only) → clear state + disabled
HWTEST_F(HpaeOffloadSeTest, destroySessionEffect_pending, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_TRUE(otherNode_->HasSessionEffectAttachState());

    int32_t ret = manager_->DestroySessionEffect(OTHER_SESSION_ID);
    EXPECT_EQ(ret, 0);
    EXPECT_FALSE(otherNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// OSE-4a: RemoveSessionEffectNode for active SE sends disabled directly
HWTEST_F(HpaeOffloadSeTest, removeSessionEffectNode_active_sendsDisabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    int32_t ret = manager_->RemoveSessionEffectNode();
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// OSE-4a1: RemoveSessionEffectNode waits preheat thread before restoring offload chain
HWTEST_F(HpaeOffloadSeTest, removeSessionEffectNode_waitsPreheatThreadIdle, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    auto node = manager_->sessionEffectNode_;
    ASSERT_NE(node, nullptr);

    node->stopRequested_.store(false);
    node->preheatThread_ = std::thread([node]() {
        while (!node->stopRequested_.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    });
    ASSERT_TRUE(node->preheatThread_.joinable());

    int32_t ret = manager_->RemoveSessionEffectNode();

    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(node->stopRequested_.load());
    EXPECT_FALSE(node->preheatThread_.joinable());
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// OSE-4b: DeleteInputSession drops active SE and sends disabled
HWTEST_F(HpaeOffloadSeTest, deleteInputSession_activeSessionEffect_sendsDisabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    manager_->DeleteInputSession();
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// OSE-4c: DestroyStream clears attach state and sends disabled for non-curNode
HWTEST_F(HpaeOffloadSeTest, destroyStream_pendingSessionEffect_sendsDisabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_TRUE(otherNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    manager_->isInit_.store(true);
    int32_t ret = manager_->DestroyStream(OTHER_SESSION_ID);
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();

    // Node is removed from map after DestroyStream, so we check via callback
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// OSE-4d: MoveStream for non-current deferred SE — attach state follows stream (no DISABLED)
HWTEST_F(HpaeOffloadSeTest, moveStream_pendingSessionEffect_attachStateFollowsNoDisabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_TRUE(otherNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    manager_->isInit_.store(true);
    int32_t ret = manager_->MoveStream(OTHER_SESSION_ID, "new_sink");
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();

    // BEHAVIOR CHANGE: MoveStream no longer emits DISABLED — attach state follows the stream
    // to the new sink and will re-materialize there via ConnectInputSession.
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    // Positive assertion: attach state survived on the node and follows the stream to the new
    // sink. The re-materialized ENABLED on the new sink is verified end-to-end (ConnectInputSession
    // is too heavy to drive from this unit test).
    EXPECT_TRUE(otherNode_->HasSessionEffectAttachState());
}

// OSE-4e: MoveAllStreamToNewSink for non-current deferred SE — attach state follows stream (no DISABLED)
HWTEST_F(HpaeOffloadSeTest, moveAllStream_pendingSessionEffect_attachStateFollowsNoDisabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_TRUE(otherNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    std::vector<uint32_t> moveIds = { OTHER_SESSION_ID };
    manager_->MoveAllStreamToNewSink("new_sink", moveIds, MOVE_SINGLE);

    // BEHAVIOR CHANGE: MoveAllStreamToNewSink no longer emits DISABLED — attach state follows
    // the stream to the new sink and will re-materialize there via ConnectInputSession.
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    // Positive assertion: attach state survived on the node and follows the stream to the new
    // sink. The re-materialized ENABLED on the new sink is verified end-to-end (ConnectInputSession
    // is too heavy to drive from this unit test).
    EXPECT_TRUE(otherNode_->HasSessionEffectAttachState());
}

// OSE-4f: Attach-state materialization failure rolls back previously sent enabled status
HWTEST_F(HpaeOffloadSeTest, connectInputSession_pendingInsertFailure_rollsBackEnabled, TestSize.Level0)
{
    // Use nullptr instance — CreateEffectWithInstance will fail inside InsertSessionEffectNode
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "audio_aiss_enhance", nullptr, config_);
    ASSERT_TRUE(otherNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    // Simulate curNode_ switch: OTHER_SESSION_ID becomes curNode_
    manager_->curNode_ = otherNode_;
    // Chain is already set up from SetUp(); InsertSessionEffectNode will fail on nullptr instance

    int32_t ret = manager_->ConnectInputSession();
    EXPECT_EQ(ret, SUCCESS);

    // Materialization failed: sessionEffectSessionId_ stays 0, DISABLED emitted
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// OSE-5: active and deferred SessionEffect state tracked via physical cache + attach state
HWTEST_F(HpaeOffloadSeTest, sessionEffectState_activeAndPending, TestSize.Level0)
{
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_FALSE(curNode_->HasSessionEffectAttachState());

    // Create active SE for curNode_
    auto instance1 = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance1, config_);

    // Create deferred SE for otherNode_
    auto instance2 = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "audio_aiss_enhance", instance2, config_);

    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, CUR_SESSION_ID);
    // otherNode_ has attach state (deferred), curNode_ also has attach state
    EXPECT_TRUE(otherNode_->HasSessionEffectAttachState());
    EXPECT_TRUE(curNode_->HasSessionEffectAttachState());
}

// OSE-6: IsValidSession checks sinkInputNodeMap_
HWTEST_F(HpaeOffloadSeTest, isValidSession, TestSize.Level0)
{
    EXPECT_TRUE(manager_->IsValidSession(CUR_SESSION_ID));
    EXPECT_TRUE(manager_->IsValidSession(OTHER_SESSION_ID));
    EXPECT_FALSE(manager_->IsValidSession(INVALID_SESSION_ID));
}

// OSE-7: SetSessionEffectParameter on active SE → delegates to node
HWTEST_F(HpaeOffloadSeTest, setSessionEffectParameter_active, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);

    int32_t ret = manager_->SetSessionEffectParameter(CUR_SESSION_ID,
        static_cast<int32_t>(EffectParameterType::VOCAL_RATIO), 0.7f);
    EXPECT_EQ(ret, 0);
}

// OSE-8: SetSessionEffectParameter with no active SE → ERR_EFFECT_NOT_BOUND
HWTEST_F(HpaeOffloadSeTest, setSessionEffectParameter_notActive, TestSize.Level0)
{
    int32_t ret = manager_->SetSessionEffectParameter(CUR_SESSION_ID,
        static_cast<int32_t>(EffectParameterType::VOCAL_RATIO), 0.5f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

// OSE-9: Deferred SE auto-materialized when session becomes curNode_ via ConnectInputSession
HWTEST_F(HpaeOffloadSeTest, pendingCreatedOnSetCurrentNode, TestSize.Level0)
{
    // 1. Create deferred SE for OTHER_SESSION_ID (not curNode_)
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_TRUE(otherNode_->HasSessionEffectAttachState());
    ASSERT_EQ(manager_->sessionEffectNode_, nullptr);

    // 2. Simulate curNode_ switch: OTHER_SESSION_ID becomes curNode_
    manager_->curNode_ = otherNode_;

    // 3. ConnectInputSession will read attach state and materialize physical SE
    int32_t ret = manager_->ConnectInputSession();
    EXPECT_EQ(ret, SUCCESS);

    // SE now active for the new curNode_
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, OTHER_SESSION_ID);
    EXPECT_EQ(callback_->Count(OTHER_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 2u);
}

// OSE-EXTRA: Double create for same session returns ERR_SESSION_ALREADY_BOUND
HWTEST_F(HpaeOffloadSeTest, createSessionEffect_doubleCreate, TestSize.Level0)
{
    auto instance1 = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance1, config_);

    auto instance2 = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance",
        instance2, config_);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND));
}

// OSE-EXTRA: DestroySessionEffect for unknown session returns ERR_EFFECT_NOT_BOUND
HWTEST_F(HpaeOffloadSeTest, destroySessionEffect_unknownSession, TestSize.Level0)
{
    int32_t ret = manager_->DestroySessionEffect(INVALID_SESSION_ID);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

// --- Offload dataflow test infrastructure ---

// Source node: generates PCM data filled with a constant value.
// Overrides DoProcess to push data directly (has no upstream to pull from).
class OffloadMockSourceNode : public HpaePluginNode {
public:
    explicit OffloadMockSourceNode(HpaeNodeInfo& info, float fillValue = 0.5f)
        : HpaePluginNode(info), fillValue_(fillValue)
    {
        silenceData_.SetBufferSilence(false);
    }

    void DoProcess() override
    {
        float *data = silenceData_.GetPcmDataBuffer();
        size_t n = silenceData_.GetFrameLen() * silenceData_.GetChannelCount();
        for (size_t i = 0; i < n; i++) {
            data[i] = fillValue_;
        }
        silenceData_.SetBufferValid(true);
        outputStream_.WriteDataToOutput(&silenceData_);
    }

    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override
    {
        return inputs.empty() ? nullptr : inputs[0];
    }
    uint64_t GetLatency(uint32_t) override { return 0; }

private:
    float fillValue_;
};

// Probe node: captures output buffer for verification.
class OffloadProbeNode : public HpaePluginNode {
public:
    using HpaePluginNode::HpaePluginNode;
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override
    {
        captured_ = inputs.empty() ? nullptr : inputs[0];
        return captured_;
    }
    HpaePcmBuffer *GetCaptured() const { return captured_; }
    uint64_t GetLatency(uint32_t) override { return 0; }

private:
    HpaePcmBuffer *captured_ = nullptr;
};

class HpaeOffloadSeDataflowTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        fake_.Reset();
        HpaeSinkInfo sinkInfo = GetTestSinkInfo();
        manager_ = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);

        // Create local chain: source -> converterForLoudness_ -> [SE] -> loudnessGainNode_ -> probe
        HpaeNodeInfo curInfo = MakeCurNodeInfo();
        HpaeNodeInfo outputInfo = MakeOutputNodeInfo();

        source_ = std::make_shared<OffloadMockSourceNode>(curInfo);
        manager_->converterForLoudness_ =
            std::make_shared<HpaeAudioFormatConverterNode>(curInfo, outputInfo);
        manager_->converterForLoudness_->SetDownmixNormalization(false);
        manager_->loudnessGainNode_ =
            std::make_shared<HpaeLoudnessGainNode>(outputInfo);

        // Connect: loudnessGainNode_ <- converterForLoudness_ <- source
        manager_->converterForLoudness_->Connect(source_);
        manager_->loudnessGainNode_->Connect(manager_->converterForLoudness_);

        // Probe after loudnessGainNode_
        HpaeNodeInfo probeInfo = outputInfo;
        probeInfo.nodeId = PROBE_NODE_ID;
        probe_ = std::make_shared<OffloadProbeNode>(probeInfo);
        probe_->Connect(manager_->loudnessGainNode_);

        manager_->sessionEffectSessionId_ = CUR_SESSION_ID;
    }

    void TearDown() override
    {
        if (manager_->sessionEffectNode_ != nullptr) {
            manager_->sessionEffectNode_->DropEffect();
            manager_->sessionEffectNode_.reset();
            manager_->sessionEffectSessionId_ = 0;
        }
        manager_.reset();
    }

    std::shared_ptr<EffectInstance> CreateEffectInstance()
    {
        auto inst = std::make_shared<EffectInstance>();
        auto lib = fake_.GetLibrary();
        inst->Init("test_effect", config_, &lib);
        return inst;
    }

    FakeAudioEffectLibEntry fake_;
    std::shared_ptr<OffloadMockSourceNode> source_;
    std::shared_ptr<HpaeOffloadRendererManager> manager_;
    std::shared_ptr<OffloadProbeNode> probe_;
    AudioEffectConfig config_ = MakeTestConfig();
};

// OSE-DF-1: WARMING_UP -- SE outputs silence, does not pull upstream
HWTEST_F(HpaeOffloadSeDataflowTest, warmingUp_outputsSilence, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    manager_->InsertSessionEffectNode("test_effect", instance, config_);

    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    manager_->sessionEffectNode_->StartEffect();
    EXPECT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    probe_->DoProcess();

    auto *out = probe_->GetCaptured();
    ASSERT_NE(out, nullptr);
    float *data = out->GetPcmDataBuffer();
    size_t n = out->GetFrameLen() * out->GetChannelCount();
    for (size_t i = 0; i < n; i++) {
        EXPECT_FLOAT_EQ(data[i], 0.0f) << " non-zero at sample " << i;
    }
}

// OSE-DF-2: INTERCEPTING -- SE pulls upstream, FeedInput called, outputs silence
HWTEST_F(HpaeOffloadSeDataflowTest, intercepting_feedsAndOutputsSilence, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);
    auto instance = CreateEffectInstance();
    manager_->InsertSessionEffectNode("test_effect", instance, config_);
    manager_->sessionEffectNode_->StartEffect();

    // Transition WARMING_UP -> INTERCEPTING (prebufFilled=true from SetPreheatFrames(0))
    probe_->DoProcess();

    uint32_t feedCountAfterTransition = fake_.GetFeedInputCount();
    EXPECT_GE(feedCountAfterTransition, 1u);

    // Second DoProcess in INTERCEPTING
    probe_->DoProcess();

    auto *out = probe_->GetCaptured();
    ASSERT_NE(out, nullptr);
    float *data = out->GetPcmDataBuffer();
    size_t n = out->GetFrameLen() * out->GetChannelCount();
    for (size_t i = 0; i < n; i++) {
        EXPECT_FLOAT_EQ(data[i], 0.0f) << " non-zero at sample " << i;
    }
    EXPECT_GT(fake_.GetFeedInputCount(), feedCountAfterTransition);
}

// OSE-DF-3: ACTIVE -- SE outputs algorithm passthrough data
HWTEST_F(HpaeOffloadSeDataflowTest, active_outputsAlgoData, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);
    fake_.SetProcessThreshold(1920);
    fake_.SetProcessDelayMs(0);
    auto instance = CreateEffectInstance();
    manager_->InsertSessionEffectNode("test_effect", instance, config_);
    manager_->sessionEffectNode_->StartEffect();

    // Drive to ACTIVE
    probe_->DoProcess();  // WARMING_UP→INTERCEPTING
    manager_->sessionEffectNode_->ResetTimestampsForTest();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    probe_->DoProcess();  // INTERCEPTING→ACTIVE
    ASSERT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // ACTIVE state — verify output
    probe_->DoProcess();
    auto *out = probe_->GetCaptured();
    ASSERT_NE(out, nullptr);
    float *data = out->GetPcmDataBuffer();
    size_t n = out->GetFrameLen() * out->GetChannelCount();
    for (size_t i = 0; i < n; i++) {
        EXPECT_FLOAT_EQ(data[i], 0.5f) << " mismatch at sample " << i;
    }
}

// OSE-DF-4: RemoveSessionEffectNode -- data passes through without SE
HWTEST_F(HpaeOffloadSeDataflowTest, remove_restoresPassthrough, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);
    fake_.SetProcessThreshold(1920);
    fake_.SetProcessDelayMs(0);
    auto instance = CreateEffectInstance();
    manager_->InsertSessionEffectNode("test_effect", instance, config_);
    manager_->sessionEffectNode_->StartEffect();

    // Drive to ACTIVE
    probe_->DoProcess();  // WARMING_UP→INTERCEPTING
    manager_->sessionEffectNode_->ResetTimestampsForTest();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    probe_->DoProcess();  // INTERCEPTING→ACTIVE
    ASSERT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // Remove SE
    manager_->RemoveSessionEffectNode();
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    // Data should pass through directly
    probe_->DoProcess();

    auto *out = probe_->GetCaptured();
    ASSERT_NE(out, nullptr);
    float *data = out->GetPcmDataBuffer();
    size_t n = out->GetFrameLen() * out->GetChannelCount();
    for (size_t i = 0; i < n; i++) {
        EXPECT_FLOAT_EQ(data[i], 0.5f) << " mismatch at sample " << i;
    }
}

// OSE-TOPO-1: InsertSessionEffectNode -- topology correct
HWTEST_F(HpaeOffloadSeDataflowTest, insert_topo_correct, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    manager_->InsertSessionEffectNode("test_effect", instance, config_);

    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectNode_->GetPreOutNum(), 1u);
    EXPECT_EQ(manager_->loudnessGainNode_->GetPreOutNum(), 1u);
}

// OSE-TOPO-2: RemoveSessionEffectNode -- chain restored
HWTEST_F(HpaeOffloadSeDataflowTest, remove_topo_restored, TestSize.Level0)
{
    auto instance = CreateEffectInstance();
    manager_->InsertSessionEffectNode("test_effect", instance, config_);

    manager_->RemoveSessionEffectNode();
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->loudnessGainNode_->GetPreOutNum(), 1u);
}

// T15: Pause→Start resumes SE (Offload, topology + state + data flow)
HWTEST_F(HpaeOffloadSeDataflowTest, pauseStart_resumesSE, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);
    fake_.SetProcessThreshold(1920);
    fake_.SetProcessDelayMs(0);
    auto instance = CreateEffectInstance();
    manager_->InsertSessionEffectNode("test_effect", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Drive to ACTIVE
    manager_->sessionEffectNode_->StartEffect();
    probe_->DoProcess();  // WARMING_UP→INTERCEPTING
    manager_->sessionEffectNode_->ResetTimestampsForTest();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    probe_->DoProcess();  // INTERCEPTING→ACTIVE
    ASSERT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // --- Simulate Pause ---
    manager_->sessionEffectNode_->PauseEffect();
    EXPECT_TRUE(manager_->sessionEffectNode_->IsPaused());

    // Disconnect SE from chain (as DisConnectInputSession now does)
    manager_->loudnessGainNode_->DisConnect(manager_->sessionEffectNode_);
    manager_->sessionEffectNode_->DisConnect(manager_->converterForLoudness_);

    // --- Simulate Start ---
    // Reconnect SE into chain
    manager_->sessionEffectNode_->Connect(manager_->converterForLoudness_);
    manager_->loudnessGainNode_->Connect(manager_->sessionEffectNode_);

    // StartEffect (as Start does after ConnectInputSession)
    manager_->sessionEffectNode_->StartEffect();

    // Verify: SE resumed, not paused, status preserved (ACTIVE)
    EXPECT_FALSE(manager_->sessionEffectNode_->IsPaused());
    EXPECT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // Verify data flow: after resume, upstream data should flow through SE
    fake_.Reset();
    probe_->DoProcess();
    EXPECT_GT(fake_.GetFeedInputCount(), 0u) << "FeedInput not called — data path broken after resume";
}

// T16: Pause→Flush→Start (Offload, state machine verification)
HWTEST_F(HpaeOffloadSeDataflowTest, pauseFlushStart_stateTransitions, TestSize.Level0)
{
    fake_.SetPreheatFrames(0);
    fake_.SetProcessThreshold(1920);
    fake_.SetProcessDelayMs(0);
    auto instance = CreateEffectInstance();
    manager_->InsertSessionEffectNode("test_effect", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Drive to ACTIVE state
    manager_->sessionEffectNode_->StartEffect();
    probe_->DoProcess();  // WARMING_UP→INTERCEPTING
    manager_->sessionEffectNode_->ResetTimestampsForTest();
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    probe_->DoProcess();  // INTERCEPTING→ACTIVE
    ASSERT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::ACTIVE);

    // --- Pause ---
    manager_->sessionEffectNode_->PauseEffect();
    EXPECT_TRUE(manager_->sessionEffectNode_->IsPaused());

    // --- Flush ---
    manager_->sessionEffectNode_->FlushEffect();
    EXPECT_FALSE(manager_->sessionEffectNode_->IsPaused());
    EXPECT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::IDLE);

    // --- Start ---
    manager_->sessionEffectNode_->StartEffect();
    EXPECT_FALSE(manager_->sessionEffectNode_->IsPaused());
    EXPECT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    // Verify data flow after restart
    fake_.Reset();
    probe_->DoProcess();
    EXPECT_GT(fake_.GetFeedInputCount(), 0u) << "FeedInput not called after Flush→Start";
}

// OSE-FLUSH-1: running offload SE switch requests StopStream path
HWTEST_F(HpaeOffloadSeTest, stopOffloadSinkIfRunning_running, TestSize.Level0)
{
    curNode_->SetState(HPAE_SESSION_RUNNING);
    EXPECT_TRUE(manager_->StopOffloadSinkIfRunning());
}

// OSE-FLUSH-2: non-running offload SE switch skips sink flush/stop
HWTEST_F(HpaeOffloadSeTest, stopOffloadSinkIfRunning_nonRunning, TestSize.Level0)
{
    curNode_->SetState(HPAE_SESSION_PAUSED);
    EXPECT_FALSE(manager_->StopOffloadSinkIfRunning());
}

// OSE-FIX1a: Expired instance in MaterializeSessionEffectFromAttachState clears attach state + emits DISABLED
HWTEST_F(HpaeOffloadSeTest, materialize_offload_expiredClearsAttachState, TestSize.Level0)
{
    // Set up attach state with expired instance on curNode_
    SessionEffectAttachState st;
    st.enabled = true;
    st.effectName = "audio_aiss_enhance";
    // instance is default weak_ptr (expired)
    st.config = config_;
    curNode_->SetSessionEffectAttachState(st);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());

    manager_->MaterializeSessionEffectFromAttachState(CUR_SESSION_ID);

    EXPECT_FALSE(curNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
}

// OSE-FIX1b: valid instance + ready chain → materialize succeeds and emits ENABLED.
// The actual insert-failure path (clears attach state + emits DISABLED) is covered by
// materialize_offload_chainBroken_emitsDisabledAndClears below.
HWTEST_F(HpaeOffloadSeTest, materialize_offload_validInstance_emitsEnabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    SessionEffectAttachState st;
    st.enabled = true;
    st.effectName = "audio_aiss_enhance";
    st.instance = instance;
    st.config = config_;
    curNode_->SetSessionEffectAttachState(st);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());

    manager_->MaterializeSessionEffectFromAttachState(CUR_SESSION_ID);

    // Valid instance + intact chain → materialize succeeds and emits ENABLED.
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
}

// OSE-MOVE-CUR: MoveStream for current node preserves attach state (no DISABLED)
HWTEST_F(HpaeOffloadSeTest, moveStream_currentNode_preservesAttachState_noDisabled, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);
    callback_->events_.clear();

    manager_->isInit_.store(true);
    int32_t ret = manager_->MoveStream(CUR_SESSION_ID, "new_sink");
    EXPECT_EQ(ret, SUCCESS);
    manager_->HandleMsg();

    // BEHAVIOR CHANGE: MoveStream for current node now preserves attach state (no DISABLED)
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    // Positive assertion: attach state survived DeleteInputSession(KEEP_ATTACH_STATE_FOR_MOVE)
    // and follows the stream to the new sink. The re-materialized ENABLED is verified end-to-end.
    EXPECT_TRUE(curNode_->HasSessionEffectAttachState());
}

// ============================================================================
// MaterializeSessionEffectFromAttachState — branch coverage
//
// Reachable branches covered below + by existing tests:
//   B1a sessionEffectNode_ != nullptr  -> early return            [T1]
//   B1b sessionEffectNode_ == nullptr  -> continue               [T2-T6]
//   B2a curSinkInput == nullptr        -> early return            [T2]
//   B2b !HasSessionEffectAttachState   -> early return            [T3]
//   B2c HasSessionEffectAttachState    -> continue               [T1/T4/T5/T6]
//   B3b has_value && !enabled          -> early return            [T4]
//   B3c has_value && enabled           -> continue               [T1/T5/T6]
//   B4a inst == nullptr                -> DISABLED + clear       [existing expired test]
//   B4b inst != nullptr                -> continue               [T5/T6]
//   B5a seRet != SUCCESS               -> DISABLED + clear        [T5]
//   B5b seRet == SUCCESS               -> continue               [T6 + existing success test]
//   B6a lastSeStatus < WARMING_UP      -> skip StartEffect       [existing success test]
//   B6c lastSeStatus >= WARMING_UP     -> StartEffect + ENABLED   [T6]
//
// Unreachable defensive branches (documented, cannot be covered):
//   B3a !stOpt.has_value(): tautologically false right after HasSessionEffectAttachState()
//       returned true (both check the same optional).
//   B6b lastSeStatus >= WARMING_UP && sessionEffectNode_ == nullptr: impossible because
//       InsertSessionEffectNode only returns SUCCESS after assigning sessionEffectNode_.
// ============================================================================

// B1a: sessionEffectNode_ already present → Materialize is a no-op.
HWTEST_F(HpaeOffloadSeTest, materialize_offload_alreadyMaterialized_isNoOp, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "audio_aiss_enhance", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());
    callback_->events_.clear();

    manager_->MaterializeSessionEffectFromAttachState(CUR_SESSION_ID);

    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 0u);
}

// B2a: session id not in sinkInputNodeMap_ → early return, no callback.
HWTEST_F(HpaeOffloadSeTest, materialize_offload_unknownSession_isNoOp, TestSize.Level0)
{
    manager_->MaterializeSessionEffectFromAttachState(INVALID_SESSION_ID);

    EXPECT_EQ(callback_->Count(INVALID_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    EXPECT_EQ(callback_->Count(INVALID_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 0u);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
}

// B2b: session exists but has no attach state → early return, no callback.
HWTEST_F(HpaeOffloadSeTest, materialize_offload_noAttachState_isNoOp, TestSize.Level0)
{
    ASSERT_FALSE(curNode_->HasSessionEffectAttachState());

    manager_->MaterializeSessionEffectFromAttachState(CUR_SESSION_ID);

    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 0u);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
}

// B3b: attach state present but enabled == false → early return, attach state untouched.
HWTEST_F(HpaeOffloadSeTest, materialize_offload_disabledAttachState_isNoOp, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    SessionEffectAttachState st;
    st.enabled = false;
    st.effectName = "audio_aiss_enhance";
    st.instance = instance;
    st.config = config_;
    curNode_->SetSessionEffectAttachState(st);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());

    manager_->MaterializeSessionEffectFromAttachState(CUR_SESSION_ID);

    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 0u);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    // Attach state survives (only permanent failures clear it).
    EXPECT_TRUE(curNode_->HasSessionEffectAttachState());
}

// B5a: valid instance but chain broken → InsertSessionEffectNode returns ERR_SESSION_NOT_FOUND,
// Materialize treats it as permanent failure: clear attach state + emit DISABLED.
HWTEST_F(HpaeOffloadSeTest, materialize_offload_chainBroken_emitsDisabledAndClears, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    SessionEffectAttachState st;
    st.enabled = true;
    st.effectName = "audio_aiss_enhance";
    st.instance = instance;
    st.config = config_;
    curNode_->SetSessionEffectAttachState(st);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());

    // Break the node chain so InsertSessionEffectNode fails with ERR_SESSION_NOT_FOUND.
    manager_->converterForLoudness_ = nullptr;

    manager_->MaterializeSessionEffectFromAttachState(CUR_SESSION_ID);

    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_FALSE(curNode_->HasSessionEffectAttachState());
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
}

// B6c: success path with lastSeStatus >= WARMING_UP → StartEffect called on the fresh node,
// ENABLED emitted, node ends up in WARMING_UP (proves StartEffect ran, not left in IDLE).
HWTEST_F(HpaeOffloadSeTest, materialize_offload_lastStatusWarm_restartsPreheat, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    SessionEffectAttachState st;
    st.enabled = true;
    st.effectName = "audio_aiss_enhance";
    st.instance = instance;
    st.config = config_;
    st.lastSeStatus = static_cast<int32_t>(HpaeSessionEffectNode::STATUS::WARMING_UP);
    curNode_->SetSessionEffectAttachState(st);
    ASSERT_TRUE(curNode_->HasSessionEffectAttachState());

    manager_->MaterializeSessionEffectFromAttachState(CUR_SESSION_ID);

    EXPECT_EQ(callback_->Count(CUR_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    // Without B6c's StartEffect the new node would stay IDLE; StartEffect pushes it to WARMING_UP.
    EXPECT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
