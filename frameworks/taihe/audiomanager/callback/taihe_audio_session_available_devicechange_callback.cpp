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
#define LOG_TAG "TaiheAudioSessionAvailableDeviceChangeCallback"
#endif

#include "taihe_audio_session_available_devicechange_callback.h"

#include "audio_errors.h"
#include "audio_manager_log.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_manager_callbacks.h"

namespace ANI::Audio {
TaiheAudioSessionAvailableDeviceChangeCallback::TaiheAudioSessionAvailableDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioSessionAvailableDeviceChangeCallback: instance create");
}

TaiheAudioSessionAvailableDeviceChangeCallback::~TaiheAudioSessionAvailableDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioSessionAvailableDeviceChangeCallback: instance destroy");
}

void TaiheAudioSessionAvailableDeviceChangeCallback::SaveSessionAvailbleDeviceChangeCbRef(
    OHOS::AudioStandard::AudioDeviceUsage usage, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); ++it) {
        if ((*it).first == nullptr) {
            AUDIO_ERR_LOG("SaveSessionAvailbleDeviceChangeCbRef: (*it).first is null");
            continue;
        }
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback,
            "SaveSessionAvailbleDeviceChangeCbRef: audio manager has same callback, nothing to do");
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

void TaiheAudioSessionAvailableDeviceChangeCallback::RemoveSessionAvailbleDeviceChangeCbRef(
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); ++it) {
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveSessionAvailbleDeviceChangeCbRef: find js callback, erase it");
            availableDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveSessionAvailbleDeviceChangeCbRef: js callback no find");
}

void TaiheAudioSessionAvailableDeviceChangeCallback::RemoveAllSessionAvailbleDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
    availableDeviceChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks: remove all js callbacks success");
}

int32_t TaiheAudioSessionAvailableDeviceChangeCallback::GetSessionAvailbleDeviceChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return availableDeviceChangeCbList_.size();
}

void TaiheAudioSessionAvailableDeviceChangeCallback::OnAvailableDeviceChange(
    const OHOS::AudioStandard::AudioDeviceUsage usage,
    const OHOS::AudioStandard::DeviceChangeAction &deviceChangeAction)
{
    AUDIO_INFO_LOG("OnAvailableDeviceChange:DeviceChangeType: %{public}d, DeviceFlag:%{public}d",
        deviceChangeAction.type, deviceChangeAction.flag);
    
    // A2DP_IN to SCO
    for (const auto &availableDesc : deviceChangeAction.deviceDescriptors) {
        if (availableDesc->deviceType_ == OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_A2DP_IN) {
            availableDesc->deviceType_ = OHOS::AudioStandard::DEVICE_TYPE_BLUETOOTH_SCO;
        }
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); it++) {
        if (usage == (*it).second) {
            std::unique_ptr<AudioSessionAvailbleDeviceJsCallback> cb =
                std::make_unique<AudioSessionAvailbleDeviceJsCallback>();
            cb->callback = (*it).first;
            cb->callbackName = AVAILABLE_DEVICE_CHANGE_CALLBACK_NAME;
            cb->deviceChangeAction = deviceChangeAction;
            OnJsCallbackAvailbleDeviceChange(cb);
        }
    }
}

void TaiheAudioSessionAvailableDeviceChangeCallback::SafeJsCallbackAvailbleDeviceChangeWork(
    AudioSessionAvailbleDeviceJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAvailbleDeviceChange: no memory");
    std::shared_ptr<AudioSessionAvailbleDeviceJsCallback> safeContext(
        static_cast<AudioSessionAvailbleDeviceJsCallback*>(event),
        [](AudioSessionAvailbleDeviceJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;
    AUDIO_INFO_LOG("SafeJsCallbackAvailbleDeviceChangeWork: safe js callback working.");

    do {
        std::shared_ptr<taihe::callback<void(DeviceChangeAction const& data)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(DeviceChangeAction const& data)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueDeviceChangeAction(event->deviceChangeAction));
    } while (0);
}

void TaiheAudioSessionAvailableDeviceChangeCallback::OnJsCallbackAvailbleDeviceChange(
    std::unique_ptr<AudioSessionAvailbleDeviceJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAvailbleDeviceChange: OnJsCallbackDeviceChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioSessionAvailbleDeviceJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackAvailbleDeviceChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnAvailableDeviceChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio