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
#define LOG_TAG "TaiheAudioManagerInterruptCallback"
#endif

#include "taihe_audio_manager_interrupt_callback.h"
#include <mutex>
#include <thread>
#include "taihe_audio_manager_callbacks.h"
#include "taihe_param_utils.h"

using namespace ANI::Audio;

namespace ANI::Audio {
std::mutex TaiheAudioManagerInterruptCallback::sWorkerMutex_;
TaiheAudioManagerInterruptCallback::TaiheAudioManagerInterruptCallback(ani_env *env)
    : env_(env)
{
    AUDIO_INFO_LOG("instance create");
}

TaiheAudioManagerInterruptCallback::~TaiheAudioManagerInterruptCallback()
{
    AUDIO_INFO_LOG("instance destroy");
}

void TaiheAudioManagerInterruptCallback::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback)
{
    CHECK_AND_RETURN_LOG(!callbackName.compare(INTERRUPT_CALLBACK_NAME),
        "SaveCallbackReference: Unknown callback type: %{public}s", callbackName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    bool isSameCallback = true;
    for (auto it = audioManagerInterruptCallbackList_.begin(); it != audioManagerInterruptCallbackList_.end(); ++it) {
        isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: the callback already exists");
    }
    CHECK_AND_RETURN_LOG(callback != nullptr, "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    audioManagerInterruptCallbackList_.push_back(cb);
    AUDIO_INFO_LOG("SaveCallbackReference success, list size [%{public}zu]", audioManagerInterruptCallbackList_.size());
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
    CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
    mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
}

void TaiheAudioManagerInterruptCallback::RemoveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> &callback)
{
    CHECK_AND_RETURN_LOG(!callbackName.compare(INTERRUPT_CALLBACK_NAME),
        "RemoveCallbackReference: Unknown callback type: %{public}s", callbackName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioManagerInterruptCallbackList_.begin(); it != audioManagerInterruptCallbackList_.end(); ++it) {
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
        if (isSameCallback) {
            audioManagerInterruptCallbackList_.erase(it);
            AUDIO_INFO_LOG("RemoveCallbackReference success, list size [%{public}zu]",
                audioManagerInterruptCallbackList_.size());
            return;
        }
    }
    AUDIO_ERR_LOG("RemoveCallbackReference: js callback no find");
}

void TaiheAudioManagerInterruptCallback::RemoveAllCallbackReferences(const std::string &callbackName)
{
    CHECK_AND_RETURN_LOG(!callbackName.compare(INTERRUPT_CALLBACK_NAME),
        "RemoveCallbackReference: Unknown callback type: %{public}s", callbackName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    audioManagerInterruptCallbackList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbackReference: remove all js callbacks success");
}

int32_t TaiheAudioManagerInterruptCallback::GetInterruptCallbackListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return audioManagerInterruptCallbackList_.size();
}

void TaiheAudioManagerInterruptCallback::OnInterrupt(const OHOS::AudioStandard::InterruptAction &interruptAction)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("OnInterrupt action: %{public}d IntType: %{public}d, IntHint: %{public}d, activated: %{public}d",
        interruptAction.actionType, interruptAction.interruptType, interruptAction.interruptHint,
        interruptAction.activated);
    CHECK_AND_RETURN_LOG(audioManagerInterruptCallbackList_.size() != 0,
        "Cannot find the reference of interrupt callback");
    for (auto it = audioManagerInterruptCallbackList_.begin(); it != audioManagerInterruptCallbackList_.end(); ++it) {
        std::unique_ptr<AudioManagerInterruptJsCallback> cb = std::make_unique<AudioManagerInterruptJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = *it;
        cb->callbackName = INTERRUPT_CALLBACK_NAME;
        cb->interruptAction = interruptAction;
        OnJsCallbackAudioManagerInterrupt(cb);
    }
}

void TaiheAudioManagerInterruptCallback::SafeJsCallbackAudioManagerInterruptWork(ani_env *env,
    AudioManagerInterruptJsCallback *event)
{
    std::lock_guard<std::mutex> lock(sWorkerMutex_);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<AudioManagerInterruptJsCallback> safeContext(
        static_cast<AudioManagerInterruptJsCallback*>(event),
        [](AudioManagerInterruptJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(InterruptAction const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(InterruptAction const&)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueInterruptAction(event->interruptAction));
    } while (0);
}

void TaiheAudioManagerInterruptCallback::OnJsCallbackAudioManagerInterrupt(
    std::unique_ptr<AudioManagerInterruptJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRendererState: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioManagerInterruptJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr, this]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAudioManagerInterruptWork(this->env_, event);
        }
    };
    mainHandler_->PostTask(task, "OnInterrupt", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio