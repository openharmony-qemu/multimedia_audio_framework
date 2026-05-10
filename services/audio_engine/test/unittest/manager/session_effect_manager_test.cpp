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
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "session_effect_manager.h"
#include "hpae_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static constexpr uint32_t TEST_SAMPLE_RATE = 48000;
static constexpr uint32_t TEST_CHANNEL_COUNT = 2;

static AudioEffectConfig MakeTestConfig()
{
    AudioEffectConfig config;
    config.inputCfg.samplingRate = TEST_SAMPLE_RATE;
    config.inputCfg.channels = TEST_CHANNEL_COUNT;
    config.outputCfg.samplingRate = TEST_SAMPLE_RATE;
    config.outputCfg.channels = TEST_CHANNEL_COUNT;
    return config;
}

class SessionEffectManagerTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        hpaeManager_ = std::make_shared<HpaeManager>();
        sem_ = std::make_unique<SessionEffectManager>(*hpaeManager_);
        config_ = MakeTestConfig();
    }

    void TearDown() override
    {
        sem_.reset();
        hpaeManager_.reset();
    }

    // Process expired delayed tasks in HpaeManager's queue
    void ProcessDelayedTasks()
    {
        hpaeManager_->GetDelayedTaskQueue().ProcessExpiredTasks();
    }

    std::shared_ptr<HpaeManager> hpaeManager_;
    std::unique_ptr<SessionEffectManager> sem_;
    AudioEffectConfig config_;
};

// Bind creates a new EffectInstance in ACTIVE state
HWTEST_F(SessionEffectManagerTest, bind_newInstance_isActive, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance;
    int32_t ret = sem_->BindSessionEffect(1, "ai_audio_enhance", config_, instance);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    ASSERT_NE(instance, nullptr);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::ACTIVE);
    EXPECT_EQ(instance->GetBoundSessionId(), 1u);
    EXPECT_EQ(instance->GetEffectName(), "ai_audio_enhance");
}

// Bind same session + same effect is idempotent — returns SUCCESS with existing instance
HWTEST_F(SessionEffectManagerTest, bind_duplicateSession_alreadyBound, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "ai_audio_enhance", config_, instance);

    std::shared_ptr<EffectInstance> instance2;
    int32_t ret = sem_->BindSessionEffect(1, "ai_audio_enhance", config_, instance2);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(instance2, instance);  // same instance returned
}

// Unbind sets instance to PENDING_RELEASE
HWTEST_F(SessionEffectManagerTest, unbind_setsPendingRelease, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "ai_audio_enhance", config_, instance);

    int32_t ret = sem_->UnbindSessionEffect(1);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::PENDING_RELEASE);
    EXPECT_EQ(instance->GetBoundSessionId(), 0u);
}

// Unbind non-existent session returns ERR_EFFECT_NOT_BOUND
HWTEST_F(SessionEffectManagerTest, unbind_noEffect_returnsNotBound, TestSize.Level0)
{
    int32_t ret = sem_->UnbindSessionEffect(999);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND));
}

// Bind reuses PENDING_RELEASE instance when re-binding same effect name
HWTEST_F(SessionEffectManagerTest, bind_reusesPendingReleaseInstance, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance1;
    sem_->BindSessionEffect(1, "ai_audio_enhance", config_, instance1);
    sem_->UnbindSessionEffect(1);

    std::shared_ptr<EffectInstance> instance2;
    int32_t ret = sem_->BindSessionEffect(2, "ai_audio_enhance", config_, instance2);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(instance2, instance1); // same instance reused
    EXPECT_EQ(instance2->GetState(), EffectInstance::EffectState::ACTIVE);
    EXPECT_EQ(instance2->GetBoundSessionId(), 2u);
}

// LazyReleaseExpired destroys instance when generation matches
HWTEST_F(SessionEffectManagerTest, lazyReleaseExpired_destroysOnGenerationMatch, TestSize.Level0)
{
    // Set short timeout BEFORE unbind so the delayed task uses it
    sem_->SetLazyReleaseTimeout(10);

    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "ai_audio_enhance", config_, instance);
    sem_->UnbindSessionEffect(1);

    EXPECT_TRUE(sem_->HasPendingEffectInstance("ai_audio_enhance"));

    // Wait for timeout to expire
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Trigger delayed task processing
    ProcessDelayedTasks();

    EXPECT_FALSE(sem_->HasPendingEffectInstance("ai_audio_enhance"));
}

// LazyReleaseExpired is ignored when generation doesn't match
HWTEST_F(SessionEffectManagerTest, lazyReleaseExpired_ignoredOnGenerationMismatch, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "ai_audio_enhance", config_, instance);
    sem_->UnbindSessionEffect(1);

    // Manually re-bind (bumps generation)
    sem_->BindSessionEffect(2, "ai_audio_enhance", config_, instance);

    // Old generation's lazy release should be ignored (gen doesn't match)
    // The old delayed task will fire but be ignored
    ProcessDelayedTasks();

    // Instance should still be ACTIVE (bound to session 2)
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::ACTIVE);
    EXPECT_EQ(instance->GetBoundSessionId(), 2u);
}

// GetBoundSessionId returns correct session
HWTEST_F(SessionEffectManagerTest, getBoundSessionId_returnsCorrectSession, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(42, "test_effect", config_, instance);
    EXPECT_EQ(sem_->GetBoundSessionId("test_effect"), 42u);
}

// GetBoundSessionId returns 0 for unknown effect
HWTEST_F(SessionEffectManagerTest, getBoundSessionId_unknownEffect_returnsZero, TestSize.Level0)
{
    EXPECT_EQ(sem_->GetBoundSessionId("nonexistent"), 0u);
}

// GetAllActiveEffectNames returns only ACTIVE effects
HWTEST_F(SessionEffectManagerTest, getAllActiveEffectNames_returnsOnlyActive, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> inst1;
    std::shared_ptr<EffectInstance> inst2;
    sem_->BindSessionEffect(1, "effect_a", config_, inst1);
    sem_->BindSessionEffect(2, "effect_b", config_, inst2);

    auto names = sem_->GetAllActiveEffectNames();
    EXPECT_EQ(names.size(), 2u);

    sem_->UnbindSessionEffect(1);
    names = sem_->GetAllActiveEffectNames();
    EXPECT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "effect_b");
}

// HasPendingEffectInstance
HWTEST_F(SessionEffectManagerTest, hasPendingEffectInstance_correctStatus, TestSize.Level0)
{
    EXPECT_FALSE(sem_->HasPendingEffectInstance("test_effect"));

    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "test_effect", config_, instance);
    EXPECT_FALSE(sem_->HasPendingEffectInstance("test_effect"));

    sem_->UnbindSessionEffect(1);
    EXPECT_TRUE(sem_->HasPendingEffectInstance("test_effect"));
}

// SM-D1: OnSessionDestroyed for bound session → PENDING_RELEASE (se-state-management.md §2.3)
HWTEST_F(SessionEffectManagerTest, onSessionDestroyed_boundSession_setsPendingRelease, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "test_effect", config_, instance);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::ACTIVE);

    sem_->OnSessionDestroyed(1);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::PENDING_RELEASE);
    EXPECT_TRUE(sem_->HasPendingEffectInstance("test_effect"));
}

// SM-D2: OnSessionDestroyed for unbound session → no-op
HWTEST_F(SessionEffectManagerTest, onSessionDestroyed_unboundSession_noop, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "test_effect", config_, instance);

    // Destroy a different session — should not affect the bound instance
    sem_->OnSessionDestroyed(999);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::ACTIVE);
    EXPECT_EQ(instance->GetBoundSessionId(), 1u);
}

// SM-D3: OnSessionDestroyed after Unbind is idempotent (already PENDING_RELEASE → no-op)
HWTEST_F(SessionEffectManagerTest, onSessionDestroyed_afterUnbind_noop, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "test_effect", config_, instance);

    sem_->UnbindSessionEffect(1);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::PENDING_RELEASE);

    // OnSessionDestroyed same session after Unbind → should be no-op
    // (timer already running from Unbind, no new timer posted)
    sem_->OnSessionDestroyed(1);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::PENDING_RELEASE);
    EXPECT_TRUE(sem_->HasPendingEffectInstance("test_effect"));
}

// Deinit clears all instances
HWTEST_F(SessionEffectManagerTest, deinit_clearsAll, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> inst1;
    std::shared_ptr<EffectInstance> inst2;
    sem_->BindSessionEffect(1, "effect_a", config_, inst1);
    sem_->BindSessionEffect(2, "effect_b", config_, inst2);

    sem_->Deinit();

    auto names = sem_->GetAllActiveEffectNames();
    EXPECT_TRUE(names.empty());
}

// Bind rebinds when ACTIVE instance is bound to different session
HWTEST_F(SessionEffectManagerTest, bind_rebindsActiveFromOtherSession, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> inst1;
    sem_->BindSessionEffect(1, "test_effect", config_, inst1);
    EXPECT_EQ(inst1->GetBoundSessionId(), 1u);

    std::shared_ptr<EffectInstance> inst2;
    int32_t ret = sem_->BindSessionEffect(2, "test_effect", config_, inst2);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(inst2, inst1); // same instance, rebound
    EXPECT_EQ(inst2->GetBoundSessionId(), 2u);
}

// Multiple effects can be bound simultaneously
HWTEST_F(SessionEffectManagerTest, multipleEffects_simultaneouslyActive, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> inst1;
    std::shared_ptr<EffectInstance> inst2;
    sem_->BindSessionEffect(1, "effect_a", config_, inst1);
    sem_->BindSessionEffect(2, "effect_b", config_, inst2);

    EXPECT_NE(inst1, inst2);
    EXPECT_EQ(sem_->GetBoundSessionId("effect_a"), 1u);
    EXPECT_EQ(sem_->GetBoundSessionId("effect_b"), 2u);

    auto names = sem_->GetAllActiveEffectNames();
    EXPECT_EQ(names.size(), 2u);
}

// Bind same session with different effectName → ERR_SESSION_ALREADY_BOUND
HWTEST_F(SessionEffectManagerTest, bind_sameSessionDifferentName_alreadyBound, TestSize.Level0)
{
    std::shared_ptr<EffectInstance> inst1;
    sem_->BindSessionEffect(1, "effect_a", config_, inst1);

    // Same session, different effectName → should reject
    std::shared_ptr<EffectInstance> inst2;
    int32_t ret = sem_->BindSessionEffect(1, "effect_b", config_, inst2);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND));
}

// SM-L5: Fast bind/unbind loop — generation mismatch prevents stale lazy tasks
HWTEST_F(SessionEffectManagerTest, lazyRelease_fastBindUnbindLoop, TestSize.Level0)
{
    sem_->SetLazyReleaseTimeout(10);  // short timeout for test

    std::shared_ptr<EffectInstance> instance;
    // Cycle 1: bind -> unbind (generation becomes 1)
    sem_->BindSessionEffect(1, "test_effect", config_, instance);
    sem_->UnbindSessionEffect(1);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::PENDING_RELEASE);

    // Cycle 2: bind -> unbind (generation becomes 2)
    sem_->BindSessionEffect(2, "test_effect", config_, instance);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::ACTIVE);
    sem_->UnbindSessionEffect(2);
    EXPECT_EQ(instance->GetState(), EffectInstance::EffectState::PENDING_RELEASE);

    // Wait for timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ProcessDelayedTasks();

    // Only the last unbind's task should have destroyed the instance
    EXPECT_FALSE(sem_->HasPendingEffectInstance("test_effect"));
}

// SM-M2: Different effectNames have independent lazy release lifecycles
HWTEST_F(SessionEffectManagerTest, differentEffectNames_lazyIndependent, TestSize.Level0)
{
    sem_->SetLazyReleaseTimeout(10);

    std::shared_ptr<EffectInstance> instA;
    std::shared_ptr<EffectInstance> instB;
    sem_->BindSessionEffect(1, "effect_a", config_, instA);
    sem_->BindSessionEffect(2, "effect_b", config_, instB);

    // Unbind effect_a -> PENDING_RELEASE
    sem_->UnbindSessionEffect(1);
    EXPECT_EQ(instA->GetState(), EffectInstance::EffectState::PENDING_RELEASE);

    // Wait for effect_a lazy timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    ProcessDelayedTasks();

    // effect_a should be destroyed
    EXPECT_FALSE(sem_->HasPendingEffectInstance("effect_a"));
    EXPECT_EQ(sem_->GetBoundSessionId("effect_a"), 0u);

    // effect_b must remain ACTIVE
    EXPECT_EQ(instB->GetState(), EffectInstance::EffectState::ACTIVE);
    EXPECT_EQ(sem_->GetBoundSessionId("effect_b"), 2u);
    auto names = sem_->GetAllActiveEffectNames();
    EXPECT_EQ(names.size(), 1u);
    EXPECT_EQ(names[0], "effect_b");
}

// SM-Q3: GetPendingEffectInstance returns pointer for PENDING_RELEASE state
HWTEST_F(SessionEffectManagerTest, getPendingEffectInstance, TestSize.Level0)
{
    // No instance -> returns nullptr
    auto pending = sem_->GetPendingEffectInstance("test_effect");
    EXPECT_EQ(pending, nullptr);

    // ACTIVE -> returns nullptr (not pending)
    std::shared_ptr<EffectInstance> instance;
    sem_->BindSessionEffect(1, "test_effect", config_, instance);
    pending = sem_->GetPendingEffectInstance("test_effect");
    EXPECT_EQ(pending, nullptr);

    // PENDING_RELEASE -> returns the instance
    sem_->UnbindSessionEffect(1);
    pending = sem_->GetPendingEffectInstance("test_effect");
    ASSERT_NE(pending, nullptr);
    EXPECT_EQ(pending->GetState(), EffectInstance::EffectState::PENDING_RELEASE);
    EXPECT_EQ(pending, instance);  // same shared_ptr
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
