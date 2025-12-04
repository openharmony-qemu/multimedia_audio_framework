#include "audio_tool_calculate.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
namespace OHOS{
namespace AudioStandard {
#if USE_ARM_NEON == 1
// constexpr int ALIGIN_FLOAT_SIZE = 8;
#endif

inline bool Is16ByteAligned(const void *ptr) {
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return (arrress & 0xF) == 0;
}
inline bool Is16ByteAligned(const void * ptr) {
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    return (address & 0xF) == 0;
}
template <typename T, typename R,
    typename = std::enable_if_t<std::is_arithmetic_v<T> && std::is_arithmetic_v<R>>>
inline std::vector<R> SumPcmAbsNormal(const T *pcm, uint32_t num_samples, int32_t channels, size_t split)
{
    Trace trace("SumPcmAbsNormal");
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
std::vector<int64_t> SumS32SingleAbsNeno(const int32_t* data, uint32_t num_samples)
{
    std::vector<int64_t> sum(1, 0);
#if USE_ARM_NEON == 1
    int64x2_t sum_vec = vdupq_n_s64(0);
    for (uint32_t i = 0; i + 3 < num_samples; i += 4) {
        int32x4_t v = vld1q_s32(data + i);      // 加载4个采样
        int32x4_t abs_v = vabsq_s32(v);           // 取绝对值
        int64x2_t pair_sum = vpaddlq_s32(abs_v);  // 4->2宽累加
        sum_vec = vaddq_s64(sum_vec, pair_sum);   // 累加到sum_vec
    }
    sum[0] = vgetq_lane_s64(sum_vec, 0) + vgetq_lane_s64(sum_vec, 1);
#endif
    return sum;
}
std::vector<int64_t> SumS32StereoAbsNeno(const int32_t* data, uint32_t num_samples)
{
    std::vector<int64_t> sum(2, 0);
#if USE_ARM_NEON == 1
    uint64x2_t sum_left_64x2 = vdupq_n_u64(0);
    uint64x2_t sum_right_64x2 = vdupq_n_u64(0);
    for (uint32_t i = 0; i + 3 < num_samples; i += 4) {
        // 加载并解交错4个样本
        int32x4x2_t samples = vld2q_s32(data);
        data += 8;

        // 计算绝对值
        int32x4_t left_abs = vabsq_s32(samples.val[0]);
        int32x4_t right_abs = vabsq_s32(samples.val[1]);

        // 零开销扩展到64位
        uint64x2_t left_low = vmovl_u32(vget_low_u32(vreinterpretq_u32_s32(left_abs)));
        uint64x2_t left_high = vmovl_high_u32(vreinterpretq_u32_s32(left_abs));
        uint64x2_t right_low = vmovl_u32(vget_low_u32(vreinterpretq_u32_s32(right_abs)));
        uint64x2_t right_high = vmovl_high_u32(vreinterpretq_u32_s32(right_abs));

        // 累加
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
        Trace trace("SumS32SingleAbsNeno");
        return SumS32SingleAbsNeno(pcm, num_samples);
    } else {
        Trace trace("SumS32StereoAbsNeno");
        return SumS32StereoAbsNeno(pcm, num_samples);
    }
    return sum;
}
std::vector<int64_t> AudioToolCalculate::SumAudioS32AbsPcm(const int32_t* pcm, uint32_t num_samples,
    int32_t channels, size_t split)
{
    if (!Is16ByteAligned(pcm) || channels > 2 || split > 1) {
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
    int32x4_t sum_vec = vdupq_n_s32(0);  // 累加器，32位
     for (uint32_t i = 0; i + 7 <= num_samples; i += 8) {
        int16x8_t v = vld1q_s16(&pcm[i]);           // 加载 8 个 int16
        int16x8_t abs_v = vabsq_s16(v);             // 取绝对值（S16）
        int32x4_t vabs_lo = vmovl_s16(vget_low_s16(abs_v));  // 前4
        int32x4_t vabs_hi = vmovl_s16(vget_high_s16(abs_v)); // 后4
        sum_vec = vaddq_s32(sum_vec, vabs_lo);
        sum_vec = vaddq_s32(sum_vec, vabs_hi);
     }
     sum[0] = vaddvq_s32(sum_vec);
#endif
    return sum;
}
std::vector<float> AudioToolCalculate::SumAudioF32AbsPcm(const float *pcm, uint32_t num_samples,
    int32_t channels, size_t split)
{
    if (!Is16ByteAligned(pcm) || channels > 2 || split > 1) {
        return SumPcmAbsNormal<float, float>(pcm, num_samples, channels, split);
    }
#if USE_ARM_NEON == 1
    return SumF32AbsNeno(pcm, num_samples, channels);
#else
    return SumPcmAbsNormal<float, float>(pcm, num_samples, channels, split);
#endif
}
std::vector<int32_t> SumS16StereoAbsNeno(const int16_t* pcm, uint32_t num_samples)
{
    std::vector<int32_t> sum(2, 0);
#if USE_ARM_NEON == 1
     uint32x4_t sum_left_32x4 = vdupq_n_u32(0);
     uint32x4_t sum_right_32x4 = vdupq_n_u32(0);
      // 每次处理8个样本
    for (uint32_t i = 0; i + 7 < num_samples; i += 8) {
        int16x8x2_t samples = vld2q_s16(pcm);
        pcm += 16;

        // 计算绝对值
        int16x8_t left_abs = vabsq_s16(samples.val[0]);
        int16x8_t right_abs = vabsq_s16(samples.val[1]);

        // 零开销扩展到32位
        uint32x4_t left_low = vmovl_u16(vget_low_u16(vreinterpretq_u16_s16(left_abs)));
        uint32x4_t left_high = vmovl_high_u16(vreinterpretq_u16_s16(left_abs));
        uint32x4_t right_low = vmovl_u16(vget_low_u16(vreinterpretq_u16_s16(right_abs)));
        uint32x4_t right_high = vmovl_high_u16(vreinterpretq_u16_s16(right_abs));

        // 累加
        sum_left_32x4 = vaddq_u32(sum_left_32x4, left_low);
        sum_left_32x4 = vaddq_u32(sum_left_32x4, left_high);
        sum_right_32x4 = vaddq_u32(sum_right_32x4, right_low);
        sum_right_32x4 = vaddq_u32(sum_right_32x4, right_high);
     }
     sum[0] = vaddvq_u32(sum_left_32x4);
     sum[1] = vaddvq_u32(sum_right_32x4);
     AUDIO_INFO_LOG("SumS16StereoAbsNeno, sum 0 :%{public}d", sum[0]);
#endif
    return sum;
}
std::vector<int32_t> SumS16AbsNeno(const int16_t* pcm, uint32_t num_samples, int32_t channels)
{
    std::vector<int32_t> sum(channels, 0);
    AUDIO_INFO_LOG("SumS16AbsNeno");
    if (channels == 1) {
        AUDIO_INFO_LOG("SumS16AbsNeno channel 1");
        Trace trace("SumS16SingleAbsNeno");
        return SumS16SingleAbsNeno(pcm, num_samples);
    } else {
        AUDIO_INFO_LOG("SumS16AbsNeno channel 2");
        Trace trace("SumS16StereoAbsNeno");
        return SumS16StereoAbsNeno(pcm, num_samples);
    }
   return sum;
}
std::vector<int32_t> AudioToolCalculate::SumAudioS16AbsPcm(const int16_t* pcm, uint32_t num_samples,
    int32_t channels, size_t split)
{
    AUDIO_INFO_LOG("SumAudioS16AbsPcm1");
    if (!Is16ByteAligned(pcm) || channels > 2 || split > 1) {
        return SumPcmAbsNormal<int16_t, int32_t>(pcm, num_samples, channels, split);
    }
#if USE_ARM_NEON == 1
    AUDIO_INFO_LOG("SumAudioS16AbsPcm2");
    return SumS16AbsNeno(pcm, num_samples, channels);
#else
    AUDIO_INFO_LOG("SumAudioS16AbsPcm3");
    return SumPcmAbsNormal<int16_t, int32_t>(pcm, num_samples, channels, split);
#endif
}
std::vector<int32_t> SumU8SingleNeno(const uint8_t* pcm, uint32_t num_samples)
{
    std::vector<int32_t> sum(1, 0);
#if USE_ARM_NEON == 1
    uint32x4_t acc32 = vdupq_n_u32(0);
    for (uint32_t i = 0; i + 15 < num_samples; i += 16) {
        uint8x16_t samples = vld1q_u8(pcm);
        // 将8位扩展到16位
        uint16x8_t low = vmovl_u8(vget_low_u8(samples));
        uint16x8_t high = vmovl_u8(vget_high_u8(samples));
        // 累加到32位向量
        acc32 = vpadalq_u16(acc32, low);
        acc32 = vpadalq_u16(acc32, high);
        pcm += 16;
    }
    sum[0] = vaddvq_u32(acc32);
#endif
    return sum;
}
std::vector<int32_t> SumU8StereoNeno(const uint8_t *data, uint32_t num_samples)
{
    std::vector<int32_t> sum(2, 0);
#if USE_ARM_NEON == 1
    uint32x4_t sum_left_32x4 = vdupq_n_u32(0);
    uint32x4_t sum_right_32x4 = vdupq_n_u32(0);
    // 每次处理16个样本
    for (uint32_t i = 0; i + 15 < num_samples; i += 16) {
        // 加载并解交错16个样本
        uint8x16x2_t samples = vld2q_u8(data);
        data += 32;

        // U8无符号数绝对值即本身
        uint16x8_t left_low = vmovl_u8(vget_low_u8(samples.val[0]));
        uint16x8_t left_high = vmovl_high_u8(samples.val[0]);
        uint16x8_t right_low = vmovl_u8(vget_low_u8(samples.val[1]));
        uint16x8_t right_high = vmovl_high_u8(samples.val[1]);

        // 累加左声道
        sum_left_32x4 = vaddq_u32(sum_left_32x4, vaddl_u16(vget_low_u16(left_low), vget_high_u16(left_low)));
        sum_left_32x4 = vaddq_u32(sum_left_32x4, vaddl_u16(vget_low_u16(left_high), vget_high_u16(left_high)));
        
        // 累加右声道
        sum_right_32x4 = vaddq_u32(sum_right_32x4, vaddl_u16(vget_low_u16(right_low), vget_high_u16(right_low)));
        sum_right_32x4 = vaddq_u32(sum_right_32x4, vaddl_u16(vget_low_u16(right_high), vget_high_u16(right_high)));
    }

    // 水平求和
    sum[0] = vaddvq_u32(sum_left_32x4);
    sum[1] = vaddvq_u32(sum_right_32x4);
#endif
    return sum;
}
std::vector<int32_t> SumU8AbsNeno(const uint8_t *pcm, uint32_t num_samples, int32_t channels)
{
    std::vector<int32_t> sum(channels, 0);
    if (channels == 1) {
        Trace trace("SumU8SingleNeno");
        return SumU8SingleNeno(pcm, num_samples);
    } else {
        Trace trace("SumU8StereoNeno");
        return SumU8StereoNeno(pcm, num_samples);
    }
    return sum;
}
std::vector<int32_t> AudioToolCalculate::SumAudioU8AbsPcm(const uint8_t *pcm, uint32_t num_samples,
    int32_t channels, size_t split)
{
    if (!Is16ByteAligned(pcm) || channels > 2 || split > 1) {
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
    float32x4_t sum_vec = vdupq_n_f32(0.0f);  // 初始化累加向量为 0
    const uint32_t samples_per_loop = 8;
    for (uint32_t i = 0; i + samples_per_loop <= num_samples; i += samples_per_loop) {
        float32x4_t v0 = vld1q_f32(&pcm[i]);
        float32x4_t v1 = vld1q_f32(&pcm[i + 4]);
        sum_vec = vaddq_f32(sum_vec, vabsq_f32(v0));
        sum_vec = vaddq_f32(sum_vec, vabsq_f32(v1));
    }
    // 水平求和：将 4 个 lane 相加
    sum[0] = vaddvq_f32(sum_vec);
#endif
    return sum;
}
std::vector<float> SumF32StereoAbsNeno(const float *pcm, uint32_t num_samples)
{
    std::vector<float> sum(2, 0);
#if USE_ARM_NEON == 1
    float32x4_t sum_left_32x4 = vdupq_n_f32(0.0f);
    float32x4_t sum_right_32x4 = vdupq_n_f32(0.0f);
    // 每次处理4个样本
    for ( uint32_t i = 0; i + 3 < num_samples; i += 4) {
        // 加载并解交错4个样本
        float32x4x2_t samples = vld2q_f32(pcm);
        pcm += 8;

        // 计算绝对值
        float32x4_t left_abs = vabsq_f32(samples.val[0]);
        float32x4_t right_abs = vabsq_f32(samples.val[1]);

        // 累加
        sum_left_32x4 = vaddq_f32(sum_left_32x4, left_abs);
        sum_right_32x4 = vaddq_f32(sum_right_32x4, right_abs);
    }
    // 水平求和
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
        Trace trace("SumF32SingleAbsNeno");
        return SumF32SingleAbsNeno(pcm, num_samples);
    } else {
        Trace trace("SumF32StereoAbsNeno");
        return SumF32StereoAbsNeno(pcm, num_samples);
    }
    return sum;
}
}
}