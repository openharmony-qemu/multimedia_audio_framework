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
#define LOG_TAG "NapiAudioVolumeKeyEvent"
#endif

#include "js_native_api.h"
#include "napi_audio_volume_key_event.h"
#include "napi_audio_enum.h"
#include "napi_audio_error.h"
#include "napi_param_utils.h"
#include "audio_manager_log.h"

using namespace std;
namespace OHOS {
namespace AudioStandard {
NapiAudioVolumeKeyEvent::NapiAudioVolumeKeyEvent(napi_env env)
    :env_(env)
{
    AUDIO_INFO_LOG("Constructor");
}

NapiAudioVolumeKeyEvent::~NapiAudioVolumeKeyEvent()
{
    AUDIO_INFO_LOG("Destructor");
    napi_remove_env_cleanup_hook(env_, NapiAudioVolumeKeyEvent::Cleanup, this);
}

void NapiAudioVolumeKeyEvent::CreateVolumeTsfn(napi_env env)
{
    regVolumeTsfn_ = true;
    napi_value cbName;
    std::string callbackName = "volumeChange";
    napi_create_string_utf8(env, callbackName.c_str(), callbackName.length(), &cbName);
    napi_add_env_cleanup_hook(env, Cleanup, this);
    napi_create_threadsafe_function(env, nullptr, nullptr, cbName, 0, 1, this,
        nullptr, nullptr, SafeJsCallbackVolumeEventWork, &amVolEntTsfn_);
}

bool NapiAudioVolumeKeyEvent::GetVolumeTsfnFlag()
{
    return regVolumeTsfn_;
}

napi_threadsafe_function NapiAudioVolumeKeyEvent::GetTsfn()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return amVolEntTsfn_;
}

void NapiAudioVolumeKeyEvent::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_PRERELEASE_LOGI("OnVolumeKeyEvent is called volumeType=%{public}d, volumeLevel=%{public}d,"
        "isUpdateUi=%{public}d", volumeEvent.volumeType, volumeEvent.volume, volumeEvent.updateUi);
    CHECK_AND_RETURN_LOG(audioVolumeKeyEventJsCallback_ != nullptr,
        "NapiAudioVolumeKeyEvent:No JS callback registered return");
    std::unique_ptr<AudioVolumeKeyEventJsCallback> cb = std::make_unique<AudioVolumeKeyEventJsCallback>();
    CHECK_AND_RETURN_LOG(cb != nullptr, "No memory");
    cb->callback = audioVolumeKeyEventJsCallback_;
    cb->callbackName = VOLUME_KEY_EVENT_CALLBACK_NAME;
    cb->volumeEvent.volumeType = volumeEvent.volumeType;
    cb->volumeEvent.volume = volumeEvent.volume;
    cb->volumeEvent.updateUi = volumeEvent.updateUi;
    cb->volumeEvent.volumeGroupId = volumeEvent.volumeGroupId;
    cb->volumeEvent.networkId = volumeEvent.networkId;

    return OnJsCallbackVolumeEvent(cb);
}

void NapiAudioVolumeKeyEvent::SaveCallbackReference(const std::string &callbackName, napi_value args)
{
    std::lock_guard<std::mutex> lock(mutex_);
    napi_ref callback = nullptr;
    const int32_t refCount = 1;
    napi_status status = napi_create_reference(env_, args, refCount, &callback);
    CHECK_AND_RETURN_LOG(status == napi_ok && callback != nullptr,
        "NapiAudioVolumeKeyEvent: creating reference for callback fail");
    callback_ = callback;
    std::shared_ptr<AutoRef> cb = std::make_shared<AutoRef>(env_, callback);
    if (callbackName == VOLUME_KEY_EVENT_CALLBACK_NAME) {
        audioVolumeKeyEventJsCallback_ = cb;
    } else {
        AUDIO_ERR_LOG("NapiAudioVolumeKeyEvent: Unknown callback type: %{public}s", callbackName.c_str());
    }
}

void NapiAudioVolumeKeyEvent::SafeJsCallbackVolumeEventWork(napi_env env, napi_value js_cb, void *context, void *data)
{
    AudioVolumeKeyEventJsCallback *event = reinterpret_cast<AudioVolumeKeyEventJsCallback *>(data);
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr),
        "OnJsCallbackVolumeEvent: no memory");
    std::shared_ptr<AudioVolumeKeyEventJsCallback> safeContext(
        static_cast<AudioVolumeKeyEventJsCallback*>(event),
        [](AudioVolumeKeyEventJsCallback *ptr) {
            delete ptr;
    });
    std::string request = event->callbackName;
    napi_ref callback = event->callback->cb_;
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
    AUDIO_INFO_LOG("SafeJsCallbackVolumeEventWork: safe js callback working.");

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

void NapiAudioVolumeKeyEvent::Cleanup(void *data)
{
    NapiAudioVolumeKeyEvent *context = reinterpret_cast<NapiAudioVolumeKeyEvent*>(data);
    napi_threadsafe_function tsfn = context->GetTsfn();
    std::unique_lock<std::mutex> lock(context->mutex_);
    context->amVolEntTsfn_ = nullptr;
    lock.unlock();
    AUDIO_INFO_LOG("Cleanup: safe thread resource release.");
    napi_release_threadsafe_function(tsfn, napi_tsfn_abort);
}

void NapiAudioVolumeKeyEvent::OnJsCallbackVolumeEvent(std::unique_ptr<AudioVolumeKeyEventJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackVolumeEvent: jsCb.get() is null");
        return;
    }

    AudioVolumeKeyEventJsCallback *event = jsCb.release();
    CHECK_AND_RETURN_LOG((event != nullptr) && (event->callback != nullptr), "event is nullptr.");
    if (amVolEntTsfn_ == nullptr) {
        AUDIO_INFO_LOG("OnJsCallbackVolumeEvent: tsfn nullptr.");
        return;
    }
    napi_acquire_threadsafe_function(amVolEntTsfn_);
    napi_call_threadsafe_function(amVolEntTsfn_, event, napi_tsfn_blocking);
}

bool NapiAudioVolumeKeyEvent::ContainSameJsCallback(napi_value args)
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
