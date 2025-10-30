/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_H
#define TAIHE_AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_system_manager.h"
#include "audio_group_manager.h"
#include "event_handler.h"
#include "taihe_work.h"
#include "taihe_audio_manager.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
const std::string AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_NAME = "streamVolumeChange";
class TaiheAudioStreamVolumeChangeCallback : public OHOS::AudioStandard::StreamVolumeChangeCallback,
    public std::enable_shared_from_this<TaiheAudioStreamVolumeChangeCallback> {
public:
    explicit TaiheAudioStreamVolumeChangeCallback();
    virtual ~TaiheAudioStreamVolumeChangeCallback();
    void OnStreamVolumeChange(OHOS::AudioStandard::StreamVolumeEvent volumeEvent) override;
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback);
    bool ContainSameJsCallback(std::shared_ptr<uintptr_t> callback);

private:
    struct AudioStreamVolumeChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::StreamVolumeEvent volumeEvent;
    };

    void OnJsCallbackStreamVolumeChange(std::unique_ptr<AudioStreamVolumeChangeJsCallback> &jsCb);
    static void SafeJsCallbackStreamVolumeChangeWork(AudioStreamVolumeChangeJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<uintptr_t> callback_ = nullptr;
    std::shared_ptr<AutoRef> audioStreamVolumeChangeCallback_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio

#endif // TAIHE_AUDIO_STREAM_VOLUME_CHANGE_CALLBACK_H