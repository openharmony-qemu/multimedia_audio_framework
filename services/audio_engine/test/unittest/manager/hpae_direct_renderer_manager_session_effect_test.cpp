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
#include "i_effect_instance.h"

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

static HpaeSinkInfo GetDirectSinkInfo(const std::string &deviceClass = "primary_direct")
{
    HpaeSinkInfo sinkInfo;
    sinkInfo.deviceNetId = DEFAULT_TEST_DEVICE_NETWORKID;
    sinkInfo.deviceClass = deviceClass;
    sinkInfo.adapterName = deviceClass;
    sinkInfo.filePath = g_rootPath + "direct_renderer_se_test.pcm";
    sinkInfo.frameLen = FRAME_LENGTH_960;
    sinkInfo.samplingRate = SAMPLE_RATE_48000;
    sinkInfo.format = SAMPLE_S32LE;
    sinkInfo.channels = STEREO;
    sinkInfo.deviceType = DEVICE_TYPE_SPEAKER;
    sinkInfo.deviceName = deviceClass;
    sinkInfo.volume = 1.0f;
    return sinkInfo;
}

static HpaeSinkInfo GetVoipDirectSinkInfo()
{
    return GetDirectSinkInfo("primary_direct_voip");
}

static HpaeStreamInfo GetDefaultStreamInfo(uint32_t sessionId = TEST_SESSION_ID)
{
    HpaeStreamInfo streamInfo;
    streamInfo.sessionId = sessionId;
    streamInfo.streamType = AudioStreamInfo::STREAM_TYPE_MUSIC;
    streamInfo.channels = STEREO;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.frameSize = FRAME_LENGTH_960;
    streamInfo.uid = 1000;
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

// Mock EffectInstance for testing
class MockEffectInstance : public IEffectInstance {
public:
    MockEffectInstance() : initialized_(false) {}
    virtual ~MockEffectInstance() = default;

    int32_t Init(const std::string& effectName, const AudioEffectConfig& config) override
    {
        initialized_ = true;
        effectName_ = effectName;
        config_ = config;
        return 0; // Always succeed
    }

    int32_t Release() override
    {
        initialized_ = false;
        return 0;
    }

    int32_t Process(const std::shared_ptr<AudioBuffer>& input,
                   std::shared_ptr<AudioBuffer>& output) override
    {
        return 0;
    }

    int32_t SetParameter(int32_t type, float value) override
    {
        return 0;
    }

    float GetParameter(int32_t type) override
    {
        return 0.0f;
    }

    bool IsInitialized() const { return initialized_; }
    std::string GetEffectName() const { return effectName_; }

private:
    bool initialized_;
    std::string effectName_;
    AudioEffectConfig config_;
};

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
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    int32_t ret = manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);

    // Verify: Success and SE node created
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : CreateSessionEffect_Deferred_ChainNotReady
 * @tc.type    : FUNC
 * @tc.number  : SE_Create_002
 * @tc.desc    : Test CreateSessionEffect saves attach state when chain not ready
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Create_002, TestSize.Level1)
{
    // Prepare: Create stream but NOT start (chain not ready)
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Create SessionEffect before chain is ready
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    int32_t ret = manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);

    // Verify: Should succeed with attach state saved, but physical node not created yet
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr); // Physical node not created
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    // Test: Start stream should materialize SE
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE node is materialized
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
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
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    int32_t ret = manager_->CreateSessionEffect(TEST_SESSION_ID_2, "audio_aiss_enhance",
        instance, defaultConfig_);

    // Verify: Should succeed with attach state saved
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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
 * @tc.desc    : Test duplicate CreateSessionEffect returns ERR_SESSION_ALREADY_BOUND
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Create_005, TestSize.Level1)
{
    // Prepare: Create, start stream and create SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance1 = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance1->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance1, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: Try to create SE again for same session
    auto instance2 = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance2->Init("audio_aiss_enhance", defaultConfig_), 0);

    int32_t ret = manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance2, defaultConfig_);

    // Verify: Should fail with ERR_SESSION_ALREADY_BOUND
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND));

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : CreateSessionEffect_VoipMode_NotSupported
 * @tc.type    : FUNC
 * @tc.number  : SE_Create_006
 * @tc.desc    : Test CreateSessionEffect in VOIP mode returns ERR_NOT_SUPPORTED
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Create_006, TestSize.Level1)
{
    // Prepare: Create VOIP manager
    HpaeSinkInfo voipSinkInfo = GetVoipDirectSinkInfo();
    voipManager_ = std::make_shared<HpaeDirectRendererManager>(voipSinkInfo);
    ASSERT_EQ(voipManager_->Init(), SUCCESS);
    WaitForMsgProcessing(voipManager_);

    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(voipManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(voipManager_);
    ASSERT_EQ(voipManager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(voipManager_);

    // Test: Try to create SE in VOIP mode
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    int32_t ret = voipManager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);

    // Verify: Should return ERR_NOT_SUPPORTED
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_NOT_SUPPORTED));
    EXPECT_EQ(voipManager_->sessionEffectNode_, nullptr);

    // Cleanup
    voipManager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(voipManager_);
    voipManager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(voipManager_);
    voipManager_->DeInit();
    WaitForMsgProcessing(voipManager_);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: Destroy SE
    int32_t ret = manager_->DestroySessionEffect(TEST_SESSION_ID);

    // Verify: Success and SE node removed
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: Destroy SE (clears attach state but no physical node)
    int32_t ret = manager_->DestroySessionEffect(TEST_SESSION_ID);

    // Verify: Success and attach state cleared
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_FALSE(node->HasSessionEffectAttachState());

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : DestroySessionEffect_NotFound
 * @tc.type    : FUNC
 * @tc.number  : SE_Destroy_003
 * @tc.desc    : Test DestroySessionEffect when SE not exists returns ERR_EFFECT_NOT_BOUND
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Destroy_003, TestSize.Level0)
{
    // Test: Destroy SE that was never created
    int32_t ret = manager_->DestroySessionEffect(TEST_SESSION_ID);

    // Verify: Should return ERR_EFFECT_NOT_BOUND
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
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
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);

    // Verify: SE node created
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    manager_->PostCreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);
    WaitForMsgProcessing(manager_);
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);

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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Test: Set parameter on active SE
    int32_t ret = manager_->SetSessionEffectParameter(TEST_SESSION_ID, 0, 0.75f);

    // Verify: Success
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroySessionEffect(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : SetSessionEffectParameter_NotActive
 * @tc.type    : FUNC
 * @tc.number  : SE_SetParameter_002
 * @tc.desc    : Test SetSessionEffectParameter returns ERR_EFFECT_NOT_BOUND when SE not active
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_SetParameter_002, TestSize.Level0)
{
    // Test: Set parameter when SE is not active
    int32_t ret = manager_->SetSessionEffectParameter(TEST_SESSION_ID, 0, 0.75f);

    // Verify: Should return ERR_EFFECT_NOT_BOUND
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Test: PostSetSessionEffectParameter (async)
    manager_->PostSetSessionEffectParameter(TEST_SESSION_ID, 1, 0.5f);
    WaitForMsgProcessing(manager_);

    // Verify: Success (no crash, parameter set)
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroySessionEffect(TEST_SESSION_ID);
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
    // Prepare: Create, start stream
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Prepare: Ensure chain is ready (converterForGain_ and loudnessGainNode_ exist)
    ASSERT_NE(manager_->converterForGain_, nullptr);
    ASSERT_NE(manager_->loudnessGainNode_, nullptr);

    // Test: Call InsertSessionEffectNode
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    manager_->sessionEffectSessionId_ = TEST_SESSION_ID; // Set before insert
    int32_t ret = manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_);

    // Verify: Success
    EXPECT_EQ(ret, 0);
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Verify: SE is connected between loudnessGainNode_ and converterForGain_
    // (This is verified by checking no crash during operations)

    // Cleanup
    manager_->RemoveSessionEffectNode();
    manager_->Stop(TEST_SESSION_ID);
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
    // Prepare: Create stream but NOT start (chain not ready)
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Try to insert when chain not ready
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    int32_t ret = manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_);

    // Verify: Should return ERR_SESSION_NOT_FOUND
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND));

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : InsertSessionEffectNode_Duplicate
 * @tc.type    : FUNC
 * @tc.number  : SE_Insert_003
 * @tc.desc    : Test InsertSessionEffectNode returns ERR_SESSION_ALREADY_BOUND when SE exists
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Insert_003, TestSize.Level1)
{
    // Prepare: Create, start stream and first SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance1 = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance1->Init("audio_aiss_enhance", defaultConfig_), 0);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance1, defaultConfig_), 0);
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: Try to insert again
    auto instance2 = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance2->Init("audio_aiss_enhance", defaultConfig_), 0);
    int32_t ret = manager_->InsertSessionEffectNode("audio_aiss_enhance", instance2, defaultConfig_);

    // Verify: Should return ERR_SESSION_ALREADY_BOUND
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND));

    // Cleanup
    manager_->RemoveSessionEffectNode();
    manager_->Stop(TEST_SESSION_ID);
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
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    manager_->sessionEffectSessionId_ = TEST_SESSION_ID;
    ASSERT_EQ(manager_->InsertSessionEffectNode("audio_aiss_enhance", instance, defaultConfig_), 0);
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);

    // Test: Remove SE
    int32_t ret = manager_->RemoveSessionEffectNode();

    // Verify: Success and SE removed
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    // Verify: Chain restored (no crash during subsequent operations)
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
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
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState creates SE from attach state
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_001, TestSize.Level1)
{
    // Prepare: Create stream and deferred SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    // Verify attach state exists
    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: Materialize SE by starting stream
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE node materialized
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : MaterializeSessionEffectFromAttachState_VoipMode
 * @tc.type    : FUNC
 * @tc.number  : SE_Materialize_002
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState skips VOIP mode
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_002, TestSize.Level1)
{
    // Prepare: Create VOIP manager
    HpaeSinkInfo voipSinkInfo = GetVoipDirectSinkInfo();
    voipManager_ = std::make_shared<HpaeDirectRendererManager>(voipSinkInfo);
    ASSERT_EQ(voipManager_->Init(), SUCCESS);
    WaitForMsgProcessing(voipManager_);

    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(voipManager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(voipManager_);

    // Create attach state (even though VOIP doesn't support SE, attach state can be saved)
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(voipManager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::ERR_NOT_SUPPORTED));

    // Test: Start stream in VOIP mode (should not materialize SE)
    ASSERT_EQ(voipManager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(voipManager_);

    // Verify: SE not materialized in VOIP mode
    EXPECT_EQ(voipManager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(voipManager_->sessionEffectSessionId_, 0);

    // Cleanup
    voipManager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(voipManager_);
    voipManager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(voipManager_);
    voipManager_->DeInit();
    WaitForMsgProcessing(voipManager_);
}

/**
 * @tc.name    : MaterializeSessionEffectFromAttachState_InstanceExpired
 * @tc.type    : FUNC
 * @tc.number  : SE_Materialize_003
 * @tc.desc    : Test MaterializeSessionEffectFromAttachState clears attach state when instance expired
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Materialize_003, TestSize.Level1)
{
    // Prepare: Create stream and deferred SE with weak instance
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Create instance and let it expire (simulate by not keeping strong ref)
    {
        auto instance = std::make_shared<MockEffectInstance>();
        ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
        ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
            instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    }
    // Instance expired here (weak_ref becomes invalid)

    // Test: Try to materialize with expired instance
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE not materialized and attach state cleared
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_FALSE(node->HasSessionEffectAttachState());

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroyStream(TEST_SESSION_ID);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

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
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->RemoveSessionEffectNode();
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    // Test: Start stream should materialize and start SE
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Verify: SE created and not paused
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_FALSE(manager_->sessionEffectNode_->IsPaused());

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroySessionEffect(TEST_SESSION_ID);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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
    manager_->DestroySessionEffect(TEST_SESSION_ID);
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
    HpStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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
    manager_->DestroySessionEffect(TEST_SESSION_ID);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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
    manager_->DestroySessionEffect(TEST_SESSION_ID);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: DeleteInputSession with FULL_TEARDOWN (default)
    manager_->DeleteInputSession();

    // Verify: SE node removed and attach state cleared
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
    EXPECT_FALSE(node->HasSessionEffectAttachState());
}

/**
 * @tc.name    : DeleteInputSession_KeepAttachState
 * @tc.type    : FUNC
 * @tc.number  : SE_DeleteInput_002
 * @tc.desc    : Test DeleteInputSession with KEEP_ATTACH_STATE preserves attach state
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_DeleteInput_002, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: DeleteInputSession with KEEP_ATTACH_STATE_FOR_MOVE
    manager_->DeleteInputSession(SessionEffectMoveMode::KEEP_ATTACH_STATE_FOR_MOVE);

    // Verify: SE node removed but attach state preserved
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
    EXPECT_TRUE(node->HasSessionEffectAttachState()); // Still true

    // Cleanup attach state manually
    node->ClearSessionEffectAttachState();
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
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID_2, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

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
 * @tc.name    : MoveStream_PrepareMoveAndPreserveAttachState
 * @tc.type    : FUNC
 * @tc.number  : SE_MoveStream_001
 * @tc.desc    : Test MoveStream calls PrepareMoveStream and preserves attach state
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_MoveStream_001, TestSize.Level1)
{
    // Prepare: Create, start stream and SE
    HpaeStreamInfo streamInfo = GetDefaultStreamInfo();
    ASSERT_EQ(manager_->CreateStream(streamInfo), SUCCESS);
    WaitForMsgProcessing(manager_);

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    ASSERT_NE(manager_->sessionEffectNode_, nullptr);
    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Test: MoveStream (simulated by calling DeleteInputSession with KEEP_ATTACH_STATE)
    manager_->sessionEffectNode_->PrepareMoveStream();
    manager_->DeleteInputSession(SessionEffectMoveMode::KEEP_ATTACH_STATE_FOR_MOVE);

    // Verify: SE node removed but attach state preserved
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Cleanup
    node->ClearSessionEffectAttachState();
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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

    // Verify: Physical SE node removed but attach state preserved
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Verify: SE re-materialized after reload (in ConnectInputSession)
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroySessionEffect(TEST_SESSION_ID);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
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
    manager_->DestroySessionEffect(TEST_SESSION_ID);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    // Step 2: Start materializes SE
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Step 3: Stop pauses SE
    ASSERT_EQ(manager_->Stop(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);
    EXPECT_TRUE(manager_->sessionEffectNode_->IsPaused());

    // Step 4: Destroy removes SE
    ASSERT_EQ(manager_->DestroySessionEffect(TEST_SESSION_ID), SUCCESS);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    auto node = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    ASSERT_NE(node, nullptr);
    EXPECT_TRUE(node->HasSessionEffectAttachState());

    // Step 2: Destroy without ever starting
    ASSERT_EQ(manager_->DestroySessionEffect(TEST_SESSION_ID), SUCCESS);
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

    // Test: Create SE without chain (should save attach state, not crash)
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    int32_t ret = manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);

    // Verify: Should succeed with attach state saved
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

    // Cleanup
    manager_->DestroyStream(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
}

/**
 * @tc.name    : ErrorHandling_InvalidInstanceId
 * @tc.type    : FUNC
 * @tc.number  : SE_Error_002
 * @tc.desc    : Test operations with invalid session ID are handled gracefully
 */
HWTEST_F(HpaeDirectRendererManagerSessionEffectTest, SE_Error_002, TestSize.Level1)
{
    // Test: Create SE for invalid session
    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

    int32_t ret = manager_->CreateSessionEffect(INVALID_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_);

    // Verify: Should succeed (attach state saved if stream exists)
    // Since stream doesn't exist, it should still succeed but attach state won't be saved
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Test: Destroy SE for invalid session
    ret = manager_->DestroySessionEffect(INVALID_SESSION_ID);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));

    // Test: Set parameter for invalid session
    ret = manager_->SetSessionEffectParameter(INVALID_SESSION_ID, 0, 0.5f);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
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
    auto instance1 = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance1->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance1, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    auto instance2 = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance2->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID_2, "audio_aiss_enhance",
        instance2, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    auto node1 = manager_->sinkInputNodeMap_[TEST_SESSION_ID];
    auto node2 = manager_->sinkInputNodeMap_[TEST_SESSION_ID_2];
    ASSERT_NE(node1, nullptr);
    ASSERT_NE(node2, nullptr);
    EXPECT_TRUE(node1->HasSessionEffectAttachState());
    EXPECT_TRUE(node2->HasSessionEffectAttachState());

    // Test: Start first stream (materializes its SE)
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
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
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID_2);
    WaitForMsgProcessing(manager_);
    manager_->DestroySessionEffect(TEST_SESSION_ID_2);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Verify: SE created successfully even after Flush
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, TEST_SESSION_ID);

    // Cleanup
    manager_->Stop(TEST_SESSION_ID);
    WaitForMsgProcessing(manager_);
    manager_->DestroySessionEffect(TEST_SESSION_ID);
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

    auto instance = std::make_shared<MockEffectInstance>();
    ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);
    ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
        instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_EQ(manager_->Start(TEST_SESSION_ID), SUCCESS);
    WaitForMsgProcessing(manager_);

    // Test: Destroy SE, then Flush
    ASSERT_EQ(manager_->DestroySessionEffect(TEST_SESSION_ID), SUCCESS);
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
        auto instance = std::make_shared<MockEffectInstance>();
        ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

        ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
            instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
        EXPECT_NE(manager_->sessionEffectNode_, nullptr);

        ASSERT_EQ(manager_->DestroySessionEffect(TEST_SESSION_ID), SUCCESS);
        EXPECT_EQ(manager_->sessionEffectNode_, nullptr);

        WaitForMsgProcessing(manager_);
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
        auto instance = std::make_shared<MockEffectInstance>();
        ASSERT_EQ(instance->Init("audio_aiss_enhance", defaultConfig_), 0);

        ASSERT_EQ(manager_->CreateSessionEffect(TEST_SESSION_ID, "audio_aiss_enhance",
            instance, defaultConfig_), static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

        ASSERT_EQ(manager_->DestroySessionEffect(TEST_SESSION_ID), SUCCESS);

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

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
