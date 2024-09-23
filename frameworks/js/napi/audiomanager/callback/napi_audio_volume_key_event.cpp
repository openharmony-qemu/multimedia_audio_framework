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
    AUDIO_DEBUG_LOG("NapiAudioVolumeKeyEvent::Constructor");
}

NapiAudioVolumeKeyEvent::~NapiAudioVolumeKeyEvent()
{
    AUDIO_DEBUG_LOG("NapiAudioVolumeKeyEvent::Destructor");
}

void NapiAudioVolumeKeyEvent::OnVolumeKeyEvent(VolumeEvent volumeEvent)
{
    std::lock_guard<std::mutex> lock(mutex_);
    AUDIO_INFO_LOG("OnVolumeKeyEvent is called volumeType=%{public}d, volumeLevel=%{public}d, isUpdateUi=%{public}d",
        volumeEvent.volumeType, volumeEvent.volume, volumeEvent.updateUi);
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

void NapiAudioVolumeKeyEvent::OnJsCallbackVolumeEvent(std::unique_ptr<AudioVolumeKeyEventJsCallback> &jsCb)
{
    if (jsCb.get() == nullptr) {
        AUDIO_ERR_LOG("OnJsCallbackVolumeEvent: jsCb.get() is null");
        return;
    }

    AudioVolumeKeyEventJsCallback *event = jsCb.get();
    auto task = [event]() {
        std::shared_ptr<AudioVolumeKeyEventJsCallback> context(
            static_cast<AudioVolumeKeyEventJsCallback*>(event),
            [](AudioVolumeKeyEventJsCallback* ptr) {
                delete ptr;
        });
        CHECK_AND_RETURN_LOG(event != nullptr, "event is nullptr");
        std::string request = event->callbackName;
        CHECK_AND_RETURN_LOG(event->callback != nullptr, "event is nullptr");
        napi_env env = event->callback->env_;
        napi_ref callback = event->callback->cb_;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        CHECK_AND_RETURN_LOG(scope != nullptr, "scope is nullptr");
        AUDIO_INFO_LOG("JsCallBack %{public}s, doWork", request.c_str());
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
    };
    if (napi_status::napi_ok != napi_send_event(env_, task, napi_eprio_immediate)) {
        AUDIO_ERR_LOG("OnJsCallbackVolumeEvent: Failed to SendEvent");
    } else {
        jsCb.release();
    }
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