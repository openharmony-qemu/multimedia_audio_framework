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
    AUDIO_DEBUG_LOG("NapiCapturerPeriodPositionCallback: instance destroy");
}

void NapiCapturerPeriodPositionCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiCapturerPeriodPositionCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    if (callbackName == PERIOD_REACH_CALLBACK_NAME) {
        capturerPeriodPositionCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("NapiCapturerPeriodPositionCallback: Unknown callback type: %{public}s", callbackName.c_str());
    }
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

void NapiCapturerPeriodPositionCallback::OnJsCapturerPeriodPositionCallback(
    std::unique_ptr<CapturerPeriodPositionJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop nullptr, No memory");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "work nullptr, No memory");
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiCapturerPeriodPositionCallback: OnJsCapturerPeriodPositionCallback: jsCb.get() is null");
        delete work;
        return;
    }

    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {},
        WorkCapturerPeriodPositionCallbackDone, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}

void NapiCapturerPeriodPositionCallback::WorkCapturerPeriodPositionCallbackDone(uv_work_t *work, int status)
{
    // Js Thread
    std::shared_ptr<CapturerPeriodPositionJsCallback> context(
        static_cast<CapturerPeriodPositionJsCallback*>(work->data),
        [work](CapturerPeriodPositionJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    CapturerPeriodPositionJsCallback *event = reinterpret_cast<CapturerPeriodPositionJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    std::string request = event->callbackName;

    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_DEBUG_LOG("NapiCapturerPeriodPositionCallback: JsCallBack %{public}s, uv_queue_work_with_qos start",
        request.c_str());
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());

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
}  // namespace AudioStandard
}  // namespace OHOS
