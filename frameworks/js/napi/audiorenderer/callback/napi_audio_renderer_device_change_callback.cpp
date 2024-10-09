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
#define LOG_TAG "NapiAudioRendererDeviceChangeCallback"
#endif

#include "napi_audio_renderer_device_change_callback.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioRendererDeviceChangeCallback::NapiAudioRendererDeviceChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_INFO_LOG("instance create");
}

NapiAudioRendererDeviceChangeCallback::~NapiAudioRendererDeviceChangeCallback()
{
    AUDIO_INFO_LOG("instance destroy");
}

void NapiAudioRendererDeviceChangeCallback::AddCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    bool isEquals = false;
    napi_value copyValue = nullptr;

    for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
        napi_get_reference_value(env_, *ref, &copyValue);
        CHECK_AND_RETURN_LOG(napi_strict_equals(env_, copyValue, args, &isEquals) == napi_ok,
            "get napi_strict_equals failed");
        CHECK_AND_RETURN_LOG(!isEquals, "js Callback already exist");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "AudioRendererDeviceChangeCallbackNapi: creating reference for callback fail");

    callbacks_.push_back(callback);
    AUDIO_INFO_LOG("AddCallbackReference successful");
}

void NapiAudioRendererDeviceChangeCallback::RemoveCallbackReference(napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;
    napi_value copyValue = nullptr;

    if (args == nullptr) {
        for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
            napi_status ret = napi_delete_reference(env, *ref);
            CHECK_AND_RETURN_LOG(napi_ok == ret, "delete callback reference failed");
        }
        callbacks_.clear();
        AUDIO_INFO_LOG("Remove all JS Callback");
        return;
    }

    for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
        napi_get_reference_value(env, *ref, &copyValue);
        CHECK_AND_RETURN_LOG(copyValue != nullptr, "copyValue is nullptr");
        CHECK_AND_RETURN_LOG(napi_strict_equals(env, args, copyValue, &isEquals) == napi_ok,
            "get napi_strict_equals failed");

        if (isEquals == true) {
            AUDIO_INFO_LOG("found JS Callback, delete it!");
            callbacks_.remove(*ref);
            napi_status status = napi_delete_reference(env, *ref);
            CHECK_AND_RETURN_LOG(status == napi_ok, "deleting reference for callback fail");
            return;
        }
    }

    AUDIO_INFO_LOG("RemoveCallbackReference success");
}

void NapiAudioRendererDeviceChangeCallback::RemoveAllCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
        napi_status ret = napi_delete_reference(env_, *ref);
        CHECK_AND_RETURN_LOG(napi_ok == ret, "delete callback reference failed");
    }
    callbacks_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks successful");
}

int32_t NapiAudioRendererDeviceChangeCallback::GetCallbackListSize() const
{
    return callbacks_.size();
}

void NapiAudioRendererDeviceChangeCallback::OnOutputDeviceChange(const DeviceInfo &deviceInfo,
    const AudioStreamDeviceChangeReason reason)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
        OnJsCallbackRendererDeviceInfo(*ref, deviceInfo);
    }
}

void NapiAudioRendererDeviceChangeCallback::WorkCallbackCompleted(uv_work_t *work, int status)
{
    // Js Thread
    std::shared_ptr<AudioRendererDeviceChangeJsCallback> context(
        static_cast<AudioRendererDeviceChangeJsCallback*>(work->data),
        [work](AudioRendererDeviceChangeJsCallback* ptr) {
            delete ptr;
            delete work;
    });

    AudioRendererDeviceChangeJsCallback *event = reinterpret_cast<AudioRendererDeviceChangeJsCallback*>(work->data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback_ != nullptr),
        "OnJsCallbackRendererDeviceInfo: No memory");

    napi_env env = event->env_;
    napi_ref callback = event->callback_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        // Call back function
        napi_value args[ARGS_ONE] = { nullptr };
        nstatus = NapiParamUtils::SetValueDeviceInfo(env, event->deviceInfo_, args[0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            " fail to convert to jsobj");

        const size_t argCount = 1;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call devicechange callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioRendererDeviceChangeCallback::OnJsCallbackRendererDeviceInfo(napi_ref method,
    const DeviceInfo &deviceInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "OnJsCallbackRendererDeviceInfo loop_ is nullptr");
    CHECK_AND_RETURN_LOG(method != nullptr, "OnJsCallbackRendererDeviceInfo method is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackRendererDeviceInfo: No memoryr");

    work->data = new AudioRendererDeviceChangeJsCallback {method, env_, deviceInfo};
    if (work->data == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRendererDeviceInfo failed: No memory");
        delete work;
        return;
    }

    AUDIO_ERR_LOG("OnJsCallbackRendererDeviceInfo");
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackCompleted, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        if (work != nullptr) {
            if (work->data != nullptr) {
                delete reinterpret_cast<AudioRendererDeviceChangeJsCallback*>(work->data);
            }
            delete work;
        }
    }
}

NapiAudioRendererOutputDeviceChangeWithInfoCallback::NapiAudioRendererOutputDeviceChangeWithInfoCallback(napi_env env)
    : env_(env)
{
    AUDIO_INFO_LOG("instance create");
}

NapiAudioRendererOutputDeviceChangeWithInfoCallback::~NapiAudioRendererOutputDeviceChangeWithInfoCallback()
{
    AUDIO_INFO_LOG("instance destroy");
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::AddCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    bool isEquals = false;
    napi_value copyValue = nullptr;

    for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
        napi_get_reference_value(env_, *ref, &copyValue);
        CHECK_AND_RETURN_LOG(napi_strict_equals(env_, copyValue, args, &isEquals) == napi_ok,
            "get napi_strict_equals failed");
        CHECK_AND_RETURN_LOG(!isEquals, "js Callback already exist");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "creating reference for callback fail");

    callbacks_.push_back(callback);
    AUDIO_INFO_LOG("successful");
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::RemoveCallbackReference(napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;
    napi_value copyValue = nullptr;

    if (args == nullptr) {
        for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
            napi_status ret = napi_delete_reference(env, *ref);
            CHECK_AND_RETURN_LOG(napi_ok == ret, "delete callback reference failed");
        }
        callbacks_.clear();
        AUDIO_INFO_LOG("Remove all JS Callback");
        return;
    }

    for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
        napi_get_reference_value(env, *ref, &copyValue);
        CHECK_AND_RETURN_LOG(copyValue != nullptr, "copyValue is nullptr");
        CHECK_AND_RETURN_LOG(napi_strict_equals(env, args, copyValue, &isEquals) == napi_ok,
            "get napi_strict_equals failed");

        if (isEquals == true) {
            AUDIO_INFO_LOG("found JS Callback, delete it!");
            callbacks_.remove(*ref);
            napi_status status = napi_delete_reference(env, *ref);
            CHECK_AND_RETURN_LOG(status == napi_ok, "deleting reference for callback fail");
            return;
        }
    }

    AUDIO_INFO_LOG("RemoveCallbackReference success");
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::RemoveAllCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
        napi_status ret = napi_delete_reference(env_, *ref);
        CHECK_AND_RETURN_LOG(napi_ok == ret, "delete callback reference failed");
    }
    callbacks_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks successful");
}

int32_t NapiAudioRendererOutputDeviceChangeWithInfoCallback::GetCallbackListSize() const
{
    return callbacks_.size();
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::OnOutputDeviceChange(const DeviceInfo &deviceInfo,
    const AudioStreamDeviceChangeReason reason)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto ref = callbacks_.begin(); ref != callbacks_.end(); ++ref) {
        OnJsCallbackOutputDeviceInfo(*ref, deviceInfo, reason);
    }
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::WorkCallbackCompleted(uv_work_t *work, int status)
{
    // Js Thread
    std::shared_ptr<AudioRendererOutputDeviceChangeWithInfoJsCallback> context(
        static_cast<AudioRendererOutputDeviceChangeWithInfoJsCallback*>(work->data),
        [work](AudioRendererOutputDeviceChangeWithInfoJsCallback *ptr) {
            delete ptr;
            delete work;
    });

    AudioRendererOutputDeviceChangeWithInfoJsCallback *event
        = reinterpret_cast<AudioRendererOutputDeviceChangeWithInfoJsCallback*>(work->data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback_ != nullptr),
        "OnJsCallbackRendererDeviceInfo: No memory");

    napi_env env = event->env_;
    napi_ref callback = event->callback_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        // Call back function
        constexpr size_t argCount = ARGS_ONE;
        napi_value args[argCount] = {};
        napi_create_object(env, &args[PARAM0]);
        napi_value deviceObj = nullptr;
        nstatus = NapiParamUtils::SetValueDeviceInfo(env, event->deviceInfo_, deviceObj);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && deviceObj != nullptr,
            " fail to convert to jsobj");
        napi_set_named_property(env, args[PARAM0], "devices", deviceObj);

        nstatus = NapiParamUtils::SetValueInt32(env, "changeReason", static_cast<const int32_t> (event->reason_),
            args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && deviceObj != nullptr,
            " fail to convert to jsobj");

        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call devicechange callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::OnJsCallbackOutputDeviceInfo(napi_ref method,
    const DeviceInfo &deviceInfo, AudioStreamDeviceChangeReason reason)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "OnJsCallbackOutputDeviceInfo loop_ is nullptr");
    CHECK_AND_RETURN_LOG(method != nullptr, "OnJsCallbackOutputDeviceInfo method is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackOutputDeviceInfo: No memoryr");

    work->data = new AudioRendererOutputDeviceChangeWithInfoJsCallback {method, env_, deviceInfo, reason};
    if (work->data == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackOutputDeviceInfo failed: No memory");
        delete work;
        return;
    }

    AUDIO_INFO_LOG("OnJsCallback");
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackCompleted, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        if (work != nullptr) {
            if (work->data != nullptr) {
                delete reinterpret_cast<AudioRendererOutputDeviceChangeWithInfoJsCallback*>(work->data);
            }
            delete work;
        }
    }
}
}  // namespace AudioStandard
}  // namespace OHOS