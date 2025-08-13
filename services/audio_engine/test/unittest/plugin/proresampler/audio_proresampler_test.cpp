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
#include <vector>
#include <map>
#include <gtest/gtest.h>
#include "audio_engine_log.h"
#include "audio_proresampler.h"
#include "audio_stream_info.h"
#include "securec.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const static std::vector<uint32_t>  TEST_CHANNELS = {MONO, STEREO, CHANNEL_6};

const static std::map<uint32_t, uint32_t> TEST_SAMPLE_RATE_COMBINATION = { // {input, output} combination
    {SAMPLE_RATE_24000, SAMPLE_RATE_48000},
    {SAMPLE_RATE_16000, SAMPLE_RATE_48000},
    {SAMPLE_RATE_44100, SAMPLE_RATE_192000},
    {SAMPLE_RATE_48000, SAMPLE_RATE_24000},
    {SAMPLE_RATE_48000, SAMPLE_RATE_16000},
    {SAMPLE_RATE_192000, SAMPLE_RATE_44100},
};

constexpr uint32_t INVALID_QUALITY = -1;
constexpr uint32_t QUALITY_ONE = 1;
constexpr uint32_t FRAME_LEN_20MS = 20;
constexpr uint32_t FRAME_LEN_40MS = 40;
constexpr uint32_t MS_PER_SECOND = 1000;

class AudioProResamplerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioProResamplerTest::SetUp() {}

void AudioProResamplerTest::TearDown() {}

HWTEST_F(AudioProResamplerTest, InitTest, TestSize.Level0)
{
    // test invalid input
    int32_t err = RESAMPLER_ERR_SUCCESS;
    SingleStagePolyphaseResamplerInit(STEREO, SAMPLE_RATE_24000, SAMPLE_RATE_48000, INVALID_QUALITY, &err);
    EXPECT_EQ(err, RESAMPLER_ERR_INVALID_ARG);

    // test valid input
    SingleStagePolyphaseResamplerInit(STEREO, SAMPLE_RATE_24000, SAMPLE_RATE_48000, QUALITY_ONE, &err);
    EXPECT_EQ(err, RESAMPLER_ERR_SUCCESS);

    // test 11025 input
    ProResampler resampler1(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);

    // test other input
    ProResampler resampler2(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
}

HWTEST_F(AudioProResamplerTest, ProcessTest, TestSize.Level0)
{
    // test all input/output combination
    for (uint32_t channels: TEST_CHANNELS) {
        for (auto pair: TEST_SAMPLE_RATE_COMBINATION) {
            uint32_t inRate = pair.first;
            uint32_t outRate = pair.second;
            uint32_t inFrameLen = inRate * FRAME_LEN_20MS / MS_PER_SECOND;
            uint32_t outFrameLen = outRate * FRAME_LEN_20MS / MS_PER_SECOND;
            ProResampler resampler(inRate, outRate, channels, QUALITY_ONE);
            std::vector<float> in(inFrameLen * channels);
            std::vector<float> out(outFrameLen * channels);
            int32_t ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
            EXPECT_EQ(ret, EOK);
        }
    }

    // test 11025 spetial case
    ProResampler resampler(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    uint32_t inFrameLen = SAMPLE_RATE_11025 * FRAME_LEN_40MS / MS_PER_SECOND;
    uint32_t outFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND;
    std::vector<float> in(inFrameLen * STEREO);
    std::vector<float> out(outFrameLen * STEREO);
    // Process first 40ms frame, send first half of data to output
    int32_t ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
    inFrameLen = 0;
    // no new data in, send stored 20ms
    ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
    // no data left, send 0s
    ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
}

HWTEST_F(AudioProResamplerTest, UpdateRatesTest, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_96000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);

    resampler.UpdateRates(SAMPLE_RATE_11025, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_11025);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_11025 * FRAME_LEN_40MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
}

HWTEST_F(AudioProResamplerTest, UpdateChannel, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.channels_, STEREO);

    resampler.UpdateChannels(CHANNEL_6);
    EXPECT_EQ(resampler.channels_, CHANNEL_6);
}

/*
 * @tc.name  : Test UpdateRates API.
 * @tc.type  : FUNC
 * @tc.number: UpdateRatesTest_01.
 * @tc.desc  : Test UpdateRates, set inRate is 2.
 */
HWTEST_F(AudioProResamplerTest, UpdateRatesTest_01, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_96000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);

    resampler.UpdateRates(2, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.inRate_, 2);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.expectedInFrameLen_, 2 * FRAME_LEN_40MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
}

/*
 * @tc.name  : Test ErrCodeToString API.
 * @tc.type  : FUNC
 * @tc.number: ErrCodeToString_01.
 * @tc.desc  : Test ErrCodeToString, set errCode is RESAMPLER_ERR_SUCCESS.
 */
HWTEST_F(AudioProResamplerTest, ErrCodeToString_01, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);

    std::string ret = resampler.ErrCodeToString(RESAMPLER_ERR_SUCCESS);
    ASSERT_STREQ(ret.c_str(), "RESAMPLER_ERR_SUCCESS");
}

/*
 * @tc.name  : Test ErrCodeToString API.
 * @tc.type  : FUNC
 * @tc.number: ErrCodeToString_02.
 * @tc.desc  : Test ErrCodeToString, set errCode is RESAMPLER_ERR_ALLOC_FAILED.
 */
HWTEST_F(AudioProResamplerTest, ErrCodeToString_02, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);

    std::string ret = resampler.ErrCodeToString(RESAMPLER_ERR_ALLOC_FAILED);
    ASSERT_STREQ(ret.c_str(), "RESAMPLER_ERR_ALLOC_FAILED");
}

/*
 * @tc.name  : Test ErrCodeToString API.
 * @tc.type  : FUNC
 * @tc.number: ErrCodeToString_03.
 * @tc.desc  : Test ErrCodeToString, set errCode is RESAMPLER_ERR_OVERFLOW.
 */
HWTEST_F(AudioProResamplerTest, ErrCodeToString_03, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);

    std::string ret = resampler.ErrCodeToString(RESAMPLER_ERR_OVERFLOW);
    ASSERT_STREQ(ret.c_str(), "RESAMPLER_ERR_OVERFLOW");
}

/*
 * @tc.name  : Test ErrCodeToString API.
 * @tc.type  : FUNC
 * @tc.number: ErrCodeToString_04.
 * @tc.desc  : Test ErrCodeToString, set errCode is 7.
 */
HWTEST_F(AudioProResamplerTest, ErrCodeToString_04, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);

    std::string ret = resampler.ErrCodeToString(7);
    ASSERT_STREQ(ret.c_str(), "Unknown Error Code");
}

/*
 * @tc.name  : Test ErrCodeToString API.
 * @tc.type  : FUNC
 * @tc.number: ErrCodeToString_05.
 * @tc.desc  : Test ErrCodeToString, set errCode is RESAMPLER_ERR_INVALID_ARG.
 */
HWTEST_F(AudioProResamplerTest, ErrCodeToString_05, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);

    std::string ret = resampler.ErrCodeToString(RESAMPLER_ERR_INVALID_ARG);
    ASSERT_STREQ(ret.c_str(), "RESAMPLER_ERR_INVALID_ARG");
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS