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
#define LOG_TAG "TaiheActiveVolumeTypeChangeCallback"
#endif

#include "taihe_active_volume_type_change_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "taihe_param_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {
TaiheAudioManagerActiveVolumeTypeChangeCallback::TaiheAudioManagerActiveVolumeTypeChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioManagerActiveVolumeTypeChangeCallback: instance create");
}

TaiheAudioManagerActiveVolumeTypeChangeCallback::~TaiheAudioManagerActiveVolumeTypeChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioManagerActiveVolumeTypeChangeCallback: instance destroy");
}

void TaiheAudioManagerActiveVolumeTypeChangeCallback::SafeJsCallbackActiveVolumeTypeChangeWork(
    AudioManagerActiveVolumeTypeChangeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<AudioManagerActiveVolumeTypeChangeJsCallback> safeContext(
        static_cast<AudioManagerActiveVolumeTypeChangeJsCallback*>(event),
        [](AudioManagerActiveVolumeTypeChangeJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(AudioVolumeType)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioVolumeType)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheAudioEnum::GetJsAudioVolumeType(event->activeVolumeTypeChangeEvent));
    } while (0);
}

void TaiheAudioManagerActiveVolumeTypeChangeCallback::OnActiveVolumeTypeChanged(
    const OHOS::AudioStandard::AudioVolumeType &event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("callback change enter");
    for (auto iter : activeVolumeTypeChangeList_) {
        std::unique_ptr<AudioManagerActiveVolumeTypeChangeJsCallback> cb =
            std::make_unique<AudioManagerActiveVolumeTypeChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = iter;
        cb->callbackName = ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME;
        cb->activeVolumeTypeChangeEvent = event;
        OnJsCallbackActiveVolumeTypeChange(cb);
    }
}

void TaiheAudioManagerActiveVolumeTypeChangeCallback::OnJsCallbackActiveVolumeTypeChange(
    std::unique_ptr<AudioManagerActiveVolumeTypeChangeJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAppVolumeChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioManagerActiveVolumeTypeChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackActiveVolumeTypeChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnActiveVolumeTypeChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioManagerActiveVolumeTypeChangeCallback::SaveActiveVolumeTypeChangeCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter : activeVolumeTypeChangeList_) {
        if (iter == nullptr) {
            AUDIO_ERR_LOG("SaveActiveVolumeTypeChangeCallbackReference: iter is null");
            continue;
        }
        if (IsSameCallback(callback, iter->cb_)) {
            AUDIO_ERR_LOG("activeVolumeTypeChangeList_ has same callback, nothing to do");
            return;
        }
    }
    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioManagerActiveVolumeTypeChangeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME) {
        activeVolumeTypeChangeList_.push_back(cb);
    }  else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioManagerActiveVolumeTypeChangeCallback::RemoveSelfActiveVolumeTypeChangeCbRef(
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter = activeVolumeTypeChangeList_.begin(); iter != activeVolumeTypeChangeList_.end();) {
        if (*iter == nullptr) {
            AUDIO_ERR_LOG("RemoveSelfAudioVolumeChangeCbRef: *iter is null");
            continue;
        }
        if (IsSameCallback(callback, (*iter)->cb_)) {
            AUDIO_INFO_LOG("find js callback, erase it");
            activeVolumeTypeChangeList_.erase(iter++);
        } else {
            iter++;
        }
    }
    AUDIO_INFO_LOG("remove callback finish");
}

bool TaiheAudioManagerActiveVolumeTypeChangeCallback::IsSameCallback(const std::shared_ptr<uintptr_t> &callback,
    const std::shared_ptr<uintptr_t> &listCallback)
{
    return TaiheParamUtils::IsSameRef(callback, listCallback);
}

void TaiheAudioManagerActiveVolumeTypeChangeCallback::RemoveAllActiveVolumeTypeChangeCbRef()
{
    std::lock_guard<std::mutex> lock(mutex_);
    activeVolumeTypeChangeList_.clear();
    AUDIO_INFO_LOG("remove callback finish");
}

void TaiheAudioManagerActiveVolumeTypeChangeCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(activeVolumeTypeChangeCallback_ != nullptr, "activeVolumeTypeChangeCallback_ is null");

    activeVolumeTypeChangeCallback_->cb_ = nullptr;
    activeVolumeTypeChangeCallback_ = nullptr;
    AUDIO_INFO_LOG("remove callback reference successful.");
}

int32_t TaiheAudioManagerActiveVolumeTypeChangeCallback::GetActiveVolumeTypeChangeListSize()
{
    return activeVolumeTypeChangeList_.size();
}
} // namespace ANI::Audio