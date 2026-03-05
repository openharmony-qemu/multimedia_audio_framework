/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#ifndef TAIHE_AUDIO_SESSION_INPUT_DEVICE_CALLBACK_H
#define TAIHE_AUDIO_SESSION_INPUT_DEVICE_CALLBACK_H

#include "event_handler.h"
#include "audio_session_manager.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string AUDIO_SESSION_INPUT_DEVICE_CALLBACK_NAME = "currentInputDeviceChanged";

class TaiheAudioSessionInputDeviceCallback : public OHOS::AudioStandard::AudioSessionCurrentInputDeviceChangedCallback,
    public std::enable_shared_from_this<TaiheAudioSessionInputDeviceCallback> {
public:
    explicit TaiheAudioSessionInputDeviceCallback();
    virtual ~TaiheAudioSessionInputDeviceCallback();

    void OnAudioSessionCurrentInputDeviceChanged(
        const OHOS::AudioStandard::CurrentInputDeviceChangedEvent &deviceEvent);
    void SaveCallbackReference(const std::shared_ptr<uintptr_t> &callback);
    bool ContainSameJsCallback(std::shared_ptr<uintptr_t> callback);
    
private:
    struct AudioSessionInputDeviceJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::CurrentInputDeviceChangedEvent audioSessionDeviceEvent;
    };

    void OnJsCallbackAudioSessionInputDevice(std::unique_ptr<AudioSessionInputDeviceJsCallback> &jsCb);
    static void SafeJsCallbackAudioSessionInputDeviceWork(AudioSessionInputDeviceJsCallback *event);

    std::shared_ptr<AutoRef> audioSessionInputDeviceJsCallback_ = nullptr;
    std::shared_ptr<uintptr_t> callback_ = nullptr;
    std::mutex mutex_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif /* TAIHE_AUDIO_SESSION_INPUT_DEVICE_CALLBACK_H */