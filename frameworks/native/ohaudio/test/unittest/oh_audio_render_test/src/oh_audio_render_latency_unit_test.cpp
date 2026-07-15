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
} // namespace AudioStandard
} // namespace OHOS
