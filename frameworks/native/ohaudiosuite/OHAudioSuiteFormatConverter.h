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

#ifndef OH_AUDIO_SUITE_FORMAT_CONVERTER_H
#define OH_AUDIO_SUITE_FORMAT_CONVERTER_H

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <numeric>
#include "native_audio_suite_engine.h"
#include "audio_suite_manager.h"
#include "OHAudioSuiteNodeBuilder.h"
#include "audio_suite_format_conversion.h"
#include "audio_suite_pcm_buffer.h"
#include "native_audio_converter.h"

namespace OHOS {
namespace AudioStandard {

struct ProcessableBytesCalculationParams {
    int32_t inputBytesPerFrame;
    int32_t maxOutputFrames;
    int32_t outputBytesPerFrame;
    int32_t minBytesForIntMs;
    int32_t alignedCachedSize;
};

struct OutputCopyParams {
    void* audioData;
    uint32_t outputCapacity;
    int32_t* outputSize;
    int32_t processableSize;
    int32_t framesToProcess;
};

struct InputProcessParams {
    int32_t inputBytesPerFrame;
    int32_t maxOutputFrames;
    int32_t outputBytesPerFrame;
    int32_t outputCapacity;
};

struct ProcessingContext {
    uint32_t totalOutputSize;
    uint8_t* outputBufferPtr;
    uint32_t remainingCapacity;
    std::vector<uint8_t> backupInputData;
};
class OHAudioConverter {
public:
    OHAudioConverter() = default;
    ~OHAudioConverter();

    OH_AudioConverter_Result Create(const OH_AudioConverter_Format* inputFormat,
                                        const OH_AudioConverter_Format* outputFormat);
    OH_AudioConverter_Result Destroy();

    OH_AudioConverter_Result SetRequestDataCallback(
        OH_AudioConverter_RequestDataCallback callback, void* userData);

    OH_AudioConverter_Result Process(
        void* audioData, int32_t outputCapacity,
        int32_t* outputSize);

private:
    void Reset();

    OH_AudioConverter_Result WrapInputData(void* audioData, uint32_t audioDataSize, bool finished,
                                        AudioSuite::AudioSuitePcmBuffer& buffer);
    AudioSuite::AudioSuitePcmBuffer* DoConvert(AudioSuite::AudioSuitePcmBuffer* inputBuffer);
    void CopyOutputData(AudioSuite::AudioSuitePcmBuffer* buffer, void* outputBuffer,
                        uint32_t maxFrameSize, uint32_t* actualFrameSize);
    OH_AudioConverter_Result ConvertToPcmFormat(
        const OH_AudioConverter_Format *inputFormat, const OH_AudioConverter_Format *outputFormat);

    OH_AudioConverter_Result ValidateProcessParams(void* audioData, int32_t* outputSize);
    void CalculateOutputFormatParams(int32_t outputCapacity,
        int32_t* outputBytesPerFrame, int32_t* maxOutputFrames);
    int32_t CalculateMinBytesForIntMs(uint32_t inputBytesPerFrame);
    OH_AudioConverter_Result ReadAndCacheInputData(bool* shouldStopProcessing);
    OH_AudioConverter_Result CalculateMinBytesForIntMsInternal(uint32_t inputBytesPerFrame,
        int32_t* minBytesForIntMs, int32_t* alignedCachedSize, bool* shouldStopProcessing, uint32_t outputCapacity);
    OH_AudioConverter_Result CalculateProcessableBytesInternal(const ProcessableBytesCalculationParams& params,
        uint32_t* alignedProcessableBytes, bool* shouldStopProcessing);
    OH_AudioConverter_Result WrapAndConvertInputData(int32_t processableSize,
        AudioSuite::AudioSuitePcmBuffer* outConvertedBuffer);
    OH_AudioConverter_Result ProcessConversion(void* audioData, int32_t outputCapacity,
        int32_t* outputSize, int32_t outputBytesPerFrame, int32_t maxOutputFrames);
    OH_AudioConverter_Result CheckAndProcessInputData(const InputProcessParams &params, int32_t *minBytesForIntMs,
        int32_t *alignedCachedSize, uint32_t *alignedProcessableBytes, bool *shouldStopProcessing);
    OH_AudioConverter_Result InitializeProcessingContext(void* audioData, int32_t outputCapacity,
        int32_t inputBytesPerFrame, ProcessingContext& context, bool& shouldStopProcessing);
    OH_AudioConverter_Result ValidateOutputCapacity(AudioSuite::AudioSuitePcmBuffer& convertedBuffer,
        uint32_t remainingCapacity, uint32_t* framesToWrite);
    void UpdateProcessingState(AudioSuite::AudioSuitePcmBuffer& convertedBuffer,
        uint32_t processableSize, ProcessingContext& context, uint32_t actualFramesWritten);
    OH_AudioConverter_Result ProcessSingleBlock(const InputProcessParams& params,
        ProcessingContext& context, int32_t inputBytesPerFrame,
        int32_t outputBytesPerFrame, bool& shouldStopProcessing);
    OH_AudioConverter_Result ProcessCachedOutputData(void* audioData, int32_t outputCapacity,
        int32_t outputBytesPerFrame, ProcessingContext& context);
    OH_AudioConverter_Result ProcessDataBlocks(ProcessingContext& context, int32_t inputBytesPerFrame,
        int32_t outputBytesPerFrame);

    std::unique_ptr<AudioSuite::AudioSuiteFormatConversion> formatConversion_;

    OH_AudioConverter_RequestDataCallback requestCallback_ = nullptr;
    void* callbackUserData_ = nullptr;

    AudioSuite::PcmBufferFormat inputFormat_;
    AudioSuite::PcmBufferFormat outputFormat_;

    bool inputFinished_ = false;
    std::vector<uint8_t> cachedInputData_;
    std::vector<uint8_t> cachedOutputData_;

    std::mutex mutex_;

    int32_t createCount_ = 0;
    int32_t destroyCount_ = 0;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_SUITE_ENGINE_H