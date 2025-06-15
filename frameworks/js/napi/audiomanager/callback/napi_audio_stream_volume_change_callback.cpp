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
    CHECK_AND_RETURN_LOG(audioStreamVolumeChangeCallback_ != nullptr,
        "NapiAudioStreamVolumeChangeCallback:No JS callback registered return");
    std::unique_ptr<AudioStreamVolumeChangeJsCallback> cb = std::make_unique<AudioStreamVolumeChangeJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioStreamVolumeChangeCallback_;
    cb->callbackName = AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME;
    cb->volumeEvent.streamUsage = volumeEvent.streamUsage;
    cb->volumeEvent.volume = volumeEvent.volume;
    cb->volumeEvent.updateUi = volumeEvent.updateUi;

    return OnJsCallbackStreamVolumeChange(cb);
}

void NapiAudioStreamVolumeChangeCallback::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioStreamVolumeChangeCallback: creating reference for callback fail");
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME) {
        audioStreamVolumeChangeCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("NapiAudioStreamVolumeChangeCallback: Unknown callback type: %{public}s", callbackName.c_str());
    }
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

bool NapiAudioStreamVolumeChangeCallback::ContainSameJsCallback(napi_value args)
{
    bool isEquals = false;
    napi_value copyValue = nullptr;

    napi_get_reference_value(env_, callback_, &copyValue);
    CHECK_AND_RETURN_RET_LOG(args != nullptr, false, "args is nullptr");
    CHECK_AND_RETURN_RET_LOG(napi_strict_equals(env_, copyValue, args, &isEquals) == napi_ok, false,
        "Get napi_strict_equals failed");

    return isEquals;
}
} // namespace AudioStandard
} // namespace OHOS