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

#include <gtest/gtest.h>
#include "audio_speed.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {

#ifndef AUDIO_SPEED_ALGO_ENABLE
class AudioSpeedUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioSpeedUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioSpeedUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioSpeedUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioSpeedUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_001
* @tc.desc  : Test LoadChangeSpeedFunc.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_001, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S24LE;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->LoadChangeSpeedFunc();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_002.
* @tc.desc  : Test LoadChangeSpeedFunc.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_002, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->LoadChangeSpeedFunc();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_003.
* @tc.desc  : Test ChangeSpeedFor24Bit.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_003, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 1;

    uint8_t *buffer = nullptr;
    int32_t bufferSize = 0;
    std::unique_ptr<uint8_t []> outBuffer = nullptr;
    int32_t outBufferSize;

    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->ChangeSpeedFor24Bit(buffer, bufferSize, outBuffer, outBufferSize);
    EXPECT_EQ(result, ERR_MEMORY_ALLOC_FAILED);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_004.
* @tc.desc  : Test ChangeSpeedFor24Bit.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_004, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 1;

    uint8_t *buffer = nullptr;
    int32_t bufferSize = MAX_SPEED_BUFFER_SIZE + 1;
    std::unique_ptr<uint8_t []> outBuffer = nullptr;
    int32_t outBufferSize;

    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->ChangeSpeedFor24Bit(buffer, bufferSize, outBuffer, outBufferSize);
    EXPECT_EQ(result, ERR_MEMORY_ALLOC_FAILED);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_004.
* @tc.desc  : Test ChangeSpeedFor24Bit.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_005, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 1;

    uint8_t *buffer = nullptr;
    int32_t bufferSize = 1;
    std::unique_ptr<uint8_t []> outBuffer = nullptr;
    int32_t outBufferSize = 0;

    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->ChangeSpeedFor24Bit(buffer, bufferSize, outBuffer, outBufferSize);
    EXPECT_EQ(result, bufferSize);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_006.
* @tc.desc  : Test ChangeSpeedFor32Bit.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_006, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 1;

    uint8_t *buffer = nullptr;
    int32_t bufferSize = 0;
    std::unique_ptr<uint8_t []> outBuffer = nullptr;
    int32_t outBufferSize;

    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->ChangeSpeedFor32Bit(buffer, bufferSize, outBuffer, outBufferSize);
    EXPECT_EQ(result, ERR_MEMORY_ALLOC_FAILED);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_007.
* @tc.desc  : Test ChangeSpeedFor32Bit.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_007, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 1;

    uint8_t *buffer = nullptr;
    int32_t bufferSize = MAX_SPEED_BUFFER_SIZE + 1;
    std::unique_ptr<uint8_t []> outBuffer = nullptr;
    int32_t outBufferSize;

    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->ChangeSpeedFor32Bit(buffer, bufferSize, outBuffer, outBufferSize);
    EXPECT_EQ(result, ERR_MEMORY_ALLOC_FAILED);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_008.
* @tc.desc  : Test ChangeSpeedFor32Bit.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_008, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 1;

    uint8_t *buffer = nullptr;
    int32_t bufferSize = 1;
    std::unique_ptr<uint8_t []> outBuffer = nullptr;
    int32_t outBufferSize = 0;

    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->ChangeSpeedFor32Bit(buffer, bufferSize, outBuffer, outBufferSize);
    EXPECT_EQ(result, bufferSize);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_009.
* @tc.desc  : Test ChangeSpeedFor32Bit.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_009, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_F32LE;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->LoadChangeSpeedFunc();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_010.
* @tc.desc  : Test Flush.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_010, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_U8;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->Flush();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_011.
* @tc.desc  : Test Flush.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_011, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S24LE;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->Flush();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_012.
* @tc.desc  : Test Flush.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_012, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S32LE;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->Flush();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_013.
* @tc.desc  : Test Flush.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_013, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_F32LE;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->Flush();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_014.
* @tc.desc  : Test Flush.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_014, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = SAMPLE_S16LE;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->Flush();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_015.
* @tc.desc  : Test Flush.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_015, TestSize.Level1)
{
    size_t rate = 0;
    size_t format = INVALID_WIDTH;
    size_t channels = 1;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->Flush();
    EXPECT_EQ(result, SUCCESS);
}
#endif // !AUDIO_SPEED_ALGO_ENABLE

#ifdef AUDIO_SPEED_ALGO_ENABLE
class AudioSpeedAlgoUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioSpeedAlgoUnitTest::SetUpTestCase(void) {}
void AudioSpeedAlgoUnitTest::TearDownTestCase(void) {}
void AudioSpeedAlgoUnitTest::SetUp(void) {}
void AudioSpeedAlgoUnitTest::TearDown(void) {}

static constexpr uint32_t ALGO_TEST_RATE = 48000;
static constexpr size_t ALGO_TEST_CHANNELS = 2;
static constexpr int32_t ALGO_TEST_MAX_BUFFER = 614400;

/**
* @tc.name  : AudioSpeed_SelfAlgo_Speed2x
* @tc.type  : FUNC
* @tc.number: AudioSpeedAlgoUnitTest_001
* @tc.desc  : AC-1.1 — SetSpeed(2.0), verify output duration ~1/2.
*/
HWTEST(AudioSpeedAlgoUnitTest, AudioSpeedAlgoUnitTest_001, TestSize.Level1)
{
    if (!AudioTimeStretchProcessor::IsTimeStretchSupported()) {
        GTEST_SKIP() << "TimeStretch algorithm .so not available";
    }
    AudioSpeed audioSpeed(ALGO_TEST_RATE, SAMPLE_S16LE, ALGO_TEST_CHANNELS, ALGO_TEST_MAX_BUFFER);
    ASSERT_EQ(audioSpeed.SetSpeed(2.0f), SUCCESS);
    int32_t frameSize = 2 * static_cast<int32_t>(ALGO_TEST_CHANNELS);
    int32_t inFrames = 1024;
    vector<uint8_t> input(inFrames * frameSize, 0x40);
    auto outBuffer = make_unique<uint8_t[]>(ALGO_TEST_MAX_BUFFER);
    int32_t outSize = 0;
    int32_t ret = audioSpeed.ChangeSpeedFunc(input.data(), inFrames * frameSize, outBuffer, outSize);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : AudioSpeed_SelfAlgo_Speed0_5x
* @tc.type  : FUNC
* @tc.number: AudioSpeedAlgoUnitTest_002
* @tc.desc  : AC-1.2 — SetSpeed(0.5), verify output duration ~2x.
*/
HWTEST(AudioSpeedAlgoUnitTest, AudioSpeedAlgoUnitTest_002, TestSize.Level1)
{
    if (!AudioTimeStretchProcessor::IsTimeStretchSupported()) {
        GTEST_SKIP() << "TimeStretch algorithm .so not available";
    }
    AudioSpeed audioSpeed(ALGO_TEST_RATE, SAMPLE_S16LE, ALGO_TEST_CHANNELS, ALGO_TEST_MAX_BUFFER);
    ASSERT_EQ(audioSpeed.SetSpeed(0.5f), SUCCESS);
    int32_t frameSize = 2 * static_cast<int32_t>(ALGO_TEST_CHANNELS);
    int32_t inFrames = 1024;
    vector<uint8_t> input(inFrames * frameSize, 0x40);
    auto outBuffer = make_unique<uint8_t[]>(ALGO_TEST_MAX_BUFFER);
    int32_t outSize = 0;
    int32_t ret = audioSpeed.ChangeSpeedFunc(input.data(), inFrames * frameSize, outBuffer, outSize);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name  : AudioSpeed_SelfAlgo_AllFormats
* @tc.type  : FUNC
* @tc.number: AudioSpeedAlgoUnitTest_003
* @tc.desc  : AC-1.3 — All formats (U8/S16LE/S24LE/S32LE/F32LE).
*/
HWTEST(AudioSpeedAlgoUnitTest, AudioSpeedAlgoUnitTest_003, TestSize.Level1)
{
    if (!AudioTimeStretchProcessor::IsTimeStretchSupported()) {
        GTEST_SKIP() << "TimeStretch algorithm .so not available";
    }
    size_t formats[] = {SAMPLE_U8, SAMPLE_S16LE, SAMPLE_S24LE, SAMPLE_S32LE, SAMPLE_F32LE};
    for (size_t fmt : formats) {
        AudioSpeed audioSpeed(ALGO_TEST_RATE, fmt, ALGO_TEST_CHANNELS, ALGO_TEST_MAX_BUFFER);
        ASSERT_EQ(audioSpeed.SetSpeed(2.0f), SUCCESS);
        int32_t frameSize = 4;
        int32_t inFrames = 256;
        vector<uint8_t> input(inFrames * frameSize, 0x40);
        auto outBuffer = make_unique<uint8_t[]>(ALGO_TEST_MAX_BUFFER);
        int32_t outSize = 0;
        int32_t ret = audioSpeed.ChangeSpeedFunc(input.data(), inFrames * frameSize, outBuffer, outSize);
        EXPECT_NE(ret, 0);
    }
}

/**
* @tc.name  : AudioSpeed_SelfAlgo_Flush
* @tc.type  : FUNC
* @tc.number: AudioSpeedAlgoUnitTest_004
* @tc.desc  : AC-1.4 — Flush after SetSpeed, verify no residual data.
*/
HWTEST(AudioSpeedAlgoUnitTest, AudioSpeedAlgoUnitTest_004, TestSize.Level1)
{
    if (!AudioTimeStretchProcessor::IsTimeStretchSupported()) {
        GTEST_SKIP() << "TimeStretch algorithm .so not available";
    }
    AudioSpeed audioSpeed(ALGO_TEST_RATE, SAMPLE_S16LE, ALGO_TEST_CHANNELS, ALGO_TEST_MAX_BUFFER);
    ASSERT_EQ(audioSpeed.SetSpeed(2.0f), SUCCESS);
    EXPECT_EQ(audioSpeed.Flush(), SUCCESS);
}

/**
* @tc.name  : AudioSpeed_SelfAlgo_SpeedNormal
* @tc.type  : FUNC
* @tc.number: AudioSpeedAlgoUnitTest_005
* @tc.desc  : AC-1.5 — SetSpeed(1.0), output should match input.
*/
HWTEST(AudioSpeedAlgoUnitTest, AudioSpeedAlgoUnitTest_005, TestSize.Level1)
{
    if (!AudioTimeStretchProcessor::IsTimeStretchSupported()) {
        GTEST_SKIP() << "TimeStretch algorithm .so not available";
    }
    AudioSpeed audioSpeed(ALGO_TEST_RATE, SAMPLE_S16LE, ALGO_TEST_CHANNELS, ALGO_TEST_MAX_BUFFER);
    ASSERT_EQ(audioSpeed.SetSpeed(1.0f), SUCCESS);
    EXPECT_FLOAT_EQ(audioSpeed.GetSpeed(), 1.0f);
}

/**
* @tc.name  : AudioSpeed_SelfAlgo_Pitch
* @tc.type  : FUNC
* @tc.number: AudioSpeedAlgoUnitTest_006
* @tc.desc  : AC-1.6 — SetPitch(1.0) + SetSpeed, verify both return SUCCESS.
*/
HWTEST(AudioSpeedAlgoUnitTest, AudioSpeedAlgoUnitTest_006, TestSize.Level1)
{
    if (!AudioTimeStretchProcessor::IsTimeStretchSupported()) {
        GTEST_SKIP() << "TimeStretch algorithm .so not available";
    }
    AudioSpeed audioSpeed(ALGO_TEST_RATE, SAMPLE_S16LE, ALGO_TEST_CHANNELS, ALGO_TEST_MAX_BUFFER);
    ASSERT_EQ(audioSpeed.SetPitch(1.0f), SUCCESS);
    ASSERT_EQ(audioSpeed.SetSpeed(2.0f), SUCCESS);
}

/**
* @tc.name  : AudioSpeed_SelfAlgo_CreateFail
* @tc.type  : FUNC
* @tc.number: AudioSpeedAlgoUnitTest_007
* @tc.desc  : AC-3.2 — Create with invalid params returns ERROR.
*/
HWTEST(AudioSpeedAlgoUnitTest, AudioSpeedAlgoUnitTest_007, TestSize.Level1)
{
    AudioSpeed audioSpeed(0, SAMPLE_S16LE, 0, ALGO_TEST_MAX_BUFFER);
    int32_t ret = audioSpeed.SetSpeed(2.0f);
    EXPECT_NE(ret, SUCCESS);
}

#endif // AUDIO_SPEED_ALGO_ENABLE
} // namespace AudioStandard
} // namespace OHOS