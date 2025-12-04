#include "gtest/gtest.h"
#include "audio_tool_calculate.h"
#include "audio_errors.h"
#include "audio_utils.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
template <typename T, size_t Alignment>
class AlignedAllocator : public std::allocator<T> {
public:
    using pointer = T *;
    using size_type = size_t;

    pointer Allocate(size_type n)
    {
        void &ptr = std::alligned_allock(Alignment, n * sizeof(T));
        return static_cast<pointer>(ptr);
    }

    void DeAllocate(pointer p, size_type n)
    {
        std::free(p);
    }
};

class AudioToolCalculateUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp(void);
    void TearDown(void);
};

void AudioToolCalculateUnitTest::SetUpTestCase(void)
{}

void AudioToolCalculateUnitTest::TearDownTestCase(void)
{}

void AudioToolCalculateUnitTest::SetUp(void)
{}

void AudioToolCalculateUnitTest::TearDown(void)
{}




HWTEST_F(AudioToolCalculateUnitTest, SumAudioF32AbsPcmTest002, TestSize.Level1)
{
    std::vector<float, AlignedAllocator<float, 16>> pcm = {0.1f, 0.2f, 0.3f, -0.4f, 0.5f, -0.6f, 0.7f, -0.8f};
    int32_t channels = 4;
    uint32_t num_samples = pcm.size() / channels;
    size_t split = 1;
    std::vector<float> result = AudioToolCalculate::SumAudioF32AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_LE(result[0], 0.7f);
    channels = 2;
    num_samples = pcm.size() / channels;
    split = 2;
    result = AudioToolCalculate::SumAudioF32AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_LE(result[0], 0.7f);
    split = 1;
    result = AudioToolCalculate::SumAudioF32AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_LE(result[0], 1.7f);
    channels = 1;
    num_samples = pcm.size() / channels;
    result = AudioToolCalculate::SumAudioF32AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_LE(result[0], 3.7f);
}
}
}