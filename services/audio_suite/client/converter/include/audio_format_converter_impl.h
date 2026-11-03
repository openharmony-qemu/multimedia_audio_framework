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

class AudioFormatConverterImpl : public AudioFormatConverter {
public:
    AudioFormatConverterImpl();
    ~AudioFormatConverterImpl() override;

    int32_t Init(const PcmBufferFormat &inputFormat, const PcmBufferFormat &outputFormat,
        const uint32_t resampleQuality, const bool enableDataAlignment);

    int32_t SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback) override;
    int32_t Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize) override;

    size_t GetDelayDataSize() override;
    void Destroy() override;

private:
    // Internal implementation method
    int32_t CopyOutputData(
        AudioSuitePcmBuffer& buffer, void* outputBuffer, uint32_t &totalOutputSize, uint32_t &remainingCapacity);

    // Calculation related methods
    uint32_t CalculateMinBytesForIntMs() const;
    int32_t CalculateMinBytesForIntMsInternal(uint32_t& alignedCachedSize, bool& shouldStopProcessing);
    int32_t CalculateProcessableBytesInternal(const ProcessableBytesCalculationParams& params,
        uint32_t& alignedProcessableBytes, bool& shouldStopProcessing);
    uint32_t CalculatePaddedOutputFrames(uint32_t paddedInputBytes) const;
    uint32_t CalculatePaddingInputLen(uint32_t inputSize) const;

    // Data reading and caching
    int32_t ReadAndCacheInputData(bool* shouldStopProcessing);

    // Processing method
    int32_t WrapAndConvertInputData(uint32_t processableSize, AudioSuitePcmBuffer* outConvertedBuffer);
    int32_t CheckAndProcessInputData(const InputProcessParams& params, uint32_t& alignedCachedSize,
        uint32_t& alignedProcessableBytes, bool& shouldStopProcessing);
    int32_t InitializeProcessingContext(bool& shouldStopProcessing);
    int32_t ProcessSingleBlock(void* audioData,
        uint32_t &totalOutputSize, uint32_t &remainingCapacit, bool& shouldStopProcessing);
    int32_t ProcessCachedOutputData(void* audioData, uint32_t outputCapacity, uint32_t &totalOutputSize);
    int32_t ProcessDataBlocks(void* audioData, uint32_t &totalOutputSize, uint32_t &remainingCapacit);
    int32_t ProcessConversion(void* audioData, uint32_t outputCapacity, uint32_t* outputSize);

    std::mutex mutex_;
    // for callback
    std::shared_ptr<FormatConverterDataCallback> dataCallback_;

    // for data
    std::vector<uint8_t> cachedInputData_;
    std::vector<uint8_t> cachedOutputData_;
    std::vector<uint8_t> outputDataVector_;
    bool inputFinished_ = false;
    bool destroyed_ = false;

    // for convert
    PcmBufferFormat inputFormat_;
    PcmBufferFormat outputFormat_;
    AudioSuiteFormatConversion formatConversion_;
    uint32_t processMinMs_ = 0;
    uint32_t processMinInBytes_ = 0;
    // input Bit depth byptes * channels
    uint32_t inSingleFrameBytes_ = 0;
    // output Bit depth byptes * channels
    uint32_t outSingleFrameBytes_ = 0;

    // for data alignment
    bool enableDataAlignment_ = false;
    // for delay frames
    bool hasDelay_ = false;
    bool isFirstProcess_ = true;
    bool isPadded_ = false;
    uint32_t delayFrames_ = 0;
    // for padding removal
    uint32_t paddedInputBytes_ = 0;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FORMAT_CONVERTER_IMPL_H