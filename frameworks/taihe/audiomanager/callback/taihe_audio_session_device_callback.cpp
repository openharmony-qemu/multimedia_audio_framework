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
#ifndef LOG_TAG
#define LOG_TAG "NapiAudioSessionDeviceCallback"
#endif
#include <thread>
#include "taihe_audio_session_device_callback.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioSessionDeviceCallback::TaiheAudioSessionDeviceCallback()
{
    AUDIO_INFO_LOG("TaiheAudioSessionDeviceCallback::Constructor");
}

TaiheAudioSessionDeviceCallback::~TaiheAudioSessionDeviceCallback()
{
    AUDIO_INFO_LOG("TaiheAudioSessionDeviceCallback::Destructor");
}

void TaiheAudioSessionDeviceCallback::OnAudioSessionCurrentDeviceChanged(
    const OHOS::AudioStandard::CurrentOutputDeviceChangedEvent &deviceEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionCurrentDeviceChanged is called changeReason=%{public}d, "
        "recommendedAction=%{public}d", deviceEvent.changeReason, deviceEvent.recommendedAction);
    CHECK_AND_RETURN_LOG(audioSessionDeviceJsCallback_ != nullptr,
        "OnAudioSessionCurrentDeviceChanged:No JS callback registered return");
    std::unique_ptr<AudioSessionDeviceJsCallback> cb = std::make_unique<AudioSessionDeviceJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioSessionDeviceJsCallback_;
    cb->callbackName = AUDIO_SESSION_DEVICE_CALLBACK_NAME;
    cb->audioSessionDeviceEvent.devices = deviceEvent.devices;
    cb->audioSessionDeviceEvent.changeReason = deviceEvent.changeReason;
    cb->audioSessionDeviceEvent.recommendedAction = deviceEvent.recommendedAction;

    return OnJsCallbackAudioSessionDevice(cb);
}

void TaiheAudioSessionDeviceCallback::SaveCallbackReference(const std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    audioSessionDeviceJsCallback_ = cb;

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioSessionDeviceCallback::SafeJsCallbackAudioSessionDeviceWork(AudioSessionDeviceJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackSystemVolumeChange: no memory");
    std::shared_ptr<AudioSessionDeviceJsCallback> safeContext(
        static_cast<AudioSessionDeviceJsCallback*>(event),
        [](AudioSessionDeviceJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    AUDIO_INFO_LOG("SafeJsCallbackAudioSessionDeviceWork: safe js callback working.");

    do {
        std::shared_ptr<taihe::callback<void(CurrentOutputDeviceChangedEvent const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(CurrentOutputDeviceChangedEvent const&)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s fail to call SetaudioSessionDevice callback",
            request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueCurrentOutputDeviceChangedEvent(event->audioSessionDeviceEvent));
    } while (0);
}

void TaiheAudioSessionDeviceCallback::OnJsCallbackAudioSessionDevice(
    std::unique_ptr<AudioSessionDeviceJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("TaiheAudioSessionDeviceCallback: OnJsCallbackAudioSessionDevice: jsCb.get() is null");
        return;
    }
    AudioSessionDeviceJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAudioSessionDeviceWork(event);
        }
    };
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    mainHandler_->PostTask(task, "OnCurrentOutputDeviceChanged", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

bool TaiheAudioSessionDeviceCallback::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    return TaiheParamUtils::IsSameRef(callback, callback_);
}
} // namespace ANI::Audio