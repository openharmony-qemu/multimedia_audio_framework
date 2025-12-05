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
        void *ptr = std::aligned_alloc(Alignment, n * sizeof(T));
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


/**
 * @tc.name  : SumAudioS32AbsPcmTest001
 * @tc.number: SumAudioS32AbsPcmTest001
 * @tc.desc  : 当输入参数不满足16字节对齐、channels>2或split>1时,应调用SumPcmAbsNormal函数
 */
HWTEST_F(AudioToolCalculateUnitTest, SumAudioS32AbsPcmTest001, TestSize.Level1)
{
    std::vector<int32_t, AlignedAllocator<int32_t, 16>> pcm(16, 0);
    int32_t channels = 1;
    uint32_t num_samples = pcm.size() - 1;
    size_t split = 1; 
    std::vector<int64_t> result = AudioToolCalculate::SumAudioS32AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
    channels = 4;
    num_samples = pcm.size() / channels - 1;
    result = AudioToolCalculate::SumAudioS32AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
    split = 2;
    result = AudioToolCalculate::SumAudioS32AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name  : SumAudioS32AbsPcmTest002
 * @tc.number: SumAudioS32AbsPcmTest002
 * @tc.desc  : 当输入参数满足16字节对齐、channels<=2且split<=1时,并使用ARM NEON优化,应调用SumS32AbsNemo函数
 */
HWTEST_F(AudioToolCalculateUnitTest, SumAudioS32AbsPcmTest002, TestSize.Level1)
{
    std::vector<int32_t, AlignedAllocator<int32_t, 16>> pcm = {1, -2, 3, -4, 5, -6, 7, -8};
    int32_t channels = 4;
    uint32_t num_samples = pcm.size() / channels;
    size_t split = 1;
    std::vector<int64_t> result = AudioToolCalculate::SumAudioS32AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 6);
    channels = 2;
    num_samples = pcm.size() / channels;
    split = 2;
    result = AudioToolCalculate::SumAudioS32AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 6);
    split = 1;
    result = AudioToolCalculate::SumAudioS32AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 16);
    channels = 1;
    num_samples = pcm.size() / channels;
    result = AudioToolCalculate::SumAudioS32AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 36);
}

/**
 * @tc.name  : SumAudioU8AbsPcmTest002
 * @tc.number: SumAudioU8AbsPcmTest002
 * @tc.desc  : 当输入参数满足16字节对齐、channels<=2且split<=1时,并使用ARM NEON优化,应调用SumAudioU8AbsPcm函数
 */
HWTEST_F(AudioToolCalculateUnitTest, SumAudioU8AbsPcmTest002, TestSize.Level1)
{
    std::vector<uint8_t, AlignedAllocator<uint8_t, 16>> pcm(16, 0);
    int32_t channels = 1;
    uint32_t num_samples = pcm.size() / channels - 1;
    size_t split = 1; 
    std::vector<int32_t> result = AudioToolCalculate::SumAudioU8AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
    channels = 4;
    num_samples = pcm.size() / channels - 1;
    result = AudioToolCalculate::SumAudioU8AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
    split = 2;
    result = AudioToolCalculate::SumAudioU8AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name  : SumAudioU8AbsPcmTest001
 * @tc.number: SumAudioU8AbsPcmTest001
 * @tc.desc  : 当输入参数满足16字节对齐、channels<=2且split<=1时,并使用ARM NEON优化,应调用SumAudioU8AbsPcm函数
 */
HWTEST_F(AudioToolCalculateUnitTest, SumAudioU8AbsPcmTest001, TestSize.Level1)
{
    std::vector<uint8_t, AlignedAllocator<uint8_t, 16>> pcm(32, 1);
    int32_t channels = 4;
    uint32_t num_samples = pcm.size() / channels;
    size_t split = 1;
    std::vector<int32_t> result = AudioToolCalculate::SumAudioU8AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 8);
    channels = 2;
    num_samples = pcm.size() / channels;
    split = 2;
    result = AudioToolCalculate::SumAudioU8AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 8);
    split = 1;
    result = AudioToolCalculate::SumAudioU8AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 16);
    channels = 1;
    num_samples = pcm.size() / channels;
    result = AudioToolCalculate::SumAudioU8AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 32);
}

/**
 * @tc.name  : SumAudioS16AbsPcmTest001
 * @tc.number: SumAudioS16AbsPcmTest001
 * @tc.desc  : 当输入参数不满足16字节对齐、channels>2或split>1时,应调用SumPcmAbsNormal函数
 */
HWTEST_F(AudioToolCalculateUnitTest, SumAudioS16AbsPcmTest001, TestSize.Level1)
{
    std::vector<int16_t, AlignedAllocator<int16_t, 16>> pcm(16, 0);
    int32_t channels = 1;
    uint32_t num_samples = pcm.size() / channels - 1;
    size_t split = 1;
    std::vector<int32_t> result = AudioToolCalculate::SumAudioS16AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
    channels = 4;
    num_samples = pcm.size() / channels - 1;
    result = AudioToolCalculate::SumAudioS16AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
    split = 2;
    result = AudioToolCalculate::SumAudioS16AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name  : SumAudioS16AbsPcmTest002
 * @tc.number: SumAudioS16AbsPcmTest002
 * @tc.desc  : 当输入参数满足16字节对齐、channels<=2且split<=1时,并使用ARM NEON优化,应调用SumS32AbsNemo函数
 */
HWTEST_F(AudioToolCalculateUnitTest, SumAudioS16AbsPcmTest002, TestSize.Level1)
{
    std::vector<int16_t, AlignedAllocator<int16_t, 16>> pcm = {1, -2, 3, -4, 5, -6, 7, -8, 1, -2, 3, -4, 5, -6, 7, -8};
    int32_t channels = 4;
    uint32_t num_samples = pcm.size() / channels;
    size_t split = 1; 
    std::vector<int32_t> result = AudioToolCalculate::SumAudioS16AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 12);
    channels = 2;
    num_samples = pcm.size() / channels;
    split = 2;
    result = AudioToolCalculate::SumAudioS16AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 12);
    split = 1;
    result = AudioToolCalculate::SumAudioS16AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 32);
    channels = 1;
    num_samples = pcm.size() / channels;
    result = AudioToolCalculate::SumAudioS16AbsPcm(pcm.data(), num_samples, channels, split);
    EXPECT_EQ(result[0], 72);
}

/**
 * @tc.name  : SumAudioF32AbsPcmTest001
 * @tc.number: SumAudioF32AbsPcmTest001
 * @tc.desc  : 当输入参数不满足16字节对齐、channels>2或split>1时,应调用SumPcmAbsNormal函数
 */
HWTEST_F(AudioToolCalculateUnitTest, SumAudioF32AbsPcmTest001, TestSize.Level1)
{
    std::vector<float, AlignedAllocator<float, 16>> pcm(16, 0.f);
    int32_t channels = 1;
    uint32_t num_samples = pcm.size() / channels - 1;
    size_t split = 1;
    std::vector<float> result;
    result = AudioToolCalculate::SumAudioF32AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
    channels = 4;
    num_samples = pcm.size() / channels - 1;
    split = 3;
    result = AudioToolCalculate::SumAudioF32AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
    split = 2;
    result = AudioToolCalculate::SumAudioF32AbsPcm(pcm.data() + 1, num_samples, channels, split);
    EXPECT_FALSE(result.empty());
}

/**
 * @tc.name  : SumAudioF32AbsPcmTest002
 * @tc.number: SumAudioF32AbsPcmTest002
 * @tc.desc  : 当输入参数满足16字节对齐、channels<=2且split<=1时,并使用ARM NEON优化,应调用SumS32AbsNemo函数
 */
HWTEST_F(AudioToolCalculateUnitTest, SumAudioF32AbsPcmTest002, TestSize.Level1)
{
    std::vector<float, AlignedAllocator<float, 16>> pcm = {0.1f, -0.2f, 0.3f, -0.4f, 0.5f, -0.6f, 0.7f, -0.8f};
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