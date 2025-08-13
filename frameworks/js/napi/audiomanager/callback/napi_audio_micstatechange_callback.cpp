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
#define LOG_TAG "NapiAudioManagerMicStateChangeCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_micstatechange_callback.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioManagerMicStateChangeCallback::NapiAudioManagerMicStateChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioManagerMicStateChangeCallback: instance create");
}

NapiAudioManagerMicStateChangeCallback::~NapiAudioManagerMicStateChangeCallback()
{
    if (regAmMicStateChgTsfn_) {
        napi_release_threadsafe_function(amMicStateChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioManagerMicStateChangeCallback: instance destroy");
}

void NapiAudioManagerMicStateChangeCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioManagerMicStateChangeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    CHECK_AND_RETURN_LOG(callbackName == MIC_STATE_CHANGE_CALLBACK_NAME,
        "NapiAudioManagerMicStateChangeCallback: Unknown callback type: %{public}s", callbackName.c_str());
    micStateChangeCallback_ = cb;
}

void NapiAudioManagerMicStateChangeCallback::CreateManagerMicStateChangeTsfn(napi_env env)
{
    regAmMicStateChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "ManagerMicStateChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        MicStateChangeTsfnFinalize, nullptr, SafeJsCallbackMicStateChangeWork, &amMicStateChgTsfn_);
}

bool NapiAudioManagerMicStateChangeCallback::GetManagerMicStateChangeTsfnFlag()
{
    return regAmMicStateChgTsfn_;
}

void NapiAudioManagerMicStateChangeCallback::RemoveCallbackReference(const napi_value args)
{
    if (!IsSameCallback(args)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    napi_delete_reference(env_, micStateChangeCallback_->cb_);
    micStateChangeCallback_->cb_ = nullptr;
    micStateChangeCallback_ = nullptr;
    AUDIO_INFO_LOG("Remove callback reference successful.");
}

bool NapiAudioManagerMicStateChangeCallback::IsSameCallback(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (micStateChangeCallback_ == nullptr) {
        return false;
    }
    if (args == nullptr) {
        return true;
    }
    napi_value micStateChangeCallback = nullptr;
    napi_get_reference_value(env_, micStateChangeCallback_->cb_, &micStateChangeCallback);
    bool isEquals = false;
    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, args, micStateChangeCallback, &isEquals) == napi_ok, false,
        "get napi_strict_equals failed");
    return isEquals;
}

void NapiAudioManagerMicStateChangeCallback::OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(micStateChangeCallback_ != nullptr, "callback not registered by JS client");

    std::unique_ptr<AudioManagerMicStateChangeJsCallback> cb = std::make_unique<AudioManagerMicStateChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");

    cb->callback = micStateChangeCallback_;
    cb->callbackName = MIC_STATE_CHANGE_CALLBACK_NAME;
    cb->micStateChangeEvent = micStateChangeEvent;
    return OnJsCallbackMicStateChange(cb);
}

void NapiAudioManagerMicStateChangeCallback::SafeJsCallbackMicStateChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioManagerMicStateChangeJsCallback *event = reinterpret_cast<AudioManagerMicStateChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackMicStateChange: no memory");
    std::shared_ptr<AudioManagerMicStateChangeJsCallback> safeContext(
        static_cast<AudioManagerMicStateChangeJsCallback*>(data),
        [](AudioManagerMicStateChangeJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackMicStateChangeWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueMicStateChange(env, event->micStateChangeEvent, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
            "%{public}s fail to create DeviceChange callback", request.c_str());
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call DeviceChange callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioManagerMicStateChangeCallback::MicStateChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("MicStateChangeTsfnFinalize: safe thread resource release.");
}

void NapiAudioManagerMicStateChangeCallback::OnJsCallbackMicStateChange(
    std::unique_ptr<AudioManagerMicStateChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackMicStateChange: jsCb.get() is null");
        return;
    }
    AudioManagerMicStateChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amMicStateChgTsfn_);
    napi_call_threadsafe_function(amMicStateChgTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS