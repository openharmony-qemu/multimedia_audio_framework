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

#ifndef AUDIO_SUITE_FORMAT_CONVERSION_H
#define AUDIO_SUITE_FORMAT_CONVERSION_H

#include <vector>
#include "securec.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_proresampler.h"
#include "hpae_format_convert.h"
#include "channel_converter.h"
#include "audio_suite_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteFormatConversion {
public:
    ~AudioSuiteFormatConversion() = default;

    AudioSuitePcmBuffer *Process(AudioSuitePcmBuffer *inPcmBuffer, PcmBufferFormat &outFormat);
    void Reset();

private:
    int32_t Resample(AudioSuitePcmBuffer *in, AudioSuitePcmBuffer *out);
    int32_t ChannelConvert(AudioSuitePcmBuffer *in, AudioSuitePcmBuffer *out);

    AudioSuitePcmBuffer rateOut_;
    AudioSuitePcmBuffer channelOut_;
    AudioSuitePcmBuffer formatFloatOut_;
    AudioSuitePcmBuffer formatOut_;

    std::unique_ptr<HPAE::ProResampler> proResampler_ = nullptr;
    HPAE::ChannelConverter channelConverter_;

    struct ResampleCfg {
        uint32_t inRate;
        uint32_t outRate;
        uint32_t channels;
    };
    struct ResampleCfg resampleCfg_ = {0};
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif