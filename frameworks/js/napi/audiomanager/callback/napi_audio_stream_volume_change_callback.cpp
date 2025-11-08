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
#define LOG_TAG "NapiAudioStreamVolumeChangeCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_stream_volume_change_callback.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_manager_log.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
NapiAudioStreamVolumeChangeCallback::NapiAudioStreamVolumeChangeCallback(napi_env env)
    :env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiAudioStreamVolumeChangeCallback::~NapiAudioStreamVolumeChangeCallback()
{
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiAudioStreamVolumeChangeCallback::CreateStreamVolumeChangeTsfn(napi_env env)
{
    regVolumeTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "streamVolumeChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_add_env_cleanup_hook(env, CleanUp, this);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, this,
        StreamVolumeChangeTsfnFinalize, nullptr, SafeJsCallbackStreamVolumeChangeWork, &amVolEntTsfn_);
}

bool NapiAudioStreamVolumeChangeCallback::GetVolumeTsfnFlag()
{
    return regVolumeTsfn_;
}

napi_threadsafe_function NapiAudioStreamVolumeChangeCallback::GetTsfn()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return amVolEntTsfn_;
}

void NapiAudioStreamVolumeChangeCallback::OnStreamVolumeChange(StreamVolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_PRERELEASE_LOGI("OnStreamVolumeChange is called streamUsage=%{public}d, volumeLevel=%{public}d,"
        "isUpdateUi=%{public}d", volumeEvent.streamUsage, volumeEvent.volume, volumeEvent.updateUi);
    CHECK_AND_RETURN_LOG(!audioStreamVolumeChangeCbList_.empty(), "no JS callback registered return");
    for (auto &item : audioStreamVolumeChangeCbList_) {
        std::unique_ptr<AudioStreamVolumeChangeJsCallback> cb = std::make_unique<AudioStreamVolumeChangeJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
        cb->callback = item;
        cb->callbackName = AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME;
        cb->volumeEvent.streamUsage = volumeEvent.streamUsage;
        cb->volumeEvent.volume = volumeEvent.volume;
        cb->volumeEvent.updateUi = volumeEvent.updateUi;
        OnJsCallbackStreamVolumeChange(cb);
    }
}

void NapiAudioStreamVolumeChangeCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;

    CHECK_AND_RETURN_LOG(callbackName == AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME,
        "unknown callback type: %{public}s", callbackName.c_str());

    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr, "creating reference for callback fail");

    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    audioStreamVolumeChangeCbList_.push_back(cb);
    AUDIO_INFO_LOG("save callback ref success, list size=%{public}zu", audioStreamVolumeChangeCbList_.size());
}

void NapiAudioStreamVolumeChangeCallback::RemoveCallbackReference(napi_env env, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioStreamVolumeChangeCbList_.begin();
         it != audioStreamVolumeChangeCbList_.end(); ++it) {
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
            audioStreamVolumeChangeCbList_.erase(it);
            AUDIO_INFO_LOG("remove js callback success, list size=%{public}zu",
                audioStreamVolumeChangeCbList_.size());
            return;
        }
    }
    AUDIO_WARNING_LOG("no matching callback found to remove");
}

void NapiAudioStreamVolumeChangeCallback::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioStreamVolumeChangeCbList_) {
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
    audioStreamVolumeChangeCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

void NapiAudioStreamVolumeChangeCallback::SafeJsCallbackStreamVolumeChangeWork(
    napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioStreamVolumeChangeJsCallback *event = reinterpret_cast<AudioStreamVolumeChangeJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackStreamVolumeChange: no memory");
    std::shared_ptr<AudioStreamVolumeChangeJsCallback> safeContext(
        static_cast<AudioStreamVolumeChangeJsCallback*>(event),
        [](AudioStreamVolumeChangeJsCallback *ptr) {
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
        NapiParamUtils::SetValueStreamVolumeEvent(env, event->volumeEvent, args[PARAM0]);
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

void NapiAudioStreamVolumeChangeCallback::StreamVolumeChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("StreamVolumeChangeTsfnFinalize: CleanUp is removed.");
    NapiAudioStreamVolumeChangeCallback *context = reinterpret_cast<NapiAudioStreamVolumeChangeCallback*>(data);
    napi_remove_env_cleanup_hook(env, NapiAudioStreamVolumeChangeCallback::CleanUp, context);
    if (context->GetTsfn() != nullptr) {
        AUDIO_INFO_LOG("StreamVolumeChangeTsfnFinalize: context is released.");
        delete context;
    }
}

void NapiAudioStreamVolumeChangeCallback::CleanUp(void *data)
{
    NapiAudioStreamVolumeChangeCallback *context = reinterpret_cast<NapiAudioStreamVolumeChangeCallback*>(data);
    napi_threadsafe_function tsfn = context->GetTsfn();
    std::unique_lock<std::mutex> lock(context->mutex_);
    context->amVolEntTsfn_ = nullptr;
    lock.unlock();
    AUDIO_INFO_LOG("CleanUp: safe thread resource release.");
    napi_release_threadsafe_function(tsfn, napi_tsfn_abort);
}

void NapiAudioStreamVolumeChangeCallback::OnJsCallbackStreamVolumeChange(
    std::unique_ptr<AudioStreamVolumeChangeJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackStreamVolumeChange: jsCb.get() is null");
        return;
    }

    AudioStreamVolumeChangeJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    if (amVolEntTsfn_ == nullptr) {
        AUDIO_INFO_LOG("OnJsCallbackStreamVolumeChange: tsfn nullptr.");
        return;
    }
    napi_acquire_threadsafe_function(amVolEntTsfn_);
    napi_call_threadsafe_function(amVolEntTsfn_, event, napi_tsfn_blocking);
}

bool NapiAudioStreamVolumeChangeCallback::IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback)
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

bool NapiAudioStreamVolumeChangeCallback::ContainSameJsCallback(napi_value args)
{
    CHECK_AND_RETURN_RET_LOG(args != nullptr, false, "args is nullptr");

    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioStreamVolumeChangeCbList_) {
        napi_ref ref = item->GetRef();
        bool isEquals = IsSameCallback(env_, args, ref);
        if (isEquals) {
            return true;
        }
    }
    return false;
}

int32_t NapiAudioStreamVolumeChangeCallback::GetStreamVolumeCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(audioStreamVolumeChangeCbList_.size());
}
} // namespace AudioStandard
} // namespace OHOS