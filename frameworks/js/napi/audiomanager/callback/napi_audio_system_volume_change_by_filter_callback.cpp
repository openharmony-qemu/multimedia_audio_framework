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
#define LOG_TAG "NapiAudioSystemVolumeChangeByFilterCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_system_volume_change_by_filter_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_manager_log.h"

namespace OHOS {
namespace AudioStandard {
napi_ref NapiAudioSystemVolumeChangeByFilterCallback::sConstructor_ = nullptr;

NapiAudioSystemVolumeChangeByFilterCallback::NapiAudioSystemVolumeChangeByFilterCallback(napi_env env)
    :env_(env)
{
    AUDIO_INFO_LOG("Constructor");
}

NapiAudioSystemVolumeChangeByFilterCallback::~NapiAudioSystemVolumeChangeByFilterCallback()
{
    AUDIO_INFO_LOG("Destructor");
}

void NapiAudioSystemVolumeChangeByFilterCallback::CreateSystemVolumeChangeByFilterTsfn(napi_env env)
{
    regVolumeTsfn_ = true;
    napi_value cbName;
    std::string callbackName = AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_NAME;
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_add_env_cleanup_hook(env, CleanUp, this);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, this,
        SystemVolumeChangeByFilterTsfnFinalize, nullptr, SafeJsCallbackSystemVolumeChangeByFilterWork,
        &systemVolumeChangeByFilterTsfn_);
}

bool NapiAudioSystemVolumeChangeByFilterCallback::GetVolumeTsfnFlag()
{
    return regVolumeTsfn_;
}

napi_threadsafe_function NapiAudioSystemVolumeChangeByFilterCallback::GetTsfn()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return systemVolumeChangeByFilterTsfn_;
}

void NapiAudioSystemVolumeChangeByFilterCallback::OnSystemVolumeChangeByFilter(const VolumeEvent &volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_PRERELEASE_LOGI("OnSystemVolumeChangeByFilter is called volumeType=%{public}d, volumeLevel=%{public}d,"
        "appUid=%{public}d", volumeEvent.volumeType, volumeEvent.volume, volumeEvent.appUid);
    CHECK_AND_RETURN_LOG(!audioSystemVolumeChangeByFilterCbList_.empty(), "No JS callback registered return");

    for (auto &item : audioSystemVolumeChangeByFilterCbList_) {
        std::unique_ptr<AudioSystemVolumeChangeByFilterJsCallback> cb =
            std::make_unique<AudioSystemVolumeChangeByFilterJsCallback>();
        CHECK_AND_CONTINUE_LOG(cb != nullptr, "No memory");
        cb->callback = item;
        cb->callbackName = AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_NAME;
        cb->volumeEvent.volumeType = volumeEvent.volumeType;
        cb->volumeEvent.volume = volumeEvent.volume;
        cb->volumeEvent.updateUi = volumeEvent.updateUi;
        cb->volumeEvent.volumeGroupId = volumeEvent.volumeGroupId;
        cb->volumeEvent.networkId = volumeEvent.networkId;

        OnJsCallbackSystemVolumeChangeByFilter(cb);
    }
}

void NapiAudioSystemVolumeChangeByFilterCallback::SaveCallbackReference(const std::string &callbackName,
    napi_value args, const SystemVolumeFilter &filter)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;
    std::string taskName = "NapiAudioSystemVolumeChangeByFilterCallback::destroy";

    for (auto &item : audioSystemVolumeChangeByFilterCbList_) {
        if (item == nullptr) {
            continue;
        }
        bool isSameCallback = IsSameCallback(env_, args, item->GetRef());
        CHECK_AND_RETURN_LOG(!isSameCallback, "duplicate JS callback registration");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callbackRef != nullptr,
        "creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef, taskName);
    audioSystemVolumeChangeByFilterCbList_.push_back(cb);
    filter_ = filter;
    AUDIO_INFO_LOG("save callback ref success, list size=%{public}zu, uid=%{public}d",
        audioSystemVolumeChangeByFilterCbList_.size(), filter.uid);
}

void NapiAudioSystemVolumeChangeByFilterCallback::SafeJsCallbackSystemVolumeChangeByFilterWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSystemVolumeChangeByFilterJsCallback *event =
        reinterpret_cast<AudioSystemVolumeChangeByFilterJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackSystemVolumeChangeByFilter: no memory");
    std::shared_ptr<AudioSystemVolumeChangeByFilterJsCallback> safeContext(
        static_cast<AudioSystemVolumeChangeByFilterJsCallback*>(event),
        [](AudioSystemVolumeChangeByFilterJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");

    do {
        napi_value jsCallback = nullptr;
        napi_status nstatus = napi_get_reference_value(env, callback, &jsCallback);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueVolumeEvent(env, event->volumeEvent, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create volumeChangeByFilter callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call volumeChangeByFilter callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSystemVolumeChangeByFilterCallback::SystemVolumeChangeByFilterTsfnFinalize(
    napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("SystemVolumeChangeByFilterTsfnFinalize: CleanUp is removed.");
    NapiAudioSystemVolumeChangeByFilterCallback *context =
        reinterpret_cast<NapiAudioSystemVolumeChangeByFilterCallback*>(data);
    napi_remove_env_cleanup_hook(env, NapiAudioSystemVolumeChangeByFilterCallback::CleanUp, context);
    AUDIO_INFO_LOG("SystemVolumeChangeByFilterTsfnFinalize: tsfn released.");
}

void NapiAudioSystemVolumeChangeByFilterCallback::CleanUp(void *data)
{
    NapiAudioSystemVolumeChangeByFilterCallback *context =
        reinterpret_cast<NapiAudioSystemVolumeChangeByFilterCallback*>(data);
    napi_threadsafe_function tsfn = context->GetTsfn();
    std::unique_lock<std::mutex> lock(context->mutex_);
    context->systemVolumeChangeByFilterTsfn_ = nullptr;
    lock.unlock();
    AUDIO_INFO_LOG("CleanUp: safe thread resource release.");
    napi_release_threadsafe_function(tsfn, napi_tsfn_abort);
}

void NapiAudioSystemVolumeChangeByFilterCallback::OnJsCallbackSystemVolumeChangeByFilter(
    std::unique_ptr<AudioSystemVolumeChangeByFilterJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackSystemVolumeChangeByFilter: jsCb.get() is null");
        return;
    }

    AudioSystemVolumeChangeByFilterJsCallback *event = jsCb.release();
    if (event == nullptr || event->callback == nullptr) {
        delete event;
        AUDIO_ERR_LOG("event is nullptr.");
        return;
    }
    if (systemVolumeChangeByFilterTsfn_ == nullptr) {
        AUDIO_INFO_LOG("OnJsCallbackSystemVolumeChangeByFilter: tsfn nullptr.");
        return;
    }
    napi_acquire_threadsafe_function(systemVolumeChangeByFilterTsfn_);
    napi_call_threadsafe_function(systemVolumeChangeByFilterTsfn_, event, napi_tsfn_blocking);
}

void NapiAudioSystemVolumeChangeByFilterCallback::RemoveCallbackReference(napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioSystemVolumeChangeByFilterCbList_.begin();
         it != audioSystemVolumeChangeByFilterCbList_.end(); ++it) {
        if (*it == nullptr) {
            continue;
        }
        bool isSameCallback = IsSameCallback(env, args, (*it)->cb_);
        if (isSameCallback) {
            napi_status delStatus = napi_delete_reference(env, (*it)->cb_);
            if (delStatus == napi_ok) {
                (*it)->cb_ = nullptr;
            } else {
                AUDIO_ERR_LOG("failed to delete napi reference for callback");
            }
            audioSystemVolumeChangeByFilterCbList_.erase(it);
            AUDIO_INFO_LOG("remove js callback success, list size=%{public}zu",
                audioSystemVolumeChangeByFilterCbList_.size());
            return;
        }
    }
    AUDIO_WARNING_LOG("no matching callback found to remove");
}

void NapiAudioSystemVolumeChangeByFilterCallback::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioSystemVolumeChangeByFilterCbList_) {
        if (item == nullptr) {
            continue;
        }
        napi_status delStatus = napi_delete_reference(env_, item->cb_);
        if (delStatus == napi_ok) {
            item->cb_ = nullptr;
        } else {
            AUDIO_ERR_LOG("failed to delete napi reference for callback");
        }
    }
    audioSystemVolumeChangeByFilterCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

bool NapiAudioSystemVolumeChangeByFilterCallback::IsSameCallback(
    napi_env env, napi_value callback, napi_ref refCallback)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env, refCallback, &copyValue);
    if (napi_strict_equals(env, copyValue, callback, &isEquals) != napi_ok) {
        AUDIO_ERR_LOG("Get napi_strict_equals failed");
        return false;
    }

    return isEquals;
}

bool NapiAudioSystemVolumeChangeByFilterCallback::ContainSameJsCallback(napi_value args)
{
    CHECK_AND_RETURN_RET_LOG(args != nullptr, false, "args is nullptr");
    
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioSystemVolumeChangeByFilterCbList_) {
        if (item == nullptr) {
            continue;
        }
        napi_ref ref = item->GetRef();
        bool isEquals = IsSameCallback(env_, args, ref);
        if (isEquals) {
            return true;
        }
    }
    return false;
}

int32_t NapiAudioSystemVolumeChangeByFilterCallback::GetSystemVolumeByFilterCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(audioSystemVolumeChangeByFilterCbList_.size());
}
} // namespace AudioStandard
} // namespace OHOS