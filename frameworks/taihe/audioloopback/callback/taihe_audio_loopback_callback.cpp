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
#define LOG_TAG "TaiheAudioLoopbackCallback"
#endif

#include "taihe_audio_loopback_callback.h"
#include "taihe_param_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {
TaiheAudioLoopbackCallback::TaiheAudioLoopbackCallback()
{
    AUDIO_DEBUG_LOG("instance create");
}

TaiheAudioLoopbackCallback::~TaiheAudioLoopbackCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheAudioLoopbackCallback::OnStatusChange(const OHOS::AudioStandard::AudioLoopbackStatus status,
    const OHOS::AudioStandard::StateChangeCmdType __attribute__((unused)) cmdType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("OnStateChange is called, status: %{public}d", status);
    CHECK_AND_RETURN_LOG(statusChangeCallback_ != nullptr, "Cannot find the reference of stateChange callback");

    std::unique_ptr<AudioLoopbackJsCallback> cb = std::make_unique<AudioLoopbackJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = statusChangeCallback_;
    cb->callbackName = STATUS_CHANGE_CALLBACK_NAME;
    cb->status = status;
    return OnJsCallbackStatusChange(cb);
}

void TaiheAudioLoopbackCallback::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this, callbackName](std::shared_ptr<AutoRef> generatedCallback) {
        if (callbackName == STATUS_CHANGE_CALLBACK_NAME) {
            statusChangeCallback_ = generatedCallback;
            return;
        }
    };
    SaveCallbackReferenceInner(callbackName, callback, successed);
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioLoopbackCallback::RemoveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void()> successed = [this, callbackName]() {
        if (callbackName == STATUS_CHANGE_CALLBACK_NAME) {
            statusChangeCallback_ = nullptr;
            return;
        }
    };
    RemoveCallbackReferenceInner(callbackName, callback, successed);
}

std::shared_ptr<AutoRef> TaiheAudioLoopbackCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> cb = nullptr;
    if (callbackName == STATUS_CHANGE_CALLBACK_NAME) {
        return statusChangeCallback_;
    }
    AUDIO_ERR_LOG("TaiheAudioLoopbackCallback->GetCallback Unknown callback type: %{public}s", callbackName.c_str());
    return cb;
}

bool TaiheAudioLoopbackCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == STATUS_CHANGE_CALLBACK_NAME) {
        return true;
    }
    return false;
}

void TaiheAudioLoopbackCallback::SafeJsCallbackStatusChangeWork(AudioLoopbackJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackStatusChange: no memory");
    std::shared_ptr<AudioLoopbackJsCallback> safeContext(
        static_cast<AudioLoopbackJsCallback*>(event),
        [](AudioLoopbackJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    do {
        std::shared_ptr<taihe::callback<void(AudioLoopbackStatus)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioLoopbackStatus)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheAudioEnum::ToTaiheAudioLoopbackStatus(event->status));
    } while (0);
}

void TaiheAudioLoopbackCallback::OnJsCallbackStatusChange(std::unique_ptr<AudioLoopbackJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackStatusChange: jsCb.get() is null");
        return;
    }

    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioLoopbackJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackStatusChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnStatusChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio