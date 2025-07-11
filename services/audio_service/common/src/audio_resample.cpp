/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "audio_resample.h"
#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_utils.h"
#include "audio_proresampler.h"
#include <cinttypes>

namespace OHOS {
namespace AudioStandard {
AudioResample::AudioResample(uint32_t channels, uint32_t inRate, uint32_t outRate, int32_t quantity)
    : resampler_(std::make_unique<HPAE::ProResampler>(inRate, outRate, channels, quantity))
{
}

bool AudioResample::IsResampleInit() const noexcept
{
    if (resampler_) {
        return true;
    }
    return false;
}

AudioResample::~AudioResample()
{
    if (!resampler_) {
        return;
    }
    resampler_->Reset();
    resampler_ = nullptr;
}

int32_t AudioResample::ProcessFloatResample(const std::vector<float> &input, std::vector<float> &output)
{
    if (!resampler_) {
        return ERR_INVALID_PARAM;
    }
    Trace trace("AudioResample::ProcessFloatResample");
    if (resampler_->GetChannels() <= 0) {
        return ERR_INVALID_PARAM;
    }
    uint32_t inSize = input.size() / resampler_->GetChannels();
    uint32_t outSize = output.size() / resampler_->GetChannels();
    return resampler_->Process(input.data(), inSize, output.data(), outSize);
}
} // namespace AudioStandard
} // namespace OHOS
