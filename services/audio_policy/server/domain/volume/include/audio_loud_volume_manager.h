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

#ifndef AUDIO_LOUD_VOLUME_MANAGER_H
#define AUDIO_LOUD_VOLUME_MANAGER_H

#include <mutex>
#include <pthread.h>

#include "audio_stream_info.h"
#include "iaudio_policy_interface.h"
#include "audio_active_device.h"
#include "audio_volume_manager.h"


namespace OHOS {
namespace AudioStandard {

#ifdef FEATURE_MULTIMODALINPUT_INPUT
class LoudVolumeManager {
public:
    explicit LoudVolumeManager();
    virtual ~LoudVolumeManager();
    bool loudVolumeModeEnable_ = false;
    bool ReloadLoudVolumeMode(const AudioStreamType streamInFocus, SetLoudVolMode setVolMode);
    bool ReloadLoudVolumeModeSwitch(LoudVolumeHoldType funcHoldType, SetLoudVolMode setVolMode);
    void SetLoudVolumeHoldMap(LoudVolumeHoldType funcHoldType, bool state);
    bool ClearLoudVolumeHoldMap(LoudVolumeHoldType funcHoldType);
    bool GetLoudVolumeHoldMap(LoudVolumeHoldType funcHoldType, bool &state);
    bool CheckLoudVolumeMode(const int32_t volLevel, const int32_t keyType, const AudioStreamType &streamInFocus);
    bool IsSkipCloseLoudVolType(AudioStreamType streamType);
    bool FindLoudVolStreamTypeEnable(AudioStreamType streamType, LoudVolumeHoldType &funcHoldType);
private:
    AudioVolumeManager &audioVolumeManager_;
    AudioActiveDevice &audioActiveDevice_;
    int32_t triggerTime = 0;
    int64_t upTriggerTimeMSec = 0;
    std::mutex loudVolTrigTimeMutex_;
    std::mutex setLoudVolHoldMutex_;
    std::unordered_map<LoudVolumeHoldType, bool> loudVolumeHoldMap_;
    const std::map<AudioVolumeType, LoudVolumeHoldType> LOUD_VOL_STREAM_TYPE_ENABLE = {
        {STREAM_MUSIC, LOUD_VOLUME_MODE_MUSIC},
        {STREAM_VOICE_CALL, LOUD_VOLUME_MODE_VOICE},
        {STREAM_VOICE_CALL_ASSISTANT, LOUD_VOLUME_MODE_VOICE},
    };
    const std::map<AudioVolumeType, AudioVolumeType> CONCURRENCY_KSIP_CLOSE_LOUD_VOL_TYPE = {
        {STREAM_RING, STREAM_RING},
    };
};
#endif
}
}
#endif