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
#ifndef LOG_TAG
#define LOG_TAG "TaiheAudioSessionInputDeviceCallback"
#endif
#include <thread>
#include "taihe_audio_session_input_device_callback.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioSessionInputDeviceCallback::TaiheAudioSessionInputDeviceCallback()
{
    AUDIO_INFO_LOG("TaiheAudioSessionInputDeviceCallback::Constructor");
}

TaiheAudioSessionInputDeviceCallback::~TaiheAudioSessionInputDeviceCallback()
{
    AUDIO_INFO_LOG("TaiheAudioSessionInputDeviceCallback::Destructor");
}

void TaiheAudioSessionInputDeviceCallback::OnAudioSessionCurrentInputDeviceChanged(
    const OHOS::AudioStandard::CurrentInputDeviceChangedEvent &deviceEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionCurrentInputDeviceChanged is called changeReason=%{public}d",
        deviceEvent.changeReason);
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(audioSessionInputDeviceJsCallback_ != nullptr,
        "OnAudioSessionCurrentInputDeviceChanged:No JS callback registered return");

    std::unique_ptr<AudioSessionInputDeviceJsCallback> cb = std::make_unique<AudioSessionInputDeviceJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioSessionInputDeviceJsCallback_;
    cb->callbackName = AUDIO_SESSION_INPUT_DEVICE_CALLBACK_NAME;
    cb->audioSessionDeviceEvent.devices = deviceEvent.devices;
    cb->audioSessionDeviceEvent.changeReason = deviceEvent.changeReason;

    return OnJsCallbackAudioSessionInputDevice(cb);
}

void TaiheAudioSessionInputDeviceCallback::SaveCallbackReference(const std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    audioSessionInputDeviceJsCallback_ = cb;

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioSessionInputDeviceCallback::SafeJsCallbackAudioSessionInputDeviceWork(
    AudioSessionInputDeviceJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAudioSessionInputDevice: no memory");
    std::shared_ptr<AudioSessionInputDeviceJsCallback> safeContext(
        static_cast<AudioSessionInputDeviceJsCallback*>(event),
        [](AudioSessionInputDeviceJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    AUDIO_INFO_LOG("SafeJsCallbackAudioSessionInputDeviceWork: safe js callback working.");

    do {
        std::shared_ptr<taihe::callback<void(CurrentInputDeviceChangedEvent const& data)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(CurrentInputDeviceChangedEvent const& data)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueCurrentInputDeviceChangedEvent(event->audioSessionDeviceEvent));
    } while (0);
}

void TaiheAudioSessionInputDeviceCallback::OnJsCallbackAudioSessionInputDevice(
    std::unique_ptr<AudioSessionInputDeviceJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("TaiheAudioSessionInputDeviceCallback: OnJsCallbackAudioSessionInputDevice: jsCb.get() is null");
        return;
    }
    AudioSessionInputDeviceJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAudioSessionInputDeviceWork(event);
        }
    };
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    mainHandler_->PostTask(task, "OnCurrentInputDeviceChanged", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

bool TaiheAudioSessionInputDeviceCallback::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    return TaiheParamUtils::IsSameRef(callback, callback_);
}
} // namespace ANI::Audio