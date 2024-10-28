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
#define LOG_TAG "NapiAudioRendererCallback"
#endif

#include "napi_audio_renderer_callback.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioRendererCallback::NapiAudioRendererCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiAudioRendererCallback::~NapiAudioRendererCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiAudioRendererCallback::OnInterrupt(const InterruptEvent &interruptEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("OnInterrupt is called,hintType: %{public}d", interruptEvent.hintType);
    CHECK_AND_RETURN_LOG(interruptCallback_ != nullptr, "Cannot find the reference of interrupt callback");

    std::unique_ptr<AudioRendererJsCallback> cb = std::make_unique<AudioRendererJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = interruptCallback_;
    cb->callbackName = INTERRUPT_CALLBACK_NAME;
    cb->interruptEvent = interruptEvent;
    return OnJsCallbackInterrupt(cb);
}

void NapiAudioRendererCallback::OnStateChange(const RendererState state,
    const StateChangeCmdType __attribute__((unused)) cmdType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("OnStateChange is called, state: %{public}d", state);
    CHECK_AND_RETURN_LOG(stateChangeCallback_ != nullptr, "Cannot find the reference of stateChange callback");

    std::unique_ptr<AudioRendererJsCallback> cb = std::make_unique<AudioRendererJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = stateChangeCallback_;
    cb->callbackName = STATE_CHANGE_CALLBACK_NAME;
    cb->state = state;
    return OnJsCallbackStateChange(cb);
}

void NapiAudioRendererCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
        interruptCallback_ = cb;
    } else if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
        stateChangeCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void NapiAudioRendererCallback::RemoveCallbackReference(const std::string &callbackName)
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

void NapiAudioRendererCallback::OnJsCallbackInterrupt(std::unique_ptr<AudioRendererJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackInterrupt: jsCb.get() is null");
        return;
    }

    AudioRendererJsCallback *event = jsCb.get();
    auto task = [event]() {
        std::shared_ptr<AudioRendererJsCallback> context(
            static_cast<AudioRendererJsCallback*>(event),
            [](AudioRendererJsCallback* ptr) {
                delete ptr;
            });
        CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
        std::string request = event->callbackName;

        CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
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
            NapiParamUtils::SetInterruptEvent(env, event->interruptEvent, args[0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
                "%{public}s fail to create Interrupt callback", request.c_str());

            const size_t argCount = ARGS_ONE;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call Interrupt callback", request.c_str());
        } while (0);
        napi_close_handle_scope(env, scope);
    };
    if (napi_status::napi_ok != napi_send_event(env_, task, napi_eprio_immediate)) {
        AUDIO_ERR_LOG("OnJsCallbackInterrupt: Failed to SendEvent");
    } else {
        jsCb.release();
    }
}

void NapiAudioRendererCallback::OnJsCallbackStateChange(std::unique_ptr<AudioRendererJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackStateChange: jsCb.get() is null");
        return;
    }

    AudioRendererJsCallback *event = jsCb.get();
    auto task = [event]() {
        std::shared_ptr<AudioRendererJsCallback> context(
            static_cast<AudioRendererJsCallback*>(event),
            [](AudioRendererJsCallback* ptr) {
                delete ptr;
            });
        CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
        std::string request = event->callbackName;

        CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
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
            napi_value args[1] = { nullptr };
            nstatus = NapiParamUtils::SetValueInt32(env, event->state, args[0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
                "%{public}s fail to create Interrupt callback", request.c_str());

            const size_t argCount = 1;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call Interrupt callback", request.c_str());
        } while (0);
        napi_close_handle_scope(env, scope);
    };
    if (napi_status::napi_ok != napi_send_event(env_, task, napi_eprio_immediate)) {
        AUDIO_ERR_LOG("OnJsCallbackStateChange: Failed to SendEvent");
    } else {
        jsCb.release();
    }
}
}  // namespace AudioStandard
}  // namespace OHOS
