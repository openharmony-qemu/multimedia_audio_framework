/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 */

#include "gtest/gtest.h"
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>
#include <chrono>

#include "hpae_manager.h"
#include "hpae_define.h"
#include "hpae_mocks.h"
#include "audio_errors.h"
#include "test_case_common.h"
#include "fake_audio_effect_lib_entry.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {

static constexpr uint32_t TEST_SESSION_ID = 123456;
static constexpr uint32_t TEST_FRAME_LEN = 882;
static constexpr uint32_t WAIT_FOR_MSG_MS = 500;
static constexpr uint32_t US_PER_MS = 1000;
static constexpr uint32_t TEST_UID = 111111;
static constexpr int32_t TEST_WRITE_VALUE = 100;
static constexpr uint32_t POLL_INTERVAL_US = 10000;     // 10ms
static constexpr uint32_t WAIT_SHORT_US = 200000;       // 200ms
static constexpr uint32_t WAIT_LONG_US = 500000;        // 500ms
static constexpr uint32_t VERIFY_WAIT_US = 100000;      // 100ms
static constexpr uint32_t WARMUP_WAIT_US = 50000;        // 50ms

AudioModuleInfo GetTestAudioModuleInfo()
{
    AudioModuleInfo info;
    info.lib = "libmodule-hdi-sink.z.so";
    info.channels = "2";
    info.rate = "48000";
    info.name = "Speaker_File_E2E";
    info.adapterName = "file_io";
    info.className = "file_io";
    info.bufferSize = "7680";
    info.format = "s32le";
    info.fixedLatency = "1";
    info.offloadEnable = "0";
    info.networkId = "LocalDevice";
    info.fileName = "/data/file_io_48000_2_s32le.pcm";
    info.needEmptyChunk = true;
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    info.deviceType = typeValue.str();
    return info;
}

void WaitForMsgProcessing()
{
    usleep(WAIT_FOR_MSG_MS * US_PER_MS);
}

}  // namespace

class SessionEffectE2EFixture : public ::testing::Test {
protected:
    void SetUp() override
    {
        fakeLib_.SetProcessDelayMs(1);

        hpaeManager_ = std::make_shared<HpaeManager>();
        hpaeManager_->Init();
        sleep(1);

        // Inject fake library into SessionEffectManager (must be member, not local)
        fakeLibStruct_ = fakeLib_.GetLibrary();
        hpaeManager_->sessionEffectManager_->SetAudioEffectLibraryForTest(&fakeLibStruct_);

        // Full pipeline setup via OpenAudioPort (creates RendererManager with Init)
        AudioModuleInfo audioModuleInfo = GetTestAudioModuleInfo();
        hpaeManager_->OpenAudioPort(audioModuleInfo);
        hpaeManager_->SetDefaultSink(audioModuleInfo.name);
        WaitForMsgProcessing();

        // Create stream
        HpaeStreamInfo streamInfo;
        streamInfo.channels = STEREO;
        streamInfo.samplingRate = SAMPLE_RATE_44100;
        streamInfo.format = SAMPLE_S16LE;
        streamInfo.frameLen = TEST_FRAME_LEN;
        streamInfo.sessionId = TEST_SESSION_ID;
        streamInfo.streamType = STREAM_MUSIC;
        streamInfo.streamClassType = HPAE_STREAM_CLASS_TYPE_PLAY;
        streamInfo.uid = TEST_UID;
        hpaeManager_->CreateStream(streamInfo);
        WaitForMsgProcessing();

        // Register write callback
        writeCallback_ = std::make_shared<WriteFixedValueCb>(SAMPLE_S16LE, TEST_WRITE_VALUE);
        hpaeManager_->RegisterWriteCallback(TEST_SESSION_ID, writeCallback_);

        // Start playback
        hpaeManager_->Start(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
        WaitForMsgProcessing();
        sleep(1);
    }

    void TearDown() override
    {
        hpaeManager_->Stop(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
        WaitForMsgProcessing();
        hpaeManager_->Release(HPAE_STREAM_CLASS_TYPE_PLAY, TEST_SESSION_ID);
        WaitForMsgProcessing();
        hpaeManager_->DeInit();
        hpaeManager_.reset();
    }

    // Effect control
    void EnableEffect()
    {
        hpaeManager_->SetAissEnabled(TEST_SESSION_ID, true);
        WaitForMsgProcessing();
    }

    void DisableEffect()
    {
        hpaeManager_->SetAissEnabled(TEST_SESSION_ID, false);
        WaitForMsgProcessing();
    }

    // Verification helpers
    bool IsAlgorithmProcessing()
    {
        return fakeLib_.GetFeedInputCount() > 0 && fakeLib_.GetGetOutputCount() > 0;
    }

    uint32_t GetFeedInputCount() const { return fakeLib_.GetFeedInputCount(); }
    uint32_t GetGetOutputCount() const { return fakeLib_.GetGetOutputCount(); }
    uint32_t GetCreateCount() const { return fakeLib_.GetCreateCount(); }
    uint32_t GetProcessCount() const { return fakeLib_.GetProcessCount(); }
    void ResetFakeLibCounters() { fakeLib_.Reset(); }

    void WaitUntilAlgorithmProcessing(uint32_t timeoutMs = 5000)
    {
        auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(timeoutMs);
        while (std::chrono::steady_clock::now() < deadline) {
            usleep(POLL_INTERVAL_US);
            if (IsAlgorithmProcessing()) {
                return;
            }
        }
    }

    void WaitUntilAlgorithmIdle(uint32_t timeoutMs = 3000)
    {
        auto feedBefore = GetFeedInputCount();
        usleep(WAIT_SHORT_US);
        if (GetFeedInputCount() == feedBefore) {
            return;
        }
        usleep(WAIT_LONG_US);
    }

    std::shared_ptr<HpaeManager> hpaeManager_;
    std::shared_ptr<WriteFixedValueCb> writeCallback_;
    FakeAudioEffectLibEntry fakeLib_;
    AudioEffectLibrary fakeLibStruct_;
};

// Smoke test: fixture compiles and sets up correctly
HWTEST_F(SessionEffectE2EFixture, FixtureSetup, TestSize.Level0)
{
    EXPECT_NE(hpaeManager_, nullptr);
    EXPECT_NE(hpaeManager_->sessionEffectManager_, nullptr);
}

// E2E-7: Bypass mode — no effect enabled, algorithm never called
HWTEST_F(SessionEffectE2EFixture, BypassMode_NoEffectNoAlgorithmCalls, TestSize.Level0)
{
    usleep(WAIT_SHORT_US);  // Run pipeline for 200ms
    EXPECT_EQ(GetFeedInputCount(), 0u);
    EXPECT_EQ(GetCreateCount(), 0u);
}

// E2E-1: Enable → algorithm starts processing
HWTEST_F(SessionEffectE2EFixture, Enable_EffectReachesActive, TestSize.Level0)
{
    uint32_t createCountBefore = GetCreateCount();
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    EXPECT_EQ(GetCreateCount(), createCountBefore + 1);
    EXPECT_GT(GetFeedInputCount(), 0u);
    EXPECT_GT(GetGetOutputCount(), 0u);
}

// E2E-2: Active → Disable → algorithm stops
HWTEST_F(SessionEffectE2EFixture, Disable_EffectStopsAlgorithm, TestSize.Level0)
{
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    ASSERT_GT(GetFeedInputCount(), 0u);

    DisableEffect();
    usleep(WAIT_SHORT_US);

    uint32_t feedAfter = GetFeedInputCount();
    usleep(VERIFY_WAIT_US);
    EXPECT_EQ(GetFeedInputCount(), feedAfter);
}

// E2E-3: Lazy reuse within 3s
HWTEST_F(SessionEffectE2EFixture, LazyReuse_InstanceReused, TestSize.Level0)
{
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    uint32_t createCountAfterFirst = GetCreateCount();
    ASSERT_EQ(createCountAfterFirst, 1u);

    DisableEffect();
    usleep(WAIT_SHORT_US);

    // Re-enable within 3s — should reuse instance
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    EXPECT_EQ(GetCreateCount(), createCountAfterFirst);
    EXPECT_GT(GetFeedInputCount(), 0u);
}

// E2E-6: Lazy expire after 3s
HWTEST_F(SessionEffectE2EFixture, LazyExpire_NewInstanceCreated, TestSize.Level0)
{
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    ASSERT_EQ(GetCreateCount(), 1u);

    DisableEffect();
    usleep(WAIT_SHORT_US);

    // Wait for lazy release to expire (>3s)
    sleep(4);

    ResetFakeLibCounters();
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    EXPECT_EQ(GetCreateCount(), 1u);
    EXPECT_GT(GetFeedInputCount(), 0u);
}

// E2E-4: Toggle 3 times
HWTEST_F(SessionEffectE2EFixture, ToggleThrice_AllSwitchesSucceed, TestSize.Level0)
{
    // Round 1
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    ASSERT_GT(GetFeedInputCount(), 0u);
    DisableEffect();
    usleep(WAIT_SHORT_US);

    // Round 2
    ResetFakeLibCounters();
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    ASSERT_GT(GetFeedInputCount(), 0u);
    DisableEffect();
    usleep(WAIT_SHORT_US);

    // Round 3
    ResetFakeLibCounters();
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);
    EXPECT_GT(GetFeedInputCount(), 0u);
    EXPECT_GT(GetGetOutputCount(), 0u);
}

// E2E-5: Disable during WARMING_UP
HWTEST_F(SessionEffectE2EFixture, DisableDuringWarmup_NoCrash, TestSize.Level0)
{
    EnableEffect();
    usleep(WARMUP_WAIT_US);  // 50ms — still in early WARMING_UP

    DisableEffect();
    usleep(WAIT_SHORT_US);

    // Algorithm calls should stop
    uint32_t feedCount = GetFeedInputCount();
    usleep(VERIFY_WAIT_US);
    EXPECT_EQ(GetFeedInputCount(), feedCount);
}

// E2E-8: Active mode — algorithm round-trip
HWTEST_F(SessionEffectE2EFixture, ActiveMode_AlgorithmProcessesData, TestSize.Level0)
{
    EnableEffect();
    WaitUntilAlgorithmProcessing(5000);

    EXPECT_EQ(GetCreateCount(), 1u);
    EXPECT_GT(GetFeedInputCount(), 0u);
    EXPECT_GT(GetGetOutputCount(), 0u);

    uint32_t feedCount = GetFeedInputCount();
    uint32_t outputCount = GetGetOutputCount();
    EXPECT_GE(outputCount, feedCount / 2);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS