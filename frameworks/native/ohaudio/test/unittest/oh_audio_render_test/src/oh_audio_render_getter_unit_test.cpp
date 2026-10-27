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
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_001
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Return true if the result state is
 *             AUDIOSTREAM_STATE_PREPARED.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_State state;
    result = OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_PREPARED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_002
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Return true if the result state is
 *             AUDIOSTREAM_STATE_RUNNING.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioRenderer_Start(audioRenderer);

    OH_AudioStream_State state;
    result = OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_RUNNING);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_003
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Return true if the result state is
 *             AUDIOSTREAM_STATE_PAUSED.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioRenderer_Start(audioRenderer);
    OH_AudioRenderer_Pause(audioRenderer);

    OH_AudioStream_State state;
    result = OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_PAUSED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_004
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Return true if the result state is
 *             AUDIOSTREAM_STATE_STOPPED.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioRenderer_Start(audioRenderer);
    OH_AudioRenderer_Stop(audioRenderer);

    OH_AudioStream_State state;
    result = OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_STOPPED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_005
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_005, TestSize.Level0)
{
    OH_AudioStream_State state;
    OH_AudioStream_Result result = OH_AudioRenderer_GetCurrentState(nullptr, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_006
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if state is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_006, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioRenderer_Start(audioRenderer);
    OH_AudioRenderer_Stop(audioRenderer);

    result = OH_AudioRenderer_GetCurrentState(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatencyMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetLatencyMode_001
 * @tc.desc  : Test OH_AudioRenderer_GetLatencyMode interface. Returns true if latencyMode is
 *             AUDIOSTREAM_LATENCY_MODE_NORMAL,because default latency mode is AUDIOSTREAM_LATENCY_MODE_NORMAL.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetLatencyMode_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    result = OH_AudioRenderer_GetLatencyMode(audioRenderer, &latencyMode);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(latencyMode == AUDIOSTREAM_LATENCY_MODE_NORMAL);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatencyMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetLatencyMode_002
 * @tc.desc  : Test OH_AudioRenderer_GetLatencyMode interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetLatencyMode_002, TestSize.Level0)
{
    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    OH_AudioStream_Result result = OH_AudioRenderer_GetLatencyMode(nullptr, &latencyMode);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatencyMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetLatencyMode_003
 * @tc.desc  : Test OH_AudioRenderer_GetLatencyMode interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if latencyMode is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetLatencyMode_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetLatencyMode(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetStreamId API via legal state.
 * @tc.number: OH_AudioRenderer_GetStreamId_001
 * @tc.desc  : Test OH_AudioRenderer_GetStreamId interface. Returns true if the result is AUDIOSTREAM_SUCCESS.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetStreamId_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    uint32_t streamId;
    result = OH_AudioRenderer_GetStreamId(audioRenderer, &streamId);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetStreamId API via legal state.
 * @tc.number: OH_AudioRenderer_GetStreamId_002
 * @tc.desc  : Test OH_AudioRenderer_GetStreamId interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetStreamId_002, TestSize.Level0)
{
    uint32_t streamId;
    OH_AudioStream_Result result = OH_AudioRenderer_GetStreamId(nullptr, &streamId);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetStreamId API via legal state.
 * @tc.number: OH_AudioRenderer_GetStreamId_003
 * @tc.desc  : Test OH_AudioRenderer_GetStreamId interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if streamId is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetStreamId_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetStreamId(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSamplingRate API via legal state.
 * @tc.number: OH_Audio_Render_GetSamplingRate_001
 * @tc.desc  : Test OH_AudioRenderer_GetSamplingRate interface. Returns true if samplingRate is
 *             SAMPLE_RATE_48000,because default samplingRate is SAMPLE_RATE_48000.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSamplingRate_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int32_t rate;
    result = OH_AudioRenderer_GetSamplingRate(audioRenderer, &rate);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(rate == SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSamplingRate API via legal state.
 * @tc.number: OH_Audio_Render_GetSamplingRate_002
 * @tc.desc  : Test OH_AudioRenderer_GetSamplingRate interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSamplingRate_002, TestSize.Level0)
{
    int32_t rate;
    OH_AudioStream_Result result = OH_AudioRenderer_GetSamplingRate(nullptr, &rate);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSamplingRate API via legal state.
 * @tc.number: OH_Audio_Render_GetSamplingRate_003
 * @tc.desc  : Test OH_AudioRenderer_GetSamplingRate interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if rate is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSamplingRate_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetSamplingRate(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSampleFormat API via legal state.
 * @tc.number: OH_Audio_Render_GetSampleFormat_001
 * @tc.desc  : Test OH_AudioRenderer_GetSampleFormat interface. Returns true if sampleFormat is
 *             AUDIOSTREAM_SAMPLE_S16LE,because default sampleFormat is AUDIOSTREAM_SAMPLE_S16LE.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSampleFormat_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_SampleFormat sampleFormat;
    result = OH_AudioRenderer_GetSampleFormat(audioRenderer, &sampleFormat);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(sampleFormat == AUDIOSTREAM_SAMPLE_S16LE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSampleFormat API via legal state.
 * @tc.number: OH_Audio_Render_GetSampleFormat_002
 * @tc.desc  : Test OH_AudioRenderer_GetSampleFormat interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSampleFormat_002, TestSize.Level0)
{
    OH_AudioStream_SampleFormat sampleFormat;
    OH_AudioStream_Result result = OH_AudioRenderer_GetSampleFormat(nullptr, &sampleFormat);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSampleFormat API via legal state.
 * @tc.number: OH_Audio_Render_GetSampleFormat_003
 * @tc.desc  : Test OH_AudioRenderer_GetSampleFormat interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if sampleFormat is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSampleFormat_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetSampleFormat(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEncodingType API via legal state.
 * @tc.number: OH_Audio_Render_GetEncodingType_001
 * @tc.desc  : Test OH_AudioRenderer_GetEncodingType interface. Returns true if encodingType is
 *             ENCODING_PCM,because default encodingType is ENCODING_PCM.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetEncodingType_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_EncodingType encodingType;
    result = OH_AudioRenderer_GetEncodingType(audioRenderer, &encodingType);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(encodingType == AUDIOSTREAM_ENCODING_TYPE_RAW);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEncodingType API via legal state.
 * @tc.number: OH_Audio_Render_GetEncodingType_002
 * @tc.desc  : Test OH_AudioRenderer_GetEncodingType interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetEncodingType_002, TestSize.Level0)
{
    OH_AudioStream_EncodingType encodingType;
    OH_AudioStream_Result result = OH_AudioRenderer_GetEncodingType(nullptr, &encodingType);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEncodingType API via legal state.
 * @tc.number: OH_Audio_Render_GetEncodingType_003
 * @tc.desc  : Test OH_AudioRenderer_GetEncodingType interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if encodingType is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetEncodingType_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetEncodingType(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFramesWritten API via legal state.
 * @tc.number: OH_AudioRenderer_GetFramesWritten_001
 * @tc.desc  : Test OH_AudioRenderer_GetFramesWritten interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFramesWritten_001, TestSize.Level0)
{
    int64_t frames;
    OH_AudioStream_Result result = OH_AudioRenderer_GetFramesWritten(nullptr, &frames);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFramesWritten API via legal state.
 * @tc.number: OH_AudioRenderer_GetFramesWritten_002
 * @tc.desc  : Test OH_AudioRenderer_GetFramesWritten interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if frames is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFramesWritten_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetFramesWritten(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFramesWritten API via legal state.
 * @tc.number: OH_AudioRenderer_GetFramesWritten_003
 * @tc.desc  : Test OH_AudioRenderer_GetFramesWritten interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFramesWritten_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int64_t frames;
    result = OH_AudioRenderer_GetFramesWritten(audioRenderer, &frames);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererInfo API via legal state.
 * @tc.number: OH_Audio_Render_GetRendererInfo_001
 * @tc.desc  : Test OH_AudioRenderer_GetRendererInfo interface. Returns true if usage is STREAM_USAGE_MEDIA and content
 *             is CONTENT_TYPE_MUSIC.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetRendererInfo_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_Usage usage;
    result = OH_AudioRenderer_GetRendererInfo(audioRenderer, &usage);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(usage, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererInfo API via legal state.
 * @tc.number: OH_Audio_Render_GetRendererInfo_002
 * @tc.desc  : Test OH_AudioRenderer_GetRendererInfo interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetRendererInfo_002, TestSize.Level0)
{
    OH_AudioStream_Usage usage;
    OH_AudioStream_Result result = OH_AudioRenderer_GetRendererInfo(nullptr, &usage);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererInfo API via legal state.
 * @tc.number: OH_Audio_Render_GetRendererInfo_003
 * @tc.desc  : Test OH_AudioRenderer_GetRendererInfo interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if usage is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetRendererInfo_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetRendererInfo(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererPrivacy API.
 * @tc.number: OH_AudioRenderer_GetRendererPrivacy_001
 * @tc.desc  : Test OH_AudioRenderer_GetRendererPrivacy interface with default privacy AUDIO_STREAM_PRIVACY_TYPE_PUBLIC.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetRendererPrivacy_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_PrivacyType privacyType;
    result = OH_AudioRenderer_GetRendererPrivacy(audioRenderer, &privacyType);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(privacyType, AUDIO_STREAM_PRIVACY_TYPE_PUBLIC);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererPrivacy API.
 * @tc.number: OH_AudioRenderer_GetRendererPrivacy_002
 * @tc.desc  : Test OH_AudioRenderer_GetRendererPrivacy interface with privacy AUDIO_STREAM_PRIVACY_TYPE_PRIVATE.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetRendererPrivacy_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStream_Result privacyResult = OH_AudioStreamBuilder_SetRendererPrivacy(builder,
        AUDIO_STREAM_PRIVACY_TYPE_PRIVATE);
    EXPECT_EQ(privacyResult, AUDIOSTREAM_SUCCESS);
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_PrivacyType privacyType;
    result = OH_AudioRenderer_GetRendererPrivacy(audioRenderer, &privacyType);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(privacyType, AUDIO_STREAM_PRIVACY_TYPE_PRIVATE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererPrivacy API.
 * @tc.number: OH_AudioRenderer_GetRendererPrivacy_003
 * @tc.desc  : Test OH_AudioRenderer_GetRendererPrivacy interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetRendererPrivacy_003, TestSize.Level0)
{
    OH_AudioStream_PrivacyType privacyType;
    OH_AudioStream_Result result = OH_AudioRenderer_GetRendererPrivacy(nullptr, &privacyType);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererPrivacy API.
 * @tc.number: OH_AudioRenderer_GetRendererPrivacy_004
 * @tc.desc  : Test OH_AudioRenderer_GetRendererPrivacy interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if privacy is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetRendererPrivacy_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetRendererPrivacy(audioRenderer, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelLayout API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelLayout_001
 * @tc.desc  : Test OH_AudioRenderer_GetChannelLayout interface. Returns true if channelLayout is
 *             CH_LAYOUT_UNKNOWN, because default channelLayout is CH_LAYOUT_UNKNOWN.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelLayout_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioChannelLayout channelLayout;
    result = OH_AudioRenderer_GetChannelLayout(audioRenderer, &channelLayout);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(channelLayout == OH_AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelLayout API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelLayout_002
 * @tc.desc  : Test OH_AudioRenderer_GetChannelLayout interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelLayout_002, TestSize.Level0)
{
    OH_AudioChannelLayout channelLayout;
    OH_AudioStream_Result result = OH_AudioRenderer_GetChannelLayout(nullptr, &channelLayout);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelLayout API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelLayout_003
 * @tc.desc  : Test OH_AudioRenderer_GetChannelLayout interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if channelLayout is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelLayout_003, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetChannelLayout(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEffectMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetEffectMode_001
 * @tc.desc  : Test OH_AudioRenderer_GetEffectMode interface. Returns true if effect mode is the same as set.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetEffectMode_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_AudioEffectMode effectMode;
    result = OH_AudioRenderer_SetEffectMode(audioRenderer, OH_AudioStream_AudioEffectMode::EFFECT_DEFAULT);
    EXPECT_TRUE(result == OH_AudioStream_Result::AUDIOSTREAM_SUCCESS);
    result = OH_AudioRenderer_GetEffectMode(audioRenderer, &effectMode);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(effectMode == OH_AudioStream_AudioEffectMode::EFFECT_DEFAULT);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEffectMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetEffectMode_002
 * @tc.desc  : Test OH_AudioRenderer_GetEffectMode interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetEffectMode_002, TestSize.Level0)
{
    OH_AudioStream_AudioEffectMode effectMode;
    OH_AudioStream_Result result = OH_AudioRenderer_GetEffectMode(nullptr, &effectMode);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEffectMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetEffectMode_003
 * @tc.desc  : Test OH_AudioRenderer_GetEffectMode interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if effectMode is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetEffectMode_003, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetEffectMode(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

#ifdef AUDIO_OH_RENDER_UNIT_TEST
/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_001
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::mutex mutex;
    std::condition_variable cv;
    int32_t count = 0;
    writeCallbackMock.Install([&count, &mutex, &cv](OH_AudioRenderer* renderer, void* userData,
        void* buffer,
        int32_t bufferLen) {
            std::lock_guard lock(mutex);
            cv.notify_one();

            // sleep time trigger underflow
            if (count == 1) {
                std::this_thread::sleep_for(200ms);
            }
            count++;
        });

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::unique_lock lock(mutex);
    cv.wait_for(lock, 1s, [&count] {
        // count > 1 ensure sleeped
        return count > 1;
    });
    lock.unlock();

    uint32_t underFlowCount;
    result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_002
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::mutex mutex;
    std::condition_variable cv;
    int32_t count = 0;
    writeCallbackMock.Install([&count, &mutex, &cv](OH_AudioRenderer* renderer, void* userData,
        void* buffer,
        int32_t bufferLen) {
            std::lock_guard lock(mutex);
            cv.notify_one();

            // sleep time trigger underflow
            if (count == 0) {
                std::this_thread::sleep_for(200ms);
            }
            count++;
        });

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::unique_lock lock(mutex);
    cv.wait_for(lock, 1s, [&count] {
        // count > 1 ensure sleeped
        return count > 1;
    });
    lock.unlock();

    uint32_t underFlowCount;
    result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(underFlowCount, 0);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_003
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
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

    uint32_t underFlowCount;
    result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(underFlowCount, 0);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_004
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_004, TestSize.Level0)
{
    for (auto sleepTimes : {200ms, 400ms, 600ms}) {
        OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

        OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
        OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
        OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
        OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

        OHAudioRendererWriteCallbackMock writeCallbackMock;

        OH_AudioRenderer_Callbacks callbacks;
        callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
        OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

        OH_AudioRenderer* audioRenderer;
        OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
        EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

        std::mutex mutex;
        std::condition_variable cv;
        int32_t count = 0;
        writeCallbackMock.Install([&count, &mutex, &cv, sleepTimes](OH_AudioRenderer* renderer, void* userData,
            void* buffer,
            int32_t bufferLen) {
                std::lock_guard lock(mutex);
                cv.notify_one();

                // sleep time trigger underflow
                if (count == 1) {
                    std::this_thread::sleep_for(sleepTimes);
                }
                count++;
            });

        result = OH_AudioRenderer_Start(audioRenderer);
        EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

        std::unique_lock lock(mutex);
        cv.wait_for(lock, 1s, [&count] {
            // count > 1 ensure sleeped
            return count > 10;
        });
        lock.unlock();

        uint32_t underFlowCount = 0;
        result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
        EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

        OH_AudioRenderer_Stop(audioRenderer);
        OH_AudioRenderer_Release(audioRenderer);

        OH_AudioStreamBuilder_Destroy(builder);
    }
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_005
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
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

    uint32_t underFlowCount;
    result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(underFlowCount, 0);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}
#endif
/**
 * @tc.name  : Test OH_AudioRenderer_GetVolume API via illegal state.
 * @tc.number: OH_Audio_Render_GetVolume_001
 * @tc.desc  : Test OH_AudioRenderer_GetVolume interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetVolume_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    float volume;
    OH_AudioStream_Result result = OH_AudioRenderer_GetVolume(audioRenderer, &volume);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetVolume API via legal state.
 * @tc.number: OH_Audio_Render_GetVolume_002
 * @tc.desc  : Test OH_AudioRenderer_GetVolume interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetVolume_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volume;
    result = OH_AudioRenderer_GetVolume(audioRenderer, &volume);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetVolume API via legal state.
 * @tc.number: OH_Audio_Render_GetVolume_003
 * @tc.desc  : Test OH_AudioRenderer_GetVolume interface after set volume call.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetVolume_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = 0.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    float volumeGet;
    result = OH_AudioRenderer_GetVolume(audioRenderer, &volumeGet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(volumeGet, 0.5);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetVolume API via legal state.
 * @tc.number: OH_Audio_Render_GetVolume_004
 * @tc.desc  : Test OH_AudioRenderer_GetVolume interface after set volume fails.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetVolume_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = 0.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    volumeSet = 1.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    float volumeGet;
    result = OH_AudioRenderer_GetVolume(audioRenderer, &volumeGet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(volumeGet, 0.5);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLoudnessGain API via illegal stream usage.
 * @tc.number: OH_Audio_Render_GetLoudnessGain_001
 * @tc.desc  : Test OH_AudioRenderer_GetLoudnessGain interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLoudnessGain_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_GAME;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float loudnessGain;
    result = OH_AudioRenderer_GetLoudnessGain(audioRenderer, &loudnessGain);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(loudnessGain, 0.0f);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLoudnessGain API via illegal latency mode.
 * @tc.number: OH_Audio_Render_GetLoudnessGain_002
 * @tc.desc  : Test OH_AudioRenderer_GetLoudnessGain interface after set loudnessGain call.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLoudnessGain_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_FAST;
    result = OH_AudioStreamBuilder_SetLatencyMode(builder, latencyMode);
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float loudnessGainSet = VALID_LOUDNESS_GAIN;
    result = OH_AudioRenderer_SetLoudnessGain(audioRenderer, loudnessGainSet);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    float loudnessGainGet;
    result = OH_AudioRenderer_GetLoudnessGain(audioRenderer, &loudnessGainGet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_FLOAT_EQ(loudnessGainGet, 0.0f);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLoudnessGain API via legal and illegal loudnessGain.
 * @tc.number: OH_Audio_Render_GetLoudnessGain_003
 * @tc.desc  : Test OH_AudioRenderer_GetLoudnessGain interface after set loudnessGain fails.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLoudnessGain_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float loudnessGainSet = VALID_LOUDNESS_GAIN;
    result = OH_AudioRenderer_SetLoudnessGain(audioRenderer, loudnessGainSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    loudnessGainSet = INVALID_LOUDNESS_GAIN;
    result = OH_AudioRenderer_SetLoudnessGain(audioRenderer, loudnessGainSet);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    float loudnessGainGet = 0.0f;
    result = OH_AudioRenderer_GetLoudnessGain(audioRenderer, &loudnessGainGet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_FLOAT_EQ(loudnessGainGet, VALID_LOUDNESS_GAIN);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelCount API via illegal state.
 * @tc.number: OH_Audio_Render_GetChannelCount_001
 * @tc.desc  : Test OH_AudioRenderer_GetChannelCount interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetChannelCount_001, TestSize.Level0)
{
    int32_t channelCount = 0;
    OH_AudioStream_Result result = OH_AudioRenderer_GetChannelCount(nullptr, &channelCount);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelCount API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelCount_002
 * @tc.desc  : Test OH_AudioRenderer_GetChannelCount interface. Returns AUDIOSTREAM_SUCCESS.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelCount_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int32_t channelCount = 0;
    result = OH_AudioRenderer_GetChannelCount(audioRenderer, &channelCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelCount API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelCount_003
 * @tc.desc  : Test OH_AudioRenderer_GetChannelCount interface with nullptr channelCount.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelCount_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetChannelCount(audioRenderer, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_002
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if GetAudioTime error
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_MONOTONIC;
    int64_t framePosition = 0;
    int64_t timestamp = 0;

    result = OH_AudioRenderer_GetTimestamp(audioRenderer, clockId, &framePosition, &timestamp);

    EXPECT_NE(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_003
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_MONOTONIC;
    int64_t framePosition = 0;
    int64_t timestamp = 0;

    result = OH_AudioRenderer_GetTimestamp(nullptr, clockId, &framePosition, &timestamp);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_004
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             when clockId is CLOCK_REALTIME instead of CLOCK_MONOTONIC .
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_REALTIME;
    int64_t framePosition = 0;
    int64_t timestamp = 0;

    result = OH_AudioRenderer_GetTimestamp(audioRenderer, clockId, &framePosition, &timestamp);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_005
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if framePosition is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_MONOTONIC;
    int64_t timestamp = 0;

    result = OH_AudioRenderer_GetTimestamp(audioRenderer, clockId, nullptr, &timestamp);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_006
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if timestamp is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_006, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_MONOTONIC;
    int64_t framePosition = 0;

    result = OH_AudioRenderer_GetTimestamp(audioRenderer, clockId, &framePosition, nullptr);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFrameSizeInCallback API via legal state.
 * @tc.number: OH_AudioRenderer_GetFrameSizeInCallback_001
 * @tc.desc  : Test OH_AudioRenderer_GetFrameSizeInCallback interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFrameSizeInCallback_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int32_t frameSize = 0;

    result = OH_AudioRenderer_GetFrameSizeInCallback(nullptr, &frameSize);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFrameSizeInCallback API via legal state.
 * @tc.number: OH_AudioRenderer_GetFrameSizeInCallback_002
 * @tc.desc  : Test OH_AudioRenderer_GetFrameSizeInCallback interface. Returns  AUDIOSTREAM_SUCCESS
 *             if all is right.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFrameSizeInCallback_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int32_t frameSize = 0;

    result = OH_AudioRenderer_GetFrameSizeInCallback(audioRenderer, &frameSize);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFrameSizeInCallback API via legal state.
 * @tc.number: OH_AudioRenderer_GetFrameSizeInCallback_003
 * @tc.desc  : Test OH_AudioRenderer_GetFrameSizeInCallback interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if frameSize is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFrameSizeInCallback_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetFrameSizeInCallback(audioRenderer, nullptr);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_GetSpeed_001
 * @tc.desc  : Test OH_AudioRenderer_GetSpeed interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSpeed_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    float speed = 1.0 ;

    result = OH_AudioRenderer_GetSpeed(nullptr, &speed);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_GetSpeed_002
 * @tc.desc  : Test OH_AudioRenderer_GetSpeed interface. Returns  AUDIOSTREAM_SUCCESS
 *             if all is right.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSpeed_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    float speed = 1.0 ;

    result = OH_AudioRenderer_GetSpeed(audioRenderer, &speed);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_GetSpeed_003
 * @tc.desc  : Test OH_AudioRenderer_GetSpeed interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if speed is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSpeed_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetSpeed(audioRenderer, nullptr);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_GetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSilentModeAndMixWithOthers_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    bool on = true;
    result = OH_AudioRenderer_GetSilentModeAndMixWithOthers(nullptr, &on);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_GetSilentModeAndMixWithOthers_002
 * @tc.desc  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_SUCCESS
 *             if renderer is not nullptr .
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSilentModeAndMixWithOthers_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    bool on = true;
    result = OH_AudioRenderer_SetSilentModeAndMixWithOthers(audioRenderer, on);
    result = OH_AudioRenderer_GetSilentModeAndMixWithOthers(audioRenderer, &on);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_GetSilentModeAndMixWithOthers_003
 * @tc.desc  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if on is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSilentModeAndMixWithOthers_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetSilentModeAndMixWithOthers(audioRenderer, nullptr);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLoudnessGain API via legal and illegal loudnessGain.
 * @tc.number: OH_Audio_Render_GetLoudnessGain_004
 * @tc.desc  : Test OH_AudioRenderer_GetLoudnessGain interface after set loudnessGain fails.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetLoudnessGain_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_GAME;
    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_FAST;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    result = OH_AudioStreamBuilder_SetLatencyMode(builder, latencyMode);
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float loudnessGain;
    result = OH_AudioRenderer_GetLoudnessGain(audioRenderer, &loudnessGain);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(loudnessGain, 0.0f);
    OH_AudioStreamBuilder_Destroy(builder);
}
} // namespace AudioStandard
} // namespace OHOS
