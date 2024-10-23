/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_GROUP_MANAGER_H
#define ST_AUDIO_GROUP_MANAGER_H

#include <cstdlib>
#include <map>
#include <mutex>
#include <vector>
#include <unordered_map>

#include "parcel.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioRingerModeCallback {
public:
    virtual ~AudioRingerModeCallback() = default;
    /**
     * Called when ringer mode is updated.
     *
     * @param ringerMode Indicates the updated ringer mode value.
     * For details, refer RingerMode enum in audio_info.h
     */
    virtual void OnRingerModeUpdated(const AudioRingerMode &ringerMode) = 0;
};

class AudioManagerMicStateChangeCallback {
public:
    virtual ~AudioManagerMicStateChangeCallback() = default;
    /**
     * Called when the microphone state changes
     *
     * @param micStateChangeEvent Microphone Status Information.
     */
    virtual void OnMicStateUpdated(const MicStateChangeEvent &micStateChangeEvent) = 0;
};

class AudioGroupManager {
public:
    AudioGroupManager(int32_t groupId);
    virtual ~AudioGroupManager();

    int32_t SetVolume(AudioVolumeType volumeType, int32_t volume, int32_t flag = 0);
    AudioStreamType GetActiveVolumeType(const int32_t clientUid);
    int32_t GetVolume(AudioVolumeType volumeType);
    int32_t GetMaxVolume(AudioVolumeType volumeType);
    int32_t GetMinVolume(AudioVolumeType volumeType);
    int32_t SetMute(AudioVolumeType volumeType, bool mute);
    int32_t IsStreamMute(AudioVolumeType volumeType, bool &isMute);
    int32_t Init();
    bool IsAlived();
    int32_t GetGroupId();
    int32_t SetRingerModeCallback(const int32_t clientId,
        const std::shared_ptr<AudioRingerModeCallback> &callback);
    int32_t UnsetRingerModeCallback(const int32_t clientId) const;
    int32_t UnsetRingerModeCallback(const int32_t clientId,
        const std::shared_ptr<AudioRingerModeCallback> &callback) const;
    int32_t SetRingerMode(AudioRingerMode ringMode) const;
    AudioRingerMode GetRingerMode() const;
    int32_t SetMicrophoneMute(bool isMute);
    int32_t SetMicrophoneMutePersistent(const bool isMute, const PolicyType type);
    bool GetPersistentMicMuteState();
    bool IsMicrophoneMuteLegacy();
    bool IsMicrophoneMute();
    int32_t SetMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback);
    int32_t UnsetMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback);
    bool IsVolumeUnadjustable();
    int32_t AdjustVolumeByStep(VolumeAdjustType adjustType);
    int32_t AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType);
    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType);
    float GetMaxAmplitude(const int32_t deviceId);
private:
    int32_t groupId_;
    ConnectType connectType_ = CONNECT_TYPE_LOCAL;
    std::string netWorkId_ = LOCAL_NETWORK_ID;
    int32_t cbClientId_ = -1;
    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t CONST_FACTOR = 100;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_GROUP_MANAGER_H
