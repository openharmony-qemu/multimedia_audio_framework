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

#include "audio_stream_concurrency_detector.h"
#include "audio_info.h"

#include "aiut_log.h"

#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace testing::ext;

namespace {
constexpr int32_t TEST_APP_UID = 1001;
constexpr int32_t TEST_APP_PID = 2001;
constexpr int32_t TEST_APP_TOKEN_ID = 3001;
} // namespace

namespace OHOS {
namespace AudioStandard {

class AudioStreamConcurrencyDetectorAiut : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}

    static AudioProcessConfig GetTestProcessConfig(int32_t uid = TEST_APP_UID,
        AudioMode mode = AUDIO_MODE_PLAYBACK)
    {
        AudioProcessConfig config{};
        config.appInfo.appUid = uid;
        config.appInfo.appPid = TEST_APP_PID;
        config.appInfo.appTokenId = TEST_APP_TOKEN_ID;
        config.audioMode = mode;
        config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
        config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
        config.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
        config.streamInfo.channels = AudioChannel::STEREO;
        config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
        return config;
    }
};

/**
 * @tc.name  : AudioStreamConcurrencyDetector_GetInstance_ValidSingleton
 * @tc.number: AudioStreamConcurrencyDetector_GetInstance_ValidSingleton
 * @tc.desc  : Test GetInstance returns a valid singleton instance.
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_GetInstance_ValidSingleton, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& instance = AudioStreamConcurrencyDetector::GetInstance();
    AIUT_ASSERT_NE_INFO_LOG(&instance, nullptr, "GetInstance should return valid reference, ptr=%p", &instance);
    auto& instance2 = AudioStreamConcurrencyDetector::GetInstance();
    AIUT_EXPECT_EQ_INFO_LOG(&instance, &instance2,
        "GetInstance should return same singleton reference, ptr1=%p ptr2=%p", &instance, &instance2);
    (void)instance;
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_UpdateWriteTime_RecordMode
 * @tc.number: AudioStreamConcurrencyDetector_UpdateWriteTime_RecordMode
 * @tc.desc  : Test UpdateWriteTime when audioMode is RECORD (should return early).
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_UpdateWriteTime_RecordMode, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = true;
    detector.streamConcurrInfoMap_.clear();
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_RECORD);
    detector.UpdateWriteTime(config, 1);
    AIUT_EXPECT_TRUE_INFO_LOG(detector.streamConcurrInfoMap_.empty(),
        "streamConcurrInfoMap_ should remain empty for RECORD mode early return, size=%zu",
        detector.streamConcurrInfoMap_.size());
    detector.isEnabled_ = false;
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_UpdateWriteTime_Disabled
 * @tc.number: AudioStreamConcurrencyDetector_UpdateWriteTime_Disabled
 * @tc.desc  : Test UpdateWriteTime when isEnabled_ is false (should return early).
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_UpdateWriteTime_Disabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = false;
    detector.streamConcurrInfoMap_.clear();
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    detector.UpdateWriteTime(config, 2);
    AIUT_EXPECT_TRUE_INFO_LOG(detector.streamConcurrInfoMap_.empty(),
        "streamConcurrInfoMap_ should remain empty when disabled, size=%zu",
        detector.streamConcurrInfoMap_.size());
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_UpdateWriteTime_ValidConditions
 * @tc.number: AudioStreamConcurrencyDetector_UpdateWriteTime_ValidConditions
 * @tc.desc  : Test UpdateWriteTime with valid conditions, should update record.
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut,
    AudioStreamConcurrencyDetector_UpdateWriteTime_ValidConditions, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = true;
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    detector.streamConcurrInfoMap_.clear();
    detector.UpdateWriteTime(config, 3);
    AIUT_EXPECT_TRUE_INFO_LOG(
        !detector.streamConcurrInfoMap_.empty(),
        "streamConcurrInfoMap_ should not be empty after UpdateWriteTime");
    detector.isEnabled_ = false;
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_RemoveStream_RecordMode
 * @tc.number: AudioStreamConcurrencyDetector_RemoveStream_RecordMode
 * @tc.desc  : Test RemoveStream when audioMode is RECORD (should return early).
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_RemoveStream_RecordMode, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = true;
    detector.streamConcurrInfoMap_.clear();
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][1].startTime = 100;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][1].updateTime = 200;
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_RECORD);
    detector.RemoveStream(config, 1);
    AIUT_EXPECT_FALSE_INFO_LOG(detector.streamConcurrInfoMap_.empty(),
        "streamConcurrInfoMap_ should not be modified for RECORD mode early return, size=%zu",
        detector.streamConcurrInfoMap_.size());
    detector.isEnabled_ = false;
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_RemoveStream_Disabled
 * @tc.number: AudioStreamConcurrencyDetector_RemoveStream_Disabled
 * @tc.desc  : Test RemoveStream when isEnabled_ is false (should return early).
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_RemoveStream_Disabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = false;
    detector.streamConcurrInfoMap_.clear();
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][2].startTime = 100;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][2].updateTime = 200;
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    detector.RemoveStream(config, 2);
    AIUT_EXPECT_FALSE_INFO_LOG(detector.streamConcurrInfoMap_.empty(),
        "streamConcurrInfoMap_ should not be modified when disabled, size=%zu",
        detector.streamConcurrInfoMap_.size());
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_RemoveStream_UidNotInMap
 * @tc.number: AudioStreamConcurrencyDetector_RemoveStream_UidNotInMap
 * @tc.desc  : Test RemoveStream when uid is not in map (should return early).
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_RemoveStream_UidNotInMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = true;
    detector.streamConcurrInfoMap_.clear();
    AudioProcessConfig config = GetTestProcessConfig(9999, AUDIO_MODE_PLAYBACK);
    detector.RemoveStream(config, 1);
    AIUT_EXPECT_TRUE_INFO_LOG(detector.streamConcurrInfoMap_.empty(),
        "streamConcurrInfoMap_ should remain empty when uid not in map, size=%zu",
        detector.streamConcurrInfoMap_.size());
    detector.isEnabled_ = false;
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_RemoveStream_UsageNotInMap
 * @tc.number: AudioStreamConcurrencyDetector_RemoveStream_UsageNotInMap
 * @tc.desc  : Test RemoveStream when streamUsage is not in map (should return early).
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_RemoveStream_UsageNotInMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = true;
    detector.streamConcurrInfoMap_.clear();
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_ALARM][1].startTime = 100;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_ALARM][1].updateTime = 200;
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    detector.RemoveStream(config, 1);
    AIUT_EXPECT_EQ_INFO_LOG(detector.streamConcurrInfoMap_[1001].count(STREAM_USAGE_ALARM), 1u,
        "ALARM entry should be preserved when MEDIA not found, count=%zu",
        detector.streamConcurrInfoMap_[1001].count(STREAM_USAGE_ALARM));
    detector.isEnabled_ = false;
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_RemoveStream_StreamIdNotInMap
 * @tc.number: AudioStreamConcurrencyDetector_RemoveStream_StreamIdNotInMap
 * @tc.desc  : Test RemoveStream when streamId is not in map (should return early).
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut,
    AudioStreamConcurrencyDetector_RemoveStream_StreamIdNotInMap, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = true;
    detector.streamConcurrInfoMap_.clear();
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][1].startTime = 100;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][1].updateTime = 200;
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    detector.RemoveStream(config, 999);
    AIUT_EXPECT_EQ_INFO_LOG(detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA].count(1), 1u,
        "streamId=1 should be preserved when streamId=999 not found, count=%zu",
        detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA].count(1));
    detector.isEnabled_ = false;
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_RemoveStream_Success
 * @tc.number: AudioStreamConcurrencyDetector_RemoveStream_Success
 * @tc.desc  : Test RemoveStream successfully removes a stream entry.
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_RemoveStream_Success, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = true;
    detector.streamConcurrInfoMap_.clear();
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][1].startTime = 100;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][1].updateTime = 200;
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    detector.RemoveStream(config, 1);
    AIUT_EXPECT_TRUE_INFO_LOG(
        detector.streamConcurrInfoMap_.empty(),
        "streamConcurrInfoMap_ should be empty after removing last stream");
    detector.isEnabled_ = false;
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_RemoveStream_MultipleStreams
 * @tc.number: AudioStreamConcurrencyDetector_RemoveStream_MultipleStreams
 * @tc.desc  : Test RemoveStream when map has multiple streams, only removes one.
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut,
    AudioStreamConcurrencyDetector_RemoveStream_MultipleStreams, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.isEnabled_ = true;
    detector.streamConcurrInfoMap_.clear();
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][1].startTime = 100;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][1].updateTime = 200;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][2].startTime = 300;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][2].updateTime = 400;
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    detector.RemoveStream(config, 1);
    auto& usageMap = detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA];
    AIUT_EXPECT_EQ_INFO_LOG(usageMap.count(2), 1u, "stream 2 should still exist, count=%zu", usageMap.count(2));
    detector.isEnabled_ = false;
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_UpdateRecord_SetStartTime
 * @tc.number: AudioStreamConcurrencyDetector_UpdateRecord_SetStartTime
 * @tc.desc  : Test UpdateRecord sets startTime when it is 0.
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut, AudioStreamConcurrencyDetector_UpdateRecord_SetStartTime, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.streamConcurrInfoMap_.clear();
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    detector.UpdateRecord(config, 10);
    auto& info = detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][10];
    AIUT_EXPECT_TRUE_INFO_LOG(info.startTime != 0, "startTime should be set, startTime=%llu", info.startTime);
    AIUT_EXPECT_TRUE_INFO_LOG(info.updateTime != 0, "updateTime should be set, updateTime=%llu", info.updateTime);
    AIUT_EXPECT_EQ_INFO_LOG(info.startTime, info.updateTime,
        "startTime should equal updateTime on first call, startTime=%llu updateTime=%llu",
        info.startTime, info.updateTime);
    detector.streamConcurrInfoMap_.clear();
}

/**
 * @tc.name  : AudioStreamConcurrencyDetector_UpdateRecord_PreserveStartTime
 * @tc.number: AudioStreamConcurrencyDetector_UpdateRecord_PreserveStartTime
 * @tc.desc  : Test UpdateRecord does not change startTime when it is already set.
 */
HWTEST_F(AudioStreamConcurrencyDetectorAiut,
    AudioStreamConcurrencyDetector_UpdateRecord_PreserveStartTime, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto& detector = AudioStreamConcurrencyDetector::GetInstance();
    detector.streamConcurrInfoMap_.clear();
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][10].startTime = 1000;
    detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][10].updateTime = 1000;
    AudioProcessConfig config = GetTestProcessConfig(1001, AUDIO_MODE_PLAYBACK);
    detector.UpdateRecord(config, 10);
    auto& info = detector.streamConcurrInfoMap_[1001][STREAM_USAGE_MEDIA][10];
    AIUT_EXPECT_EQ_INFO_LOG(info.startTime, 1000ULL,
        "startTime should remain 1000, startTime=%llu", info.startTime);
    AIUT_EXPECT_TRUE_INFO_LOG(info.updateTime > 1000,
        "updateTime should be updated, updateTime=%llu", info.updateTime);
    detector.streamConcurrInfoMap_.clear();
}

} // namespace AudioStandard
} // namespace OHOS
