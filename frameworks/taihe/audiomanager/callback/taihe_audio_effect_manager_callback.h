/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_EFFECT_MANAGER_CALLBACK_H
#define TAIHE_AUDIO_EFFECT_MANAGER_CALLBACK_H

#include "event_handler.h"
#include "audio_system_manager.h"
#include "audio_effect_manager.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string AUDIO_SEPARATION_EFFECT_ENABLED_CHANGE_CALLBACK_NAME = "audioSeparationEffectEnabledChange";

class TaiheAudioSeparationEffectEnabledChangeCallback :
    public OHOS::AudioStandard::AudioSeparationEffectEnabledChangeCallback,
    public std::enable_shared_from_this<TaiheAudioSeparationEffectEnabledChangeCallback> {
public:
    explicit TaiheAudioSeparationEffectEnabledChangeCallback();
    virtual ~TaiheAudioSeparationEffectEnabledChangeCallback();
    void SaveAudioSeparationEffectEnabledChangeCallbackReference(std::shared_ptr<uintptr_t> callback);
    void RemoveAudioSeparationEffectEnabledChangeCallbackReference(std::shared_ptr<uintptr_t> callback);
    void RemoveAllAudioSeparationEffectEnabledChangeCallbackReference();
    int32_t GetAudioSeparationEffectEnabledChangeCbListSize();
    void OnAudioSeparationEffectEnabledChange(bool enabled) override;

private:
    struct AudioSeparationEffectEnabledJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        bool enabled;
    };

    void OnJsCallbackAudioSeparationEffectEnabled(std::unique_ptr<AudioSeparationEffectEnabledJsCallback> &jsCb);
    static void SafeJsCallbackAudioSeparationEffectEnabledWork(AudioSeparationEffectEnabledJsCallback *event);

    std::mutex mutex_;
    std::list<std::shared_ptr<AutoRef>> audioSeparationEffectEnabledChangeCbList_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_EFFECT_MANAGER_CALLBACK_H