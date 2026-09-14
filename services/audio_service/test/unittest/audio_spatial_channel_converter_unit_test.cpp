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

#include <cstring>
#include <vector>
#include <gtest/gtest.h>
#include "audio_spatial_channel_converter.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t AVS3METADATA_SIZE = 19824;
static constexpr int32_t AUDIO_VIVID_SAMPLES_PER_FRAME = 1024;
static constexpr int32_t HOA_FIRST_ORDER_CHANNELS = 4;
static constexpr int32_t HOA_SECOND_ORDER_CHANNELS = 9;
static constexpr int32_t HOA_THIRD_ORDER_CHANNELS = 16;
static constexpr int8_t BYTES_PER_SAMPLE_F32LE = 4;
static constexpr int32_t PCM_PATTERN_MODULUS = 251;

class AudioApatialChannelCoverterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioApatialChannelCoverterUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioApatialChannelCoverterUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioApatialChannelCoverterUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioApatialChannelCoverterUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_001
 * @tc.desc  : Test GetPcmLength interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_001, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    int32_t channels = 1;
    int8_t bps = 1;
    audioSpatialChannelConverter->encoding_ = ENCODING_PCM;
    auto result = audioSpatialChannelConverter->GetPcmLength(channels, bps);
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_002
 * @tc.desc  : Test GetPcmLength interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_002, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    int32_t channels = 1;
    int8_t bps = 1;
    audioSpatialChannelConverter->encoding_ = ENCODING_AUDIOVIVID;
    auto result = audioSpatialChannelConverter->GetPcmLength(channels, bps);
    EXPECT_EQ(result, 1024);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_003
 * @tc.desc  : Test GetMetaSize interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_003, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->encoding_ = ENCODING_PCM;
    auto result = audioSpatialChannelConverter->GetMetaSize();
    EXPECT_EQ(result, 0);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_004
 * @tc.desc  : Test GetMetaSize interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_004, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->encoding_ = ENCODING_AUDIOVIVID;
    auto result = audioSpatialChannelConverter->GetMetaSize();
    EXPECT_EQ(result, AVS3METADATA_SIZE);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_005
 * @tc.desc  : Test Init interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_005, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    AudioStreamParams info;
    ConverterConfig cfg;

    info.format = SAMPLE_U8;
    info.channels = 1;
    info.encoding = 1;
    info.samplingRate = 1;
    cfg.outChannelLayout = 1;

    auto result = audioSpatialChannelConverter->Init(info, cfg);
    EXPECT_EQ(result, true);
    EXPECT_EQ(audioSpatialChannelConverter->outChannelLayout_, CH_LAYOUT_UNKNOWN);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_006
 * @tc.desc  : Test CheckInputValid interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_006, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    BufferDesc bufDesc;
    bufDesc.buffer = nullptr;
    bufDesc.metaBuffer = nullptr;
    auto result = audioSpatialChannelConverter->CheckInputValid(bufDesc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_007
 * @tc.desc  : Test CheckInputValid interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_007, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    uint8_t meta = 1;
    BufferDesc bufDesc;
    bufDesc.buffer = nullptr;
    bufDesc.metaBuffer = &meta;
    auto result = audioSpatialChannelConverter->CheckInputValid(bufDesc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_008
 * @tc.desc  : Test CheckInputValid interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_008, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    uint8_t meta = 1;
    BufferDesc bufDesc;
    bufDesc.buffer = &meta;
    bufDesc.metaBuffer = nullptr;
    auto result = audioSpatialChannelConverter->CheckInputValid(bufDesc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_009
 * @tc.desc  : Test CheckInputValid interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_009, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    uint8_t buffer1 = 1;
    uint8_t buffer2 = 1;
    BufferDesc bufDesc;
    bufDesc.buffer = &buffer1;
    bufDesc.metaBuffer = &buffer2;
    bufDesc.bufLength = 1;
    audioSpatialChannelConverter->encoding_ = ENCODING_PCM;
    auto result = audioSpatialChannelConverter->CheckInputValid(bufDesc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_010
 * @tc.desc  : Test CheckInputValid interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_010, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    uint8_t buffer1 = 1;
    uint8_t buffer2 = 1;
    BufferDesc bufDesc;
    bufDesc.buffer = &buffer1;
    bufDesc.metaBuffer = &buffer2;
    bufDesc.bufLength = 0;
    bufDesc.metaLength = 1;
    audioSpatialChannelConverter->encoding_ = ENCODING_PCM;
    auto result = audioSpatialChannelConverter->CheckInputValid(bufDesc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_011
 * @tc.desc  : Test CheckInputValid interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_011, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    uint8_t buffer1 = 1;
    uint8_t buffer2 = 1;
    BufferDesc bufDesc;
    bufDesc.buffer = &buffer1;
    bufDesc.metaBuffer = &buffer2;
    bufDesc.bufLength = 0;
    bufDesc.metaLength = 0;
    audioSpatialChannelConverter->encoding_ = ENCODING_PCM;
    auto result = audioSpatialChannelConverter->CheckInputValid(bufDesc);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_012
 * @tc.desc  : Test CheckInputValid interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_012, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    BufferDesc bufDesc;
    audioSpatialChannelConverter->loadSuccess_ = true;
    audioSpatialChannelConverter->CheckInputValid(bufDesc);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_013
 * @tc.desc  : Test CheckInputValid interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_013, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    BufferDesc bufDesc;
    audioSpatialChannelConverter->loadSuccess_ = false;
    audioSpatialChannelConverter->CheckInputValid(bufDesc);
}

/**
 * @tc.name  : Test AudioApatialChannelCoverterUnitTest API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverterUnitTest_014
 * @tc.desc  : Test AddAlgoHandle interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_014, TestSize.Level1)
{
    auto libLoader = std::make_shared<LibLoader>();
    ASSERT_TRUE(libLoader != nullptr);

    Library library;
    auto reslut = libLoader->AddAlgoHandle(library);
    EXPECT_EQ(reslut, false);
}
/**
 * @tc.name  : Test process API
 * @tc.type  : FUNC
 * @tc.number: process_013
 * @tc.desc  : Test process interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_015, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    BufferDesc bufDesc;
    audioSpatialChannelConverter->loadSuccess_ = false;
    audioSpatialChannelConverter->Process(bufDesc);

    uint8_t buffer1 = 1;
    bufDesc.buffer = &buffer1;
    audioSpatialChannelConverter->Process(bufDesc);
}

/**
 * @tc.name  : Test GetInputBufferSize API
 * @tc.type  : FUNC
 * @tc.number: GetInputBufferSize_001
 * @tc.desc  : Test GetInputBufferSize interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, GetInputBufferSize_001, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);
    size_t bufferSize = 0;
    bool result = audioSpatialChannelConverter->GetInputBufferSize(bufferSize);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : Test GetOutputBufferStream API
 * @tc.type  : FUNC
 * @tc.number: GetOutputBufferStream_001
 * @tc.desc  : Test GetOutputBufferStream interface.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, GetOutputBufferStream_001, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);
    uint8_t *buffer = nullptr;
    uint32_t bufferLen = 0;

    audioSpatialChannelConverter->GetOutputBufferStream(buffer, bufferLen);

    EXPECT_EQ(buffer, nullptr);
}

/**
 * @tc.name  : Test IsAlgoLoaded API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_016
 * @tc.desc  : Test IsAlgoLoaded returns false for a freshly value-initialized converter.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_016, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    EXPECT_EQ(audioSpatialChannelConverter->IsAlgoLoaded(), false);
}

/**
 * @tc.name  : Test Init HOA fallback contract API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_017
 * @tc.desc  : Test Init with HOA stream keeps the always-true contract, reports algo not
 *             loaded and falls back to input geometry when the algo library is unavailable.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_017, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    AudioStreamParams info;
    info.format = AudioSampleFormat::SAMPLE_F32LE;
    info.channels = AudioChannel::CHANNEL_9;
    info.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;
    info.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_HOA_ORDER2_ACN_N3D;
    ConverterConfig cfg;

    auto result = audioSpatialChannelConverter->Init(info, cfg);
    EXPECT_EQ(result, true);
    EXPECT_EQ(audioSpatialChannelConverter->IsAlgoLoaded(), false);
    EXPECT_EQ(audioSpatialChannelConverter->outChannel_, HOA_SECOND_ORDER_CHANNELS);
    EXPECT_EQ(audioSpatialChannelConverter->outChannelLayout_, CH_LAYOUT_UNKNOWN);

    uint8_t channels = 0;
    uint64_t channelLayout = CH_LAYOUT_5POINT1POINT2;
    audioSpatialChannelConverter->ConverterChannels(channels, channelLayout);
    EXPECT_EQ(channels, static_cast<uint8_t>(AudioChannel::CHANNEL_9));
    EXPECT_EQ(channelLayout, static_cast<uint64_t>(AudioChannelLayout::CH_LAYOUT_UNKNOWN));
}

/**
 * @tc.name  : Test IsAlgoLoaded API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_018
 * @tc.desc  : Test IsAlgoLoaded returns true when loadSuccess_ is set directly.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_018, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->loadSuccess_ = true;
    EXPECT_EQ(audioSpatialChannelConverter->IsAlgoLoaded(), true);
}

/**
 * @tc.name  : Test CheckInputValid API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_019
 * @tc.desc  : Test CheckInputValid accepts a second order HOA frame with exact pcm and meta size.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_019, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->encoding_ = ENCODING_AUDIOVIVID;
    audioSpatialChannelConverter->inChannel_ = HOA_SECOND_ORDER_CHANNELS;
    audioSpatialChannelConverter->bps_ = BYTES_PER_SAMPLE_F32LE;

    size_t pcmLength = static_cast<size_t>(HOA_SECOND_ORDER_CHANNELS) * AUDIO_VIVID_SAMPLES_PER_FRAME
        * BYTES_PER_SAMPLE_F32LE;
    std::vector<uint8_t> pcmBuffer(pcmLength);
    std::vector<uint8_t> metaBuffer(static_cast<size_t>(AVS3METADATA_SIZE));

    BufferDesc bufDesc;
    bufDesc.buffer = pcmBuffer.data();
    bufDesc.bufLength = pcmLength;
    bufDesc.metaBuffer = metaBuffer.data();
    bufDesc.metaLength = static_cast<size_t>(AVS3METADATA_SIZE);

    EXPECT_EQ(audioSpatialChannelConverter->CheckInputValid(bufDesc), true);
}

/**
 * @tc.name  : Test CheckInputValid API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_020
 * @tc.desc  : Test CheckInputValid rejects wrong pcm length and wrong meta length for HOA frames.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_020, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->encoding_ = ENCODING_AUDIOVIVID;
    audioSpatialChannelConverter->inChannel_ = HOA_SECOND_ORDER_CHANNELS;
    audioSpatialChannelConverter->bps_ = BYTES_PER_SAMPLE_F32LE;

    size_t pcmLength = static_cast<size_t>(HOA_SECOND_ORDER_CHANNELS) * AUDIO_VIVID_SAMPLES_PER_FRAME
        * BYTES_PER_SAMPLE_F32LE;
    std::vector<uint8_t> pcmBuffer(pcmLength);
    std::vector<uint8_t> metaBuffer(static_cast<size_t>(AVS3METADATA_SIZE));

    BufferDesc bufDesc;
    bufDesc.buffer = pcmBuffer.data();
    bufDesc.metaBuffer = metaBuffer.data();

    bufDesc.bufLength = pcmLength - 1;
    bufDesc.metaLength = static_cast<size_t>(AVS3METADATA_SIZE);
    EXPECT_EQ(audioSpatialChannelConverter->CheckInputValid(bufDesc), false);

    bufDesc.bufLength = pcmLength;
    bufDesc.metaLength = static_cast<size_t>(AVS3METADATA_SIZE) + 1;
    EXPECT_EQ(audioSpatialChannelConverter->CheckInputValid(bufDesc), false);
}

/**
 * @tc.name  : Test GetInputBufferSize API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_021
 * @tc.desc  : Test GetInputBufferSize returns the full HOA frame size for a second order stream.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_021, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->encoding_ = ENCODING_AUDIOVIVID;
    audioSpatialChannelConverter->inChannel_ = HOA_SECOND_ORDER_CHANNELS;
    audioSpatialChannelConverter->bps_ = BYTES_PER_SAMPLE_F32LE;

    size_t bufferSize = 0;
    EXPECT_EQ(audioSpatialChannelConverter->GetInputBufferSize(bufferSize), true);
    size_t expectedSize = static_cast<size_t>(HOA_SECOND_ORDER_CHANNELS) * AUDIO_VIVID_SAMPLES_PER_FRAME
        * BYTES_PER_SAMPLE_F32LE;
    EXPECT_EQ(bufferSize, expectedSize);
}

/**
 * @tc.name  : Test AllocateMem and GetOutputBufferStream API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_022
 * @tc.desc  : Test AllocateMem allocates the output buffer with the expected frame size.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_022, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->encoding_ = ENCODING_AUDIOVIVID;
    audioSpatialChannelConverter->outChannel_ = HOA_SECOND_ORDER_CHANNELS;
    audioSpatialChannelConverter->bps_ = BYTES_PER_SAMPLE_F32LE;

    EXPECT_EQ(audioSpatialChannelConverter->AllocateMem(), true);

    uint8_t *buffer = nullptr;
    uint32_t bufferLen = 0;
    audioSpatialChannelConverter->GetOutputBufferStream(buffer, bufferLen);
    EXPECT_NE(buffer, nullptr);
    size_t expectedLen = static_cast<size_t>(HOA_SECOND_ORDER_CHANNELS) * AUDIO_VIVID_SAMPLES_PER_FRAME
        * BYTES_PER_SAMPLE_F32LE;
    EXPECT_EQ(bufferLen, static_cast<uint32_t>(expectedLen));
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_023
 * @tc.desc  : Test Process copies input pcm to the output buffer when the algo is not loaded.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_023, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->encoding_ = ENCODING_AUDIOVIVID;
    audioSpatialChannelConverter->inChannel_ = HOA_SECOND_ORDER_CHANNELS;
    audioSpatialChannelConverter->outChannel_ = HOA_SECOND_ORDER_CHANNELS;
    audioSpatialChannelConverter->bps_ = BYTES_PER_SAMPLE_F32LE;
    audioSpatialChannelConverter->loadSuccess_ = false;
    ASSERT_EQ(audioSpatialChannelConverter->AllocateMem(), true);

    size_t pcmLength = static_cast<size_t>(HOA_SECOND_ORDER_CHANNELS) * AUDIO_VIVID_SAMPLES_PER_FRAME
        * BYTES_PER_SAMPLE_F32LE;
    std::vector<uint8_t> pcmBuffer(pcmLength);
    for (size_t i = 0; i < pcmLength; i++) {
        pcmBuffer[i] = static_cast<uint8_t>(i % PCM_PATTERN_MODULUS);
    }
    std::vector<uint8_t> metaBuffer(static_cast<size_t>(AVS3METADATA_SIZE));

    BufferDesc bufDesc;
    bufDesc.buffer = pcmBuffer.data();
    bufDesc.bufLength = pcmLength;
    bufDesc.metaBuffer = metaBuffer.data();
    bufDesc.metaLength = static_cast<size_t>(AVS3METADATA_SIZE);

    audioSpatialChannelConverter->Process(bufDesc);

    uint8_t *buffer = nullptr;
    uint32_t bufferLen = 0;
    audioSpatialChannelConverter->GetOutputBufferStream(buffer, bufferLen);
    ASSERT_NE(buffer, nullptr);
    EXPECT_EQ(bufferLen, static_cast<uint32_t>(pcmLength));
    EXPECT_EQ(memcmp(buffer, pcmBuffer.data(), pcmLength), 0);
}

/**
 * @tc.name  : Test GetPcmLength API
 * @tc.type  : FUNC
 * @tc.number: AudioApatialChannelCoverter_024
 * @tc.desc  : Test GetPcmLength for first, second and third order HOA frame sizes.
 */
HWTEST(AudioApatialChannelCoverterUnitTest, AudioApatialChannelCoverter_024, TestSize.Level1)
{
    auto audioSpatialChannelConverter = std::make_shared<AudioSpatialChannelConverter>();
    ASSERT_TRUE(audioSpatialChannelConverter != nullptr);

    audioSpatialChannelConverter->encoding_ = ENCODING_AUDIOVIVID;

    int32_t hoaChannels[] = {HOA_FIRST_ORDER_CHANNELS, HOA_SECOND_ORDER_CHANNELS, HOA_THIRD_ORDER_CHANNELS};
    for (int32_t channels : hoaChannels) {
        size_t expectedLength = static_cast<size_t>(channels) * AUDIO_VIVID_SAMPLES_PER_FRAME
            * BYTES_PER_SAMPLE_F32LE;
        EXPECT_EQ(audioSpatialChannelConverter->GetPcmLength(channels, BYTES_PER_SAMPLE_F32LE), expectedLength);
    }
}
} // namespace AudioStandard
} // namespace OHOS