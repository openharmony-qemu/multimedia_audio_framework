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

#include "native_audio_suite_engine.h"
#include "oh_audio_converter_test.h"
#include "OHAudioSuiteFormatConverter.h"
#include "audio_suite_log.h"
#include <fstream>
#include <thread>
#include <atomic>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cstring>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

constexpr uint32_t DEFAULT_BUFFER_SIZE = 1024;
constexpr uint32_t DOUBLE_BUFFER_SIZE = 2048;
constexpr uint32_t LARGE_BUFFER_SIZE = 10240;
constexpr uint32_t EXTRA_LARGE_BUFFER_SIZE = 50000;
constexpr uint32_t SMALL_BUFFER_SIZE = 3;
constexpr uint32_t CUSTOM_BUFFER_SIZE_1022 = 1022;
constexpr uint32_t CUSTOM_BUFFER_SIZE_19200 = 19200;
constexpr uint32_t CUSTOM_BUFFER_SIZE_9600 = 9600;
constexpr uint32_t CUSTOM_BUFFER_SIZE_1600 = 1600;


constexpr int32_t LOOP_COUNT_2 = 2;
constexpr int32_t LOOP_COUNT_6 = 6;
constexpr int32_t LOOP_COUNT_10 = 10;

constexpr int32_t SIGNAL_AMPLITUDE_1000 = 1000;
constexpr int32_t SIGNAL_AMPLITUDE_5000 = 5000;
constexpr int32_t SIGNAL_AMPLITUDE_3000 = 3000;

static int32_t g_requestDataCallCount = 0;

static int32_t RequestDataCallback(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    g_requestDataCallCount++;
    return DEFAULT_BUFFER_SIZE;
}

static int32_t RequestDataCallbackNoData(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    (void)outInputData;
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_NO_AVAILABLE_DATA;
    }
    return 0;
}

static int32_t RequestDataCallbackOutStatusFinish(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    (void)outInputData;
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
    }
    return 0;
}

static int32_t RequestDataCallbackHaveData(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    return DEFAULT_BUFFER_SIZE;
}

static int32_t g_callbackCallCount = 0;
static int32_t g_callbackDataSize = DEFAULT_BUFFER_SIZE;
static int32_t g_totalBytesRead = 0;

static int32_t RequestDataCallbackCustomSize(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DOUBLE_BUFFER_SIZE] = {0};
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
    }
    g_callbackCallCount++;
    return g_callbackDataSize;
}

static int32_t g_callbackCallCountSixteen = 0;

static int32_t RequestDataCallback_016(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[CUSTOM_BUFFER_SIZE_19200] = {0};
    uint32_t moveRight = 8;
    for (int32_t i = 0; i < CUSTOM_BUFFER_SIZE_19200 / LOOP_COUNT_2; i += LOOP_COUNT_2) {
        int16_t sample =
            static_cast<int16_t>(SIGNAL_AMPLITUDE_1000 * sin(2 * M_PI * i / 48000));
        testData[i] = sample & 0xFF;
        testData[i + 1] = (sample >> moveRight) & 0xFF;
    }
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    
    g_callbackCallCountSixteen++;
    return CUSTOM_BUFFER_SIZE_19200;
}

static int32_t g_callbackCallCountSeveneen = 0;

static int32_t RequestDataCallback_017(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[CUSTOM_BUFFER_SIZE_9600] = {0};
    uint32_t moveRight = 8;
    for (int32_t i = 0; i < CUSTOM_BUFFER_SIZE_9600 / LOOP_COUNT_2; i += LOOP_COUNT_2) {
        int16_t sample =
            static_cast<int16_t>(SIGNAL_AMPLITUDE_5000 * sin(2 * M_PI * i / 48000));
        testData[i] = sample & 0xFF;
        testData[i + 1] = (sample >> moveRight) & 0xFF;
    }
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    
    g_callbackCallCountSeveneen++;
    return CUSTOM_BUFFER_SIZE_9600;
}

static int32_t g_callbackCallCountEighteen = 0;

static int32_t RequestDataCallback_018(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[CUSTOM_BUFFER_SIZE_1600] = {0};
    uint32_t moveRight = 8;
    for (int32_t i = 0; i < CUSTOM_BUFFER_SIZE_1600 / LOOP_COUNT_2; i += LOOP_COUNT_2) {
        int16_t sample =
            static_cast<int16_t>(SIGNAL_AMPLITUDE_3000 * sin(2 * M_PI * i / 8000));
        testData[i] = sample & 0xFF;
        testData[i + 1] = (sample >> moveRight) & 0xFF;
    }
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    
    g_callbackCallCountEighteen++;
    return CUSTOM_BUFFER_SIZE_1600;
}

static int32_t g_callCountTwentyFour = 0;

static int32_t RequestDataCallback_024(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        if (g_callCountTwentyFour == 0) {
            *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
        } else {
            *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        }
    }
    
    g_callCountTwentyFour++;
    return DEFAULT_BUFFER_SIZE;
}

static int32_t g_callCountThirty = 0;

static int32_t RequestDataCallback_030(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        if (g_callCountThirty < LOOP_COUNT_2 + 1) {
            *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
        } else {
            *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        }
    }
    
    g_callCountThirty++;
    return DEFAULT_BUFFER_SIZE;
}

void OHAudioConverterTest::SetUpTestCase(void) { }

void OHAudioConverterTest::TearDownTestCase(void) { }

void OHAudioConverterTest::SetUp(void)
{
    g_requestDataCallCount = 0;
    g_callbackCallCount = 0;
    g_callbackDataSize = DEFAULT_BUFFER_SIZE;
    g_totalBytesRead = 0;
}

void OHAudioConverterTest::TearDown(void) { }

/**
 * @tc.name  : Test OH_AudioConverter_Create.
 * @tc.number: OH_AudioConverter_Create_001
 * @tc.desc  : Test nullptr parameters.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Create_001, TestSize.Level0)
{
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    ret = OH_AudioConverter_Create(&inputFormat, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    ret = OH_AudioConverter_Create(nullptr, &outputFormat, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    ret = OH_AudioConverter_Create(&inputFormat, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    ret = OH_AudioConverter_Create(nullptr, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioConverter_Create.
 * @tc.number: OH_AudioConverter_Create_002
 * @tc.desc  : Test success.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Create_002, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_NE(converter, nullptr);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Create.
 * @tc.number: OH_AudioConverter_Create_003
 * @tc.desc  : Test create with different sample rates.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Create_003, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_8000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_U8;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_16000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_NE(converter, nullptr);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Destroy.
 * @tc.number: OH_AudioConverter_Destroy_001
 * @tc.desc  : Test success.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Destroy_001, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(nullptr);

    OH_AudioConverter_Destroy(converter);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_001
 * @tc.desc  : Test nullptr converter.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_001, TestSize.Level0)
{
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_002
 * @tc.desc  : Test nullptr callback.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_002, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, nullptr, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_003
 * @tc.desc  : Test success.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_003, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_004
 * @tc.desc  : Test set callback multiple times.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_004, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_005
 * @tc.desc  : Test Set a callback function after destroying the converter.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_005, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
 
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
 
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(converter);
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_NOT_INITIALIZED);
 
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_001
 * @tc.desc  : Test nullptr converter.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_001, TestSize.Level0)
{
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    OH_AudioConverter_Result ret = OH_AudioConverter_Process(nullptr, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_002
 * @tc.desc  : Test nullptr outputData.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_002, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, nullptr, LARGE_BUFFER_SIZE, &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_003
 * @tc.desc  : Test nullptr outputSize.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_003, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_004
 * @tc.desc  : Test callback not set.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_004, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_005
 * @tc.desc  : Test success with callback returning finished.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_005, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;

    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(g_requestDataCallCount, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_006
 * @tc.desc  : Test callback returning no available data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_006, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackNoData, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(outputSize, 0);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackOutStatusFinish, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_007
 * @tc.desc  : Test callback returning have data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_007, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackHaveData, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_008
 * @tc.desc  : Test process after input finished.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_008, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_009
 * @tc.desc  : Test mono to stereo conversion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_009, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_010
 * @tc.desc  : Test sample rate conversion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_010, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_8000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_011
 * @tc.desc  : Test cache accumulation - data not aligned.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_011, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackCustomSize, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    g_callbackDataSize = CUSTOM_BUFFER_SIZE_1022;
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_012
 * @tc.desc  : Test cache satisfied after accumulation.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_012, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackCustomSize, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    g_callbackDataSize = CUSTOM_BUFFER_SIZE_1022;
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);

    g_callbackDataSize = LOOP_COUNT_2;
    outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_013。
 * @tc.desc  : Test output buffer too small.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_013, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_22050;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[SMALL_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_014
 * @tc.desc  : Test multiple Process calls.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_014, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_015
 * @tc.desc  : Test empty input data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_015, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackNoData, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_016
 * @tc.desc  : Test functional format conversion with real audio data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_016, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    g_callbackCallCountSixteen = 0;
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_016, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[EXTRA_LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);
    EXPECT_GT(g_callbackCallCountSixteen, 0);
    
    bool hasNonZeroData = false;
    for (int32_t i = 0; i < outputSize; i++) {
        if (outputData[i] != 0) {
            hasNonZeroData = true;
            break;
        }
    }
    EXPECT_TRUE(hasNonZeroData);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_017
 * @tc.desc  : Test mono to stereo conversion with real audio data.
 *
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_017, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    int32_t inputBytes = CUSTOM_BUFFER_SIZE_9600;
    g_callbackCallCountSeveneen = 0;
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_017, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[19200] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);
    EXPECT_GT(g_callbackCallCountSeveneen, 0);
    
    EXPECT_EQ(outputSize, inputBytes * LOOP_COUNT_2);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_018
 * @tc.desc  : Test sample rate conversion with real audio data.
 *
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_018, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_8000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    g_callbackCallCountEighteen= 0;
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_018, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[9600] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);
    EXPECT_GT(g_callbackCallCountEighteen, 0);
    
    EXPECT_EQ(outputSize, CUSTOM_BUFFER_SIZE_1600 * LOOP_COUNT_6);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_024
 * @tc.desc  : Test callback returning DATA_FINISHED status.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_024, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    g_callCountTwentyFour = 0;
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_024, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_025
 * @tc.desc  : Test padding logic when input finished.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_025, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
      
    auto callback = [](void* userData, const void** outInputData,
        OH_AudioConverter_InputStatus* outStatus) -> int32_t {
        (void)userData;
        static uint8_t testData[CUSTOM_BUFFER_SIZE_1022] = {0};
        
        if (outInputData != nullptr) {
            *outInputData = testData;
        }
        if (outStatus != nullptr) {
            *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
        }
        return CUSTOM_BUFFER_SIZE_1022;
    };
    
    ret = OH_AudioConverter_SetInputCallback(converter, callback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_026
 * @tc.desc  : Test invalid input format.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_026, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = static_cast<OH_Audio_SampleRate>(0);
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_027
 * @tc.desc  : Test invalid output format.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_027, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = static_cast<OH_Audio_SampleRate>(0);
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_028
 * @tc.desc  : Test 11025Hz special handling.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_028, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_11025;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[SMALL_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_029
 * @tc.desc  : Test output buffer exactly enough.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_029, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[DEFAULT_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_030
 * @tc.desc  : Test multiple Process calls until completion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_030, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    g_callCountThirty = 0;
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_030, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t totalOutputSize = 0;
    int32_t processCount = 0;
    
    while (processCount < LOOP_COUNT_10) {
        int32_t outputSize = 0;
        ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
        EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
        
        totalOutputSize += outputSize;
        processCount++;
        
        if (outputSize == 0) {
            break;
        }
    }
    
    EXPECT_GT(totalOutputSize, 0);
    EXPECT_LE(processCount, LOOP_COUNT_10);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_031
 * @tc.desc  : Test stereo to mono conversion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_031, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_032
 * @tc.desc  : Test 16000Hz sample rate conversion.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_032, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_16000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE_BACK;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    OH_AudioConverter_Destroy(converter);
}

// 400KB = 400 * 1024 = 1048576 bytes
constexpr uint32_t INPUT_DATA_SIZE = 1024 * 400;
constexpr uint32_t INPUT_DATA_SIZE_PLUS = 2 * 400 * 1024;

static int32_t g_callCountThirtyFour = 0;

/**
 * @brief Callback function that returns data larger than 400KB
 */
static int32_t RequestDataCallback_LargerThanOneMB(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static uint8_t testData[INPUT_DATA_SIZE_PLUS] = {0};
    *outInputData = testData;
    *outStatus = OH_AudioConverter_InputStatus::AUDIOCONVERTER_INPUT_HAVE_DATA;
    g_callCountThirtyFour++;
    return INPUT_DATA_SIZE_PLUS;
}

/**
 * @brief Callback function that returns data exactly equal to 400KB
 */
static int32_t RequestDataCallback_EqualsOneMB(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    std::vector<uint8_t>testData(INPUT_DATA_SIZE, 0);
    if (outInputData != nullptr) {
        *outInputData = testData.data();
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_DATA_FINISHED;
    }
    g_callCountThirtyFour++;
    return testData.capacity();
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_034
 * @tc.desc  : Test callback returns data larger than 400KB.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_034, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_LargerThanOneMB, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // Allocate buffer larger than 400KB to hold the output
    std::vector<uint8_t> outputData(INPUT_DATA_SIZE_PLUS, 0);
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData.data(), INPUT_DATA_SIZE_PLUS * 2, &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_CALLBACK_INVALID);
    EXPECT_EQ(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_035
 * @tc.desc  : Test callback returns data exactly equal to 400KB.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_035, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_EqualsOneMB, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // Allocate buffer larger than 400KB to hold the output
    std::vector<uint8_t> outputData(INPUT_DATA_SIZE, 0);
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData.data(), INPUT_DATA_SIZE * 2, &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(outputSize, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_036
 * @tc.desc  : Test invalid input format.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_036, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_22POINT2;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_INVALID_PARAM);
    
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_037
 * @tc.desc  : Test Destroy converter post-processing data.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_037, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;
    
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    
    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    OH_AudioConverter_Destroy(converter);
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_NOT_INITIALIZED);
    
    OH_AudioConverter_Destroy(converter);
}

// Multithreaded testing related
static std::atomic<int32_t> g_mtCallbackCallCount(0);
static std::atomic<int32_t> g_mtSuccessCount(0);
static std::atomic<int32_t> g_mtFailCount(0);

static int32_t RequestDataCallbackMultithread(void* userData, const void** outInputData,
    OH_AudioConverter_InputStatus* outStatus)
{
    (void)userData;
    static thread_local uint8_t testData[DEFAULT_BUFFER_SIZE] = {0};
    if (outInputData != nullptr) {
        *outInputData = testData;
    }
    if (outStatus != nullptr) {
        *outStatus = AUDIOCONVERTER_INPUT_HAVE_DATA;
    }
    g_mtCallbackCallCount++;
    return DEFAULT_BUFFER_SIZE;
}

static void ThreadCreateDestroyTask(int iterations)
{
    for (int j = 0; j < iterations; j++) {
        OH_AudioConverter_Format inputFormat;
        inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
        inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
        inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
        inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

        OH_AudioConverter_Format outputFormat;
        outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
        outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
        outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
        outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;

        OH_AudioConverter* converter = nullptr;
        OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
        if (ret == AUDIOCONVERTER_SUCCESS && converter != nullptr) {
            OH_AudioConverter_Destroy(converter);
            g_mtSuccessCount++;
        } else {
            g_mtFailCount++;
        }
    }
}

// Single-threaded test task
static void ThreadTestTask(int threadId, int iterations)
{
    for (int i = 0; i < iterations; i++) {
        OH_AudioConverter_Format inputFormat;
        inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
        inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
        inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
        inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

        OH_AudioConverter_Format outputFormat;
        outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
        outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
        outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_MONO;
        outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE;

        OH_AudioConverter* converter = nullptr;
        OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
        if (ret != AUDIOCONVERTER_SUCCESS || converter == nullptr) {
            g_mtFailCount++;
            continue;
        }
        ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackMultithread, nullptr);
        if (ret != AUDIOCONVERTER_SUCCESS) {
            g_mtFailCount++;
            OH_AudioConverter_Destroy(converter);
            continue;
        }
        uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
        int32_t outputSize = 0;
        ret = OH_AudioConverter_Process(converter, outputData, LARGE_BUFFER_SIZE, &outputSize);
        if (ret != AUDIOCONVERTER_SUCCESS) {
            g_mtFailCount++;
        } else {
            g_mtSuccessCount++;
        }
        OH_AudioConverter_Destroy(converter);
    }
}

/**
 * @tc.name  : Test OH_AudioConverter multithread independent converters.
 * @tc.number: OH_AudioConverter_Multithread_001
 * @tc.desc  : Test multiple threads with independent converters.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Multithread_001, TestSize.Level0)
{
    g_mtSuccessCount = 0;
    g_mtFailCount = 0;

    const int numThreads = 4;
    const int iterations = 10;

    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(ThreadTestTask, i, iterations);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(g_mtSuccessCount, numThreads * iterations);
    EXPECT_EQ(g_mtFailCount, 0);
}

/**
 * @tc.name  : Test OH_AudioConverter concurrent create/destroy.
 * @tc.number: OH_AudioConverter_Multithread_002
 * @tc.desc  : Test concurrent create and destroy operations.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Multithread_002, TestSize.Level0)
{
    g_mtSuccessCount = 0;
    g_mtFailCount = 0;

    const int numThreads = 8;
    const int iterations = 20;

    std::vector<std::thread> threads;

    for (int i = 0; i < numThreads; i++) {
        threads.emplace_back(ThreadCreateDestroyTask, iterations);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    EXPECT_EQ(g_mtSuccessCount, numThreads * iterations);
    EXPECT_EQ(g_mtFailCount, 0);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process with full cache consumption
 * @tc.number: OH_AudioConverter_Process_Cached_001
 * @tc.desc  : Test Process when all cached data is consumed (branch 5)
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_Cached_001, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackCustomSize, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    g_callbackDataSize = 100;  // Set smaller data size

    uint8_t outputData[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    // Continue calling until there is no more data.
    int32_t totalOutput = outputSize;
    int32_t maxIterations = 10;
    int32_t iterations = 0;
    while (outputSize > 0 && iterations < maxIterations) {
        outputSize = 0;
        ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
        if (ret == AUDIOCONVERTER_SUCCESS) {
            totalOutput += outputSize;
        }
        iterations++;
    }

    EXPECT_GT(totalOutput, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process cache alignment
 * @tc.number: OH_AudioConverter_Process_Cached_002
 * @tc.desc  : Test Process with non-frame-aligned output capacity
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_Cached_002, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallbackHaveData, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[CUSTOM_BUFFER_SIZE_1022] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    EXPECT_EQ(outputSize % 4, 0);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioConverter_Process with partial cache consumption
 * @tc.number: OH_AudioConverter_Process_Cached_003
 * @tc.desc  : Test Process when only part of cached data is consumed (branch 4)
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_Cached_003, TestSize.Level0)
{
    OH_AudioConverter_Format inputFormat;
    inputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    inputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_48000;
    inputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    inputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter_Format outputFormat;
    outputFormat.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    outputFormat.samplingRate = OH_Audio_SampleRate::SAMPLE_RATE_44100;
    outputFormat.channelLayout = OH_AudioChannelLayout::CH_LAYOUT_STEREO;
    outputFormat.sampleFormat = OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE;

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback_016, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    uint8_t outputData[DEFAULT_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    outputSize = 0;
    ret = OH_AudioConverter_Process(converter, outputData, sizeof(outputData), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

}
}
