/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MULTIMEDIA_AUDIO_VOLUME_GROUP_MANAGER_CALLBACK_H
#define MULTIMEDIA_AUDIO_VOLUME_GROUP_MANAGER_CALLBACK_H
#include "audio_group_manager.h"
#include "audio_system_manager.h"
#include "multimedia_audio_ffi.h"

namespace OHOS {
namespace AudioStandard {
class CjAudioRingerModeCallback : public AudioRingerModeCallback {
public:
    CjAudioRingerModeCallback() = default;
    virtual ~CjAudioRingerModeCallback() = default;
    void RegisterFunc(std::function<void(int32_t)> cjCallback);
    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override;

private:
    std::function<void(int32_t)> func_{};
    std::mutex cbMutex_;
};

class CjAudioManagerMicStateChangeCallback : public AudioManagerMicStateChangeCallback {
public:
    CjAudioManagerMicStateChangeCallback() = default;
    virtual ~CjAudioManagerMicStateChangeCallback() = default;
    void RegisterFunc(std::function<void(CMicStateChangeEvent)> cjCallback);
    void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) override;

private:
    std::function<void(CMicStateChangeEvent)> func_{};
    std::mutex cbMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // MULTIMEDIA_AUDIO_VOLUME_GROUP_MANAGER_CALLBACK_H
