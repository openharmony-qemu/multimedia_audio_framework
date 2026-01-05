/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "oh_audio_render_unit_test.h"

using namespace testing::ext;
using namespace OHOS::AudioStandard::OHAudioRenderUT;

namespace OHOS {
namespace AudioStandard {
/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency API invalid renderer.
 * @tc.number: OH_Audio_Render_GetLatency_001
 * @tc.desc  : Returns AUDIOSTREAM_ERROR_INVALID_PARAM when renderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_001, TestSize.Level0)
{
    int32_t latencyMs = 0;
    OH_AudioStream_Result result = OH_AudioRenderer_GetLatency(nullptr, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency API invalid latency pointer.
 * @tc.number: OH_Audio_Render_GetLatency_002
 * @tc.desc  : Returns AUDIOSTREAM_ERROR_INVALID_PARAM when latencyMs is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency API invalid latency type.
 * @tc.number: OH_Audio_Render_GetLatency_003
 * @tc.desc  : Returns AUDIOSTREAM_ERROR_INVALID_PARAM when latency type is invalid.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, static_cast<OH_AudioStream_LatencyType>(-1), &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL type success.
 * @tc.number: OH_Audio_Render_GetLatency_004
 * @tc.desc  : Expect success when latency type is AUDIOSTREAM_LATENCY_TYPE_ALL.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE type success.
 * @tc.number: OH_Audio_Render_GetLatency_005
 * @tc.desc  : Expect success when latency type is AUDIOSTREAM_LATENCY_TYPE_SOFTWARE.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE type success.
 * @tc.number: OH_Audio_Render_GetLatency_006
 * @tc.desc  : Expect success when latency type is AUDIOSTREAM_LATENCY_TYPE_HARDWARE.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_006, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL type success with movie usage.
 * @tc.number: OH_Audio_Render_GetLatency_007
 * @tc.desc  : Expect success when latency type is ALL and usage is movie.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_007, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MOVIE);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE type success with movie usage.
 * @tc.number: OH_Audio_Render_GetLatency_008
 * @tc.desc  : Expect success when latency type is SOFTWARE and usage is movie.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_008, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MOVIE);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE type success with movie usage.
 * @tc.number: OH_Audio_Render_GetLatency_009
 * @tc.desc  : Expect success when latency type is HARDWARE and usage is movie.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_009, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MOVIE);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL type success with music usage and S32 format.
 * @tc.number: OH_Audio_Render_GetLatency_010
 * @tc.desc  : Expect success when latency type is ALL and usage is music with S32 sample format.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_010, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE type success with music usage and S32 format.
 * @tc.number: OH_Audio_Render_GetLatency_011
 * @tc.desc  : Expect success when latency type is SOFTWARE and usage is music with S32 sample format.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_011, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE type success with music usage and S32 format.
 * @tc.number: OH_Audio_Render_GetLatency_012
 * @tc.desc  : Expect success when latency type is HARDWARE and usage is music with S32 sample format.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_012, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with music usage, S32 format, 48000 rate.
 * @tc.number: OH_Audio_Render_GetLatency_013
 * @tc.desc  : Expect success when type ALL, usage music, format S32LE, rate 48000.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_013, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with music usage, S32 format, 48000 rate.
 * @tc.number: OH_Audio_Render_GetLatency_014
 * @tc.desc  : Expect success when type SOFTWARE, usage music, format S32LE, rate 48000.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_014, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with music usage, S32 format, 48000 rate.
 * @tc.number: OH_Audio_Render_GetLatency_015
 * @tc.desc  : Expect success when type HARDWARE, usage music, format S32LE, rate 48000.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_015, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with voip usage, S32 format, 48000 rate.
 * @tc.number: OH_Audio_Render_GetLatency_016
 * @tc.desc  : Expect success when type ALL, usage voip, format S32LE, rate 48000.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_016, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_VOICE_COMMUNICATION);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with voip usage, S32 format, 48000 rate.
 * @tc.number: OH_Audio_Render_GetLatency_017
 * @tc.desc  : Expect success when type SOFTWARE, usage voip, format S32LE, rate 48000.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_017, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_VOICE_COMMUNICATION);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with voip usage, S32 format, 48000 rate.
 * @tc.number: OH_Audio_Render_GetLatency_018
 * @tc.desc  : Expect success when type HARDWARE, usage voip, format S32LE, rate 48000.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_018, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_VOICE_COMMUNICATION);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_019
 * @tc.desc  : Expect success when type ALL and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_019, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_020
 * @tc.desc  : Expect success when type SOFTWARE and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_020, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_021
 * @tc.desc  : Expect success when type HARDWARE and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_021, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with movie usage and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_022
 * @tc.desc  : Expect success when type ALL, usage movie and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_022, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MOVIE);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with movie usage and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_023
 * @tc.desc  : Expect success when type SOFTWARE, usage movie and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_023, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MOVIE);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with movie usage and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_024
 * @tc.desc  : Expect success when type HARDWARE, usage movie and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_024, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MOVIE);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with music usage, S32 format, low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_025
 * @tc.desc  : Expect success when type ALL, usage music, format S32LE and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_025, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with music usage, S32 format, low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_026
 * @tc.desc  : Expect success when type SOFTWARE, usage music, format S32LE and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_026, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with music usage, S32 format, low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_027
 * @tc.desc  : Expect success when type HARDWARE, usage music, format S32LE and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_027, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with music usage, S32 format, 48000 rate and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_028
 * @tc.desc  : Expect success when type ALL, usage music, format S32LE, rate 48000 and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_028, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with music usage, S32 format, 48000 rate and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_029
 * @tc.desc  : Expect success when type SOFTWARE, usage music, format S32LE, rate 48000 and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_029, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with music usage, S32 format, 48000 rate and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_030
 * @tc.desc  : Expect success when type HARDWARE, usage music, format S32LE, rate 48000 and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_030, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with voip usage, S32 format, 48000 rate and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_031
 * @tc.desc  : Expect success when type ALL, usage voip, format S32LE, rate 48000 and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_031, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_VOICE_COMMUNICATION);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with voip usage, S32 format, 48000 rate and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_032
 * @tc.desc  : Expect success when type SOFTWARE, usage voip, format S32LE, rate 48000 and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_032, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_VOICE_COMMUNICATION);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with voip usage, S32 format, 48000 rate and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_033
 * @tc.desc  : Expect success when type HARDWARE, usage voip, format S32LE, rate 48000 and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_033, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_VOICE_COMMUNICATION);
    OH_AudioStreamBuilder_SetSampleFormat(builder, AUDIOSTREAM_SAMPLE_S32LE);
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with game usage.
 * @tc.number: OH_Audio_Render_GetLatency_034
 * @tc.desc  : Expect success when type ALL and usage game.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_034, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_GAME);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with game usage.
 * @tc.number: OH_Audio_Render_GetLatency_035
 * @tc.desc  : Expect success when type SOFTWARE and usage game.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_035, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_GAME);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with game usage.
 * @tc.number: OH_Audio_Render_GetLatency_036
 * @tc.desc  : Expect success when type HARDWARE and usage game.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_036, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_GAME);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency ALL with game usage and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_037
 * @tc.desc  : Expect success when type ALL, usage game and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_037, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_GAME);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_ALL, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency SOFTWARE with game usage and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_038
 * @tc.desc  : Expect success when type SOFTWARE, usage game and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_038, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_GAME);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_SOFTWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatency HARDWARE with game usage and low latency mode.
 * @tc.number: OH_Audio_Render_GetLatency_039
 * @tc.desc  : Expect success when type HARDWARE, usage game and latency mode FAST.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLatency_039, TestSize.Level2)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_GAME);
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioRenderer* renderer = nullptr;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t latencyMs = 0;
    result = OH_AudioRenderer_GetLatency(renderer, AUDIOSTREAM_LATENCY_TYPE_HARDWARE, &latencyMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(renderer);
    OH_AudioStreamBuilder_Destroy(builder);
}
} // namespace AudioStandard
} // namespace OHOS
