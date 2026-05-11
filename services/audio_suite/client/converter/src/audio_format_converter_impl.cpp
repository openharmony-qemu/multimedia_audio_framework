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

#include "audio_format_converter_impl.h"
#include "audio_suite_common.h"
#include "audio_suite_log.h"
#include "audio_log.h"
#include "audio_errors.h"
#include <cmath>
#include <cstring>
#include <securec.h>
#include <numeric>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// The set of sampling rates supported by format validation
static const std::unordered_set<AudioSamplingRate> SUPPORTED_SAMPLE_RATES = {
    AudioSamplingRate::SAMPLE_RATE_8000,
    AudioSamplingRate::SAMPLE_RATE_11025,
    AudioSamplingRate::SAMPLE_RATE_12000,
    AudioSamplingRate::SAMPLE_RATE_16000,
    AudioSamplingRate::SAMPLE_RATE_22050,
    AudioSamplingRate::SAMPLE_RATE_24000,
    AudioSamplingRate::SAMPLE_RATE_32000,
    AudioSamplingRate::SAMPLE_RATE_44100,
    AudioSamplingRate::SAMPLE_RATE_48000,
    AudioSamplingRate::SAMPLE_RATE_64000,
    AudioSamplingRate::SAMPLE_RATE_88200,
    AudioSamplingRate::SAMPLE_RATE_96000,
    AudioSamplingRate::SAMPLE_RATE_176400,
    AudioSamplingRate::SAMPLE_RATE_192000
};

// The set of sampling formats supported by format validation
static const std::unordered_set<AudioSampleFormat> SUPPORTED_SAMPLE_FORMATS = {
    AudioSampleFormat::SAMPLE_U8,
    AudioSampleFormat::SAMPLE_S16LE,
    AudioSampleFormat::SAMPLE_S24LE,
    AudioSampleFormat::SAMPLE_S32LE,
    AudioSampleFormat::SAMPLE_F32LE
};

constexpr int32_t MAX_CALLBACK_DATA_SIZE = 400 * 1024; // 400KB
constexpr uint32_t MIN_FRAMES_FOR_20MS = 20;
constexpr uint32_t MILLISECONDS_PER_SECOND = 1000;

// Sample rate validation constants
constexpr uint32_t REQUIRED_MS_FOR_11025HZ = 40;
constexpr uint32_t MAX_DURATION_MS = 60;
constexpr uint32_t MIN_DURATION_MS = 20;
constexpr uint32_t MAX_RESAMPLE_QUALITY = 10;
constexpr uint32_t MIN_RESAMPLE_QUALITY = 1;

std::mutex AudioFormatConverter::createMutex_;

AudioFormatConverterImpl::AudioFormatConverterImpl()
    : destroyed_(false), inputFinished_(false)
{
}

AudioFormatConverterImpl::~AudioFormatConverterImpl()
{
}

std::shared_ptr<AudioFormatConverter> AudioFormatConverter::Create(
    const PcmBufferFormat &inputFormat, const PcmBufferFormat &outputFormat, const uint32_t resampleQuality)
{
    std::lock_guard<std::mutex> lock(createMutex_);

    // Format validation: Sampling rate
    if (SUPPORTED_SAMPLE_RATES.find(inputFormat.sampleRate) == SUPPORTED_SAMPLE_RATES.end() ||
        SUPPORTED_SAMPLE_RATES.find(outputFormat.sampleRate) == SUPPORTED_SAMPLE_RATES.end()) {
        AUDIO_ERR_LOG("Create: unsupported sampling rate");
        return nullptr;
    }

    // Format verification: Audio channel layout
    if (LAYOUT_TO_CHANNEL.find(inputFormat.channelLayout) == LAYOUT_TO_CHANNEL.end() ||
        LAYOUT_TO_CHANNEL.find(outputFormat.channelLayout) == LAYOUT_TO_CHANNEL.end()) {
        AUDIO_ERR_LOG("Create: unsupported channel layout");
        return nullptr;
    }

    // Format validation: Sampling format
    if (SUPPORTED_SAMPLE_FORMATS.find(inputFormat.sampleFormat) == SUPPORTED_SAMPLE_FORMATS.end() ||
        SUPPORTED_SAMPLE_FORMATS.find(outputFormat.sampleFormat) == SUPPORTED_SAMPLE_FORMATS.end()) {
        AUDIO_ERR_LOG("Create: unsupported sample format");
        return nullptr;
    }

    CHECK_AND_RETURN_RET_LOG(resampleQuality >= MIN_RESAMPLE_QUALITY && resampleQuality <= MAX_RESAMPLE_QUALITY,
        nullptr, "Resampling level assignment error");
    
    AUDIO_INFO_LOG("Create: src=[%{public}u %{public}u], dst=[%{public}u %{public}u]",
        inputFormat.sampleRate, LAYOUT_TO_CHANNEL.at(inputFormat.channelLayout),
        outputFormat.sampleRate, LAYOUT_TO_CHANNEL.at(outputFormat.channelLayout));
    auto audioFormatConverter = std::make_shared<AudioFormatConverterImpl>();
    audioFormatConverter->inputFormat_ = inputFormat;
    audioFormatConverter->outputFormat_ = outputFormat;

    audioFormatConverter->formatConversion_ = std::make_unique<AudioSuiteFormatConversion>();
    CHECK_AND_RETURN_RET_LOG(audioFormatConverter->formatConversion_ != nullptr,
        nullptr, "Create: Failed to create format conversion instance");

    audioFormatConverter->rawFormatConversion_ = std::make_unique<AudioSuiteRawFormatConversion>(resampleQuality);
    CHECK_AND_RETURN_RET_LOG(audioFormatConverter->rawFormatConversion_ != nullptr,
        nullptr, "Create: failed to create rawFormatConversion");
 
    int32_t ret = audioFormatConverter->rawFormatConversion_->Init(
        audioFormatConverter->inputFormat_, audioFormatConverter->outputFormat_);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "Create: rawFormatConversion init failed");

    return audioFormatConverter;
}

int32_t AudioFormatConverterImpl::SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_ILLEGAL_STATE, "SetInputCallback: converter already destroyed");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_CALLBACK_NOT_FOUND, "Callback not set");
    dataCallback_ = std::move(callback);
    return SUCCESS;
}

uint32_t AudioFormatConverterImpl::CalculateMinBytesForIntMs(uint32_t inputBytesPerFrame) const
{
    uint64_t inputBytesPerSec = static_cast<uint64_t>(inputFormat_.sampleRate) * inputBytesPerFrame;

    uint64_t gcd =
        std::gcd(static_cast<uint64_t>(inputFormat_.sampleRate), static_cast<uint64_t>(outputFormat_.sampleRate));
    uint64_t gcdSecond = std::gcd(gcd, SECONDS_TO_MS);
    CHECK_AND_RETURN_RET_LOG(
        gcdSecond != 0, ERR_OPERATION_FAILED, "The minimum processing time cannot be divided by zero.");
    uint32_t minTimeMs = static_cast<uint32_t>(SECONDS_TO_MS / gcdSecond);
    CHECK_AND_RETURN_RET_LOG(
        SECONDS_TO_MS != 0, ERR_OPERATION_FAILED, "The minimum number of bytes to process cannot be divided by zero.");
    return static_cast<uint32_t>(inputBytesPerSec * minTimeMs / SECONDS_TO_MS);
}

int32_t AudioFormatConverterImpl::ReadAndCacheInputData(bool* shouldStopProcessing)
{
    const void* inputDataPtr = nullptr;
    InputDataStatus status = InputDataStatus::HAVE_DATA;
    CHECK_AND_RETURN_RET_LOG(dataCallback_ != nullptr, ERR_CALLBACK_NOT_FOUND, "Callback function not set.");
    int32_t inputSize = dataCallback_->OnRequestData(&inputDataPtr, &status);
    if (inputSize > MAX_CALLBACK_DATA_SIZE || inputSize < 0) {
        AUDIO_ERR_LOG("The callback function returned an error indicating the amount of data was incorrect: %{public}d",
            inputSize);
        return ERR_INVALID_WRITE;
    }

    if (status == InputDataStatus::DATA_FINISHED) {
        inputFinished_ = true;
    }

    if (status == InputDataStatus::NO_AVAILABLE_DATA) {
        *shouldStopProcessing = true;
        return SUCCESS;
    }

    if (inputSize == 0 && status != InputDataStatus::DATA_FINISHED) {
        *shouldStopProcessing = true;
        return SUCCESS;
    }

    if (inputSize > 0 && inputDataPtr != nullptr) {
        cachedInputData_.insert(cachedInputData_.end(),
            static_cast<const uint8_t*>(inputDataPtr),
            static_cast<const uint8_t*>(inputDataPtr) + inputSize);
    }

    *shouldStopProcessing = false;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::CalculateMinBytesForIntMsInternal(uint32_t inputBytesPerFrame,
    uint32_t& minBytesForIntMs, uint32_t& alignedCachedSize, bool& shouldStopProcessing, uint32_t outputCapacity)
{
    uint32_t cachedSize = static_cast<uint32_t>(cachedInputData_.size());
    minBytesForIntMs = CalculateMinBytesForIntMs(inputBytesPerFrame);
    CHECK_AND_RETURN_RET_LOG(
        minBytesForIntMs != 0, ERR_INVALID_PARAM, "The minimum amount of data to process cannot be 0.");
    alignedCachedSize = (cachedSize / minBytesForIntMs) * minBytesForIntMs;
    if (alignedCachedSize == 0) {
        if (inputFinished_ && cachedSize > 0) {
            uint32_t paddingSize = minBytesForIntMs - cachedSize;
            cachedInputData_.insert(cachedInputData_.end(), paddingSize, 0);
            alignedCachedSize = minBytesForIntMs;
            AUDIO_DEBUG_LOG("Process: padding %{public}u bytes for final data to meet int ms", paddingSize);
            shouldStopProcessing = false;
        } else {
            AUDIO_DEBUG_LOG("Process: waiting for more data, cachedSize=%{public}d, minBytesForIntMs=%{public}d",
                cachedSize,
                minBytesForIntMs);
            shouldStopProcessing = true;
            return SUCCESS;
        }
    }

    shouldStopProcessing = false;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::CalculateProcessableBytesInternal(
    const ProcessableBytesCalculationParams& params, uint32_t& alignedProcessableBytes, bool& shouldStopProcessing)
{
    double sampleRateRatio =
        static_cast<double>(inputFormat_.sampleRate) / static_cast<double>(outputFormat_.sampleRate);
    uint32_t estimatedInputFrames = static_cast<uint32_t>(params.maxOutputFrames * sampleRateRatio);
    if (estimatedInputFrames == 0 && params.maxOutputFrames > 0) {
        estimatedInputFrames = 1;
    }
    uint32_t maxProcessableBytes = std::min(params.alignedCachedSize,
        static_cast<uint32_t>(estimatedInputFrames * params.inputBytesPerFrame));

    bool is11025SampleRate = (inputFormat_.sampleRate == AudioSamplingRate::SAMPLE_RATE_11025 ||
                              outputFormat_.sampleRate == AudioSamplingRate::SAMPLE_RATE_11025);

    if (!is11025SampleRate) {
        uint32_t maxFramesFor20ms = (inputFormat_.sampleRate * MIN_FRAMES_FOR_20MS) / MILLISECONDS_PER_SECOND;
        uint32_t maxBytesFor20ms = maxFramesFor20ms * params.inputBytesPerFrame;
        maxProcessableBytes = std::min(maxProcessableBytes, maxBytesFor20ms);
    }
    CHECK_AND_RETURN_RET_LOG(params.minBytesForIntMs > 0,
        ERR_INVALID_PARAM,
        "The minimum number of bytes of input data to be processed cannot be zero.");
    if (maxProcessableBytes >= params.minBytesForIntMs) {
        if (is11025SampleRate) {
            alignedProcessableBytes = params.minBytesForIntMs;
        } else {
            alignedProcessableBytes = (maxProcessableBytes / params.minBytesForIntMs) * params.minBytesForIntMs;
        }
    } else if (params.alignedCachedSize >= params.minBytesForIntMs) {
        alignedProcessableBytes = params.minBytesForIntMs;
        AUDIO_DEBUG_LOG("Process: output capacity small, processing min unit and caching excess");
    } else {
        alignedProcessableBytes = 0;
    }

    if (alignedProcessableBytes == 0) {
        AUDIO_DEBUG_LOG("Process: insufficient data, maxProcessableBytes=%{public}d, minBytesForIntMs=%{public}d",
            maxProcessableBytes, params.minBytesForIntMs);
        shouldStopProcessing = true;
    }
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::WrapAndConvertInputData(
    uint32_t processableSize, AudioSuitePcmBuffer* outConvertedBuffer)
{
    AudioSuitePcmBuffer wrappedInput;
    int32_t wrapResult = WrapInputData(
        cachedInputData_.data(),
        processableSize,
        inputFinished_,
        wrappedInput);
    CHECK_AND_RETURN_RET_LOG(wrapResult == 0, ERR_OPERATION_FAILED, "WrapInputData failed: %{public}d", wrapResult);

    AudioSuitePcmBuffer* tempBuffer = DoConvert(&wrappedInput);
    CHECK_AND_RETURN_RET_LOG(tempBuffer != nullptr, ERR_OPERATION_FAILED, "DoConvert failed: null output");
    *outConvertedBuffer = *tempBuffer;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::CheckAndProcessInputData(
    const InputProcessParams& params, uint32_t& minBytesForIntMs, uint32_t& alignedCachedSize,
    uint32_t& alignedProcessableBytes, bool& shouldStopProcessing)
{
    int32_t ret = CalculateMinBytesForIntMsInternal(
        params.inputBytesPerFrame, minBytesForIntMs, alignedCachedSize, shouldStopProcessing, params.outputCapacity);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to calculate minimum number of bytes to process");
    if (shouldStopProcessing) {
        return SUCCESS;
    }

    ProcessableBytesCalculationParams processableParams = {
        params.inputBytesPerFrame, params.maxOutputFrames,
        params.outputBytesPerFrame, minBytesForIntMs, alignedCachedSize
    };
    ret = CalculateProcessableBytesInternal(processableParams, alignedProcessableBytes, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Insufficient output capacity");

    return SUCCESS;
}

int32_t AudioFormatConverterImpl::InitializeProcessingContext(void* audioData, uint32_t outputCapacity,
    uint32_t inputBytesPerFrame, ProcessingContext& context, bool& shouldStopProcessing)
{
    context.outputBufferPtr = static_cast<uint8_t*>(audioData);

    AUDIO_DEBUG_LOG("Process: processed block, outputCapacity=%{public}d", outputCapacity);

    uint32_t cachedSize = static_cast<uint32_t>(cachedInputData_.size());
    uint32_t tempMinBytesForIntMs = CalculateMinBytesForIntMs(inputBytesPerFrame);
    CHECK_AND_RETURN_RET_LOG(tempMinBytesForIntMs != 0,
        ERR_INVALID_PARAM,
        "The minimum processing unit for input and output formats cannot be 0.");
    uint32_t tempAlignedCachedSize = (cachedSize / tempMinBytesForIntMs) * tempMinBytesForIntMs;

    if (tempAlignedCachedSize == 0 && !inputFinished_) {
        int32_t ret = ReadAndCacheInputData(&shouldStopProcessing);
        if (ret != 0) {
            return ret;
        }
        if (shouldStopProcessing) {
            return SUCCESS;
        }
    }
    shouldStopProcessing = false;

    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ValidateOutputCapacity(
    AudioSuitePcmBuffer& convertedBuffer, uint32_t remainingCapacity, uint32_t* framesToWrite)
{
    uint32_t availableFrames = convertedBuffer.GetFrameLen();
    uint32_t outputSampleSize = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat);
    uint32_t outputBytesPerFrame = outputFormat_.channelCount * outputSampleSize;
    CHECK_AND_RETURN_RET_LOG(
        outputBytesPerFrame != 0, ERR_INVALID_PARAM, "The output format frame length cannot be 0.");
    uint32_t maxFramesByCapacity = remainingCapacity / outputBytesPerFrame;

    *framesToWrite = (availableFrames < maxFramesByCapacity) ? availableFrames : maxFramesByCapacity;

    CHECK_AND_RETURN_RET_LOG(!(*framesToWrite == 0 && availableFrames > 0),
        ERR_BUFFER_TOO_SMALL,
        "Process failed: output buffer too small for one frame, required=%{public}d, remaining=%{public}d",
        outputBytesPerFrame,
        remainingCapacity);

    return SUCCESS;
}

void AudioFormatConverterImpl::UpdateProcessingState(
    AudioSuitePcmBuffer& convertedBuffer,
    uint32_t processableSize, ProcessingContext& context, uint32_t actualFramesWritten)
{
    uint32_t outputSampleSize = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat);
    uint32_t actualOutputDataSize = actualFramesWritten * outputFormat_.channelCount * outputSampleSize;
    context.totalOutputSize += actualOutputDataSize;
    context.outputBufferPtr += actualOutputDataSize;
    context.remainingCapacity -= actualOutputDataSize;
    cachedInputData_.erase(cachedInputData_.begin(), cachedInputData_.begin() + processableSize);

    AUDIO_DEBUG_LOG("Process: processed block, outputBytes=%{public}d, totalOutput=%{public}d, remaining=%{public}d",
        actualOutputDataSize, context.totalOutputSize, context.remainingCapacity);
}

int32_t AudioFormatConverterImpl::ProcessSingleBlock(
    const InputProcessParams& params, ProcessingContext& context, uint32_t inputBytesPerFrame,
    uint32_t outputBytesPerFrame, bool& shouldStopProcessing)
{
    shouldStopProcessing = false;
    if (context.remainingCapacity < outputBytesPerFrame) {
        shouldStopProcessing = true;
        return SUCCESS;
    }

    uint32_t minBytesForIntMs = 0;
    uint32_t alignedCachedSize = 0;
    uint32_t alignedProcessableBytes = 0;
    CHECK_AND_RETURN_RET_LOG(
        outputBytesPerFrame != 0, ERR_INVALID_PARAM, "The output format frame length cannot be 0.");
    uint32_t currentMaxOutputFrames = context.remainingCapacity / outputBytesPerFrame;

    InputProcessParams inputParams = {
        inputBytesPerFrame, currentMaxOutputFrames, outputBytesPerFrame, context.remainingCapacity};
    int32_t ret = CheckAndProcessInputData(
        inputParams, minBytesForIntMs, alignedCachedSize, alignedProcessableBytes, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Input data processing failed");
    if (shouldStopProcessing) {
        return SUCCESS;
    }

    uint32_t processableSize = alignedProcessableBytes;

    AudioSuitePcmBuffer convertedBuffer;
    ret = WrapAndConvertInputData(processableSize, &convertedBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Data encapsulation and transformation process failed");

    uint32_t framesToWrite = 0;
    ret = ValidateOutputCapacity(convertedBuffer, context.remainingCapacity, &framesToWrite);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Output capacity validity verification failed.");

    ret = CopyOutputData(&convertedBuffer, context.outputBufferPtr, framesToWrite, &framesToWrite);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Data copy to output buffer failed.");

    UpdateProcessingState(convertedBuffer, processableSize, context, framesToWrite);
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessCachedOutputData(void* audioData, uint32_t outputCapacity,
    uint32_t outputBytesPerFrame, ProcessingContext& context)
{
    if (cachedOutputData_.empty()) {
        return SUCCESS;
    }

    uint32_t cachedSize = static_cast<uint32_t>(cachedOutputData_.size());
    uint32_t bytesToCopy = (cachedSize < static_cast<uint32_t>(outputCapacity)) ? cachedSize : outputCapacity;

    if (bytesToCopy < static_cast<uint32_t>(outputBytesPerFrame)) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(
        outputBytesPerFrame != 0, ERR_INVALID_PARAM, "The output format frame length cannot be 0.");
    uint32_t framesToCopy = bytesToCopy / outputBytesPerFrame;
    bytesToCopy = framesToCopy * outputBytesPerFrame;

    errno_t copyRet = memcpy_s(audioData, bytesToCopy, cachedOutputData_.data(), bytesToCopy);
    CHECK_AND_RETURN_RET_LOG(
        copyRet == 0, ERR_OPERATION_FAILED, "ProcessCachedOutputData failed: memcpy_s for cached output data failed");

    context.totalOutputSize = bytesToCopy;
    context.outputBufferPtr += bytesToCopy;
    context.remainingCapacity = outputCapacity - bytesToCopy;

    if (bytesToCopy < cachedSize) {
        cachedOutputData_.erase(cachedOutputData_.begin(), cachedOutputData_.begin() + bytesToCopy);
    } else {
        cachedOutputData_.clear();
    }

    AUDIO_DEBUG_LOG(
        "ProcessCachedOutputData: returned %{public}u bytes from cached output data, remaining cached: %{public}zu",
        bytesToCopy,
        cachedOutputData_.size());
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessDataBlocks(ProcessingContext& context, uint32_t inputBytesPerFrame,
    uint32_t outputBytesPerFrame)
{
    int32_t ret = 0;
    CHECK_AND_RETURN_RET_LOG(
        outputBytesPerFrame != 0, ERR_INVALID_PARAM, "The output format frame length cannot be 0.");
    uint32_t maxOutputFrames = context.remainingCapacity / outputBytesPerFrame;
    while (maxOutputFrames > 0) {
        bool shouldStopProcessing = false;
        InputProcessParams inputParams = {inputBytesPerFrame,
            maxOutputFrames,
            outputBytesPerFrame,
            context.remainingCapacity};

        ret = ProcessSingleBlock(inputParams, context, inputBytesPerFrame, outputBytesPerFrame, shouldStopProcessing);
        if (ret != 0 || shouldStopProcessing) {
            break;
        }
        maxOutputFrames = context.remainingCapacity / outputBytesPerFrame;
    }
    return ret;
}

int32_t AudioFormatConverterImpl::ProcessConversion(void* audioData, uint32_t outputCapacity,
    uint32_t* outputSize, uint32_t outputBytesPerFrame, uint32_t maxOutputFrames)
{
    uint32_t inputSampleSize = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat);
    uint32_t inputBytesPerFrame = inputSampleSize * inputFormat_.channelCount;

    CHECK_AND_RETURN_RET_LOG(inputBytesPerFrame != 0 && outputBytesPerFrame != 0,
        ERR_AUDIO_SUITE_UNSUPPORTED_FORMAT,
        "Process failed: invalid input or output format.");
    ProcessingContext context;
    context.totalOutputSize = 0;
    context.outputBufferPtr = static_cast<uint8_t*>(audioData);
    context.remainingCapacity = outputCapacity;

    int32_t ret = ProcessCachedOutputData(audioData, outputCapacity, outputBytesPerFrame, context);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to process output buffer data.");

    if (context.remainingCapacity < static_cast<uint32_t>(outputBytesPerFrame)) {
        *outputSize = context.totalOutputSize;
        return SUCCESS;
    }

    bool shouldStopProcessing = false;
    ret = InitializeProcessingContext(
        context.outputBufferPtr, context.remainingCapacity, inputBytesPerFrame, context, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Input buffer initialization failed.");
    if (shouldStopProcessing) {
        *outputSize = context.totalOutputSize;
        return SUCCESS;
    }

    ret = ProcessDataBlocks(context, inputBytesPerFrame, outputBytesPerFrame);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to process data in the input buffer.");
    *outputSize = context.totalOutputSize;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::Process(
    void* outputData, uint32_t outputCapacity, uint32_t* outputSize)
{
    CHECK_AND_RETURN_RET_LOG(
        outputData != nullptr && outputSize != nullptr, ERR_INVALID_PARAM, "Process: Invalid parameters.");

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_ILLEGAL_STATE, "Process: converter already destroyed");
    *outputSize = 0;

    CHECK_AND_RETURN_RET_LOG(formatConversion_ != nullptr, ERR_ILLEGAL_STATE, "Process: Converter not initialized");

    uint32_t outputSampleSize = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat);
    uint32_t outputBytesPerFrame = outputFormat_.channelCount * outputSampleSize;

    CHECK_AND_RETURN_RET_LOG(outputBytesPerFrame != 0, ERR_INVALID_PARAM, "Process: Invalid output format");

    CHECK_AND_RETURN_RET_LOG(
        outputCapacity >= outputBytesPerFrame, ERR_BUFFER_TOO_SMALL, "Process: Output buffer too small");
    uint32_t maxOutputFrames = outputCapacity / outputBytesPerFrame;

    int32_t ret = ProcessConversion(outputData, outputCapacity, outputSize, outputBytesPerFrame, maxOutputFrames);
    
    return ret;
}

AudioConvertResult AudioFormatConverterImpl::ValidateBasicParams(const uint8_t* inData, uint32_t inBytes)
{
    AudioConvertResult result = {nullptr, 0, ERR_ILLEGAL_STATE};
    CHECK_AND_RETURN_RET_LOG(!destroyed_, result, "Process: converter already destroyed");

    result = {nullptr, 0, ERR_INVALID_PARAM};
    CHECK_AND_RETURN_RET_LOG(inData != nullptr, result, "Process: inData is nullptr");

    CHECK_AND_RETURN_RET_LOG(inBytes > 0, result, "Process: inBytes is 0");

    CHECK_AND_RETURN_RET_LOG(inputFormat_.channelCount == LAYOUT_TO_CHANNEL.at(inputFormat_.channelLayout),
        result, "Process: input channelCount and channelLayout do not match");

    CHECK_AND_RETURN_RET_LOG(outputFormat_.channelCount == LAYOUT_TO_CHANNEL.at(outputFormat_.channelLayout),
        result, "Process: output channelCount and channelLayout do not match");

    return {nullptr, 0, SUCCESS};
}
 
AudioConvertResult AudioFormatConverterImpl::ValidateSampleRateParams(uint32_t inBytes)
{
    AudioConvertResult result = {nullptr, 0, ERR_INVALID_PARAM};
    uint32_t inputSampleRate = static_cast<uint32_t>(inputFormat_.sampleRate);
    uint32_t inputSampleSize = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat);
    uint32_t inputBytesPerFrame = inputSampleSize * inputFormat_.channelCount;

    if (inputFormat_.sampleRate == AudioSamplingRate::SAMPLE_RATE_11025 ||
        outputFormat_.sampleRate == AudioSamplingRate::SAMPLE_RATE_11025) {
        // 11025Hz: must be exactly 40ms
        uint32_t requiredBytesPerCall =
            (REQUIRED_MS_FOR_11025HZ * inputSampleRate * inputBytesPerFrame) / MILLISECONDS_PER_SECOND;
        CHECK_AND_RETURN_RET_LOG(requiredBytesPerCall != 0, result,
            "Process: Failed to calculate required bytes per call");
        CHECK_AND_RETURN_RET_LOG(inBytes == requiredBytesPerCall, result,
            "Process: inBytes(%{public}u) must be exactly 40ms data (%{public}u bytes) for 11025Hz",
            inBytes, requiredBytesPerCall);
    } else {
        // Other sample rates: max 60ms, must be multiple of 20ms
        uint32_t minBytesPerCall = (MIN_DURATION_MS * inputSampleRate * inputBytesPerFrame) / MILLISECONDS_PER_SECOND;
        uint32_t maxBytesPerCall = (MAX_DURATION_MS * inputSampleRate * inputBytesPerFrame) / MILLISECONDS_PER_SECOND;
        
        CHECK_AND_RETURN_RET_LOG(minBytesPerCall != 0, result,
            "Process: Failed to calculate minimum bytes per call");
        CHECK_AND_RETURN_RET_LOG(inBytes <= maxBytesPerCall, result,
            "Process: inBytes(%{public}u) exceeds 60ms limit (%{public}u bytes)",
            inBytes, maxBytesPerCall);
        CHECK_AND_RETURN_RET_LOG(inBytes % minBytesPerCall == 0, result,
            "Process: inBytes(%{public}u) must be multiple of 20ms data (%{public}u bytes)",
            inBytes, minBytesPerCall);
    }
    return {nullptr, 0, SUCCESS};
}
 
AudioConvertResult AudioFormatConverterImpl::ExecuteConversion(const uint8_t* inData, uint32_t inputBytes)
{
    AudioConvertResult result = {nullptr, 0, ERR_NO_MEMORY};
    CHECK_AND_RETURN_RET_LOG(rawFormatConversion_ != nullptr, result, "Process: failed to create rawFormatConversion");

    int32_t ret = rawFormatConversion_->Process(inData, inputBytes, outputDataVector_);
    result = {nullptr, 0, ERR_OPERATION_FAILED};
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, result, "Process: conversion failed");

    CHECK_AND_RETURN_RET_LOG(!outputDataVector_.empty(), result, "Process: output vector is empty");

    result.outData = outputDataVector_.data();
    result.outByteCount = static_cast<uint32_t>(outputDataVector_.size());
    result.errCode = SUCCESS;

    AUDIO_DEBUG_LOG("Process: outputDataSize=%{public}zu, outByteCount=%{public}u",
        outputDataVector_.size(), result.outByteCount);

    return result;
}
 
AudioConvertResult AudioFormatConverterImpl::SyncProcess(const uint8_t* inData, uint32_t inBytes)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Step 1: Validate basic parameters
    AudioConvertResult result = ValidateBasicParams(inData, inBytes);
    CHECK_AND_RETURN_RET_LOG(result.errCode == SUCCESS, result, "Process: basic params validation failed");

    // Step 2: Validate sample rate specific parameters
    result = ValidateSampleRateParams(inBytes);
    CHECK_AND_RETURN_RET_LOG(result.errCode == SUCCESS, result, "Process: sample rate params validation failed");

    // Step 3: Execute conversion
    return ExecuteConversion(inData, inBytes);
}

int32_t AudioFormatConverterImpl::WrapInputData(
    const void* inputData, uint32_t inputSize, bool finished,
    AudioSuitePcmBuffer& buffer)
{
    if (finished && inputSize == 0) {
        return SUCCESS;
    }

    uint32_t sampleSize = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat);
    uint32_t bytesPerFrame = inputFormat_.channelCount * sampleSize;

    CHECK_AND_RETURN_RET_LOG(bytesPerFrame != 0 && inputFormat_.sampleRate != 0,
        ERR_INVALID_PARAM,
        "WrapInputData failed: invalid format, bytesPerFrame=%{public}d, sampleRate=%{public}d",
        bytesPerFrame,
        inputFormat_.sampleRate);

    int32_t dataDurationMs = static_cast<int32_t>(std::round(
        static_cast<double>(inputSize) * 1000.0 / (static_cast<double>(inputFormat_.sampleRate) * bytesPerFrame)));
    int32_t result = buffer.ResizePcmBuffer(inputFormat_, dataDurationMs);
    CHECK_AND_RETURN_RET_LOG(result == 0, ERR_MEMORY_ALLOC_FAILED, "WrapInputData: Memory allocation failed.");

    CHECK_AND_RETURN_RET_LOG(buffer.GetPcmData() != nullptr && inputData != nullptr,
        ERR_INVALID_PARAM,
        "The pointer to the data storage buffer is null.");
 
    uint32_t bufferSize = buffer.GetDataSize();
    CHECK_AND_RETURN_RET_LOG(
        bufferSize >= inputSize, ERR_OPERATION_FAILED, "WrapInputData: memcpy_s Insufficient target buffer size");
    errno_t ret = memcpy_s(buffer.GetPcmData(), bufferSize, inputData, inputSize);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "WrapInputData: memcpy_s Execution failed");

    return SUCCESS;
}

AudioSuitePcmBuffer* AudioFormatConverterImpl::DoConvert(AudioSuitePcmBuffer* inputBuffer)
{
    CHECK_AND_RETURN_RET_LOG(formatConversion_ != nullptr, nullptr, "DoConvert failed: formatConversion is null");
    AudioSuitePcmBuffer* outputBuffer = formatConversion_->Process(inputBuffer, outputFormat_);
    return outputBuffer;
}

int32_t AudioFormatConverterImpl::CopyOutputData(
    AudioSuitePcmBuffer* buffer, void* outputBuffer, uint32_t maxFrameSize, uint32_t* actualFrameSize)
{
    CHECK_AND_RETURN_RET_LOG(
        buffer != nullptr, ERR_INVALID_PARAM, "CopyOutputData: buffer pointer is nullptr.");
    uint32_t availableFrames = buffer->GetFrameLen();
    *actualFrameSize = (availableFrames < maxFrameSize)
                      ? availableFrames : maxFrameSize;
    CHECK_AND_RETURN_RET_LOG(
        outputBuffer != nullptr, ERR_INVALID_PARAM, "CopyOutputData: outputBuffer is nullptr but framesToWrite > 0");
    
    uint32_t outputSampleSize = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat);
    uint32_t outputBytes = *actualFrameSize * outputFormat_.channelCount * outputSampleSize;

    CHECK_AND_RETURN_RET_LOG(
        buffer->GetPcmData() != nullptr, ERR_INVALID_PARAM, "CopyOutputData: buffer->GetPcmData is nullptr");
    int32_t ret = memcpy_s(outputBuffer, outputBytes, buffer->GetPcmData(), outputBytes);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "memcpy_s failed: memcpy_s for cached data failed");

    if (*actualFrameSize < availableFrames) {
        uint32_t totalBytes = availableFrames * outputFormat_.channelCount * outputSampleSize;
        uint32_t copiedBytes = outputBytes;
        uint32_t remainingBytes = totalBytes - copiedBytes;
        uint32_t existingSize = static_cast<uint32_t>(cachedOutputData_.size());

        cachedOutputData_.resize(existingSize + remainingBytes);
        errno_t ret = memcpy_s(cachedOutputData_.data() + existingSize, remainingBytes,
            static_cast<const uint8_t*>(buffer->GetPcmData()) + copiedBytes, remainingBytes);
        CHECK_AND_RETURN_RET_LOG(
            ret == 0, ERR_OPERATION_FAILED, "memcpy_s failed: memcpy_s for cachedOutputData failed");

        AUDIO_ERR_LOG("CopyOutputData: cached %{public}u bytes of uncopied output data", remainingBytes);
    }
    return SUCCESS;
}

void AudioFormatConverterImpl::Destroy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(!destroyed_, "Destroy: already destroyed, skipping");
    if (formatConversion_ != nullptr) {
        formatConversion_->Reset();
        formatConversion_ = nullptr;
    }
    CHECK_AND_RETURN_LOG(rawFormatConversion_ != nullptr, "Destroy: Synchronous format conversion object is nullptr");
    rawFormatConversion_->Reset();
    rawFormatConversion_ = nullptr;

    inputFinished_ = false;
    cachedInputData_.clear();
    cachedOutputData_.clear();
    outputDataVector_.clear();
    dataCallback_ = nullptr;
    destroyed_ = true;

    AUDIO_INFO_LOG("Destroy: success");
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS