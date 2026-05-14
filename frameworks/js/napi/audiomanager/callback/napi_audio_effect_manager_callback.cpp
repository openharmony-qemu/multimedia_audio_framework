/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "NapiAudioEffectMgrCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_effect_manager_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_manager_callbacks.h"

namespace OHOS {
namespace AudioStandard {
bool NapiAudioSeparationEffectEnabledChangeCallback::onAudioSeparationEffectEnabledChangeFlag_;
using namespace std;

NapiAudioSeparationEffectEnabledChangeCallback::NapiAudioSeparationEffectEnabledChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioSeparationEffectEnabledChangeCallback: instance create");
}

NapiAudioSeparationEffectEnabledChangeCallback::~NapiAudioSeparationEffectEnabledChangeCallback()
{
    if (regAmSeparationEffectEnableTsfn_) {
        napi_release_threadsafe_function(amSeparationEffectEnableTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioSeparationEffectEnabledChangeCallback: instance destroy");
}

void NapiAudioSeparationEffectEnabledChangeCallback::CreateSeparationEffectEnableTsfn(napi_env env)
{
    napi_value cbName;
    regAmSeparationEffectEnableTsfn_ = true;
    std::string callbackName = "AudioSeparationEffectEnabledChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env_, nullptr, nullptr, cbName, 0, 1, nullptr,
        AudioSeparationEffectEnabledTsfnFinalize, nullptr,
        SafeJsCallbackAudioSeparationEffectEnabledWork, &amSeparationEffectEnableTsfn_);
}

bool NapiAudioSeparationEffectEnabledChangeCallback::GetSeparationEffectEnableTsfnFlag()
{
    return regAmSeparationEffectEnableTsfn_;
}

void NapiAudioSeparationEffectEnabledChangeCallback::SaveAudioSeparationEffectEnabledChangeCallbackReference(
    napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    std::string taskName = "NapiAudioSeparationEffectEnabledChangeCallback::destroy";
    
    for (auto it = audioSeparationEffectEnabledChangeCbList_.begin();
        it != audioSeparationEffectEnabledChangeCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env_, args, (*it)->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback,
            "SaveCallbackReference: audio effect manager has same callback");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioSeparationEffectEnabledChangeCallback: creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback, taskName);
    CHECK_AND_RETURN_LOG(cb != nullptr,
        "NapiAudioSeparationEffectEnabledChangeCallback: creating callback failed");

    audioSeparationEffectEnabledChangeCbList_.push_back(cb);
}

void NapiAudioSeparationEffectEnabledChangeCallback::RemoveAudioSeparationEffectEnabledChangeCallbackReference(
    napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioSeparationEffectEnabledChangeCbList_.begin();
        it != audioSeparationEffectEnabledChangeCbList_.end(); ++it) {
        bool isSameCallback = NapiAudioManagerCallback::IsSameCallback(env, args, (*it)->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("RemoveAudioSeparationEffectEnabledChangeCallbackReference: find js callback, delete it");
            napi_delete_reference(env, (*it)->cb_);
            (*it)->cb_ = nullptr;
            audioSeparationEffectEnabledChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("RemoveAudioSeparationEffectEnabledChangeCallbackReference: js callback no find");
}

void NapiAudioSeparationEffectEnabledChangeCallback::RemoveAllAudioSeparationEffectEnabledChangeCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioSeparationEffectEnabledChangeCbList_.begin();
        it != audioSeparationEffectEnabledChangeCbList_.end(); ++it) {
        napi_delete_reference(env_, (*it)->cb_);
        (*it)->cb_ = nullptr;
    }
    audioSeparationEffectEnabledChangeCbList_.clear();
    AUDIO_INFO_LOG("RemoveAllAudioSeparationEffectEnabledChangeCallbackReference: remove all js callbacks success");
}

int32_t NapiAudioSeparationEffectEnabledChangeCallback::GetAudioSeparationEffectEnabledChangeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return audioSeparationEffectEnabledChangeCbList_.size();
}

void NapiAudioSeparationEffectEnabledChangeCallback::OnAudioSeparationEffectEnabledChange(bool enabled)
{
    AUDIO_INFO_LOG("OnAudioSeparationEffectEnabledChange entered");
    std::lock_guard<std::mutex> lock(mutex_);

    for (auto it = audioSeparationEffectEnabledChangeCbList_.begin();
        it != audioSeparationEffectEnabledChangeCbList_.end(); it++) {
        std::unique_ptr<AudioSeparationEffectEnabledJsCallback> cb =
            std::make_unique<AudioSeparationEffectEnabledJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory!!");
        cb->callback = (*it);
        cb->enabled = enabled;
        onAudioSeparationEffectEnabledChangeFlag_ = true;
        OnJsCallbackAudioSeparationEffectEnabled(cb);
    }
}

void NapiAudioSeparationEffectEnabledChangeCallback::SafeJsCallbackAudioSeparationEffectEnabledWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSeparationEffectEnabledJsCallback *event =
        reinterpret_cast<AudioSeparationEffectEnabledJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackAudioSeparationEffectEnabled: no memory");
    std::shared_ptr<AudioSeparationEffectEnabledJsCallback> safeContext(
        static_cast<AudioSeparationEffectEnabledJsCallback*>(data),
        [](AudioSeparationEffectEnabledJsCallback *ptr) {
            delete ptr;
            ptr = nullptr;
    });
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAudioSeparationEffectEnabledWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "callback get reference value fail");
        
        napi_value args[ARGS_ONE] = { nullptr };
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;

        NapiParamUtils::SetValueBoolean(env, event->enabled, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr, "fail to convert to jsobj");

        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "Fail to call audio separation effect enabled callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSeparationEffectEnabledChangeCallback::AudioSeparationEffectEnabledTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AudioSeparationEffectEnabledTsfnFinalize: safe thread resource release.");
}

void NapiAudioSeparationEffectEnabledChangeCallback::OnJsCallbackAudioSeparationEffectEnabled(
    std::unique_ptr<AudioSeparationEffectEnabledJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAudioSeparationEffectEnabled: jsCb.get() is null");
        return;
    }

    AudioSeparationEffectEnabledJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    event->callbackName = "AudioSeparationEffectEnabledChange";

    napi_acquire_threadsafe_function(amSeparationEffectEnableTsfn_);
    napi_call_threadsafe_function(amSeparationEffectEnableTsfn_, event, napi_tsfn_blocking);
}

} // namespace AudioStandard
} // namespace OHOS