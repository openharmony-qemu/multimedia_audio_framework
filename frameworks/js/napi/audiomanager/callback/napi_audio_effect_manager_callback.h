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
#ifndef NAPI_AUDIO_EFFECT_MANAGER_CALLBACK_H
#define NAPI_AUDIO_EFFECT_MANAGER_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "napi_audio_manager.h"
#include "audio_effect_manager.h"

namespace OHOS {
namespace AudioStandard {
inline const std::string AUDIO_SEPARATION_EFFECT_ENABLED_CHANGE_CALLBACK_NAME =
    "audioSeparationEffectEnabledChange";

class NapiAudioSeparationEffectEnabledChangeCallback : public AudioSeparationEffectEnabledChangeCallback {
public:
    explicit NapiAudioSeparationEffectEnabledChangeCallback(napi_env env);
    virtual ~NapiAudioSeparationEffectEnabledChangeCallback();
    
    void SaveAudioSeparationEffectEnabledChangeCallbackReference(napi_value args);
    void RemoveAudioSeparationEffectEnabledChangeCallbackReference(napi_env env, napi_value args);
    void RemoveAllAudioSeparationEffectEnabledChangeCallbackReference();
    int32_t GetAudioSeparationEffectEnabledChangeCbListSize();
    
    void OnAudioSeparationEffectEnabledChange(bool enabled) override;
    void CreateSeparationEffectEnableTsfn(napi_env env);
    bool GetSeparationEffectEnableTsfnFlag();

private:
    struct AudioSeparationEffectEnabledJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        bool enabled;
    };

    void OnJsCallbackAudioSeparationEffectEnabled(
        std::unique_ptr<AudioSeparationEffectEnabledJsCallback> &jsCb);
    static void SafeJsCallbackAudioSeparationEffectEnabledWork(
        napi_env env, napi_value js_cb, void *context, void *data);
    static void AudioSeparationEffectEnabledTsfnFinalize(
        napi_env env, void *data, void *hint);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> audioSeparationEffectEnabledChangeCbList_;
    static bool onAudioSeparationEffectEnabledChangeFlag_;
    bool regAmSeparationEffectEnableTsfn_ = false;
    napi_threadsafe_function amSeparationEffectEnableTsfn_ = nullptr;
};

} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_EFFECT_MANAGER_CALLBACK_H */