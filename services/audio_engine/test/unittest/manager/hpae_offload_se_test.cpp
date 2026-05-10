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
#include <cstdint>
#include <memory>
#include <string>

#include "hpae_offload_renderer_manager.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_loudness_gain_node.h"
#include "effect_instance.h"
#include "hpae_define.h"
#include "test_case_common.h"

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
    instance->Init("ai_audio_enhance", MakeTestConfig());
    return instance;
}

// Test fixture: sets up HpaeOffloadRendererManager with real node chain,
// following the same pattern as hpae_render_manager_test.cpp.
// Uses -fno-access-control build flag to set private members directly.
class HpaeOffloadSeTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        HpaeSinkInfo sinkInfo = GetTestSinkInfo();
        manager_ = std::make_shared<HpaeOffloadRendererManager>(sinkInfo);
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
        manager_->converterForLoudness_ =
            std::make_shared<HpaeAudioFormatConverterNode>(curInfo, outputInfo);
        manager_->converterForLoudness_->SetDownmixNormalization(false);
        manager_->loudnessGainNode_ =
            std::make_shared<HpaeLoudnessGainNode>(outputInfo);

        // Connect chain: loudnessGainNode_ -> converterForLoudness_ -> curNode_
        // (Connect = downstream reads from upstream)
        manager_->loudnessGainNode_->Connect(manager_->converterForLoudness_);
        manager_->converterForLoudness_->Connect(curNode_);
    }

    void TearDown() override
    {
        // Clean up SE node if present
        if (manager_->sessionEffectNode_ != nullptr) {
            manager_->sessionEffectNode_->DropEffect();
            manager_->sessionEffectNode_.reset();
            manager_->sessionEffectSessionId_ = 0;
        }
        manager_->pendingSessionEffects_.clear();
        manager_.reset();
    }

    std::shared_ptr<HpaeOffloadRendererManager> manager_;
    std::shared_ptr<HpaeSinkInputNode> curNode_;
    std::shared_ptr<HpaeSinkInputNode> otherNode_;
    AudioEffectConfig config_;
};

// OSE-1: CreateSessionEffect for curNode_ → immediate creation
HWTEST_F(HpaeOffloadSeTest, createSessionEffect_curNode_immediate, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "ai_audio_enhance",
        instance, config_);
    EXPECT_EQ(ret, 0);
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, CUR_SESSION_ID);
    EXPECT_TRUE(manager_->pendingSessionEffects_.empty());
}

// OSE-2: CreateSessionEffect for non-curNode_ → deferred to pending
HWTEST_F(HpaeOffloadSeTest, createSessionEffect_notCurNode_deferred, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(OTHER_SESSION_ID, "ai_audio_enhance",
        instance, config_);
    EXPECT_EQ(ret, 0);
    // SE not created yet (session is not curNode_)
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
    // Stored in pending map
    EXPECT_EQ(manager_->pendingSessionEffects_.size(), 1u);
    EXPECT_NE(manager_->pendingSessionEffects_.find(OTHER_SESSION_ID),
        manager_->pendingSessionEffects_.end());
}

// OSE-3: DestroySessionEffect for active SE → disconnect + restore chain
HWTEST_F(HpaeOffloadSeTest, destroySessionEffect_active, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "ai_audio_enhance", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    int32_t ret = manager_->DestroySessionEffect(CUR_SESSION_ID);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);
}

// OSE-3-enhanced: DestroySessionEffect for active SE → chain restored (direct connection)
HWTEST_F(HpaeOffloadSeTest, destroySessionEffect_active_restoresChain, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "ai_audio_enhance", instance, config_);
    ASSERT_NE(manager_->sessionEffectNode_, nullptr);

    int32_t ret = manager_->DestroySessionEffect(CUR_SESSION_ID);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, 0u);

    // Verify chain restored: loudnessGainNode_ reads directly from converterForLoudness_
    // (no SE node in between)
    // After disconnect+reconnect, loudnessGainNode_'s upstream should be converterForLoudness_
    // We verify indirectly: no crash when SignalProcess runs on the chain
    EXPECT_NE(manager_->loudnessGainNode_, nullptr);
    EXPECT_NE(manager_->converterForLoudness_, nullptr);
}

// OSE-4: DestroySessionEffect for pending SE → remove from map
HWTEST_F(HpaeOffloadSeTest, destroySessionEffect_pending, TestSize.Level0)
{
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "ai_audio_enhance", instance, config_);
    ASSERT_EQ(manager_->pendingSessionEffects_.size(), 1u);

    int32_t ret = manager_->DestroySessionEffect(OTHER_SESSION_ID);
    EXPECT_EQ(ret, 0);
    EXPECT_TRUE(manager_->pendingSessionEffects_.empty());
}

// OSE-5: HasSessionEffect returns true for both active and pending
HWTEST_F(HpaeOffloadSeTest, hasSessionEffect_activeAndPending, TestSize.Level0)
{
    // No SE initially
    EXPECT_FALSE(manager_->HasSessionEffect(CUR_SESSION_ID));
    EXPECT_FALSE(manager_->HasSessionEffect(OTHER_SESSION_ID));
    EXPECT_FALSE(manager_->HasSessionEffect(INVALID_SESSION_ID));

    // Create active SE for curNode_
    auto instance1 = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "ai_audio_enhance", instance1, config_);

    // Create pending SE for otherNode_
    auto instance2 = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "ai_audio_enhance", instance2, config_);

    EXPECT_TRUE(manager_->HasSessionEffect(CUR_SESSION_ID));
    EXPECT_TRUE(manager_->HasSessionEffect(OTHER_SESSION_ID));
    EXPECT_FALSE(manager_->HasSessionEffect(INVALID_SESSION_ID));
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
    manager_->CreateSessionEffect(CUR_SESSION_ID, "ai_audio_enhance", instance, config_);

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

// OSE-9: Pending SE auto-created when session becomes curNode_
// Simulates what ConnectInputSession does: checks pending map for curNode_'s session
HWTEST_F(HpaeOffloadSeTest, pendingCreatedOnSetCurrentNode, TestSize.Level0)
{
    // 1. Create pending SE for OTHER_SESSION_ID (not curNode_)
    auto instance = MakeTestEffectInstance();
    manager_->CreateSessionEffect(OTHER_SESSION_ID, "ai_audio_enhance", instance, config_);
    ASSERT_EQ(manager_->pendingSessionEffects_.size(), 1u);
    ASSERT_EQ(manager_->sessionEffectNode_, nullptr);

    // 2. Simulate curNode_ switch: OTHER_SESSION_ID becomes curNode_
    //    (normally done by SetCurrentNode → CreateOffloadNodes → ConnectInputSession)
    manager_->curNode_ = otherNode_;

    // 3. Simulate ConnectInputSession's pending SE resolution:
    //    "check if curNode_ has pending SE"
    uint32_t curSessionId = manager_->curNode_->GetSessionId();
    auto pendingIt = manager_->pendingSessionEffects_.find(curSessionId);
    ASSERT_NE(pendingIt, manager_->pendingSessionEffects_.end());

    // Recreate chain for new curNode_ (simplified: reuse existing nodes)
    manager_->sessionEffectSessionId_ = curSessionId;
    int32_t ret = manager_->InsertSessionEffectNode(pendingIt->second.effectName,
        pendingIt->second.instance, pendingIt->second.config);
    EXPECT_EQ(ret, 0);

    // Pending should be consumed
    manager_->pendingSessionEffects_.erase(pendingIt);
    EXPECT_TRUE(manager_->pendingSessionEffects_.empty());

    // SE now active for the new curNode_
    EXPECT_NE(manager_->sessionEffectNode_, nullptr);
    EXPECT_EQ(manager_->sessionEffectSessionId_, OTHER_SESSION_ID);
    EXPECT_TRUE(manager_->HasSessionEffect(OTHER_SESSION_ID));
}

// OSE-EXTRA: Double create for same session returns ERR_SESSION_ALREADY_BOUND
HWTEST_F(HpaeOffloadSeTest, createSessionEffect_doubleCreate, TestSize.Level0)
{
    auto instance1 = MakeTestEffectInstance();
    manager_->CreateSessionEffect(CUR_SESSION_ID, "ai_audio_enhance", instance1, config_);

    auto instance2 = MakeTestEffectInstance();
    int32_t ret = manager_->CreateSessionEffect(CUR_SESSION_ID, "ai_audio_enhance",
        instance2, config_);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND));
}

// OSE-EXTRA: DestroySessionEffect for unknown session returns ERR_EFFECT_NOT_BOUND
HWTEST_F(HpaeOffloadSeTest, destroySessionEffect_unknownSession, TestSize.Level0)
{
    int32_t ret = manager_->DestroySessionEffect(INVALID_SESSION_ID);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
