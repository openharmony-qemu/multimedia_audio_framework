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
#define LOG_TAG "TaiheAudioSpatializationEnabledChangeCallback"
#endif

#include "taihe_audio_spatialization_manager_callback.h"
#include <mutex>
#include <thread>
#include "taihe_param_utils.h"
#include "taihe_audio_manager_callbacks.h"

namespace ANI::Audio {
using namespace std;

bool TaiheAudioSpatializationEnabledChangeCallback::onSpatializationEnabledChangeFlag_;
bool TaiheAudioHeadTrackingEnabledChangeCallback::onHeadTrackingEnabledChangeFlag_;

TaiheAudioSpatializationEnabledChangeCallback::TaiheAudioSpatializationEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioSpatializationEnabledChangeCallback: instance create");
}

TaiheAudioSpatializationEnabledChangeCallback::~TaiheAudioSpatializationEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioSpatializationEnabledChangeCallback: instance destroy");
}

void TaiheAudioSpatializationEnabledChangeCallback::SaveSpatializationEnabledChangeCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!callbackName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbList_.begin();
            it != spatializationEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        CHECK_AND_RETURN_LOG(callback != nullptr, "creating reference for callback fail");
        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
        CHECK_AND_RETURN_LOG(cb != nullptr, "creating callback failed");
        spatializationEnabledChangeCbList_.push_back(cb);
    } else if (!callbackName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto anyDeviceIt = spatializationEnabledChangeCbForAnyDeviceList_.begin();
            anyDeviceIt != spatializationEnabledChangeCbForAnyDeviceList_.end(); ++anyDeviceIt) {
            if (*anyDeviceIt == nullptr) {
                AUDIO_ERR_LOG("SaveSpatializationEnabledChangeCallbackReference: *anyDeviceIt is null");
                continue;
            }
            bool isSame = TaiheAudioManagerCallback::IsSameCallback(callback, (*anyDeviceIt)->cb_);
            CHECK_AND_RETURN_LOG(!isSame, "SaveCallbackReference: spatialization manager has same callback");
        }

        CHECK_AND_RETURN_LOG(callback != nullptr, "creating reference for callback fail");
        std::shared_ptr<AutoRef> anyDeviceCb = std::make_shared<AutoRef>(callback);
        CHECK_AND_RETURN_LOG(anyDeviceCb != nullptr, "creating callback failed");
        spatializationEnabledChangeCbForAnyDeviceList_.push_back(anyDeviceCb);
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioSpatializationEnabledChangeCallback::RemoveSpatializationEnabledChangeCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!callbackName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbList_.begin();
            it != spatializationEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: find js callback, erase it");
                spatializationEnabledChangeCbList_.erase(it);
                return;
            }
        }
    } else if (!callbackName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto anyDeviceIt = spatializationEnabledChangeCbForAnyDeviceList_.begin();
            anyDeviceIt != spatializationEnabledChangeCbForAnyDeviceList_.end(); ++anyDeviceIt) {
            if (*anyDeviceIt == nullptr) {
                AUDIO_ERR_LOG("RemoveSpatializationEnabledChangeCallbackReference: *anyDeviceIt is null");
                continue;
            }
            bool isSame = TaiheAudioManagerCallback::IsSameCallback(callback, (*anyDeviceIt)->cb_);
            if (isSame) {
                AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: find js callback, erase it");
                spatializationEnabledChangeCbForAnyDeviceList_.erase(anyDeviceIt);
                return;
            }
        }
    }
    AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: js callback no find");
}

void TaiheAudioSpatializationEnabledChangeCallback::RemoveAllSpatializationEnabledChangeCallbackReference(
    const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!callbackName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        spatializationEnabledChangeCbList_.clear();
    } else if (!callbackName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        spatializationEnabledChangeCbForAnyDeviceList_.clear();
    }
    AUDIO_INFO_LOG("RemoveAllSpatializationEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t TaiheAudioSpatializationEnabledChangeCallback::GetSpatializationEnabledChangeCbListSize(
    const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ((!callbackName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) ?
        spatializationEnabledChangeCbList_.size():
        spatializationEnabledChangeCbForAnyDeviceList_.size());
}

void TaiheAudioSpatializationEnabledChangeCallback::OnSpatializationEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("enter");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = spatializationEnabledChangeCbList_.begin(); it != spatializationEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioSpatializationEnabledJsCallback> cb =
            std::make_unique<AudioSpatializationEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onSpatializationEnabledChangeFlag_ = true;
        OnJsCallbackSpatializationEnabled(cb);
    }
    return;
}

void TaiheAudioSpatializationEnabledChangeCallback::OnSpatializationEnabledChangeForAnyDevice(
    const std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    AUDIO_INFO_LOG("OnSpatializationEnabledChange by the speified device entered");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = spatializationEnabledChangeCbList_.begin(); it != spatializationEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioSpatializationEnabledJsCallback> cb =
            std::make_unique<AudioSpatializationEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onSpatializationEnabledChangeFlag_ = true;
        OnJsCallbackSpatializationEnabled(cb);
    }
    for (auto anyDeviceIt = spatializationEnabledChangeCbForAnyDeviceList_.begin();
        anyDeviceIt != spatializationEnabledChangeCbForAnyDeviceList_.end(); anyDeviceIt++) {
        std::unique_ptr<AudioSpatializationEnabledJsCallback> anyDeviceCb =
            std::make_unique<AudioSpatializationEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(anyDeviceCb != nullptr, "No memory!!");
        anyDeviceCb->callback = (*anyDeviceIt);
        anyDeviceCb->deviceDescriptor = deviceDescriptor;
        anyDeviceCb->enabled = enabled;
        onSpatializationEnabledChangeFlag_ = false;
        OnJsCallbackSpatializationEnabled(anyDeviceCb);
    }
    return;
}

void TaiheAudioSpatializationEnabledChangeCallback::SafeJsCallbackSpatializationEnabledWork(
    AudioSpatializationEnabledJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackSpatializationEnabledWork: no memory");
    std::shared_ptr<AudioSpatializationEnabledJsCallback> safeContext(
        static_cast<AudioSpatializationEnabledJsCallback*>(event),
        [](AudioSpatializationEnabledJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    AUDIO_INFO_LOG("SafeJsCallbackSpatializationEnabledWork: safe js callback working.");

    do {
        std::shared_ptr<taihe::callback<void(AudioSpatialEnabledStateForDevice const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioSpatialEnabledStateForDevice const&)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "get reference value fail");

        if (!onSpatializationEnabledChangeFlag_) {
            OHOS::AudioStandard::AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice;
            audioSpatialEnabledStateForDevice.deviceDescriptor = event->deviceDescriptor;
            audioSpatialEnabledStateForDevice.enabled = event->enabled;
            AudioSpatialEnabledStateForDevice spatialEnabledStateForDevice =
                TaiheParamUtils::ToTaiheAudioSpatialEnabledStateForDevice(audioSpatialEnabledStateForDevice);
            (*cacheCallback)(spatialEnabledStateForDevice);
        }
    } while (0);
}

void TaiheAudioSpatializationEnabledChangeCallback::OnJsCallbackSpatializationEnabled(
    std::unique_ptr<AudioSpatializationEnabledJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackSpatializationEnabled: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioSpatializationEnabledJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    event->callbackName = "AudioSpatializationEnabled";
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackSpatializationEnabledWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnSpatializationEnabledChangeForAnyDevice", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

TaiheAudioCurrentSpatializationEnabledChangeCallback::TaiheAudioCurrentSpatializationEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioCurrentSpatializationEnabledChangeCallback: instance create");
}

TaiheAudioCurrentSpatializationEnabledChangeCallback::~TaiheAudioCurrentSpatializationEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioCurrentSpatializationEnabledChangeCallback: instance destroy");
}

void TaiheAudioCurrentSpatializationEnabledChangeCallback::SaveCurrentSpatializationEnabledChangeCallbackReference(
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = spatializationEnabledChangeCbForCurrentDeviceList_.begin();
        it != spatializationEnabledChangeCbForCurrentDeviceList_.end(); ++it) {
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
    }

    CHECK_AND_RETURN_LOG(callback != nullptr,
        "TaiheAudioCurrentSpatializationEnabledChangeCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "TaiheAudioCurrentSpatializationEnabledChangeCallback:create callback failed");

    spatializationEnabledChangeCbForCurrentDeviceList_.push_back(cb);
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioCurrentSpatializationEnabledChangeCallback::RemoveCurrentSpatializationEnabledChangeCallbackReference(
    std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = spatializationEnabledChangeCbForCurrentDeviceList_.begin();
        it != spatializationEnabledChangeCbForCurrentDeviceList_.end(); ++it) {
        bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveCurrentSpatializationEnabledChangeCallbackReference: find js callback,"
                "erase it");
            spatializationEnabledChangeCbForCurrentDeviceList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCurrentSpatializationEnabledChangeCallbackReference: js callback no find");
}

void TaiheAudioCurrentSpatializationEnabledChangeCallback::RemoveAllCurrentSpatializationEnabledChangeCallbackReference(
    )
{
    std::lock_guard<std::mutex> lock(mutex_);
    spatializationEnabledChangeCbForCurrentDeviceList_.clear();
    AUDIO_INFO_LOG("RemoveAllCurrentSpatializationEnabledChangeCallbackReference: remove all js callbacks"
        "success");
}

int32_t TaiheAudioCurrentSpatializationEnabledChangeCallback::GetCurrentSpatializationEnabledChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return spatializationEnabledChangeCbForCurrentDeviceList_.size();
}

void TaiheAudioCurrentSpatializationEnabledChangeCallback::OnSpatializationEnabledChangeForCurrentDevice(
    const bool &enabled)
{
    AUDIO_INFO_LOG("enter");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = spatializationEnabledChangeCbForCurrentDeviceList_.begin();
        it != spatializationEnabledChangeCbForCurrentDeviceList_.end(); it++) {
        std::unique_ptr<AudioSpatializationEnabledForCurrentDeviceJsCallback> cb =
            std::make_unique<AudioSpatializationEnabledForCurrentDeviceJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        OnJsCallbackSpatializationEnabledForCurrentDevice(cb);
    }

    return;
}

void TaiheAudioCurrentSpatializationEnabledChangeCallback::SafeJsCallbackSpatializationEnabledForCurrentDeviceWork(
    AudioSpatializationEnabledForCurrentDeviceJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackSpatializationEnabledForCurrentDeviceWork: no memory");
    std::shared_ptr<AudioSpatializationEnabledForCurrentDeviceJsCallback> safeContext(
        static_cast<AudioSpatializationEnabledForCurrentDeviceJsCallback*>(event),
        [](AudioSpatializationEnabledForCurrentDeviceJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    AUDIO_INFO_LOG("SafeJsCallbackSpatializationEnabledForCurrentDeviceWork: safe js callback working.");

    do {
        std::shared_ptr<taihe::callback<void(bool)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(bool)>>(event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "get reference value fail");
        (*cacheCallback)(event->enabled);
    } while (0);
}

void TaiheAudioCurrentSpatializationEnabledChangeCallback::OnJsCallbackSpatializationEnabledForCurrentDevice(
    std::unique_ptr<AudioSpatializationEnabledForCurrentDeviceJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackSpatializationEnabledForCurrentDevice: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioSpatializationEnabledForCurrentDeviceJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackSpatializationEnabledForCurrentDeviceWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnSpatializationEnabledChangeForCurrentDevice", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}

TaiheAudioHeadTrackingEnabledChangeCallback::TaiheAudioHeadTrackingEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioHeadTrackingEnabledChangeCallback: instance create");
}

TaiheAudioHeadTrackingEnabledChangeCallback::~TaiheAudioHeadTrackingEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("TaiheAudioHeadTrackingEnabledChangeCallback: instance destroy");
}

void TaiheAudioHeadTrackingEnabledChangeCallback::SaveHeadTrackingEnabledChangeCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!callbackName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        CHECK_AND_RETURN_LOG(callback != nullptr, "creating reference for callback fail");
        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(callback);
        CHECK_AND_RETURN_LOG(cb != nullptr, "creating callback failed");

        headTrackingEnabledChangeCbList_.push_back(cb);
    } else if (!callbackName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto anyDeviceIt = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
            anyDeviceIt != headTrackingEnabledChangeCbForAnyDeviceList_.end(); ++anyDeviceIt) {
            if (*anyDeviceIt == nullptr) {
                AUDIO_ERR_LOG("SaveHeadTrackingEnabledChangeCallbackReference: *anyDeviceIt is null");
                continue;
            }
            bool isSame = TaiheAudioManagerCallback::IsSameCallback(callback, (*anyDeviceIt)->cb_);
            CHECK_AND_RETURN_LOG(!isSame, "SaveCallbackReference: spatialization manager has same callback");
        }

        CHECK_AND_RETURN_LOG(callback != nullptr, "creating reference for callback fail");
        std::shared_ptr<AutoRef> anyDeviceCb = std::make_shared<AutoRef>(callback);
        CHECK_AND_RETURN_LOG(anyDeviceCb != nullptr, "creating callback failed");
        headTrackingEnabledChangeCbForAnyDeviceList_.push_back(anyDeviceCb);
    }

    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        CHECK_AND_RETURN_LOG(runner != nullptr, "runner is null");
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    } else {
        AUDIO_DEBUG_LOG("mainHandler_ is not nullptr");
    }
}

void TaiheAudioHeadTrackingEnabledChangeCallback::RemoveHeadTrackingEnabledChangeCallbackReference(
    const std::string &callbackName, std::shared_ptr<uintptr_t> callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!callbackName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = TaiheAudioManagerCallback::IsSameCallback(callback, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: find js callback, erase it");
                headTrackingEnabledChangeCbList_.erase(it);
                return;
            }
        }
    } else if (!callbackName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto anyDeviceIt = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
            anyDeviceIt != headTrackingEnabledChangeCbForAnyDeviceList_.end(); ++anyDeviceIt) {
            if (*anyDeviceIt == nullptr) {
                AUDIO_ERR_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: *anyDeviceIt is null");
                continue;
            }
            bool isSame = TaiheAudioManagerCallback::IsSameCallback(callback, (*anyDeviceIt)->cb_);
            if (isSame) {
                AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: find js callback, erase it");
                headTrackingEnabledChangeCbForAnyDeviceList_.erase(anyDeviceIt);
                return;
            }
        }
    }
    AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: js callback no find");
}

void TaiheAudioHeadTrackingEnabledChangeCallback::RemoveAllHeadTrackingEnabledChangeCallbackReference(const std::string
    &callbackName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!callbackName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        headTrackingEnabledChangeCbList_.clear();
    } else if (!callbackName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        headTrackingEnabledChangeCbForAnyDeviceList_.clear();
    }
    AUDIO_INFO_LOG("RemoveAllHeadTrackingEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t TaiheAudioHeadTrackingEnabledChangeCallback::GetHeadTrackingEnabledChangeCbListSize(
    const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ((!callbackName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) ?
        headTrackingEnabledChangeCbList_.size():
        headTrackingEnabledChangeCbForAnyDeviceList_.size());
}

void TaiheAudioHeadTrackingEnabledChangeCallback::OnHeadTrackingEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("enter");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioHeadTrackingEnabledJsCallback> cb =
            std::make_unique<AudioHeadTrackingEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onHeadTrackingEnabledChangeFlag_ = true;
        OnJsCallbackHeadTrackingEnabled(cb);
    }

    return;
}

void TaiheAudioHeadTrackingEnabledChangeCallback::OnHeadTrackingEnabledChangeForAnyDevice(
    const std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
{
    AUDIO_INFO_LOG("OnHeadTrackingEnabledChange by the specified device entered");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioHeadTrackingEnabledJsCallback> cb =
            std::make_unique<AudioHeadTrackingEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onHeadTrackingEnabledChangeFlag_ = true;
        OnJsCallbackHeadTrackingEnabled(cb);
    }
    for (auto anyDeviceIt = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
        anyDeviceIt != headTrackingEnabledChangeCbForAnyDeviceList_.end(); anyDeviceIt++) {
        std::unique_ptr<AudioHeadTrackingEnabledJsCallback> anyDeviceCb =
            std::make_unique<AudioHeadTrackingEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(anyDeviceCb != nullptr, "No memory!!");
        anyDeviceCb->callback = (*anyDeviceIt);
        anyDeviceCb->deviceDescriptor = deviceDescriptor;
        anyDeviceCb->enabled = enabled;
        onHeadTrackingEnabledChangeFlag_ = false;
        OnJsCallbackHeadTrackingEnabled(anyDeviceCb);
    }

    return;
}

void TaiheAudioHeadTrackingEnabledChangeCallback::SafeJsCallbackHeadTrackingEnabledWork(
    AudioHeadTrackingEnabledJsCallback *event)
{
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackHeadTrackingEnabledWork: no memory");
    std::shared_ptr<AudioHeadTrackingEnabledJsCallback> safeContext(
        static_cast<AudioHeadTrackingEnabledJsCallback*>(event),
        [](AudioHeadTrackingEnabledJsCallback *ptr) {
            if (ptr != nullptr) {
                delete ptr;
            }
    });
    AUDIO_INFO_LOG("SafeJsCallbackHeadTrackingEnabledWork: safe js callback working.");

    do {
        std::shared_ptr<taihe::callback<void(AudioSpatialEnabledStateForDevice const&)>> cacheCallback =
            std::reinterpret_pointer_cast<taihe::callback<void(AudioSpatialEnabledStateForDevice const&)>>(
                event->callback->cb_);
        CHECK_AND_BREAK_LOG(cacheCallback != nullptr, "get reference value fail");
        AUDIO_INFO_LOG("SafeJsCallbackHeadTrackingEnabledWork: safe js callback working.");

        if (!onHeadTrackingEnabledChangeFlag_) {
            OHOS::AudioStandard::AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice;
            audioSpatialEnabledStateForDevice.deviceDescriptor = event->deviceDescriptor;
            audioSpatialEnabledStateForDevice.enabled = event->enabled;
            AudioSpatialEnabledStateForDevice spatialEnabledStateForDevice =
                TaiheParamUtils::ToTaiheAudioSpatialEnabledStateForDevice(audioSpatialEnabledStateForDevice);
            (*cacheCallback)(spatialEnabledStateForDevice);
        }
    } while (0);
}

void TaiheAudioHeadTrackingEnabledChangeCallback::OnJsCallbackHeadTrackingEnabled(
    std::unique_ptr<AudioHeadTrackingEnabledJsCallback> &jsCb)
{
    if (jsCb == nullptr || jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackHeadTrackingEnabled: jsCb.get() is null");
        return;
    }
    CHECK_AND_RETURN_LOG(mainHandler_ != nullptr, "mainHandler_ is nullptr");
    AudioHeadTrackingEnabledJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    auto sharePtr = shared_from_this();
    auto task = [event, sharePtr]() {
        if (sharePtr != nullptr) {
            sharePtr->SafeJsCallbackHeadTrackingEnabledWork(event);
        }
    };
    mainHandler_->PostTask(task, "OnHeadTrackingEnabledChangeForAnyDevice", 0,
        OHOS::AppExecFwk::EventQueue::Priority::IMMEDIATE, {});
}
} // namespace ANI::Audio