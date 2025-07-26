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
#define LOG_TAG "NapiAudioSessionStateCallback"
#endif
#include <thread>
#include "js_native_api.h"
#include "napi_audio_session_state_callback.h"
#include "napi_param_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioSessionStateCallback::NapiAudioSessionStateCallback(napi_env env)
    :env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSessionStateCallback::Constructor");
}

NapiAudioSessionStateCallback::~NapiAudioSessionStateCallback()
{
    if (regAmSessionStateChgTsfn_) {
        napi_release_threadsafe_function(amSessionStateChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioSessionStateCallback::Destructor");
}

void NapiAudioSessionStateCallback::OnAudioSessionStateChanged(const AudioSessionStateChangedEvent &stateEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionStateChanged is called AudioSessionStateChangedEvent=%{public}d",
        stateEvent.stateChangeHint);
    CHECK_AND_RETURN_LOG(audioSessionStateJsCallback_ != nullptr,
        "OnAudioSessionStateChanged:No JS callback registered return");
    std::unique_ptr<AudioSessionStateJsCallback> cb = std::make_unique<AudioSessionStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioSessionStateJsCallback_;
    cb->callbackName = AUDIO_SESSION_STATE_CALLBACK_NAME;
    cb->audioSessionStateEvent.stateChangeHint = stateEvent.stateChangeHint;

    return OnJsCallbackAudioSessionState(cb);
}

void NapiAudioSessionStateCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioSessionStateCallback: creating reference for callback fail");
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    audioSessionStateJsCallback_ = cb;
}

void NapiAudioSessionStateCallback::CreateAudioSessionStateTsfn(napi_env env)
{
    regAmSessionStateChgTsfn_ = true;
    std::string callbackName = "audioSessionStateChanged";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, AudioSessionStateTsfnFinalize,
        nullptr, SafeJsCallbackAudioSessionStateWork, &amSessionStateChgTsfn_);
}

bool NapiAudioSessionStateCallback::GetAudioSessionStateTsfnFlag() const
{
    return regAmSessionStateChgTsfn_;
}

void NapiAudioSessionStateCallback::SafeJsCallbackAudioSessionStateWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSessionStateJsCallback *event = reinterpret_cast<AudioSessionStateJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAudioSessionState: no memory");

    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAudioSessionStateWork: safe js callback working.");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        napi_value args[ARGS_ONE] = { nullptr };
        napi_create_object(env, &args[PARAM0]);
        NapiParamUtils::SetValueInt32(env, "stateChangeHint",
            static_cast<int32_t>(event->audioSessionStateEvent.stateChangeHint), args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to stateChangeHint callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call SetaudioSessionState callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSessionStateCallback::AudioSessionStateTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AudioSessionStateTsfnFinalize: safe thread resource release.");
}

void NapiAudioSessionStateCallback::OnJsCallbackAudioSessionState(std::unique_ptr<AudioSessionStateJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiAudioSessionStateCallback: OnJsCallbackAudioSessionState: jsCb.get() is null");
        return;
    }

    AudioSessionStateJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amSessionStateChgTsfn_);
    napi_call_threadsafe_function(amSessionStateChgTsfn_, event, napi_tsfn_blocking);
}

bool NapiAudioSessionStateCallback::ContainSameJsCallback(napi_value args)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env_, callback_, &copyValue);
    CHECK_AND_RETURN_RET_LOG(args != nullptr, false, "args is nullptr");
    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, copyValue, args, &isEquals) == napi_ok, false,
        "Get napi_strict_equals failed");

    return isEquals;
}
} // namespace AudioStandard
} // namespace OHOS