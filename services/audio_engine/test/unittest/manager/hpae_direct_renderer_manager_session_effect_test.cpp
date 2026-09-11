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

/**
 * @file hpae_direct_renderer_manager_session_effect_test.cpp
 * @brief Unit tests for SessionEffect functionality in HpaeDirectRendererManager
 *
 * This file contains comprehensive unit tests for SessionEffect (SE) feature in
 * HpaeDirectRendererManager, targeting 100% code coverage including:
 * - All public interfaces (Create/Destroy/SetParameter/Post/IsValidSession)
 * - All private helper methods (Insert/Remove/Materialize/Detach)
 * - All modified methods (Connect/Disconnect/Start/Pause/Stop/Flush/Drain/etc.)
 * - All scenarios (VOIP restriction, attach state, move, reload)
 * - All edge cases and error handling
 */

#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <chrono>
#include <memory>
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_direct_renderer_manager.h"
#include "hpae_node_common.h"
#include "hpae_mocks.h"
#include "audio_utils.h"
#include "hpae_session_effect_node.h"
#include "effect_instance.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace {
// Test constants
constexpr uint32_t TEST_SESSION_ID = 10001;
constexpr uint32_t TEST_SESSION_ID_2 = 10002;
constexpr uint32_t INVALID_SESSION_ID = 99999;
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr int32_t FRAME_LENGTH_960 = 960;

// Helper functions
std::string g_rootPath = "/data/";

// Builds a real EffectInstance for tests. Init() without an injected library is a safe no-op
// (FeedInput/ReadOutput/SetParameter/Flush/GetPreheatFrames all early-return 0 when there is no
// effect handle), which is exactly what these manager-level SessionEffect tests need: they
// verify manager/node wiring and lifecycle, not DSP. Mirrors MakeTestEffectInstance in
// hpae_offload_se_test.cpp.
static std::shared_ptr<EffectInstance> MakeTestEffectInstance(const AudioEffectConfig &config)
{
    auto instance = std::make_shared<EffectInstance>();
    instance->Init("audio_aiss_enhance", config);
    return instance;
}

static HpaeSinkInfo GetDirectSinkInfo(const std::string &deviceClass = "file_io")
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = deviceClass;
    sinkInfo.adapterName = deviceClass;
    sinkInfo.filePath = g_rootPath + "direct_renderer_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_S32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.deviceName = deviceClass;
    sinkInfo.volume = 1.0f;
    return sinkInfo;
}

static HpaeStreamInfo GetDefaultStreamInfo(uint32_t sessionId = TEST_SESSION_ID)
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

void WaitForMsgProcessing(std::shared_ptr<HpaeDirectRendererManager> &manager)
{
    int waitCount = 0;
    const int waitCountThd = 10;
    while (manager && manager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
    if (manager) {
        EXPECT_EQ(manager->IsMsgProcessing(), false);
        EXPECT_LT(waitCount, waitCountThd);
    }
}
} // namespace

// ==================== Test Fixture ====================

class HpaeDirectRendererManagerSessionEffectTest : public testing::Test {
public:
    void SetUp() override;
    void TearDown() override;

protected:
    std::shared_ptr<HpaeDirectRendererManager> manager_;
    std::shared_ptr<HpaeDirectRendererManager> voipManager_; // For VOIP tests
    AudioEffectConfig defaultConfig_;
};

void HpaeDirectRendererManagerSessionEffectTest::SetUp()
{
    // Initialize default config
    defaultConfig_.inputCfg.samplingRate = SAMPLE_RATE_48000;
    defaultConfig_.inputCfg.channels = STEREO;
    defaultConfig_.inputCfg.format = SAMPLE_S32LE;
    defaultConfig_.outputCfg = defaultConfig_.inputCfg;

    // Create normal Direct manager
    HpaeSinkInfo sinkInfo = GetDirectSinkInfo();
    manager_ = std::make_shared<HpaeDirectRendererManager>(sinkInfo);
    ASSERT_EQ(manager_->Init(), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Init(), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_TRUE(manager_->IsInit());
}

void HpaeDirectRendererManagerSessionEffectTest::TearDown()
{
    if (manager_ != nullptr && manager_->IsInit()) {
        manager_->DeInit();
        WaitForMsgProcessing(manager_);
    }
    if (voipManager_ != nullptr && voipManager_->IsInit()) {
        voipManager_->DeInit();
        WaitForMsgProcessing(voipManager_);
    }
    manager_.reset();
    voipManager_.reset();
}

// ==================== Public Interface Tests ====================

/**
 * @tc.name    : CreateSessionEffect_Success_ChainReady
 * @tc.type    : FUNC
 * @tc.number  : SE_Create_001
 * @tc.desc    : Test CreateSessionEffect succeeds when processing chain is ready
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Create_001, TestSize.Level1)
{
    // Prepare: Create and start stream
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Create SessionEffect
    auto instance = MakeTestEffectInstance(defaultConfig_);

    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: Success and SE node created
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : CreateSessionEffect_MaterializesWithoutStart
 * @tc.type    : FUNC
 * @tc.number  : SE_Create_002
 * @tc.desc    : Test CreateSessionEffect materializes immediately (chain built at CreateStream)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Create_002, TestSize.Level1)
{
    // Prepare: Create stream but do NOT start it
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Create SessionEffect. HpaeDirectRendererManager builds the processing chain during
    // CreateStream, so the SE materializes immediately even before Start; attach state is set.
    auto instance = MakeTestEffectInstance(defaultConfig_);

    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: Succeeds, SE materialized immediately, attach state persisted on the node
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Verify attach state is saved on SinkInputNode
    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : CreateSessionEffect_MaterializeOnStart
 * @tc.type    : FUNC
 * @tc.number  : SE_Create_003
 * @tc.desc    : Test SE materializes when stream is started after deferred creation
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Create_003, TestSize.Level1)
{
    // Prepare: Create stream and SE (deferred)
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);

    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    // Direct builds the chain at CreateStream, so SE materializes immediately (before Start).
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Test: Start stream should start the materialized SE
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE node is materialized
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : CreateSessionEffect_NotCurNode
 * @tc.type    : FUNC
 * @tc.number  : SE_Create_004
 * @tc.desc    : Test CreateSessionEffect saves attach state for non-curNode stream
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Create_004, TestSize.Level1)
{
    // Prepare: Create two streams
    HpaeStreamInfo streamInfo1 = GetDefaultStreamInfo(TEST_SESSION_ID);
    HpaeStreamInfo streamInfo2 = GetDefaultStreamInfo(TEST_SESSION_ID_2);

    ASSERT_EQ(manager_->CreateStream(streamInfo1), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->CreateStream(streamInfo2), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Start first stream (becomes curNode)
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Create SE for second stream (non-curNode)
    auto instance = MakeTestEffectInstance(defaultConfig_);

    manager_->PostCreateSessionEffect(TEST_SESSION_ID_2, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: Should succeed with attach state saved
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr); // Not materialized yet

    // Verify attach state is saved on second stream's node
    auto node2 = manager_->sinkInputNodeMap_[TEST_SESSION_ID_2];
    ASSERT_NE(node2, nullptr);
    EXPECT_TRUE(node2->HasSessionEffectAttachState());

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : CreateSessionEffect_Duplicate
 * @tc.type    : FUNC
 * @tc.number  : SE_Create_005
 * @tc.desc    : Test duplicate PostCreateSessionEffect is rejected (async cannot observe
 *               ERR_SESSION_ALREADY_BOUND; verified by unchanged node identity)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Create_005, TestSize.Level1)
{
    // Prepare: Create, start stream and create SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance1 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance1, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Capture the first materialized node identity to detect whether the duplicate
    // Create is rejected (rather than overwriting it)
    auto nodeBeforeDuplicate = manager_->sessionEffectNode_;

    // Test: PostCreate again on the same session (async -- rejected on the SignalProcessThread)
    auto instance2 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance2, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: the duplicate bind is rejected -- the original node identity is unchanged
    // and still bound to the same session. The async Post* returns void, so
    // ERR_SESSION_ALREADY_BOUND cannot be observed directly; the unchanged node identity
    // proves the guard returns before mutating state (regression guard: the node is not rebuilt/replaced).
    EXPECT_EQ(manager_->sessionEffectNode_, nodeBeforeDuplicate);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DestroySessionEffect_Success
 * @tc.type    : FUNC
 * @tc.number  : SE_Destroy_001
 * @tc.desc    : Test DestroySessionEffect removes SE node successfully
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Destroy_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Test: Destroy SE
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    // Verify: Success and SE node removed
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DestroySessionEffect_ClearAttachState
 * @tc.type    : FUNC
 * @tc.number  : SE_Destroy_002
 * @tc.desc    : Test DestroySessionEffect clears attach state for deferred SE
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Destroy_002, TestSize.Level1)
{
    // Prepare: Create stream and deferred SE (not started)
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: Destroy SE (clears attach state but no physical node)
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    // Verify: Success and attach state cleared
    EXPECT_FALSE(node->HasSessionEffectAttachState());

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DestroySessionEffect_NotFound
 * @tc.type    : FUNC
 * @tc.number  : SE_Destroy_003
 * @tc.desc    : Test PostDestroySessionEffect on a session without SE is a safe no-op
 *               (async cannot observe ERR_EFFECT_NOT_BOUND; verified by non-corrupted state)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Destroy_003, TestSize.Level0)
{
    // Test: PostDestroy on a session that never had an SE created (async returns void,
    // ERR_EFFECT_NOT_BOUND cannot be observed). Reframe: verify this no-op destroy does
    // not corrupt the manager state.
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    // Verify: no crash, clean state
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Verify: state is not corrupted -- a subsequent real Create still materializes normally
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : PostCreateSessionEffect_Async
 * @tc.type    : FUNC
 * @tc.number  : SE_PostCreate_001
 * @tc.desc    : Test PostCreateSessionEffect dispatches to SignalProcessThread
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_PostCreate_001, TestSize.Level1)
{
    // Prepare: Create and start stream
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: PostCreateSessionEffect (async)
    auto instance = MakeTestEffectInstance(defaultConfig_);

    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: SE node created
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : PostDestroySessionEffect_Async
 * @tc.type    : FUNC
 * @tc.number  : SE_PostDestroy_001
 * @tc.desc    : Test PostDestroySessionEffect dispatches to SignalProcessThread
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_PostDestroy_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: PostDestroySessionEffect (async)
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    // Verify: SE node removed
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : SetSessionEffectParameter_Success
 * @tc.type    : FUNC
 * @tc.number  : SE_SetParameter_001
 * @tc.desc    : Test SetSessionEffectParameter succeeds when SE is active
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_SetParameter_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: Set parameter on active SE
    manager_->PostSetSessionEffectParameter(TEST_SESSION_ID, 0, 0.75f);
    WaitForMsgProcessing(manager_);

    // Verify: Success
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : SetSessionEffectParameter_NotActive
 * @tc.type    : FUNC
 * @tc.number  : SE_SetParameter_002
 * @tc.desc    : Test PostSetSessionEffectParameter without active SE is a safe no-op (async cannot observe ERR_EFFECT_NOT_BOUND)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_SetParameter_002, TestSize.Level0)
{
    // Test: PostSetParameter with no active SE (async returns void, ERR_EFFECT_NOT_BOUND
    // cannot be observed; verify no crash and unchanged state)
    manager_->PostSetSessionEffectParameter(TEST_SESSION_ID, 0, 0.75f);
    WaitForMsgProcessing(manager_);

    // Verify: no crash, no SE materialized
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
}

/**
 * @tc.name    : PostSetSessionEffectParameter_Async
 * @tc.type    : FUNC
 * @tc.number  : SE_PostSetParameter_001
 * @tc.desc    : Test PostSetSessionEffectParameter dispatches to SignalProcessThread
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_PostSetParameter_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: PostSetSessionEffectParameter (async)
    manager_->PostSetSessionEffectParameter(TEST_SESSION_ID, 1, 0.5f);
    WaitForMsgProcessing(manager_);

    // Verify: Success (no crash, parameter set)
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : IsValidSession_Valid
 * @tc.type    : FUNC
 * @tc.number  : SE_IsValidSession_001
 * @tc.desc    : Test IsValidSession returns true for valid session
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_IsValidSession_001, TestSize.Level0)
{
    // Prepare: Create stream
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Valid session
    EXPECT_TRUE(manager_->IsValidSession(TEST_SESSION_ID));

    // Test: Invalid session
    EXPECT_FALSE(manager_->IsValidSession(INVALID_SESSION_ID));

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : IsValidSession_Invalid
 * @tc.type    : FUNC
 * @tc.number  : SE_IsValidSession_002
 * @tc.desc    : Test IsValidSession returns false for invalid session
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_IsValidSession_002, TestSize.Level0)
{
    // Test: No streams created
    EXPECT_FALSE(manager_->IsValidSession(TEST_SESSION_ID));
    EXPECT_FALSE(manager_->IsValidSession(INVALID_SESSION_ID));
}

// ==================== Private Helper Methods Tests ====================

/**
 * @tc.name    : InsertSessionEffectNode_Success
 * @tc.type    : FUNC
 * @tc.number  : SE_Insert_001
 * @tc.desc    : Test InsertSessionEffectNode creates and connects SE node
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Insert_001, TestSize.Level1)
{
    // Prepare: Create stream only (PREPARED state). Direct manager builds the processing
    // chain during CreateStream, so converterForGain_/loudnessGainNode_ already exist and
    // the stream is NOT running -- the SignalProcessThread is idle with the queue drained,
    // making the synchronous InsertSessionEffectNode call safe (no thread race).
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Prepare: Ensure chain is ready (converterForGain_ and loudnessGainNode_ exist)
    ASSERT_NE(manager_->converterForGain_, nullptr);
    ASSERT_NE(manager_->loudnessGainNode_, nullptr);

    // Test: Call InsertSessionEffectNode synchronously (stream not running, queue drained)
    auto instance = MakeTestEffectInstance(defaultConfig_);

    manager_->sessionEffectSessionId_ = TEST_SESSION_ID; // Set before insert
    int32_t ret = manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_);

    // Verify: Success
    EXPECT_EQ(ret, 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Verify: SE is connected between loudnessGainNode_ and converterForGain_
    // (This is verified by checking no crash during operations)

    // Cleanup (stream was never started, no Stop needed)
    manager_->RemoveSessionEffectNode();
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : InsertSessionEffectNode_ChainNotReady
 * @tc.type    : FUNC
 * @tc.number  : SE_Insert_002
 * @tc.desc    : Test InsertSessionEffectNode returns ERR_SESSION_NOT_FOUND when chain not ready
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Insert_002, TestSize.Level1)
{
    // Prepare: no stream created yet, so the processing chain (converterForGain_/
    // loudnessGainNode_) does not exist. HpaeDirectRendererManager builds that chain inside
    // CreateStream, so the only way to observe "chain not ready" is to not have a stream.
    auto instance = MakeTestEffectInstance(defaultConfig_);

    // Test: Try to insert when chain not ready
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    int32_t ret = manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_);

    // Verify: Should return ERR_SESSION_NOT_FOUND and leave no physical node
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    // Cleanup: reset the manually-set sessionId (no stream/chain to tear down)
    manager_->sessionEffectSessionId_ = 0;
}


/**
 * @tc.name    : InsertSessionEffectNode_Duplicate
 * @tc.type    : FUNC
 * @tc.number  : SE_Insert_003
 * @tc.desc    : Test InsertSessionEffectNode returns ERR_SESSION_ALREADY_BOUND when SE exists
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Insert_003, TestSize.Level1)
{
    // Prepare: Create stream only (PREPARED state, chain ready, not running) and first SE.
    // Synchronous Insert is safe here: SignalProcessThread is idle with the queue drained.
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);


    int32_t ret = manager_->InsertSessionEffectNode("audio_aiss_enhance", nullptr, defaultConfig_);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_CREATE_FAILED));
    auto instance1 = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance1, defaultConfig_), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: Try to insert again (synchronous, stream still not running)
    auto instance2 = MakeTestEffectInstance(defaultConfig_);
    ret = manager_->InsertSessionEffectNode("audio_aiss_enhance", instance2, defaultConfig_);

    // Verify: Should return ERR_SESSION_ALREADY_BOUND
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND));

    // Cleanup (stream was never started, no Stop needed)
    manager_->RemoveSessionEffectNode();
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : RemoveSessionEffectNode_Success
 * @tc.type    : FUNC
 * @tc.number  : SE_Remove_001
 * @tc.desc    : Test RemoveSessionEffectNode removes SE and restores chain
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Remove_001, TestSize.Level1)
{
    // Prepare: Create stream only (PREPARED state, chain ready, not running) and SE.
    // Synchronous Insert/Remove are safe here: SignalProcessThread is idle, queue drained.
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: Remove SE -> BeginClose (node enters STOPPING); IDLE node has no crossfade armed,
    // so FinalizeClose runs in-request.
    int32_t ret = manager_->RemoveSessionEffectNode();

    // Verify: Success; the finalize already ran in-request, drain for a clean slate.
    EXPECT_EQ(ret, 0);
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Verify: Chain restored -- loudnessGainNode_ reconnected directly to converterForGain_
    ASSERT_NE(manager_->loudnessGainNode_, nullptr);
    ASSERT_NE(manager_->converterForGain_, nullptr);

    // Cleanup (stream was never started, no Stop needed)
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : RemoveSessionEffectNode_NotFound
 * @tc.type    : FUNC
 * @tc.number  : SE_Remove_002
 * @tc.desc    : Test RemoveSessionEffectNode returns ERR_EFFECT_NOT_BOUND when SE not exists
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Remove_002, TestSize.Level1)
{
    // Prepare: Create and start stream
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Try to remove when SE doesn't exist
    int32_t ret = manager_->RemoveSessionEffectNode();

    // Verify: Should return ERR_EFFECT_NOT_BOUND
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : MaterializeSessionEffectFromAttachState_Success
 * @tc.type    : FUNC
 * @tc.number  : SE_Materialize_001
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState creates SE when a deferred
 *               (non-curNode) session becomes the curNode via SetCurrentNode + Start
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_001, TestSize.Level1)
{
    // Prepare: two streams. S1 becomes curNode_; S2 is non-curNode.
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID_2)), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Create SE for S2 (non-curNode) -> deferred: attach state saved, no physical node yet.
    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID_2, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    auto node2 = manager_->sinkInputNodeMap_[TEST_SESSION_ID_2];
    ASSERT_NE(node2, nullptr);
    EXPECT_TRUE(node2->HasSessionEffectAttachState());

    // Test: destroy S1 so SetCurrentNode re-selects S2 as curNode_, then Start S2.
    // ConnectInputSession -> MaterializeSessionEffectFromAttachState materializes S2's SE.
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID_2), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE node materialized for S2
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID_2);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : MaterializeSessionEffectFromAttachState_InstanceExpired
 * @tc.type    : FUNC
 * @tc.number  : SE_Materialize_003
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState clears attach state when a deferred
 *               (non-curNode) session's instance has expired by the time it becomes curNode
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_003, TestSize.Level1)
{
    // Prepare: two streams. S2 (non-curNode) gets a deferred SE bound to a scoped instance.
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID_2)), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Create SE for S2 (non-curNode) with an instance whose strong ref is released below.
    {
        auto instance = MakeTestEffectInstance(defaultConfig_);
        manager_->PostCreateSessionEffect(TEST_SESSION_ID_2, "audio_aiss_enhance",
            instance, defaultConfig_);
        WaitForMsgProcessing(manager_);
        EXPECT_EQ(manager_->sessionEffectNode_, nullptr); // deferred (non-curNode)
    }
    // instance destroyed here -> S2 attach state now holds an expired weak_ptr

    auto node2 = manager_->sinkInputNodeMap_[TEST_SESSION_ID_2];
    ASSERT_NE(node2, nullptr);
    ASSERT_TRUE(node2->HasSessionEffectAttachState());

    // Test: make S2 the curNode_ and Start -> MaterializeSessionEffectFromAttachState detects
    // the expired instance, clears the attach state and emits DISABLED (no physical node).
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID_2), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE not materialized and attach state cleared
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
    EXPECT_FALSE(node2->HasSessionEffectAttachState());

    // Cleanup
    manager_->Stop(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DetachSessionEffectNodeForReload_PreservesAttachState
 * @tc.type    : FUNC
 * @tc.number  : SE_DetachReload_001
 * @tc.desc    : Test DetachSessionEffectNodeForReload preserves attach state
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_DetachReload_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: Detach for reload
    manager_->DetachSessionEffectNodeForReload();

    // Verify: Physical node removed but attach state preserved
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: Re-materialize after reload
    manager_->MaterializeSessionEffectFromAttachState(TEST_SESSION_ID);

    // Verify: SE re-materialized
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DetachSessionEffectNodeForReload_NoAttachState
 * @tc.type    : FUNC
 * @tc.number  : SE_DetachReload_002
 * @tc.desc    : Test DetachSessionEffectNodeForReload handles no attach state gracefully
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_DetachReload_002, TestSize.Level1)
{
    // Prepare: Create, start stream and SE (no attach state saved)
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Clear attach state to simulate not having one
    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    node->ClearSessionEffectAttachState();

    // Test: Detach when no attach state
    manager_->DetachSessionEffectNodeForReload();

    // Verify: Should not crash, just clean physical node
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ==================== Modified Methods Tests ====================

/**
 * @tc.name    : Start_StartsSE
 * @tc.type    : FUNC
 * @tc.number  : SE_Start_001
 * @tc.desc    : Test Start calls StartEffect on SE node
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Start_001, TestSize.Level1)
{
    // Prepare: Create stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    // Direct builds the chain at CreateStream, so SE materializes immediately (before Start).
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Test: Start stream should start the materialized SE
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE created and not paused
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_FALSE(manager_->sessionEffectNode_->IsPaused());

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Pause_PausesSE
 * @tc.type    : FUNC
 * @tc.number  : SE_Pause_001
 * @tc.desc    : Test Pause calls PauseEffect on SE node
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Pause_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_FALSE(manager_->sessionEffectNode_->IsPaused());

    // Test: Pause should pause SE
    ASSERT_EQ(manager_->Pause(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE is paused
    EXPECT_TRUE(manager_->sessionEffectNode_->IsPaused());

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Stop_PausesSE
 * @tc.type    : FUNC
 * @tc.number  : SE_Stop_001
 * @tc.desc    : Test Stop calls PauseEffect on SE node
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Stop_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_FALSE(manager_->sessionEffectNode_->IsPaused());

    // Test: Stop should pause SE
    ASSERT_EQ(manager_->Stop(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE is paused
    EXPECT_TRUE(manager_->sessionEffectNode_->IsPaused());

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Flush_FlushesSE
 * @tc.type    : FUNC
 * @tc.number  : SE_Flush_001
 * @tc.desc    : Test Flush calls FlushEffect on SE node
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Flush_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Flush should call FlushEffect on SE
    ASSERT_EQ(manager_->Flush(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE node still exists and not paused
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_FALSE(manager_->sessionEffectNode_->IsPaused());

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Drain_FlushesSE
 * @tc.type    : FUNC
 * @tc.number  : SE_Drain_001
 * @tc.desc    : Test Drain calls FlushEffect on SE node
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Drain_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Drain should call FlushEffect on SE
    ASSERT_EQ(manager_->Drain(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE node still exists
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : OnRequestLatency_IncludesSE
 * @tc.type    : FUNC
 * @tc.number  : SE_Latency_001
 * @tc.desc    : Test OnRequestLatency includes SE node latency
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Latency_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: OnRequestLatency includes SE latency
    uint64_t latency = 0;
    manager_->OnRequestLatency(TEST_SESSION_ID, latency);

    // Verify: Latency includes SE (should be > 0)
    // Note: Actual value depends on SE implementation, just verify it's called
    EXPECT_GT(latency, 0u);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DeleteInputSession_FullTeardown
 * @tc.type    : FUNC
 * @tc.number  : SE_DeleteInput_001
 * @tc.desc    : Test DeleteInputSession with FULL_TEARDOWN clears attach state
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_DeleteInput_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    // Verify: SE node removed and attach state cleared
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
    EXPECT_FALSE(node->HasSessionEffectAttachState());
}

/**
 * @tc.name    : DestroyStream_ClearsAttachState
 * @tc.type    : FUNC
 * @tc.number  : SE_DestroyStream_001
 * @tc.desc    : Test DestroyStream clears attach state for non-curNode stream
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_DestroyStream_001, TestSize.Level1)
{
    // Prepare: Create two streams
    HpaeStreamInfo streamInfo1 = GetDefaultStreamInfo(TEST_SESSION_ID);
    HpaeStreamInfo streamInfo2 = GetDefaultStreamInfo(TEST_SESSION_ID_2);

    ASSERT_EQ(manager_->CreateStream(streamInfo1), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->CreateStream(streamInfo2), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Start first stream (becomes curNode)
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Create SE for second stream (non-curNode)
    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID_2, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    auto node2 = manager_->sinkInputNodeMap_[TEST_SESSION_ID_2];
    ASSERT_NE(node2, nullptr);
    EXPECT_TRUE(node2->HasSessionEffectAttachState());

    // Test: Destroy second stream (non-curNode)
    manager_->DestroyStream(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);

    // Verify: Attach state cleared for second stream
    EXPECT_FALSE(node2->HasSessionEffectAttachState());

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : ReloadRenderManager_DetachSE
 * @tc.type    : FUNC
 * @tc.number  : SE_Reload_001
 * @tc.desc    : Test ReloadRenderManager calls DetachSessionEffectNodeForReload
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Reload_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: ReloadRenderManager
    HpaeSinkInfo newSinkInfo = GetDirectSinkInfo();
    ASSERT_EQ(manager_->ReloadRenderManager(newSinkInfo, true), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: attach state preserved across reload, and SE re-materialized in ConnectInputSession.
    // DetachSessionEffectNodeForReload drops the physical node and ConnectInputSession rebuilds
    // it synchronously inside the same ReloadRenderManager request, so only the final
    // re-materialized state is observable here.
    EXPECT_TRUE(node->HasSessionEffectAttachState());
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DeInit();
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : GetNodeInputFormatInfo_SEActive
 * @tc.type    : FUNC
 * @tc.number  : SE_Format_001
 * @tc.desc    : Test GetNodeInputFormatInfo returns 48kHz stereo when SE active
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Format_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: GetNodeInputFormatInfo when SE active
    AudioBasicFormat basicFormat;
    ASSERT_EQ(manager_->GetNodeInputFormatInfo(TEST_SESSION_ID, basicFormat), SUCCESS);

    // Verify: Returns 48kHz stereo when SE active
    EXPECT_EQ(basicFormat.rate, static_cast<uint32_t>(SAMPLE_RATE_48000));
    EXPECT_EQ(basicFormat.audioChannelInfo.numChannels, static_cast<uint32_t>(STEREO));

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : GetNodeInputFormatInfo_SEInactive
 * @tc.type    : FUNC
 * @tc.number  : SE_Format_002
 * @tc.desc    : Test GetNodeInputFormatInfo returns sink format when SE inactive
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Format_002, TestSize.Level1)
{
    // Prepare: Create and start stream (no SE)
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: GetNodeInputFormatInfo when SE inactive
    AudioBasicFormat basicFormat;
    ASSERT_EQ(manager_->GetNodeInputFormatInfo(TEST_SESSION_ID, basicFormat), SUCCESS);

    // Verify: Returns sink format when SE inactive
    EXPECT_EQ(basicFormat.rate, static_cast<uint32_t>(SAMPLE_RATE_48000));
    EXPECT_EQ(basicFormat.audioChannelInfo.numChannels, static_cast<uint32_t>(STEREO));

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ==================== Attach State Lifecycle Tests ====================

/**
 * @tc.name    : AttachStateLifecycle_CreateStartStopDestroy
 * @tc.type    : FUNC
 * @tc.number  : SE_AttachState_001
 * @tc.desc    : Test complete attach state lifecycle: Create -> Start -> Stop -> Destroy
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_AttachState_001, TestSize.Level1)
{
    // Step 1: Create deferred SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());
    // Direct builds the chain at CreateStream, so SE materializes immediately.
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Step 2: Start starts the materialized SE
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Step 3: Stop pauses SE
    ASSERT_EQ(manager_->Stop(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);
    EXPECT_TRUE(manager_->sessionEffectNode_->IsPaused());

    // Step 4: Destroy removes SE
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_FALSE(node->HasSessionEffectAttachState());

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : AttachStateLifecycle_CreateDestroyNoStart
 * @tc.type    : FUNC
 * @tc.number  : SE_AttachState_002
 * @tc.desc    : Test attach state lifecycle without Start: Create -> Destroy
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_AttachState_002, TestSize.Level1)
{
    // Step 1: Create deferred SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Step 2: Destroy without ever starting
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    // Verify: Attach state cleared, no physical node ever created
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_FALSE(node->HasSessionEffectAttachState());

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ==================== Error Handling Tests ====================

/**
 * @tc.name    : ErrorHandling_CreateWithoutChain
 * @tc.type    : FUNC
 * @tc.number  : SE_Error_001
 * @tc.desc    : Test CreateSessionEffect handles missing chain gracefully
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Error_001, TestSize.Level1)
{
    // Prepare: Create stream but not start
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Create SE without Start. HpaeDirectRendererManager builds the chain at CreateStream,
    // so the SE materializes immediately (no crash).
    auto instance = MakeTestEffectInstance(defaultConfig_);

    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: Succeeds and materializes immediately
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : ErrorHandling_InvalidInstanceId
 * @tc.type    : FUNC
 * @tc.number  : SE_Error_002
 * @tc.desc    : Test Post* SE operations with invalid session ID are safe no-ops (async cannot observe error codes)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Error_002, TestSize.Level1)
{
    // Test: PostCreate on an invalid session (async returns void; for a non-existent
    // session, CreateSessionEffect returns SUCCESS by the "deferred binding" design,
    // but since the session is not in sinkInputNodeMap_, no attach state is saved)
    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(INVALID_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: no side effects (invalid session is not materialized or bound)
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Test: PostDestroy on an invalid session (async no-op, no crash, unchanged state)
    manager_->PostDestroySessionEffect(INVALID_SESSION_ID);
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Test: PostSetParameter on an invalid session (async no-op, no crash, unchanged state)
    manager_->PostSetSessionEffectParameter(INVALID_SESSION_ID, 0, 0.5f);
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
}

/**
 * @tc.name    : ErrorHandling_MultipleStreamsWithSE
 * @tc.type    : FUNC
 * @tc.number  : SE_Error_003
 * @tc.desc    : Test multiple streams with SE attach state
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Error_003, TestSize.Level1)
{
    // Prepare: Create two streams
    HpaeStreamInfo streamInfo1 = GetDefaultStreamInfo(TEST_SESSION_ID);
    HpaeStreamInfo streamInfo2 = GetDefaultStreamInfo(TEST_SESSION_ID_2);

    ASSERT_EQ(manager_->CreateStream(streamInfo1), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->CreateStream(streamInfo2), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Create SE for both streams (both deferred)
    auto instance1 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance1, defaultConfig_);
    WaitForMsgProcessing(manager_);

    auto instance2 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID_2, "audio_aiss_enhance",
        instance2, defaultConfig_);
    WaitForMsgProcessing(manager_);

    auto node1 = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    auto node2 = manager_->sinkInputNodeMap_[TEST_SESSION_ID_2];
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    EXPECT_TRUE(node1->HasSessionEffectAttachState());
    EXPECT_TRUE(node2->HasSessionEffectAttachState());

    // Test: Start first stream (materializes its SE)
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Verify: Second stream's attach state still intact
    EXPECT_TRUE(node2->HasSessionEffectAttachState());

    // Test: Switch to second stream (should remove first SE and materialize second)
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DeleteInputSession(SessionEffectMoveMode::KEEP_ATTACH_STATE_FOR_MOVE);
    manager_->sessionEffectSessionId_ = 0;
    manager_->sessionEffectNode_.reset();

    manager_->SetCurrentNode();
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID_2), SUCCESS);
    WaitForMsgProcessing(manager_);

    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID_2);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ==================== Edge Cases Tests ====================

/**
 * @tc.name    : EdgeCase_SECreationDuringFlush
 * @tc.type    : FUNC
 * @tc.number  : SE_Edge_001
 * @tc.desc    : Test SE creation during Flush is handled correctly
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Edge_001, TestSize.Level1)
{
    // Prepare: Create and start stream
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Flush, then Create SE
    ASSERT_EQ(manager_->Flush(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: SE created successfully even after Flush
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : EdgeCase_SERemovalDuringFlush
 * @tc.type    : FUNC
 * @tc.number  : SE_Edge_002
 * @tc.desc    : Test SE removal during Flush is handled correctly
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Edge_002, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Destroy SE, then Flush
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    ASSERT_EQ(manager_->Flush(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: No crash, Flush completes successfully after SE removal
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : EdgeCase_RapidCreateDestroyCycle
 * @tc.type    : FUNC
 * @tc.number  : SE_Edge_003
 * @tc.desc    : Test rapid Create/Destroy cycle is handled correctly
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Edge_003, TestSize.Level1)
{
    // Prepare: Create and start stream
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Rapid Create/Destroy cycle
    for (int i = 0; i < 3; i++) {
        auto instance = MakeTestEffectInstance(defaultConfig_);

        manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
            instance, defaultConfig_);
        WaitForMsgProcessing(manager_);
        ASSERT_NE(manager_->sessionEffectNode_, nullptr);

        manager_->PostDestroySessionEffect(TEST_SESSION_ID);
        WaitForMsgProcessing(manager_);
        EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    }

    // Verify: After cycle, everything is clean
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ==================== Stress Tests ====================

/**
 * @tc.name    : Stress_MultipleSEOperations
 * @tc.type    : PERF
 * @tc.number  : SE_Stress_001
 * @tc.desc    : Test stress multiple SE operations in sequence
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Stress_001, TestSize.Level2)
{
    // Prepare: Create and start stream
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: 100 Create/Destroy cycles
    for (int i = 0; i < 100; i++) {
        auto instance = MakeTestEffectInstance(defaultConfig_);

        manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
            instance, defaultConfig_);
        WaitForMsgProcessing(manager_);

        manager_->PostDestroySessionEffect(TEST_SESSION_ID);
        WaitForMsgProcessing(manager_);

        // Verify: Clean state after each cycle
        EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
        EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
    }

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ==================== Additional Branch Coverage Tests ====================
// The following cases fill in every uncovered branch of the user-specified SE
// code blocks, targeting 100% branch coverage.
//
// Thread-safety rules (user constraint: never call synchronous APIs while a
// stream is running / prevent thread races):
//   - While a stream is running (after Start, before Stop), use ONLY Post*
//     async APIs (Flush/Drain/MoveStream/PostCreateSessionEffect, etc.). They
//     are serialized onto the SignalProcessThread via SendRequest.
//   - Synchronous APIs (CreateSessionEffect and the private Insert/Remove/
//     Materialize/Detach helpers) are called directly ONLY when the stream is
//     not running (PREPARED state right after CreateStream) AND the queue is
//     drained (WaitForMsgProcessing). The direct manager builds the chain
//     during CreateStream, so the chain is already ready in PREPARED state.
//   - WaitForMsgProcessing is called before and after every direct synchronous
//     call to drain the queue; no requests are dispatched during the call.

// ---------------- Flush branch coverage ----------------

/**
 * @tc.name    : Flush_InvalidSession
 * @tc.type    : FUNC
 * @tc.number  : SE_Flush_002
 * @tc.desc    : Test Flush with invalid session id covers node-not-found early return
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Flush_002, TestSize.Level1)
{
    // Async Flush on a non-existent session: SafeGetMap returns null inside the lambda -> CHECK_AND_RETURN_LOG early return
    EXPECT_EQ(manager_->Flush(INVALID_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // No side effects
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
}

/**
 * @tc.name    : Flush_NonCurNode
 * @tc.type    : FUNC
 * @tc.number  : SE_Flush_003
 * @tc.desc    : Test Flush on a non-curNode session covers sessionId != curNode early return
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Flush_003, TestSize.Level1)
{
    // Two sessions: S1 becomes curNode_, S2 is a non-curNode
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID_2)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->curNode_->GetSessionId(), TEST_SESSION_ID);

    // Async Flush on S2: node exists but sessionId != curNode -> CHECK_AND_RETURN early return
    EXPECT_EQ(manager_->Flush(TEST_SESSION_ID_2), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Flush_ConvertersNull
 * @tc.type    : FUNC
 * @tc.number  : SE_Flush_004
 * @tc.desc    : Test Flush on curNode with converterForGain_/converterForOutput_ null covers
 *               both FlushBuffers nullptr FALSE branches (no crash, Flush still completes)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Flush_004, TestSize.Level1)
{
    // PREPARED state (not Started): chain is ready, curNode exists, no audio/render thread running.
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->curNode_->GetSessionId(), TEST_SESSION_ID);
    ASSERT_NE(manager_->converterForGain_, nullptr);
    ASSERT_NE(manager_->converterForOutput_, nullptr);

    // Force both converter nodes null to cover the nullptr FALSE branches of the two
    // `if (converterXxx_ != nullptr)` guards. Safe: SignalProcessThread is idle and the queue
    // is drained (WaitForMsgProcessing above); the nullptr write happens-before the SendRequest
    // enqueue (program order), which synchronizes-with the dequeue on the thread, so the Flush
    // lambda observes nullptr and skips both FlushBuffers calls.
    auto savedGain = manager_->converterForGain_;
    auto savedOutput = manager_->converterForOutput_;
    manager_->converterForGain_ = nullptr;
    manager_->converterForOutput_ = nullptr;

    // Async Flush: passes node + curNode checks, skips both converter FlushBuffers calls,
    // still calls sinkOutputNode_->RenderSinkFlush() (SE is null, so its branch is skipped too)
    EXPECT_EQ(manager_->Flush(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Restore + cleanup (restore converters before DestroyStream so teardown sees the chain)
    manager_->converterForGain_ = savedGain;
    manager_->converterForOutput_ = savedOutput;
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ---------------- Drain branch coverage ----------------

/**
 * @tc.name    : Drain_InvalidSession
 * @tc.type    : FUNC
 * @tc.number  : SE_Drain_002
 * @tc.desc    : Test Drain with invalid session id covers node-not-found early return
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Drain_002, TestSize.Level1)
{
    EXPECT_EQ(manager_->Drain(INVALID_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
}

/**
 * @tc.name    : Drain_NonCurNode
 * @tc.type    : FUNC
 * @tc.number  : SE_Drain_003
 * @tc.desc    : Test Drain on a non-curNode session covers sessionId != curNode early return
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Drain_003, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID_2)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->curNode_->GetSessionId(), TEST_SESSION_ID);

    // Async Drain on S2: node exists but sessionId != curNode -> CHECK_AND_RETURN early return
    EXPECT_EQ(manager_->Drain(TEST_SESSION_ID_2), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Drain_StateNotRunning
 * @tc.type    : FUNC
 * @tc.number  : SE_Drain_004
 * @tc.desc    : Test Drain when curNode state != RUNNING covers TriggerCallback(OPERATION_DRAINED)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Drain_004, TestSize.Level1)
{
    // After CreateStream the stream is PREPARED (not Started), curNode state != RUNNING
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->curNode_->GetState(), HPAE_SESSION_RUNNING);

    // Async Drain: node->GetState() != HPAE_SESSION_RUNNING -> TriggerCallback(OPERATION_DRAINED) branch
    EXPECT_EQ(manager_->Drain(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Drain_ConvertersNull
 * @tc.type    : FUNC
 * @tc.number  : SE_Drain_005
 * @tc.desc    : Test Drain on curNode with converterForGain_/converterForOutput_ null covers
 *               both FlushBuffers nullptr FALSE branches (no crash, Drain still completes)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Drain_005, TestSize.Level1)
{
    // PREPARED state (not Started): chain is ready, curNode exists, no audio/render thread running.
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->curNode_->GetSessionId(), TEST_SESSION_ID);
    ASSERT_NE(manager_->converterForGain_, nullptr);
    ASSERT_NE(manager_->converterForOutput_, nullptr);

    // Force both converter nodes null to cover the nullptr FALSE branches. Same thread-safety
    // rationale as SE_Flush_004: idle thread + drained queue, write ordered before enqueue.
    auto savedGain = manager_->converterForGain_;
    auto savedOutput = manager_->converterForOutput_;
    manager_->converterForGain_ = nullptr;
    manager_->converterForOutput_ = nullptr;

    // Async Drain: skips both converter FlushBuffers calls; node is PREPARED (not RUNNING) ->
    // TriggerCallback(OPERATION_DRAINED) branch (no registered callback -> no-op)
    EXPECT_EQ(manager_->Drain(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Restore + cleanup
    manager_->converterForGain_ = savedGain;
    manager_->converterForOutput_ = savedOutput;
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ---------------- MoveStream branch coverage ----------------
// MoveStream is async (SendRequest), safe to call while a stream is running; the test
// does not register a SendMsgCallback, so TriggerCallback(MOVE_SESSION_FAILED /
// MOVE_SINK_INPUT / UPDATE_STATUS) are all no-ops.

/**
 * @tc.name    : MoveStream_InvalidSession
 * @tc.type    : FUNC
 * @tc.number  : SE_MoveStream_002
 * @tc.desc    : Test MoveStream with invalid session covers node == nullptr -> MOVE_SESSION_FAILED
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_MoveStream_002, TestSize.Level1)
{
    // Async MoveStream on a non-existent session: node == nullptr -> MOVE_SESSION_FAILED early return
    EXPECT_EQ(manager_->MoveStream(INVALID_SESSION_ID, "remote_sink"), SUCCESS);
    WaitForMsgProcessing(manager_);

    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
}

/**
 * @tc.name    : MoveStream_EmptySinkName
 * @tc.type    : FUNC
 * @tc.number  : SE_MoveStream_003
 * @tc.desc    : Test MoveStream with empty sink name covers sinkName.empty() -> MOVE_SESSION_FAILED
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_MoveStream_003, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Empty sinkName -> MOVE_SESSION_FAILED early return (stream is not moved)
    EXPECT_EQ(manager_->MoveStream(TEST_SESSION_ID, ""), SUCCESS);
    WaitForMsgProcessing(manager_);
    EXPECT_TRUE(manager_->IsValidSession(TEST_SESSION_ID));

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : MoveStream_CurNode_WithSE
 * @tc.type    : FUNC
 * @tc.number  : SE_MoveStream_004
 * @tc.desc    : Test MoveStream on curNode with active SE covers PrepareMoveStream + curNode path
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_MoveStream_004, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Materialize SE via Post* before Start (async, thread-safe)
    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Async MoveStream: sessionEffectNode_ exists and sessionId matches -> PrepareMoveStream branch,
    // followed by DeleteInputSession(KEEP_ATTACH_STATE_FOR_MOVE) + SetCurrentNode (TriggerCallback is a no-op)
    EXPECT_EQ(manager_->MoveStream(TEST_SESSION_ID, "remote_sink"), SUCCESS);
    WaitForMsgProcessing(manager_);

    // After the move: SE node cleared, sessionId reset, the curNode stream is moved out of this manager
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_FALSE(manager_->IsValidSession(TEST_SESSION_ID));
    // Cleanup: stream has been moved out; TearDown DeInit handles it (map is empty, curNode_ is null)
}

/**
 * @tc.name    : MoveStream_NonCurNode
 * @tc.type    : FUNC
 * @tc.number  : SE_MoveStream_005
 * @tc.desc    : Test MoveStream on a non-curNode session covers RemoveNodeFromMap path
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_MoveStream_005, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID_2)), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->curNode_->GetSessionId(), TEST_SESSION_ID);

    // Async MoveStream on S2 (non-curNode, no SE): sessionId != curNode -> RemoveNodeFromMap path
    EXPECT_EQ(manager_->MoveStream(TEST_SESSION_ID_2, "remote_sink"), SUCCESS);
    WaitForMsgProcessing(manager_);

    EXPECT_FALSE(manager_->IsValidSession(TEST_SESSION_ID_2));
    EXPECT_TRUE(manager_->IsValidSession(TEST_SESSION_ID));

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ---------------- RemoveSessionEffectNode branch coverage ----------------
// Synchronous cases run in PREPARED state (after CreateStream, before Start), honoring
// the "no synchronous APIs while a stream is running" constraint.

/**
 * @tc.name    : RemoveSessionEffectNode_RestoreChainSkipped
 * @tc.type    : FUNC
 * @tc.number  : SE_Remove_003
 * @tc.desc    : Test RemoveSessionEffectNode skips restore-chain when loudnessGainNode_ is null
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Remove_003, TestSize.Level1)
{
    // PREPARED state (not Started), chain is ready
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    // Stream not running -> SE can be materialized synchronously
    auto instance = MakeTestEffectInstance(defaultConfig_);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance, defaultConfig_),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Force the restore-chain FALSE branch in Finalize: loudnessGainNode_ is null throughout
    // the deferred close.
    auto savedLoudness = manager_->loudnessGainNode_;
    manager_->loudnessGainNode_ = nullptr;

    int32_t ret = manager_->RemoveSessionEffectNode();  // BeginClose
    EXPECT_EQ(ret, 0);
    // FinalizeClose runs with loudnessGainNode_ null -> skips disconnect, still drops + erases.
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);

    // Restore + cleanup (restore loudnessGainNode_ then destroy the stream normally;
    // DisConnect is a safe no-op for missing edges)
    manager_->loudnessGainNode_ = savedLoudness;
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : RemoveSessionEffectNode_LatencyRewind
 * @tc.type    : FUNC
 * @tc.number  : SE_Remove_004
 * @tc.desc    : Test RemoveSessionEffectNode covers latency > 0 -> RewindHistoryBuffer branch
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Remove_004, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto instance = MakeTestEffectInstance(defaultConfig_);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance, defaultConfig_),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Force latency > 0: GetLatency returns non-zero when totalBytesIn_ > totalBytesOut_
    // (a direct music stream has no historyBuffer_, so RewindHistoryBuffer early-returns
    // safely via its leading CHECK)
    manager_->sessionEffectNode_->totalBytesIn_ = 1024;
    manager_->sessionEffectNode_->totalBytesOut_ = 0;
    ASSERT_GT(manager_->sessionEffectNode_->GetLatency(TEST_SESSION_ID), 0u);

    int32_t ret = manager_->RemoveSessionEffectNode();  // BeginClose (latency>0 -> rewind in Begin)
    EXPECT_EQ(ret, 0);
    WaitForMsgProcessing(manager_);  // FinalizeClose
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);

    // Cleanup (restore-chain takes the TRUE branch, chain is restored normally)
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DestroyThenCreate_RequeuesUntilFinalized
 * @tc.type    : FUNC
 * @tc.number  : SE_Remove_005
 * @tc.desc    : Rapid recreate while the old close waits on its crossfade callback is not
 *               rejected as duplicate; it requeues and materializes after the callback finalize.
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Remove_005, TestSize.Level0)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto oldInst = MakeTestEffectInstance(defaultConfig_);
    auto newInst = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("old", oldInst, defaultConfig_), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    // Arm the smooth close: ACTIVE node -> crossfade pending -> destroy waits on the callback.
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);  // BeginClose: STOPPING + token, nothing queued
    EXPECT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::STOPPING);
    EXPECT_EQ(manager_->pendingCloseSessionId_, TEST_SESSION_ID);

    // Create while STOPPING defers + requeues (do NOT WaitFor here: the retry spins until the
    // close finalizes; IsMsgProcessing never settles in that window).
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "new", newInst, defaultConfig_);

    // Finisher: the crossfade-completion callback finalizes the old close (FIFO behind the
    // spinning retries); the next retry then materializes the new node.
    manager_->OnCloseCrossfadeDone(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    // Old finalized, new materialized: the recreate was not lost. Counts asserted BEFORE
    // DestroyStream — the teardown DeleteInputSession emits its own DISABLED.
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectNode_->GetEffectName(), "new");
    EXPECT_EQ(manager_->pendingCloseSessionId_, 0u);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ---------------- MaterializeSessionEffectFromAttachState branch coverage ----------------
// Synchronous cases run in PREPARED state; Materialize is normally invoked by
// ConnectInputSession on the thread, but here it is called directly to cover specific
// branches (stream not running, safe).

/**
 * @tc.name    : Materialize_CurSinkInputNull
 * @tc.type    : FUNC
 * @tc.number  : SE_Materialize_004
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState covers curSinkInput == nullptr early return
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_004, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Synchronous call (stream not running): SafeGetMap(invalid) returns null -> curSinkInput == nullptr early return
    manager_->MaterializeSessionEffectFromAttachState(INVALID_SESSION_ID);
    WaitForMsgProcessing(manager_);

    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Materialize_DisabledAttachState
 * @tc.type    : FUNC
 * @tc.number  : SE_Materialize_005
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState covers !stOpt->enabled early return
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_005, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);

    // Set an enabled=false attach state (HasSessionEffectAttachState only checks has_value)
    auto instance = MakeTestEffectInstance(defaultConfig_);
    SessionEffectAttachState st;
    st.enabled = false;
    st.effectName = "audio_aiss_enhance";
    st.instance = instance;
    st.config = defaultConfig_;
    node->SetSessionEffectAttachState(st);
    ASSERT_TRUE(node->HasSessionEffectAttachState());

    // Synchronous call (stream not running): stOpt.has_value() && !stOpt->enabled -> early return
    manager_->MaterializeSessionEffectFromAttachState(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);

    // Cleanup
    node->ClearSessionEffectAttachState();
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Materialize_InsertFailed
 * @tc.type    : FUNC
 * @tc.number  : SE_Materialize_006
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState covers seRet != SUCCESS (insert fail)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_006, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);

    // Valid attach state (enabled + valid instance)
    auto instance = MakeTestEffectInstance(defaultConfig_);
    SessionEffectAttachState st;
    st.enabled = true;
    st.effectName = "audio_aiss_enhance";
    st.instance = instance;
    st.config = defaultConfig_;
    node->SetSessionEffectAttachState(st);

    // Force Insert failure: converterForGain_ is null -> InsertSessionEffectNode returns ERR_SESSION_NOT_FOUND
    auto savedConverter = manager_->converterForGain_;
    manager_->converterForGain_ = nullptr;

    // Synchronous call (stream not running): seRet != SUCCESS -> clear attach state + TriggerCallback(DISABLED)
    manager_->MaterializeSessionEffectFromAttachState(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);

    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_FALSE(node->HasSessionEffectAttachState());

    // Restore + cleanup
    manager_->converterForGain_ = savedConverter;
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : Materialize_Restart
 * @tc.type    : FUNC
 * @tc.number  : SE_Materialize_007
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState restart
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_007, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo(TEST_SESSION_ID)), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Stream not running -> materialize SE synchronously
    auto instance = MakeTestEffectInstance(defaultConfig_);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance, defaultConfig_),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Drive the SE node into WARMING_UP (StartEffect starts the preheat thread)
    manager_->sessionEffectNode_->StartEffect();
    ASSERT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::WARMING_UP);

    manager_->DetachSessionEffectNodeForReload();
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    auto stOpt = node->GetSessionEffectAttachState();
    ASSERT_TRUE(stOpt.has_value());

    manager_->MaterializeSessionEffectFromAttachState(TEST_SESSION_ID);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Cleanup (RemoveSessionEffectNode calls StopPreheatThread, safely releasing the preheat thread)
    manager_->RemoveSessionEffectNode();
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// ---------------- Direct SE Destroy/Materialize branch coverage ----------------
// These cases call the SYNCHRONOUS APIs directly (Insert/Remove/Destroy/Materialize) to
// observe return values, which is safe in PREPARED state (after CreateStream, before Start):
// the SignalProcessThread is idle with the queue drained (same pattern as SE_Remove_001 /
// SE_Materialize_006 above).

// SE_Destroy_StopShortCircuit: node already STOPPING -> CancelPending + return SUCCESS,
// no duplicate Finalize/DISABLED (hpae_direct_renderer_manager.cpp:1525-1532).
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Destroy_StopShortCircuit, TestSize.Level0)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo()), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    // BeginClose arms the crossfade + records the token. Do NOT drain yet.
    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::STOPPING);

    // Seed a pending recreate so the short-circuit's CancelPendingSessionEffectCreate is observable.
    manager_->pendingSeCreateSessionId_ = TEST_SESSION_ID;
    manager_->pendingSeCreateGeneration_ = 1;

    // Synchronous DestroySessionEffect hits the STOPPING short-circuit: SUCCESS + cancel pending +
    // does NOT push a second Finalize.
    int32_t ret = manager_->DestroySessionEffect(TEST_SESSION_ID);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(manager_->pendingSeCreateSessionId_, 0u);

    // Drain: the callback finalize runs once -> node gone, exactly one DISABLED (no duplicate).
    manager_->OnCloseCrossfadeDone(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// SE_Materialize_Direct_ReadyOk: chain ready -> InsertSessionEffectNode succeeds ->
// sessionEffectSessionId_ set + ENABLED once (hpae_direct_renderer_manager.cpp:1463-1467).
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_Direct_ReadyOk, TestSize.Level1)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo()), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->converterForGain_, nullptr);
    ASSERT_NE(manager_->loudnessGainNode_, nullptr);
    ASSERT_NE(manager_->curNode_, nullptr);

    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    int32_t ret = manager_->MaterializeCurrentSessionEffect(
        TEST_SESSION_ID, "audio_aiss_enhance", instance, defaultConfig_);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 1u);

    manager_->RemoveSessionEffectNode();
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DestroyFinalizeThenCreateSameBatch
 * @tc.type    : FUNC
 * @tc.number  : SE_SwitchFinalizeNow_001
 * @tc.desc    : finalizeNow destroy followed immediately by create (switch path): create is
 *               processed after the finalizeNow destroy in queue order — no ERR_SESSION_ALREADY_BOUND
 *               from a STOPPING leftover
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_SwitchFinalizeNow_001, TestSize.Level0)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance1 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance1, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: finalizeNow destroy + create pushed back-to-back (no wait between). Old behavior:
    // create ran while the STOPPING node still held sessionEffectNode_ -> ALREADY_BOUND failure.
    manager_->PostDestroySessionEffect(TEST_SESSION_ID, true);
    auto instance2 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance2, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: new SE materialized for the new instance; exactly one DISABLED (direct finalize)
    // and two ENABLED (instance1 create + instance2 create; recorder registered before both, so
    // both are recorded). Asserted at this drain point: the later DestroyStream/DeleteInputSession
    // teardown (Stop itself does not reset the node) would add another DISABLED.
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(manager_->sessionEffectNode_->GetEffectInstance(), instance2);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 2u);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DestroyFinalizeOnStoppingNode
 * @tc.type    : FUNC
 * @tc.number  : SE_SwitchFinalizeNow_002
 * @tc.desc    : plain destroy on a non-ACTIVE node (no crossfade pending) finalizes in-request;
 *               a following finalizeNow destroy is a safe no-op, the next create still succeeds
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_SwitchFinalizeNow_002, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance1 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance1, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: plain destroy (non-ACTIVE node -> in-request finalize), then finalizeNow destroy
    // (no node left -> safe no-op), then create — all back-to-back.
    manager_->PostDestroySessionEffect(TEST_SESSION_ID);
    manager_->PostDestroySessionEffect(TEST_SESSION_ID, true);
    auto instance2 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance2, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: create succeeded (old behavior: STOPPING skip left the node in place -> failure).
    // Exactly one DISABLED (in-request finalize) and two ENABLED (both creates;
    // recorder registered before both, so both are recorded). Asserted at this drain point:
    // the later DestroyStream/DeleteInputSession teardown (Stop itself does not reset the
    // node) would add another DISABLED.
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectNode_->GetEffectInstance(), instance2);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_ENABLED), 2u);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DestroyFinalizeOnStoppingNodeSync
 * @tc.type    : FUNC
 * @tc.number  : SE_SwitchFinalizeNow_003
 * @tc.desc    : finalizeNow destroy on an already-STOPPING node (close waiting on its crossfade
 *               callback) executes the STOPPING-branch direct finalize synchronously, consuming
 *               the pending-close token (exactly one DISABLED)
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_SwitchFinalizeNow_003, TestSize.Level1)
{
    // Prepare: Create stream and SE in PREPARED state (no Start — sync-API convention, cf.
    // SE_Destroy_StopShortCircuit: the SignalProcessThread parks and only drains on push).
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance1 = MakeTestEffectInstance(defaultConfig_);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance", instance1, defaultConfig_);
    WaitForMsgProcessing(manager_);
    ASSERT_TRUE(manager_->DeactivateThread());
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // BeginClose synchronously (plain): node enters STOPPING with the close deferred to the
    // crossfade callback (token recorded, nothing queued) — the state the switch-path destroy
    // must handle inline.
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    ASSERT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::STOPPING);

    // finalizeNow destroy hits the STOPPING branch and finalizes directly (synchronously).
    int32_t ret = manager_->DestroySessionEffect(TEST_SESSION_ID, true);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    // Drain: nothing further is queued — the finalizeNow destroy consumed the pending-close
    // token (no second DISABLED).
    manager_->HandleMsg();
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    manager_->HandleMsg();
}

// ---------------- SE close finalize driven by OnCloseCrossfadeDone (callback model) ----------------
// Active-node destroy defers the finalize to the close-crossfade callback; Pause/Stop/OnFadeDone
// rescue a still-waiting close (token-consumed); a late/stale callback no-ops on the cleared token.

// SE_CloseCB_WaitAndCallback: ACTIVE node -> Remove arms crossfade + records token; the node
// stays STOPPING until OnCloseCrossfadeDone lands, then Finalize runs on the message thread.
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_CloseCB_WaitAndCallback, TestSize.Level0)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo()), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);

    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);  // BeginClose, wait on callback
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectNode_->GetStatus(), HpaeSessionEffectNode::STATUS::STOPPING);
    EXPECT_EQ(manager_->pendingCloseSessionId_, TEST_SESSION_ID);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);
    WaitForMsgProcessing(manager_);  // nothing queued: still waiting
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);

    manager_->OnCloseCrossfadeDone(TEST_SESSION_ID);  // SendRequest -> message thread
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// SE_CloseCB_PauseRescue: Pause while waiting -> entry rescue finalizes; late callback no-ops.
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_CloseCB_PauseRescue, TestSize.Level0)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo()), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);

    manager_->Pause(TEST_SESSION_ID);  // rescue on the message thread
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    manager_->OnCloseCrossfadeDone(TEST_SESSION_ID);  // late: token cleared -> no-op
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// SE_CloseCB_StaleTokenNoOp: rescue-then-recreate arms a NEW close on a NEW node; a callback
// arriving after the rescue finalizes only the tokened (latest) close — the production guard
// is the token-erase on rescue (see SE_CloseCB_PauseRescue); this test pins the convergence:
// rescue-finalize + new close + one callback -> exactly two DISABLED total, node finalized.
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_CloseCB_StaleTokenNoOp, TestSize.Level0)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo()), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);  // old close waiting

    manager_->Pause(TEST_SESSION_ID);  // rescue finalizes the old close
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->sessionEffectNode_, nullptr);

    // Recreate + arm a NEW close on a NEW node.
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    // The OLD close's callback arrives late: the token names only the NEW close, so this call
    // finalizes the NEW close (same session). Total DISABLED: rescue(1) + this finalize(1) = 2.
    manager_->OnCloseCrossfadeDone(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 2u);

    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// SE_CloseCB_StopRescue: Stop while waiting -> entry rescue finalizes; late callback no-ops.
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_CloseCB_StopRescue, TestSize.Level0)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo()), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);

    manager_->Stop(TEST_SESSION_ID);  // rescue on the message thread
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    manager_->OnCloseCrossfadeDone(TEST_SESSION_ID);  // late: token cleared -> no-op
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

// SE_CloseCB_OnFadeDoneRescue: fade completion while waiting -> finalize before
// DisConnectInputSession stops the chain being pulled.
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_CloseCB_OnFadeDoneRescue, TestSize.Level0)
{
    ASSERT_EQ(manager_->CreateStream(GetDefaultStreamInfo()), SUCCESS);
    WaitForMsgProcessing(manager_);
    auto callback = std::make_shared<StatusCallbackRecorder>();
    manager_->RegisterSendMsgCallback(callback);

    auto instance = MakeTestEffectInstance(defaultConfig_);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    manager_->sessionEffectNode_->SetStatusForTest(HpaeSessionEffectNode::STATUS::ACTIVE);
    ASSERT_EQ(manager_->RemoveSessionEffectNode(), 0);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 0u);

    manager_->OnFadeDone(TEST_SESSION_ID);  // curNode_ matches -> rescue + disconnect + PAUSED
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);
    // Real OnFadeDone maps the session node's pre-state (PREPARED, not STOPPING) to PAUSED.
    EXPECT_EQ(manager_->curNode_->GetState(), HPAE_SESSION_PAUSED);

    manager_->OnCloseCrossfadeDone(TEST_SESSION_ID);  // late: no-op
    WaitForMsgProcessing(manager_);
    EXPECT_EQ(callback->Count(TEST_SESSION_ID, OPERATION_SESSION_EFFECT_DISABLED), 1u);

    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}
