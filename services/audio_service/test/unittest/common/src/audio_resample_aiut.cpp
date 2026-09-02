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

#include <memory>
#include <vector>

#include "audio_resample.h"
#include "audio_proresampler.h"

#include "aiut_log.h"

#include "audio_errors.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioResampleAiut : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name  : AudioResample_Constructor_ValidParams
 * @tc.number: AudioResample_Constructor_ValidParams
 * @tc.desc  : Test AudioResample constructor with valid parameters, resampler_ should be initialized.
 */
HWTEST_F(AudioResampleAiut, AudioResample_Constructor_ValidParams, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    uint32_t channels = 2;
    uint32_t inRate = 48000;
    uint32_t outRate = 44100;
    int32_t quantity = 10;
    auto resample = std::make_unique<AudioResample>(channels, inRate, outRate, quantity);
    AIUT_ASSERT_NE_INFO_LOG(resample, nullptr, "AudioResample create failed, ptr=%p", resample.get());
    AIUT_EXPECT_TRUE_INFO_LOG(resample->IsResampleInit(), "IsResampleInit should return true after construction");
}

/**
 * @tc.name  : AudioResample_IsResampleInit_ValidResampler
 * @tc.number: AudioResample_IsResampleInit_ValidResampler
 * @tc.desc  : Test IsResampleInit returns true when resampler_ is valid.
 */
HWTEST_F(AudioResampleAiut, AudioResample_IsResampleInit_ValidResampler, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioResample resample(2, 48000, 44100, 10);
    AIUT_EXPECT_TRUE_INFO_LOG(resample.IsResampleInit(), "IsResampleInit should return true");
}

/**
 * @tc.name  : AudioResample_IsResampleInit_NullResampler
 * @tc.number: AudioResample_IsResampleInit_NullResampler
 * @tc.desc  : Test IsResampleInit returns false when resampler_ is nullptr.
 */
HWTEST_F(AudioResampleAiut, AudioResample_IsResampleInit_NullResampler, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioResample resample(2, 48000, 44100, 10);
    resample.resampler_ = nullptr;
    AIUT_EXPECT_FALSE_INFO_LOG(resample.IsResampleInit(), "IsResampleInit should return false when resampler_ is null");
}

/**
 * @tc.name  : AudioResample_ProcessFloatResample_NullResampler
 * @tc.number: AudioResample_ProcessFloatResample_NullResampler
 * @tc.desc  : Test ProcessFloatResample returns ERR_INVALID_PARAM when resampler_ is nullptr.
 */
HWTEST_F(AudioResampleAiut, AudioResample_ProcessFloatResample_NullResampler, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioResample resample(2, 48000, 44100, 10);
    resample.resampler_ = nullptr;
    std::vector<float> input(10, 0.5f);
    std::vector<float> output(10, 0.0f);
    int32_t ret = resample.ProcessFloatResample(input, output);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "ProcessFloatResample should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : AudioResample_ProcessFloatResample_ValidInput
 * @tc.number: AudioResample_ProcessFloatResample_ValidInput
 * @tc.desc  : Test ProcessFloatResample returns ERR_INVALID_PARAM when channels is 0.
 */
HWTEST_F(AudioResampleAiut, AudioResample_ProcessFloatResample_ValidInput, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioResample resample(0, 48000, 44100, 10);
    std::vector<float> input(10, 0.5f);
    std::vector<float> output(10, 0.0f);
    int32_t ret = resample.ProcessFloatResample(input, output);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "ProcessFloatResample should return ERR_INVALID_PARAM when channels is 0, ret=%d", ret);
}

/**
 * @tc.name  : AudioResample_ProcessFloatResample_InputSizeNotDivisible
 * @tc.number: AudioResample_ProcessFloatResample_InputSizeNotDivisible
 * @tc.desc  : Test ProcessFloatResample with input size not divisible by channels.
 */
HWTEST_F(AudioResampleAiut, AudioResample_ProcessFloatResample_InputSizeNotDivisible, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioResample resample(2, 48000, 44100, 10);
    std::vector<float> input(3, 0.5f);
    std::vector<float> output(4, 0.0f);
    int32_t ret = resample.ProcessFloatResample(input, output);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "ProcessFloatResample should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : AudioResample_ProcessFloatResample_OutputSizeNotDivisible
 * @tc.number: AudioResample_ProcessFloatResample_OutputSizeNotDivisible
 * @tc.desc  : Test ProcessFloatResample with output size not divisible by channels.
 */
HWTEST_F(AudioResampleAiut, AudioResample_ProcessFloatResample_OutputSizeNotDivisible, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioResample resample(2, 48000, 44100, 10);
    std::vector<float> input(4, 0.5f);
    std::vector<float> output(3, 0.0f);
    int32_t ret = resample.ProcessFloatResample(input, output);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM,
        "ProcessFloatResample should return ERR_INVALID_PARAM, ret=%d", ret);
}

/**
 * @tc.name  : AudioResample_ProcessFloatResample_ValidInputAndOutput
 * @tc.number: AudioResample_ProcessFloatResample_ValidInputAndOutput
 * @tc.desc  : Test ProcessFloatResample with valid input and output sizes returns success.
 */
HWTEST_F(AudioResampleAiut, AudioResample_ProcessFloatResample_ValidInputAndOutput, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioResample resample(2, 48000, 44100, 10);
    std::vector<float> input(10, 0.5f);
    std::vector<float> output(10, 0.0f);
    int32_t ret = resample.ProcessFloatResample(input, output);
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0,
        "ProcessFloatResample should return 0 for valid input when resampler init, ret=%d", ret);
}

/**
 * @tc.name  : AudioResample_Destructor_ValidResampler
 * @tc.number: AudioResample_Destructor_ValidResampler
 * @tc.desc  : Test AudioResample destructor with valid resampler_.
 */
HWTEST_F(AudioResampleAiut, AudioResample_Destructor_ValidResampler, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto resample = std::make_unique<AudioResample>(2, 48000, 44100, 10);
    AIUT_ASSERT_NE_INFO_LOG(resample, nullptr, "AudioResample create failed, ptr=%p", resample.get());
    resample.reset();
    AIUT_INFO_LOG("destructor completed successfully");
}

/**
 * @tc.name  : AudioResample_Destructor_NullResampler
 * @tc.number: AudioResample_Destructor_NullResampler
 * @tc.desc  : Test AudioResample destructor with nullptr resampler_.
 */
HWTEST_F(AudioResampleAiut, AudioResample_Destructor_NullResampler, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto resample = std::make_unique<AudioResample>(2, 48000, 44100, 10);
    resample->resampler_ = nullptr;
    resample.reset();
    AIUT_INFO_LOG("destructor with null resampler_ completed successfully");
}

} // namespace AudioStandard
} // namespace OHOS
