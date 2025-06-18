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
constexpr uint32_t QUALICY_ONE = 1;
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

TEST_F(AudioProResamplerTest, InitTest)
{
    // test invalid input
    int32_t err = RESAMPLER_ERR_SUCCESS;
    SingleStagePolyphaseResamplerInit(STEREO, SAMPLE_RATE_24000, SAMPLE_RATE_48000, INVALID_QUALITY, &err);
    EXPECT_EQ(err, RESAMPLER_ERR_INVALID_ARG);

    // test valid input
    SingleStagePolyphaseResamplerInit(STEREO, SAMPLE_RATE_24000, SAMPLE_RATE_48000, QUALICY_ONE, &err);
    EXPECT_EQ(err, RESAMPLER_ERR_SUCCESS);

    // test 11025 input
    ProResampler resampler1(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALICY_ONE);

    // test other input
    ProResampler resampler2(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALICY_ONE);
}

TEST_F(AudioProResamplerTest, ProcessTest)
{
    // test all input/output combination
    for (uint32_t channels: TEST_CHANNELS) {
        for (auto pair: TEST_SAMPLE_RATE_COMBINATION) {
            uint32_t inRate = pair.first;
            uint32_t outRate = pair.second;
            uint32_t inFrameLen = inRate * FRAME_LEN_20MS / MS_PER_SECOND;
            uint32_t outFrameLen = outRate * FRAME_LEN_20MS / MS_PER_SECOND;
            ProResampler resampler(inRate, outRate, channels, QUALICY_ONE);
            std::vector<float> in(inFrameLen * channels);
            std::vector<float> out(outFrameLen * channels);
            int32_t ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
            EXPECT_EQ(ret, EOK);
        }
    }

    // test 11025 spetial case
    ProResampler resampler(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALICY_ONE);
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

TEST_F(AudioProResamplerTest, UpdateRatesTest)
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

TEST_F(AudioProResamplerTest, UpdateChannel)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.channels_, STEREO);

    resampler.UpdateChannels(CHANNEL_6);
    EXPECT_EQ(resampler.channels_, CHANNEL_6);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS