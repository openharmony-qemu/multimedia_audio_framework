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

#include "js_native_api.h"
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
    if (regArStateChgTsfn_) {
        napi_release_threadsafe_function(arStateChgTsfn_, napi_tsfn_abort);
    } else if (regArInterruptTsfn_) {
        napi_release_threadsafe_function(arInterruptTsfn_, napi_tsfn_abort);
    }
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
    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this, callbackName](std::shared_ptr<AutoRef> generatedCallback) {
        if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
            interruptCallback_ = generatedCallback;
            return;
        }
        if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
            stateChangeCallback_ = generatedCallback;
            return;
        }
    };
    NapiAudioRendererCallbackInner::SaveCallbackReferenceInner(callbackName, args, successed);
}

void NapiAudioRendererCallback::RemoveCallbackReference(const std::string &callbackName, napi_env env,
    napi_value callback, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    // create function that will operate while save callback reference success.
    std::function<void()> successed = [this, callbackName]() {
        if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
            interruptCallback_ = nullptr;
            return;
        }
        if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
            stateChangeCallback_ = nullptr;
            return;
        }
    };
    RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

std::shared_ptr<AutoRef> NapiAudioRendererCallback::GetCallback(const std::string &callbackName)
{
    std::shared_ptr<AutoRef> cb = nullptr;
    if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME) {
        return interruptCallback_;
    }
    if (callbackName == STATE_CHANGE_CALLBACK_NAME) {
        return stateChangeCallback_;
    }
    AUDIO_ERR_LOG("NapiAudioRendererCallback->GetCallback Unknown callback type: %{public}s", callbackName.c_str());
    return cb;
}

void NapiAudioRendererCallback::CreateArInterrupt(napi_env env)
{
    regArInterruptTsfn_ = true;
    std::string callbackName = "Interrupt";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        InterruptTsfnFinalize, nullptr, SafeJsCallbackInterruptWork, &arInterruptTsfn_);
}

void NapiAudioRendererCallback::CreateArStateChange(napi_env env)
{
    regArStateChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "stateChange";
    napi_create_string_utf8(env_, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        StateChangeTsfnFinalize, nullptr, SafeJsCallbackStateChangeWork, &arStateChgTsfn_);
}

bool NapiAudioRendererCallback::GetArInterruptTsfnFlag()
{
    return regArInterruptTsfn_;
}

bool NapiAudioRendererCallback::GetArStateChangeTsfnFlag()
{
    return regArStateChgTsfn_;
}

napi_env &NapiAudioRendererCallback::GetEnv()
{
    return env_;
}

bool NapiAudioRendererCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == INTERRUPT_CALLBACK_NAME || callbackName == AUDIO_INTERRUPT_CALLBACK_NAME ||
        callbackName == STATE_CHANGE_CALLBACK_NAME) {
        return true;
    }
    return false;
}

void NapiAudioRendererCallback::SafeJsCallbackInterruptWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioRendererJsCallback *event = reinterpret_cast<AudioRendererJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackInterrupt: no memory");
    std::shared_ptr<AudioRendererJsCallback> safeContext(
        static_cast<AudioRendererJsCallback*>(data),
        [](AudioRendererJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackInterruptWork: safe js callback working.");

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
}

void NapiAudioRendererCallback::InterruptTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("StateChangeTsfnFinalize: safe thread resource release.");
}

void NapiAudioRendererCallback::OnJsCallbackInterrupt(std::unique_ptr<AudioRendererJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackInterrupt: jsCb.get() is null");
        return;
    }
    
    AudioRendererJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(arInterruptTsfn_);
    napi_call_threadsafe_function(arInterruptTsfn_, event, napi_tsfn_blocking);
}

void NapiAudioRendererCallback::SafeJsCallbackStateChangeWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioRendererJsCallback *event = reinterpret_cast<AudioRendererJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackStateChange: no memory");
    std::shared_ptr<AudioRendererJsCallback> safeContext(
        static_cast<AudioRendererJsCallback*>(data),
        [](AudioRendererJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackStateChangeWork: safe js callback working.");

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
}

void NapiAudioRendererCallback::StateChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("StateChangeTsfnFinalize: safe thread resource release.");
}

void NapiAudioRendererCallback::OnJsCallbackStateChange(std::unique_ptr<AudioRendererJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackStateChange: jsCb.get() is null");
        return;
    }
    
    AudioRendererJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(arStateChgTsfn_);
    napi_call_threadsafe_function(arStateChgTsfn_, event, napi_tsfn_blocking);
}
}  // namespace AudioStandard
}  // namespace OHOS