#ifndef AUDIO_TOOL_CALCULATE_H
#define AUDIO_TOOL_CALCULATE_H
#if !defined(DISABLE_SIMD) && \
    (defined(__aarch64__) || (defined(__arm__) && defined(__ARM_NEON__)))
// enable arm Simd
#include <arm_neon.h>
#define USE_ARM_NEON 1
#else
// disable SIMD.
#define USE_ARM_NEON 0
#endif
#include <vector>
namespace OHOS {
namespace AudioStandard {
class AudioToolCalculate {
public:
    static std::vector<int64_t> SumAudioS32AbsPcm(const int32_t *pcm, uint32_t num_samples, int32_t channel,
        size_t split);
    static std::vector<int32_t> SumAudioS16AbsPcm(const int16_t *pcm, uint32_t num_samples, int32_t channel,
        size_t split);
    static std::vector<int32_t> SumAudioU8AbsPcm(const uint8_t *pcm, uint32_t num_samples, int32_t channel,
        size_t split);
    static std::vector<float> SumAudioF32AbsPcm(const float *pcm, uint32_t num_samples, int32_t channel,
        size_t split);
};
}
}
#endif