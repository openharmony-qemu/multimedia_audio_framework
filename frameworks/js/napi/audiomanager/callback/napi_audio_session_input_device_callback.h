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
#ifndef NAPI_AUDIO_SESSION_INPUT_DEVICE_CALLBACK_H
#define NAPI_AUDIO_SESSION_INPUT_DEVICE_CALLBACK_H

#include "audio_session_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"

namespace OHOS {
namespace AudioStandard {

const std::string AUDIO_SESSION_INPUT_DEVICE_CALLBACK_NAME = "currentInputDeviceChanged";

class NapiAudioSessionInputDeviceCallback : public AudioSessionCurrentInputDeviceChangedCallback {
public:
    explicit NapiAudioSessionInputDeviceCallback(napi_env env);
    virtual ~NapiAudioSessionInputDeviceCallback();
    
    void OnAudioSessionCurrentInputDeviceChanged(const CurrentInputDeviceChangedEvent &deviceEvent);
    void SaveCallbackReference(napi_value args);
    void CreateAudioSessionInputDeviceTsfn(napi_env env);
    bool GetAudioSessionInputDeviceTsfnFlag() const;
    bool ContainSameJsCallback(napi_value args);
    
private:
    struct AudioSessionInputDeviceJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        CurrentInputDeviceChangedEvent audioSessionDeviceEvent;
    };

    void OnJsCallbackAudioSessionInputDevice(std::unique_ptr<AudioSessionInputDeviceJsCallback> &jsCb);
    static void AudioSessionInputDeviceTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackAudioSessionInputDeviceWork(napi_env env, napi_value js_cb, void *context, void *data);

    std::shared_ptr<AutoRef> audioSessionInputDeviceJsCallback_ = nullptr;
    napi_ref callback_ = nullptr;
    std::mutex mutex_;
    napi_env env_;
    bool regAmSessionInputDeviceChgTsfn_ = false;
    napi_threadsafe_function amSessionInputDeviceChgTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_SESSION_INPUT_DEVICE_CALLBACK_H */