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
#include "audio_limiter.h"
#include "audio_service_log.h"
#include "audio_stream_info.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const int32_t BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT = 7680; // buffer size for 20ms 2channel 48000Hz 32bit
const int32_t BUFFER_SIZE_20MS_2CH_44100HZ_16_BIT = 3528; // buffer size for 20ms 2channel 44100Hz 16bit
const int32_t AUDIO_MS_PER_S = 1000;
const int32_t PROC_COUNT = 4; // process 4 times
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
    limiter = std::make_shared<AudioLimiter>(sinkIndex);
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
    EXPECT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
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
    EXPECT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, MONO);
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
    EXPECT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = limiter->Process(frameLen, inBuffer, outBuffer);
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
    EXPECT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = limiter->Process(0, inBuffer, outBuffer);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process_003
 * @tc.desc  : Test Process interface when framelen is vaild.
 */
HWTEST_F(AudioLimiterUnitTest, Process_003, TestSize.Level1)
{
    EXPECT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(BUFFER_SIZE_20MS_2CH_44100HZ_16_BIT, SAMPLE_S16LE + 1, SAMPLE_RATE_44100, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = BUFFER_SIZE_20MS_2CH_44100HZ_16_BIT / (SAMPLE_S16LE + 1);
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = limiter->Process(frameLen, inBuffer, outBuffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_001
 * @tc.desc  : Test GetLatency interface.
 */
HWTEST_F(AudioLimiterUnitTest, GetLatency_001, TestSize.Level1)
{
    EXPECT_NE(limiter, nullptr);

    int32_t ret = limiter->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiter->GetLatency();
    EXPECT_EQ(ret, BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT * AUDIO_MS_PER_S /
        (SAMPLE_F32LE * SAMPLE_RATE_48000 * STEREO * PROC_COUNT));
}
} // namespace AudioStandard
} // namespace OHOS
