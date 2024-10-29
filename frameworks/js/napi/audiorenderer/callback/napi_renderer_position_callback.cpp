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
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiRendererPositionCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == MARK_REACH_CALLBACK_NAME) {
        renderPositionCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
    }
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

void NapiRendererPositionCallback::OnJsRendererPositionCallback(std::unique_ptr<RendererPositionJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsRendererPositionCallback: jsCb.get() is null");
        return;
    }

    RendererPositionJsCallback *event = jsCb.get();
    auto task = [event]() {
        std::shared_ptr<RendererPositionJsCallback> context(
            static_cast<RendererPositionJsCallback*>(event),
            [](RendererPositionJsCallback* ptr) {
                delete ptr;
            });
        CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
        std::string request = event->callbackName;

        CHECK_AND_RETURN_LOG(event->callback != nullptr, "event->callback is nullptr");
        napi_env env = event->callback->env_;
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
            NapiParamUtils::SetValueInt64(env, event->position, args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
                "%{public}s fail to create position callback", request.c_str());

            const size_t argCount = ARGS_ONE;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call position callback", request.c_str());
        } while (0);
        napi_close_handle_scope(env, scope);
    };
    if (napi_status::napi_ok != napi_send_event(env_, task, napi_eprio_immediate)) {
        AUDIO_ERR_LOG("OnJsRendererPositionCallback: Failed to SendEvent");
    } else {
        jsCb.release();
    }
}
}  // namespace AudioStandard
}  // namespace OHOS