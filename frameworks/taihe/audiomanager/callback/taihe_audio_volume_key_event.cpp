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
#define LOG_TAG "TaiheAudioVolumeKeyEvent"
#endif

#include "taihe_audio_volume_key_event.h"
#include <mutex>
#include <thread>
#include "audio_manager_log.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
TaiheAudioVolumeKeyEvent::TaiheAudioVolumeKeyEvent()
{
    AUDIO_INFO_LOG("TaiheAudioVolumeKeyEvent::Constructor");
}

TaiheAudioVolumeKeyEvent::~TaiheAudioVolumeKeyEvent()
{
    AUDIO_INFO_LOG("TaiheAudioVolumeKeyEvent::Destructor");
}

void TaiheAudioVolumeKeyEvent::OnVolumeKeyEvent(OHOS::AudioStandard::VolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_PRERELEASE_LOGI("OnVolumeKeyEvent is called volumeType=%{public}d, volumeLevel=%{public}d,"
        "isUpdateUi=%{public}d", volumeEvent.volumeType, volumeEvent.volume, volumeEvent.updateUi);
    CHECK_AND_RETURN_LOG(audioVolumeKeyEventJsCallback_ != nullptr,
        "TaiheAudioVolumeKeyEvent:No JS callback registered return");
    std::unique_ptr<AudioVolumeKeyEventJsCallback> cb = std::make_unique<AudioVolumeKeyEventJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioVolumeKeyEventJsCallback_;
    cb->callbackName = VOLUME_KEY_EVENT_CALLBACK_NAME;
    cb->volumeEvent.volumeType = volumeEvent.volumeType;
    cb->volumeEvent.volume = volumeEvent.volume;
    cb->volumeEvent.updateUi = volumeEvent.updateUi;
    cb->volumeEvent.volumeGroupId = volumeEvent.volumeGroupId;
    cb->volumeEvent.networkId = volumeEvent.networkId;

    return OnJsCallbackVolumeEvent(cb);
}

void TaiheAudioVolumeKeyEvent::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> cacheCallback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callback_ = cacheCallback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(cacheCallback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    if (callbackName == VOLUME_KEY_EVENT_CALLBACK_NAME) {
        audioVolumeKeyEventJsCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("TaiheAudioVolumeKeyEvent: Unknown callback type: %{public}s", callbackName.c_str());
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioVolumeKeyEvent::SafeJsCallbackVolumeEventWork(AudioVolumeKeyEventJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<AudioVolumeKeyEventJsCallback> safeContext(
        static_cast<AudioVolumeKeyEventJsCallback*>(event),
        [](AudioVolumeKeyEventJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
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

void TaiheAudioVolumeKeyEvent::OnJsCallbackVolumeEvent(std::unique_ptr<AudioVolumeKeyEventJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackVolumeEvent: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioVolumeKeyEventJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackVolumeEventWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnVolumeChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

bool TaiheAudioVolumeKeyEvent::ContainSameJsCallback(std::shared_ptr<uintptr_t> callback)
{
    return TaiheParamUtils::IsSameRef(callback, callback_);
}

TaiheAudioVolumeKeyEventEx::TaiheAudioVolumeKeyEventEx()
{
    AUDIO_INFO_LOG("Constructor");
}

TaiheAudioVolumeKeyEventEx::~TaiheAudioVolumeKeyEventEx()
{
    AUDIO_INFO_LOG("Destructor");
}

void TaiheAudioVolumeKeyEventEx::OnVolumeDegreeEvent(OHOS::AudioStandard::VolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_PRERELEASE_LOGI("OnVolumeDegreeEvent is called volumeType=%{public}d, volumeDegree=%{public}d,"
        "isUpdateUi=%{public}d", volumeEvent.volumeType, volumeEvent.volumeDegree, volumeEvent.updateUi);

    for (auto &item : audioVolumeKeyEventCbList_) {
        std::unique_ptr<AudioVolumeKeyEventJsCallback> cb = std::make_unique<AudioVolumeKeyEventJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = item;
        cb->callbackName = VOLUME_DEGREE_CHANGE_EVENT_CALLBACK_NAME;
        cb->volumeEvent.volumeType = volumeEvent.volumeType;
        cb->volumeEvent.volume = volumeEvent.volume;
        cb->volumeEvent.volumeDegree = volumeEvent.volumeDegree;
        cb->volumeEvent.updateUi = volumeEvent.updateUi;
        cb->volumeEvent.volumeGroupId = volumeEvent.volumeGroupId;
        cb->volumeEvent.networkId = volumeEvent.networkId;
        OnJsCallbackVolumeEvent(cb);
    }
}

void TaiheAudioVolumeKeyEventEx::SaveCallbackReference(const std::string &callbackName,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(callbackName == VOLUME_DEGREE_CHANGE_EVENT_CALLBACK_NAME,
        "Unknown callback type: %{public}s", callbackName.c_str());
    for (auto &item : audioVolumeKeyEventCbList_) {
        if (item == nullptr) {
            continue;
        }
        bool isSameCallback = TaiheParamUtils::IsSameRef(callback, item->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "has same callback, nothing to do");
    }

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    audioVolumeKeyEventCbList_.push_back(cb);
    AUDIO_INFO_LOG("save callback ref success, list size [%{public}zu]", audioVolumeKeyEventCbList_.size());

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioVolumeKeyEventEx::RemoveCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioVolumeKeyEventCbList_.begin(); it != audioVolumeKeyEventCbList_.end(); ++it) {
        std::shared_ptr<AutoRef> temp = (*it);
        if (temp == nullptr) {
            continue;
        }
        bool isSameCallback = TaiheParamUtils::IsSameRef(callback, temp->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("find key event callback, remove it");
            temp->cb_ = nullptr;
            audioVolumeKeyEventCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("remove nothing");
}

void TaiheAudioVolumeKeyEventEx::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioVolumeKeyEventCbList_) {
        if (item == nullptr) {
            continue;
        }
        item->cb_ = nullptr;
    }
    audioVolumeKeyEventCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

int32_t TaiheAudioVolumeKeyEventEx::GetVolumeKeyEventCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(audioVolumeKeyEventCbList_.size());
}

void TaiheAudioVolumeKeyEventEx::OnJsCallbackVolumeEvent(std::unique_ptr<AudioVolumeKeyEventJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackVolumeEvent: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioVolumeKeyEventJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackVolumeEventWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnVolumePercentageChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioVolumeKeyEventEx::SafeJsCallbackVolumeEventWork(AudioVolumeKeyEventJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<AudioVolumeKeyEventJsCallback> safeContext(
        static_cast<AudioVolumeKeyEventJsCallback*>(event),
        [](AudioVolumeKeyEventJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(VolumeEvent const& data)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(VolumeEvent const& data)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueVolumeEvent(event->volumeEvent));
    } while (0);
}
} // namespace ANI::Audio
