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

#include "linear_pos_time_model.h"
#include "aiut_log.h"
#include "audio_service_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_SAMPLE_RATE = 44100;
constexpr int32_t TEST_SAMPLE_RATE_INVALID_LOW = 0;
constexpr int32_t TEST_SAMPLE_RATE_INVALID_HIGH = 400000;
constexpr uint64_t TEST_FRAME_0 = 0;
constexpr uint64_t TEST_FRAME_44100 = 44100;
constexpr uint64_t TEST_FRAME_88200 = 88200;
constexpr uint64_t TEST_FRAME_132300 = 132300;
constexpr uint64_t TEST_FRAME_176400 = 176400;
constexpr uint64_t TEST_FRAME_220500 = 220500;
constexpr int64_t TEST_NANO_TIME_0 = 0;
constexpr int64_t TEST_NANO_TIME_1S = 1000000000;
constexpr int64_t TEST_NANO_TIME_2S = 2000000000;
constexpr int64_t TEST_NANO_TIME_3S = 3000000000;
constexpr int64_t TEST_NANO_TIME_4S = 4000000000;
constexpr int64_t TEST_NANO_TIME_5S = 5000000000;
constexpr int64_t TEST_NANO_TIME_6S = 6000000000;
constexpr int64_t TEST_NANO_TIME_7S = 7000000000;
constexpr int64_t TEST_NANO_TIME_8S = 8000000000;
constexpr int64_t TEST_NANO_TIME_INVALID = -1;
constexpr uint64_t TEST_SPAN_COUNT = 1024;
}  // namespace

class LinearPosTimeModelAiut : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() {}
    void TearDown() {}
};

// ==================== ConfigSampleRate Tests ====================

/**
 * @tc.name  : Test ConfigSampleRate with valid rate
 * @tc.number: LinearPosTimeModel_ConfigSampleRate_Valid
 * @tc.desc  : Test ConfigSampleRate returns true with valid sample rate.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_ConfigSampleRate_Valid, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    bool ret = model.ConfigSampleRate(TEST_SAMPLE_RATE);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "ConfigSampleRate should return true for 44100");
}

/**
 * @tc.name  : Test ConfigSampleRate with already configed
 * @tc.number: LinearPosTimeModel_ConfigSampleRate_AlreadyConfiged
 * @tc.desc  : Test ConfigSampleRate returns false when already configed.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_ConfigSampleRate_AlreadyConfiged, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    bool ret = model.ConfigSampleRate(48000);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "ConfigSampleRate should return false when already configed");
}

/**
 * @tc.name  : Test ConfigSampleRate with invalid low rate
 * @tc.number: LinearPosTimeModel_ConfigSampleRate_InvalidLow
 * @tc.desc  : Test ConfigSampleRate returns false with rate <= 0.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_ConfigSampleRate_InvalidLow, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    bool ret = model.ConfigSampleRate(TEST_SAMPLE_RATE_INVALID_LOW);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "ConfigSampleRate should return false for rate 0");
}

/**
 * @tc.name  : Test ConfigSampleRate with invalid high rate
 * @tc.number: LinearPosTimeModel_ConfigSampleRate_InvalidHigh
 * @tc.desc  : Test ConfigSampleRate returns false with rate > MAX_SUPPORT_SAMPLE_RATE.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_ConfigSampleRate_InvalidHigh, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    bool ret = model.ConfigSampleRate(TEST_SAMPLE_RATE_INVALID_HIGH);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "ConfigSampleRate should return false for rate > 384000");
}

// ==================== GetFrameStamp Tests ====================

/**
 * @tc.name  : Test GetFrameStamp when not configed
 * @tc.number: LinearPosTimeModel_GetFrameStamp_NotConfiged
 * @tc.desc  : Test GetFrameStamp returns false when sample rate not configed.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_GetFrameStamp_NotConfiged, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    uint64_t frame = 0;
    int64_t nanoTime = 0;
    bool ret = model.GetFrameStamp(frame, nanoTime);
    AIUT_EXPECT_FALSE_INFO_LOG(ret, "GetFrameStamp should return false when not configed");
}

/**
 * @tc.name  : Test GetFrameStamp after ResetFrameStamp
 * @tc.number: LinearPosTimeModel_GetFrameStamp_AfterReset
 * @tc.desc  : Test GetFrameStamp returns true and correct values after ResetFrameStamp.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_GetFrameStamp_AfterReset, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_44100, TEST_NANO_TIME_1S);
    uint64_t frame = 0;
    int64_t nanoTime = 0;
    bool ret = model.GetFrameStamp(frame, nanoTime);
    AIUT_EXPECT_TRUE_INFO_LOG(ret, "GetFrameStamp should return true after config");
    AIUT_EXPECT_EQ_INFO_LOG(frame, TEST_FRAME_44100, "frame should be 44100");
    AIUT_EXPECT_EQ_INFO_LOG(nanoTime, TEST_NANO_TIME_1S, "nanoTime should be 1s");
}

// ==================== ResetFrameStamp Tests ====================

/**
 * @tc.name  : Test ResetFrameStamp clears state
 * @tc.number: LinearPosTimeModel_ResetFrameStamp
 * @tc.desc  : Test ResetFrameStamp sets new stamp frame and time.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_ResetFrameStamp, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    uint64_t frame = 0;
    int64_t nanoTime = 0;
    model.GetFrameStamp(frame, nanoTime);
    AIUT_EXPECT_EQ_INFO_LOG(frame, TEST_FRAME_0, "frame should be 0");
    AIUT_EXPECT_EQ_INFO_LOG(nanoTime, TEST_NANO_TIME_0, "nanoTime should be 0");
}

// ==================== SetSpanCount Tests ====================

/**
 * @tc.name  : Test SetSpanCount
 * @tc.number: LinearPosTimeModel_SetSpanCount
 * @tc.desc  : Test SetSpanCount sets the span count without breaking model functionality.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_SetSpanCount, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    model.SetSpanCount(TEST_SPAN_COUNT);
    int64_t time = model.GetTimeOfPos(TEST_FRAME_44100);
    AIUT_EXPECT_EQ_INFO_LOG(time, TEST_NANO_TIME_1S,
        "GetTimeOfPos should work after SetSpanCount, time=%lld", time);
}

// ==================== GetTimeOfPos Tests ====================

/**
 * @tc.name  : Test GetTimeOfPos when not configed
 * @tc.number: LinearPosTimeModel_GetTimeOfPos_NotConfiged
 * @tc.desc  : Test GetTimeOfPos returns -1 when not configed.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_GetTimeOfPos_NotConfiged, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    int64_t time = model.GetTimeOfPos(TEST_FRAME_44100);
    AIUT_EXPECT_EQ_INFO_LOG(time, TEST_NANO_TIME_INVALID,
        "GetTimeOfPos should return -1 when not configed");
}

/**
 * @tc.name  : Test GetTimeOfPos with forward position
 * @tc.number: LinearPosTimeModel_GetTimeOfPos_Forward
 * @tc.desc  : Test GetTimeOfPos with posInFrame >= stampFrame_.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_GetTimeOfPos_Forward, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    int64_t time = model.GetTimeOfPos(TEST_FRAME_44100);
    AIUT_EXPECT_EQ_INFO_LOG(time, TEST_NANO_TIME_1S,
        "GetTimeOfPos(44100) should return 1s, got time=%lld", time);
}

/**
 * @tc.name  : Test GetTimeOfPos with backward position
 * @tc.number: LinearPosTimeModel_GetTimeOfPos_Backward
 * @tc.desc  : Test GetTimeOfPos with posInFrame < stampFrame_.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_GetTimeOfPos_Backward, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_44100, TEST_NANO_TIME_1S);
    int64_t time = model.GetTimeOfPos(TEST_FRAME_0);
    AIUT_EXPECT_EQ_INFO_LOG(time, TEST_NANO_TIME_0,
        "GetTimeOfPos(0) should return 0, got time=%lld", time);
}

/**
 * @tc.name  : Test GetTimeOfPos with too large position
 * @tc.number: LinearPosTimeModel_GetTimeOfPos_TooLarge
 * @tc.desc  : Test GetTimeOfPos with posInFrame - stampFrame_ >= sampleRate.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_GetTimeOfPos_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    uint64_t largePos = static_cast<uint64_t>(TEST_SAMPLE_RATE) + TEST_FRAME_44100;
    int64_t time = model.GetTimeOfPos(largePos);
    AIUT_EXPECT_EQ_INFO_LOG(time, TEST_NANO_TIME_2S,
        "GetTimeOfPos should still return calculated time for large pos");
}

/**
 * @tc.name  : Test GetTimeOfPos with too small position
 * @tc.number: LinearPosTimeModel_GetTimeOfPos_TooSmall
 * @tc.desc  : Test GetTimeOfPos with stampFrame_ - posInFrame >= sampleRate.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_GetTimeOfPos_TooSmall, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_44100, TEST_NANO_TIME_1S);
    int64_t time = model.GetTimeOfPos(TEST_FRAME_0);
    AIUT_EXPECT_EQ_INFO_LOG(time, TEST_NANO_TIME_0,
        "GetTimeOfPos should return calculated time for small pos");
}

// ==================== UpdateFrameStamp Tests ====================

/**
 * @tc.name  : Test UpdateFrameStamp with reasonable data
 * @tc.number: LinearPosTimeModel_UpdateFrameStamp_Reasonable
 * @tc.desc  : Test UpdateFrameStamp returns CHECK_SUCCESS with reasonable data.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_UpdateFrameStamp_Reasonable, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    CheckPosTimeRes ret = model.UpdateFrameStamp(TEST_FRAME_44100, TEST_NANO_TIME_1S);
    AIUT_EXPECT_EQ_INFO_LOG(ret, CHECK_SUCCESS,
        "UpdateFrameStamp reasonable should return CHECK_SUCCESS");
    uint64_t frame = 0;
    int64_t nanoTime = 0;
    model.GetFrameStamp(frame, nanoTime);
    AIUT_EXPECT_EQ_INFO_LOG(frame, TEST_FRAME_44100, "stampFrame should be updated");
}

/**
 * @tc.name  : Test UpdateFrameStamp with same frame and time
 * @tc.number: LinearPosTimeModel_UpdateFrameStamp_Same
 * @tc.desc  : Test UpdateFrameStamp returns CHECK_SUCCESS with same frame and time.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_UpdateFrameStamp_Same, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    CheckPosTimeRes ret = model.UpdateFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    AIUT_EXPECT_EQ_INFO_LOG(ret, CHECK_SUCCESS,
        "UpdateFrameStamp same should return CHECK_SUCCESS");
}

/**
 * @tc.name  : Test UpdateFrameStamp with unreasonable data returns CHECK_FAILED
 * @tc.number: LinearPosTimeModel_UpdateFrameStamp_Unreasonable_CheckFailed
 * @tc.desc  : Test UpdateFrameStamp returns CHECK_FAILED with unreasonable data
 *             when statistics count < MAX_STATISTICS_COUNT.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_UpdateFrameStamp_Unreasonable_CheckFailed, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    CheckPosTimeRes ret = model.UpdateFrameStamp(TEST_FRAME_44100, TEST_NANO_TIME_2S);
    AIUT_EXPECT_EQ_INFO_LOG(ret, CHECK_FAILED,
        "UpdateFrameStamp unreasonable should return CHECK_FAILED");
}

/**
 * @tc.name  : Test UpdateFrameStamp with unreasonable data returns NEED_MODIFY
 * @tc.number: LinearPosTimeModel_UpdateFrameStamp_Unreasonable_NeedModify
 * @tc.desc  : Test UpdateFrameStamp returns NEED_MODIFY when 5 consecutive unreasonable
 *             data points are reasonable relative to each other.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_UpdateFrameStamp_Unreasonable_NeedModify, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    model.UpdateFrameStamp(TEST_FRAME_44100, TEST_NANO_TIME_2S);
    model.UpdateFrameStamp(TEST_FRAME_88200, TEST_NANO_TIME_3S);
    model.UpdateFrameStamp(TEST_FRAME_132300, TEST_NANO_TIME_4S);
    model.UpdateFrameStamp(TEST_FRAME_176400, TEST_NANO_TIME_5S);
    CheckPosTimeRes ret = model.UpdateFrameStamp(TEST_FRAME_220500, TEST_NANO_TIME_6S);
    AIUT_EXPECT_EQ_INFO_LOG(ret, NEED_MODIFY,
        "UpdateFrameStamp should return NEED_MODIFY after 5 reasonable pairs");
}

/**
 * @tc.name  : Test UpdateFrameStamp with unreasonable pairs returns CHECK_FAILED
 * @tc.number: LinearPosTimeModel_UpdateFrameStamp_UnreasonablePairs
 * @tc.desc  : Test UpdateFrameStamp returns CHECK_FAILED when pairs are unreasonable
 *             relative to each other.
 * @tc.owner : audio_route_common
 */
HWTEST_F(LinearPosTimeModelAiut, LinearPosTimeModel_UpdateFrameStamp_UnreasonablePairs, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    LinearPosTimeModel model;
    model.ConfigSampleRate(TEST_SAMPLE_RATE);
    model.ResetFrameStamp(TEST_FRAME_0, TEST_NANO_TIME_0);
    // These pairs have unreasonable delta time between them
    model.UpdateFrameStamp(TEST_FRAME_44100, TEST_NANO_TIME_2S);
    model.UpdateFrameStamp(TEST_FRAME_88200, TEST_NANO_TIME_5S);
    model.UpdateFrameStamp(TEST_FRAME_132300, TEST_NANO_TIME_6S);
    model.UpdateFrameStamp(TEST_FRAME_176400, TEST_NANO_TIME_7S);
    CheckPosTimeRes ret = model.UpdateFrameStamp(TEST_FRAME_220500, TEST_NANO_TIME_8S);
    AIUT_EXPECT_EQ_INFO_LOG(ret, CHECK_FAILED,
        "UpdateFrameStamp should return CHECK_FAILED for unreasonable pairs");
}

}  // namespace AudioStandard
}  // namespace OHOS
