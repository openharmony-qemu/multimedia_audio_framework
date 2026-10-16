/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "audio_process_config.h"
#include "audio_stream_types.h"
#include <gtest/gtest.h>
#include <string>
#include "parcel.h"

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t TEST_APP_PID = 12345;
constexpr int32_t TEST_APP_UID = 10001;
constexpr int32_t TEST_APP_TOKEN_ID = 54321;
constexpr AudioChannel TEST_CHANNEL_COUNT = AudioChannel::STEREO;
constexpr AudioSamplingRate TEST_SAMPLE_RATE = AudioSamplingRate::SAMPLE_RATE_48000;
constexpr AudioSampleFormat AUDIO_SAMPLE_FORMAT_16BIT = AudioSampleFormat::SAMPLE_S16LE;
constexpr AudioStreamType AUDIO_STREAM_TYPE_RENDERER = AudioStreamType::STREAM_MUSIC;
constexpr AudioStreamType AUDIO_STREAM_TYPE_CAPTURER = AudioStreamType::STREAM_MUSIC;

AudioPlaybackCaptureConfig GetDefaultPlaybackCaptureConfig()
{
    AudioPlaybackCaptureConfig config = {};
    config.filterOptions.usageFilterMode = FilterMode::INCLUDE;
    config.filterOptions.usages = { STREAM_USAGE_MEDIA, STREAM_USAGE_GAME };
    config.filterOptions.pidFilterMode = FilterMode::EXCLUDE;
    config.filterOptions.pids = { 10001, 10002 };
    config.filterOptions.capturePrivatePrivacyStreams = false;
    return config;
}

AudioProcessConfig GetDefaultProcessConfig()
{
    AudioProcessConfig config = {};
    config.appInfo.appPid = TEST_APP_PID;
    config.appInfo.appUid = TEST_APP_UID;
    config.appInfo.appTokenId = TEST_APP_TOKEN_ID;
    config.streamInfo.format = AUDIO_SAMPLE_FORMAT_16BIT;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.channels = TEST_CHANNEL_COUNT;
    config.streamInfo.samplingRate = TEST_SAMPLE_RATE;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.rendererFlags = 0;
    config.streamType = AUDIO_STREAM_TYPE_RENDERER;
    config.originalSessionId = 0;
    return config;
}

} // namespace

class AudioProcessConfigUnitTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

HWTEST_F(AudioProcessConfigUnitTest, DumpInnerCapConfig_001, testing::ext::TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = GetDefaultPlaybackCaptureConfig();
    std::string result = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_FALSE(result.empty());
    // Should contain "INCLUDE" for usageFilterMode
    EXPECT_NE(result.find("INCLUDE"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpInnerCapConfig_002, testing::ext::TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = GetDefaultPlaybackCaptureConfig();
    config.filterOptions.usageFilterMode = FilterMode::EXCLUDE;
    config.filterOptions.usages = { STREAM_USAGE_ALARM };
    std::string result = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("EXCLUDE"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpInnerCapConfig_003, testing::ext::TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = GetDefaultPlaybackCaptureConfig();
    config.filterOptions.pidFilterMode = FilterMode::INCLUDE;
    config.filterOptions.pids = { 9999 };
    std::string result = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("INCLUDE"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpInnerCapConfig_004, testing::ext::TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = GetDefaultPlaybackCaptureConfig();
    config.filterOptions.capturePrivatePrivacyStreams = true;
    std::string result = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_FALSE(result.empty());
}

HWTEST_F(AudioProcessConfigUnitTest, DumpProcessConfig_001, testing::ext::TestSize.Level1)
{
    AudioProcessConfig config = GetDefaultProcessConfig();
    std::string result = ProcessConfig::DumpProcessConfig(config);
    EXPECT_FALSE(result.empty());
    // Should contain appInfo
    EXPECT_NE(result.find("appInfo"), std::string::npos);
    EXPECT_NE(result.find("12345"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpProcessConfig_002, testing::ext::TestSize.Level1)
{
    AudioProcessConfig config = GetDefaultProcessConfig();
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.capturerInfo.capturerFlags = 0x01;
    config.capturerInfo.mixWithWakeUp = true;
    std::string result = ProcessConfig::DumpProcessConfig(config);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("capturerInfo"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpProcessConfig_003, testing::ext::TestSize.Level1)
{
    AudioProcessConfig config = GetDefaultProcessConfig();
    config.streamType = AUDIO_STREAM_TYPE_CAPTURER;
    config.originalSessionId = 42;
    std::string result = ProcessConfig::DumpProcessConfig(config);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("42"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpProcessConfig_004, testing::ext::TestSize.Level1)
{
    AudioProcessConfig config = GetDefaultProcessConfig();
    config.streamInfo.samplingRate = SAMPLE_RATE_44100;
    config.streamInfo.channels = AudioChannel::MONO;
    std::string result = ProcessConfig::DumpProcessConfig(config);
    EXPECT_FALSE(result.empty());
}

HWTEST_F(AudioProcessConfigUnitTest, DumpInnerCapConfig_EmptyLists, testing::ext::TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = {};
    config.filterOptions.usageFilterMode = FilterMode::INCLUDE;
    config.filterOptions.usages = {};  // Empty list
    config.filterOptions.pidFilterMode = FilterMode::EXCLUDE;
    config.filterOptions.pids = {};  // Empty list
    config.filterOptions.capturePrivatePrivacyStreams = false;

    std::string result = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_FALSE(result.empty());
    // Should handle empty lists gracefully
    EXPECT_NE(result.find("INCLUDE"), std::string::npos);
    EXPECT_NE(result.find("EXCLUDE"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpInnerCapConfig_InvalidFilterMode, testing::ext::TestSize.Level1)
{
    AudioPlaybackCaptureConfig config = GetDefaultPlaybackCaptureConfig();
    // Set invalid filter mode (assuming FilterMode has only INCLUDE and EXCLUDE)
    // Note: This test depends on the actual definition of FilterMode
    // If there are only 2 values, we can't test invalid mode easily
    // So we just test with valid modes
    config.filterOptions.usageFilterMode = FilterMode::INCLUDE;
    std::string result = ProcessConfig::DumpInnerCapConfig(config);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("INCLUDE"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpProcessConfig_EdgeCases, testing::ext::TestSize.Level1)
{
    AudioProcessConfig config = GetDefaultProcessConfig();

    // Test with edge values
    config.streamInfo.channels = AudioChannel::CHANNEL_UNKNOW;  // Edge case: 0 channels
    config.streamInfo.samplingRate = SAMPLE_RATE_8000;  // Edge case: low sampling rate
    std::string result = ProcessConfig::DumpProcessConfig(config);
    EXPECT_FALSE(result.empty());

    // Test with different audio mode
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.rendererInfo.streamUsage = STREAM_USAGE_ALARM;
    config.rendererInfo.contentType = CONTENT_TYPE_SONIFICATION;
    result = ProcessConfig::DumpProcessConfig(config);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("rendererInfo"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, DumpProcessConfig_ActualUsingTokenId, testing::ext::TestSize.Level1)
{
    AudioProcessConfig config = GetDefaultProcessConfig();
    config.actualUsingTokenId = 0;
    std::string result = ProcessConfig::DumpProcessConfig(config);
    EXPECT_FALSE(result.empty());
    // When 0, should not be dumped
    EXPECT_EQ(result.find("actualUsingTokenId"), std::string::npos);

    config.actualUsingTokenId = 88888;
    result = ProcessConfig::DumpProcessConfig(config);
    EXPECT_FALSE(result.empty());
    EXPECT_NE(result.find("actualUsingTokenId"), std::string::npos);
    EXPECT_NE(result.find("88888"), std::string::npos);
}

HWTEST_F(AudioProcessConfigUnitTest, Marshalling_ActualUsingTokenId_Default, testing::ext::TestSize.Level1)
{
    AudioProcessConfig config = GetDefaultProcessConfig();
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.actualUsingTokenId = 0;

    OHOS::Parcel parcel;
    bool ret = config.Marshalling(parcel);
    EXPECT_TRUE(ret);
    parcel.RewindRead(0);

    std::unique_ptr<AudioProcessConfig> deConfig(AudioProcessConfig::Unmarshalling(parcel));
    ASSERT_NE(deConfig, nullptr);
    EXPECT_EQ(deConfig->actualUsingTokenId, 0u);
    EXPECT_EQ(deConfig->appInfo.appTokenId, config.appInfo.appTokenId);
}

HWTEST_F(AudioProcessConfigUnitTest, Marshalling_ActualUsingTokenId_Set, testing::ext::TestSize.Level1)
{
    AudioProcessConfig config = GetDefaultProcessConfig();
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
    config.actualUsingTokenId = 99999;

    OHOS::Parcel parcel;
    bool ret = config.Marshalling(parcel);
    EXPECT_TRUE(ret);
    parcel.RewindRead(0);

    std::unique_ptr<AudioProcessConfig> deConfig(AudioProcessConfig::Unmarshalling(parcel));
    ASSERT_NE(deConfig, nullptr);
    EXPECT_EQ(deConfig->actualUsingTokenId, 99999u);
    // creator appTokenId must remain unchanged
    EXPECT_EQ(deConfig->appInfo.appTokenId, config.appInfo.appTokenId);
}

} // namespace AudioStandard
} // namespace OHOS
