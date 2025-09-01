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

#ifndef AUDIO_SESSION_AVAILABLE_DEVICECHANGE_CALLBACK_NAPI_H
#define AUDIO_SESSION_AVAILABLE_DEVICECHANGE_CALLBACK_NAPI_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AVAILABLE_DEVICE_CHANGE_CALLBACK_NAME = "availableDeviceChange";

class NapiAudioSessionAvailableDeviceChangeCallback : public AudioManagerAvailableDeviceChangeCallback {
public:
    explicit NapiAudioSessionAvailableDeviceChangeCallback(napi_env env);
    virtual ~NapiAudioSessionAvailableDeviceChangeCallback();
    void SaveCallbackReference(const std::string &callbackName, napi_value callback);
    void OnAvailableDeviceChange(const AudioDeviceUsage usage, const DeviceChangeAction &deviceChangeAction) override;

    void SaveSessionAvailbleDeviceChangeCbRef(AudioDeviceUsage usage, napi_value callback);
    void RemoveSessionAvailbleDeviceChangeCbRef(napi_env env, napi_value callback);
    void RemoveAllSessionAvailbleDeviceChangeCb();
    int32_t GetSessionAvailbleDeviceChangeCbListSize();
    void CreateSessionDevChgTsfn(napi_env env);
    bool GetSessionDevChgTsfnFlag();

private:
    struct AudioSessionAvailbleDeviceJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        DeviceChangeAction deviceChangeAction;
    };

    void OnJsCallbackAvailbleDeviceChange(std::unique_ptr<AudioSessionAvailbleDeviceJsCallback> &jsCb);
    static void AvailbleDeviceChangeTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackAvailbleDeviceChangeWork(napi_env env, napi_value js_cb, void *context, void *data);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> deviceChangeCallback_ = nullptr;
    std::list<std::pair<std::shared_ptr<AutoRef>, AudioDeviceUsage>> availableDeviceChangeCbList_;
    bool regAmRouDevChgTsfn_ = false;
    napi_threadsafe_function amRouDevChgTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_SESSION_AVAILABLE_DEVICECHANGE_CALLBACK_NAPI_H */
