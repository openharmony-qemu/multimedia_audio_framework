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
#ifndef TAIHE_AUDIO_MICSTATECHANGE_CALLBACK_H
#define TAIHE_AUDIO_MICSTATECHANGE_CALLBACK_H

#include <mutex>
#include <thread>
#include "event_handler.h"
#include "audio_system_manager.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_manager_log.h"
#include "taihe_param_utils.h"
#include "taihe_audio_error.h"
#include "taihe_audio_enum.h"
#include "taihe_work.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
const std::string MIC_STATE_CHANGE_CALLBACK_NAME = "micStateChange";

class TaiheAudioManagerMicStateChangeCallback : public OHOS::AudioStandard::AudioManagerMicStateChangeCallback,
public std::enable_shared_from_this<TaiheAudioManagerMicStateChangeCallback> {
public:
    explicit TaiheAudioManagerMicStateChangeCallback();
    virtual ~TaiheAudioManagerMicStateChangeCallback();
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback);
    bool IsSameCallback(std::shared_ptr<uintptr_t> &callback);
    void RemoveCallbackReference(std::shared_ptr<uintptr_t> callback);
    void OnMicStateUpdated(const OHOS::AudioStandard::MicStateChangeEvent &micStateChangeEvent) override;

private:
    struct AudioManagerMicStateChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::MicStateChangeEvent micStateChangeEvent;
    };

    void OnJsCallbackMicStateChange(std::unique_ptr<AudioManagerMicStateChangeJsCallback> &jsCb);
    static void SafeJsCallbackMicStateChangeWork(AudioManagerMicStateChangeJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> micStateChangeCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_MICSTATECHANGE_CALLBACK_H
