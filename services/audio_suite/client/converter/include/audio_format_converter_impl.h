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

#ifndef AUDIO_FORMAT_CONVERTER_IMPL_H
#define AUDIO_FORMAT_CONVERTER_IMPL_H

#include "audio_suite_pcm_buffer.h"
#include "audio_suite_base.h"
#include "audio_suite_format_conversion.h"
#include "audio_format_converter.h"
#include "audio_suite_raw_format_conversion.h"
#include "audio_errors.h"
#include <functional>
#include <memory>
#include <vector>
#include <mutex>
#include <cstdint>
#include <unordered_set>
#include <unordered_map>

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// Format converter configuration
struct FormatConverterConfig {
    PcmBufferFormat inputFormat;
    PcmBufferFormat outputFormat;
};

// Handleable byte calculation parameters
struct ProcessableBytesCalculationParams {
    uint32_t inputBytesPerFrame;
    uint32_t maxOutputFrames;
    uint32_t outputBytesPerFrame;
    uint32_t minBytesForIntMs;
    uint32_t alignedCachedSize;
};

// Enter processing parameters
struct InputProcessParams {
    uint32_t inputBytesPerFrame;
    uint32_t maxOutputFrames;
    uint32_t outputBytesPerFrame;
    uint32_t outputCapacity;
};

// Processing context
struct ProcessingContext {
    uint32_t totalOutputSize;
    uint8_t* outputBufferPtr;
    uint32_t remainingCapacity;
};

class AudioFormatConverterImpl : public AudioFormatConverter {
public:
    AudioFormatConverterImpl();
    ~AudioFormatConverterImpl() override;

    int32_t SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback) override;

    int32_t Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize) override;

    AudioConvertResult SyncProcess(const uint8_t* inData, uint32_t inBytes) override;

    void Destroy() override;

    PcmBufferFormat inputFormat_;
    PcmBufferFormat outputFormat_;
    std::unique_ptr<AudioSuiteFormatConversion> formatConversion_ = nullptr;
    std::unique_ptr<AudioSuiteRawFormatConversion> rawFormatConversion_ = nullptr;
    bool destroyed_ = false;
    std::mutex mutex_;
    std::vector<uint8_t> outputDataVector_;

private:
    // Internal implementation method
    int32_t WrapInputData(const void* inputData, uint32_t inputSize, bool finished,
                          AudioSuitePcmBuffer& buffer);
    AudioSuitePcmBuffer* DoConvert(AudioSuitePcmBuffer* inputBuffer);
    int32_t CopyOutputData(AudioSuitePcmBuffer* buffer, void* outputBuffer,
                        uint32_t maxFrameSize, uint32_t* actualFrameSize);
    
    // Calculation related methods
    uint32_t CalculateMinBytesForIntMs(uint32_t inputBytesPerFrame) const;
    int32_t CalculateMinBytesForIntMsInternal(uint32_t inputBytesPerFrame,
        uint32_t& minBytesForIntMs, uint32_t& alignedCachedSize, bool& shouldStopProcessing, uint32_t outputCapacity);
    int32_t CalculateProcessableBytesInternal(const ProcessableBytesCalculationParams& params,
        uint32_t& alignedProcessableBytes, bool& shouldStopProcessing);
    
    // Data reading and caching
    int32_t ReadAndCacheInputData(bool* shouldStopProcessing);
    
    // Processing method
    int32_t WrapAndConvertInputData(uint32_t processableSize, AudioSuitePcmBuffer* outConvertedBuffer);
    int32_t CheckAndProcessInputData(const InputProcessParams& params, uint32_t& minBytesForIntMs,
        uint32_t& alignedCachedSize, uint32_t& alignedProcessableBytes, bool& shouldStopProcessing);
    int32_t InitializeProcessingContext(void* audioData, uint32_t outputCapacity,
        uint32_t inputBytesPerFrame, ProcessingContext& context, bool& shouldStopProcessing);
    int32_t ValidateOutputCapacity(AudioSuitePcmBuffer& convertedBuffer,
        uint32_t remainingCapacity, uint32_t* framesToWrite);
    void UpdateProcessingState(AudioSuitePcmBuffer& convertedBuffer,
        uint32_t processableSize, ProcessingContext& context, uint32_t actualFramesWritten);
    int32_t ProcessSingleBlock(const InputProcessParams& params,
        ProcessingContext& context, uint32_t inputBytesPerFrame,
        uint32_t outputBytesPerFrame, bool& shouldStopProcessing);
    int32_t ProcessCachedOutputData(void* audioData, uint32_t outputCapacity,
        uint32_t outputBytesPerFrame, ProcessingContext& context);
    int32_t ProcessDataBlocks(ProcessingContext& context, uint32_t inputBytesPerFrame,
        uint32_t outputBytesPerFrame);
    int32_t ProcessConversion(void* audioData, uint32_t outputCapacity,
        uint32_t* outputSize, uint32_t outputBytesPerFrame, uint32_t maxOutputFrames);
    AudioConvertResult ValidateBasicParams(const uint8_t* inData, uint32_t inBytes);
    AudioConvertResult ValidateSampleRateParams(uint32_t inBytes);
    AudioConvertResult ExecuteConversion(const uint8_t* inData, uint32_t inputBytes);
    
    std::shared_ptr<FormatConverterDataCallback> dataCallback_;
    
    std::vector<uint8_t> cachedInputData_;
    std::vector<uint8_t> cachedOutputData_;
    bool inputFinished_ = false;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FORMAT_CONVERTER_IMPL_H