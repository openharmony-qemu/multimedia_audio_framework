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

#include "audio_suite_pcm_buffer.h"

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
    // Return value: Data size (bytes), negative value indicates an error.
    // Output parameters: data - data pointer, status - data status
    virtual int32_t OnRequestData(const void** data, InputDataStatus* status) = 0;
};

// Format converter interface class (pure virtual interface)
class IAudioFormatConverter {
public:
    virtual ~IAudioFormatConverter() = default;

    // Interface 1: Create - Initialize the converter (with format parameters and format validation)
    virtual int32_t Create(const PcmBufferFormat& inputFormat, const PcmBufferFormat& outputFormat) = 0;

    // Interface 2: SetInputCallback - Sets the data request callback
    virtual int32_t SetInputCallback(std::shared_ptr<FormatConverterDataCallback> callback) = 0;

    // Interface 3: Process - Processes audio data
    virtual int32_t Process(void* outputData, uint32_t outputCapacity, uint32_t* outputSize) = 0;

    // Interface 4: Destroy - Destruction Converter
    virtual void Destroy() = 0;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_FORMAT_CONVERTER_INTERFACE_H