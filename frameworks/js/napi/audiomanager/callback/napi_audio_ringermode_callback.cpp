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
#define LOG_TAG "NapiAudioRingerModeCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_ringermode_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_enum.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioRingerModeCallback::NapiAudioRingerModeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiAudioRingerModeCallback::~NapiAudioRingerModeCallback()
{
    if (regAmRmChgTsfn_) {
        napi_release_threadsafe_function(amRmChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiAudioRingerModeCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioRingerModeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == RINGERMODE_CALLBACK_NAME) {
        ringerModeCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("NapiAudioRingerModeCallback: Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void NapiAudioRingerModeCallback::CreateRingModeTsfn(napi_env env)
{
    regAmRmChgTsfn_ = true;
    std::string callbackName = "RingerMode";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, RingModeTsfnFinalize,
        nullptr, SafeJsCallbackRingModeWork, &amRmChgTsfn_);
}

bool NapiAudioRingerModeCallback::GetRingModeTsfnFlag()
{
    return regAmRmChgTsfn_;
}

void NapiAudioRingerModeCallback::RemoveCallbackReference(const napi_value args)
{
    if (!IsSameCallback(args)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    ringerModeCallback_->cb_ = nullptr;
    ringerModeCallback_ = nullptr;
    AUDIO_INFO_LOG("Remove callback reference successful.");
}

bool NapiAudioRingerModeCallback::IsSameCallback(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (ringerModeCallback_ == nullptr) {
        return false;
    }
    if (args == nullptr) {
        return true;
    }
    napi_value ringerModeCallback = nullptr;
    napi_get_reference_value(env_, ringerModeCallback_->cb_, &ringerModeCallback);
    bool isEquals = false;
    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, args, ringerModeCallback, &isEquals) == napi_ok, false,
        "get napi_strict_equals failed");
    return isEquals;
}

void NapiAudioRingerModeCallback::OnRingerModeUpdated(const AudioRingerMode &ringerMode)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("NapiAudioRingerModeCallback: ringer mode: %{public}d", ringerMode);
    CHECK_AND_RETURN_LOG(ringerModeCallback_ != nullptr, "Cannot find the reference of ringer mode callback");

    std::unique_ptr<AudioRingerModeJsCallback> cb = std::make_unique<AudioRingerModeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = ringerModeCallback_;
    cb->callbackName = RINGERMODE_CALLBACK_NAME;
    cb->ringerMode = ringerMode;
    return OnJsCallbackRingerMode(cb);
}

static NapiAudioEnum::AudioRingMode GetJsAudioRingMode(int32_t ringerMode)
{
    NapiAudioEnum::AudioRingMode result;

    switch (ringerMode) {
        case RINGER_MODE_SILENT:
            result = NapiAudioEnum::RINGER_MODE_SILENT;
            break;
        case RINGER_MODE_VIBRATE:
            result = NapiAudioEnum::RINGER_MODE_VIBRATE;
            break;
        case RINGER_MODE_NORMAL:
            result = NapiAudioEnum::RINGER_MODE_NORMAL;
            break;
        default:
            result = NapiAudioEnum::RINGER_MODE_NORMAL;
            break;
    }

    return result;
}

void NapiAudioRingerModeCallback::SafeJsCallbackRingModeWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioRingerModeJsCallback *event = reinterpret_cast<AudioRingerModeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackRingerMode: no memory");
    std::shared_ptr<AudioRingerModeJsCallback> safeContext(
        static_cast<AudioRingerModeJsCallback*>(data),
        [](AudioRingerModeJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr.");
    AUDIO_INFO_LOG("SafeJsCallbackRingModeWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueInt32(env, GetJsAudioRingMode(event->ringerMode), args[PARAM0]);
        CHECK_AND_BREAK_LOG(args[PARAM0] != nullptr,
            "%{public}s fail to create ringer mode callback", request.c_str());
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call ringer mode callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioRingerModeCallback::RingModeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("RingModeTsfnFinalize: safe thread resource release.");
}

void NapiAudioRingerModeCallback::OnJsCallbackRingerMode(std::unique_ptr<AudioRingerModeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiAudioRingerModeCallback: OnJsCallbackRingerMode: jsCb.get() is null");
        return;
    }

    AudioRingerModeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amRmChgTsfn_);
    napi_call_threadsafe_function(amRmChgTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS