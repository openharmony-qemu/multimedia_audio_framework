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
#define LOG_TAG "NapiAudioCapturerInfoChangeCallback"
#endif

#include "napi_audio_capturer_info_change_callback.h"
#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "napi_param_utils.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioCapturerInfoChangeCallback::NapiAudioCapturerInfoChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("Instance create");
}

NapiAudioCapturerInfoChangeCallback::~NapiAudioCapturerInfoChangeCallback()
{
    AUDIO_DEBUG_LOG("Instance destroy");
}

void NapiAudioCapturerInfoChangeCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "Creating reference for callback fail");

    callback_ = callback;
}

bool NapiAudioCapturerInfoChangeCallback::ContainSameJsCallback(napi_value args)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env_, callback_, &copyValue);
    CHECK_AND_RETURN_RET_LOG(args != nullptr, false, "args is nullptr");

    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, copyValue, args, &isEquals) == napi_ok, false,
        "Get napi_strict_equals failed");

    return isEquals;
}

void NapiAudioCapturerInfoChangeCallback::OnStateChange(const AudioCapturerChangeInfo &capturerChangeInfo)
{
    OnJsCallbackCapturerChangeInfo(callback_, capturerChangeInfo);
}

void NapiAudioCapturerInfoChangeCallback::OnJsCallbackCapturerChangeInfo(napi_ref method,
    const AudioCapturerChangeInfo &capturerChangeInfo)
{
    CHECK_AND_RETURN_LOG(method != nullptr, "method is nullptr");
    AudioCapturerChangeInfoJsCallback *event =
        new AudioCapturerChangeInfoJsCallback {method, env_, capturerChangeInfo};

    auto task = [event]() {
        std::shared_ptr<AudioCapturerChangeInfoJsCallback> context(
            static_cast<AudioCapturerChangeInfoJsCallback*>(event),
            [](AudioCapturerChangeInfoJsCallback* ptr) {
                delete ptr;
        });
        if (event == nullptr || event->callback_ == nullptr) {
            AUDIO_ERR_LOG("OnJsCallbackCapturerChangeInfo: no memory");
            return;
        }
        napi_env env = event->env_;
        napi_ref callback = event->callback_;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
        do {
            napi_value jsCallback = nullptr;
            napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "Callback get reference value fail");
            // Call back function
            napi_value args[ARGS_ONE] = { nullptr };
            NapiParamUtils::SetAudioCapturerChangeInfoDescriptors(env, event->capturerChangeInfo_, args[0]);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
                "Fail to convert to jsobj");

            const size_t argCount = ARGS_ONE;
            napi_value result = nullptr;
            nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
            CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call capturer callback");
        } while (0);
        napi_close_handle_scope(env, scope);
    };
    if (napi_status::napi_ok != napi_send_event(env_, task, napi_eprio_immediate)) {
        AUDIO_ERR_LOG("OnJsCallbackCapturerChangeInfo: Failed to SendEvent");
    }
}
}  // namespace AudioStandard
}  // namespace OHOS
