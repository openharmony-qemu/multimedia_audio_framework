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

#include "gtest/gtest.h"
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>

#include "hpae_format_convert.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

// Internal constants and helpers from hpae_format_convert.cpp needed for testing
static constexpr float FLOAT_EPS = 1e-6f;
static constexpr int BIT_DEPTH_TWO = 2;
static constexpr int BIT_8 = 8;
static constexpr int BIT_16 = 16;
static constexpr int BIT_32 = 32;

static uint32_t Read24Bit(const uint8_t *p)
{
    return (static_cast<uint32_t>(p[BIT_DEPTH_TWO]) << BIT_16) |
           (static_cast<uint32_t>(p[1]) << BIT_8) |
           static_cast<uint32_t>(p[0]);
}

static void Write24Bit(uint8_t *p, uint32_t u)
{
    p[BIT_DEPTH_TWO] = static_cast<uint8_t>(u >> BIT_16);
    p[1] = static_cast<uint8_t>(u >> BIT_8);
    p[0] = static_cast<uint8_t>(u);
}

static void ConvertFromU8ToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--, a++, b++) {
        *b = static_cast<float>(*a - static_cast<uint8_t>(0x80U)) * (1.0 / 0x80U);
    }
}

static void ConvertFrom16BitToFloat(unsigned n, const int16_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1 << (BIT_16 - 1)));
    }
}

static void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b)
{
    constexpr int OFFSET_BIT_24 = 3;
    for (; n > 0; n--) {
        int32_t s = Read24Bit(a) << BIT_8;
        *b = s * (1.0f / (1U << (BIT_32 - 1)));
        a += OFFSET_BIT_24;
        b++;
    }
}

static void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << (BIT_32 - 1)));
    }
}

static constexpr uint32_t TEST_FRAMES = 4;
static constexpr uint32_t NUM_TWO = 2;
static constexpr uint32_t NUM_THREE = 3;
static constexpr uint32_t NUM_SIX = 6;
static constexpr uint32_t NUM_NINE = 9;
static constexpr uint32_t SHIFT_BIT_WIDTH_8 = 8;
static constexpr uint32_t SHIFT_BIT_WIDTH_16 = 16;
static constexpr uint32_t FLOAT_SAMPLE_FORMAT = 99;

class HpaeFormatConvertTest : public ::testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : ConvertToFloatU8
 * @tc.type  : FUNC
 * @tc.number: ConvertToFloat_001
 * @tc.desc  : Test ConvertToFloat with SAMPLE_U8 format.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertToFloat_001, TestSize.Level0)
{
    uint8_t u8Data[] = {0x80, 0xFF, 0x00, 0xC0};
    float output[TEST_FRAMES] = {0};
    ConvertToFloat(SAMPLE_U8, TEST_FRAMES, u8Data, output);
    EXPECT_FLOAT_EQ(output[0], 0.0f);
    EXPECT_FLOAT_EQ(output[1], 127.0f / 128.0f);
    EXPECT_FLOAT_EQ(output[NUM_TWO], -1.0f);
    EXPECT_FLOAT_EQ(output[NUM_THREE], 0.5f);
}

/**
 * @tc.name  : ConvertToFloatS16LE
 * @tc.type  : FUNC
 * @tc.number: ConvertToFloat_002
 * @tc.desc  : Test ConvertToFloat with SAMPLE_S16LE format.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertToFloat_002, TestSize.Level0)
{
    int16_t s16Data[] = {0, INT16_MAX, INT16_MIN, 12345};
    float output[TEST_FRAMES] = {0};
    ConvertToFloat(SAMPLE_S16LE, TEST_FRAMES, s16Data, output);
    EXPECT_FLOAT_EQ(output[0], 0.0f);
    EXPECT_FLOAT_EQ(output[1], static_cast<float>(INT16_MAX) / 32768.0f);
    EXPECT_FLOAT_EQ(output[NUM_TWO], -1.0f);
    EXPECT_NEAR(output[NUM_THREE], 12345.0f / 32768.0f, 1e-6f);
}

/**
 * @tc.name  : ConvertToFloatS24LE
 * @tc.type  : FUNC
 * @tc.number: ConvertToFloat_003
 * @tc.desc  : Test ConvertToFloat with SAMPLE_S24LE format.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertToFloat_003, TestSize.Level0)
{
    uint8_t s24Data[NUM_SIX] = {0};
    Write24Bit(s24Data, 0x7FFFFF);
    Write24Bit(s24Data + NUM_THREE, 0x800000);
    float output[NUM_TWO] = {0};
    ConvertToFloat(SAMPLE_S24LE, NUM_TWO, s24Data, output);
    EXPECT_FLOAT_EQ(output[0], 8388607.0f / 8388608.0f);
    EXPECT_FLOAT_EQ(output[1], -1.0f);
}

/**
 * @tc.name  : ConvertToFloatS32LE
 * @tc.type  : FUNC
 * @tc.number: ConvertToFloat_004
 * @tc.desc  : Test ConvertToFloat with SAMPLE_S32LE format.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertToFloat_004, TestSize.Level0)
{
    int32_t s32Data[] = {0, INT32_MAX, INT32_MIN, 123456789};
    float output[TEST_FRAMES] = {0};
    ConvertToFloat(SAMPLE_S32LE, TEST_FRAMES, s32Data, output);
    EXPECT_FLOAT_EQ(output[0], 0.0f);
    EXPECT_FLOAT_EQ(output[1], static_cast<float>(INT32_MAX) / 2147483648.0f);
    EXPECT_FLOAT_EQ(output[NUM_TWO], -1.0f);
    EXPECT_NEAR(output[NUM_THREE], 123456789.0f / 2147483648.0f, 1e-6f);
}

/**
 * @tc.name  : ConvertToFloatDefaultMemcpy
 * @tc.type  : FUNC
 * @tc.number: ConvertToFloat_005
 * @tc.desc  : Test ConvertToFloat with default format (memcpy fallback).
 */
HWTEST_F(HpaeFormatConvertTest, ConvertToFloat_005, TestSize.Level0)
{
    float floatData[] = {0.0f, 0.5f, -0.5f, 1.0f};
    float output[TEST_FRAMES] = {0};
    ConvertToFloat(static_cast<AudioSampleFormat>(FLOAT_SAMPLE_FORMAT), TEST_FRAMES, floatData, output);
    for (size_t i = 0; i < TEST_FRAMES; ++i) {
        EXPECT_FLOAT_EQ(output[i], floatData[i]);
    }
}

/**
 * @tc.name  : ConvertToFloatNullSrc
 * @tc.type  : FUNC
 * @tc.number: ConvertToFloat_006
 * @tc.desc  : Test ConvertToFloat with null src pointer returns early without crash.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertToFloat_006, TestSize.Level0)
{
    float output[TEST_FRAMES] = {0};
    ConvertToFloat(SAMPLE_S16LE, TEST_FRAMES, nullptr, output);
    EXPECT_FLOAT_EQ(output[0], 0.0f);
}

/**
 * @tc.name  : ConvertToFloatNullDst
 * @tc.type  : FUNC
 * @tc.number: ConvertToFloat_007
 * @tc.desc  : Test ConvertToFloat with null dst pointer returns early without crash.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertToFloat_007, TestSize.Level0)
{
    int16_t s16Data[] = {0, 1, 2, 3};
    ConvertToFloat(SAMPLE_S16LE, TEST_FRAMES, s16Data, nullptr);
}

/**
 * @tc.name  : ConvertToFloatBothNull
 * @tc.type  : FUNC
 * @tc.number: ConvertToFloat_008
 * @tc.desc  : Test ConvertToFloat with both null pointers returns early without crash.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertToFloat_008, TestSize.Level0)
{
    ConvertToFloat(SAMPLE_S16LE, TEST_FRAMES, nullptr, nullptr);
}

/**
 * @tc.name  : ConvertFromFloatU8
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloat_001
 * @tc.desc  : Test ConvertFromFloat with SAMPLE_U8 format.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertFromFloat_001, TestSize.Level0)
{
    float testFloat[TEST_FRAMES] = {0.0f, 0.5f, -0.5f, 1.0f - FLOAT_EPS};
    uint8_t u8Output[TEST_FRAMES] = {0};
    ConvertFromFloat(SAMPLE_U8, TEST_FRAMES, testFloat, u8Output);
    EXPECT_EQ(u8Output[0], 128);
    EXPECT_EQ(u8Output[1], 191);
    EXPECT_EQ(u8Output[NUM_TWO], 64);
    EXPECT_EQ(u8Output[NUM_THREE], 254);
}

/**
 * @tc.name  : ConvertFromFloatS16LE
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloat_002
 * @tc.desc  : Test ConvertFromFloat with SAMPLE_S16LE format.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertFromFloat_002, TestSize.Level0)
{
    float testFloat[TEST_FRAMES] = {0.0f, 0.5f, -0.5f, 1.0f - FLOAT_EPS};
    int16_t s16Output[TEST_FRAMES] = {0};
    ConvertFromFloat(SAMPLE_S16LE, TEST_FRAMES, testFloat, s16Output);
    EXPECT_EQ(s16Output[0], 0);
    EXPECT_EQ(s16Output[1], 16384);
    EXPECT_EQ(s16Output[NUM_TWO], -16384);
    EXPECT_EQ(s16Output[NUM_THREE], 32767);
}

/**
 * @tc.name  : ConvertFromFloatS24LE
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloat_003
 * @tc.desc  : Test ConvertFromFloat with SAMPLE_S24LE format.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertFromFloat_003, TestSize.Level0)
{
    float testFloat[NUM_TWO] = {0.0f, 0.5f};
    uint8_t s24Output[NUM_THREE * NUM_TWO] = {0};
    ConvertFromFloat(SAMPLE_S24LE, NUM_TWO, testFloat, s24Output);
    int32_t val0 = (s24Output[NUM_TWO] << SHIFT_BIT_WIDTH_16) |
                   (s24Output[1] << SHIFT_BIT_WIDTH_8) |
                   (s24Output[0]);
    EXPECT_EQ(val0, 0);
    int32_t val1 = (s24Output[NUM_THREE + NUM_TWO] << SHIFT_BIT_WIDTH_16) |
                   (s24Output[NUM_THREE + 1] << SHIFT_BIT_WIDTH_8) |
                   (s24Output[NUM_THREE + 0]);
    EXPECT_NEAR(val1, 1 << 22, 16);
}

/**
 * @tc.name  : ConvertFromFloatS32LE
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloat_004
 * @tc.desc  : Test ConvertFromFloat with SAMPLE_S32LE format.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertFromFloat_004, TestSize.Level0)
{
    float testFloat[TEST_FRAMES] = {0.0f, 0.5f, -0.5f, 1.0f - FLOAT_EPS};
    int32_t s32Output[TEST_FRAMES] = {0};
    ConvertFromFloat(SAMPLE_S32LE, TEST_FRAMES, testFloat, s32Output);
    EXPECT_EQ(s32Output[0], 0);
    EXPECT_EQ(s32Output[1], 1073741824);
    EXPECT_EQ(s32Output[NUM_TWO], -1073741824);
    EXPECT_EQ(s32Output[NUM_THREE], 2147481472);
}

/**
 * @tc.name  : ConvertFromFloatDefaultMemcpy
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloat_005
 * @tc.desc  : Test ConvertFromFloat with default format (memcpy fallback).
 */
HWTEST_F(HpaeFormatConvertTest, ConvertFromFloat_005, TestSize.Level0)
{
    float floatData[] = {0.5f, 0.6f, 0.7f, 0.8f};
    float output[TEST_FRAMES] = {0};
    ConvertFromFloat(static_cast<AudioSampleFormat>(FLOAT_SAMPLE_FORMAT), TEST_FRAMES, floatData, output);
    EXPECT_FLOAT_EQ(output[0], 0.5f);
    EXPECT_FLOAT_EQ(output[1], 0.6f);
    EXPECT_FLOAT_EQ(output[NUM_TWO], 0.7f);
    EXPECT_FLOAT_EQ(output[NUM_THREE], 0.8f);
}

/**
 * @tc.name  : ConvertFromFloatNullSrc
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloat_006
 * @tc.desc  : Test ConvertFromFloat with null src pointer returns early without crash.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertFromFloat_006, TestSize.Level0)
{
    float output = 0.0f;
    ConvertFromFloat(SAMPLE_S16LE, 1, nullptr, &output);
    EXPECT_FLOAT_EQ(output, 0.0f);
}

/**
 * @tc.name  : ConvertFromFloatNullDst
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloat_007
 * @tc.desc  : Test ConvertFromFloat with null dst pointer returns early without crash.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertFromFloat_007, TestSize.Level0)
{
    float src = 0.5f;
    ConvertFromFloat(SAMPLE_S16LE, 1, &src, nullptr);
}

/**
 * @tc.name  : ConvertFromFloatBothNull
 * @tc.type  : FUNC
 * @tc.number: ConvertFromFloat_008
 * @tc.desc  : Test ConvertFromFloat with both null pointers returns early without crash.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertFromFloat_008, TestSize.Level0)
{
    ConvertFromFloat(SAMPLE_S16LE, 1, nullptr, nullptr);
}

/**
 * @tc.name  : RoundTripU8
 * @tc.type  : FUNC
 * @tc.number: RoundTrip_001
 * @tc.desc  : Test round-trip conversion for SAMPLE_U8 format.
 */
HWTEST_F(HpaeFormatConvertTest, RoundTrip_001, TestSize.Level0)
{
    uint8_t original[] = {0x40, 0x80, 0xC0, 0xFF};
    float intermediate[TEST_FRAMES] = {0};
    uint8_t result[TEST_FRAMES] = {0};
    ConvertToFloat(SAMPLE_U8, TEST_FRAMES, original, intermediate);
    ConvertFromFloat(SAMPLE_U8, TEST_FRAMES, intermediate, result);
    for (size_t i = 0; i < TEST_FRAMES; ++i) {
        EXPECT_NEAR(result[i], original[i], 1);
    }
}

/**
 * @tc.name  : RoundTripS16LE
 * @tc.type  : FUNC
 * @tc.number: RoundTrip_002
 * @tc.desc  : Test round-trip conversion for SAMPLE_S16LE format.
 */
HWTEST_F(HpaeFormatConvertTest, RoundTrip_002, TestSize.Level0)
{
    int16_t original[] = {0, 10000, -10000, 32000};
    float intermediate[TEST_FRAMES] = {0};
    int16_t result[TEST_FRAMES] = {0};
    ConvertToFloat(SAMPLE_S16LE, TEST_FRAMES, original, intermediate);
    ConvertFromFloat(SAMPLE_S16LE, TEST_FRAMES, intermediate, result);
    for (size_t i = 0; i < TEST_FRAMES; ++i) {
        EXPECT_EQ(result[i], original[i]);
    }
}

/**
 * @tc.name  : RoundTripS24LE
 * @tc.type  : FUNC
 * @tc.number: RoundTrip_003
 * @tc.desc  : Test round-trip conversion for SAMPLE_S24LE format.
 */
HWTEST_F(HpaeFormatConvertTest, RoundTrip_003, TestSize.Level0)
{
    uint8_t original[NUM_THREE * NUM_TWO] = {0};
    Write24Bit(original, 0x123456);
    Write24Bit(original + NUM_THREE, 0xABCDEF);
    float intermediate[NUM_TWO] = {0};
    uint8_t result[NUM_THREE * NUM_TWO] = {0};
    ConvertToFloat(SAMPLE_S24LE, NUM_TWO, original, intermediate);
    ConvertFromFloat(SAMPLE_S24LE, NUM_TWO, intermediate, result);
    for (size_t i = 0; i < NUM_THREE * NUM_TWO; ++i) {
        EXPECT_EQ(result[i], original[i]);
    }
}

/**
 * @tc.name  : RoundTripS32LE
 * @tc.type  : FUNC
 * @tc.number: RoundTrip_004
 * @tc.desc  : Test round-trip conversion for SAMPLE_S32LE format.
 */
HWTEST_F(HpaeFormatConvertTest, RoundTrip_004, TestSize.Level0)
{
    int32_t original[] = {0, 100000000, -100000000, 2147483647};
    float intermediate[TEST_FRAMES] = {0};
    int32_t result[TEST_FRAMES] = {0};
    ConvertToFloat(SAMPLE_S32LE, TEST_FRAMES, original, intermediate);
    ConvertFromFloat(SAMPLE_S32LE, TEST_FRAMES, intermediate, result);
    EXPECT_EQ(result[0], original[0]);
    EXPECT_EQ(result[1], original[1]);
    EXPECT_EQ(result[NUM_TWO], original[NUM_TWO]);
}

/**
 * @tc.name  : RoundTripF32LE
 * @tc.type  : FUNC
 * @tc.number: RoundTrip_005
 * @tc.desc  : Test round-trip conversion for F32LE (default memcpy) format.
 */
HWTEST_F(HpaeFormatConvertTest, RoundTrip_005, TestSize.Level0)
{
    float original[] = {0.25f, -0.75f, 0.0f, 0.999f};
    float intermediate[TEST_FRAMES] = {0};
    float result[TEST_FRAMES] = {0};
    ConvertToFloat(static_cast<AudioSampleFormat>(FLOAT_SAMPLE_FORMAT), TEST_FRAMES, original, intermediate);
    ConvertFromFloat(static_cast<AudioSampleFormat>(FLOAT_SAMPLE_FORMAT), TEST_FRAMES, intermediate, result);
    for (size_t i = 0; i < TEST_FRAMES; ++i) {
        EXPECT_FLOAT_EQ(result[i], original[i]);
    }
}

/**
 * @tc.name  : CapMaxPositiveBoundary
 * @tc.type  : FUNC
 * @tc.number: CapMax_001
 * @tc.desc  : Test CapMax clamps values >= 1.0 to 1.0 - FLOAT_EPS.
 */
HWTEST_F(HpaeFormatConvertTest, CapMax_001, TestSize.Level0)
{
    float input = 1.0f;
    uint8_t u8Output = 0;
    ConvertFromFloat(SAMPLE_U8, 1, &input, &u8Output);
    EXPECT_EQ(u8Output, 254);

    int16_t s16Output = 0;
    ConvertFromFloat(SAMPLE_S16LE, 1, &input, &s16Output);
    EXPECT_EQ(s16Output, 32767);

    int32_t s32Output = 0;
    ConvertFromFloat(SAMPLE_S32LE, 1, &input, &s32Output);
    EXPECT_EQ(s32Output, 2147481472);
}

/**
 * @tc.name  : CapMaxNegativeBoundary
 * @tc.type  : FUNC
 * @tc.number: CapMax_002
 * @tc.desc  : Test CapMax clamps values <= -1.0 to -1.0 + FLOAT_EPS.
 */
HWTEST_F(HpaeFormatConvertTest, CapMax_002, TestSize.Level0)
{
    float input = -1.0f;
    int16_t s16Output = 0;
    ConvertFromFloat(SAMPLE_S16LE, 1, &input, &s16Output);
    EXPECT_EQ(s16Output, -32767);

    int32_t s32Output = 0;
    ConvertFromFloat(SAMPLE_S32LE, 1, &input, &s32Output);
    EXPECT_EQ(s32Output, -2147481472);
}

/**
 * @tc.name  : CapMaxLargePositive
 * @tc.type  : FUNC
 * @tc.number: CapMax_003
 * @tc.desc  : Test CapMax clamps large positive values to 1.0 - FLOAT_EPS.
 */
HWTEST_F(HpaeFormatConvertTest, CapMax_003, TestSize.Level0)
{
    float input = 2.0f;
    uint8_t u8Output = 0;
    ConvertFromFloat(SAMPLE_U8, 1, &input, &u8Output);
    EXPECT_EQ(u8Output, 254);

    int16_t s16Output = 0;
    ConvertFromFloat(SAMPLE_S16LE, 1, &input, &s16Output);
    EXPECT_EQ(s16Output, 32767);
}

/**
 * @tc.name  : CapMaxLargeNegative
 * @tc.type  : FUNC
 * @tc.number: CapMax_004
 * @tc.desc  : Test CapMax clamps large negative values to -1.0 + FLOAT_EPS.
 */
HWTEST_F(HpaeFormatConvertTest, CapMax_004, TestSize.Level0)
{
    float input = -2.0f;
    int16_t s16Output = 0;
    ConvertFromFloat(SAMPLE_S16LE, 1, &input, &s16Output);
    EXPECT_EQ(s16Output, -32767);

    int32_t s32Output = 0;
    ConvertFromFloat(SAMPLE_S32LE, 1, &input, &s32Output);
    EXPECT_EQ(s32Output, -2147481472);
}

/**
 * @tc.name  : Convert24BitFullScale
 * @tc.type  : FUNC
 * @tc.number: Convert24Bit_001
 * @tc.desc  : Test 24-bit full scale positive and negative values.
 */
HWTEST_F(HpaeFormatConvertTest, Convert24Bit_001, TestSize.Level0)
{
    uint8_t s24Input[12] = {0};
    Write24Bit(s24Input, 0x7FFFFF);
    Write24Bit(s24Input + NUM_THREE, 0x800000);
    Write24Bit(s24Input + NUM_SIX, 0x123456);
    Write24Bit(s24Input + NUM_NINE, 0xABCDEF);
    float floatOutput[TEST_FRAMES] = {0};
    ConvertFrom24BitToFloat(TEST_FRAMES, s24Input, floatOutput);
    EXPECT_FLOAT_EQ(floatOutput[0], 8388607.0f / 8388608.0f);
    EXPECT_FLOAT_EQ(floatOutput[1], -1.0f);
}

/**
 * @tc.name  : ConvertU8ZeroAndExtremes
 * @tc.type  : FUNC
 * @tc.number: ConvertU8_001
 * @tc.desc  : Test U8 conversion with zero, min, and max values.
 */
HWTEST_F(HpaeFormatConvertTest, ConvertU8_001, TestSize.Level0)
{
    uint8_t u8Data[] = {0x80, 0x00, 0xFF};
    float output[NUM_THREE] = {0};
    ConvertFromU8ToFloat(NUM_THREE, u8Data, output);
    EXPECT_FLOAT_EQ(output[0], 0.0f);
    EXPECT_FLOAT_EQ(output[1], -1.0f);
    EXPECT_FLOAT_EQ(output[NUM_TWO], 127.0f / 128.0f);
}

/**
 * @tc.name  : Convert16BitZeroAndExtremes
 * @tc.type  : FUNC
 * @tc.number: Convert16Bit_001
 * @tc.desc  : Test 16-bit conversion with zero, min, and max values.
 */
HWTEST_F(HpaeFormatConvertTest, Convert16Bit_001, TestSize.Level0)
{
    int16_t s16Data[] = {0, INT16_MAX, INT16_MIN};
    float output[NUM_THREE] = {0};
    ConvertFrom16BitToFloat(NUM_THREE, s16Data, output);
    EXPECT_FLOAT_EQ(output[0], 0.0f);
    EXPECT_FLOAT_EQ(output[1], static_cast<float>(INT16_MAX) / 32768.0f);
    EXPECT_FLOAT_EQ(output[NUM_TWO], -1.0f);
}

/**
 * @tc.name  : Convert32BitZeroAndExtremes
 * @tc.type  : FUNC
 * @tc.number: Convert32Bit_001
 * @tc.desc  : Test 32-bit conversion with zero, min, and max values.
 */
HWTEST_F(HpaeFormatConvertTest, Convert32Bit_001, TestSize.Level0)
{
    int32_t s32Data[] = {0, INT32_MAX, INT32_MIN};
    float output[NUM_THREE] = {0};
    ConvertFrom32BitToFloat(NUM_THREE, s32Data, output);
    EXPECT_FLOAT_EQ(output[0], 0.0f);
    EXPECT_FLOAT_EQ(output[1], static_cast<float>(INT32_MAX) / 2147483648.0f);
    EXPECT_FLOAT_EQ(output[NUM_TWO], -1.0f);
}

/**
 * @tc.name  : ReadWrite24BitRoundTrip
 * @tc.type  : FUNC
 * @tc.number: ReadWrite24Bit_001
 * @tc.desc  : Test Read24Bit and Write24Bit round-trip consistency.
 */
HWTEST_F(HpaeFormatConvertTest, ReadWrite24Bit_001, TestSize.Level0)
{
    uint8_t data[NUM_THREE] = {0xAA, 0xBB, 0xCC};
    uint32_t value = Read24Bit(data);
    EXPECT_EQ(value, 0xCCBBAA);

    uint8_t newData[NUM_THREE] = {0};
    Write24Bit(newData, 0xDDEEFF);
    EXPECT_EQ(newData[0], 0xFF);
    EXPECT_EQ(newData[1], 0xEE);
    EXPECT_EQ(newData[NUM_TWO], 0xDD);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
