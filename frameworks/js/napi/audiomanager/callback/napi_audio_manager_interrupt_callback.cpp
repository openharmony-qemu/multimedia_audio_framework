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

void NapiAudioManagerInterruptCallback::RemoveCallbackReference(const std::string &callbackName, napi_value args)
{
    CHECK_AND_RETURN_LOG(!callbackName.compare(INTERRUPT_CALLBACK_NAME),
        "RemoveCallbackReference: Unknown callback type: %{public}s", callbackName.c_str());

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioManagerInterruptCallbackList_.begin(); it != audioManagerInterruptCallbackList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        if (isSameCallback) {
            napi_status status = napi_delete_reference(env_, (*it)->cb_);
            (*it)->cb_ = nullptr;
            CHECK_AND_RETURN_LOG(status == napi_ok, "RemoveCallbackReference: delete reference for callback fail");
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
    for (auto it = audioManagerInterruptCallbackList_.begin(); it != audioManagerInterruptCallbackList_.end(); ++it) {
        napi_status ret = napi_delete_reference(env_, (*it)->cb_);
        if (ret != napi_ok) {
            AUDIO_ERR_LOG("RemoveAllCallbackReferences: napi_delete_reference err.");
        }
        (*it)->cb_ = nullptr;
    }
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

void NapiAudioManagerInterruptCallback::WorkCallbackInterruptDone(uv_work_t *work, int status)
{
    std::shared_ptr<AudioManagerInterruptJsCallback> context(
        static_cast<AudioManagerInterruptJsCallback*>(work->data),
        [work](AudioManagerInterruptJsCallback* ptr) {
            delete ptr;
            delete work;
    });
    CHECK_AND_RETURN_LOG(work != nullptr, "work is nullptr");
    AudioManagerInterruptJsCallback *event = reinterpret_cast<AudioManagerInterruptJsCallback *>(work->data);
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
    std::string request = event->callbackName;
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
    napi_env env = event->callback->env_;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_DEBUG_LOG("JsCallBack %{public}s, uv_queue_work_with_qos start", request.c_str());
    do {
        CHECK_AND_BREAK_LOG(status != UV_ECANCELED, "%{public}s canceled", request.c_str());
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

void NapiAudioManagerInterruptCallback::OnJsCallbackAudioManagerInterrupt(
    std::unique_ptr<AudioManagerInterruptJsCallback> &jsCb)
{
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    CHECK_AND_RETURN_LOG(loop != nullptr, "loop is nullptr");

    uv_work_t *work = new(std::nothrow) uv_work_t;
    CHECK_AND_RETURN_LOG(work != nullptr, "OnJsCallbackAudioManagerInterrupt: No memory");

    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAudioManagerInterrupt: jsCb.get() is null");
        delete work;
        return;
    }
    work->data = reinterpret_cast<void *>(jsCb.get());
    int ret = uv_queue_work_with_qos(loop, work, [] (uv_work_t *work) {}, WorkCallbackInterruptDone,
        uv_qos_default);
    if (ret != 0) {
        AUDIO_ERR_LOG("OnJsCallbackAudioManagerInterrupt: Failed to execute libuv work queue");
        delete work;
    } else {
        jsCb.release();
    }
}
} // namespace AudioStandard
} // namespace OHOS