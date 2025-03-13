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

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_001
* @tc.desc  : Test OHAudioCapturerErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer* audioCapturer = nullptr;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, audioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_ILLEGAL_STATE;
    auto ret = oHAudioCapturerErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_002
* @tc.desc  : Test OHAudioCapturerErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer* audioCapturer = nullptr;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, audioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_INVALID_PARAM;
    auto ret = oHAudioCapturerErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_003
* @tc.desc  : Test OHAudioCapturerErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer* audioCapturer = nullptr;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, audioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_SYSTEM;
    auto ret = oHAudioCapturerErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_004
* @tc.desc  : Test OHAudioCapturerErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_004, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer* audioCapturer = nullptr;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, audioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    auto ret = oHAudioCapturerErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_005
* @tc.desc  : Test OHAudioCapturerErrorCallback::OnError()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_005, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnError = [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    EXPECT_NE(oHAudioCapturerErrorCallback->callbacks_.OH_AudioCapturer_OnError, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    oHAudioCapturerErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_006
* @tc.desc  : Test OHAudioCapturerErrorCallback::OnError()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_006, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    EXPECT_NE(oHAudioCapturerErrorCallback->errorCallback_, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    oHAudioCapturerErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_007
* @tc.desc  : Test OHAudioCapturerErrorCallback::OnError()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_007, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnError = nullptr;
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    oHAudioCapturerErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_008
* @tc.desc  : Test OHAudioCapturerErrorCallback::OnError()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_008, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback = nullptr;
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    oHAudioCapturerErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHCapturerServiceDiedCallback API
* @tc.number: OHCapturerServiceDiedCallback_001
* @tc.desc  : Test OHCapturerServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioCaptureUnitTest, OHCapturerServiceDiedCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHCapturerServiceDiedCallback =
        std::make_shared<OHCapturerServiceDiedCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHCapturerServiceDiedCallback, nullptr);

    EXPECT_NE(oHCapturerServiceDiedCallback->errorCallback_, nullptr);

    oHCapturerServiceDiedCallback->OnAudioPolicyServiceDied();
}

/**
* @tc.name  : Test OHCapturerServiceDiedCallback API
* @tc.number: OHCapturerServiceDiedCallback_002
* @tc.desc  : Test OHCapturerServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioCaptureUnitTest, OHCapturerServiceDiedCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback = nullptr;

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHCapturerServiceDiedCallback =
        std::make_shared<OHCapturerServiceDiedCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHCapturerServiceDiedCallback, nullptr);

    oHCapturerServiceDiedCallback->OnAudioPolicyServiceDied();
}

/**
* @tc.name  : Test OHCapturerServiceDiedCallback API
* @tc.number: OHCapturerServiceDiedCallback_003
* @tc.desc  : Test OHCapturerServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioCaptureUnitTest, OHCapturerServiceDiedCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHCapturerServiceDiedCallback =
        std::make_shared<OHCapturerServiceDiedCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHCapturerServiceDiedCallback, nullptr);

    oHCapturerServiceDiedCallback->OnAudioPolicyServiceDied();
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback API
* @tc.number: OHAudioCapturerModeCallback_001
* @tc.desc  : Test OHAudioCapturerModeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 0;

    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback API
* @tc.number: OHAudioCapturerModeCallback_002
* @tc.desc  : Test OHAudioCapturerModeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 0;

    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback API
* @tc.number: OHAudioCapturerModeCallback_003
* @tc.desc  : Test OHAudioCapturerModeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_OnReadDataCallback readcallback =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(readcallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 0;

    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback API
* @tc.number: OHAudioCapturerModeCallback_004
* @tc.desc  : Test OHAudioCapturerModeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_004, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer_OnReadDataCallback readcallback =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(readcallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    oHAudioCapturerModeCallback->callbacks_ = callbacks;
    EXPECT_EQ(oHAudioCapturerModeCallback->callbacks_.OH_AudioCapturer_OnReadData, nullptr);

    size_t length = 0;

    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerDeviceChangeCallback API
* @tc.number: OHAudioCapturerDeviceChangeCallback_001
* @tc.desc  : Test OHAudioCapturerDeviceChangeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerDeviceChangeCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnStreamEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Event event) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.streamEventCallbackType_ = STREAM_EVENT_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerDeviceChangeCallback =
        std::make_shared<OHAudioCapturerDeviceChangeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerDeviceChangeCallback, nullptr);

    AudioDeviceDescriptor deviceInfo;

    oHAudioCapturerDeviceChangeCallback->OnStateChange(deviceInfo);
}

/**
* @tc.name  : Test OHAudioCapturerDeviceChangeCallback API
* @tc.number: OHAudioCapturerDeviceChangeCallback_002
* @tc.desc  : Test OHAudioCapturerDeviceChangeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerDeviceChangeCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnStreamEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Event event) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.streamEventCallbackType_ = STREAM_EVENT_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerDeviceChangeCallback =
        std::make_shared<OHAudioCapturerDeviceChangeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerDeviceChangeCallback, nullptr);

    AudioDeviceDescriptor deviceInfo;

    oHAudioCapturerDeviceChangeCallback->OnStateChange(deviceInfo);
}

/**
* @tc.name  : Test OHAudioCapturerDeviceChangeCallback API
* @tc.number: OHAudioCapturerDeviceChangeCallback_003
* @tc.desc  : Test OHAudioCapturerDeviceChangeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerDeviceChangeCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_OnDeviceChangeCallback deviceChangeCallBack =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioDeviceDescriptorArray* deviceArray) ->
        void { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.streamEventCallbackType_ = STREAM_EVENT_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerDeviceChangeCallback =
        std::make_shared<OHAudioCapturerDeviceChangeCallback>(deviceChangeCallBack, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerDeviceChangeCallback, nullptr);

    AudioDeviceDescriptor deviceInfo;

    oHAudioCapturerDeviceChangeCallback->OnStateChange(deviceInfo);
}

/**
* @tc.name  : Test OHAudioCapturerDeviceChangeCallback API
* @tc.number: OHAudioCapturerDeviceChangeCallback_004
* @tc.desc  : Test OHAudioCapturerDeviceChangeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerDeviceChangeCallback_004, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer_OnDeviceChangeCallback deviceChangeCallBack =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioDeviceDescriptorArray* deviceArray) ->
        void { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.streamEventCallbackType_ = STREAM_EVENT_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerDeviceChangeCallback =
        std::make_shared<OHAudioCapturerDeviceChangeCallback>(deviceChangeCallBack, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerDeviceChangeCallback, nullptr);

    oHAudioCapturerDeviceChangeCallback->callbacks_ = callbacks;
    EXPECT_EQ(oHAudioCapturerDeviceChangeCallback->callbacks_.OH_AudioCapturer_OnStreamEvent, nullptr);

    AudioDeviceDescriptor deviceInfo;

    oHAudioCapturerDeviceChangeCallback->OnStateChange(deviceInfo);
}

/**
* @tc.name  : Test OHAudioCapturerCallback API
* @tc.number: OHAudioCapturerCallback_001
* @tc.desc  : Test OHAudioCapturerCallback::OnInterrupt()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_OnInterruptCallback interruptCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerCallback =
        std::make_shared<OHAudioCapturerCallback>(interruptCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerCallback, nullptr);

    InterruptEvent interruptEvent;

    oHAudioCapturerCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHAudioCapturerCallback API
* @tc.number: OHAudioCapturerCallback_002
* @tc.desc  : Test OHAudioCapturerCallback::OnInterrupt()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer_OnInterruptCallback interruptCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerCallback =
        std::make_shared<OHAudioCapturerCallback>(interruptCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerCallback, nullptr);

    oHAudioCapturerCallback->callbacks_ = callbacks;
    EXPECT_EQ(oHAudioCapturerCallback->callbacks_.OH_AudioCapturer_OnInterruptEvent, nullptr);


    InterruptEvent interruptEvent;

    oHAudioCapturerCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHAudioCapturerCallback API
* @tc.number: OHAudioCapturerCallback_003
* @tc.desc  : Test OHAudioCapturerCallback::OnInterrupt()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnInterruptEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerCallback =
        std::make_shared<OHAudioCapturerCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerCallback, nullptr);

    InterruptEvent interruptEvent;

    oHAudioCapturerCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_001
* @tc.desc  : Test OHAudioCapturer::SetReadDataCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_001, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    capturerCallbacks.onReadDataCallback =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_002
* @tc.desc  : Test OHAudioCapturer::SetReadDataCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_002, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.onReadDataCallback =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_003
* @tc.desc  : Test OHAudioCapturer::SetReadDataCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_003, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_004
* @tc.desc  : Test OHAudioCapturer::SetReadDataCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_004, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_005
* @tc.desc  : Test OHAudioCapturer::SetStreamEventCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_005, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_WITH_RESULT;
    capturerCallbacks.onDeviceChangeCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioDeviceDescriptorArray* deviceArray) ->
        void { return; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_006
* @tc.desc  : Test OHAudioCapturer::SetStreamEventCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_006, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.onDeviceChangeCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioDeviceDescriptorArray* deviceArray) ->
        void { return; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_007
* @tc.desc  : Test OHAudioCapturer::SetStreamEventCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_007, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnStreamEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Event event) ->
        int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_008
* @tc.desc  : Test OHAudioCapturer::SetStreamEventCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_008, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnStreamEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Event event) ->
        int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_009
* @tc.desc  : Test OHAudioCapturer::SetInterruptCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_009, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_WITH_RESULT;
    capturerCallbacks.onInterruptEventCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_010
* @tc.desc  : Test OHAudioCapturer::SetInterruptCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_010, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.onInterruptEventCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_011
* @tc.desc  : Test OHAudioCapturer::SetInterruptCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_011, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnInterruptEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_012
* @tc.desc  : Test OHAudioCapturer::SetInterruptCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_012, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_WITH_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnInterruptEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_013
* @tc.desc  : Test OHAudioCapturer::SetErrorCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_013, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_WITH_RESULT;
    capturerCallbacks.onErrorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_014
* @tc.desc  : Test OHAudioCapturer::SetErrorCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_014, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.onErrorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_015
* @tc.desc  : Test OHAudioCapturer::SetErrorCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_015, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnError =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_016
* @tc.desc  : Test OHAudioCapturer::SetErrorCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_016, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_WITH_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnError =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    oHAudioCapturer->audioCapturer_ = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}
} // namespace AudioStandard
} // namespace OHOS
