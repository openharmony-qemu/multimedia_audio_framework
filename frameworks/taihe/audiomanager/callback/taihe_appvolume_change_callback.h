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

#ifndef TAIHE_APPVOLUME_CHANGE_CALLBACK_H
#define TAIHE_APPVOLUME_CHANGE_CALLBACK_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_system_manager.h"
#include "audio_group_manager.h"
#include "event_handler.h"
#include "taihe_work.h"
#include "taihe_audio_manager.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
const std::string APP_VOLUME_CHANGE_CALLBACK_NAME_FOR_UID = "appVolumeChangeForUid";
const std::string APP_VOLUME_CHANGE_CALLBACK_NAME = "appVolumeChange";
class TaiheAudioManagerAppVolumeChangeCallback : public OHOS::AudioStandard::AudioManagerAppVolumeChangeCallback,
    public std::enable_shared_from_this<TaiheAudioManagerAppVolumeChangeCallback> {
public:
    explicit TaiheAudioManagerAppVolumeChangeCallback();
    virtual ~TaiheAudioManagerAppVolumeChangeCallback();
    void SaveVolumeChangeCallbackForUidReference(const std::string &callbackName,
        std::shared_ptr<uintptr_t> &callback, int32_t appUid);
    void SaveSelfVolumdChangeCallbackReference(const std::string &callbackName, std::shared_ptr<uintptr_t> &callback);
    bool IsSameCallback(std::shared_ptr<uintptr_t> &callback, std::shared_ptr<uintptr_t> &listCallback);
    void OnAppVolumeChangedForUid(int32_t appUid, const OHOS::AudioStandard::VolumeEvent &event) override;
    void OnSelfAppVolumeChanged(const OHOS::AudioStandard::VolumeEvent &event) override;

    void RemoveAudioVolumeChangeForUidCbRef(std::shared_ptr<uintptr_t> callback);
    void RemoveAllAudioVolumeChangeForUidCbRef();
    void RemoveSelfAudioVolumeChangeCbRef(std::shared_ptr<uintptr_t> callback);
    void RemoveAllSelfAudioVolumeChangeCbRef();
    int32_t GetAppVolumeChangeForUidListSize();
    int32_t GetSelfAppVolumeChangeListSize();

private:
    struct AudioManagerAppVolumeChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::VolumeEvent appVolumeChangeEvent;
    };

    void OnJsCallbackAppVolumeChange(std::unique_ptr<AudioManagerAppVolumeChangeJsCallback> &jsCb);
    static void SafeJsCallbackAppVolumeChangeWork(AudioManagerAppVolumeChangeJsCallback *event);

    std::mutex mutex_;
    std::list<std::pair<std::shared_ptr<AutoRef>, int32_t>> appVolumeChangeForUidList_;
    std::list<std::shared_ptr<AutoRef>> selfAppVolumeChangeList_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio

#endif // TAIHE_APPVOLUME_CHANGE_CALLBACK_H