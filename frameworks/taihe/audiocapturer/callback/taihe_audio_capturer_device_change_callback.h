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

#ifndef TAIHE_AUDIO_CAPTURER_DEVICE_CHANGE_CALLBACK_H
#define TAIHE_AUDIO_CAPTURER_DEVICE_CHANGE_CALLBACK_H

#include "audio_capturer.h"
#include "event_handler.h"
#include "taihe_audio_capturer_callback_inner.h"
#include "taihe_audio_capturer_callbacks.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

class TaiheAudioCapturerDeviceChangeCallback : public OHOS::AudioStandard::AudioCapturerDeviceChangeCallback,
    public TaiheAudioCapturerCallbackInner,
    public std::enable_shared_from_this<TaiheAudioCapturerDeviceChangeCallback> {
public:
    explicit TaiheAudioCapturerDeviceChangeCallback();
    ~TaiheAudioCapturerDeviceChangeCallback() override;
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback) override;
    void RemoveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback) override;
    void OnStateChange(const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo) override;
    bool ContainSameJsCallback(std::shared_ptr<uintptr_t> callback);
    bool CheckIfTargetCallbackName(const std::string &callbackName) override;
protected:
    std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) override;

private:
    struct AudioCapturerDeviceChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        array<AudioDeviceDescriptor> deviceInfo_ = array<AudioDeviceDescriptor>(nullptr, 0);
    };

    void OnJsCallbackCapturerDeviceInfo(std::unique_ptr<AudioCapturerDeviceChangeJsCallback> &jsCb);
    static void SafeJsCallbackCapturerDeviceInfoWork(AudioCapturerDeviceChangeJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<uintptr_t> callback_ = nullptr;
    std::shared_ptr<AutoRef> callbackPtr_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_CAPTURER_DEVICE_CHANGE_CALLBACK_H