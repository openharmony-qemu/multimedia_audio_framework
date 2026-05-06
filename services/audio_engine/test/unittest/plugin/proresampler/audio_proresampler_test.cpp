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
constexpr uint32_t FRAME_LEN_60MS = 60;
constexpr uint32_t FRAME_LEN_100MS = 100; // used for testing arbitrary input sizes
constexpr uint32_t MS_PER_SECOND = 1000;
constexpr uint32_t SAMPLE_RATE_48010 = 48010;
constexpr uint32_t INVALID_SAMPLE_RATE = 2;
constexpr uint32_t INVALID_CHANNELS = 100;
constexpr uint32_t INVALID_CHANNELS2 = 0;

class AudioProResamplerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioProResamplerTest::SetUp() {}

void AudioProResamplerTest::TearDown() {}

HWTEST_F(AudioProResamplerTest, InitTest_001, TestSize.Level0)
{
    // test invalid input
    int32_t err = RESAMPLER_ERR_SUCCESS;
    SingleStagePolyphaseResamplerInit(STEREO, SAMPLE_RATE_24000, SAMPLE_RATE_48000, INVALID_QUALITY, &err);
    EXPECT_EQ(err, RESAMPLER_ERR_INVALID_ARG);

    // inRate and outRate should be different
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.state_, nullptr);

    // test valid input
    SingleStagePolyphaseResamplerInit(STEREO, SAMPLE_RATE_24000, SAMPLE_RATE_48000, QUALITY_ONE, &err);
    EXPECT_EQ(err, RESAMPLER_ERR_SUCCESS);

    // test 11025 input -- unified: expectedInFrameLen_ = 11025 * 20 / 1000 = 220
    ProResampler resampler1(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler1.inRate_, SAMPLE_RATE_11025);
    EXPECT_EQ(resampler1.outRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler1.expectedInFrameLen_, SAMPLE_RATE_11025 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler1.expectedOutFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler1.tmpOutBuf_.size(), resampler1.expectedOutFrameLen_ * STEREO);

    // test other input
    ProResampler resampler2(SAMPLE_RATE_48000, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler2.inRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler2.outRate_, SAMPLE_RATE_44100);
    EXPECT_EQ(resampler2.expectedInFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler2.expectedOutFrameLen_, SAMPLE_RATE_44100 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler2.tmpOutBuf_.size(), resampler2.expectedOutFrameLen_ * STEREO);

    // test custom sample rate that is not multiples of 50 -- unified: expectedInFrameLen_ = 48010 * 20 / 1000 = 960
    ProResampler resampler3(SAMPLE_RATE_48010, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler3.inRate_, SAMPLE_RATE_48010);
    EXPECT_EQ(resampler3.outRate_, SAMPLE_RATE_44100);
    EXPECT_EQ(resampler3.expectedInFrameLen_, SAMPLE_RATE_48010 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler3.expectedOutFrameLen_, SAMPLE_RATE_44100 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler3.tmpOutBuf_.size(), resampler3.expectedOutFrameLen_ * STEREO);
}

/*
 * @tc.name  : Test Process API.
 * @tc.type  : FUNC
 * @tc.number: ProcessTest_001.
 * @tc.desc  : Test Process, unified method for all standard rate combinations.
 */
HWTEST_F(AudioProResamplerTest, ProcessTest_001, TestSize.Level0)
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
}

/*
 * @tc.name  : Test Process API.
 * @tc.type  : FUNC
 * @tc.number: ProcessTest_002.
 * @tc.desc  : Test Process with 11025Hz input using unified method (arbitrary input sizes).
 */
HWTEST_F(AudioProResamplerTest, ProcessTest_002, TestSize.Level0)
{
    // test 11025Hz with 20ms input
    ProResampler resampler(SAMPLE_RATE_11025, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    uint32_t inFrameLen = SAMPLE_RATE_11025 * FRAME_LEN_20MS / MS_PER_SECOND; // 220
    uint32_t outFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND; // 960
    std::vector<float> in(inFrameLen * STEREO);
    std::vector<float> out(outFrameLen * STEREO);
    int32_t ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);

    // test 11025Hz with 40ms input (441 frames) -- unified Process handles arbitrary sizes
    inFrameLen = SAMPLE_RATE_11025 * FRAME_LEN_40MS / MS_PER_SECOND; // 441
    std::vector<float> in40ms(inFrameLen * STEREO);
    outFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_40MS / MS_PER_SECOND; // 1920
    std::vector<float> out40ms(outFrameLen * STEREO);
    ret = resampler.Process(in40ms.data(), inFrameLen, out40ms.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
}

/*
 * @tc.name  : Test Process API.
 * @tc.type  : FUNC
 * @tc.number: ProcessTest_003.
 * @tc.desc  : Test Process with 10Hz-resolution non-standard rate (48010Hz).
 */
HWTEST_F(AudioProResamplerTest, ProcessTest_003, TestSize.Level0)
{
    // test custom sample rate that is not multiples of 50
    ProResampler resampler(SAMPLE_RATE_48010, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    uint32_t inFrameLen = SAMPLE_RATE_48010 * FRAME_LEN_20MS / MS_PER_SECOND; // 960
    uint32_t outFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND; // 960
    std::vector<float> in(inFrameLen * STEREO);
    std::vector<float> out(outFrameLen * STEREO);
    int32_t ret = resampler.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);

    // test with 100ms input -- unified Process handles arbitrary sizes
    inFrameLen = SAMPLE_RATE_48010 * FRAME_LEN_100MS / MS_PER_SECOND; // 4801
    outFrameLen = SAMPLE_RATE_48000 * FRAME_LEN_100MS / MS_PER_SECOND; // 4800
    std::vector<float> in100ms(inFrameLen * STEREO);
    std::vector<float> out100ms(outFrameLen * STEREO);
    ret = resampler.Process(in100ms.data(), inFrameLen, out100ms.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
}

/*
 * @tc.name  : Test Process API.
 * @tc.type  : FUNC
 * @tc.number: ProcessTest_004.
 * @tc.desc  : Test Process, when outFrameLen more than 20ms.
 */
HWTEST_F(AudioProResamplerTest, ProcessTest_004, TestSize.Level0)
{
    ProResampler resampler1(SAMPLE_RATE_8000, SAMPLE_RATE_16000, STEREO, QUALITY_ONE);
    uint32_t inFrameLen = SAMPLE_RATE_8000 * FRAME_LEN_60MS / MS_PER_SECOND;
    uint32_t outFrameLen = SAMPLE_RATE_16000 * FRAME_LEN_60MS / MS_PER_SECOND;
    std::vector<float> in(inFrameLen * STEREO);
    std::vector<float> out(outFrameLen * STEREO);

    EXPECT_EQ(resampler1.tmpOutBuf_.size(), resampler1.expectedOutFrameLen_ * resampler1.channels_);
    int32_t ret = resampler1.Process(in.data(), inFrameLen, out.data(), outFrameLen);
    EXPECT_EQ(ret, EOK);
    EXPECT_EQ(resampler1.tmpOutBuf_.size(), outFrameLen * resampler1.channels_);
}

/*
 * @tc.name  : Test UpdateChannels API.
 * @tc.type  : FUNC
 * @tc.number: UpdateChannelsTest_001.
 * @tc.desc  : Test UpdateChannels, from valid channel state to invalid state.
 */
HWTEST_F(AudioProResamplerTest, UpdateChannelsTest_001, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.channels_, STEREO);
    EXPECT_NE(resampler.state_, nullptr);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);

    resampler.UpdateChannels(INVALID_CHANNELS);
    EXPECT_EQ(resampler.channels_, INVALID_CHANNELS);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);
}

/*
 * @tc.name  : Test UpdateChannels API.
 * @tc.type  : FUNC
 * @tc.number: UpdateChannelsTest_002.
 * @tc.desc  : Test UpdateChannels, from invalid channel state to valid state.
 */
HWTEST_F(AudioProResamplerTest, UpdateChannelsTest_002, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, INVALID_CHANNELS, QUALITY_ONE);
    EXPECT_EQ(resampler.channels_, INVALID_CHANNELS);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);

    resampler.UpdateChannels(CHANNEL_6);
    EXPECT_EQ(resampler.channels_, CHANNEL_6);
    EXPECT_NE(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * CHANNEL_6);
}

/*
 * @tc.name  : Test UpdateChannels API.
 * @tc.type  : FUNC
 * @tc.number: UpdateChannelsTest_003.
 * @tc.desc  : Test UpdateChannels, from valid to valid or invalid to invalid.
 */
HWTEST_F(AudioProResamplerTest, UpdateChannelsTest_003, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.channels_, STEREO);
    EXPECT_NE(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);

    resampler.UpdateChannels(CHANNEL_6);
    EXPECT_EQ(resampler.channels_, CHANNEL_6);
    EXPECT_NE(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * CHANNEL_6);

    ProResampler resampler1(SAMPLE_RATE_48000, SAMPLE_RATE_96000, INVALID_CHANNELS, QUALITY_ONE);
    EXPECT_EQ(resampler1.channels_, INVALID_CHANNELS);
    EXPECT_EQ(resampler1.state_, nullptr);
    EXPECT_EQ(resampler1.tmpOutBuf_.size(), 0);

    resampler1.UpdateChannels(INVALID_CHANNELS2);
    EXPECT_EQ(resampler1.channels_, INVALID_CHANNELS2);
    EXPECT_EQ(resampler1.state_, nullptr);
    EXPECT_EQ(resampler1.tmpOutBuf_.size(), 0);
}

/*
 * @tc.name  : Test UpdateChannels API.
 * @tc.type  : FUNC
 * @tc.number: UpdateChannelsTest_004.
 * @tc.desc  : Test UpdateChannels, when customSampleRate is valid buffsize will update.
 */
HWTEST_F(AudioProResamplerTest, UpdateChannelsTest_004, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48010, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.channels_, STEREO);
    EXPECT_NE(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);

    resampler.UpdateChannels(CHANNEL_6);
    EXPECT_EQ(resampler.channels_, CHANNEL_6);
    EXPECT_NE(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * CHANNEL_6);
    // invalid channel, release memory
    resampler.UpdateChannels(INVALID_CHANNELS);
    EXPECT_EQ(resampler.channels_, INVALID_CHANNELS);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);
}

/*
 * @tc.name  : Test UpdateChannels API.
 * @tc.type  : FUNC
 * @tc.number: UpdateChannelsTest_005.
 * @tc.desc  : Test UpdateChannels, when customSampleRate is 11025Hz, buffsize will update.
 */
HWTEST_F(AudioProResamplerTest, UpdateChannelsTest_005, TestSize.Level0)
{
    ProResampler resampler1(SAMPLE_RATE_11025, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler1.channels_, STEREO);
    EXPECT_NE(resampler1.state_, nullptr);
    EXPECT_EQ(resampler1.tmpOutBuf_.size(), resampler1.expectedOutFrameLen_ * STEREO);

    resampler1.UpdateChannels(CHANNEL_6);
    EXPECT_EQ(resampler1.channels_, CHANNEL_6);
    EXPECT_NE(resampler1.state_, nullptr);
    EXPECT_EQ(resampler1.tmpOutBuf_.size(), resampler1.expectedOutFrameLen_ * CHANNEL_6);
    // invalid channel, release memory
    resampler1.UpdateChannels(INVALID_CHANNELS);
    EXPECT_EQ(resampler1.channels_, INVALID_CHANNELS);
    EXPECT_EQ(resampler1.state_, nullptr);
    EXPECT_EQ(resampler1.tmpOutBuf_.size(), 0);
}

/*
 * @tc.name  : Test UpdateChannels API.
 * @tc.type  : FUNC
 * @tc.number: UpdateChannelsTest_006.
 * @tc.desc  : Test UpdateChannels, when inRate = outRate.
 */
HWTEST_F(AudioProResamplerTest, UpdateChannelsTest_006, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);

    resampler.UpdateChannels(CHANNEL_6);
    EXPECT_EQ(resampler.channels_, CHANNEL_6);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);
}

/*
 * @tc.name  : Test UpdateRates API.
 * @tc.type  : FUNC
 * @tc.number: UpdateRatesTest001.
 * @tc.desc  : Test UpdateRates, to 11025 input.
 */
HWTEST_F(AudioProResamplerTest, UpdateRates_001, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_96000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);

    resampler.UpdateRates(SAMPLE_RATE_11025, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_11025);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_11025 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);
}

/*
 * @tc.name  : Test UpdateRates API.
 * @tc.type  : FUNC
 * @tc.number: UpdateRatesTest002.
 * @tc.desc  : Test UpdateRates, update inRate from valid to invalid.
 */
HWTEST_F(AudioProResamplerTest, UpdateRatesTest_002, TestSize.Level0)
{
    // initialize resampler to valid state
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_96000);
    EXPECT_NE(resampler.state_, nullptr);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);

    // update resampler to invalid state
    int32_t ret = resampler.UpdateRates(INVALID_SAMPLE_RATE, SAMPLE_RATE_48000);
    EXPECT_EQ(ret, RESAMPLER_ERR_INVALID_ARG);
    EXPECT_EQ(resampler.inRate_, INVALID_SAMPLE_RATE);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);
}

/*
 * @tc.name  : Test UpdateRates API.
 * @tc.type  : FUNC
 * @tc.number: UpdateRatesTest_003.
 * @tc.desc  : Test UpdateRates, inRate update from 48000 to 48010.
 */
HWTEST_F(AudioProResamplerTest, UpdateRatesTest_003, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_96000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);

    resampler.UpdateRates(SAMPLE_RATE_48010, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48010);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48010 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);
}

/*
 * @tc.name  : Test UpdateRates API.
 * @tc.type  : FUNC
 * @tc.number: UpdateRatesTest_004.
 * @tc.desc  : Test UpdateRates, update resampler from invalid state to valid state.
 */
HWTEST_F(AudioProResamplerTest, UpdateRatesTest_004, TestSize.Level0)
{
    // initialize resampler to invalid state
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_48000, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);

    resampler.UpdateRates(SAMPLE_RATE_48000, SAMPLE_RATE_96000);
    EXPECT_NE(resampler.state_, nullptr);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_96000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);
}

/*
 * @tc.name  : Test UpdateRates API.
 * @tc.type  : FUNC
 * @tc.number: UpdateRatesTest_005.
 * @tc.desc  : Test UpdateRates, set inRate is 48010, outRate update from 44100 to 48000
 */
HWTEST_F(AudioProResamplerTest, UpdateRatesTest_005, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48010, SAMPLE_RATE_44100, STEREO, QUALITY_ONE);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48010);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_44100);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48010 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_44100 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);

    resampler.UpdateRates(SAMPLE_RATE_48010, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.inRate_, SAMPLE_RATE_48010);
    EXPECT_EQ(resampler.outRate_, SAMPLE_RATE_48000);
    EXPECT_EQ(resampler.expectedInFrameLen_, SAMPLE_RATE_48010 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_48000 * FRAME_LEN_20MS / MS_PER_SECOND);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), resampler.expectedOutFrameLen_ * STEREO);
}

/*
 * @tc.name  : Test UpdateRates API.
 * @tc.type  : FUNC
 * @tc.number: UpdateRatesTest_006.
 * @tc.desc  : Test UpdateRates, when channel_ is invalid
 */
HWTEST_F(AudioProResamplerTest, UpdateRatesTest_006, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, INVALID_CHANNELS, QUALITY_ONE);
    EXPECT_EQ(resampler.channels_, INVALID_CHANNELS);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);

    resampler.UpdateRates(SAMPLE_RATE_48000, SAMPLE_RATE_44100);
    EXPECT_EQ(resampler.state_, nullptr);
    EXPECT_EQ(resampler.tmpOutBuf_.size(), 0);
    EXPECT_EQ(resampler.expectedOutFrameLen_, SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND);
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

/*
 * @tc.name  : Test Process API.
 * @tc.type  : FUNC
 * @tc.number: ProcessTest_NullInBuffer.
 * @tc.desc  : Test Process with null inBuffer returns RESAMPLER_ERR_INVALID_ARG.
 */
HWTEST_F(AudioProResamplerTest, ProcessTest_NullInBuffer, TestSize.Level0)
{
    ProResampler resampler(SAMPLE_RATE_48000, SAMPLE_RATE_96000, STEREO, QUALITY_ONE);
    uint32_t outFrameLen = SAMPLE_RATE_96000 * FRAME_LEN_20MS / MS_PER_SECOND;
    std::vector<float> out(outFrameLen * STEREO);

    int32_t ret = resampler.Process(nullptr, 960, out.data(), outFrameLen);
    EXPECT_EQ(ret, RESAMPLER_ERR_INVALID_ARG);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
