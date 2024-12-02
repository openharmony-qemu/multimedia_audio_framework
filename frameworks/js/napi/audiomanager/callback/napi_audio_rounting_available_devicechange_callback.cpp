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

#include "js_native_api.h"
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
    if (regAmRouDevChgTsfn_) {
        napi_release_threadsafe_function(amRouDevChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioRountingAvailableDeviceChangeCallback: instance destroy");
}

void NapiAudioRountingAvailableDeviceChangeCallback::CreateRouDevChgTsfn(napi_env env)
{
    regAmRouDevChgTsfn_ = true;
    std::string callbackName = "RountingAvailableDeviceChange";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        AvailbleDeviceChangeTsfnFinalize, nullptr, SafeJsCallbackAvailbleDeviceChangeWork, &amRouDevChgTsfn_);
}

bool NapiAudioRountingAvailableDeviceChangeCallback::GetRouDevChgTsfnFlag()
{
    return regAmRouDevChgTsfn_;
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
            AUDIO_INFO_LOG("RemoveRoutingAvailbleDeviceChangeCbRef: find js callback, erase it");
            availableDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveRoutingAvailbleDeviceChangeCbRef: js callback no find");
}

void NapiAudioRountingAvailableDeviceChangeCallback::RemoveAllRoutinAvailbleDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
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

void NapiAudioRountingAvailableDeviceChangeCallback::SafeJsCallbackAvailbleDeviceChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioRountingJsCallback *event = reinterpret_cast<AudioRountingJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAvailbleDeviceChange: no memory");
    std::shared_ptr<AudioRountingJsCallback> safeContext(
        static_cast<AudioRountingJsCallback*>(data),
        [](AudioRountingJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAvailbleDeviceChangeWork: safe js callback working.");

    do {
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

void NapiAudioRountingAvailableDeviceChangeCallback::AvailbleDeviceChangeTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AvailbleDeviceChangeTsfnFinalize: safe thread resource release.");
}

void NapiAudioRountingAvailableDeviceChangeCallback::OnJsCallbackAvailbleDeviceChange(
    std::unique_ptr<AudioRountingJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAvailbleDeviceChange: OnJsCallbackDeviceChange: jsCb.get() is null");
        return;
    }

    AudioRountingJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amRouDevChgTsfn_);
    napi_call_threadsafe_function(amRouDevChgTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS