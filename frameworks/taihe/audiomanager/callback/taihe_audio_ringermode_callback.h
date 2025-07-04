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
#ifndef TAIHE_AUDIO_RINGMODE_CALLBACK_H
#define TAIHE_AUDIO_RINGMODE_CALLBACK_H

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
const std::string RINGERMODE_CALLBACK_NAME = "ringerModeChange";

class TaiheAudioRingerModeCallback : public OHOS::AudioStandard::AudioRingerModeCallback,
public std::enable_shared_from_this<TaiheAudioRingerModeCallback> {
public:
    explicit TaiheAudioRingerModeCallback();
    virtual ~TaiheAudioRingerModeCallback();
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback);
    bool IsSameCallback(std::shared_ptr<uintptr_t> &callback);
    void RemoveCallbackReference(std::shared_ptr<uintptr_t> callback);
    void OnRingerModeUpdated(const OHOS::AudioStandard::AudioRingerMode &ringerMode) override;

private:
    struct AudioRingerModeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::AudioRingerMode ringerMode = OHOS::AudioStandard::RINGER_MODE_NORMAL;
    };

    void OnJsCallbackRingerMode(std::unique_ptr<AudioRingerModeJsCallback> &jsCb);
    static void SafeJsCallbackRingModeWork(AudioRingerModeJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> ringerModeCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio
#endif // TAIHE_AUDIO_RINGMODE_CALLBACK_H
