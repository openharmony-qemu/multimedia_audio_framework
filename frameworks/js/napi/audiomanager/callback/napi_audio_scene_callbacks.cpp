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
#define LOG_TAG "NapiAudioSceneChangedCallback"
#endif

#include "js_native_api.h"
#include "napi_audio_scene_callbacks.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "napi_param_utils.h"
#include "napi_audio_error.h"
#include "napi_audio_enum.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AudioStandard {
bool NapiAudioSceneChangedCallback::IsSameCallback(napi_env env, napi_value callback, napi_ref refCallback)
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

NapiAudioSceneChangedCallback::NapiAudioSceneChangedCallback(napi_env env)
    : env_(env)
{
    AUDIO_DEBUG_LOG("instance create");
}

NapiAudioSceneChangedCallback::~NapiAudioSceneChangedCallback()
{
    if (regAmSceneChgTsfn_) {
        napi_release_threadsafe_function(amSceneChgTsfn_, napi_tsfn_abort);
    }
    AUDIO_DEBUG_LOG("instance destroy");
}

void NapiAudioSceneChangedCallback::SaveCallbackReference(const std::string &callbackName, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callbackRef = nullptr;
    const int32_t refCount = ARGS_ONE;

    CHECK_AND_RETURN_LOG(callbackName == AUDIO_SCENE_CHANGE_CALLBACK_NAME,
        "Unknown callback type: %{public}s", callbackName.c_str());
    for (auto &item : audioSceneChangeCbList_) {
        if (item == nullptr) {
            continue;
        }
        bool isSameCallback = IsSameCallback(env_, callback, item->cb_);
        CHECK_AND_RETURN_LOG(!isSameCallback, "has same callback, nothing to do");
    }

    napi_status status = napi_create_reference(env_, callback, refCount, &callbackRef);
    CHECK_AND_RETURN_LOG(status == napi_ok && callbackRef != nullptr, "creating reference for callback fail");
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callbackRef);
    audioSceneChangeCbList_.push_back(cb);
    AUDIO_INFO_LOG("save callback ref success, list size [%{public}zu]", audioSceneChangeCbList_.size());
}

void NapiAudioSceneChangedCallback::RemoveCallbackReference(napi_env env, napi_value callback)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto it = audioSceneChangeCbList_.begin(); it != audioSceneChangeCbList_.end(); ++it) {
        std::shared_ptr<AutoRef> temp = (*it);
        if (temp == nullptr) {
            continue;
        }
        bool isSameCallback = IsSameCallback(env_, callback, temp->cb_);
        if (isSameCallback) {
            AUDIO_INFO_LOG("find audioSceneChanged callback, remove it");
            napi_delete_reference(env_, temp->cb_);
            temp->cb_ = nullptr;
            audioSceneChangeCbList_.erase(it);
            return;
        }
    }
    AUDIO_INFO_LOG("remove audioSceneChanged callback no find");
}

void NapiAudioSceneChangedCallback::RemoveAllCallbackReference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &item : audioSceneChangeCbList_) {
        if (item == nullptr) {
            continue;
        }
        napi_delete_reference(env_, item->cb_);
        item->cb_ = nullptr;
    }
    audioSceneChangeCbList_.clear();
    AUDIO_INFO_LOG("remove all js callback success");
}

int32_t NapiAudioSceneChangedCallback::GetAudioSceneCbListSize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return static_cast<int32_t>(audioSceneChangeCbList_.size());
}

void NapiAudioSceneChangedCallback::OnAudioSceneChange(const AudioScene audioScene)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("audioScene status [%{public}d]", audioScene);

    for (auto &item : audioSceneChangeCbList_) {
        std::unique_ptr<AudioSceneJsCallback> cb = std::make_unique<AudioSceneJsCallback>();
        CHECK_AND_RETURN_LOG(cb != nullptr, "no memory");
        cb->callback = item;
        cb->callbackName = AUDIO_SCENE_CHANGE_CALLBACK_NAME;
        cb->audioScene = NapiAudioEnum::GetJsAudioScene(audioScene);
        OnJsCallbackAudioSceneChange(cb);
    }
}

void NapiAudioSceneChangedCallback::SafeJsCallbackAudioSceneChangeWork(napi_env env, napi_value js_cb,
    void *context, void *data)
{
    AudioSceneJsCallback *event = reinterpret_cast<AudioSceneJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event or event->callback is nullptr");
    std::shared_ptr<AudioSceneJsCallback> safeContext(
        static_cast<AudioSceneJsCallback*>(data),
        [](AudioSceneJsCallback *ptr) {
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
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && jsCallback != nullptr, "%{public}s get reference value fail",
            request.c_str());
        napi_value args[ARGS_ONE] = { nullptr };
        NapiParamUtils::SetValueInt32(env, event->audioScene, args[PARAM0]);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok && args[PARAM0] != nullptr,
            "%{public}s fail to create audioScene callback", request.c_str());
        const size_t argCount = ARGS_ONE;
        napi_value result = nullptr;
        nstatus = napi_call_function(env, nullptr, jsCallback, argCount, args, &result);
        CHECK_AND_BREAK_LOG(nstatus == napi_ok, "%{public}s fail to call audioScene callback", request.c_str());
    } while (0);
    napi_close_handle_scope(env, scope);
}

void NapiAudioSceneChangedCallback::AudioSceneChangeTsfnFinalize(napi_env env, void *data, void *hint)
{
    AUDIO_INFO_LOG("safe thread resource release.");
}

void NapiAudioSceneChangedCallback::OnJsCallbackAudioSceneChange(std::unique_ptr<AudioSceneJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("jsCb.get() is null");
        return;
    }

    AudioSceneJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event or event->callback is nullptr");

    napi_acquire_threadsafe_function(amSceneChgTsfn_);
    napi_call_threadsafe_function(amSceneChgTsfn_, event, napi_tsfn_blocking);
}

void NapiAudioSceneChangedCallback::CreateSceneChgTsfn(napi_env env)
{
    if (regAmSceneChgTsfn_) {
        AUDIO_INFO_LOG("amSceneChgTsfn_ has been created");
        return;
    }
    regAmSceneChgTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "AudioSceneChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, nullptr,
        AudioSceneChangeTsfnFinalize, nullptr, SafeJsCallbackAudioSceneChangeWork, &amSceneChgTsfn_);
}

bool NapiAudioSceneChangedCallback::GetSceneChgTsfnFlag() const
{
    return regAmSceneChgTsfn_;
}
} // namespace AudioStandard
} // namespace OHOS