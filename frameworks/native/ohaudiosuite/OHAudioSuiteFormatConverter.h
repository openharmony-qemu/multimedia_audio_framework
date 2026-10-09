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

#include <memory>
#include "native_audio_converter.h"
#include "audio_format_converter.h"

namespace OHOS {
namespace AudioStandard {

class OHAudioConverter {
public:
    explicit OHAudioConverter(std::shared_ptr<AudioSuite::AudioFormatConverter> impl)
        : impl_(std::move(impl)) {};
    ~OHAudioConverter() = default;

    std::shared_ptr<AudioSuite::AudioFormatConverter> GetAudioFormatConverter() const;

private:
    // InnerAPI 实例
    std::shared_ptr<AudioSuite::AudioFormatConverter> impl_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_SUITE_FORMAT_CONVERTER_H