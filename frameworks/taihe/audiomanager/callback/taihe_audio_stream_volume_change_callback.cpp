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
#define LOG_TAG "TaiheAudioStreamVolumeChangeCallback"
#endif

#include "taihe_audio_stream_volume_change_callback.h"

namespace ANI::Audio {
TaiheAudioStreamVolumeChangeCallback::TaiheAudioStreamVolumeChangeCallback()
{
    AUDIO_INFO_LOG("TaiheAudioStreamVolumeChangeCallback::Constructor");
}

TaiheAudioStreamVolumeChangeCallback::~TaiheAudioStreamVolumeChangeCallback()
{
    AUDIO_INFO_LOG("TaiheAudioStreamVolumeChangeCallback::Destructor");
}

void TaiheAudioStreamVolumeChangeCallback::OnStreamVolumeChange(OHOS::AudioStandard::StreamVolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_PRERELEASE_LOGI("OnStreamVolumeChange is called streamUsage=%{public}d, volumeLevel=%{public}d,"
        "isUpdateUi=%{public}d", volumeEvent.streamUsage, volumeEvent.volume, volumeEvent.updateUi);
    CHECK_AND_RETURN_LOG(audioStreamVolumeChangeCallback_ != nullptr,
        "TaiheAudioStreamVolumeChangeCallback:No JS callback registered return");
    std::unique_ptr<AudioStreamVolumeChangeJsCallback> cb = std::make_unique<AudioStreamVolumeChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioStreamVolumeChangeCallback_;
    cb->callbackName = AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME;
    cb->volumeEvent.streamUsage = volumeEvent.streamUsage;
    cb->volumeEvent.volume = volumeEvent.volume;
    cb->volumeEvent.updateUi = volumeEvent.updateUi;

    return OnJsCallbackStreamVolumeChange(cb);
}

void TaiheAudioStreamVolumeChangeCallback::OnJsCallbackStreamVolumeChange(
    std::unique_ptr<AudioStreamVolumeChangeJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackStreamVolumeChange: jsCb.get() is null");
        return;
    }
    AudioStreamVolumeChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackStreamVolumeChangeWork(event);
        }
    };
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    mainHandler_->PostTask(task, "OnStreamVolumeChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioStreamVolumeChangeCallback::SafeJsCallbackStreamVolumeChangeWork(
    AudioStreamVolumeChangeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackStreamVolumeChange: no memory");
    std::shared_ptr<AudioStreamVolumeChangeJsCallback> safeContext(
        static_cast<AudioStreamVolumeChangeJsCallback*>(event),
        [](AudioStreamVolumeChangeJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(StreamVolumeEvent const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(StreamVolumeEvent const&)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueStreamVolumeEvent(event->volumeEvent));
    } while (0);
}

void TaiheAudioStreamVolumeChangeCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioStreamVolumeChangeCallback: creating reference for callback fail");
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME) {
        audioStreamVolumeChangeCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("TaiheAudioStreamVolumeChangeCallback: Unknown callback type: %{public}s", callbackName.c_str());
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

bool TaiheAudioStreamVolumeChangeCallback::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    return TaiheParamUtils::IsSameRef(callback, callback_);
}
} // namespace ANI::Audio