/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "oh_audio_capture_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
void OHAudioCaptureUnitTest::SetUpTestCase(void) { }

void OHAudioCaptureUnitTest::TearDownTestCase(void) { }

void OHAudioCaptureUnitTest::SetUp(void) { }

void OHAudioCaptureUnitTest::TearDown(void) { }

OH_AudioStreamBuilder* OHAudioCaptureUnitTest::CreateCapturerBuilder()
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStreamBuilder_Create(&builder, type);
    return builder;
}

/**
* @tc.name  : Test OH_AudioStreamBuilder_GenerateCapturer API via legal state.
* @tc.number: OH_Audio_Capture_Generate_001
* @tc.desc  : Test OH_AudioStreamBuilder_GenerateCapturer interface. Returns true, if the result is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Generate_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioStreamBuilder_GenerateCapturer API via illegal OH_AudioStream_Type.
* @tc.number: OH_Audio_Capture_Generate_002
* @tc.desc  : Test OH_AudioStreamBuilder_GenerateCapturer interface. Returns error code, if the stream type is
*             AUDIOSTREAM_TYPE_RENDERER.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Generate_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer* audioCapturer;
    result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Start API via legal state.
* @tc.number: Audio_Capturer_Start_001
* @tc.desc  : Test OH_AudioCapturer_Start interface. Returns true if start is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Start_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Start(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Start API via illegal state.
* @tc.number: Audio_Capturer_Start_002
* @tc.desc  : Test OH_AudioCapturer_Start interface. Returns error code, if Start interface is called twice.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Start_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Start(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Pause API via legal state.
* @tc.number: OH_Audio_Capture_Pause_001
* @tc.desc  : Test OH_AudioCapturer_Pause interface. Returns true if Pause is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Pause_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Pause(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Pause API via illegal state, Pause without Start first.
* @tc.number: OH_Audio_Capture_Pause_002
* @tc.desc  : Test OH_AudioCapturer_Pause interface. Returns error code, if Pause without Start first.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Pause_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Pause(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Stop API via legal state.
* @tc.number: OH_Audio_Capture_Stop_001
* @tc.desc  : Test OH_AudioCapturer_Stop interface. Returns true if Stop is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Stop_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Stop(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Stop API via illegal state, Stop without Start first.
* @tc.number: OH_Audio_Capture_Stop_002
* @tc.desc  : Test OH_AudioCapturer_Stop interface. Returns error code, if Stop without Start first.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Stop_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Stop(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Flush API via legal state.
* @tc.number: OH_Audio_Capture_Flush_001
* @tc.desc  : Test OH_AudioCapturer_Flush interface. Returns true if Flush is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Flush_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Flush(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Flush API via illegal state.
* @tc.number: OH_Audio_Capture_Flush_002
* @tc.desc  : Test OH_AudioCapturer_Flush interface. Returns error code, if Flush without Start first.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Flush_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Flush(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Release API via legal state.
* @tc.number: OH_Audio_Capture_Release_001
* @tc.desc  : Test OH_AudioCapturer_Release interface. Returns true if Release is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Release_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Release(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetLatencyMode API via legal state.
* @tc.number: OH_Audio_Capture_GetParameter_001
* @tc.desc  : Test OH_AudioCapturer_GetLatencyMode interface. Returns true if latencyMode is
*             AUDIOSTREAM_LATENCY_MODE_NORMAL,because default latency mode is AUDIOSTREAM_LATENCY_MODE_NORMAL.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetParameter_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    result = OH_AudioCapturer_GetLatencyMode(audioCapturer, &latencyMode);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(latencyMode == AUDIOSTREAM_LATENCY_MODE_NORMAL);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_001
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_PREPARED.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_PREPARED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_002
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_RUNNING.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioCapturer_Start(audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_RUNNING);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_003
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_PAUSED.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioCapturer_Start(audioCapturer);
    OH_AudioCapturer_Pause(audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(state, AUDIOSTREAM_STATE_PAUSED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_004
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_STOPPED.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioCapturer_Start(audioCapturer);
    OH_AudioCapturer_Stop(audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(state, AUDIOSTREAM_STATE_STOPPED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_005
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_RELEASED.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioCapturer_Start(audioCapturer);
    OH_AudioCapturer_Release(audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    // The internal capturer has been released, returning an error.
    EXPECT_EQ(state, AUDIOSTREAM_STATE_INVALID);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetStreamId API via legal state.
* @tc.number: OH_Audio_Capture_GetParameter_002
* @tc.desc  : Test OH_AudioCapturer_GetStreamId interface. Returns true if the result is AUDIOSTREAM_SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetParameter_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    uint32_t streamId;
    result = OH_AudioCapturer_GetStreamId(audioCapturer, &streamId);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetSamplingRate API via legal state.
* @tc.number: OH_Audio_Capture_GetSamplingRate_001
* @tc.desc  : Test OH_AudioCapturer_GetSamplingRate interface. Returns true if samplingRate is
*             SAMPLE_RATE_48000,because default samplingRate is SAMPLE_RATE_48000.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetSamplingRate_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    int32_t rate;
    result = OH_AudioCapturer_GetSamplingRate(audioCapturer, &rate);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(rate == SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetSampleFormat API via legal state.
* @tc.number: OH_Audio_Capture_GetSampleFormat_001
* @tc.desc  : Test OH_AudioCapturer_GetSampleFormat interface. Returns true if sampleFormat is
*             AUDIOSTREAM_SAMPLE_S16LE,because default sampleFormat is AUDIOSTREAM_SAMPLE_S16LE.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapture_GetSampleFormat_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_SampleFormat sampleFormat;
    result = OH_AudioCapturer_GetSampleFormat(audioCapturer, &sampleFormat);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(sampleFormat == AUDIOSTREAM_SAMPLE_S16LE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetEncodingType API via legal state.
* @tc.number: OH_Audio_Capture_GetEncodingType_001
* @tc.desc  : Test OH_AudioCapturer_GetEncodingType interface. Returns true if encodingType is
*             ENCODING_PCM,because default encodingType is ENCODING_PCM.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetEncodingType_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_EncodingType encodingType;
    result = OH_AudioCapturer_GetEncodingType(audioCapturer, &encodingType);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(encodingType == AUDIOSTREAM_ENCODING_TYPE_RAW);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCapturerInfo API via legal state.
* @tc.number: OH_Audio_Capture_GetCapturerInfo_001
* @tc.desc  : Test OH_AudioCapturer_GetCapturerInfo interface. Returns true if sourceType is
*             SOURCE_TYPE_MIC,because default sourceType is SOURCE_TYPE_MIC.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetCapturerInfo_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_SourceType sourceType;
    result = OH_AudioCapturer_GetCapturerInfo(audioCapturer, &sourceType);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(sourceType == AUDIOSTREAM_SOURCE_TYPE_MIC);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetTimestamp API via illegal state.
* @tc.number: OH_AudioCapturer_GetTimestamp_001
* @tc.desc  : Test OH_AudioCapturer_GetTimestamp interface. Return false if clockId is not CLOCK_MONOTONIC
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetTimestamp_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    int64_t framePosition;
    int64_t timestamp;
    result = OH_AudioCapturer_GetTimestamp(audioCapturer, CLOCK_REALTIME, &framePosition, &timestamp);
    EXPECT_TRUE(result != AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetTimestamp API via illegal state.
* @tc.number: OH_AudioCapturer_GetTimestamp_002
* @tc.desc  : Test OH_AudioCapturer_GetTimestamp interface.Returns false if capturer is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetTimestamp_002, TestSize.Level0)
{
    OH_AudioCapturer* audioCapturer = nullptr;
    int64_t framePosition;
    int64_t timestamp;
    OH_AudioStream_Result result = OH_AudioCapturer_GetTimestamp(audioCapturer, CLOCK_MONOTONIC,
        &framePosition, &timestamp);
    EXPECT_TRUE(result != AUDIOSTREAM_SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
