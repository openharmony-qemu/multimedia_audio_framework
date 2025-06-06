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
#include "hpae_pcm_dumper.h"
#include "hpae_format_convert.h"
#include "hpae_format_convert.cpp"

#define FLOAT_EPS_24BIT (1.0f / 8388608.0f)

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static std::string g_rootCapturerPath = "/data/source_file_io_48000_2_s16le.pcm";
constexpr uint32_t TEST_FRAMES = 4;
constexpr uint32_t NUM_TWO = 2;
constexpr uint32_t NUM_THREE = 3;
constexpr uint32_t NUM_SIX = 6;
constexpr uint32_t NUM_NINE = 9;
constexpr uint32_t FLOAT_SAMPLE_FORMAT = 99;
constexpr uint32_t SHIFT_BIT_WIDTH_22 = 22;
constexpr uint32_t SHIFT_BIT_WIDTH_16 = 16;
constexpr uint32_t SHIFT_BIT_WIDTH_8 = 8;

class HpaePcmUtilsTest : public ::testing::Test {
protected:
    void SetUp();
    void TearDown();
};

void HpaePcmUtilsTest::SetUp()
{}

void HpaePcmUtilsTest::TearDown()
{}

TEST_F(HpaePcmUtilsTest, pcmDumperValidFile)
{
    HpaePcmDumper dumper(g_rootCapturerPath);
    EXPECT_EQ(dumper.filename_, g_rootCapturerPath);
}

TEST_F(HpaePcmUtilsTest, pcmDumpeInvalidFile)
{
    std::string invalidFilename = "";
    HpaePcmDumper dumper(invalidFilename);
    EXPECT_EQ(dumper.filename_, invalidFilename);
}

TEST_F(HpaePcmUtilsTest, readWrite24Bit)
{
    uint8_t data[NUM_THREE] = {0xAA, 0xBB, 0xCC};
    uint32_t value = Read24Bit(data);
    EXPECT_EQ(value, 0xCCBBAA);

    uint8_t newData[NUM_THREE];
    Write24Bit(newData, 0xDDEEFF);
    EXPECT_EQ(newData[0], 0xFF);
    EXPECT_EQ(newData[1], 0xEE);
    EXPECT_EQ(newData[NUM_TWO], 0xDD);
}

TEST_F(HpaePcmUtilsTest, convertU8ToFloat)
{
    uint8_t u8Data[] = {0x80, 0xFF, 0x00, 0xC0};
    float floatData[TEST_FRAMES];
    ConvertFromU8ToFloat(TEST_FRAMES, u8Data, floatData);
    EXPECT_FLOAT_EQ(floatData[0], 0.0f);
    EXPECT_FLOAT_EQ(floatData[1], 0.9921875f);
    EXPECT_FLOAT_EQ(floatData[NUM_TWO], -1.0f);
    EXPECT_FLOAT_EQ(floatData[NUM_THREE], 0.5f);
}

TEST_F(HpaePcmUtilsTest, boundaryValueHandling)
{
    float boundaryFloat[NUM_TWO] = {
        1.0f + FLOAT_EPS,
        -1.0f - NUM_TWO * FLOAT_EPS
    };
    int16_t s16Data[NUM_TWO];
    ConvertFromFloatTo16Bit(NUM_TWO, boundaryFloat, s16Data);
    EXPECT_EQ(s16Data[0], INT16_MAX);
    EXPECT_EQ(s16Data[1], -32767); // -32767: INT_MIN
}

TEST_F(HpaePcmUtilsTest, convertToFloatAllFormats)
{
    float testFloat[TEST_FRAMES] = {
        0.0f,
        0.5f,
        -0.5f,
        1.0f - FLOAT_EPS
    };
    float output[TEST_FRAMES];
    uint8_t u8Data[] = {128, 255, 0, 192};
    ConvertToFloat(SAMPLE_U8, TEST_FRAMES, u8Data, output);
    int16_t s16Data[] = {0, INT16_MAX, INT16_MIN, 12345};
    ConvertToFloat(SAMPLE_S16LE, TEST_FRAMES, s16Data, output);
    uint8_t s24Data[NUM_THREE * TEST_FRAMES] = {0};
    Write24Bit(s24Data, 0x7FFFFF);
    Write24Bit(s24Data + NUM_THREE, 0x800000);
    ConvertToFloat(SAMPLE_S24LE, NUM_TWO, s24Data, output);
    int32_t s32Data[] = {0, INT32_MAX, INT32_MIN, 123456789};
    ConvertToFloat(SAMPLE_S32LE, TEST_FRAMES, s32Data, output);
    float floatData[TEST_FRAMES];
    ConvertToFloat(static_cast<AudioSampleFormat>(FLOAT_SAMPLE_FORMAT), TEST_FRAMES, testFloat, floatData);
}

TEST_F(HpaePcmUtilsTest, memcpyFallback)
{
    float src[TEST_FRAMES] = {0.1f, 0.2f, 0.3f, 0.4f};
    float dest[1];
    ConvertToFloat(static_cast<AudioSampleFormat>(FLOAT_SAMPLE_FORMAT), TEST_FRAMES, src, dest);
    EXPECT_FLOAT_EQ(dest[0], 0.1f);
    float newSrc[TEST_FRAMES] = {0.5f, 0.6f, 0.7f, 0.8f};
    float newDest[TEST_FRAMES];
    ConvertFromFloat(static_cast<AudioSampleFormat>(FLOAT_SAMPLE_FORMAT), TEST_FRAMES, newSrc, newDest);
    EXPECT_FLOAT_EQ(newDest[0], 0.5f);
    EXPECT_FLOAT_EQ(newDest[1], 0.6f);
    EXPECT_FLOAT_EQ(newDest[NUM_TWO], 0.7f);
    EXPECT_FLOAT_EQ(newDest[NUM_THREE], 0.8f);
}

TEST_F(HpaePcmUtilsTest, bit24SpecialHandling)
{
    uint8_t s24Input[12] = {0}; // 12: inputsize
    float floatOutput[4]; // // 4: outputsize
    Write24Bit(s24Input, 0x7FFFFF);
    Write24Bit(s24Input + NUM_THREE, 0x800000);
    Write24Bit(s24Input + NUM_SIX, 0x123456);
    Write24Bit(s24Input + NUM_NINE, 0xABCDEF);
    ConvertFrom24BitToFloat(4, s24Input, floatOutput); // 4: size
    EXPECT_FLOAT_EQ(floatOutput[0], 1.0f - FLOAT_EPS_24BIT);
    EXPECT_FLOAT_EQ(floatOutput[1], -1.0f);
}

TEST_F(HpaePcmUtilsTest, convertFromFloatAllFormats)
{
    float testFloat[TEST_FRAMES] = {
        0.0f,
        0.5f,
        -0.5f,
        1.0f - FLOAT_EPS
    };
    // U8: [0, 255], 128 is zero
    uint8_t u8Output[TEST_FRAMES];
    ConvertFromFloat(SAMPLE_U8, TEST_FRAMES, testFloat, u8Output);
    EXPECT_EQ(u8Output[0], 128); // 128: 0.0f
    EXPECT_EQ(u8Output[1], 191); // 191: 0.5f
    EXPECT_EQ(u8Output[NUM_TWO], 64);  // 64: -0.5f
    EXPECT_EQ(u8Output[NUM_THREE], 254); // 254: near 1.0f
    // S16LE: [-32768, 32767]
    int16_t s16Output[TEST_FRAMES];
    ConvertFromFloat(SAMPLE_S16LE, TEST_FRAMES, testFloat, s16Output);
    EXPECT_EQ(s16Output[0], 0); // 0.0f
    EXPECT_EQ(s16Output[1], 16384); // 16384: 0.5f * 32768
    EXPECT_EQ(s16Output[NUM_TWO], -16384); // -16384: -0.5f * 32768
    EXPECT_EQ(s16Output[NUM_THREE], 32767); // 32767: near 1.0f
    uint8_t s24Output[NUM_THREE * TEST_FRAMES];
    ConvertFromFloat(SAMPLE_S24LE, TEST_FRAMES, testFloat, s24Output);
    for (int i = 0; i < TEST_FRAMES; ++i) {
        int32_t val = (s24Output[i * NUM_THREE + NUM_TWO] << SHIFT_BIT_WIDTH_16) |
                      (s24Output[i * NUM_THREE + 1] << SHIFT_BIT_WIDTH_8) |
                      (s24Output[i * NUM_THREE + 0]);
        if (val & 0x800000) {
            val |= ~0xFFFFFF;
        }
        if (i == 0) EXPECT_EQ(val, 0);
        if (i == 1) EXPECT_NEAR(val, (1 << SHIFT_BIT_WIDTH_22), 16);  // 16: expected num
        if (i == NUM_TWO) EXPECT_NEAR(val, -(1 << SHIFT_BIT_WIDTH_22), 16); // 16: expected num
        if (i == NUM_THREE) EXPECT_EQ(val, 8388599); // 8388599: expected num
    }
    // S32LE: [-2^31, 2^31-1]
    int32_t s32Output[TEST_FRAMES];
    ConvertFromFloat(SAMPLE_S32LE, TEST_FRAMES, testFloat, s32Output);
    EXPECT_EQ(s32Output[0], 0); // 0.0f
    EXPECT_EQ(s32Output[1], 1073741824); // 1073741824: 0.5f * 2^31
    EXPECT_EQ(s32Output[NUM_TWO], -1073741824); // -1073741824: -0.5f * 2^31
    EXPECT_EQ(s32Output[NUM_THREE], 2147481472); // 2147481472: near 1.0f
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS