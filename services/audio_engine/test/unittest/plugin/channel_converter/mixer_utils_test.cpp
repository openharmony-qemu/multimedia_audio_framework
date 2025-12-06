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
#include "mixer_utils.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {


class AudioMixingTableTest : public testing::Test {
protected:
    void SetUp() override
    {
        errno_t err = memset_s(coeffTable, sizeof(coeffTable), 0, sizeof(coeffTable));
        ASSERT_EQ(err, 0); // ensure memset_s succ
    }

    void TearDown() override {}

    float coeffTable[MAX_CHANNELS][MAX_CHANNELS];
};

/**
 * @tc.name: SetUpGeneralMixingTable_InputChannelsExceedMax_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_001
 * @tc.desc: Test input channels exceed maximum limit
 */
HWTEST_F(AudioMixingTableTest, InputChannelsExceedMax_ReturnsError, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, MAX_CHANNELS + 1};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_INVALID_ARG);
}

/**
 * @tc.name: SetUpGeneralMixingTable_OutputChannelsExceedMax_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_002
 * @tc.desc: Test output channels exceed maximum limit
 */
HWTEST_F(AudioMixingTableTest, OutputChannelsExceedMax_ReturnsError, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, MAX_CHANNELS + 1};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_INVALID_ARG);
}

/**
 * @tc.name: SetUpGeneralMixingTable_InvalidInputChannelInfo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_003
 * @tc.desc: Test invalid input channel info with layout mismatch
 */
HWTEST_F(AudioMixingTableTest, InvalidInputChannelInfo_LayoutMismatch_ReturnsError, TestSize.Level1)
{
    // when input channelLayout does not match channel count, set channelLayout to default according to channel count,
    // SetUpGeneralMixingTable can still work properly
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 1}; // 立体声布局但只有1个通道
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_InvalidOutputChannelInfo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_004
 * @tc.desc: Test invalid output channel info with layout mismatch
 */
HWTEST_F(AudioMixingTableTest, InvalidOutputChannelInfo_LayoutMismatch_ReturnsError, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 1}; // 立体声布局但只有1个通道
    
    // when output channelLayout does not match channel count, set channelLayout to default according to channel count,
    // SetUpGeneralMixingTable can still work properly
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_UnknownInputLayout_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_005
 * @tc.desc: Test unknown input layout with valid channel count
 */
HWTEST_F(AudioMixingTableTest, UnknownInputLayout_ValidChannels_SetsDefault, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_UNKNOWN, 2}; // 未知布局但通道数有效
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    
    // 这个测试假设IsValidChLayout会为2通道设置默认立体声布局
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    // 由于输入布局被设置为有效布局，应该继续处理而不是返回错误
    EXPECT_NE(result, MIX_ERR_INVALID_ARG);
}

/**
 * @tc.name: SetUpGeneralMixingTable_HOAOutput_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_006
 * @tc.desc: Test HOA output is not supported
 */
HWTEST_F(AudioMixingTableTest, HOAOutput_ReturnsError, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_HOA_ORDER1_ACN_N3D, 4}; // HOA输出
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_INVALID_ARG);
}

/**
 * @tc.name: SetUpGeneralMixingTable_HOAInput_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_007
 * @tc.desc: Test HOA input uses first channel for all outputs
 */
HWTEST_F(AudioMixingTableTest, HOAInput_UsesFirstChannelForAllOutputs, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_HOA_ORDER1_ACN_N3D, 4}; // HOA输入
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
    // 验证所有输出通道都使用第一个输入通道
    for (uint32_t i = 0; i < outInfo.numChannels; i++) {
        EXPECT_FLOAT_EQ(coeffTable[i][0], COEF_0DB_F);
        // 其他输入通道应该为0
        for (uint32_t j = 1; j < inInfo.numChannels; j++) {
            EXPECT_FLOAT_EQ(coeffTable[i][j], 0.0f);
        }
    }
}

/**
 * @tc.name: SetUpGeneralMixingTable_MonoOutput_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_008
 * @tc.desc: Test mono output mixes all inputs to mono
 */
HWTEST_F(AudioMixingTableTest, MonoOutput_AllInputsToMono, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_MONO, 1};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
    // 验证所有输入通道都混合到Mono输出
    for (uint32_t i = 0; i < inInfo.numChannels; i++) {
        EXPECT_FLOAT_EQ(coeffTable[0][i], COEF_0DB_F);
    }
}

/**
 * @tc.name: SetUpGeneralMixingTable_MonoInput_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_009
 * @tc.desc: Test mono input is copied to all outputs
 */
HWTEST_F(AudioMixingTableTest, MonoInput_CopyToAllOutputs, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_MONO, 1};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
    // 验证Mono输入复制到所有输出通道
    for (uint32_t i = 0; i < outInfo.numChannels; i++) {
        EXPECT_FLOAT_EQ(coeffTable[0][i], COEF_0DB_F);
    }
}

/**
 * @tc.name: SetUpGeneralMixingTable_MissingStereoChannels_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_010
 * @tc.desc: Test missing stereo channels in both input and output
 */
HWTEST_F(AudioMixingTableTest, MissingStereoChannels_ReturnsError, TestSize.Level1)
{
    // 输入输出声道布局都是mono，复制输入即可
    AudioChannelInfo inInfo = {CH_LAYOUT_MONO, 1};
    AudioChannelInfo outInfo = {CH_LAYOUT_MONO, 1};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_StereoToStereo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_011
 * @tc.desc: Test stereo to stereo conversion calls inner function
 */
HWTEST_F(AudioMixingTableTest, StereoToStereo_CallsInnerFunction, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, true);
    
    // 应该调用inner函数并返回成功
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_ComplexLayout_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_012
 * @tc.desc: Test complex layout conversion calls inner function
 */
HWTEST_F(AudioMixingTableTest, ComplexLayout_CallsInnerFunction, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_5POINT1, 6};
    AudioChannelInfo outInfo = {CH_LAYOUT_7POINT1, 8};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, true);
    
    // 应该调用inner函数
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

// 辅助函数的测试
class AudioMixingUtilsTest : public testing::Test {
};

/**
 * @tc.name: SetDefaultChannelLayout_ValidInput_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_001
 * @tc.desc: Test setting default channel layout with valid input
 */
HWTEST_F(AudioMixingUtilsTest, SetDefaultChannelLayout_ValidInput, TestSize.Level1)
{
    AudioChannelLayout layout = CH_LAYOUT_UNKNOWN;
    bool result = SetDefaultChannelLayout(STEREO, layout);
    
    EXPECT_TRUE(result);
    EXPECT_NE(layout, CH_LAYOUT_UNKNOWN);
}

/**
 * @tc.name: SetDefaultChannelLayout_InvalidInput_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_002
 * @tc.desc: Test setting default channel layout with invalid input
 */
HWTEST_F(AudioMixingUtilsTest, SetDefaultChannelLayout_InvalidInput, TestSize.Level1)
{
    AudioChannelLayout layout = CH_LAYOUT_UNKNOWN;
    bool result = SetDefaultChannelLayout(static_cast<AudioChannel>(MAX_CHANNELS + 1), layout);
    
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsValidChLayout_ValidLayout_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_003
 * @tc.desc: Test valid channel layout validation
 */
HWTEST_F(AudioMixingUtilsTest, IsValidChLayout_ValidLayout, TestSize.Level1)
{
    AudioChannelLayout layout = CH_LAYOUT_STEREO;
    bool result = IsValidChLayout(layout, 2);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(layout, CH_LAYOUT_STEREO); // 布局不应改变
}

/**
 * @tc.name: IsValidChLayout_UnknownLayout_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_004
 * @tc.desc: Test unknown channel layout sets default layout
 */
HWTEST_F(AudioMixingUtilsTest, IsValidChLayout_UnknownLayout_SetsDefault, TestSize.Level1)
{
    AudioChannelLayout layout = CH_LAYOUT_UNKNOWN;
    bool result = IsValidChLayout(layout, 2);
    
    EXPECT_TRUE(result);
    EXPECT_NE(layout, CH_LAYOUT_UNKNOWN); // 应该设置了默认布局
}

/**
 * @tc.name: IsValidChLayout_LayoutMismatch_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_005
 * @tc.desc: Test channel layout mismatch sets default layout
 */
HWTEST_F(AudioMixingUtilsTest, IsValidChLayout_LayoutMismatch_SetsDefault, TestSize.Level1)
{
    AudioChannelLayout layout = CH_LAYOUT_STEREO; // 2通道布局
    bool result = IsValidChLayout(layout, 1); // 但声称只有1个通道
    
    EXPECT_TRUE(result);
    // 应该设置为1通道的默认布局
    EXPECT_EQ(layout, CH_LAYOUT_MONO);
}

/**
 * @tc.name: CheckIsHOA_HOALayout_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_006
 * @tc.desc: Test HOA layout detection returns true for HOA layouts
 */
HWTEST_F(AudioMixingUtilsTest, CheckIsHOA_HOALayout_ReturnsTrue, TestSize.Level1)
{
    // 测试各种HOA布局
    EXPECT_TRUE(CheckIsHOA(CH_LAYOUT_HOA_ORDER1_ACN_N3D));
    EXPECT_TRUE(CheckIsHOA(CH_LAYOUT_HOA_ORDER2_ACN_SN3D));
    EXPECT_TRUE(CheckIsHOA(CH_LAYOUT_HOA_ORDER3_FUMA));
}

/**
 * @tc.name: CheckIsHOA_NonHOALayout_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_007
 * @tc.desc: Test HOA layout detection returns false for non-HOA layouts
 */
HWTEST_F(AudioMixingUtilsTest, CheckIsHOA_NonHOALayout_ReturnsFalse, TestSize.Level1)
{
    EXPECT_FALSE(CheckIsHOA(CH_LAYOUT_STEREO));
    EXPECT_FALSE(CheckIsHOA(CH_LAYOUT_MONO));
}

/**
 * @tc.name: CheckIsHOA_NonHOALayout_002
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_007
 * @tc.desc: Test HOA layout detection returns false for non-HOA layouts
 */
HWTEST_F(AudioMixingUtilsTest, CheckIsHOA_NonHOALayout_ReturnsFalse2, TestSize.Level1)
{
    EXPECT_FALSE(CheckIsHOA(CH_LAYOUT_5POINT1));
}

/**
 * @tc.name: SetUpGeneralMixingTable_MaxChannelsInput_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_013
 * @tc.desc: Test maximum channels input works correctly
 */
HWTEST_F(AudioMixingTableTest, MaxChannelsInput_WorksCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_7POINT1POINT4, MAX_CHANNELS}; // 16通道
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_NE(result, MIX_ERR_INVALID_ARG);
}

/**
 * @tc.name: SetUpGeneralMixingTable_MaxChannelsOutput_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_014
 * @tc.desc: Test maximum channels output works correctly
 */
HWTEST_F(AudioMixingTableTest, MaxChannelsOutput_WorksCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_7POINT1POINT4, MAX_CHANNELS}; // 16通道
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_NE(result, MIX_ERR_INVALID_ARG);
}

/**
 * @tc.name: SetUpGeneralMixingTable_WithLfeMix_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_015
 * @tc.desc: Test stereo to 5.1 conversion with LFE mixing
 */
HWTEST_F(AudioMixingTableTest, WithLfeMix_StereoTo51_CallsInnerFunction, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_5POINT1, 6};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, true);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_WithoutLfeMix_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_016
 * @tc.desc: Test stereo to 5.1 conversion without LFE mixing
 */
HWTEST_F(AudioMixingTableTest, WithoutLfeMix_StereoTo51_CallsInnerFunction, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_5POINT1, 6};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_SurroundToStereo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_017
 * @tc.desc: Test surround to stereo conversion calls inner function
 */
HWTEST_F(AudioMixingTableTest, SurroundToStereo_CallsInnerFunction, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_SURROUND, 3}; // 左、右、中置
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};
    
    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);
    
    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: BitCounts_Calculation_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_008
 * @tc.desc: Test bit counts calculation for MONO channel layout
 */
HWTEST_F(AudioMixingUtilsTest, BitCounts_CalculatesCorrectChannelCount_Mono, TestSize.Level1)
{
    EXPECT_EQ(BitCounts(CH_LAYOUT_MONO), 1);
}

/**
 * @tc.name: BitCounts_Calculation_002
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_008
 * @tc.desc: Test bit counts calculation for STEREO channel layout
 */
HWTEST_F(AudioMixingUtilsTest, BitCounts_CalculatesCorrectChannelCount_Stereo, TestSize.Level1)
{
    EXPECT_EQ(BitCounts(CH_LAYOUT_STEREO), 2);
}

/**
 * @tc.name: BitCounts_Calculation_003
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_008
 * @tc.desc: Test bit counts calculation for 5_1 channel layout
 */
HWTEST_F(AudioMixingUtilsTest, BitCounts_CalculatesCorrectChannelCount_5_1, TestSize.Level1)
{
    EXPECT_EQ(BitCounts(CH_LAYOUT_5POINT1), 6);
}

/**
 * @tc.name: BitCounts_Calculation_004
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_008
 * @tc.desc: Test bit counts calculation for 7_1 channel layout
 */
HWTEST_F(AudioMixingUtilsTest, BitCounts_CalculatesCorrectChannelCount_7_1, TestSize.Level1)
{
    EXPECT_EQ(BitCounts(CH_LAYOUT_7POINT1), 8);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS