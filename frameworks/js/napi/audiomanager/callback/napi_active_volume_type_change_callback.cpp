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
#define LOG_TAG "NapiActiveVolumeTypeChangeCallback"
#endif

#include "napi_active_volume_type_change_callback.h"
#include "js_native_api.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_enum.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioManagerActiveVolumeTypeChangeCallback::NapiAudioManagerActiveVolumeTypeChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiAudioManagerActiveVolumeTypeChangeCallback::~NapiAudioManagerActiveVolumeTypeChangeCallback()
{
    if (regAmActiveVolumeTypeChgTsfn_) {
        napi_release_threadsafe_function(amActiveVolumeTypeChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::CreateManagerActiveVolumeTypeChangeTsfn(napi_env env)
{
    regAmActiveVolumeTypeChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "ManagerActiveVolumeTypeChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr, ActiveVolumeTypeChangeTsfnFinalize,
        nullptr, SafeJsCallbackActiveVolumeTypeChangeWork, &amActiveVolumeTypeChgTsfn_);
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::ActiveVolumeTypeChangeTsfnFinalize(napi_env env, void *data,
    void *hint)
{
    AUDIO_INFO_LOG("safe thread resource release.");
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::SafeJsCallbackActiveVolumeTypeChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioManagerActiveVolumeTypeChangeJsCallback *event =
        reinterpret_cast<AudioManagerActiveVolumeTypeChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "no memory");
    std::shared_ptr<AudioManagerActiveVolumeTypeChangeJsCallback> safeContext(
        static_cast<AudioManagerActiveVolumeTypeChangeJsCallback*>(data),
        [](AudioManagerActiveVolumeTypeChangeJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "get reference value fail, %{public}s",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueInt32(env, NapiAudioEnum::GetJsAudioVolumeType(event->activeVolumeTypeChangeEvent),
            args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
            "fail to create active volume type change callback, %{public}s", request.c_str());
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "fail to call DeviceChange callback, %{public}s",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::OnActiveVolumeTypeChanged(const AudioVolumeType &event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("callback change enter");
    for (auto iter : activeVolumeTypeChangeList_) {
        std::unique_ptr<AudioManagerActiveVolumeTypeChangeJsCallback> cb =
            std::make_unique<AudioManagerActiveVolumeTypeChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = iter;
        cb->callbackName = ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME;
        cb->activeVolumeTypeChangeEvent = event;
        OnJsCallbackActiveVolumeTypeChange(cb);
    }
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::OnJsCallbackActiveVolumeTypeChange(
    std::unique_ptr<AudioManagerActiveVolumeTypeChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("jsCb.get() is null");
        return;
    }
    AudioManagerActiveVolumeTypeChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr.");
    CHECK_AND_RETURN_LOG(event->callback != nullptr, "callback is nullptr.");

    napi_acquire_threadsafe_function(amActiveVolumeTypeChgTsfn_);
    napi_call_threadsafe_function(amActiveVolumeTypeChgTsfn_, event, napi_tsfn_blocking);
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::SaveActiveVolumeTypeChangeCallbackReference(
    const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter : activeVolumeTypeChangeList_) {
        if (IsSameCallback(env_, args, iter->cb_)) {
            AUDIO_ERR_LOG("activeVolumeTypeChangeList_ has same callback, nothing to do");
            return;
        }
    }
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME) {
        activeVolumeTypeChangeList_.push_back(cb);
    }  else {
        AUDIO_ERR_LOG("Unknown callback type: %{public}s",
            callbackName.c_str());
    }
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::RemoveSelfActiveVolumeTypeChangeCbRef(napi_env env,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter = activeVolumeTypeChangeList_.begin(); iter != activeVolumeTypeChangeList_.end();) {
        if (IsSameCallback(env, callback, (*iter)->cb_)) {
            AUDIO_INFO_LOG("find js callback, erase it");
            activeVolumeTypeChangeList_.erase(iter++);
        } else {
            iter++;
        }
    }
    AUDIO_INFO_LOG("remove callback finish");
}

bool NapiAudioManagerActiveVolumeTypeChangeCallback::IsSameCallback(napi_env env, napi_value callback,
    napi_ref refCallback)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env, refCallback, &copyValue);
    if (napi_strict_equals(env, copyValue, callback, &isEquals) != napi_ok) {
        AUDIO_ERR_LOG("get napi_strict_equals failed");
        return false;
    }

    return isEquals;
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::RemoveAllActiveVolumeTypeChangeCbRef()
{
    std::lock_guard<std::mutex> lock(mutex_);
    activeVolumeTypeChangeList_.clear();
    AUDIO_INFO_LOG("remove callback finish");
}

void NapiAudioManagerActiveVolumeTypeChangeCallback::RemoveCallbackReference(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    CHECK_AND_RETURN_LOG(activeVolumeTypeChangeCallback_ != nullptr, "activeVolumeTypeChangeCallback_ is null");

    napi_delete_reference(env_, activeVolumeTypeChangeCallback_->cb_);
    activeVolumeTypeChangeCallback_->cb_ = nullptr;
    activeVolumeTypeChangeCallback_ = nullptr;
    AUDIO_INFO_LOG("remove callback reference successful.");
}

int32_t NapiAudioManagerActiveVolumeTypeChangeCallback::GetActiveVolumeTypeChangeListSize()
{
    return activeVolumeTypeChangeList_.size();
}

bool NapiAudioManagerActiveVolumeTypeChangeCallback::GetManagerActiveVolumeTypeChangeTsfnFlag()
{
    return regAmActiveVolumeTypeChgTsfn_;
}
}
}