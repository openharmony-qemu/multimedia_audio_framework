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
#ifdef AUDIO_SPEED_ALGO_ENABLE
#include "audio_suite_time_stretch.h"
#endif

using namespace testing::ext;
using namespace std;
namespace OHOS {
namespace AudioStandard {

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
    size_t rate = 48000;
    size_t format = SAMPLE_S24LE;
    size_t channels = 2;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    audioSpeed->Init();
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
    size_t rate = 48000;
    size_t format = SAMPLE_S32LE;
    size_t channels = 2;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    audioSpeed->Init();
    auto result = audioSpeed->LoadChangeSpeedFunc();
    EXPECT_EQ(result, SUCCESS);
}

#ifndef AUDIO_SPEED_ALGO_ENABLE
/**
* @tc.name  : Test AudioSpeed API
* @tc.type  : FUNC
* @tc.number: AudioSpeedUnitTest_003.
* @tc.desc  : Test ChangeSpeedFor24Bit.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeedUnitTest_003, TestSize.Level1)
{
    size_t rate = 48000;
    size_t format = SAMPLE_S32LE;
    size_t channels = 2;

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
#endif // AUDIO_SPEED_ALGO_ENABLE

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
    audioSpeed->Init();
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

/**
* @tc.name  : Test CalculateMaxSpeedBufferSize for speed >= 1.0
* @tc.type  : FUNC
* @tc.number: AudioSpeed_CalculateMaxSpeedBufferSize_001
* @tc.desc  : Test dynamic output buffer size for speed >= 1.0 (factor 1, base 100ms).
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_CalculateMaxSpeedBufferSize_001, TestSize.Level1)
{
    // 48kHz, 2ch, S16LE: MAX_CBBUF_IN_USEC(100ms) = 100000 * 48000 * 4 / 1000000 = 19200 bytes
    int32_t cacheBufferSize = 19200;
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 2, 1.0f), cacheBufferSize);
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 2, 2.0f), cacheBufferSize);
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 2, 4.0f), cacheBufferSize);
}

/**
* @tc.name  : Test CalculateMaxSpeedBufferSize for slow speed
* @tc.type  : FUNC
* @tc.number: AudioSpeed_CalculateMaxSpeedBufferSize_002
* @tc.desc  : Test dynamic output buffer size for speed < 1.0 (factor = ceil(1/speed)+1).
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_CalculateMaxSpeedBufferSize_002, TestSize.Level1)
{
    int32_t cacheBufferSize = 19200; // 48kHz, 2ch, S16LE
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 2, 0.5f), cacheBufferSize * 3);
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 2, 0.3f), cacheBufferSize * 5);
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 2, 0.125f), cacheBufferSize * 9);
}

/**
* @tc.name  : Test CalculateMaxSpeedBufferSize invalid and fallback
* @tc.type  : FUNC
* @tc.number: AudioSpeed_CalculateMaxSpeedBufferSize_003
* @tc.desc  : Test factor 1 for zero/negative speed and MAX_SPEED_BUFFER_SIZE fallback for invalid params.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_CalculateMaxSpeedBufferSize_003, TestSize.Level1)
{
    int32_t cacheBufferSize = 19200; // 48kHz, 2ch, S16LE
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 2, 0.0f), cacheBufferSize);
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 2, -1.0f), cacheBufferSize);
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(0, SAMPLE_S16LE, 2, 0.5f), MAX_SPEED_BUFFER_SIZE);
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(48000, SAMPLE_S16LE, 0, 0.5f), MAX_SPEED_BUFFER_SIZE);
}

/**
* @tc.name  : Test CalculateMaxSpeedBufferSize worst-case format
* @tc.type  : FUNC
* @tc.number: AudioSpeed_CalculateMaxSpeedBufferSize_004
* @tc.desc  : Test 192kHz/8ch/float 100ms equals MAX_SPEED_BUFFER_SIZE at speed >= 1.0.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_CalculateMaxSpeedBufferSize_004, TestSize.Level1)
{
    // 192kHz, 8ch, F32LE: MAX_CBBUF_IN_USEC(100ms) = 100000 * 192000 * 32 / 1000000 = 614400 == MAX_SPEED_BUFFER_SIZE
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(192000, SAMPLE_F32LE, 8, 1.0f), MAX_SPEED_BUFFER_SIZE);
    EXPECT_EQ(AudioSpeed::CalculateMaxSpeedBufferSize(192000, SAMPLE_F32LE, 8, 2.0f), MAX_SPEED_BUFFER_SIZE);
}

#ifdef AUDIO_SPEED_ALGO_ENABLE

static bool IsAudioSuiteAvailable()
{
    return AudioSuiteTimeStretch::IsTimeStretchSupported();
}

/**
* @tc.name  : Test AudioSpeed AlgoPath
* @tc.type  : FUNC
* @tc.number: AudioSpeed_AlgoPath_Init_001
* @tc.desc  : Test AudioSpeed algo path Init.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_AlgoPath_Init_001, TestSize.Level1)
{
    if (!IsAudioSuiteAvailable()) {
        GTEST_SKIP() << "libaudio_time_stretch.z.so not available";
    }
    size_t rate = 48000;
    size_t format = SAMPLE_S16LE;
    size_t channels = 2;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    auto result = audioSpeed->Init();
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed AlgoPath
* @tc.type  : FUNC
* @tc.number: AudioSpeed_AlgoPath_SetSpeed_GetSpeed_001
* @tc.desc  : Test AudioSpeed algo path SetSpeed and GetSpeed.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_AlgoPath_SetSpeed_GetSpeed_001, TestSize.Level1)
{
    if (!IsAudioSuiteAvailable()) {
        GTEST_SKIP() << "libaudio_time_stretch.z.so not available";
    }
    size_t rate = 48000;
    size_t format = SAMPLE_S16LE;
    size_t channels = 2;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    audioSpeed->Init();
    audioSpeed->SetSpeed(2.0f);
    EXPECT_FLOAT_EQ(audioSpeed->GetSpeed(), 2.0f);
    audioSpeed->SetSpeed(0.5f);
    EXPECT_FLOAT_EQ(audioSpeed->GetSpeed(), 0.5f);
}

/**
* @tc.name  : Test AudioSpeed AlgoPath
* @tc.type  : FUNC
* @tc.number: AudioSpeed_AlgoPath_Process_AllFormats_001
* @tc.desc  : Test AudioSpeed algo path Process with all formats.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_AlgoPath_Process_AllFormats_001, TestSize.Level1)
{
    if (!IsAudioSuiteAvailable()) {
        GTEST_SKIP() << "libaudio_time_stretch.z.so not available";
    }
    size_t rate = 48000;
    size_t channels = 2;
    size_t formats[] = {SAMPLE_U8, SAMPLE_S16LE, SAMPLE_S32LE, SAMPLE_F32LE};
    for (size_t fmt : formats) {
        auto audioSpeed = std::make_shared<AudioSpeed>(rate, fmt, channels);
        audioSpeed->Init();
        audioSpeed->SetSpeed(2.0f);
        int32_t bufferSize = 4096;
        auto inBuffer = std::make_unique<uint8_t[]>(bufferSize);
        auto outBuffer = std::make_unique<uint8_t[]>(bufferSize * 8);
        int32_t outBufferSize = 0;
        auto ret = audioSpeed->ChangeSpeedFunc(inBuffer.get(), bufferSize, outBuffer, outBufferSize);
        EXPECT_NE(ret, 0);
    }
}

/**
* @tc.name  : Test AudioSpeed AlgoPath
* @tc.type  : FUNC
* @tc.number: AudioSpeed_AlgoPath_Flush_001
* @tc.desc  : Test AudioSpeed algo path Flush.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_AlgoPath_Flush_001, TestSize.Level1)
{
    if (!IsAudioSuiteAvailable()) {
        GTEST_SKIP() << "libaudio_time_stretch.z.so not available";
    }
    size_t rate = 48000;
    size_t format = SAMPLE_F32LE;
    size_t channels = 2;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    audioSpeed->Init();
    audioSpeed->SetSpeed(2.0f);
    auto ret = audioSpeed->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSpeed Priority
* @tc.type  : FUNC
* @tc.number: AudioSpeed_Priority_AlgoPreferred_001
* @tc.desc  : Test algo path preferred when .so available.
*/
HWTEST(AudioSpeedUnitTest, AudioSpeed_Priority_AlgoPreferred_001, TestSize.Level1)
{
    if (!IsAudioSuiteAvailable()) {
        GTEST_SKIP() << "libaudio_time_stretch.z.so not available";
    }
    size_t rate = 48000;
    size_t format = SAMPLE_S16LE;
    size_t channels = 2;
    auto audioSpeed = std::make_shared<AudioSpeed>(rate, format, channels);
    audioSpeed->Init();
    audioSpeed->SetSpeed(2.0f);
    // When algo available, Process should work via AudioSuiteTimeStretch
    int32_t bufferSize = 4096;
    auto inBuffer = std::make_unique<uint8_t[]>(bufferSize);
    auto outBuffer = std::make_unique<uint8_t[]>(bufferSize * 8);
    int32_t outBufferSize = 0;
    auto ret = audioSpeed->ChangeSpeedFunc(inBuffer.get(), bufferSize, outBuffer, outBufferSize);
    EXPECT_NE(ret, 0);
}

#endif // AUDIO_SPEED_ALGO_ENABLE
} // namespace AudioStandard
} // namespace OHOS