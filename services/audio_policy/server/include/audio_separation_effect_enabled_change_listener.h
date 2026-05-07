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
#ifndef AUDIO_STANDARD_SEPARATION_EFFECT_ENABLED_CHANGE_LISTENER_H
#define AUDIO_STANDARD_SEPARATION_EFFECT_ENABLED_CHANGE_LISTENER_H

#include "standard_audio_separation_effect_enabled_change_listener_stub.h"
#include "audio_separation_effect_enabled_change_callback.h"

namespace OHOS {
namespace AudioStandard {

class AudioSeparationEffectEnabledChangeListener : public StandardAudioSeparationEffectEnabledChangeListenerStub {
public:
    AudioSeparationEffectEnabledChangeListener();
    virtual ~AudioSeparationEffectEnabledChangeListener();

    int32_t OnAudioSeparationEffectEnabledChange(bool enabled) override;
    void SetCallback(const std::weak_ptr<AudioSeparationEffectEnabledChangeCallback> &callback);
private:
    std::weak_ptr<AudioSeparationEffectEnabledChangeCallback> callback_;
};

} // AudioStandard
} // OHOS

#endif // AUDIO_STANDARD_SEPARATION_EFFECT_ENABLED_CHANGE_LISTENER_H