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
#ifndef NAPI_AUDIO_SESSION_DEVICE_CALLBACK_H
#define NAPI_AUDIO_SESSION_DEVICE_CALLBACK_H

#include "audio_session_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"

namespace OHOS {
namespace AudioStandard {

const std::string AUDIO_SESSION_DEVICE_CALLBACK_NAME = "currentOutputDeviceChanged";

class NapiAudioSessionDeviceCallback : public AudioSessionCurrentDeviceChangedCallback {
public:
    explicit NapiAudioSessionDeviceCallback(napi_env env);
    virtual ~NapiAudioSessionDeviceCallback();
    
    void OnAudioSessionCurrentDeviceChanged(const CurrentOutputDeviceChangedEvent &deviceEvent);
    void SaveCallbackReference(napi_value args);
    void CreateAudioSessionDeviceTsfn(napi_env env);
    bool GetAudioSessionDeviceTsfnFlag() const;
    bool ContainSameJsCallback(napi_value args);
    
private:
    struct AudioSessionDeviceJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        CurrentOutputDeviceChangedEvent audioSessionDeviceEvent;
    };

    void OnJsCallbackAudioSessionDevice(std::unique_ptr<AudioSessionDeviceJsCallback> &jsCb);
    static void AudioSessionDeviceTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackAudioSessionDeviceWork(napi_env env, napi_value js_cb, void *context, void *data);

    std::shared_ptr<AutoRef> audioSessionDeviceJsCallback_ = nullptr;
    napi_ref callback_ = nullptr;
    std::mutex mutex_;
    napi_env env_;
    bool regAmSessionDeviceChgTsfn_ = false;
    napi_threadsafe_function amSessionDeviceChgTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* NAPI_AUDIO_SESSION_DEVICE_CALLBACK_H */