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

#ifndef AUDIO_ROUTING_AVAILABLE_DEVICECHANGE_CALLBACK_NAPI_H
#define AUDIO_ROUTING_AVAILABLE_DEVICECHANGE_CALLBACK_NAPI_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AVAILABLE_DEVICE_CHANGE_CALLBACK_NAME = "availableDeviceChange";

class NapiAudioRountingAvailableDeviceChangeCallback : public AudioManagerAvailableDeviceChangeCallback {
public:
    explicit NapiAudioRountingAvailableDeviceChangeCallback(napi_env env);
    virtual ~NapiAudioRountingAvailableDeviceChangeCallback();
    void SaveCallbackReference(const std::string &callbackName, napi_value callback);
    void OnAvailableDeviceChange(const AudioDeviceUsage usage, const DeviceChangeAction &deviceChangeAction) override;

    void SaveRoutingAvailbleDeviceChangeCbRef(AudioDeviceUsage usage, napi_value callback);
    void RemoveRoutingAvailbleDeviceChangeCbRef(napi_env env, napi_value callback);
    void RemoveAllRoutinAvailbleDeviceChangeCb();
    int32_t GetRoutingAvailbleDeviceChangeCbListSize();

private:
    struct AudioRountingJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        DeviceChangeAction deviceChangeAction;
    };

    void OnJsCallbackAvailbleDeviceChange(std::unique_ptr<AudioRountingJsCallback> &jsCb);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::shared_ptr<AutoRef> deviceChangeCallback_ = nullptr;
    std::list<std::pair<std::shared_ptr<AutoRef>, AudioDeviceUsage>> availableDeviceChangeCbList_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_ROUTING_AVAILABLE_DEVICECHANGE_CALLBACK_NAPI_H */