/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include <mutex>
#include <cinttypes>
#include "errors.h"
#include "ipc_skeleton.h"
#include "ffrt.h"

#include "audio_utils.h"
#include "audio_setting_provider.h"
#include "audio_policy_log.h"
#include "audio_info.h"
#include "audio_setting_provider.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

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

    enum class VolumeDbAccessExceptionFuncId : int32_t {
        UNKNOWN = 0,
        SAVE_VOLUME_INTERNA_A,
        SAVE_VOLUME_INTERNA_B,
        GET_VOLUME_INTERNAL_A,
        GET_VOLUME_INTERNAL_B,
        SAVE_MUTE_STATUS_INTERNAL,
        GET_MUTE_STATUS_INTERNAL_A,
        GET_MUTE_STATUS_INTERNAL_B,
        GET_MUTE_AFFECTED,
        GET_MUTE_TRANSFER_STATUS,
        SAVE_MUTE_TRANSFER_STATUS,
        SAVE_RINGER_MODE,
        GET_RINGER_MODE,
        SAVE_SAFE_STATUS,
        GET_SAFE_STATUS_A,
        GET_SAFE_STATUS_B,
        SAVE_SAFE_VOLUME_TIME_A,
        SAVE_SAFE_VOLUME_TIME_B,
        GET_SAFE_VOLUME_TIME_A,
        GET_SAFE_VOLUME_TIME_B,
        SET_RESTORE_VOLUME_LEVEL_A,
        SET_RESTORE_VOLUME_LEVEL_B,
        GET_RESTORE_VOLUME_LEVEL_A,
        GET_RESTORE_VOLUME_LEVEL_B,
        SAVE_SYSTEM_SOUND_URL,
        GET_SYSTEM_SOUND_URL,
        SAVE_MIC_MUTE_STATE,
        GET_MIC_MUTE_STATE,
    };

    VolumeDataMaintainer();
    ~VolumeDataMaintainer();

    void SetDataShareReady(std::atomic<bool> isDataShareReady);
    bool SaveVolume(DeviceType type, AudioStreamType streamType, int32_t volumeLevel,
        std::string networkId = LOCAL_NETWORK_ID);
    bool GetVolume(DeviceType deviceType, AudioStreamType streamType, std::string networkId = LOCAL_NETWORK_ID);
    void SetStreamVolume(AudioStreamType streamType, int32_t volumeLevel);
    void SetAppVolume(int32_t appUid, int32_t volumeLevel);
    void GetAppMute(int32_t appUid, bool &isMute);
    void GetAppMuteOwned(int32_t appUid, bool &isMute);
    void SetAppVolumeMuted(int32_t appUid, bool muted);
    void SetAppStreamMuted(int32_t appUid, AudioStreamType streamType, bool muted);
    bool IsAppStreamMuted(int32_t appUid, AudioStreamType streamType);
    int32_t GetStreamVolume(AudioStreamType streamType);
    int32_t GetDeviceVolume(DeviceType deviceType, AudioStreamType streamType);
    int32_t GetAppVolume(int32_t appUid);
    bool IsSetAppVolume(int32_t appUid);
    std::unordered_map<AudioStreamType, int32_t> GetVolumeMap();

    bool GetMuteStatusWithDatabaseVolumeName(const std::string &databaseVolumeName, AudioStreamType streamType);
    bool SaveMuteStatusWithDatabaseVolumeName(const std::string &databaseVolumeName, AudioStreamType streamType,
        bool muteStatus);
    bool GetVolumeWithDatabaseVolumeName(const std::string &databaseVolumeName, AudioStreamType streamType);
    bool SaveVolumeWithDatabaseVolumeName(const std::string &databaseVolumeName, AudioStreamType streamType,
        int32_t volumeLevel);
    bool SaveMuteStatus(DeviceType deviceType, AudioStreamType streamType, bool muteStatus,
        std::string networkId = LOCAL_NETWORK_ID);
    bool GetMuteStatus(DeviceType deviceType, AudioStreamType streamType, std::string networkId = LOCAL_NETWORK_ID);
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
    bool SetRestoreVolumeLevel(DeviceType deviceType, int32_t volume);
    bool GetRestoreVolumeLevel(DeviceType deviceType, int32_t &volume);
    void RegisterCloned();
    bool SaveMicMuteState(bool isMute);
    bool GetMicMuteState(bool &isMute);
    bool CheckOsAccountReady();

    void SetVolumeDegree(AudioStreamType streamType, int32_t volumeDegree);
    int32_t GetVolumeDegree(AudioStreamType streamType);
    bool SaveVolumeDegree(DeviceType type, AudioStreamType streamType, int32_t volumeDegree,
        std::string networkId = LOCAL_NETWORK_ID);
    bool GetVolumeDegree(DeviceType deviceType, AudioStreamType streamType, std::string networkId = LOCAL_NETWORK_ID);
private:
    static std::string GetVolumeKeyForDataShare(DeviceType deviceType, AudioStreamType streamType,
        std::string networkId = LOCAL_NETWORK_ID);
    static std::string GetMuteKeyForDataShare(DeviceType deviceType, AudioStreamType streamType,
        std::string networkId = LOCAL_NETWORK_ID);
    static std::string GetVolumeKeyForDatabaseVolumeName(std::string databaseVolumeName, AudioStreamType streamType);
    static std::string GetMuteKeyForDatabaseVolumeName(std::string databaseVolumeName, AudioStreamType streamType);
    static std::string GetDeviceTypeName(DeviceType deviceType);
    bool SaveVolumeInternal(DeviceType type, AudioStreamType streamType, int32_t volumeLevel, std::string networkId);
    int32_t GetDeviceVolumeInternal(DeviceType deviceType, AudioStreamType streamType);
    bool GetVolumeInternal(DeviceType deviceType, AudioStreamType streamType, std::string networkId);
    void SetStreamVolumeInternal(AudioStreamType streamType, int32_t volumeLevel);
    bool SaveMuteStatusInternal(DeviceType deviceType, AudioStreamType streamType, bool muteStatus,
        std::string networkId = LOCAL_NETWORK_ID);
    bool GetMuteStatusInternal(DeviceType deviceType, AudioStreamType streamType,
        std::string networkId = LOCAL_NETWORK_ID);
    bool GetStreamMuteInternal(AudioStreamType streamType);
    int32_t GetStreamVolumeInternal(AudioStreamType streamType);
    void WriteVolumeDbAccessExceptionEvent(int32_t errorCase, int32_t errorMsg);

    ffrt::mutex volumeMutex_;
    ffrt::mutex volumeForDbMutex_;
    std::unordered_map<AudioStreamType, bool> muteStatusMap_; // save System volume Mutestatus map
    std::unordered_map<AudioStreamType, int32_t> volumeLevelMap_; // save system volume map
    std::unordered_map<int32_t, int32_t> appVolumeLevelMap_; // save App volume map
    std::unordered_map<int32_t, std::unordered_map<int32_t, bool>> appMuteStatusMap_; // save App volume Mutestatus map

    // Stores the mute status of audio streams used by the app.
    std::unordered_map<int32_t, std::unordered_map<AudioStreamType, bool>> appStreamMuteMap_;
    std::unordered_map<AudioStreamType, int32_t> volumeDegreeMap_; // save system volume degree map
    bool isSettingsCloneHaveStarted_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // VOLUME_DATA_MAINTAINER_H
