/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioSpatializationMgrCallback"
#endif

#include "napi_audio_spatialization_manager_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_manager_callbacks.h"

namespace OHOS {
namespace AudioStandard {
bool NapiAudioSpatializationEnabledChangeCallback::onSpatializationEnabledChangeFlag_;
bool NapiAudioHeadTrackingEnabledChangeCallback::onHeadTrackingEnabledChangeFlag_;
using namespace std;
NapiAudioSpatializationEnabledChangeCallback::NapiAudioSpatializationEnabledChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSpatializationEnabledChangeCallback: instance create");
}

NapiAudioSpatializationEnabledChangeCallback::~NapiAudioSpatializationEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioSpatializationEnabledChangeCallback: instance destroy");
}

void NapiAudioSpatializationEnabledChangeCallback::SaveSpatializationEnabledChangeCallbackReference(napi_value args,
    const std::string cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;

    if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbList_.begin();
            it != spatializationEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        napi_status status = napi_create_reference(env_, args, refCount, &callback);
        CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
            "NapiAudioSpatializationEnabledChangeCallback: creating reference for callback fail");

        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
        CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioSpatializationEnabledChangeCallback: creating callback failed");
        spatializationEnabledChangeCbList_.push_back(cb);
    } else if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbForAnyDeviceList_.begin();
            it != spatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        napi_status status = napi_create_reference(env_, args, refCount, &callback);
        CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
            "NapiAudioSpatializationEnabledChangeCallback: creating reference for callback fail");

        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
        CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioSpatializationEnabledChangeCallback: creating callback failed");
        spatializationEnabledChangeCbForAnyDeviceList_.push_back(cb);
    }
}

void NapiAudioSpatializationEnabledChangeCallback::RemoveSpatializationEnabledChangeCallbackReference(napi_env env,
    napi_value args, const std::string cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbList_.begin();
            it != spatializationEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: find js callback, delete it");
                napi_delete_reference(env, (*it)->cb_);
                (*it)->cb_ = nullptr;
                spatializationEnabledChangeCbList_.erase(it);
                return;
            }
        }
    } else if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbForAnyDeviceList_.begin();
            it != spatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: find js callback, delete it");
                napi_delete_reference(env, (*it)->cb_);
                (*it)->cb_ = nullptr;
                spatializationEnabledChangeCbForAnyDeviceList_.erase(it);
                return;
            }
        }
    }
    AUDIO_INFO_LOG("RemoveSpatializationEnabledChangeCallbackReference: js callback no find");
}

void NapiAudioSpatializationEnabledChangeCallback::RemoveAllSpatializationEnabledChangeCallbackReference(
    const std::string cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbList_.begin();
            it != spatializationEnabledChangeCbList_.end(); ++it) {
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
        }
        spatializationEnabledChangeCbList_.clear();
    } else if (!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = spatializationEnabledChangeCbForAnyDeviceList_.begin();
            it != spatializationEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
        }
        spatializationEnabledChangeCbForAnyDeviceList_.clear();
    }
    AUDIO_INFO_LOG("RemoveAllSpatializationEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t NapiAudioSpatializationEnabledChangeCallback::GetSpatializationEnabledChangeCbListSize(const std::string cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ((!cbName.compare(SPATIALIZATION_ENABLED_CHANGE_CALLBACK_NAME)) ? spatializationEnabledChangeCbList_.size():
        spatializationEnabledChangeCbForAnyDeviceList_.size());
}

void NapiAudioSpatializationEnabledChangeCallback::OnSpatializationEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("OnSpatializationEnabledChange entered");
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

void NapiAudioSpatializationEnabledChangeCallback::OnSpatializationEnabledChangeForAnyDevice(
    const sptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
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
    for (auto it = spatializationEnabledChangeCbForAnyDeviceList_.begin();
        it != spatializationEnabledChangeCbForAnyDeviceList_.end(); it++) {
        std::unique_ptr<AudioSpatializationEnabledJsCallback> cb =
            std::make_unique<AudioSpatializationEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->deviceDescriptor = deviceDescriptor;
        cb->enabled = enabled;
        onSpatializationEnabledChangeFlag_ = false;
        OnJsCallbackSpatializationEnabled(cb);
    }

    return;
}

void NapiAudioSpatializationEnabledChangeCallback::WorkCallbackInterruptDone(uv_work_t *work, int status)
{
    std::shared_ptr<AudioSpatializationEnabledJsCallback> context(
        static_cast<AudioSpatializationEnabledJsCallback*>(work->data),
        [work](AudioSpatializationEnabledJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioSpatializationEnabledJsCallback *event = reinterpret_cast<AudioSpatializationEnabledJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        if (onSpatializationEnabledChangeFlag_) {
            NapiParamUtils::SetValueBoolean(env, event->enabled, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        } else {
            AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice;
            audioSpatialEnabledStateForDevice.deviceDescriptor = event->deviceDescriptor;
            audioSpatialEnabledStateForDevice.enabled = event->enabled;
            NapiParamUtils::SetAudioSpatialEnabledStateForDevice(env, audioSpatialEnabledStateForDevice, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        }

        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call spatialization enabled callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSpatializationEnabledChangeCallback::OnJsCallbackSpatializationEnabled(
    std::unique_ptr<AudioSpatializationEnabledJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackSpatializationEnabled: No memory");

    work->data = reinterpret_cast<void *>(jsCb.get());
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackInterruptDone,
        uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}

NapiAudioHeadTrackingEnabledChangeCallback::NapiAudioHeadTrackingEnabledChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioHeadTrackingEnabledChangeCallback: instance create");
}

NapiAudioHeadTrackingEnabledChangeCallback::~NapiAudioHeadTrackingEnabledChangeCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioHeadTrackingEnabledChangeCallback: instance destroy");
}

void NapiAudioHeadTrackingEnabledChangeCallback::SaveHeadTrackingEnabledChangeCallbackReference(napi_value args,
    const std::string cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        napi_status status = napi_create_reference(env_, args, refCount, &callback);
        CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
            "NapiAudioHeadTrackingEnabledChangeCallback: creating reference for callback fail");

        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
        CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioHeadTrackingEnabledChangeCallback: creating callback failed");

        headTrackingEnabledChangeCbList_.push_back(cb);
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
            it != headTrackingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: spatialization manager has same callback");
        }

        napi_status status = napi_create_reference(env_, args, refCount, &callback);
        CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
            "NapiAudioHeadTrackingEnabledChangeCallback: creating reference for callback fail");

        std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
        CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioHeadTrackingEnabledChangeCallback: creating callback failed");

        headTrackingEnabledChangeCbForAnyDeviceList_.push_back(cb);
    }
}

void NapiAudioHeadTrackingEnabledChangeCallback::RemoveHeadTrackingEnabledChangeCallbackReference(napi_env env,
    napi_value args, const std::string cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: find js callback, delete it");
                napi_delete_reference(env, (*it)->cb_);
                (*it)->cb_ = nullptr;
                headTrackingEnabledChangeCbList_.erase(it);
                return;
            }
        }
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
            it != headTrackingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
            if (isSameCallback) {
                AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: find js callback, delete it");
                napi_delete_reference(env, (*it)->cb_);
                (*it)->cb_ = nullptr;
                headTrackingEnabledChangeCbForAnyDeviceList_.erase(it);
                return;
            }
        }
    }
    AUDIO_INFO_LOG("RemoveHeadTrackingEnabledChangeCallbackReference: js callback no find");
}

void NapiAudioHeadTrackingEnabledChangeCallback::RemoveAllHeadTrackingEnabledChangeCallbackReference(const std::string
    cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbList_.begin(); it != headTrackingEnabledChangeCbList_.end(); ++it) {
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
        }
        headTrackingEnabledChangeCbList_.clear();
    } else if (!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICES_CALLBACK_NAME)) {
        for (auto it = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
            it != headTrackingEnabledChangeCbForAnyDeviceList_.end(); ++it) {
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
        }
        headTrackingEnabledChangeCbForAnyDeviceList_.clear();
    }
    AUDIO_INFO_LOG("RemoveAllHeadTrackingEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t NapiAudioHeadTrackingEnabledChangeCallback::GetHeadTrackingEnabledChangeCbListSize(const std::string cbName)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return ((!cbName.compare(HEAD_TRACKING_ENABLED_CHANGE_CALLBACK_NAME)) ? headTrackingEnabledChangeCbList_.size():
        headTrackingEnabledChangeCbForAnyDeviceList_.size());
}

void NapiAudioHeadTrackingEnabledChangeCallback::OnHeadTrackingEnabledChange(const bool &enabled)
{
    AUDIO_INFO_LOG("OnHeadTrackingEnabledChange entered");
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

void NapiAudioHeadTrackingEnabledChangeCallback::OnHeadTrackingEnabledChangeForAnyDevice(
    const sptr<AudioDeviceDescriptor> &deviceDescriptor, const bool &enabled)
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
    for (auto it = headTrackingEnabledChangeCbForAnyDeviceList_.begin();
        it != headTrackingEnabledChangeCbForAnyDeviceList_.end(); it++) {
        std::unique_ptr<AudioHeadTrackingEnabledJsCallback> cb =
            std::make_unique<AudioHeadTrackingEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->deviceDescriptor = deviceDescriptor;
        cb->enabled = enabled;
        onHeadTrackingEnabledChangeFlag_ = false;
        OnJsCallbackHeadTrackingEnabled(cb);
    }

    return;
}

void NapiAudioHeadTrackingEnabledChangeCallback::WorkCallbackInterruptDone(uv_work_t *work, int status)
{
    std::shared_ptr<AudioHeadTrackingEnabledJsCallback> context(
        static_cast<AudioHeadTrackingEnabledJsCallback*>(work->data),
        [work](AudioHeadTrackingEnabledJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioHeadTrackingEnabledJsCallback *event = reinterpret_cast<AudioHeadTrackingEnabledJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        if (onHeadTrackingEnabledChangeFlag_) {
            NapiParamUtils::SetValueBoolean(env, event->enabled, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        } else {
            AudioSpatialEnabledStateForDevice audioSpatialEnabledStateForDevice;
            audioSpatialEnabledStateForDevice.deviceDescriptor = event->deviceDescriptor;
            audioSpatialEnabledStateForDevice.enabled = event->enabled;
            NapiParamUtils::SetAudioSpatialEnabledStateForDevice(env, audioSpatialEnabledStateForDevice, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");
        }

        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call head tracking enabled callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioHeadTrackingEnabledChangeCallback::OnJsCallbackHeadTrackingEnabled(
    std::unique_ptr<AudioHeadTrackingEnabledJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackHeadTrackingEnabled: No memory");

    work->data = reinterpret_cast<void *>(jsCb.get());
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackInterruptDone,
        uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}
} // namespace AudioStandard
} // namespace OHOS