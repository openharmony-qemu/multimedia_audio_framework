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
#ifndef LOG_TAG
#define LOG_TAG "TaiheAudioPreferredInputDeviceChangeCallback"
#endif

#include "taihe_audio_routing_manager_callbacks.h"

namespace ANI::Audio {
TaiheAudioPreferredInputDeviceChangeCallback::TaiheAudioPreferredInputDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioPreferredInputDeviceChangeCallback: instance create");
}

TaiheAudioPreferredInputDeviceChangeCallback::~TaiheAudioPreferredInputDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioPreferredInputDeviceChangeCallback: instance destroy");
}

bool TaiheAudioPreferredInputDeviceChangeCallback::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    if (callback_ == nullptr) {
        return false;
    }
    return TaiheParamUtils::IsSameRef(callback, callback_->cb_);
}

void TaiheAudioPreferredInputDeviceChangeCallback::SaveCallbackReference(std::shared_ptr<uintptr_t> &callback)
{
    CHECK_AND_RETURN_LOG(callback != nullptr, "SaveCallbackReference: creating reference for callback fail");
    callback_ = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "Memory allocation failed!!");

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioPreferredInputDeviceChangeCallback::OnPreferredInputDeviceUpdated(
    const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &desc)
{
    std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> cb =
        std::make_unique<AudioActiveInputDeviceChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");

    cb->callback = callback_;
    cb->callbackName = PREFERRED_INPUT_DEVICE_CALLBACK_NAME;
    cb->desc = desc;
    OnJsCallbackActiveInputDeviceChange(cb);
    return;
}

void TaiheAudioPreferredInputDeviceChangeCallback::OnJsCallbackActiveInputDeviceChange(
    std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackActiveInputDeviceChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioActiveInputDeviceChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackActiveInputDeviceChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnPreferredInputDeviceChangeForCapturerInfo", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioPreferredInputDeviceChangeCallback::SafeJsCallbackActiveInputDeviceChangeWork(
    AudioActiveInputDeviceChangeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackActiveInputDeviceChange: no memory");
    std::shared_ptr<AudioActiveInputDeviceChangeJsCallback> safeContext(
        static_cast<AudioActiveInputDeviceChangeJsCallback*>(event),
        [](AudioActiveInputDeviceChangeJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(array_view<AudioDeviceDescriptor>)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(array_view<AudioDeviceDescriptor>)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetDeviceDescriptors(event->desc));
    } while (0);
}

TaiheAudioPreferredOutputDeviceChangeCallback::TaiheAudioPreferredOutputDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioPreferredOutputDeviceChangeCallback: instance create");
}

TaiheAudioPreferredOutputDeviceChangeCallback::~TaiheAudioPreferredOutputDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioPreferredOutputDeviceChangeCallback: instance destroy");
}

bool TaiheAudioPreferredOutputDeviceChangeCallback::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    if (callback_ == nullptr) {
        return false;
    }
    return TaiheParamUtils::IsSameRef(callback, callback_->cb_);
}

void TaiheAudioPreferredOutputDeviceChangeCallback::SaveCallbackReference(std::shared_ptr<uintptr_t> &callback)
{
    CHECK_AND_RETURN_LOG(callback != nullptr, "SaveCallbackReference: creating reference for callback fail");
    callback_ = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(callback_ != nullptr, "Memory allocation failed!!");

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioPreferredOutputDeviceChangeCallback::OnPreferredOutputDeviceUpdated(
    const std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &desc)
{
    std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> cb =
        std::make_unique<AudioActiveOutputDeviceChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");

    cb->callback = callback_;
    cb->callbackName = PREFERRED_INPUT_DEVICE_CALLBACK_NAME;
    cb->desc = desc;
    OnJsCallbackActiveOutputDeviceChange(cb);
    return;
}

void TaiheAudioPreferredOutputDeviceChangeCallback::OnJsCallbackActiveOutputDeviceChange(
    std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackActiveOutputDeviceChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioActiveOutputDeviceChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackActiveOutputDeviceChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnPreferredInputDeviceChangeForCapturerInfo", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioPreferredOutputDeviceChangeCallback::SafeJsCallbackActiveOutputDeviceChangeWork(
    AudioActiveOutputDeviceChangeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackActiveInputDeviceChange: no memory");
    std::shared_ptr<AudioActiveOutputDeviceChangeJsCallback> safeContext(
        static_cast<AudioActiveOutputDeviceChangeJsCallback*>(event),
        [](AudioActiveOutputDeviceChangeJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(array_view<AudioDeviceDescriptor>)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(array_view<AudioDeviceDescriptor>)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetDeviceDescriptors(event->desc));
    } while (0);
}
} // namespace ANI::Audio