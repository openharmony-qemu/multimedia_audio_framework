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
#define LOG_TAG "NapiAudioRendererErrorCallback"
#endif

#include "napi_audio_renderer_error_callback.h"

#include "js_native_api.h"
#include "audio_errors.h"
#include "audio_renderer_log.h"
#include "napi_audio_error.h"
namespace OHOS {
namespace AudioStandard {
NapiRendererErrorCallback::NapiRendererErrorCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiRendererErrorCallback::~NapiRendererErrorCallback()
{
    if (regArErrTsfn_) {
        napi_release_threadsafe_function(arErrTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiRendererErrorCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
            renderErrorCallback_ = generatedCallback;
    };
    NapiAudioRendererCallbackInner::SaveCallbackReferenceInner(callbackName, args, successed);
    AUDIO_DEBUG_LOG("SaveAudioErrorCallback successful");
}

std::shared_ptr<AutoRef> &NapiRendererErrorCallback::GetCallback(const std::string &callbackName)
{
    return renderErrorCallback_;
}

void NapiRendererErrorCallback::RemoveCallbackReference(
    const std::string &callbackName, napi_env env, napi_value callback, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    //create function that will operate while save callback reference success.
    std::function<void()> successed = [this]() {
        renderErrorCallback_ = nullptr;
    };
    RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

napi_env &NapiRendererErrorCallback::GetEnv()
{
    return env_;
}

bool NapiRendererErrorCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    return (callbackName == RENDER_ERROR_CALLBACK_NAME);
}

void NapiRendererErrorCallback::CreateErrorTsfn(napi_env env)
{
    regArErrTsfn_ = true;
    std::string callbackName = "renderError";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, ErrorTsfnFinalize,
        nullptr, SafeJsCallbackErrorWork, &arErrTsfn_);
}

void NapiRendererErrorCallback::OnError(AudioErrors errorCode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("render error");
    CHECK_AND_RETURN_LOG(renderErrorCallback_ != nullptr, "Cannot find the reference of error callback");

    std::unique_ptr<RendererErrorJsCallback> cb = std::make_unique<RendererErrorJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = renderErrorCallback_;
    cb->callbackName = RENDER_ERROR_CALLBACK_NAME;
    cb->errorCode = errorCode;
    return OnJsRendererErrorCallback(cb);
}

void NapiRendererErrorCallback::SafeJsCallbackErrorWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    RendererErrorJsCallback *event = reinterpret_cast<RendererErrorJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "RendererErrorJsCallback: no memory");
    std::shared_ptr<RendererErrorJsCallback> safeContext(
        static_cast<RendererErrorJsCallback*>(data),
        [](RendererErrorJsCallback *ptr) {
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

        // Call back function
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

void NapiRendererErrorCallback::ErrorTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("ErrorTsfnFinalize: safe thread resource release.");
}

void NapiRendererErrorCallback::OnJsRendererErrorCallback(std::unique_ptr<RendererErrorJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererErrorCallback: jsCb.get() is null");
        return;
    }

    RendererErrorJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(arErrTsfn_);
    napi_call_threadsafe_function(arErrTsfn_, event, napi_tsfn_blocking);
}
}  // namespace AudioStandard
}  // namespace OHOS
