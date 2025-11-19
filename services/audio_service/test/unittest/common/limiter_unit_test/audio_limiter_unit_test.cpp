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
const int32_t BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT = 7680;  // buffer size for 20ms 2channel 48000Hz 32bit
const int32_t AUDIO_MS_PER_S = 1000;
const int32_t PROC_COUNT = 4;  // process 4 times

constexpr int32_t DEFAULT_INPUT_BYTE_PER_SAMPLE = sizeof(float);
constexpr int32_t DEFAULT_INPUT_SAMPLE_RATE = 48000;
constexpr int32_t DEFAULT_INPUT_CHANNEL_NUM = 2;
constexpr int32_t DEFAULT_FRAME_TIME_MS = 20;
constexpr int32_t DEFAULT_INPUT_SAMPLE_COUNT =
    DEFAULT_INPUT_SAMPLE_RATE * DEFAULT_INPUT_CHANNEL_NUM * DEFAULT_FRAME_TIME_MS / 1000;
constexpr int32_t DEFAULT_INPUT_FRAME_BYTES = DEFAULT_INPUT_SAMPLE_COUNT * DEFAULT_INPUT_BYTE_PER_SAMPLE;
constexpr int32_t SPECIAL_FRAME_TIME_MS = 40;
constexpr int32_t SPECIAL_INPUT_SAMPLE_RATE = 22050;
constexpr int32_t SPECIAL_INPUT_SAMPLE_COUNT =
    SPECIAL_INPUT_SAMPLE_RATE * DEFAULT_INPUT_CHANNEL_NUM * SPECIAL_FRAME_TIME_MS / 1000;
constexpr int32_t SPECIAL_INPUT_FRAME_BYTES = SPECIAL_INPUT_SAMPLE_COUNT * DEFAULT_INPUT_BYTE_PER_SAMPLE;

constexpr int32_t SUPPORT_NORMAL_SAMPLE_RATES[] = {
    8000, 12000, 16000, 24000, 32000, 44100, 48000, 64000, 88200, 96000, 176400, 192000};

class AudioLimiterUnitTest : public testing::Test {
public:
    void SetUp();
    void TearDown();

    std::shared_ptr<AudioLimiter> limiter_;
};

void AudioLimiterUnitTest::SetUp(void)
{
    int32_t sinkIndex = 1;
    limiter_ = std::make_shared<AudioLimiter>(sinkIndex);
}

void AudioLimiterUnitTest::TearDown(void)
{}

/**
 * @tc.name  : Test SetConfig API
 * @tc.type  : FUNC
 * @tc.number: SetConfig_001
 * @tc.desc  : Test SetConfig interface when config in vaild.
 */
HWTEST_F(AudioLimiterUnitTest, SetConfig_001, TestSize.Level1)
{
    EXPECT_NE(limiter_, nullptr);

    int32_t ret = limiter_->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
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
    EXPECT_NE(limiter_, nullptr);

    int32_t ret = limiter_->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, MONO);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process_001
 * @tc.desc  : Test Process interface when framelen is vaild.
 */
HWTEST_F(AudioLimiterUnitTest, Process_001, TestSize.Level1)
{
    EXPECT_NE(limiter_, nullptr);

    int32_t ret = limiter_->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = limiter_->Process(frameLen, inBuffer, outBuffer);
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
    EXPECT_NE(limiter_, nullptr);

    int32_t ret = limiter_->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    int32_t frameLen = BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT / SAMPLE_F32LE;
    std::vector<float> inBufferVector(frameLen, 0);
    std::vector<float> outBufferVector(frameLen, 0);
    float *inBuffer = inBufferVector.data();
    float *outBuffer = outBufferVector.data();
    ret = limiter_->Process(0, inBuffer, outBuffer);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test GetLatency API
 * @tc.type  : FUNC
 * @tc.number: GetLatency_001
 * @tc.desc  : Test GetLatency interface.
 */
HWTEST_F(AudioLimiterUnitTest, GetLatency_001, TestSize.Level1)
{
    EXPECT_NE(limiter_, nullptr);

    int32_t ret = limiter_->SetConfig(BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT, SAMPLE_F32LE, SAMPLE_RATE_48000, STEREO);
    EXPECT_EQ(ret, SUCCESS);
    ret = limiter_->GetLatency();
    EXPECT_EQ(ret,
        BUFFER_SIZE_20MS_2CH_48000HZ_FLOAT * AUDIO_MS_PER_S /
            (static_cast<uint8_t>(SAMPLE_F32LE) * static_cast<uint32_t>(SAMPLE_RATE_48000) *
                static_cast<uint8_t>(STEREO) * PROC_COUNT));
}

HWTEST_F(AudioLimiterUnitTest, SetConfig_WillFailed_While_InputFrameBytes_LE0, TestSize.Level1)
{
    int32_t ret =
        limiter_->SetConfig(0, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));
    ret = limiter_->SetConfig(-1, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));
}

HWTEST_F(AudioLimiterUnitTest, SetConfig_WillFailed_While_BytePerSample_NE32, TestSize.Level1)
{
    int32_t ret =
        limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, 0, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));

    ret = limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, -1, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));

    ret = limiter_->SetConfig(
        DEFAULT_INPUT_FRAME_BYTES, sizeof(uint16_t), DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));
}

HWTEST_F(AudioLimiterUnitTest, SetConfig_WillFailed_While_SampleRate_NotVaild, TestSize.Level1)
{
    int32_t ret =
        limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, 1);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));
    ret = limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, 0);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));
    ret = limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, -1);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));
}

HWTEST_F(AudioLimiterUnitTest, SetConfig_WillFailed_While_ChannelNum_NE2, TestSize.Level1)
{
    int32_t ret =
        limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, 0);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));

    ret = limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, 1);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));

    ret = limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, -1);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));
}

HWTEST_F(AudioLimiterUnitTest, SetConfig_WillFailed_While_InputFrameLen_NotEvenNumber, TestSize.Level1)
{
    // frame 441
    uint32_t inputFrameBytes =
        22050 * DEFAULT_INPUT_CHANNEL_NUM * DEFAULT_INPUT_BYTE_PER_SAMPLE * DEFAULT_FRAME_TIME_MS / 1000;
    int32_t ret = limiter_->SetConfig(
        inputFrameBytes, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));

    // frameLen 882
    inputFrameBytes = 44100 * DEFAULT_INPUT_CHANNEL_NUM * DEFAULT_INPUT_BYTE_PER_SAMPLE * DEFAULT_FRAME_TIME_MS / 1000;
    ret = limiter_->SetConfig(
        inputFrameBytes, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));
}

HWTEST_F(AudioLimiterUnitTest, SetConfig_WillSuccess, TestSize.Level1)
{
    int32_t ret = limiter_->SetConfig(
        DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));
}

HWTEST_F(AudioLimiterUnitTest, Process_WillFailed_Before_SetConfig, TestSize.Level1)
{
    float in;
    float out;
    int32_t ret = limiter_->Process(DEFAULT_INPUT_SAMPLE_COUNT, &in, &out);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_NOT_STARTED));
}

HWTEST_F(AudioLimiterUnitTest, Process_WillFailed_While_InputSampleCount_InBuffer_IS_NULL, TestSize.Level1)
{
    float out;
    int32_t ret = limiter_->Process(DEFAULT_INPUT_SAMPLE_COUNT, nullptr, &out);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_NULL_POINTER));
}

HWTEST_F(AudioLimiterUnitTest, Process_WillFailed_While_InputSampleCount_OutBuffer_IS_NULL, TestSize.Level1)
{
    float in;
    int32_t ret = limiter_->Process(DEFAULT_INPUT_SAMPLE_COUNT, &in, nullptr);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_NULL_POINTER));
}

HWTEST_F(AudioLimiterUnitTest, Process_WillFailed_While_InputSampleCount_Not_Match_SetConfig, TestSize.Level1)
{
    int32_t ret = limiter_->SetConfig(
        DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

    float in;
    float out;
    ret = limiter_->Process(DEFAULT_INPUT_SAMPLE_COUNT + 1, &in, &out);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));

    ret = limiter_->Process(0, &in, &out);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));

    ret = limiter_->Process(-1, &in, &out);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));
}

HWTEST_F(AudioLimiterUnitTest, Process_WillSucess, TestSize.Level1)
{
    int32_t ret = limiter_->SetConfig(
        DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

    float in[DEFAULT_INPUT_SAMPLE_COUNT];
    float out[DEFAULT_INPUT_SAMPLE_COUNT];
    ret = limiter_->Process(DEFAULT_INPUT_SAMPLE_COUNT, in, out);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));
}

HWTEST_F(AudioLimiterUnitTest, Process_WillFailed_While_After_SetConfig_failed, TestSize.Level1)
{
    int32_t ret = limiter_->SetConfig(
        DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

    float in[DEFAULT_INPUT_SAMPLE_COUNT];
    float out[DEFAULT_INPUT_SAMPLE_COUNT];
    ret = limiter_->Process(DEFAULT_INPUT_SAMPLE_COUNT, in, out);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

    ret = limiter_->SetConfig(DEFAULT_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, DEFAULT_INPUT_SAMPLE_RATE, 0);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_INVALID_PARAM));

    ret = limiter_->Process(DEFAULT_INPUT_SAMPLE_COUNT, in, out);
    EXPECT_EQ(ret, static_cast<int32_t>(ERR_NOT_STARTED));
}

HWTEST_F(AudioLimiterUnitTest, Process_WillSuccess_While_Input_Special_SampleRate, TestSize.Level1)
{
    int32_t ret = limiter_->SetConfig(
        SPECIAL_INPUT_FRAME_BYTES, DEFAULT_INPUT_BYTE_PER_SAMPLE, SPECIAL_INPUT_SAMPLE_RATE, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

    float in[SPECIAL_INPUT_SAMPLE_COUNT];
    float out[SPECIAL_INPUT_SAMPLE_COUNT];
    ret = limiter_->Process(SPECIAL_INPUT_SAMPLE_COUNT, in, out);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));
}

HWTEST_F(AudioLimiterUnitTest, Check_Support_Normale_SampleRate, TestSize.Level1)
{
    const int arraySize = sizeof(SUPPORT_NORMAL_SAMPLE_RATES) / sizeof(SUPPORT_NORMAL_SAMPLE_RATES[0]);
    int32_t ret;
    const size_t maxInputSamples = 192000 * DEFAULT_INPUT_CHANNEL_NUM * DEFAULT_FRAME_TIME_MS / 1000;
    float in[maxInputSamples];
    float out[maxInputSamples];
    for (int i = 0; i < arraySize; ++i) {
        int32_t inputSampleCount =
            SUPPORT_NORMAL_SAMPLE_RATES[i] * DEFAULT_INPUT_CHANNEL_NUM * DEFAULT_FRAME_TIME_MS / 1000;
        int32_t inputBytes = inputSampleCount * DEFAULT_INPUT_BYTE_PER_SAMPLE;
        EXPECT_GE(maxInputSamples, inputSampleCount);
        if (maxInputSamples < inputSampleCount) {
            return;
        }
        ret = limiter_->SetConfig(
            inputBytes, DEFAULT_INPUT_BYTE_PER_SAMPLE, SUPPORT_NORMAL_SAMPLE_RATES[i], DEFAULT_INPUT_CHANNEL_NUM);
        EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

        ret = limiter_->Process(inputSampleCount, in, out);
        EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));
    }
}

HWTEST_F(AudioLimiterUnitTest, Check_Special_SampleRate, TestSize.Level1)
{
    const size_t maxInputSamples = 22050 * DEFAULT_INPUT_CHANNEL_NUM * 40 / 1000;
    float in[maxInputSamples];
    float out[maxInputSamples];

    int32_t inputSampleCount = 22050 * DEFAULT_INPUT_CHANNEL_NUM * 40 / 1000;
    int32_t inputBytes = inputSampleCount * DEFAULT_INPUT_BYTE_PER_SAMPLE;
    EXPECT_GE(maxInputSamples, inputSampleCount);
    if (maxInputSamples < inputSampleCount) {
        return;
    }
    int32_t ret = limiter_->SetConfig(inputBytes, DEFAULT_INPUT_BYTE_PER_SAMPLE, 22050, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

    ret = limiter_->Process(inputSampleCount, in, out);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

    inputSampleCount = 11025 * DEFAULT_INPUT_CHANNEL_NUM * 80 / 1000;
    inputBytes = inputSampleCount * DEFAULT_INPUT_BYTE_PER_SAMPLE;
    EXPECT_GE(maxInputSamples, inputSampleCount);
    if (maxInputSamples < inputSampleCount) {
        return;
    }
    ret = limiter_->SetConfig(inputBytes, DEFAULT_INPUT_BYTE_PER_SAMPLE, 11025, DEFAULT_INPUT_CHANNEL_NUM);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));

    ret = limiter_->Process(inputSampleCount, in, out);
    EXPECT_EQ(ret, static_cast<int32_t>(SUCCESS));
}
}  // namespace AudioStandard
}  // namespace OHOS
