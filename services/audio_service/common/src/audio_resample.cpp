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
#include <cinttypes>
#ifdef SPEEX_ENABLE
#include <speex/speex_resampler.h>
#endif

namespace OHOS {
namespace AudioStandard {
struct AudioResample::SpeexResample {
#ifdef SPEEX_ENABLE
    SpeexResamplerState *resampler;
    uint32_t channelCount_;
#endif
};
AudioResample::AudioResample(uint32_t channels, uint32_t inRate, uint32_t outRate, int32_t quantity) : speex_(nullptr)
{
#ifdef SPEEX_ENABLE
    int32_t error;
    speex_ = std::make_unique<SpeexResample>();
    speex_->channelCount_ = channels;
    speex_->resampler = speex_resampler_init(channels, inRate, outRate, quantity, &error);
    if (speex_->resampler) {
        speex_resampler_skip_zeros(speex_->resampler);
    } else {
        AUDIO_INFO_LOG("create resample failed.");
        speex_ = nullptr;
    }
#endif
}

bool AudioResample::IsResampleInit() const noexcept
{
    if (speex_) {
        return true;
    }
    return false;
}

AudioResample::~AudioResample()
{
    if (!speex_) {
        return;
    }
#ifdef SPEEX_ENABLE
    if (!speex_->resampler)
        return;
    speex_resampler_destroy(speex_->resampler);
#endif
}

int32_t AudioResample::ProcessFloatResample(const std::vector<float> &input, std::vector<float> &output)
{
    int32_t ret = 0;
    if (!speex_) {
        return ERR_INVALID_PARAM;
    }
#ifdef SPEEX_ENABLE
    Trace trace("AudioResample::ProcessFloatResample");
    if (speex_->channelCount_ <= 0) {
        return ERR_INVALID_PARAM;
    }
    uint32_t inSize = input.size() / speex_->channelCount_;
    uint32_t outSize = output.size() / speex_->channelCount_;
    ret = speex_resampler_process_interleaved_float(speex_->resampler, input.data(), &inSize, output.data(), &outSize);
    AUDIO_DEBUG_LOG("after in size:%{public}d,out size:%{public}d,result:%{public}d", inSize, outSize, ret);
#endif
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
