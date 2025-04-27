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

#include "js_native_api.h"
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
    if (regAcInfoChgTsfn_) {
        napi_release_threadsafe_function(acInfoChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("Instance destroy");
}

void NapiAudioCapturerInfoChangeCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // create function that will operate while save callback reference success.
    std::function<void(std::shared_ptr<AutoRef> generatedCallback)> successed =
        [this](std::shared_ptr<AutoRef> generatedCallback) {
            callbackPtr_ = generatedCallback;
            callback_ = callbackPtr_->cb_;
        };
        NapiAudioCapturerCallbackInner::SaveCallbackReferenceInner(callbackName, args, successed);
}

void NapiAudioCapturerInfoChangeCallback::RemoveCallbackReference(const std::string &callbackName, napi_env env,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::function<void()> successed =
        [this]() {
        callbackPtr_ = nullptr;
        callback_ = nullptr;
        };
    NapiAudioCapturerCallbackInner::RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

void NapiAudioCapturerInfoChangeCallback::CreateCaptureInfoChangeTsfn(napi_env env)
{
    regAcInfoChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "AudioCapturerChangeInfo";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env_, nullptr, nullptr, cbName, 0, 1, nullptr, CapturerChangeInfoTsfnFinalize,
        nullptr, SafeJsCallbackCapturerChangeInfoWork, &acInfoChgTsfn_);
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

void NapiAudioCapturerInfoChangeCallback::SafeJsCallbackCapturerChangeInfoWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioCapturerChangeInfoJsCallback *event = reinterpret_cast<AudioCapturerChangeInfoJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback_) != nullptr,
        "OnJsCallbackCapturerChangeInfo: no memory");
    std::shared_ptr<AudioCapturerChangeInfoJsCallback> safeContext(
        static_cast<AudioCapturerChangeInfoJsCallback*>(data),
        [](AudioCapturerChangeInfoJsCallback *ptr) {
            delete ptr;
    });
    if (event == nullptr || event->callback_ == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackCapturerChangeInfo: no memory");
        return;
    }
    napi_ref callback = event->callback_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackCapturerChangeInfoWork: safe js callback working.");
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
}

void NapiAudioCapturerInfoChangeCallback::CapturerChangeInfoTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("CapturerChangeInfoTsfnFinalize: safe thread resource release.");
}

void NapiAudioCapturerInfoChangeCallback::OnJsCallbackCapturerChangeInfo(napi_ref method,
    const AudioCapturerChangeInfo &capturerChangeInfo)
{
    CHECK_AND_RETURN_LOG(method != nullptr, "method is nullptr");
    AudioCapturerChangeInfoJsCallback *event =
        new AudioCapturerChangeInfoJsCallback {method, env_, capturerChangeInfo};
    
    napi_acquire_threadsafe_function(acInfoChgTsfn_);
    napi_call_threadsafe_function(acInfoChgTsfn_, event, napi_tsfn_blocking);
}

napi_env &NapiAudioCapturerInfoChangeCallback::GetEnv()
{
    return env_;
}

std::shared_ptr<AutoRef> NapiAudioCapturerInfoChangeCallback::GetCallback(const std::string &callbackName)
{
    return callbackPtr_;
}

bool NapiAudioCapturerInfoChangeCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    if (callbackName == AUDIO_CAPTURER_CHANGE_CALLBACK_NAME) {
        return true;
    }
    return false;
}
}  // namespace AudioStandard
}  // namespace OHOS
