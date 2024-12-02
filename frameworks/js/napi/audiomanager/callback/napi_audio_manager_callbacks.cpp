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

#include "js_native_api.h"
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
    if (regAmMicBlockedTsfn_) {
        napi_release_threadsafe_function(amMicBlockedTsfn_, napi_tsfn_abort);
    } else if (regAmDevChgTsfn_) {
        napi_release_threadsafe_function(amDevChgTsfn_, napi_tsfn_abort);
    }
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
    } else if (callbackName == MICROPHONE_BLOCKED_CALLBACK_NAME) {
        onMicroPhoneBlockedCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("NapiAudioManagerCallback: Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void NapiAudioManagerCallback::CreateMicBlockedTsfn(napi_env env)
{
    regAmMicBlockedTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "MicBlocked";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        MicrophoneBlockedTsfnFinalize, nullptr, SafeJsCallbackMicrophoneBlockedWork, &amMicBlockedTsfn_);
}

bool NapiAudioManagerCallback::GetMicBlockedTsfnFlag()
{
    return regAmMicBlockedTsfn_;
}

void NapiAudioManagerCallback::CreateDevChgTsfn(napi_env env)
{
    regAmDevChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "ManagerDeviceChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        DeviceChangeTsfnFinalize, nullptr, SafeJsCallbackDeviceChangeWork, &amDevChgTsfn_);
}

bool NapiAudioManagerCallback::GetDevChgTsfnFlag()
{
    return regAmDevChgTsfn_;
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
            AUDIO_INFO_LOG("RemoveRoutingManagerDeviceChangeCbRef: find js callback, erase it");
            routingManagerDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveRoutingManagerDeviceChangeCbRef: js callback no find");
}

void NapiAudioManagerCallback::RemoveAllRoutingManagerDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
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

void NapiAudioManagerCallback::OnMicrophoneBlocked(const MicrophoneBlockedInfo &microphoneBlockedInfo)
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

void NapiAudioManagerCallback::SaveMicrophoneBlockedCallbackReference(napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;

    for (auto it = microphoneBlockedCbList_.begin(); it != microphoneBlockedCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, callback, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "audio manager has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr, "creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    microphoneBlockedCbList_.push_back({cb});
    AUDIO_INFO_LOG("SaveMicrophoneBlocked callback ref success, list size [%{public}zu]",
        microphoneBlockedCbList_.size());
}

void NapiAudioManagerCallback::RemoveMicrophoneBlockedCallbackReference(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = microphoneBlockedCbList_.begin(); it != microphoneBlockedCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, callback, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("find microphoneBlocked callback, remove it");
            napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
            microphoneBlockedCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("remove microphoneBlocked callback no find");
}

void NapiAudioManagerCallback::RemoveAllMicrophoneBlockedCallback()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = microphoneBlockedCbList_.begin(); it != microphoneBlockedCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it)->cb_);
        (*it)->cb_ = nullptr;
    }
    microphoneBlockedCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
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
            AUDIO_INFO_LOG("RemoveAudioManagerDeviceChangeCbRef: find js callback, erase it");
            audioManagerDeviceChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference: js callback no find");
}

void NapiAudioManagerCallback::RemoveAllAudioManagerDeviceChangeCb()
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioManagerDeviceChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks: remove all js callbacks success");
}

void NapiAudioManagerCallback::SafeJsCallbackDeviceChangeWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioManagerJsCallback *event = reinterpret_cast<AudioManagerJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackDeviceChange: no memory");
    std::shared_ptr<AudioManagerJsCallback> safeContext(
        static_cast<AudioManagerJsCallback*>(data),
        [](AudioManagerJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackDeviceChangeWork: safe js callback working.");

    do {
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

void NapiAudioManagerCallback::DeviceChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("DeviceChangeTsfnFinalize: safe thread resource release.");
}

void NapiAudioManagerCallback::OnJsCallbackDeviceChange(std::unique_ptr<AudioManagerJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiAudioManagerCallback: OnJsCallbackDeviceChange: jsCb.get() is null");
        return;
    }
    AudioManagerJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    
    napi_acquire_threadsafe_function(amDevChgTsfn_);
    napi_call_threadsafe_function(amDevChgTsfn_, event, napi_tsfn_blocking);
}

void NapiAudioManagerCallback::SafeJsCallbackMicrophoneBlockedWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioManagerJsCallback *event = reinterpret_cast<AudioManagerJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackMicrophoneBlocked: no memory");
    std::shared_ptr<AudioManagerJsCallback> safeContext(
        static_cast<AudioManagerJsCallback*>(data),
        [](AudioManagerJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackMicrophoneBlockedWork: safe capture state callback working.");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueBlockedDeviceAction(env, event->microphoneBlockedInfo, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
            "%{public}s fail to create microphoneBlocked callback", request.c_str());
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call microphoneBlocked callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioManagerCallback::MicrophoneBlockedTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("RingModeTsfnFinalize: safe thread resource release.");
}

void NapiAudioManagerCallback::OnJsCallbackMicrophoneBlocked(std::unique_ptr<AudioManagerJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("jsCb.get() is null");
        return;
    }

    AudioManagerJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amMicBlockedTsfn_);
    napi_call_threadsafe_function(amMicBlockedTsfn_, event, napi_tsfn_blocking);
}
}  // namespace AudioStandard
}  // namespace OHOS