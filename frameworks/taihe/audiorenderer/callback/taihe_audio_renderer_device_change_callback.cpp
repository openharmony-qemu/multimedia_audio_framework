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
#define LOG_TAG "TaiheAudioRendererDeviceChangeCallback"
#endif

#include "taihe_audio_renderer_device_change_callback.h"
#include <thread>
#include "taihe_audio_renderer_callback.h"
#include "taihe_audio_enum.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
std::mutex TaiheAudioRendererDeviceChangeCallback::sWorkerMutex_;
std::mutex TaiheAudioRendererOutputDeviceChangeWithInfoCallback::sWorkerMutex_;
TaiheAudioRendererDeviceChangeCallback::TaiheAudioRendererDeviceChangeCallback(ani_env *env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}
TaiheAudioRendererDeviceChangeCallback::~TaiheAudioRendererDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheAudioRendererDeviceChangeCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        CHECK_AND_RETURN_LOG(callback == (*autoRef)->cb_, "callback already exits");
    }
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        callbacks_.push_back(generatedCallback);
    };
    TaiheAudioRendererCallbackInner::SaveCallbackReferenceInner(callbackName, callback, successed);
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
    mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
}

void TaiheAudioRendererDeviceChangeCallback::RemoveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;

    if (callback == nullptr) {
        callbacks_.clear();
        AUDIO_INFO_LOG("Remove all JS Callback");
        return;
    }

    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        if (TaiheParamUtils::IsSameRef(callback, ((*autoRef)->cb_))) {
            isEquals = true;
        }

        if (isEquals == true) {
            AUDIO_INFO_LOG("found JS Callback, delete it!");
            callbacks_.remove(*autoRef);
            return;
        }
    }

    AUDIO_INFO_LOG("RemoveCallbackReference success");
}

std::shared_ptr<AutoRef> &TaiheAudioRendererDeviceChangeCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> callbackCur = std::make_shared<AutoRef>(env_, nullptr);
    return callbackCur;
}

bool TaiheAudioRendererDeviceChangeCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    return (callbackName == DEVICECHANGE_CALLBACK_NAME);
}

void TaiheAudioRendererDeviceChangeCallback::RemoveAllCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks successful");
}

void TaiheAudioRendererDeviceChangeCallback::OnOutputDeviceChange(
    const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo,
    const OHOS::AudioStandard::AudioStreamDeviceChangeReason reason)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> sptr =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>(deviceInfo);
    AudioDeviceDescriptor outputAudioDevice = TaiheParamUtils::SetDeviceDescriptor(sptr);
    std::vector<AudioDeviceDescriptor> res;
    res.push_back(outputAudioDevice);
    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        std::unique_ptr<AudioRendererDeviceChangeJsCallback> cb =
            std::make_unique<AudioRendererDeviceChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = (*autoRef);
        cb->callbackName = DEVICECHANGE_CALLBACK_NAME;
        cb->deviceInfo_ = taihe::array<AudioDeviceDescriptor>(res);
        OnJsCallbackRendererDeviceInfo(cb);
    }
}

int32_t TaiheAudioRendererDeviceChangeCallback::GetCallbackListSize() const
{
    return callbacks_.size();
}

void TaiheAudioRendererDeviceChangeCallback::OnJsCallbackRendererDeviceInfo(
    std::unique_ptr<AudioRendererDeviceChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererPeriodPositionCallback: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioRendererDeviceChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr, this]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackRendererDeviceInfoWork(this->env_, event);
        }
    };
    mainHandler_->PostTask(task, "OnOutputDeviceChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioRendererDeviceChangeCallback::SafeJsCallbackRendererDeviceInfoWork(ani_env *env,
    AudioRendererDeviceChangeJsCallback *event)
{
    std::lock_guard<std::mutex> lock(sWorkerMutex_);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackInterruptWork: no memory");
    std::shared_ptr<AudioRendererDeviceChangeJsCallback> safeContext(
        static_cast<AudioRendererDeviceChangeJsCallback*>(event),
        [](AudioRendererDeviceChangeJsCallback *ptr) {
            delete ptr;
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

TaiheAudioRendererOutputDeviceChangeWithInfoCallback::TaiheAudioRendererOutputDeviceChangeWithInfoCallback(ani_env *env)
    : env_(env)
{
    AUDIO_INFO_LOG("instance create");
}
TaiheAudioRendererOutputDeviceChangeWithInfoCallback::~TaiheAudioRendererOutputDeviceChangeWithInfoCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheAudioRendererOutputDeviceChangeWithInfoCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        CHECK_AND_RETURN_LOG(callback == (*autoRef)->cb_, "callback already exits");
    }

    CHECK_AND_RETURN_LOG(callback != nullptr, "creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    callbacks_.push_back(cb);
    AUDIO_INFO_LOG("successful");
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
    mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
}

std::shared_ptr<AutoRef> &TaiheAudioRendererOutputDeviceChangeWithInfoCallback::GetCallback(
    const std::string &callbackName)
{
    std::shared_ptr<AutoRef> callbackCur = std::make_shared<AutoRef>(env_, nullptr);
    return callbackCur;
}

bool TaiheAudioRendererOutputDeviceChangeWithInfoCallback::CheckIfTargetCallbackName(
    const std::string &callbackName)
{
    if (callbackName == OUTPUT_DEVICECHANGE_WITH_INFO) {
        return true;
    }
    return false;
}

void TaiheAudioRendererOutputDeviceChangeWithInfoCallback::RemoveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;

    if (callback == nullptr) {
        callbacks_.clear();
        AUDIO_INFO_LOG("Remove all JS Callback");
        return;
    }

    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        if (callback == (*autoRef)->cb_) {
            isEquals = true;
        }

        if (isEquals == true) {
            AUDIO_INFO_LOG("found JS Callback, delete it!");
            callbacks_.remove(*autoRef);
            return;
        }
    }

    AUDIO_INFO_LOG("RemoveCallbackReference success");
}

int32_t TaiheAudioRendererOutputDeviceChangeWithInfoCallback::GetCallbackListSize() const
{
    return callbacks_.size();
}

void TaiheAudioRendererOutputDeviceChangeWithInfoCallback::OnOutputDeviceChange(
    const OHOS::AudioStandard::AudioDeviceDescriptor &deviceInfo,
    const OHOS::AudioStandard::AudioStreamDeviceChangeReason reason)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> sptr =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>(deviceInfo);
    AudioDeviceDescriptor outputAudioDevice = TaiheParamUtils::SetDeviceDescriptor(sptr);
    std::vector<AudioDeviceDescriptor> res;
    res.push_back(outputAudioDevice);
    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        std::unique_ptr<AudioRendererOutputDeviceChangeWithInfoJsCallback> cb =
            std::make_unique<AudioRendererOutputDeviceChangeWithInfoJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = (*autoRef);
        cb->callbackName = DEVICECHANGE_CALLBACK_NAME;
        cb->deviceInfo_ = taihe::array<AudioDeviceDescriptor>(res);
        cb->reason_ = reason;
        OnJsCallbackOutputDeviceInfo(cb);
    }
}

void TaiheAudioRendererOutputDeviceChangeWithInfoCallback::OnJsCallbackOutputDeviceInfo(
    std::unique_ptr<AudioRendererOutputDeviceChangeWithInfoJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererPeriodPositionCallback: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioRendererOutputDeviceChangeWithInfoJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr, this]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackOutputDeviceInfoWork(this->env_, event);
        }
    };
    mainHandler_->PostTask(task, "OnOutputDeviceChangeWithInfo", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE,
        {});
}

void TaiheAudioRendererOutputDeviceChangeWithInfoCallback::SafeJsCallbackOutputDeviceInfoWork(ani_env *env,
    AudioRendererOutputDeviceChangeWithInfoJsCallback *event)
{
    std::lock_guard<std::mutex> lock(sWorkerMutex_);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackInterruptWork: no memory");
    std::shared_ptr<AudioRendererOutputDeviceChangeWithInfoJsCallback> safeContext(
        static_cast<AudioRendererOutputDeviceChangeWithInfoJsCallback*>(event),
        [](AudioRendererOutputDeviceChangeWithInfoJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    do {
        std::shared_ptr<taihe::callback<void(AudioStreamDeviceChangeInfo const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioStreamDeviceChangeInfo const&)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        AudioStreamDeviceChangeInfo audioStreamDeviceChangeInfo {
            .devices = event->deviceInfo_,
            .changeReason = TaiheAudioEnum::ToTaiheAudioStreamDeviceChangeReason(event->reason_),
        };
        (*cacheCallback)(audioStreamDeviceChangeInfo);
    } while (0);
}
} // namespace ANI::Audio
