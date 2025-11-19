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
#define LOG_TAG "TaiheAudioManagerCallback"
#endif

#include "taihe_audio_manager_callbacks.h"

namespace ANI::Audio {
TaiheAudioManagerCallback::TaiheAudioManagerCallback()
{
    AUDIO_INFO_LOG("instance create");
}

TaiheAudioManagerCallback::~TaiheAudioManagerCallback()
{
    AUDIO_INFO_LOG("instance destroy");
}

bool TaiheAudioManagerCallback::IsSameCallback(std::shared_ptr<uintptr_t> &callback,
    std::shared_ptr<uintptr_t> &listCallback)
{
    return TaiheParamUtils::IsSameRef(callback, listCallback);
}

void TaiheAudioManagerCallback::SaveMicrophoneBlockedCallbackReference(std::shared_ptr<uintptr_t> &callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = microphoneBlockedCbList_.begin(); it != microphoneBlockedCbList_.end(); ++it) {
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "audio manager has same callback, nothing to do");
    }

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");
    microphoneBlockedCbList_.push_back({cb});
    AUDIO_INFO_LOG("SaveMicrophoneBlocked callback ref success, list size [%{public}zu]",
        microphoneBlockedCbList_.size());

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

int32_t TaiheAudioManagerCallback::GetMicrophoneBlockedCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return microphoneBlockedCbList_.size();
}

void TaiheAudioManagerCallback::RemoveAllMicrophoneBlockedCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = microphoneBlockedCbList_.begin(); it != microphoneBlockedCbList_.end(); ++it) {
        (*it)->cb_ = nullptr;
    }
    microphoneBlockedCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

void TaiheAudioManagerCallback::RemoveMicrophoneBlockedCallbackReference(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = microphoneBlockedCbList_.begin(); it != microphoneBlockedCbList_.end(); ++it) {
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("find microphoneBlocked callback, remove it");
            (*it)->cb_ = nullptr;
            microphoneBlockedCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("remove microphoneBlocked callback no find");
}

void TaiheAudioManagerCallback::OnMicrophoneBlocked(const OHOS::AudioStandard::MicrophoneBlockedInfo
    &microphoneBlockedInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("status [%{public}d]", microphoneBlockedInfo.blockStatus);

    for (auto it = microphoneBlockedCbList_.begin(); it != microphoneBlockedCbList_.end(); it++) {
        std::unique_ptr<AudioManagerJsCallback> cb = std::make_unique<AudioManagerJsCallback>();
        cb->callback = *it;
        cb->callbackName = MICROPHONE_BLOCKED_CALLBACK_NAME;
        cb->microphoneBlockedInfo = microphoneBlockedInfo;
        OnJsCallbackMicrophoneBlocked(cb);
    }
    return;
}

void TaiheAudioManagerCallback::OnJsCallbackMicrophoneBlocked(std::unique_ptr<AudioManagerJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioManagerJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackMicrophoneBlockedWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnMicBlockStatusChanged", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioManagerCallback::SafeJsCallbackMicrophoneBlockedWork(AudioManagerJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackMicrophoneBlocked: no memory");
    std::shared_ptr<AudioManagerJsCallback> safeContext(
        static_cast<AudioManagerJsCallback*>(event),
        [](AudioManagerJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    std::string request = event->callbackName;

    AUDIO_INFO_LOG("SafeJsCallbackMicrophoneBlockedWork: safe capture state callback working.");
    do {
        std::shared_ptr<taihe::callback<void(DeviceBlockStatusInfo const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(DeviceBlockStatusInfo const&)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "%{public}s get reference value fail", request.c_str());
        (*cacheCallback)(TaiheParamUtils::SetValueBlockedDeviceAction(event->microphoneBlockedInfo));
    } while (0);
}

void TaiheAudioManagerCallback::SaveRoutingManagerDeviceChangeCbRef(OHOS::AudioStandard::DeviceFlag deviceFlag,
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = routingManagerDeviceChangeCbList_.begin(); it != routingManagerDeviceChangeCbList_.end(); ++it) {
        bool isSameCallback = IsSameCallback(callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: has same callback, nothing to do");
    }
    CHECK_AND_RETURN_LOG(callback != nullptr, "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "Memory allocation failed!!");

    routingManagerDeviceChangeCbList_.push_back({cb, deviceFlag});
    AUDIO_INFO_LOG("Save routing device change callback ref success, deviceFlag [%{public}d], list size [%{public}zu]",
        deviceFlag, routingManagerDeviceChangeCbList_.size());

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

int32_t TaiheAudioManagerCallback::GetRoutingManagerDeviceChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return routingManagerDeviceChangeCbList_.size();
}

void TaiheAudioManagerCallback::RemoveRoutingManagerDeviceChangeCbRef(std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = routingManagerDeviceChangeCbList_.begin(); it != routingManagerDeviceChangeCbList_.end(); ++it) {
        if ((*it).first == nullptr) {
            AUDIO_ERR_LOG("RemoveRoutingManagerDeviceChangeCbRef: (*it).first is null");
            continue;
        }
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveRoutingManagerDeviceChangeCbRef: find js callback, erase it");
            routingManagerDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveRoutingManagerDeviceChangeCbRef: js callback no find");
}

void TaiheAudioManagerCallback::RemoveAllRoutingManagerDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
    routingManagerDeviceChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllRoutingManagerDeviceChangeCb: remove all js callbacks success");
}

void TaiheAudioManagerCallback::OnDeviceChange(const OHOS::AudioStandard::DeviceChangeAction &deviceChangeAction)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("OnDeviceChange: type[%{public}d], flag [%{public}d]",
        deviceChangeAction.type, deviceChangeAction.flag);

    for (auto it = audioManagerDeviceChangeCbList_.begin(); it != audioManagerDeviceChangeCbList_.end(); it++) {
        if (deviceChangeAction.flag == (*it).second) {
            std::unique_ptr<AudioManagerJsCallback> cb = std::make_unique<AudioManagerJsCallback>();
            cb->callback = (*it).first;
            cb->callbackName = DEVICE_CHANGE_CALLBACK_NAME;
            cb->deviceChangeAction = deviceChangeAction;
            OnJsCallbackDeviceChange(cb);
        }
    }

    for (auto routingManagerIt = routingManagerDeviceChangeCbList_.begin();
        routingManagerIt != routingManagerDeviceChangeCbList_.end(); routingManagerIt++) {
        if (deviceChangeAction.flag == (*routingManagerIt).second) {
            std::unique_ptr<AudioManagerJsCallback> routingManagerCb = std::make_unique<AudioManagerJsCallback>();
            routingManagerCb->callback = (*routingManagerIt).first;
            routingManagerCb->callbackName = DEVICE_CHANGE_CALLBACK_NAME;
            routingManagerCb->deviceChangeAction = deviceChangeAction;
            OnJsCallbackDeviceChange(routingManagerCb);
        }
    }
    return;
}

void TaiheAudioManagerCallback::OnJsCallbackDeviceChange(std::unique_ptr<AudioManagerJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("TaiheAudioManagerCallback: OnJsCallbackDeviceChange: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioManagerJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackDeviceChangeWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnDeviceChange", 0, OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

void TaiheAudioManagerCallback::SafeJsCallbackDeviceChangeWork(AudioManagerJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<AudioManagerJsCallback> safeContext(
        static_cast<AudioManagerJsCallback*>(event),
        [](AudioManagerJsCallback *ptr) {
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
