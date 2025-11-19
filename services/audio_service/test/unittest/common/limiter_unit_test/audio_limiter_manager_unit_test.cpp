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
#include "audio_limiter_manager.h"
#include "audio_service_log.h"
#include "audio_stream_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const int32_t TEST_MAX_REQUEST = 7680;  // buffer size for 20ms 2channel 48000Hz
const int32_t AUDIO_MS_PER_S = 1000;
const int32_t PROC_COUNT = 4;  // process 4 times
static AudioLmtManager *limiterManager;

class AudioLimiterManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioLimiterManagerUnitTest::SetUpTestCase(void)
{}

void AudioLimiterManagerUnitTest::TearDownTestCase(void)
{}

void AudioLimiterManagerUnitTest::SetUp(void)
{
    limiterManager = AudioLmtManager::GetInstance();
}

void AudioLimiterManagerUnitTest::TearDown(void)
{}

/**
 * @tc.name  : Test CreateLimiter API
 * @tc.type  : FUNC
 * @tc.number: CreateLimiter_001
 * @tc.desc  : Test CreateLimiter interface when first create.
 */
HWTEST_F(AudioLimiterManagerUnitTest, CreateLimiter_001, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test CreateLimiter API
 * @tc.type  : FUNC
 * @tc.number: CreateLimiter_002
 * @tc.desc  : Test CreateLimiter interface when repeate create use the same sinkIndex.
 */
HWTEST_F(AudioLimiterManagerUnitTest, CreateLimiter_002, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetLimiterConfig API
 * @tc.type  : FUNC
 * @tc.number: SetLimiterConfig_001
 * @tc.desc  : Test SetLimiterConfig interface when config is vaild.
 */
HWTEST_F(AudioLimiterManagerUnitTest, SetLimiterConfig_001, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->SetLimiterConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetLimiterConfig API
 * @tc.type  : FUNC
 * @tc.number: SetLimiterConfig_002
 * @tc.desc  : Test SetLimiterConfig interface when config is invaild.
 */
HWTEST_F(AudioLimiterManagerUnitTest, SetLimiterConfig_002, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->SetLimiterConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, MONO);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test ProcessLimiter API
 * @tc.type  : FUNC
 * @tc.number: ProcessLimiter_001
 * @tc.desc  : Test ProcessLimiter interface when inBuffer or outBuffer is nullptr.
 */
HWTEST_F(AudioLimiterManagerUnitTest, ProcessLimiter_001, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t frameLen = TEST_MAX_REQUEST / SAMPLE_F32LE;
    float *inBuffer = nullptr;
    float *outBuffer = nullptr;
    int32_t ret = limiterManager->ProcessLimiter(sinkIndex, frameLen, inBuffer, outBuffer);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test ProcessLimiter API
 * @tc.type  : FUNC
 * @tc.number: ProcessLimiter_002
 * @tc.desc  : Test ProcessLimiter interface when frameLen is vaild.
 */
HWTEST_F(AudioLimiterManagerUnitTest, ProcessLimiter_002, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->SetLimiterConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = TEST_MAX_REQUEST / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = limiterManager->ProcessLimiter(sinkIndex, frameLen, inBuffer, outBuffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ProcessLimiter API
 * @tc.type  : FUNC
 * @tc.number: ProcessLimiter_003
 * @tc.desc  : Test ProcessLimiter interface when frameLen is invaild.
 */
HWTEST_F(AudioLimiterManagerUnitTest, ProcessLimiter_003, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->SetLimiterConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = TEST_MAX_REQUEST / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = limiterManager->ProcessLimiter(sinkIndex, 0, inBuffer, outBuffer);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test ReleaseLimiter API
 * @tc.type  : FUNC
 * @tc.number: ReleaseLimiter_001
 * @tc.desc  : Test ReleaseLimiter interface when limiter has been created.
 */
HWTEST_F(AudioLimiterManagerUnitTest, ReleaseLimiter_001, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->ReleaseLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test ReleaseLimiter API
 * @tc.type  : FUNC
 * @tc.number: ReleaseLimiter_002
 * @tc.desc  : Test ReleaseLimiter interface when limiter has been created and released.
 */
HWTEST_F(AudioLimiterManagerUnitTest, ReleaseLimiter_002, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->ReleaseLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->ReleaseLimiter(sinkIndex);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_001
 * @tc.desc  : Test GetLatency interface when limiter has not been created.
 */
HWTEST_F(AudioLimiterManagerUnitTest, GetLatency_001, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    uint32_t latency = limiterManager->GetLatency(sinkIndex);
    EXPECT_EQ(latency, 0);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_002
 * @tc.desc  : Test GetLatency interface when limiter has been created.
 */
HWTEST_F(AudioLimiterManagerUnitTest, GetLatency_002, TestSize.Level1)
{
    EXPECT_NE(limiterManager, nullptr);

    int32_t sinkIndex = 0;
    int32_t ret = limiterManager->CreateLimiter(sinkIndex);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->SetLimiterConfig(sinkIndex, TEST_MAX_REQUEST, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiterManager->GetLatency(sinkIndex);
    EXPECT_EQ(ret,
        TEST_MAX_REQUEST * AUDIO_MS_PER_S /
            (static_cast<uint8_t>(SAMPLE_F32LE) * static_cast<uint32_t>(SAMPLE_RATE_48000) *
                static_cast<uint8_t>(STEREO) * PROC_COUNT));
}
}  // namespace AudioStandard
}  // namespace OHOS