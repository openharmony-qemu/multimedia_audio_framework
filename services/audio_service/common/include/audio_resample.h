/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef AUDIO_RESAMPLE_H
#define AUDIO_RESAMPLE_H

#include <vector>

namespace OHOS {
namespace AudioStandard {
class AudioResample {
public:
    AudioResample(uint32_t channels, uint32_t inRate, uint32_t outRate, int32_t quantity);
    ~AudioResample();
    bool IsResampleInit() const noexcept;
    int32_t ProcessFloatResample(const std::vector<float> &input, std::vector<float> &output);

private:
    struct SpeexResample;
    std::unique_ptr<SpeexResample> speex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
