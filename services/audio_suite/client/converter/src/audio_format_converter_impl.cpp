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
constexpr uint32_t RESAMPLE_DELAY_MAX_MS = 2;

AudioFormatConverterImpl::AudioFormatConverterImpl()
{
}

AudioFormatConverterImpl::~AudioFormatConverterImpl()
{
}

std::shared_ptr<AudioFormatConverter> AudioFormatConverter::Create(
    const AudioFormat &inputFormat, const AudioFormat &outputFormat,
    const uint32_t resampleQuality, bool enableDataAlignment)
{
    auto inLayoutIt = LAYOUT_TO_CHANNEL.find(inputFormat.audioChannelInfo.channelLayout);
    CHECK_AND_RETURN_RET_LOG(inLayoutIt != LAYOUT_TO_CHANNEL.end(), nullptr,
        "Create: unsupported input channel layout");
    auto outLayoutIt = LAYOUT_TO_CHANNEL.find(outputFormat.audioChannelInfo.channelLayout);
    CHECK_AND_RETURN_RET_LOG(outLayoutIt != LAYOUT_TO_CHANNEL.end(), nullptr,
        "Create: unsupported output channel layout");

    uint32_t inChannelCount = (inputFormat.audioChannelInfo.numChannels != 0)
        ? inputFormat.audioChannelInfo.numChannels
        : static_cast<uint32_t>(inLayoutIt->second);
    uint32_t outChannelCount = (outputFormat.audioChannelInfo.numChannels != 0)
        ? outputFormat.audioChannelInfo.numChannels
        : static_cast<uint32_t>(outLayoutIt->second);

    PcmBufferFormat inputPcmFormat(
        inputFormat.rate, inChannelCount,
        inputFormat.audioChannelInfo.channelLayout, inputFormat.format);
    PcmBufferFormat outputPcmFormat(
        outputFormat.rate, outChannelCount,
        outputFormat.audioChannelInfo.channelLayout, outputFormat.format);

    auto audioFormatConverter = std::make_shared<AudioFormatConverterImpl>();
    int32_t ret = audioFormatConverter->Init(inputPcmFormat, outputPcmFormat, resampleQuality, enableDataAlignment);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "Create: rawFormatConversion init failed");

    return audioFormatConverter;
}

int32_t AudioFormatConverterImpl::Init(
    const PcmBufferFormat &inputFormat, const PcmBufferFormat &outputFormat, const uint32_t resampleQuality,
    bool enableDataAlignment)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Format validation: Sampling rate
    if (SUPPORTED_SAMPLE_RATES.find(inputFormat.sampleRate) == SUPPORTED_SAMPLE_RATES.end() ||
        SUPPORTED_SAMPLE_RATES.find(outputFormat.sampleRate) == SUPPORTED_SAMPLE_RATES.end()) {
        AUDIO_ERR_LOG("Create: unsupported sampling rate");
        return ERR_INVALID_PARAM;
    }

    // Format verification: Audio channel layout
    if (LAYOUT_TO_CHANNEL.find(inputFormat.channelLayout) == LAYOUT_TO_CHANNEL.end() ||
        LAYOUT_TO_CHANNEL.find(outputFormat.channelLayout) == LAYOUT_TO_CHANNEL.end()) {
        AUDIO_ERR_LOG("Create: unsupported channel layout");
        return ERR_INVALID_PARAM;
    }

    // Format verification: channel count
    CHECK_AND_RETURN_RET_LOG(inputFormat.channelCount != 0 &&
        inputFormat.channelCount == LAYOUT_TO_CHANNEL.at(inputFormat.channelLayout),
        ERR_INVALID_PARAM, "Create: input channelCount/layout mismatch");
    CHECK_AND_RETURN_RET_LOG(outputFormat.channelCount != 0 &&
        outputFormat.channelCount == LAYOUT_TO_CHANNEL.at(outputFormat.channelLayout),
        ERR_INVALID_PARAM, "Create: output channelCount/layout mismatch");

    // Format validation: Sampling format
    if (SUPPORTED_SAMPLE_FORMATS.find(inputFormat.sampleFormat) == SUPPORTED_SAMPLE_FORMATS.end() ||
        SUPPORTED_SAMPLE_FORMATS.find(outputFormat.sampleFormat) == SUPPORTED_SAMPLE_FORMATS.end()) {
        AUDIO_ERR_LOG("Create: unsupported sample format");
        return ERR_INVALID_PARAM;
    }

    CHECK_AND_RETURN_RET_LOG(resampleQuality >= MIN_RESAMPLE_QUALITY && resampleQuality <= MAX_RESAMPLE_QUALITY,
        ERR_INVALID_PARAM, "Resampling level assignment error");
    
    AUDIO_INFO_LOG("Create: src=[%{public}u %{public}u], dst=[%{public}u %{public}u]",
        inputFormat.sampleRate, LAYOUT_TO_CHANNEL.at(inputFormat.channelLayout),
        outputFormat.sampleRate, LAYOUT_TO_CHANNEL.at(outputFormat.channelLayout));

    inputFormat_ = inputFormat;
    outputFormat_ = outputFormat;
    hasDelay_ = inputFormat_.sampleRate != outputFormat_.sampleRate;
    enableDataAlignment_ = enableDataAlignment;

    // Calculate the minimum duration for a single processing instance.
    uint32_t gcd =
        std::gcd(static_cast<uint32_t>(inputFormat_.sampleRate), static_cast<uint32_t>(outputFormat_.sampleRate));
    uint32_t gcdSecond = std::gcd(gcd, SECONDS_TO_MS);
    CHECK_AND_RETURN_RET_LOG(gcdSecond != 0, ERR_INVALID_PARAM, "The minimum processing time must greater zero");
    processMinMs_ = SECONDS_TO_MS / gcdSecond;
    inSingleFrameBytes_ = AudioSuiteUtil::GetSampleSize(inputFormat_.sampleFormat) * inputFormat_.channelCount;
    outSingleFrameBytes_ = AudioSuiteUtil::GetSampleSize(outputFormat_.sampleFormat) * outputFormat_.channelCount;
    processMinInBytes_ = CalculateMinBytesForIntMs();
    AUDIO_INFO_LOG("Create: processMinMs:%{public}u, inSingleFrameBytes:%{public}u, outSingleFrameBytes:%{public}u,"
        "processMinInBytes:%{public}u", processMinMs_, inSingleFrameBytes_, outSingleFrameBytes_, processMinInBytes_);
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_ILLEGAL_STATE, "SetInputCallback: converter already destroyed");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_CALLBACK_NOT_FOUND, "Callback not set");
    dataCallback_ = std::move(callback);
    return SUCCESS;
}

uint32_t AudioFormatConverterImpl::CalculateMinBytesForIntMs() const
{
    uint32_t inputBytesPerSec = static_cast<uint32_t>(inputFormat_.sampleRate) * inSingleFrameBytes_;
    return static_cast<uint32_t>(inputBytesPerSec * processMinMs_ / SECONDS_TO_MS);
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

int32_t AudioFormatConverterImpl::CalculateMinBytesForIntMsInternal(
    uint32_t& alignedCachedSize, bool& shouldStopProcessing)
{
    uint32_t cachedSize = static_cast<uint32_t>(cachedInputData_.size());
    CHECK_AND_RETURN_RET_LOG(processMinInBytes_ != 0, ERR_INVALID_PARAM, "Minimum process input byptes cannot be 0.");
    alignedCachedSize = (cachedSize / processMinInBytes_) * processMinInBytes_;
    
    if (alignedCachedSize == 0) {
        if (inputFinished_ && !isPadded_) {
            paddedInputBytes_ = CalculatePaddingInputLen(cachedSize);
            cachedInputData_.insert(cachedInputData_.end(), paddedInputBytes_, 0);
            alignedCachedSize = cachedInputData_.size() / processMinInBytes_ * processMinInBytes_;
            AUDIO_INFO_LOG("Process: padding %{public}u bytes for final data to meet int ms", paddedInputBytes_);
            isPadded_ = true;
            shouldStopProcessing = false;
        } else {
            AUDIO_DEBUG_LOG("Process: waiting for more data, cachedSize=%{public}d, processMinInBytes=%{public}d",
                cachedSize, processMinInBytes_);
            shouldStopProcessing = true;
            return SUCCESS;
        }
    }

    shouldStopProcessing = false;
    return SUCCESS;
}

uint32_t AudioFormatConverterImpl::CalculatePaddingInputLen(uint32_t inputSize) const
{
    if (!inputFinished_) {
        return 0;
    }

    if (inputSize == 0 && (!enableDataAlignment_ || !hasDelay_)) {
        return 0;
    }

    if (inputSize > processMinInBytes_) {
        return 0;
    }

    uint32_t paddingLen = processMinInBytes_ - inputSize;
    // In scenarios with latency, at least 2ms of data must be filled. Otherwise, the data will be incomplete.
    if (enableDataAlignment_ && hasDelay_) {
        uint32_t minPaddingLen = RESAMPLE_DELAY_MAX_MS * inSingleFrameBytes_ *
            static_cast<uint32_t>(inputFormat_.sampleRate) / MILLISECONDS_PER_SECOND;
        if (paddingLen < minPaddingLen) {
            uint32_t additionalPadding = minPaddingLen - paddingLen;
            uint32_t additionalUnits = (additionalPadding + processMinInBytes_ - 1) / processMinInBytes_;
            paddingLen += additionalUnits * processMinInBytes_;
        }
    }
    return paddingLen;
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

uint32_t AudioFormatConverterImpl::CalculatePaddedOutputFrames(uint32_t paddedInputBytes) const
{
    if (paddedInputBytes == 0) {
        return 0;
    }
    
    uint32_t paddedInputFrames = paddedInputBytes / inSingleFrameBytes_;
    double sampleRateRatio = static_cast<double>(outputFormat_.sampleRate) /
        static_cast<double>(inputFormat_.sampleRate);
    double estimatedOutput = std::floor(paddedInputFrames * sampleRateRatio);
    uint32_t paddedOutputFrames = (estimatedOutput >= static_cast<double>(delayFrames_)) ?
        static_cast<uint32_t>(estimatedOutput) - delayFrames_ : 0;
    AUDIO_INFO_LOG("paddedOutputFrames:%{public}u.", paddedOutputFrames);
    return paddedOutputFrames;
}

int32_t AudioFormatConverterImpl::WrapAndConvertInputData(
    uint32_t processableSize, AudioSuitePcmBuffer* outConvertedBuffer)
{
    if (inputFinished_ && processableSize == 0) {
        return SUCCESS;
    }

    AudioSuitePcmBuffer inputPcm;
    int32_t dataDurationMs = static_cast<int32_t>(std::round(
        static_cast<double>(processableSize) * 1000.0 /
        (static_cast<double>(inputFormat_.sampleRate) * inSingleFrameBytes_)));
    int32_t result = inputPcm.ResizePcmBuffer(inputFormat_, dataDurationMs);
    CHECK_AND_RETURN_RET_LOG(result == 0, ERR_MEMORY_ALLOC_FAILED, "Memory allocation failed.");

    CHECK_AND_RETURN_RET_LOG(inputPcm.GetPcmData() != nullptr && cachedInputData_.data() != nullptr,
        ERR_INVALID_PARAM,
        "The pointer to the data storage buffer is null.");
 
    uint32_t bufferSize = inputPcm.GetDataSize();
    CHECK_AND_RETURN_RET_LOG(
        bufferSize >= processableSize, ERR_OPERATION_FAILED, "memcpy_s Insufficient target buffer size");
    errno_t ret = memcpy_s(inputPcm.GetPcmData(), bufferSize, cachedInputData_.data(), processableSize);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "memcpy_s Execution failed");

    AudioSuitePcmBuffer* tempBuffer = formatConversion_.Process(&inputPcm, outputFormat_);
    CHECK_AND_RETURN_RET_LOG(tempBuffer != nullptr, ERR_OPERATION_FAILED, "DoConvert failed: null output");
    *outConvertedBuffer = *tempBuffer;

    cachedInputData_.erase(cachedInputData_.begin(), cachedInputData_.begin() + processableSize);
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::CheckAndProcessInputData(
    const InputProcessParams& params, uint32_t& alignedCachedSize,
    uint32_t& alignedProcessableBytes, bool& shouldStopProcessing)
{
    int32_t ret = CalculateMinBytesForIntMsInternal(alignedCachedSize, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to calculate minimum number of bytes to process");
    if (shouldStopProcessing) {
        return SUCCESS;
    }

    ProcessableBytesCalculationParams processableParams = {
        params.inputBytesPerFrame, params.maxOutputFrames,
        params.outputBytesPerFrame, processMinInBytes_, alignedCachedSize
    };
    ret = CalculateProcessableBytesInternal(processableParams, alignedProcessableBytes, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Insufficient output capacity");

    return SUCCESS;
}

int32_t AudioFormatConverterImpl::InitializeProcessingContext(bool& shouldStopProcessing)
{
    uint32_t cachedSize = static_cast<uint32_t>(cachedInputData_.size());
    CHECK_AND_RETURN_RET_LOG(processMinInBytes_ != 0, ERR_INVALID_PARAM, "Minimum process input byptes cannot be 0.");
    uint32_t tempAlignedCachedSize = (cachedSize / processMinInBytes_) * processMinInBytes_;

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

int32_t AudioFormatConverterImpl::ProcessSingleBlock(void* audioData,
    uint32_t &totalOutputSize, uint32_t &remainingCapacity, bool& shouldStopProcessing)
{
    shouldStopProcessing = false;
    if (remainingCapacity < outSingleFrameBytes_) {
        shouldStopProcessing = true;
        return SUCCESS;
    }

    uint32_t alignedCachedSize = 0;
    uint32_t alignedProcessableBytes = 0;
    CHECK_AND_RETURN_RET_LOG(outSingleFrameBytes_ != 0, ERR_INVALID_PARAM, "The output frame length cannot be 0.");
    uint32_t currentMaxOutputFrames = remainingCapacity / outSingleFrameBytes_;

    InputProcessParams inputParams = {
        inSingleFrameBytes_, currentMaxOutputFrames, outSingleFrameBytes_, remainingCapacity};
    int32_t ret = CheckAndProcessInputData(
        inputParams, alignedCachedSize, alignedProcessableBytes, shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Input data processing failed");
    if (shouldStopProcessing) {
        return SUCCESS;
    }

    AudioSuitePcmBuffer convertedBuffer;
    ret = WrapAndConvertInputData(alignedProcessableBytes, &convertedBuffer);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Data encapsulation and transformation process failed");

    ret = CopyOutputData(convertedBuffer, reinterpret_cast<uint8_t *>(audioData) + totalOutputSize,
        totalOutputSize, remainingCapacity);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Data copy to output buffer failed.");

    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessCachedOutputData(void* audioData, uint32_t outputCapacity,
    uint32_t &totalOutputSize)
{
    if (cachedOutputData_.empty()) {
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(outSingleFrameBytes_ != 0, ERR_INVALID_PARAM, "The output frame length cannot be 0.");
    uint32_t cachedSize = static_cast<uint32_t>(cachedOutputData_.size());
    uint32_t copySize = (outputCapacity / outSingleFrameBytes_) * outSingleFrameBytes_;
    copySize = copySize > cachedSize ? cachedSize : copySize;

    errno_t err = memcpy_s(audioData, outputCapacity, cachedOutputData_.data(), copySize);
    CHECK_AND_RETURN_RET_LOG(err == 0, ERR_OPERATION_FAILED, "memcpy_s failed, err:%{public}u", err);

    totalOutputSize += copySize;

    if (copySize < cachedSize) {
        cachedOutputData_.erase(cachedOutputData_.begin(), cachedOutputData_.begin() + copySize);
    } else {
        cachedOutputData_.clear();
    }

    AUDIO_DEBUG_LOG("copy %{public}u bytes, remaining size: %{public}zu", copySize, cachedOutputData_.size());
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::ProcessDataBlocks(
    void* audioData, uint32_t &totalOutputSize, uint32_t &remainingCapacity)
{
    int32_t ret = 0;
    CHECK_AND_RETURN_RET_LOG(outSingleFrameBytes_ != 0, ERR_INVALID_PARAM, "The output frame length cannot be 0.");
    uint32_t maxOutputFrames = remainingCapacity / outSingleFrameBytes_;
    while (maxOutputFrames > 0) {
        bool shouldStopProcessing = false;
        ret = ProcessSingleBlock(audioData, totalOutputSize, remainingCapacity, shouldStopProcessing);
        if (ret != 0 || shouldStopProcessing) {
            break;
        }
        maxOutputFrames = remainingCapacity / outSingleFrameBytes_;
    }
    return ret;
}

int32_t AudioFormatConverterImpl::ProcessConversion(void* audioData, uint32_t outputCapacity, uint32_t* outputSize)
{
    uint32_t totalOutputSize = 0;
    uint32_t remainingCapacity = outputCapacity;
    int32_t ret = ProcessCachedOutputData(audioData, outputCapacity, totalOutputSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to process output buffer data.");
    remainingCapacity -= totalOutputSize;

    if (remainingCapacity < static_cast<uint32_t>(outSingleFrameBytes_)) {
        *outputSize = totalOutputSize;
        return SUCCESS;
    }

    bool shouldStopProcessing = false;
    ret = InitializeProcessingContext(shouldStopProcessing);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Input buffer initialization failed.");
    if (shouldStopProcessing) {
        *outputSize = totalOutputSize;
        return SUCCESS;
    }

    ret = ProcessDataBlocks(audioData, totalOutputSize, remainingCapacity);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Failed to process data in the input buffer.");
    *outputSize = totalOutputSize;
    return SUCCESS;
}

int32_t AudioFormatConverterImpl::Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize)
{
    CHECK_AND_RETURN_RET_LOG(
        outputData != nullptr && outputSize != nullptr, ERR_INVALID_PARAM, "Process: Invalid parameters.");

    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_RET_LOG(!destroyed_, ERR_ILLEGAL_STATE, "Process: converter already destroyed");
    CHECK_AND_RETURN_RET_LOG(outputCapacity >= outSingleFrameBytes_, ERR_BUFFER_TOO_SMALL,
        "Output buffer too small, must greater %{public}u.", outSingleFrameBytes_);
    *outputSize = 0;
    return ProcessConversion(outputData, outputCapacity, outputSize);
}

int32_t AudioFormatConverterImpl::CopyOutputData(
    AudioSuitePcmBuffer& buffer, void* outputBuffer, uint32_t &totalOutputSize, uint32_t &remainingCapacity)
{
    CHECK_AND_RETURN_RET_LOG(outputBuffer != nullptr, ERR_INVALID_PARAM, "outputBuffer is nullptr");
    uint32_t delayFrames = 0;
    uint32_t delaySize = 0;
    if (isFirstProcess_ && hasDelay_) {
        delayFrames_ = formatConversion_.GetDelayFrames();
        if (enableDataAlignment_) {
            delayFrames = delayFrames_ > buffer.GetFrameLen() ? buffer.GetFrameLen() : delayFrames_;
            delaySize = delayFrames * outSingleFrameBytes_;
        }
        isFirstProcess_ = false;
    }

    uint32_t paddedOutputFrames = 0;
    if (enableDataAlignment_ && paddedInputBytes_ > 0 && cachedInputData_.empty()) {
        paddedOutputFrames = CalculatePaddedOutputFrames(paddedInputBytes_);
    }

    uint32_t availableFrames = buffer.GetFrameLen() - delayFrames;
    if (availableFrames > paddedOutputFrames) {
        availableFrames -= paddedOutputFrames;
    } else {
        availableFrames = 0;
    }
    
    CHECK_AND_RETURN_RET_LOG(outSingleFrameBytes_ != 0, ERR_INVALID_PARAM, "The output frame length cannot be 0.");
    uint32_t maxFramesByCapacity = remainingCapacity / outSingleFrameBytes_;
    uint32_t framesToWrite = (availableFrames < maxFramesByCapacity) ? availableFrames : maxFramesByCapacity;
    uint32_t outputBytes = framesToWrite * outSingleFrameBytes_;

    CHECK_AND_RETURN_RET_LOG(buffer.GetPcmData() != nullptr, ERR_INVALID_PARAM, "pcm data is nullptr");
    int32_t ret = memcpy_s(outputBuffer, remainingCapacity, buffer.GetPcmData() + delaySize, outputBytes);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "memcpy_s failed: memcpy_s for cached data failed");

    if (framesToWrite < availableFrames) {
        uint32_t remainingBytes = availableFrames * outSingleFrameBytes_ - outputBytes;
        uint32_t existingSize = static_cast<uint32_t>(cachedOutputData_.size());

        cachedOutputData_.resize(existingSize + remainingBytes);
        errno_t ret = memcpy_s(cachedOutputData_.data() + existingSize, remainingBytes,
            buffer.GetPcmData() + delaySize + outputBytes, remainingBytes);
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_OPERATION_FAILED, "memcpy_s fail, ret:%{public}d.", ret);

        AUDIO_DEBUG_LOG("CopyOutputData: cached %{public}u bytes of uncopied output data", remainingBytes);
    }

    totalOutputSize += outputBytes;
    remainingCapacity -= outputBytes;
    return SUCCESS;
}

size_t AudioFormatConverterImpl::GetDelayDataSize()
{
    return delayFrames_ * outSingleFrameBytes_;
}

void AudioFormatConverterImpl::Destroy()
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(!destroyed_, "Destroy: already destroyed, skipping");
    formatConversion_.Reset();

    inputFinished_ = false;
    cachedInputData_.clear();
    cachedOutputData_.clear();
    outputDataVector_.clear();
    dataCallback_ = nullptr;
    hasDelay_ = false;
    isFirstProcess_ = true;
    isPadded_ = false;
    destroyed_ = true;
    paddedInputBytes_ = 0;
    delayFrames_ = 0;
    enableDataAlignment_ = false;
    AUDIO_INFO_LOG("Destroy: success");
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS