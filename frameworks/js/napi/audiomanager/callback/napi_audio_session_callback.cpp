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
#define LOG_TAG "NapiAudioSessionCallback"
#endif

#include "napi_audio_session_callback.h"
#include "napi_param_utils.h"

namespace OHOS {
namespace AudioStandard {

NapiAudioSessionCallback::NapiAudioSessionCallback(napi_env env)
    :env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSessionCallback::Constructor");
}

NapiAudioSessionCallback::~NapiAudioSessionCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioSessionCallback::Destructor");
}

void NapiAudioSessionCallback::OnAudioSessionDeactive(const AudioSessionDeactiveEvent &deactiveEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionDeactive is called AudioSessionDeactiveEvent=%{public}d",
        deactiveEvent.deactiveReason);
    CHECK_AND_RETURN_LOG(audioSessionJsCallback_ != nullptr,
        "OnAudioSessionDeactive:No JS callback registered return");
    std::unique_ptr<AudioSessionJsCallback> cb = std::make_unique<AudioSessionJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioSessionJsCallback_;
    cb->callbackName = AUDIO_SESSION_CALLBACK_NAME;
    cb->audioSessionDeactiveEvent.deactiveReason = deactiveEvent.deactiveReason;

    return OnJsCallbackAudioSession(cb);
}

void NapiAudioSessionCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioSessionCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    audioSessionJsCallback_ = cb;
}

void NapiAudioSessionCallback::WorkCallbackAudioSessionChangeDone(uv_work_t *work, int status)
{
    std::shared_ptr<AudioSessionJsCallback> context(
        static_cast<AudioSessionJsCallback*>(work->data),
        [work](AudioSessionJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioSessionJsCallback *event = reinterpret_cast<AudioSessionJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    std::string request = event->callbackName;
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());

        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetAudioSessionDeactiveEvent(env, event->audioSessionDeactiveEvent, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to SetAudioSessionDeactiveEvent callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call SetaudioSession callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSessionCallback::OnJsCallbackAudioSession(std::unique_ptr<AudioSessionJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackDeviceChange: No memory");

    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackDeviceChange: jsCb.get() is null");
        delete work;
        return;
    }

    work->data = reinterpret_cast<void *>(jsCb.get());
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackAudioSessionChangeDone,
        uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}
} // namespace AudioStandard
} // namespace OHOS