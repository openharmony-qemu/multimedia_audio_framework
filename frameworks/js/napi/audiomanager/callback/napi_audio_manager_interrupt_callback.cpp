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
#define LOG_TAG "NapiAudioManagerInterruptCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_manager_interrupt_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_audio_manager_callbacks.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioManagerInterruptCallback::NapiAudioManagerInterruptCallback(napi_env env)
    : env_(env)
{
    AUDIO_INFO_LOG("instance create");
}

NapiAudioManagerInterruptCallback::~NapiAudioManagerInterruptCallback()
{
    if (regAmInterruptTsfn_) {
        napi_release_threadsafe_function(amInterruptTsfn_, napi_tsfn_abort);
    }
    AUDIO_INFO_LOG("instance destroy");
}

void NapiAudioManagerInterruptCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    CHECK_AND_RETURN_LOG(!callbackName.compare(INTERRUPT_CALLBACK_NAME),
        "SaveCallbackReference: Unknown callback type: %{public}s", callbackName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    bool isSameCallback = true;
    for (auto it = audioManagerInterruptCallbackList_.begin(); it != audioManagerInterruptCallbackList_.end(); ++it) {
        isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "SaveCallbackReference: the callback already exists");
    }
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "SaveCallbackReference: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    audioManagerInterruptCallbackList_.push_back(cb);
    AUDIO_INFO_LOG("SaveCallbackReference success, list size [%{public}zu]", audioManagerInterruptCallbackList_.size());
}

void NapiAudioManagerInterruptCallback::CreateManagerInterruptTsfn(napi_env env)
{
    regAmInterruptTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "ManagerInterrupt";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        AudioManagerInterruptTsfnFinalize, nullptr, SafeJsCallbackAudioManagerInterruptWork,
        &amInterruptTsfn_);
}

bool NapiAudioManagerInterruptCallback::GetManagerInterruptTsfnFlag()
{
    return regAmInterruptTsfn_;
}

void NapiAudioManagerInterruptCallback::RemoveCallbackReference(const std::string &callbackName, napi_value args)
{
    CHECK_AND_RETURN_LOG(!callbackName.compare(INTERRUPT_CALLBACK_NAME),
        "RemoveCallbackReference: Unknown callback type: %{public}s", callbackName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioManagerInterruptCallbackList_.begin(); it != audioManagerInterruptCallbackList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        if (isSameCallback) {
            audioManagerInterruptCallbackList_.erase(it);
            AUDIO_INFO_LOG("RemoveCallbackReference success, list size [%{public}zu]",
                audioManagerInterruptCallbackList_.size());
            return;
        }
    }
    AUDIO_ERR_LOG("RemoveCallbackReference: js callback no find");
}

void NapiAudioManagerInterruptCallback::RemoveAllCallbackReferences(const std::string &callbackName)
{
    CHECK_AND_RETURN_LOG(!callbackName.compare(INTERRUPT_CALLBACK_NAME),
        "RemoveCallbackReference: Unknown callback type: %{public}s", callbackName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    audioManagerInterruptCallbackList_.clear();
    AUDIO_INFO_LOG("RemoveAllCallbackReference: remove all js callbacks success");
}

int32_t NapiAudioManagerInterruptCallback::GetInterruptCallbackListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return audioManagerInterruptCallbackList_.size();
}

void NapiAudioManagerInterruptCallback::OnInterrupt(const InterruptAction &interruptAction)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("OnInterrupt action: %{public}d IntType: %{public}d, IntHint: %{public}d, activated: %{public}d",
        interruptAction.actionType, interruptAction.interruptType, interruptAction.interruptHint,
        interruptAction.activated);
    CHECK_AND_RETURN_LOG(audioManagerInterruptCallbackList_.size() != 0,
        "Cannot find the reference of interrupt callback");
    for (auto it = audioManagerInterruptCallbackList_.begin(); it != audioManagerInterruptCallbackList_.end(); ++it) {
        std::unique_ptr<AudioManagerInterruptJsCallback> cb = std::make_unique<AudioManagerInterruptJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = *it;
        cb->callbackName = INTERRUPT_CALLBACK_NAME;
        cb->interruptAction = interruptAction;
        OnJsCallbackAudioManagerInterrupt(cb);
    }
}

void NapiAudioManagerInterruptCallback::SafeJsCallbackAudioManagerInterruptWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioManagerInterruptJsCallback *event = reinterpret_cast<AudioManagerInterruptJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAudioManagerInterrupt: no memory");
    std::shared_ptr<AudioManagerInterruptJsCallback> safeContext(
        static_cast<AudioManagerInterruptJsCallback*>(data),
        [](AudioManagerInterruptJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAudioManagerInterruptWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueInterruptAction(env, event->interruptAction, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create Interrupt callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call Interrupt callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioManagerInterruptCallback::AudioManagerInterruptTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AudioManagerInterruptTsfnFinalize: safe thread resource release.");
}

void NapiAudioManagerInterruptCallback::OnJsCallbackAudioManagerInterrupt(
    std::unique_ptr<AudioManagerInterruptJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackRendererState: jsCb.get() is null");
        return;
    }

    AudioManagerInterruptJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amInterruptTsfn_);
    napi_call_threadsafe_function(amInterruptTsfn_, event, napi_tsfn_blocking);
}
} // namespace AudioStandard
} // namespace OHOS