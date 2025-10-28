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

#ifndef TAIHE_ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_H
#define TAIHE_ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_H

#include "ohos.multimedia.audio.proj.hpp"
#include "ohos.multimedia.audio.impl.hpp"
#include "taihe/runtime.hpp"
#include "audio_system_manager.h"
#include "event_handler.h"
#include "taihe_work.h"
#include "taihe_audio_manager.h"

namespace ANI::Audio {
using namespace taihe;
using namespace ohos::multimedia::audio;
const std::string ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_NAME = "activeVolumeTypeChange";
class TaiheAudioManagerActiveVolumeTypeChangeCallback :
    public OHOS::AudioStandard::AudioManagerActiveVolumeTypeChangeCallback,
    public std::enable_shared_from_this<TaiheAudioManagerActiveVolumeTypeChangeCallback> {
public:
    explicit TaiheAudioManagerActiveVolumeTypeChangeCallback();
    virtual ~TaiheAudioManagerActiveVolumeTypeChangeCallback();
    void OnActiveVolumeTypeChanged(const OHOS::AudioStandard::AudioVolumeType &event) override;
    void SaveActiveVolumeTypeChangeCallbackReference(const std::string &callbackName,
        std::shared_ptr<uintptr_t> &callback);
    bool IsSameCallback(const std::shared_ptr<uintptr_t> &callback, const std::shared_ptr<uintptr_t> &listCallback);
    void RemoveSelfActiveVolumeTypeChangeCbRef(std::shared_ptr<uintptr_t> callback);
    void RemoveAllActiveVolumeTypeChangeCbRef();
    void RemoveCallbackReference(std::shared_ptr<uintptr_t> callback);
    int32_t GetActiveVolumeTypeChangeListSize();

private:
    struct AudioManagerActiveVolumeTypeChangeJsCallback {
        std::shared_ptr<AutoRef> callback = nullptr;
        std::string callbackName = "unknown";
        OHOS::AudioStandard::AudioVolumeType activeVolumeTypeChangeEvent;
    };

    void OnJsCallbackActiveVolumeTypeChange(std::unique_ptr<AudioManagerActiveVolumeTypeChangeJsCallback> &jsCb);
    static void SafeJsCallbackActiveVolumeTypeChangeWork(AudioManagerActiveVolumeTypeChangeJsCallback *event);

    std::mutex mutex_;
    std::shared_ptr<AutoRef> activeVolumeTypeChangeCallback_ = nullptr;
    std::list<std::shared_ptr<AutoRef>> activeVolumeTypeChangeList_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> mainHandler_ = nullptr;
};
} // namespace ANI::Audio

#endif // TAIHE_ACTIVE_VOLUME_TYPE_CHANGE_CALLBACK_H