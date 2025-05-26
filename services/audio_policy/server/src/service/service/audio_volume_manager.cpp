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
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "i_policy_provider.h"
#include "audio_spatialization_service.h"
#include "audio_safe_volume_notification.h"

#include "audio_server_proxy.h"
#include "audio_policy_utils.h"

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
    return true;
}
void AudioVolumeManager::DeInit(void)
{
    volumeVector_ = nullptr;
    sharedAbsVolumeScene_ = nullptr;
    policyVolumeMap_ = nullptr;
    safeVolumeExit_ = true;
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

int32_t AudioVolumeManager::GetMaxVolumeLevel(AudioVolumeType volumeType) const
{
    if (volumeType == STREAM_ALL) {
        volumeType = STREAM_MUSIC;
    }
    return audioPolicyManager_.GetMaxVolumeLevel(volumeType);
}

int32_t AudioVolumeManager::GetMinVolumeLevel(AudioVolumeType volumeType) const
{
    if (volumeType == STREAM_ALL) {
        volumeType = STREAM_MUSIC;
    }
    return audioPolicyManager_.GetMinVolumeLevel(volumeType);
}

bool AudioVolumeManager::GetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume &vol)
{
    CHECK_AND_RETURN_RET_LOG(volumeVector_ != nullptr, false, "Get shared memory failed!");
    size_t index = 0;
    if (!IPolicyProvider::GetVolumeIndex(streamType, GetVolumeGroupForDevice(deviceType), index) ||
        index >= IPolicyProvider::GetVolumeVectorSize()) {
        return false;
    }
    vol.isMute = volumeVector_[index].isMute;
    vol.volumeFloat = volumeVector_[index].volumeFloat;
    vol.volumeInt = volumeVector_[index].volumeInt;
    return true;
}

bool AudioVolumeManager::SetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume vol)
{
    CHECK_AND_RETURN_RET_LOG(volumeVector_ != nullptr, false, "Set shared memory failed!");
    size_t index = 0;
    if (!IPolicyProvider::GetVolumeIndex(streamType, GetVolumeGroupForDevice(deviceType), index) ||
        index >= IPolicyProvider::GetVolumeVectorSize()) {
        AUDIO_INFO_LOG("Don't find and Set Shared Volume failed");
        return false;
    }
    volumeVector_[index].isMute = vol.isMute;
    volumeVector_[index].volumeFloat = vol.volumeFloat;
    volumeVector_[index].volumeInt = vol.volumeInt;
    AUDIO_INFO_LOG("Success Set Shared Volume with StreamType:%{public}d, DeviceType:%{public}d", streamType,
        deviceType);

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

int32_t AudioVolumeManager::GetAppVolumeLevel(int32_t appUid)
{
    return audioPolicyManager_.GetAppVolumeLevel(appUid);
}

int32_t AudioVolumeManager::GetSystemVolumeLevel(AudioStreamType streamType)
{
    Trace trace("AudioVolumeManager::GetSystemVolumeLevel");
    if (streamType == STREAM_RING && !IsRingerModeMute()) {
        AUDIO_PRERELEASE_LOGW("return 0 when dual tone ring");
        return DUAL_TONE_RING_VOLUME;
    }
    {
        DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC && streamType != STREAM_VOICE_CALL &&
            curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
            A2dpDeviceConfigInfo info;
            bool ret = audioA2dpDevice_.GetA2dpDeviceInfo(btDevice, info);
            if (ret && info.absVolumeSupport) {
                return info.mute ? 0 : info.volumeLevel;
            }
        }
    }
    return audioPolicyManager_.GetSystemVolumeLevel(streamType);
}

int32_t AudioVolumeManager::GetSystemVolumeLevelNoMuteState(AudioStreamType streamType)
{
    return audioPolicyManager_.GetSystemVolumeLevelNoMuteState(streamType);
}

void AudioVolumeManager::SetVolumeForSwitchDevice(DeviceType deviceType, const std::string &newSinkName)
{
    Trace trace("AudioVolumeManager::SetVolumeForSwitchDevice:" + std::to_string(deviceType));
    // Load volume from KvStore and set volume for each stream type
    audioPolicyManager_.SetVolumeForSwitchDevice(deviceType);

    // The volume of voice_call needs to be adjusted separately
    if (audioSceneManager_.GetAudioScene(true) == AUDIO_SCENE_PHONE_CALL) {
        SetVoiceCallVolume(GetSystemVolumeLevel(STREAM_VOICE_CALL));
    }

    UpdateVolumeForLowLatency();
}

int32_t AudioVolumeManager::SetVoiceRingtoneMute(bool isMute)
{
    AUDIO_INFO_LOG("Set Voice Ringtone is %{public}d", isMute);
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
    AudioServerProxy::GetInstance().SetVoiceVolumeProxy(volumeDb);
    AUDIO_INFO_LOG("SetVoiceVolume: %{public}f", volumeDb);
}

void AudioVolumeManager::UpdateVolumeForLowLatency()
{
    Trace trace("AudioVolumeManager::UpdateVolumeForLowLatency");
    // update volumes for low latency streams when loading volumes from the database.
    Volume vol = {false, 1.0f, 0};
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    for (auto iter = VOLUME_TYPE_LIST.begin(); iter != VOLUME_TYPE_LIST.end(); iter++) {
        vol.isMute = GetStreamMute(*iter);
        vol.volumeInt = static_cast<uint32_t>(GetSystemVolumeLevelNoMuteState(*iter));
        vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDb(*iter,
            (vol.isMute ? 0 : vol.volumeInt), curOutputDeviceType);
        SetSharedVolume(*iter, curOutputDeviceType, vol);
    }
    SetSharedAbsVolumeScene(audioPolicyManager_.IsAbsVolumeScene());
}

void AudioVolumeManager::InitKVStore()
{
    audioPolicyManager_.InitKVStore();
    UpdateVolumeForLowLatency();
    AudioSpatializationService::GetAudioSpatializationService().InitSpatializationState();
}

void AudioVolumeManager::CheckToCloseNotification(AudioStreamType streamType, int32_t volumeLevel)
{
    AUDIO_INFO_LOG("enter");
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
    DeviceCategory curOutputDeviceCategory = audioActiveDevice_.GetCurrentOutputDeviceCategory();
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

bool AudioVolumeManager::IsAppVolumeMute(int32_t appUid, bool owned)
{
    AUDIO_INFO_LOG("enter AudioVolumeManager::IsAppVolumeMute");
    bool result = audioPolicyManager_.IsAppVolumeMute(appUid, owned);
    return result;
}

int32_t AudioVolumeManager::SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel)
{
    int32_t result;
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC && streamType !=STREAM_VOICE_CALL &&
        curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        result = SetA2dpDeviceVolume(btDevice, volumeLevel, true);
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
#else
    (void)result;
#endif
    }
    int32_t sVolumeLevel = SelectDealSafeVolume(streamType, volumeLevel);
    CheckToCloseNotification(streamType, volumeLevel);
    CHECK_AND_RETURN_RET_LOG(sVolumeLevel == volumeLevel, ERR_SET_VOL_FAILED_BY_SAFE_VOL,
        "safevolume did not deal");
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

int32_t AudioVolumeManager::SetSystemVolumeLevelWithDevice(AudioStreamType streamType, int32_t volumeLevel,
    DeviceType deviceType)
{
    int32_t result;
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    if (VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC && streamType !=STREAM_VOICE_CALL &&
        curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        std::string btDevice = audioActiveDevice_.GetActiveBtDeviceMac();
        result = SetA2dpDeviceVolume(btDevice, volumeLevel, true);
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
        }
#else
    (void)result;
#endif
    }
    int32_t sVolumeLevel = SelectDealSafeVolume(streamType, volumeLevel);
    CheckToCloseNotification(streamType, volumeLevel);
    CHECK_AND_RETURN_RET_LOG(sVolumeLevel == volumeLevel, ERR_SET_VOL_FAILED_BY_SAFE_VOL,
        "safevolume did not deal");
    result = audioPolicyManager_.SetSystemVolumeLevelWithDevice(VolumeUtils::GetVolumeTypeFromStreamType(streamType),
        volumeLevel, deviceType);
    if (result == SUCCESS && (streamType == STREAM_VOICE_CALL || streamType == STREAM_VOICE_COMMUNICATION)) {
        SetVoiceCallVolume(volumeLevel);
    }
    Volume vol = {false, 1.0f, 0};
    vol.isMute = volumeLevel == 0 ? true : false;
    vol.volumeInt = static_cast<uint32_t>(volumeLevel);
    vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDb(streamType, volumeLevel, curOutputDeviceType);
    SetSharedVolume(streamType, curOutputDeviceType, vol);
    return result;
}

int32_t AudioVolumeManager::SelectDealSafeVolume(AudioStreamType streamType, int32_t volumeLevel)
{
    int32_t sVolumeLevel = volumeLevel;
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    DeviceCategory curOutputDeviceCategory = audioActiveDevice_.GetCurrentOutputDeviceCategory();
    if (sVolumeLevel > audioPolicyManager_.GetSafeVolumeLevel() &&
        VolumeUtils::GetVolumeTypeFromStreamType(streamType) == STREAM_MUSIC) {
        switch (curOutputDeviceType) {
            case DEVICE_TYPE_BLUETOOTH_A2DP:
            case DEVICE_TYPE_BLUETOOTH_SCO:
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
    if (curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_A2DP || curOutputDeviceType == DEVICE_TYPE_BLUETOOTH_SCO) {
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

int32_t AudioVolumeManager::HandleAbsBluetoothVolume(const std::string &macAddress, const int32_t volumeLevel)
{
    int32_t sVolumeLevel = 0;
    if (isBtFirstBoot_) {
        sVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
        AUDIO_INFO_LOG("Btfirstboot set volume use safe volume");
        isBtFirstBoot_ = false;
        Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(macAddress, sVolumeLevel);
    } else {
        sVolumeLevel = DealWithSafeVolume(volumeLevel, true);
        if (sVolumeLevel != volumeLevel) {
            Bluetooth::AudioA2dpManager::SetDeviceAbsVolume(macAddress, sVolumeLevel);
        }
    }
    return sVolumeLevel;
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

int32_t AudioVolumeManager::DealWithSafeVolume(const int32_t volumeLevel, bool isA2dpDevice)
{
    if (isA2dpDevice) {
        DeviceCategory curOutputDeviceCategory = audioActiveDevice_.GetCurrentOutputDeviceCategory();
        AUDIO_INFO_LOG("bluetooth Category:%{public}d", curOutputDeviceCategory);
        if (curOutputDeviceCategory == BT_SOUNDBOX || curOutputDeviceCategory == BT_CAR) {
            return volumeLevel;
        }
    }

    int32_t sVolumeLevel = volumeLevel;
    safeStatusBt_ = audioPolicyManager_.GetCurrentDeviceSafeStatus(DEVICE_TYPE_BLUETOOTH_A2DP);
    safeStatus_ = audioPolicyManager_.GetCurrentDeviceSafeStatus(DEVICE_TYPE_WIRED_HEADSET);
    if ((safeStatusBt_ == SAFE_INACTIVE && isA2dpDevice) ||
        (safeStatus_ == SAFE_INACTIVE && !isA2dpDevice)) {
        CreateCheckMusicActiveThread();
        return sVolumeLevel;
    }

    if ((isA2dpDevice && safeStatusBt_ == SAFE_ACTIVE) ||
        (!isA2dpDevice && safeStatus_ == SAFE_ACTIVE)) {
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
        if (audioActiveDevice_.GetCurrentOutputDeviceCategory() != BT_CAR &&
            audioActiveDevice_.GetCurrentOutputDeviceCategory() != BT_SOUNDBOX) {
            return true;
        }
    }
    return false;
}

int32_t AudioVolumeManager::CheckActiveMusicTime()
{
    AUDIO_INFO_LOG("enter");
    int32_t safeVolume = audioPolicyManager_.GetSafeVolumeLevel();
    while (!safeVolumeExit_) {
        bool activeMusic = audioSceneManager_.IsStreamActive(STREAM_MUSIC);
        bool isUpSafeVolume = GetSystemVolumeLevel(STREAM_MUSIC) > safeVolume ? true : false;
        streamMusicVol_ = isUpSafeVolume ? GetSystemVolumeLevel(STREAM_MUSIC) : streamMusicVol_;
        DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
        AUDIO_INFO_LOG("activeMusic:%{public}d, deviceType_:%{public}d, isUpSafeVolume:%{public}d",
            activeMusic, curOutputDeviceType, isUpSafeVolume);
        if (activeMusic && (safeStatusBt_ == SAFE_INACTIVE) && isUpSafeVolume &&
            IsBlueTooth(curOutputDeviceType)) {
            audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP, streamMusicVol_);
            CheckBlueToothActiveMusicTime(safeVolume);
        } else if (activeMusic && (safeStatus_ == SAFE_INACTIVE) && isUpSafeVolume &&
            IsWiredHeadSet(curOutputDeviceType)) {
            audioPolicyManager_.SetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET, streamMusicVol_);
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

void AudioVolumeManager::RestoreSafeVolume(AudioStreamType streamType, int32_t safeVolume)
{
    userSelect_ = false;
    isDialogSelectDestroy_.store(false);

    if (GetSystemVolumeLevel(streamType) <= safeVolume) {
        AUDIO_INFO_LOG("current volume <= safe volume, don't update volume.");
        return;
    }

    AUDIO_INFO_LOG("restore safe volume.");
    audioPolicyManager_.SetRestoreVolumeFlag(true);
    SetSystemVolumeLevel(streamType, safeVolume);
    audioPolicyManager_.SetRestoreVolumeFlag(false);
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
    AUDIO_INFO_LOG("enter");
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
        SetSystemVolumeLevel(STREAM_MUSIC, volumeLevel);
    }
}

int32_t AudioVolumeManager::SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support)
{
    // Maximum number of attempts, preventing situations where a2dp device has not yet finished coming online.
    int maxRetries = 3;
    int retryCount = 0;
    while (retryCount < maxRetries) {
        retryCount++;
        if (audioA2dpDevice_.SetA2dpDeviceAbsVolumeSupport(macAddress, support)) {
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
    const DeviceType &deviceType)
{
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
    result = audioPolicyManager_.SetStreamMute(streamType, mute, streamUsage, curOutputDeviceType);

    Volume vol = {false, 1.0f, 0};
    vol.isMute = mute;
    vol.volumeInt = static_cast<uint32_t>(GetSystemVolumeLevelNoMuteState(streamType));
    vol.volumeFloat = audioPolicyManager_.GetSystemVolumeInDb(streamType,
        (mute ? 0 : vol.volumeInt), curOutputDeviceType);
    SetSharedVolume(streamType, curOutputDeviceType, vol);

    return result;
}

bool AudioVolumeManager::GetStreamMute(AudioStreamType streamType) const
{
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
    for (auto& v : volumeGroups_) {
        sptr<VolumeGroupInfo> info = new(std::nothrow) VolumeGroupInfo(v->volumeGroupId_, v->mappingId_, v->groupName_,
            v->networkId_, v->connectType_);
        volumeGroupInfos.push_back(info);
    }
}

int32_t AudioVolumeManager::DealWithEventVolume(const int32_t notificationId)
{
    DeviceType curOutputDeviceType = audioActiveDevice_.GetCurrentOutputDeviceType();
    int32_t restoreVolume = 0;
    const int32_t ONE_VOLUME_LEVEL = 1;
    int32_t safeVolumeLevel = audioPolicyManager_.GetSafeVolumeLevel();
    int32_t ret = 0;
    bool isRestoreFlag = false;
    if (IsBlueTooth(curOutputDeviceType)) {
        switch (notificationId) {
            case RESTORE_VOLUME_NOTIFICATION_ID:
                restoreVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_BLUETOOTH_A2DP);
                isRestoreFlag = restoreVolume > safeVolumeLevel ? true : false;
                ret = isRestoreFlag ? SetSystemVolumeLevel(STREAM_MUSIC, restoreVolume) : ERROR;
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
                restoreVolume = audioPolicyManager_.GetRestoreVolumeLevel(DEVICE_TYPE_WIRED_HEADSET);
                isRestoreFlag = restoreVolume > safeVolumeLevel ? true : false;
                ret = isRestoreFlag ? SetSystemVolumeLevel(STREAM_MUSIC, restoreVolume) : ERROR;
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
    AUDIO_INFO_LOG("Set RingerModeMute_: %{public}d", flag);
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

}
}
