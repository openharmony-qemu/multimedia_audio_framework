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

#include <thread>
#include <chrono>
#include "oh_audio_render_unit_test.h"

using namespace testing::ext;
using namespace std::chrono;
using namespace OHOS::AudioStandard::OHAudioRenderUT;

namespace OHOS {
namespace AudioStandard {
/**
 * @tc.name  : Test OH_AudioRenderer_SetVolume API via illegal state.
 * @tc.number: OH_Audio_Render_SetVolume_001
 * @tc.desc  : Test OH_AudioRenderer_SetVolume interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolume_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    float volumeSet = 0.5;
    OH_AudioStream_Result result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolume API via legal state.
 * @tc.number: OH_Audio_Render_SetVolume_002
 * @tc.desc  : Test OH_AudioRenderer_SetVolume interface between minimum and maximum volumes.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolume_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = 0.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolume API via legal state.
 * @tc.number: OH_Audio_Render_SetVolume_003
 * @tc.desc  : Test OH_AudioRenderer_SetVolume interface for minimum and maximum volumes.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolume_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = MIN_AUDIO_VOLUME;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    volumeSet = MAX_AUDIO_VOLUME;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolume API via illegal state.
 * @tc.number: OH_Audio_Render_SetVolume_004
 * @tc.desc  : Test OH_AudioRenderer_SetVolume interface out of volumes range.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolume_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = -0.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    volumeSet = -1.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolumeWithRamp API via illegal state.
 * @tc.number: OH_Audio_Render_SetVolumeWithRamp_001
 * @tc.desc  : Test OH_AudioRenderer_SetVolumeWithRamp interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolumeWithRamp_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    float volumeSet = MIN_AUDIO_VOLUME;
    int32_t durationMs = DURATIONMS;
    OH_AudioStream_Result result = OH_AudioRenderer_SetVolumeWithRamp(audioRenderer, volumeSet, durationMs);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolumeWithRamp API via legal state.
 * @tc.number: OH_Audio_Render_SetVolumeWithRamp_002
 * @tc.desc  : Test OH_AudioRenderer_SetVolumeWithRamp interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolumeWithRamp_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = MIN_AUDIO_VOLUME;
    int32_t durationMs = DURATIONMS;
    result = OH_AudioRenderer_SetVolumeWithRamp(audioRenderer, volumeSet, durationMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetLoudnessGain API via illegal state.
 * @tc.number: OH_Audio_Render_SetLoudnessGain_001
 * @tc.desc  : Test OH_AudioRenderer_SetLoudnessGain interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetLoudnessGain_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    float loudnessGain = VALID_LOUDNESS_GAIN;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    result = OH_AudioRenderer_SetLoudnessGain(audioRenderer, loudnessGain);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    usage = AUDIOSTREAM_USAGE_GAME;
    result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    result = OH_AudioRenderer_SetLoudnessGain(audioRenderer, loudnessGain);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via illegal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_001
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    uint32_t samplePos = 1;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    OH_AudioStream_Result result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via legal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_002
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    uint32_t samplePos = 1;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via illegal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_003
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface with incorrect samplepos value.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    uint32_t samplePos = 0;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via legal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_004
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface with callback.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_004, TestSize.Level0)
{
    // 1. create
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)LATENCY_MODE);
    OH_AudioStreamBuilder_SetSampleFormat(builder, (OH_AudioStream_SampleFormat)SAMPLE_FORMAT);
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteData;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, nullptr);
    // 3. set buffer size to FRAME_SIZE
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder, FRAME_SIZE);

    OH_AudioRenderer* audioRenderer;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(g_flag, 0);
    uint32_t samplePos = 1;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 4. start
    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_EQ(g_flag, 1);
    // 5. stop and release client
    result = OH_AudioRenderer_Stop(audioRenderer);
    result = OH_AudioRenderer_Release(audioRenderer);

    // 6. destroy the builder
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via legal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_005
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface multiple times.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_005, TestSize.Level0)
{
    // 1. create
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)LATENCY_MODE);
    OH_AudioStreamBuilder_SetSampleFormat(builder, (OH_AudioStream_SampleFormat)SAMPLE_FORMAT);
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteData;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, nullptr);
    // 3. set buffer size to FRAME_SIZE
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder, FRAME_SIZE);

    OH_AudioRenderer* audioRenderer;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    uint32_t samplePos = 1;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 4. start
    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_EQ(g_flag, 1);
    // 5. stop and release client
    result = OH_AudioRenderer_Stop(audioRenderer);
    result = OH_AudioRenderer_Release(audioRenderer);

    // 6. destroy the builder
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetDefaultOutputDevice API via legal state.
 * @tc.number: OH_AudioRenderer_SetDefaultOutputDevice
 * @tc.desc  : Test OH_AudioRenderer_SetDefaultOutputDevice interface
 *             overwrites OH_AudioRenderer_SetDefaultOutputDevice interface with valid result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetDefaultOutputDevice_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_MESSAGE;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::this_thread::sleep_for(1s);

    result = OH_AudioRenderer_SetDefaultOutputDevice(audioRenderer, AUDIO_DEVICE_TYPE_EARPIECE);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS || result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    std::this_thread::sleep_for(5s);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetDefaultOutputDevice API via legal state.
 * @tc.number: OH_AudioRenderer_SetDefaultOutputDevice
 * @tc.desc  : Test OH_AudioRenderer_SetDefaultOutputDevice interface
 *             overwrites OH_AudioRenderer_SetDefaultOutputDevice interface with valid result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetDefaultOutputDevice_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_MESSAGE;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetDefaultOutputDevice API via legal state.
 * @tc.number: OH_AudioRenderer_SetDefaultOutputDevice
 * @tc.desc  : Test OH_AudioRenderer_SetDefaultOutputDevice interface
 *             overwrites OH_AudioRenderer_SetDefaultOutputDevice interface with valid result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetDefaultOutputDevice_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_RINGTONE;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::this_thread::sleep_for(1s);

    result = OH_AudioRenderer_SetDefaultOutputDevice(audioRenderer, AUDIO_DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    std::this_thread::sleep_for(5s);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_SetSpeed_001
 * @tc.desc  : Test OH_AudioRenderer_SetSpeed interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetSpeed_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    float speed = 1.0 ;

    result = OH_AudioRenderer_SetSpeed(nullptr, speed);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_SetSpeed_002
 * @tc.desc  : Test OH_AudioRenderer_SetSpeed interface. Returns  AUDIOSTREAM_SUCCESS
 *             if all is right.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetSpeed_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    float speed = 1.0;

    result = OH_AudioRenderer_SetSpeed(audioRenderer, speed);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    float expSpeed ;
    result =OH_AudioRenderer_GetSpeed(audioRenderer, &expSpeed);
    EXPECT_EQ(speed, expSpeed);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_SetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test OH_AudioRenderer_SetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetSilentModeAndMixWithOthers_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    bool on = true;
    result = OH_AudioRenderer_SetSilentModeAndMixWithOthers(nullptr, on);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_SetSilentModeAndMixWithOthers_002
 * @tc.desc  : Test OH_AudioRenderer_SetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_SUCCESS
 *             if all is right.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetSilentModeAndMixWithOthers_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    bool on = true;
    result = OH_AudioRenderer_SetSilentModeAndMixWithOthers(audioRenderer, on);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}
} // namespace AudioStandard
} // namespace OHOS
