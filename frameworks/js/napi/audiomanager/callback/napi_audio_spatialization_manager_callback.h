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
const std::string SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE_CALLBACK_NAME =
    "spatializationEnabledChangeForCurrentDevice";
const std::string HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME = "headTrackingEnabledChange";
const std::string HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME =
    "headTrackingEnabledChangeForAnyDevice";
class NapiAudioSpatializationEnabledChangeCallback : public AudioSpatializationEnabledChangeCallback {
public:
    explicit NapiAudioSpatializationEnabledChangeCallback(napi_env env);
    virtual ~NapiAudioSpatializationEnabledChangeCallback();
    void SaveSpatializationEnabledChangeCallbackReference(napi_value args, const std::string &cbName);
    void RemoveSpatializationEnabledChangeCallbackReference(napi_env env, napi_value args, const std::string &cbName);
    void RemoveAllSpatializationEnabledChangeCallbackReference(const std::string &cbName);
    int32_t GetSpatializationEnabledChangeCbListSize(const std::string &cbName);
    void OnSpatializationEnabledChange(const bool &enabled) override;
    void OnSpatializationEnabledChangeForAnyDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;
    void CreateSpatEnableTsfn(napi_env env);
    bool GetSpatEnableTsfnFlag();

private:
    struct AudioSpatializationEnabledJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor;
        std::string callbackName = "unknown";
        bool enabled;
    };

    void OnJsCallbackSpatializationEnabled(std::unique_ptr<AudioSpatializationEnabledJsCallback> &jsCb);
    static void SafeJsCallbackSpatializationEnabledWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void SpatializationEnabledTsfnFinalize(napi_env env, void *data, void *hint);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbList_;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbForAnyDeviceList_;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbForCurrentDeviceList_;
    static bool onSpatializationEnabledChangeFlag_;
    bool regAmSpatEnable_ = false;
    napi_threadsafe_function amSpatEnableTsfn_ = nullptr;
};

class NapiAudioCurrentSpatializationEnabledChangeCallback :
    public AudioSpatializationEnabledChangeForCurrentDeviceCallback {
public:
    explicit NapiAudioCurrentSpatializationEnabledChangeCallback(napi_env env);
    virtual ~NapiAudioCurrentSpatializationEnabledChangeCallback();
    void SaveCurrentSpatializationEnabledChangeCallbackReference(napi_value args, const std::string &cbName);
    void RemoveCurrentSpatializationEnabledChangeCallbackReference(napi_env env, napi_value args,
        const std::string &cbName);
    void RemoveAllCurrentSpatializationEnabledChangeCallbackReference(const std::string &cbName);
    int32_t GetCurrentSpatializationEnabledChangeCbListSize(const std::string &cbName);
    void OnSpatializationEnabledChangeForCurrentDevice(const bool &enabled) override;
    void CreateCurrentSpatEnableForCurrentDeviceTsfn(napi_env env);
    bool GetCurrentSpatEnableForCurrentDeviceTsfnFlag();

private:
    struct AudioSpatializationEnabledForCurrentDeviceJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        bool enabled;
    };

    void OnJsCallbackSpatializationEnabledForCurrentDevice(
        std::unique_ptr<AudioSpatializationEnabledForCurrentDeviceJsCallback> &jsCb);
    static void SafeJsCallbackSpatializationEnabledForCurrentDeviceWork(napi_env env, napi_value js_cb, void *context,
        void *data);
    static void SpatializationEnabledForCurrentDeviceTsfnFinalize(napi_env env, void *data, void *hint);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbForCurrentDeviceList_;
    bool regAmSpatEnableForCurrentDevice_ = false;
    napi_threadsafe_function amSpatEnableForCurrentDeviceTsfn_ = nullptr;
};

class NapiAudioHeadTrackingEnabledChangeCallback : public AudioHeadTrackingEnabledChangeCallback {
public:
    explicit NapiAudioHeadTrackingEnabledChangeCallback(napi_env env);
    virtual ~NapiAudioHeadTrackingEnabledChangeCallback();
    void SaveHeadTrackingEnabledChangeCallbackReference(napi_value args, const std::string &cbName);
    void RemoveHeadTrackingEnabledChangeCallbackReference(napi_env env, napi_value args, const std::string &cbName);
    void RemoveAllHeadTrackingEnabledChangeCallbackReference(const std::string &cbName);
    int32_t GetHeadTrackingEnabledChangeCbListSize(const std::string &cbName);
    void OnHeadTrackingEnabledChange(const bool &enabled) override;
    void OnHeadTrackingEnabledChangeForAnyDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const bool &enabled) override;
    void CreateHeadTrackingTsfn(napi_env env);
    bool GetHeadTrackingTsfnFlag();

private:
    struct AudioHeadTrackingEnabledJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor;
        std::string callbackName = "unknown";
        bool enabled;
    };

    void OnJsCallbackHeadTrackingEnabled(std::unique_ptr<AudioHeadTrackingEnabledJsCallback> &jsCb);
    static void SafeJsCallbackHeadTrackingEnabledWork(napi_env env, napi_value js_cb, void *context, void *data);
    static void HeadTrackingEnabledTsfnFinalize(napi_env env, void *data, void *hint);

    std::mutex mutex_;
    napi_env env_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> headTrackingEnabledChangeCbList_;
    std::list<std::shared_ptr<AutoRef>> headTrackingEnabledChangeCbForAnyDeviceList_;
    static bool onHeadTrackingEnabledChangeFlag_;
    bool regAmHeadTrkTsfn_ = false;
    napi_threadsafe_function amHeadTrkTsfn_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif /* AUDIO_SPATIALIZATION_MANAGER_CALLBACK_NAPI_H */