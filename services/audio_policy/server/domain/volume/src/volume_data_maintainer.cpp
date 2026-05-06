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
#include "audio_connected_device.h"
#include "audio_bundle_manager.h"
#include <nlohmann/json.hpp>
#include "audio_zone_service.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_SAFE_VOLUME_STATE = "audio_safe_volume_state";
const std::string AUDIO_SAFE_VOLUME_STATE_BT = "audio_safe_volume_state_bt";
const std::string AUDIO_SAFE_VOLUME_STATE_SLE = "audio_safe_volume_state_sle";
const std::string UNSAFE_VOLUME_MUSIC_ACTIVE_MS = "unsafe_volume_music_active_ms";
const std::string UNSAFE_VOLUME_MUSIC_ACTIVE_MS_BT = "unsafe_volume_music_active_ms_bt";
const std::string UNSAFE_VOLUME_MUSIC_ACTIVE_MS_SLE = "unsafe_volume_music_active_ms_sle";
const std::string LEGACY_HIGH_VOLUME_CHECK_TIME_S = "legacy_high_volume_check_time_s";
const std::string UNSAFE_VOLUME_LEVEL = "unsafe_volume_level";
const std::string UNSAFE_VOLUME_LEVEL_BT = "unsafe_volume_level_bt";
const std::string UNSAFE_VOLUME_LEVEL_SLE = "unsafe_volume_level_sle";
const std::string SETTINGS_CLONED = "settingsCloneStatus";
const int32_t INVALIAD_SETTINGS_CLONE_STATUS = -1;
const int32_t SETTINGS_CLONING_STATUS = 1;
const int32_t SETTINGS_CLONED_STATUS = 0;
constexpr int32_t MAX_SAFE_STATUS = 2;
constexpr int32_t DEFAULT_SYSTEM_VOLUME_FOR_EFFECT = 5;
static constexpr int32_t DEFAULT_VOLUME_LEVEL = 7;
static constexpr int32_t DEFAULT_VOLUME_DEGREE = 50;

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
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_REMOTE_CAST
};

static std::map<VolumeDataMaintainer::VolumeDataMaintainerStreamType, AudioStreamType> AUDIO_STREAMTYPE_MAP = {
    {VolumeDataMaintainer::VT_STREAM_ALARM, STREAM_ALARM},
    {VolumeDataMaintainer::VT_STREAM_DTMF, STREAM_DTMF},
    {VolumeDataMaintainer::VT_STREAM_TTS, STREAM_VOICE_ASSISTANT},
    {VolumeDataMaintainer::VT_STREAM_ACCESSIBILITY, STREAM_ACCESSIBILITY},
};

static const std::map<AudioVolumeType, std::string> AUDIO_VOLUME_TYPE_KEY_MAP = {
    {STREAM_MUSIC, "music"},
    {STREAM_RING, "ring"},
    {STREAM_SYSTEM, "system"},
    {STREAM_NOTIFICATION, "notification"},
    {STREAM_ALARM, "alarm"},
    {STREAM_VOICE_CALL, "voice_call"},
    {STREAM_VOICE_ASSISTANT, "voice_assistant"},
    {STREAM_ACCESSIBILITY, "accessibility"},
    {STREAM_ULTRASONIC, "ultrasonic"},
    {STREAM_NAVIGATION, "navigation"},
};

static const std::string VOLUME_LEVEL_KEY = "_volume";
static const std::string VOLUME_DEGREE_KEY = "_degree";
static const std::string MUTE_STATUS_KEY = "_mute_status";

static const std::map<DeviceGroupType, std::string> DEVICE_GROUP_TYPE_KEY_MAP = {
    {DeviceGroupType::EARPIECE, "_earpiece"},
    {DeviceGroupType::BUILT_IN, "_builtin"},
    {DeviceGroupType::WIRED, "_wired"},
    {DeviceGroupType::WIRELESS, "_wireless"},
    {DeviceGroupType::DP, "_dp"},
    {DeviceGroupType::REMOTE_CAST, "_remote_cast"},
    {DeviceGroupType::HEARING_AID, "_hearing_aid"},
    {DeviceGroupType::LINE_DIGITAL, "_line_digital"},
};

bool VolumeDataMaintainer::CheckOsAccountReady()
{
    return AudioSettingProvider::CheckOsAccountReady();
}

void VolumeDataMaintainer::SetDataShareReady(std::atomic<bool> isDataShareReady)
{
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    audioSettingProvider.SetDataShareReady(std::atomic_load(&isDataShareReady));
    AUDIO_INFO_LOG("SetDataShareReady, isDataShareReady: %{public}d", std::atomic_load(&isDataShareReady));
    isDataShareReady_ = isDataShareReady;
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

void VolumeDataMaintainer::SetSystemAppVolume(int32_t appUid, int32_t volumeLevel)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    systemAppVolumeLevelMap_[appUid] = volumeLevel;
}

bool VolumeDataMaintainer::IsSetSystemAppVolume(int32_t appUid)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    return systemAppVolumeLevelMap_.find(appUid) != systemAppVolumeLevelMap_.end();
}

int32_t VolumeDataMaintainer::GetSystemAppVolume(int32_t appUid)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    return systemAppVolumeLevelMap_.find(appUid) != systemAppVolumeLevelMap_.end() ?
        systemAppVolumeLevelMap_[appUid] : MAX_VOLUME_DEGREE;
}

void VolumeDataMaintainer::SetSystemAppVolumeMuted(int32_t appUid, bool muted)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    systemAppVolumeMuteStatusMap_[appUid] = muted;
}

bool VolumeDataMaintainer::IsSystemAppVolumeMuted(int32_t appUid)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    auto it = systemAppVolumeMuteStatusMap_.find(appUid);
    return (it != systemAppVolumeMuteStatusMap_.end()) ? it->second : false;
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
                auto desc = audioConnectedDevice_.GetDeviceByDeviceType(device);
                SaveMuteToDb(desc, AUDIO_STREAMTYPE_MAP[streamtype], true);
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
        case DEVICE_TYPE_NEARLINK:
            ret = settingProvider.PutIntValue(AUDIO_SAFE_VOLUME_STATE_SLE, static_cast<int32_t>(safeStatus));
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
        case DEVICE_TYPE_NEARLINK:
            ret = settingProvider.GetIntValue(AUDIO_SAFE_VOLUME_STATE_SLE, value);
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

bool VolumeDataMaintainer::SaveLegacyHighVolumeTime(int64_t time)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = settingProvider.PutLongValue(LEGACY_HIGH_VOLUME_CHECK_TIME_S, time, "secure");
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "save legacy high volume to db error");
    return true;
}

bool VolumeDataMaintainer::GetLegacyHighVolumeTime(int64_t &time)
{
    AudioSettingProvider& settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = settingProvider.GetLongValue(LEGACY_HIGH_VOLUME_CHECK_TIME_S, time, "secure");
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "get legacy high volume from db failed");
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
        case DEVICE_TYPE_NEARLINK:
            ret = settingProvider.PutLongValue(UNSAFE_VOLUME_MUSIC_ACTIVE_MS_SLE, time, "secure");
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
        case DEVICE_TYPE_NEARLINK:
            ret = settingProvider.GetLongValue(UNSAFE_VOLUME_MUSIC_ACTIVE_MS_SLE, time, "secure");
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
        case DEVICE_TYPE_NEARLINK:
            ret = settingProvider.PutIntValue(UNSAFE_VOLUME_LEVEL_SLE, volume);
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
        case DEVICE_TYPE_NEARLINK:
            ret = settingProvider.GetIntValue(UNSAFE_VOLUME_LEVEL_SLE, value);
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
    if (hasRegisterCloned_) {
        AUDIO_WARNING_LOG("has registered cloned observer");
        return;
    }
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
    } else {
        hasRegisterCloned_ = true;
        AUDIO_INFO_LOG("RegisterObserver success");
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

DeviceGroupType VolumeDataMaintainer::GetDeviceGroupTypeByDeviceType(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
            return DeviceGroupType::EARPIECE;
        case DEVICE_TYPE_SPEAKER:
            return DeviceGroupType::BUILT_IN;
        case DEVICE_TYPE_DP:
        case DEVICE_TYPE_HDMI:
        case DEVICE_TYPE_REMOTE_DAUDIO:
            return DeviceGroupType::DP;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_NEARLINK:
            return DeviceGroupType::WIRELESS;
        case DEVICE_TYPE_HEARING_AID:
            return DeviceGroupType::HEARING_AID;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return DeviceGroupType::WIRED;
        case DEVICE_TYPE_REMOTE_CAST:
            return DeviceGroupType::REMOTE_CAST;
        case DEVICE_TYPE_LINE_DIGITAL:
            return DeviceGroupType::LINE_DIGITAL;
        default:
            AUDIO_ERR_LOG("device %{public}d is invalid!", deviceType);
            return DeviceGroupType::INVALID;
    }
}

std::string VolumeDataMaintainer::GetDeviceGroupTypeKey(DeviceType deviceType)
{
    DeviceGroupType deviceGroupType = GetDeviceGroupTypeByDeviceType(deviceType);
    if (deviceGroupType == DeviceGroupType::INVALID || DEVICE_GROUP_TYPE_KEY_MAP.count(deviceGroupType) == 0) {
        AUDIO_ERR_LOG("device %{public}d is not supported for dataShare", deviceType);
        return "";
    }
    return DEVICE_GROUP_TYPE_KEY_MAP.at(deviceGroupType);
}

std::string VolumeDataMaintainer::GetVolumeValueKey(AudioVolumeType volumeType, VolumeKeyType keyType)
{
    if (AUDIO_VOLUME_TYPE_KEY_MAP.count(volumeType) == 0) {
        AUDIO_ERR_LOG("volumeType %{public}d is not supported for datashare", volumeType);
        return "";
    }
    std::string keyTypeStr = "";
    switch (keyType) {
        case VolumeKeyType::LEVEL:
        case VolumeKeyType::DEGREE: // the degree str is at the end of the string!
            keyTypeStr = VOLUME_LEVEL_KEY;
            break;
        case VolumeKeyType::MUTE:
            keyTypeStr = MUTE_STATUS_KEY;
            break;
        default:
            AUDIO_ERR_LOG("The VolumeKeyType %{public}d is an invalid param!", static_cast<int32_t>(keyType));
            return "";
    }
    return AUDIO_VOLUME_TYPE_KEY_MAP.at(volumeType) + keyTypeStr;
}

std::string VolumeDataMaintainer::GetVolumeKeyForDatabaseVolumeName(
    std::string databaseVolumeName, AudioStreamType streamType)
{
    std::string volumeValueKey = GetVolumeValueKey(streamType, VolumeKeyType::LEVEL);
    if (volumeValueKey == "") {
        AUDIO_ERR_LOG("streamType %{public}d is not supported for datashare", streamType);
        return "";
    }
    return databaseVolumeName + "_" + volumeValueKey;
}

std::string VolumeDataMaintainer::GetMuteKeyForDatabaseVolumeName(
    std::string databaseVolumeName, AudioStreamType streamType)
{
    std::string muteKey = GetVolumeValueKey(streamType, VolumeKeyType::MUTE);
    if (muteKey == "") {
        AUDIO_ERR_LOG("streamType %{public}d is not supported for datashare", streamType);
        return "";
    }
    return databaseVolumeName + "_" + muteKey;
}

std::string VolumeDataMaintainer::GetVolumeKeyForDataShare(DeviceType deviceType, AudioStreamType streamType,
    VolumeKeyType keyType, std::string networkId)
{
    std::string volumeValueKey = GetVolumeValueKey(streamType, keyType);
    std::string deviceKey = GetDeviceGroupTypeKey(deviceType);
    if (volumeValueKey == "" || deviceKey == "") {
        AUDIO_ERR_LOG("The device %{public}d or streamType %{public}d is invalid!", deviceType, streamType);
        return "";
    }

    if (VolumeUtils::IsPCVolumeEnable() && streamType == AudioStreamType::STREAM_MUSIC &&
        deviceType == DeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
        volumeValueKey = AUDIO_VOLUME_TYPE_KEY_MAP.at(STREAM_VOICE_CALL) + VOLUME_LEVEL_KEY;
    }
    if (streamType == AudioStreamType::STREAM_VOICE_ASSISTANT &&
        deviceType == DeviceType::DEVICE_TYPE_BLUETOOTH_SCO) {
        deviceKey += "_sco"; // special
    }

    if (networkId != "LocalDevice" && deviceType == DEVICE_TYPE_SPEAKER) {
        deviceKey += "_distributed";
    }

    return volumeValueKey + deviceKey;
}

std::string VolumeDataMaintainer::GetVolumeValueMapKey(std::shared_ptr<AudioDeviceDescriptor> device, int32_t zoneId)
{
    auto deviceGroupType = GetDeviceGroupTypeByDeviceType(device->getType());
    std::string deviceGroupStr = std::to_string(static_cast<int32_t>(deviceGroupType));
    if (device->deviceType_ == DEVICE_TYPE_SPEAKER && device->networkId_ != LOCAL_NETWORK_ID) {
        deviceGroupStr = "DMSDP"; // distributed speaker.
    }
#ifdef FEATURE_AUDIO_ZONE
    return "zone_" + GetZoneKey(zoneId) + "_deviceGroup_" + deviceGroupStr;
#else
    return "deviceGroup_" + deviceGroupStr;
#endif
}

void VolumeDataMaintainer::SaveSystemVolumeForEffect(DeviceType deviceType, AudioStreamType streamType,
    int32_t volumeLevel)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    deviceTypeToSystemVolumeForEffectMap_[deviceType][streamType] = volumeLevel;
}

int32_t VolumeDataMaintainer::GetSystemVolumeForEffect(DeviceType deviceType, AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeMutex_);
    if (deviceTypeToSystemVolumeForEffectMap_.find(deviceType) != deviceTypeToSystemVolumeForEffectMap_.end() &&
        deviceTypeToSystemVolumeForEffectMap_[deviceType].find(streamType) !=
        deviceTypeToSystemVolumeForEffectMap_[deviceType].end()) {
        return deviceTypeToSystemVolumeForEffectMap_[deviceType][streamType];
    }

    return DEFAULT_SYSTEM_VOLUME_FOR_EFFECT;
}

std::string VolumeDataMaintainer::GetVolumeKey(const std::shared_ptr<AudioDeviceDescriptor> &device,
    AudioStreamType streamType, VolumeKeyType keyType)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, "", "GetVolumeKey device is null");
    std::string ret;
    do {
        if (Util::IsDualToneStreamType(streamType)) {
            ret = GetVolumeKeyForDataShare(DEVICE_TYPE_SPEAKER, streamType, keyType, LOCAL_NETWORK_ID);
            break;
        }
        if (device->volumeBehavior_.isReady && device->volumeBehavior_.databaseVolumeName != "") {
            ret = GetVolumeKeyForDatabaseVolumeName(device->volumeBehavior_.databaseVolumeName, streamType);
            break;
        }
        ret = GetVolumeKeyForDataShare(device->deviceType_, streamType, keyType, device->networkId_);
    } while (false);

    if (keyType == VolumeKeyType::DEGREE && !ret.empty()) {
        ret += VOLUME_DEGREE_KEY;  // special
    }
    return ret;
}

std::string VolumeDataMaintainer::GetZoneKey(int32_t zoneId)
{
    if (zoneId > 0) {
        return AudioZoneService::GetInstance().GetZoneNameById(zoneId);
    }
    return "";
}

std::string VolumeDataMaintainer::GetMuteKey(std::shared_ptr<AudioDeviceDescriptor> device, AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, "", "GetMuteKey device is null");
    if (Util::IsDualToneStreamType(streamType)) {
        return GetVolumeKeyForDataShare(DEVICE_TYPE_SPEAKER, streamType, VolumeKeyType::MUTE, LOCAL_NETWORK_ID);
    }
    if (device->volumeBehavior_.isReady && device->volumeBehavior_.databaseVolumeName != "") {
        return GetMuteKeyForDatabaseVolumeName(device->volumeBehavior_.databaseVolumeName, streamType);
    }
    return GetVolumeKeyForDataShare(device->deviceType_, streamType, VolumeKeyType::MUTE, device->networkId_);
}

void VolumeDataMaintainer::SetVolumeList(std::vector<AudioStreamType> volumeList)
{
    volumeList_ = volumeList;
}

void VolumeDataMaintainer::InitDeviceVolumeMap(std::shared_ptr<AudioDeviceDescriptor> device, int32_t zoneId)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "InitDeviceVolumeMap device is null");
    CHECK_AND_RETURN_LOG(isDataShareReady_, "isDataShareReady_ is false");
    CHECK_AND_RETURN_LOG(!GetLoadFlagFromMap(device, VolumeKeyType::LEVEL, zoneId),
        "The volumes of device %{public}s has been loaded!", device->GetName().c_str());
    LoadDeviceVolumeMapFromDb(device, zoneId);
    AUDIO_INFO_LOG("InitDeviceVolumeMap device %{public}s", device->GetName().c_str());
}

void VolumeDataMaintainer::InitZoneVolumeMap(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(zoneId > 0, "InitZoneVolumeMap zoneId is invalid");
    if (desc != nullptr && desc->deviceType_ != DEVICE_TYPE_NONE && desc->deviceType_ != DEVICE_TYPE_INVALID) {
        InitDeviceVolumeMap(desc, zoneId);
        AUDIO_INFO_LOG("InitZoneVolumeMap zoneId %{public}d, deviceType %{public}d", zoneId, desc->deviceType_);
        return;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        AudioZoneService::GetInstance().GetAllOutputDevices(zoneId);
    for (auto &desc : descs) {
        InitDeviceVolumeMap(desc, zoneId);
    }
    AUDIO_INFO_LOG("InitZoneVolumeMap zoneId %{public}d", zoneId);
}

void VolumeDataMaintainer::LoadDeviceVolumeMapFromDb(std::shared_ptr<AudioDeviceDescriptor> device, int32_t zoneId)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "LoadDeviceVolumeMapFromDb device is null");
    AUDIO_INFO_LOG("LoadDeviceVolumeMapFromDb device %{public}s", device->GetName().c_str());
    std::vector<IntValueInfo> infos;
    std::vector<IntValueInfo> degreeInfos;
    std::vector<AudioStreamType> volumeList = volumeList_;
    if (AudioVolumeUtils::GetInstance().IsDistributedDevice(device)) {
        volumeList = DISTRIBUTED_VOLUME_TYPE_LIST;
    }
    for (auto stream : volumeList) {
        IntValueInfo info = GetVolumeLevelInfo(device, stream, zoneId);
        infos.push_back(info);

        IntValueInfo degreeInfo = GetVolumeDegreeInfo(device, stream, zoneId);
        degreeInfos.push_back(degreeInfo);
        AUDIO_INFO_LOG("Load %{public}s dftValue %{public}d",
            info.key.c_str(), info.defaultValue);
    }

    bool readDb = false;
    if (AudioVolumeUtils::GetInstance().IsDistributedDevice(device)) {
        if (device->volumeBehavior_.isReady && device->volumeBehavior_.databaseVolumeName != "") {
            readDb = true;
        }
    } else {
        readDb = true;
    }
    if (readDb) {
        std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
        AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        audioSettingProvider.GetIntValues(infos, "system");
        audioSettingProvider.GetIntValues(degreeInfos, "system");
    }

    bool loadFlag = true;
    for (size_t i = 0; i < volumeList.size(); i++) {
        int32_t validDegree  = VolumeUtils::VolumeDegreeToLevel(degreeInfos[i].value,
            infos[i].maxValue) == infos[i].value ? degreeInfos[i].value : -1;
        VolumeScale volume{infos[i].value, validDegree, infos[i].maxValue};
        SaveVolumeToMap(device, volumeList[i], volume, zoneId);
        loadFlag = loadFlag && infos[i].loadFlag;
    }
    SaveLoadFlagToMap(device, VolumeKeyType::LEVEL, loadFlag, zoneId);

    CheckNotificationVolumeState(device, zoneId);
}

void VolumeDataMaintainer::CheckNotificationVolumeState(std::shared_ptr<AudioDeviceDescriptor> device, int32_t zoneId)
{
    // There is no notification volume on PC. (only all and system)
    CHECK_AND_RETURN(!VolumeUtils::IsPCVolumeEnable());

    // If the user upgrades from the version without notification volume,
    // it is necessary to check whether the initial value of the notification volume is correct.
    if (CheckVolumeState(device, STREAM_NOTIFICATION) != SUCCESS) {
        // There is no notification volume value. Use ringtone volume as the initial value.
        int32_t ringtoneVolumeLevel = LoadVolumeFromMap(device, STREAM_RING, zoneId);
        int32_t ringtoneVolumeDegree = LoadVolumeDegreeFromMap(device, STREAM_RING, zoneId);
        VolumeScale ringtoneVolume{ringtoneVolumeLevel, ringtoneVolumeDegree};
        SaveVolumeToMap(device, STREAM_NOTIFICATION, ringtoneVolume, zoneId);
        SaveVolumeToDb(device, STREAM_NOTIFICATION, ringtoneVolume, zoneId);
    }
}

IntValueInfo VolumeDataMaintainer::GetVolumeLevelInfo(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t zoneId)
{
    int32_t dftVolume = AudioVolumeUtils::GetInstance().GetDefaultVolumeLevel(device, streamType, zoneId);
    int32_t maxVolume = AudioVolumeUtils::GetInstance().GetMaxVolumeLevel(device, streamType, zoneId);
    IntValueInfo info {
#ifdef FEATURE_AUDIO_ZONE
        .key = GetVolumeKey(device, streamType) + GetZoneKey(zoneId),
#else
        .key = GetVolumeKey(device, streamType),
#endif
        .defaultValue = dftVolume,
        .value = dftVolume,
        .maxValue = maxVolume,
        .loadFlag = false,
    };
    return info;
}

IntValueInfo VolumeDataMaintainer::GetVolumeDegreeInfo(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t zoneId)
{
    IntValueInfo degreeInfo {
#ifdef FEATURE_AUDIO_ZONE
        .key = GetVolumeKey(device, streamType, VolumeKeyType::DEGREE) + GetZoneKey(zoneId),
#else
        .key = GetVolumeKey(device, streamType, VolumeKeyType::DEGREE),
#endif
        .defaultValue = -1,
        .value = -1,
        .maxValue = MAX_VOLUME_DEGREE,
        .loadFlag = false,
    };
    return degreeInfo;
}

int32_t VolumeDataMaintainer::SaveVolumeToDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, const VolumeScale &volume, int32_t zoneId)
{
    int32_t volumeLevel = volume.VolumeLevel();
    int32_t volumeDegree = volume.VolumeDegree();
    CHECK_AND_RETURN_RET_LOG(volumeLevel >= 0 && volumeDegree >= 0, ERR_INVALID_PARAM,
        "invalid input, volumeLevel: %{public}d, volumeDegree: %{public}d", volumeLevel, volumeDegree);
    return SaveVolumeToDbInner(device, streamType, volume, zoneId);
}

int32_t VolumeDataMaintainer::SaveVolumeToDbInner(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, const VolumeScale &volume, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "SaveVolumeToDb device is null");
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (AudioVolumeUtils::GetInstance().IsDistributedDevice(device)) {
        if (!device->volumeBehavior_.isReady) {
            return SUCCESS;
        }
        if (device->volumeBehavior_.databaseVolumeName == "") {
            return SUCCESS;
        }
    }
#ifdef FEATURE_AUDIO_ZONE
    std::string volumeKey = GetVolumeKey(device, volumeType) + GetZoneKey(zoneId);
    std::string volumeDegreeKey = GetVolumeKey(device, volumeType, VolumeKeyType::DEGREE)
        + GetZoneKey(zoneId);
#else
    std::string volumeKey = GetVolumeKey(device, volumeType);
    std::string volumeDegreeKey = GetVolumeKey(device, volumeType, VolumeKeyType::DEGREE);
#endif
    if (!volumeKey.compare("")) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(VolumeDbAccessExceptionFuncId::SAVE_VOLUME_INTERNA_A),
            ERR_READ_FAILED);
        AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] is not supported for datashare",
            device->GetName().c_str(), streamType);
        return ERROR;
    }

    {
        std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
        AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        ErrCode ret = audioSettingProvider.PutIntValue(volumeKey, volume.VolumeLevel(), "system");
        ErrCode ret2 = audioSettingProvider.PutIntValue(volumeDegreeKey, volume.VolumeDegree(), "system");
        if (ret != SUCCESS || ret2 != SUCCESS) {
            WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(
                VolumeDbAccessExceptionFuncId::SAVE_VOLUME_INTERNA_B), static_cast<int32_t>(ret));
            AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] Save volume to datashare failed, " \
                "ret %{public}d, ret2 %{public}d",
                device->GetName().c_str(), streamType, ret, ret2);
            return ERROR;
        }
        AUDIO_INFO_LOG("[device %{public}s, streamType %{public}d]"\
            "Save volume to datashare success, volumeLevel %{public}d, volumeDegree %{public}d",
            device->GetName().c_str(), streamType, volume.VolumeLevel(), volume.VolumeDegree());
    }
    return SUCCESS;
}

void VolumeDataMaintainer::SaveVolumeToMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, VolumeScale &volume, int32_t zoneId)
{
    int32_t volumeLevel = volume.VolumeLevel();
    int32_t volumeDegree = volume.VolumeDegree();
    CHECK_AND_RETURN_LOG(volumeLevel >= 0 && volumeDegree >= 0,
        "invalid input, volumeLevel: %{public}d, volumeDegree: %{public}d", volumeLevel, volumeDegree);
    SaveVolumeToMapInner(device, streamType, volume, zoneId);
}

void VolumeDataMaintainer::SaveVolumeToMapInner(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, VolumeScale &volume, int32_t zoneId)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "SaveVolumeToMap device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(volumeType)) {
        device = ringerDevice_;
    }

    std::string volumeKey = GetVolumeValueMapKey(device, zoneId);
    auto &volumeLevel = volumeValueMap_[volumeKey][volumeType].volumeLevel_;
    auto &volumeDegree = volumeValueMap_[volumeKey][volumeType].volumeDegree_;

    volume.Adjust({volumeLevel, volumeDegree});
    volumeLevel = volume.VolumeLevel();
    volumeDegree = volume.VolumeDegree();
    AUDIO_INFO_LOG("[zoneId %{public}d, device %{public}s, streamType %{public}d]"\
        "Save volume success, volumeLevel %{public}d, volumeDegree %{public}d",
        zoneId, device->GetName().c_str(), volumeType, volumeLevel, volumeDegree);
}

void VolumeDataMaintainer::SaveLoadFlagToMap(std::shared_ptr<AudioDeviceDescriptor> device,
    VolumeKeyType type, bool loadFlag, int32_t zoneId)
{
    std::lock_guard<ffrt::mutex> lock(loadFlagMutex_);
    std::string volumeKey = GetVolumeValueMapKey(device, zoneId);
    AUDIO_INFO_LOG("volumeKey %{public}s, type %{public}d, loadFlag %{public}d",
        volumeKey.c_str(), static_cast<int32_t>(type), loadFlag);
    loadFlagMap_[volumeKey][type] = loadFlag;
}

bool VolumeDataMaintainer::GetLoadFlagFromMap(std::shared_ptr<AudioDeviceDescriptor> device,
    VolumeKeyType type, int32_t zoneId)
{
    std::lock_guard<ffrt::mutex> lock(loadFlagMutex_);
    std::string volumeKey = GetVolumeValueMapKey(device, zoneId);
    if (loadFlagMap_.count(volumeKey) == 0 || loadFlagMap_[volumeKey].count(type) == 0) {
        AUDIO_INFO_LOG("Not found: volumeKey %{public}s, type %{public}d",
            volumeKey.c_str(), static_cast<int32_t>(type));
        return false;
    }
    AUDIO_INFO_LOG("volumeKey %{public}s, type %{public}d, loadFlag %{public}d",
        volumeKey.c_str(), static_cast<int32_t>(type), loadFlagMap_[volumeKey][type]);
    return loadFlagMap_[volumeKey][type];
}

int32_t VolumeDataMaintainer::LoadVolumeFromMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, DEFAULT_VOLUME_LEVEL, "LoadVolumeFromMap device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);

    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(volumeType)) {
        device = ringerDevice_;
    }
    int32_t defaultVolume = DEFAULT_VOLUME_LEVEL;
    CHECK_AND_RETURN_RET_LOG(device != nullptr, defaultVolume, "LoadVolumeFromMap device is null");
    if (volumeType == STREAM_ALL) {
        AUDIO_INFO_LOG("replace stream all to music");
        volumeType = STREAM_MUSIC;
    }
    std::string volumeKey = GetVolumeValueMapKey(device, zoneId);
    CHECK_AND_RETURN_RET_LOG(volumeValueMap_.contains(volumeKey), defaultVolume,
        "key %{public}s not in map", volumeKey.c_str());
    CHECK_AND_RETURN_RET_LOG(volumeValueMap_[volumeKey].contains(volumeType), defaultVolume,
        "key %{public}s stream %{public}d not in map", volumeKey.c_str(), volumeType);
    AUDIO_DEBUG_LOG("volumeKey: %{public}s, volumeType: %{public}d, volumeLevel: %{public}d",
        volumeKey.c_str(), volumeType, volumeValueMap_[volumeKey][volumeType].volumeLevel_);
    return volumeValueMap_[volumeKey][volumeType].volumeLevel_;
}

void VolumeDataMaintainer::InitDeviceMuteMap(std::shared_ptr<AudioDeviceDescriptor> device, int32_t zoneId)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "InitDeviceMuteMap device is null");
    CHECK_AND_RETURN_LOG(isDataShareReady_, "isDataShareReady_ is false");
    CHECK_AND_RETURN_LOG(!GetLoadFlagFromMap(device, VolumeKeyType::MUTE, zoneId),
        "The mute stats of device %{public}s has been loaded!", device->GetName().c_str());
    LoadDeviceMuteMapFromDb(device, zoneId);
    AUDIO_INFO_LOG("InitDeviceMuteMap device %{public}s", device->GetName().c_str());
}

void VolumeDataMaintainer::InitZoneMuteMap(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_LOG(isDataShareReady_, "isDataShareReady_ is false");
    if (desc != nullptr && desc->deviceType_ != DEVICE_TYPE_NONE && desc->deviceType_ != DEVICE_TYPE_INVALID) {
        InitDeviceMuteMap(desc, zoneId);
        AUDIO_INFO_LOG("InitZoneMuteMap zoneId %{public}d, deviceType %{public}d", zoneId, desc->deviceType_);
        return;
    }
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
        AudioZoneService::GetInstance().GetAllOutputDevices(zoneId);
    for (auto &desc : descs) {
        InitDeviceMuteMap(desc, zoneId);
    }
    AUDIO_INFO_LOG("InitZoneMuteMap zoneId %{public}d", zoneId);
}

void VolumeDataMaintainer::LoadDeviceMuteMapFromDb(std::shared_ptr<AudioDeviceDescriptor> device, int32_t zoneId)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "LoadDeviceMuteMapFromDb device is null");
    AUDIO_INFO_LOG("LoadDeviceMuteMapFromDb device %{public}s", device->GetName().c_str());
    std::vector<BoolValueInfo> infos;
    std::vector<AudioStreamType> volumeList = volumeList_;
    if (AudioVolumeUtils::GetInstance().IsDistributedDevice(device)) {
        volumeList = DISTRIBUTED_VOLUME_TYPE_LIST;
    }
    for (auto stream : volumeList) {
        BoolValueInfo info {
#ifdef FEATURE_AUDIO_ZONE
            .key = GetMuteKey(device, stream) + GetZoneKey(zoneId),
#else
            .key = GetMuteKey(device, stream),
#endif
            .defaultValue = false,
            .value = false,
            .loadFlag = false,
        };
        infos.push_back(info);
        AUDIO_INFO_LOG("Load mute by key: %{public}s", info.key.c_str());
    }

    bool readDb = false;
    if (AudioVolumeUtils::GetInstance().IsDistributedDevice(device)) {
        if (device->volumeBehavior_.isReady && device->volumeBehavior_.databaseVolumeName != "") {
            readDb = true;
        }
    } else {
        readDb = true;
    }
    if (readDb) {
        std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
        AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        audioSettingProvider.GetBoolValues(infos, "system");
    }

    bool loadFlag = true;
    for (size_t i = 0; i < volumeList.size(); i++) {
        SaveMuteToMap(device, volumeList[i], infos[i].value);
        loadFlag = loadFlag && infos[i].loadFlag;
    }
    SaveLoadFlagToMap(device, VolumeKeyType::MUTE, loadFlag, zoneId);

    // If the user upgrades from the version without notification volume,
    // it is necessary to check whether the initial value of the notification mute status is correct.
    if (CheckMuteState(device, STREAM_NOTIFICATION) != SUCCESS) {
        // There is no notification mute status value. Use ringtone volume as the initial value.
        bool ringtoneMute = LoadMuteFromMap(device, STREAM_RING, zoneId);
        SaveMuteToMap(device, STREAM_NOTIFICATION, ringtoneMute, zoneId);
        SaveMuteToDb(device, STREAM_NOTIFICATION, ringtoneMute, zoneId);
    }
}

int32_t VolumeDataMaintainer::SaveMuteToDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, bool muteStatus, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is null");
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (AudioVolumeUtils::GetInstance().IsDistributedDevice(device)) {
        if (!device->volumeBehavior_.isReady) {
            return SUCCESS;
        }
        if (device->volumeBehavior_.databaseVolumeName == "") {
            return SUCCESS;
        }
    }
#ifdef FEATURE_AUDIO_ZONE
    std::string muteKey = GetMuteKey(device, volumeType) + GetZoneKey(zoneId);
#else
    std::string muteKey = GetMuteKey(device, volumeType);
#endif
    if (!muteKey.compare("")) {
        WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(
            VolumeDbAccessExceptionFuncId::SAVE_MUTE_STATUS_INTERNAL), ERR_READ_FAILED);
        AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] is not supported for datashare",
            device->GetName().c_str(), streamType);
        return ERROR;
    }

    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.PutBoolValue(muteKey, muteStatus, "system");
    AUDIO_INFO_LOG("muteKey:%{public}s, muteStatus:%{public}d, res: %{public}d",
        muteKey.c_str(), muteStatus, ret);
    return ret;
}

void VolumeDataMaintainer::SaveMuteToMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, bool muteStatus, int32_t zoneId)
{
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(volumeType)) {
        device = ringerDevice_;
    }

    std::string volumeKey = GetVolumeValueMapKey(device, zoneId);
    volumeValueMap_[volumeKey][volumeType].mute_ = muteStatus;
    AUDIO_INFO_LOG("SaveMuteToMap zoneId %{public}d device %{public}s streamType %{public}d muteStatus %{public}d",
        zoneId, device->GetName().c_str(), streamType, muteStatus);
}

bool VolumeDataMaintainer::LoadMuteFromMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t zoneId)
{
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, false, "device is null");
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(volumeType)) {
        device = ringerDevice_;
    }

    std::string volumeKey = GetVolumeValueMapKey(device, zoneId);
    CHECK_AND_RETURN_RET_LOG(volumeValueMap_.contains(volumeKey), false,
        "key %{public}s not in muteStatusMap_", volumeKey.c_str());
    CHECK_AND_RETURN_RET_LOG(volumeValueMap_[volumeKey].contains(volumeType), false,
        "key %{public}s volumeType %{public}d not in muteStatusMap_", volumeKey.c_str(), volumeType);
    AUDIO_INFO_LOG("volumeKey: %{public}s, volumeType: %{public}d, muteStatus: %{public}d",
        volumeKey.c_str(), volumeType, volumeValueMap_[volumeKey][volumeType].mute_);
    return volumeValueMap_[volumeKey][volumeType].mute_;
}

// open for speical need
int32_t VolumeDataMaintainer::LoadVolumeFromDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is null");
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    int32_t volumeLevel = 0;
    std::string volumeKey = GetVolumeKey(device, volumeType);
    if (!volumeKey.compare("")) {
        WriteVolumeDbAccessExceptionEvent(
            static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_VOLUME_INTERNAL_A),
            ERR_READ_FAILED);
        AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] is not supported for "\
            "datashare", device->GetName().c_str(), streamType);
        return volumeLevel;
    }

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.GetIntValue(volumeKey, volumeLevel, "system");
    if (ret != SUCCESS) {
        WriteVolumeDbAccessExceptionEvent(
            static_cast<int32_t>(VolumeDbAccessExceptionFuncId::GET_VOLUME_INTERNAL_B),
            static_cast<int32_t>(ret));
        AUDIO_ERR_LOG("Get volumeLevel From DataBase failed");
        return 0;
    } else {
        AUDIO_INFO_LOG("Get volumeLevel From DataBase volumeLevel from datashare %{public}d", volumeLevel);
    }
    return volumeLevel;
}

int32_t VolumeDataMaintainer::LoadVolumeDegreeFromMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t zoneId)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, DEFAULT_VOLUME_DEGREE, "param device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(volumeType)) {
        device = ringerDevice_;
    }
    int32_t defaultVolumeDegree = DEFAULT_VOLUME_DEGREE;
    CHECK_AND_RETURN_RET_LOG(device != nullptr, defaultVolumeDegree, "device is null");
    if (volumeType == STREAM_ALL) {
        AUDIO_INFO_LOG("replace stream all to music");
        volumeType = STREAM_MUSIC;
    }
    std::string volumeKey = GetVolumeValueMapKey(device, zoneId);
    CHECK_AND_RETURN_RET_LOG(volumeValueMap_.contains(volumeKey), defaultVolumeDegree,
        "device %{public}s not in map", device->GetName().c_str());
    CHECK_AND_RETURN_RET_LOG(volumeValueMap_[volumeKey].contains(volumeType), defaultVolumeDegree,
        "device %{public}s stream %{public}d not in map", device->GetName().c_str(), volumeType);
    AUDIO_INFO_LOG("[device %{public}s, streamType %{public}d] volumeDegree %{public}d",
        device->GetName().c_str(), volumeType, volumeValueMap_[volumeKey][volumeType].volumeDegree_);
    return volumeValueMap_[volumeKey][volumeType].volumeDegree_;
}

int32_t VolumeDataMaintainer::LoadVolumeDegreeFromDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is null");
    int32_t volumeDegree = 0;
    std::string volumeKey = GetVolumeKey(device, streamType, VolumeKeyType::DEGREE);
    if (!volumeKey.compare("")) {
        AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] is not supported",
            device->GetName().c_str(), streamType);
        return volumeDegree;
    }

    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = audioSettingProvider.GetIntValue(volumeKey, volumeDegree, "system");
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Get volumeDegree From DataBase failed");
        return 0;
    } else {
        AUDIO_DEBUG_LOG("Get volumeDegree From DataBase, volumeDegree:%{public}d", volumeDegree);
    }
    return volumeDegree;
}

int32_t VolumeDataMaintainer::CheckVolumeState(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    std::string volumeKey = GetVolumeKey(device, streamType);
    CHECK_AND_RETURN_RET_LOG(volumeKey != "", ERROR, "volumeKey is empty");
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    int32_t volumeState = 0;
    return audioSettingProvider.GetIntValue(volumeKey, volumeState, "system");
}

int32_t VolumeDataMaintainer::CheckMuteState(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    std::string muteKey = GetMuteKey(device, streamType);
    CHECK_AND_RETURN_RET_LOG(muteKey != "", ERROR, "muteKey is empty");
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    bool muteState = false;
    return audioSettingProvider.GetBoolValue(muteKey, muteState, "system");
}

int32_t VolumeDataMaintainer::SaveSystemAppVolumeLevelToDb()
{
    std::unordered_map<std::string, int32_t> tempMap;
    std::unordered_map<int32_t, int32_t> localSystemAppVolumeLevelMap;
    {
        std::lock_guard<ffrt::mutex> lock(volumeMutex_);
        CHECK_AND_RETURN_RET(!systemAppVolumeLevelMap_.empty(), SUCCESS);
        localSystemAppVolumeLevelMap = systemAppVolumeLevelMap_;
    }
    for (const auto &[uid, level] : localSystemAppVolumeLevelMap) {
        std::string bundleName = AudioBundleManager::GetBundleNameFromUid(uid);
        CHECK_AND_CONTINUE_LOG(!bundleName.empty(), "bundleName is empty");
        CHECK_AND_CONTINUE(level != MAX_VOLUME_DEGREE);
        tempMap[bundleName] = level;
    }
    nlohmann::json j = tempMap;
    std::string jsonStr = j.dump();
    std::string key = "app_volume_level";
    {
        std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
        AudioSettingProvider &audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        ErrCode ret = audioSettingProvider.PutStringValue(key, jsonStr, "system");
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Save system app volume level failed");
    }
    return SUCCESS;
}

int32_t VolumeDataMaintainer::LoadSystemAppVolumeLevelFromDb(
    std::unordered_map<std::string, int32_t> &systemAppVolumeLevelMap)
{
    std::string key = "app_volume_level";
    std::string jsonStr = "";
    {
        std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
        AudioSettingProvider &audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        ErrCode ret = audioSettingProvider.GetStringValue(key, jsonStr, "system");
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_READ_FAILED, "Get system app volume level failed");
    }
    if (!jsonStr.empty()) {
        nlohmann::json j = nlohmann::json::parse(jsonStr, nullptr, false);
        CHECK_AND_RETURN_RET_LOG(!j.is_discarded() && j.is_object(), ERROR, "Invalid json string");
        for (auto &[key, value] : j.items()) {
            if (value.is_number_integer()) {
                systemAppVolumeLevelMap.emplace(key, value.get<int32_t>());
            }
        }
    }
    return SUCCESS;
}

int32_t VolumeDataMaintainer::SaveSystemAppVolumeMuteStatusToDb()
{
    std::unordered_map<std::string, bool> tempMap;
    std::unordered_map<int32_t, bool> localSystemAppVolumeMuteStatusMap;
    {
        std::lock_guard<ffrt::mutex> lock(volumeMutex_);
        CHECK_AND_RETURN_RET(!systemAppVolumeMuteStatusMap_.empty(), SUCCESS);
        localSystemAppVolumeMuteStatusMap = systemAppVolumeMuteStatusMap_;
    }
    for (const auto &[uid, muteStatus] : localSystemAppVolumeMuteStatusMap) {
        std::string bundleName = AudioBundleManager::GetBundleNameFromUid(uid);
        CHECK_AND_CONTINUE_LOG(!bundleName.empty(), "bundleName is empty");
        tempMap[bundleName] = muteStatus;
    }
    nlohmann::json j = tempMap;
    std::string jsonStr = j.dump();
    std::string key = "app_volume_mute_status";
    {
        std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
        AudioSettingProvider &audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        ErrCode ret = audioSettingProvider.PutStringValue(key, jsonStr, "system");
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "Save system app volume mute status failed");
    }
    return SUCCESS;
}

int32_t VolumeDataMaintainer::LoadSystemAppVolumeMuteStatusFromDb(
    std::unordered_map<std::string, bool> &systemAppVolumeMuteStatusMap)
{
    std::string key = "app_volume_mute_status";
    std::string jsonStr = "";
    {
        std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
        AudioSettingProvider &audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        ErrCode ret = audioSettingProvider.GetStringValue(key, jsonStr, "system");
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_READ_FAILED, "Get system app volume mute status failed");
    }
    if (!jsonStr.empty()) {
        nlohmann::json j = nlohmann::json::parse(jsonStr, nullptr, false);
        CHECK_AND_RETURN_RET_LOG(!j.is_discarded() && j.is_object(), ERROR, "Invalid json string");
        for (const auto &[key, value] : j.items()) {
            if (value.is_boolean()) {
                systemAppVolumeMuteStatusMap.emplace(key, value.get<bool>());
            }
        }
    }
    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
