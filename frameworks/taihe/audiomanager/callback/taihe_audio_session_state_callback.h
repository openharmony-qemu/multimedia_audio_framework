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
#ifndef TAIHE_AUDIO_SESSION_STATE_CALLBACK_H
#define TAIHE_AUDIO_SESSION_STATE_CALLBACK_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "event_handler.h"
#include "audio_session_manager.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string AUDIO_SESSION_STATE_CALLBACK_NAME = "audioSessionStateChanged";

class TaiheAudioSessionStateCallback : public OHOS::AudioStandard::AudioSessionStateChangedCallback,
    public std::enable_shared_from_this<TaiheAudioSessionStateCallback> {
public:
    explicit TaiheAudioSessionStateCallback();
    virtual ~TaiheAudioSessionStateCallback();
    
    void OnAudioSessionStateChanged(const OHOS::AudioStandard::AudioSessionStateChangedEvent &stateEvent);
    void SaveCallbackReference(std::shared_ptr<uintptr_t> &callback);
    bool ContainSameJsCallback(std::shared_ptr<uintptr_t> callback);

private:
    struct AudioSessionStateJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::AudioSessionStateChangedEvent audioSessionStateEvent;
    };

    void OnJsCallbackAudioSessionState(std::unique_ptr<AudioSessionStateJsCallback> &jsCb);
    static void SafeJsCallbackAudioSessionStateWork(AudioSessionStateJsCallback *event);

    std::shared_ptr<AutoRef> audioSessionStateJsCallback_ = nullptr;
    std::shared_ptr<uintptr_t> callback_ = nullptr;
    std::mutex mutex_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_INTERRUPT_MANAGER_H