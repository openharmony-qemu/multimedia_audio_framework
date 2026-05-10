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
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_common.h"
#include <memory>
#include <mutex>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <string>
#include <thread>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "OHAudioSuiteEngine.h"
#include "audio_suite_capabilities.h"
#include "audio_format_converter_impl.h"
#include "audio_format_converter.h"

using OHOS::AudioStandard::OHAudioSuiteEngine;
using OHOS::AudioStandard::OHAudioSuitePipeline;
using OHOS::AudioStandard::OHAudioNode;
using OHOS::AudioStandard::OHAudioSuiteNodeBuilder;
using namespace OHOS::AudioStandard;

static std::atomic<int32_t> g_globalCreateCount(0);
static std::atomic<int32_t> g_globalDestroyCount(0);

static std::unordered_set<OH_AudioConverter*> g_activeConverters;
static std::mutex g_activeConvertersMutex;

// CAPI callback implementation class (adapter pattern)
class ConverterDataCallbackImpl : public AudioSuite::FormatConverterDataCallback {
public:
    ConverterDataCallbackImpl(OH_AudioConverter_RequestDataCallback callback, void *userData)
        : callback_(callback), userData_(userData)
    {}

    int32_t OnRequestData(const void** data, AudioSuite::InputDataStatus* status) override
    {
        CHECK_AND_RETURN_RET_LOG(callback_ != nullptr, -1, "The callback function pointer is nullptr.");

        OH_AudioConverter_InputStatus cStatus;
        int32_t size = callback_(userData_, data, &cStatus);

        // Enumeration type conversion: OH_AudioConverter_InputStatus -> InputDataStatus
        *status = static_cast<AudioSuite::InputDataStatus>(cStatus);

        return size;
    }

private:
    OH_AudioConverter_RequestDataCallback callback_;
    void *userData_;
};

static std::unordered_map<OH_AudioChannelLayout, AudioChannel> MAP_LAYOUT_TO_CHANNEL = {
    {OH_AudioChannelLayout::CH_LAYOUT_MONO, AudioChannel::MONO},
    {OH_AudioChannelLayout::CH_LAYOUT_STEREO, AudioChannel::STEREO},
    {OH_AudioChannelLayout::CH_LAYOUT_STEREO_DOWNMIX, AudioChannel::STEREO},
    {OH_AudioChannelLayout::CH_LAYOUT_2POINT1, AudioChannel::CHANNEL_3},
    {OH_AudioChannelLayout::CH_LAYOUT_3POINT0, AudioChannel::CHANNEL_3},
    {OH_AudioChannelLayout::CH_LAYOUT_SURROUND, AudioChannel::CHANNEL_3},
    {OH_AudioChannelLayout::CH_LAYOUT_3POINT1, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_4POINT0, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_QUAD_SIDE, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_QUAD, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_2POINT0POINT2, AudioChannel::CHANNEL_4},
    {OH_AudioChannelLayout::CH_LAYOUT_4POINT1, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT0, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT0_BACK, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_2POINT1POINT2, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_3POINT0POINT2, AudioChannel::CHANNEL_5},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT1, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT1_BACK, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT0, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_HEXAGONAL, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_3POINT1POINT2, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT0_FRONT, AudioChannel::CHANNEL_6},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT1, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT1_BACK, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_6POINT1_FRONT, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT0, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT0_FRONT, AudioChannel::CHANNEL_7},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT1, AudioChannel::CHANNEL_8},
    {OH_AudioChannelLayout::CH_LAYOUT_OCTAGONAL, AudioChannel::CHANNEL_8},
    {OH_AudioChannelLayout::CH_LAYOUT_5POINT1POINT2, AudioChannel::CHANNEL_8},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE, AudioChannel::CHANNEL_8},
    {OH_AudioChannelLayout::CH_LAYOUT_7POINT1_WIDE_BACK, AudioChannel::CHANNEL_8},
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
        case ERR_INVALID_WRITE:
            return AUDIOCONVERTER_ERROR_CALLBACK_INVALID;
        case ERR_CALLBACK_NOT_FOUND:
            return AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET;
        default:
            return AUDIOCONVERTER_ERROR_SYSTEM;
    }
}

static OHAudioConverter *ConvertAudioSuiteConverter(OH_AudioConverter *audioSuiteConverter)
{
    return reinterpret_cast<OHAudioConverter *>(audioSuiteConverter);
}

OH_AudioConverter_Result OH_AudioConverter_Create(
    const OH_AudioConverter_Format* inputFormat,
    const OH_AudioConverter_Format* outputFormat,
    OH_AudioConverter** converter)
{
    if (converter == nullptr) {
        AUDIO_ERR_LOG("Converter creation failed: The passed address is nullptr.");
        return AUDIOCONVERTER_ERROR_INVALID_PARAM;
    }

    OHAudioConverter *suiteConverter = new OHAudioConverter();
    CHECK_AND_RETURN_RET_LOG(suiteConverter != nullptr,
        AUDIOCONVERTER_ERROR_NOT_INITIALIZED, "Get suiteConverter suiteConverter is nullptr");

    OH_AudioConverter_Result ret = suiteConverter->Create(inputFormat, outputFormat);
    if (ret != AUDIOCONVERTER_SUCCESS) {
        AUDIO_ERR_LOG("Converter creation failed:Format validation failed.");
        delete suiteConverter;
        return ret;
    }
    *converter = (OH_AudioConverter*)suiteConverter;

    {
        std::lock_guard<std::mutex> lock(g_activeConvertersMutex);
        g_activeConverters.insert(*converter);
    }

    g_globalCreateCount++;
    AUDIO_DEBUG_LOG("OH_AudioConverter_Create:global create count=%{public}d", g_globalCreateCount.load());
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OH_AudioConverter_SetInputCallback(
    OH_AudioConverter* converter,
    OH_AudioConverter_RequestDataCallback callback,
    void* userData)
{
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM,
        "Failed to set callback function: The passed address is nullptr.");

    {
        std::lock_guard<std::mutex> lock(g_activeConvertersMutex);
        if (g_activeConverters.find(converter) == g_activeConverters.end()) {
            AUDIO_ERR_LOG("Failed to set callback: Converter not active or already destroyed");
            return AUDIOCONVERTER_ERROR_NOT_INITIALIZED;
        }
    }

    OHAudioConverter *suiteConverter = ConvertAudioSuiteConverter(converter);

    CHECK_AND_RETURN_RET_LOG(suiteConverter != nullptr,
        AUDIOCONVERTER_ERROR_NOT_INITIALIZED, "SetInputCallback suiteConverter is nullptr");

    return suiteConverter->SetRequestDataCallback(callback, userData);
}

OH_AudioConverter_Result OH_AudioConverter_Process(
    OH_AudioConverter* converter,
    void* audioData,
    int32_t outputCapacity,
    int32_t* outputSize)
{
    CHECK_AND_RETURN_RET_LOG(converter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM,
        "Data processing failed: Converter is a null pointer");

    {
        std::lock_guard<std::mutex> lock(g_activeConvertersMutex);
        if (g_activeConverters.find(converter) == g_activeConverters.end()) {
            AUDIO_ERR_LOG("Process failed: Converter not active or already destroyed");
            return AUDIOCONVERTER_ERROR_NOT_INITIALIZED;
        }
    }

    OHAudioConverter *suiteConverter = ConvertAudioSuiteConverter(converter);

    CHECK_AND_RETURN_RET_LOG(suiteConverter != nullptr,
        AUDIOCONVERTER_ERROR_NOT_INITIALIZED, "Process suiteConverter is nullptr");

    OH_AudioConverter_Result ret = suiteConverter->Process(
        audioData, static_cast<uint32_t>(outputCapacity), reinterpret_cast<uint32_t *>(outputSize));
    return ret;
}

void OH_AudioConverter_Destroy(OH_AudioConverter* converter)
{
    if (converter == nullptr) {
        AUDIO_ERR_LOG("Data Destroy failed: Converter is a null pointer");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(g_activeConvertersMutex);
        if (g_activeConverters.find(converter) == g_activeConverters.end()) {
            AUDIO_ERR_LOG("Data Destroy: Converter not active or already destroyed");
            return;
        }
    }
    OHAudioConverter *suiteConverter = ConvertAudioSuiteConverter(converter);
    if (suiteConverter == nullptr) {
        AUDIO_ERR_LOG("Data Destroy failed: OHAudioConverter Converter is a null pointer");
        return;
    }
    suiteConverter->Destroy();
    delete suiteConverter;
    {
        std::lock_guard<std::mutex> lock(g_activeConvertersMutex);
        g_activeConverters.erase(converter);
    }
    g_globalDestroyCount++;
    if (g_globalDestroyCount.load() != g_globalCreateCount.load()) {
        AUDIO_ERR_LOG("OH_AudioConverter_Destroy:global destroy count=%{public}d, global create count=%{public}d",
            g_globalDestroyCount.load(),
            g_globalCreateCount.load());
    }
    return;
}

namespace OHOS {
namespace AudioStandard {
using namespace OHOS::AudioStandard::AudioSuite;
using namespace OHOS::AudioStandard;

OHAudioConverter::~OHAudioConverter()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (impl_ != nullptr) {
        impl_->Destroy();
    }
}

OH_AudioConverter_Result OHAudioConverter::ConvertToPcmFormat(
    const OH_AudioConverter_Format *inputFormat, const OH_AudioConverter_Format *outputFormat)
{
    inputFormat_ = PcmBufferFormat(static_cast<AudioSamplingRate>(inputFormat->samplingRate),
        MAP_LAYOUT_TO_CHANNEL[inputFormat->channelLayout],
        static_cast<AudioChannelLayout>(inputFormat->channelLayout),
        static_cast<AudioSampleFormat>(inputFormat->sampleFormat));

    outputFormat_ = PcmBufferFormat(static_cast<AudioSamplingRate>(outputFormat->samplingRate),
        MAP_LAYOUT_TO_CHANNEL[outputFormat->channelLayout],
        static_cast<AudioChannelLayout>(outputFormat->channelLayout),
        static_cast<AudioSampleFormat>(outputFormat->sampleFormat));

    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::Create(
    const OH_AudioConverter_Format* inputFormat,
    const OH_AudioConverter_Format* outputFormat)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(inputFormat != nullptr && outputFormat != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM,
        "Create failed: format is nullptr");

    CHECK_AND_RETURN_RET_LOG(inputFormat->encodingType == 0 && outputFormat->encodingType == 0,
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT,
        "Create failed: unsupported audio encoding");

    ConvertToPcmFormat(inputFormat, outputFormat);

    AUDIO_INFO_LOG("Create: src=[%{public}u %{public}u], dst=[%{public}u %{public}u]",
        inputFormat->samplingRate,
        MAP_LAYOUT_TO_CHANNEL[inputFormat->channelLayout],
        outputFormat->samplingRate,
        MAP_LAYOUT_TO_CHANNEL[outputFormat->channelLayout]);

    impl_ = AudioFormatConverter::Create(inputFormat_, outputFormat_);
    CHECK_AND_RETURN_RET_LOG(
        impl_ != nullptr, AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT, "Create failed: memory alloc failed");
    
    isInitialized_ = true;
    return AUDIOCONVERTER_SUCCESS;
}

void OHAudioConverter::Destroy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!isInitialized_) {
        AUDIO_ERR_LOG("Destroy: converter is not initialized or already destroyed");
        return;
    }
    if (impl_ != nullptr) {
        impl_->Destroy();
    }
    isInitialized_ = false;
    AUDIO_DEBUG_LOG("Destroy: success");
    return;
}

OH_AudioConverter_Result OHAudioConverter::SetRequestDataCallback(
    OH_AudioConverter_RequestDataCallback callback,
    void* userData)
{
    if (callback == nullptr) {
        return AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    int32_t ret = 0;
    CHECK_AND_RETURN_RET_LOG(
        impl_ != nullptr, AUDIOCONVERTER_ERROR_NOT_INITIALIZED, "The audio format converter pointer is empty.");
    auto callbackImpl = std::make_shared<ConverterDataCallbackImpl>(callback, userData);
    ret = impl_->SetInputCallback(callbackImpl);
    return ConvertConverterError(ret);
}

OH_AudioConverter_Result OHAudioConverter::Process(
    void* audioData, uint32_t outputCapacity, uint32_t* outputSize)
{
    if (audioData == nullptr || outputSize == nullptr) {
        AUDIO_ERR_LOG("Process failed: null parameter");
        return AUDIOCONVERTER_ERROR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(
        impl_ != nullptr, AUDIOCONVERTER_ERROR_NOT_INITIALIZED, "Process failed: impl_ is nullptr");

    int32_t ret = impl_->Process(audioData, outputCapacity, outputSize);
    return ConvertConverterError(ret);
}
}
}
