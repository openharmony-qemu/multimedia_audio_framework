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
#include <vector>
#include <cinttypes>
#include "audio_engine_log.h"
#include "channel_converter.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t TEST_BUFFER_LEN = 10;
constexpr bool MIX_FLE = true;
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
    FRONT_LEFT | FRONT_RIGHT | TOP_BACK_CENTER | TOP_BACK_RIGHT |
    STEREO_LEFT | STEREO_RIGHT |
    WIDE_LEFT | WIDE_RIGHT |
    SURROUND_DIRECT_LEFT | SURROUND_DIRECT_RIGHT | LOW_FREQUENCY_2 |
    TOP_SIDE_LEFT | TOP_SIDE_RIGHT |
    BOTTOM_FRONT_CENTER | BOTTOM_FRONT_LEFT | BOTTOM_FRONT_RIGHT
);

// define channelLayout set to cover all channels as input
const static std::set<AudioChannelLayout> FULL_CH_LAYOUT_SET = {
    FIRST_PART_CH_LAYOUTS,
    SECOND_PART_CH_LAYOUTS
};

const static std::set<AudioChannelLayout> GENERAL_INPUT_CH_LAYOUT_SET = {
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
const static uint32_t NUM_11 = 11;
const static uint32_t NUM_13 = 13;
const static uint32_t NUM_15 = 15;
const static std::set<uint32_t> INVALID_CHANNELS = {
    NUM_11, NUM_13, NUM_15,
};
class ChannelConverterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void ChannelConverterTest::SetUp() {}

void ChannelConverterTest::TearDown() {}

/**
 * @tc.name : Test SetParam API
 * @tc.type : FUNC
 * @tc.number : SetParam
 * @tc.desc : Test SetParam interface with normal input and output channelLayout
*/
HWTEST_F(ChannelConverterTest, ChannelConverterTestSetParam_001, TestSize.Level0)
{
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    ChannelConverter converter;
    // valid param, predefined downmix rules
    for (AudioChannelLayout inLayout: GENERAL_INPUT_CH_LAYOUT_SET) {
        inChannelInfo.numChannels = BitCounts(inLayout);
        inChannelInfo.channelLayout = inLayout;
        for (AudioChannelLayout outLayout: FULL_CH_LAYOUT_SET) {
            outChannelInfo.channelLayout = outLayout;
            outChannelInfo.numChannels = MAX_CHANNELS;
            int32_t ret = converter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE);
            AUDIO_INFO_LOG("SetParamRetSuccessAndSetupDownMixTable inLayout %{public}" PRIu64 ""
                "outLayout: %{public}" PRIu64 "", inLayout, outLayout);
            EXPECT_EQ(ret, MIX_ERR_SUCCESS);
        }
    }

    // test setup mono input or output
    AudioChannelInfo monoChannelInfo = {CH_LAYOUT_MONO, 1};
    int32_t ret = converter.SetParam(monoChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    ret = converter.SetParam(inChannelInfo, monoChannelInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
}

/**
 * @tc.name : Test SetParam API
 * @tc.type : FUNC
 * @tc.number : SetParam
 * @tc.desc : Test SetParam interface with HOA input and output
*/
HWTEST_F(ChannelConverterTest, ChannelConverterTestSetParam_002, TestSize.Level0)
{
    AudioChannelInfo inChannelInfo = {CH_LAYOUT_HOA_ORDER1_ACN_N3D, BitCounts(CH_LAYOUT_HOA_ORDER1_ACN_N3D)};
    AudioChannelInfo outChannelInfo = {CH_LAYOUT_9POINT1POINT4, BitCounts(CH_LAYOUT_9POINT1POINT4)};
    ChannelConverter converter;
    int32_t ret = converter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    inChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    inChannelInfo.numChannels = STEREO;
    outChannelInfo.channelLayout = CH_LAYOUT_HOA_ORDER2_ACN_N3D;
    outChannelInfo.numChannels = BitCounts(CH_LAYOUT_HOA_ORDER2_ACN_N3D);
    ret = converter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_INVALID_ARG);
}

/**
 * @tc.name : Test SetParam API
 * @tc.type : FUNC
 * @tc.number : SetParam
 * @tc.desc : Test SetParam interface with invalid channel number, channel number of 11, 13, 15 are not supported
*/
HWTEST_F(ChannelConverterTest, ChannelConverterTestSetParam_003, TestSize.Level0)
{
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo = {CH_LAYOUT_9POINT1POINT4, BitCounts(CH_LAYOUT_9POINT1POINT4)};
    ChannelConverter converter;
    for (uint32_t numChannels: INVALID_CHANNELS) {
        inChannelInfo.channelLayout = CH_LAYOUT_UNKNOWN;
        inChannelInfo.numChannels = numChannels;
        int32_t ret = converter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE);
        EXPECT_EQ(ret, MIX_ERR_INVALID_ARG);
        EXPECT_EQ(converter.GetInChannelInfo().channelLayout, inChannelInfo.channelLayout);
        EXPECT_EQ(converter.GetInChannelInfo().numChannels, inChannelInfo.numChannels);
        EXPECT_EQ(converter.GetOutChannelInfo().channelLayout, outChannelInfo.channelLayout);
        EXPECT_EQ(converter.GetOutChannelInfo().numChannels, outChannelInfo.numChannels);
        EXPECT_EQ(converter.isInitialized_, false);
    }
}

HWTEST_F(ChannelConverterTest, ChannelConverterProcessTest_001, TestSize.Level0)
{
    // test upmix
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = MONO;
    inChannelInfo.channelLayout = CH_LAYOUT_MONO;
    outChannelInfo.numChannels = STEREO;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    ChannelConverter channelConverter;
    std::vector<float> in(TEST_BUFFER_LEN * MONO, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * STEREO, 0.0f);
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);
    EXPECT_EQ(channelConverter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), MIX_ERR_SUCCESS);
    
    // test downmix
    inChannelInfo.numChannels = CHANNEL_6;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    in.resize(TEST_BUFFER_LEN * CHANNEL_6, 0.0f);
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);
    EXPECT_EQ(channelConverter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), MIX_ERR_SUCCESS);
}

HWTEST_F(ChannelConverterTest, ChannelConverterProcessTest_002, TestSize.Level0)
{
    // test process when channelConverter is invalid
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = NUM_11;
    inChannelInfo.channelLayout = CH_LAYOUT_UNKNOWN;
    outChannelInfo.numChannels = NUM_13;
    outChannelInfo.channelLayout = CH_LAYOUT_UNKNOWN;
    ChannelConverter channelConverter;
    std::vector<float> in(TEST_BUFFER_LEN * NUM_11, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * NUM_13, 0.0f);
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_INVALID_ARG);
    EXPECT_EQ(channelConverter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), MIX_ERR_ALLOC_FAILED);
}

HWTEST_F(ChannelConverterTest, ChannelConverterNormalizationTest_001, TestSize.Level0)
{
    AudioChannelInfo inChannelInfo;
    AudioChannelInfo outChannelInfo;
    inChannelInfo.numChannels = CHANNEL_6;
    inChannelInfo.channelLayout = CH_LAYOUT_5POINT1;
    outChannelInfo.numChannels = CHANNEL_8;
    outChannelInfo.channelLayout = CH_LAYOUT_5POINT1POINT2;
    ChannelConverter channelConverter;
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);
    EXPECT_EQ(channelConverter.downMixer_.normalizing_, true);

    channelConverter.SetDownmixNormalization(false);
    // setting is stored in channelConverter
    EXPECT_EQ(channelConverter.downmixNormalizing_, false);
    // for upmix, do not change downmix normalizaiton state
    EXPECT_EQ(channelConverter.downMixer_.normalizing_, true);
    // for downmix default normalization state is true and can be set to false
    outChannelInfo.numChannels = STEREO;
    outChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    EXPECT_EQ(channelConverter.SetParam(inChannelInfo, outChannelInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);
    // set to downmix state, normalization setting passed to downmixer
    EXPECT_EQ(channelConverter.downmixNormalizing_, false);
    EXPECT_EQ(channelConverter.downMixer_.normalizing_, false);
    channelConverter.SetDownmixNormalization(true);
    EXPECT_EQ(channelConverter.downmixNormalizing_, true);
    EXPECT_EQ(channelConverter.downMixer_.normalizing_, true);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS