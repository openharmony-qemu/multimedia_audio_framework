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

#include "audio_volume_manager.h"
#include "../fuzz_utils.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
typedef void (*TestPtr)();

void AudioVolumeManagerInitSharedVolumeFuzzTest()
{
    std::shared_ptr<AudioSharedMemory> buffer;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.InitSharedVolume(buffer);
}

void AudioVolumeManagerSetVoiceRingtoneMuteFuzzTest()
{
    bool isMute = g_fuzzUtils.GetData<bool>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.SetVoiceRingtoneMute(isMute);
}

void AudioVolumeManagerHandleAbsBluetoothVolumeFuzzTest()
{
    std::string macAddress = "11:22:33:44:55:66";
    int32_t volumeLevel = g_fuzzUtils.GetData<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.isBtFirstBoot_ = g_fuzzUtils.GetData<bool>();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = g_fuzzUtils.GetData<DeviceCategory>();
    audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel);
}

void AudioVolumeManagerIsWiredHeadSetFuzzTest()
{
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.IsWiredHeadSet(deviceType);
}

void AudioVolumeManagerIsBlueToothFuzzTest()
{
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.IsBlueTooth(deviceType);
}

void AudioVolumeManagerCheckMixActiveMusicTimeFuzzTest()
{
    int32_t safeVolume = g_fuzzUtils.GetData<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = g_fuzzUtils.GetData<int64_t>();
    audioVolumeManager.CheckMixActiveMusicTime(safeVolume);
}

void AudioVolumeManagerCheckBlueToothActiveMusicTimeFuzzTest()
{
    int32_t safeVolume = g_fuzzUtils.GetData<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = g_fuzzUtils.GetData<int64_t>();
    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);
}

void AudioVolumeManagerCheckWiredActiveMusicTimeFuzzTest()
{
    int32_t safeVolume = g_fuzzUtils.GetData<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = g_fuzzUtils.GetData<int64_t>();
    audioVolumeManager.CheckWiredActiveMusicTime(safeVolume);
}

void AudioVolumeManagerRestoreSafeVolumeFuzzTest()
{
    AudioStreamType streamType = g_fuzzUtils.GetData<AudioStreamType>();
    int32_t safeVolume = g_fuzzUtils.GetData<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.RestoreSafeVolume(streamType, safeVolume);
}

void AudioVolumeManagerSetSafeVolumeCallbackFuzzTest()
{
    AudioStreamType streamType = g_fuzzUtils.GetData<AudioStreamType>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();

    audioVolumeManager.SetSafeVolumeCallback(streamType);
}

void AudioVolumeManagerChangeDeviceSafeStatusFuzzTest()
{
    SafeStatus safeStatus = g_fuzzUtils.GetData<SafeStatus>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.ChangeDeviceSafeStatus(safeStatus);
}

void AudioVolumeManagerSetAbsVolumeSceneAsyncFuzzTest()
{
    std::string macAddress = "11:22:33:44:55:66";
    bool support = g_fuzzUtils.GetData<bool>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioActiveDevice_.SetActiveBtDeviceMac(macAddress);
    audioVolumeManager.SetAbsVolumeSceneAsync(macAddress, support, 0);
}

void AudioVolumeManagerDealWithEventVolumeFuzzTest()
{
    int32_t notificationId = g_fuzzUtils.GetData<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = g_fuzzUtils.GetData<DeviceCategory>();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = g_fuzzUtils.GetData<DeviceType>();
    audioVolumeManager.DealWithEventVolume(notificationId);
}

void AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.UpdateSafeVolumeByS4();
}

void AudioVolumeManagerSetDeviceAbsVolumeSupportedFuzzTest()
{
    std::string macAddress = "11:22:33:44:55:66";
    bool support = g_fuzzUtils.GetData<bool>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioActiveDevice_.SetActiveBtDeviceMac(macAddress);
    audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support, 0);
}

void AudioVolumeManagerSetStreamMuteFuzzTest()
{
    AudioVolumeType streamType = g_fuzzUtils.GetData<AudioVolumeType>();
    bool mute = g_fuzzUtils.GetData<bool>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    StreamUsage streamUsage = g_fuzzUtils.GetData<StreamUsage>();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    std::string macAddress = "11:22:33:44:55:66";
    audioVolumeManager.audioActiveDevice_.SetActiveBtDeviceMac(macAddress);
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    a2dpDeviceConfigInfo.absVolumeSupport = g_fuzzUtils.GetData<bool>();
    audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({macAddress, a2dpDeviceConfigInfo});
    audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType);
}

void AudioVolumeManagerGetMaxVolumeLevelFuzzTest()
{
    AudioVolumeType streamType = g_fuzzUtils.GetData<AudioVolumeType>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.GetMaxVolumeLevel(streamType);
}

void AudioVolumeManagerGetMinVolumeLevelFuzzTest()
{
    AudioVolumeType streamType = g_fuzzUtils.GetData<AudioVolumeType>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.GetMinVolumeLevel(streamType);
}

void AudioVolumeManagerGetAllDeviceVolumeInfoFuzzTest()
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    DeviceRole deviceRole = g_fuzzUtils.GetData<DeviceRole>();

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(
        deviceType, deviceRole);
    audioVolumeManager->audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);
    audioVolumeManager->GetAllDeviceVolumeInfo();
}

void AudioVolumeManagerInitFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler = std::make_shared<AudioPolicyServerHandler>();

    audioVolumeManager.DeInit();
    audioVolumeManager.Init(audioPolicyServerHandler);
}

void AudioVolumeManagerInitKVStoreFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.InitKVStore();
}

void AudioVolumeManagerForceVolumeKeyControlTypeFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    AudioVolumeType volumeType = g_fuzzUtils.GetData<AudioVolumeType>();
    int32_t duration = g_fuzzUtils.GetData<int32_t>();
    audioVolumeManager.forceControlVolumeTypeMonitor_ = make_shared<ForceControlVolumeTypeMonitor>();
    audioVolumeManager.ForceVolumeKeyControlType(volumeType, duration);
}

void AudioVolumeManagerSetAdjustVolumeForZoneFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t zoneId = g_fuzzUtils.GetData<int32_t>();
    audioVolumeManager.SetAdjustVolumeForZone(zoneId);
}

void AudioVolumeManagerGetSystemVolumeLevelFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = g_fuzzUtils.GetData<AudioStreamType>();
    int32_t zoneId = g_fuzzUtils.GetData<int32_t>();
    bool flag = g_fuzzUtils.GetData<bool>();
    audioVolumeManager.SetRingerModeMute(flag);
    audioVolumeManager.GetSystemVolumeLevel(streamType, zoneId);
}

void AudioVolumeManagerCheckToCloseNotificationFuzzTest()
{
    bool isPCVolumeEnable = g_fuzzUtils.GetData<bool>();
    VolumeUtils::SetPCVolumeEnable(isPCVolumeEnable);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = g_fuzzUtils.GetData<AudioStreamType>();
    int32_t volumeLevel = g_fuzzUtils.GetData<int32_t>();
    audioVolumeManager.increaseNIsShowing_ = g_fuzzUtils.GetData<bool>();
    audioVolumeManager.restoreNIsShowing_ = g_fuzzUtils.GetData<bool>();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = g_fuzzUtils.GetData<DeviceType>();
    audioVolumeManager.CheckToCloseNotification(streamType, volumeLevel);
}

void AudioVolumeManagerSetAppVolumeMutedFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t appUid = g_fuzzUtils.GetData<int32_t>();
    bool muted = g_fuzzUtils.GetData<bool>();
    audioVolumeManager.SetAppVolumeMuted(appUid, muted);
}

void AudioVolumeManagerIsAppVolumeMuteFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t appUid = g_fuzzUtils.GetData<int32_t>();
    bool owned = g_fuzzUtils.GetData<bool>();
    bool isMute;
    audioVolumeManager.IsAppVolumeMute(appUid, owned, isMute);
}

void AudioVolumeManagerHandleNearlinkDeviceAbsVolumeFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = g_fuzzUtils.GetData<AudioStreamType>();
    int32_t volumeLevel = g_fuzzUtils.GetData<int32_t>();
    DeviceType curOutputDeviceType = g_fuzzUtils.GetData<DeviceType>();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.macAddress_ = "11:22:33:44:55:66";
    bool isClear = g_fuzzUtils.GetData<bool>();
    if (isClear) {
        audioVolumeManager.audioActiveDevice_.currentActiveDevice_.macAddress_.clear();
    }
    audioVolumeManager.HandleNearlinkDeviceAbsVolume(streamType, volumeLevel, curOutputDeviceType);
}

void AudioVolumeManagerSetA2dpDeviceVolumeFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    std::string macAddress = "11:22:33:44:55:66";
    int32_t volumeLevel = g_fuzzUtils.GetData<int32_t>();
    bool internalCall = g_fuzzUtils.GetData<bool>();
    bool isAdd = g_fuzzUtils.GetData<bool>();
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    if (isAdd) {
        audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({macAddress, a2dpDeviceConfigInfo});
    }
    audioVolumeManager.SetA2dpDeviceVolume(macAddress, volumeLevel, internalCall);
    audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_.clear();
}

void AudioVolumeManagerCancelSafeVolumeNotificationFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t notificationId = g_fuzzUtils.GetData<int32_t>();
    audioVolumeManager.CancelSafeVolumeNotification(notificationId);
}

void AudioVolumeManagerSetRestoreVolumeLevelFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    int32_t curDeviceVolume = g_fuzzUtils.GetData<int32_t>();
    audioVolumeManager.SetRestoreVolumeLevel(deviceType, curDeviceVolume);
}

void AudioVolumeManagerCheckLowerDeviceVolumeFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    audioVolumeManager.CheckLowerDeviceVolume(deviceType);
}

void AudioVolumeManagerOnReceiveEventFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AAFwk::Want want;
    EventFwk::CommonEventData eventData;
    std::string action;
    bool isRestore = g_fuzzUtils.GetData<bool>();
    if (isRestore) {
        action = AUDIO_RESTORE_VOLUME_EVENT;
    } else {
        action = AUDIO_INCREASE_VOLUME_EVENT;
    }
    want.SetAction(action);
    eventData.SetWant(want);
    audioVolumeManager.OnReceiveEvent(eventData);
}

void AudioVolumeManagerSetDeviceSafeVolumeStatusFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.userSelect_ = g_fuzzUtils.GetData<bool>();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = g_fuzzUtils.GetData<DeviceType>();
    audioVolumeManager.SetDeviceSafeVolumeStatus();
}

void AudioVolumeManagerDisableSafeMediaVolumeFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.DisableSafeMediaVolume();
}

void AudioVolumeManagerResetRingerModeMuteFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.ResetRingerModeMute();
}

void AudioVolumeManagerGetLoadFlagFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    bool isLoad = g_fuzzUtils.GetData<bool>();
    audioVolumeManager.SetDefaultDeviceLoadFlag(isLoad);
    audioVolumeManager.GetLoadFlag();
}

void AudioVolumeManagerOnTimerExpiredFuzzTest()
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.OnTimerExpired();
    audioVolumeManager.GetForceControlVolumeType();
}

vector<TestFuncs> g_testFuncs = {
    AudioVolumeManagerInitSharedVolumeFuzzTest,
    AudioVolumeManagerSetVoiceRingtoneMuteFuzzTest,
    AudioVolumeManagerHandleAbsBluetoothVolumeFuzzTest,
    AudioVolumeManagerIsWiredHeadSetFuzzTest,
    AudioVolumeManagerIsBlueToothFuzzTest,
    AudioVolumeManagerCheckMixActiveMusicTimeFuzzTest,
    AudioVolumeManagerCheckBlueToothActiveMusicTimeFuzzTest,
    AudioVolumeManagerCheckWiredActiveMusicTimeFuzzTest,
    AudioVolumeManagerRestoreSafeVolumeFuzzTest,
    AudioVolumeManagerSetSafeVolumeCallbackFuzzTest,
    AudioVolumeManagerChangeDeviceSafeStatusFuzzTest,
    AudioVolumeManagerSetAbsVolumeSceneAsyncFuzzTest,
    AudioVolumeManagerDealWithEventVolumeFuzzTest,
    AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest,
    AudioVolumeManagerSetDeviceAbsVolumeSupportedFuzzTest,
    AudioVolumeManagerSetStreamMuteFuzzTest,
    AudioVolumeManagerGetMaxVolumeLevelFuzzTest,
    AudioVolumeManagerGetMinVolumeLevelFuzzTest,
    AudioVolumeManagerGetAllDeviceVolumeInfoFuzzTest,
    AudioVolumeManagerInitFuzzTest,
    AudioVolumeManagerInitKVStoreFuzzTest,
    AudioVolumeManagerForceVolumeKeyControlTypeFuzzTest,
    AudioVolumeManagerSetAdjustVolumeForZoneFuzzTest,
    AudioVolumeManagerGetSystemVolumeLevelFuzzTest,
    AudioVolumeManagerCheckToCloseNotificationFuzzTest,
    AudioVolumeManagerSetAppVolumeMutedFuzzTest,
    AudioVolumeManagerIsAppVolumeMuteFuzzTest,
    AudioVolumeManagerHandleNearlinkDeviceAbsVolumeFuzzTest,
    AudioVolumeManagerSetA2dpDeviceVolumeFuzzTest,
    AudioVolumeManagerCancelSafeVolumeNotificationFuzzTest,
    AudioVolumeManagerSetRestoreVolumeLevelFuzzTest,
    AudioVolumeManagerCheckLowerDeviceVolumeFuzzTest,
    AudioVolumeManagerOnReceiveEventFuzzTest,
    AudioVolumeManagerSetDeviceSafeVolumeStatusFuzzTest,
    AudioVolumeManagerDisableSafeMediaVolumeFuzzTest,
    AudioVolumeManagerResetRingerModeMuteFuzzTest,
    AudioVolumeManagerGetLoadFlagFuzzTest,
    AudioVolumeManagerOnTimerExpiredFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}