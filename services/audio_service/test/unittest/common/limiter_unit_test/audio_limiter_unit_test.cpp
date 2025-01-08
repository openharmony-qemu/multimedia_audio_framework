/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_limiter.h"
#include "audio_log.h"
#include "audio_stream_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

constexpr int32_t TEST_MAX_REQUEST = 7680;

static std::shared_ptr<AudioLimiter> limiter;
class AudioLimiterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioLimiterUnitTest::SetUpTestCase(void) {}

void AudioLimiterUnitTest::TearDownTestCase(void)
{
    limiter.reset();
}

void AudioLimiterUnitTest::SetUp(void)
{
    int32_t sinkIndex = 1;
    std::shared_ptr<AudioLimiter> limiter = std::make_shared<AudioLimiter>(sinkIndex);
}

void AudioLimiterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test SetConfig API
 * @tc.type  : FUNC
 * @tc.number: SetConfig_001
 * @tc.desc  : Test SetConfig interface when config in vaild.
 */
HWTEST_F(AudioLimiterUnitTest, SetConfig_001, TestSize.Level1)
{
    EXCEPT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetConfig API
 * @tc.type  : FUNC
 * @tc.number: SetConfig_002
 * @tc.desc  : Test SetConfig interface when config is invaild.
 */
HWTEST_F(AudioLimiterUnitTest, SetConfig_002, TestSize.Level1)
{
    EXCEPT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, MONO);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process_001
 * @tc.desc  : Test Process interface when framelen is vaild.
 */
HWTEST_F(AudioLimiterUnitTest, Process_001, TestSize.Level1)
{
    EXCEPT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    std::vector<float> inBuffer(TEST_MAX_REQUEST, 0);
    std::vector<float> outBuffer(TEST_MAX_REQUEST, 0);
    float *inBuffer = inBuffer.data();
    float *outBuffer = outBuffer.data();
    result = limiter->Process(TEST_MAX_REQUEST, inBuffer, outBuffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process_002
 * @tc.desc  : Test Process interface when framelen is invaild.
 */
HWTEST_F(AudioLimiterUnitTest, Process_002, TestSize.Level1)
{
    EXCEPT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    std::vector<float> inBuffer(TEST_MAX_REQUEST, 0);
    std::vector<float> outBuffer(TEST_MAX_REQUEST, 0);
    float *inBuffer = inBuffer.data();
    float *outBuffer = outBuffer.data();
    result = limiter->Process(0, inBuffer, outBuffer);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_001
 * @tc.desc  : Test GetLatency interface.
 */
HWTEST_F(AudioLimiterUnitTest, GetLatency_001, TestSize.Level1)
{
    EXCEPT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiter->GetLatency();
    EXPECT_EQ(ret, TEST_MAX_REQUEST / (SAMPLE_F32LE * SAMPLE_RATE_48000 * STEREO) * AUDIO_MS_PER_S);
}
} // namespace AudioStandard
} // namespace OHOS
