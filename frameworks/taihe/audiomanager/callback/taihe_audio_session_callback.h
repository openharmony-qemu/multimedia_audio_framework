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
#ifndef TAIHE_AUDIO_SESSION_CALLBACK_H
#define TAIHE_AUDIO_SESSION_CALLBACK_H

#include "event_handler.h"
#include "audio_session_manager.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;

const std::string AUDIO_SESSION_CALLBACK_NAME = "audioSessionDeactivated";

class TaiheAudioSessionCallback : public OHOS::AudioStandard::AudioSessionCallback,
    public std::enable_shared_from_this<TaiheAudioSessionCallback> {
public:
    explicit TaiheAudioSessionCallback();
    virtual ~TaiheAudioSessionCallback();

    void OnAudioSessionDeactive(const OHOS::AudioStandard::AudioSessionDeactiveEvent &deactiveEvent);
    void SaveCallbackReference(std::shared_ptr<uintptr_t> callback);

private:
    struct AudioSessionJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::AudioSessionDeactiveEvent audioSessionDeactiveEvent;
    };

    void OnJsCallbackAudioSession(std::unique_ptr<AudioSessionJsCallback> &jsCb);
    static void SafeJsCallbackAudioSessionWork(AudioSessionJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> audioSessionJsCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_INTERRUPT_MANAGER_H