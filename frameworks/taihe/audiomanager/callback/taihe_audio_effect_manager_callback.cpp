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
#define LOG_TAG "TaiheAudioSeparationEffectEnabledChangeCallback"
#endif

#include "taihe_audio_effect_manager_callback.h"
#include <mutex>
#include <thread>
#include "taihe_param_utils.h"
#include "taihe_audio_manager_callbacks.h"

namespace ANI::Audio {
using namespace std;

TaiheAudioSeparationEffectEnabledChangeCallback::TaiheAudioSeparationEffectEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioSeparationEffectEnabledChangeCallback: instance create");
}

TaiheAudioSeparationEffectEnabledChangeCallback::~TaiheAudioSeparationEffectEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioSeparationEffectEnabledChangeCallback: instance destroy");
}

void TaiheAudioSeparationEffectEnabledChangeCallback::SaveAudioSeparationEffectEnabledChangeCallbackReference(
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = audioSeparationEffectEnabledChangeCbList_.begin();
        it != audioSeparationEffectEnabledChangeCbList_.end(); ++it) {
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: effect manager has same callback");
    }

    CHECK_AND_RETURN_LOG(callback != nullptr, "creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "creating callback failed");
    audioSeparationEffectEnabledChangeCbList_.push_back(cb);

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioSeparationEffectEnabledChangeCallback::RemoveAudioSeparationEffectEnabledChangeCallbackReference(
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioSeparationEffectEnabledChangeCbList_.begin();
        it != audioSeparationEffectEnabledChangeCbList_.end(); ++it) {
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveAudioSeparationEffectEnabledChangeCallbackReference: find js callback, erase it");
            audioSeparationEffectEnabledChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveAudioSeparationEffectEnabledChangeCallbackReference: js callback no find");
}

void TaiheAudioSeparationEffectEnabledChangeCallback::RemoveAllAudioSeparationEffectEnabledChangeCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioSeparationEffectEnabledChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllAudioSeparationEffectEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t TaiheAudioSeparationEffectEnabledChangeCallback::GetAudioSeparationEffectEnabledChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return audioSeparationEffectEnabledChangeCbList_.size();
}

void TaiheAudioSeparationEffectEnabledChangeCallback::OnAudioSeparationEffectEnabledChange(bool enabled)
{
    AUDIO_INFO_LOG("enter");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = audioSeparationEffectEnabledChangeCbList_.begin();
        it != audioSeparationEffectEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioSeparationEffectEnabledJsCallback> cb =
            std::make_unique<AudioSeparationEffectEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        OnJsCallbackAudioSeparationEffectEnabled(cb);
    }
    return;
}

void TaiheAudioSeparationEffectEnabledChangeCallback::OnJsCallbackAudioSeparationEffectEnabled(
    std::unique_ptr<AudioSeparationEffectEnabledJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb->callback == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAudioSeparationEffectEnabled: jsCb or jsCb->callback is null");
        return;
    }

    if (mainHandler_ != nullptr) {
        AudioSeparationEffectEnabledJsCallback *event = jsCb.release();
        bool ret = mainHandler_->PostTask([event]() {
            SafeJsCallbackAudioSeparationEffectEnabledWork(event);
        });
        if (!ret) {
            AUDIO_ERR_LOG("OnJsCallbackAudioSeparationEffectEnabled: PostTask failed");
            if (event != nullptr) {
                delete event;
                event = nullptr;
            }
        }
    } else {
        AUDIO_ERR_LOG("OnJsCallbackAudioSeparationEffectEnabled: mainHandler_ is nullptr");
    }
}

void TaiheAudioSeparationEffectEnabledChangeCallback::SafeJsCallbackAudioSeparationEffectEnabledWork(
    AudioSeparationEffectEnabledJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackAudioSeparationEffectEnabledWork: no memory");
    std::shared_ptr<AudioSeparationEffectEnabledJsCallback> safeContext(
        static_cast<AudioSeparationEffectEnabledJsCallback*>(event),
        [](AudioSeparationEffectEnabledJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
                ptr = nullptr;
            }
    });
    AUDIO_INFO_LOG("SafeJsCallbackAudioSeparationEffectEnabledWork: safe js callback working.");

    std::shared_ptr<taihe::callback<void(bool)>> cacheCallback =
        std::reinterpret_pointer_cast<taihe::callback<void(bool)>, uintptr_t>(event->callback->cb_);
    CHECK_AND_RETURN_LOG(cacheCallback != nullptr, "get reference value fail");
    (*cacheCallback)(event->enabled);
}
} // namespace ANI::Audio