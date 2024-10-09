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
#define LOG_TAG "NapiAudioRoutingMgrCallbacks"
#endif

#include "napi_audio_routing_manager_callbacks.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_audio_manager_callbacks.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioPreferredOutputDeviceChangeCallback::NapiAudioPreferredOutputDeviceChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioPreferredOutputDeviceChangeCallback: instance create");
}

NapiAudioPreferredOutputDeviceChangeCallback::~NapiAudioPreferredOutputDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioPreferredOutputDeviceChangeCallback: instance destroy");
}

void NapiAudioPreferredOutputDeviceChangeCallback::SaveCallbackReference(AudioStreamType streamType,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;

    bool isSameCallback = true;
    for (auto it = preferredOutputDeviceCbList_.begin(); it != preferredOutputDeviceCbList_.end(); ++it) {
        isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    preferredOutputDeviceCbList_.push_back({cb, streamType});
    AUDIO_INFO_LOG("Save callback reference success, prefer ouput device callback list size [%{public}zu]",
        preferredOutputDeviceCbList_.size());
}

void NapiAudioPreferredOutputDeviceChangeCallback::RemoveCallbackReference(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (callback == nullptr) {
        AUDIO_INFO_LOG("RemoveCallbackReference: js callback is nullptr, remove all callback reference");
        RemoveAllCallbacks();
        return;
    }
    for (auto it = preferredOutputDeviceCbList_.begin(); it != preferredOutputDeviceCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveCallbackReference: find js callback, delete it");
            napi_status status = napi_delete_reference(env, (*it).first->cb_);
            (*it).first->cb_ = nullptr;
            CHECK_AND_RETURN_LOG(status == napi_ok, "RemoveCallbackReference: delete reference for callback fail");
            preferredOutputDeviceCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference: js callback no find");
}

void NapiAudioPreferredOutputDeviceChangeCallback::RemoveAllCallbacks()
{
    for (auto it = preferredOutputDeviceCbList_.begin(); it != preferredOutputDeviceCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it).first->cb_);
        (*it).first->cb_ = nullptr;
    }
    preferredOutputDeviceCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks: remove all js callbacks success");
}

void NapiAudioPreferredOutputDeviceChangeCallback::OnPreferredOutputDeviceUpdated(
    const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(preferredOutputDeviceCbList_.size() > 0,
        "Cannot find the reference of prefer device callback");
    AUDIO_DEBUG_LOG("OnPreferredOutputDeviceUpdated: Cb list size [%{public}zu]",
        preferredOutputDeviceCbList_.size());

    for (auto it = preferredOutputDeviceCbList_.begin(); it != preferredOutputDeviceCbList_.end(); it++) {
        std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> cb =
            std::make_unique<AudioActiveOutputDeviceChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");

        cb->callback = (*it).first;
        cb->callbackName = PREFERRED_OUTPUT_DEVICE_CALLBACK_NAME;
        cb->desc = desc;
        OnJsCallbackActiveOutputDeviceChange(cb);
    }
    return;
}

void NapiAudioPreferredOutputDeviceChangeCallback::WorkCallbackInterruptDone(uv_work_t *work, int status)
{
    std::shared_ptr<AudioActiveOutputDeviceChangeJsCallback> context(
        static_cast<AudioActiveOutputDeviceChangeJsCallback*>(work->data),
        [work](AudioActiveOutputDeviceChangeJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioActiveOutputDeviceChangeJsCallback *event =
        reinterpret_cast<AudioActiveOutputDeviceChangeJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    std::string request = event->callbackName;
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());

        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetDeviceDescriptors(env, event->desc, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create ringer mode callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call ringer mode callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioPreferredOutputDeviceChangeCallback::OnJsCallbackActiveOutputDeviceChange(
    std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackDeviceChange: No memory");

    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackDeviceChange: jsCb.get() is null");
        delete work;
        return;
    }

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

NapiAudioPreferredInputDeviceChangeCallback::NapiAudioPreferredInputDeviceChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioPreferredInputDeviceChangeCallback: instance create");
}

NapiAudioPreferredInputDeviceChangeCallback::~NapiAudioPreferredInputDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioPreferredInputDeviceChangeCallback: instance destroy");
}

void NapiAudioPreferredInputDeviceChangeCallback::SaveCallbackReference(SourceType sourceType, napi_value callback)
{
    std::lock_guard<std::mutex> lock(preferredInputListMutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;

    bool isSameCallback = true;
    for (auto it = preferredInputDeviceCbList_.begin(); it != preferredInputDeviceCbList_.end(); ++it) {
        isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    preferredInputDeviceCbList_.push_back({cb, sourceType});
    AUDIO_INFO_LOG("Save callback reference success, prefer input device callback list size [%{public}zu]",
        preferredInputDeviceCbList_.size());
}

void NapiAudioPreferredInputDeviceChangeCallback::RemoveCallbackReference(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(preferredInputListMutex_);

    if (callback == nullptr) {
        AUDIO_INFO_LOG("RemoveCallbackReference: js callback is nullptr, remove all callback reference");
        RemoveAllCallbacks();
        return;
    }
    for (auto it = preferredInputDeviceCbList_.begin(); it != preferredInputDeviceCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveCallbackReference: find js callback, delete it");
            napi_status status = napi_delete_reference(env, (*it).first->cb_);
            (*it).first->cb_ = nullptr;
            CHECK_AND_RETURN_LOG(status == napi_ok, "RemoveCallbackReference: delete reference for callback fail");
            preferredInputDeviceCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference: js callback no find");
}

void NapiAudioPreferredInputDeviceChangeCallback::RemoveAllCallbacks()
{
    for (auto it = preferredInputDeviceCbList_.begin(); it != preferredInputDeviceCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it).first->cb_);
        (*it).first->cb_ = nullptr;
    }
    preferredInputDeviceCbList_.clear();
}

void NapiAudioPreferredInputDeviceChangeCallback::OnPreferredInputDeviceUpdated(
    const std::vector<sptr<AudioDeviceDescriptor>> &desc)
{
    std::lock_guard<std::mutex> lock(preferredInputListMutex_);
    CHECK_AND_RETURN_LOG(preferredInputDeviceCbList_.size() > 0, "Cannot find the reference of prefer device callback");
    AUDIO_DEBUG_LOG("OnPreferredInputDeviceUpdated: Cb list size [%{public}zu]", preferredInputDeviceCbList_.size());

    for (auto it = preferredInputDeviceCbList_.begin(); it != preferredInputDeviceCbList_.end(); it++) {
        std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> cb =
            std::make_unique<AudioActiveInputDeviceChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");

        cb->callback = (*it).first;
        cb->callbackName = PREFERRED_INPUT_DEVICE_CALLBACK_NAME;
        cb->desc = desc;
        OnJsCallbackActiveInputDeviceChange(cb);
    }
    return;
}

void NapiAudioPreferredInputDeviceChangeCallback::WorkCallbackInterruptDone(uv_work_t *work, int status)
{
    std::shared_ptr<AudioActiveInputDeviceChangeJsCallback> context(
        static_cast<AudioActiveInputDeviceChangeJsCallback*>(work->data),
        [work](AudioActiveInputDeviceChangeJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioActiveInputDeviceChangeJsCallback *event =
        reinterpret_cast<AudioActiveInputDeviceChangeJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    std::string request = event->callbackName;
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());

        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetDeviceDescriptors(env, event->desc, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create ringer mode callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call ringer mode callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioPreferredInputDeviceChangeCallback::OnJsCallbackActiveInputDeviceChange(
    std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackDeviceChange: No memory");

    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackDeviceChange: jsCb.get() is null");
        delete work;
        return;
    }

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
}  // namespace AudioStandard
}  // namespace OHOS
