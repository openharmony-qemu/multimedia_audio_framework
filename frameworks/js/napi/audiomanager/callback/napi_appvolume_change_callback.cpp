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
#define LOG_TAG "NapiAppVolumeChangeCallback"
#endif

#include "js_native_api.h"
#include "napi_appvolume_change_callback.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_enum.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AudioStandard {
NapiAudioManagerAppVolumeChangeCallback::NapiAudioManagerAppVolumeChangeCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("NapiAudioManagerAppVolumeChangeCallback: instance create");
}

NapiAudioManagerAppVolumeChangeCallback::~NapiAudioManagerAppVolumeChangeCallback()
{
    if (regAmAppVolumeChgTsfn_) {
        napi_release_threadsafe_function(amAppVolumeChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("NapiAudioManagerAppVolumeChangeCallback: instance destroy");
}

void NapiAudioManagerAppVolumeChangeCallback::SaveVolumeChangeCallbackForUidReference(
    const std::string &callbackName, napi_value args, int32_t appUid)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter : appVolumeChangeForUidList_) {
        if (iter.second == appUid && IsSameCallback(env_, args, iter.first->cb_)) {
            AUDIO_ERR_LOG("appVolumeChangeForUidList_ has same callback and appUid, nothing to do");
            return;
        }
    }
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioManagerAppVolumeChangeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID) {
        appVolumeChangeForUidList_.push_back({cb, appUid});
    }  else {
        AUDIO_ERR_LOG("NapiAudioManagerAppVolumeChangeCallback: Unknown callback type: %{public}s",
            callbackName.c_str());
    }
}

void NapiAudioManagerAppVolumeChangeCallback::SaveSelfVolumdChangeCallbackReference(
    const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter : selfAppVolumeChangeList_) {
        if (IsSameCallback(env_, args, iter->cb_)) {
            AUDIO_ERR_LOG("selfAppVolumeChangeList_ has same callback, nothing to do");
            return;
        }
    }
    napi_ref callback = nullptr;
    const int32_t refCount = ARGS_ONE;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioManagerAppVolumeChangeCallback: creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == APP_VOLUME_CHANGE_CALLBACK_NAME) {
        selfAppVolumeChangeList_.push_back(cb);
    }  else {
        AUDIO_ERR_LOG("NapiAudioManagerAppVolumeChangeCallback: Unknown callback type: %{public}s",
            callbackName.c_str());
    }
}

void NapiAudioManagerAppVolumeChangeCallback::CreateManagerAppVolumeChangeTsfn(napi_env env)
{
    regAmAppVolumeChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "ManagerAppVolumeChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        AppVolumeChangeTsfnFinalize, nullptr, SafeJsCallbackAppVolumeChangeWork, &amAppVolumeChgTsfn_);
}

bool NapiAudioManagerAppVolumeChangeCallback::GetManagerAppVolumeChangeTsfnFlag()
{
    return regAmAppVolumeChgTsfn_;
}

void NapiAudioManagerAppVolumeChangeCallback::RemoveCallbackReference(const napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_delete_reference(env_, appVolumeChangeCallback_->cb_);
    appVolumeChangeCallback_->cb_ = nullptr;
    appVolumeChangeCallback_ = nullptr;
    AUDIO_INFO_LOG("Remove callback reference successful.");
}

bool NapiAudioManagerAppVolumeChangeCallback::IsSameCallback(napi_env env, napi_value callback,
    napi_ref refCallback)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env, refCallback, &copyValue);
    if (napi_strict_equals(env, copyValue, callback, &isEquals) != napi_ok) {
        AUDIO_ERR_LOG("IsSameCallback: get napi_strict_equals failed");
        return false;
    }

    return isEquals;
}

// LCOV_EXCL_START
void NapiAudioManagerAppVolumeChangeCallback::OnAppVolumeChangedForUid(int32_t appUid, const VolumeEvent &event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!PermissionUtil::VerifySystemPermission()) {
        AUDIO_ERR_LOG("OnAppVolumeChangedForUid: No system permission");
        return;
    }
    AUDIO_DEBUG_LOG("OnAppVolumeChangedForUid: appUid[%{public}d]", appUid);
    for (auto iter : appVolumeChangeForUidList_) {
        if (appUid != iter.second) {
            continue;
        }
        std::unique_ptr<AudioManagerAppVolumeChangeJsCallback> cb =
            std::make_unique<AudioManagerAppVolumeChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = iter.first;
        cb->callbackName = APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID;
        cb->appVolumeChangeEvent = event;
        OnJsCallbackAppVolumeChange(cb);
    }
}
// LCOV_EXCL_STOP

void NapiAudioManagerAppVolumeChangeCallback::OnSelfAppVolumeChanged(const VolumeEvent &event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_DEBUG_LOG("enter");
    for (auto iter : selfAppVolumeChangeList_) {
        std::unique_ptr<AudioManagerAppVolumeChangeJsCallback> cb =
            std::make_unique<AudioManagerAppVolumeChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = iter;
        cb->callbackName = APP_VOLUME_CHANGE_CALLBACK_NAME;
        cb->appVolumeChangeEvent = event;
        OnJsCallbackAppVolumeChange(cb);
    }
}

void NapiAudioManagerAppVolumeChangeCallback::SafeJsCallbackAppVolumeChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioManagerAppVolumeChangeJsCallback *event = reinterpret_cast<AudioManagerAppVolumeChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "SafeJsCallbackAppVolumeChangeWork: no memory");
    std::shared_ptr<AudioManagerAppVolumeChangeJsCallback> safeContext(
        static_cast<AudioManagerAppVolumeChangeJsCallback*>(data),
        [](AudioManagerAppVolumeChangeJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackAppVolumeChangeWork: safe js callback working.");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueVolumeEvent(env, event->appVolumeChangeEvent, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[0] != nullptr,
            "%{public}s fail to create volume change callback", request.c_str());
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call DeviceChange callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioManagerAppVolumeChangeCallback::AppVolumeChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("AppVolumeChangeTsfnFinalize: safe thread resource release.");
}

void NapiAudioManagerAppVolumeChangeCallback::OnJsCallbackAppVolumeChange(
    std::unique_ptr<AudioManagerAppVolumeChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackAppVolumeChange: jsCb.get() is null");
        return;
    }
    AudioManagerAppVolumeChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");

    napi_acquire_threadsafe_function(amAppVolumeChgTsfn_);
    napi_call_threadsafe_function(amAppVolumeChgTsfn_, event, napi_tsfn_blocking);
}

void NapiAudioManagerAppVolumeChangeCallback::RemoveAllAudioVolumeChangeForUidCbRef()
{
    std::lock_guard<std::mutex> lock(mutex_);
    appVolumeChangeForUidList_.clear();
    AUDIO_INFO_LOG("RemoveAllAudioVolumeChangeForUidCbRef: remove callback finish");
}

void NapiAudioManagerAppVolumeChangeCallback::RemoveAudioVolumeChangeForUidCbRef(napi_env env,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter = appVolumeChangeForUidList_.begin(); iter != appVolumeChangeForUidList_.end();) {
        if (IsSameCallback(env, callback, iter->first->cb_)) {
            AUDIO_INFO_LOG("RemoveAudioVolumeChangeForUidCbRef: find js callback, erase it");
            appVolumeChangeForUidList_.erase(iter++);
        } else {
            iter++;
        }
    }
    AUDIO_INFO_LOG("RemoveAudioVolumeChangeForUidCbRef: remove callback finish");
}

void NapiAudioManagerAppVolumeChangeCallback::RemoveSelfAudioVolumeChangeCbRef(napi_env env,
    napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto iter = selfAppVolumeChangeList_.begin(); iter != selfAppVolumeChangeList_.end();) {
        if (IsSameCallback(env, callback, (*iter)->cb_)) {
            AUDIO_INFO_LOG("RemoveSelfAudioVolumeChangeCbRef: find js callback, erase it");
            selfAppVolumeChangeList_.erase(iter++);
        } else {
            iter++;
        }
    }
    AUDIO_INFO_LOG("RemoveSelfAudioVolumeChangeCbRef: remove callback finish");
}

void NapiAudioManagerAppVolumeChangeCallback::RemoveAllSelfAudioVolumeChangeCbRef()
{
    std::lock_guard<std::mutex> lock(mutex_);
    selfAppVolumeChangeList_.clear();
    AUDIO_INFO_LOG("RemoveAllSelfAudioVolumeChangeCbRef: remove callback finish");
}

int32_t NapiAudioManagerAppVolumeChangeCallback::GetAppVolumeChangeForUidListSize()
{
    return appVolumeChangeForUidList_.size();
}

int32_t NapiAudioManagerAppVolumeChangeCallback::GetSelfAppVolumeChangeListSize()
{
    return selfAppVolumeChangeList_.size();
}

}
}