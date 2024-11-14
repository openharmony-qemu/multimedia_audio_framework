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

#include "js_native_api.h"
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
    if (regAmOutputDevChgTsfn_) {
        napi_release_threadsafe_function(amOutputDevChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioPreferredOutputDeviceChangeCallback: instance destroy");
}

void NapiAudioPreferredOutputDeviceChangeCallback::CreatePreferredOutTsfn(napi_env env)
{
    regAmOutputDevChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "PreferredOutputDeviceChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        ActiveOutputDeviceChangeTsfnFinalize, nullptr, SafeJsCallbackActiveOutputDeviceChangeWork,
        &amOutputDevChgTsfn_);
}

bool NapiAudioPreferredOutputDeviceChangeCallback::GetPreferredOutTsfnFlag()
{
    return regAmOutputDevChgTsfn_;
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
            preferredOutputDeviceCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference: js callback no find");
}

void NapiAudioPreferredOutputDeviceChangeCallback::RemoveAllCallbacks()
{
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

void NapiAudioPreferredOutputDeviceChangeCallback::SafeJsCallbackActiveOutputDeviceChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioActiveOutputDeviceChangeJsCallback *event = reinterpret_cast<AudioActiveOutputDeviceChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackActiveOutputDeviceChange: no memory");
    std::shared_ptr<AudioActiveOutputDeviceChangeJsCallback> safeContext(
        static_cast<AudioActiveOutputDeviceChangeJsCallback*>(data),
        [](AudioActiveOutputDeviceChangeJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackActiveOutputDeviceChangeWork: safe js callback working.");

    do {
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

void NapiAudioPreferredOutputDeviceChangeCallback::ActiveOutputDeviceChangeTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("RingModeTsfnFinalize: safe thread resource release.");
}

void NapiAudioPreferredOutputDeviceChangeCallback::OnJsCallbackActiveOutputDeviceChange(
    std::unique_ptr<AudioActiveOutputDeviceChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackActiveOutputDeviceChange: jsCb.get() is null");
        return;
    }

    AudioActiveOutputDeviceChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amOutputDevChgTsfn_);
    napi_call_threadsafe_function(amOutputDevChgTsfn_, event, napi_tsfn_blocking);
}

NapiAudioPreferredInputDeviceChangeCallback::NapiAudioPreferredInputDeviceChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioPreferredInputDeviceChangeCallback: instance create");
}

NapiAudioPreferredInputDeviceChangeCallback::~NapiAudioPreferredInputDeviceChangeCallback()
{
    if (regAmInputDevChgTsfn_) {
        napi_release_threadsafe_function(amInputDevChgTsfn_, napi_tsfn_abort);
    }
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

void NapiAudioPreferredInputDeviceChangeCallback::CreatePerferredInTsfn(napi_env env)
{
    regAmInputDevChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "PreferredInputDeviceChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        ActiveInputDeviceChangeTsfnFinalize, nullptr, SafeJsCallbackActiveInputDeviceChangeWork,
        &amInputDevChgTsfn_);
}

bool NapiAudioPreferredInputDeviceChangeCallback::GetPerferredInTsfnFlag()
{
    return regAmInputDevChgTsfn_;
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
            AUDIO_INFO_LOG("RemoveCallbackReference: find js callback, erase it");
            preferredInputDeviceCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference: js callback no find");
}

void NapiAudioPreferredInputDeviceChangeCallback::RemoveAllCallbacks()
{
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

void NapiAudioPreferredInputDeviceChangeCallback::SafeJsCallbackActiveInputDeviceChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioActiveInputDeviceChangeJsCallback *event = reinterpret_cast<AudioActiveInputDeviceChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackActiveInputDeviceChange: no memory");
    std::shared_ptr<AudioActiveInputDeviceChangeJsCallback> safeContext(
        static_cast<AudioActiveInputDeviceChangeJsCallback*>(data),
        [](AudioActiveInputDeviceChangeJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetDeviceDescriptors(env, event->desc, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create input device change callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call device change callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioPreferredInputDeviceChangeCallback::ActiveInputDeviceChangeTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("ActiveInputDeviceChangeTsfnFinalize: safe thread resource release.");
}

void NapiAudioPreferredInputDeviceChangeCallback::OnJsCallbackActiveInputDeviceChange(
    std::unique_ptr<AudioActiveInputDeviceChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackActiveInputDeviceChange: jsCb.get() is null");
        return;
    }

    AudioActiveInputDeviceChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amInputDevChgTsfn_);
    napi_call_threadsafe_function(amInputDevChgTsfn_, event, napi_tsfn_blocking);
}
}  // namespace AudioStandard
}  // namespace OHOS
