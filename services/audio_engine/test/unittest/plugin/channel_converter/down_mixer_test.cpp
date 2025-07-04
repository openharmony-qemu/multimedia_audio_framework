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
#include <cinttypes>
#include <map>
#include <set>
#include <vector>
#include "audio_engine_log.h"
#include "down_mixer.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

// need full audio channel layouts to cover all cases during setting up downmix table -- first part
constexpr static AudioChannelLayout FIRST_PART_CH_LAYOUTS = static_cast<AudioChannelLayout> (
    FRONT_LEFT | FRONT_RIGHT | FRONT_CENTER | LOW_FREQUENCY |
    BACK_LEFT | BACK_RIGHT |
    FRONT_LEFT_OF_CENTER | FRONT_RIGHT_OF_CENTER |
    BACK_CENTER | SIDE_LEFT | SIDE_RIGHT |
    TOP_CENTER | TOP_FRONT_LEFT | TOP_FRONT_CENTER | TOP_FRONT_RIGHT | TOP_BACK_LEFT
);

// need full audio channel layouts to cover all cases during setting up downmix table -- second part
constexpr static AudioChannelLayout SECOND_PART_CH_LAYOUTS = static_cast<AudioChannelLayout> (
    TOP_CENTER | TOP_BACK_LEFT | TOP_BACK_CENTER | TOP_BACK_RIGHT |
    STEREO_LEFT | STEREO_RIGHT |
    WIDE_LEFT | WIDE_RIGHT |
    SURROUND_DIRECT_LEFT | SURROUND_DIRECT_RIGHT | LOW_FREQUENCY_2 |
    TOP_SIDE_LEFT | TOP_SIDE_RIGHT |
    BOTTOM_FRONT_CENTER | BOTTOM_FRONT_LEFT | BOTTOM_FRONT_RIGHT
);
// for test predefined downmix rules
const static std::set<AudioChannelLayout> OUTPUT_CH_LAYOUT_SET = {
    CH_LAYOUT_STEREO,
    CH_LAYOUT_5POINT1,
    CH_LAYOUT_5POINT1POINT2,
    CH_LAYOUT_5POINT1POINT4,
    CH_LAYOUT_7POINT1,
    CH_LAYOUT_7POINT1POINT2,
    CH_LAYOUT_7POINT1POINT4
};

const static std::set<AudioChannelLayout> GENERAL_OUTPUT_CH_LAYOUT_SET = {
    CH_LAYOUT_SURROUND,
    CH_LAYOUT_3POINT1,
    CH_LAYOUT_4POINT0,
    CH_LAYOUT_QUAD_SIDE,
    CH_LAYOUT_QUAD,
    CH_LAYOUT_4POINT1,
    CH_LAYOUT_5POINT0,
    CH_LAYOUT_5POINT0_BACK,
    CH_LAYOUT_2POINT1POINT2,
    CH_LAYOUT_3POINT0POINT2,
    CH_LAYOUT_5POINT1_BACK,
    CH_LAYOUT_6POINT0,
    CH_LAYOUT_HEXAGONAL,
    CH_LAYOUT_3POINT1POINT2,
    CH_LAYOUT_6POINT0_FRONT,
    CH_LAYOUT_6POINT1,
    CH_LAYOUT_6POINT1_BACK,
    CH_LAYOUT_7POINT0,
    CH_LAYOUT_OCTAGONAL,
    CH_LAYOUT_7POINT1_WIDE_BACK,
    CH_LAYOUT_7POINT1_WIDE,
    CH_LAYOUT_10POINT2,
    CH_LAYOUT_9POINT1POINT4,
};

// define channelLayout set to cover all channels as input
const static std::set<AudioChannelLayout> FULL_CH_LAYOUT_SET = {
    FIRST_PART_CH_LAYOUTS,
    SECOND_PART_CH_LAYOUTS
};

const static std::map<AudioChannel, AudioChannelLayout> DOWNMIX_CHANNEL_COUNT_MAP = {
    {MONO, CH_LAYOUT_MONO},
    {STEREO, CH_LAYOUT_STEREO},
    {CHANNEL_3, CH_LAYOUT_SURROUND},
    {CHANNEL_4, CH_LAYOUT_3POINT1},
    {CHANNEL_5, CH_LAYOUT_4POINT1},
    {CHANNEL_6, CH_LAYOUT_5POINT1},
    {CHANNEL_7, CH_LAYOUT_6POINT1},
    {CHANNEL_8, CH_LAYOUT_5POINT1POINT2},
    {CHANNEL_9, CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {CHANNEL_10, CH_LAYOUT_7POINT1POINT2},
    {CHANNEL_12, CH_LAYOUT_7POINT1POINT4},
    {CHANNEL_13, CH_LAYOUT_UNKNOWN},
    {CHANNEL_14, CH_LAYOUT_9POINT1POINT4},
    {CHANNEL_16, CH_LAYOUT_9POINT1POINT6}
};

constexpr uint32_t TEST_FORMAT_SIZE = 4;
constexpr uint32_t TEST_FRAME_LEN = 100;
constexpr uint32_t TEST_BUFFER_LEN = 10;
constexpr bool MIX_FLE = true;

static uint32_t BitCounts(uint64_t bits)
{
    uint32_t num = 0;
    for (; bits != 0; bits &= bits - 1) {
        num++;
    }
    return num;
}

class DownMixerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void DownMixerTest::SetUp() {}

void DownMixerTest::TearDown() {}

/**
 * @tc.name : Test SetParam API
 * @tc.type : FUNC
 * @tc.number : SetParam
 * @tc.desc : Test SetParam interface
*/
HWTEST_F(DownMixerTest, SetParamTest, TestSize.Level0)
{
    // invalid input Param
    DownMixer downMixer;
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = MAX_CHANNELS + 1;
    inChannelInfo.channelLayout = CH_LAYOUT_UNKNOWN;
    outChannelInfo.numChannels = MAX_CHANNELS + 1;
    outChannelInfo.channelLayout = CH_LAYOUT_UNKNOWN;
    int32_t ret = downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
    EXPECT_EQ(ret, DMIX_ERR_INVALID_ARG);
    
    // valid param, predefined downmix rules
    for (AudioChannelLayout outLayout: OUTPUT_CH_LAYOUT_SET) {
        outChannelInfo.numChannels = BitCounts(outLayout);
        outChannelInfo.channelLayout = outLayout;
        for (AudioChannelLayout inLayout: FULL_CH_LAYOUT_SET) {
            inChannelInfo.channelLayout = inLayout;
            inChannelInfo.numChannels = MAX_CHANNELS;
            int32_t ret = downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
            AUDIO_INFO_LOG("SetParamRetSuccessAndSetupDownMixTable inLayout %{public}" PRIu64 ""
                "outLayout: %{public}" PRIu64 "", inLayout, outLayout);
            EXPECT_EQ(ret, DMIX_ERR_SUCCESS);
        }
    }
    
    // valid param, general downmix table rule
    for (AudioChannelLayout outLayout: GENERAL_OUTPUT_CH_LAYOUT_SET) {
        outChannelInfo.numChannels = BitCounts(outLayout);
        outChannelInfo.channelLayout = outLayout;
        for (AudioChannelLayout inLayout: FULL_CH_LAYOUT_SET) {
            inChannelInfo.channelLayout = inLayout;
            inChannelInfo.numChannels = MAX_CHANNELS;
            int32_t ret = downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE);
            AUDIO_INFO_LOG("SetParamRetSuccessAndSetupDownMixTable inLayout %{public}" PRIu64 ""
                "outLayout: %{public}" PRIu64 "", inLayout, outLayout);
            EXPECT_EQ(ret, DMIX_ERR_SUCCESS);
        }
    }
}

/**
 * @tc.name : Test SetDefaultChannelLayout API
 * @tc.type : FUNC
 * @tc.number : SetParam
 * @tc.desc : Test SetDefaultChannelLayout interface
*/
HWTEST_F(DownMixerTest, SetDefaultChannelLayoutTest, TestSize.Level0)
{
    DownMixer downMixer;
    for (auto pair : DOWNMIX_CHANNEL_COUNT_MAP) {
        AUDIO_INFO_LOG("fist: %{public}d, second %{public}" PRIu64 ".", pair.first, pair.second);
        EXPECT_EQ(pair.second, downMixer.SetDefaultChannelLayout(pair.first));
    }
}


/**
 * @tc.name : Test CheckIsHOA API
 * @tc.type : FUNC
 * @tc.number : SetParam
 * @tc.desc : Test CheckIsHOA interface
*/
HWTEST_F(DownMixerTest, CheckIsHOATest, TestSize.Level0)
{
    DownMixer downMixer;
    EXPECT_EQ(true, downMixer.CheckIsHOA(CH_LAYOUT_HOA_ORDER2_ACN_SN3D));
    EXPECT_EQ(false, downMixer.CheckIsHOA(CH_LAYOUT_UNKNOWN));
}

/**
 * @tc.name  : Test Process API
 * @tc.type  : FUNC
 * @tc.number: Process
 * @tc.desc  : Test Process interface.
 */
HWTEST_F(DownMixerTest, ProcesTest, TestSize.Level0)
{
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    inChannelInfo.numChannels = CHANNEL_6;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    outChannelInfo.numChannels = STEREO;

    // test uninitialized
    DownMixer downMixer;
    std::vector<float> in(TEST_BUFFER_LEN * CHANNEL_6, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * STEREO, 0.0f);
    uint32_t testInBufferSize = in.size() * TEST_FORMAT_SIZE;
    uint32_t testOutBufferSize = out.size() * TEST_FORMAT_SIZE;
    EXPECT_EQ(downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize),
        DMIX_ERR_ALLOC_FAILED);
    
    // test input and output buffer length smaller than expected
    EXPECT_EQ(downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE), DMIX_ERR_SUCCESS);
    EXPECT_EQ(downMixer.Process(TEST_FRAME_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize),
        DMIX_ERR_ALLOC_FAILED);

    // test process usual channel layout
    EXPECT_EQ(downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize),
        DMIX_ERR_SUCCESS);

    // test process HOA
    inChannelInfo.channelLayout = CH_LAYOUT_HOA_ORDER2_ACN_SN3D;
    inChannelInfo.numChannels = CHANNEL_9;
    in.resize(CHANNEL_9 * TEST_BUFFER_LEN, 0.0f);
    testInBufferSize = in.size() * TEST_FORMAT_SIZE;
    EXPECT_EQ(downMixer.SetParam(inChannelInfo, outChannelInfo, TEST_FORMAT_SIZE, MIX_FLE), DMIX_ERR_SUCCESS);
    EXPECT_EQ(downMixer.Process(TEST_BUFFER_LEN, in.data(), testInBufferSize, out.data(), testOutBufferSize),
        DMIX_ERR_SUCCESS);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
