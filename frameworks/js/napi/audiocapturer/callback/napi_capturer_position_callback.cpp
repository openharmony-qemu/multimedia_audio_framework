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
#define LOG_TAG "NapiCapturerPositionCallback"
#endif

#include "js_native_api.h"
#include "napi_capturer_position_callback.h"
#include "napi_audio_capturer_callbacks.h"
#include "napi_param_utils.h"

namespace OHOS {
namespace AudioStandard {
NapiCapturerPositionCallback::NapiCapturerPositionCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiCapturerPositionCallback: instance create");
}

NapiCapturerPositionCallback::~NapiCapturerPositionCallback()
{
    if (regAcPosTsfn_) {
        napi_release_threadsafe_function(acPosTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiCapturerPositionCallback: instance destroy");
}

void NapiCapturerPositionCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        capturerPositionCallback_ = generatedCallback;
    };
    SaveCallbackReferenceInner(callbackName, args, successed);
}

void NapiCapturerPositionCallback::RemoveCallbackReference(const std::string &callbackName, napi_env env,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while remove callback reference success.
    std::function<void()> successed =
        [this]() {
        capturerPositionCallback_ = nullptr;
        };
    RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

void NapiCapturerPositionCallback::CreateCapturePositionTsfn(napi_env env)
{
    regAcPosTsfn_ = true;
    std::string callbackName = "CapturePosition";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        CapturePostionTsfnFinalize, nullptr, SafeJsCallbackCapturerPositionWork, &acPosTsfn_);
}

bool NapiCapturerPositionCallback::GetCapturePositionFlag()
{
    return regAcPosTsfn_;
}

void NapiCapturerPositionCallback::OnMarkReached(const int64_t &framePosition)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("NapiCapturerPositionCallback: mark reached");
    CHECK_AND_RETURN_LOG(capturerPositionCallback_ != nullptr, "Cannot find the reference of position callback");

    std::unique_ptr<CapturerPositionJsCallback> cb = std::make_unique<CapturerPositionJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = capturerPositionCallback_;
    cb->callbackName = MARK_REACH_CALLBACK_NAME;
    cb->position = framePosition;
    return OnJsCapturerPositionCallback(cb);
}

void NapiCapturerPositionCallback::SafeJsCallbackCapturerPositionWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    CapturerPositionJsCallback *event = reinterpret_cast<CapturerPositionJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCapturerPositionCallback: no memory");
    std::shared_ptr<CapturerPositionJsCallback> safeContext(
        static_cast<CapturerPositionJsCallback*>(data),
        [](CapturerPositionJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackCapturerPositionWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        napi_value args[ARGS_ONE] = { nullptr };
        napi_create_int64(env, event->position, &args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create position callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call position callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiCapturerPositionCallback::CapturePostionTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("CapturePostionTsfnFinalize: safe thread resource release.");
}

void NapiCapturerPositionCallback::OnJsCapturerPositionCallback(std::unique_ptr<CapturerPositionJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCapturerPositionCallback: jsCb.get() is null");
        return;
    }

    CapturerPositionJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(acPosTsfn_);
    napi_call_threadsafe_function(acPosTsfn_, event, napi_tsfn_blocking);
}

napi_env &NapiCapturerPositionCallback::GetEnv()
{
    return env_;
}

std::shared_ptr<AutoRef> NapiCapturerPositionCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> cb = nullptr;
    if (callbackName == MARK_REACH_CALLBACK_NAME) {
        return capturerPositionCallback_;
    }
    return cb;
}

bool NapiCapturerPositionCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == MARK_REACH_CALLBACK_NAME) {
        return true;
    }
    return false;
}
}  // namespace AudioStandard
}  // namespace OHOS