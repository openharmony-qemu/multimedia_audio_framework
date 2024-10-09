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
#ifndef I_AUDIO_MCR_MIXER_H
#define I_AUDIO_MCR_MIXER_H
#include "audio_effect.h"
namespace OHOS {
namespace AudioStandard {
namespace IntegrationMCR {
class IAudioMcrMixer {
public:
    virtual int32_t InitMixer(AudioChannelLayout mode, int32_t channels) = 0;
    virtual int32_t Apply(const int32_t &frameLength, float *input, float *output) = 0;
};
} // namespace IntegrationMCR
} // namespace AudioStandard
} // namespace OHOS
typedef int32_t AudioMcrMixerClassCreateFunc(OHOS::AudioStandard::IntegrationMCR::IAudioMcrMixer **mixer);
#endif