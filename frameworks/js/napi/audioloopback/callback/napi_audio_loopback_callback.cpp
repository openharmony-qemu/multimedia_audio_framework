/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioLoopbackCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_loopback_callback.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"

namespace OHOS {
namespace AudioStandard {

NapiAudioLoopbackCallback::NapiAudioLoopbackCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiAudioLoopbackCallback::~NapiAudioLoopbackCallback()
{
    if (regArStatusChgTsfn_) {
        napi_release_threadsafe_function(arStatusChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiAudioLoopbackCallback::OnStatusChange(const AudioLoopbackStatus status,
    const StateChangeCmdType __attribute__((unused)) cmdType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("OnStatusChange is called, status: %{public}d", status);
    CHECK_AND_RETURN_LOG(statusChangeCallback_ != nullptr, "Cannot find the reference of statusChange callback");

    std::unique_ptr<AudioLoopbackJsCallback> cb = std::make_unique<AudioLoopbackJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = statusChangeCallback_;
    cb->callbackName = STATUS_CHANGE_CALLBACK_NAME;
    cb->status = status;
    return OnJsCallbackStatusChange(cb);
}

void NapiAudioLoopbackCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this, callbackName](std::shared_ptr<AutoRef> generatedCallback) {
        if (callbackName == STATUS_CHANGE_CALLBACK_NAME) {
            statusChangeCallback_ = generatedCallback;
            return;
        }
    };
    SaveCallbackReferenceInner(callbackName, args, successed);
}

void NapiAudioLoopbackCallback::RemoveCallbackReference(const std::string &callbackName, napi_env env,
    napi_value callback, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void()> successed = [this, callbackName]() {
        if (callbackName == STATUS_CHANGE_CALLBACK_NAME) {
            statusChangeCallback_ = nullptr;
            return;
        }
    };
    RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

std::shared_ptr<AutoRef> NapiAudioLoopbackCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> cb = nullptr;
    if (callbackName == STATUS_CHANGE_CALLBACK_NAME) {
        return statusChangeCallback_;
    }
    AUDIO_ERR_LOG("NapiAudioLoopbackCallback->GetCallback Unknown callback type: %{public}s", callbackName.c_str());
    return cb;
}

void NapiAudioLoopbackCallback::CreateArStatusChange(napi_env env)
{
    regArStatusChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "statusChange";
    napi_create_string_utf8(env_, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        StatusChangeTsfnFinalize, nullptr, SafeJsCallbackStatusChangeWork, &arStatusChgTsfn_);
}

bool NapiAudioLoopbackCallback::GetArStatusChangeTsfnFlag()
{
    return regArStatusChgTsfn_;
}

napi_env &NapiAudioLoopbackCallback::GetEnv()
{
    return env_;
}

bool NapiAudioLoopbackCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == STATUS_CHANGE_CALLBACK_NAME) {
        return true;
    }
    return false;
}

void NapiAudioLoopbackCallback::SafeJsCallbackStatusChangeWork(napi_env env, napi_value js_cb, void *context,
    void *data)
{
    AudioLoopbackJsCallback *event = reinterpret_cast<AudioLoopbackJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackStatusChange: no memory");
    std::shared_ptr<AudioLoopbackJsCallback> safeContext(
        static_cast<AudioLoopbackJsCallback*>(data),
        [](AudioLoopbackJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackStatusChangeWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[1] = { nullptr };
        nstatus = NapiParamUtils::SetValueInt32(env, event->status, args[0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create StatusChange callback", request.c_str());

        const size_t argCount = 1;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call StatusChange callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioLoopbackCallback::StatusChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("StatusChangeTsfnFinalize: safe thread resource release.");
}

void NapiAudioLoopbackCallback::OnJsCallbackStatusChange(std::unique_ptr<AudioLoopbackJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackStatusChange: jsCb.get() is null");
        return;
    }
    
    AudioLoopbackJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(arStatusChgTsfn_);
    napi_call_threadsafe_function(arStatusChgTsfn_, event, napi_tsfn_blocking);
}
}  // namespace AudioStandard
}  // namespace OHOS