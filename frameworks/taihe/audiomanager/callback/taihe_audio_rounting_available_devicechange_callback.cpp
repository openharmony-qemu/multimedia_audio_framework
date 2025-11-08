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
#define LOG_TAG "TaiheAudioRountingAvailableDeviceChangeCallback"
#endif

#include "taihe_audio_rounting_available_devicechange_callback.h"
#include <mutex>
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_manager_callbacks.h"

namespace ANI::Audio {
TaiheAudioRountingAvailableDeviceChangeCallback::TaiheAudioRountingAvailableDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioRountingAvailableDeviceChangeCallback: instance create");
}

TaiheAudioRountingAvailableDeviceChangeCallback::~TaiheAudioRountingAvailableDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioRountingAvailableDeviceChangeCallback: instance destroy");
}

void TaiheAudioRountingAvailableDeviceChangeCallback::SaveRoutingAvailbleDeviceChangeCbRef(
    OHOS::AudioStandard::AudioDeviceUsage usage, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); ++it) {
        if ((*it).first == nullptr) {
            AUDIO_ERR_LOG("SaveRoutingAvailbleDeviceChangeCbRef: (*it).first is null");
            continue;
        }
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback,
            "SaveRoutingAvailbleDeviceChangeCbRef: audio manager has same callback, nothing to do");
    }

    CHECK_AND_RETURN_LOG(callback != nullptr, "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    availableDeviceChangeCbList_.push_back({cb, usage});
    AUDIO_INFO_LOG("SaveRoutingAvailbleDeviceChange callback ref success, usage [%{public}d], list size [%{public}zu]",
        usage, availableDeviceChangeCbList_.size());
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioRountingAvailableDeviceChangeCallback::RemoveRoutingAvailbleDeviceChangeCbRef(
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); ++it) {
        if ((*it).first == nullptr) {
            AUDIO_ERR_LOG("RemoveRoutingAvailbleDeviceChangeCbRef: (*it).first is null");
            continue;
        }
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveRoutingAvailbleDeviceChangeCbRef: find js callback, erase it");
            availableDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveRoutingAvailbleDeviceChangeCbRef: js callback no find");
}

void TaiheAudioRountingAvailableDeviceChangeCallback::RemoveAllRoutinAvailbleDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
    availableDeviceChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks: remove all js callbacks success");
}

int32_t TaiheAudioRountingAvailableDeviceChangeCallback::GetRoutingAvailbleDeviceChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return availableDeviceChangeCbList_.size();
}

void TaiheAudioRountingAvailableDeviceChangeCallback::OnAvailableDeviceChange(
    const OHOS::AudioStandard::AudioDeviceUsage usage,
    const OHOS::AudioStandard::DeviceChangeAction &deviceChangeAction)
{
    AUDIO_INFO_LOG("OnAvailableDeviceChange:DeviceChangeType: %{public}d, DeviceFlag:%{public}d",
        deviceChangeAction.type, deviceChangeAction.flag);
    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); it++) {
        if (usage == (*it).second) {
            std::unique_ptr<AudioRountingJsCallback> cb = std::make_unique<AudioRountingJsCallback>();
            cb->callback = (*it).first;
            cb->callbackName = AVAILABLE_DEVICE_CHANGE_CALLBACK_NAME;
            cb->deviceChangeAction = deviceChangeAction;
            OnJsCallbackAvailbleDeviceChange(cb);
        }
    }
}

void TaiheAudioRountingAvailableDeviceChangeCallback::OnJsCallbackAvailbleDeviceChange(
    std::unique_ptr<AudioRountingJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAvailbleDeviceChange: OnJsCallbackDeviceChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioRountingJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAvailbleDeviceChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnAvailableDeviceChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioRountingAvailableDeviceChangeCallback::SafeJsCallbackAvailbleDeviceChangeWork(
    AudioRountingJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<AudioRountingJsCallback> safeContext(
        static_cast<AudioRountingJsCallback*>(event),
        [](AudioRountingJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    do {
        std::shared_ptr<taihe::callback<void(DeviceChangeAction const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(DeviceChangeAction const&)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueDeviceChangeAction(event->deviceChangeAction));
    } while (0);
}
} // namespace ANI::Audio