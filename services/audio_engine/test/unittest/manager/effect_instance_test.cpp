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

#include "effect_instance.h"
#include "fake_audio_effect_lib_entry.h"

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

class EffectInstanceTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        instance_ = std::make_shared<EffectInstance>();
    }

    void TearDown() override
    {
        instance_.reset();
    }

    std::shared_ptr<EffectInstance> instance_;
    AudioEffectConfig config_ = MakeTestConfig();
};

// Init sets state to ACTIVE
HWTEST_F(EffectInstanceTest, init_setsStateActive, TestSize.Level0)
{
    int32_t ret = instance_->Init("test_effect", config_);
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::ACTIVE);
}

// Release sets state to IDLE
HWTEST_F(EffectInstanceTest, release_setsStateIdle, TestSize.Level0)
{
    instance_->Init("test_effect", config_);
    int32_t ret = instance_->Release();
    EXPECT_EQ(ret, static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::IDLE);
}

// Default state is IDLE
HWTEST_F(EffectInstanceTest, defaultState_isIdle, TestSize.Level0)
{
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::IDLE);
}

// GetEffectName returns name after Init
HWTEST_F(EffectInstanceTest, getEffectName_returnsInitName, TestSize.Level0)
{
    instance_->Init("ai_audio_enhance", config_);
    EXPECT_EQ(instance_->GetEffectName(), "ai_audio_enhance");
}

// GetAudioConfig returns config after Init
HWTEST_F(EffectInstanceTest, getAudioConfig_returnsInitConfig, TestSize.Level0)
{
    instance_->Init("test_effect", config_);
    const auto &cfg = instance_->GetAudioConfig();
    EXPECT_EQ(cfg.inputCfg.samplingRate, 48000u);
    EXPECT_EQ(cfg.inputCfg.channels, 2u);
}

// SetBoundSessionId / GetBoundSessionId
HWTEST_F(EffectInstanceTest, boundSessionId_roundTrip, TestSize.Level0)
{
    instance_->SetBoundSessionId(42);
    EXPECT_EQ(instance_->GetBoundSessionId(), 42u);

    instance_->SetBoundSessionId(0);
    EXPECT_EQ(instance_->GetBoundSessionId(), 0u);
}

// MarkPendingRelease sets state and increments generation
HWTEST_F(EffectInstanceTest, markPendingRelease_setsStateAndIncrementsGeneration, TestSize.Level0)
{
    instance_->Init("test_effect", config_);
    instance_->SetBoundSessionId(100);

    EXPECT_EQ(instance_->GetReleaseGeneration(), 0u);

    instance_->MarkPendingRelease();
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::PENDING_RELEASE);
    EXPECT_EQ(instance_->GetReleaseGeneration(), 1u);
    EXPECT_EQ(instance_->GetBoundSessionId(), 0u); // cleared

    instance_->MarkPendingRelease();
    EXPECT_EQ(instance_->GetReleaseGeneration(), 2u);
}

// PendingTaskId round-trip
HWTEST_F(EffectInstanceTest, pendingTaskId_roundTrip, TestSize.Level0)
{
    EXPECT_EQ(instance_->GetPendingTaskId(), 0u);
    instance_->SetPendingTaskId(12345);
    EXPECT_EQ(instance_->GetPendingTaskId(), 12345u);
}

// SetState direct manipulation
HWTEST_F(EffectInstanceTest, setState_direct, TestSize.Level0)
{
    instance_->SetState(EffectInstance::EffectState::ACTIVE);
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::ACTIVE);

    instance_->SetState(EffectInstance::EffectState::ERROR);
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::ERROR);
}

// Phase 1 stubs: all return SUCCESS
HWTEST_F(EffectInstanceTest, phase1Stubs_returnSuccess, TestSize.Level0)
{
    instance_->Init("test_effect", config_);

    EXPECT_EQ(instance_->FeedInput(nullptr, 0), 0);
    EXPECT_EQ(instance_->ReadOutput(nullptr, 0, nullptr), 0);
    EXPECT_EQ(instance_->GetInputLevel(), 0u);
    EXPECT_EQ(instance_->TriggerProcess(), 0);
    EXPECT_EQ(instance_->Flush(), 0);
    EXPECT_EQ(instance_->SetParameter(0, 0.0f), 0);
}

// ClearBuffers resets skip counter (doesn't crash)
HWTEST_F(EffectInstanceTest, clearBuffers_resetsSkip_noop, TestSize.Level0)
{
    instance_->Init("test_effect", config_);
    instance_->ClearBuffers();
    // No crash, skip counter reset
}

// Init-Release cycle can be repeated
HWTEST_F(EffectInstanceTest, initReleaseCycle_canRepeat, TestSize.Level0)
{
    instance_->Init("effect_a", config_);
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::ACTIVE);
    instance_->Release();
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::IDLE);

    instance_->Init("effect_b", config_);
    EXPECT_EQ(instance_->GetEffectName(), "effect_b");
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::ACTIVE);
    instance_->Release();
}

// --- Phase 3A-1: Algorithm integration tests ---

class EffectInstanceAlgoTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        fake_.Reset();
        fake_.SetProcessDelayMs(0);
        instance_ = std::make_shared<EffectInstance>();
    }
    void TearDown() override
    {
        instance_.reset();
    }
    FakeAudioEffectLibEntry fake_;
    std::shared_ptr<EffectInstance> instance_;
    AudioEffectConfig config_ = MakeTestConfig();
};

// Init with library creates handle
HWTEST_F(EffectInstanceAlgoTest, init_withLib_createsHandle, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    int32_t ret = instance_->Init("test_effect", config_, &lib);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::ACTIVE);
    EXPECT_EQ(fake_.GetCreateCount(), 1u);
}

// Init with failing library returns error
HWTEST_F(EffectInstanceAlgoTest, init_withLibFail_returnsError, TestSize.Level0)
{
    fake_.SetCreateFail(true);
    auto lib = fake_.GetLibrary();
    int32_t ret = instance_->Init("test_effect", config_, &lib);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::IDLE);
}

// Init without library keeps Phase 1 behavior
HWTEST_F(EffectInstanceAlgoTest, init_noLib_phase1Behavior, TestSize.Level0)
{
    int32_t ret = instance_->Init("test_effect", config_);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::ACTIVE);
    EXPECT_EQ(fake_.GetCreateCount(), 0u);
}

// Release destroys handle when created with library
HWTEST_F(EffectInstanceAlgoTest, release_destroysHandle, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    EXPECT_EQ(fake_.GetCreateCount(), 1u);
    EXPECT_EQ(fake_.GetReleaseCount(), 0u);

    instance_->Release();
    EXPECT_EQ(fake_.GetReleaseCount(), 1u);
    EXPECT_EQ(instance_->GetState(), EffectInstance::EffectState::IDLE);
}

// Init-Release cycle with library can repeat
HWTEST_F(EffectInstanceAlgoTest, initReleaseCycle_withLib, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("effect_a", config_, &lib);
    EXPECT_EQ(fake_.GetCreateCount(), 1u);
    instance_->Release();
    EXPECT_EQ(fake_.GetReleaseCount(), 1u);

    instance_->Init("effect_b", config_, &lib);
    EXPECT_EQ(fake_.GetCreateCount(), 2u);
    EXPECT_EQ(instance_->GetEffectName(), "effect_b");
    instance_->Release();
    EXPECT_EQ(fake_.GetReleaseCount(), 2u);
}

// EI-C1: StartEffect sends ENABLE
HWTEST_F(EffectInstanceAlgoTest, startEffect_sendsEnable, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.Reset();

    instance_->StartEffect();

    auto &log = fake_.GetCallLog();
    bool hasEnable = false;
    for (auto &e : log) {
        if (e.cmdCode == EFFECT_CMD_ENABLE) hasEnable = true;
    }
    EXPECT_TRUE(hasEnable);
}

// EI-C2: StopEffect sends DISABLE
HWTEST_F(EffectInstanceAlgoTest, stopEffect_sendsDisable, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.Reset();

    instance_->StopEffect();

    auto &log = fake_.GetCallLog();
    bool hasDisable = false;
    for (auto &e : log) {
        if (e.cmdCode == EFFECT_CMD_DISABLE) hasDisable = true;
    }
    EXPECT_TRUE(hasDisable);
}

// EI-C3: Flush sends FLUSH
HWTEST_F(EffectInstanceAlgoTest, flush_sendsFlush, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.Reset();

    instance_->Flush();
    EXPECT_EQ(fake_.GetFlushCount(), 1u);
}

// EI-C4: Flush unsupported returns error
HWTEST_F(EffectInstanceAlgoTest, flush_unsupported_returnsError, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.Reset();
    fake_.SetFlushFail(true);

    int32_t ret = instance_->Flush();
    EXPECT_NE(ret, 0);
}

// EI-C5: SetParameter sends SET_PARAM
HWTEST_F(EffectInstanceAlgoTest, setParameter_sendsSetParam, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.Reset();

    instance_->SetParameter(1, 0.5f);

    auto &log = fake_.GetCallLog();
    bool hasSetParam = false;
    for (auto &e : log) {
        if (e.cmdCode == EFFECT_CMD_SET_PARAM) hasSetParam = true;
    }
    EXPECT_TRUE(hasSetParam);
}

// EI-C6: GetPreheatFrames queries and caches result
HWTEST_F(EffectInstanceAlgoTest, getPreheatFrames_cachesResult, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.Reset();
    fake_.SetPreheatFrames(48000);

    uint32_t frames = instance_->GetPreheatFrames();
    EXPECT_EQ(frames, 48000u);

    auto paramCount = fake_.GetParamLog().size();
    frames = instance_->GetPreheatFrames();
    EXPECT_EQ(frames, 48000u);
    EXPECT_EQ(fake_.GetParamLog().size(), paramCount);  // cached
}

// --- EI-B: Transparent proxy tests ---

// EI-B1: FeedInput proxies data to algorithm's feedInput
HWTEST_F(EffectInstanceAlgoTest, feedInput_proxies_to_feedInput, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);

    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    int32_t ret = instance_->FeedInput(reinterpret_cast<const uint8_t *>(data), sizeof(data));
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(fake_.GetFeedInputCount(), 1u);
    EXPECT_EQ(instance_->GetInputLevel(), sizeof(data));
}

// EI-B2: ReadOutput gets output from algorithm
HWTEST_F(EffectInstanceAlgoTest, readOutput_gets_algo_output, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);

    // Feed enough data to trigger processing (192000 floats = 2s stereo)
    std::vector<float> data(fakeProcessThresholdFloats);
    for (size_t i = 0; i < data.size(); i += 2) {
        data[i] = 1.0f;
        data[i + 1] = 2.0f;
    }
    instance_->FeedInput(reinterpret_cast<const uint8_t *>(data.data()),
                         data.size() * sizeof(float));
    fake_.WaitForProcessing();

    float out[4] = {};
    int32_t ret = instance_->ReadOutput(reinterpret_cast<uint8_t *>(out), sizeof(out));
    EXPECT_GT(ret, 0);
    // L/R swapped: original [1,2] -> [2,1]
    EXPECT_FLOAT_EQ(out[0], 2.0f);
    EXPECT_FLOAT_EQ(out[1], 1.0f);
}

// EI-B3: ReadOutput returns 0 when algorithm has no output
HWTEST_F(EffectInstanceAlgoTest, readOutput_empty_when_no_algo_output, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);

    float out[4] = {};
    int32_t ret = instance_->ReadOutput(reinterpret_cast<uint8_t *>(out), sizeof(out));
    EXPECT_EQ(ret, 0);
}

// EI-B5: ReadOutput with skipCount discards frames before reading
HWTEST_F(EffectInstanceAlgoTest, readOutput_withSkip_discardsFrames, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);

    std::vector<float> data(fakeProcessThresholdFloats * 3);
    for (size_t i = 0; i < data.size(); i += 2) {
        data[i] = 1.0f;
        data[i + 1] = 2.0f;
    }
    instance_->FeedInput(reinterpret_cast<const uint8_t *>(data.data()),
                         data.size() * sizeof(float));
    fake_.WaitForProcessing();

    uint32_t skipCount = 2;
    float out[4] = {};
    int32_t ret = instance_->ReadOutput(reinterpret_cast<uint8_t *>(out), sizeof(out), &skipCount);
    EXPECT_GT(ret, 0);
    EXPECT_EQ(skipCount, 0u);
    EXPECT_FLOAT_EQ(out[0], 2.0f);
    EXPECT_FLOAT_EQ(out[1], 1.0f);
}

// EI-B6: ReadOutput with partial skip updates remaining count
HWTEST_F(EffectInstanceAlgoTest, readOutput_partialSkip_updatesRemaining, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);

    std::vector<float> data(fakeProcessThresholdFloats);
    for (size_t i = 0; i < data.size(); i += 2) {
        data[i] = 1.0f;
        data[i + 1] = 2.0f;
    }
    instance_->FeedInput(reinterpret_cast<const uint8_t *>(data.data()),
                         data.size() * sizeof(float));
    fake_.WaitForProcessing();

    // Use a buffer large enough to consume all output in one getOutput call.
    // skipCount=2: 1st skip drains all output (success), 2nd finds empty (partial).
    std::vector<float> largeBuf(fakeProcessThresholdFloats);
    uint32_t skipCount = 2;
    int32_t ret = instance_->ReadOutput(
        reinterpret_cast<uint8_t *>(largeBuf.data()), largeBuf.size() * sizeof(float), &skipCount);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(skipCount, 1u);
}

// EI-B7: ReadOutput with skipCount=nullptr behaves as pure read
HWTEST_F(EffectInstanceAlgoTest, readOutput_nullptrSkip_pureRead, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);

    std::vector<float> data(fakeProcessThresholdFloats);
    for (size_t i = 0; i < data.size(); i += 2) {
        data[i] = 1.0f;
        data[i + 1] = 2.0f;
    }
    instance_->FeedInput(reinterpret_cast<const uint8_t *>(data.data()),
                         data.size() * sizeof(float));
    fake_.WaitForProcessing();

    float out[4] = {};
    int32_t ret = instance_->ReadOutput(reinterpret_cast<uint8_t *>(out), sizeof(out));
    EXPECT_GT(ret, 0);
    EXPECT_FLOAT_EQ(out[0], 2.0f);
    EXPECT_FLOAT_EQ(out[1], 1.0f);
}

// --- Phase 3A-2: SeProcessStats counter tests ---

// EI-D1: Stats counter accumulates frames on FeedInput
HWTEST_F(EffectInstanceAlgoTest, stats_accumulatesOnFeedInput, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);

    // 480 samples * 2 channels = 960 floats = 3840 bytes
    size_t frameBytes = 480 * config_.inputCfg.channels * sizeof(float);
    std::vector<float> data(frameBytes / sizeof(float), 1.0f);

    instance_->SetStatsReportThresholdForTest(999999); // prevent threshold report
    instance_->FeedInput(reinterpret_cast<const uint8_t *>(data.data()), frameBytes);

    const auto &stats = instance_->GetStatsForTest();
    EXPECT_EQ(stats.framesProcessed, 480);
    EXPECT_EQ(stats.processErrors, 0);
}

// EI-D2: Stats counter tracks errors on FeedInput failure
HWTEST_F(EffectInstanceAlgoTest, stats_tracksErrorsOnFeedInputFail, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);
    fake_.SetFeedInputFail(true);

    size_t frameBytes = 480 * config_.inputCfg.channels * sizeof(float);
    std::vector<float> data(frameBytes / sizeof(float), 1.0f);

    instance_->SetStatsReportThresholdForTest(999999);
    instance_->FeedInput(reinterpret_cast<const uint8_t *>(data.data()), frameBytes);

    const auto &stats = instance_->GetStatsForTest();
    EXPECT_EQ(stats.processErrors, 1);
    EXPECT_EQ(stats.framesProcessed, 0);
}

// EI-D3: Stats counter resets on Release
HWTEST_F(EffectInstanceAlgoTest, stats_resetsOnRelease, TestSize.Level0)
{
    auto lib = fake_.GetLibrary();
    instance_->Init("test_effect", config_, &lib);
    fake_.SetPreheatFrames(0);

    size_t frameBytes = 480 * config_.inputCfg.channels * sizeof(float);
    std::vector<float> data(frameBytes / sizeof(float), 1.0f);
    instance_->SetStatsReportThresholdForTest(999999);
    instance_->FeedInput(reinterpret_cast<const uint8_t *>(data.data()), frameBytes);
    EXPECT_GT(instance_->GetStatsForTest().framesProcessed, 0);

    instance_->Release();
    EXPECT_EQ(instance_->GetStatsForTest().framesProcessed, 0);
    EXPECT_EQ(instance_->GetStatsForTest().processErrors, 0);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
