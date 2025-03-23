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
    int32_t bufferSize = 100001;
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
    int32_t bufferSize = 100001;
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
} // namespace AudioStandard
} // namespace OHOS