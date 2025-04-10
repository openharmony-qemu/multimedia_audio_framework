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
#define LOG_TAG "NapiAudioRenderErrorCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_render_error_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_audio_error.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioRenderErrorCallback::NapiAudioRenderErrorCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiAudioRenderErrorCallback::~NapiAudioRenderErrorCallback()
{
    if (regArErrTsfn_) {
        napi_release_threadsafe_function(arErrTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiAudioRenderErrorCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioRenderErrorCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    CHECK_AND_RETURN_LOG(callbackName == RENDER_ERROR_CALLBACK_NAME,
        "NapiAudioRenderErrorCallback: Unknown callback type: %{public}s", callbackName.c_str());
    renderErrorCallback_ = cb;
}

void NapiAudioRenderErrorCallback::CreateErrorTsfn(napi_env env)
{
    regArErrTsfn_ = true;
    std::string callbackName = "renderError";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, ErrorTsfnFinalize,
        nullptr, SafeJsCallbackErrorWork, &arErrTsfn_);
}

bool NapiAudioRenderErrorCallback::GetErrorTsfnFlag()
{
    return regArErrTsfn_;
}

void NapiAudioRenderErrorCallback::RemoveCallbackReference(const napi_value args)
{
    if (!IsSameCallback(args)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    napi_delete_reference(env_, renderErrorCallback_->cb_);
    renderErrorCallback_->cb_ = nullptr;
    renderErrorCallback_ = nullptr;
    AUDIO_INFO_LOG("Remove callback reference successful.");
}

bool NapiAudioRenderErrorCallback::IsSameCallback(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (renderErrorCallback_ == nullptr) {
        return false;
    }
    if (args == nullptr) {
        return true;
    }
    napi_value renderErrorCallback = nullptr;
    napi_get_reference_value(env_, renderErrorCallback_->cb_, &renderErrorCallback);
    bool isEquals = false;
    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, args, renderErrorCallback, &isEquals) == napi_ok, false,
        "get napi_strict_equals failed");
    return isEquals;
}

void NapiAudioRenderErrorCallback::OnFormatUnsupportedError(const AudioErrors &errorCode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(renderErrorCallback_ != nullptr, "Cannot find the reference of error callback");

    std::unique_ptr<AudioRenderErrorJsCallback> cb = std::make_unique<AudioRenderErrorJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = renderErrorCallback_;
    cb->callbackName = RENDER_ERROR_CALLBACK_NAME;
    cb->errorCode = errorCode;
    return OnJsCallbackRenderError(cb);
}

void NapiAudioRenderErrorCallback::SafeJsCallbackErrorWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioRenderErrorJsCallback *event = reinterpret_cast<AudioRenderErrorJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackRenderError: no memory");
    std::shared_ptr<AudioRenderErrorJsCallback> safeContext(
        static_cast<AudioRenderErrorJsCallback*>(data),
        [](AudioRenderErrorJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackErrorWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueInt64(env, event->errorCode, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create error callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call error callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioRenderErrorCallback::ErrorTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("ErrorTsfnFinalize: safe thread resource release.");
}

void NapiAudioRenderErrorCallback::OnJsCallbackRenderError(std::unique_ptr<AudioRenderErrorJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRenderError: jsCb.get() is null");
        return;
    }

    AudioRenderErrorJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(arErrTsfn_);
    napi_call_threadsafe_function(arErrTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS
