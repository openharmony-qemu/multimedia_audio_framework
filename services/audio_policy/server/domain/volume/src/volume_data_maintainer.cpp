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

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_SAFE_VOLUME_STATE = "audio_safe_volume_state";
const std::string AUDIO_SAFE_VOLUME_STATE_BT = "audio_safe_volume_state_bt";
const std::string AUDIO_SAFE_VOLUME_STATE_SLE = "audio_safe_volume_state_sle";
const std::string UNSAFE_VOLUME_MUSIC_ACTIVE_MS = "unsafe_volume_music_active_ms";
const std::string UNSAFE_VOLUME_MUSIC_ACTIVE_MS_BT = "unsafe_volume_music_active_ms_bt";
const std::string UNSAFE_VOLUME_MUSIC_ACTIVE_MS_SLE = "unsafe_volume_music_active_ms_sle";
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

bool VolumeDataMaintainer::CheckOsAccountReady()
{
    return AudioSettingProvider::CheckOsAccountReady();
}

void VolumeDataMaintainer::SetDataShareReady(std::atomic<bool> isDataShareReady)
{
    AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    audioSettingProvider.SetDataShareReady(std::atomic_load(&isDataShareReady));
    AUDIO_INFO_LOG("SetDataShareReady, isDataShareReady: %{public}d", std::atomic_load(&isDataShareReady));
    if (isDataShareReady) {
        auto descs = audioConnectedDevice_.GetCopy();
        for (auto &desc : descs) {
            CHECK_AND_CONTINUE(desc != nullptr);
            InitDeviceVolumeMap(desc);
            InitDeviceMuteMap(desc);
        }
    }
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
        case DEVICE_TYPE_LINE_DIGITAL:
            type = "_line_digital";
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

std::string VolumeDataMaintainer::GetVolumeKey(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, "", "GetVolumeKey device is null");
    if (Util::IsDualToneStreamType(streamType)) {
        return GetVolumeKeyForDataShare(DEVICE_TYPE_SPEAKER, streamType, LOCAL_NETWORK_ID);
    }
    if (device->volumeBehavior_.isReady && device->volumeBehavior_.databaseVolumeName != "") {
        return GetVolumeKeyForDatabaseVolumeName(device->volumeBehavior_.databaseVolumeName, streamType);
    }
    return GetVolumeKeyForDataShare(device->deviceType_, streamType, device->networkId_);
}

std::string VolumeDataMaintainer::GetMuteKey(std::shared_ptr<AudioDeviceDescriptor> device, AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, "", "GetMuteKey device is null");
    if (Util::IsDualToneStreamType(streamType)) {
        return GetMuteKeyForDataShare(DEVICE_TYPE_SPEAKER, streamType, LOCAL_NETWORK_ID);
    }
    if (device->volumeBehavior_.isReady && device->volumeBehavior_.databaseVolumeName != "") {
        return GetMuteKeyForDatabaseVolumeName(device->volumeBehavior_.databaseVolumeName, streamType);
    }
    return GetMuteKeyForDataShare(device->deviceType_, streamType, device->networkId_);
}

void VolumeDataMaintainer::SetVolumeList(std::vector<AudioStreamType> volumeList)
{
    volumeList_ = volumeList;
}

void VolumeDataMaintainer::InitDeviceVolumeMap(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "InitDeviceVolumeMap device is null");
    LoadDeviceVolumeMapFromDb(device);
    AUDIO_INFO_LOG("InitDeviceVolumeMap device %{public}s", device->GetName().c_str());
}

void VolumeDataMaintainer::DeInitDeviceVolumeMap(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "DeInitDeviceVolumeMap device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    volumeLevelMap_.erase(device->GetName());
    AUDIO_INFO_LOG("DeInitDeviceVolumeMap device %{public}s", device->GetName().c_str());
}

void VolumeDataMaintainer::LoadDeviceVolumeMapFromDb(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "LoadDeviceVolumeMapFromDb device is null");
    AUDIO_INFO_LOG("LoadDeviceVolumeMapFromDb device %{public}s", device->GetName().c_str());
    std::vector<IntValueInfo> infos;
    std::vector<AudioStreamType> volumeList = volumeList_;
    if (AudioVolumeUtils::GetInstance().IsDistributedDevice(device)) {
        volumeList = DISTRIBUTED_VOLUME_TYPE_LIST;
    }
    for (auto stream : volumeList) {
        int32_t dftVolume = AudioVolumeUtils::GetInstance().GetDefaultVolumeLevel(device, stream);
        int32_t maxVolume = AudioVolumeUtils::GetInstance().GetMaxVolumeLevel(device, stream);
        IntValueInfo info {
            .key = GetVolumeKey(device, stream),
            .defaultValue = dftVolume,
            .value = dftVolume,
            .maxValue = maxVolume,
        };
        infos.push_back(info);
        AUDIO_INFO_LOG("Load %{public}s dftValue %{public}d", info.key.c_str(), dftVolume);
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
    }
    for (size_t i = 0; i < volumeList.size(); i++) {
        SaveVolumeToMap(device, volumeList[i], infos[i].value);
        int32_t volumeDegree = VolumeUtils::VolumeLevelToDegree(infos[i].value, infos[i].maxValue);
        SaveVolumeDegreeToMap(device, volumeList[i], volumeDegree);
    }
}

int32_t VolumeDataMaintainer::SaveVolumeToDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t volumeLevel)
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
    std::string volumeKey = GetVolumeKey(device, streamType);
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
        ErrCode ret = audioSettingProvider.PutIntValue(volumeKey, volumeLevel, "system");
        if (ret != SUCCESS) {
            WriteVolumeDbAccessExceptionEvent(static_cast<int32_t>(
                VolumeDbAccessExceptionFuncId::SAVE_VOLUME_INTERNA_B), static_cast<int32_t>(ret));
            AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] Save volume to datashare failed, ret %{public}d",
                device->GetName().c_str(), streamType, ret);
            return ERROR;
        }
        AUDIO_INFO_LOG("[device %{public}s, streamType %{public}d]"\
            "Save volume to datashare success, volumeLevel %{public}d",
            device->GetName().c_str(), streamType, volumeLevel);
    }
    return SUCCESS;
}
void VolumeDataMaintainer::SaveVolumeToMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t volumeLevel)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "SaveVolumeToMap device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(streamType)) {
        device = ringerDevice_;
    }
    volumeLevelMap_[device->GetName()][volumeType] = volumeLevel;
    AUDIO_INFO_LOG("[device %{public}s, streamType %{public}d]"\
        "Save volume to volumeLevelMap success, volumeLevel %{public}d",
        device->GetName().c_str(), volumeType, volumeLevel);
}

int32_t VolumeDataMaintainer::LoadVolumeFromMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(device != nullptr, DEFAULT_VOLUME_LEVEL, "LoadVolumeFromMap device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);

    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(streamType)) {
        device = ringerDevice_;
    }
    int32_t defaultVolume = DEFAULT_VOLUME_LEVEL;
    CHECK_AND_RETURN_RET_LOG(device != nullptr, defaultVolume, "LoadVolumeFromMap device is null");
    if (volumeType == STREAM_ALL) {
        AUDIO_INFO_LOG("replace stream all to music");
        volumeType = STREAM_MUSIC;
    }
    CHECK_AND_RETURN_RET_LOG(volumeLevelMap_.contains(device->GetName()), defaultVolume,
        "device %{public}s not in map", device->GetName().c_str());
    CHECK_AND_RETURN_RET_LOG(volumeLevelMap_[device->GetName()].contains(volumeType), defaultVolume,
        "device %{public}s stream %{public}d not in map", device->GetName().c_str(), volumeType);
    return volumeLevelMap_[device->GetName()][volumeType];
}

void VolumeDataMaintainer::InitDeviceMuteMap(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "InitDeviceMuteMap device is null");
    LoadDeviceMuteMapFromDb(device);
    AUDIO_INFO_LOG("InitDeviceMuteMap device %{public}s", device->GetName().c_str());
}

void VolumeDataMaintainer::DeInitDeviceMuteMap(std::shared_ptr<AudioDeviceDescriptor> device)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "DeInitDeviceMuteMap device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    muteStatusMap_.erase(device->GetName());
    AUDIO_INFO_LOG("DeInitDeviceMuteMap device %{public}s", device->GetName().c_str());
}

void VolumeDataMaintainer::LoadDeviceMuteMapFromDb(std::shared_ptr<AudioDeviceDescriptor> device)
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
            .key = GetVolumeKey(device, stream),
            .defaultValue = false,
            .value = false
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
    for (size_t i = 0; i < volumeList.size(); i++) {
        SaveMuteToMap(device, volumeList[i], infos[i].value);
    }
}

int32_t VolumeDataMaintainer::SaveMuteToDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, bool muteStatus)
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
    std::string muteKey = GetMuteKey(device, streamType);
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
    AudioStreamType streamType, bool muteStatus)
{
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(streamType)) {
        device = ringerDevice_;
    }
    muteStatusMap_[device->GetName()][volumeType] = muteStatus;
    AUDIO_INFO_LOG("SaveMuteToMap device %{public}s streamType %{public}d muteStatus %{public}d",
        device->GetName().c_str(), streamType, muteStatus);
}
bool VolumeDataMaintainer::LoadMuteFromMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, false, "device is null");
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Util::IsDualToneStreamType(streamType)) {
        device = ringerDevice_;
    }
    CHECK_AND_RETURN_RET_LOG(muteStatusMap_.contains(device->GetName()), false,
        "device %{public}s not in muteStatusMap_", device->GetName().c_str());
    CHECK_AND_RETURN_RET_LOG(muteStatusMap_[device->GetName()].contains(volumeType), false,
        "device %{public}s volumeType %{public}d not in muteStatusMap_", device->GetName().c_str(), volumeType);
    return muteStatusMap_[device->GetName()][volumeType];
}

// open for speical need
int32_t VolumeDataMaintainer::LoadVolumeFromDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is null");
    int32_t volumeLevel = 0;
    std::string volumeKey = GetVolumeKey(device, streamType);
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

void VolumeDataMaintainer::SaveVolumeDegreeToMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t volumeDegree)
{
    CHECK_AND_RETURN_LOG(device != nullptr, "device is null");
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    volumeDegreeMap_[device->GetName()][volumeType] = volumeDegree;
    AUDIO_INFO_LOG("[device %{public}s, streamType %{public}d]"\
        "Save volume success, volumeDegree %{public}d",
        device->GetName().c_str(), volumeType, volumeDegree);
}

int32_t VolumeDataMaintainer::LoadVolumeDegreeFromMap(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeForMapMutex_);
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    int32_t defaultVolumeDegree = DEFAULT_VOLUME_DEGREE;
    CHECK_AND_RETURN_RET_LOG(device != nullptr, defaultVolumeDegree, "device is null");
    if (volumeType == STREAM_ALL) {
        AUDIO_INFO_LOG("replace stream all to music");
        volumeType = STREAM_MUSIC;
    }
    CHECK_AND_RETURN_RET_LOG(volumeDegreeMap_.contains(device->GetName()), defaultVolumeDegree,
        "device %{public}s not in map", device->GetName().c_str());
    CHECK_AND_RETURN_RET_LOG(volumeDegreeMap_[device->GetName()].contains(volumeType), defaultVolumeDegree,
        "device %{public}s stream %{public}d not in map", device->GetName().c_str(), volumeType);
    AUDIO_INFO_LOG("[device %{public}s, streamType %{public}d] volumeDegree %{public}d",
        device->GetName().c_str(), volumeType, volumeDegreeMap_[device->GetName()][volumeType]);
    return volumeDegreeMap_[device->GetName()][volumeType];
}

int32_t VolumeDataMaintainer::SaveVolumeDegreeToDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType, int32_t volumeDegree)
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
    std::string volumeKey = GetVolumeKey(device, streamType);
    if (!volumeKey.compare("")) {
        AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] is not supported",
            device->GetName().c_str(), streamType);
        return ERROR;
    }
    volumeKey += "_degree";

    {
        std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
        AudioSettingProvider& audioSettingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
        ErrCode ret = audioSettingProvider.PutIntValue(volumeKey, volumeDegree, "system");
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] save volume failed, ret %{public}d",
                device->GetName().c_str(), streamType, ret);
            return ERROR;
        }
        AUDIO_INFO_LOG("[device %{public}s, streamType %{public}d]"\
            "Save volume success, volumeDegree %{public}d",
            device->GetName().c_str(), streamType, volumeDegree);
    }
    return SUCCESS;
}

int32_t VolumeDataMaintainer::LoadVolumeDegreeFromDb(std::shared_ptr<AudioDeviceDescriptor> device,
    AudioStreamType streamType)
{
    std::lock_guard<ffrt::mutex> lock(volumeForDbMutex_);
    CHECK_AND_RETURN_RET_LOG(device != nullptr, ERROR, "device is null");
    int32_t volumeDegree = 0;
    std::string volumeKey = GetVolumeKey(device, streamType);
    if (!volumeKey.compare("")) {
        AUDIO_ERR_LOG("[device %{public}s, streamType %{public}d] is not supported",
            device->GetName().c_str(), streamType);
        return volumeDegree;
    }
    volumeKey += "_degree";

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

} // namespace AudioStandard
} // namespace OHOS
