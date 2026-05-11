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

#ifndef AUDIO_SUITE_RAW_FORMAT_CONVERSION_H
#define AUDIO_SUITE_RAW_FORMAT_CONVERSION_H

#include <memory>
#include <vector>
#include <mutex>
#include "audio_stream_info.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_proresampler.h"
#include "hpae_format_convert.h"
#include "channel_converter.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// Raw format conversion class using primitive conversion functions
class AudioSuiteRawFormatConversion {
public:
    AudioSuiteRawFormatConversion(const uint32_t resampleQuality);
    ~AudioSuiteRawFormatConversion();

    // Initialize with input and output formats
    int32_t Init(const PcmBufferFormat& inputFormat, const PcmBufferFormat& outputFormat);

    // Perform conversion on input data
    int32_t Process(const uint8_t* inData, uint32_t inDataSize, std::vector<uint8_t>& outputVector);

    // Reset internal state
    void Reset();

private:
    // Perform resampling with all necessary calculations and special handling
    int32_t ProcessResample(const float* inputData, uint32_t inSampleCount,
        float*& outData, uint32_t& outSampleCount);

    // Perform channel conversion with all necessary calculations
    int32_t ProcessChannelConvert(const float* inputData, uint32_t inSampleCount,
        float*& outData, uint32_t& outSampleCount);

    // Convert input data to float format
    int32_t ConvertToFloat(const uint8_t* inData, uint32_t inDataSize,
        std::vector<float>& outBuffer);

    // Convert float data to output format
    int32_t ConvertFromFloat(const float* inData, uint32_t inSampleCount,
        std::vector<uint8_t>& outBuffer);

    // Internal buffers
    std::vector<float> floatBuffer_;       // After convert to float
    std::vector<float> resampleBuffer_;    // After resample
    std::vector<float> channelBuffer_;     // After channel convert
    std::vector<uint8_t> outputBuffer_;    // Final output

    // Converter instances
    std::unique_ptr<HPAE::ProResampler> proResampler_ = nullptr;
    HPAE::ChannelConverter channelConverter_;

    // Configuration
    PcmBufferFormat inputFormat_;
    PcmBufferFormat outputFormat_;
    bool initialized_;
    bool formatsSame_;  // true if input and output formats are identical

    // Thread safety
    std::mutex mutex_;
    uint32_t resampleQuality_ = 1;
    static constexpr uint32_t DOUBLE_FRAME = 2;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SUITE_RAW_FORMAT_CONVERSION_H