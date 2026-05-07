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
#ifndef LOG_TAG
#define LOG_TAG "AudioSeparationEffectEnabledChangeListener"
#endif

#include "audio_separation_effect_enabled_change_listener.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

constexpr int32_t ON_SEPARATION_EFFECT_ENABLED_CHANGE = 1;

AudioSeparationEffectEnabledChangeListener::AudioSeparationEffectEnabledChangeListener()
{
    AUDIO_DEBUG_LOG("AudioSeparationEffectEnabledChangeListener: instance create");
}

AudioSeparationEffectEnabledChangeListener::~AudioSeparationEffectEnabledChangeListener()
{
    AUDIO_DEBUG_LOG("AudioSeparationEffectEnabledChangeListener: instance destroy");
}

int StandardAudioSeparationEffectEnabledChangeListenerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (code == ON_SEPARATION_EFFECT_ENABLED_CHANGE) {
        bool enabled = data.ReadBool();
        OnAudioSeparationEffectEnabledChange(enabled);
        return SUCCESS;
    }
    AUDIO_ERR_LOG("OnRemoteRequest: unknown code %{public}d", code);
    return ERR_INVALID_OPERATION;
}

void AudioSeparationEffectEnabledChangeListener::OnAudioSeparationEffectEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("OnAudioSeparationEffectEnabledChange received: enabled=%{public}d", enabled);
    if (callback_) {
        auto cb = callback_.lock();
        if (cb != nullptr) {
            cb->OnAudioSeparationEffectEnabledChange(enabled);
        } else {
            AUDIO_ERR_LOG("callback_ lock failed");
        }
    }
}

void AudioSeparationEffectEnabledChangeListener::SetCallback(
    const std::weak_ptr<AudioSeparationEffectEnabledChangeCallback> &callback)
{
    callback_ = callback;
}

} // AudioStandard
} // OHOS