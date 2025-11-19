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
#define LOG_TAG "TaiheAudioSessionCallback"
#endif

#include "taihe_audio_session_callback.h"
#include <mutex>
#include <thread>
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioSessionCallback::TaiheAudioSessionCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioSessionCallback::Constructor");
}

TaiheAudioSessionCallback::~TaiheAudioSessionCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioSessionCallback::Destructor");
}

void TaiheAudioSessionCallback::OnAudioSessionDeactive(
    const OHOS::AudioStandard::AudioSessionDeactiveEvent &deactiveEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionDeactive is called AudioSessionDeactiveEvent=%{public}d",
        deactiveEvent.deactiveReason);
    CHECK_AND_RETURN_LOG(audioSessionJsCallback_ != nullptr,
        "OnAudioSessionDeactive:No JS callback registered return");
    std::unique_ptr<AudioSessionJsCallback> cb = std::make_unique<AudioSessionJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioSessionJsCallback_;
    cb->callbackName = AUDIO_SESSION_CALLBACK_NAME;
    cb->audioSessionDeactiveEvent.deactiveReason = deactiveEvent.deactiveReason;

    return OnJsCallbackAudioSession(cb);
}

void TaiheAudioSessionCallback::SaveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioSessionCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    audioSessionJsCallback_ = cb;
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioSessionCallback::SafeJsCallbackAudioSessionWork(AudioSessionJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAudioSession: no memory");
    std::shared_ptr<AudioSessionJsCallback> safeContext(
        static_cast<AudioSessionJsCallback*>(event),
        [](AudioSessionJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });

    AUDIO_INFO_LOG("SafeJsCallbackAudioSessionWork: safe js callback working.");
    do {
        std::shared_ptr<taihe::callback<void(AudioSessionDeactivatedEvent const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioSessionDeactivatedEvent const&)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "get reference value fail");
        AudioSessionDeactivatedEvent sessionDeactivatedEvent = TaiheParamUtils::ToTaiheSessionDeactivatedEvent(
            event->audioSessionDeactiveEvent);
        (*cacheCallback)(sessionDeactivatedEvent);
    } while (0);
}

void TaiheAudioSessionCallback::OnJsCallbackAudioSession(std::unique_ptr<AudioSessionJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("TaiheAudioSessionCallback: OnJsCallbackAudioSession: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioSessionJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAudioSessionWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnAudioSessionDeactivated", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio