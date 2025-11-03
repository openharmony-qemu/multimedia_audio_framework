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
#define LOG_TAG "TaiheAudioCapturerDeviceChangeCallback"
#endif

#include "taihe_audio_capturer_device_change_callback.h"
#include <mutex>
#include <thread>
#include "taihe_audio_capturer_callbacks.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioCapturerDeviceChangeCallback::TaiheAudioCapturerDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("Instance create");
}

TaiheAudioCapturerDeviceChangeCallback::~TaiheAudioCapturerDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("Instance destroy");
}

void TaiheAudioCapturerDeviceChangeCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        callbackPtr_ = generatedCallback;
        CHECK_AND_RETURN_LOG(callbackPtr_ != nullptr, "callbackPtr_ is null");
        callback_ = callbackPtr_->cb_;
    };
    TaiheAudioCapturerCallbackInner::SaveCallbackReferenceInner(callbackName, callback, successed);
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioCapturerDeviceChangeCallback::RemoveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::function<void()> successed = [this]() {
        callbackPtr_ = nullptr;
        callback_ = nullptr;
    };
    TaiheAudioCapturerCallbackInner::RemoveCallbackReferenceInner(callbackName, callback, successed);
}

bool TaiheAudioCapturerDeviceChangeCallback::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    return TaiheParamUtils::IsSameRef(callback, callback_);
}

void TaiheAudioCapturerDeviceChangeCallback::OnStateChange(const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> sptr =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>(deviceInfo);
    AudioDeviceDescriptor outputAudioDevice = TaiheParamUtils::SetDeviceDescriptor(sptr);
    std::vector<AudioDeviceDescriptor> res;
    res.push_back(outputAudioDevice);
    std::unique_ptr<AudioCapturerDeviceChangeJsCallback> cb =
        std::make_unique<AudioCapturerDeviceChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = callbackPtr_;
    cb->callbackName = INPUTDEVICE_CHANGE_CALLBACK_NAME;
    cb->deviceInfo_ = taihe::array<AudioDeviceDescriptor>(res);
    OnJsCallbackCapturerDeviceInfo(cb);
}

void TaiheAudioCapturerDeviceChangeCallback::OnJsCallbackCapturerDeviceInfo(
    std::unique_ptr<AudioCapturerDeviceChangeJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackVolumeEvent: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioCapturerDeviceChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackCapturerDeviceInfoWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnInputDeviceChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioCapturerDeviceChangeCallback::SafeJsCallbackCapturerDeviceInfoWork(
    AudioCapturerDeviceChangeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackInterruptWork: no memory");
    std::shared_ptr<AudioCapturerDeviceChangeJsCallback> safeContext(
        static_cast<AudioCapturerDeviceChangeJsCallback*>(event),
        [](AudioCapturerDeviceChangeJsCallback *ptr) {
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
        (*cacheCallback)(event->deviceInfo_);
    } while (0);
}

std::shared_ptr<AutoRef> TaiheAudioCapturerDeviceChangeCallback::GetCallback(const std::string &callbackName)
{
    return callbackPtr_;
}

bool TaiheAudioCapturerDeviceChangeCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    return (callbackName == INPUTDEVICE_CHANGE_CALLBACK_NAME);
}
} // namespace ANI::Audio