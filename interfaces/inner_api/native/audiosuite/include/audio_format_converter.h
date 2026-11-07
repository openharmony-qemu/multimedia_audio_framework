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

#ifndef AUDIO_FORMAT_CONVERTER_INTERFACE_H
#define AUDIO_FORMAT_CONVERTER_INTERFACE_H

#include <cstdint>
#include <memory>
#include <mutex>

#include "audio_errors.h"
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

// InnerAPI data state enumeration (consistent with CAPI)
enum class InputDataStatus {
    HAVE_DATA = 1,          // data available
    NO_AVAILABLE_DATA = 2,  // No data yet
    DATA_FINISHED = 3,      // End of data flow
};

// Abstract class for format converter data request callback
class FormatConverterDataCallback {
public:
    virtual ~FormatConverterDataCallback() = default;
    /*
    * Return value: Data size (bytes), negative value indicates an error.
    * Output parameters: data - data pointer, status - data status
    */
    virtual int32_t OnRequestData(const void** data, InputDataStatus* status) = 0;
};

class AudioFormatConverter {
public:
    virtual ~AudioFormatConverter() = default;

    static std::shared_ptr<AudioFormatConverter> Create(
        const AudioConverterFormat &inputFormat, const AudioConverterFormat &outputFormat,
        const uint32_t resampleQuality = 1, const bool enableDataAlignment = false);

    // Get minimum input/output sizes (bytes) for the converter instance.
    virtual int32_t GetMinInOutSizes(uint32_t &minInBytes, uint32_t &minOutBytes) = 0;

    // Input data is passed using a callback method, the output data and the original data are aligned in position.
    virtual int32_t SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback) = 0;
    virtual int32_t Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize) = 0;

    // Input data is transferred through the interface
    virtual size_t GetDelayDataSize() = 0;
    virtual int32_t GetDelaySize(uint32_t &delaySize) = 0;

    // Direct processing: writes converted data to caller-provided buffer.
    // inBytes must be a multiple of minInBytes. Output overflow is buffered internally.
    virtual int32_t DirectProcess(const uint8_t* inData, uint32_t inBytes,
        void* outData, uint32_t outCapacity, uint32_t &outSize) = 0;

    // Flush all internal buffers. The converter remains usable.
    virtual int32_t Flush() = 0;

    virtual void Destroy() = 0;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FORMAT_CONVERTER_INTERFACE_H