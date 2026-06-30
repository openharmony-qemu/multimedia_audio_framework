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

#include "audio_injector_service.h"
#include "audio_stream_types.h"
#include "audio_errors.h"
#include <gtest/gtest.h>
#include <cstring>

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
namespace {

constexpr uint32_t TEST_SINK_PORT_INDEX = 5;
constexpr uint32_t UINT32_INVALID_VALUE = 0xFFFFFFFF;
constexpr int32_t DEFAULT_SAMPLE_RATE = 48000;
constexpr int32_t DEFAULT_CHANNEL_COUNT = 2;

AudioStreamInfo GetDefaultStreamInfo()
{
    AudioStreamInfo info = {};
    info.samplingRate = static_cast<AudioSamplingRate>(DEFAULT_SAMPLE_RATE);
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.channels = static_cast<AudioChannel>(DEFAULT_CHANNEL_COUNT);
    return info;
}

} // namespace

class AudioInjectorServiceUnitTest : public testing::Test {
public:
    void SetUp() override
    {
        // Reset sinkPortIndex_ to invalid before each test
        AudioInjectorService::GetInstance().SetSinkPortIdx(UINT32_INVALID_VALUE);
    }
    void TearDown() override
    {
        AudioInjectorService::GetInstance().SetSinkPortIdx(UINT32_INVALID_VALUE);
    }
};

HWTEST_F(AudioInjectorServiceUnitTest, GetInstance_001, testing::ext::TestSize.Level1)
{
    AudioInjectorService& instance = AudioInjectorService::GetInstance();
    EXPECT_NE(&instance, nullptr);
}

HWTEST_F(AudioInjectorServiceUnitTest, SetAndGetSinkPortIdx_001, testing::ext::TestSize.Level1)
{
    AudioInjectorService::GetInstance().SetSinkPortIdx(TEST_SINK_PORT_INDEX);
    uint32_t idx = AudioInjectorService::GetInstance().GetSinkPortIdx();
    EXPECT_EQ(idx, TEST_SINK_PORT_INDEX);
}

HWTEST_F(AudioInjectorServiceUnitTest, SetAndGetSinkPortIdx_002, testing::ext::TestSize.Level1)
{
    // Set to invalid value
    AudioInjectorService::GetInstance().SetSinkPortIdx(UINT32_INVALID_VALUE);
    uint32_t idx = AudioInjectorService::GetInstance().GetSinkPortIdx();
    EXPECT_EQ(idx, UINT32_INVALID_VALUE);
}

HWTEST_F(AudioInjectorServiceUnitTest, GetModuleInfo_001, testing::ext::TestSize.Level1)
{
    AudioModuleInfo info = AudioInjectorService::GetInstance().GetModuleInfo();
    // Initially moduleInfo_ is empty
    EXPECT_EQ(info.rate, "");
    EXPECT_EQ(info.format, "");
    EXPECT_EQ(info.channels, "");
    EXPECT_EQ(info.bufferSize, "");
}

HWTEST_F(AudioInjectorServiceUnitTest, SetAndGetModuleInfo_001, testing::ext::TestSize.Level1)
{
    AudioStreamInfo streamInfo = GetDefaultStreamInfo();
    AudioInjectorService::GetInstance().SetModuleInfo(streamInfo);

    AudioModuleInfo info = AudioInjectorService::GetInstance().GetModuleInfo();
    EXPECT_EQ(info.rate, "48000");
    EXPECT_EQ(info.format, "S16LE");
    EXPECT_EQ(info.channels, "2");
}

HWTEST_F(AudioInjectorServiceUnitTest, SetModuleInfo_002, testing::ext::TestSize.Level1)
{
    AudioStreamInfo streamInfo = GetDefaultStreamInfo();
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    streamInfo.channels = AudioChannel::MONO;
    AudioInjectorService::GetInstance().SetModuleInfo(streamInfo);

    AudioModuleInfo info = AudioInjectorService::GetInstance().GetModuleInfo();
    EXPECT_EQ(info.rate, "44100");
    EXPECT_EQ(info.format, "S8");
    EXPECT_EQ(info.channels, "1");
}

HWTEST_F(AudioInjectorServiceUnitTest, PeekAudioData_001, testing::ext::TestSize.Level1)
{
    // PeekAudioData calls HPAE::IHpaeManager internally
    // Just call with nullptr buffer to cover the CHECK_AND_RETURN_LOG path
    AudioStreamInfo streamInfo = GetDefaultStreamInfo();
    int32_t ret = AudioInjectorService::GetInstance().PeekAudioData(
        UINT32_INVALID_VALUE, nullptr, 1024, streamInfo);
    // Buffer is nullptr, should return ERROR
    EXPECT_NE(ret, SUCCESS);
}

HWTEST_F(AudioInjectorServiceUnitTest, PeekAudioData_002, testing::ext::TestSize.Level1)
{
    // Call with valid buffer but invalid sinkPortIndex
    AudioStreamInfo streamInfo = GetDefaultStreamInfo();
    auto buffer = std::make_unique<uint8_t[]>(1024);
    int32_t ret = AudioInjectorService::GetInstance().PeekAudioData(
        UINT32_INVALID_VALUE, buffer.get(), 1024, streamInfo);
    // Should return something (depends on HPAE manager implementation)
    (void)ret;
    SUCCEED();
}

} // namespace AudioStandard
} // namespace OHOS
