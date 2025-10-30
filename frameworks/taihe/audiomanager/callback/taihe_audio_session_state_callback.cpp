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
#define LOG_TAG "TaiheAudioSessionStateCallback"
#endif
#include <thread>
#include "taihe_audio_session_state_callback.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioSessionStateCallback::TaiheAudioSessionStateCallback()
{
    AUDIO_INFO_LOG("TaiheAudioSessionStateCallback::Constructor");
}

TaiheAudioSessionStateCallback::~TaiheAudioSessionStateCallback()
{
    AUDIO_INFO_LOG("TaiheAudioSessionStateCallback::Destructor");
}

void TaiheAudioSessionStateCallback::OnAudioSessionStateChanged(
    const OHOS::AudioStandard::AudioSessionStateChangedEvent &stateEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionStateChanged is called AudioSessionStateChangedEvent=%{public}d",
        stateEvent.stateChangeHint);
    CHECK_AND_RETURN_LOG(audioSessionStateJsCallback_ != nullptr,
        "OnAudioSessionStateChanged:No JS callback registered return");
    std::unique_ptr<AudioSessionStateJsCallback> cb = std::make_unique<AudioSessionStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioSessionStateJsCallback_;
    cb->callbackName = AUDIO_SESSION_STATE_CALLBACK_NAME;
    cb->audioSessionStateEvent.stateChangeHint = stateEvent.stateChangeHint;

    return OnJsCallbackAudioSessionState(cb);
}

void TaiheAudioSessionStateCallback::SaveCallbackReference(std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    audioSessionStateJsCallback_ = cb;

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioSessionStateCallback::SafeJsCallbackAudioSessionStateWork(AudioSessionStateJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackSystemVolumeChange: no memory");
    std::shared_ptr<AudioSessionStateJsCallback> safeContext(
        static_cast<AudioSessionStateJsCallback*>(event),
        [](AudioSessionStateJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    AUDIO_INFO_LOG("SafeJsCallbackAudioSessionStateWork: safe js callback working.");

    do {
        std::shared_ptr<taihe::callback<void(AudioSessionStateChangedEvent const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioSessionStateChangedEvent const&)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s fail to call SetaudioSessionState callback",
            request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueAudioSessionStateChangedEvent(event->audioSessionStateEvent));
    } while (0);
}

void TaiheAudioSessionStateCallback::OnJsCallbackAudioSessionState(std::unique_ptr<AudioSessionStateJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("TaiheAudioSessionStateCallback: OnJsCallbackAudioSessionState: jsCb.get() is null");
        return;
    }
    AudioSessionStateJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAudioSessionStateWork(event);
        }
    };
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    mainHandler_->PostTask(task, "OnAudioSessionStateChanged", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

bool TaiheAudioSessionStateCallback::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    return TaiheParamUtils::IsSameRef(callback, callback_);
}
} // namespace ANI::Audio