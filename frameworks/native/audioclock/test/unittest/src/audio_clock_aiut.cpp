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

#include "audio_clock_aiut.h"
#include "aiut_log.h"
#include "audio_source_clock.h"
#include "audio_primary_source_clock.h"
#include "capturer_clock.h"
#include "capturer_clock_manager.h"
#include "audio_utils.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_SAMPLE_RATE_48K = 48000;
constexpr uint32_t TEST_SAMPLE_RATE_96K = 96000;
constexpr uint32_t TEST_SESSION_ID_1 = 1;
constexpr uint32_t TEST_SESSION_ID_2 = 2;
constexpr uint64_t TEST_POSITION_INC = 960;
constexpr uint64_t TEST_TIMESTAMP_1 = 1'000'000'000;
constexpr uint64_t TEST_TIMESTAMP_2 = 1'020'000'000;
constexpr uint64_t TEST_TIMESTAMP_3 = 1'040'000'000;
constexpr uint64_t TEST_TIMESTAMP_4 = 1'100'000'000;
constexpr uint64_t TEST_TIMESTAMP_5 = 1'120'000'000;
constexpr uint64_t TEST_FIRST_TIMESTAMP = 1'000'000;
constexpr uint64_t TEST_POSITION_1 = 0;
constexpr uint64_t TEST_POSITION_2 = 960;
constexpr uint64_t TEST_POSITION_3 = 1920;
constexpr uint64_t TEST_POSITION_4 = 2880;
constexpr uint64_t TEST_POSITION_5 = 3840;
constexpr uint64_t TEST_TIMESTAMP_4_IN_CAPTURER = 1'120'000'000;
constexpr uint64_t TEST_TIMESTAMP_5_IN_CAPTURER = 1'140'000'000;

constexpr AudioSampleFormat TEST_FORMAT_S16 = AudioSampleFormat::SAMPLE_S16LE;
constexpr uint32_t TEST_CHANNEL_2 = 2;
constexpr uint64_t TEST_POSITION_INC_S16_STEREO = 960 * 2 * 2;
}

// ==================== CapturerClockAiut ====================

void CapturerClockAiut::SetUpTestCase(void) {}
void CapturerClockAiut::TearDownTestCase(void) {}
void CapturerClockAiut::SetUp(void) {}
void CapturerClockAiut::TearDown(void) {}

// ==================== CapturerClock Tests ====================

/**
 * @tc.name  : Test CapturerClock initial state is stopped
 * @tc.number: CapturerClock_InitialState_001
 * @tc.desc  : Test CapturerClock does not set timestamp before Start() is called.
 */
HWTEST_F(CapturerClockAiut, CapturerClock_InitialState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto clock = std::make_shared<CapturerClock>(TEST_SAMPLE_RATE_48K);
    AIUT_ASSERT_NE_INFO_LOG(clock, nullptr, "clock is null");

    clock->SetTimeStampByPosition(TEST_TIMESTAMP_1, TEST_SAMPLE_RATE_48K, TEST_POSITION_INC);
    AIUT_EXPECT_EQ_INFO_LOG(clock->timestamp_, static_cast<uint64_t>(0),
        "timestamp should be 0 before Start(), got=%llu",
        static_cast<unsigned long long>(clock->timestamp_));
}

/**
 * @tc.name  : Test CapturerClock Start/Stop and SetTimeStampByPosition
 * @tc.number: CapturerClock_StartStopTimestamp_001
 * @tc.desc  : Test CapturerClock sets and gets timestamps correctly after Start().
 */
HWTEST_F(CapturerClockAiut, CapturerClock_StartStopTimestamp_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto clock = std::make_shared<CapturerClock>(TEST_SAMPLE_RATE_48K);
    AIUT_ASSERT_NE_INFO_LOG(clock, nullptr, "clock is null");

    clock->Start();
    clock->SetTimeStampByPosition(TEST_TIMESTAMP_1, TEST_SAMPLE_RATE_48K, TEST_POSITION_INC);
    clock->SetTimeStampByPosition(TEST_TIMESTAMP_2, TEST_SAMPLE_RATE_48K, TEST_POSITION_INC);

    uint64_t timestamp = 0;
    bool result = clock->GetTimeStampByPosition(TEST_POSITION_1, timestamp);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "GetTimeStampByPosition should succeed");
    AIUT_EXPECT_EQ_INFO_LOG(timestamp, TEST_TIMESTAMP_1,
        "timestamp at pos 0 should match, got=%llu",
        static_cast<unsigned long long>(timestamp));

    result = clock->GetTimeStampByPosition(TEST_POSITION_2, timestamp);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "GetTimeStampByPosition should succeed");
    AIUT_EXPECT_EQ_INFO_LOG(timestamp, TEST_TIMESTAMP_2,
        "timestamp at pos 960 should match, got=%llu",
        static_cast<unsigned long long>(timestamp));

    clock->Stop();
}

/**
 * @tc.name  : Test CapturerClock interpolation between timestamps
 * @tc.number: CapturerClock_Interpolation_001
 * @tc.desc  : Test CapturerClock interpolates timestamps for positions between two anchors.
 */
HWTEST_F(CapturerClockAiut, CapturerClock_Interpolation_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto clock = std::make_shared<CapturerClock>(TEST_SAMPLE_RATE_48K);
    AIUT_ASSERT_NE_INFO_LOG(clock, nullptr, "clock is null");

    clock->Start();
    clock->SetTimeStampByPosition(TEST_TIMESTAMP_1, TEST_SAMPLE_RATE_48K, TEST_POSITION_INC);
    clock->SetTimeStampByPosition(TEST_TIMESTAMP_2, TEST_SAMPLE_RATE_48K, TEST_POSITION_INC);

    uint64_t timestamp = 0;
    clock->GetTimeStampByPosition(TEST_POSITION_3, timestamp);
    AIUT_EXPECT_EQ_INFO_LOG(timestamp, TEST_TIMESTAMP_3,
        "interpolated timestamp should match, got=%llu",
        static_cast<unsigned long long>(timestamp));

    clock->Stop();
}

/**
 * @tc.name  : Test CapturerClock with different sample rate
 * @tc.number: CapturerClock_DifferentSampleRate_001
 * @tc.desc  : Test CapturerClock with different source and capturer sample rates.
 */
HWTEST_F(CapturerClockAiut, CapturerClock_DifferentSampleRate_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto clock = std::make_shared<CapturerClock>(TEST_SAMPLE_RATE_48K);
    AIUT_ASSERT_NE_INFO_LOG(clock, nullptr, "clock is null");

    clock->Start();
    clock->logTimestamp_ = 0;
    clock->SetTimeStampByPosition(TEST_TIMESTAMP_4, TEST_SAMPLE_RATE_96K, TEST_POSITION_INC * 2);
    clock->SetTimeStampByPosition(TEST_TIMESTAMP_5, TEST_SAMPLE_RATE_96K, TEST_POSITION_INC * 2);

    uint64_t timestamp = 0;
    clock->GetTimeStampByPosition(TEST_POSITION_4, timestamp);
    AIUT_EXPECT_EQ_INFO_LOG(timestamp, TEST_TIMESTAMP_4_IN_CAPTURER,
        "timestamp should be adjusted for sample rate, got=%llu",
        static_cast<unsigned long long>(timestamp));

    clock->GetTimeStampByPosition(TEST_POSITION_5, timestamp);
    AIUT_EXPECT_EQ_INFO_LOG(timestamp, TEST_TIMESTAMP_5_IN_CAPTURER,
        "timestamp should be adjusted for sample rate, got=%llu",
        static_cast<unsigned long long>(timestamp));
}

// ==================== AudioSourceClockAiut ====================

void AudioSourceClockAiut::SetUpTestCase(void) {}
void AudioSourceClockAiut::TearDownTestCase(void) {}
void AudioSourceClockAiut::SetUp(void) {}
void AudioSourceClockAiut::TearDown(void) {}

// ==================== AudioSourceClock Tests ====================

/**
 * @tc.name  : Test AudioSourceClock Init with S16LE format
 * @tc.number: AudioSourceClock_InitS16_001
 * @tc.desc  : Test AudioSourceClock::Init calculates sizePerPos correctly for S16LE stereo.
 */
HWTEST_F(AudioSourceClockAiut, AudioSourceClock_InitS16_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    srcClock->Init(TEST_SAMPLE_RATE_48K, TEST_FORMAT_S16, TEST_CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->sizePerPos_,
        static_cast<uint32_t>(BYTE_SIZE_SAMPLE_S16 * TEST_CHANNEL_2),
        "sizePerPos should be 4 for S16LE stereo, got=%u", srcClock->sizePerPos_);
}

/**
 * @tc.name  : Test AudioSourceClock Init with U8 format
 * @tc.number: AudioSourceClock_InitU8_001
 * @tc.desc  : Test AudioSourceClock::Init calculates sizePerPos correctly for U8 format.
 */
HWTEST_F(AudioSourceClockAiut, AudioSourceClock_InitU8_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    srcClock->Init(TEST_SAMPLE_RATE_48K, AudioSampleFormat::SAMPLE_U8, TEST_CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->sizePerPos_,
        static_cast<uint32_t>(BYTE_SIZE_SAMPLE_U8 * TEST_CHANNEL_2),
        "sizePerPos should be 2 for U8 stereo, got=%u", srcClock->sizePerPos_);
}

/**
 * @tc.name  : Test AudioSourceClock Init with S24LE format
 * @tc.number: AudioSourceClock_InitS24_001
 * @tc.desc  : Test AudioSourceClock::Init calculates sizePerPos correctly for S24LE format.
 */
HWTEST_F(AudioSourceClockAiut, AudioSourceClock_InitS24_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    srcClock->Init(TEST_SAMPLE_RATE_48K, AudioSampleFormat::SAMPLE_S24LE, TEST_CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->sizePerPos_,
        static_cast<uint32_t>(BYTE_SIZE_SAMPLE_S24 * TEST_CHANNEL_2),
        "sizePerPos should be 6 for S24LE stereo, got=%u", srcClock->sizePerPos_);
}

/**
 * @tc.name  : Test AudioSourceClock Init with S32LE format
 * @tc.number: AudioSourceClock_InitS32_001
 * @tc.desc  : Test AudioSourceClock::Init calculates sizePerPos correctly for S32LE format.
 */
HWTEST_F(AudioSourceClockAiut, AudioSourceClock_InitS32_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    srcClock->Init(TEST_SAMPLE_RATE_48K, AudioSampleFormat::SAMPLE_S32LE, TEST_CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->sizePerPos_,
        static_cast<uint32_t>(BYTE_SIZE_SAMPLE_S32 * TEST_CHANNEL_2),
        "sizePerPos should be 8 for S32LE stereo, got=%u", srcClock->sizePerPos_);
}

/**
 * @tc.name  : Test AudioSourceClock Init with INVALID_WIDTH defaults to S16
 * @tc.number: AudioSourceClock_InitInvalid_001
 * @tc.desc  : Test AudioSourceClock::Init defaults to S16 byte size for INVALID_WIDTH format.
 */
HWTEST_F(AudioSourceClockAiut, AudioSourceClock_InitInvalid_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    srcClock->Init(TEST_SAMPLE_RATE_48K, AudioSampleFormat::INVALID_WIDTH, TEST_CHANNEL_2);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->sizePerPos_,
        static_cast<uint32_t>(BYTE_SIZE_SAMPLE_S16 * TEST_CHANNEL_2),
        "INVALID_WIDTH should default to S16, sizePerPos=%u", srcClock->sizePerPos_);
}

/**
 * @tc.name  : Test AudioSourceClock UpdateSessionId
 * @tc.number: AudioSourceClock_UpdateSessionId_001
 * @tc.desc  : Test AudioSourceClock::UpdateSessionId updates the session ID list.
 */
HWTEST_F(AudioSourceClockAiut, AudioSourceClock_UpdateSessionId_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    std::vector<int32_t> sessionIdList = {1, 2, 3};
    srcClock->UpdateSessionId(sessionIdList);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->sessionIdList_.size(), static_cast<size_t>(3),
        "sessionIdList should have 3 elements, got=%zu", srcClock->sessionIdList_.size());
}

/**
 * @tc.name  : Test AudioSourceClock GetTimestamp returns non-zero
 * @tc.number: AudioSourceClock_GetTimestamp_001
 * @tc.desc  : Test AudioSourceClock::GetTimestamp returns a positive nanosecond value.
 */
HWTEST_F(AudioSourceClockAiut, AudioSourceClock_GetTimestamp_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    uint64_t ts = srcClock->GetTimestamp(0);
    AIUT_EXPECT_TRUE_INFO_LOG(ts > 0, "GetTimestamp should return positive value");
}

/**
 * @tc.name  : Test AudioSourceClock Renew with no session IDs
 * @tc.number: AudioSourceClock_RenewNoSession_001
 * @tc.desc  : Test AudioSourceClock::Renew with empty session list does not crash.
 */
HWTEST_F(AudioSourceClockAiut, AudioSourceClock_RenewNoSession_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");
    srcClock->Init(TEST_SAMPLE_RATE_48K, TEST_FORMAT_S16, TEST_CHANNEL_2);

    // Renew with no sessions registered - should not crash
    srcClock->logTimestamp_ = 0;
    srcClock->Renew(TEST_POSITION_INC_S16_STEREO);
    // No assertion needed beyond "doesn't crash"
    AIUT_EXPECT_TRUE_INFO_LOG(true, "Renew with no sessions completed without crash");
}

// ==================== AudioPrimarySourceClockAiut ====================

void AudioPrimarySourceClockAiut::SetUpTestCase(void) {}
void AudioPrimarySourceClockAiut::TearDownTestCase(void) {}
void AudioPrimarySourceClockAiut::SetUp(void) {}
void AudioPrimarySourceClockAiut::TearDown(void) {}

// ==================== AudioCapturerSourceClock Tests ====================

/**
 * @tc.name  : Test AudioCapturerSourceClock initial GetFrameCnt is zero
 * @tc.number: PrimarySourceClock_InitialFrameCnt_001
 * @tc.desc  : Test AudioCapturerSourceClock::GetFrameCnt returns 0 after construction.
 */
HWTEST_F(AudioPrimarySourceClockAiut, PrimarySourceClock_InitialFrameCnt_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioCapturerSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    AIUT_EXPECT_EQ_INFO_LOG(srcClock->GetFrameCnt(), static_cast<uint32_t>(0),
        "initial frame count should be 0, got=%u", srcClock->GetFrameCnt());
}

/**
 * @tc.name  : Test AudioCapturerSourceClock SetFirstTimestampFromHdi and Renew
 * @tc.number: PrimarySourceClock_SetHdiTimestamp_001
 * @tc.desc  : Test AudioCapturerSourceClock sets HDI timestamp and increments frame count.
 */
HWTEST_F(AudioPrimarySourceClockAiut, PrimarySourceClock_SetHdiTimestamp_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioCapturerSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");
    srcClock->Init(TEST_SAMPLE_RATE_48K, TEST_FORMAT_S16, TEST_CHANNEL_2);

    srcClock->SetFirstTimestampFromHdi(TEST_FIRST_TIMESTAMP);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->firstTimeStamp_, TEST_FIRST_TIMESTAMP,
        "firstTimeStamp should match HDI value");

    srcClock->Renew(0);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->lastTs_, static_cast<uint64_t>(0),
        "lastTs should be 0 after Renew(0)");

    srcClock->Renew(TEST_POSITION_INC_S16_STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->GetFrameCnt(), static_cast<uint32_t>(1),
        "frame count should be 1 after one Renew");
    AIUT_EXPECT_TRUE_INFO_LOG(srcClock->lastTs_ > 0, "lastTs should be positive after Renew");
}

/**
 * @tc.name  : Test AudioCapturerSourceClock switches to system clock
 * @tc.number: PrimarySourceClock_SwitchToSystemClock_001
 * @tc.desc  : Test AudioCapturerSourceClock switches to system clock after 20 frames.
 */
HWTEST_F(AudioPrimarySourceClockAiut, PrimarySourceClock_SwitchToSystemClock_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioCapturerSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");
    srcClock->Init(TEST_SAMPLE_RATE_48K, TEST_FORMAT_S16, TEST_CHANNEL_2);

    srcClock->SetFirstTimestampFromHdi(TEST_FIRST_TIMESTAMP);
    AIUT_EXPECT_FALSE_INFO_LOG(srcClock->isGetTimeStampFromSystemClock_,
        "should start with HDI timestamp mode");

    for (size_t i = 0; i < 10; i++) {
        srcClock->Renew(TEST_POSITION_INC_S16_STEREO);
    }
    AIUT_EXPECT_FALSE_INFO_LOG(srcClock->isGetTimeStampFromSystemClock_,
        "should still be in HDI mode after 10 frames");

    for (size_t i = 0; i < 20; i++) {
        srcClock->Renew(TEST_POSITION_INC_S16_STEREO);
    }
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->GetFrameCnt(), static_cast<uint32_t>(21),
        "frame count should be 21, got=%u", srcClock->GetFrameCnt());
    AIUT_EXPECT_TRUE_INFO_LOG(srcClock->isGetTimeStampFromSystemClock_,
        "should switch to system clock after 20 frames");
}

/**
 * @tc.name  : Test AudioCapturerSourceClock zero HDI timestamp triggers system clock
 * @tc.number: PrimarySourceClock_ZeroHdiTimestamp_001
 * @tc.desc  : Test AudioCapturerSourceClock switches to system clock when HDI timestamp is 0.
 */
HWTEST_F(AudioPrimarySourceClockAiut, PrimarySourceClock_ZeroHdiTimestamp_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioCapturerSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");
    srcClock->Init(TEST_SAMPLE_RATE_48K, TEST_FORMAT_S16, TEST_CHANNEL_2);

    srcClock->SetFirstTimestampFromHdi(0);
    srcClock->Renew(TEST_POSITION_INC_S16_STEREO);
    AIUT_EXPECT_TRUE_INFO_LOG(srcClock->isGetTimeStampFromSystemClock_,
        "should switch to system clock when HDI timestamp is 0");
}

/**
 * @tc.name  : Test AudioCapturerSourceClock Reset clears state
 * @tc.number: PrimarySourceClock_Reset_001
 * @tc.desc  : Test AudioCapturerSourceClock::Reset clears all internal state.
 */
HWTEST_F(AudioPrimarySourceClockAiut, PrimarySourceClock_Reset_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto srcClock = std::make_shared<AudioCapturerSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");
    srcClock->Init(TEST_SAMPLE_RATE_48K, TEST_FORMAT_S16, TEST_CHANNEL_2);

    srcClock->SetFirstTimestampFromHdi(TEST_FIRST_TIMESTAMP);
    for (size_t i = 0; i < 5; i++) {
        srcClock->Renew(TEST_POSITION_INC_S16_STEREO);
    }
    AIUT_EXPECT_TRUE_INFO_LOG(srcClock->GetFrameCnt() > 0, "frame count should be > 0 before reset");

    srcClock->Reset();
    AIUT_EXPECT_EQ_INFO_LOG(srcClock->GetFrameCnt(), static_cast<uint32_t>(0),
        "frame count should be 0 after Reset");
}

// ==================== CapturerClockManagerAiut ====================

void CapturerClockManagerAiut::SetUpTestCase(void) {}
void CapturerClockManagerAiut::TearDownTestCase(void) {}
void CapturerClockManagerAiut::SetUp(void) {}
void CapturerClockManagerAiut::TearDown(void) {}

// ==================== CapturerClockManager Tests ====================

/**
 * @tc.name  : Test CapturerClockManager CreateCapturerClock
 * @tc.number: ClockManager_CreateCapturerClock_001
 * @tc.desc  : Test CapturerClockManager::CreateCapturerClock creates and stores a clock.
 */
HWTEST_F(CapturerClockManagerAiut, ClockManager_CreateCapturerClock_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = CapturerClockManager::GetInstance();

    auto clock = manager.CreateCapturerClock(TEST_SESSION_ID_1, TEST_SAMPLE_RATE_48K);
    AIUT_ASSERT_NE_INFO_LOG(clock, nullptr, "created clock should not be null");

    auto retrieved = manager.GetCapturerClock(TEST_SESSION_ID_1);
    AIUT_ASSERT_NE_INFO_LOG(retrieved, nullptr, "retrieved clock should not be null");
    AIUT_EXPECT_EQ_INFO_LOG(retrieved.get(), clock.get(),
        "retrieved clock should be the same instance");

    manager.DeleteCapturerClock(TEST_SESSION_ID_1);
}

/**
 * @tc.name  : Test CapturerClockManager GetCapturerClock for unknown session
 * @tc.number: ClockManager_GetUnknownClock_001
 * @tc.desc  : Test CapturerClockManager::GetCapturerClock returns nullptr for unknown session.
 */
HWTEST_F(CapturerClockManagerAiut, ClockManager_GetUnknownClock_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = CapturerClockManager::GetInstance();

    auto clock = manager.GetCapturerClock(9999);
    AIUT_EXPECT_EQ_INFO_LOG(clock, nullptr,
        "GetCapturerClock should return nullptr for unknown session");
}

/**
 * @tc.name  : Test CapturerClockManager DeleteCapturerClock
 * @tc.number: ClockManager_DeleteCapturerClock_001
 * @tc.desc  : Test CapturerClockManager::DeleteCapturerClock removes the clock.
 */
HWTEST_F(CapturerClockManagerAiut, ClockManager_DeleteCapturerClock_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = CapturerClockManager::GetInstance();

    manager.CreateCapturerClock(TEST_SESSION_ID_1, TEST_SAMPLE_RATE_48K);
    AIUT_EXPECT_NE_INFO_LOG(manager.GetCapturerClock(TEST_SESSION_ID_1), nullptr,
        "clock should exist before delete");

    manager.DeleteCapturerClock(TEST_SESSION_ID_1);
    AIUT_EXPECT_EQ_INFO_LOG(manager.GetCapturerClock(TEST_SESSION_ID_1), nullptr,
        "clock should be null after delete");
}

/**
 * @tc.name  : Test CapturerClockManager RegisterAudioSourceClock
 * @tc.number: ClockManager_RegisterSourceClock_001
 * @tc.desc  : Test CapturerClockManager::RegisterAudioSourceClock registers and retrieves.
 */
HWTEST_F(CapturerClockManagerAiut, ClockManager_RegisterSourceClock_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = CapturerClockManager::GetInstance();

    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    bool result = manager.RegisterAudioSourceClock(TEST_SESSION_ID_1, srcClock);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "RegisterAudioSourceClock should succeed");

    auto retrieved = manager.GetAudioSourceClock(TEST_SESSION_ID_1);
    AIUT_ASSERT_NE_INFO_LOG(retrieved, nullptr, "retrieved source clock should not be null");

    manager.DeleteAudioSourceClock(TEST_SESSION_ID_1);
}

/**
 * @tc.name  : Test CapturerClockManager duplicate RegisterAudioSourceClock fails
 * @tc.number: ClockManager_DuplicateRegister_001
 * @tc.desc  : Test CapturerClockManager::RegisterAudioSourceClock returns false
 *             when registering the same session ID twice.
 */
HWTEST_F(CapturerClockManagerAiut, ClockManager_DuplicateRegister_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = CapturerClockManager::GetInstance();

    auto srcClock = std::make_shared<AudioSourceClock>();
    AIUT_ASSERT_NE_INFO_LOG(srcClock, nullptr, "srcClock is null");

    bool result1 = manager.RegisterAudioSourceClock(TEST_SESSION_ID_2, srcClock);
    AIUT_EXPECT_TRUE_INFO_LOG(result1, "first register should succeed");

    bool result2 = manager.RegisterAudioSourceClock(TEST_SESSION_ID_2, srcClock);
    AIUT_EXPECT_FALSE_INFO_LOG(result2, "duplicate register should fail");

    manager.DeleteAudioSourceClock(TEST_SESSION_ID_2);
}

/**
 * @tc.name  : Test CapturerClockManager DeleteAudioSourceClock
 * @tc.number: ClockManager_DeleteSourceClock_001
 * @tc.desc  : Test CapturerClockManager::DeleteAudioSourceClock removes the source clock.
 */
HWTEST_F(CapturerClockManagerAiut, ClockManager_DeleteSourceClock_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = CapturerClockManager::GetInstance();

    auto srcClock = std::make_shared<AudioSourceClock>();
    manager.RegisterAudioSourceClock(TEST_SESSION_ID_1, srcClock);
    AIUT_EXPECT_NE_INFO_LOG(manager.GetAudioSourceClock(TEST_SESSION_ID_1), nullptr,
        "source clock should exist before delete");

    manager.DeleteAudioSourceClock(TEST_SESSION_ID_1);
    AIUT_EXPECT_EQ_INFO_LOG(manager.GetAudioSourceClock(TEST_SESSION_ID_1), nullptr,
        "source clock should be null after delete");
}

/**
 * @tc.name  : Test CapturerClockManager GetAudioSourceClock for unknown
 * @tc.number: ClockManager_GetUnknownSourceClock_001
 * @tc.desc  : Test CapturerClockManager::GetAudioSourceClock returns nullptr for
 *             unknown capture ID.
 */
HWTEST_F(CapturerClockManagerAiut, ClockManager_GetUnknownSourceClock_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto &manager = CapturerClockManager::GetInstance();

    auto clock = manager.GetAudioSourceClock(9999);
    AIUT_EXPECT_EQ_INFO_LOG(clock, nullptr,
        "GetAudioSourceClock should return nullptr for unknown capture ID");
}

} // namespace AudioStandard
} // namespace OHOS
