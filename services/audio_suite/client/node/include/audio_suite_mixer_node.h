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

#ifndef AUDIO_SUITE_MIXER_NODE_H
#define AUDIO_SUITE_MIXER_NODE_H

#include <memory>
#include "audio_suite_process_node.h"
#include "audio_limiter.h"
#include "audio_suite_pcm_buffer.h"
#include "channel_converter.h"
#include "hpae_format_convert.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteMixerNode : public AudioSuiteProcessNode {
public:
    AudioSuiteMixerNode();
    virtual ~AudioSuiteMixerNode();
    bool Reset() override;
protected:
    AudioSuitePcmBuffer *SignalProcess(const std::vector<AudioSuitePcmBuffer *> &inputs) override;
private:
    int32_t InitAudioLimiter();
    AudioSuitePcmBuffer *preProcess(AudioSuitePcmBuffer *input);
    AudioSuitePcmBuffer *preRateProcess(AudioSuitePcmBuffer *input,
        uint32_t sampleRate, uint32_t channelCount);
    AudioSuitePcmBuffer *preChannelProcess(AudioSuitePcmBuffer *input,
        uint32_t sampleRate, uint32_t channelCount);

    AudioSamplingRate rate_;

    AudioSuitePcmBuffer mixerOutput_;
    AudioSuitePcmBuffer tmpOutput_;
    AudioSuitePcmBuffer channelOutput_;
    AudioSuitePcmBuffer rateOutput_;

    uint32_t frameLen_;

    std::unique_ptr<AudioLimiter> limiter_ = nullptr;

    uint32_t channel_sampleRate_ = SAMPLE_RATE_192000;
    uint32_t channel_channelCount_ = STEREO;

    uint32_t rate_sampleRate_ = SAMPLE_RATE_192000;
    uint32_t rate_channelCount_ = STEREO;

    AudioSamplingRate mixrate_ = SAMPLE_RATE_8000;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif //AUDIO_SUITE_MIXER_NODE_H