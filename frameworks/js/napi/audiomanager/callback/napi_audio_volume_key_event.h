/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef NAPI_AUDIO_VOLUME_KEY_EVENT_CALLBACK_H
#define NAPI_AUDIO_VOLUME_KEY_EVENT_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

const std::string VOLUME_KEY_EVENT_CALLBACK_NAME = "volumeChange";

class NapiAudioVolumeKeyEvent : public VolumeKeyEventCallback {
public:
    explicit NapiAudioVolumeKeyEvent(napi_env env);
    virtual ~NapiAudioVolumeKeyEvent();
    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override;
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    bool ContainSameJsCallback(napi_value args);
    void CreateVolumeTsfn(napi_env env);
    bool GetVolumeTsfnFlag();
    napi_threadsafe_function GetTsfn();
    
private:
    struct AudioVolumeKeyEventJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        VolumeEvent volumeEvent;
    };

    void OnJsCallbackVolumeEvent(std::unique_ptr<AudioVolumeKeyEventJsCallback> &jsCb);
    static void Cleanup(void *data);
    static void SafeJsCallbackVolumeEventWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void VolumeEventTsfnFinalize(napi_env env, void *data, void *hint);

    std::shared_ptr<AutoRef> audioVolumeKeyEventJsCallback_ = nullptr;
    std::mutex mutex_;
    napi_env env_;
    napi_ref callback_ = nullptr;
    static napi_ref sConstructor_;
    bool regVolumeTsfn_ = false;
    napi_threadsafe_function amVolEntTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_INTERRUPT_MANAGER_H */