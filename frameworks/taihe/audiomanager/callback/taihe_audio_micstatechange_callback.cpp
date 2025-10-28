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
#define LOG_TAG "TaiheAudioManagerMicStateChangeCallback"
#endif

#include "taihe_audio_micstatechange_callback.h"

namespace ANI::Audio {
TaiheAudioManagerMicStateChangeCallback::TaiheAudioManagerMicStateChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioManagerMicStateChangeCallback: instance create");
}

TaiheAudioManagerMicStateChangeCallback::~TaiheAudioManagerMicStateChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioManagerMicStateChangeCallback: instance destroy");
}

void TaiheAudioManagerMicStateChangeCallback::OnMicStateUpdated(const OHOS::AudioStandard::MicStateChangeEvent
    &micStateChangeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(micStateChangeCallback_ != nullptr, "callback not registered by JS client");

    std::unique_ptr<AudioManagerMicStateChangeJsCallback> cb = std::make_unique<AudioManagerMicStateChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");

    cb->callback = micStateChangeCallback_;
    cb->callbackName = MIC_STATE_CHANGE_CALLBACK_NAME;
    cb->micStateChangeEvent = micStateChangeEvent;
    return OnJsCallbackMicStateChange(cb);
}

void TaiheAudioManagerMicStateChangeCallback::OnJsCallbackMicStateChange(
    std::unique_ptr<AudioManagerMicStateChangeJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackMicStateChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioManagerMicStateChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackMicStateChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnMicStateChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioManagerMicStateChangeCallback::SafeJsCallbackMicStateChangeWork(
    AudioManagerMicStateChangeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackRingerMode: no memory");
    std::shared_ptr<AudioManagerMicStateChangeJsCallback> safeContext(
        static_cast<AudioManagerMicStateChangeJsCallback*>(event),
        [](AudioManagerMicStateChangeJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(MicStateChangeEvent const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(MicStateChangeEvent const&)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueMicStateChange(event->micStateChangeEvent));
    } while (0);
}

void TaiheAudioManagerMicStateChangeCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioManagerMicStateChangeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    CHECK_AND_RETURN_LOG(callbackName == MIC_STATE_CHANGE_CALLBACK_NAME,
        "TaiheAudioManagerMicStateChangeCallback: Unknown callback type: %{public}s", callbackName.c_str());
    micStateChangeCallback_ = cb;
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

bool TaiheAudioManagerMicStateChangeCallback::IsSameCallback(std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (micStateChangeCallback_ == nullptr) {
        return false;
    }
    return TaiheParamUtils::IsSameRef(callback, micStateChangeCallback_->cb_);
}

void TaiheAudioManagerMicStateChangeCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    CHECK_AND_RETURN_LOG(micStateChangeCallback_ != nullptr, "micStateChangeCallback_ is nullptr");
    if (!IsSameCallback(callback)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    micStateChangeCallback_->cb_ = nullptr;
    micStateChangeCallback_ = nullptr;
    AUDIO_INFO_LOG("Remove callback reference successful.");
}
} // namespace ANI::Audio