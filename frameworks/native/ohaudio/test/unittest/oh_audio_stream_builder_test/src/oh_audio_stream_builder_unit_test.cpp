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

#include "oh_audio_stream_builder_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

namespace {
    const int32_t SAMPLING_RATE_44100 = 44100;
    const int32_t SAMPLING_RATE_48000 = 48000;
    const int32_t SAMPLING_RATE_88200 = 88200;
    const int32_t SAMPLING_RATE_176400 = 176400;
    const int32_t SAMPLING_RATE_192000 = 192000;
    const int32_t SAMPLING_RATE_WRONG_NUM = -1;

    const int32_t CHANNEL_COUNT_1 = 1;
    const int32_t CHANNEL_COUNT_16 = 16;
    const int32_t CHANNEL_COUNT_18 = 18;

    FILE *g_file;
    bool g_readEnd = false;
}

const int32_t SAMPLING_RATE = 48000; // 48000:SAMPLING_RATE value
const int32_t CHANNEL_COUNT = 2; // 2:CHANNEL_COUNT value
const int32_t g_latencyMode = 0;
const int32_t g_sampleFormat = 1;
const int32_t g_frameSize = 240; // 240:g_frameSize value

static int32_t AudioRendererOnWriteDataInterrupt(OH_AudioRenderer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return 0;
}

static int32_t AudioRendererInterruptEvent(OH_AudioRenderer* renderer,
    void* userData,
    OH_AudioInterrupt_ForceType type,
    OH_AudioInterrupt_Hint hint)
{
    printf("AudioRendererInterruptEvent type = %d, hint = %d\n", type, hint);
    return 0;
}

static int32_t AudioRendererOnWriteData(OH_AudioRenderer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    size_t readCount = fread(buffer, bufferLen, 1, g_file);
    if (!readCount) {
        g_readEnd = true;
        if (ferror(g_file)) {
            printf("Error reading myfile");
        } else if (feof(g_file)) {
            printf("EOF found");
        }
    }

    return 0;
}

static int32_t AudioRendererWriteDataWithMetadataCallback(OH_AudioRenderer *renderer,
    void *userData, void *audioData, int32_t audioDataSize, void *metadata, int32_t metadataSize)
{
    size_t readCount = fread(audioData, audioDataSize, 1, g_file);
    if (!readCount) {
        g_readEnd = true;
        if (ferror(g_file)) {
            printf("Error reading myfile");
        } else if (feof(g_file)) {
            printf("EOF found");
        }
        return 0;
    }
    readCount = fread(metadata, metadataSize, 1, g_file);
    if (!readCount) {
        g_readEnd = true;
        if (ferror(g_file)) {
            printf("Error reading myfile");
        } else if (feof(g_file)) {
            printf("EOF found");
        }
    }
    return 0;
}

static int32_t AudioCapturerOnReadData(
    OH_AudioCapturer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    printf("callback success\n");
    return 0;
}

static OH_AudioData_Callback_Result OnWriteDataCallbackWithValidData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

static OH_AudioData_Callback_Result OnWriteDataCallbackWithInvalidData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return AUDIO_DATA_CALLBACK_RESULT_INVALID;
}


void OHAudioStreamBuilderUnitTest::SetUpTestCase(void) { }

void OHAudioStreamBuilderUnitTest::TearDownTestCase(void) { }

void OHAudioStreamBuilderUnitTest::SetUp(void) { }

void OHAudioStreamBuilderUnitTest::TearDown(void) { }

/**
 * @tc.name  : Test OH_AudioStreamBuilder_Create API via legal state, AUDIOSTREAM_TYPE_RENDERER.
 * @tc.number: OH_AudioStreamBuilder_Create_001
 * @tc.desc  : Test OH_AudioStreamBuilder_Create interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_Create_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = nullptr;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_Create API via legal state, AUDIOSTREAM_TYPE_CAPTURER.
 * @tc.number: OH_AudioStreamBuilder_Create_001
 * @tc.desc  : Test OH_AudioStreamBuilder_Create interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_Create_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_Destroy API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_Destroy_001
 * @tc.desc  : Test OH_AudioStreamBuilder_Destroy interface. Returns error code, if builder pointer is nullptr.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_Destroy_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = nullptr;

    OH_AudioStream_Result result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSamplingRate API via legal state, samplingRate is 48000.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSamplingRate interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = SAMPLING_RATE_48000;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSamplingRate API via legal state, samplingRate is 44100.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSamplingRate interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = SAMPLING_RATE_44100;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSamplingRate API via legal state, samplingRate is 88200.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_003
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSamplingRate interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = SAMPLING_RATE_88200;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSamplingRate API via legal state, samplingRate is 176400.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_018
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSamplingRate interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_018, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = SAMPLING_RATE_176400;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSamplingRate API via legal state, samplingRate is 192000.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_019
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSamplingRate interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_019, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = SAMPLING_RATE_192000;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSamplingRate API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_020
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSamplingRate interface. Returns error code, if samplingRate is -1.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_020, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = SAMPLING_RATE_WRONG_NUM;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetChannelCount API via legal state, channelCount is 1.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_004
 * @tc.desc  : Test OH_AudioStreamBuilder_SetChannelCount interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t channelCount = CHANNEL_COUNT_1;
    result = OH_AudioStreamBuilder_SetChannelCount(builder, channelCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetChannelCount API via legal state, channelCount is 16.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_005
 * @tc.desc  : Test OH_AudioStreamBuilder_SetChannelCount interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t channelCount = CHANNEL_COUNT_16;
    result = OH_AudioStreamBuilder_SetChannelCount(builder, channelCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetChannelCount API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_006
 * @tc.desc  : Test OH_AudioStreamBuilder_SetChannelCount interface. Returns error code, if channelCount is 8.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_006, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t channelCount = CHANNEL_COUNT_18;
    result = OH_AudioStreamBuilder_SetChannelCount(builder, channelCount);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetEncodingType API via legal state, AUDIOSTREAM_ENCODING_TYPE_RAW.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_007
 * @tc.desc  : Test OH_AudioStreamBuilder_SetEncodingType interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_007, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_EncodingType encodingType = AUDIOSTREAM_ENCODING_TYPE_RAW;
    result = OH_AudioStreamBuilder_SetEncodingType(builder, encodingType);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInfo API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_008
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInfo interface. Returns error code, if stream type is
 *             AUDIOSTREAM_TYPE_CAPTURER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_008, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInfo API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_009
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInfo interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_009, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
    result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInfo API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_010
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInfo interface. Returns error code, if builder type
 *             is AUDIOSTREAM_TYPE_CAPTURER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_010, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
    result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererCallback API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_011
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererCallback interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_011, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteData;
    result = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, NULL);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererCallback API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_012
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererCallback interface. Returns error code, builder type
 *             is AUDIOSTREAM_TYPE_CAPTURER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_012, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteData;
    result = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, NULL);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererPrivacy API.
 * @tc.number: OH_AudioStreamBuilder_SetRendererPrivacy_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererPrivacy interface via illegal stream type.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererPrivacy_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetRendererPrivacy(builder, AUDIO_STREAM_PRIVACY_TYPE_PUBLIC);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererPrivacy API.
 * @tc.number: OH_AudioStreamBuilder_SetRendererPrivacy_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererPrivacy interface via public privacy.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererPrivacy_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetRendererPrivacy(builder, AUDIO_STREAM_PRIVACY_TYPE_PUBLIC);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererPrivacy API.
 * @tc.number: OH_AudioStreamBuilder_SetRendererPrivacy_003
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererPrivacy interface via private privacy.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererPrivacy_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetRendererPrivacy(builder, AUDIO_STREAM_PRIVACY_TYPE_PRIVATE);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetCapturerInfo API via legal state,
 *             sourceType is AUDIOSTREAM_SOURCE_TYPE_MIC.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_013
 * @tc.desc  : Test OH_AudioStreamBuilder_SetCapturerInfo interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_013, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_MIC;
    result = OH_AudioStreamBuilder_SetCapturerInfo(builder, sourceType);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetCapturerInfo API via legal state,
 *             sourceType is AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_014
 * @tc.desc  : Test OH_AudioStreamBuilder_SetCapturerInfo interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_014, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION ;
    result = OH_AudioStreamBuilder_SetCapturerInfo(builder, sourceType);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetCapturerInfo API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_015
 * @tc.desc  : Test OH_AudioStreamBuilder_SetCapturerInfo interface. Returns error code, if the builder type is
 *             AUDIOSTREAM_TYPE_RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_015, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_VOICE_RECOGNITION ;
    result = OH_AudioStreamBuilder_SetCapturerInfo(builder, sourceType);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetCapturerCallback API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_016
 * @tc.desc  : Test OH_AudioStreamBuilder_SetCapturerCallback interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_016, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData = AudioCapturerOnReadData;
    result = OH_AudioStreamBuilder_SetCapturerCallback(builder, callbacks, NULL);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetCapturerCallback API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_017
 * @tc.desc  : Test OH_AudioStreamBuilder_SetCapturerCallback interface. Returns error code, if the builder type is
 *             AUDIOSTREAM_TYPE_RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_017, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData = AudioCapturerOnReadData;
    result = OH_AudioStreamBuilder_SetCapturerCallback(builder, callbacks, NULL);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetVolumeMode API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetParameter_021
 * @tc.desc  : Test OH_AudioStreamBuilder_SetVolumeMode interface. Returns error code, if the builder type is
 *             AUDIOSTREAM_TYPE_RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetParameter_021, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_VolumeMode mode = AUDIOSTREAM_VOLUMEMODE_APP_INDIVIDUAL;
    result = OH_AudioStreamBuilder_SetVolumeMode(builder, mode);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSampleFormat API via legal state. Test value is AUDIOSTREAM_SAMPLE_U8.
 * @tc.number: OH_AudioStreamBuilder_SetSampleFormat_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSampleFormat interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSampleFormat_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SampleFormat format = AUDIOSTREAM_SAMPLE_U8;
    result = OH_AudioStreamBuilder_SetSampleFormat(builder, format);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSampleFormat API via legal state. Test value is AUDIOSTREAM_SAMPLE_S16LE.
 * @tc.number: OH_AudioStreamBuilder_SetSampleFormat_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSampleFormat interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSampleFormat_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SampleFormat format = AUDIOSTREAM_SAMPLE_S16LE;
    result = OH_AudioStreamBuilder_SetSampleFormat(builder, format);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSampleFormat API via legal state. Test value is AUDIOSTREAM_SAMPLE_S24LE.
 * @tc.number: OH_AudioStreamBuilder_SetSampleFormat_003
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSampleFormat interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSampleFormat_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SampleFormat format = AUDIOSTREAM_SAMPLE_S24LE;
    result = OH_AudioStreamBuilder_SetSampleFormat(builder, format);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSampleFormat API via legal state. Test value is AUDIOSTREAM_SAMPLE_S32LE.
 * @tc.number: OH_AudioStreamBuilder_SetSampleFormat_004
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSampleFormat interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSampleFormat_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SampleFormat format = AUDIOSTREAM_SAMPLE_S32LE;
    result = OH_AudioStreamBuilder_SetSampleFormat(builder, format);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSampleFormat API via legal state. Test value is AUDIOSTREAM_SAMPLE_F32LE.
 * @tc.number: OH_AudioStreamBuilder_SetSampleFormat_005
 * @tc.desc  : Test OH_AudioStreamBuilder_SetSampleFormat interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSampleFormat_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SampleFormat format = AUDIOSTREAM_SAMPLE_F32LE;
    result = OH_AudioStreamBuilder_SetSampleFormat(builder, format);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetLatencyMode API via legal state, AUDIOSTREAM_LATENCY_MODE_NORMAL.
 * @tc.number: OH_AudioStreamBuilder_SetLatencyMode_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetLatencyMode interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetLatencyMode_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    result = OH_AudioStreamBuilder_SetLatencyMode(builder, latencyMode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetCapturerInfo API via illegal state,
 *             sourceType is AUDIOSTREAM_SOURCE_TYPE_INVALID.
 * @tc.number: OH_AudioStreamBuilder_SetCapturerInfo_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetCapturerInfo interface. Returns error code, if source type is
 *             AUDIOSTREAM_SOURCE_TYPE_INVALID.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetCapturerInfo_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_INVALID;
    result = OH_AudioStreamBuilder_SetCapturerInfo(builder, sourceType);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetCapturerInfo API via legal state,
 *             sourceType is AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION.
 * @tc.number: OH_AudioStreamBuilder_SetCapturerInfo_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetCapturerInfo interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetCapturerInfo_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_VOICE_COMMUNICATION;
    result = OH_AudioStreamBuilder_SetCapturerInfo(builder, sourceType);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInfo API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererInfo_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInfo interface. Returns error code, if usage type is
 *             AUDIOSTREAM_USAGE_UNKNOWN.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererInfo_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_UNKNOWN;
    result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInfo API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererInfo_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInfo interface. Returns error code, if content type is
 *             AUDIOSTREAM_CONTENT_TYPE_UNKNOWN.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererInfo_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInfo API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererInfo_003
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInfo interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererInfo_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_MUSIC;
    result = OH_AudioStreamBuilder_SetRendererInfo(builder, usage);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetChannelLayout API via legal state, AUDIOSTREAM_LATENCY_MODE_NORMAL.
 * @tc.number: OH_AudioStreamBuilder_SetChannelLayout_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetChannelLayout interface. Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetChannelLayout_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioChannelLayout channelLayout = CH_LAYOUT_UNKNOWN;
    result = OH_AudioStreamBuilder_SetChannelLayout(builder, channelLayout);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetWriteDataWithMetadataCallback API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetWriteDataWithMetadataCallback_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetWriteDataWithMetadataCallback interface.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetWriteDataWithMetadataCallback_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_WriteDataWithMetadataCallback callback = AudioRendererWriteDataWithMetadataCallback;
    result = OH_AudioStreamBuilder_SetWriteDataWithMetadataCallback(builder, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInterruptMode API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererInterruptMode_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInterruptMode interface with nullptr builder.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetInterruptMode_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = nullptr;

    OH_AudioInterrupt_Mode mode = AUDIOSTREAM_INTERRUPT_MODE_SHARE;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder, mode);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInterruptMode API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererInterruptMode_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInterruptMode interface with share mode.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetInterruptMode_002, TestSize.Level0)
{
    // 1. create builder1 builder2
    OH_AudioStreamBuilder* builder1;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder1, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 2. set builder1 builder2 params
    OH_AudioStreamBuilder_SetSamplingRate(builder1, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder1, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder1, (OH_AudioStream_LatencyMode)g_latencyMode);
    OH_AudioStreamBuilder_SetSampleFormat(builder1, (OH_AudioStream_SampleFormat)g_sampleFormat);
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataInterrupt;
    callbacks.OH_AudioRenderer_OnInterruptEvent = AudioRendererInterruptEvent;
    result = OH_AudioStreamBuilder_SetRendererCallback(builder1, callbacks, nullptr);
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder1, g_frameSize);

    OH_AudioInterrupt_Mode mode = AUDIOSTREAM_INTERRUPT_MODE_SHARE;
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder1, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder1, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 3. create audioRenderer1 audioRenderer2
    OH_AudioRenderer* audioRenderer1;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder1, &audioRenderer1);

    OH_AudioRenderer* audioRenderer2;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder1, &audioRenderer2);

    // 4. start
    result = OH_AudioRenderer_Start(audioRenderer1);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    sleep(1);
    result = OH_AudioRenderer_Start(audioRenderer2);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);


    // 5. stop and release client
    OH_AudioRenderer_Stop(audioRenderer2);
    OH_AudioRenderer_Release(audioRenderer2);

    OH_AudioRenderer_Stop(audioRenderer1);
    OH_AudioRenderer_Release(audioRenderer1);

    result = OH_AudioStreamBuilder_Destroy(builder1);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInterruptMode API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererInterruptMode_003
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInterruptMode interface with independent mode.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetInterruptMode_003, TestSize.Level0)
{
    // 1. create builder
    OH_AudioStreamBuilder* builder1;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder1, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 2. set builder params
    OH_AudioStreamBuilder_SetSamplingRate(builder1, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder1, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder1, (OH_AudioStream_LatencyMode)g_latencyMode);
    OH_AudioStreamBuilder_SetSampleFormat(builder1, (OH_AudioStream_SampleFormat)g_sampleFormat);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataInterrupt;
    callbacks.OH_AudioRenderer_OnInterruptEvent = AudioRendererInterruptEvent;
    result = OH_AudioStreamBuilder_SetRendererCallback(builder1, callbacks, nullptr);
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder1, g_frameSize);

    OH_AudioInterrupt_Mode mode = AUDIOSTREAM_INTERRUPT_MODE_INDEPENDENT;
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder1, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder1, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 3. create audioRenderer1 audioRenderer2
    OH_AudioRenderer* audioRenderer1;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder1, &audioRenderer1);

    OH_AudioRenderer* audioRenderer2;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder1, &audioRenderer2);

    // 4. start
    result = OH_AudioRenderer_Start(audioRenderer1);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    sleep(1);
    result = OH_AudioRenderer_Start(audioRenderer2);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    sleep(2);

    // 5. stop and release client
    OH_AudioRenderer_Stop(audioRenderer2);
    OH_AudioRenderer_Release(audioRenderer2);

    OH_AudioRenderer_Stop(audioRenderer1);
    OH_AudioRenderer_Release(audioRenderer1);

    result = OH_AudioStreamBuilder_Destroy(builder1);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInterruptMode API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererInterruptMode_004
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInterruptMode interface with independent mode and share mode.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererInterruptMode_004, TestSize.Level0)
{
    // 1. create builder1 builder2
    OH_AudioStreamBuilder* builder1;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder1, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStreamBuilder* builder2;
    result = OH_AudioStreamBuilder_Create(&builder2, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 2. set builder1 builder2 params
    OH_AudioStreamBuilder_SetSamplingRate(builder1, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder1, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder1, (OH_AudioStream_LatencyMode)g_latencyMode);
    OH_AudioStreamBuilder_SetSampleFormat(builder1, (OH_AudioStream_SampleFormat)g_sampleFormat);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataInterrupt;
    callbacks.OH_AudioRenderer_OnInterruptEvent = AudioRendererInterruptEvent;
    result = OH_AudioStreamBuilder_SetRendererCallback(builder1, callbacks, nullptr);
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder1, g_frameSize);

    OH_AudioStreamBuilder_SetSamplingRate(builder2, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder2, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder2, (OH_AudioStream_LatencyMode)g_latencyMode);
    OH_AudioStreamBuilder_SetSampleFormat(builder2, (OH_AudioStream_SampleFormat)g_sampleFormat);
    result = OH_AudioStreamBuilder_SetRendererCallback(builder2, callbacks, nullptr);
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder2, g_frameSize);

    OH_AudioInterrupt_Mode mode = AUDIOSTREAM_INTERRUPT_MODE_INDEPENDENT;
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder1, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    mode = AUDIOSTREAM_INTERRUPT_MODE_SHARE;
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder2, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 3. create audioRenderer1 audioRenderer2
    OH_AudioRenderer* audioRenderer1;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder1, &audioRenderer1);

    OH_AudioRenderer* audioRenderer2;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder2, &audioRenderer2);

    // 4. start
    result = OH_AudioRenderer_Start(audioRenderer1);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    sleep(1);
    result = OH_AudioRenderer_Start(audioRenderer2);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    sleep(2);

    // 5. stop and release client
    OH_AudioRenderer_Stop(audioRenderer2);
    OH_AudioRenderer_Release(audioRenderer2);

    OH_AudioRenderer_Stop(audioRenderer1);
    OH_AudioRenderer_Release(audioRenderer1);

    result = OH_AudioStreamBuilder_Destroy(builder2);
    result = OH_AudioStreamBuilder_Destroy(builder1);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInterruptMode API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererInterruptMode_005
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInterruptMode interface with independent mode and share mode.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererInterruptMode_005, TestSize.Level0)
{
    // 1. create builder1
    OH_AudioStreamBuilder* builder1;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder1, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 2. set builder1 params
    OH_AudioStreamBuilder_SetSamplingRate(builder1, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder1, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder1, (OH_AudioStream_LatencyMode)g_latencyMode);
    OH_AudioStreamBuilder_SetSampleFormat(builder1, (OH_AudioStream_SampleFormat)g_sampleFormat);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataInterrupt;
    callbacks.OH_AudioRenderer_OnInterruptEvent = AudioRendererInterruptEvent;
    result = OH_AudioStreamBuilder_SetRendererCallback(builder1, callbacks, nullptr);
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder1, g_frameSize);

    OH_AudioRenderer_Callbacks callbacks2;
    callbacks2.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataInterrupt;
    result = OH_AudioStreamBuilder_SetRendererCallback(builder1, callbacks2, nullptr);

    OH_AudioInterrupt_Mode mode = AUDIOSTREAM_INTERRUPT_MODE_SHARE;
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder1, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    mode = AUDIOSTREAM_INTERRUPT_MODE_INDEPENDENT;
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder1, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 3. create audioRenderer1 audioRenderer2
    OH_AudioRenderer* audioRenderer1;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder1, &audioRenderer1);

    OH_AudioRenderer* audioRenderer2;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder1, &audioRenderer2);

    // 4. start
    result = OH_AudioRenderer_Start(audioRenderer1);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    sleep(1);
    result = OH_AudioRenderer_Start(audioRenderer2);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    sleep(2);

    // 5. stop and release client
    OH_AudioRenderer_Stop(audioRenderer2);
    OH_AudioRenderer_Release(audioRenderer2);

    OH_AudioRenderer_Stop(audioRenderer1);
    OH_AudioRenderer_Release(audioRenderer1);

    result = OH_AudioStreamBuilder_Destroy(builder1);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererInterruptMode API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetInterruptMode_006
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererInterruptMode interface with invalid interrupt mode.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetInterruptMode_006, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = nullptr;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioInterrupt_Mode mode = (OH_AudioInterrupt_Mode)(-2);
    result = OH_AudioStreamBuilder_SetRendererInterruptMode(builder, mode);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererWriteDataCallback_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with VALID result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererWriteDataCallback_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCallbackWithValidData;
    result = OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via legal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererWriteDataCallback_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with INVALID result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererWriteDataCallback_002, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCallbackWithInvalidData;
    result = OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererWriteDataCallback_003
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with VALID result.
 *             Returns error code, if stream type is AUDIOSTREAM_TYPE_CAPTURER
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererWriteDataCallback_003, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCallbackWithValidData;
    result = OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via illegal state.
 * @tc.number: OH_AudioStreamBuilder_SetRendererWriteDataCallback_004
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with INVALID result.
 *             Returns error code, if stream type is AUDIOSTREAM_TYPE_CAPTURER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererWriteDataCallback_004, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCallbackWithInvalidData;
    result = OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_GenerateRenderer API via illegal state.
 * @tc.number: c_001
 * @tc.desc  : Test OH_AudioStreamBuilder_GenerateRenderer interface
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_GenerateRenderer_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer *audioRenderer = nullptr;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_GenerateCapturer API via illegal state.
 * @tc.number: c_001
 * @tc.desc  : Test OH_AudioStreamBuilder_GenerateCapturer interface
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_GenerateCapturer_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer *audioCapturer = nullptr;
    result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetVolumeMode API via illegal state.
 * @tc.number: c_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetVolumeMode interface
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetVolumeMode_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetVolumeMode(builder, static_cast<OH_AudioStream_VolumeMode>(3));
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererPrivacy API.
 * @tc.number: OH_AudioStreamBuilder_SetRendererPrivacy_004
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererPrivacy interface via private privacy.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetRendererPrivacy_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetRendererPrivacy(builder, static_cast<OH_AudioStream_PrivacyType>(3));
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_NE(builder, nullptr);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetPlaybackCaptureMode MODE_DEFAULT state.
 * @tc.number: c_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetPlaybackCaptureMode interface
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetPlaybackCaptureMode_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    uint32_t mode = AUDIOSTREAM_PLAYBACKCAPTURE_MODE_DEFAULT;
    result = OH_AudioStreamBuilder_SetPlaybackCaptureMode(builder, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetPlaybackCaptureMode EXCLUDING_SELF state.
 * @tc.number: c_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetPlaybackCaptureMode interface
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetPlaybackCaptureMode_002, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    uint32_t mode = AUDIOSTREAM_PLAYBACKCAPTURE_MODE_EXCLUDING_SELF;
    result = OH_AudioStreamBuilder_SetPlaybackCaptureMode(builder, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetPlaybackCaptureMode _INVALID_PARAM state.
 * @tc.number: c_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetPlaybackCaptureMode interface
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetPlaybackCaptureMode_003, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    uint32_t mode = -1;
    result = OH_AudioStreamBuilder_SetPlaybackCaptureMode(builder, mode);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetPlaybackCaptureMode SetCapturerInfo state.
 * @tc.number: c_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetPlaybackCaptureMode interface
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetPlaybackCaptureMode_004, TestSize.Level3)
{
    OH_AudioStreamBuilder *builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    uint32_t mode = AUDIOSTREAM_PLAYBACKCAPTURE_MODE_DEFAULT;
    result = OH_AudioStreamBuilder_SetPlaybackCaptureMode(builder, mode);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioStream_SourceType sourceType = AUDIOSTREAM_SOURCE_TYPE_PLAYBACK_CAPTURE;
    result = OH_AudioStreamBuilder_SetCapturerInfo(builder, sourceType);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSamplingRate API with 384000 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_001
 * @tc.desc  : Test SetSamplingRate with 384000 (newly added standard rate) for RENDERER type.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_001, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 384000;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetSamplingRate API with 384000 for capturer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_002
 * @tc.desc  : Test SetSamplingRate with 384000 (newly added standard rate) for CAPTURER type.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_002, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 384000;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate API with 10Hz custom rate for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_003
 * @tc.desc  : Test SetSamplingRate with 48010 (10Hz custom rate) for RENDERER type.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_003, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 48010;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate API with 10Hz custom rate 8010 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_004
 * @tc.desc  : Test SetSamplingRate with 8010 (10Hz custom rate near lower bound) for RENDERER type.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_004, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 8010;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate API with 10Hz custom rate 383990 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_005
 * @tc.desc  : Test SetSamplingRate with 383990 (10Hz custom rate near upper bound) for RENDERER type.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_005, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 383990;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate API with 10Hz custom rate 44100 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_006
 * @tc.desc  : Test SetSamplingRate with 44100 (standard rate) still works after 10Hz changes.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_006, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 44100;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate API with 10Hz custom rate for capturer (should fail).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_007
 * @tc.desc  : Test SetSamplingRate with 48010 for CAPTURER type, should return INVALID_PARAM.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_007, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 48010;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate API with invalid rate not divisible by 10.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_008
 * @tc.desc  : Test SetSamplingRate with 48001 (not divisible by 10) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_008, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 48001;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate API with rate below valid range.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_009
 * @tc.desc  : Test SetSamplingRate with 7990 (below 8000) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_009, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 7990;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate API with rate above valid range.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_010
 * @tc.desc  : Test SetSamplingRate with 384010 (above 384000) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_010, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 384010;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with custom rate then override with standard rate.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_011
 * @tc.desc  : Test that setting a custom rate then a standard rate clears customSampleRate.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_011, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // First set custom rate
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 48010);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // Then override with standard rate
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 48000);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 10Hz rate 8000 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_012
 * @tc.desc  : Test SetSamplingRate with 8000 (lower boundary, standard rate) still works.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_012, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 8000;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 10Hz custom rate 12050 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_013
 * @tc.desc  : Test SetSamplingRate with 12050 (10Hz custom rate) for RENDERER type.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_013, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 12050;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with negative rate for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_014
 * @tc.desc  : Test SetSamplingRate with negative rate for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_014, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = -10;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with zero rate for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_015
 * @tc.desc  : Test SetSamplingRate with 0 for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_015, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 0;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 10Hz custom rate 11030 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_016
 * @tc.desc  : Test SetSamplingRate with 11030 (non-standard 10Hz rate) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_016, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 11030;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 10Hz custom rate 44110 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_017
 * @tc.desc  : Test SetSamplingRate with 44110 (near standard 44100) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_017, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 44110;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 10Hz custom rate 96010 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_018
 * @tc.desc  : Test SetSamplingRate with 96010 (near standard 96000) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_018, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 96010;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 10Hz custom rate 8010 for capturer (should fail).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_019
 * @tc.desc  : Test SetSamplingRate with 8010 for CAPTURER, should return INVALID_PARAM.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_019, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 8010;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 10Hz custom rate 44110 for capturer (should fail).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_020
 * @tc.desc  : Test SetSamplingRate with 44110 for CAPTURER, should return INVALID_PARAM.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_020, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 44110;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 11025 for capturer (standard rate).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_021
 * @tc.desc  : Test SetSamplingRate with 11025 (standard rate) for CAPTURER still works.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_021, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 11025;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 8000 for capturer (standard rate).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_022
 * @tc.desc  : Test SetSamplingRate with 8000 (lower boundary standard rate) for CAPTURER still works.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_022, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 8000;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 11035 for renderer (not divisible by 10).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_023
 * @tc.desc  : Test SetSamplingRate with 11035 (not divisible by 10, not 11025) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_023, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 11035;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 100 for renderer (too small).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_024
 * @tc.desc  : Test SetSamplingRate with 100 (far below 8000) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_024, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 100;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate switch between custom and standard rates repeatedly.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_025
 * @tc.desc  : Test switching between custom 10Hz rates and standard rates multiple times.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_025, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // Custom -> standard -> custom -> standard
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 48010);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 48000);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 44110);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 44100);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 10Hz custom rate 176410 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_026
 * @tc.desc  : Test SetSamplingRate with 176410 (near standard 176400) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_026, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 176410;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with negative rate for capturer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_027
 * @tc.desc  : Test SetSamplingRate with -10 for CAPTURER, should return INVALID_PARAM.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_027, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = -10;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with INT32_MAX for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_028
 * @tc.desc  : Test SetSamplingRate with INT32_MAX (2147483647) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_028, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = INT32_MAX;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 64010 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_029
 * @tc.desc  : Test SetSamplingRate with 64010 (near standard 64000) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_029, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 64010;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 192010 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_030
 * @tc.desc  : Test SetSamplingRate with 192010 (near standard 192000, upper mid range) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_030, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    int32_t samplingRate = 192010;
    result = OH_AudioStreamBuilder_SetSamplingRate(builder, samplingRate);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 16010 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_031
 * @tc.desc  : Test SetSamplingRate with 16010 (near standard 16000) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_031, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 16010);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 22050 for renderer (standard rate).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_032
 * @tc.desc  : Test SetSamplingRate with 22050 (standard rate, odd number) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_032, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 22050);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 22060 for renderer (near 22050, not standard).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_033
 * @tc.desc  : Test SetSamplingRate with 22060 (10Hz rate, near standard 22050) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_033, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 22060);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 32010 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_034
 * @tc.desc  : Test SetSamplingRate with 32010 (near standard 32000) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_034, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 32010);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 88210 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_035
 * @tc.desc  : Test SetSamplingRate with 88210 (near standard 88200) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_035, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 88210);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 16010 for capturer (should fail).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_036
 * @tc.desc  : Test SetSamplingRate with 16010 (10Hz custom) for CAPTURER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_036, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 16010);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 383990 for capturer (should fail).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_037
 * @tc.desc  : Test SetSamplingRate with 383990 (upper boundary 10Hz custom) for CAPTURER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_037, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 383990);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 96010 for capturer (should fail).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_038
 * @tc.desc  : Test SetSamplingRate with 96010 (10Hz custom near 96000) for CAPTURER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_038, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 96010);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 16000 for capturer (standard rate).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_039
 * @tc.desc  : Test SetSamplingRate with 16000 (standard rate) for CAPTURER still works.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_039, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 16000);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 96000 for capturer (standard rate).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_040
 * @tc.desc  : Test SetSamplingRate with 96000 (standard rate) for CAPTURER still works.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_040, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 96000);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 5 for renderer (not divisible by 10, too small).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_041
 * @tc.desc  : Test SetSamplingRate with 5 for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_041, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 5);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 8005 for renderer (not divisible by 10).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_042
 * @tc.desc  : Test SetSamplingRate with 8005 (in range but not divisible by 10) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_042, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 8005);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 11026 for renderer (not 11025, not divisible by 10).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_043
 * @tc.desc  : Test SetSamplingRate with 11026 (near 11025, not divisible by 10) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_043, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 11026);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 500000 for renderer (above range).
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_044
 * @tc.desc  : Test SetSamplingRate with 500000 (divisible by 10 but above 384000) for RENDERER, should fail.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_044, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 500000);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test SetSamplingRate with 24010 for renderer.
 * @tc.number: OH_AudioStreamBuilder_SetSamplingRate_10Hz_045
 * @tc.desc  : Test SetSamplingRate with 24010 (near standard 24000) for RENDERER.
 */
HWTEST(OHAudioStreamBuilderUnitTest, OH_AudioStreamBuilder_SetSamplingRate_10Hz_045, TestSize.Level1)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_SetSamplingRate(builder, 24010);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioStreamBuilder_Destroy(builder);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
