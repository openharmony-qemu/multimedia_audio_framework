/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioVolumeManager"
#endif

#include "audio_volume_manager.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "i_policy_provider.h"
#include "audio_spatialization_service.h"
#include "audio_safe_volume_notification.h"

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"
#include "sle_audio_device_manager.h"

namespace OHOS {
namespace AudioStandard {

static const int64_t WAIT_RINGER_MODE_MUTE_RESET_TIME_MS = 500; // 500ms
const int32_t DUAL_TONE_RING_VOLUME = 0;
static std::string GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

const int32_t ONE_MINUTE = 60;
const uint32_t ABS_VOLUME_SUPPORT_RETRY_INTERVAL_IN_MICROSECONDS = 10000;
constexpr int32_t CANCEL_FORCE_CONTROL_VOLUME_TYPE = -1;

static const std::vector<AudioVolumeType> VOLUME_TYPE_LIST = {
    STREAM_VOICE_CALL,
    STREAM_RING,
    STREAM_MUSIC,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_ACCESSIBILITY,
    STREAM_ULTRASONIC,
    STREAM_SYSTEM,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_ALL
};

static const std::vector<AudioStreamType> AUDIO_STREAMTYPE_VOLUME_LIST = {
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_SYSTEM,
    STREAM_NOTIFICATION,
    STREAM_ALARM,
    STREAM_DTMF,
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_ACCESSIBILITY,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
};

bool AudioVolumeManager::Init(std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler)
{
    audioPolicyServerHandler_ = audioPolicyServerHandler;
    if (policyVolumeMap_ == nullptr) {
        size_t mapSize = IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume) + sizeof(bool);
        AUDIO_INFO_LOG("InitSharedVolume create shared volume map with size %{public}zu", mapSize);
        policyVolumeMap_ = AudioSharedMemory::CreateFormLocal(mapSize, "PolicyVolumeMap");
        CHECK_AND_RETURN_RET_LOG(policyVolumeMap_ != nullptr && policyVolumeMap_->GetBase() != nullptr,
            false, "Get shared memory failed!");
        volumeVector_ = reinterpret_cast<Volume *>(policyVolumeMap_->GetBase());
        sharedAbsVolumeScene_ = reinterpret_cast<bool *>(policyVolumeMap_->GetBase()) +
            IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume);
    }
    if (forceControlVolumeTypeMonitor_ == nullptr) {
        forceControlVolumeTypeMonitor_ = std::make_shared<ForceControlVolumeTypeMonitor>();
    }
    return true;
}
void AudioVolumeManager::DeInit(void)
{
    volumeVector_ = nullptr;
    sharedAbsVolumeScene_ = nullptr;
    policyVolumeMap_ = nullptr;
    safeVolumeExit_ = true;
    forceControlVolumeTypeMonitor_ = nullptr;
    if (calculateLoopSafeTime_ != nullptr && calculateLoopSafeTime_->joinable()) {
        calculateLoopSafeTime_->join();
        calculateLoopSafeTime_.reset();
        calculateLoopSafeTime_ = nullptr;
    }
    if (safeVolumeDialogThrd_ != nullptr && safeVolumeDialogThrd_->joinable()) {
        safeVolumeDialogThrd_->join();
        safeVolumeDialogThrd_.reset();
        safeVolumeDialogThrd_ = nullptr;
    }
    audioPolicyServerHandler_ = nullptr;
}

int32_t AudioVolumeManager::GetMaxVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType) const
{
    if (volumeType == STREAM_ALL) {
        volumeType = STREAM_MUSIC;
    }
    return audioPolicyManager_.GetMaxVolumeLevel(volumeType, deviceType);
}

int32_t AudioVolumeManager::GetMinVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType) const
{
    if (volumeType == STREAM_ALL) {
        volumeType = STREAM_MUSIC;
    }
    return audioPolicyManager_.GetMinVolumeLevel(volumeType, deviceType);
}

bool AudioVolumeManager::SetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume vol)
{
    CHECK_AND_RETURN_RET_LOG(volumeVector_ != nullptr, false, "Set shared memory failed!");
    size_t index = 0;
    if (!IPolicyProvider::GetVolumeIndex(streamType, GetVolumeGroupForDevice(deviceType), index) ||
        index >= IPolicyProvider::GetVolumeVectorSize()) {
        AUDIO_INFO_LOG("not find device %{public}d, stream %{public}d", deviceType, streamType);
        return false;
    }
    if (deviceType == DEVICE_TYPE_NEARLINK && streamType == STREAM_VOICE_CALL) {
        vol.volumeFloat = 1.0f;
    }
    volumeVector_[index].isMute = vol.isMute;
    volumeVector_[index].volumeFloat = vol.volumeFloat;
    volumeVector_[index].volumeInt = vol.volumeInt;
    volumeVector_[index].volumeDegree = vol.volumeDegree;
    AUDIO_INFO_LOG("Success Set Shared Volume with StreamType:%{public}d, DeviceType:%{public}d, \
        volume:%{public}d, volumeDegree:%{public}d",
        streamType, deviceType, vol.volumeInt, vol.volumeDegree);

    AudioServerProxy::GetInstance().NotifyStreamVolumeChangedProxy(streamType, vol.volumeFloat);
    return true;
}

int32_t AudioVolumeManager::InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer)
{
    AUDIO_INFO_LOG("InitSharedVolume start");
    CHECK_AND_RETURN_RET_LOG(policyVolumeMap_ != nullptr && policyVolumeMap_->GetBase() != nullptr,
        ERR_OPERATION_FAILED, "Get shared memory failed!");

    // init volume map
    // todo device
    for (size_t i = 0; i < IPolicyProvider::GetVolumeVectorSize(); i++) {
        bool isMute = audioPolicyManager_.GetStreamMute(g_volumeIndexVector[i].first);
        int32_t currentVolumeLevel = audioPolicyManager_.GetSystemVolumeLevelNoMuteState(g_volumeIndexVector[i].first);
        float volFloat = audioPolicyManager_.GetSystemVolumeInDb(g_volumeIndexVector[i].first,
            (isMute ? 0 : currentVolumeLevel), audioActiveDevice_.GetCurrentOutputDeviceType());
        volumeVector_[i].isMute = isMute;
        volumeVector_[i].volumeFloat = volFloat;
        volumeVector_[i].volumeInt = static_cast<uint32_t>(currentVolumeLevel);
    }
    SetSharedAbsVolumeScene(false);
    buffer = policyVolumeMap_;

    return SUCCESS;
}

void AudioVolumeManager::SetSharedAbsVolumeScene(const bool support)
{
    CHECK_AND_RETURN_LOG(sharedAbsVolumeScene_ != nullptr, "sharedAbsVolumeScene is nullptr");
    *sharedAbsVolumeScene_ = support;
}

int32_t AudioVolumeManager::GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel)
{
    return audioPolicyManager_.GetAppVolumeLevel(appUid, volumeLevel);
}

int32_t AudioVolumeManager::GetSystemVolumeLevel(AudioStreamType streamType, int32_t zoneId)
{
    if (zoneId > 0) {
        return audioPolicyManager_.GetZoneVolumeLevel(zoneId, streamType);
    }
    if (streamType == STREAM_RING && !IsRingerModeMute()) {
        AUDIO_PRERELEASE_LOGW("return 0 when dual tone ring");
        return DUAL_TONE_RING_VOLUME;
    }
    auto volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    {
        DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        if (volumeType == STREAM_MUSIC &&
            curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
            A2dpDeviceConfigInfo info;
            bool ret = audioA2dpDevice_.GetA2dpDeviceInfo(btDevice, info);
            if (ret && info.absVolumeSupport) {
                return info.mute ? 0 : info.volumeLevel;
            }
        }
    }
    auto deviceDesc = audioActiveDevice_.GetCurrentOutputDevice();
    if (deviceDesc.deviceType_ == DEVICE_TYPE_NEARLINK &&
        (volumeType == STREAM_MUSIC || volumeType == STREAM_VOICE_CALL)) {
        return SleAudioDeviceManager::GetInstance().GetVolumeLevelByVolumeType(volumeType, deviceDesc);
    }
    int32_t volume = audioPolicyManager_.GetSystemVolumeLevel(streamType);
    Trace trace("AudioVolumeManager::GetSystemVolumeLevel device" + std::to_string(deviceDesc.deviceType_) + " stream "
        + std::to_string(streamType) + " volume " + std::to_string(volume));
    return volume;
}

int32_t AudioVolumeManager::GetSystemVolumeLevelNoMuteState(AudioStreamType streamType)
{
    return audioPolicyManager_.GetSystemVolumeLevelNoMuteState(streamType);
}

int32_t AudioVolumeManager::SetVolumeForSwitchDevice(AudioDeviceDescriptor deviceDescriptor,
    const std::string &newSinkName, bool enableSetVoiceCallVolume)
{
    Trace trace("AudioVolumeManager::SetVolumeForSwitchDevice:" + std::to_string(deviceDescriptor.deviceType_));
    // Load volume from KvStore and set volume for each stream type
    audioPolicyManager_.SetVolumeForSwitchDevice(deviceDescriptor);

    // The volume of voice_call needs to be adjusted separately
    if (enableSetVoiceCallVolume && audioSceneManager_.GetAudioScene(true) == AUDIO_SCENE_PHONE_CALL) {
        SetVoiceCallVolume(GetSystemVolumeLevel(STREAM_VOICE_CALL));
    }
    return SUCCESS;
}

int32_t AudioVolumeManager::SetVoiceRingtoneMute(bool isMute)
{
    isVoiceRingtoneMute_ = isMute ? true : false;
    SetVoiceCallVolume(GetSystemVolumeLevel(STREAM_VOICE_CALL));
    return SUCCESS;
}

void AudioVolumeManager::SetVoiceCallVolume(int32_t volumeLevel)
{
    Trace trace("AudioVolumeManager::SetVoiceCallVolume" + std::to_string(volumeLevel));
    // set voice volume by the interface from hdi.
    CHECK_AND_RETURN_LOG(volumeLevel != 0, "SetVoiceVolume: volume of voice_call cannot be set to 0");
    float volumeDb = static_cast<float>(volumeLevel) /
        static_cast<float>(audioPolicyManager_.GetMaxVolumeLevel(STREAM_VOICE_CALL));
    volumeDb = isVoiceRingtoneMute_ ? 0 : volumeDb;
    if (audioActiveDevice_.GetCurrentOutputDeviceType() == DEVICE_TYPE_NEARLINK) {
        volumeDb = 1;
    }
    AudioServerProxy::GetInstance().SetVoiceVolumeProxy(volumeDb);
    AUDIO_INFO_LOG("%{public}f", volumeDb);
}

void AudioVolumeManager::InitKVStore()
{
    audioPolicyManager_.InitKVStore();
    AudioSpatializationService::GetAudioSpatializationService().InitSpatializationState();
}

void AudioVolumeManager::CheckToCloseNotification(AudioStreamType streamType, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("enter.");
    int32_t sVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
    if (volumeLevel < sVolumeLevel && DeviceIsSupportSafeVolume() &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC) {
        AUDIO_INFO_LOG("user select lower volume should close notification.");
        if (increaseNIsShowing_) {
            CancelSafeVolumeNotification(INCREASE_VOLUME_NOTIFICATION_ID);
            increaseNIsShowing_ = false;
        }
        if (restoreNIsShowing_) {
            CancelSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
            restoreNIsShowing_ = false;
        }
    }
}

bool AudioVolumeManager::DeviceIsSupportSafeVolume()
{
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    DeviceCategory curOutputDeviceCategory = audioPolicyManager_.GetCurrentOutputDeviceCategory();
    switch (curOutputDeviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            if (curOutputDeviceCategory != BT_SOUNDBOX &&
                curOutputDeviceCategory != BT_CAR) {
                return true;
            }
            [[fallthrough]];
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return true;
        default:
            AUDIO_INFO_LOG("current device unsupport safe volume:%{public}d", curOutputDeviceType);
            return false;
    }
}

int32_t AudioVolumeManager::SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("enter AudioVolumeManager::SetAppVolumeLevel");
    // audioPolicyManager_ : AudioAdapterManager
    int32_t result = audioPolicyManager_.SetAppVolumeLevel(appUid, volumeLevel);
    return result;
}

int32_t AudioVolumeManager::SetAppVolumeMuted(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("enter AudioVolumeManager::SetAppVolumeMuted");
    int32_t result = audioPolicyManager_.SetAppVolumeMuted(appUid, muted);
    return result;
}

int32_t AudioVolumeManager::IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute)
{
    AUDIO_INFO_LOG("enter AudioVolumeManager::IsAppVolumeMute");
    int32_t result = audioPolicyManager_.IsAppVolumeMute(appUid, owned, isMute);
    return result;
}

int32_t AudioVolumeManager::SetAppRingMuted(int32_t appUid, bool muted)
{
    AUDIO_INFO_LOG("enter AudioVolumeManager::SetAppRingMuted");
    int32_t result = audioPolicyManager_.SetAppRingMuted(appUid, muted);
    return result;
}

bool AudioVolumeManager::IsAppRingMuted(int32_t appUid)
{
    AUDIO_INFO_LOG("enter AudioVolumeManager::IsAppRingMuted");
    return audioPolicyManager_.IsAppRingMuted(appUid);
}

int32_t AudioVolumeManager::GetVolumeAdjustZoneId()
{
    return audioPolicyManager_.GetVolumeAdjustZoneId();
}

int32_t AudioVolumeManager::SetAdjustVolumeForZone(int32_t zoneId)
{
    if (zoneId == 0) {
        AudioDeviceDescriptor currentActiveDevice = audioActiveDevice_.GetCurrentOutputDevice();
        audioPolicyManager_.SetVolumeForSwitchDevice(currentActiveDevice);
    }
    return audioPolicyManager_.SetAdjustVolumeForZone(zoneId);
}

int32_t AudioVolumeManager::HandleA2dpAbsVolume(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType curOutputDeviceType)
{
    std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
    int32_t result = SetA2dpDeviceVolume(btDevice, volumeLevel, true);
    Volume vol = {false, 1.0f, 0};
    vol.isMute = volumeLevel == 0 ? true : false;
    vol.volumeInt = static_cast<uint32_t>(volumeLevel);
    vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDb(streamType, volumeLevel, curOutputDeviceType);
    SetSharedVolume(streamType, curOutputDeviceType, vol);
#ifdef BLUETOOTH_ENABLE
    if (result == SUCCESS) {
        // set to avrcp device
        return Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(btDevice, volumeLevel);
    } else if (result == ERR_UNKNOWN) {
        AUDIO_INFO_LOG("UNKNOWN RESULT set abs safe volume");
        return Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(btDevice,
            audioPolicyManager_.GetSafeVolumeLevel());
    } else {
        AUDIO_ERR_LOG("AudioVolumeManager::SetSystemVolumeLevel set abs volume failed");
    }
    return result;
#else
    return SUCCESS;
#endif
}

int32_t AudioVolumeManager::HandleNearlinkDeviceAbsVolume(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType curOutputDeviceType)
{
    std::string nearlinkDevice = audioActiveDevice_.GetCurrentOutputDeviceMacAddr();
    if (nearlinkDevice.empty()) {
        AUDIO_ERR_LOG("nearlink device is empty");
        return ERR_UNKNOWN;
    }

    Volume vol = {false, 1.0f, 0};
    vol.isMute = volumeLevel == 0 ? true : false;
    vol.volumeInt = static_cast<uint32_t>(volumeLevel);
    vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDb(streamType, volumeLevel, curOutputDeviceType);
    SetSharedVolume(streamType, curOutputDeviceType, vol);

    int32_t result = SetNearlinkDeviceVolume(nearlinkDevice, streamType, volumeLevel, true);
    if (result == SUCCESS) {
        auto volumeValue = SleAudioDeviceManager::GetInstance().GetVolumeLevelByVolumeType(streamType,
            audioActiveDevice_.GetCurrentOutputDevice());
        return SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(nearlinkDevice, streamType, volumeValue);
    } else if (result == ERR_UNKNOWN) {
        AUDIO_INFO_LOG("UNKNOWN RESULT set abs safe volume");
        return SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(nearlinkDevice, streamType,
            audioPolicyManager_.GetSafeVolumeLevel());
    }
    return result;
}

int32_t AudioVolumeManager::SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel,
    int32_t zoneId)
{
    if (zoneId > 0) {
        return audioPolicyManager_.SetZoneVolumeLevel(zoneId,
            VolumeUtils::GetVolumeTypeFromStreamType(streamType), volumeLevel);
    }
    int32_t result = ERROR;
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    curOutputDeviceType_ = curOutputDeviceType;
    auto volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (volumeType == STREAM_MUSIC &&
        streamType != STREAM_VOICE_CALL &&
        curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        result = HandleA2dpAbsVolume(streamType, volumeLevel, curOutputDeviceType);
    }

    if (curOutputDeviceType == DEVICE_TYPE_NEARLINK &&
        (volumeType == STREAM_MUSIC || volumeType == STREAM_VOICE_CALL)) {
        result = HandleNearlinkDeviceAbsVolume(streamType, volumeLevel, curOutputDeviceType);
    }

    if (result == SUCCESS) {
        return result;
    }

    int32_t sVolumeLevel = SelectDealSafeVolume(streamType, volumeLevel);
    CheckToCloseNotification(streamType, volumeLevel);
    if (volumeLevel != sVolumeLevel) {
        volumeLevel = sVolumeLevel;
        AUDIO_INFO_LOG("safevolume did not deal");
    }
    result = audioPolicyManager_.SetSystemVolumeLevel(VolumeUtils::GetVolumeTypeFromStreamType(streamType),
        volumeLevel);
    if (result == SUCCESS && (streamType == STREAM_VOICE_CALL || streamType == STREAM_VOICE_COMMUNICATION)) {
        SetVoiceCallVolume(volumeLevel);
    }
    // todo
    Volume vol = {false, 1.0f, 0};
    vol.isMute = volumeLevel == 0 ? true : false;
    vol.volumeInt = static_cast<uint32_t>(volumeLevel);
    vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDb(streamType, volumeLevel, curOutputDeviceType);
    SetSharedVolume(streamType, curOutputDeviceType, vol);
    return result;
}

int32_t AudioVolumeManager::SaveSpecifiedDeviceVolume(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType)
{
    int32_t sVolumeLevel = volumeLevel;
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP || deviceType == DEVICE_TYPE_BLUETOOTH_SCO ||
        deviceType == DEVICE_TYPE_USB_HEADSET || deviceType == DEVICE_TYPE_USB_ARM_HEADSET ||
        deviceType == DEVICE_TYPE_WIRED_HEADSET || deviceType == DEVICE_TYPE_WIRED_HEADPHONES ||
        deviceType == DEVICE_TYPE_NEARLINK) {
        sVolumeLevel = SelectDealSafeVolume(streamType, volumeLevel, deviceType);
    }
    int32_t result = audioPolicyManager_.SaveSpecifiedDeviceVolume(
        VolumeUtils::GetVolumeTypeFromStreamType(streamType), sVolumeLevel, deviceType);
    return result;
}

int32_t AudioVolumeManager::SelectDealSafeVolume(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType)
{
    int32_t sVolumeLevel = volumeLevel;
    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) != STREAM_MUSIC) {
        // Safe Volume only applying to  STREAM_MUSIC
        return sVolumeLevel;
    }
    DeviceType curOutputDeviceType = (deviceType == DEVICE_TYPE_NONE) ?
        audioActiveDevice_.GetCurrentOutputDeviceType() : deviceType;
    DeviceCategory curOutputDeviceCategory = audioPolicyManager_.GetCurrentOutputDeviceCategory();
    if (sVolumeLevel > audioPolicyManager_.GetSafeVolumeLevel()) {
        switch (curOutputDeviceType) {
            case DEVICE_TYPE_BLUETOOTH_A2DP:
            case DEVICE_TYPE_BLUETOOTH_SCO:
            case DEVICE_TYPE_NEARLINK:
                if (curOutputDeviceCategory == BT_SOUNDBOX || curOutputDeviceCategory == BT_CAR) {
                    return sVolumeLevel;
                }
                if (isBtFirstBoot_) {
                    sVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
                    AUDIO_INFO_LOG("Btfirstboot set volume use safe volume");
                } else {
                    sVolumeLevel = DealWithSafeVolume(volumeLevel, true);
                }
                break;
            case DEVICE_TYPE_WIRED_HEADSET:
            case DEVICE_TYPE_WIRED_HEADPHONES:
            case DEVICE_TYPE_USB_HEADSET:
            case DEVICE_TYPE_USB_ARM_HEADSET:
                sVolumeLevel = DealWithSafeVolume(volumeLevel, false);
                break;
            default:
                AUDIO_INFO_LOG("unsupport safe volume:%{public}d", curOutputDeviceType);
                break;
        }
    }
    if (curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP || curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_SCO ||
        curOutputDeviceType == DEVICE_TYPE_NEARLINK) {
        isBtFirstBoot_ = false;
    }
    return sVolumeLevel;
}

int32_t AudioVolumeManager::SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volumeLevel,
    bool internalCall)
{
    if (audioA2dpDevice_.SetA2dpDeviceVolumeLevel(macAddress, volumeLevel) == false) {
        return ERROR;
    }
    int32_t sVolumeLevel = volumeLevel;
    if (volumeLevel > audioPolicyManager_.GetSafeVolumeLevel()) {
        if (internalCall) {
            sVolumeLevel = DealWithSafeVolume(volumeLevel, true);
        } else {
            sVolumeLevel = HandleAbsBluetoothVolume(macAddress, volumeLevel);
        }
    }
    isBtFirstBoot_ = false;
    if (audioA2dpDevice_.SetA2dpDeviceVolumeLevel(macAddress, sVolumeLevel) == false) {
        return ERROR;
    }
    bool mute = sVolumeLevel == 0 ? true : false;

    if (internalCall) {
        CheckToCloseNotification(STREAM_MUSIC, volumeLevel);
    }

    audioA2dpDevice_.SetA2dpDeviceMute(macAddress, mute);
    audioPolicyManager_.SetAbsVolumeMute(mute);
    AUDIO_INFO_LOG("success for macaddress:[%{public}s], volume value:[%{public}d]",
        GetEncryptAddr(macAddress).c_str(), sVolumeLevel);
    CHECK_AND_RETURN_RET_LOG(sVolumeLevel == volumeLevel, ERR_UNKNOWN, "safevolume did not deal");
    return SUCCESS;
}

int32_t AudioVolumeManager::HandleAbsBluetoothVolume(const std::string &macAddress, const int32_t volumeLevel,
    bool isNearlinkDevice, AudioStreamType streamType)
{
    int32_t sVolumeLevel = 0;
    if (isBtFirstBoot_) {
        sVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
        AUDIO_INFO_LOG("Btfirstboot set volume use safe volume");
        isBtFirstBoot_ = false;
        if (!isNearlinkDevice) {
            Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(macAddress, sVolumeLevel);
        } else {
            SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(macAddress, streamType, sVolumeLevel);
        }
    } else {
        sVolumeLevel = DealWithSafeVolume(volumeLevel, true);
        if (sVolumeLevel != volumeLevel) {
            if (!isNearlinkDevice) {
                Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(macAddress, sVolumeLevel);
            } else {
                SleAudioDeviceManager::GetInstance().SetDeviceAbsVolume(macAddress, streamType, sVolumeLevel);
            }
        }
    }
    return sVolumeLevel;
}

int32_t AudioVolumeManager::SetNearlinkDeviceVolume(const std::string &macAddress, AudioStreamType streamType,
    int32_t volumeLevel, bool internalCall)
{
    int32_t ret = SleAudioDeviceManager::GetInstance().SetNearlinkDeviceVolumeLevel(macAddress, streamType,
        volumeLevel);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetNearlinkDeviceVolumeLevel failed");
    int32_t sVolumeLevel = volumeLevel;
    // Voice call does not support safe volume
    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC) {
        if (volumeLevel > audioPolicyManager_.GetSafeVolumeLevel()) {
            if (internalCall) {
                sVolumeLevel = DealWithSafeVolume(volumeLevel, true);
            } else {
                sVolumeLevel = HandleAbsBluetoothVolume(macAddress, volumeLevel, true, streamType);
            }
        }
        isBtFirstBoot_ = false;
    }
    ret = SleAudioDeviceManager::GetInstance().SetNearlinkDeviceVolumeLevel(macAddress, streamType, sVolumeLevel);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetDeviceAbsVolume failed");
    ret = audioPolicyManager_.SetSystemVolumeLevel(VolumeUtils::GetVolumeTypeFromStreamType(streamType),
        sVolumeLevel);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "SetSystemVolumeLevel failed");

    bool mute = sVolumeLevel == 0 && (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC);

    if (internalCall) {
        CheckToCloseNotification(streamType, volumeLevel);
    }

    SleAudioDeviceManager::GetInstance().SetNearlinkDeviceMute(macAddress, streamType, mute);
    audioPolicyManager_.SetAbsVolumeMute(mute);
    AUDIO_INFO_LOG("success for macaddress:[%{public}s], volume value:[%{public}d], streamType [%{public}d]",
        GetEncryptAddr(macAddress).c_str(), sVolumeLevel, streamType);
    CHECK_AND_RETURN_RET_LOG(sVolumeLevel == volumeLevel, ERR_UNKNOWN, "safevolume did not deal");
    return SUCCESS;
}

void AudioVolumeManager::PublishSafeVolumeNotification(int32_t notificationId)
{
    void *libHandle = dlopen("libaudio_safe_volume_notification_impl.z.so", RTLD_LAZY);
    if (libHandle == nullptr) {
        AUDIO_ERR_LOG("dlopen failed %{public}s", __func__);
        return;
    }
    CreateSafeVolumeNotification *createSafeVolumeNotificationImpl =
        reinterpret_cast<CreateSafeVolumeNotification*>(dlsym(libHandle, "CreateSafeVolumeNotificationImpl"));
    if (createSafeVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("createSafeVolumeNotificationImpl failed %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    AudioSafeVolumeNotification *audioSafeVolumeNotificationImpl = createSafeVolumeNotificationImpl();
    if (audioSafeVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("audioSafeVolumeNotificationImpl is nullptr %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    audioSafeVolumeNotificationImpl->PublishSafeVolumeNotification(notificationId);
    delete audioSafeVolumeNotificationImpl;
#ifndef TEST_COVERAGE
    dlclose(libHandle);
#endif
}

void AudioVolumeManager::CancelSafeVolumeNotification(int32_t notificationId)
{
    void *libHandle = dlopen("libaudio_safe_volume_notification_impl.z.so", RTLD_LAZY);
    if (libHandle == nullptr) {
        AUDIO_ERR_LOG("dlopen failed %{public}s", __func__);
        return;
    }
    CreateSafeVolumeNotification *createSafeVolumeNotificationImpl =
        reinterpret_cast<CreateSafeVolumeNotification*>(dlsym(libHandle, "CreateSafeVolumeNotificationImpl"));
    if (createSafeVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("createSafeVolumeNotificationImpl failed %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    AudioSafeVolumeNotification *audioSafeVolumeNotificationImpl = createSafeVolumeNotificationImpl();
    if (audioSafeVolumeNotificationImpl == nullptr) {
        AUDIO_ERR_LOG("audioSafeVolumeNotificationImpl is nullptr %{public}s", __func__);
#ifndef TEST_COVERAGE
        dlclose(libHandle);
#endif
        return;
    }
    audioSafeVolumeNotificationImpl->CancelSafeVolumeNotification(notificationId);
    delete audioSafeVolumeNotificationImpl;
#ifndef TEST_COVERAGE
    dlclose(libHandle);
#endif
}

int32_t AudioVolumeManager::DealWithSafeVolume(const int32_t volumeLevel, bool isBtDevice)
{
    if (isBtDevice) {
        DeviceCategory curOutputDeviceCategory = audioPolicyManager_.GetCurrentOutputDeviceCategory();
        AUDIO_INFO_LOG("bluetooth Category:%{public}d", curOutputDeviceCategory);
        if (curOutputDeviceCategory == BT_SOUNDBOX || curOutputDeviceCategory == BT_CAR) {
            return volumeLevel;
        }
    }

    int32_t sVolumeLevel = volumeLevel;
    safeStatusBt_ = audioPolicyManager_.GetCurrentDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP);
    safeStatus_ = audioPolicyManager_.GetCurrentDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET);
    if ((safeStatusBt_ == SAFE_INACTIVE && isBtDevice) ||
        (safeStatus_ == SAFE_INACTIVE && !isBtDevice)) {
        CreateCheckMusicActiveThread();
        return sVolumeLevel;
    }

    if ((isBtDevice && safeStatusBt_ == SAFE_ACTIVE) ||
        (!isBtDevice && safeStatus_ == SAFE_ACTIVE)) {
        sVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
        if (restoreNIsShowing_) {
            CancelSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
            restoreNIsShowing_ = false;
        }
        PublishSafeVolumeNotification(INCREASE_VOLUME_NOTIFICATION_ID);
        increaseNIsShowing_ = true;
        return sVolumeLevel;
    }
    return sVolumeLevel;
}

void AudioVolumeManager::CreateCheckMusicActiveThread()
{
    std::lock_guard<std::mutex> lock(checkMusicActiveThreadMutex_);
    if (calculateLoopSafeTime_ == nullptr) {
        calculateLoopSafeTime_ = std::make_unique<std::thread>([this] { this->CheckActiveMusicTime(); });
        pthread_setname_np(calculateLoopSafeTime_->native_handle(), "OS_AudioPolicySafe");
    }
}

bool AudioVolumeManager::IsWiredHeadSet(const DeviceType &deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            return true;
        default:
            return false;
    }
}

bool AudioVolumeManager::IsBlueTooth(const DeviceType &deviceType)
{
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP || deviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
        if (audioPolicyManager_.GetCurrentOutputDeviceCategory() != BT_CAR &&
            audioPolicyManager_.GetCurrentOutputDeviceCategory() != BT_SOUNDBOX) {
            return true;
        }
    }
    return false;
}

void AudioVolumeManager::SetRestoreVolumeLevel(DeviceType deviceType, int32_t curDeviceVolume)
{
    int32_t btDeviceVol = audioPolicyManager_.GetDeviceVolume(DEVICE_TYPE_BLUETOOTH_A2DP, STREAM_MUSIC);
    int32_t wiredDeviceVol = audioPolicyManager_.GetDeviceVolume(DEVICE_TYPE_WIRED_HEADSET, STREAM_MUSIC);
    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();

    btRestoreVol_ = btDeviceVol > safeVolume ? btDeviceVol : btRestoreVol_;
    audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, btRestoreVol_);
    wiredRestoreVol_ = wiredDeviceVol > safeVolume ? wiredDeviceVol : wiredRestoreVol_;
    audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, wiredRestoreVol_);

    AUDIO_INFO_LOG("btDeviceVol : %{public}d, wiredDeviceVol : %{public}d, curDeviceVolume : %{public}d",
        btDeviceVol, wiredDeviceVol, curDeviceVolume);

    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        AUDIO_INFO_LOG("set bt restore volume to db");
        btRestoreVol_ = curDeviceVolume > safeVolume ? curDeviceVolume : btRestoreVol_;
        audioPolicyManager_.SetRestoreVolumeLevel(deviceType, btRestoreVol_);
    } else if (deviceType == DEVICE_TYPE_WIRED_HEADSET) {
        AUDIO_INFO_LOG("set wired restore volume to db");
        wiredRestoreVol_ = curDeviceVolume > safeVolume ? curDeviceVolume : wiredRestoreVol_;
        audioPolicyManager_.SetRestoreVolumeLevel(deviceType, wiredRestoreVol_);
    }
}

int32_t AudioVolumeManager::CheckActiveMusicTime()
{
    AUDIO_INFO_LOG("enter");
    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();
    while (!safeVolumeExit_) {
        bool activeMusic = audioSceneManager_.IsStreamActive(STREAM_MUSIC);
        int32_t curDeviceVolume = GetSystemVolumeLevel(STREAM_MUSIC);
        bool isUpSafeVolume = curDeviceVolume > safeVolume ? true : false;
        DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
        AUDIO_INFO_LOG("activeMusic:%{public}d, deviceType_:%{public}d, isUpSafeVolume:%{public}d",
            activeMusic, curOutputDeviceType, isUpSafeVolume);
        if (activeMusic && (safeStatusBt_ == SAFE_INACTIVE) && isUpSafeVolume &&
            IsBlueTooth(curOutputDeviceType)) {
            SetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, curDeviceVolume);
            CheckBlueToothActiveMusicTime(safeVolume);
        } else if (activeMusic && (safeStatus_ == SAFE_INACTIVE) && isUpSafeVolume &&
            IsWiredHeadSet(curOutputDeviceType)) {
            SetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, curDeviceVolume);
            CheckWiredActiveMusicTime(safeVolume);
        } else {
            startSafeTime_ = 0;
            startSafeTimeBt_ = 0;
        }
        sleep(ONE_MINUTE);
    }
    return 0;
}

bool AudioVolumeManager::CheckMixActiveMusicTime(int32_t safeVolume)
{
    int64_t mixSafeTime = activeSafeTimeBt_ + activeSafeTime_;
    AUDIO_INFO_LOG("mix device cumulative time: %{public}" PRId64, mixSafeTime);
    if (mixSafeTime >= ONE_MINUTE * audioPolicyManager_.GetSafeVolumeTimeout()) {
        AUDIO_INFO_LOG("mix device safe volume timeout");
        ChangeDeviceSafeStatus(SAFE_ACTIVE);
        RestoreSafeVolume(STREAM_MUSIC, safeVolume);
        startSafeTimeBt_ = 0;
        startSafeTime_ = 0;
        activeSafeTimeBt_ = 0;
        activeSafeTime_ = 0;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP, 0);
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET, 0);
        return true;
    }
    return false;
}

void AudioVolumeManager::CheckBlueToothActiveMusicTime(int32_t safeVolume)
{
    if (startSafeTimeBt_ == 0) {
        startSafeTimeBt_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    int32_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (activeSafeTimeBt_ >= ONE_MINUTE * audioPolicyManager_.GetSafeVolumeTimeout()) {
        AUDIO_INFO_LOG("bluetooth device safe volume timeout");
        ChangeDeviceSafeStatus(SAFE_ACTIVE);
        RestoreSafeVolume(STREAM_MUSIC, safeVolume);
        startSafeTimeBt_ = 0;
        activeSafeTimeBt_ = 0;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP, 0);
        PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else if (CheckMixActiveMusicTime(safeVolume)) {
        PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else if (currentTime - startSafeTimeBt_ >= ONE_MINUTE) {
        activeSafeTimeBt_ = audioPolicyManager_.GetCurentDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP);
        activeSafeTimeBt_ += currentTime - startSafeTimeBt_;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_BLUETOOTH_A2DP, activeSafeTimeBt_);
        startSafeTimeBt_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        AUDIO_INFO_LOG("bluetooth safe volume 1 min timeout, cumulative time: %{public}" PRId64, activeSafeTimeBt_);
    }
    startSafeTime_ = 0;
}

void AudioVolumeManager::CheckWiredActiveMusicTime(int32_t safeVolume)
{
    if (startSafeTime_ == 0) {
        startSafeTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    }
    int32_t currentTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    if (activeSafeTime_ >= ONE_MINUTE * audioPolicyManager_.GetSafeVolumeTimeout()) {
        AUDIO_INFO_LOG("wired device safe volume timeout");
        ChangeDeviceSafeStatus(SAFE_ACTIVE);
        RestoreSafeVolume(STREAM_MUSIC, safeVolume);
        startSafeTime_ = 0;
        activeSafeTime_ = 0;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET, 0);
        PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else if (CheckMixActiveMusicTime(safeVolume)) {
        PublishSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = true;
    } else if (currentTime - startSafeTime_ >= ONE_MINUTE) {
        activeSafeTime_ = audioPolicyManager_.GetCurentDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET);
        activeSafeTime_ += currentTime - startSafeTime_;
        audioPolicyManager_.SetDeviceSafeTime(DEVICE_TYPE_WIRED_HEADSET, activeSafeTime_);
        startSafeTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        AUDIO_INFO_LOG("wired safe volume 1 min timeout, cumulative time: %{public}" PRId64, activeSafeTime_);
    }
    startSafeTimeBt_ = 0;
}

void AudioVolumeManager::CheckLowerDeviceVolume(DeviceType deviceType)
{
    int32_t btVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP);
    int32_t wiredVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET);

    AUDIO_INFO_LOG("btVolume : %{public}d, wiredVolume : %{public}d", btVolume, wiredVolume);

    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            if (btVolume > safeVolume) {
                AUDIO_INFO_LOG("wired device timeout, set bt device to safe volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, safeVolume, DEVICE_TYPE_BLUETOOTH_A2DP);
            }
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            if (wiredVolume > safeVolume) {
                AUDIO_INFO_LOG("bt device timeout, set wired device to safe volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, safeVolume, DEVICE_TYPE_WIRED_HEADSET);
            }
            break;
        default:
            AUDIO_ERR_LOG("current device not set safe volume");
            break;
    }
}

void AudioVolumeManager::RestoreSafeVolume(AudioStreamType streamType, int32_t safeVolume)
{
    userSelect_ = false;
    isDialogSelectDestroy_.store(false);

    if (GetSystemVolumeLevel(streamType) <= safeVolume) {
        AUDIO_INFO_LOG("current volume <= safe volume, don't update volume.");
        return;
    }

    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();

    AUDIO_INFO_LOG("restore safe volume.");
    SetSystemVolumeLevel(streamType, safeVolume);
    CheckLowerDeviceVolume(curOutputDeviceType);
    SetSafeVolumeCallback(streamType);
}

void AudioVolumeManager::SetSafeVolumeCallback(AudioStreamType streamType)
{
    CHECK_AND_RETURN_LOG(VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC,
        "streamtype:%{public}d no need to set safe volume callback.", streamType);
    VolumeEvent volumeEvent;
    volumeEvent.volumeType = streamType;
    volumeEvent.volume = GetSystemVolumeLevel(streamType);
    volumeEvent.updateUi = true;
    volumeEvent.volumeGroupId = 0;
    volumeEvent.networkId = LOCAL_NETWORK_ID;
    if (audioPolicyServerHandler_ != nullptr && IsRingerModeMute()) {
        audioPolicyServerHandler_->SendVolumeKeyEventCallback(volumeEvent);
    }
}

void AudioVolumeManager::OnReceiveEvent(const EventFwk::CommonEventData &eventData)
{
    AUDIO_INFO_LOG("enter.");
    const AAFwk::Want& want = eventData.GetWant();
    std::string action = want.GetAction();
    if (action == AUDIO_RESTORE_VOLUME_EVENT) {
        AUDIO_INFO_LOG("AUDIO_RESTORE_VOLUME_EVENT has been received");
        std::lock_guard<std::mutex> lock(notifyMutex_);
        CancelSafeVolumeNotification(RESTORE_VOLUME_NOTIFICATION_ID);
        restoreNIsShowing_ = false;
        ChangeDeviceSafeStatus(SAFE_INACTIVE);
        DealWithEventVolume(RESTORE_VOLUME_NOTIFICATION_ID);
        SetSafeVolumeCallback(STREAM_MUSIC);
    } else if (action == AUDIO_INCREASE_VOLUME_EVENT) {
        AUDIO_INFO_LOG("AUDIO_INCREASE_VOLUME_EVENT has been received");
        std::lock_guard<std::mutex> lock(notifyMutex_);
        CancelSafeVolumeNotification(INCREASE_VOLUME_NOTIFICATION_ID);
        increaseNIsShowing_ = false;
        ChangeDeviceSafeStatus(SAFE_INACTIVE);
        DealWithEventVolume(INCREASE_VOLUME_NOTIFICATION_ID);
        SetSafeVolumeCallback(STREAM_MUSIC);
    }
}

void AudioVolumeManager::SetDeviceSafeVolumeStatus()
{
    if (!userSelect_) {
        return;
    }

    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    switch (curOutputDeviceType) {
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_NEARLINK:
            safeStatusBt_ = SAFE_INACTIVE;
            audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_);
            CreateCheckMusicActiveThread();
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            safeStatus_ = SAFE_INACTIVE;
            audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET, safeStatus_);
            CreateCheckMusicActiveThread();
            break;
        default:
            AUDIO_INFO_LOG("safeVolume unsupported device:%{public}d", curOutputDeviceType);
            break;
    }
}

void AudioVolumeManager::ChangeDeviceSafeStatus(SafeStatus safeStatus)
{
    AUDIO_INFO_LOG("change all support safe volume devices status.");

    safeStatusBt_ = safeStatus;
    audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP, safeStatusBt_);

    safeStatus_ = safeStatus;
    audioPolicyManager_.SetDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET, safeStatus_);

    CreateCheckMusicActiveThread();
}

int32_t AudioVolumeManager::DisableSafeMediaVolume()
{
    AUDIO_INFO_LOG("Enter");
    std::lock_guard<std::mutex> lock(dialogMutex_);
    userSelect_ = true;
    isDialogSelectDestroy_.store(true);
    dialogSelectCondition_.notify_all();
    SetDeviceSafeVolumeStatus();
    return SUCCESS;
}

void AudioVolumeManager::SetAbsVolumeSceneAsync(const std::string &macAddress, const bool support)
{
    usleep(SET_BT_ABS_SCENE_DELAY_MS);
    std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
    AUDIO_INFO_LOG("success for macAddress:[%{public}s], support: %{public}d, active bt:[%{public}s]",
        GetEncryptAddr(macAddress).c_str(), support, GetEncryptAddr(btDevice).c_str());

    if (btDevice == macAddress) {
        audioPolicyManager_.SetAbsVolumeScene(support);
        SetSharedAbsVolumeScene(support);
        int32_t volumeLevel = audioPolicyManager_.GetSystemVolumeLevelNoMuteState(STREAM_MUSIC);
        audioPolicyManager_.SetSystemVolumeLevel(STREAM_MUSIC, volumeLevel);
    }
}

int32_t AudioVolumeManager::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    // Maximum number of attempts, preventing situations where a2dp device has not yet finished coming online.
    int maxRetries = 3;
    int retryCount = 0;
    while (retryCount < maxRetries) {
        retryCount++;
        int32_t currentVolume =  audioPolicyManager_.GetSystemVolumeLevelNoMuteState(STREAM_MUSIC);
        bool currentMute =  audioPolicyManager_.GetStreamMute(STREAM_MUSIC);
        if (audioA2dpDevice_.SetA2dpDeviceAbsVolumeSupport(macAddress, support, currentVolume, currentMute)) {
            break;
        }
        CHECK_AND_RETURN_RET_LOG(retryCount != maxRetries, ERROR,
            "failed, can't find device for macAddress:[%{public}s]", GetEncryptAddr(macAddress).c_str());;
        usleep(ABS_VOLUME_SUPPORT_RETRY_INTERVAL_IN_MICROSECONDS);
    }

    // The delay setting is due to move a2dp sink after this
    std::thread setAbsSceneThrd(&AudioVolumeManager::SetAbsVolumeSceneAsync, this, macAddress, support);
    setAbsSceneThrd.detach();

    return SUCCESS;
}

int32_t AudioVolumeManager::SetStreamMute(AudioStreamType streamType, bool mute, const StreamUsage &streamUsage,
    const DeviceType &deviceType, int32_t zoneId)
{
    if (zoneId > 0) {
        return audioPolicyManager_.SetZoneMute(zoneId, streamType, mute, streamUsage, deviceType);
    }
    int32_t result = SUCCESS;
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    if (deviceType != DEVICE_TYPE_NONE) {
        AUDIO_INFO_LOG("set stream mute for specified device [%{public}d]", deviceType);
        curOutputDeviceType = deviceType;
    }
    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC &&
        curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        if (audioA2dpDevice_.SetA2dpDeviceMute(btDevice, mute)) {
            audioPolicyManager_.SetAbsVolumeMute(mute);
            Volume vol = {false, 1.0f, 0};
            vol.isMute = mute;
            vol.volumeInt = static_cast<uint32_t>(GetSystemVolumeLevelNoMuteState(streamType));
            vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDb(streamType,
                (mute ? 0 : vol.volumeInt), curOutputDeviceType);
            SetSharedVolume(streamType, curOutputDeviceType, vol);
#ifdef BLUETOOTH_ENABLE
            // set to avrcp device
            int32_t volumeLevel;
            audioA2dpDevice_.GetA2dpDeviceVolumeLevel(btDevice, volumeLevel);
            return Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(btDevice,
                volumeLevel);
#endif
        }
    }
    result = audioPolicyManager_.SetStreamMute(streamType, mute, streamUsage, curOutputDeviceType,
        audioActiveDevice_.GetCurrentOutputDevice().networkId_);

    Volume vol = {false, 1.0f, 0};
    vol.isMute = mute;
    vol.volumeInt = static_cast<uint32_t>(GetSystemVolumeLevelNoMuteState(streamType));
    vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDb(streamType,
        (mute ? 0 : vol.volumeInt), curOutputDeviceType);
    SetSharedVolume(streamType, curOutputDeviceType, vol);

    return result;
}

bool AudioVolumeManager::GetStreamMute(AudioStreamType streamType, int32_t zoneId) const
{
    if (zoneId > 0) {
        return audioPolicyManager_.GetZoneMute(zoneId, streamType);
    }
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC &&
        curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        A2dpDeviceConfigInfo info;
        bool ret = audioA2dpDevice_.GetA2dpDeviceInfo(btDevice, info);
        if (ret == false || !info.absVolumeSupport) {
            AUDIO_WARNING_LOG("Get failed for macAddress:[%{public}s]", GetEncryptAddr(btDevice).c_str());
        } else {
            return info.mute;
        }
    }
    return audioPolicyManager_.GetStreamMute(streamType);
}

void AudioVolumeManager::UpdateGroupInfo(GroupType type, std::string groupName, int32_t& groupId,
    std::string networkId, bool connected, int32_t mappingId)
{
    std::lock_guard<std::mutex> lock(volumeGroupsMutex_);
    ConnectType connectType = CONNECT_TYPE_LOCAL;
    if (networkId != LOCAL_NETWORK_ID) {
        connectType = CONNECT_TYPE_DISTRIBUTED;
    }
    if (type == GroupType::VOLUME_TYPE) {
        auto isPresent = [&groupName, &networkId] (const sptr<VolumeGroupInfo> &volumeInfo) {
            return ((groupName == volumeInfo->groupName_) || (networkId == volumeInfo->networkId_));
        };

        auto iter = std::find_if(volumeGroups_.begin(), volumeGroups_.end(), isPresent);
        if (iter != volumeGroups_.end()) {
            groupId = (*iter)->volumeGroupId_;
            // if status is disconnected, remove the group that has none audio device
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> devsInGroup =
                audioConnectedDevice_.GetDevicesForGroup(type, groupId);
            if (!connected && devsInGroup.size() == 0) {
                volumeGroups_.erase(iter);
            }
            return;
        }
        if (groupName != GROUP_NAME_NONE && connected) {
            groupId = AudioGroupHandle::GetInstance().GetNextId(type);
            sptr<VolumeGroupInfo> volumeGroupInfo = new(std::nothrow) VolumeGroupInfo(groupId,
                mappingId, groupName, networkId, connectType);
            volumeGroups_.push_back(volumeGroupInfo);
        }
    } else {
        auto isPresent = [&groupName, &networkId] (const sptr<InterruptGroupInfo> &info) {
            return ((groupName == info->groupName_) || (networkId == info->networkId_));
        };

        auto iter = std::find_if(interruptGroups_.begin(), interruptGroups_.end(), isPresent);
        if (iter != interruptGroups_.end()) {
            groupId = (*iter)->interruptGroupId_;
            // if status is disconnected, remove the group that has none audio device
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> devsInGroup =
                audioConnectedDevice_.GetDevicesForGroup(type, groupId);
            if (!connected && devsInGroup.size() == 0) {
                interruptGroups_.erase(iter);
            }
            return;
        }
        if (groupName != GROUP_NAME_NONE && connected) {
            groupId = AudioGroupHandle::GetInstance().GetNextId(type);
            sptr<InterruptGroupInfo> interruptGroupInfo = new(std::nothrow) InterruptGroupInfo(groupId, mappingId,
                groupName, networkId, connectType);
            interruptGroups_.push_back(interruptGroupInfo);
        }
    }
}

void AudioVolumeManager::GetVolumeGroupInfo(std::vector<sptr<VolumeGroupInfo>>& volumeGroupInfos)
{
    std::lock_guard<std::mutex> lock(volumeGroupsMutex_);
    for (auto& v : volumeGroups_) {
        sptr<VolumeGroupInfo> info = new(std::nothrow) VolumeGroupInfo(v->volumeGroupId_, v->mappingId_, v->groupName_,
            v->networkId_, v->connectType_);
        volumeGroupInfos.push_back(info);
    }
}

int32_t AudioVolumeManager::CheckRestoreDeviceVolume(DeviceType deviceType)
{
    int32_t ret = 0;
    int32_t btRestoreVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP);
    int32_t wiredRestoreVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET);

    AUDIO_INFO_LOG("btRestoreVolume: %{public}d, wiredRestoreVolume: %{public}d", btRestoreVolume, wiredRestoreVolume);

    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();
    switch (deviceType) {
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            if (wiredRestoreVolume > safeVolume) {
                AUDIO_INFO_LOG("restore active wired device volume");
                ret = SetSystemVolumeLevel(STREAM_MUSIC, wiredRestoreVolume);
            }
            if (btRestoreVolume > safeVolume) {
                AUDIO_INFO_LOG("restore other bt device volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, btRestoreVolume, DEVICE_TYPE_BLUETOOTH_A2DP);
            }
            break;
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_BLUETOOTH_A2DP:
            if (btRestoreVolume > safeVolume) {
                AUDIO_INFO_LOG("restore active bt device volume");
                ret = SetSystemVolumeLevel(STREAM_MUSIC, btRestoreVolume);
            }
            if (wiredRestoreVolume > safeVolume) {
                AUDIO_INFO_LOG("restore other wired device volume");
                SaveSpecifiedDeviceVolume(STREAM_MUSIC, wiredRestoreVolume, DEVICE_TYPE_WIRED_HEADSET);
            }
            break;
        default:
            ret = ERROR;
            AUDIO_ERR_LOG("current device not set safe volume");
            break;
    }

    return ret;
}

int32_t AudioVolumeManager::DealWithEventVolume(const int32_t notificationId)
{
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    int32_t safeVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
    const int32_t ONE_VOLUME_LEVEL = 1;
    int32_t ret = 0;
    if (IsBlueTooth(curOutputDeviceType)) {
        switch (notificationId) {
            case RESTORE_VOLUME_NOTIFICATION_ID:
                ret = CheckRestoreDeviceVolume(DEVICE_TYPE_BLUETOOTH_A2DP);
                break;
            case INCREASE_VOLUME_NOTIFICATION_ID:
                ret = SetSystemVolumeLevel(STREAM_MUSIC, safeVolumeLevel + ONE_VOLUME_LEVEL);
                break;
            default:
                AUDIO_ERR_LOG("current state unsupport safe volume");
        }
    } else if (IsWiredHeadSet(curOutputDeviceType)) {
        switch (notificationId) {
            case RESTORE_VOLUME_NOTIFICATION_ID:
                ret = CheckRestoreDeviceVolume(DEVICE_TYPE_WIRED_HEADSET);
                break;
            case INCREASE_VOLUME_NOTIFICATION_ID:
                ret = SetSystemVolumeLevel(STREAM_MUSIC, safeVolumeLevel + ONE_VOLUME_LEVEL);
                break;
            default:
                AUDIO_ERR_LOG("current state unsupport safe volume");
        }
    } else {
        AUDIO_ERR_LOG("current output device unsupport safe volume");
        ret = ERROR;
    }
    return ret;
}

int32_t AudioVolumeManager::ResetRingerModeMute()
{
    if (audioPolicyManager_.SetStreamMute(STREAM_RING, true) == SUCCESS) {
        SetRingerModeMute(true);
    }
    return SUCCESS;
}

bool AudioVolumeManager::IsRingerModeMute()
{
    return ringerModeMute_.load();
}

void AudioVolumeManager::SetRingerModeMute(bool flag)
{
    ringerModeMute_.store(flag);
}

bool AudioVolumeManager::GetVolumeGroupInfosNotWait(std::vector<sptr<VolumeGroupInfo>> &infos)
{
    if (!isPrimaryMicModuleInfoLoaded_) {
        return false;
    }

    GetVolumeGroupInfo(infos);
    return true;
}

void AudioVolumeManager::SetDefaultDeviceLoadFlag(bool isLoad)
{
    isPrimaryMicModuleInfoLoaded_.store(isLoad);
}

bool AudioVolumeManager::GetLoadFlag()
{
    return isPrimaryMicModuleInfoLoaded_.load();
}

void AudioVolumeManager::NotifyVolumeGroup()
{
    std::lock_guard<std::mutex> lock(defaultDeviceLoadMutex_);
    SetDefaultDeviceLoadFlag(true);
}

void AudioVolumeManager::UpdateSafeVolumeByS4()
{
    AUDIO_INFO_LOG("Reset isBtFirstBoot by S4 reboot");
    isBtFirstBoot_ = true;
    return audioPolicyManager_.UpdateSafeVolumeByS4();
}

std::vector<std::shared_ptr<AllDeviceVolumeInfo>> AudioVolumeManager::GetAllDeviceVolumeInfo()
{
    std::vector<std::shared_ptr<AllDeviceVolumeInfo>> allDeviceVolumeInfo = {};
    std::shared_ptr<AllDeviceVolumeInfo> deviceVolumeInfo = std::make_shared<AllDeviceVolumeInfo>();
    auto deviceList = audioConnectedDevice_.GetDevicesInner(DeviceFlag::ALL_L_D_DEVICES_FLAG);
    for (auto &device : deviceList) {
        for (auto &streamType : AUDIO_STREAMTYPE_VOLUME_LIST) {
            if (streamType == STREAM_VOICE_CALL_ASSISTANT) {
                continue;
            }
            deviceVolumeInfo = audioPolicyManager_.GetAllDeviceVolumeInfo(device->deviceType_, streamType);
            if (deviceVolumeInfo != nullptr) {
                allDeviceVolumeInfo.push_back(deviceVolumeInfo);
            }
        }
    }
    return allDeviceVolumeInfo;
}

void AudioVolumeManager::SaveSystemVolumeLevelInfo(AudioStreamType streamType, int32_t volumeLevel,
    int32_t appUid, std::string invocationTime)
{
    AdjustVolumeInfo systemVolumeLevelInfo;
    systemVolumeLevelInfo.deviceType = curOutputDeviceType_;
    systemVolumeLevelInfo.streamType = streamType;
    systemVolumeLevelInfo.volumeLevel = volumeLevel;
    systemVolumeLevelInfo.appUid = appUid;
    systemVolumeLevelInfo.invocationTime = invocationTime;
    systemVolumeLevelInfo_->Add(systemVolumeLevelInfo);
}

void AudioVolumeManager::SaveVolumeKeyRegistrationInfo(std::string keyType, std::string registrationTime,
    int32_t subscriptionId, bool registrationResult)
{
    VolumeKeyEventRegistration volumeKeyEventRegistration;
    volumeKeyEventRegistration.keyType = keyType;
    volumeKeyEventRegistration.subscriptionId = subscriptionId;
    volumeKeyEventRegistration.registrationTime = registrationTime;
    volumeKeyEventRegistration.registrationResult = registrationResult;
    volumeKeyRegistrations_->Add(volumeKeyEventRegistration);
}

void AudioVolumeManager::GetSystemVolumeLevelInfo(std::vector<AdjustVolumeInfo> &systemVolumeLevelInfo)
{
    systemVolumeLevelInfo = systemVolumeLevelInfo_->GetData();
}

void AudioVolumeManager::GetVolumeKeyRegistrationInfo(std::vector<VolumeKeyEventRegistration> &keyRegistrationInfo)
{
    keyRegistrationInfo = volumeKeyRegistrations_->GetData();
}

int32_t AudioVolumeManager::ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration)
{
    CHECK_AND_RETURN_RET_LOG(duration >= CANCEL_FORCE_CONTROL_VOLUME_TYPE, ERR_INVALID_PARAM, "invalid duration");
    CHECK_AND_RETURN_RET_LOG(forceControlVolumeTypeMonitor_ != nullptr, ERR_UNKNOWN,
        "forceControlVolumeTypeMonitor_ is nullptr");
    std::lock_guard<std::mutex> lock(forceControlVolumeTypeMutex_);
    needForceControlVolumeType_ = (duration == CANCEL_FORCE_CONTROL_VOLUME_TYPE ? false : true);
    forceControlVolumeType_ = (duration == CANCEL_FORCE_CONTROL_VOLUME_TYPE ? STREAM_DEFAULT : volumeType);
    forceControlVolumeTypeMonitor_->SetTimer(duration, forceControlVolumeTypeMonitor_);
    return SUCCESS;
}

void AudioVolumeManager::OnTimerExpired()
{
    std::lock_guard<std::mutex> lock(forceControlVolumeTypeMutex_);
    needForceControlVolumeType_ = false;
    forceControlVolumeType_ = STREAM_DEFAULT;
}

bool AudioVolumeManager::IsNeedForceControlVolumeType()
{
    std::lock_guard<std::mutex> lock(forceControlVolumeTypeMutex_);
    return needForceControlVolumeType_;
}

AudioVolumeType AudioVolumeManager::GetForceControlVolumeType()
{
    std::lock_guard<std::mutex> lock(forceControlVolumeTypeMutex_);
    return forceControlVolumeType_;
}

ForceControlVolumeTypeMonitor::~ForceControlVolumeTypeMonitor()
{
    std::lock_guard<std::mutex> lock(monitorMtx_);
    StopMonitor();
}

void ForceControlVolumeTypeMonitor::OnTimeOut()
{
    {
        std::lock_guard<std::mutex> lock(monitorMtx_);
        StopMonitor();
    }
    audioVolumeManager_.OnTimerExpired();
}

void ForceControlVolumeTypeMonitor::StartMonitor(int32_t duration,
    std::shared_ptr<ForceControlVolumeTypeMonitor> cb)
{
    int32_t cbId = DelayedSingleton<AudioPolicyStateMonitor>::GetInstance()->RegisterCallback(
        cb, duration, CallbackType::ONE_TIME);
    if (cbId == INVALID_CB_ID) {
        AUDIO_ERR_LOG("Register AudioPolicyStateMonitor failed");
    } else {
        cbId_ = cbId;
    }
}

void ForceControlVolumeTypeMonitor::StopMonitor()
{
    if (cbId_ != INVALID_CB_ID) {
        DelayedSingleton<AudioPolicyStateMonitor>::GetInstance()->UnRegisterCallback(cbId_);
        cbId_ = INVALID_CB_ID;
    }
}

void ForceControlVolumeTypeMonitor::SetTimer(int32_t duration,
    std::shared_ptr<ForceControlVolumeTypeMonitor> cb)
{
    std::lock_guard<std::mutex> lock(monitorMtx_);
    StopMonitor();
    if (duration == CANCEL_FORCE_CONTROL_VOLUME_TYPE) {
        return;
    }
    duration_ = (duration > MAX_DURATION_TIME_S ? MAX_DURATION_TIME_S : duration);
    StartMonitor(duration_, cb);
}

int32_t AudioVolumeManager::SetSystemVolumeDegree(AudioStreamType streamType, int32_t volumeDegree,
    int32_t zoneId)
{
    int32_t volumeLevel = VolumeUtils::VolumeDegreeToLevel(volumeDegree);
    int32_t currentVolumeDegree = GetSystemVolumeDegree(streamType);
    int32_t currentVolumeLevel = VolumeUtils::VolumeDegreeToLevel(currentVolumeDegree);
    if (volumeLevel == currentVolumeLevel) {
        volumeDegree = currentVolumeDegree;
        AUDIO_WARNING_LOG("volume level dont change, keep volume degree=%{public}d", volumeDegree);
    }

    Volume vol{};
    vol.isMute = volumeDegree == 0;
    vol.volumeInt = volumeLevel;
    vol.volumeDegree = static_cast<uint32_t>(volumeDegree);
    vol.volumeFloat = audioPolicyManager_.CalculateVolumeDb(volumeDegree, MAX_VOLUME_DEGREE);
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    SetSharedVolume(streamType, curOutputDeviceType, vol);

    return audioPolicyManager_.SetSystemVolumeDegree(VolumeUtils::GetVolumeTypeFromStreamType(streamType),
        volumeDegree);
}

int32_t AudioVolumeManager::GetSystemVolumeDegree(AudioStreamType streamType)
{
    return audioPolicyManager_.GetSystemVolumeDegree(streamType);
}

int32_t AudioVolumeManager::GetMinVolumeDegree(AudioVolumeType volumeType) const
{
    if (volumeType == STREAM_ALL) {
        volumeType = STREAM_MUSIC;
    }
    return audioPolicyManager_.GetMinVolumeDegree(volumeType);
}
}
}
