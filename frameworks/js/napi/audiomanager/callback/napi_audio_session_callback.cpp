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
#include <thread>
#include "js_native_api.h"
#include "napi_audio_session_callback.h"
#include "napi_param_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioSessionCallback::NapiAudioSessionCallback(napi_env env)
    :env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSessionCallback::Constructor");
}

NapiAudioSessionCallback::~NapiAudioSessionCallback()
{
    if (regAmSessionChgTsfn_) {
        napi_release_threadsafe_function(amSessionChgTsfn_, napi_tsfn_abort);
    }
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

void NapiAudioSessionCallback::CreateAudioSessionTsfn(napi_env env)
{
    regAmSessionChgTsfn_ = true;
    std::string callbackName = "AudioSession";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, AudioSessionTsfnFinalize,
        nullptr, SafeJsCallbackAudioSessionWork, &amSessionChgTsfn_);
}

bool NapiAudioSessionCallback::GetAudioSessionTsfnFlag()
{
    return regAmSessionChgTsfn_;
}

void NapiAudioSessionCallback::SafeJsCallbackAudioSessionWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSessionJsCallback *event = reinterpret_cast<AudioSessionJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAudioSession: no memory");
    std::shared_ptr<AudioSessionJsCallback> safeContext(
        static_cast<AudioSessionJsCallback*>(data),
        [](AudioSessionJsCallback *ptr) {
            delete ptr;
    });

    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAudioSessionWork: safe js callback working.");
    do {
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

void NapiAudioSessionCallback::AudioSessionTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AudioSessionTsfnFinalize: safe thread resource release.");
}

void NapiAudioSessionCallback::OnJsCallbackAudioSession(std::unique_ptr<AudioSessionJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiAudioSessionCallback: OnJsCallbackAudioSession: jsCb.get() is null");
        return;
    }

    AudioSessionJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amSessionChgTsfn_);
    napi_call_threadsafe_function(amSessionChgTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS