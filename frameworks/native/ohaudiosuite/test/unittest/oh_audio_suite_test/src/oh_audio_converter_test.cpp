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
#include "native_audio_converter.h"
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

// Ex-mode test constants (defined at file scope for visibility across all test cases)
constexpr int32_t EX_TEST_IN_RATE = 44100;
constexpr int32_t EX_TEST_OUT_RATE = 48000;
constexpr uint32_t EX_TEST_FRAME_BYTES = 4; // S16LE * 2 channels
// gcd(44100, 48000) = 300 → minInFrames=147, minOutFrames=160
constexpr uint32_t EX_TEST_MIN_IN_BYTES = 147u * EX_TEST_FRAME_BYTES; // 588
constexpr uint32_t EX_TEST_MIN_OUT_BYTES = 160u * EX_TEST_FRAME_BYTES; // 640
constexpr uint32_t EX_TEST_OUTPUT_CAP = 50000;
constexpr int32_t EX_RATE_OUT_OF_RANGE_LOW = 7990;
constexpr int32_t EX_RATE_OUT_OF_RANGE_HIGH = 5644810;
constexpr int32_t EX_RATE_NOT_MULTIPLE_OF_TEN = 48001;
constexpr int32_t EX_RATE_EXEMPTED_11025 = 11025;

static void SetupExConverterFormat(OH_AudioSuite_ConverterFormatEx& format, int32_t rate,
    OH_AudioChannelLayout layout, OH_Audio_SampleFormat sampleFormat)
{
    format.encodingType = OH_Audio_EncodingType::AUDIO_ENCODING_TYPE_RAW;
    format.samplingRate = rate;
    format.channelLayout = layout;
    format.sampleFormat = sampleFormat;
}

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
 * @tc.name  : Test OH_AudioConverter_SetInputCallback.
 * @tc.number: OH_AudioConverter_SetInputCallback_006
 * @tc.desc  : Test SetInputCallback after DirectProcess (runtime mutex) returns UNSUPPORTED_OPERATION.
 *             Covers impl directProcessMode_ check (direction B: DirectProcess first, then SetInputCallback).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_SetInputCallback_006, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    // Lock direct mode first via a successful DirectProcess
    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    ASSERT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc),
        AUDIOCONVERTER_SUCCESS);

    // DirectProcess already locked → SetInputCallback → UNSUPPORTED_OPERATION
    OH_AudioConverter_Result ret = OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_UNSUPPORTED_OPERATION);

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

/**
 * @tc.name  : Test OH_AudioConverter_Process.
 * @tc.number: OH_AudioConverter_Process_038
 * @tc.desc  : Test Process after DirectProcess (runtime mutex) returns CALLBACK_NOT_SET.
 *             Process has no explicit mutex guard; with no callback set it returns CALLBACK_NOT_SET
 *             (impl ReadAndCacheInputData dataCallback_==nullptr implicit detection).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Process_038, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    // Lock direct mode first via a successful DirectProcess
    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    ASSERT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc),
        AUDIOCONVERTER_SUCCESS);

    // DirectProcess already locked, no callback set → Process → CALLBACK_NOT_SET
    uint8_t processOutput[LARGE_BUFFER_SIZE] = {0};
    int32_t outputSize = 0;
    OH_AudioConverter_Result ret = OH_AudioConverter_Process(
        converter, processOutput, sizeof(processOutput), &outputSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET);

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

/**
 * @tc.name  : Test OH_AudioConverter_Destroy RemoveConverter if branch.
 * @tc.number: OH_AudioConverter_Destroy_002
 * @tc.desc  : Test RemoveConverter if covers 2 branches after refactor:
 *             (1) erase>0 → true → execute delete (normal destroy)
 *             (2) erase==0 → false → skip delete (double destroy)
 *             Note: nullptr is guarded by CHECK_AND_RETURN_LOG before the if,
 *             so it returns early and never reaches the erase/delete path.
 *             Destroy_001 already covers both branches; this test documents
 *             the branch analysis explicitly.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioConverter_Destroy_002, TestSize.Level0)
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
 
    // Branch 1: erase > 0 → true → delete executed (normal destroy)
    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter);
    ASSERT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    ASSERT_TRUE(converter != nullptr);
    OH_AudioConverter_Destroy(converter);
 
    // Branch 2: erase == 0 → false → no delete (already removed, no double-delete)
    // Note: converter is a dangling pointer after the first Destroy (delete'd internally).
    // This is technically UB-border but safe in practice: GetAudioSuiteConverter and
    // unordered_set::find/erase only compare pointer values, never dereference it.
    // The purpose is to verify the erase==0 branch does not double-delete.
    OH_AudioConverter_Destroy(converter);
 
    // Verify a new converter can still be created and destroyed normally
    OH_AudioConverter* converter2 = nullptr;
    ret = OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter2);
    ASSERT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    ASSERT_TRUE(converter2 != nullptr);
    OH_AudioConverter_Destroy(converter2);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_001
 * @tc.desc  : Test success path.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_001, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_NE(converter, nullptr);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_002
 * @tc.desc  : Test nullptr converter output pointer.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_002, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, nullptr),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_003
 * @tc.desc  : Test nullptr inputFormat / outputFormat.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_003, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(nullptr, &outputFormat, &converter),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, nullptr, &converter),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_004
 * @tc.desc  : Test invalid encoding type (non-PCM).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_004, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    inputFormat.encodingType = static_cast<OH_Audio_EncodingType>(999); // invalid
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_005
 * @tc.desc  : Test sample rate out of range (low and high).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_005, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_RATE_OUT_OF_RANGE_LOW,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);

    SetupExConverterFormat(inputFormat, EX_RATE_OUT_OF_RANGE_HIGH,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_006
 * @tc.desc  : Test sample rate not multiple of 10 (and not 11025).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_006, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_RATE_NOT_MULTIPLE_OF_TEN,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_007
 * @tc.desc  : Test invalid sample format.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_007, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, static_cast<OH_Audio_SampleFormat>(999));
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_008
 * @tc.desc  : Test invalid channel layout.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_008, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        static_cast<OH_AudioChannelLayout>(999), AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    EXPECT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterCreateEx.
 * @tc.number: OH_AudioSuite_ConverterCreateEx_009
 * @tc.desc  : Test 11025 exempted rate succeeds.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterCreateEx_009, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_RATE_EXEMPTED_11025,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    OH_AudioConverter_Result ret = OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    if (converter != nullptr) {
        OH_AudioConverter_Destroy(converter);
    }
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetMinInOutSizes.
 * @tc.number: OH_AudioSuite_ConverterGetMinInOutSizes_001
 * @tc.desc  : Test success path.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetMinInOutSizes_001, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    uint32_t inputMinSize = 0;
    uint32_t outputMinSize = 0;
    OH_AudioConverter_Result ret = OH_AudioSuite_ConverterGetMinInOutSizes(converter, &inputMinSize, &outputMinSize);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(inputMinSize, 0u);
    EXPECT_GT(outputMinSize, 0u);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetMinInOutSizes.
 * @tc.number: OH_AudioSuite_ConverterGetMinInOutSizes_002
 * @tc.desc  : Test nullptr converter / output pointers.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetMinInOutSizes_002, TestSize.Level0)
{
    EXPECT_EQ(OH_AudioSuite_ConverterGetMinInOutSizes(nullptr, nullptr, nullptr),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);

    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(OH_AudioSuite_ConverterGetMinInOutSizes(converter, nullptr, nullptr),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetMinInOutSizes.
 * @tc.number: OH_AudioSuite_ConverterGetMinInOutSizes_003
 * @tc.desc  : Test converter not active (destroyed) returns INVALID_PARAM.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetMinInOutSizes_003, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(converter);
    uint32_t inputMinSize = 0;
    uint32_t outputMinSize = 0;
    EXPECT_EQ(OH_AudioSuite_ConverterGetMinInOutSizes(converter, &inputMinSize, &outputMinSize),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetMinInOutSizes.
 * @tc.number: OH_AudioSuite_ConverterGetMinInOutSizes_004
 * @tc.desc  : Test min size calculation for high sample rate combinations.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetMinInOutSizes_004, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, 3528000,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, 48000,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    uint32_t inputMinSize = 0;
    uint32_t outputMinSize = 0;
    EXPECT_EQ(OH_AudioSuite_ConverterGetMinInOutSizes(converter, &inputMinSize, &outputMinSize),
        AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(inputMinSize, 588u);
    EXPECT_EQ(outputMinSize, 8u);
    OH_AudioConverter_Destroy(converter);

    SetupExConverterFormat(inputFormat, 5644800,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    SetupExConverterFormat(outputFormat, 48000,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    inputMinSize = 0;
    outputMinSize = 0;
    EXPECT_EQ(OH_AudioSuite_ConverterGetMinInOutSizes(converter, &inputMinSize, &outputMinSize),
        AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(inputMinSize, 2352u);
    EXPECT_EQ(outputMinSize, 20u);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetDelaySize.
 * @tc.number: OH_AudioSuite_ConverterGetDelaySize_001
 * @tc.desc  : Test nullptr converter.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetDelaySize_001, TestSize.Level0)
{
    uint32_t delaySize = 0;
    EXPECT_EQ(OH_AudioSuite_ConverterGetDelaySize(nullptr, &delaySize),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetDelaySize.
 * @tc.number: OH_AudioSuite_ConverterGetDelaySize_002
 * @tc.desc  : Test nullptr delaySize output.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetDelaySize_002, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(OH_AudioSuite_ConverterGetDelaySize(converter, nullptr),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetDelaySize.
 * @tc.number: OH_AudioSuite_ConverterGetDelaySize_003
 * @tc.desc  : Test converter not active (destroyed) returns INVALID_PARAM.
 *             Covers GetAudioSuiteConverter if-false branch (converter not found).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetDelaySize_003, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(converter);
    // After destroy: converter not in registry → GetAudioSuiteConverter returns nullptr
    uint32_t delaySize = 0;
    EXPECT_EQ(OH_AudioSuite_ConverterGetDelaySize(converter, &delaySize), AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetDelaySize.
 * @tc.number: OH_AudioSuite_ConverterGetDelaySize_004
 * @tc.desc  : Test before first process returns SUCCESS with delaySize 0.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetDelaySize_004, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE, OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE, OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter), AUDIOCONVERTER_SUCCESS);
    uint32_t delaySize = 1;
    EXPECT_EQ(OH_AudioSuite_ConverterGetDelaySize(converter, &delaySize), AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(delaySize, 0u);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterGetDelaySize.
 * @tc.number: OH_AudioSuite_ConverterGetDelaySize_005
 * @tc.desc  : Test after first DirectProcess returns SUCCESS with delay > 0.
 *             Covers ConvertConverterError(SUCCESS) branch.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterGetDelaySize_005, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    ASSERT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc),
        AUDIOCONVERTER_SUCCESS);

    uint32_t delaySize = 0;
    EXPECT_EQ(OH_AudioSuite_ConverterGetDelaySize(converter, &delaySize),
        AUDIOCONVERTER_SUCCESS);
    EXPECT_GT(delaySize, 0);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterDirectProcess.
 * @tc.number: OH_AudioSuite_ConverterDirectProcess_001
 * @tc.desc  : Test nullptr converter.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterDirectProcess_001, TestSize.Level0)
{
    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    EXPECT_EQ(OH_AudioSuite_ConverterDirectProcess(nullptr, &inDesc, &outDesc),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterDirectProcess.
 * @tc.number: OH_AudioSuite_ConverterDirectProcess_004
 * @tc.desc  : Test converter not active (destroyed) returns INVALID_PARAM.
 *             Covers GetAudioSuiteConverter if-false branch.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterDirectProcess_002, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(converter);

    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    EXPECT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc), AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterDirectProcess.
 * @tc.number: OH_AudioSuite_ConverterDirectProcess_005
 * @tc.desc  : Test input not multiple of minInBytes returns INVALID_PARAM.
 *             Covers ConvertConverterError(ERR_INVALID_PARAM) branch.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterDirectProcess_003, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    // EX_TEST_MIN_IN_BYTES + 1 is not a multiple of minInBytes
    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES + 1u, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES + 1u};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0u};
    EXPECT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc),
        AUDIOCONVERTER_ERROR_INVALID_PARAM);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterDirectProcess.
 * @tc.number: OH_AudioSuite_ConverterDirectProcess_006
 * @tc.desc  : Test output buffer too small returns BUFFER_TOO_SMALL.
 *             Covers ConvertConverterError(ERR_BUFFER_TOO_SMALL) branch.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterDirectProcess_004, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    uint32_t inputMultiple = EX_TEST_MIN_IN_BYTES * 10u;
    std::vector<uint8_t> inputData(inputMultiple, 1);
    uint32_t smallCap = EX_TEST_MIN_OUT_BYTES; // too small for 10× input
    std::vector<uint8_t> outputData(smallCap, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), inputMultiple};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), smallCap, 0};
    EXPECT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc),
        AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterDirectProcess.
 * @tc.number: OH_AudioSuite_ConverterDirectProcess_007
 * @tc.desc  : Test success path covers ConvertConverterError(SUCCESS).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterDirectProcess_005, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    EXPECT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc),
        AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterDirectProcess.
 * @tc.number: OH_AudioSuite_ConverterDirectProcess_006
 * @tc.desc  : Test DirectProcess after SetInputCallback (runtime mutex) returns UNSUPPORTED_OPERATION.
 *             Covers impl dataCallback_ check (direction SetInputCallback first, then DirectProcess).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterDirectProcess_006, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    // Lock callback mode first via a successful SetInputCallback
    ASSERT_EQ(OH_AudioConverter_SetInputCallback(converter, RequestDataCallback, nullptr),
        AUDIOCONVERTER_SUCCESS);

    // Callback already set → DirectProcess → UNSUPPORTED_OPERATION
    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    EXPECT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_OPERATION);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterDirectProcess.
 * @tc.number: OH_AudioSuite_ConverterDirectProcess_007
 * @tc.desc  : Test DirectProcess on public-API-created (non-Ex) converter succeeds.
 *             Covers impl runtime-mutex relaxation: OH_AudioConverter_Create instances can use
 *             the direct stream (no static isExMode_ gate anymore).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterDirectProcess_007, TestSize.Level0)
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
    ASSERT_EQ(OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    // Public API created (non-Ex) + DirectProcess → SUCCESS
    // Input is 48000/2ch/S16LE: gcd(48000,44100)=300, minInFrames=48000/300=160,
    // minInBytes = 160 * 4B = 640 (not EX_TEST_MIN_IN_BYTES which is for a 44100 input).
    constexpr uint32_t minInBytes48000To44100 = 640;
    std::vector<uint8_t> inputData(minInBytes48000To44100, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), minInBytes48000To44100};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    OH_AudioConverter_Result ret = OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterFlush.
 * @tc.number: OH_AudioSuite_ConverterFlush_001
 * @tc.desc  : Test nullptr converter returns INVALID_PARAM.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterFlush_001, TestSize.Level0)
{
    EXPECT_EQ(OH_AudioSuite_ConverterFlush(nullptr), AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterFlush.
 * @tc.number: OH_AudioSuite_ConverterFlush_002
 * @tc.desc  : Test converter not active (destroyed) returns INVALID_PARAM.
 *             Covers GetAudioSuiteConverter if-false branch.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterFlush_002, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(converter);
    EXPECT_EQ(OH_AudioSuite_ConverterFlush(converter), AUDIOCONVERTER_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterFlush.
 * @tc.number: OH_AudioSuite_ConverterFlush_003
 * @tc.desc  : Test Flush after process succeeds.
 *             Covers ConvertConverterError(SUCCESS) branch via Flush path.
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterFlush_003, TestSize.Level0)
{
    OH_AudioSuite_ConverterFormatEx inputFormat;
    SetupExConverterFormat(inputFormat, EX_TEST_IN_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);
    OH_AudioSuite_ConverterFormatEx outputFormat;
    SetupExConverterFormat(outputFormat, EX_TEST_OUT_RATE,
        OH_AudioChannelLayout::CH_LAYOUT_STEREO, AUDIO_SAMPLE_S16LE);

    OH_AudioConverter* converter = nullptr;
    ASSERT_EQ(OH_AudioSuite_ConverterCreateEx(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    // Process first to initialize internal state
    std::vector<uint8_t> inputData(EX_TEST_MIN_IN_BYTES, 1);
    std::vector<uint8_t> outputData(EX_TEST_OUTPUT_CAP, 0);
    OH_AudioSuite_ConverterInputInfo inDesc = {inputData.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    ASSERT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc, &outDesc), AUDIOCONVERTER_SUCCESS);

    // Flush should succeed
    EXPECT_EQ(OH_AudioSuite_ConverterFlush(converter), AUDIOCONVERTER_SUCCESS);

    uint32_t delaySize = 1;
    EXPECT_EQ(OH_AudioSuite_ConverterGetDelaySize(converter, &delaySize), AUDIOCONVERTER_SUCCESS);
    EXPECT_EQ(delaySize, 0u);

    // Process again after flush should succeed
    std::vector<uint8_t> inputData2(EX_TEST_MIN_IN_BYTES, 2);
    OH_AudioSuite_ConverterInputInfo inDesc2 = {inputData2.data(), EX_TEST_MIN_IN_BYTES};
    OH_AudioSuite_ConverterOutputInfo outDesc2 = {outputData.data(), EX_TEST_OUTPUT_CAP, 0};
    EXPECT_EQ(OH_AudioSuite_ConverterDirectProcess(converter, &inDesc2, &outDesc2), AUDIOCONVERTER_SUCCESS);
    OH_AudioConverter_Destroy(converter);
}

/**
 * @tc.name  : Test OH_AudioSuite_ConverterFlush.
 * @tc.number: OH_AudioSuite_ConverterFlush_004
 * @tc.desc  : Test Flush on public-API-created (non-Ex) converter succeeds.
 *             Covers impl runtime-mutex relaxation: OH_AudioConverter_Create instances can Flush
 *             (no static isExMode_ gate anymore).
 */
HWTEST_F(OHAudioConverterTest, OH_AudioSuite_ConverterFlush_004, TestSize.Level0)
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
    ASSERT_EQ(OH_AudioConverter_Create(&inputFormat, &outputFormat, &converter),
        AUDIOCONVERTER_SUCCESS);

    // Public API created (non-Ex) + Flush → SUCCESS
    OH_AudioConverter_Result ret = OH_AudioSuite_ConverterFlush(converter);
    EXPECT_EQ(ret, AUDIOCONVERTER_SUCCESS);

    OH_AudioConverter_Destroy(converter);
}

}
}
