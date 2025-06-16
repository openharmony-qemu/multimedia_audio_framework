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

#include "js_native_api.h"
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
    if (regArDevInfoTsfn_) {
        napi_release_threadsafe_function(arDevInfoTsfn_, napi_tsfn_abort);
    }
    AUDIO_INFO_LOG("instance destroy");
}

void NapiAudioRendererDeviceChangeCallback::AddCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback;
    const int32_t refCount = 1;
    bool isEquals = false;
    napi_value copyValue = nullptr;

    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        napi_get_reference_value(env_, (*autoRef)->cb_, &copyValue);
        CHECK_AND_RETURN_LOG(napi_strict_equals(env_, copyValue, args, &isEquals) == napi_ok,
            "get napi_strict_equals failed");
        CHECK_AND_RETURN_LOG(!isEquals, "js callback already exits");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioRendererDeviceChangeCallback: create reference for callback fail");
    
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    callbacks_.push_back(cb);
    AUDIO_DEBUG_LOG("AddAudioRendererDeviceChangeCallback sucessful");
}

void NapiAudioRendererDeviceChangeCallback::CreateRendererDeviceChangeTsfn(napi_env env)
{
    regArDevInfoTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "AudioRendererDeviceChange";
    napi_create_string_utf8(env_, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, RendererDeviceInfoTsfnFinalize,
        nullptr, SafeJsCallbackRendererDeviceInfoWork, &arDevInfoTsfn_);
}

bool NapiAudioRendererDeviceChangeCallback::GetRendererDeviceChangeTsfnFlag()
{
    return regArDevInfoTsfn_;
}

void NapiAudioRendererDeviceChangeCallback::RemoveCallbackReference(napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;
    napi_value copyValue = nullptr;

    if (args == nullptr) {
        for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
            napi_status ret = napi_delete_reference(env, (*autoRef)->cb_);
            CHECK_AND_RETURN_LOG(napi_ok == ret, "delete callback reference failed");
            (*autoRef)->cb_ = nullptr;
        }
        callbacks_.clear();
        AUDIO_INFO_LOG("Remove all JS Callback");
        return;
    }

    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        napi_get_reference_value(env, (*autoRef)->cb_, &copyValue);
        CHECK_AND_RETURN_LOG(copyValue != nullptr, "copyValue is nullptr");
        CHECK_AND_RETURN_LOG(napi_strict_equals(env, args, copyValue, &isEquals) == napi_ok,
            "get napi_strict_equals failed");

        if (isEquals == true) {
            AUDIO_INFO_LOG("found JS Callback, delete it!");
            callbacks_.remove(*autoRef);
            napi_status status = napi_delete_reference(env, (*autoRef)->cb_);
            CHECK_AND_RETURN_LOG(status == napi_ok, "deleting callback reference failed");
            (*autoRef)->cb_ = nullptr;
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveCallbackReference success");
}

void NapiAudioRendererDeviceChangeCallback::RemoveAllCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks successful");
}

int32_t NapiAudioRendererDeviceChangeCallback::GetCallbackListSize() const
{
    return callbacks_.size();
}

void NapiAudioRendererDeviceChangeCallback::OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
    const AudioStreamDeviceChangeReason reason)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        OnJsCallbackRendererDeviceInfo((*autoRef)->cb_, deviceInfo);
    }
}

void NapiAudioRendererDeviceChangeCallback::SafeJsCallbackRendererDeviceInfoWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioRendererDeviceChangeJsCallback *event = reinterpret_cast<AudioRendererDeviceChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback_ != nullptr),
        "SafeJsCallbackRendererDeviceInfoWork: No memory");
    std::shared_ptr<AudioRendererDeviceChangeJsCallback> safeContext(
        static_cast<AudioRendererDeviceChangeJsCallback*>(data),
        [](AudioRendererDeviceChangeJsCallback *ptr) {
            delete ptr;
    });
    napi_ref callback = event->callback_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackRendererDeviceInfoWork: safe js callback working.");

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

void NapiAudioRendererDeviceChangeCallback::RendererDeviceInfoTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("RendererDeviceInfoTsfnFinalize: safe thread resource release.");
}

void NapiAudioRendererDeviceChangeCallback::OnJsCallbackRendererDeviceInfo(napi_ref method,
    const AudioDeviceDescriptor &deviceInfo)
{
    CHECK_AND_RETURN_LOG(method != nullptr, "OnJsCallbackRendererDeviceInfo method is nullptr");
    AudioRendererDeviceChangeJsCallback *event =
        new AudioRendererDeviceChangeJsCallback {method, env_, deviceInfo};
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr.");

    napi_acquire_threadsafe_function(arDevInfoTsfn_);
    napi_call_threadsafe_function(arDevInfoTsfn_, event, napi_tsfn_blocking);
}

NapiAudioRendererOutputDeviceChangeWithInfoCallback::NapiAudioRendererOutputDeviceChangeWithInfoCallback(napi_env env)
    : env_(env)
{
    AUDIO_INFO_LOG("instance create");
}

NapiAudioRendererOutputDeviceChangeWithInfoCallback::~NapiAudioRendererOutputDeviceChangeWithInfoCallback()
{
    if (regArOutputDevChg_) {
        napi_release_threadsafe_function(arOutputDevChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_INFO_LOG("instance destroy");
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::AddCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    bool isEquals = false;
    napi_value copyValue = nullptr;

    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        napi_get_reference_value(env_, (*autoRef)->cb_, &copyValue);
        CHECK_AND_RETURN_LOG(napi_strict_equals(env_, copyValue, args, &isEquals) == napi_ok,
            "get napi_strict_equals failed");
        CHECK_AND_RETURN_LOG(!isEquals, "js Callback already exist");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    callbacks_.push_back(cb);
    AUDIO_INFO_LOG("successful");
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::CreateOutputDeviceChangeTsfn(napi_env env)
{
    regArOutputDevChg_ = true;
    napi_value cbName;
    std::string callbackName = "AROutputDeviceChangeWithInfo";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, OutputDeviceInfoTsfnFinalize,
        nullptr, SafeJsCallbackOutputDeviceInfoWork, &arOutputDevChgTsfn_);
}

bool NapiAudioRendererOutputDeviceChangeWithInfoCallback::GetOutputDeviceChangeTsfnFlag()
{
    return regArOutputDevChg_;
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::RemoveCallbackReference(napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    bool isEquals = false;
    napi_value copyValue = nullptr;

    if (args == nullptr) {
        callbacks_.clear();
        AUDIO_INFO_LOG("Remove all JS Callback");
        return;
    }

    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        napi_get_reference_value(env, (*autoRef)->cb_, &copyValue);
        CHECK_AND_RETURN_LOG(copyValue != nullptr, "copyValue is nullptr");
        CHECK_AND_RETURN_LOG(napi_strict_equals(env, args, copyValue, &isEquals) == napi_ok,
            "get napi_strict_equals failed");

        if (isEquals == true) {
            AUDIO_INFO_LOG("found JS Callback, delete it!");
            callbacks_.remove(*autoRef);
            return;
        }
    }

    AUDIO_INFO_LOG("RemoveCallbackReference success");
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::RemoveAllCallbacks()
{
    std::lock_guard<std::mutex> lock(mutex_);
    callbacks_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbacks successful");
}

int32_t NapiAudioRendererOutputDeviceChangeWithInfoCallback::GetCallbackListSize() const
{
    return callbacks_.size();
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
    const AudioStreamDeviceChangeReason reason)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto autoRef = callbacks_.begin(); autoRef != callbacks_.end(); ++autoRef) {
        OnJsCallbackOutputDeviceInfo((*autoRef)->cb_, deviceInfo, reason);
    }
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::SafeJsCallbackOutputDeviceInfoWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioRendererOutputDeviceChangeWithInfoJsCallback *event =
        reinterpret_cast<AudioRendererOutputDeviceChangeWithInfoJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback_ != nullptr),
        "OnJsCallbackOutputDeviceInfo: No memory");
    std::shared_ptr<AudioRendererOutputDeviceChangeWithInfoJsCallback> safeContext(
        static_cast<AudioRendererOutputDeviceChangeWithInfoJsCallback*>(data),
        [](AudioRendererOutputDeviceChangeWithInfoJsCallback *ptr) {
            delete ptr;
    });
    napi_ref callback = event->callback_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackOutputDeviceInfoWork: safe js callback working.");

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

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::OutputDeviceInfoTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("OutputDeviceInfoTsfnFinalize: safe thread resource release.");
}

void NapiAudioRendererOutputDeviceChangeWithInfoCallback::OnJsCallbackOutputDeviceInfo(napi_ref method,
    const AudioDeviceDescriptor &deviceInfo, AudioStreamDeviceChangeReason reason)
{
    CHECK_AND_RETURN_LOG(method != nullptr, "OnJsCallbackOutputDeviceInfo method is nullptr");
    AudioRendererOutputDeviceChangeWithInfoJsCallback *event =
        new AudioRendererOutputDeviceChangeWithInfoJsCallback {method, env_, deviceInfo, reason};
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr.");

    napi_acquire_threadsafe_function(arOutputDevChgTsfn_);
    napi_call_threadsafe_function(arOutputDevChgTsfn_, event, napi_tsfn_blocking);
}
}  // namespace AudioStandard
}  // namespace OHOS