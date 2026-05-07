/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#ifndef TAIHE_AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_H
#define TAIHE_AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_system_manager.h"
#include "audio_group_manager.h"
#include "audio_info.h"
#include "event_handler.h"
#include "taihe_work.h"
#include "taihe_audio_manager.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {

inline const char *AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_NAME = "systemVolumeChangeByFilter";
class TaiheAudioSystemVolumeChangeByFilterCallback : public OHOS::AudioStandard::SystemVolumeChangeByFilterCallback,
    public std::enable_shared_from_this<TaiheAudioSystemVolumeChangeByFilterCallback> {
public:
    explicit TaiheAudioSystemVolumeChangeByFilterCallback();
    virtual ~TaiheAudioSystemVolumeChangeByFilterCallback();
    void OnSystemVolumeChangeByFilter(const OHOS::AudioStandard::VolumeEvent &volumeEvent) override;
    void SaveCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> callback,
        const OHOS::AudioStandard::SystemVolumeFilter &filter);
    bool ContainSameJsCallback(std::shared_ptr<uintptr_t> callback);
    void RemoveCallbackReference(std::shared_ptr<uintptr_t> callback);
    void RemoveAllCallbackReference();
    int32_t GetSystemVolumeByFilterCbListSize();
    OHOS::AudioStandard::SystemVolumeFilter GetFilter() const { return filter_; }
    void SetFilter(const OHOS::AudioStandard::SystemVolumeFilter &filter) { filter_ = filter; }

private:
    struct AudioSystemVolumeChangeByFilterJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::VolumeEvent volumeEvent;
    };

    void OnJsCallbackSystemVolumeChangeByFilter(std::unique_ptr<AudioSystemVolumeChangeByFilterJsCallback> &jsCb);
    static void SafeJsCallbackSystemVolumeChangeByFilterWork(AudioSystemVolumeChangeByFilterJsCallback *event);

    std::mutex mutex_;
    std::list<std::shared_ptr<AutoRef>> audioSystemVolumeChangeByFilterCbList_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
    OHOS::AudioStandard::SystemVolumeFilter filter_;
};
} // namespace ANI::Audio

#endif // TAIHE_AUDIO_SYSTEM_VOLUME_CHANGE_BY_FILTER_CALLBACK_H