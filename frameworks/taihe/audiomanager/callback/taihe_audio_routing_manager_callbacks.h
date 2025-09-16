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
#ifndef TAIHE_AUDIO_ROUTING_MANAGER_CALLBACK_H
#define TAIHE_AUDIO_ROUTING_MANAGER_CALLBACK_H

#include <mutex>
#include <thread>
#include "event_handler.h"
#include "audio_system_manager.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "taihe_param_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string PREFERRED_OUTPUT_DEVICE_CALLBACK_NAME = "preferredOutputDeviceChangeForRendererInfo";
const std::string PREFER_OUTPUT_DEVICE_CALLBACK_NAME = "preferOutputDeviceChangeForRendererInfo";
const std::string PREFERRED_INPUT_DEVICE_CALLBACK_NAME  = "preferredInputDeviceChangeForCapturerInfo";

class TaiheAudioPreferredInputDeviceChangeCallback :
    public OHOS::AudioStandard::AudioPreferredInputDeviceChangeCallback,
    public std::enable_shared_from_this<TaiheAudioPreferredInputDeviceChangeCallback> {
public:
    explicit TaiheAudioPreferredInputDeviceChangeCallback();
    virtual ~TaiheAudioPreferredInputDeviceChangeCallback();
    void SaveCallbackReference(std::shared_ptr<uintptr_t> &callback);
    void OnPreferredInputDeviceUpdated(const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>>
        &desc) override;

    bool ContainSameJsCallback(std::shared_ptr<uintptr_t> callback);

private:
    struct AudioActiveInputDeviceChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> desc;
    };

    void OnJsCallbackActiveInputDeviceChange(std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> &jsCb);
    static void SafeJsCallbackActiveInputDeviceChangeWork(AudioActiveInputDeviceChangeJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> callback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};

class TaiheAudioPreferredOutputDeviceChangeCallback :
    public OHOS::AudioStandard::AudioPreferredOutputDeviceChangeCallback,
    public std::enable_shared_from_this<TaiheAudioPreferredOutputDeviceChangeCallback> {
public:
    explicit TaiheAudioPreferredOutputDeviceChangeCallback();
    virtual ~TaiheAudioPreferredOutputDeviceChangeCallback();
    void SaveCallbackReference(std::shared_ptr<uintptr_t> &callback);
    void OnPreferredOutputDeviceUpdated(
        const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &desc) override;
    bool ContainSameJsCallback(std::shared_ptr<uintptr_t> callback);
private:
    struct AudioActiveOutputDeviceChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> desc;
    };

    void OnJsCallbackActiveOutputDeviceChange(std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> &jsCb);
    static void SafeJsCallbackActiveOutputDeviceChangeWork(AudioActiveOutputDeviceChangeJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> callback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_ROUTING_MANAGER_CALLBACK_H