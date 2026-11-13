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

HWTEST_F(ChannelConverterTest, ChannelConverter_StereoDownmix_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO_DOWNMIX, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_MONO, 1};

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);

    std::vector<float> in(TEST_BUFFER_LEN * 2, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * 1, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
}

HWTEST_F(ChannelConverterTest, ChannelConverter_StereoDownmix_002, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_5POINT1, 6};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO_DOWNMIX, 2};

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);

    std::vector<float> in(TEST_BUFFER_LEN * 6, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * 2, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
}

HWTEST_F(ChannelConverterTest, ChannelConverter_StereoDownmix_003, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO_DOWNMIX, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);

    std::vector<float> in(TEST_BUFFER_LEN * 2, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * 2, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
}

HWTEST_F(ChannelConverterTest, ChannelConverter_StereoDownmix_004, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO_DOWNMIX, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_5POINT1, 6};

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);

    std::vector<float> in(TEST_BUFFER_LEN * 2, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * 6, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
}

HWTEST_F(ChannelConverterTest, ChannelConverter_StereoDownmix_005, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_7POINT1, 8};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO_DOWNMIX, 2};

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);

    std::vector<float> in(TEST_BUFFER_LEN * 8, 0.0f);
    std::vector<float> out(TEST_BUFFER_LEN * 2, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);
}

HWTEST_F(ChannelConverterTest, ChannelConverter_UpmixGain_Default_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_6POINT0, 6};

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    std::vector<float> in(2 * TEST_BUFFER_LEN, 1.0f);
    std::vector<float> out(6 * TEST_BUFFER_LEN, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    bool foundClipping = false;
    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        if (out[i] > 1.01f) {
            foundClipping = true;
            break;
        }
    }
    EXPECT_TRUE(foundClipping);
}

HWTEST_F(ChannelConverterTest, ChannelConverter_UpmixGain_Attenuated_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_6POINT0, 6};

    converter.SetUpmixCoef(0.5f);

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    std::vector<float> in(2 * TEST_BUFFER_LEN, 1.0f);
    std::vector<float> out(6 * TEST_BUFFER_LEN, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        EXPECT_LE(out[i], 1.51f);
    }
}

HWTEST_F(ChannelConverterTest, ChannelConverter_UpmixGain_Switch_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_6POINT0, 6};

    converter.SetUpmixCoef(0.5f);
    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    std::vector<float> in(2 * TEST_BUFFER_LEN, 1.0f);
    std::vector<float> out(6 * TEST_BUFFER_LEN, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        EXPECT_LE(out[i], 1.51f);
    }

    converter.SetUpmixCoef(1.0f);
    ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    std::fill(out.begin(), out.end(), 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    bool foundClipping = false;
    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        if (out[i] > 1.01f) {
            foundClipping = true;
            break;
        }
    }
    EXPECT_TRUE(foundClipping);
}

HWTEST_F(ChannelConverterTest, ChannelConverter_UpmixGain_5Point1_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_5POINT1, 6};

    converter.SetUpmixCoef(0.5f);

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    std::vector<float> in(2 * TEST_BUFFER_LEN, 0.8f);
    std::vector<float> out(6 * TEST_BUFFER_LEN, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        EXPECT_LE(out[i], 1.21f);
    }
}

HWTEST_F(ChannelConverterTest, ChannelConverter_MonoUpmixNoCoef_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_MONO, 1};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    converter.SetUpmixCoef(0.5f);

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    std::vector<float> in(TEST_BUFFER_LEN, 1.0f);
    std::vector<float> out(2 * TEST_BUFFER_LEN, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 2 * TEST_BUFFER_LEN; i++) {
        EXPECT_FLOAT_EQ(out[i], 1.0f);
    }
}

HWTEST_F(ChannelConverterTest, ChannelConverter_MonoUpmixNoCoef_002, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_MONO, 1};
    AudioChannelInfo outInfo = {CH_LAYOUT_5POINT1, 6};

    converter.SetUpmixCoef(0.3f);

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    std::vector<float> in(TEST_BUFFER_LEN, 0.8f);
    std::vector<float> out(6 * TEST_BUFFER_LEN, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        EXPECT_FLOAT_EQ(out[i], 0.8f);
    }
}

HWTEST_F(ChannelConverterTest, ChannelConverter_MonoUpmixNoCoef_003, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_MONO, 1};
    AudioChannelInfo outInfo = {CH_LAYOUT_7POINT1, 8};

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    std::vector<float> in(TEST_BUFFER_LEN, 0.5f);
    std::vector<float> out(8 * TEST_BUFFER_LEN, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 8 * TEST_BUFFER_LEN; i++) {
        EXPECT_FLOAT_EQ(out[i], 0.5f);
    }
}

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

/**
 * @tc.name  : SetParam_FormatSize_Indirect_Test_001
 * @tc.desc  :  GetFormatSize
 */
HWTEST_F(ChannelConverterTest, SetParam_FormatSize_Indirect_Test_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo in = { CH_LAYOUT_STEREO, 2 };
    AudioChannelInfo out = { CH_LAYOUT_STEREO, 2 };

    EXPECT_EQ(converter.SetParam(in, out, SAMPLE_S16LE, false), MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.SetParam(in, out, SAMPLE_F32LE, false), MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.SetParam(in, out, SAMPLE_S24LE, false), MIX_ERR_SUCCESS);
}


/**
 * @tc.name  : ChannelConverter_Reset_001
 * @tc.type  : FUNC
 * @tc.desc  : Reset
 */
HWTEST_F(ChannelConverterTest, ChannelConverter_Reset_001, TestSize.Level1)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_MONO, 1};

    converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, true);
    ASSERT_EQ(converter.isInitialized_, true);

    converter.Reset();
    EXPECT_EQ(converter.isInitialized_, false);
    std::vector<float> in(10, 1.0f);
    std::vector<float> out(10, 0.0f);
    int32_t ret = converter.Process(5, in.data(), in.size()*4, out.data(), out.size()*4);
    EXPECT_NE(ret, MIX_ERR_SUCCESS);
}

// SetParam 之后设置系数也应立即生效(等价于 SetParam 前设 0.5 的 UpmixGain_Attenuated_001)
HWTEST_F(ChannelConverterTest, ChannelConverter_UpmixGain_AfterSetParam_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_6POINT0, 6};

    int32_t ret = converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE);
    ASSERT_EQ(ret, MIX_ERR_SUCCESS);

    converter.SetUpmixCoef(0.5f); // 改造前不生效 -> 输出 clipping(>1.51)使本断言失败

    std::vector<float> in(2 * TEST_BUFFER_LEN, 1.0f);
    std::vector<float> out(6 * TEST_BUFFER_LEN, 0.0f);
    ret = converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
                            out.data(), out.size() * sizeof(float));
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        EXPECT_LE(out[i], 1.51f); // 与 UpmixGain_Attenuated_001 同一边界
    }
}

// 事后切换系数(不重新 SetParam)也能即时生效:0.5 衰减 -> 1.0 恢复 clipping
HWTEST_F(ChannelConverterTest, ChannelConverter_UpmixGain_AfterSetParam_Switch_001, TestSize.Level0)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_6POINT0, 6};
    ASSERT_EQ(converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);

    std::vector<float> in(2 * TEST_BUFFER_LEN, 1.0f);

    converter.SetUpmixCoef(0.5f);
    std::vector<float> outAtt(6 * TEST_BUFFER_LEN, 0.0f);
    ASSERT_EQ(converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
        outAtt.data(), outAtt.size() * sizeof(float)), MIX_ERR_SUCCESS);

    converter.SetUpmixCoef(1.0f); // 事后切回 1.0,应恢复 clipping(改造前不生效,保持衰减)
    std::vector<float> outFull(6 * TEST_BUFFER_LEN, 0.0f);
    ASSERT_EQ(converter.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
        outFull.data(), outFull.size() * sizeof(float)), MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        EXPECT_LE(outAtt[i], 1.51f);
    }
    bool foundClipping = false;
    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        if (outFull[i] > 1.01f) {
            foundClipping = true;
            break;
        }
    }
    EXPECT_TRUE(foundClipping);
}

// 幂等:重复设置相同系数不叠加;与单次设置结果逐样本相等
HWTEST_F(ChannelConverterTest, ChannelConverter_UpmixGain_Idempotent_001, TestSize.Level0)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_6POINT0, 6};
    std::vector<float> in(2 * TEST_BUFFER_LEN, 1.0f);

    ChannelConverter once;
    ASSERT_EQ(once.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);
    once.SetUpmixCoef(0.5f);
    std::vector<float> outOnce(6 * TEST_BUFFER_LEN, 0.0f);
    ASSERT_EQ(once.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
        outOnce.data(), outOnce.size() * sizeof(float)), MIX_ERR_SUCCESS);

    ChannelConverter repeat;
    ASSERT_EQ(repeat.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);
    repeat.SetUpmixCoef(0.5f);
    repeat.SetUpmixCoef(0.5f); // 重复设置,改造若用"再乘一次"会叠加 -> 与 once 不等 -> 失败
    std::vector<float> outRepeat(6 * TEST_BUFFER_LEN, 0.0f);
    ASSERT_EQ(repeat.Process(TEST_BUFFER_LEN, in.data(), in.size() * sizeof(float),
        outRepeat.data(), outRepeat.size() * sizeof(float)), MIX_ERR_SUCCESS);

    for (uint32_t i = 0; i < 6 * TEST_BUFFER_LEN; i++) {
        EXPECT_FLOAT_EQ(outRepeat[i], outOnce[i]);
    }
}

/*
 * @tc.name  : ChannelConverter_GetMixTable_001
 * @tc.type  : FUNC
 * @tc.number: ChannelConverter_GetMixTable_001
 * @tc.desc  : Test GetMixTable leaves the caller table untouched when SetParam was never called.
 */
HWTEST_F(ChannelConverterTest, ChannelConverter_GetMixTable_001, TestSize.Level1)
{
    ChannelConverter converter;
    float table[MAX_CHANNELS][MAX_CHANNELS];
    std::fill(&table[0][0], &table[0][0] + MAX_CHANNELS * MAX_CHANNELS, -1.0f);

    converter.GetMixTable(table); // isInitialized_ == false: guard returns without writing
    for (uint32_t i = 0; i < MAX_CHANNELS; i++) {
        for (uint32_t j = 0; j < MAX_CHANNELS; j++) {
            EXPECT_EQ(table[i][j], -1.0f);
        }
    }
}

/*
 * @tc.name  : ChannelConverter_GetMixTable_002
 * @tc.type  : FUNC
 * @tc.number: ChannelConverter_GetMixTable_002
 * @tc.desc  : Test GetMixTable on the downmix path: coefficients land in [0, 1], the inactive
 *             region stays zero, and the exposed table matches what Process applies
 *             (impulse on input ch 0 reproduces column table[*][0]).
 */
HWTEST_F(ChannelConverterTest, ChannelConverter_GetMixTable_002, TestSize.Level1)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_5POINT1, 6};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    ASSERT_EQ(converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);

    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0}};
    converter.GetMixTable(table);

    // downmix convention is table[out][in]: active region is [0..outCh)[0..inCh)
    for (uint32_t i = 0; i < MAX_CHANNELS; i++) {
        for (uint32_t j = 0; j < MAX_CHANNELS; j++) {
            if (i < outInfo.numChannels && j < inInfo.numChannels) {
                EXPECT_GE(table[i][j], 0.0f);
                EXPECT_LE(table[i][j], 1.0f);
            } else {
                EXPECT_EQ(table[i][j], 0.0f); // inactive region untouched
            }
        }
    }

    // impulse on input channel 0 (FL): out[i] == table[i][0] as MixProcess applies it
    std::vector<float> in(inInfo.numChannels, 0.0f);
    in[0] = 1.0f;
    std::vector<float> out(outInfo.numChannels, 0.0f);
    ASSERT_EQ(converter.Process(1, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), MIX_ERR_SUCCESS);
    for (uint32_t i = 0; i < outInfo.numChannels; i++) {
        EXPECT_FLOAT_EQ(out[i], table[i][0]);
    }
}

/*
 * @tc.name  : ChannelConverter_GetMixTable_003
 * @tc.type  : FUNC
 * @tc.number: ChannelConverter_GetMixTable_003
 * @tc.desc  : Test GetMixTable on the upmix path: upmix stores the table transposed
 *             (table[in][out], read back via table[j][i] in MixProcess); an impulse on
 *             input ch 0 must reproduce row table[0][*] through Process.
 */
HWTEST_F(ChannelConverterTest, ChannelConverter_GetMixTable_003, TestSize.Level1)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_6POINT0, 6};
    ASSERT_EQ(converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);

    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0}};
    converter.GetMixTable(table);

    // upmix convention is table[in][out]: active region is [0..inCh)[0..outCh)
    for (uint32_t i = 0; i < MAX_CHANNELS; i++) {
        for (uint32_t j = 0; j < MAX_CHANNELS; j++) {
            if (i < inInfo.numChannels && j < outInfo.numChannels) {
                EXPECT_GE(table[i][j], 0.0f); // upmix cross coefficients may exceed 1.0
            } else {
                EXPECT_EQ(table[i][j], 0.0f); // inactive region untouched
            }
        }
    }

    // impulse on input channel 0 (L): upmix reads the table transposed, out[i] == table[0][i]
    std::vector<float> in(inInfo.numChannels, 0.0f);
    in[0] = 1.0f;
    std::vector<float> out(outInfo.numChannels, 0.0f);
    ASSERT_EQ(converter.Process(1, in.data(), in.size() * sizeof(float), out.data(),
        out.size() * sizeof(float)), MIX_ERR_SUCCESS);
    for (uint32_t i = 0; i < outInfo.numChannels; i++) {
        EXPECT_FLOAT_EQ(out[i], table[0][i]);
    }
}

/*
 * @tc.name  : ChannelConverter_GetMixTable_004
 * @tc.type  : FUNC
 * @tc.number: ChannelConverter_GetMixTable_004
 * @tc.desc  : Test GetMixTable guard comes back after Reset: the caller table is left untouched.
 */
HWTEST_F(ChannelConverterTest, ChannelConverter_GetMixTable_004, TestSize.Level1)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_5POINT1, 6};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    ASSERT_EQ(converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);
    ASSERT_EQ(converter.isInitialized_, true);

    converter.Reset();
    EXPECT_EQ(converter.isInitialized_, false);

    float table[MAX_CHANNELS][MAX_CHANNELS];
    std::fill(&table[0][0], &table[0][0] + MAX_CHANNELS * MAX_CHANNELS, -1.0f);
    converter.GetMixTable(table);
    for (uint32_t i = 0; i < MAX_CHANNELS; i++) {
        for (uint32_t j = 0; j < MAX_CHANNELS; j++) {
            EXPECT_EQ(table[i][j], -1.0f);
        }
    }
}

/*
 * @tc.name  : ChannelConverter_SetInChannelInfo_001
 * @tc.type  : FUNC
 * @tc.number: ChannelConverter_SetInChannelInfo_001
 * @tc.desc  : Test SetInChannelInfo re-bakes the mix table on the changed side only:
 *             downmix stays / switches to upmix when in channels drop, output side untouched.
 */
HWTEST_F(ChannelConverterTest, ChannelConverter_SetInChannelInfo_001, TestSize.Level1)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_5POINT1, 6};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    ASSERT_EQ(converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);

    // 3ch in > 2ch out: stays on the downmix path
    AudioChannelInfo surroundInfo = {CH_LAYOUT_SURROUND, BitCounts(CH_LAYOUT_SURROUND)};
    EXPECT_EQ(converter.SetInChannelInfo(surroundInfo), MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);
    EXPECT_EQ(converter.GetInChannelInfo().channelLayout, CH_LAYOUT_SURROUND);
    EXPECT_EQ(converter.GetInChannelInfo().numChannels, BitCounts(CH_LAYOUT_SURROUND));
    EXPECT_EQ(converter.GetOutChannelInfo().channelLayout, CH_LAYOUT_STEREO);
    EXPECT_EQ(converter.GetOutChannelInfo().numChannels, STEREO);

    // 1ch in <= 2ch out: switches to the upmix path, still initialized
    AudioChannelInfo monoInfo = {CH_LAYOUT_MONO, MONO};
    EXPECT_EQ(converter.SetInChannelInfo(monoInfo), MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);
    EXPECT_EQ(converter.GetInChannelInfo().channelLayout, CH_LAYOUT_MONO);
    EXPECT_EQ(converter.GetInChannelInfo().numChannels, MONO);
}

/*
 * @tc.name  : ChannelConverter_SetOutChannelInfo_001
 * @tc.type  : FUNC
 * @tc.number: ChannelConverter_SetOutChannelInfo_001
 * @tc.desc  : Test SetOutChannelInfo re-bakes the mix table on the changed side only:
 *             upmix stays / switches to downmix when out channels drop; HOA output is rejected.
 */
HWTEST_F(ChannelConverterTest, ChannelConverter_SetOutChannelInfo_001, TestSize.Level1)
{
    ChannelConverter converter;
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_5POINT1, 6};
    ASSERT_EQ(converter.SetParam(inInfo, outInfo, SAMPLE_F32LE, MIX_FLE), MIX_ERR_SUCCESS);

    // 2ch in < 4ch out: stays on the upmix path
    AudioChannelInfo quadInfo = {CH_LAYOUT_QUAD, BitCounts(CH_LAYOUT_QUAD)};
    EXPECT_EQ(converter.SetOutChannelInfo(quadInfo), MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);
    EXPECT_EQ(converter.GetOutChannelInfo().channelLayout, CH_LAYOUT_QUAD);
    EXPECT_EQ(converter.GetOutChannelInfo().numChannels, BitCounts(CH_LAYOUT_QUAD));
    EXPECT_EQ(converter.GetInChannelInfo().channelLayout, CH_LAYOUT_STEREO);
    EXPECT_EQ(converter.GetInChannelInfo().numChannels, STEREO);

    // 2ch in > 1ch out: switches to the downmix path
    AudioChannelInfo monoInfo = {CH_LAYOUT_MONO, MONO};
    EXPECT_EQ(converter.SetOutChannelInfo(monoInfo), MIX_ERR_SUCCESS);
    EXPECT_EQ(converter.isInitialized_, true);
    EXPECT_EQ(converter.GetOutChannelInfo().channelLayout, CH_LAYOUT_MONO);
    EXPECT_EQ(converter.GetOutChannelInfo().numChannels, MONO);

    // HOA output is rejected by SetUpGeneralMixingTable: converter falls back to uninitialized
    AudioChannelInfo hoaInfo = {CH_LAYOUT_HOA_ORDER2_ACN_N3D, BitCounts(CH_LAYOUT_HOA_ORDER2_ACN_N3D)};
    EXPECT_EQ(converter.SetOutChannelInfo(hoaInfo), MIX_ERR_INVALID_ARG);
    EXPECT_EQ(converter.isInitialized_, false);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS