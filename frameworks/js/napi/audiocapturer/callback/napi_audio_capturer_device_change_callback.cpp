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
    AUDIO_DEBUG_LOG("Instance destroy");
}

void NapiAudioCapturerDeviceChangeCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "Creating reference for callback fail");

    callback_ = callback;
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

void NapiAudioCapturerDeviceChangeCallback::OnStateChange(const DeviceInfo &deviceInfo)
{
    OnJsCallbackCapturerDeviceInfo(callback_, deviceInfo);
}

void NapiAudioCapturerDeviceChangeCallback::WorkCallbackCompleted(uv_work_t *work, int status)
{
    // Js Thread
    std::shared_ptr<AudioCapturerDeviceChangeJsCallback> context(
        static_cast<AudioCapturerDeviceChangeJsCallback*>(work->data),
        [work](AudioCapturerDeviceChangeJsCallback* ptr) {
            delete ptr;
            delete work;
    });

    AudioCapturerDeviceChangeJsCallback *event = reinterpret_cast<AudioCapturerDeviceChangeJsCallback*>(work->data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback_) != nullptr,
        "OnJsCallbackCapturerDeviceInfo: no memory");

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

void NapiAudioCapturerDeviceChangeCallback::OnJsCallbackCapturerDeviceInfo(napi_ref method,
    const DeviceInfo &deviceInfo)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "Loop is nullptr");
    CHECK_AND_RETURN_LOG(method != nullptr, "method is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackCapturerDeviceInfo: no memory");

    work->data = new AudioCapturerDeviceChangeJsCallback {method, env_, deviceInfo};
    if (work->data == nullptr) {
        AUDIO_ERR_LOG("work data malloc failed: No memory");
        delete work;
        return;
    }

    int ret = uv_queue_work(loop, work, [] (uv_work_t *work) {}, WorkCallbackCompleted);
    if (ret != 0) {
        AUDIO_ERR_LOG("Failed to execute libuv work queue");
        if (work != nullptr) {
            if (work->data != nullptr) {
                delete reinterpret_cast<AudioCapturerDeviceChangeJsCallback*>(work->data);
            }
            delete work;
        }
    }
}
}  // namespace AudioStandard
}  // namespace OHOS
