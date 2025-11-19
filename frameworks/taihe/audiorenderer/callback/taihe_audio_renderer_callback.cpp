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
#define LOG_TAG "TaiheAudioRendererCallback"
#endif

#include "taihe_audio_renderer_callback.h"
#include <thread>
#include "taihe_audio_enum.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioRendererCallback::TaiheAudioRendererCallback()
{
    AUDIO_DEBUG_LOG("instance create");
}
TaiheAudioRendererCallback::~TaiheAudioRendererCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void TaiheAudioRendererCallback::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this, callbackName](std::shared_ptr<AutoRef> generatedCallback) {
        if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
            interruptCallback_ = generatedCallback;
            return;
        }
        if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
            stateChangeCallback_ = generatedCallback;
            return;
        }
    };
    TaiheAudioRendererCallbackInner::SaveCallbackReferenceInner(callbackName, callback, successed);
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioRendererCallback::RemoveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void()> successed = [this, callbackName]() {
        if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
            interruptCallback_ = nullptr;
            return;
        }
        if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
            stateChangeCallback_ = nullptr;
            return;
        }
    };
    RemoveCallbackReferenceInner(callbackName, callback, successed);
}

std::shared_ptr<AutoRef> TaiheAudioRendererCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> cb = nullptr;
    if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
        return interruptCallback_;
    }
    if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
        return stateChangeCallback_;
    }
    AUDIO_ERR_LOG("TaiheAudioRendererCallback->GetCallback Unknown callback type: %{public}s", callbackName.c_str());
    return cb;
}

bool TaiheAudioRendererCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME ||
        callbackName == STATE_CHANGE_CALLBACK_NAME) {
        return true;
    }
    return false;
}

void TaiheAudioRendererCallback::OnInterrupt(const OHOS::AudioStandard::InterruptEvent &interruptEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("OnInterrupt is called,hintType: %{public}d", interruptEvent.hintType);
    CHECK_AND_RETURN_LOG(interruptCallback_ != nullptr, "Cannot find the reference of interrupt callback");

    std::unique_ptr<AudioRendererJsCallback> cb = std::make_unique<AudioRendererJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = interruptCallback_;
    cb->callbackName = AUDIO_INTERRUPT_CALLBACK_NAME;
    cb->interruptEvent = interruptEvent;
    return OnJsCallbackInterrupt(cb);
}

void TaiheAudioRendererCallback::OnStateChange(const OHOS::AudioStandard::RendererState state,
    const OHOS::AudioStandard::StateChangeCmdType __attribute__((unused)) cmdType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("OnStateChange is called, state: %{public}d", state);
    CHECK_AND_RETURN_LOG(stateChangeCallback_ != nullptr, "Cannot find the reference of stateChange callback");

    std::unique_ptr<AudioRendererJsCallback> cb = std::make_unique<AudioRendererJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = stateChangeCallback_;
    cb->callbackName = STATE_CHANGE_CALLBACK_NAME;
    cb->state = state;
    return OnJsCallbackStateChange(cb);
}

void TaiheAudioRendererCallback::OnJsCallbackInterrupt(std::unique_ptr<AudioRendererJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackInterrupt: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioRendererJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackInterruptWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnAudioInterrupt", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioRendererCallback::SafeJsCallbackInterruptWork(AudioRendererJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackInterruptWork: no memory");
    std::shared_ptr<AudioRendererJsCallback> safeContext(
        static_cast<AudioRendererJsCallback*>(event),
        [](AudioRendererJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    InterruptEvent interruptEvent = {
        .eventType = TaiheAudioEnum::ToTaiheInterruptType(event->interruptEvent.eventType),
        .forceType = TaiheAudioEnum::ToTaiheInterruptForceType(event->interruptEvent.forceType),
        .hintType = TaiheAudioEnum::ToTaiheInterruptHint(event->interruptEvent.hintType),
    };
    do {
        std::shared_ptr<taihe::callback<void(InterruptEvent const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(InterruptEvent const&)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(interruptEvent);
    } while (0);
}

void TaiheAudioRendererCallback::OnJsCallbackStateChange(std::unique_ptr<AudioRendererJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackStateChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioRendererJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackStateChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnStateChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioRendererCallback::SafeJsCallbackStateChangeWork(AudioRendererJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackStateChangeWork: no memory");
    std::shared_ptr<AudioRendererJsCallback> safeContext(
        static_cast<AudioRendererJsCallback*>(event),
        [](AudioRendererJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(AudioState)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioState)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheAudioEnum::ToTaiheAudioState(event->state));
    } while (0);
}
} // namespace ANI::Audio
