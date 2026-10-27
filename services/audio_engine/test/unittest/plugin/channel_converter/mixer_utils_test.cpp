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
#include <limits>
#include <vector>
#include "securec.h"
#include "mixer_utils.h"
#include "audio_proresampler.h"

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

/**
 * @tc.name  : SetUpGeneralMixingTable_MixBottom_Fallback_002
 * @tc.desc  : Test mapping from Bottom Front Left (BFL) to Front Left (FL)
 * when the output has no bottom layer (Fallback).
 */
HWTEST_F(AudioMixingUtilsTest, SetUpGeneralMixingTable_MixBottom_Fallback_002, TestSize.Level0)
{
    // 修复 1: 增加嵌套括号解决 -Wmissing-braces 错误
    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0.0f}};

    // 修复 2: 使用 static_cast 解决 rvalue 类型不匹配问题
    // Input: FL | FR | BFL (3ch)
    AudioChannelInfo inInfo = {
        static_cast<AudioChannelLayout>(CH_LAYOUT_STEREO | BOTTOM_FRONT_LEFT), 3
    };

    // Output: FL | FR (2ch), no MASK_BOTTOM
    AudioChannelInfo outInfo = { CH_LAYOUT_STEREO, 2 };

    int32_t ret = SetUpGeneralMixingTable(table, inInfo, outInfo, false);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    // BFL is input index 2
    // FL is output index 0
    // Expected: BFL signal projects to FL with 0dB (1.0f)
    EXPECT_NEAR(table[0][2], COEF_0DB_F, 0.0001f);
}

/**
 * @tc.name  : MixTopFront_Fallback_To_Mid_002
 * @tc.desc  : Test mapping from Top Front Left (TFL) to Front Left (FL)
 * when the output has no top layer (Fallback).
 */
HWTEST_F(AudioMixingUtilsTest, MixTopFront_Fallback_To_Mid_002, TestSize.Level0)
{
    // 修复 1: 嵌套大括号以满足 -Wmissing-braces 编译要求
    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0.0f}};

    // 修复 2: 使用 static_cast 显式转换位运算结果为 AudioChannelLayout 类型
    // Input: FL + FR + TOP_FRONT_LEFT (3ch)
    AudioChannelInfo inInfo = {
        static_cast<AudioChannelLayout>(CH_LAYOUT_STEREO | TOP_FRONT_LEFT), 3
    };

    // Output: Standard Stereo FL + FR (2ch)
    AudioChannelInfo outInfo = { CH_LAYOUT_STEREO, 2 };

    int32_t ret = SetUpGeneralMixingTable(table, inInfo, outInfo, false);
    EXPECT_EQ(ret, MIX_ERR_SUCCESS);

    // TFL is input index 2.
    // FL is output index 0.
    // Expected: Direct projection (0dB)
    EXPECT_NEAR(table[0][2], COEF_0DB_F, 0.0001f);
}

// ========== 新增30条测试用例 ==========

/**
 * @tc.name: SetUpGeneralMixingTable_3ChannelToStereo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_018
 * @tc.desc: Test 3 channel surround to stereo conversion
 */
HWTEST_F(AudioMixingTableTest, Surround3ChToStereo_ConvertsCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_SURROUND, 3}; // FL, FR, FC
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_4Point1ToStereo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_019
 * @tc.desc: Test 4.1 channel to stereo conversion
 */
HWTEST_F(AudioMixingTableTest, Channel4Point1ToStereo_ConvertsCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_3POINT1, 4};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_5Point1ToStereo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_020
 * @tc.desc: Test 5.1 channel to stereo conversion
 */
HWTEST_F(AudioMixingTableTest, Channel5Point1ToStereo_ConvertsCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_5POINT1, 6};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_7Point1ToStereo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_021
 * @tc.desc: Test 7.1 channel to stereo conversion
 */
HWTEST_F(AudioMixingTableTest, Channel7Point1ToStereo_ConvertsCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_7POINT1, 8};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_StereoTo7Point1_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_022
 * @tc.desc: Test stereo to 7.1 channel upmix
 */
HWTEST_F(AudioMixingTableTest, StereoTo7Point1_UpmixCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_7POINT1, 8};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_5Point1To7Point1_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_023
 * @tc.desc: Test 5.1 to 7.1 channel conversion
 */
HWTEST_F(AudioMixingTableTest, Channel5Point1To7Point1_ConvertsCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_5POINT1, 6};
    AudioChannelInfo outInfo = {CH_LAYOUT_7POINT1, 8};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_WithLfeChannel_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_025
 * @tc.desc: Test with LFE channel mixing enabled
 */
HWTEST_F(AudioMixingTableTest, WithLfeChannel_MixingEnabled, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_5POINT1, 6};
    AudioChannelInfo outInfo = {CH_LAYOUT_7POINT1, 8};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, true);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_LfeToStereo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_026
 * @tc.desc: Test LFE to stereo downmix with mixLfe=true
 */
HWTEST_F(AudioMixingTableTest, LfeToStereo_MixLfeTrue, TestSize.Level1)
{
    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0.0f}};
    AudioChannelInfo inInfo = {CH_LAYOUT_2POINT1, 3}; // FL, FR, LFE
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(table, inInfo, outInfo, true);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_LfeToStereo_002
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_027
 * @tc.desc: Test LFE to stereo downmix with mixLfe=false
 */
HWTEST_F(AudioMixingTableTest, LfeToStereo_MixLfeFalse, TestSize.Level1)
{
    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0.0f}};
    AudioChannelInfo inInfo = {CH_LAYOUT_2POINT1, 3}; // FL, FR, LFE
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(table, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_SideLeftToFrontLeft_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_028
 * @tc.desc: Test side left falls back to front left
 */
HWTEST_F(AudioMixingTableTest, SideLeftFallBackToFrontLeft, TestSize.Level1)
{
    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0.0f}};
    AudioChannelInfo inInfo = {static_cast<AudioChannelLayout>(CH_LAYOUT_STEREO | SIDE_LEFT), 3};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(table, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
    // SIDE_LEFT should map to FRONT_LEFT
    EXPECT_NEAR(table[0][2], COEF_0DB_F, 0.0001f);
}

/**
 * @tc.name: SetUpGeneralMixingTable_SideRightToFrontRight_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_029
 * @tc.desc: Test side right falls back to front right
 */
HWTEST_F(AudioMixingTableTest, SideRightFallBackToFrontRight, TestSize.Level1)
{
    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0.0f}};
    AudioChannelInfo inInfo = {static_cast<AudioChannelLayout>(CH_LAYOUT_STEREO | SIDE_RIGHT), 3};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(table, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
    // SIDE_RIGHT should map to FRONT_RIGHT
    EXPECT_NEAR(table[1][2], COEF_0DB_F, 0.0001f);
}

/**
 * @tc.name: SetUpGeneralMixingTable_FrontLeftOfCenter_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_037
 * @tc.desc: Test front left of center maps to front left
 */
HWTEST_F(AudioMixingTableTest, FrontLeftOfCenterMapsToFrontLeft, TestSize.Level1)
{
    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0.0f}};
    AudioChannelInfo inInfo = {static_cast<AudioChannelLayout>(CH_LAYOUT_STEREO | FRONT_LEFT_OF_CENTER), 3};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(table, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
    // FRONT_LEFT_OF_CENTER should map to FRONT_LEFT
    EXPECT_NEAR(table[0][2], COEF_0DB_F, 0.0001f);
}

/**
 * @tc.name: SetUpGeneralMixingTable_6Point1ToStereo_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_041
 * @tc.desc: Test 6.1 channel to stereo conversion
 */
HWTEST_F(AudioMixingTableTest, Channel6Point1ToStereo_ConvertsCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_6POINT1, 7};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_4Point1ToMono_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_042
 * @tc.desc: Test 4.1 to mono downmix
 */
HWTEST_F(AudioMixingTableTest, Channel4Point1ToMono_DownmixCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_3POINT1, 4};
    AudioChannelInfo outInfo = {CH_LAYOUT_MONO, 1};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_MonoTo5Point1_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_043
 * @tc.desc: Test mono to 5.1 upmix
 */
HWTEST_F(AudioMixingTableTest, MonoTo5Point1_UpmixCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_MONO, 1};
    AudioChannelInfo outInfo = {CH_LAYOUT_5POINT1, 6};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_StereoTo6Point1_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_044
 * @tc.desc: Test stereo to 6.1 upmix
 */
HWTEST_F(AudioMixingTableTest, StereoTo6Point1_UpmixCorrectly, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_STEREO, 2};
    AudioChannelInfo outInfo = {CH_LAYOUT_6POINT1, 7};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
}

/**
 * @tc.name: SetUpGeneralMixingTable_HOAInputVariousOutput_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_045
 * @tc.desc: Test HOA input to various output layouts
 */
HWTEST_F(AudioMixingTableTest, HOAInputTo7Point1_UsesFirstChannel, TestSize.Level1)
{
    AudioChannelInfo inInfo = {CH_LAYOUT_HOA_ORDER2_ACN_N3D, 9};
    AudioChannelInfo outInfo = {CH_LAYOUT_7POINT1, 8};

    int32_t result = SetUpGeneralMixingTable(coeffTable, inInfo, outInfo, false);

    EXPECT_EQ(result, MIX_ERR_SUCCESS);
    // All outputs should use first input channel
    for (uint32_t i = 0; i < outInfo.numChannels; i++) {
        EXPECT_FLOAT_EQ(coeffTable[i][0], COEF_0DB_F);
    }
}

/**
 * @tc.name: SetUpGeneralMixingTable_InvalidLayout_001
 * @tc.type: FUNC
 * @tc.number: SetUpGeneralMixingTable_047
 * @tc.desc: Test input without stereo layout fails validation
 */
HWTEST_F(AudioMixingTableTest, InputWithoutStereoLayout_ReturnsError, TestSize.Level1)
{
    float table[MAX_CHANNELS][MAX_CHANNELS] = {{0.0f}};
    // 使用没有FRONT_LEFT和FRONT_RIGHT的布局
    AudioChannelInfo inInfo = {static_cast<AudioChannelLayout>(BACK_LEFT), 1};
    AudioChannelInfo outInfo = {CH_LAYOUT_STEREO, 2};

    int32_t result = SetUpGeneralMixingTable(table, inInfo, outInfo, false);

    // 应该失败，因为输入布局不包含FRONT_LEFT和FRONT_RIGHT
    EXPECT_EQ(result, MIX_ERR_INVALID_ARG);
}

/**
 * @tc.name: SetDefaultChannelLayout_AllChannels_001
 * @tc.type: FUNC
 * @tc.number: AudioMixingUtils_010
 * @tc.desc: Test SetDefaultChannelLayout for all valid channel counts
 */
HWTEST_F(AudioMixingUtilsTest, SetDefaultChannelLayout_AllValidChannels, TestSize.Level1)
{
    AudioChannelLayout layout;

    EXPECT_TRUE(SetDefaultChannelLayout(MONO, layout));
    EXPECT_EQ(layout, CH_LAYOUT_MONO);

    EXPECT_TRUE(SetDefaultChannelLayout(STEREO, layout));
    EXPECT_EQ(layout, CH_LAYOUT_STEREO);

    EXPECT_TRUE(SetDefaultChannelLayout(CHANNEL_3, layout));
    EXPECT_EQ(layout, CH_LAYOUT_SURROUND);

    EXPECT_TRUE(SetDefaultChannelLayout(CHANNEL_4, layout));
    EXPECT_EQ(layout, CH_LAYOUT_3POINT1);

    EXPECT_TRUE(SetDefaultChannelLayout(CHANNEL_5, layout));
    EXPECT_EQ(layout, CH_LAYOUT_4POINT1);

    EXPECT_TRUE(SetDefaultChannelLayout(CHANNEL_6, layout));
    EXPECT_EQ(layout, CH_LAYOUT_5POINT1);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

namespace {
constexpr int32_t MIXER_TEST_RATE = 48000;
constexpr int32_t MIXER_TEST_FRAMES = 960; // 20ms stereo block, 960*2 divisible by 4
constexpr int32_t MIXER_TEST_BLOCKS = 30;  // let the limiter gain ramp settle

std::vector<int16_t> MakeS16Block(int32_t frames, int32_t channels, int16_t amplitude)
{
    return std::vector<int16_t>(static_cast<size_t>(frames) * channels, amplitude);
}

double MeanAbs(const int16_t *block, size_t count)
{
    int64_t sum = 0;
    size_t n = count < 96 ? count : 96; // head samples are enough on a constant block
    for (size_t i = 0; i < n; ++i) {
        sum += block[i] > 0 ? block[i] : -block[i];
    }
    return n == 0 ? 0.0 : static_cast<double>(sum) / static_cast<double>(n);
}
} // namespace

class AudioMixerFacadeTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: AudioMixerCreate_001
 * @tc.number: AudioMixerCreate_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerCreate_001, TestSize.Level0)
{
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_S16_LE);
    EXPECT_NE(handle, nullptr);
    AudioMixerDestroy(handle);

    EXPECT_EQ(AudioMixerCreate(4000, 2, AUDIO_PCM_S16_LE), nullptr);
    EXPECT_EQ(AudioMixerCreate(384001, 2, AUDIO_PCM_S16_LE), nullptr);
    EXPECT_EQ(AudioMixerCreate(MIXER_TEST_RATE, 3, AUDIO_PCM_S16_LE), nullptr);
    EXPECT_EQ(AudioMixerCreate(MIXER_TEST_RATE, 2, static_cast<AudioPcmFormat>(20)), nullptr);
    AudioMixerDestroy(nullptr);
}

/**
 * @tc.name: AudioMixerProcess_001
 * @tc.number: AudioMixerProcess_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerProcess_001, TestSize.Level0)
{
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int16_t> blockA = MakeS16Block(MIXER_TEST_FRAMES, 2, 0x2000); // 0.25
    std::vector<int16_t> blockB = MakeS16Block(MIXER_TEST_FRAMES, 2, -0x2000);
    struct AudioPcmMixStream streams[2] = {};
    streams[0].data = blockA.data();
    streams[0].sampleCount = MIXER_TEST_FRAMES;
    streams[0].volume = 1.0f;
    streams[1].data = blockB.data();
    streams[1].sampleCount = MIXER_TEST_FRAMES;
    streams[1].volume = 1.0f;

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, streams, 2);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        ASSERT_EQ(result.outSampleCount, MIXER_TEST_FRAMES);
    }
    // A + B cancels out, output stays silent after the limiter settles
    const int16_t *out = reinterpret_cast<const int16_t *>(result.outData);
    for (int32_t i = 0; i < MIXER_TEST_FRAMES * 2; ++i) {
        EXPECT_EQ(out[i], 0);
    }
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerVolume_001
 * @tc.number: AudioMixerVolume_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerVolume_001, TestSize.Level0)
{
    // volume 0.0 must mute the single stream exactly
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int16_t> block = MakeS16Block(MIXER_TEST_FRAMES, 2, 0x2000);
    struct AudioPcmMixStream stream = {};
    stream.data = block.data();
    stream.sampleCount = MIXER_TEST_FRAMES;
    stream.volume = 0.0f;

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &stream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    const int16_t *out = reinterpret_cast<const int16_t *>(result.outData);
    for (int32_t i = 0; i < MIXER_TEST_FRAMES * 2; ++i) {
        EXPECT_EQ(out[i], 0);
    }
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerVolume_002
 * @tc.number: AudioMixerVolume_002
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerVolume_002, TestSize.Level0)
{
    // volume 0.5 scales the stream level to about half of volume 1.0
    std::vector<int16_t> block = MakeS16Block(MIXER_TEST_FRAMES, 2, 9830); // ~0.3, below limiter threshold
    double levels[2] = {0.0, 0.0};
    const float volumes[2] = {1.0f, 0.5f};
    for (int32_t v = 0; v < 2; ++v) {
        AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_S16_LE);
        ASSERT_NE(handle, nullptr);
        struct AudioPcmMixStream stream = {};
        stream.data = block.data();
        stream.sampleCount = MIXER_TEST_FRAMES;
        stream.volume = volumes[v];
        struct AudioMixResult result = {};
        for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
            result = AudioMixerProcess(handle, &stream, 1);
            ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        }
        const int16_t *out = reinterpret_cast<const int16_t *>(result.outData);
        levels[v] = MeanAbs(out, static_cast<size_t>(MIXER_TEST_FRAMES) * 2);
        AudioMixerDestroy(handle);
    }
    EXPECT_GT(levels[0], 0.0);
    double ratio = levels[1] / levels[0];
    EXPECT_GT(ratio, 0.3);
    EXPECT_LT(ratio, 0.7);
}

/**
 * @tc.name: AudioMixerMono_001
 * @tc.number: AudioMixerMono_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerMono_001, TestSize.Level0)
{
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 1, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int16_t> block = MakeS16Block(MIXER_TEST_FRAMES, 1, 0x2000);
    struct AudioPcmMixStream streams[2] = {};
    streams[0].data = block.data();
    streams[0].sampleCount = MIXER_TEST_FRAMES;
    streams[0].volume = 1.0f;
    streams[1].data = block.data();
    streams[1].sampleCount = MIXER_TEST_FRAMES;
    streams[1].volume = 1.0f;

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, streams, 2);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        ASSERT_EQ(result.outSampleCount, MIXER_TEST_FRAMES);
    }
    // 0.25 + 0.25 = 0.5, below limiter threshold: steady output around 0.5 amplitude
    const int16_t *out = reinterpret_cast<const int16_t *>(result.outData);
    double level = MeanAbs(out, static_cast<size_t>(MIXER_TEST_FRAMES));
    EXPECT_GT(level, 0.3 * 32767);
    EXPECT_LT(level, 0.7 * 32767);
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerInvalid_001
 * @tc.number: AudioMixerInvalid_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerInvalid_001, TestSize.Level0)
{
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int16_t> block = MakeS16Block(MIXER_TEST_FRAMES, 2, 0x2000);
    struct AudioPcmMixStream streams[2] = {};
    streams[0].data = block.data();
    streams[0].sampleCount = MIXER_TEST_FRAMES;
    streams[0].volume = 1.0f;
    streams[1] = streams[0];

    EXPECT_EQ(AudioMixerProcess(nullptr, streams, 1).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(AudioMixerProcess(handle, streams, 0).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(AudioMixerProcess(handle, nullptr, 1).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);

    streams[1].sampleCount = MIXER_TEST_FRAMES / 2; // unequal sampleCount
    EXPECT_EQ(AudioMixerProcess(handle, streams, 2).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    streams[1].sampleCount = MIXER_TEST_FRAMES;

    streams[1].volume = 1.5f; // volume out of range
    EXPECT_EQ(AudioMixerProcess(handle, streams, 2).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    streams[1].volume = 1.0f;

    // first successful call locks the block length
    struct AudioMixResult result = AudioMixerProcess(handle, streams, 2);
    ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    streams[0].sampleCount = MIXER_TEST_FRAMES / 2;
    streams[1].sampleCount = MIXER_TEST_FRAMES / 2;
    EXPECT_EQ(AudioMixerProcess(handle, streams, 2).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);

    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerNanVolume_001
 * @tc.number: AudioMixerNanVolume_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerNanVolume_001, TestSize.Level0)
{
    // NaN must be rejected: NaN >= x and NaN <= x are both false under IEEE754
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int16_t> block = MakeS16Block(MIXER_TEST_FRAMES, 2, 0x2000);
    struct AudioPcmMixStream stream = {};
    stream.data = block.data();
    stream.sampleCount = MIXER_TEST_FRAMES;
    stream.volume = std::numeric_limits<float>::quiet_NaN();
    EXPECT_EQ(AudioMixerProcess(handle, &stream, 1).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);

    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerS24_001
 * @tc.number: AudioMixerS24_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerS24_001, TestSize.Level0)
{
    // 0.25 amplitude as S24 little endian {0x00, 0x00, 0x20} = 0x200000
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_S24_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> block(static_cast<size_t>(MIXER_TEST_FRAMES) * 2 * 3, 0);
    for (size_t i = 0; i + 2 < block.size(); i += 3) {
        block[i] = 0x00;
        block[i + 1] = 0x00;
        block[i + 2] = 0x20;
    }
    struct AudioPcmMixStream stream = {};
    stream.data = block.data();
    stream.sampleCount = MIXER_TEST_FRAMES;
    stream.volume = 1.0f;

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &stream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    // steady output around 0.25 amplitude: 24-bit value 0x200000 = 2097152
    const uint8_t *out = result.outData;
    uint32_t sample = (static_cast<uint32_t>(out[2]) << 16) | (static_cast<uint32_t>(out[1]) << 8) |
        static_cast<uint32_t>(out[0]);
    EXPECT_GT(sample, 1600000u); // ~0.19 * 2^23
    EXPECT_LT(sample, 2600000u); // ~0.31 * 2^23
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerS32_001
 * @tc.number: AudioMixerS32_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerS32_001, TestSize.Level0)
{
    // 0.25 amplitude as S32: 0x20000000 = 536870912
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_S32_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int32_t> block(static_cast<size_t>(MIXER_TEST_FRAMES) * 2, 0x20000000);
    struct AudioPcmMixStream stream = {};
    stream.data = block.data();
    stream.sampleCount = MIXER_TEST_FRAMES;
    stream.volume = 1.0f;

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &stream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    const int32_t *out = reinterpret_cast<const int32_t *>(result.outData);
    EXPECT_GT(out[0], 406000000); // ~0.19 * 2^31
    EXPECT_LT(out[0], 666000000); // ~0.31 * 2^31
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerF32_001
 * @tc.number: AudioMixerF32_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerF32_001, TestSize.Level0)
{
    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, 2, AUDIO_PCM_F32_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<float> block(static_cast<size_t>(MIXER_TEST_FRAMES) * 2, 0.25f);
    struct AudioPcmMixStream stream = {};
    stream.data = block.data();
    stream.sampleCount = MIXER_TEST_FRAMES;
    stream.volume = 1.0f;

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &stream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    const float *out = reinterpret_cast<const float *>(result.outData);
    EXPECT_GT(out[0], 0.19f);
    EXPECT_LT(out[0], 0.31f);
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerResampleMix_001
 * @tc.number: AudioMixerResampleMix_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerResampleMix_001, TestSize.Level0)
{
    // Full BT composition: resample a 48k S16 stream to 44.1k through the float pipeline,
    // convert back to S16, then mix it with a native 44.1k S16 stream.
    using OHOS::AudioStandard::HPAE::ProResampler;
    using OHOS::AudioStandard::HPAE::QUALITY_FAST;
    constexpr int32_t inRate = 48000;
    constexpr int32_t mixRate = 44100;
    constexpr int32_t channels = 2;
    constexpr int32_t inFrames = 320; // 320 * 44100 / 48000 = 294 exactly
    constexpr int32_t mixFrames = inFrames * mixRate / inRate; // 294, 294 * 2 divisible by 4
    const int16_t amplitude = 0x2000; // 0.25 per leg, sum 0.5 stays below the limiter threshold

    ProResampler resampler(inRate, mixRate, channels, QUALITY_FAST);
    AudioMixHandle handle = AudioMixerCreate(mixRate, channels, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int16_t> inBlock = MakeS16Block(inFrames, channels, amplitude);
    std::vector<int16_t> nativeBlock = MakeS16Block(mixFrames, channels, amplitude);
    std::vector<float> floatIn(static_cast<size_t>(inFrames) * channels);
    std::vector<float> floatOut(static_cast<size_t>(mixFrames) * channels);
    std::vector<int16_t> resampled(static_cast<size_t>(mixFrames) * channels);

    struct AudioPcmMixStream streams[2] = {};
    streams[0].data = resampled.data();
    streams[0].sampleCount = mixFrames;
    streams[0].volume = 1.0f;
    streams[1].data = nativeBlock.data();
    streams[1].sampleCount = mixFrames;
    streams[1].volume = 1.0f;

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        ASSERT_EQ(ConvertPcmToFloat(AUDIO_PCM_S16_LE, inBlock.data(), inFrames * channels,
            floatIn.data()), AUDIO_PCM_ALGO_SUCCESS);
        ASSERT_EQ(resampler.Process(floatIn.data(), inFrames, floatOut.data(), mixFrames), 0);
        ASSERT_EQ(ConvertFloatToPcm(AUDIO_PCM_S16_LE, floatOut.data(), mixFrames * channels,
            resampled.data()), AUDIO_PCM_ALGO_SUCCESS);
        result = AudioMixerProcess(handle, streams, 2);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        ASSERT_EQ(result.outSampleCount, mixFrames);
    }
    // steady state: resampled leg (~0.25) + native leg (0.25) ~= 0.5; a broken resample leg
    // would drop the level back to ~0.25 and fail the lower bound
    const int16_t *out = reinterpret_cast<const int16_t *>(result.outData);
    double level = MeanAbs(out, static_cast<size_t>(mixFrames) * channels);
    EXPECT_GT(level, 0.3 * 32767);
    EXPECT_LT(level, 0.7 * 32767);
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerBoundary_001
 * @tc.number: AudioMixerBoundary_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerBoundary_001, TestSize.Level0)
{
    constexpr int32_t mixerMaxStreams = 1024; // facade gate: stream count in [1, 1024]
    constexpr int32_t max60MsFramesAt48k = 2880; // 48000 Hz * 60 ms block cap
    constexpr int32_t mixerStereo = 2;
    constexpr int16_t mixerTestAmplitude = 0x2000; // ~0.25 full scale, below limiter threshold

    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, mixerStereo, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);
    std::vector<int16_t> block = MakeS16Block(MIXER_TEST_FRAMES, mixerStereo, mixerTestAmplitude);
    struct AudioPcmMixStream stream = {block.data(), MIXER_TEST_FRAMES, 1.0f};

    // streamCount above the 1024 gate, rejected before any array access
    EXPECT_EQ(AudioMixerProcess(handle, &stream, mixerMaxStreams + 1).errCode,
        AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    // first stream sampleCount not positive
    struct AudioPcmMixStream zeroCount = {block.data(), 0, 1.0f};
    EXPECT_EQ(AudioMixerProcess(handle, &zeroCount, 1).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    // stream data null
    struct AudioPcmMixStream nullData = {nullptr, MIXER_TEST_FRAMES, 1.0f};
    EXPECT_EQ(AudioMixerProcess(handle, &nullData, 1).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    // block longer than 60 ms
    std::vector<int16_t> overBlock = MakeS16Block(max60MsFramesAt48k + 1, mixerStereo,
        mixerTestAmplitude);
    struct AudioPcmMixStream overLen = {overBlock.data(), max60MsFramesAt48k + 1, 1.0f};
    EXPECT_EQ(AudioMixerProcess(handle, &overLen, 1).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    // 1 stereo frame is processable geometry (2 chunks of 1 sample) but differs from
    // the locked 960-frame block length, so the lock gate rejects it
    std::vector<int16_t> misalignBlock = MakeS16Block(1, mixerStereo, mixerTestAmplitude);
    struct AudioPcmMixStream misalign = {misalignBlock.data(), 1, 1.0f};
    EXPECT_EQ(AudioMixerProcess(handle, &misalign, 1).errCode, AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerStereoOddAlgoFrame_001
 * @tc.number: AudioMixerStereoOddAlgoFrame_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerStereoOddAlgoFrame_001, TestSize.Level0)
{
    constexpr int32_t mixerStereo = 2;
    constexpr int16_t mixerTestAmplitude = 0x2000; // ~0.25 full scale, below limiter threshold
    // 2 stereo frames: totalSamples = 4 passes the facade gate but the limiter
    // algoFrameLen = 1 is odd, driving the unaligned-history path of ProcessAlgo
    constexpr int32_t oddAlgoFrames = 2;
    // 6 stereo frames: algoFrameLen = 3, exercises the unaligned loop body
    constexpr int32_t unalignedLoopFrames = 6;

    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, mixerStereo, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);
    std::vector<int16_t> block2 = MakeS16Block(oddAlgoFrames, mixerStereo, mixerTestAmplitude);
    struct AudioPcmMixStream stream2 = {block2.data(), oddAlgoFrames, 1.0f};
    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &stream2, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        EXPECT_EQ(result.outSampleCount, oddAlgoFrames);
    }
    // limiter output lags input by one algo chunk (a quarter block); a fresh handle starts
    // from zeroed history, so the non-zero assertion belongs after the loop
    EXPECT_GT(MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        oddAlgoFrames * mixerStereo), 0.0);
    AudioMixerDestroy(handle);

    // a fresh handle because the first successful call locks the block length
    AudioMixHandle handle6 = AudioMixerCreate(MIXER_TEST_RATE, mixerStereo, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle6, nullptr);
    std::vector<int16_t> block6 = MakeS16Block(unalignedLoopFrames, mixerStereo,
        mixerTestAmplitude);
    struct AudioPcmMixStream stream6 = {block6.data(), unalignedLoopFrames, 1.0f};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle6, &stream6, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        EXPECT_EQ(result.outSampleCount, unalignedLoopFrames);
    }
    EXPECT_GT(MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        unalignedLoopFrames * mixerStereo), 0.0);
    AudioMixerDestroy(handle6);
}

/**
 * @tc.name: AudioMixerLimiterLoud_001
 * @tc.number: AudioMixerLimiterLoud_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerLimiterLoud_001, TestSize.Level0)
{
    // Loud blocks above the limiter threshold drive the envelope attack, gain reduction
    // and the gain attack step to a louder block; trailing quiet blocks drive the
    // envelope decay and the release. L/R amplitudes alternate so both std::max(|L|, |R|)
    // comparison edges are taken.
    constexpr int32_t mixerStereo = 2;
    constexpr int16_t loudLeftAmplitude = 0x7A00;  // ~0.953, above the 0.92 limiter threshold
    constexpr int16_t loudRightAmplitude = 0x6000; // ~0.75, other side of the L/R max
    constexpr int16_t fullScaleAmplitude = 0x7FFF; // ~1.0: target gain drops below the
                                                     // settled gain, hitting the gain attack
    constexpr int16_t quietAmplitude = 0x2000;      // ~0.25, below threshold: decay + release
    constexpr int32_t stepBlocks = 8;               // blocks per louder / quieter step
    constexpr double fullScaleS16 = 32767.0;
    constexpr double limiterHeadroomRatio = 0.98;  // settled loud output is gain-scaled input
    constexpr double outputSanityRatio = 0.80;     // attenuation must stay moderate
    constexpr double quietLevelMaxRatio = 0.5;    // released output falls back near 0.25

    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, mixerStereo, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int16_t> loudBlock = MakeS16Block(MIXER_TEST_FRAMES, mixerStereo, 0);
    bool loudOnLeft = true;
    for (int32_t i = 0; i < MIXER_TEST_FRAMES * mixerStereo; i += mixerStereo) {
        loudBlock[i] = loudOnLeft ? loudLeftAmplitude : loudRightAmplitude;
        loudBlock[i + 1] = loudOnLeft ? loudRightAmplitude : loudLeftAmplitude;
        loudOnLeft = !loudOnLeft;
    }
    double loudInputLevel = (loudLeftAmplitude + loudRightAmplitude) * 0.5;
    std::vector<int16_t> louderBlock = MakeS16Block(MIXER_TEST_FRAMES, mixerStereo,
        fullScaleAmplitude);
    std::vector<int16_t> quietBlock = MakeS16Block(MIXER_TEST_FRAMES, mixerStereo,
        quietAmplitude);

    struct AudioPcmMixStream loudStream = {loudBlock.data(), MIXER_TEST_FRAMES, 1.0f};
    struct AudioPcmMixStream louderStream = {louderBlock.data(), MIXER_TEST_FRAMES, 1.0f};
    struct AudioPcmMixStream quietStream = {quietBlock.data(), MIXER_TEST_FRAMES, 1.0f};

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &loudStream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    // settled: envelope ~0.953, target gain = 0.92 / 0.953, output ~= input * 0.965
    double loudLevel = MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        static_cast<size_t>(MIXER_TEST_FRAMES) * mixerStereo);
    EXPECT_LT(loudLevel, loudInputLevel * limiterHeadroomRatio);
    EXPECT_GT(loudLevel, loudInputLevel * outputSanityRatio);

    for (int32_t i = 0; i < stepBlocks; ++i) {
        result = AudioMixerProcess(handle, &louderStream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    for (int32_t i = 0; i < stepBlocks; ++i) {
        result = AudioMixerProcess(handle, &quietStream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    // released: quiet history at ~0.25 comes out at roughly unit gain
    double quietLevel = MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        static_cast<size_t>(MIXER_TEST_FRAMES) * mixerStereo);
    EXPECT_GT(quietLevel, 0.0);
    EXPECT_LT(quietLevel, fullScaleS16 * quietLevelMaxRatio);
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerLimiterLoudMono_001
 * @tc.number: AudioMixerLimiterLoudMono_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerLimiterLoudMono_001, TestSize.Level0)
{
    // Mono twin of AudioMixerLimiterLoud_001: the mono envelope and gain helpers have
    // their own attack and decay branches.
    constexpr int32_t mixerMono = 1;
    constexpr int16_t loudAmplitude = 0x7A00; // ~0.953, above the 0.92 limiter threshold
    constexpr int16_t quietAmplitude = 0x2000;
    constexpr int32_t quietBlocks = 8;
    constexpr double fullScaleS16 = 32767.0;
    constexpr double limiterHeadroomRatio = 0.98;
    constexpr double outputSanityRatio = 0.80;
    constexpr double quietLevelMaxRatio = 0.5;

    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, mixerMono, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<int16_t> loudBlock = MakeS16Block(MIXER_TEST_FRAMES, mixerMono, loudAmplitude);
    std::vector<int16_t> quietBlock = MakeS16Block(MIXER_TEST_FRAMES, mixerMono, quietAmplitude);
    struct AudioPcmMixStream loudStream = {loudBlock.data(), MIXER_TEST_FRAMES, 1.0f};
    struct AudioPcmMixStream quietStream = {quietBlock.data(), MIXER_TEST_FRAMES, 1.0f};

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &loudStream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    double loudLevel = MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        static_cast<size_t>(MIXER_TEST_FRAMES) * mixerMono);
    EXPECT_LT(loudLevel, loudAmplitude * limiterHeadroomRatio);
    EXPECT_GT(loudLevel, loudAmplitude * outputSanityRatio);

    for (int32_t i = 0; i < quietBlocks; ++i) {
        result = AudioMixerProcess(handle, &quietStream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    double quietLevel = MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        static_cast<size_t>(MIXER_TEST_FRAMES) * mixerMono);
    EXPECT_GT(quietLevel, 0.0);
    EXPECT_LT(quietLevel, fullScaleS16 * quietLevelMaxRatio);
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerS24TinySignal_001
 * @tc.number: AudioMixerS24TinySignal_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerS24TinySignal_001, TestSize.Level0)
{
    // 24-bit value 1 converts to ~1.19e-7, below the limiter's 1e-6 epsilon: the
    // sub-epsilon branch must keep the target gain at 1.0 instead of dividing by it.
    constexpr int32_t mixerStereo = 2;
    constexpr int32_t s24BytesPerSample = 3;
    constexpr int32_t s24MsbIndex = 2;
    constexpr int32_t s24Bit16 = 16;
    constexpr int32_t s24Bit8 = 8;
    constexpr uint8_t s24TinyLsb = 1; // 24-bit value 1 -> ~1.19e-7 float level
    constexpr uint32_t s24TinyMax = 1; // round trip of a sub-epsilon sample stays 0 or 1

    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, mixerStereo, AUDIO_PCM_S24_LE);
    ASSERT_NE(handle, nullptr);

    std::vector<uint8_t> block(static_cast<size_t>(MIXER_TEST_FRAMES) * mixerStereo *
        s24BytesPerSample, 0);
    for (size_t i = 0; i < block.size(); i += s24BytesPerSample) {
        block[i] = s24TinyLsb;
    }
    struct AudioPcmMixStream stream = {block.data(), MIXER_TEST_FRAMES, 1.0f};

    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &stream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
    }
    const uint8_t *out = result.outData;
    uint32_t sample = (static_cast<uint32_t>(out[s24MsbIndex]) << s24Bit16) |
        (static_cast<uint32_t>(out[1]) << s24Bit8) | static_cast<uint32_t>(out[0]);
    EXPECT_LE(sample, s24TinyMax);
    AudioMixerDestroy(handle);
}

/**
 * @tc.name: AudioMixerTailLoopGeometry_001
 * @tc.number: AudioMixerTailLoopGeometry_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerTailLoopGeometry_001, TestSize.Level0)
{
    // Block sizes whose algoFrameLen is below one NEON iteration (stereo 6, mono 3
    // samples) leave the whole chunk to the scalar remainder loops on NEON builds.
    constexpr int32_t mixerStereo = 2;
    constexpr int32_t mixerMono = 1;
    constexpr int32_t stereoTailFrames = 12; // totalSamples 24 -> algoFrameLen 6
    constexpr int32_t monoTailFrames = 12;   // totalSamples 12 -> algoFrameLen 3
    constexpr int16_t tailAmplitude = 0x2000;

    AudioMixHandle handle = AudioMixerCreate(MIXER_TEST_RATE, mixerStereo, AUDIO_PCM_S16_LE);
    ASSERT_NE(handle, nullptr);
    std::vector<int16_t> stereoBlock = MakeS16Block(stereoTailFrames, mixerStereo,
        tailAmplitude);
    struct AudioPcmMixStream stereoStream = {stereoBlock.data(), stereoTailFrames, 1.0f};
    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(handle, &stereoStream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        EXPECT_EQ(result.outSampleCount, stereoTailFrames);
    }
    EXPECT_GT(MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        stereoTailFrames * mixerStereo), 0.0);
    AudioMixerDestroy(handle);

    AudioMixHandle mono = AudioMixerCreate(MIXER_TEST_RATE, mixerMono, AUDIO_PCM_S16_LE);
    ASSERT_NE(mono, nullptr);
    std::vector<int16_t> monoBlock = MakeS16Block(monoTailFrames, mixerMono, tailAmplitude);
    struct AudioPcmMixStream monoStream = {monoBlock.data(), monoTailFrames, 1.0f};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(mono, &monoStream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        EXPECT_EQ(result.outSampleCount, monoTailFrames);
    }
    EXPECT_GT(MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        monoTailFrames * mixerMono), 0.0);
    AudioMixerDestroy(mono);
}

/**
 * @tc.name: AudioMixerArbitraryBlock_001
 * @tc.number: AudioMixerArbitraryBlock_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioMixerArbitraryBlock_001, TestSize.Level0)
{
    // Block lengths not divisible by 4 used to be rejected; the limiter now falls back
    // to 3 / 2 / 1 equal chunks. mono@44.1k 20ms = 882 is the BT dual-path regression.
    constexpr int32_t rateAt44100 = 44100;
    constexpr int32_t mixerMono = 1;
    constexpr int32_t mixerStereo = 2;
    constexpr int32_t mono20MsAt44100 = 882; // 44100 * 20 / 1000, 3 chunks of 294
    constexpr int32_t monoPrimeFrames = 881; // prime, single chunk
    constexpr int32_t monoAlignedFrames = 880; // 4 chunks of 220, unchanged path
    constexpr int32_t oneStereoFrame = 1;     // 2 chunks of 1, odd-history path
    constexpr int16_t testAmplitude = 0x2000;

    const int32_t frameCounts[] = {mono20MsAt44100, monoPrimeFrames, monoAlignedFrames};
    for (int32_t frameCount : frameCounts) {
        AudioMixHandle handle = AudioMixerCreate(rateAt44100, mixerMono, AUDIO_PCM_S16_LE);
        ASSERT_NE(handle, nullptr);
        std::vector<int16_t> block = MakeS16Block(frameCount, mixerMono, testAmplitude);
        struct AudioPcmMixStream stream = {block.data(), frameCount, 1.0f};
        struct AudioMixResult result = {};
        for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
            result = AudioMixerProcess(handle, &stream, 1);
            ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
            EXPECT_EQ(result.outSampleCount, frameCount);
        }
        EXPECT_GT(MeanAbs(reinterpret_cast<const int16_t *>(result.outData), frameCount), 0.0);
        AudioMixerDestroy(handle);
    }

    // 1 stereo frame: 2 samples chunk into 2 x 1 and run the odd-history path
    AudioMixHandle stereo = AudioMixerCreate(MIXER_TEST_RATE, mixerStereo, AUDIO_PCM_S16_LE);
    ASSERT_NE(stereo, nullptr);
    std::vector<int16_t> frame = MakeS16Block(oneStereoFrame, mixerStereo, testAmplitude);
    struct AudioPcmMixStream stereoStream = {frame.data(), oneStereoFrame, 1.0f};
    struct AudioMixResult result = {};
    for (int32_t i = 0; i < MIXER_TEST_BLOCKS; ++i) {
        result = AudioMixerProcess(stereo, &stereoStream, 1);
        ASSERT_EQ(result.errCode, AUDIO_PCM_ALGO_SUCCESS);
        EXPECT_EQ(result.outSampleCount, oneStereoFrame);
    }
    EXPECT_GT(MeanAbs(reinterpret_cast<const int16_t *>(result.outData),
        oneStereoFrame * mixerStereo), 0.0);
    AudioMixerDestroy(stereo);
}

/**
 * @tc.name: AudioPcmConvertChannels_001
 * @tc.number: AudioPcmConvertChannels_001
 */
HWTEST_F(AudioMixerFacadeTest, AudioPcmConvertChannels_001, TestSize.Level0)
{
    constexpr int32_t mixerMono = 1;
    constexpr int32_t mixerStereo = 2;
    constexpr int32_t badChannels = 3;
    constexpr int32_t sampleCount = 64;
    constexpr int32_t s24BytesPerSample = 3;
    constexpr int32_t s24MsbIndex = 2;
    constexpr int32_t s24Bit16 = 16;
    constexpr int32_t s24Bit8 = 8;
    constexpr int16_t upmixAmplitude = 0x2000;  // 0.25
    constexpr int16_t leftAmplitude = 0x2000;   // (0.25 + 0.125) / 2 = 0.1875
    constexpr int16_t rightAmplitude = 0x1000;  // -> 6144 = 0x1800 exactly in S16
    constexpr int16_t downmixExpected = 0x1800;

    // mono -> stereo: each sample duplicated to L and R
    std::vector<int16_t> monoBlock = MakeS16Block(sampleCount, mixerMono, upmixAmplitude);
    std::vector<int16_t> stereoOut(static_cast<size_t>(sampleCount) * mixerStereo, 0);
    ASSERT_EQ(AudioPcmConvertChannels(AUDIO_PCM_S16_LE, mixerMono, mixerStereo, monoBlock.data(),
        sampleCount, stereoOut.data()), AUDIO_PCM_ALGO_SUCCESS);
    EXPECT_EQ(stereoOut[0], upmixAmplitude);
    EXPECT_EQ(stereoOut[1], upmixAmplitude);
    EXPECT_EQ(stereoOut[sampleCount * mixerStereo - 1], upmixAmplitude);

    // stereo -> mono: each L/R pair averaged, exact in the float domain
    std::vector<int16_t> stereoBlock(static_cast<size_t>(sampleCount) * mixerStereo, 0);
    for (int32_t i = 0; i < sampleCount; ++i) {
        stereoBlock[mixerStereo * i] = leftAmplitude;
        stereoBlock[mixerStereo * i + 1] = rightAmplitude;
    }
    std::vector<int16_t> monoOut(sampleCount, 0);
    ASSERT_EQ(AudioPcmConvertChannels(AUDIO_PCM_S16_LE, mixerStereo, mixerMono, stereoBlock.data(),
        sampleCount, monoOut.data()), AUDIO_PCM_ALGO_SUCCESS);
    EXPECT_EQ(monoOut[0], downmixExpected);
    EXPECT_EQ(monoOut[sampleCount - 1], downmixExpected);

    // S24 round trip keeps the sample value in both duplicated channels
    std::vector<uint8_t> s24In(static_cast<size_t>(sampleCount) * mixerMono * s24BytesPerSample, 0);
    for (size_t i = 0; i < s24In.size(); i += s24BytesPerSample) {
        s24In[i + s24MsbIndex] = 0x20; // 0x200000 = 0.25 full scale
    }
    std::vector<uint8_t> s24Out(static_cast<size_t>(sampleCount) * mixerStereo * s24BytesPerSample, 0);
    ASSERT_EQ(AudioPcmConvertChannels(AUDIO_PCM_S24_LE, mixerMono, mixerStereo, s24In.data(),
        sampleCount, s24Out.data()), AUDIO_PCM_ALGO_SUCCESS);
    uint32_t left = static_cast<uint32_t>(s24Out[s24MsbIndex]) << s24Bit16 |
        static_cast<uint32_t>(s24Out[1]) << s24Bit8 | static_cast<uint32_t>(s24Out[0]);
    uint32_t right = static_cast<uint32_t>(s24Out[s24BytesPerSample * mixerStereo + s24MsbIndex]) << s24Bit16 |
        static_cast<uint32_t>(s24Out[s24BytesPerSample * mixerStereo + 1]) << s24Bit8 |
        static_cast<uint32_t>(s24Out[s24BytesPerSample * mixerStereo]);
    EXPECT_EQ(left, 0x200000u);
    EXPECT_EQ(right, 0x200000u);

    // invalid: same channels, unsupported channels, null data, non-positive count
    EXPECT_EQ(AudioPcmConvertChannels(AUDIO_PCM_S16_LE, mixerMono, mixerMono, monoBlock.data(),
        sampleCount, stereoOut.data()), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(AudioPcmConvertChannels(AUDIO_PCM_S16_LE, badChannels, mixerMono, monoBlock.data(),
        sampleCount, stereoOut.data()), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(AudioPcmConvertChannels(AUDIO_PCM_S16_LE, mixerMono, mixerStereo, nullptr,
        sampleCount, stereoOut.data()), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(AudioPcmConvertChannels(AUDIO_PCM_S16_LE, mixerMono, mixerStereo, monoBlock.data(),
        0, stereoOut.data()), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
}