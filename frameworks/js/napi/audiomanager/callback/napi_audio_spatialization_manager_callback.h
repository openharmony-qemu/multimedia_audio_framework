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
#ifndef AUDIO_SPATIALIZATION_MANAGER_CALLBACK_NAPI_H
#define AUDIO_SPATIALIZATION_MANAGER_CALLBACK_NAPI_H

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_work.h"
#include "napi_audio_manager.h"
#include "audio_system_manager.h"
#include "audio_spatialization_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME = "spatializationEnabledChange";
const std::string SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME =
    "spatializationEnabledChangeForAnyDevice";
const std::string HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME = "headTrackingEnabledChange";
const std::string HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME =
    "headTrackingEnabledChangeForAnyDevice";
class NapiAudioSpatializationEnabledChangeCallback : public AudioSpatializationEnabledChangeCallback {
public:
    explicit NapiAudioSpatializationEnabledChangeCallback(napi_env env);
    virtual ~NapiAudioSpatializationEnabledChangeCallback();
    void SaveSpatializationEnabledChangeCallbackReference(napi_value args, const std::string cbName);
    void RemoveSpatializationEnabledChangeCallbackReference(napi_env env, napi_value args, const std::string cbName);
    void RemoveAllSpatializationEnabledChangeCallbackReference(const std::string cbName);
    int32_t GetSpatializationEnabledChangeCbListSize(const std::string cbName);
    void OnSpatializationEnabledChange(const bool &enabled) override;
    void OnSpatializationEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;

private:
    struct AudioSpatializationEnabledJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        sptr<AudioDeviceDescriptor> deviceDescriptor;
        bool enabled;
    };

    void OnJsCallbackSpatializationEnabled(std::unique_ptr<AudioSpatializationEnabledJsCallback> &jsCb);
    static void WorkCallbackInterruptDone(uv_work_t *work, int status);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbList_;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbForAnyDeviceList_;
    static bool onSpatializationEnabledChangeFlag_;
};

class NapiAudioHeadTrackingEnabledChangeCallback : public AudioHeadTrackingEnabledChangeCallback {
public:
    explicit NapiAudioHeadTrackingEnabledChangeCallback(napi_env env);
    virtual ~NapiAudioHeadTrackingEnabledChangeCallback();
    void SaveHeadTrackingEnabledChangeCallbackReference(napi_value args, const std::string cbName);
    void RemoveHeadTrackingEnabledChangeCallbackReference(napi_env env, napi_value args, const std::string cbName);
    void RemoveAllHeadTrackingEnabledChangeCallbackReference(const std::string cbName);
    int32_t GetHeadTrackingEnabledChangeCbListSize(const std::string cbName);
    void OnHeadTrackingEnabledChange(const bool &enabled) override;
    void OnHeadTrackingEnabledChangeForAnyDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;

private:
    struct AudioHeadTrackingEnabledJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        sptr<AudioDeviceDescriptor> deviceDescriptor;
        bool enabled;
    };

    void OnJsCallbackHeadTrackingEnabled(std::unique_ptr<AudioHeadTrackingEnabledJsCallback> &jsCb);
    static void WorkCallbackInterruptDone(uv_work_t *work, int status);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> headTrackingEnabledChangeCbList_;
    std::list<std::shared_ptr<AutoRef>> headTrackingEnabledChangeCbForAnyDeviceList_;
    static bool onHeadTrackingEnabledChangeFlag_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_SPATIALIZATION_MANAGER_CALLBACK_NAPI_H */