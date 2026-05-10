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
#include "audio_format_converter_impl.h"
#include "audio_format_converter.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioConverter {
public:
    OHAudioConverter() = default;
    ~OHAudioConverter();

    OH_AudioConverter_Result Create(const OH_AudioConverter_Format* inputFormat,
                                        const OH_AudioConverter_Format* outputFormat);
    void Destroy();

    OH_AudioConverter_Result SetRequestDataCallback(
        OH_AudioConverter_RequestDataCallback callback, void* userData);

    OH_AudioConverter_Result Process(
        void* audioData, uint32_t outputCapacity,
        uint32_t* outputSize);

private:
    OH_AudioConverter_Result ConvertToPcmFormat(
        const OH_AudioConverter_Format *inputFormat, const OH_AudioConverter_Format *outputFormat);

    // InnerAPI 实例
    std::shared_ptr<AudioSuite::AudioFormatConverter> impl_;

    AudioSuite::PcmBufferFormat inputFormat_;
    AudioSuite::PcmBufferFormat outputFormat_;

    std::mutex mutex_;
    bool isInitialized_ = false;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_SUITE_FORMAT_CONVERTER_H