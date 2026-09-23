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
#define LOG_TAG "NapiAudioVolumeLimitExceededCallback"
#endif

#include "napi_audio_volume_limit_exceeded_callback.h"

#include "audio_manager_log.h"
#include "napi_audio_enum.h"
#include "napi_param_utils.h"

namespace OHOS {
namespace AudioStandard {

NapiAudioVolumeLimitExceededCallback::NapiAudioVolumeLimitExceededCallback(napi_env env) : env_(env)
{
    AUDIO_INFO_LOG("Constructor");
}

NapiAudioVolumeLimitExceededCallback::~NapiAudioVolumeLimitExceededCallback()
{
    AUDIO_INFO_LOG("Destructor");
    if (amVolLimitExceededTsfn_ != nullptr) {
        napi_release_threadsafe_function(amVolLimitExceededTsfn_, napi_tsfn_abort);
    }
}

void NapiAudioVolumeLimitExceededCallback::OnVolumeLimitExceeded(const VolumeLimitExceedEvent &event)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto &item : audioVolumeLimitExceededCbList_) {
        auto cb = std::make_unique<AudioVolumeLimitExceededJsCallback>();
        cb->callback = item;
        cb->volumeLimitExceedEvent = event;
        OnJsCallbackVolumeLimitExceeded(cb);
    }
}

napi_status NapiAudioVolumeLimitExceededCallback::SaveCallbackReference(
    const std::string &callbackName, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;
    std::string taskName = "NapiAudioVolumeLimitExceededCallback::destroy";

    CHECK_AND_RETURN_RET_LOG(callbackName == VOLUME_LIMIT_EXCEEDED_CALLBACK_NAME, napi_invalid_arg,
        "Unknown callback type: %{public}s", callbackName.c_str());
    for (const auto &item : audioVolumeLimitExceededCbList_) {
        if (item == nullptr) {
            continue;
        }
        bool isSameCallback = IsSameCallback(env_, callback, item->cb_);
        CHECK_AND_RETURN_RET_LOG(!isSameCallback, napi_ok, "has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "creating reference for callback fail");
    CHECK_AND_RETURN_RET_LOG(callbackRef != nullptr, napi_generic_failure, "callback reference is nullptr");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef, taskName);
    audioVolumeLimitExceededCbList_.push_back(cb);
    AUDIO_INFO_LOG("save callback ref success, list size [%{public}zu]", audioVolumeLimitExceededCbList_.size());
    return napi_ok;
}

napi_status NapiAudioVolumeLimitExceededCallback::RemoveCallbackReference(napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioVolumeLimitExceededCbList_.begin();
        it != audioVolumeLimitExceededCbList_.end(); ++it) {
        std::shared_ptr<AutoRef> temp = *it;
        if (temp == nullptr) {
            continue;
        }
        if (IsSameCallback(env_, callback, temp->cb_)) {
            AUDIO_INFO_LOG("find volume limit exceeded callback, remove it");
            napi_status status = napi_delete_reference(env_, temp->cb_);
            CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "delete callback reference failed");
            temp->cb_ = nullptr;
            audioVolumeLimitExceededCbList_.erase(it);
            return napi_ok;
        }
    }
    AUDIO_INFO_LOG("remove nothing");
    return napi_ok;
}

void NapiAudioVolumeLimitExceededCallback::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioVolumeLimitExceededCbList_) {
        if (item == nullptr) {
            continue;
        }
        napi_delete_reference(env_, item->cb_);
        item->cb_ = nullptr;
    }
    audioVolumeLimitExceededCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

int32_t NapiAudioVolumeLimitExceededCallback::GetVolumeLimitExceededCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(audioVolumeLimitExceededCbList_.size());
}

bool NapiAudioVolumeLimitExceededCallback::GetVolumeLimitExceededTsfnFlag() const
{
    return regVolumeLimitExceededTsfn_;
}

napi_status NapiAudioVolumeLimitExceededCallback::CreateVolumeLimitExceededTsfn()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (regVolumeLimitExceededTsfn_) {
        return napi_ok;
    }
    napi_value callbackName = nullptr;
    napi_status status = napi_create_string_utf8(env_, VOLUME_LIMIT_EXCEEDED_CALLBACK_NAME.c_str(),
        VOLUME_LIMIT_EXCEEDED_CALLBACK_NAME.length(), &callbackName);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "create callback name failed");
    napi_threadsafe_function tsfn = nullptr;
    status = napi_create_threadsafe_function(env_, nullptr, nullptr, callbackName, 0, 1, nullptr,
        nullptr, nullptr, SafeJsCallbackVolumeLimitExceededWork, &tsfn);
    CHECK_AND_RETURN_RET_LOG(status == napi_ok, status, "create threadsafe function failed");
    CHECK_AND_RETURN_RET_LOG(tsfn != nullptr, napi_generic_failure, "threadsafe function is nullptr");
    amVolLimitExceededTsfn_ = tsfn;
    regVolumeLimitExceededTsfn_ = true;
    return napi_ok;
}

bool NapiAudioVolumeLimitExceededCallback::IsSameCallback(
    napi_env env, napi_value callback, napi_ref callbackRef)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;
    napi_status status = napi_get_reference_value(env, callbackRef, &copyValue);
    if (status != napi_ok || napi_strict_equals(env, copyValue, callback, &isEquals) != napi_ok) {
        AUDIO_ERR_LOG("get napi_strict_equals failed");
        return false;
    }
    return isEquals;
}

void NapiAudioVolumeLimitExceededCallback::SafeJsCallbackVolumeLimitExceededWork(
    napi_env env, napi_value jsCallback, void *context, void *data)
{
    std::unique_ptr<AudioVolumeLimitExceededJsCallback> event(
        reinterpret_cast<AudioVolumeLimitExceededJsCallback *>(data));
    if (env == nullptr) {
        return;
    }
    CHECK_AND_RETURN_LOG(event != nullptr && event->callback != nullptr, "callback event is nullptr");
    if (event->callback->cb_ == nullptr) {
        return;
    }

    napi_handle_scope scope = nullptr;
    napi_status status = napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(status == napi_ok && scope != nullptr, "open handle scope failed");
    do {
        napi_value callback = nullptr;
        status = napi_get_reference_value(env, event->callback->cb_, &callback);
        CHECK_AND_BREAK_LOG(status == napi_ok && callback != nullptr,
            "get reference value fail");
        napi_value args[ARGS_ONE] = {nullptr};
        status = napi_create_object(env, &args[PARAM0]);
        CHECK_AND_BREAK_LOG(status == napi_ok && args[PARAM0] != nullptr, "create event failed");
        const VolumeLimitExceedEvent &nativeEvent = event->volumeLimitExceedEvent;
        status = NapiParamUtils::SetValueInt32(env, "volumeType",
            NapiAudioEnum::GetJsAudioVolumeType(nativeEvent.volumeType), args[PARAM0]);
        CHECK_AND_BREAK_LOG(status == napi_ok, "convert volumeType failed");
        status = NapiParamUtils::SetValueInt32(env, "currentVolume", nativeEvent.currentVolume, args[PARAM0]);
        CHECK_AND_BREAK_LOG(status == napi_ok, "convert currentVolume failed");
        status = NapiParamUtils::SetValueInt32(env, "volumeThreshold", nativeEvent.volumeThreshold, args[PARAM0]);
        CHECK_AND_BREAK_LOG(status == napi_ok, "convert volumeThreshold failed");
        status = NapiParamUtils::SetValueInt32(env, "uid", nativeEvent.uid, args[PARAM0]);
        CHECK_AND_BREAK_LOG(status == napi_ok, "convert uid failed");
        napi_value result = nullptr;
        status = napi_call_function(env, nullptr, callback, ARGS_ONE, args, &result);
        CHECK_AND_BREAK_LOG(status == napi_ok, "fail to call callback");
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioVolumeLimitExceededCallback::OnJsCallbackVolumeLimitExceeded(
    std::unique_ptr<AudioVolumeLimitExceededJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackVolumeLimitExceeded: jsCb.get() is null");
        return;
    }

    AudioVolumeLimitExceededJsCallback *event = jsCb.get();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    if (amVolLimitExceededTsfn_ == nullptr) {
        AUDIO_INFO_LOG("OnJsCallbackVolumeLimitExceeded: tsfn nullptr.");
        return;
    }
    napi_acquire_threadsafe_function(amVolLimitExceededTsfn_);
    jsCb.release();
    napi_call_threadsafe_function(amVolLimitExceededTsfn_, event, napi_tsfn_blocking);
    napi_release_threadsafe_function(amVolLimitExceededTsfn_, napi_tsfn_release);
}

} // namespace AudioStandard
} // namespace OHOS
