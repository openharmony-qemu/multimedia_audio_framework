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

#include <gtest/gtest.h>
#include "audio_spatial_channel_converter.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t AVS3METADATA_SIZE = 19824;

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
} // namespace AudioStandard
} // namespace OHOS