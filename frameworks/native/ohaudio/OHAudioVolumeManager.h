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

#ifndef OH_AUDIO_VOLUME_MANAGER_H
#define OH_AUDIO_VOLUME_MANAGER_H

#include "audio_system_manager.h"
#include "audio_policy_interface.h"
#include "native_audio_volume_manager.h"

namespace OHOS {
namespace AudioStandard {
class OHStreamVolumeChangeCallback : public StreamVolumeChangeCallback {
public:
    OHStreamVolumeChangeCallback(OH_AudioVolumeManager_OnStreamVolumeChangeCallback callback, StreamUsage usage,
        void *userData)
        : callback_(callback), usage_(usage), userData_(userData)
    {
    }

    void OnStreamVolumeChange(StreamVolumeEvent streamVolumeEvent) override;
private:
    OH_AudioVolumeManager_OnStreamVolumeChangeCallback callback_ = nullptr;
    StreamUsage usage_ = StreamUsage::STREAM_USAGE_INVALID;
    void *userData_ = nullptr;
};

class OHAudioRingerModeCallback : public AudioRingerModeCallback {
public:
    OHAudioRingerModeCallback(OH_AudioVolumeManager_OnRingerModeChangeCallback callback, void *userData)
        : callback_(callback), userData_(userData)
    {
    }

    void OnRingerModeUpdated(const AudioRingerMode &ringerMode) override;
private:
    OH_AudioVolumeManager_OnRingerModeChangeCallback callback_ = nullptr;
    void *userData_ = nullptr;
};

class OHAudioVolumeManager {
public:
    ~OHAudioVolumeManager() = default;

    static OHAudioVolumeManager *GetInstance();
    int32_t GetMaxVolumeByUsage(StreamUsage streamUsage);
    int32_t GetMinVolumeByUsage(StreamUsage streamUsage);
    int32_t GetVolumeByUsage(StreamUsage streamUsage);
    int32_t IsMuteByUsage(StreamUsage streamUsage, bool &isMute);
    int32_t GetRingerMode();
    int32_t SetStreamVolumeChangeCallback(OH_AudioVolumeManager_OnStreamVolumeChangeCallback callback,
        StreamUsage streamUsage, void *userData);
    int32_t UnsetStreamVolumeChangeCallback(OH_AudioVolumeManager_OnStreamVolumeChangeCallback callback);
    int32_t SetAudioRingerModeChangeCallback(OH_AudioVolumeManager_OnRingerModeChangeCallback callback, void *userData);
    int32_t UnsetAudioRingerModeChangeCallback(OH_AudioVolumeManager_OnRingerModeChangeCallback callback);

private:
    OHAudioVolumeManager();

    AudioSystemManager *audioSystemManager_ = nullptr;
    std::shared_ptr<AudioGroupManager> audioGroupManager_ = nullptr;
    std::map<OH_AudioVolumeManager_OnRingerModeChangeCallback,
        std::shared_ptr<OHAudioRingerModeCallback>> ringerModeCallbacks_;
    std::map<OH_AudioVolumeManager_OnStreamVolumeChangeCallback,
        std::pair<StreamUsage, std::shared_ptr<OHStreamVolumeChangeCallback>>> streamVolumeCallbacks_;

    std::mutex ringerModeCbMutex_;
    std::mutex streamVolumeCbMutex_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // OH_AUDIO_VOLUME_MANAGER_H