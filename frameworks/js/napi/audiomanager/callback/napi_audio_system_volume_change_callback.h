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
#ifndef NAPI_AUDIO_SYSTEM_VOLUME_CHANGE_CALLBACK_H
#define NAPI_AUDIO_SYSTEM_VOLUME_CHANGE_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

const std::string AUDIO_SYSTEM_VOLUME_CHANGE_CALLBACK_NAME = "systemVolumeChange";

class NapiAudioSystemVolumeChangeCallback : public SystemVolumeChangeCallback {
public:
    explicit NapiAudioSystemVolumeChangeCallback(napi_env env);
    virtual ~NapiAudioSystemVolumeChangeCallback();
    void OnSystemVolumeChange(VolumeEvent volumeEvent) override;
    void RemoveCallbackReference(napi_env env, napi_value args);
    void RemoveAllCallbackReference();
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    bool ContainSameJsCallback(napi_value args);
    bool IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback);
    void CreateSystemVolumeChangeTsfn(napi_env env);
    bool GetVolumeTsfnFlag();
    int32_t GetSystemVolumeCbListSize();
    napi_threadsafe_function GetTsfn();

private:
    struct AudioSystemVolumeChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        VolumeEvent volumeEvent;
    };

    void OnJsCallbackSystemVolumeChange(std::unique_ptr<AudioSystemVolumeChangeJsCallback> &jsCb);
    static void CleanUp(void *data);
    static void SafeJsCallbackSystemVolumeChangeWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void SystemVolumeChangeTsfnFinalize(napi_env env, void *data, void *hint);

    std::list<std::shared_ptr<AutoRef>> audioSystemVolumeChangeCbList_;
    std::mutex mutex_;
    napi_env env_;
    static napi_ref sConstructor_;
    bool regVolumeTsfn_ = false;
    napi_threadsafe_function amVolEntTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_INTERRUPT_MANAGER_H */