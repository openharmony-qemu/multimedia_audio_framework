/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#ifndef NAPI_AUDIO_ROUTING_MANAGER_CALLBACK_H
#define NAPI_AUDIO_ROUTING_MANAGER_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {

const std::string PREFERRED_OUTPUT_DEVICE_CALLBACK_NAME = "preferredOutputDeviceChangeForRendererInfo";
const std::string PREFER_OUTPUT_DEVICE_CALLBACK_NAME = "preferOutputDeviceChangeForRendererInfo";
const std::string PREFERRED_INPUT_DEVICE_CALLBACK_NAME  = "preferredInputDeviceChangeForCapturerInfo";

class NapiAudioPreferredOutputDeviceChangeCallback : public AudioPreferredOutputDeviceChangeCallback {
public:
    explicit NapiAudioPreferredOutputDeviceChangeCallback(napi_env env);
    virtual ~NapiAudioPreferredOutputDeviceChangeCallback();
    void SaveCallbackReference(napi_value callback);
    void OnPreferredOutputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override;
    void CreatePreferredOutTsfn(napi_env env);
    bool GetPreferredOutTsfnFlag();
    bool ContainSameJsCallback(napi_value args);

private:
    struct AudioActiveOutputDeviceChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    };

    void OnJsCallbackActiveOutputDeviceChange(std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> &jsCb);
    static void SafeJsCallbackActiveOutputDeviceChangeWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void ActiveOutputDeviceChangeTsfnFinalize(napi_env env, void *data, void *hint);

    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> callback_ = nullptr;
    bool regAmOutputDevChgTsfn_ = false;
    napi_threadsafe_function amOutputDevChgTsfn_ = nullptr;
};

class NapiAudioPreferredInputDeviceChangeCallback : public AudioPreferredInputDeviceChangeCallback {
public:
    explicit NapiAudioPreferredInputDeviceChangeCallback(napi_env env);
    virtual ~NapiAudioPreferredInputDeviceChangeCallback();
    void SaveCallbackReference(napi_value callback);
    void OnPreferredInputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override;
    void CreatePreferredInTsfn(napi_env env);
    bool GetPreferredInTsfnFlag();
    bool ContainSameJsCallback(napi_value args);

private:
    struct AudioActiveInputDeviceChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    };

    void OnJsCallbackActiveInputDeviceChange(std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> &jsCb);
    static void SafeJsCallbackActiveInputDeviceChangeWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void ActiveInputDeviceChangeTsfnFinalize(napi_env env, void *data, void *hint);

    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> callback_ = nullptr;
    bool regAmInputDevChgTsfn_ = false;
    napi_threadsafe_function amInputDevChgTsfn_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_ROUTING_MANAGER_CALLBACK_H */