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

#ifndef TAIHE_AUDIO_CAPTURER_CALLBACKS_H
#define TAIHE_AUDIO_CAPTURER_CALLBACKS_H

#include "audio_capturer.h"
#include "event_handler.h"
#include "taihe_audio_capturer_callback_inner.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
const std::string INTERRUPT_CALLBACK_NAME = "interrupt";
const std::string AUDIO_INTERRUPT_CALLBACK_NAME = "audioInterrupt";
const std::string STATE_CHANGE_CALLBACK_NAME = "stateChange";
const std::string MARK_REACH_CALLBACK_NAME = "markReach";
const std::string PERIOD_REACH_CALLBACK_NAME = "periodReach";
const std::string INPUTDEVICE_CHANGE_CALLBACK_NAME = "inputDeviceChange";
const std::string AUDIO_CAPTURER_CHANGE_CALLBACK_NAME = "audioCapturerChange";
const std::string READ_DATA_CALLBACK_NAME = "readData";

class TaiheAudioCapturerCallback : public OHOS::AudioStandard::AudioCapturerCallback,
    public TaiheAudioCapturerCallbackInner, public std::enable_shared_from_this<TaiheAudioCapturerCallback> {
public:
    explicit TaiheAudioCapturerCallback();
    ~TaiheAudioCapturerCallback() override;
    void OnInterrupt(const OHOS::AudioStandard::InterruptEvent &interruptEvent) override;
    void OnStateChange(const OHOS::AudioStandard::CapturerState state) override;
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback) override;
    void RemoveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback) override;
    bool CheckIfTargetCallbackName(const std::string &callbackName) override;
protected:
    std::shared_ptr<AutoRef> GetCallback(const std::string &callbackName) override;

private:
    struct AudioCapturerJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::InterruptEvent interruptEvent;
        OHOS::AudioStandard::CapturerState state;
    };

    void OnJsCallbackInterrupt(std::unique_ptr<AudioCapturerJsCallback> &jsCb);
    void OnJsCallbackStateChange(std::unique_ptr<AudioCapturerJsCallback> &jsCb);
    static void SafeJsCallbackInterruptWork(AudioCapturerJsCallback *event);
    static void SafeJsCallbackStateChangeWork(AudioCapturerJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> interruptCallback_ = nullptr;
    std::shared_ptr<AutoRef> stateChangeCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio

#endif // TAIHE_AUDIO_CAPTURER_CALLBACKS_H
