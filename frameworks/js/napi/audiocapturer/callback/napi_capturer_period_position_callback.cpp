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
#define LOG_TAG "NapiCapturerPeriodPositionCallback"
#endif

#include "js_native_api.h"
#include "napi_capturer_period_position_callback.h"

#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "napi_param_utils.h"
#include "napi_audio_capturer_callbacks.h"

namespace OHOS {
namespace AudioStandard {
NapiCapturerPeriodPositionCallback::NapiCapturerPeriodPositionCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiCapturerPeriodPositionCallback: instance create");
}

NapiCapturerPeriodPositionCallback::~NapiCapturerPeriodPositionCallback()
{
    if (regAcPeriodPosTsfn_) {
        napi_release_threadsafe_function(acPeriodPosTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiCapturerPeriodPositionCallback: instance destroy");
}

void NapiCapturerPeriodPositionCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
        capturerPeriodPositionCallback_ = generatedCallback;
    };
    SaveCallbackReferenceInner(callbackName, args, successed);
}

void NapiCapturerPeriodPositionCallback::RemoveCallbackReference(const std::string &callbackName, napi_env env,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void()> successed =
        [this]() {
        capturerPeriodPositionCallback_ = nullptr;
        };
    RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

void NapiCapturerPeriodPositionCallback::CreatePeriodPositionTsfn(napi_env env)
{
    regAcPeriodPosTsfn_ = true;
    std::string callbackName = "PeriodPosition";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        CapturerPeriodPositionTsfnFinalize, nullptr, SafeJsCallbackCapturerPeriodPositionWork,
        &acPeriodPosTsfn_);
}

void NapiCapturerPeriodPositionCallback::OnPeriodReached(const int64_t &frameNumber)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("NapiCapturerPeriodPositionCallback: period reached");
    CHECK_AND_RETURN_LOG(capturerPeriodPositionCallback_ != nullptr, "Cannot find the reference of position callback");

    std::unique_ptr<CapturerPeriodPositionJsCallback> cb = std::make_unique<CapturerPeriodPositionJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = capturerPeriodPositionCallback_;
    cb->callbackName = PERIOD_REACH_CALLBACK_NAME;
    cb->position = frameNumber;
    return OnJsCapturerPeriodPositionCallback(cb);
}

void NapiCapturerPeriodPositionCallback::SafeJsCallbackCapturerPeriodPositionWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    CapturerPeriodPositionJsCallback *event = reinterpret_cast<CapturerPeriodPositionJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCapturerPeriodPositionCallback: no memory");
    std::shared_ptr<CapturerPeriodPositionJsCallback> safeContext(
        static_cast<CapturerPeriodPositionJsCallback*>(data),
        [](CapturerPeriodPositionJsCallback *ptr) {
            delete ptr;
    });
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "callback is nullptr");
    napi_ref callback = event->callback->cb_;
    std::string request = event->callbackName;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackCapturerPeriodPositionWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
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

void NapiCapturerPeriodPositionCallback::CapturerPeriodPositionTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("CapturerPeriodPositionTsfnFinalize: safe thread resource release.");
}

void NapiCapturerPeriodPositionCallback::OnJsCapturerPeriodPositionCallback(
    std::unique_ptr<CapturerPeriodPositionJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCapturerPeriodPositionCallback: jsCb.get() is null");
        return;
    }
    CapturerPeriodPositionJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCapturerPeriodPositionCallback: event is nullptr.");

    napi_acquire_threadsafe_function(acPeriodPosTsfn_);
    napi_call_threadsafe_function(acPeriodPosTsfn_, event, napi_tsfn_blocking);
}

napi_env &NapiCapturerPeriodPositionCallback::GetEnv()
{
    return env_;
}

std::shared_ptr<AutoRef> NapiCapturerPeriodPositionCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> cb = nullptr;
    if (callbackName == PERIOD_REACH_CALLBACK_NAME) {
        return capturerPeriodPositionCallback_;
    }
    return cb;
}

bool NapiCapturerPeriodPositionCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == PERIOD_REACH_CALLBACK_NAME) {
        return true;
    }
    return false;
}
}  // namespace AudioStandard
}  // namespace OHOS
