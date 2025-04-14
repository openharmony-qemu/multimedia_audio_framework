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
#define LOG_TAG "NapiAudioCapturerDeviceChangeCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_capturer_device_change_callback.h"
#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "napi_param_utils.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
NapiAudioCapturerDeviceChangeCallback::NapiAudioCapturerDeviceChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("Instance create");
}

NapiAudioCapturerDeviceChangeCallback::~NapiAudioCapturerDeviceChangeCallback()
{
    if (regAcDevChgTsfn_) {
        napi_release_threadsafe_function(acDevChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("Instance destroy");
}

void NapiAudioCapturerDeviceChangeCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
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

void NapiAudioCapturerDeviceChangeCallback::RemoveCallbackReference(
    const std::string &callbackName, napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::function<void()> successed = [this]() {
        callbackPtr_ = nullptr;
        callback_ = nullptr;
    };
    NapiAudioCapturerCallbackInner::RemoveCallbackReferenceInner(callbackName, env, callback, successed);
}

void NapiAudioCapturerDeviceChangeCallback::CreateCaptureDeviceChangeTsfn(napi_env env)
{
    regAcDevChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "AudioCapturerDeviceChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, CaptureDeviceInfoTsfnFinalize,
        nullptr, SafeJsCallbackCapturerDeviceInfoWork, &acDevChgTsfn_);
}

bool NapiAudioCapturerDeviceChangeCallback::ContainSameJsCallback(napi_value args)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env_, callback_, &copyValue);
    CHECK_AND_RETURN_RET_LOG(args != nullptr, false, "args is nullptr");

    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, copyValue, args, &isEquals) == napi_ok, false,
        "Get napi_strict_equals failed");

    return isEquals;
}

void NapiAudioCapturerDeviceChangeCallback::OnStateChange(const AudioDeviceDescriptor &deviceInfo)
{
    OnJsCallbackCapturerDeviceInfo(callback_, deviceInfo);
}

void NapiAudioCapturerDeviceChangeCallback::SafeJsCallbackCapturerDeviceInfoWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioCapturerDeviceChangeJsCallback *event = reinterpret_cast<AudioCapturerDeviceChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback_) != nullptr,
        "OnJsCallbackCapturerDeviceInfo: no memory");
    std::shared_ptr<AudioCapturerDeviceChangeJsCallback> safeContext(
        static_cast<AudioCapturerDeviceChangeJsCallback*>(data),
        [](AudioCapturerDeviceChangeJsCallback *ptr) {
            delete ptr;
    });
    napi_ref callback = event->callback_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackCapturerDeviceInfoWork: safe js callback working.");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "Callback get reference value fail");
        // Call back function
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueDeviceInfo(env, event->deviceInfo_, args[0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            " Fail to convert to jsobj");
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call devicechange callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioCapturerDeviceChangeCallback::CaptureDeviceInfoTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("RingModeTsfnFinalize: safe thread resource release.");
}

void NapiAudioCapturerDeviceChangeCallback::OnJsCallbackCapturerDeviceInfo(napi_ref method,
    const AudioDeviceDescriptor &deviceInfo)
{
    CHECK_AND_RETURN_LOG(method != nullptr, "method is nullptr");
    AudioCapturerDeviceChangeJsCallback *event = new AudioCapturerDeviceChangeJsCallback {method, env_, deviceInfo};

    if (event == nullptr) {
        AUDIO_ERR_LOG("event data malloc failed: No memory");
        return;
    }

    napi_acquire_threadsafe_function(acDevChgTsfn_);
    napi_call_threadsafe_function(acDevChgTsfn_, event, napi_tsfn_blocking);
}

napi_env &NapiAudioCapturerDeviceChangeCallback::GetEnv()
{
    return env_;
}

std::shared_ptr<AutoRef> NapiAudioCapturerDeviceChangeCallback::GetCallback(const std::string &callbackName)
{
    return callbackPtr_;
}

bool NapiAudioCapturerDeviceChangeCallback::CheckIfTargetCallbackName(const std::string &callbackName)
{
    return (callbackName == INPUTDEVICE_CHANGE_CALLBACK_NAME);
}
}  // namespace AudioStandard
}  // namespace OHOS
