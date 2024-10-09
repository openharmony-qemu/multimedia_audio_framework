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
#ifndef NAPI_AUDIO_RENDERER_DEVICE_CHANGE_CALLBACK_H
#define NAPI_AUDIO_RENDERER_DEVICE_CHANGE_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_renderer.h"

namespace OHOS {
namespace AudioStandard {
class NapiAudioRendererDeviceChangeCallback : public AudioRendererOutputDeviceChangeCallback {
public:
    explicit NapiAudioRendererDeviceChangeCallback(napi_env env);
    virtual ~NapiAudioRendererDeviceChangeCallback();
    void AddCallbackReference(napi_value args);
    void RemoveCallbackReference(napi_env env, napi_value args);
    void OnOutputDeviceChange(const DeviceInfo &deviceInfo, const AudioStreamDeviceChangeReason reason) override;
    void RemoveAllCallbacks();
    int32_t GetCallbackListSize() const;

private:
    struct AudioRendererDeviceChangeJsCallback {
        napi_ref callback_;
        napi_env env_;
        DeviceInfo deviceInfo_;
    };

    static void WorkCallbackCompleted(uv_work_t* work, int status);
    void OnJsCallbackRendererDeviceInfo(napi_ref method, const DeviceInfo &deviceInfo);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<napi_ref> callbacks_ {};
};

class NapiAudioRendererOutputDeviceChangeWithInfoCallback : public AudioRendererOutputDeviceChangeCallback {
public:
    explicit NapiAudioRendererOutputDeviceChangeWithInfoCallback(napi_env env);
    virtual ~NapiAudioRendererOutputDeviceChangeWithInfoCallback();
    void AddCallbackReference(napi_value args);
    void RemoveCallbackReference(napi_env env, napi_value args);
    void OnOutputDeviceChange(const DeviceInfo &deviceInfo, const AudioStreamDeviceChangeReason reason) override;
    void RemoveAllCallbacks();
    int32_t GetCallbackListSize() const;

private:
    struct AudioRendererOutputDeviceChangeWithInfoJsCallback {
        napi_ref callback_;
        napi_env env_;
        DeviceInfo deviceInfo_;
        AudioStreamDeviceChangeReason reason_;
    };

    static void WorkCallbackCompleted(uv_work_t* work, int status);
    void OnJsCallbackOutputDeviceInfo(napi_ref method, const DeviceInfo &deviceInfo,
        const AudioStreamDeviceChangeReason reason);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<napi_ref> callbacks_ {};
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // NAPI_AUDIO_RENDERER_DEVICE_CHANGE_CALLBACK_H
