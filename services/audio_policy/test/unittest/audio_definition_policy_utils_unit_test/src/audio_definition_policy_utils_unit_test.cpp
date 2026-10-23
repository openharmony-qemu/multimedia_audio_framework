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
#include "audio_definition_policy_utils_unit_test.h"
#include "audio_device_descriptor.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
const std::string TEST_MAC = "11:22:33:44:55:66";

DeviceStreamInfo MakeCap(std::set<AudioSamplingRate> rates, AudioSampleFormat fmt,
    std::set<AudioChannelLayout> layouts)
{
    DeviceStreamInfo cap;
    cap.samplingRate = rates;
    cap.format = fmt;
    cap.formats.insert(fmt);
    cap.channelLayout = layouts;
    return cap;
}

std::shared_ptr<AudioStreamDescriptor> MakePlaybackStream(
    uint32_t rate, AudioSampleFormat fmt, const std::string &mac = TEST_MAC)
{
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->audioMode_ = AUDIO_MODE_PLAYBACK;
    desc->streamInfo_.samplingRate = static_cast<AudioSamplingRate>(rate);
    desc->streamInfo_.format = fmt;
    desc->streamInfo_.channels = AudioChannel::STEREO;
    auto dev = std::make_shared<AudioDeviceDescriptor>();
    dev->macAddress_ = mac;
    dev->deviceRole_ = OUTPUT_DEVICE;
    desc->newDeviceDescs_.push_back(dev);
    return desc;
}
}

void AudioDefinitionPolicyUtilsUnitTest::SetUpTestCase(void) {}
void AudioDefinitionPolicyUtilsUnitTest::TearDownTestCase(void) {}
void AudioDefinitionPolicyUtilsUnitTest::SetUp(void) {}
void AudioDefinitionPolicyUtilsUnitTest::TearDown(void) {}

/* ---- SelectBestOutputFormat ---- */

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_EmptyCaps, TestSize.Level1)
{
    AudioStreamInfo app(SAMPLE_RATE_192000, ENCODING_PCM, SAMPLE_S24LE, AudioChannel::STEREO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {});
    EXPECT_EQ(result.rate, SAMPLE_RATE_48000);
    EXPECT_EQ(result.format, SAMPLE_S16LE);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_ExactMatch, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_192000}, SAMPLE_S24LE, {CH_LAYOUT_STEREO});
    AudioStreamInfo app(SAMPLE_RATE_192000, ENCODING_PCM, SAMPLE_S24LE, AudioChannel::STEREO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.rate, SAMPLE_RATE_192000);
    EXPECT_EQ(result.format, SAMPLE_S24LE);
    EXPECT_EQ(result.channelLayout, CH_LAYOUT_STEREO);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_RateDowngrade, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_48000, SAMPLE_RATE_96000}, SAMPLE_S24LE, {CH_LAYOUT_STEREO});
    AudioStreamInfo app(SAMPLE_RATE_192000, ENCODING_PCM, SAMPLE_S24LE, AudioChannel::STEREO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.rate, SAMPLE_RATE_96000);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_AllRatesAboveApp, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_192000, SAMPLE_RATE_384000}, SAMPLE_S24LE, {CH_LAYOUT_STEREO});
    AudioStreamInfo app(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, AudioChannel::STEREO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.rate, SAMPLE_RATE_192000);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_FormatExactMatch, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_48000}, SAMPLE_S32LE, {CH_LAYOUT_STEREO});
    AudioStreamInfo app(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S32LE, AudioChannel::STEREO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.format, SAMPLE_S32LE);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_FormatDowngrade, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_48000}, SAMPLE_S16LE, {CH_LAYOUT_STEREO});
    cap.formats.insert(SAMPLE_S24LE);
    cap.formats.insert(SAMPLE_S16LE);
    AudioStreamInfo app(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S24LE, AudioChannel::STEREO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.format, SAMPLE_S24LE);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_FormatAllAboveApp, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_48000}, SAMPLE_S24LE, {CH_LAYOUT_STEREO});
    cap.formats.clear();
    cap.formats.insert(SAMPLE_S24LE);
    cap.formats.insert(SAMPLE_S32LE);
    AudioStreamInfo app(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, AudioChannel::STEREO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.format, SAMPLE_S32LE);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_ChannelExactMatch, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_48000}, SAMPLE_S16LE, {CH_LAYOUT_MONO, CH_LAYOUT_STEREO});
    AudioStreamInfo app(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, AudioChannel::STEREO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.channelLayout, CH_LAYOUT_STEREO);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_ChannelDowngrade, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_48000}, SAMPLE_S16LE, {CH_LAYOUT_MONO, CH_LAYOUT_STEREO});
    AudioStreamInfo app(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, AudioChannel::CHANNEL_4);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.channelLayout, CH_LAYOUT_STEREO);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, SelectBestOutputFormat_ChannelAllAboveApp, TestSize.Level1)
{
    DeviceStreamInfo cap = MakeCap({SAMPLE_RATE_48000}, SAMPLE_S16LE, {CH_LAYOUT_STEREO, CH_LAYOUT_4POINT0});
    AudioStreamInfo app(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, AudioChannel::MONO);
    auto result = AudioDefinitionPolicyUtils::SelectBestOutputFormat(app, {cap});
    EXPECT_EQ(result.channelLayout, CH_LAYOUT_STEREO);
}

/* ---- FindHighestFormatStreamOnDevice ---- */

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, FindHighest_NoMatch, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(48000, SAMPLE_S16LE, "AA:BB:CC:DD:EE:FF");
    AudioStreamInfo info;
    bool found = AudioDefinitionPolicyUtils::FindHighestFormatStreamOnDevice(
        TEST_MAC, {s1}, nullptr, info);
    EXPECT_FALSE(found);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, FindHighest_SingleStream, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(192000, SAMPLE_S24LE);
    AudioStreamInfo info;
    bool found = AudioDefinitionPolicyUtils::FindHighestFormatStreamOnDevice(
        TEST_MAC, {s1}, nullptr, info);
    EXPECT_TRUE(found);
    EXPECT_EQ(info.samplingRate, SAMPLE_RATE_192000);
    EXPECT_EQ(info.format, SAMPLE_S24LE);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, FindHighest_MultiplePickHighest, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(48000, SAMPLE_S16LE);
    auto s2 = MakePlaybackStream(192000, SAMPLE_S24LE);
    AudioStreamInfo info;
    bool found = AudioDefinitionPolicyUtils::FindHighestFormatStreamOnDevice(
        TEST_MAC, {s1, s2}, nullptr, info);
    EXPECT_TRUE(found);
    EXPECT_EQ(info.samplingRate, SAMPLE_RATE_192000);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, FindHighest_SameRateHigherFormat, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(96000, SAMPLE_S16LE);
    auto s2 = MakePlaybackStream(96000, SAMPLE_S24LE);
    AudioStreamInfo info;
    bool found = AudioDefinitionPolicyUtils::FindHighestFormatStreamOnDevice(
        TEST_MAC, {s1, s2}, nullptr, info);
    EXPECT_TRUE(found);
    EXPECT_EQ(info.format, SAMPLE_S24LE);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, FindHighest_NullAndNonPlaybackSkipped, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(192000, SAMPLE_S24LE);
    std::shared_ptr<AudioStreamDescriptor> nullStream = nullptr;
    auto s2 = std::make_shared<AudioStreamDescriptor>();
    s2->audioMode_ = AUDIO_MODE_RECORD;
    auto dev = std::make_shared<AudioDeviceDescriptor>();
    dev->macAddress_ = TEST_MAC;
    s2->newDeviceDescs_.push_back(dev);
    AudioStreamInfo info;
    bool found = AudioDefinitionPolicyUtils::FindHighestFormatStreamOnDevice(
        TEST_MAC, {nullStream, s2}, s1, info);
    EXPECT_TRUE(found);
    EXPECT_EQ(info.samplingRate, SAMPLE_RATE_192000);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, FindHighest_ExtraStreamIncluded, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(48000, SAMPLE_S16LE);
    auto extra = MakePlaybackStream(384000, SAMPLE_S32LE);
    AudioStreamInfo info;
    bool found = AudioDefinitionPolicyUtils::FindHighestFormatStreamOnDevice(
        TEST_MAC, {s1}, extra, info);
    EXPECT_TRUE(found);
    EXPECT_EQ(info.samplingRate, SAMPLE_RATE_384000);
}

/* ---- HasVoipStreamOnDevice ---- */

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, HasVoip_NoStreams, TestSize.Level1)
{
    bool result = AudioDefinitionPolicyUtils::HasVoipStreamOnDevice(TEST_MAC, {}, nullptr);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, HasVoip_NoVoipStream, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(48000, SAMPLE_S16LE);
    s1->routeFlag_ = AUDIO_OUTPUT_FLAG_NORMAL;
    bool result = AudioDefinitionPolicyUtils::HasVoipStreamOnDevice(TEST_MAC, {s1}, nullptr);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, HasVoip_VoipInStreams, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(48000, SAMPLE_S16LE);
    s1->routeFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
    bool result = AudioDefinitionPolicyUtils::HasVoipStreamOnDevice(TEST_MAC, {s1}, nullptr);
    EXPECT_TRUE(result);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, HasVoip_VoipInPipe, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(48000, SAMPLE_S16LE);
    s1->routeFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->streamDescriptors_.push_back(s1);
    bool result = AudioDefinitionPolicyUtils::HasVoipStreamOnDevice(TEST_MAC, {}, pipeInfo);
    EXPECT_TRUE(result);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, HasVoip_MacMismatch, TestSize.Level1)
{
    auto s1 = MakePlaybackStream(48000, SAMPLE_S16LE, "AA:BB:CC:DD:EE:FF");
    s1->routeFlag_ = AUDIO_OUTPUT_FLAG_VOIP;
    bool result = AudioDefinitionPolicyUtils::HasVoipStreamOnDevice(TEST_MAC, {s1}, nullptr);
    EXPECT_FALSE(result);
}

HWTEST_F(AudioDefinitionPolicyUtilsUnitTest, HasVoip_NullStream, TestSize.Level1)
{
    std::shared_ptr<AudioStreamDescriptor> nullStream = nullptr;
    bool result = AudioDefinitionPolicyUtils::HasVoipStreamOnDevice(TEST_MAC, {nullStream}, nullptr);
    EXPECT_FALSE(result);
}
} // namespace AudioStandard
} // namespace OHOS
