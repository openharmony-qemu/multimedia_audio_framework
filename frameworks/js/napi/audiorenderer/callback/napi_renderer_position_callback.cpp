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
#define LOG_TAG "NapiRendererPositionCallback"
#endif

#include "napi_renderer_position_callback.h"

#include "js_native_api.h"
#include "audio_errors.h"
#include "audio_renderer_log.h"
#include "napi_audio_error.h"
#include "napi_audio_renderer_callback.h"
namespace OHOS {
namespace AudioStandard {
NapiRendererPositionCallback::NapiRendererPositionCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiRendererPositionCallback::~NapiRendererPositionCallback()
{
    if (regArPosTsfn_) {
        napi_release_threadsafe_function(arPosTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiRendererPositionCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        renderPositionCallback_ = generatedCallback;
    };
    NapiAudioRendererCallbackInner::SaveCallbackReferenceInner(callbackName, args, successed);
    AUDIO_DEBUG_LOG("SaveAudioPositionCallback successful");
}

std::shared_ptr<AutoRef> NapiRendererPositionCallback::GetCallback(const std::string &callbackName)
{
    return renderPositionCallback_;
}

void NapiRendererPositionCallback::RemoveCallbackReference(
    const std::string &callbackName, napi_env env, napi_value callback, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    //create function that will operate while save callback reference success.
    std::function<void()> successed = [this]() {
        renderPositionCallback_ = nullptr;
    };
    RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

napi_env &NapiRendererPositionCallback::GetEnv()
{
    return env_;
}

bool NapiRendererPositionCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    return (callbackName == MARK_REACH_CALLBACK_NAME);
}

void NapiRendererPositionCallback::CreateMarkReachedTsfn(napi_env env)
{
    regArPosTsfn_ = true;
    std::string callbackName = "MarkReached";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, PositionTsfnFinalize,
        nullptr, SafeJsCallbackPositionWork, &arPosTsfn_);
}

bool NapiRendererPositionCallback::GetMarkReachedTsfnFlag()
{
    return regArPosTsfn_;
}

void NapiRendererPositionCallback::OnMarkReached(const int64_t &framePosition)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("mark reached");
    CHECK_AND_RETURN_LOG(renderPositionCallback_ != nullptr, "Cannot find the reference of position callback");

    std::unique_ptr<RendererPositionJsCallback> cb = std::make_unique<RendererPositionJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = renderPositionCallback_;
    cb->callbackName = MARK_REACH_CALLBACK_NAME;
    cb->position = framePosition;
    return OnJsRendererPositionCallback(cb);
}

void NapiRendererPositionCallback::SafeJsCallbackPositionWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    RendererPositionJsCallback *event = reinterpret_cast<RendererPositionJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsRendererPositionCallback: no memory");
    std::shared_ptr<RendererPositionJsCallback> safeContext(
        static_cast<RendererPositionJsCallback*>(data),
        [](RendererPositionJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackPositionWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueInt64(env, event->position, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create position callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call position callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiRendererPositionCallback::PositionTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("PositionTsfnFinalize: safe thread resource release.");
}

void NapiRendererPositionCallback::OnJsRendererPositionCallback(std::unique_ptr<RendererPositionJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererPositionCallback: jsCb.get() is null");
        return;
    }

    RendererPositionJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(arPosTsfn_);
    napi_call_threadsafe_function(arPosTsfn_, event, napi_tsfn_blocking);
}
}  // namespace AudioStandard
}  // namespace OHOS