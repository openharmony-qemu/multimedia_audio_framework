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
#ifndef NAPI_AUDIO_MANAGER_CALLBACK_H
#define NAPI_AUDIO_MANAGER_CALLBACK_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string DEVICE_CHANGE_CALLBACK_NAME = "deviceChange";
const std::string MIC_STATE_CHANGE_CALLBACK_NAME = "micStateChange";
const std::string MICROPHONE_BLOCKED_CALLBACK_NAME = "micBlockStatusChanged";

class NapiAudioManagerCallback : public AudioManagerDeviceChangeCallback,
    public AudioManagerMicrophoneBlockedCallback {
public:
    static bool IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback);

    explicit NapiAudioManagerCallback(napi_env env);
    virtual ~NapiAudioManagerCallback();
    void SaveCallbackReference(const std::string &callbackName, napi_value args);
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override;
    void OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo) override;
    void SaveMicrophoneBlockedCallbackReference(napi_value callback);
    void RemoveMicrophoneBlockedCallbackReference(napi_env env, napi_value callback);
    void RemoveAllMicrophoneBlockedCallback();
    int32_t GetMicrophoneBlockedCbListSize();
    int32_t GetAudioManagerDeviceChangeCbListSize();
    void SaveAudioManagerDeviceChangeCbRef(DeviceFlag deviceFlag, napi_value callback);
    void RemoveAudioManagerDeviceChangeCbRef(napi_env env, napi_value callback);
    void RemoveAllAudioManagerDeviceChangeCb();

    void SaveRoutingManagerDeviceChangeCbRef(DeviceFlag deviceFlag, napi_value callback);
    void RemoveRoutingManagerDeviceChangeCbRef(napi_env env, napi_value callback);
    void RemoveAllRoutingManagerDeviceChangeCb();
    int32_t GetRoutingManagerDeviceChangeCbListSize();
    void CreateMicBlockedTsfn(napi_env env);
    bool GetMicBlockedTsfnFlag();
    void CreateDevChgTsfn(napi_env env);
    bool GetDevChgTsfnFlag();

private:
    struct AudioManagerJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        DeviceChangeAction deviceChangeAction;
        MicrophoneBlockedInfo microphoneBlockedInfo;
    };

    void OnJsCallbackDeviceChange(std::unique_ptr<AudioManagerJsCallback> &jsCb);
    void OnJsCallbackMicrophoneBlocked(std::unique_ptr<AudioManagerJsCallback> &jsCb);
    static void MicrophoneBlockedTsfnFinalize(napi_env env, void *data, void *hint);
    static void SafeJsCallbackMicrophoneBlockedWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void SafeJsCallbackDeviceChangeWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void DeviceChangeTsfnFinalize(napi_env env, void *data, void *hint);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> deviceChangeCallback_ = nullptr;
    std::shared_ptr<AutoRef> onMicroPhoneBlockedCallback_ = nullptr;
    std::list<std::pair<std::shared_ptr<AutoRef>, DeviceFlag>> audioManagerDeviceChangeCbList_;
    std::list<std::pair<std::shared_ptr<AutoRef>, DeviceFlag>> routingManagerDeviceChangeCbList_;
    std::list<std::shared_ptr<AutoRef>> microphoneBlockedCbList_;
    bool regAmMicBlockedTsfn_ = false;
    bool regAmDevChgTsfn_ = false;
    napi_threadsafe_function amMicBlockedTsfn_ = nullptr;
    napi_threadsafe_function amDevChgTsfn_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif /* NAPI_AUDIO_MANAGER_CALLBACK_H */