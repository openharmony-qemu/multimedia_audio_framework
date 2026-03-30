/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

using OHOS::AudioStandard::OHAudioSuiteEngine;
using OHOS::AudioStandard::OHAudioSuitePipeline;
using OHOS::AudioStandard::OHAudioNode;
using OHOS::AudioStandard::OHAudioSuiteNodeBuilder;
using namespace OHOS::AudioStandard;

static std::atomic<int32_t> g_globalCreateCount(0);
static std::atomic<int32_t> g_globalDestroyCount(0);

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

static const std::unordered_set<OH_Audio_SampleRate> SUPPORTED_SAMPLE_RATES = {
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
    OH_Audio_SampleRate::SAMPLE_RATE_192000};

static const std::unordered_set<OH_Audio_SampleFormat> SUPPORTED_SAMPLE_FORMATS = {
    OH_Audio_SampleFormat::AUDIO_SAMPLE_U8,
    OH_Audio_SampleFormat::AUDIO_SAMPLE_S16LE,
    OH_Audio_SampleFormat::AUDIO_SAMPLE_S24LE,
    OH_Audio_SampleFormat::AUDIO_SAMPLE_S32LE,
    OH_Audio_SampleFormat::AUDIO_SAMPLE_F32LE
};

static OHAudioConverter *ConvertAudioSuiteConverter(OH_AudioConverter *audioSuiteConverter)
{
    return (OHAudioConverter *)audioSuiteConverter;
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
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Get suiteConverter suiteConverter is nullptr");

    OH_AudioConverter_Result ret = suiteConverter->Create(inputFormat, outputFormat);
    if (ret != AUDIOCONVERTER_SUCCESS) {
        AUDIO_ERR_LOG("Converter creation failed:Format validation failed.");
        delete suiteConverter;
        return ret;
    }
    *converter = (OH_AudioConverter*)suiteConverter;

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

    OHAudioConverter *suiteConverter = ConvertAudioSuiteConverter(converter);

    CHECK_AND_RETURN_RET_LOG(suiteConverter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "SetInputCallback suiteConverter is nullptr");

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

    OHAudioConverter *suiteConverter = ConvertAudioSuiteConverter(converter);

    CHECK_AND_RETURN_RET_LOG(suiteConverter != nullptr,
        AUDIOCONVERTER_ERROR_INVALID_PARAM, "Process suiteConverter is nullptr");

    OH_AudioConverter_Result ret = suiteConverter->Process(audioData, outputCapacity, outputSize);
    return ret;
}

void OH_AudioConverter_Destroy(OH_AudioConverter* converter)
{
    if (converter == nullptr) {
        AUDIO_ERR_LOG("Data Destroy failed: Converter is a null pointer");
        return;
    }

    OHAudioConverter *suiteConverter = ConvertAudioSuiteConverter(converter);

    if (suiteConverter == nullptr) {
        AUDIO_ERR_LOG("Data Destroy failed: OHAudioConverter Converter is a null pointer");
        return;
    }
    
    suiteConverter->Destroy();

    delete suiteConverter;
    g_globalDestroyCount++;
    if (g_globalDestroyCount.load() != g_globalCreateCount.load()) {
        AUDIO_ERR_LOG("OH_AudioConverter_Destroy:global destroy count=%{public}d, global create count=%{public}d",
            g_globalDestroyCount.load(),
            g_globalCreateCount.load());
    }
    AUDIO_ERR_LOG("void OH_AudioConverter_Destroy202");
    return;
}

namespace OHOS {
namespace AudioStandard {
using namespace OHOS::AudioStandard::AudioSuite;
using namespace OHOS::AudioStandard;

OHAudioConverter::~OHAudioConverter()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (formatConversion_ != nullptr) {
        formatConversion_->Reset();
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

    if (SUPPORTED_SAMPLE_RATES.find(inputFormat->samplingRate) == SUPPORTED_SAMPLE_RATES.end() ||
        SUPPORTED_SAMPLE_RATES.find(outputFormat->samplingRate) == SUPPORTED_SAMPLE_RATES.end()) {
        AUDIO_ERR_LOG("Create failed: unsupported sampling rate");
        return AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT;
    }

    if (MAP_LAYOUT_TO_CHANNEL.find(inputFormat->channelLayout) == MAP_LAYOUT_TO_CHANNEL.end() ||
        MAP_LAYOUT_TO_CHANNEL.find(outputFormat->channelLayout) == MAP_LAYOUT_TO_CHANNEL.end()) {
        AUDIO_ERR_LOG("Create failed: unsupported channel layout");
        return AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT;
    }

    if (SUPPORTED_SAMPLE_FORMATS.find(inputFormat->sampleFormat) == SUPPORTED_SAMPLE_FORMATS.end() ||
        SUPPORTED_SAMPLE_FORMATS.find(outputFormat->sampleFormat) == SUPPORTED_SAMPLE_FORMATS.end()) {
        AUDIO_ERR_LOG("Create failed: unsupported sample format");
        return AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT;
    }

    ConvertToPcmFormat(inputFormat, outputFormat);

    AUDIO_INFO_LOG("Create: src=[%{public}u %{public}u], dst=[%{public}u %{public}u]",
        inputFormat->samplingRate,
        MAP_LAYOUT_TO_CHANNEL[inputFormat->channelLayout],
        outputFormat->samplingRate,
        MAP_LAYOUT_TO_CHANNEL[outputFormat->channelLayout]);

    formatConversion_ = std::make_unique<AudioSuite::AudioSuiteFormatConversion>();
    CHECK_AND_RETURN_RET_LOG(
        formatConversion_ != nullptr, AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED, "Create failed: memory alloc failed");

    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::Destroy()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (formatConversion_ != nullptr) {
        formatConversion_->Reset();
    }
    
    requestCallback_ = nullptr;
    callbackUserData_ = nullptr;
    inputFinished_ = false;
    cachedInputData_.clear();
    cachedOutputData_.clear();

    AUDIO_INFO_LOG("Destroy: success");
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::SetRequestDataCallback(
    OH_AudioConverter_RequestDataCallback callback,
    void* userData)
{
    if (callback == nullptr) {
        return AUDIOCONVERTER_ERROR_CALLBACK_INVALID;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    requestCallback_ = callback;
    callbackUserData_ = userData;
    
    AUDIO_INFO_LOG("SetRequestDataCallback: success");
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::ValidateProcessParams(
    void* audioData, int32_t* outputSize)
{
    if (audioData == nullptr || outputSize == nullptr) {
        AUDIO_ERR_LOG("Process failed: null parameter");
        return AUDIOCONVERTER_ERROR_INVALID_PARAM;
    }

    if (requestCallback_ == nullptr) {
        AUDIO_ERR_LOG("Process failed: callback not set");
        return AUDIOCONVERTER_ERROR_CALLBACK_NOT_SET;
    }

    return AUDIOCONVERTER_SUCCESS;
}

void OHAudioConverter::CalculateOutputFormatParams(
    int32_t outputCapacity, int32_t* outputBytesPerFrame, int32_t* maxOutputFrames)
{
    uint32_t outputSampleSize = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat);
    *outputBytesPerFrame = outputSampleSize * outputFormat_.channelCount;
    *maxOutputFrames = outputCapacity / *outputBytesPerFrame;
}

int32_t OHAudioConverter::CalculateMinBytesForIntMs(uint32_t inputBytesPerFrame)
{
    uint64_t inputBytesPerSec = static_cast<uint64_t>(inputFormat_.sampleRate) * inputBytesPerFrame;

    int64_t gcd = std::gcd(static_cast<int64_t>(inputFormat_.sampleRate),
                          static_cast<int64_t>(outputFormat_.sampleRate));
    uint64_t gcdSecond = static_cast<uint64_t>(std::gcd(gcd, SECONDS_TO_MS));
    uint32_t minTimeMs = static_cast<uint32_t>(SECONDS_TO_MS / gcdSecond);

    return static_cast<int32_t>(inputBytesPerSec * minTimeMs / SECONDS_TO_MS);
}

OH_AudioConverter_Result OHAudioConverter::ReadAndCacheInputData(bool* shouldStopProcessing)
{
    const void* inputDataPtr = nullptr;
    OH_AudioConverter_InputStatus inputStatus; //需要初始化一个不存在的值

    int32_t inputSize = requestCallback_(
        callbackUserData_,
        &inputDataPtr,
        &inputStatus);
    
    const int32_t MAX_CALLBACK_DATA_SIZE = 400 * 1024;
    if (inputSize > MAX_CALLBACK_DATA_SIZE || inputSize < 0) {
        AUDIO_ERR_LOG("The callback function returned an error indicating the amount of data was incorrect: %{public}d",
            inputSize);
        return AUDIOCONVERTER_ERROR_SYSTEM;
    }

    if (inputStatus == AUDIOCONVERTER_INPUT_DATA_FINISHED) {
        inputFinished_ = true;
    }

    if (inputStatus == AUDIOCONVERTER_INPUT_NO_AVAILABLE_DATA) {
        *shouldStopProcessing = true;
        return AUDIOCONVERTER_SUCCESS;
    }

    if (inputSize == 0 && inputStatus != AUDIOCONVERTER_INPUT_DATA_FINISHED) {
        *shouldStopProcessing = true;
        return AUDIOCONVERTER_SUCCESS;
    }

    if (inputSize > 0 && inputDataPtr != nullptr) {
        cachedInputData_.insert(cachedInputData_.end(),
            static_cast<const uint8_t*>(inputDataPtr),
            static_cast<const uint8_t*>(inputDataPtr) + inputSize);
    }

    *shouldStopProcessing = false;
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::CalculateMinBytesForIntMsInternal(uint32_t inputBytesPerFrame,
    int32_t *minBytesForIntMs, int32_t *alignedCachedSize, bool *shouldStopProcessing, uint32_t outputCapacity)
{
    int32_t cachedSize = static_cast<int32_t>(cachedInputData_.size());
    *minBytesForIntMs = CalculateMinBytesForIntMs(inputBytesPerFrame);

    *alignedCachedSize = (cachedSize / *minBytesForIntMs) * *minBytesForIntMs;

    if (*alignedCachedSize == 0) {

        if (inputFinished_ && cachedSize > 0) {
            int32_t paddingSize = *minBytesForIntMs - cachedSize;
            cachedInputData_.insert(cachedInputData_.end(), paddingSize, 0);
            *alignedCachedSize = *minBytesForIntMs;
            AUDIO_DEBUG_LOG("Process: padding %{public}d bytes for final data to meet int ms", paddingSize);
            *shouldStopProcessing = false;
        } else {
            AUDIO_DEBUG_LOG("Process: waiting for more data, cachedSize=%{public}d, minBytesForIntMs=%{public}d",
                cachedSize,
                *minBytesForIntMs);
            *shouldStopProcessing = true;
            return AUDIOCONVERTER_SUCCESS;
        }
    }

    AUDIO_ERR_LOG("OHAudioConverter::Process550");

    *shouldStopProcessing = false;
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::CalculateProcessableBytesInternal(
    const ProcessableBytesCalculationParams& params, uint32_t* alignedProcessableBytes, bool* shouldStopProcessing)
{
    double sampleRateRatio = static_cast<double>(inputFormat_.sampleRate) /
                            static_cast<double>(outputFormat_.sampleRate);
    int32_t estimatedInputFrames = static_cast<int32_t>(params.maxOutputFrames * sampleRateRatio);

    int32_t maxProcessableBytes = std::min(params.alignedCachedSize,
        static_cast<int32_t>(estimatedInputFrames * params.inputBytesPerFrame));
    
    bool is11025SampleRate = (inputFormat_.sampleRate == SAMPLE_RATE_11025 ||
                               outputFormat_.sampleRate == SAMPLE_RATE_11025);
    
    if (!is11025SampleRate) {
        int32_t maxFramesFor20ms = (inputFormat_.sampleRate * 20) / 1000;
        int32_t maxBytesFor20ms = maxFramesFor20ms * params.inputBytesPerFrame;
        maxProcessableBytes = std::min(maxProcessableBytes, maxBytesFor20ms);
    }

    if (is11025SampleRate) {
        if (maxProcessableBytes >= params.minBytesForIntMs) {
            *alignedProcessableBytes = params.minBytesForIntMs;
        } else {
            *alignedProcessableBytes = 0;
        }
    } else {
        *alignedProcessableBytes = (maxProcessableBytes / params.minBytesForIntMs) *
                                    params.minBytesForIntMs;
    }

    CHECK_AND_RETURN_RET_LOG(*alignedProcessableBytes != 0,
        AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL,
        "Process: output capacity too small, maxProcessableBytes=%{public}d, minBytesForIntMs=%{public}d",
        maxProcessableBytes,
        params.minBytesForIntMs);

    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::WrapAndConvertInputData(
    int32_t processableSize, AudioSuite::AudioSuitePcmBuffer* outConvertedBuffer)
{
    AudioSuite::AudioSuitePcmBuffer wrappedInput;
    OH_AudioConverter_Result wrapResult = WrapInputData(
        cachedInputData_.data(),
        processableSize,
        inputFinished_,
        wrappedInput);
    CHECK_AND_RETURN_RET_LOG(wrapResult == AUDIOCONVERTER_SUCCESS,
        AUDIOCONVERTER_ERROR_SYSTEM,
        "WrapInputData failed: %{public}d",
        wrapResult);

    AudioSuite::AudioSuitePcmBuffer* tempBuffer = DoConvert(&wrappedInput);
    CHECK_AND_RETURN_RET_LOG(tempBuffer != nullptr, AUDIOCONVERTER_ERROR_SYSTEM, "DoConvert failed: null output");
    *outConvertedBuffer = *tempBuffer;
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::CheckAndProcessInputData(
    const InputProcessParams& params, int32_t* minBytesForIntMs, int32_t* alignedCachedSize,
    uint32_t* alignedProcessableBytes, bool* shouldStopProcessing)
{
    OH_AudioConverter_Result ret = CalculateMinBytesForIntMsInternal(
        params.inputBytesPerFrame, minBytesForIntMs, alignedCachedSize, shouldStopProcessing, params.outputCapacity);
    CHECK_AND_RETURN_RET_LOG(
        ret == AUDIOCONVERTER_SUCCESS, ret, "Failed to calculate minimum number of bytes to process");
    if (*shouldStopProcessing) {
        return AUDIOCONVERTER_SUCCESS;
    }

    ProcessableBytesCalculationParams processableParams = {
        params.inputBytesPerFrame, params.maxOutputFrames,
        params.outputBytesPerFrame, *minBytesForIntMs, *alignedCachedSize
    };
    ret = CalculateProcessableBytesInternal(processableParams, alignedProcessableBytes, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOCONVERTER_SUCCESS, ret, "Insufficient output capacity");

    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::InitializeProcessingContext(void *audioData, int32_t outputCapacity,
    int32_t inputBytesPerFrame, ProcessingContext &context, bool &shouldStopProcessing)
{
    context.outputBufferPtr = static_cast<uint8_t *>(audioData);

    AUDIO_ERR_LOG("Process: processed block, outputCapacity=%{public}d", outputCapacity);

    int32_t cachedSize = static_cast<int32_t>(cachedInputData_.size());
    int32_t tempMinBytesForIntMs = CalculateMinBytesForIntMs(inputBytesPerFrame);
    int32_t tempAlignedCachedSize = (cachedSize / tempMinBytesForIntMs) * tempMinBytesForIntMs;

    if (tempAlignedCachedSize == 0 && !inputFinished_) {
        OH_AudioConverter_Result ret = ReadAndCacheInputData(&shouldStopProcessing);
        if (ret != AUDIOCONVERTER_SUCCESS) {
            return ret;
        }
        if (shouldStopProcessing) {
            return AUDIOCONVERTER_SUCCESS;
        }
    }
    shouldStopProcessing = false;

    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::ValidateOutputCapacity(
    AudioSuite::AudioSuitePcmBuffer &convertedBuffer, uint32_t remainingCapacity, uint32_t *framesToWrite)
{
    uint32_t availableFrames = convertedBuffer.GetFrameLen();
    uint32_t outputSampleSize = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat);
    uint32_t outputBytesPerFrame = outputFormat_.channelCount * outputSampleSize;

    uint32_t maxFramesByCapacity = remainingCapacity / outputBytesPerFrame;

    *framesToWrite = (availableFrames < maxFramesByCapacity) ? availableFrames : maxFramesByCapacity;

    if (*framesToWrite == 0 && availableFrames > 0) {
        AUDIO_ERR_LOG(
            "Process failed: output buffer too small for one frame, required=%{public}d, remaining=%{public}d",
            outputBytesPerFrame,
            remainingCapacity);
        return AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL;
    }
    return AUDIOCONVERTER_SUCCESS;
}

void OHAudioConverter::UpdateProcessingState(
    AudioSuite::AudioSuitePcmBuffer& convertedBuffer,
    uint32_t processableSize, ProcessingContext& context, uint32_t actualFramesWritten)
{
    uint32_t outputSampleSize = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat);
    uint32_t actualOutputDataSize = actualFramesWritten * outputFormat_.channelCount * outputSampleSize;
    context.totalOutputSize += actualOutputDataSize;
    context.outputBufferPtr += actualOutputDataSize;
    context.remainingCapacity -= actualOutputDataSize;
    cachedInputData_.erase(cachedInputData_.begin(), cachedInputData_.begin() + processableSize);

    AUDIO_ERR_LOG("Process: processed block, outputBytes=%{public}d, totalOutput=%{public}d, remaining=%{public}d",
        actualOutputDataSize, context.totalOutputSize, context.remainingCapacity);
}

OH_AudioConverter_Result OHAudioConverter::ProcessSingleBlock(
    const InputProcessParams& params, ProcessingContext& context, int32_t inputBytesPerFrame,
    int32_t outputBytesPerFrame, bool& shouldStopProcessing)
{
    shouldStopProcessing = false;
    int32_t minBytesForIntMs = 0;
    int32_t alignedCachedSize = 0;
    uint32_t alignedProcessableBytes = 0;

    int32_t currentMaxOutputFrames = context.remainingCapacity / outputBytesPerFrame;

    InputProcessParams inputParams = {
        inputBytesPerFrame, currentMaxOutputFrames, outputBytesPerFrame, context.remainingCapacity};
    OH_AudioConverter_Result ret = CheckAndProcessInputData(
        inputParams, &minBytesForIntMs, &alignedCachedSize, &alignedProcessableBytes, &shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOCONVERTER_SUCCESS, ret, "Input data processing failed");
    if (shouldStopProcessing) {
        return AUDIOCONVERTER_SUCCESS;
    }

    uint32_t processableSize = alignedProcessableBytes;

    AudioSuite::AudioSuitePcmBuffer convertedBuffer;
    ret = WrapAndConvertInputData(processableSize, &convertedBuffer);
    CHECK_AND_RETURN_RET_LOG(
        ret == AUDIOCONVERTER_SUCCESS, ret, "Data encapsulation and transformation process failed");

    uint32_t framesToWrite = 0;
    ret = ValidateOutputCapacity(convertedBuffer, context.remainingCapacity, &framesToWrite);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOCONVERTER_SUCCESS, ret, "Output capacity validity verification failed.");

    CopyOutputData(&convertedBuffer, context.outputBufferPtr, framesToWrite, &framesToWrite);

    UpdateProcessingState(convertedBuffer, processableSize, context, framesToWrite);
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::ProcessCachedOutputData(void* audioData, int32_t outputCapacity,
    int32_t outputBytesPerFrame, ProcessingContext& context)
{
    if (cachedOutputData_.empty()) {
        return AUDIOCONVERTER_SUCCESS;
    }

    uint32_t cachedSize = static_cast<uint32_t>(cachedOutputData_.size());
    uint32_t bytesToCopy = (cachedSize < static_cast<uint32_t>(outputCapacity)) ? cachedSize : outputCapacity;

    if (bytesToCopy < static_cast<uint32_t>(outputBytesPerFrame)) {
        return AUDIOCONVERTER_SUCCESS;
    }

    uint32_t framesToCopy = bytesToCopy / outputBytesPerFrame;
    bytesToCopy = framesToCopy * outputBytesPerFrame;

    errno_t copyRet = memcpy_s(audioData, bytesToCopy, cachedOutputData_.data(), bytesToCopy);
    CHECK_AND_RETURN_RET_LOG(copyRet == 0,
        AUDIOCONVERTER_ERROR_SYSTEM,
        "ProcessCachedOutputData failed: memcpy_s for cached output data failed");

    context.totalOutputSize = bytesToCopy;
    context.outputBufferPtr += bytesToCopy;
    context.remainingCapacity = outputCapacity - bytesToCopy;

    if (bytesToCopy < cachedSize) {
        cachedOutputData_.erase(cachedOutputData_.begin(), cachedOutputData_.begin() + bytesToCopy);
    } else {
        cachedOutputData_.clear();
    }

    AUDIO_ERR_LOG(
        "ProcessCachedOutputData: returned %{public}d bytes from cached output data, remaining cached: %{public}zu",
        bytesToCopy,
        cachedOutputData_.size());

    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::ProcessDataBlocks(ProcessingContext& context, int32_t inputBytesPerFrame,
    int32_t outputBytesPerFrame)
{
    OH_AudioConverter_Result ret;
    while (context.remainingCapacity > 0) {
        bool shouldStopProcessing = false;
        InputProcessParams inputParams = {inputBytesPerFrame,
            context.remainingCapacity / outputBytesPerFrame,
            outputBytesPerFrame,
            context.remainingCapacity};

        ret = ProcessSingleBlock(inputParams, context, inputBytesPerFrame, outputBytesPerFrame, shouldStopProcessing);
        if (ret != AUDIOCONVERTER_SUCCESS || shouldStopProcessing) {
            break;
        }
    }
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::ProcessConversion(
    void *audioData, int32_t outputCapacity, int32_t *outputSize, int32_t outputBytesPerFrame, int32_t maxOutputFrames)
{
    uint32_t inputSampleSize = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat);
    uint32_t inputBytesPerFrame = inputSampleSize * inputFormat_.channelCount;

    CHECK_AND_RETURN_RET_LOG(inputBytesPerFrame != 0 && outputBytesPerFrame != 0,
        AUDIOCONVERTER_ERROR_UNSUPPORTED_FORMAT,
        "Process failed: invalid input or output format");

    ProcessingContext context;
    context.totalOutputSize = 0;
    context.outputBufferPtr = static_cast<uint8_t *>(audioData);
    context.remainingCapacity = outputCapacity;

    OH_AudioConverter_Result ret = ProcessCachedOutputData(audioData, outputCapacity, outputBytesPerFrame, context);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOCONVERTER_SUCCESS, ret, "Failed to process output buffer data.");

    if (context.remainingCapacity < static_cast<uint32_t>(outputBytesPerFrame)) {
        *outputSize = context.totalOutputSize;
        return AUDIOCONVERTER_SUCCESS;
    }

    bool shouldStopProcessing = false;
    ret = InitializeProcessingContext(
        context.outputBufferPtr, context.remainingCapacity, inputBytesPerFrame, context, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOCONVERTER_SUCCESS, ret, "Input buffer initialization failed.");
    if (shouldStopProcessing) {
        *outputSize = context.totalOutputSize;
        return AUDIOCONVERTER_SUCCESS;
    }

    ret = ProcessDataBlocks(context, inputBytesPerFrame, outputBytesPerFrame);
    CHECK_AND_RETURN_RET_LOG(ret == AUDIOCONVERTER_SUCCESS, ret, "Failed to process data in the input buffer.");

    *outputSize = context.totalOutputSize;
    return AUDIOCONVERTER_SUCCESS;
}

OH_AudioConverter_Result OHAudioConverter::Process(
    void* audioData,
    int32_t outputCapacity,
    int32_t* outputSize)
{
    OH_AudioConverter_Result ret = ValidateProcessParams(audioData, outputSize);
    if (ret != AUDIOCONVERTER_SUCCESS) {
        return ret;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    *outputSize = 0;

    int32_t outputBytesPerFrame = 0;
    int32_t maxOutputFrames = 0;
    CalculateOutputFormatParams(outputCapacity, &outputBytesPerFrame, &maxOutputFrames);
    if (outputBytesPerFrame >  outputCapacity) {
        return AUDIOCONVERTER_ERROR_BUFFER_TOO_SMALL;
    }
    ret = ProcessConversion(audioData, outputCapacity, outputSize, outputBytesPerFrame, maxOutputFrames);
    return ret;
}

void OHAudioConverter::Reset()
{
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (formatConversion_ != nullptr) {
        formatConversion_->Reset();
    }
    
    inputFinished_ = false;
    cachedInputData_.clear();
    cachedOutputData_.clear();
    
    AUDIO_INFO_LOG("Reset: success");
}

OH_AudioConverter_Result OHAudioConverter::WrapInputData(
    void* audioData,
    uint32_t audioDataSize,
    bool finished,
    AudioSuite::AudioSuitePcmBuffer& buffer)
{
    if (finished && audioDataSize == 0) {
        return AUDIOCONVERTER_SUCCESS;
    }
    
    AudioSuite::PcmBufferFormat formatInfo;
    formatInfo.sampleRate = inputFormat_.sampleRate;
    formatInfo.channelCount = inputFormat_.channelCount;
    formatInfo.channelLayout = inputFormat_.channelLayout;
    formatInfo.sampleFormat = inputFormat_.sampleFormat;
    
    uint32_t sampleSize = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat);
    uint32_t bytesPerFrame = inputFormat_.channelCount * sampleSize;
    
    CHECK_AND_RETURN_RET_LOG(bytesPerFrame != 0 && inputFormat_.sampleRate != 0,
        AUDIOCONVERTER_ERROR_INVALID_PARAM,
        "WrapInputData failed: invalid format, bytesPerFrame=%{public}d, sampleRate=%{public}d",
        bytesPerFrame,
        inputFormat_.sampleRate);

    int32_t dataDurationMs = static_cast<int32_t>(std::round(
        static_cast<double>(audioDataSize) * 1000.0 / (static_cast<double>(inputFormat_.sampleRate) * bytesPerFrame)));
    int32_t result = buffer.ResizePcmBuffer(formatInfo, dataDurationMs);
    CHECK_AND_RETURN_RET_LOG(result == 0, AUDIOCONVERTER_ERROR_MEMORY_ALLOC_FAILED, "Memory allocation failed.");
    
    if (buffer.GetPcmData() != nullptr && audioData != nullptr) {
        uint32_t bufferSize = buffer.GetDataSize();
        CHECK_AND_RETURN_RET_LOG(
            bufferSize >= audioDataSize, AUDIOCONVERTER_ERROR_SYSTEM, "memcpy_s Insufficient target buffer size");
        errno_t ret = memcpy_s(buffer.GetPcmData(), bufferSize, audioData, audioDataSize);
        CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIOCONVERTER_ERROR_SYSTEM, "memcpy_s Execution failed");
    }
    
    return AUDIOCONVERTER_SUCCESS;
}

AudioSuite::AudioSuitePcmBuffer* OHAudioConverter::DoConvert(
    AudioSuite::AudioSuitePcmBuffer* inputBuffer)
{
    CHECK_AND_RETURN_RET_LOG(formatConversion_ != nullptr, nullptr, "DoConvert failed: formatConversion is null");
    
    AudioSuite::PcmBufferFormat outFormatInfo;
    outFormatInfo.sampleRate = outputFormat_.sampleRate;
    outFormatInfo.channelCount = outputFormat_.channelCount;
    outFormatInfo.channelLayout = outputFormat_.channelLayout;
    outFormatInfo.sampleFormat = outputFormat_.sampleFormat;
    
    AudioSuite::AudioSuitePcmBuffer* outputBuffer = formatConversion_->Process(inputBuffer, outFormatInfo);
    
    return outputBuffer;
}

void OHAudioConverter::CopyOutputData(
    AudioSuite::AudioSuitePcmBuffer* buffer, void* outputBuffer, uint32_t maxFrameSize, uint32_t* actualFrameSize)
{
    uint32_t availableFrames = buffer->GetFrameLen();
    *actualFrameSize = (availableFrames < maxFrameSize)
                      ? availableFrames : maxFrameSize;
    
    uint32_t outputSampleSize = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat);
    uint32_t outputBytes = *actualFrameSize * outputFormat_.channelCount * outputSampleSize;

    if (outputBuffer != nullptr && buffer->GetPcmData() != nullptr) {
        int32_t ret = memcpy_s(outputBuffer, outputBytes, buffer->GetPcmData(), outputBytes);
        if (ret != 0) {
            AUDIO_ERR_LOG("memcpy_s failed: memcpy_s for cached data failed");
            return;
        }
    }
    if (*actualFrameSize < availableFrames) {
    uint32_t totalBytes = availableFrames * outputFormat_.channelCount * outputSampleSize;
    uint32_t copiedBytes = outputBytes;
    uint32_t remainingBytes = totalBytes - copiedBytes;
    uint32_t existingSize = static_cast<uint32_t>(cachedOutputData_.size());
    
    cachedOutputData_.resize(existingSize + remainingBytes);
    errno_t ret = memcpy_s(cachedOutputData_.data() + existingSize, remainingBytes,
        static_cast<const uint8_t*>(buffer->GetPcmData()) + copiedBytes, remainingBytes);
    if (ret != 0) {
    AUDIO_ERR_LOG("CopyOutputData failed: memcpy_s for cached data failed");
    }
    
    AUDIO_ERR_LOG("CopyOutputData: cached %{public}d bytes of uncopied output data", remainingBytes);
    }
}
}
}