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

#include "audio_static_buffer_processor.h"
#include "oh_audio_buffer.h"
#include <gtest/gtest.h>
#include <algorithm>

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
namespace {

constexpr size_t TEST_BUFFER_SIZE = 1024;
constexpr float SPEED_FAST = 1.5f;
constexpr AudioSamplingRate DEFAULT_SAMPLE_RATE = AudioSamplingRate::SAMPLE_RATE_48000;
constexpr AudioChannel DEFAULT_CHANNEL_COUNT = AudioChannel::STEREO;
constexpr AudioSampleFormat AUDIO_SAMPLE_FORMAT_16BIT = AudioSampleFormat::SAMPLE_S16LE;
constexpr AudioSampleFormat AUDIO_SAMPLE_FORMAT_8BIT = AudioSampleFormat::SAMPLE_U8;
constexpr AudioSampleFormat AUDIO_SAMPLE_FORMAT_32BIT = AudioSampleFormat::SAMPLE_S32LE;

AudioStreamInfo GetDefaultStreamInfo()
{
    AudioStreamInfo info = {};
    info.samplingRate = DEFAULT_SAMPLE_RATE;
    info.format = AUDIO_SAMPLE_FORMAT_16BIT;
    info.channels = DEFAULT_CHANNEL_COUNT;
    return info;
}

std::shared_ptr<OHAudioBufferBase> CreateTestBuffer(size_t size)
{
    return OHAudioBufferBase::CreateFromLocal(static_cast<uint32_t>(size), 1);
}

} // namespace

class AudioStaticBufferProcessorUnitTest : public testing::Test {
public:
    void SetUp() override
    {
        streamInfo_ = GetDefaultStreamInfo();
        sharedBuffer_ = CreateTestBuffer(TEST_BUFFER_SIZE);
        processor_ = AudioStaticBufferProcessor::CreateInstance(streamInfo_, sharedBuffer_);
    }
    void TearDown() override
    {
        processor_ = nullptr;
        sharedBuffer_ = nullptr;
    }

protected:
    AudioStreamInfo streamInfo_;
    std::shared_ptr<OHAudioBufferBase> sharedBuffer_ = nullptr;
    std::shared_ptr<AudioStaticBufferProcessor> processor_ = nullptr;
};

HWTEST_F(AudioStaticBufferProcessorUnitTest, CreateInstance_001, testing::ext::TestSize.Level1)
{
    auto proc = AudioStaticBufferProcessor::CreateInstance(streamInfo_, sharedBuffer_);
    ASSERT_NE(proc, nullptr);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, CreateInstance_002, testing::ext::TestSize.Level1)
{
    auto proc = AudioStaticBufferProcessor::CreateInstance(streamInfo_, nullptr);
    ASSERT_EQ(proc, nullptr);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessFadeInOut_001, testing::ext::TestSize.Level1)
{
    auto buffer = std::make_unique<int8_t[]>(TEST_BUFFER_SIZE);
    std::fill(buffer.get(), buffer.get() + TEST_BUFFER_SIZE, 0);

    AudioStreamInfo info = GetDefaultStreamInfo();
    int32_t ret = AudioStaticBufferProcessor::ProcessFadeInOut(
        buffer.get(), TEST_BUFFER_SIZE, info, false);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessFadeInOut_002, testing::ext::TestSize.Level1)
{
    auto buffer = std::make_unique<int8_t[]>(TEST_BUFFER_SIZE);
    std::fill(buffer.get(), buffer.get() + TEST_BUFFER_SIZE, 0);

    AudioStreamInfo info = GetDefaultStreamInfo();
    int32_t ret = AudioStaticBufferProcessor::ProcessFadeInOut(
        buffer.get(), TEST_BUFFER_SIZE, info, true);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessFadeInOut_003, testing::ext::TestSize.Level1)
{
    AudioStreamInfo info = GetDefaultStreamInfo();
    int32_t ret = AudioStaticBufferProcessor::ProcessFadeInOut(
        nullptr, TEST_BUFFER_SIZE, info, false);
    EXPECT_NE(ret, SUCCESS);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessBufferSpeedAndPitch_001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);
    int32_t ret = processor_->ProcessBufferSpeedAndPitch(SPEED_NORMAL, PITCH_NORMAL);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessBufferSpeedAndPitch_002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);
    int32_t ret = processor_->ProcessBufferSpeedAndPitch(SPEED_FAST, PITCH_NORMAL);
    EXPECT_NE(ret, SUCCESS);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessBufferSpeedAndPitch_003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);
    int32_t ret = processor_->ProcessBufferSpeedAndPitch(SPEED_NORMAL, 1.2f);
    EXPECT_NE(ret, SUCCESS);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, GetProcessedBuffer_001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);
    uint8_t *buffer = nullptr;
    size_t bufferSize = 0;
    int32_t ret = processor_->GetProcessedBuffer(&buffer, bufferSize);
    EXPECT_EQ(ret, SUCCESS);
}

HWTEST_F(AudioStaticBufferProcessorUnitTest, SaveProcessBuffer_001, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);
    processor_->SaveProcessBuffer();
    SUCCEED();
}

/**
 * @tc.name  : Test ProcessBufferSpeedAndPitch factor calculation
 * @tc.number: AudioStaticBufferProcessor_ProcessBufferSpeedAndPitch_Factor
 * @tc.desc  : Test dynamic speedBufferFactor calculation for various speed values.
 */
HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessBufferSpeedAndPitch_Factor, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);
    int32_t inputSize = static_cast<int32_t>(sharedBuffer_->GetDataSize());

    processor_->ProcessBufferSpeedAndPitch(0.0f, PITCH_NORMAL);
    EXPECT_EQ(processor_->audioSpeed_->maxSpeedBufferSize_, inputSize * 1);

    processor_->ProcessBufferSpeedAndPitch(2.0f, PITCH_NORMAL);
    EXPECT_EQ(processor_->audioSpeed_->maxSpeedBufferSize_, inputSize * 1);

    processor_->ProcessBufferSpeedAndPitch(0.5f, PITCH_NORMAL);
    EXPECT_EQ(processor_->audioSpeed_->maxSpeedBufferSize_, inputSize * 3);

    processor_->ProcessBufferSpeedAndPitch(0.3f, PITCH_NORMAL);
    EXPECT_EQ(processor_->audioSpeed_->maxSpeedBufferSize_, inputSize * 5);

    constexpr float SPEED_SLOW_1_8 = 0.125f;
    processor_->ProcessBufferSpeedAndPitch(SPEED_SLOW_1_8, PITCH_NORMAL);
    EXPECT_EQ(processor_->audioSpeed_->maxSpeedBufferSize_, inputSize * 9);
}

/**
 * @tc.name  : Test GetProcessedBuffer with tempBuffer set
 * @tc.number: AudioStaticBufferProcessor_GetProcessedBuffer_002
 * @tc.desc  : Test GetProcessedBuffer when tempBuffer_ is set.
 */
HWTEST_F(AudioStaticBufferProcessorUnitTest, GetProcessedBuffer_002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);

    // First process speed to set tempBuffer_
    int32_t ret = processor_->ProcessBufferSpeedAndPitch(1.5f, PITCH_NORMAL);
    EXPECT_NE(ret, SUCCESS);

    // Now GetProcessedBuffer should return tempBuffer_
    uint8_t *buffer = nullptr;
    size_t bufferSize = 0;
    ret = processor_->GetProcessedBuffer(&buffer, bufferSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(buffer, nullptr);
    EXPECT_GT(bufferSize, 0u);
}

/**
 * @tc.name  : Test GetProcessedBuffer with processBuffer set
 * @tc.number: AudioStaticBufferProcessor_GetProcessedBuffer_003
 * @tc.desc  : Test GetProcessedBuffer when processBuffer_ is set.
 */
HWTEST_F(AudioStaticBufferProcessorUnitTest, GetProcessedBuffer_003, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);

    // First process speed to set tempBuffer_
    int32_t ret = processor_->ProcessBufferSpeedAndPitch(1.5f, PITCH_NORMAL);
    EXPECT_NE(ret, SUCCESS);

    // Save process buffer
    processor_->SaveProcessBuffer();

    // Now GetProcessedBuffer should return processBuffer_
    uint8_t *buffer = nullptr;
    size_t bufferSize = 0;
    ret = processor_->GetProcessedBuffer(&buffer, bufferSize);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(buffer, nullptr);
    EXPECT_GT(bufferSize, 0u);
}

/**
 * @tc.name  : Test SaveProcessBuffer multiple times
 * @tc.number: AudioStaticBufferProcessor_SaveProcessBuffer_002
 * @tc.desc  : Test SaveProcessBuffer called multiple times.
 */
HWTEST_F(AudioStaticBufferProcessorUnitTest, SaveProcessBuffer_002, testing::ext::TestSize.Level1)
{
    ASSERT_NE(processor_, nullptr);

    // First process speed to set tempBuffer_
    int32_t ret = processor_->ProcessBufferSpeedAndPitch(1.5f, PITCH_NORMAL);
    EXPECT_NE(ret, SUCCESS);

    // Save process buffer
    processor_->SaveProcessBuffer();

    // Process speed again
    ret = processor_->ProcessBufferSpeedAndPitch(1.2f, PITCH_NORMAL);
    EXPECT_NE(ret, SUCCESS);

    // Save again
    processor_->SaveProcessBuffer();

    SUCCEED();
}

/**
 * @tc.name  : Test ProcessFadeInOut with different formats
 * @tc.number: AudioStaticBufferProcessor_ProcessFadeInOut_004
 * @tc.desc  : Test ProcessFadeInOut with SAMPLE_FORMAT_8BIT.
 */
HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessFadeInOut_004, testing::ext::TestSize.Level1)
{
    auto buffer = std::make_unique<int8_t[]>(TEST_BUFFER_SIZE);
    std::fill(buffer.get(), buffer.get() + TEST_BUFFER_SIZE, 0);

    AudioStreamInfo info = GetDefaultStreamInfo();
    info.format = AUDIO_SAMPLE_FORMAT_8BIT;
    int32_t ret = AudioStaticBufferProcessor::ProcessFadeInOut(
        buffer.get(), TEST_BUFFER_SIZE, info, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ProcessFadeInOut with different formats
 * @tc.number: AudioStaticBufferProcessor_ProcessFadeInOut_005
 * @tc.desc  : Test ProcessFadeInOut with SAMPLE_FORMAT_32BIT.
 */
HWTEST_F(AudioStaticBufferProcessorUnitTest, ProcessFadeInOut_005, testing::ext::TestSize.Level1)
{
    auto buffer = std::make_unique<int8_t[]>(TEST_BUFFER_SIZE * 2);
    std::fill(buffer.get(), buffer.get() + TEST_BUFFER_SIZE * 2, 0);

    AudioStreamInfo info = GetDefaultStreamInfo();
    info.format = AUDIO_SAMPLE_FORMAT_32BIT;
    int32_t ret = AudioStaticBufferProcessor::ProcessFadeInOut(
        buffer.get(), TEST_BUFFER_SIZE * 2, info, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test CreateInstance with different stream info
 * @tc.number: AudioStaticBufferProcessor_CreateInstance_003
 * @tc.desc  : Test CreateInstance with mono channel.
 */
HWTEST_F(AudioStaticBufferProcessorUnitTest, CreateInstance_003, testing::ext::TestSize.Level1)
{
    AudioStreamInfo info = GetDefaultStreamInfo();
    info.channels = AudioChannel::MONO;
    auto buffer = CreateTestBuffer(TEST_BUFFER_SIZE / 2);
    auto proc = AudioStaticBufferProcessor::CreateInstance(info, buffer);
    ASSERT_NE(proc, nullptr);
}

/**
 * @tc.name  : Test CreateInstance with different sampling rate
 * @tc.number: AudioStaticBufferProcessor_CreateInstance_004
 * @tc.desc  : Test CreateInstance with 44100 sampling rate.
 */
HWTEST_F(AudioStaticBufferProcessorUnitTest, CreateInstance_004, testing::ext::TestSize.Level1)
{
    AudioStreamInfo info = GetDefaultStreamInfo();
    info.samplingRate = SAMPLE_RATE_44100;
    auto buffer = CreateTestBuffer(TEST_BUFFER_SIZE);
    auto proc = AudioStaticBufferProcessor::CreateInstance(info, buffer);
    ASSERT_NE(proc, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS
