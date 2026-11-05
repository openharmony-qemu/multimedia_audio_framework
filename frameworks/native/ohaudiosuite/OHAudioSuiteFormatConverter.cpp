/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef LOG_TAG
#define LOG_TAG "OHAudioSuiteFormatConverter"
#endif

#include "OHAudioSuiteFormatConverter.h"
#include <algorithm>
#include <memory>
#include <mutex>
#include <unordered_set>
#include <vector>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_format_converter.h"
#include "audio_utils.h"

using namespace OHOS::AudioStandard;
using namespace OHOS::AudioStandard::AudioSuite;

static inline const std::vector<OH_Audio_SampleRate> AUDIO_SUITE_SUPPORTED_SAMPLING_RATES {
    OH_Audio_SampleRate::SAMPLE_RATE_8000,
    OH_Audio_SampleRate::SAMPLE_RATE_11025,
    OH_Audio_SampleRate::SAMPLE_RATE_12000,
    OH_Audio_SampleRate::SAMPLE_RATE_16000,
    OH_Audio_SampleRate::SAMPLE_RATE_22050,
    OH_Audio_SampleRate::SAMPLE_RATE_24000,
    OH_Audio_SampleRate::SAMPLE_RATE_32000,
    OH_Audio_SampleRate::SAMPLE_RATE_44100,
    OH_Audio_SampleRate::SAMPLE_RATE_48000,
    OH_Audio_SampleRate::SAMPLE_RATE_64000,
    OH_Audio_SampleRate::SAMPLE_RATE_88200,
    OH_Audio_SampleRate::SAMPLE_RATE_96000,
    OH_Audio_SampleRate::SAMPLE_RATE_176400,
    OH_Audio_SampleRate::SAMPLE_RATE_192000,
};
 
static inline const std::vector<OH_Audio_SampleFormat> AUDIO_SUITE_SUPPORTED_SAMPLE_FORMAT {
    AUDIO_SAMPLE_U8,
    AUDIO_SAMPLE_S16LE,
    AUDIO_SAMPLE_S24LE,
    AUDIO_SAMPLE_S32LE,
    AUDIO_SAMPLE_F32LE,
};
 
static inline const std::vector<OH_AudioChannelLayout> AUDIO_SUITE_SUPPORTED_CHANNEL_LAYOUTS {
    OH_AudioChannelLayout::CH_LAYOUT_MONO,
    OH_AudioChannelLayout::CH_LAYOUT_STEREO,
    OH_AudioChannelLayout::CH_LAYOUT_STEREO_DOWNMIX,
    OH_AudioChannelLayout::CH_LAYOUT_2POINT1,
    OH_AudioChannelLayout::CH_LAYOUT_3POINT0,
    OH_AudioChannelLayout::CH_LAYOUT_SURROUND,
    OH_AudioChannelLayout::CH_LAYOUT_3POINT1,
    OH_AudioChannelLayout::CH_LAYOUT_4POINT0,
    OH_AudioChannelLayout::CH_LAYOUT_QUAD_SIDE,
    OH_AudioChannelLayout::CH_LAYOUT_QUAD,
    OH_AudioChannelLayout::CH_LAYOUT_2POINT0POINT2,
    OH_AudioChannelLayout::CH_LAYOUT_4POINT1,
    OH_AudioChannelLayout::CH_LAYOUT_5POINT0,
    OH_AudioChannelLayout::CH_LAYOUT_5POINT0_BACK,
    OH_AudioChannelLayout::CH_LAYOUT_2POINT1POINT2,
    OH_AudioChannelLayout::CH_LAYOUT_3POINT0POINT2,
    OH_AudioChannelLayout::CH_LAYOUT_5POINT1,
    OH_AudioChannelLayout::CH_LAYOUT_5POINT1_BACK,
    OH_AudioChannelLayout::CH_LAYOUT_6POINT0,
    OH_AudioChannelLayout::CH_LAYOUT_3POINT1POINT2,
    OH_AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT,
    OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL,
    OH_AudioChannelLayout::CH_LAYOUT_6POINT1,
    OH_AudioChannelLayout::CH_LAYOUT_6POINT1_BACK,
    OH_AudioChannelLayout::CH_LAYOUT_6POINT1_FRONT,
    OH_AudioChannelLayout::CH_LAYOUT_7POINT0,
    OH_AudioChannelLayout::CH_LAYOUT_7POINT0_FRONT,
    OH_AudioChannelLayout::CH_LAYOUT_7POINT1,
    OH_AudioChannelLayout::CH_LAYOUT_OCTAGONAL,
    OH_AudioChannelLayout::CH_LAYOUT_5POINT1POINT2,
    OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE,
    OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE_BACK,
};

namespace {
constexpr int32_t EX_MIN_SAMPLE_RATE = 8000;
constexpr int32_t EX_MAX_SAMPLE_RATE = 5644800;
constexpr int32_t EX_SAMPLE_RATE_RESOLUTION = 10;
constexpr int32_t EX_EXEMPTED_RATE_11025 = 11025;

bool CheckSystemPermission()
{
#ifndef AUDIOSUITE_CONVERTER_UT
    static bool isSystemApp = PermissionUtil::VerifySelfPermission();
    return isSystemApp;
#else
    return true;
#endif
}
}

static bool ValidateConverterFormatEx(const OH_AudioSuite_ConverterFormatEx* format)
{
    CHECK_AND_RETURN_RET_LOG(format->encodingType == AUDIO_ENCODING_TYPE_RAW,
        false, "ValidateEx: unsupported encoding type, encodingType=%{public}d", format->encodingType);
    CHECK_AND_RETURN_RET_LOG(format->samplingRate >= EX_MIN_SAMPLE_RATE &&
        format->samplingRate <= EX_MAX_SAMPLE_RATE,
        false, "ValidateEx: sample rate out of range [8000, 5644800], rate=%{public}d", format->samplingRate);
    CHECK_AND_RETURN_RET_LOG(
        format->samplingRate % EX_SAMPLE_RATE_RESOLUTION == 0
        || format->samplingRate == EX_EXEMPTED_RATE_11025,
        false, "ValidateEx: sample rate must be multiple of 10 (11025 exempted), rate=%{public}d",
        format->samplingRate);
    CHECK_AND_RETURN_RET_LOG(
        std::find(AUDIO_SUITE_SUPPORTED_SAMPLE_FORMAT.begin(), AUDIO_SUITE_SUPPORTED_SAMPLE_FORMAT.end(),
            format->sampleFormat) != AUDIO_SUITE_SUPPORTED_SAMPLE_FORMAT.end(),
        false, "ValidateEx: unsupported sample format, sampleFormat=%{public}d", format->sampleFormat);
    CHECK_AND_RETURN_RET_LOG(
        std::find(AUDIO_SUITE_SUPPORTED_CHANNEL_LAYOUTS.begin(), AUDIO_SUITE_SUPPORTED_CHANNEL_LAYOUTS.end(),
            format->channelLayout) != AUDIO_SUITE_SUPPORTED_CHANNEL_LAYOUTS.end(),
        false, "ValidateEx: unsupported channel layout, channelLayout=%{public}llu",
        static_cast<unsigned long long>(format->channelLayout));
    return true;
}

static bool ValidateConverterFormat(const OH_AudioConverter_Format* format)
{
    CHECK_AND_RETURN_RET_LOG(format->encodingType == AUDIO_ENCODING_TYPE_RAW,
        false,
        "Validate: unsupported encoding type, encodingType=%{public}d", format->encodingType);
 
    CHECK_AND_RETURN_RET_LOG(
        std::find(AUDIO_SUITE_SUPPORTED_SAMPLING_RATES.begin(), AUDIO_SUITE_SUPPORTED_SAMPLING_RATES.end(),
            format->samplingRate) != AUDIO_SUITE_SUPPORTED_SAMPLING_RATES.end(),
        false,
        "Validate: unsupported sample rate, samplingRate=%{public}d", format->samplingRate);
 
    CHECK_AND_RETURN_RET_LOG(
        std::find(AUDIO_SUITE_SUPPORTED_SAMPLE_FORMAT.begin(), AUDIO_SUITE_SUPPORTED_SAMPLE_FORMAT.end(),
            format->sampleFormat) != AUDIO_SUITE_SUPPORTED_SAMPLE_FORMAT.end(),
        false,
        "Validate: unsupported sample format, sampleFormat=%{public}d", format->sampleFormat);
 
    CHECK_AND_RETURN_RET_LOG(
        std::find(AUDIO_SUITE_SUPPORTED_CHANNEL_LAYOUTS.begin(), AUDIO_SUITE_SUPPORTED_CHANNEL_LAYOUTS.end(),
            format->channelLayout) != AUDIO_SUITE_SUPPORTED_CHANNEL_LAYOUTS.end(),
        false,
        "Validate: unsupported channel layout, channelLayout=%{public}llu",
        static_cast<unsigned long long>(format->channelLayout));
 
    return true;
}
 
static std::unordered_set<OHAudioConverter*> g_activeConverters;
static std::mutex g_activeConvertersMutex;


static std::shared_ptr<AudioFormatConverter> GetAudioSuiteConverter(OHAudioConverter* converter)
{
    std::lock_guard<std::mutex> lock(g_activeConvertersMutex);
    if (g_activeConverters.find(converter) == g_activeConverters.end()) {
        return nullptr;
    }
    return converter->GetAudioFormatConverter();
}
 
static void AddConverter(OHAudioConverter* converter)
{
    std::lock_guard<std::mutex> lock(g_activeConvertersMutex);
    g_activeConverters.insert(converter);
}
 
static void RemoveConverter(OHAudioConverter* converter)
{
    CHECK_AND_RETURN_LOG(converter != nullptr, "RemoveConverter failed: converter is nullptr");
    std::lock_guard<std::mutex> lock(g_activeConvertersMutex);
    if (g_activeConverters.erase(converter) > 0) {
        delete converter;
    }
}

// CAPI callback implementation class (adapter pattern)
class ConverterDataCallbackImpl : public AudioSuite::FormatConverterDataCallback {
public:
    ConverterDataCallbackImpl(OH_AudioConverter_RequestDataCallback callback, void *userData)
        : callback_(callback), userData_(userData)
    {}

    int32_t OnRequestData(const void** data, AudioSuite::InputDataStatus* status) override
    {
        CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, -1, "The callback function pointer is nullptr.");

        OH_AudioConverter_InputStatus cStatus = AUDIOCONVERTER_INPUT_NO_AVAILABLE_DATA;
        int32_t size = callback_(userData_, data, &cStatus);

        // Enumeration type conversion: OH_AudioConverter_InputStatus -> InputDataStatus
        *status = static_cast<AudioSuite::InputDataStatus>(cStatus);

        return size;
    }

private:
    OH_AudioConverter_RequestDataCallback callback_;
    void *userData_;
};

// Unified InnerAPI error code to CAPI error code mapping function
static OH_AudioConverter_Result ConvertConverterError(int32_t err)
{
    switch (err) {
        case SUCCESS:
            return AUDIOCONVERTER_SUCCESS;
        case ERR_INVALID_PARAM:
            return AUDIOCONVERTER_ERROR_INVALID_PARAM;
        case ERR_AUDIO_SUITE_UNSUPPORTED_FORMAT:
            return AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT;
        case ERR_OPERATION_FAILED:
            return AUDIOCONVERTER_ERROR_SYSTEM;
        case ERR_MEMORY_ALLOC_FAILED:
            return AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED;
        case ERR_BUFFER_TOO_SMALL:
            return AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL;
        case ERR_ILLEGAL_STATE:
            return AUDIOCONVERTER_ERROR_NOT_INITIALIZED;
        case ERR_NOT_SUPPORTED:
            return AUDIOCONVERTER_ERROR_UNSUPPORTED_OPERATION;
        case ERR_INVALID_WRITE:
            return AUDIOCONVERTER_ERROR_CALLBACK_INVALID;
        case ERR_CALLBACK_NOT_FOUND:
            return AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET;
        default:
            return AUDIOCONVERTER_ERROR_SYSTEM;
    }
}

OH_AudioConverter_Result OH_AudioConverter_Create(
    const OH_AudioConverter_Format* inputFormat, const OH_AudioConverter_Format* outputFormat,
    OH_AudioConverter** converter)
{
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Converter creation failed: The passed address is nullptr.");

    CHECK_AND_RETURN_RET_LOG(inputFormat != nullptr && outputFormat != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Create failed: format is nullptr");

    CHECK_AND_RETURN_RET_LOG(ValidateConverterFormat(inputFormat),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT, "Create failed: input format validation failed");

    CHECK_AND_RETURN_RET_LOG(ValidateConverterFormat(outputFormat),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT, "Create failed: output format validation failed");

    AudioConverterFormat inputFmt;
    inputFmt.channelLayout = static_cast<AudioChannelLayout>(inputFormat->channelLayout);
    inputFmt.channelCount = 0;
    inputFmt.sampleFormat = static_cast<AudioSampleFormat>(inputFormat->sampleFormat);
    inputFmt.sampleRate = static_cast<int32_t>(inputFormat->samplingRate);

    AudioConverterFormat outputFmt;
    outputFmt.channelLayout = static_cast<AudioChannelLayout>(outputFormat->channelLayout);
    outputFmt.channelCount = 0;
    outputFmt.sampleFormat = static_cast<AudioSampleFormat>(outputFormat->sampleFormat);
    outputFmt.sampleRate = static_cast<int32_t>(outputFormat->samplingRate);

    AUDIO_INFO_LOG("Create: src=[%{public}u %{public}u], dst=[%{public}u %{public}u]",
        inputFormat->samplingRate, static_cast<uint32_t>(inputFormat->channelLayout),
        outputFormat->samplingRate, static_cast<uint32_t>(outputFormat->channelLayout));

    auto impl = AudioSuite::AudioFormatConverter::Create(inputFmt, outputFmt);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT,
        "Create failed: AudioFormatConverter::Create returned nullptr, format may be unsupported");

    auto* suiteConverter = new (std::nothrow) OHAudioConverter(std::move(impl));
    CHECK_AND_RETURN_RET_LOG(suiteConverter != nullptr,
        AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED, "Create failed: OHAudioConverter alloc failed");

    *converter = reinterpret_cast<OH_AudioConverter*>(suiteConverter);
    AddConverter(suiteConverter);

    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OH_AudioSuite_ConverterCreateEx(
    const OH_AudioSuite_ConverterFormatEx* inputFormat, const OH_AudioSuite_ConverterFormatEx* outputFormat,
    OH_AudioConverter** converter)
{
    CHECK_AND_RETURN_RET_LOG(CheckSystemPermission(),
        AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP, "ConverterCreateEx permission denied, not system app");
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "ConverterCreateEx failed: The passed address is nullptr.");
    *converter = nullptr;
    CHECK_AND_RETURN_RET_LOG(inputFormat != nullptr && outputFormat != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "ConverterCreateEx failed: format is nullptr");
    CHECK_AND_RETURN_RET_LOG(ValidateConverterFormatEx(inputFormat),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT, "ConverterCreateEx failed: input format validation failed");
    CHECK_AND_RETURN_RET_LOG(ValidateConverterFormatEx(outputFormat),
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT, "ConverterCreateEx failed: output format validation failed");

    AudioConverterFormat inputFmt;
    inputFmt.channelLayout = static_cast<AudioChannelLayout>(inputFormat->channelLayout);
    inputFmt.channelCount = 0;
    inputFmt.sampleFormat = static_cast<AudioSampleFormat>(inputFormat->sampleFormat);
    inputFmt.sampleRate = static_cast<int32_t>(inputFormat->samplingRate);

    AudioConverterFormat outputFmt;
    outputFmt.channelLayout = static_cast<AudioChannelLayout>(outputFormat->channelLayout);
    outputFmt.channelCount = 0;
    outputFmt.sampleFormat = static_cast<AudioSampleFormat>(outputFormat->sampleFormat);
    outputFmt.sampleRate = static_cast<int32_t>(outputFormat->samplingRate);

    AUDIO_INFO_LOG("ConverterCreateEx: src=[%{public}d %{public}u], dst=[%{public}d %{public}u]",
        inputFormat->samplingRate, static_cast<uint32_t>(inputFormat->channelLayout),
        outputFormat->samplingRate, static_cast<uint32_t>(outputFormat->channelLayout));

    auto impl = AudioFormatConverter::Create(inputFmt, outputFmt);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT,
        "ConverterCreateEx failed: AudioFormatConverter::Create returned nullptr");

    auto* suiteConverter = new (std::nothrow) OHAudioConverter(std::move(impl));
    CHECK_AND_RETURN_RET_LOG(suiteConverter != nullptr,
        AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED, "ConverterCreateEx failed: OHAudioConverter alloc failed");

    *converter = reinterpret_cast<OH_AudioConverter*>(suiteConverter);
    AddConverter(suiteConverter);
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OH_AudioConverter_SetInputCallback(
    OH_AudioConverter* converter, OH_AudioConverter_RequestDataCallback callback, void* userData)
{
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Failed to set callback function: The passed address is nullptr.");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr,
        AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET, "Failed to set callback function: The callback is nullptr.");

    auto suiteConverter = reinterpret_cast<OHAudioConverter*>(converter);
    auto impl = GetAudioSuiteConverter(suiteConverter);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr,
        AUDIOCONVERTER_ERROR_NOT_INITIALIZED, "Failed to set callback: Converter not active or already destroyed");

    auto callbackImpl = std::make_shared<ConverterDataCallbackImpl>(callback, userData);
    int32_t ret = impl->SetInputCallback(callbackImpl);
    return ConvertConverterError(ret);
}

OH_AudioConverter_Result OH_AudioConverter_Process(
    OH_AudioConverter* converter, void* audioData, int32_t outputCapacity, int32_t* outputSize)
{
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Data processing failed: Converter is a null pointer");
    CHECK_AND_RETURN_RET_LOG(audioData != nullptr && outputSize != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Process failed: null parameter");

    auto suiteConverter = reinterpret_cast<OHAudioConverter*>(converter);
    auto impl = GetAudioSuiteConverter(suiteConverter);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr,
        AUDIOCONVERTER_ERROR_NOT_INITIALIZED, "Process failed: Converter not active or already destroyed");

    int32_t ret = impl->Process(audioData, static_cast<uint32_t>(outputCapacity),
        reinterpret_cast<uint32_t *>(outputSize));
    return ConvertConverterError(ret);
}

void OH_AudioConverter_Destroy(OH_AudioConverter* converter)
{
    CHECK_AND_RETURN_LOG(converter != nullptr, "Data Destroy failed: Converter is a null pointer");

    auto suiteConverter = reinterpret_cast<OHAudioConverter*>(converter);
    auto impl = GetAudioSuiteConverter(suiteConverter);
    CHECK_AND_RETURN_LOG(impl != nullptr,
        "Data Destroy: Converter not active or already destroyed");
    impl->Destroy();
    RemoveConverter(suiteConverter);
}

OH_AudioConverter_Result OH_AudioSuite_ConverterGetMinInOutSizes(
    OH_AudioConverter *converter, uint32_t *inputMinSize, uint32_t *outputMinSize)
{
    CHECK_AND_RETURN_RET_LOG(CheckSystemPermission(),
        AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP, "GetMinInOutSizes permission denied, not system app");
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "GetMinInOutSizes failed: converter is nullptr");
    CHECK_AND_RETURN_RET_LOG(inputMinSize != nullptr && outputMinSize != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "GetMinInOutSizes failed: output pointer is nullptr.");

    auto suiteConverter = reinterpret_cast<OHAudioConverter*>(converter);
    auto impl = GetAudioSuiteConverter(suiteConverter);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "GetMinInOutSizes failed: Converter not active or already destroyed");

    uint32_t minIn = 0;
    uint32_t minOut = 0;
    int32_t ret = impl->GetMinInOutSizes(minIn, minOut);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ConvertConverterError(ret),
        "GetMinInOutSizes failed: not initialized");
    *inputMinSize = minIn;
    *outputMinSize = minOut;
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OH_AudioSuite_ConverterGetDelaySize(
    OH_AudioConverter* converter, uint32_t* delaySize)
{
    CHECK_AND_RETURN_RET_LOG(CheckSystemPermission(),
        AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP, "GetDelaySize permission denied, not system app");
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "GetDelaySize failed: converter is nullptr");
    CHECK_AND_RETURN_RET_LOG(delaySize != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "GetDelaySize failed: delaySize is nullptr");

    auto suiteConverter = reinterpret_cast<OHAudioConverter*>(converter);
    auto impl = GetAudioSuiteConverter(suiteConverter);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "GetDelaySize failed: Converter not active or already destroyed");

    uint32_t size = 0;
    int32_t ret = impl->GetDelaySize(size);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ConvertConverterError(ret),
        "GetDelaySize failed: must be called after the first process");
    *delaySize = size;
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OH_AudioSuite_ConverterDirectProcess(
    OH_AudioConverter* converter, const OH_AudioSuite_ConverterInputInfo* inData,
    OH_AudioSuite_ConverterOutputInfo* outData)
{
    CHECK_AND_RETURN_RET_LOG(CheckSystemPermission(),
        AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP, "DirectProcess permission denied, not system app");
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "DirectProcess failed: converter is nullptr");
    CHECK_AND_RETURN_RET_LOG(inData != nullptr && outData != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "DirectProcess failed: inData or outData is nullptr");
    CHECK_AND_RETURN_RET_LOG(inData->data != nullptr && inData->size > 0,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "DirectProcess failed: invalid inData");
    CHECK_AND_RETURN_RET_LOG(outData->data != nullptr && outData->size > 0,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "DirectProcess failed: invalid outData");

    auto suiteConverter = reinterpret_cast<OHAudioConverter*>(converter);
    auto impl = GetAudioSuiteConverter(suiteConverter);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "DirectProcess failed: Converter not active or already destroyed");

    uint32_t outSize = 0;
    int32_t ret = impl->DirectProcess(
        static_cast<const uint8_t*>(inData->data), inData->size, outData->data, outData->size, outSize);
    if (ret == SUCCESS) {
        outData->writeSize = outSize;
    }
    return ConvertConverterError(ret);
}

OH_AudioConverter_Result OH_AudioSuite_ConverterFlush(OH_AudioConverter* converter)
{
    CHECK_AND_RETURN_RET_LOG(CheckSystemPermission(),
        AUDIOCONVERTER_ERROR_NOT_SYSTEM_APP, "Flush permission denied, not system app");
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Flush failed: converter is nullptr");

    auto suiteConverter = reinterpret_cast<OHAudioConverter*>(converter);
    auto impl = GetAudioSuiteConverter(suiteConverter);
    CHECK_AND_RETURN_RET_LOG(impl != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Flush failed: Converter not active or already destroyed");

    int32_t ret = impl->Flush();
    return ConvertConverterError(ret);
}

namespace OHOS {
namespace AudioStandard {

std::shared_ptr<AudioSuite::AudioFormatConverter> OHAudioConverter::GetAudioFormatConverter() const
{
    return impl_;
}
}
}
