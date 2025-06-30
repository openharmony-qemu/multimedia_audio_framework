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
#include "napi_audio_renderer_callback_inner.h"
#include "napi_audio_renderer_callback.h"

namespace OHOS {
namespace AudioStandard {
    class NapiAudioRendererDeviceChangeCallback : public AudioRendererOutputDeviceChangeCallback {
public:
    explicit NapiAudioRendererDeviceChangeCallback(napi_env env);
    ~NapiAudioRendererDeviceChangeCallback();
    void OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReason reason) override;
    void RemoveAllCallbacks();
    int32_t GetCallbackListSize() const;
    void CreateRendererDeviceChangeTsfn(napi_env env);
    bool GetRendererDeviceChangeTsfnFlag();
    void AddCallbackReference(napi_value args);
    void RemoveCallbackReference(napi_env env, napi_value args = nullptr);
private:
    struct AudioRendererDeviceChangeJsCallback {
        napi_ref callback_;
        napi_env env_;
        AudioDeviceDescriptor deviceInfo_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    };

    void OnJsCallbackRendererDeviceInfo(napi_ref method, const AudioDeviceDescriptor &deviceInfo);
    static void RendererDeviceInfoTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackRendererDeviceInfoWork(napi_env env, napi_value js_cb, void *context, void *data);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> callbacks_ {};
    bool regArDevInfoTsfn_ = false;
    napi_threadsafe_function arDevInfoTsfn_ = nullptr;
};

class NapiAudioRendererOutputDeviceChangeWithInfoCallback : public AudioRendererOutputDeviceChangeCallback {
public:
    explicit NapiAudioRendererOutputDeviceChangeWithInfoCallback(napi_env env);
    ~NapiAudioRendererOutputDeviceChangeWithInfoCallback();
    void OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReason reason) override;
    void RemoveAllCallbacks();
    int32_t GetCallbackListSize() const;
    void CreateOutputDeviceChangeTsfn(napi_env env);
    bool GetOutputDeviceChangeTsfnFlag();
    void AddCallbackReference(napi_value args);
    void RemoveCallbackReference(napi_env env, napi_value args = nullptr);

private:
    struct AudioRendererOutputDeviceChangeWithInfoJsCallback {
        napi_ref callback_;
        napi_env env_;
        AudioDeviceDescriptor deviceInfo_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
        AudioStreamDeviceChangeReason reason_;
    };

    void OnJsCallbackOutputDeviceInfo(napi_ref method, const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReason reason);
    static void SafeJsCallbackOutputDeviceInfoWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void OutputDeviceInfoTsfnFinalize(napi_env env, void *data, void *hint);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> callbacks_ {};
    bool regArOutputDevChg_ = false;
    napi_threadsafe_function arOutputDevChgTsfn_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // NAPI_AUDIO_RENDERER_DEVICE_CHANGE_CALLBACK_H
