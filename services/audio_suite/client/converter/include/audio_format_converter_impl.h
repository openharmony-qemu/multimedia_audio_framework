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
#include "audio_format_converter.h"
#include "audio_proresampler.h"
#include "audio_proresampler_process.h"
#include "hpae_format_convert.h"
#include "down_mixer.h"
#include "mixer_utils.h"
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

    int32_t Init(const AudioConverterFormat &inputFormat, const AudioConverterFormat &outputFormat,
        const uint32_t resampleQuality, const bool enableDataAlignment);

    int32_t SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback) override;
    int32_t Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize) override;

    int32_t GetMinInOutSizes(uint32_t &minInSize, uint32_t &minOutSize) override;

    size_t GetDelayDataSize() override;
    int32_t GetDelaySize(uint32_t &delaySize) override;
    int32_t DirectProcess(const uint8_t* inData, uint32_t inSize,
        void* outData, uint32_t outCapacity, uint32_t &outSize) override;
    int32_t Flush() override;

    void Destroy() override;

private:
    // Internal implementation method
    int32_t CopyOutputData(
        AudioSuitePcmBuffer& buffer, void* outputBuffer, uint32_t &totalOutputSize, uint32_t &remainingCapacity);

    // Calculation related methods
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

    // Init helpers
    int32_t ValidateCommonFormat(const AudioConverterFormat &inputFormat,
        const AudioConverterFormat &outputFormat, uint32_t resampleQuality);

    // Raw format conversion (migrated from AudioSuiteRawFormatConversion)
    int32_t InitRawConversion();
    int32_t InitResampler();
    int32_t InitChannelConverter();
    int32_t ProcessRawConversion(const uint8_t* inData, uint32_t inDataSize,
        std::vector<uint8_t>& outputVector);
    int32_t RunResampleStage(float*& currentData, uint32_t& currentSampleCount);
    int32_t RunChannelConvertStage(float*& currentData, uint32_t& currentSampleCount);
    int32_t ProcessResample(const float* inputData, uint32_t inSampleCount,
        float*& outData, uint32_t& outSampleCount);
    int32_t ProcessChannelConvert(const float* inputData, uint32_t inSampleCount,
        float*& outData, uint32_t& outSampleCount);
    int32_t ConvertToFloat(const uint8_t* inData, uint32_t inDataSize,
        std::vector<float>& outBuffer);
    int32_t ConvertFromFloat(const float* inData, uint32_t inSampleCount,
        std::vector<uint8_t>& outBuffer);
    void ResetRawConversion();

    // Direct polyphase resampler wrappers (replaces HPAE::ProResampler to avoid
    // memcpy_s ERANGE(34) when SingleStagePolyphaseResamplerProcess returns 0
    // frames during polyphase warm-up).
    int32_t ProcessResamplerInternal(const float* inBuffer, uint32_t inFrameLen,
        float* outBuffer, uint32_t outFrameLen);
    void ResetResamplerState();
    void DestroyResamplerState();

    // Channel convert (migrated from HPAE::ChannelConverter to isolate algorithm-side changes).
    // down_mixer.cpp / mixer_utils.cpp are still linked from audio_common_algorithm (judged stable).
    int32_t ChannelConvertSetParam(AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo);
    int32_t ChannelConvertProcess(uint32_t frameLen, float* in, float* out);
    void ChannelConvertSetUpmixCoef(float coef);
    void ChannelConvertReset();
    int32_t ChannelConvertMixProcess(bool isDmix, uint32_t frameLen, float* in, float* out);
    void ChannelConvertApplyUpmixGainRatio(float ratio);

    std::mutex mutex_;
    // for callback
    std::shared_ptr<FormatConverterDataCallback> dataCallback_;

    // for data
    std::vector<uint8_t> cachedInputData_;
    std::vector<uint8_t> cachedOutputData_;
    bool inputFinished_ = false;
    bool destroyed_ = false;

    // for convert
    AudioConverterFormat inputFormat_;
    AudioConverterFormat outputFormat_;
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

    // runtime mode lock: set once DirectProcess is used; callback stream is tracked by dataCallback_ != nullptr
    bool directProcessMode_ = false;
    uint32_t minInSize_ = 0;      // min input bytes per processing unit
    uint32_t minOutSize_ = 0;     // min output bytes per processing unit

    // Raw conversion members (migrated from AudioSuiteRawFormatConversion)
    // proResampler_ replaced by direct polyphase state + tmp buffer for stability.
    SingleStagePolyphaseResamplerState* resamplerState_ = nullptr;
    std::vector<float> resamplerTmpBuf_;        // polyphase intermediate buffer
    uint32_t resamplerLastDelayFrames_ = 0;     // last under-run frames (replaces GetDelayFrames)
    // Channel converter state (replaces HPAE::ChannelConverter; workSize fixed to FLOAT_SIZE_BYTES,
    // mixLfe fixed to true, matching the only call site at InitChannelConverter).
    HPAE::DownMixer channelDownMixer_;
    float channelMixTable_[HPAE::MAX_CHANNELS][HPAE::MAX_CHANNELS] = {{0}};
    AudioChannelInfo channelInInfo_ = {CH_LAYOUT_UNKNOWN, 0};
    AudioChannelInfo channelOutInfo_ = {CH_LAYOUT_UNKNOWN, 0};
    float channelUpmixCoef_ = HPAE::COEF_0DB_F;
    std::vector<float> floatBuffer_;
    std::vector<float> resampleBuffer_;
    std::vector<float> channelBuffer_;
    bool formatsSame_ = false;
    uint32_t resampleQuality_ = 1;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FORMAT_CONVERTER_IMPL_H