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
#define LOG_TAG "TaiheAudioManagerAppVolumeChangeCallback"
#endif

#include <mutex>
#include <thread>
#include "taihe_appvolume_change_callback.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "taihe_param_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"

namespace ANI::Audio {
TaiheAudioManagerAppVolumeChangeCallback::TaiheAudioManagerAppVolumeChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioManagerAppVolumeChangeCallback: instance create");
}

TaiheAudioManagerAppVolumeChangeCallback::~TaiheAudioManagerAppVolumeChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioManagerAppVolumeChangeCallback: instance destroy");
}

void TaiheAudioManagerAppVolumeChangeCallback::SaveVolumeChangeCallbackForUidReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> &callback, int32_t appUid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter : appVolumeChangeForUidList_) {
        if (iter.second == appUid && IsSameCallback(callback, iter.first->cb_)) {
            AUDIO_ERR_LOG("appVolumeChangeForUidList_ has same callback and appUid, nothing to do");
            return;
        }
    }
    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioManagerAppVolumeChangeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID) {
        appVolumeChangeForUidList_.push_back({cb, appUid});
    }  else {
        AUDIO_ERR_LOG("TaiheAudioManagerAppVolumeChangeCallback: Unknown callback type: %{public}s",
            callbackName.c_str());
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioManagerAppVolumeChangeCallback::SaveSelfVolumdChangeCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter : selfAppVolumeChangeList_) {
        if (IsSameCallback(callback, iter->cb_)) {
            AUDIO_ERR_LOG("selfAppVolumeChangeList_ has same callback, nothing to do");
            return;
        }
    }
    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioManagerAppVolumeChangeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == APP_VOLUME_CHANGE_CALLBACK_NAME) {
        selfAppVolumeChangeList_.push_back(cb);
    }  else {
        AUDIO_ERR_LOG("TaiheAudioManagerAppVolumeChangeCallback: Unknown callback type: %{public}s",
            callbackName.c_str());
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

bool TaiheAudioManagerAppVolumeChangeCallback::IsSameCallback(std::shared_ptr<uintptr_t> &callback,
    std::shared_ptr<uintptr_t> &listCallback)
{
    return TaiheParamUtils::IsSameRef(callback, listCallback);
}

void TaiheAudioManagerAppVolumeChangeCallback::OnAppVolumeChangedForUid(int32_t appUid,
    const OHOS::AudioStandard::VolumeEvent &event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!OHOS::AudioStandard::PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("OnAppVolumeChangedForUid: No system permission");
        return;
    }
    AUDIO_DEBUG_LOG("OnAppVolumeChangedForUid: appUid[%{public}d]", appUid);
    for (auto iter : appVolumeChangeForUidList_) {
        if (appUid != iter.second) {
            continue;
        }
        std::unique_ptr<AudioManagerAppVolumeChangeJsCallback> cb =
            std::make_unique<AudioManagerAppVolumeChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = iter.first;
        cb->callbackName = APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID;
        cb->appVolumeChangeEvent = event;
        OnJsCallbackAppVolumeChange(cb);
    }
    return;
}

void TaiheAudioManagerAppVolumeChangeCallback::OnSelfAppVolumeChanged(const OHOS::AudioStandard::VolumeEvent &event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("enter");
    for (auto iter : selfAppVolumeChangeList_) {
        std::unique_ptr<AudioManagerAppVolumeChangeJsCallback> cb =
            std::make_unique<AudioManagerAppVolumeChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = iter;
        cb->callbackName = APP_VOLUME_CHANGE_CALLBACK_NAME;
        cb->appVolumeChangeEvent = event;
        OnJsCallbackAppVolumeChange(cb);
    }
}

void TaiheAudioManagerAppVolumeChangeCallback::SafeJsCallbackAppVolumeChangeWork(
    AudioManagerAppVolumeChangeJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<AudioManagerAppVolumeChangeJsCallback> safeContext(
        static_cast<AudioManagerAppVolumeChangeJsCallback*>(event),
        [](AudioManagerAppVolumeChangeJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(VolumeEvent const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(VolumeEvent const&)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueVolumeEvent(event->appVolumeChangeEvent));
    } while (0);
}

void TaiheAudioManagerAppVolumeChangeCallback::OnJsCallbackAppVolumeChange(
    std::unique_ptr<AudioManagerAppVolumeChangeJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAppVolumeChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioManagerAppVolumeChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAppVolumeChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnAppVolumeChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioManagerAppVolumeChangeCallback::RemoveAllAudioVolumeChangeForUidCbRef()
{
    std::lock_guard<std::mutex> lock(mutex_);
    appVolumeChangeForUidList_.clear();
    AUDIO_INFO_LOG("RemoveAllAudioVolumeChangeForUidCbRef: remove callback finish");
}

void TaiheAudioManagerAppVolumeChangeCallback::RemoveAudioVolumeChangeForUidCbRef(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter = appVolumeChangeForUidList_.begin(); iter != appVolumeChangeForUidList_.end();) {
        if (iter->first == nullptr) {
            AUDIO_ERR_LOG("RemoveAudioVolumeChangeForUidCbRef: iter->first is null");
            continue;
        }
        if (IsSameCallback(callback, iter->first->cb_)) {
            AUDIO_INFO_LOG("RemoveAudioVolumeChangeForUidCbRef: find js callback, erase it");
            appVolumeChangeForUidList_.erase(iter++);
        } else {
            iter++;
        }
    }
    AUDIO_INFO_LOG("RemoveAudioVolumeChangeForUidCbRef: remove callback finish");
}

void TaiheAudioManagerAppVolumeChangeCallback::RemoveSelfAudioVolumeChangeCbRef(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter = selfAppVolumeChangeList_.begin(); iter != selfAppVolumeChangeList_.end();) {
        if (*iter == nullptr) {
            AUDIO_ERR_LOG("RemoveSelfAudioVolumeChangeCbRef: *iter is null");
            continue;
        }
        if (IsSameCallback(callback, (*iter)->cb_)) {
            AUDIO_INFO_LOG("RemoveSelfAudioVolumeChangeCbRef: find js callback, erase it");
            selfAppVolumeChangeList_.erase(iter++);
        } else {
            iter++;
        }
    }
    AUDIO_INFO_LOG("RemoveSelfAudioVolumeChangeCbRef: remove callback finish");
}

void TaiheAudioManagerAppVolumeChangeCallback::RemoveAllSelfAudioVolumeChangeCbRef()
{
    std::lock_guard<std::mutex> lock(mutex_);
    selfAppVolumeChangeList_.clear();
    AUDIO_INFO_LOG("RemoveAllSelfAudioVolumeChangeCbRef: remove callback finish");
}

int32_t TaiheAudioManagerAppVolumeChangeCallback::GetAppVolumeChangeForUidListSize()
{
    return appVolumeChangeForUidList_.size();
}

int32_t TaiheAudioManagerAppVolumeChangeCallback::GetSelfAppVolumeChangeListSize()
{
    return selfAppVolumeChangeList_.size();
}
} // namespace ANI::Audio