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
#define LOG_TAG "NapiAudioRendererStateCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_renderer_state_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_manager_log.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
NapiAudioRendererStateCallback::NapiAudioRendererStateCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioRendererStateCallback: instance create");
}

NapiAudioRendererStateCallback::~NapiAudioRendererStateCallback()
{
    if (regAmRendererSatTsfn_) {
        napi_release_threadsafe_function(amRendererSatTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioRendererStateCallback: instance destroy");
}

void NapiAudioRendererStateCallback::RemoveCallbackReference(const napi_value args)
{
    if (!IsSameCallback(args)) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    rendererStateCallback_->cb_ = nullptr;
    rendererStateCallback_.reset();
    AUDIO_DEBUG_LOG("Remove rendererStateCallback success");
}

bool NapiAudioRendererStateCallback::IsSameCallback(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (rendererStateCallback_ == nullptr) {
        return false;
    }
    if (args == nullptr) {
        return true;
    }
    napi_value rendererStateCallback = nullptr;
    napi_get_reference_value(env_, rendererStateCallback_->cb_, &rendererStateCallback);
    bool isEquals = false;
    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, args, rendererStateCallback, &isEquals) == napi_ok, false,
        "get napi_strict_equals failed");
    return isEquals;
}

void NapiAudioRendererStateCallback::SaveCallbackReference(napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioRendererStateCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    CHECK_AND_RETURN_LOG(cb != nullptr, "NapiAudioRendererStateCallback: creating callback failed");

    rendererStateCallback_ = cb;
}

void NapiAudioRendererStateCallback::CreateRendererStateTsfn(napi_env env)
{
    regAmRendererSatTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "RendererState";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        RendererStateTsfnFinalize, nullptr, SafeJsCallbackRendererStateWork, &amRendererSatTsfn_);
}

bool NapiAudioRendererStateCallback::GetRendererStateTsfnFlag()
{
    return regAmRendererSatTsfn_;
}

void NapiAudioRendererStateCallback::OnRendererStateChange(
    const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AUDIO_PRERELEASE_LOGI("enter");

    std::lock_guard<std::mutex> lock(mutex_);

    CHECK_AND_RETURN_LOG(rendererStateCallback_ != nullptr, "rendererStateCallback_ is nullptr!");

    std::unique_ptr<AudioRendererStateJsCallback> cb = std::make_unique<AudioRendererStateJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    for (const auto &changeInfo : audioRendererChangeInfos) {
        rendererChangeInfos.push_back(std::make_shared<AudioRendererChangeInfo>(*changeInfo));
    }

    cb->callback = rendererStateCallback_;
    cb->changeInfos = move(rendererChangeInfos);

    return OnJsCallbackRendererState(cb);
}

void NapiAudioRendererStateCallback::SafeJsCallbackRendererStateWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioRendererStateJsCallback *event = reinterpret_cast<AudioRendererStateJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackRendererState: no memory");
    std::shared_ptr<AudioRendererStateJsCallback> safeContext(
        static_cast<AudioRendererStateJsCallback*>(data),
        [](AudioRendererStateJsCallback *ptr) {
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
        NapiParamUtils::SetRendererChangeInfos(env, event->changeInfos, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "fail to convert to jsobj");

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "fail to call Interrupt callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioRendererStateCallback::RendererStateTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("RingModeTsfnFinalize: safe thread resource release.");
}

void NapiAudioRendererStateCallback::OnJsCallbackRendererState(std::unique_ptr<AudioRendererStateJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRendererState: jsCb.get() is null");
        return;
    }

    AudioRendererStateJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    
    napi_acquire_threadsafe_function(amRendererSatTsfn_);
    napi_call_threadsafe_function(amRendererSatTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS
