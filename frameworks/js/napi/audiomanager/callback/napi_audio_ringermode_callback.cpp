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

#include "napi_audio_ringermode_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_enum.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioRingerModeCallback::NapiAudioRingerModeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiAudioRingerModeCallback::~NapiAudioRingerModeCallback()
{
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

void NapiAudioRingerModeCallback::RemoveCallbackReference(const napi_value args)
{
    if (!IsSameCallback(args)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    napi_delete_reference(env_, ringerModeCallback_->cb_);
    ringerModeCallback_->cb_ = nullptr;
    ringerModeCallback_ = nullptr;
    AUDIO_INFO_LOG("Remove callback reference successful.");
}

bool NapiAudioRingerModeCallback::IsSameCallback(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (args == nullptr) {
        return true;
    }
    if (ringerModeCallback_ == nullptr) {
        return false;
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
    AUDIO_DEBUG_LOG("NapiAudioRingerModeCallback: ringer mode: %{public}d", ringerMode);
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

void NapiAudioRingerModeCallback::OnJsCallbackRingerMode(std::unique_ptr<AudioRingerModeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRendererState: jsCb.get() is null");
        return;
    }

    AudioRingerModeJsCallback *event = jsCb.get();
    auto task = [event]() {
        std::shared_ptr<AudioRingerModeJsCallback> context(
            static_cast<AudioRingerModeJsCallback*>(event),
            [](AudioRingerModeJsCallback* ptr) {
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
            napi_value args[ARGS_ONE] = { nullptr };
            NapiParamUtils::SetValueInt32(env, GetJsAudioRingMode(event->ringerMode), args[PARAM0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
                "%{public}s fail to create ringer mode callback", request.c_str());

            const size_t argCount = ARGS_ONE;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call ringer mode callback",
                request.c_str());
        } while (0);
        napi_close_handle_scope(env, scope);
    };
    if (napi_status::napi_ok != napi_send_event(env_, task, napi_eprio_immediate)) {
        AUDIO_ERR_LOG("OnJsCallbackRingerMode: Failed to SendEvent");
    } else {
        jsCb.release();
    }
}
} // namespace AudioStandard
} // namespace OHOS