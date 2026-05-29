/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "TaiheAudioSystemVolumeChangeByFilterCallback"
#endif

#include "taihe_audio_system_volume_change_by_filter_callback.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

TaiheAudioSystemVolumeChangeByFilterCallback::TaiheAudioSystemVolumeChangeByFilterCallback()
{
    AUDIO_INFO_LOG("TaiheAudioSystemVolumeChangeByFilterCallback::Constructor");
}

TaiheAudioSystemVolumeChangeByFilterCallback::~TaiheAudioSystemVolumeChangeByFilterCallback()
{
    AUDIO_INFO_LOG("TaiheAudioSystemVolumeChangeByFilterCallback::Destructor");
}

void TaiheAudioSystemVolumeChangeByFilterCallback::OnSystemVolumeChangeByFilter(
    const OHOS::AudioStandard::VolumeEvent &volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_PRERELEASE_LOGI("OnSystemVolumeChangeByFilter is called volumeType=%{public}d, volumeLevel=%{public}d,"
        "appUid=%{public}d", volumeEvent.volumeType, volumeEvent.volume, volumeEvent.appUid);
    CHECK_AND_RETURN_LOG(!audioSystemVolumeChangeByFilterCbList_.empty(),
        "TaiheAudioSystemVolumeChangeByFilterCallback:No JS callback registered return");

    for (auto &item : audioSystemVolumeChangeByFilterCbList_) {
        std::unique_ptr<AudioSystemVolumeChangeByFilterJsCallback> cb =
            std::make_unique<AudioSystemVolumeChangeByFilterJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = item;
        cb->callbackName = AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_NAME;
        cb->volumeEvent.volumeType = volumeEvent.volumeType;
        cb->volumeEvent.volume = volumeEvent.volume;
        cb->volumeEvent.updateUi = volumeEvent.updateUi;
        cb->volumeEvent.volumeGroupId = volumeEvent.volumeGroupId;
        cb->volumeEvent.networkId = volumeEvent.networkId;
        cb->volumeEvent.volumeMode = volumeEvent.volumeMode;
        cb->volumeEvent.volumeDegree = volumeEvent.volumeDegree;
        cb->volumeEvent.appUid = volumeEvent.appUid;

        OnJsCallbackSystemVolumeChangeByFilter(cb);
    }
}

void TaiheAudioSystemVolumeChangeByFilterCallback::OnJsCallbackSystemVolumeChangeByFilter(
    std::unique_ptr<AudioSystemVolumeChangeByFilterJsCallback> &jsCb)
{
    CHECK_AND_RETURN_LOG(jsCb != nullptr && jsCb.get() != nullptr,
        "OnJsCallbackSystemVolumeChangeByFilter: jsCb.get() is null");
    AudioSystemVolumeChangeByFilterJsCallback *event = jsCb.release();
    CHECK_AND_CALL_FUNC_RETURN_LOG((event != nullptr) && (event->callback != nullptr), delete event,
        "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackSystemVolumeChangeByFilterWork(event);
        }
    };
    CHECK_AND_CALL_FUNC_RETURN_LOG(mainHandler_ != nullptr, delete event, "mainHandler_ is nullptr");
    mainHandler_->PostTask(task, "OnSystemVolumeChangeByFilter", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioSystemVolumeChangeByFilterCallback::SafeJsCallbackSystemVolumeChangeByFilterWork(
    AudioSystemVolumeChangeByFilterJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackSystemVolumeChangeByFilter: no memory");
    std::shared_ptr<AudioSystemVolumeChangeByFilterJsCallback> safeContext(
        static_cast<AudioSystemVolumeChangeByFilterJsCallback*>(event),
        [](AudioSystemVolumeChangeByFilterJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
                ptr = nullptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(VolumeEvent const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(VolumeEvent const&)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueVolumeEvent(event->volumeEvent));
    } while (0);
}

void TaiheAudioSystemVolumeChangeByFilterCallback::SaveCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback,
    const OHOS::AudioStandard::SystemVolumeFilter &filter)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioSystemVolumeChangeByFilterCallback: creating reference for callback fail");

    for (auto &item : audioSystemVolumeChangeByFilterCbList_) {
        if (item == nullptr) {
            continue;
        }
        bool isSameCallback = TaiheParamUtils::IsSameRef(callback, item->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "duplicate JS callback registration");
    }

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_NAME) {
        audioSystemVolumeChangeByFilterCbList_.push_back(cb);
    } else {
        AUDIO_ERR_LOG("TaiheAudioSystemVolumeChangeByFilterCallback: Unknown callback type: %{public}s",
            callbackName.c_str());
    }
    filter_ = filter;
    AUDIO_INFO_LOG("save callback ref success, list size=%{public}zu, uid=%{public}d",
        audioSystemVolumeChangeByFilterCbList_.size(), filter.uid);
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

bool TaiheAudioSystemVolumeChangeByFilterCallback::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioSystemVolumeChangeByFilterCbList_) {
        if (item == nullptr) {
            continue;
        }
        bool isEquals = TaiheParamUtils::IsSameRef(callback, item->cb_);
        if (isEquals) {
            return true;
        }
    }
    return false;
}

void TaiheAudioSystemVolumeChangeByFilterCallback::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioSystemVolumeChangeByFilterCbList_.begin();
         it != audioSystemVolumeChangeByFilterCbList_.end(); ++it) {
        if (*it == nullptr) {
            continue;
        }
        bool isSameCallback = TaiheParamUtils::IsSameRef(callback, (*it)->cb_);
        if (isSameCallback) {
            (*it)->cb_ = nullptr;
            audioSystemVolumeChangeByFilterCbList_.erase(it);
            AUDIO_INFO_LOG("remove js callback success, list size=%{public}zu",
                audioSystemVolumeChangeByFilterCbList_.size());
            return;
        }
    }
    AUDIO_WARNING_LOG("no matching callback found to remove");
}

void TaiheAudioSystemVolumeChangeByFilterCallback::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioSystemVolumeChangeByFilterCbList_) {
        if (item == nullptr) {
            continue;
        }
        item->cb_ = nullptr;
    }
    audioSystemVolumeChangeByFilterCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

int32_t TaiheAudioSystemVolumeChangeByFilterCallback::GetSystemVolumeByFilterCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(audioSystemVolumeChangeByFilterCbList_.size());
}
} // namespace ANI::Audio