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
#define LOG_TAG "NapiAudioCapturerStateCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_capturer_state_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_manager_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
NapiAudioCapturerStateCallback::NapiAudioCapturerStateCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioCapturerStateCallback: instance create");
}

NapiAudioCapturerStateCallback::~NapiAudioCapturerStateCallback()
{
    if (regAmacStateTsfn_) {
        napi_release_threadsafe_function(amacStateTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioCapturerStateCallback: instance destroy");
}

void NapiAudioCapturerStateCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioCapturerStateCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioCapturerStateCallback: creating callback failed");

    capturerStateCallback_ = cb;
}

void NapiAudioCapturerStateCallback::RemoveCallbackReference(const napi_value args)
{
    if (!IsSameCallback(args)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    capturerStateCallback_->cb_ = nullptr;
    capturerStateCallback_.reset();
    AUDIO_DEBUG_LOG("Remove capturerStateCallback success");
}

bool NapiAudioCapturerStateCallback::IsSameCallback(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (capturerStateCallback_ == nullptr) {
        return false;
    }
    if (args == nullptr) {
        return true;
    }
    napi_value capturerStateCallback = nullptr;
    napi_get_reference_value(env_, capturerStateCallback_->cb_, &capturerStateCallback);
    bool isEquals = false;
    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, args, capturerStateCallback, &isEquals) == napi_ok, false,
        "get napi_strict_equals failed");
    return isEquals;
}

void NapiAudioCapturerStateCallback::CreateCaptureStateTsfn(napi_env env)
{
    regAmacStateTsfn_ = true;
    std::string callbackName = "AudioCapturerState";
    napi_value cbName;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        CapturerStateTsfnFinalize, nullptr, SafeJsCallbackCapturerStateWork, &amacStateTsfn_);
}

bool NapiAudioCapturerStateCallback::GetCaptureStateTsfnFlag()
{
    return regAmacStateTsfn_;
}

void NapiAudioCapturerStateCallback::OnCapturerStateChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AUDIO_INFO_LOG("OnCapturerStateChange is called");

    std::lock_guard<std::mutex> lock(mutex_);
    std::unique_ptr<AudioCapturerStateJsCallback> cb = std::make_unique<AudioCapturerStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");

    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    for (const auto &changeInfo : audioCapturerChangeInfos) {
        capturerChangeInfos.push_back(std::make_shared<AudioCapturerChangeInfo>(*changeInfo));
    }

    cb->callback = capturerStateCallback_;
    cb->changeInfos = move(capturerChangeInfos);

    return OnJsCallbackCapturerState(cb);
}

void NapiAudioCapturerStateCallback::SafeJsCallbackCapturerStateWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioCapturerStateJsCallback *event = reinterpret_cast<AudioCapturerStateJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackCapturerState: no memory");
    std::shared_ptr<AudioCapturerStateJsCallback> safeContext(
        static_cast<AudioCapturerStateJsCallback*>(data),
        [](AudioCapturerStateJsCallback *ptr) {
            delete ptr;
    });
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetCapturerChangeInfos(env, event->changeInfos, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "fail to convert to jsobj");

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "fail to call Interrupt callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioCapturerStateCallback::CapturerStateTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("CapturerStateTsfnFinalize: safe thread resource release.");
}

void NapiAudioCapturerStateCallback::OnJsCallbackCapturerState(std::unique_ptr<AudioCapturerStateJsCallback> &jsCb)
{
    AudioCapturerStateJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    
    napi_acquire_threadsafe_function(amacStateTsfn_);
    napi_call_threadsafe_function(amacStateTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS