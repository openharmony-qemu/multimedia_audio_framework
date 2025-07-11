/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_SPATIALIZATION_MANAGER_CALLBACK_H
#define TAIHE_AUDIO_SPATIALIZATION_MANAGER_CALLBACK_H

#include "event_handler.h"
#include "audio_system_manager.h"
#include "audio_spatialization_manager.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME = "spatializationEnabledChange";
const std::string SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME =
    "spatializationEnabledChangeForAnyDevice";
const std::string HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME = "headTrackingEnabledChange";
const std::string HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME =
    "headTrackingEnabledChangeForAnyDevice";

class TaiheAudioSpatializationEnabledChangeCallback :
    public OHOS::AudioStandard::AudioSpatializationEnabledChangeCallback,
    public std::enable_shared_from_this<TaiheAudioSpatializationEnabledChangeCallback> {
public:
    explicit TaiheAudioSpatializationEnabledChangeCallback();
    virtual ~TaiheAudioSpatializationEnabledChangeCallback();
    void SaveSpatializationEnabledChangeCallbackReference(const std::string &callbackName,
        std::shared_ptr<uintptr_t> callback);
    void RemoveSpatializationEnabledChangeCallbackReference(const std::string &callbackName,
        std::shared_ptr<uintptr_t> callback);
    void RemoveAllSpatializationEnabledChangeCallbackReference(const std::string &callbackName);
    int32_t GetSpatializationEnabledChangeCbListSize(const std::string &callbackName);
    void OnSpatializationEnabledChange(const bool &enabled) override;
    void OnSpatializationEnabledChangeForAnyDevice(const std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>
        &deviceDescriptor, const bool &enabled) override;

private:
    struct AudioSpatializationEnabledJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> deviceDescriptor;
        std::string callbackName = "unknown";
        bool enabled;
    };

    void OnJsCallbackSpatializationEnabled(std::unique_ptr<AudioSpatializationEnabledJsCallback> &jsCb);
    static void SafeJsCallbackSpatializationEnabledWork(AudioSpatializationEnabledJsCallback *event);

    std::mutex mutex_;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbList_;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbForAnyDeviceList_;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbForCurrentDeviceList_;
    static bool onSpatializationEnabledChangeFlag_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};

class TaiheAudioCurrentSpatializationEnabledChangeCallback : public
    OHOS::AudioStandard::AudioSpatializationEnabledChangeForCurrentDeviceCallback,
    public std::enable_shared_from_this<TaiheAudioCurrentSpatializationEnabledChangeCallback> {
public:
    explicit TaiheAudioCurrentSpatializationEnabledChangeCallback();
    virtual ~TaiheAudioCurrentSpatializationEnabledChangeCallback();
    void SaveCurrentSpatializationEnabledChangeCallbackReference(std::shared_ptr<uintptr_t> callback);
    void RemoveCurrentSpatializationEnabledChangeCallbackReference(std::shared_ptr<uintptr_t> callback);
    void RemoveAllCurrentSpatializationEnabledChangeCallbackReference();
    int32_t GetCurrentSpatializationEnabledChangeCbListSize();
    void OnSpatializationEnabledChangeForCurrentDevice(const bool &enabled) override;

private:
    struct AudioSpatializationEnabledForCurrentDeviceJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        bool enabled;
    };

    void OnJsCallbackSpatializationEnabledForCurrentDevice(
        std::unique_ptr<AudioSpatializationEnabledForCurrentDeviceJsCallback> &jsCb);
    static void SafeJsCallbackSpatializationEnabledForCurrentDeviceWork(
        AudioSpatializationEnabledForCurrentDeviceJsCallback *event);

    std::mutex mutex_;
    std::list<std::shared_ptr<AutoRef>> spatializationEnabledChangeCbForCurrentDeviceList_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};

class TaiheAudioHeadTrackingEnabledChangeCallback : public OHOS::AudioStandard::AudioHeadTrackingEnabledChangeCallback,
    public std::enable_shared_from_this<TaiheAudioHeadTrackingEnabledChangeCallback> {
public:
    explicit TaiheAudioHeadTrackingEnabledChangeCallback();
    virtual ~TaiheAudioHeadTrackingEnabledChangeCallback();
    void SaveHeadTrackingEnabledChangeCallbackReference(const std::string &callbackName,
        std::shared_ptr<uintptr_t> callback);
    void RemoveHeadTrackingEnabledChangeCallbackReference(const std::string &callbackName,
        std::shared_ptr<uintptr_t> callback);
    void RemoveAllHeadTrackingEnabledChangeCallbackReference(const std::string &callbackName);
    int32_t GetHeadTrackingEnabledChangeCbListSize(const std::string &callbackName);
    void OnHeadTrackingEnabledChange(const bool &enabled) override;
    void OnHeadTrackingEnabledChangeForAnyDevice(const std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>
        &deviceDescriptor, const bool &enabled) override;

private:
    struct AudioHeadTrackingEnabledJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> deviceDescriptor;
        std::string callbackName = "unknown";
        bool enabled;
    };

    void OnJsCallbackHeadTrackingEnabled(std::unique_ptr<AudioHeadTrackingEnabledJsCallback> &jsCb);
    static void SafeJsCallbackHeadTrackingEnabledWork(AudioHeadTrackingEnabledJsCallback *event);

    std::mutex mutex_;
    std::list<std::shared_ptr<AutoRef>> headTrackingEnabledChangeCbList_;
    std::list<std::shared_ptr<AutoRef>> headTrackingEnabledChangeCbForAnyDeviceList_;
    static bool onHeadTrackingEnabledChangeFlag_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_SPATIALIZATION_MANAGER_CALLBACK_H