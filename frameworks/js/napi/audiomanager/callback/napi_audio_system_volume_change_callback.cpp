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
#define LOG_TAG "NapiAudioSystemVolumeChangeCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_system_volume_change_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_manager_log.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
NapiAudioSystemVolumeChangeCallback::NapiAudioSystemVolumeChangeCallback(napi_env env)
    :env_(env)
{
    AUDIO_INFO_LOG("Constructor");
}

NapiAudioSystemVolumeChangeCallback::~NapiAudioSystemVolumeChangeCallback()
{
    AUDIO_INFO_LOG("Destructor");
}

void NapiAudioSystemVolumeChangeCallback::CreateSystemVolumeChangeTsfn(napi_env env)
{
    regVolumeTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "systemVolumeChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_add_env_cleanup_hook(env, CleanUp, this);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, this,
        SystemVolumeChangeTsfnFinalize, nullptr, SafeJsCallbackSystemVolumeChangeWork, &amVolEntTsfn_);
}

bool NapiAudioSystemVolumeChangeCallback::GetVolumeTsfnFlag()
{
    return regVolumeTsfn_;
}

napi_threadsafe_function NapiAudioSystemVolumeChangeCallback::GetTsfn()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return amVolEntTsfn_;
}

void NapiAudioSystemVolumeChangeCallback::OnSystemVolumeChange(VolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_PRERELEASE_LOGI("OnSystemVolumeChange is called volumeType=%{public}d, volumeLevel=%{public}d,"
        "isUpdateUi=%{public}d", volumeEvent.volumeType, volumeEvent.volume, volumeEvent.updateUi);
    CHECK_AND_RETURN_LOG(!audioSystemVolumeChangeCbList_.empty(), "No JS callback registered return");

    for (auto &item : audioSystemVolumeChangeCbList_) {
        std::unique_ptr<AudioSystemVolumeChangeJsCallback> cb = std::make_unique<AudioSystemVolumeChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = item;
        cb->callbackName = AUDIO_SYSTEM_VOLUME_CHANGE_CALLBACK_NAME;
        cb->volumeEvent.volumeType = volumeEvent.volumeType;
        cb->volumeEvent.volume = volumeEvent.volume;
        cb->volumeEvent.updateUi = volumeEvent.updateUi;
        cb->volumeEvent.volumeGroupId = volumeEvent.volumeGroupId;
        cb->volumeEvent.networkId = volumeEvent.networkId;

        OnJsCallbackSystemVolumeChange(cb);
    }
}

void NapiAudioSystemVolumeChangeCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;

    for (auto &item : audioSystemVolumeChangeCbList_) {
        if (item == nullptr) {
            continue;
        }
        bool isSameCallback = IsSameCallback(env_, args, item->GetRef());
        CHECK_AND_RETURN_LOG(!isSameCallback, "duplicate JS callback registration");
    }

    napi_status status = napi_create_reference(env_, args, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callbackRef != nullptr,
        "creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    audioSystemVolumeChangeCbList_.push_back(cb);
    AUDIO_INFO_LOG("save callback ref success, list size=%{public}zu", audioSystemVolumeChangeCbList_.size());
}

void NapiAudioSystemVolumeChangeCallback::SafeJsCallbackSystemVolumeChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioSystemVolumeChangeJsCallback *event = reinterpret_cast<AudioSystemVolumeChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackSystemVolumeChange: no memory");
    std::shared_ptr<AudioSystemVolumeChangeJsCallback> safeContext(
        static_cast<AudioSystemVolumeChangeJsCallback*>(event),
        [](AudioSystemVolumeChangeJsCallback *ptr) {
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
            "%{public}s fail to create volumeChange callback", request.c_str());

        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call volumeChange callback",
            request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSystemVolumeChangeCallback::SystemVolumeChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("SystemVolumeChangeTsfnFinalize: CleanUp is removed.");
    NapiAudioSystemVolumeChangeCallback *context = reinterpret_cast<NapiAudioSystemVolumeChangeCallback*>(data);
    napi_remove_env_cleanup_hook(env, NapiAudioSystemVolumeChangeCallback::CleanUp, context);
    if (context->GetTsfn() != nullptr) {
        AUDIO_INFO_LOG("SystemVolumeChangeTsfnFinalize: context is released.");
        delete context;
    }
}

void NapiAudioSystemVolumeChangeCallback::CleanUp(void *data)
{
    NapiAudioSystemVolumeChangeCallback *context = reinterpret_cast<NapiAudioSystemVolumeChangeCallback*>(data);
    napi_threadsafe_function tsfn = context->GetTsfn();
    std::unique_lock<std::mutex> lock(context->mutex_);
    context->amVolEntTsfn_ = nullptr;
    lock.unlock();
    AUDIO_INFO_LOG("CleanUp: safe thread resource release.");
    napi_release_threadsafe_function(tsfn, napi_tsfn_abort);
}

void NapiAudioSystemVolumeChangeCallback::OnJsCallbackSystemVolumeChange(
    std::unique_ptr<AudioSystemVolumeChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackSystemVolumeChange: jsCb.get() is null");
        return;
    }

    AudioSystemVolumeChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    if (amVolEntTsfn_ == nullptr) {
        AUDIO_INFO_LOG("OnJsCallbackSystemVolumeChange: tsfn nullptr.");
        return;
    }
    napi_acquire_threadsafe_function(amVolEntTsfn_);
    napi_call_threadsafe_function(amVolEntTsfn_, event, napi_tsfn_blocking);
}

void NapiAudioSystemVolumeChangeCallback::RemoveCallbackReference(napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioSystemVolumeChangeCbList_.begin();
         it != audioSystemVolumeChangeCbList_.end(); ++it) {
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
            audioSystemVolumeChangeCbList_.erase(it);
            AUDIO_INFO_LOG("remove js callback success, list size=%{public}zu",
                audioSystemVolumeChangeCbList_.size());
            return;
        }
    }
    AUDIO_WARNING_LOG("no matching callback found to remove");
}

void NapiAudioSystemVolumeChangeCallback::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioSystemVolumeChangeCbList_) {
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
    audioSystemVolumeChangeCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

bool NapiAudioSystemVolumeChangeCallback::IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback)
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

bool NapiAudioSystemVolumeChangeCallback::ContainSameJsCallback(napi_value args)
{
    CHECK_AND_RETURN_RET_LOG(args != nullptr, false, "args is nullptr");
    
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioSystemVolumeChangeCbList_) {
        napi_ref ref = item->GetRef();
        bool isEquals = IsSameCallback(env_, args, ref);
        if (isEquals) {
            return true;
        }
    }
    return false;
}

int32_t NapiAudioSystemVolumeChangeCallback::GetSystemVolumeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(audioSystemVolumeChangeCbList_.size());
}
} // namespace AudioStandard
} // namespace OHOS
