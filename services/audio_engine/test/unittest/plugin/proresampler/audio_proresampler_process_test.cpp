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
#include <vector>
#include <map>
#include <gtest/gtest.h>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <cstdint>
#include "audio_proresampler_process.h"
#include "audio_engine_log.h"
#include "securec.h"

using namespace testing::ext;
using namespace testing;

class AudioProResamplerProcessTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void AudioProResamplerProcessTest::SetUp() {}

void AudioProResamplerProcessTest::TearDown() {}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate API
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_01
 * @tc.desc  : Test SingleStagePolyphaseResamplerSetRate, set decimateFactor is
 *             0 and interpolateFactor is 0.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_01, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 0;
    uint32_t interpolateFactor = 0;
    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
    EXPECT_EQ(ret, RESAMPLER_ERR_INVALID_ARG);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate API
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_02
 * @tc.desc  : Test SingleStagePolyphaseResamplerSetRate, set decimateFactor is
 *             0 and interpolateFactor is 2.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_02, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 0;
    uint32_t interpolateFactor = 2;
    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
    EXPECT_EQ(ret, RESAMPLER_ERR_INVALID_ARG);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate API
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_03
 * @tc.desc  : Test SingleStagePolyphaseResamplerSetRate, set decimateFactor is
 *             2 and interpolateFactor is 0.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_03, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 0;
    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);
    EXPECT_EQ(ret, RESAMPLER_ERR_INVALID_ARG);
}

/**
 * @tc.name  : SingleStagePolyphaseResamplerResetMem_001
 * @tc.type  : FUNC
 * @tc.desc  : Test if all state variables and input memory are cleared to zero.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerResetMem_001, TestSize.Level1)
{
    SingleStagePolyphaseResamplerState state;

    state.inputIndex = 123;
    state.magicSamples = 45;
    state.subfilterNum = 6;
    state.numChannels = 2;
    state.inputMemorySize = 10;

    uint32_t totalMemSize = state.numChannels * state.inputMemorySize;
    state.inputMemory = (float*)malloc(totalMemSize * sizeof(float));
    ASSERT_NE(state.inputMemory, nullptr);
    for (uint32_t i = 0; i < totalMemSize; i++) {
        state.inputMemory[i] = 0.85f;
    }

    int32_t ret = SingleStagePolyphaseResamplerResetMem(&state);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_EQ(state.inputIndex, 0);
    EXPECT_EQ(state.magicSamples, 0);
    EXPECT_EQ(state.subfilterNum, 0);

    for (uint32_t i = 0; i < totalMemSize; i++) {
        EXPECT_EQ(state.inputMemory[i], 0.0f);
    }

    free(state.inputMemory);
    state.inputMemory = nullptr;
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerInit basic case
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerInit_001
 * @tc.desc  : Test basic initialization with mono channel.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerInit_001, TestSize.Level0)
{
    uint32_t numChannels = 1; // Mono
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);

    EXPECT_NE(state, nullptr);
    EXPECT_EQ(err, RESAMPLER_ERR_SUCCESS);

    if (state) {
        EXPECT_EQ(state->numChannels, numChannels);
        EXPECT_EQ(state->decimateFactor, 2);
        EXPECT_EQ(state->interpolateFactor, 1);
        SingleStagePolyphaseResamplerFree(state);
    }
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerInit invalid numChannels
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerInit_002
 * @tc.desc  : Test initialization with zero channels.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerInit_002, TestSize.Level0)
{
    uint32_t numChannels = 0;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);

    EXPECT_EQ(state, nullptr);
    EXPECT_EQ(err, RESAMPLER_ERR_INVALID_ARG);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerInit invalid quality negative
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerInit_003
 * @tc.desc  : Test initialization with negative quality.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerInit_003, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = -1;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);

    EXPECT_EQ(state, nullptr);
    EXPECT_EQ(err, RESAMPLER_ERR_INVALID_ARG);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerInit invalid quality too high
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerInit_004
 * @tc.desc  : Test initialization with quality > 10.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerInit_004, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 11;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);

    EXPECT_EQ(state, nullptr);
    EXPECT_EQ(err, RESAMPLER_ERR_INVALID_ARG);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerInit multi-channel
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerInit_005
 * @tc.desc  : Test initialization with 8 channels.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerInit_005, TestSize.Level0)
{
    uint32_t numChannels = 8;
    uint32_t decimateFactor = 4;
    uint32_t interpolateFactor = 3;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);

    EXPECT_NE(state, nullptr);
    EXPECT_EQ(err, RESAMPLER_ERR_SUCCESS);

    if (state) {
        EXPECT_EQ(state->numChannels, numChannels);
        SingleStagePolyphaseResamplerFree(state);
    }
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerProcess mono upsampling
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerProcess_001
 * @tc.desc  : Test mono channel upsampling (1x -> 2x).
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerProcess_001, TestSize.Level0)
{
    uint32_t numChannels = 1;
    uint32_t decimateFactor = 1;
    uint32_t interpolateFactor = 2;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 480;
    const uint32_t outputLen = 960;
    float* input = (float*)malloc(inputLen * sizeof(float));
    float* output = (float*)malloc(outputLen * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen; i++) {
        input[i] = 0.1f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_GT(outLen, 0);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerProcess stereo upsampling
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerProcess_002
 * @tc.desc  : Test stereo channel upsampling.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerProcess_002, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 1;
    uint32_t interpolateFactor = 2;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 480;
    const uint32_t outputLen = 960;
    float* input = (float*)malloc(inputLen * numChannels * sizeof(float));
    float* output = (float*)malloc(outputLen * numChannels * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen * numChannels; i++) {
        input[i] = 0.1f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_GT(outLen, 0);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerProcess multi-channel downsampling
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerProcess_003
 * @tc.desc  : Test multi-channel downsampling.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerProcess_003, TestSize.Level0)
{
    uint32_t numChannels = 4;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 960;
    const uint32_t outputLen = 480;
    float* input = (float*)malloc(inputLen * numChannels * sizeof(float));
    float* output = (float*)malloc(outputLen * numChannels * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen * numChannels; i++) {
        input[i] = 0.1f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_GT(outLen, 0);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerProcess null input
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerProcess_004
 * @tc.desc  : Test processing with null input pointer.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerProcess_004, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 1;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t outputLen = 480;
    float* output = (float*)malloc(outputLen * numChannels * sizeof(float));
    ASSERT_NE(output, nullptr);

    uint32_t inLen = 480;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, nullptr, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);

    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerProcess zero length
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerProcess_005
 * @tc.desc  : Test processing with zero input length.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerProcess_005, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 1;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 0;
    const uint32_t outputLen = 480;
    float* output = (float*)malloc(outputLen * numChannels * sizeof(float));
    ASSERT_NE(output, nullptr);

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, nullptr, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);

    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerFree with null
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerFree_001
 * @tc.desc  : Test freeing null state.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerFree_001, TestSize.Level0)
{
    SingleStagePolyphaseResamplerFree(nullptr);
    // Should not crash
    SUCCEED();
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerFree with valid state
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerFree_002
 * @tc.desc  : Test freeing valid state.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerFree_002, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    SingleStagePolyphaseResamplerFree(state);
    // Should not crash
    SUCCEED();
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSkipHalfTaps
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSkipHalfTaps_001
 * @tc.desc  : Test skipping half taps.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSkipHalfTaps_001, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    uint32_t expectedInputIndex = state->filterLength / 2;
    int32_t ret = SingleStagePolyphaseResamplerSkipHalfTaps(state);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_EQ(state->inputIndex, expectedInputIndex);

    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate initial setup
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_04
 * @tc.desc  : Test SetRate for initial setup.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_04, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    state.decimateFactor = 0;
    state.interpolateFactor = 0;
    state.isInitialized = 0;
    state.subfilterNum = 0;

    uint32_t decimateFactor = 4;
    uint32_t interpolateFactor = 3;

    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, decimateFactor, interpolateFactor);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_EQ(state.decimateFactor, 4);
    EXPECT_EQ(state.interpolateFactor, 3);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate same rate
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_05
 * @tc.desc  : Test SetRate with same rates (no change).
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_05, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    state.decimateFactor = 4;
    state.interpolateFactor = 3;
    state.isInitialized = 0;
    state.subfilterNum = 0;

    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, 4, 3);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
}

/*
 * @tc.name  : Test SingleStagePolyphaseResamplerSetRate change rate
 * @tc.type  : FUNC
 * @tc.number: SingleStagePolyphaseResamplerSetRate_06
 * @tc.desc  : Test SetRate with different rates.
 */
HWTEST_F(AudioProResamplerProcessTest, SingleStagePolyphaseResamplerSetRate_06, TestSize.Level0)
{
    SingleStagePolyphaseResamplerState state;
    state.decimateFactor = 4;
    state.interpolateFactor = 3;
    state.isInitialized = 0;
    state.subfilterNum = 1;

    int32_t ret = SingleStagePolyphaseResamplerSetRate(&state, 8, 3);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_EQ(state.decimateFactor, 8);
    EXPECT_EQ(state.interpolateFactor, 3);
}

/*
 * @tc.name  : Test mono resampling 2:1
 * @tc.type  : FUNC
 * @tc.number: ResampleMono_2to1_001
 * @tc.desc  : Test mono downsampling by 2.
 */
HWTEST_F(AudioProResamplerProcessTest, ResampleMono_2to1_001, TestSize.Level0)
{
    uint32_t numChannels = 1;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 960;
    const uint32_t outputLen = 480;
    float* input = (float*)malloc(inputLen * sizeof(float));
    float* output = (float*)malloc(outputLen * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen; i++) {
        input[i] = 0.5f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_GT(outLen, 0);
    EXPECT_LE(outLen, outputLen);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test mono resampling 1:2
 * @tc.type  : FUNC
 * @tc.number: ResampleMono_1to2_001
 * @tc.desc  : Test mono upsampling by 2.
 */
HWTEST_F(AudioProResamplerProcessTest, ResampleMono_1to2_001, TestSize.Level0)
{
    uint32_t numChannels = 1;
    uint32_t decimateFactor = 1;
    uint32_t interpolateFactor = 2;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 480;
    const uint32_t outputLen = 960;
    float* input = (float*)malloc(inputLen * sizeof(float));
    float* output = (float*)malloc(outputLen * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen; i++) {
        input[i] = 0.5f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_GT(outLen, 0);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test stereo resampling 3:2
 * @tc.type  : FUNC
 * @tc.number: ResampleStereo_3to2_001
 * @tc.desc  : Test stereo resampling with 3:2 ratio.
 */
HWTEST_F(AudioProResamplerProcessTest, ResampleStereo_3to2_001, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 3;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 960;
    const uint32_t outputLen = 1440;
    float* input = (float*)malloc(inputLen * numChannels * sizeof(float));
    float* output = (float*)malloc(outputLen * numChannels * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen * numChannels; i++) {
        input[i] = 0.5f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_GT(outLen, 0);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test downsampling by 2 mono
 * @tc.type  : FUNC
 * @tc.number: DownsampleBy2_Mono_001
 * @tc.desc  : Test mono downsampling by exactly 2 (halfband).
 */
HWTEST_F(AudioProResamplerProcessTest, DownsampleBy2_Mono_001, TestSize.Level0)
{
    uint32_t numChannels = 1;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 1000;
    const uint32_t outputLen = 500;
    float* input = (float*)malloc(inputLen * sizeof(float));
    float* output = (float*)malloc(outputLen * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen; i++) {
        input[i] = sinf(2.0f * 3.14159f * 440.0f * i / 48000.0f);
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test downsampling by 2 stereo
 * @tc.type  : FUNC
 * @tc.number: DownsampleBy2_Stereo_001
 * @tc.desc  : Test stereo downsampling by exactly 2 (halfband).
 */
HWTEST_F(AudioProResamplerProcessTest, DownsampleBy2_Stereo_001, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 1000;
    const uint32_t outputLen = 500;
    float* input = (float*)malloc(inputLen * numChannels * sizeof(float));
    float* output = (float*)malloc(outputLen * numChannels * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen * numChannels; i++) {
        input[i] = sinf(2.0f * 3.14159f * 440.0f * i / 48000.0f);
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test downsampling by 3 mono
 * @tc.type  : FUNC
 * @tc.number: DownsampleBy3_Mono_001
 * @tc.desc  : Test mono downsampling by exactly 3 (thirdband).
 */
HWTEST_F(AudioProResamplerProcessTest, DownsampleBy3_Mono_001, TestSize.Level0)
{
    uint32_t numChannels = 1;
    uint32_t decimateFactor = 3;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 960;
    const uint32_t outputLen = 320;
    float* input = (float*)malloc(inputLen * sizeof(float));
    float* output = (float*)malloc(outputLen * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen; i++) {
        input[i] = 0.5f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test downsampling by 3 stereo
 * @tc.type  : FUNC
 * @tc.number: DownsampleBy3_Stereo_001
 * @tc.desc  : Test stereo downsampling by exactly 3 (thirdband).
 */
HWTEST_F(AudioProResamplerProcessTest, DownsampleBy3_Stereo_001, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 3;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 960;
    const uint32_t outputLen = 320;
    float* input = (float*)malloc(inputLen * numChannels * sizeof(float));
    float* output = (float*)malloc(outputLen * numChannels * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen * numChannels; i++) {
        input[i] = 0.5f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test quality level 0
 * @tc.type  : FUNC
 * @tc.number: QualityLevel_0_001
 * @tc.desc  : Test resampling with lowest quality.
 */
HWTEST_F(AudioProResamplerProcessTest, QualityLevel_0_001, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 0;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);

    EXPECT_NE(state, nullptr);
    EXPECT_EQ(state->quality, 0);

    if (state) {
        SingleStagePolyphaseResamplerFree(state);
    }
}

/*
 * @tc.name  : Test quality level 10
 * @tc.type  : FUNC
 * @tc.number: QualityLevel_10_001
 * @tc.desc  : Test resampling with highest quality.
 */
HWTEST_F(AudioProResamplerProcessTest, QualityLevel_10_001, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 10;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);

    EXPECT_NE(state, nullptr);
    EXPECT_EQ(state->quality, 10);

    if (state) {
        const uint32_t inputLen = 480;
        const uint32_t outputLen = 240;
        float* input = (float*)malloc(inputLen * numChannels * sizeof(float));
        float* output = (float*)malloc(outputLen * numChannels * sizeof(float));

        ASSERT_NE(input, nullptr);
        ASSERT_NE(output, nullptr);

        for (uint32_t i = 0; i < inputLen * numChannels; i++) {
            input[i] = 0.5f;
        }

        uint32_t inLen = inputLen;
        uint32_t outLen = outputLen;

        int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);

        EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);

        free(input);
        free(output);
        SingleStagePolyphaseResamplerFree(state);
    }
}

/*
 * @tc.name  : Test reset after process
 * @tc.type  : FUNC
 * @tc.number: ResetAfterProcess_001
 * @tc.desc  : Test reset functionality after processing.
 */
HWTEST_F(AudioProResamplerProcessTest, ResetAfterProcess_001, TestSize.Level0)
{
    uint32_t numChannels = 2;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);
    ASSERT_NE(state, nullptr);

    const uint32_t inputLen = 480;
    const uint32_t outputLen = 240;
    float* input = (float*)malloc(inputLen * numChannels * sizeof(float));
    float* output = (float*)malloc(outputLen * numChannels * sizeof(float));

    ASSERT_NE(input, nullptr);
    ASSERT_NE(output, nullptr);

    for (uint32_t i = 0; i < inputLen * numChannels; i++) {
        input[i] = 0.5f;
    }

    uint32_t inLen = inputLen;
    uint32_t outLen = outputLen;

    int32_t ret = SingleStagePolyphaseResamplerProcess(state, input, &inLen, output, &outLen);
    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);

    // Before reset, check state is initialized
    EXPECT_EQ(state->isStarted, 1);

    // Now reset the state
    ret = SingleStagePolyphaseResamplerResetMem(state);

    EXPECT_EQ(ret, RESAMPLER_ERR_SUCCESS);
    EXPECT_EQ(state->inputIndex, 0);
    EXPECT_EQ(state->subfilterNum, 0);

    free(input);
    free(output);
    SingleStagePolyphaseResamplerFree(state);
}

/*
 * @tc.name  : Test multi-channel 16 channels
 * @tc.type  : FUNC
 * @tc.number: MultiChannel_16_001
 * @tc.desc  : Test resampling with 16 channels (max).
 */
HWTEST_F(AudioProResamplerProcessTest, MultiChannel_16_001, TestSize.Level0)
{
    uint32_t numChannels = 16;
    uint32_t decimateFactor = 2;
    uint32_t interpolateFactor = 1;
    int32_t quality = 5;
    int32_t err;

    SingleStagePolyphaseResamplerState* state = SingleStagePolyphaseResamplerInit(
        numChannels, decimateFactor, interpolateFactor, quality, &err);

    EXPECT_NE(state, nullptr);

    if (state) {
        EXPECT_EQ(state->numChannels, 16);
        SingleStagePolyphaseResamplerFree(state);
    }
}

using namespace testing::ext;

class AudioPcmConvertTest : public testing::Test {
public:
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: AudioFormatCalcMemLength_001
 * @tc.number: AudioFormatCalcMemLength_001
 */
HWTEST_F(AudioPcmConvertTest, AudioFormatCalcMemLength_001, TestSize.Level0)
{
    EXPECT_EQ(AudioFormatCalcMemLength(AUDIO_PCM_S16_LE, 2, 960), 3840);
    EXPECT_EQ(AudioFormatCalcMemLength(AUDIO_PCM_F32_LE, 1, 480), 1920);
    EXPECT_EQ(AudioFormatCalcMemLength(AUDIO_PCM_S24_LE, 2, 240), 1440);
    EXPECT_EQ(AudioFormatCalcMemLength(AUDIO_PCM_S16_LE, 2, 0), 0);
}

/**
 * @tc.name: AudioFormatCalcMemLength_002
 * @tc.number: AudioFormatCalcMemLength_002
 */
HWTEST_F(AudioPcmConvertTest, AudioFormatCalcMemLength_002, TestSize.Level0)
{
    EXPECT_EQ(AudioFormatCalcMemLength(AUDIO_PCM_S16_LE, 0, 960), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(AudioFormatCalcMemLength(AUDIO_PCM_S16_LE, 2, -1), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(AudioFormatCalcMemLength(static_cast<AudioPcmFormat>(20), 2, 960), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(AudioFormatCalcMemLength(AUDIO_PCM_S32_LE, 2, 1000000000), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.name: ConvertPcmToFloat_001
 * @tc.number: ConvertPcmToFloat_001
 */
HWTEST_F(AudioPcmConvertTest, ConvertPcmToFloat_001, TestSize.Level0)
{
    float out[3] = {0.0f};
    int16_t in[3] = {0x4000, static_cast<int16_t>(0x8000), 0};
    ASSERT_EQ(ConvertPcmToFloat(AUDIO_PCM_S16_LE, in, 3, out), AUDIO_PCM_ALGO_SUCCESS);
    EXPECT_NEAR(out[0], 0.5, 2e-3); // 16384/32767
    EXPECT_NEAR(out[1], -1.0, 1e-6);
    EXPECT_EQ(out[2], 0.0f);

    // S24 little endian {0x00, 0x00, 0x40} reads as 0x400000, scaled by 1/2^31 after <<8
    uint8_t in24[3] = {0x00, 0x00, 0x40};
    float out24 = 0.0f;
    ASSERT_EQ(ConvertPcmToFloat(AUDIO_PCM_S24_LE, in24, 1, &out24), AUDIO_PCM_ALGO_SUCCESS);
    EXPECT_NEAR(out24, 0.5, 2e-3);
}

/**
 * @tc.name: ConvertRoundTrip_001
 * @tc.number: ConvertRoundTrip_001
 */
HWTEST_F(AudioPcmConvertTest, ConvertRoundTrip_001, TestSize.Level0)
{
    const int16_t values[] = {1, -1, 123, -4567, 16384, -16384, 32767, -32768};
    for (int16_t v : values) {
        float f = 0.0f;
        ASSERT_EQ(ConvertPcmToFloat(AUDIO_PCM_S16_LE, &v, 1, &f), AUDIO_PCM_ALGO_SUCCESS);
        int16_t back = 0;
        ASSERT_EQ(ConvertFloatToPcm(AUDIO_PCM_S16_LE, &f, 1, &back), AUDIO_PCM_ALGO_SUCCESS);
        EXPECT_LE(std::abs(static_cast<int32_t>(back) - static_cast<int32_t>(v)), 1);
    }
}

/**
 * @tc.name: ConvertInvalid_001
 * @tc.number: ConvertInvalid_001
 */
HWTEST_F(AudioPcmConvertTest, ConvertInvalid_001, TestSize.Level0)
{
    float f = 0.0f;
    int16_t v = 1;
    EXPECT_EQ(ConvertPcmToFloat(AUDIO_PCM_S16_LE, nullptr, 1, &f), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(ConvertPcmToFloat(AUDIO_PCM_S16_LE, &v, 0, &f), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(ConvertPcmToFloat(static_cast<AudioPcmFormat>(20), &v, 1, &f), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(ConvertFloatToPcm(AUDIO_PCM_S16_LE, nullptr, 1, &f), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
    EXPECT_EQ(ConvertFloatToPcm(static_cast<AudioPcmFormat>(20), &f, 1, &v), AUDIO_PCM_ALGO_ERR_INVALID_PARAM);
}

/**
 * @tc.name: ConvertSaturate_001
 * @tc.number: ConvertSaturate_001
 */
HWTEST_F(AudioPcmConvertTest, ConvertSaturate_001, TestSize.Level0)
{
    // values beyond +-1.0 clamp to +-(1.0 - eps) before quantization, covering both
    // CapMax clamp directions on the S16/S32/S24 quantizers
    constexpr int32_t SATURATE_SAMPLE_COUNT = 2; // one overdriven positive, one negative
    constexpr int32_t S24_SAMPLE_BYTES = 3;
    constexpr int32_t S24_MSB_INDEX = 2; // little-endian 24-bit: LSB first, MSB last
    constexpr float OVERDRIVE_LEVEL = 2.0f;
    constexpr int32_t S16_NEAR_FULL_SCALE = 32000;
    constexpr int32_t S32_NEAR_FULL_SCALE = 2147418112; // 2^31 * (1 - 3e-6)
    constexpr uint8_t S24_TOP_BYTE_CLAMPED = 0x7F;

    float loud[SATURATE_SAMPLE_COUNT] = {OVERDRIVE_LEVEL, -OVERDRIVE_LEVEL};
    int16_t s16[SATURATE_SAMPLE_COUNT] = {0};
    ASSERT_EQ(ConvertFloatToPcm(AUDIO_PCM_S16_LE, loud, SATURATE_SAMPLE_COUNT, s16),
        AUDIO_PCM_ALGO_SUCCESS);
    EXPECT_GT(s16[0], S16_NEAR_FULL_SCALE);
    EXPECT_LT(s16[1], -S16_NEAR_FULL_SCALE);

    int32_t s32[SATURATE_SAMPLE_COUNT] = {0};
    ASSERT_EQ(ConvertFloatToPcm(AUDIO_PCM_S32_LE, loud, SATURATE_SAMPLE_COUNT, s32),
        AUDIO_PCM_ALGO_SUCCESS);
    EXPECT_GT(s32[0], S32_NEAR_FULL_SCALE);
    EXPECT_LT(s32[1], -S32_NEAR_FULL_SCALE);

    uint8_t s24[S24_SAMPLE_BYTES] = {0};
    ASSERT_EQ(ConvertFloatToPcm(AUDIO_PCM_S24_LE, loud, 1, s24), AUDIO_PCM_ALGO_SUCCESS);
    EXPECT_EQ(s24[S24_MSB_INDEX], S24_TOP_BYTE_CLAMPED);
}