/*
 * Host build test for hpae_format_convert.
 * Runs natively on x86_64 without OHOS runtime.
 */

#include <gtest/gtest.h>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstring>

// Host stubs provide HWTEST_F compatibility macro
#include "audio_engine_log.h"

// Include source directly (stubs/ provides audio_stream_info.h and securec.h)
#include "hpae_format_convert.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr unsigned TEST_SAMPLE_COUNT = 256;
}

class HpaeFormatConvertHostTest : public testing::Test {
public:
    void SetUp() override
    {
        srcBuf_.resize(TEST_SAMPLE_COUNT);
        dstBuf_.resize(TEST_SAMPLE_COUNT);
    }
    void TearDown() override {}
    std::vector<float> srcBuf_;
    std::vector<float> dstBuf_;
};

// ConvertToFloat: U8
HWTEST_F(HpaeFormatConvertHostTest, ConvertToFloat_U8, TestSize.Level0)
{
    std::vector<uint8_t> u8Src(TEST_SAMPLE_COUNT);
    for (unsigned i = 0; i < TEST_SAMPLE_COUNT; i++) {
        u8Src[i] = static_cast<uint8_t>(i);
    }
    ConvertToFloat(SAMPLE_U8, TEST_SAMPLE_COUNT, u8Src.data(), dstBuf_.data());
    EXPECT_GE(dstBuf_[0], -1.0f);
    EXPECT_LE(dstBuf_[0], 1.0f);
    EXPECT_GE(dstBuf_[TEST_SAMPLE_COUNT - 1], -1.0f);
    EXPECT_LE(dstBuf_[TEST_SAMPLE_COUNT - 1], 1.0f);
}

// ConvertToFloat: S16LE
HWTEST_F(HpaeFormatConvertHostTest, ConvertToFloat_S16LE, TestSize.Level0)
{
    std::vector<int16_t> s16Src(TEST_SAMPLE_COUNT);
    for (unsigned i = 0; i < TEST_SAMPLE_COUNT; i++) {
        s16Src[i] = static_cast<int16_t>(i - 128);
    }
    ConvertToFloat(SAMPLE_S16LE, TEST_SAMPLE_COUNT, s16Src.data(), dstBuf_.data());
    EXPECT_GT(dstBuf_[0], -1.0f);
    EXPECT_LT(dstBuf_[0], 1.0f);
}

// ConvertToFloat: S32LE
HWTEST_F(HpaeFormatConvertHostTest, ConvertToFloat_S32LE, TestSize.Level0)
{
    std::vector<int32_t> s32Src(TEST_SAMPLE_COUNT);
    for (unsigned i = 0; i < TEST_SAMPLE_COUNT; i++) {
        s32Src[i] = static_cast<int32_t>(i) - 128;
    }
    ConvertToFloat(SAMPLE_S32LE, TEST_SAMPLE_COUNT, s32Src.data(), dstBuf_.data());
    EXPECT_GT(dstBuf_[0], -1.0f);
    EXPECT_LT(dstBuf_[0], 1.0f);
}

// ConvertToFloat: F32LE (default path)
HWTEST_F(HpaeFormatConvertHostTest, ConvertToFloat_F32LE, TestSize.Level0)
{
    srcBuf_[0] = 0.5f;
    ConvertToFloat(SAMPLE_F32LE, TEST_SAMPLE_COUNT, srcBuf_.data(), dstBuf_.data());
    EXPECT_FLOAT_EQ(dstBuf_[0], 0.5f);
}

// ConvertToFloat: null src
HWTEST_F(HpaeFormatConvertHostTest, ConvertToFloat_NullSrc, TestSize.Level0)
{
    ConvertToFloat(SAMPLE_U8, TEST_SAMPLE_COUNT, nullptr, dstBuf_.data());
    // No crash
}

// ConvertToFloat: null dst
HWTEST_F(HpaeFormatConvertHostTest, ConvertToFloat_NullDst, TestSize.Level0)
{
    std::vector<uint8_t> u8Src(TEST_SAMPLE_COUNT, 128);
    ConvertToFloat(SAMPLE_U8, TEST_SAMPLE_COUNT, u8Src.data(), nullptr);
    // No crash
}

// ConvertFromFloat: U8
HWTEST_F(HpaeFormatConvertHostTest, ConvertFromFloat_U8, TestSize.Level0)
{
    srcBuf_[0] = 0.0f;
    ConvertFromFloat(SAMPLE_U8, TEST_SAMPLE_COUNT, srcBuf_.data(), dstBuf_.data());
    auto *u8Result = reinterpret_cast<uint8_t *>(dstBuf_.data());
    // 0.0f should map to 128 (center of U8 range)
    EXPECT_EQ(u8Result[0], 128);
}

// ConvertFromFloat: S16LE
HWTEST_F(HpaeFormatConvertHostTest, ConvertFromFloat_S16LE, TestSize.Level0)
{
    srcBuf_[0] = 0.0f;
    ConvertFromFloat(SAMPLE_S16LE, TEST_SAMPLE_COUNT, srcBuf_.data(), dstBuf_.data());
    auto *s16Result = reinterpret_cast<int16_t *>(dstBuf_.data());
    EXPECT_EQ(s16Result[0], 0);
}

// ConvertFromFloat: null src
HWTEST_F(HpaeFormatConvertHostTest, ConvertFromFloat_NullSrc, TestSize.Level0)
{
    ConvertFromFloat(SAMPLE_U8, TEST_SAMPLE_COUNT, nullptr, dstBuf_.data());
    // No crash
}

// ConvertFromFloat: null dst
HWTEST_F(HpaeFormatConvertHostTest, ConvertFromFloat_NullDst, TestSize.Level0)
{
    ConvertFromFloat(SAMPLE_U8, TEST_SAMPLE_COUNT, srcBuf_.data(), nullptr);
    // No crash
}

// RoundTrip: S16LE
HWTEST_F(HpaeFormatConvertHostTest, RoundTrip_S16LE, TestSize.Level0)
{
    std::vector<int16_t> original(TEST_SAMPLE_COUNT);
    for (unsigned i = 0; i < TEST_SAMPLE_COUNT; i++) {
        original[i] = static_cast<int16_t>(i * 100 - 10000);
    }
    std::vector<float> floatBuf(TEST_SAMPLE_COUNT);
    std::vector<int16_t> result(TEST_SAMPLE_COUNT);

    ConvertToFloat(SAMPLE_S16LE, TEST_SAMPLE_COUNT, original.data(), floatBuf.data());
    ConvertFromFloat(SAMPLE_S16LE, TEST_SAMPLE_COUNT, floatBuf.data(), result.data());

    for (unsigned i = 0; i < TEST_SAMPLE_COUNT; i++) {
        EXPECT_EQ(result[i], original[i]);
    }
}

// RoundTrip: F32LE (identity)
HWTEST_F(HpaeFormatConvertHostTest, RoundTrip_F32LE, TestSize.Level0)
{
    srcBuf_[0] = 0.25f;
    srcBuf_[1] = -0.75f;
    std::vector<float> result(TEST_SAMPLE_COUNT);
    ConvertToFloat(SAMPLE_F32LE, TEST_SAMPLE_COUNT, srcBuf_.data(), result.data());
    ConvertFromFloat(SAMPLE_F32LE, TEST_SAMPLE_COUNT, result.data(), dstBuf_.data());
    EXPECT_FLOAT_EQ(dstBuf_[0], 0.25f);
    EXPECT_FLOAT_EQ(dstBuf_[1], -0.75f);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
