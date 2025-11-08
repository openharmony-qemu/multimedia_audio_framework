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
#define LOG_TAG "TaiheAudioRingerModeCallback"
#endif

#include "taihe_audio_ringermode_callback.h"

namespace ANI::Audio {
TaiheAudioRingerModeCallback::TaiheAudioRingerModeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioRingerModeCallback: instance create");
}

TaiheAudioRingerModeCallback::~TaiheAudioRingerModeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioRingerModeCallback: instance destroy");
}

void TaiheAudioRingerModeCallback::OnRingerModeUpdated(const OHOS::AudioStandard::AudioRingerMode &ringerMode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("TaiheAudioRingerModeCallback: ringer mode: %{public}d", ringerMode);
    CHECK_AND_RETURN_LOG(ringerModeCallback_ != nullptr, "Cannot find the reference of ringer mode callback");

    std::unique_ptr<AudioRingerModeJsCallback> cb = std::make_unique<AudioRingerModeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = ringerModeCallback_;
    cb->callbackName = RINGERMODE_CALLBACK_NAME;
    cb->ringerMode = ringerMode;
    return OnJsCallbackRingerMode(cb);
}

void TaiheAudioRingerModeCallback::OnJsCallbackRingerMode(std::unique_ptr<AudioRingerModeJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("TaiheAudioRingerModeCallback: OnJsCallbackRingerMode: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioRingerModeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackRingModeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnRingerModeChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioRingerModeCallback::SafeJsCallbackRingModeWork(AudioRingerModeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackRingerMode: no memory");
    std::shared_ptr<AudioRingerModeJsCallback> safeContext(
        static_cast<AudioRingerModeJsCallback*>(event),
        [](AudioRingerModeJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(AudioRingMode)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioRingMode)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheAudioEnum::ToTaiheAudioRingMode(event->ringerMode));
    } while (0);
}

void TaiheAudioRingerModeCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioRingerModeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == RINGERMODE_CALLBACK_NAME) {
        ringerModeCallback_ = cb;
    }  else {
        AUDIO_ERR_LOG("TaiheAudioRingerModeCallback: Unknown callback type: %{public}s", callbackName.c_str());
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

bool TaiheAudioRingerModeCallback::IsSameCallback(std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (ringerModeCallback_ == nullptr) {
        return false;
    }
    return TaiheParamUtils::IsSameRef(callback, ringerModeCallback_->cb_);
}

void TaiheAudioRingerModeCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    CHECK_AND_RETURN_LOG(ringerModeCallback_ != nullptr, "ringerModeCallback_ is nullptr");
    if (!IsSameCallback(callback)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ringerModeCallback_->cb_ = nullptr;
    ringerModeCallback_ = nullptr;
    AUDIO_INFO_LOG("Remove callback reference successful.");
}
} // namespace ANI::Audio