/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef VOLUME_DATA_MAINTAINER_H
#define VOLUME_DATA_MAINTAINER_H

#include <list>
#include <unordered_map>
#include <cinttypes>

#include "ipc_skeleton.h"
#include "errors.h"
#include "mutex"

#include "audio_setting_provider.h"
#include "audio_policy_log.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t MAX_SAFE_STATUS = 2;

class VolumeDataMaintainer {
public:
    enum VolumeDataMaintainerStreamType {  // define with Dual framework
        VT_STREAM_DEFAULT = -1,
        VT_STREAM_VOICE_CALL = 0,
        VT_STREAM_SYSTEM  = 1,
        VT_STREAM_RING = 2,
        VT_STREAM_MUSIC  = 3,
        VT_STREAM_ALARM = 4,
        VT_STREAM_NOTIFICATION = 5,
        VT_STREAM_BLUETOOTH_SCO = 6,
        VT_STREAM_SYSTEM_ENFORCED = 7,
        VT_STREAM_DTMF = 8,
        VT_STREAM_TTS = 9,
        VT_STREAM_ACCESSIBILITY = 10,
        VT_STREAM_ASSISTANT = 11,
    };

    static VolumeDataMaintainer& GetVolumeDataMaintainer()
    {
        static VolumeDataMaintainer volumeDataMainTainer;
        return volumeDataMainTainer;
    }
    ~VolumeDataMaintainer();

    bool SetFirstBoot(bool fristBoot);
    bool GetFirstBoot(bool &firstBoot);

    bool SaveVolume(DeviceType type, AudioStreamType streamType, int32_t volumeLevel);
    bool GetVolume(DeviceType deviceType, AudioStreamType streamType);
    void SetStreamVolume(AudioStreamType streamType, int32_t volumeLevel);
    int32_t GetStreamVolume(AudioStreamType streamType);
    std::unordered_map<AudioStreamType, int32_t> GetVolumeMap();

    bool SaveMuteStatus(DeviceType deviceType, AudioStreamType streamType,
        bool muteStatus);
    bool GetMuteStatus(DeviceType deviceType, AudioStreamType streamType);
    bool SetStreamMuteStatus(AudioStreamType streamType, bool muteStatus);
    bool GetStreamMute(AudioStreamType streamType);

    bool GetMuteAffected(int32_t &affected);
    bool GetMuteTransferStatus(bool &status);
    bool SetMuteAffectedToMuteStatusDataBase(int32_t affected);
    bool SaveMuteTransferStatus(bool status);

    bool SaveRingerMode(AudioRingerMode ringerMode);
    bool GetRingerMode(AudioRingerMode &ringerMode);
    bool SaveSafeStatus(DeviceType deviceType, SafeStatus safeStatus);
    bool GetSafeStatus(DeviceType deviceType, SafeStatus &safeStatus);
    bool SaveSafeVolumeTime(DeviceType deviceType, int64_t time);
    bool GetSafeVolumeTime(DeviceType deviceType, int64_t &time);
    bool SaveSystemSoundUrl(const std::string &key, const std::string &value);
    bool GetSystemSoundUrl(const std::string &key, std::string &value);
    void RegisterCloned();
    bool SaveMicMuteState(bool isMute);
    bool GetMicMuteState(bool &isMute);

private:
    VolumeDataMaintainer();
    static std::string GetVolumeKeyForDataShare(DeviceType deviceType, AudioStreamType streamType);
    static std::string GetMuteKeyForDataShare(DeviceType deviceType, AudioStreamType streamType);
    static std::string GetDeviceTypeName(DeviceType deviceType);
    bool GetVolumeInternal(DeviceType deviceType, AudioStreamType streamType);
    void SetStreamVolumeInternal(AudioStreamType streamType, int32_t volumeLevel);
    bool SaveMuteStatusInternal(DeviceType deviceType, AudioStreamType streamType, bool muteStatus);
    bool GetMuteStatusInternal(DeviceType deviceType, AudioStreamType streamType);
    bool GetStreamMuteInternal(AudioStreamType streamType);
    int32_t GetStreamVolumeInternal(AudioStreamType streamType);

    std::mutex volumeMutex_;
    std::mutex volumeForDbMutex_;
    std::unordered_map<AudioStreamType, bool> muteStatusMap_; // save volume Mutestatus map
    std::unordered_map<AudioStreamType, int32_t> volumeLevelMap_; // save volume map
    bool isSettingsCloneHaveStarted_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // VOLUME_DATA_MAINTAINER_H
