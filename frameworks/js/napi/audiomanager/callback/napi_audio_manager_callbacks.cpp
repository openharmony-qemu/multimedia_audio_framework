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
#define LOG_TAG "NapiAudioManagerCallback"
#endif

#include "napi_audio_manager_callbacks.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"

namespace OHOS {
namespace AudioStandard {

bool NapiAudioManagerCallback::IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env, refCallback, &copyValue);
    if (napi_strict_equals(env, copyValue, callback, &isEquals) != napi_ok) {
        AUDIO_ERR_LOG("IsSameCallback: get napi_strict_equals failed");
        return false;
    }

    return isEquals;
}

NapiAudioManagerCallback::NapiAudioManagerCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioManagerCallback: instance create");
}

NapiAudioManagerCallback::~NapiAudioManagerCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioManagerCallback: instance destroy");
}

void NapiAudioManagerCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
                         "NapiAudioManagerCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == DEVICE_CHANGE_CALLBACK_NAME) {
        deviceChangeCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("NapiAudioManagerCallback: Unknown callback type: %{public}s", callbackName.c_str());
    }
}

int32_t NapiAudioManagerCallback::GetAudioManagerDeviceChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return audioManagerDeviceChangeCbList_.size();
}

void NapiAudioManagerCallback::SaveRoutingManagerDeviceChangeCbRef(DeviceFlag deviceFlag, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;

    for (auto it = routingManagerDeviceChangeCbList_.begin(); it != routingManagerDeviceChangeCbList_.end(); ++it) {
        bool isSameCallback = IsSameCallback(env_, callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);

    routingManagerDeviceChangeCbList_.push_back({cb, deviceFlag});
    AUDIO_INFO_LOG("Save routing device change callback ref success, deviceFlag [%{public}d], list size [%{public}zu]",
        deviceFlag, routingManagerDeviceChangeCbList_.size());
}

void NapiAudioManagerCallback::RemoveRoutingManagerDeviceChangeCbRef(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = routingManagerDeviceChangeCbList_.begin(); it != routingManagerDeviceChangeCbList_.end(); ++it) {
        bool isSameCallback = IsSameCallback(env_, callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveRoutingManagerDeviceChangeCbRef: find js callback, delete it");
            napi_delete_reference(env, (*it).first->cb_);
            (*it).first->cb_ = nullptr;
            routingManagerDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveRoutingManagerDeviceChangeCbRef: js callback no find");
}

void NapiAudioManagerCallback::RemoveAllRoutingManagerDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = routingManagerDeviceChangeCbList_.begin(); it != routingManagerDeviceChangeCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it).first->cb_);
        (*it).first->cb_ = nullptr;
    }
    routingManagerDeviceChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllRoutingManagerDeviceChangeCb: remove all js callbacks success");
}

int32_t NapiAudioManagerCallback::GetRoutingManagerDeviceChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return routingManagerDeviceChangeCbList_.size();
}

void NapiAudioManagerCallback::OnDeviceChange(const DeviceChangeAction &deviceChangeAction)
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

    for (auto it = routingManagerDeviceChangeCbList_.begin(); it != routingManagerDeviceChangeCbList_.end(); it++) {
        if (deviceChangeAction.flag == (*it).second) {
            std::unique_ptr<AudioManagerJsCallback> cb = std::make_unique<AudioManagerJsCallback>();
            cb->callback = (*it).first;
            cb->callbackName = DEVICE_CHANGE_CALLBACK_NAME;
            cb->deviceChangeAction = deviceChangeAction;
            OnJsCallbackDeviceChange(cb);
        }
    }
    return;
}

void NapiAudioManagerCallback::SaveAudioManagerDeviceChangeCbRef(DeviceFlag deviceFlag, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = 1;

    for (auto it = audioManagerDeviceChangeCbList_.begin(); it != audioManagerDeviceChangeCbList_.end(); ++it) {
        bool isSameCallback = IsSameCallback(env_, callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: audio manager has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    audioManagerDeviceChangeCbList_.push_back({cb, deviceFlag});
    AUDIO_INFO_LOG("Save manager device change callback ref success, deviceFlag [%{public}d], list size [%{public}zu]",
        deviceFlag, audioManagerDeviceChangeCbList_.size());
}

void NapiAudioManagerCallback::RemoveAudioManagerDeviceChangeCbRef(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = audioManagerDeviceChangeCbList_.begin(); it != audioManagerDeviceChangeCbList_.end(); ++it) {
        bool isSameCallback = IsSameCallback(env_, callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveAudioManagerDeviceChangeCbRef: find js callback, delete it");
            napi_delete_reference(env, (*it).first->cb_);
            (*it).first->cb_ = nullptr;
            audioManagerDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference: js callback no find");
}

void NapiAudioManagerCallback::RemoveAllAudioManagerDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioManagerDeviceChangeCbList_.begin(); it != audioManagerDeviceChangeCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it).first->cb_);
        (*it).first->cb_ = nullptr;
    }
    audioManagerDeviceChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks: remove all js callbacks success");
}

void NapiAudioManagerCallback::WorkCallbackInterruptDone(uv_work_t *work, int status)
{
    std::shared_ptr<AudioManagerJsCallback> context(
        static_cast<AudioManagerJsCallback*>(work->data),
        [work](AudioManagerJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioManagerJsCallback *event = reinterpret_cast<AudioManagerJsCallback *>(work->data);
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
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueDeviceChangeAction(env, event->deviceChangeAction, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
            "%{public}s fail to create DeviceChange callback", request.c_str());
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call DeviceChange callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioManagerCallback::OnJsCallbackDeviceChange(std::unique_ptr<AudioManagerJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackDeviceChange: No memory");

    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiAudioManagerCallback: OnJsCallbackDeviceChange: jsCb.get() is null");
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