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
#define LOG_TAG "NapiAudioSessionDeviceCallback"
#endif
#include <thread>
#include "js_native_api.h"
#include "napi_audio_session_device_callback.h"
#include "napi_param_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioSessionDeviceCallback::NapiAudioSessionDeviceCallback(napi_env env)
    :env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSessionDeviceCallback::Constructor");
}

NapiAudioSessionDeviceCallback::~NapiAudioSessionDeviceCallback()
{
    if (regAmSessionDeviceChgTsfn_) {
        napi_release_threadsafe_function(amSessionDeviceChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioSessionDeviceCallback::Destructor");
}

void NapiAudioSessionDeviceCallback::OnAudioSessionCurrentDeviceChanged(
    const CurrentOutputDeviceChangedEvent &deviceEvent)
{
    AUDIO_INFO_LOG("OnAudioSessionCurrentDeviceChanged is called changeReason=%{public}d, "
        "recommendedAction=%{public}d", deviceEvent.changeReason, deviceEvent.recommendedAction);
    CHECK_AND_RETURN_LOG(audioSessionDeviceJsCallback_ != nullptr,
        "OnAudioSessionCurrentDeviceChanged:No JS callback registered return");
    std::unique_ptr<AudioSessionDeviceJsCallback> cb = std::make_unique<AudioSessionDeviceJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioSessionDeviceJsCallback_;
    cb->callbackName = AUDIO_SESSION_DEVICE_CALLBACK_NAME;
    cb->audioSessionDeviceEvent.devices = deviceEvent.devices;
    cb->audioSessionDeviceEvent.changeReason = deviceEvent.changeReason;
    cb->audioSessionDeviceEvent.recommendedAction = deviceEvent.recommendedAction;

    return OnJsCallbackAudioSessionDevice(cb);
}

void NapiAudioSessionDeviceCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioSessionDeviceCallback: creating reference for callback fail");
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    audioSessionDeviceJsCallback_ = cb;
}

void NapiAudioSessionDeviceCallback::CreateAudioSessionDeviceTsfn(napi_env env)
{
    regAmSessionDeviceChgTsfn_ = true;
    std::string callbackName = "currentOutputDeviceChanged";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, AudioSessionDeviceTsfnFinalize,
        nullptr, SafeJsCallbackAudioSessionDeviceWork, &amSessionDeviceChgTsfn_);
}

bool NapiAudioSessionDeviceCallback::GetAudioSessionDeviceTsfnFlag() const
{
    return regAmSessionDeviceChgTsfn_;
}

void NapiAudioSessionDeviceCallback::SafeJsCallbackAudioSessionDeviceWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSessionDeviceJsCallback *event = reinterpret_cast<AudioSessionDeviceJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAudioSessionDevice: no memory");

    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAudioSessionDeviceWork: safe js callback working.");
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
        nstatus = NapiParamUtils::SetValueInt32(env, "recommendedAction",
            static_cast<int32_t>(event->audioSessionDeviceEvent.recommendedAction), args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to recommendedAction callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call SetaudioSessionDevice callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSessionDeviceCallback::AudioSessionDeviceTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AudioSessionDeviceTsfnFinalize: safe thread resource release.");
}

void NapiAudioSessionDeviceCallback::OnJsCallbackAudioSessionDevice(
    std::unique_ptr<AudioSessionDeviceJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("NapiAudioSessionDeviceCallback: OnJsCallbackAudioSessionDevice: jsCb.get() is null");
        return;
    }

    AudioSessionDeviceJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amSessionDeviceChgTsfn_);
    napi_call_threadsafe_function(amSessionDeviceChgTsfn_, event, napi_tsfn_blocking);
}

bool NapiAudioSessionDeviceCallback::ContainSameJsCallback(napi_value args)
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