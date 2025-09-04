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
#define LOG_TAG "NapiAudioSessionInputDeviceCallback"
#endif
#include <thread>
#include "js_native_api.h"
#include "napi_audio_session_input_device_callback.h"
#include "napi_param_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioSessionInputDeviceCallback::NapiAudioSessionInputDeviceCallback(napi_env env)
    :env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSessionInputDeviceCallback::Constructor");
}

NapiAudioSessionInputDeviceCallback::~NapiAudioSessionInputDeviceCallback()
{
    if (regAmSessionInputDeviceChgTsfn_) {
        napi_release_threadsafe_function(amSessionInputDeviceChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioSessionInputDeviceCallback::Destructor");
}

void NapiAudioSessionInputDeviceCallback::OnAudioSessionCurrentInputDeviceChanged(
    const CurrentInputDeviceChangedEvent &deviceEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionCurrentInputDeviceChanged is called changeReason=%{public}d",
        deviceEvent.changeReason);
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(audioSessionInputDeviceJsCallback_ != nullptr,
        "OnAudioSessionCurrentInputDeviceChanged:No JS callback registered return");

    std::unique_ptr<AudioSessionInputDeviceJsCallback> cb = std::make_unique<AudioSessionInputDeviceJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioSessionInputDeviceJsCallback_;
    cb->callbackName = AUDIO_SESSION_INPUT_DEVICE_CALLBACK_NAME;
    cb->audioSessionDeviceEvent.devices = deviceEvent.devices;
    cb->audioSessionDeviceEvent.changeReason = deviceEvent.changeReason;

    return OnJsCallbackAudioSessionInputDevice(cb);
}

void NapiAudioSessionInputDeviceCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioSessionInputDeviceCallback: creating reference for callback fail");
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    audioSessionInputDeviceJsCallback_ = cb;
}

void NapiAudioSessionInputDeviceCallback::CreateAudioSessionInputDeviceTsfn(napi_env env)
{
    std::lock_guard<std::mutex> lock(mutex_);
    regAmSessionInputDeviceChgTsfn_ = true;
    std::string callbackName = "currentInputDeviceChanged";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, AudioSessionInputDeviceTsfnFinalize,
        nullptr, SafeJsCallbackAudioSessionInputDeviceWork, &amSessionInputDeviceChgTsfn_);
}

bool NapiAudioSessionInputDeviceCallback::GetAudioSessionInputDeviceTsfnFlag() const
{
    return regAmSessionInputDeviceChgTsfn_;
}

void NapiAudioSessionInputDeviceCallback::SafeJsCallbackAudioSessionInputDeviceWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSessionInputDeviceJsCallback *event = reinterpret_cast<AudioSessionInputDeviceJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAudioSessionInputDevice: no memory");

    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAudioSessionInputDeviceWork: safe js callback working.");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());

        napi_value args[ARGS_ONE] = { nullptr };
        napi_create_object(env, &args[PARAM0]);
        napi_value deviceObj = nullptr;
        nstatus = NapiParamUtils::SetDeviceDescriptors(env, event->audioSessionDeviceEvent.devices, deviceObj);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && deviceObj != nullptr, "fail to convert to jsobj");
        napi_set_named_property(env, args[PARAM0], "devices", deviceObj);
        nstatus = NapiParamUtils::SetValueInt32(env, "changeReason",
            static_cast<int32_t>(event->audioSessionDeviceEvent.changeReason), args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to changeReason callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call SetaudioSessionDevice callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSessionInputDeviceCallback::AudioSessionInputDeviceTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AudioSessionInputDeviceTsfnFinalize: safe thread resource release.");
}

void NapiAudioSessionInputDeviceCallback::OnJsCallbackAudioSessionInputDevice(
    std::unique_ptr<AudioSessionInputDeviceJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiAudioSessionInputDeviceCallback: OnJsCallbackAudioSessionInputDevice: jsCb.get() is null");
        return;
    }

    AudioSessionInputDeviceJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amSessionInputDeviceChgTsfn_);
    napi_call_threadsafe_function(amSessionInputDeviceChgTsfn_, event, napi_tsfn_blocking);
}

bool NapiAudioSessionInputDeviceCallback::ContainSameJsCallback(napi_value args)
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