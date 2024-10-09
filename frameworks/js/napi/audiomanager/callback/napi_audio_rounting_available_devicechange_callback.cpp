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
#define LOG_TAG "NapiAudioRountingAvailableDeviceChangeCallback"
#endif

#include "napi_audio_rounting_available_devicechange_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_audio_manager_callbacks.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioRountingAvailableDeviceChangeCallback::NapiAudioRountingAvailableDeviceChangeCallback(napi_env env)
{
    env_ = env;
    AUDIO_DEBUG_LOG("NapiAudioRountingAvailableDeviceChangeCallback: instance create");
}

NapiAudioRountingAvailableDeviceChangeCallback::~NapiAudioRountingAvailableDeviceChangeCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioRountingAvailableDeviceChangeCallback: instance destroy");
}

void NapiAudioRountingAvailableDeviceChangeCallback::SaveRoutingAvailbleDeviceChangeCbRef(AudioDeviceUsage usage,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;

    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, callback, (*it).first->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback,
            "SaveRoutingAvailbleDeviceChangeCbRef: audio manager has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    availableDeviceChangeCbList_.push_back({cb, usage});
    AUDIO_INFO_LOG("SaveRoutingAvailbleDeviceChange callback ref success, usage [%{public}d], list size [%{public}zu]",
        usage, availableDeviceChangeCbList_.size());
}

void NapiAudioRountingAvailableDeviceChangeCallback::RemoveRoutingAvailbleDeviceChangeCbRef(napi_env env,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, callback, (*it).first->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveRoutingAvailbleDeviceChangeCbRef: find js callback, delete it");
            napi_delete_reference(env, (*it).first->cb_);
            (*it).first->cb_ = nullptr;
            availableDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveRoutingAvailbleDeviceChangeCbRef: js callback no find");
}

void NapiAudioRountingAvailableDeviceChangeCallback::RemoveAllRoutinAvailbleDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = availableDeviceChangeCbList_.begin(); it != availableDeviceChangeCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it).first->cb_);
        (*it).first->cb_ = nullptr;
    }
    availableDeviceChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks: remove all js callbacks success");
}

int32_t NapiAudioRountingAvailableDeviceChangeCallback::GetRoutingAvailbleDeviceChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return availableDeviceChangeCbList_.size();
}

void NapiAudioRountingAvailableDeviceChangeCallback::OnAvailableDeviceChange(
    const AudioDeviceUsage usage, const DeviceChangeAction &deviceChangeAction)
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

void NapiAudioRountingAvailableDeviceChangeCallback::WorkAvailbleDeviceChangeDone(uv_work_t *work, int status)
{
    // Js Thread
    std::shared_ptr<AudioRountingJsCallback> context(
        static_cast<AudioRountingJsCallback*>(work->data),
        [work](AudioRountingJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioRountingJsCallback *event = reinterpret_cast<AudioRountingJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    std::string request = event->callbackName;
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueDeviceChangeAction(env, event->deviceChangeAction, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create DeviceChange callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call DeviceChange callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioRountingAvailableDeviceChangeCallback::OnJsCallbackAvailbleDeviceChange(
    std::unique_ptr<AudioRountingJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is null: No memory");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackDeviceChange: No memory");

    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("AudioManagerCallbackNapi: OnJsCallbackDeviceChange: jsCb.get() is null");
        delete work;
        return;
    }

    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {},
        WorkAvailbleDeviceChangeDone, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}
} // namespace AudioStandard
} // namespace OHOS