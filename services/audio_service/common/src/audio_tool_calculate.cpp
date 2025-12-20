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
#include "audio_tool_calculate.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
namespace OHOS {
namespace AudioStandard {
#if USE_ARM_NEON == 1
const static constexpr uint32_t DEFAULT_OFFSET_3 = 3;
const static constexpr uint32_t DEFAULT_OFFSET_7 = 7;
const static constexpr uint32_t DEFAULT_OFFSET_15 = 15;

const static constexpr uint32_t DEFAULT_STEP_BY_4 = 4;
const static constexpr uint32_t DEFAULT_STEP_BY_8 = 8;
const static constexpr uint32_t DEFAULT_STEP_BY_16 = 16;
const static constexpr uint32_t DEFAULT_STEP_BY_32 = 32;
#endif

const static constexpr int32_t DEFAULT_CHANNEL_COUNT_2 = 2;

inline bool Is16ByteAligned(const void * ptr)
{
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return (address & 0xF) == 0;
}

template <typename T, typename R,
    typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<R>>>
inline std::vector<R> SumPcmAbsNormal(const T *pcm, uint32_t num_samples, int32_t channels, size_t split)
{
    std::vector<R> sum(channels, 0);
    for (uint32_t i = 0; i < num_samples - (split - 1); i += split) {
        for (int32_t j = 0; j < channels; j++) {
            sum[j] += (*pcm >= 0 ? *pcm : -*pcm);
            pcm++;
        }
        pcm += (split - 1) * channels;
    }
    return sum;
}

#if USE_ARM_NEON == 1
inline uint64x2_t Extension32bitTo64bit(int32x4_t value)
{
#ifdef __aarch64__
    return vmovl_high_u32(vreinterpretq_u32_s32(value));
#else
    return vmovl_u32(vget_high_u32(vreinterpretq_u32_s32(value)));
#endif
}

inline uint32x4_t Extension16bitTo32bit(int16x8_t value)
{
#ifdef __aarch64__
    return vmovl_high_u16(vreinterpretq_u16_s16(value));
#else
    return vmovl_u16(vget_high_u16(vreinterpretq_u16_s16(value)));
#endif
}

inline uint16x8_t Extension8bitTo16bit(uint8x16_t value)
{
#ifdef __aarch64__
    return vmovl_high_u8(value);
#else
    return vmovl_u8(vget_high_u8(value));
#endif
}

inline int32_t SafeVaddvqS32(int32x4_t value)
{
#ifdef __aarch64__
    return vaddvq_s32(value);
#else
    int32x2_t sum_pair = vadd_s32(vget_low_s32(value), vget_high_s32(value));
    return vget_lane_s32(vpadd_s32(sum_pair, sum_pair), 0);
#endif
}

inline uint32_t SafeVaddvqU32(uint32x4_t value)
{
#ifdef __aarch64__
    return vaddvq_u32(value);
#else
    uint32x2_t sum_pair = vadd_u32(vget_low_u32(value), vget_high_u32(value));
    return vget_lane_u32(vpadd_u32(sum_pair, sum_pair), 0);
#endif
}

inline float SafeVaddvqF32(float32x4_t value)
{
#ifdef __aarch64__
    return vaddvq_f32(value);
#else
    uint32x2_t sum_pair = vadd_f32(vget_low_f32(value), vget_high_f32(value));
    return vget_lane_f32(vpadd_f32(sum_pair, sum_pair), 0);
#endif
}
#endif

std::vector<int64_t> SumS32SingleAbsNeno(const int32_t* data, uint32_t num_samples)
{
    std::vector<int64_t> sum(1, 0);
#if USE_ARM_NEON == 1
    int64x2_t sum_vec = vdupq_n_s64(0);
    for (uint32_t i = 0; i + DEFAULT_OFFSET_3 < num_samples; i += DEFAULT_STEP_BY_4) {
        int32x4_t v = vld1q_s32(data + i);        // load 4 samples
        int32x4_t abs_v = vabsq_s32(v);           // take absolute values
        int64x2_t pair_sum = vpaddlq_s32(abs_v);  // 4->2 wide accumulation
        sum_vec = vaddq_s64(sum_vec, pair_sum);   // accumulate into sum_vec
    }
    sum[0] = vgetq_lane_s64(sum_vec, 0) + vgetq_lane_s64(sum_vec, 1);
#endif
    return sum;
}

std::vector<int64_t> SumS32StereoAbsNeno(const int32_t* data, uint32_t num_samples)
{
    std::vector<int64_t> sum(DEFAULT_CHANNEL_COUNT_2, 0);
#if USE_ARM_NEON == 1
    uint64x2_t sum_left_64x2 = vdupq_n_u64(0);
    uint64x2_t sum_right_64x2 = vdupq_n_u64(0);
    for (uint32_t i = 0; i + DEFAULT_OFFSET_3 < num_samples; i += DEFAULT_STEP_BY_4) {
        // load and deinterleave 4 stereo samples
        int32x4x2_t samples = vld2q_s32(data);
        data += DEFAULT_STEP_BY_8;

        // calculate absolute values
        int32x4_t left_abs = vabsq_s32(samples.val[0]);
        int32x4_t right_abs = vabsq_s32(samples.val[1]);

        // zero-overhead extension to 64-bit
        uint64x2_t left_low = vmovl_u32(vget_low_u32(vreinterpretq_u32_s32(left_abs)));
        uint64x2_t left_high = Extension32bitTo64bit(left_abs);
        uint64x2_t right_low = vmovl_u32(vget_low_u32(vreinterpretq_u32_s32(right_abs)));
        uint64x2_t right_high = Extension32bitTo64bit(right_abs);

        // accumulate
        sum_left_64x2 = vaddq_u64(sum_left_64x2, left_low);
        sum_left_64x2 = vaddq_u64(sum_left_64x2, left_high);
        sum_right_64x2 = vaddq_u64(sum_right_64x2, right_low);
        sum_right_64x2 = vaddq_u64(sum_right_64x2, right_high);
    }
    sum[0] = vgetq_lane_u64(sum_left_64x2, 0) + vgetq_lane_u64(sum_left_64x2, 1);
    sum[1] = vgetq_lane_u64(sum_right_64x2, 0) + vgetq_lane_u64(sum_right_64x2, 1);
#endif
    return sum;
}

std::vector<int64_t> SumS32AbsNeno(const int32_t* pcm, uint32_t num_samples, int32_t channels)
{
    std::vector<int64_t> sum(channels, 0);
    if (channels == 1) {
        return SumS32SingleAbsNeno(pcm, num_samples);
    } else {
        return SumS32StereoAbsNeno(pcm, num_samples);
    }
    return sum;
}

std::vector<int64_t> AudioToolCalculate::SumAudioS32AbsPcm(const int32_t* pcm, uint32_t num_samples,
    int32_t channels, size_t split)
{
    if (!Is16ByteAligned(pcm) || channels > DEFAULT_CHANNEL_COUNT_2 || split > 1) {
        return SumPcmAbsNormal<int32_t, int64_t>(pcm, num_samples, channels, split);
    }
#if USE_ARM_NEON == 1
    return SumS32AbsNeno(pcm, num_samples, channels);
#else
    return SumPcmAbsNormal<int32_t, int64_t>(pcm, num_samples, channels, split);
#endif
}

std::vector<int32_t> SumS16SingleAbsNeno(const int16_t* pcm, uint32_t num_samples)
{
    std::vector<int32_t> sum(1, 0);
#if USE_ARM_NEON == 1
    int32x4_t sum_vec = vdupq_n_s32(0);  // 32-bit accumulator
    for (uint32_t i = 0; i + DEFAULT_OFFSET_7 <= num_samples; i += DEFAULT_STEP_BY_8) {
    int16x8_t v = vld1q_s16(&pcm[i]);           // load 8 int16 samples
    int16x8_t abs_v = vabsq_s16(v);             // absolute values（S16）
    int32x4_t vabs_lo = vmovl_s16(vget_low_s16(abs_v));  // first 4 samples
    int32x4_t vabs_hi = vmovl_s16(vget_high_s16(abs_v)); // last 4 samples
    sum_vec = vaddq_s32(sum_vec, vabs_lo);
    sum_vec = vaddq_s32(sum_vec, vabs_hi);
    }
    sum[0] = SafeVaddvqS32(sum_vec);
#endif
    return sum;
}

std::vector<int32_t> SumS16StereoAbsNeno(const int16_t* pcm, uint32_t num_samples)
{
    std::vector<int32_t> sum(DEFAULT_CHANNEL_COUNT_2, 0);
#if USE_ARM_NEON == 1
    uint32x4_t sum_left_32x4 = vdupq_n_u32(0);
    uint32x4_t sum_right_32x4 = vdupq_n_u32(0);
    // 8 samples each time
    for (uint32_t i = 0; i + DEFAULT_OFFSET_7 < num_samples; i += DEFAULT_STEP_BY_8) {
        int16x8x2_t samples = vld2q_s16(pcm);
        pcm += DEFAULT_STEP_BY_16;

        // absolute values
        int16x8_t left_abs = vabsq_s16(samples.val[0]);
        int16x8_t right_abs = vabsq_s16(samples.val[1]);

        // zero-overhead extension to 32-bit
        uint32x4_t left_low = vmovl_u16(vget_low_u16(vreinterpretq_u16_s16(left_abs)));
        uint32x4_t left_high = Extension16bitTo32bit(left_abs);
        uint32x4_t right_low = vmovl_u16(vget_low_u16(vreinterpretq_u16_s16(right_abs)));
        uint32x4_t right_high = Extension16bitTo32bit(right_abs);

        // accumulate
        sum_left_32x4 = vaddq_u32(sum_left_32x4, left_low);
        sum_left_32x4 = vaddq_u32(sum_left_32x4, left_high);
        sum_right_32x4 = vaddq_u32(sum_right_32x4, right_low);
        sum_right_32x4 = vaddq_u32(sum_right_32x4, right_high);
    }
    sum[0] = SafeVaddvqU32(sum_left_32x4);
    sum[1] = SafeVaddvqU32(sum_right_32x4);
#endif
    return sum;
}

std::vector<int32_t> SumS16AbsNeno(const int16_t* pcm, uint32_t num_samples, int32_t channels)
{
    std::vector<int32_t> sum(channels, 0);
    if (channels == 1) {
        return SumS16SingleAbsNeno(pcm, num_samples);
    } else {
        return SumS16StereoAbsNeno(pcm, num_samples);
    }
    return sum;
}

std::vector<int32_t> AudioToolCalculate::SumAudioS16AbsPcm(const int16_t* pcm, uint32_t num_samples,
    int32_t channels, size_t split)
{
    if (!Is16ByteAligned(pcm) || channels > DEFAULT_CHANNEL_COUNT_2 || split > 1) {
        return SumPcmAbsNormal<int16_t, int32_t>(pcm, num_samples, channels, split);
    }
#if USE_ARM_NEON == 1
    return SumS16AbsNeno(pcm, num_samples, channels);
#else
    return SumPcmAbsNormal<int16_t, int32_t>(pcm, num_samples, channels, split);
#endif
}

std::vector<int32_t> SumU8SingleNeno(const uint8_t* pcm, uint32_t num_samples)
{
    std::vector<int32_t> sum(1, 0);
#if USE_ARM_NEON == 1
    uint32x4_t acc32 = vdupq_n_u32(0);
    for (uint32_t i = 0; i + DEFAULT_OFFSET_15 < num_samples; i += DEFAULT_STEP_BY_16) {
        uint8x16_t samples = vld1q_u8(pcm);
        // extend 8-bit to 16-bit
        uint16x8_t low = vmovl_u8(vget_low_u8(samples));
        uint16x8_t high = vmovl_u8(vget_high_u8(samples));
        // accumulate into 32-bit vector
        acc32 = vpadalq_u16(acc32, low);
        acc32 = vpadalq_u16(acc32, high);
        pcm += DEFAULT_STEP_BY_16;
    }
    sum[0] = SafeVaddvqU32(acc32);
#endif
    return sum;
}

std::vector<int32_t> SumU8StereoNeno(const uint8_t *data, uint32_t num_samples)
{
    std::vector<int32_t> sum(DEFAULT_CHANNEL_COUNT_2, 0);
#if USE_ARM_NEON == 1
    uint32x4_t sum_left_32x4 = vdupq_n_u32(0);
    uint32x4_t sum_right_32x4 = vdupq_n_u32(0);
    // process 16 samples per iteration
    for (uint32_t i = 0; i + DEFAULT_OFFSET_15 < num_samples; i += DEFAULT_STEP_BY_16) {
        // load and deinterleave 16 stereo samples
        uint8x16x2_t samples = vld2q_u8(data);
        data += DEFAULT_STEP_BY_32;

        // unsigned U8, absolute value is the value itself
        uint16x8_t left_low = vmovl_u8(vget_low_u8(samples.val[0]));
        uint16x8_t left_high = Extension8bitTo16bit(samples.val[0]);
        uint16x8_t right_low = vmovl_u8(vget_low_u8(samples.val[1]));
        uint16x8_t right_high = Extension8bitTo16bit(samples.val[1]);

        // accumulate left channel
        sum_left_32x4 = vaddq_u32(sum_left_32x4, vaddl_u16(vget_low_u16(left_low), vget_high_u16(left_low)));
        sum_left_32x4 = vaddq_u32(sum_left_32x4, vaddl_u16(vget_low_u16(left_high), vget_high_u16(left_high)));
        
        // accumulate right channel
        sum_right_32x4 = vaddq_u32(sum_right_32x4, vaddl_u16(vget_low_u16(right_low), vget_high_u16(right_low)));
        sum_right_32x4 = vaddq_u32(sum_right_32x4, vaddl_u16(vget_low_u16(right_high), vget_high_u16(right_high)));
    }

    // horizontal summation
    sum[0] = SafeVaddvqU32(sum_left_32x4);
    sum[1] = SafeVaddvqU32(sum_right_32x4);
#endif
    return sum;
}

std::vector<int32_t> SumU8AbsNeno(const uint8_t *pcm, uint32_t num_samples, int32_t channels)
{
    std::vector<int32_t> sum(channels, 0);
    if (channels == 1) {
        return SumU8SingleNeno(pcm, num_samples);
    } else {
        return SumU8StereoNeno(pcm, num_samples);
    }
    return sum;
}

std::vector<int32_t> AudioToolCalculate::SumAudioU8AbsPcm(const uint8_t *pcm, uint32_t num_samples,
    int32_t channels, size_t split)
{
    if (!Is16ByteAligned(pcm) || channels > DEFAULT_CHANNEL_COUNT_2 || split > 1) {
        return SumPcmAbsNormal<uint8_t, int32_t>(pcm, num_samples, channels, split);
    }
#if USE_ARM_NEON == 1
    return SumU8AbsNeno(pcm, num_samples, channels);
#else
    return SumPcmAbsNormal<uint8_t, int32_t>(pcm, num_samples, channels, split);
#endif
}

std::vector<float> SumF32SingleAbsNeno(const float *pcm, uint32_t num_samples)
{
    std::vector<float> sum(1, 0);
#if USE_ARM_NEON == 1
    float32x4_t sum_vec = vdupq_n_f32(0.0f);  // initialize accumulator vector to zero
    const uint32_t samples_per_loop = DEFAULT_STEP_BY_8;
    for (uint32_t i = 0; i + samples_per_loop <= num_samples; i += samples_per_loop) {
        float32x4_t v0 = vld1q_f32(&pcm[i]);
        float32x4_t v1 = vld1q_f32(&pcm[i + DEFAULT_STEP_BY_4]);
        sum_vec = vaddq_f32(sum_vec, vabsq_f32(v0));
        sum_vec = vaddq_f32(sum_vec, vabsq_f32(v1));
    }
    // horizontal sum: add all 4 lanes together
    sum[0] = SafeVaddvqF32(sum_vec);
#endif
    return sum;
}

std::vector<float> SumF32StereoAbsNeno(const float *pcm, uint32_t num_samples)
{
    std::vector<float> sum(DEFAULT_CHANNEL_COUNT_2, 0);
#if USE_ARM_NEON == 1
    float32x4_t sum_left_32x4 = vdupq_n_f32(0.0f);
    float32x4_t sum_right_32x4 = vdupq_n_f32(0.0f);
    // process 16 samples per iteration
    for (uint32_t i = 0; i + DEFAULT_OFFSET_3 < num_samples; i += DEFAULT_STEP_BY_4) {
        // load and deinterleave 16 stereo samples
        float32x4x2_t samples = vld2q_f32(pcm);
        pcm += DEFAULT_STEP_BY_8;

        // absolute value
        float32x4_t left_abs = vabsq_f32(samples.val[0]);
        float32x4_t right_abs = vabsq_f32(samples.val[1]);

        // accumulate
        sum_left_32x4 = vaddq_f32(sum_left_32x4, left_abs);
        sum_right_32x4 = vaddq_f32(sum_right_32x4, right_abs);
    }
    // horizontal sum
    float32x2_t sum_left_32x2 = vadd_f32(vget_low_f32(sum_left_32x4), vget_high_f32(sum_left_32x4));
    float32x2_t sum_right_32x2 = vadd_f32(vget_low_f32(sum_right_32x4), vget_high_f32(sum_right_32x4));
    sum[0] = vget_lane_f32(vpadd_f32(sum_left_32x2, sum_left_32x2), 0);
    sum[1] = vget_lane_f32(vpadd_f32(sum_right_32x2, sum_right_32x2), 0);
#endif
    return sum;
}

std::vector<float> SumF32AbsNeno(const float *pcm, uint32_t num_samples, int32_t channels)
{
    std::vector<float> sum(channels, 0);
    if (channels == 1) {
        return SumF32SingleAbsNeno(pcm, num_samples);
    } else {
        return SumF32StereoAbsNeno(pcm, num_samples);
    }
    return sum;
}

std::vector<float> AudioToolCalculate::SumAudioF32AbsPcm(const float *pcm, uint32_t num_samples,
    int32_t channels, size_t split)
{
    if (!Is16ByteAligned(pcm) || channels > DEFAULT_CHANNEL_COUNT_2 || split > 1) {
        return SumPcmAbsNormal<float, float>(pcm, num_samples, channels, split);
    }
#if USE_ARM_NEON == 1
    return SumF32AbsNeno(pcm, num_samples, channels);
#else
    return SumPcmAbsNormal<float, float>(pcm, num_samples, channels, split);
#endif
}
}
}