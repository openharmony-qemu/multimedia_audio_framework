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
#define LOG_TAG "NapiAudioCapturerCallback"
#endif

#include "napi_audio_capturer_callbacks.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_capturer_log.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioCapturerCallback::NapiAudioCapturerCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioCapturerCallback: instance create");
}

NapiAudioCapturerCallback::~NapiAudioCapturerCallback()
{
    AUDIO_DEBUG_LOG("NapiAudioCapturerCallback: instance destroy");
}

void NapiAudioCapturerCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioCapturerCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
        interruptCallback_ = cb;
    } else if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
        stateChangeCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("NapiAudioCapturerCallback: Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void NapiAudioCapturerCallback::RemoveCallbackReference(const std::string &callbackName)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
        interruptCallback_ = nullptr;
    } else if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
        stateChangeCallback_ = nullptr;
    } else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void NapiAudioCapturerCallback::OnInterrupt(const InterruptEvent &interruptEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("NapiAudioCapturerCallback: OnInterrupt is called, hintType: %{public}d", interruptEvent.hintType);
    CHECK_AND_RETURN_LOG(interruptCallback_ != nullptr, "Cannot find the reference of interrupt callback");

    std::unique_ptr<AudioCapturerJsCallback> cb = std::make_unique<AudioCapturerJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = interruptCallback_;
    cb->callbackName = INTERRUPT_CALLBACK_NAME;
    cb->interruptEvent = interruptEvent;
    return OnJsCallbackInterrupt(cb);
}

void NapiAudioCapturerCallback::OnJsCallbackInterrupt(std::unique_ptr<AudioCapturerJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop nullptr, No memory");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "work nullptr, No memory");

    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallBackInterrupt: jsCb.get() is null");
        delete work;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackInterruptDone, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}

void NapiAudioCapturerCallback::OnStateChange(const CapturerState state)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("NapiAudioCapturOnStateChange is called,Callback: state: %{public}d", state);
    CHECK_AND_RETURN_LOG(stateChangeCallback_ != nullptr, "Cannot find the reference of stateChange callback");

    std::unique_ptr<AudioCapturerJsCallback> cb = std::make_unique<AudioCapturerJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = stateChangeCallback_;
    cb->callbackName = STATE_CHANGE_CALLBACK_NAME;
    cb->state = state;
    return OnJsCallbackStateChange(cb);
}

void NapiAudioCapturerCallback::OnJsCallbackStateChange(std::unique_ptr<AudioCapturerJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop nullptr, No memory");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "work nullptr, No memory");
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiAudioCapturerCallback: OnJsCallbackStateChange: jsCb.get() is null");
        delete work;
        return;
    }

    work->data = reinterpret_cast<void *>(jsCb.get());

    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackStateChangeDone, uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}

void NapiAudioCapturerCallback::WorkCallbackStateChangeDone(uv_work_t *work, int status)
{
    // Js Thread
    std::shared_ptr<AudioCapturerJsCallback> context(
        static_cast<AudioCapturerJsCallback*>(work->data),
        [work](AudioCapturerJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioCapturerJsCallback *event = reinterpret_cast<AudioCapturerJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    std::string request = event->callbackName;

    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s cancelled", request.c_str());

        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[1] = { nullptr };
        nstatus = NapiParamUtils::SetValueInt32(env, event->state, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create Interrupt callback", request.c_str());

        const size_t argCount = 1;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call Interrupt callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioCapturerCallback::WorkCallbackInterruptDone(uv_work_t *work, int status)
{
    // Js Thread
    std::shared_ptr<AudioCapturerJsCallback> context(
        static_cast<AudioCapturerJsCallback*>(work->data),
        [work](AudioCapturerJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioCapturerJsCallback *event = reinterpret_cast<AudioCapturerJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    std::string request = event->callbackName;

    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;

    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s cancelled", request.c_str());

        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        // Call back function
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetInterruptEvent(env, event->interruptEvent, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create Interrupt callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call Interrupt callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}
}  // namespace AudioStandard
}  // namespace OHOS