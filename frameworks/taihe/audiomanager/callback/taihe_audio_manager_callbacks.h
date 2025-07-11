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

#ifndef TAIHE_AUDIO_MANAGER_CALLBACKS_H
#define TAIHE_AUDIO_MANAGER_CALLBACKS_H

#include <thread>
#include "event_handler.h"
#include "audio_system_manager.h"
#include "taihe_work.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string DEVICE_CHANGE_CALLBACK_NAME = "deviceChange";
const std::string MIC_STATE_CHANGE_CALLBACK_NAME = "micStateChange";
const std::string MICROPHONE_BLOCKED_CALLBACK_NAME = "micBlockStatusChanged";

class TaiheAudioManagerCallback : public OHOS::AudioStandard::AudioManagerDeviceChangeCallback,
    public OHOS::AudioStandard::AudioManagerMicrophoneBlockedCallback,
    public std::enable_shared_from_this<TaiheAudioManagerCallback> {
public:
    static bool IsSameCallback(std::shared_ptr<uintptr_t> &callback, std::shared_ptr<uintptr_t> &listCallback);

    explicit TaiheAudioManagerCallback();
    virtual ~TaiheAudioManagerCallback();
    void OnDeviceChange(const OHOS::AudioStandard::DeviceChangeAction &deviceChangeAction) override;
    void OnMicrophoneBlocked(const OHOS::AudioStandard::MicrophoneBlockedInfo &microphoneBlockedInfo) override;
    void SaveMicrophoneBlockedCallbackReference(std::shared_ptr<uintptr_t> &callback);
    void RemoveMicrophoneBlockedCallbackReference(std::shared_ptr<uintptr_t> callback);
    void RemoveAllMicrophoneBlockedCallback();
    int32_t GetMicrophoneBlockedCbListSize();

    void SaveRoutingManagerDeviceChangeCbRef(OHOS::AudioStandard::DeviceFlag deviceFlag,
        std::shared_ptr<uintptr_t> callback);
    void RemoveRoutingManagerDeviceChangeCbRef(std::shared_ptr<uintptr_t> callback);
    void RemoveAllRoutingManagerDeviceChangeCb();
    int32_t GetRoutingManagerDeviceChangeCbListSize();

private:
    struct AudioManagerJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::DeviceChangeAction deviceChangeAction;
        OHOS::AudioStandard::MicrophoneBlockedInfo microphoneBlockedInfo;
    };

    void OnJsCallbackMicrophoneBlocked(std::unique_ptr<AudioManagerJsCallback> &jsCb);
    static void SafeJsCallbackMicrophoneBlockedWork(AudioManagerJsCallback *event);
    void OnJsCallbackDeviceChange(std::unique_ptr<AudioManagerJsCallback> &jsCb);
    static void SafeJsCallbackDeviceChangeWork(AudioManagerJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> deviceChangeCallback_ = nullptr;
    std::shared_ptr<AutoRef> onMicroPhoneBlockedCallback_ = nullptr;
    std::list<std::pair<std::shared_ptr<AutoRef>, OHOS::AudioStandard::DeviceFlag>> audioManagerDeviceChangeCbList_;
    std::list<std::pair<std::shared_ptr<AutoRef>, OHOS::AudioStandard::DeviceFlag>> routingManagerDeviceChangeCbList_;
    std::list<std::shared_ptr<AutoRef>> microphoneBlockedCbList_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_MANAGER_CALLBACKS_H
