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
#ifndef LOG_TAG
#define LOG_TAG "VolumeDataMaintainer"
#endif

#include "volume_data_maintainer.h"
#include "system_ability_definition.h"
#include "audio_policy_manager_factory.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_SAFE_VOLUME_STATE = "audio_safe_volume_state";
const std::string AUDIO_SAFE_VOLUME_STATE_BT = "audio_safe_volume_state_bt";
const std::string UNSAFE_VOLUME_MUSIC_ACTIVE_MS = "unsafe_volume_music_active_ms";
const std::string UNSAFE_VOLUME_MUSIC_ACTIVE_MS_BT = "unsafe_volume_music_active_ms_bt";
const std::string UNSAFE_VOLUME_LEVEL = "unsafe_volume_level";
const std::string UNSAFE_VOLUME_LEVEL_BT = "unsafe_volume_level_bt";
const std::string SETTINGS_CLONED = "settingsCloneStatus";
const int32_t INVALIAD_SETTINGS_CLONE_STATUS = -1;
const int32_t SETTINGS_CLONING_STATUS = 1;
const int32_t SETTINGS_CLONED_STATUS = 0;
constexpr int32_t MAX_SAFE_STATUS = 2;

static const std::vector<VolumeDataMaintainer::VolumeDataMaintainerStreamType> VOLUME_MUTE_STREAM_TYPE = {
    // all volume types except STREAM_ALL
    VolumeDataMaintainer::VT_STREAM_ALARM,
    VolumeDataMaintainer::VT_STREAM_DTMF,
    VolumeDataMaintainer::VT_STREAM_TTS,
    VolumeDataMaintainer::VT_STREAM_ACCESSIBILITY,
    VolumeDataMaintainer::VT_STREAM_ASSISTANT,
};

static const std::vector<DeviceType> DEVICE_TYPE_LIST = {
    // The five devices represent the three volume groups(build-in, wireless, wired).
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_REMOTE_CAST
};

static std::map<VolumeDataMaintainer::VolumeDataMaintainerStreamType, AudioStreamType> AUDIO_STREAMTYPE_MAP = {
    {VolumeDataMaintainer::VT_STREAM_ALARM, STREAM_ALARM},
    {VolumeDataMaintainer::VT_STREAM_DTMF, STREAM_DTMF},
    {VolumeDataMaintainer::VT_STREAM_TTS, STREAM_VOICE_ASSISTANT},
    {VolumeDataMaintainer::VT_STREAM_ACCESSIBILITY, STREAM_ACCESSIBILITY},
};

static std::map<AudioStreamType, std::string> AUDIO_STREAMTYPE_VOLUME_MAP = {
    {STREAM_MUSIC, "music_volume"},
    {STREAM_RING, "ring_volume"},
    {STREAM_SYSTEM, "system_volume"},
    {STREAM_NOTIFICATION, "notification_volume"},
    {STREAM_ALARM, "alarm_volume"},
    {STREAM_DTMF, "dtmf_volume"},
    {STREAM_VOICE_CALL, "voice_call_volume"},
    {STREAM_VOICE_ASSISTANT, "voice_assistant_volume"},
    {STREAM_ACCESSIBILITY, "accessibility_volume"},
    {STREAM_ULTRASONIC, "ultrasonic_volume"},
    {STREAM_WAKEUP,  "wakeup"},
};

static std::map<AudioStreamType, std::string> AUDIO_STREAMTYPE_MUTE_STATUS_MAP = {
    {STREAM_MUSIC, "music_mute_status"},
    {STREAM_RING, "ring_mute_status"},
    {STREAM_SYSTEM, "system_mute_status"},
    {STREAM_NOTIFICATION, "notification_mute_status"},
    {STREAM_ALARM, "alarm_mute_status"},
    {STREAM_DTMF, "dtmf_mute_status"},
    {STREAM_VOICE_CALL, "voice_call_mute_status"},
    {STREAM_VOICE_ASSISTANT, "voice_assistant_mute_status"},
    {STREAM_ACCESSIBILITY, "accessibility_mute_status"},
    {STREAM_ULTRASONIC, "unltrasonic_mute_status"},
};

VolumeDataMaintainer::VolumeDataMaintainer()
{
    AUDIO_DEBUG_LOG("VolumeDataMaintainer Create");
}

VolumeDataMaintainer::~VolumeDataMaintainer()
{
    AUDIO_DEBUG_LOG("VolumeDataMaintainer Destory");
}

bool VolumeDataMaintainer::CheckOsAccountReady()
{
    return AudioSettingProvider::CheckOsAccountReady();
}

void VolumeDataMaintainer::SetDataShareReady(std::atomic<bool> isDataShareReady)
{
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    audioSettingProvider.SetDataShareReady(std::atomic_load(&isDataShareReady));
}

bool VolumeDataMaintainer::SaveVolume(DeviceType type, AudioStreamType streamType, int32_t volumeLevel,
    std::string networkId)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return SaveVolumeInternal(type, streamForVolumeMap, volumeLevel, networkId);
}

bool VolumeDataMaintainer::SaveVolumeDegree(DeviceType type, AudioStreamType streamTypeIn,
    int32_t volumeDegree, std::string networkId)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamType = VolumeUtils::GetVolumeTypeFromStreamType(streamTypeIn);

    std::string volumeKey = GetVolumeKeyForDataShare(type, streamType, networkId);
    if (!volumeKey.compare("")) {
        AUDIO_ERR_LOG("[device %{public}d, streamType %{public}d] is not supported for datashare",
            type, streamType);
        return false;
    }
    volumeKey += "_degree";

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.PutIntValue(volumeKey, volumeDegree, "system");
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Save Volume To DataBase volumeMap failed");
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::SaveVolumeInternal(DeviceType type, AudioStreamType streamType, int32_t volumeLevel,
    std::string networkId)
{
    std::string volumeKey = GetVolumeKeyForDataShare(type, streamType, networkId);
    if (!volumeKey.compare("")) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_VOLUME_INTERNA_A),
            ERR_READ_FAILED);
        AUDIO_ERR_LOG("[device %{public}d, streamType %{public}d] is not supported for datashare",
            type, streamType);
        return false;
    }

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.PutIntValue(volumeKey, volumeLevel, "system");
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_VOLUME_INTERNA_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("Save Volume To DataBase volumeMap failed");
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::GetVolume(DeviceType deviceType, AudioStreamType streamType, std::string networkId)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return GetVolumeInternal(deviceType, streamForVolumeMap, networkId);
}

bool VolumeDataMaintainer::GetVolumeDegree(DeviceType deviceType, AudioStreamType streamTypeIn, std::string networkId)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamType = VolumeUtils::GetVolumeTypeFromStreamType(streamTypeIn);
    // Voice call assistant stream is full volume by default
    if (streamType == STREAM_VOICE_CALL_ASSISTANT) {
        return true;
    }
    std::string volumeKey = GetVolumeKeyForDataShare(deviceType, streamType, networkId);
    if (!volumeKey.compare("")) {
        AUDIO_ERR_LOG("[device %{public}d, streamType %{public}d] is not supported for datashare",
            deviceType, streamType);
        return false;
    }
    volumeKey += "_degree";

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t volumeValue = 0;
    ErrCode ret = audioSettingProvider.GetIntValue(volumeKey, volumeValue, "system");
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Get streamType %{public}d, deviceType %{public}d, Volume FromDataBase volumeMap failed.",
            streamType, deviceType);
        return false;
    } else {
        volumeDegreeMap_[streamType] = volumeValue;
        AUDIO_PRERELEASE_LOGI("Get streamType %{public}d, deviceType %{public}d, "\
            "Volume FromDataBase volumeMap from datashare %{public}d.", streamType, deviceType, volumeValue);
    }

    return true;
}

bool VolumeDataMaintainer::GetVolumeInternal(DeviceType deviceType, AudioStreamType streamType, std::string networkId)
{
    // Voice call assistant stream is full volume by default
    if (streamType == STREAM_VOICE_CALL_ASSISTANT) {
        return true;
    }
    std::string volumeKey = GetVolumeKeyForDataShare(deviceType, streamType, networkId);
    if (!volumeKey.compare("")) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_VOLUME_INTERNAL_A),
            ERR_READ_FAILED);
        AUDIO_ERR_LOG("[device %{public}d, streamType %{public}d] is not supported for datashare",
            deviceType, streamType);
        return false;
    }

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t volumeValue = 0;
    ErrCode ret = audioSettingProvider.GetIntValue(volumeKey, volumeValue, "system");
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_VOLUME_INTERNAL_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("Get streamType %{public}d, deviceType %{public}d, Volume FromDataBase volumeMap failed.",
            streamType, deviceType);
        return false;
    } else {
        volumeLevelMap_[streamType] = volumeValue;
        AUDIO_PRERELEASE_LOGI("Get streamType %{public}d, deviceType %{public}d, "\
            "Volume FromDataBase volumeMap from datashare %{public}d.", streamType, deviceType, volumeValue);
    }

    return true;
}

bool VolumeDataMaintainer::SaveVolumeWithDatabaseVolumeName(const std::string &databaseVolumeName,
    AudioStreamType streamType, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("SaveVolume: databaseVolumeName [%{public}s], streamType [%{public}d], volumeLevel [%{public}d]",
        databaseVolumeName.c_str(), streamType, volumeLevel);
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    std::string volumeKey = GetVolumeKeyForDatabaseVolumeName(databaseVolumeName, streamForVolumeMap);
    if (volumeKey == "") {
        AUDIO_ERR_LOG("databaseVolumeName [%{public}s], streamType [%{public}d] is not supported for dataShare",
            databaseVolumeName.c_str(), streamType);
        return false;
    }
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.PutIntValue(volumeKey, volumeLevel, "system");
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to save volume to database!");
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::GetVolumeWithDatabaseVolumeName(const std::string &databaseVolumeName,
    AudioStreamType streamType)
{
    AUDIO_INFO_LOG("GetVolume: databaseVolumeName [%{public}s], streamType [%{public}d]",
        databaseVolumeName.c_str(), streamType);
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    std::string volumeKey = GetVolumeKeyForDatabaseVolumeName(databaseVolumeName, streamForVolumeMap);
    if (volumeKey == "") {
        AUDIO_ERR_LOG("databaseVolumeName [%{public}s], streamType [%{public}d] is not supported for dataShare",
            databaseVolumeName.c_str(), streamType);
        return false;
    }
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t volumeValue = 0;
    ErrCode ret = audioSettingProvider.GetIntValue(volumeKey, volumeValue, "system");
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to get volume from database!");
        return false;
    }
    volumeLevelMap_[streamForVolumeMap] = volumeValue;
    AUDIO_INFO_LOG("Get Volume: volumeKey [%{public}s], volumeValue [%{public}d]",
        volumeKey.c_str(), volumeValue);
    return true;
}

bool VolumeDataMaintainer::SaveMuteStatusWithDatabaseVolumeName(const std::string &databaseVolumeName,
    AudioStreamType streamType, bool muteStatus)
{
    AUDIO_INFO_LOG("SaveMuteStatus: databaseVolumeName [%{public}s], streamType [%{public}d], muteStatus [%{public}d]",
        databaseVolumeName.c_str(), streamType, muteStatus);
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    std::string muteKey = GetMuteKeyForDatabaseVolumeName(databaseVolumeName, streamForVolumeMap);
    if (muteKey == "") {
        AUDIO_ERR_LOG("databaseVolumeName [%{public}s], streamType [%{public}d] is not supported for dataShare",
            databaseVolumeName.c_str(), streamType);
        return false;
    }
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.PutBoolValue(muteKey, muteStatus, "system");
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to save mute status to database!");
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::GetMuteStatusWithDatabaseVolumeName(const std::string &databaseVolumeName,
    AudioStreamType streamType)
{
    AUDIO_INFO_LOG("GetMuteStatus: databaseVolumeName [%{public}s], streamType [%{public}d]",
        databaseVolumeName.c_str(), streamType);
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    std::string muteKey = GetMuteKeyForDatabaseVolumeName(databaseVolumeName, streamForVolumeMap);
    if (muteKey == "") {
        AUDIO_ERR_LOG("databaseVolumeName [%{public}s], streamType [%{public}d] is not supported for dataShare",
            databaseVolumeName.c_str(), streamType);
        return false;
    }
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    bool muteStatus = false;
    ErrCode ret = audioSettingProvider.GetBoolValue(muteKey, muteStatus, "system");
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to get mute status from database!");
        return false;
    }
    muteStatusMap_[streamForVolumeMap] = muteStatus;
    AUDIO_INFO_LOG("GetMuteStatus: muteKey [%{public}s], muteStatus [%{public}d]",
        muteKey.c_str(), muteStatus);
    return true;
}

void VolumeDataMaintainer::SetAppVolume(int32_t appUid, int32_t volumeLevel)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    appVolumeLevelMap_[appUid] = volumeLevel;
}

void VolumeDataMaintainer::SetAppVolumeMuted(int32_t appUid, bool muted)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    int ownedAppUid = IPCSkeleton::GetCallingUid();
    appMuteStatusMap_[appUid][ownedAppUid] = muted;
}

void VolumeDataMaintainer::SetAppStreamMuted(int32_t appUid, AudioStreamType streamType, bool muted)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    if (muted) {
        // Set mute status for the given app and stream type
        appStreamMuteMap_[appUid][streamType] = true;
    } else {
        auto uidIt = appStreamMuteMap_.find(appUid);
        if (uidIt != appStreamMuteMap_.end()) {
            // Remove the stream type if mute is false
            uidIt->second.erase(streamType);
            // If no more stream types under this appUid, remove the appUid entry
            if (uidIt->second.empty()) {
                appStreamMuteMap_.erase(uidIt);
            }
        }
    }
}

bool VolumeDataMaintainer::IsAppStreamMuted(int32_t appUid, AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    auto uidIt = appStreamMuteMap_.find(appUid);
    if (uidIt == appStreamMuteMap_.end()) {
        return false;
    }

    const auto &streamMap = uidIt->second;
    auto streamIt = streamMap.find(streamType);
    if (streamIt == streamMap.end()) {
        return false;
    }

    return streamIt->second;
}

void VolumeDataMaintainer::GetAppMute(int32_t appUid, bool &isMute)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    auto iter = appMuteStatusMap_.find(appUid);
    if (iter == appMuteStatusMap_.end()) {
        isMute = false;
    } else {
        for (auto subIter : iter->second) {
            if (subIter.second) {
                isMute = true;
                return;
            }
        }
        isMute = false;
    }
}

void VolumeDataMaintainer::GetAppMuteOwned(int32_t appUid, bool &isMute)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    int ownedAppUid = IPCSkeleton::GetCallingUid();
    auto iter = appMuteStatusMap_.find(appUid);
    if (iter == appMuteStatusMap_.end()) {
        isMute = false;
    } else {
        isMute = iter->second[ownedAppUid];
    }
}

void VolumeDataMaintainer::SetStreamVolume(AudioStreamType streamType, int32_t volumeLevel)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    SetStreamVolumeInternal(streamType, volumeLevel);
}

void VolumeDataMaintainer::SetStreamVolumeInternal(AudioStreamType streamType, int32_t volumeLevel)
{
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    volumeLevelMap_[streamForVolumeMap] = volumeLevel;
}

int32_t VolumeDataMaintainer::GetStreamVolume(AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    return GetStreamVolumeInternal(streamType);
}

int32_t VolumeDataMaintainer::GetDeviceVolume(DeviceType deviceType, AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return GetDeviceVolumeInternal(deviceType, streamForVolumeMap);
}

int32_t VolumeDataMaintainer::GetDeviceVolumeInternal(DeviceType deviceType, AudioStreamType streamType)
{
    std::string volumeKey = GetVolumeKeyForDataShare(deviceType, streamType);
    int32_t volumeValue = 0;
    if (!volumeKey.compare("")) {
        AUDIO_ERR_LOG("[device %{public}d, streamType %{public}d] is not supported for datashare",
            deviceType, streamType);
        return volumeValue;
    }

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.GetIntValue(volumeKey, volumeValue, "system");
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Get streamType %{public}d, deviceType %{public}d, Volume FromDataBase volumeMap failed.",
            streamType, deviceType);
    } else {
        AUDIO_PRERELEASE_LOGI("Get streamType %{public}d, deviceType %{public}d, "\
            "Volume FromDataBase volumeMap from datashare %{public}d.", streamType, deviceType, volumeValue);
    }

    return volumeValue;
}

bool VolumeDataMaintainer::IsSetAppVolume(int32_t appUid)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    return appVolumeLevelMap_.find(appUid) != appVolumeLevelMap_.end();
}

int32_t VolumeDataMaintainer::GetAppVolume(int32_t appUid)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    return appVolumeLevelMap_[appUid];
}


int32_t VolumeDataMaintainer::GetStreamVolumeInternal(AudioStreamType streamType)
{
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return volumeLevelMap_[streamForVolumeMap];
}

void VolumeDataMaintainer::WriteVolumeDbAccessExceptionEvent(int32_t errorCase, int32_t errorMsg)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::DB_ACCESS_EXCEPTION,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("DB_TYPE", "volume");
    bean->Add("ERROR_CASE", errorCase);
    bean->Add("ERROR_MSG", errorMsg);
    bean->Add("ERROR_DESCRIPTION", "Dateabase access failed");
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

std::unordered_map<AudioStreamType, int32_t> VolumeDataMaintainer::GetVolumeMap()
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    return volumeLevelMap_;
}

bool VolumeDataMaintainer::SaveMuteStatus(DeviceType deviceType, AudioStreamType streamType,
    bool muteStatus, std::string networkId)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    if (streamType == STREAM_RING && VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_RING) {
        AUDIO_INFO_LOG("set ring stream mute status to all device.");
        bool saveMuteResult = false;
        for (auto &device : DEVICE_TYPE_LIST) {
            // set ring stream mute status to device
            saveMuteResult = SaveMuteStatusInternal(device, streamType, muteStatus, networkId);
            if (!saveMuteResult) {
                AUDIO_INFO_LOG("save mute failed.");
                break;
            }
        }
        return saveMuteResult;
    }
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return SaveMuteStatusInternal(deviceType, streamForVolumeMap, muteStatus, networkId);
}

bool VolumeDataMaintainer::SaveMuteStatusInternal(DeviceType deviceType, AudioStreamType streamType,
    bool muteStatus, std::string networkId)
{
    std::string muteKey = GetMuteKeyForDataShare(deviceType, streamType, networkId);
    if (!muteKey.compare("")) {
        WriteVolumeDbAccessExceptionEvent(
            static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_MUTE_STATUS_INTERNAL),
            ERR_READ_FAILED);
        AUDIO_ERR_LOG("[device %{public}d, streamType %{public}d] is not supported for "\
            "datashare", deviceType, streamType);
        return false;
    }

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.PutBoolValue(muteKey, muteStatus, "system");
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("Failed to write mutestatus: %{public}d to setting db! Err: %{public}d", muteStatus, ret);
    } else {
        AUDIO_INFO_LOG("muteKey:%{public}s, muteStatus:%{public}d", muteKey.c_str(), muteStatus);
    }

    return true;
}

bool VolumeDataMaintainer::SetStreamMuteStatus(AudioStreamType streamType, bool muteStatus)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    muteStatusMap_[streamForVolumeMap] = muteStatus;
    return true;
}

bool VolumeDataMaintainer::GetMuteStatus(DeviceType deviceType, AudioStreamType streamType,
    std::string networkId)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return GetMuteStatusInternal(deviceType, streamForVolumeMap, networkId);
}

bool VolumeDataMaintainer::GetMuteStatusInternal(DeviceType deviceType, AudioStreamType streamType,
    std::string networkId)
{
    std::string muteKey = GetMuteKeyForDataShare(deviceType, streamType, networkId);
    if (!muteKey.compare("")) {
        WriteVolumeDbAccessExceptionEvent(
            static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_MUTE_STATUS_INTERNAL_A),
            ERR_READ_FAILED);
        AUDIO_ERR_LOG("[device %{public}d, streamType %{public}d] is not supported for "\
            "datashare", deviceType, streamType);
        return false;
    }

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    bool muteStatus = false;
    ErrCode ret = audioSettingProvider.GetBoolValue(muteKey, muteStatus, "system");
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(
            static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_MUTE_STATUS_INTERNAL_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("Get MuteStatus From DataBase muteStatus failed");
        return false;
    } else {
        muteStatusMap_[streamType] = muteStatus;
        AUDIO_DEBUG_LOG("Get MuteStatus From DataBase muteStatus from datashare %{public}d", muteStatus);
    }

    return true;
}

bool VolumeDataMaintainer::GetStreamMute(AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    return GetStreamMuteInternal(streamType);
}

bool VolumeDataMaintainer::GetStreamMuteInternal(AudioStreamType streamType)
{
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return muteStatusMap_[streamForVolumeMap];
}

bool VolumeDataMaintainer::GetMuteAffected(int32_t &affected)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    const std::string settingKey = "mute_streams_affected";
    int32_t value = 0;
    ErrCode ret = settingProvider.GetIntValue(settingKey, value, "system");
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_MUTE_AFFECTED),
            static_cast<int32_t>(ret));
        AUDIO_WARNING_LOG("Failed to get muteaffected failed Err: %{public}d", ret);
        return false;
    } else {
        affected = value;
    }
    return true;
}

bool VolumeDataMaintainer::GetMuteTransferStatus(bool &status)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    const std::string settingKey = "need_mute_affected_transfer";
    ErrCode ret = settingProvider.GetBoolValue(settingKey, status);
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_MUTE_TRANSFER_STATUS),
            static_cast<int32_t>(ret));
        AUDIO_WARNING_LOG("Failed to get muteaffected failed Err: %{public}d", ret);
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::SetMuteAffectedToMuteStatusDataBase(int32_t affected)
{
    // transfer mute_streams_affected to mutestatus
    for (auto &streamtype : VOLUME_MUTE_STREAM_TYPE) {
        if (static_cast<uint32_t>(affected) & (1 << streamtype)) {
            for (auto &device : DEVICE_TYPE_LIST) {
                // save mute status to database
                SaveMuteStatusInternal(device, AUDIO_STREAMTYPE_MAP[streamtype], true);
            }
        }
    }
    return true;
}

bool VolumeDataMaintainer::SaveMuteTransferStatus(bool status)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    const std::string settingKey = "need_mute_affected_transfer";
    ErrCode ret = settingProvider.PutIntValue(settingKey, status);
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(
            static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_MUTE_TRANSFER_STATUS),
            static_cast<int32_t>(ret));
        AUDIO_WARNING_LOG("Failed to SaveMuteTransferStatus: %{public}d to setting db! Err: %{public}d", status, ret);
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::SaveRingerMode(AudioRingerMode ringerMode)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    const std::string settingKey = "ringer_mode";
    ErrCode ret = settingProvider.PutIntValue(settingKey, static_cast<int32_t>(ringerMode));
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_RINGER_MODE),
            static_cast<int32_t>(ret));
        AUDIO_WARNING_LOG("Failed to write ringer_mode: %{public}d to setting db! Err: %{public}d", ringerMode, ret);
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::GetRingerMode(AudioRingerMode &ringerMode)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    const std::string settingKey = "ringer_mode";
    int32_t value = 0;
    ErrCode ret = settingProvider.GetIntValue(settingKey, value);
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_RINGER_MODE),
            static_cast<int32_t>(ret));
        AUDIO_WARNING_LOG("Failed to write ringer_mode: %{public}d to setting db! Err: %{public}d", ringerMode, ret);
        return false;
    } else {
        ringerMode = static_cast<AudioRingerMode>(value);
    }
    return true;
}

bool VolumeDataMaintainer::SaveSafeStatus(DeviceType deviceType, SafeStatus safeStatus)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = SUCCESS;
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            ret = settingProvider.PutIntValue(AUDIO_SAFE_VOLUME_STATE_BT, static_cast<int32_t>(safeStatus));
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            ret = settingProvider.PutIntValue(AUDIO_SAFE_VOLUME_STATE, static_cast<int32_t>(safeStatus));
            break;
        default:
            AUDIO_WARNING_LOG("the device type not support safe volume");
            return false;
    }
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_SAFE_STATUS),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("device:%{public}d, insert failed, safe status:%{public}d", deviceType, safeStatus);
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::GetSafeStatus(DeviceType deviceType, SafeStatus &safeStatus)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = SUCCESS;
    int32_t value = 0;
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            ret = settingProvider.GetIntValue(AUDIO_SAFE_VOLUME_STATE_BT, value);
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            ret = settingProvider.GetIntValue(AUDIO_SAFE_VOLUME_STATE, value);
            break;
        default:
            WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_SAFE_STATUS_A),
                static_cast<int32_t>(ret));
            AUDIO_WARNING_LOG("the device type not support safe volume");
            return false;
    }
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_SAFE_STATUS_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("device:%{public}d, insert failed, safe status:%{public}d", deviceType, safeStatus);
        return false;
    }
    if (value > static_cast<int32_t>(SAFE_ACTIVE)) {
        value = value - MAX_SAFE_STATUS;
        SaveSafeStatus(deviceType, static_cast<SafeStatus>(value));
    }
    safeStatus = static_cast<SafeStatus>(value);
    return true;
}

bool VolumeDataMaintainer::SaveSafeVolumeTime(DeviceType deviceType, int64_t time)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = SUCCESS;
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            ret = settingProvider.PutLongValue(UNSAFE_VOLUME_MUSIC_ACTIVE_MS_BT, time, "secure");
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            ret = settingProvider.PutLongValue(UNSAFE_VOLUME_MUSIC_ACTIVE_MS, time, "secure");
            break;
        default:
            WriteVolumeDbAccessExceptionEvent(
                static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_SAFE_VOLUME_TIME_A),
                static_cast<int32_t>(ret));
            AUDIO_WARNING_LOG("the device type not support safe volume");
            return false;
    }
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_SAFE_VOLUME_TIME_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("device:%{public}d, insert failed", deviceType);
        return false;
    }

    return true;
}

bool VolumeDataMaintainer::GetSafeVolumeTime(DeviceType deviceType, int64_t &time)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = SUCCESS;
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            ret = settingProvider.GetLongValue(UNSAFE_VOLUME_MUSIC_ACTIVE_MS_BT, time, "secure");
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            ret = settingProvider.GetLongValue(UNSAFE_VOLUME_MUSIC_ACTIVE_MS, time, "secure");
            break;
        default:
            WriteVolumeDbAccessExceptionEvent(
                static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_SAFE_VOLUME_TIME_A),
                static_cast<int32_t>(ret));
            AUDIO_WARNING_LOG("the device type not support safe mode");
            return false;
    }
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_SAFE_VOLUME_TIME_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("device:%{public}d, get safe active time failed", deviceType);
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::SetRestoreVolumeLevel(DeviceType deviceType, int32_t volume)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = SUCCESS;
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            ret = settingProvider.PutIntValue(UNSAFE_VOLUME_LEVEL_BT, volume);
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
        case DEVICE_TYPE_DP:
            ret = settingProvider.PutIntValue(UNSAFE_VOLUME_LEVEL, volume);
            break;
        default:
            WriteVolumeDbAccessExceptionEvent(
                static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SET_RESTORE_VOLUME_LEVEL_A),
                static_cast<int32_t>(ret));
            AUDIO_WARNING_LOG("the device type not support safe volume");
            return false;
    }
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(
            static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SET_RESTORE_VOLUME_LEVEL_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("device:%{public}d, insert failed", deviceType);
        return false;
    }

    return true;
}

bool VolumeDataMaintainer::GetRestoreVolumeLevel(DeviceType deviceType, int32_t &volume)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = SUCCESS;
    int32_t value = 0;
    switch (deviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            ret = settingProvider.GetIntValue(UNSAFE_VOLUME_LEVEL_BT, value);
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
        case DEVICE_TYPE_DP:
            ret = settingProvider.GetIntValue(UNSAFE_VOLUME_LEVEL, value);
            break;
        default:
            WriteVolumeDbAccessExceptionEvent(
                static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_RESTORE_VOLUME_LEVEL_A),
                static_cast<int32_t>(ret));
            AUDIO_WARNING_LOG("the device type not support safe volume");
            return false;
    }
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(
            static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_RESTORE_VOLUME_LEVEL_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("device:%{public}d, insert failed", deviceType);
        return false;
    }
    volume = value;
    return true;
}

bool VolumeDataMaintainer::SaveSystemSoundUrl(const std::string &key, const std::string &value)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = settingProvider.PutStringValue(key, value);
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_SYSTEM_SOUND_URL),
            static_cast<int32_t>(ret));
        AUDIO_WARNING_LOG("Failed to system sound url: %{public}s to setting db! Err: %{public}d", value.c_str(), ret);
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::GetSystemSoundUrl(const std::string &key, std::string &value)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = settingProvider.GetStringValue(key, value);
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_SYSTEM_SOUND_URL),
            static_cast<int32_t>(ret));
        AUDIO_WARNING_LOG("Failed to get systemsoundurl failed Err: %{public}d", ret);
        return false;
    }
    return true;
}

void VolumeDataMaintainer::RegisterCloned()
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    AudioSettingObserver::UpdateFunc updateFunc = [&](const std::string& key) {
        int32_t value = INVALIAD_SETTINGS_CLONE_STATUS;
        ErrCode result =
            AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID).GetIntValue(SETTINGS_CLONED, value);
        if (!isSettingsCloneHaveStarted_ && (value == SETTINGS_CLONING_STATUS) && (result == SUCCESS)) {
            AUDIO_INFO_LOG("clone staring");
            isSettingsCloneHaveStarted_ = true;
        }

        if (isSettingsCloneHaveStarted_ && (value == SETTINGS_CLONED_STATUS) && (result == SUCCESS)) {
            AUDIO_INFO_LOG("Get SETTINGS_CLONED success, clone done, restore.");
            AudioPolicyManagerFactory::GetAudioPolicyManager().DoRestoreData();
            isSettingsCloneHaveStarted_ = false;
        }
    };
    sptr<AudioSettingObserver> observer = settingProvider.CreateObserver(SETTINGS_CLONED, updateFunc);
    ErrCode ret = settingProvider.RegisterObserver(observer);
    if (ret != ERR_OK) {
        AUDIO_ERR_LOG("RegisterObserver failed");
    }
}

bool VolumeDataMaintainer::SaveMicMuteState(bool isMute)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    const std::string settingKey = "micmute_state";
    ErrCode ret = settingProvider.PutBoolValue(settingKey, isMute, "secure", true, AudioSettingProvider::MAIN_USER_ID);
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_MIC_MUTE_STATE),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("Failed to saveMicMuteState: %{public}d to setting db! Err: %{public}d", isMute, ret);
        return false;
    }
    return true;
}

bool VolumeDataMaintainer::GetMicMuteState(bool &isMute)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    const std::string settingKey = "micmute_state";
    ErrCode ret = settingProvider.GetBoolValue(settingKey, isMute, "secure", AudioSettingProvider::MAIN_USER_ID);
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_MIC_MUTE_STATE),
            static_cast<int32_t>(ret));
        AUDIO_WARNING_LOG("Failed to write micmute_state: %{public}d to setting db! Err: %{public}d", isMute, ret);
        return false;
    }

    return true;
}
std::string VolumeDataMaintainer::GetDeviceTypeName(DeviceType deviceType)
{
    std::string type = "";
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
            type = "_earpiece";
            return type;
        case DEVICE_TYPE_SPEAKER:
            type = "_builtin";
            return type;
        case DEVICE_TYPE_DP:
        case DEVICE_TYPE_HDMI:
            type = "_dp";
            return type;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_NEARLINK:
            type = "_wireless";
            return type;
        case DEVICE_TYPE_HEARING_AID:
            type = "_hearing_aid";
            return type;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            type = "_wired";
            return type;
        case DEVICE_TYPE_REMOTE_CAST:
            type = "_remote_cast";
            return type;
        default:
            AUDIO_ERR_LOG("device %{public}d is not supported for dataShare", deviceType);
            return "";
    }
}

std::string VolumeDataMaintainer::GetVolumeKeyForDatabaseVolumeName(
    std::string databaseVolumeName, AudioStreamType streamType)
{
    std::string type = "";
    if (!AUDIO_STREAMTYPE_VOLUME_MAP.count(streamType)) {
        return "";
    }
    type = AUDIO_STREAMTYPE_VOLUME_MAP[streamType];
    if (type == "") {
        AUDIO_ERR_LOG("streamType %{public}d is not supported for datashare", streamType);
        return "";
    }

    return databaseVolumeName + "_" + type;
}

std::string VolumeDataMaintainer::GetMuteKeyForDatabaseVolumeName(
    std::string databaseVolumeName, AudioStreamType streamType)
{
    std::string type = "";
    if (!AUDIO_STREAMTYPE_MUTE_STATUS_MAP.count(streamType)) {
        return "";
    }
    type = AUDIO_STREAMTYPE_MUTE_STATUS_MAP[streamType];
    if (type == "") {
        AUDIO_ERR_LOG("streamType %{public}d is not supported for datashare", streamType);
        return "";
    }

    return databaseVolumeName + "_" + type;
}

std::string VolumeDataMaintainer::GetVolumeKeyForDataShare(DeviceType deviceType, AudioStreamType streamType,
    std::string networkId)
{
    std::string type = "";
    if (!AUDIO_STREAMTYPE_VOLUME_MAP.count(streamType)) {
        return "";
    }
    type = AUDIO_STREAMTYPE_VOLUME_MAP[streamType];
    if (type == "") {
        AUDIO_ERR_LOG("streamType %{public}d is not supported for datashare", streamType);
        return "";
    }

    std::string deviceTypeName = GetDeviceTypeName(deviceType);
    if (deviceTypeName == "") {
        AUDIO_ERR_LOG("device %{public}d is not supported for datashare", deviceType);
        return "";
    }
    if (VolumeUtils::IsPCVolumeEnable() && streamType == AudioStreamType::STREAM_MUSIC &&
        deviceType == DeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
        type = AUDIO_STREAMTYPE_VOLUME_MAP[STREAM_VOICE_CALL];
    }
    if (streamType == AudioStreamType::STREAM_VOICE_ASSISTANT &&
        deviceType == DeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
        deviceTypeName += "_sco";
    }

    if (networkId != "LocalDevice" && deviceType == DEVICE_TYPE_SPEAKER) {
        deviceTypeName += "_distributed";
    }

    if (deviceType == DEVICE_TYPE_DP) {
        deviceTypeName += "_dp";
    }
    return type + deviceTypeName;
}

std::string VolumeDataMaintainer::GetMuteKeyForDataShare(DeviceType deviceType, AudioStreamType streamType,
    std::string networkId)
{
    std::string type = "";
    if (!AUDIO_STREAMTYPE_MUTE_STATUS_MAP.count(streamType)) {
        return "";
    }
    type = AUDIO_STREAMTYPE_MUTE_STATUS_MAP[streamType];
    if (type == "") {
        AUDIO_ERR_LOG("streamType %{public}d is not supported for datashare", streamType);
        return "";
    }

    std::string deviceTypeName = GetDeviceTypeName(deviceType);
    if (deviceTypeName == "") {
        AUDIO_ERR_LOG("device %{public}d is not supported for datashare", deviceType);
        return "";
    }
    if (VolumeUtils::IsPCVolumeEnable() && streamType == AudioStreamType::STREAM_MUSIC &&
        deviceType == DeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
        type = AUDIO_STREAMTYPE_VOLUME_MAP[STREAM_VOICE_CALL];
    }

    if (streamType == AudioStreamType::STREAM_VOICE_ASSISTANT &&
        deviceType == DeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
        deviceTypeName += "_sco";
    }

    if (networkId != "LocalDevice" && deviceType == DEVICE_TYPE_SPEAKER) {
        deviceTypeName += "_distributed";
    }

    if (deviceType == DEVICE_TYPE_DP) {
        deviceTypeName += "_dp";
    }
    return type + deviceTypeName;
}

void VolumeDataMaintainer::SetVolumeDegree(AudioStreamType streamType, int32_t volumeDegree)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    volumeDegreeMap_[streamForVolumeMap] = volumeDegree;
}

int32_t VolumeDataMaintainer::GetVolumeDegree(AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    AudioStreamType streamForVolumeMap = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    return volumeDegreeMap_[streamForVolumeMap];
}

} // namespace AudioStandard
} // namespace OHOS
