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
#include <parcel.h>

#include "audio_errors.h"
#include "audio_limiter_adapter.h"
#include "audio_service_log.h"
#include "audio_stream_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const int32_t TEST_MAX_REQUEST = 7680; // buffer size for 20ms 2channel 48000Hz

class AudioLimiterAdapterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioLimiterAdapterUnitTest::SetUpTestCase(void) {}

void AudioLimiterAdapterUnitTest::TearDownTestCase(void) {}

void AudioLimiterAdapterUnitTest::SetUp(void) {}

void AudioLimiterAdapterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test LimiterManagerCreate API
 * @tc.type  : FUNC
 * @tc.number: LimiterManagerCreate_001
 * @tc.desc  : Test LimiterManagerCreate interface when first create.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerCreate_001, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test LimiterManagerCreate API
 * @tc.type  : FUNC
 * @tc.number: LimiterManagerCreate_002
 * @tc.desc  : Test LimiterManagerCreate interface when create again.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerCreate_002, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test LimiterManagerSetConfig API
 * @tc.type  : FUNC
 * @tc.number: LimiterManagerSetConfig_001
 * @tc.desc  : Test LimiterManagerSetConfig interface when config is vaild.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerSetConfig_001, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = LimiterManagerSetConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test LimiterManagerSetConfig API
 * @tc.type  : FUNC
 * @tc.number: LimiterManagerSetConfig_002
 * @tc.desc  : Test LimiterManagerSetConfig interface when config is invaild.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerSetConfig_002, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = LimiterManagerSetConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, MONO);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test LimiterManagerProcess API
 * @tc.type  : FUNC
 * @tc.number: LimiterManagerProcess_001
 * @tc.desc  : Test LimiterManagerProcess interface when inBuffer or outBuffer is nullptr.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerProcess_001, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = TEST_MAX_REQUEST / SAMPLE_F32LE;
    float *inBuffer = nullptr;
    float *outBuffer = nullptr;
    ret = LimiterManagerProcess(sinkIndex, frameLen, inBuffer, outBuffer);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test LimiterManagerProcess API
 * @tc.type  : FUNC
 * @tc.number: LimiterManagerProcess_002
 * @tc.desc  : Test LimiterManagerProcess interface when frameLen is vaild.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerProcess_002, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = LimiterManagerSetConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = TEST_MAX_REQUEST / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = LimiterManagerProcess(sinkIndex, frameLen, inBuffer, outBuffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test LimiterManagerProcess API
 * @tc.type  : FUNC-
 * @tc.number: LimiterManagerProcess_003
 * @tc.desc  : Test LimiterManagerProcess interface when frameLen is invaild.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerProcess_003, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = LimiterManagerSetConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = TEST_MAX_REQUEST / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = LimiterManagerProcess(sinkIndex, 0, inBuffer, outBuffer);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test LimiterManagerRelease API
 * @tc.type  : FUNC
 * @tc.number: LimiterManagerRelease_001
 * @tc.desc  : Test LimiterManagerRelease interface when limiter has been created.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerRelease_001, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = LimiterManagerRelease(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test LimiterManagerRelease API
 * @tc.type  : FUNC
 * @tc.number: LimiterManagerRelease_002
 * @tc.desc  : Test LimiterManagerRelease interface when limiter has been released.
 */
HWTEST_F(AudioLimiterAdapterUnitTest, LimiterManagerRelease_002, TestSize.Level1)
{
    int32_t sinkIndex = 1;
    int32_t ret = LimiterManagerCreate(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = LimiterManagerRelease(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = LimiterManagerRelease(sinkIndex);
    EXPECT_EQ(ret, ERROR);
}
} // namespace AudioStandard
} // namespace OHOS