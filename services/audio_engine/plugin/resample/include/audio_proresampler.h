/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_PRORESAMPLER_H
#define AUDIO_PRORESAMPLER_H

#include "resampler.h"
#include "audio_proresampler_process.h"
#include <vector>
#include <string>
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class ProResampler : public Resampler {
public:
    // notice: inRate and outRate should be different, otherwise resampler will not process
    // and will return RESAMPLER_ERR_ALLOC_FAILED in Process()
    ProResampler(uint32_t inRate, uint32_t outRate, uint32_t channels, uint32_t quality);
    ~ProResampler() override;
    // disable deep copy, enable move semantics to manage memory allocated by C malloc
    ProResampler(const ProResampler &) = delete;
    ProResampler &operator=(const ProResampler &) = delete;
    ProResampler(ProResampler &&) noexcept;
    ProResampler &operator=(ProResampler &&) noexcept;
    void Reset() override;
    int32_t Process(const float *inBuffer, uint32_t inFrameSize, float *outBuffer, uint32_t outFrameSize)
        override;
    int32_t UpdateRates(uint32_t inRate, uint32_t outRate) override;
    int32_t UpdateChannels(uint32_t channels) override;
    uint32_t GetInRate() const override;
    uint32_t GetOutRate() const override;
    uint32_t GetChannels() const override;
    uint32_t GetQuality() const;
private:
    int32_t Process11025SampleRate(const float *inBuffer, uint32_t inFrameSize, float *outBuffer,
        uint32_t outFrameSize);
    int32_t ProcessOtherSampleRate(const float *inBuffer, uint32_t inFrameSize, float *outBuffer,
        uint32_t outFrameSize);
    int32_t Process10HzSampleRate(const float *inBuffer, uint32_t inFrameSize, float *outBuffer,
        uint32_t outFrameSize);
    std::string ErrCodeToString(int32_t errCode);
    int32_t ConfigBufferSizeAndExpectedInFrameLen();
    std::vector<float> buf11025_;
    std::vector<float> bufFor100ms_;
    uint32_t buf11025Index_ = 0;
    uint32_t bufFor100msIndex_ = 0;
    uint32_t inRate_;
    uint32_t outRate_;
    uint32_t channels_;
    uint32_t quality_;
    uint32_t expectedOutFrameLen_ = 0;
    uint32_t expectedInFrameLen_ = 0;
    SingleStagePolyphaseResamplerState* state_ = nullptr;
};
} // HPAE
} // AudioStandard
} // OHOS
#endif