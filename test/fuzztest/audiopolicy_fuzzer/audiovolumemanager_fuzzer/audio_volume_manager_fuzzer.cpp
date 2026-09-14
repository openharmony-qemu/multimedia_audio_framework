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
#include "audio_interrupt_service.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {
const size_t THRESHOLD = 10;
const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";
const std::string DEFAULT_MAC_ADDRESS = "11:22:33:44:55:66";
const size_t MAX_STRING_LEN = 64;
constexpr int32_t TEST_DEVICE_ID = 1001;
typedef void (*TestPtr)();

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

const vector<DeviceType> g_testDeviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

const vector<AudioStreamType> g_testAudioStreamTypes = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_ACCESSIBILITY,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

const vector<StreamUsage> g_testStreamUsages = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_ASSISTANT,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_RINGTONE,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_AUDIOBOOK,
    STREAM_USAGE_NAVIGATION,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_RANGING,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_RINGTONE,
    STREAM_USAGE_VOICE_CALL_ASSISTANT,
    STREAM_USAGE_MAX,
};

const vector<DeviceCategory> g_testDeviceCategories = {
    CATEGORY_DEFAULT,
    BT_HEADPHONE,
    BT_SOUNDBOX,
    BT_CAR,
    BT_GLASSES,
    BT_UNWEAR_HEADPHONE,
};

const vector<DeviceRole> g_testDeviceRoles = {
    INPUT_DEVICE,
    OUTPUT_DEVICE,
};

const vector<SafeStatus> g_testSafeStatuses = {
    SAFE_UNKNOWN,
    SAFE_INACTIVE,
    SAFE_ACTIVE,
};

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

void AudioVolumeManagerInitSharedVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioSharedMemory> buffer;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    if (fdp.ConsumeBool()) {
        audioVolumeManager.DeInit();
        std::shared_ptr<AudioPolicyServerHandler> handler =
            std::make_shared<AudioPolicyServerHandler>();
        std::shared_ptr<AudioInterruptService> interruptService =
            std::make_shared<AudioInterruptService>();
        audioVolumeManager.Init(handler, interruptService);
    }
    audioVolumeManager.InitSharedVolume(buffer);
}

void AudioVolumeManagerSetVoiceRingtoneMuteFuzzTest(FuzzedDataProvider& fdp)
{
    bool isMute = fdp.ConsumeBool();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.SetVoiceRingtoneMute(isMute);
}

static AudioDeviceDescriptor& CurrentActiveDevice()
{
    auto descs = AudioRouterSelectStrategy::GetInstance().GetCurrentOutputDevice(SYSTEM_UID);
    auto desc = descs.empty() || !descs.back() ? make_shared<AudioDeviceDescriptor>() : descs.back();
    if (desc->deviceId_ == 0) {
        desc->deviceId_ = TEST_DEVICE_ID;
        AudioDeviceManager::GetAudioDeviceManager().AddConnectedDevices(desc);
        AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, {desc});
    }
    desc = AudioDeviceManager::GetAudioDeviceManager().FindConnectedDeviceById(desc->deviceId_);
    return *desc;
}

void AudioVolumeManagerHandleAbsBluetoothVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.isBtFirstBoot_ = fdp.ConsumeBool();
    CurrentActiveDevice().deviceCategory_ = PickValue(fdp, g_testDeviceCategories);
    audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel);
}

void AudioVolumeManagerIsWiredHeadSetFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.safeVolumeManager_.IsWiredHeadSet(deviceType);
}

void AudioVolumeManagerIsBlueToothFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = PickValue(fdp, g_testDeviceTypes);

    audioVolumeManager.safeVolumeManager_.IsBlueTooth(deviceDesc);
}

void AudioVolumeManagerCheckMixActiveMusicTimeFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t safeVolume = fdp.ConsumeIntegral<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = fdp.ConsumeIntegral<int64_t>();
    audioVolumeManager.CheckMixActiveMusicTime(safeVolume);
}

void AudioVolumeManagerCheckBlueToothActiveMusicTimeFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t safeVolume = fdp.ConsumeIntegral<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = fdp.ConsumeIntegral<int64_t>();
    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);
}

void AudioVolumeManagerCheckWiredActiveMusicTimeFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t safeVolume = fdp.ConsumeIntegral<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTime_ = fdp.ConsumeIntegral<int64_t>();
    audioVolumeManager.CheckWiredActiveMusicTime(safeVolume);
}

void AudioVolumeManagerRestoreSafeVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t safeVolume = fdp.ConsumeIntegral<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.RestoreSafeVolume(streamType, safeVolume);
}

void AudioVolumeManagerSetSafeVolumeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();

    audioVolumeManager.SetSafeVolumeCallback(streamType);
}

void AudioVolumeManagerChangeDeviceSafeStatusFuzzTest(FuzzedDataProvider& fdp)
{
    SafeStatus safeStatus = PickValue(fdp, g_testSafeStatuses);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.ChangeDeviceSafeStatus(safeStatus);
}

void AudioVolumeManagerSetAbsVolumeSceneAsyncFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    bool support = fdp.ConsumeBool();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioActiveDevice_.AddActiveBtDevice(macAddress);
    audioVolumeManager.SetAbsVolumeSceneAsync(macAddress, support, 0);
}

void AudioVolumeManagerDealWithEventVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t notificationId = fdp.ConsumeIntegral<int32_t>();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    CurrentActiveDevice().deviceCategory_ = PickValue(fdp, g_testDeviceCategories);
    CurrentActiveDevice().deviceType_ = PickValue(fdp, g_testDeviceTypes);
    audioVolumeManager.DealWithEventVolume(notificationId);
}

void AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.isBtFirstBoot_ = fdp.ConsumeBool();
    audioVolumeManager.UpdateSafeVolumeByS4();
}

void AudioVolumeManagerSetDeviceAbsVolumeSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    bool support = fdp.ConsumeBool();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioActiveDevice_.AddActiveBtDevice(macAddress);
    audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support, 0);
}

void AudioVolumeManagerSetStreamMuteFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeType streamType = PickValue(fdp, g_testAudioStreamTypes);
    bool mute = fdp.ConsumeBool();
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    StreamUsage streamUsage = PickValue(fdp, g_testStreamUsages);
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    audioVolumeManager.audioActiveDevice_.AddActiveBtDevice(macAddress);
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    a2dpDeviceConfigInfo.absVolumeSupport = fdp.ConsumeBool();
    audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({macAddress, a2dpDeviceConfigInfo});
    audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType);
}

void AudioVolumeManagerGetMaxVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeType streamType = PickValue(fdp, g_testAudioStreamTypes);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.GetMaxVolumeLevel(streamType);
}

void AudioVolumeManagerGetMinVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeType streamType = PickValue(fdp, g_testAudioStreamTypes);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.GetMinVolumeLevel(streamType);
}

void AudioVolumeManagerGetAllDeviceVolumeInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    DeviceRole deviceRole = PickValue(fdp, g_testDeviceRoles);

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(
        deviceType, deviceRole);
    audioVolumeManager->audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);
    audioVolumeManager->GetAllDeviceVolumeInfo();
}

void AudioVolumeManagerInitFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    if (fdp.ConsumeBool()) {
        audioVolumeManager.DeInit();
    }
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler =
        fdp.ConsumeBool() ? std::make_shared<AudioPolicyServerHandler>() : nullptr;
    std::shared_ptr<AudioInterruptService> interruptService =
        fdp.ConsumeBool() ? std::make_shared<AudioInterruptService>() : nullptr;
    audioVolumeManager.Init(audioPolicyServerHandler, interruptService);
}

void AudioVolumeManagerInitKVStoreFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    if (fdp.ConsumeBool()) {
        audioVolumeManager.DeInit();
        std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
        std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
        audioVolumeManager.Init(handler, interruptService);
    }
    audioVolumeManager.InitKVStore();
}

void AudioVolumeManagerForceVolumeKeyControlTypeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioVolumeType volumeType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t duration = fdp.ConsumeIntegral<int32_t>();
    audioVolumeManager.forceControlVolumeTypeMonitor_ = make_shared<ForceControlVolumeTypeMonitor>();
    audioVolumeManager.ForceVolumeKeyControlType(volumeType, duration);
}

void AudioVolumeManagerSetAdjustVolumeForZoneFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t zoneId = fdp.ConsumeIntegral<int32_t>();
    audioVolumeManager.SetAdjustVolumeForZone(zoneId);
}

void AudioVolumeManagerGetSystemVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t zoneId = fdp.ConsumeIntegral<int32_t>();
    bool flag = fdp.ConsumeBool();
    audioVolumeManager.SetRingerModeMute(flag);
    audioVolumeManager.GetSystemVolumeLevel(streamType, zoneId);
}

void AudioVolumeManagerGetSystemVolumeDegreeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t zoneId = fdp.ConsumeIntegral<int32_t>();
    bool flag = fdp.ConsumeBool();
    audioVolumeManager.SetRingerModeMute(flag);
    audioVolumeManager.GetSystemVolumeDegree(streamType, zoneId);
}

void AudioVolumeManagerCheckToCloseNotificationFuzzTest(FuzzedDataProvider& fdp)
{
    bool isPCVolumeEnable = fdp.ConsumeBool();
    VolumeUtils::SetPCVolumeEnable(isPCVolumeEnable);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    audioVolumeManager.increaseNIsShowing_ = fdp.ConsumeBool();
    audioVolumeManager.restoreNIsShowing_ = fdp.ConsumeBool();
    CurrentActiveDevice().deviceType_ = PickValue(fdp, g_testDeviceTypes);
    audioVolumeManager.CheckToCloseNotification(streamType, volumeLevel);
}

void AudioVolumeManagerSetAppVolumeMutedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    bool muted = fdp.ConsumeBool();
    audioVolumeManager.SetAppVolumeMuted(appUid, muted);
}

void AudioVolumeManagerIsAppVolumeMuteFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    bool owned = fdp.ConsumeBool();
    bool isMute;
    audioVolumeManager.IsAppVolumeMute(appUid, owned, isMute);
}

void AudioVolumeManagerHandleNearlinkDeviceAbsVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    int32_t volumeDegree = fdp.ConsumeIntegral<int32_t>();
    VolumeScale volume = {volumeLevel, volumeDegree};
    DeviceType curOutputDeviceType = PickValue(fdp, g_testDeviceTypes);
    CurrentActiveDevice().macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    bool isClear = fdp.ConsumeBool();
    if (isClear) {
        CurrentActiveDevice().macAddress_.clear();
    }
    audioVolumeManager.HandleNearlinkDeviceAbsVolume(streamType, volume, curOutputDeviceType);
}

void AudioVolumeManagerSetA2dpDeviceVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    VolumeScale volume = volumeLevel;
    bool internalCall = fdp.ConsumeBool();
    bool isAdd = fdp.ConsumeBool();
    A2dpDeviceConfigInfo a2dpDeviceConfigInfo;
    if (isAdd) {
        audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({macAddress, a2dpDeviceConfigInfo});
    }
    audioVolumeManager.SetA2dpDeviceVolume(macAddress, volume, internalCall);
    audioVolumeManager.audioA2dpDevice_.connectedA2dpDeviceMap_.clear();
}

void AudioVolumeManagerCancelSafeVolumeNotificationFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t notificationId = fdp.ConsumeIntegral<int32_t>();
    audioVolumeManager.safeVolumeManager_.CancelSafeVolumeNotification(notificationId);
}

void AudioVolumeManagerSetRestoreVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    int32_t curDeviceVolume = fdp.ConsumeIntegral<int32_t>();
    audioVolumeManager.SetRestoreVolumeLevel(deviceType, curDeviceVolume);
}

void AudioVolumeManagerCheckLowerDeviceVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    audioVolumeManager.CheckLowerDeviceVolume(deviceType);
}

void AudioVolumeManagerOnReceiveEventFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AAFwk::Want want;
    EventFwk::CommonEventData eventData;
    std::string action;
    bool isRestore = fdp.ConsumeBool();
    if (isRestore) {
        action = AUDIO_RESTORE_VOLUME_EVENT;
    } else {
        action = AUDIO_INCREASE_VOLUME_EVENT;
    }
    want.SetAction(action);
    eventData.SetWant(want);
    audioVolumeManager.OnReceiveEvent(eventData);
}

void AudioVolumeManagerSetDeviceSafeVolumeStatusFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.userSelect_ = fdp.ConsumeBool();
    CurrentActiveDevice().deviceType_ = PickValue(fdp, g_testDeviceTypes);
    audioVolumeManager.SetDeviceSafeVolumeStatus();
}

void AudioVolumeManagerDisableSafeMediaVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    CurrentActiveDevice().deviceType_ = PickValue(fdp, g_testDeviceTypes);
    audioVolumeManager.safeStatusBt_ = PickValue(fdp, g_testSafeStatuses);
    audioVolumeManager.safeStatus_ = PickValue(fdp, g_testSafeStatuses);
    audioVolumeManager.safeStatusSle_ = PickValue(fdp, g_testSafeStatuses);
    audioVolumeManager.isSafeVolumeDialogShowing_ = fdp.ConsumeBool();
    audioVolumeManager.DisableSafeMediaVolume();
}

void AudioVolumeManagerResetRingerModeMuteFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.isVoiceRingtoneMute_ = fdp.ConsumeBool();
    audioVolumeManager.ResetRingerModeMute();
}

void AudioVolumeManagerGetLoadFlagFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    bool isLoad = fdp.ConsumeBool();
    audioVolumeManager.SetDefaultDeviceLoadFlag(isLoad);
    audioVolumeManager.GetLoadFlag();
}

void AudioVolumeManagerOnTimerExpiredFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.needForceControlVolumeType_ = fdp.ConsumeBool();
    audioVolumeManager.forceControlVolumeType_ = PickValue(fdp, g_testAudioStreamTypes);
    audioVolumeManager.OnTimerExpired();
    audioVolumeManager.GetForceControlVolumeType();
}

void AudioVolumeManagerSetAppRingMutedFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    bool muted = fdp.ConsumeBool();
    audioVolumeManager.SetAppRingMuted(appUid, muted);
    audioVolumeManager.IsAppRingMuted(appUid);
}

void AudioVolumeManagerHandleA2dpAbsVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    int32_t volumeDegree = fdp.ConsumeIntegral<int32_t>();
    VolumeScale volume = {volumeLevel, volumeDegree};
    DeviceType curOutputDeviceType = PickValue(fdp, g_testDeviceTypes);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    audioVolumeManager.HandleA2dpAbsVolume(streamType, volume, curOutputDeviceType, macAddress);
}

void AudioVolumeManagerCheckNearlinkActiveMusicTimeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t safeVolume = fdp.ConsumeIntegral<int32_t>();
    audioVolumeManager.startSafeTimeSle_ = fdp.ConsumeIntegral<std::time_t>();
    audioVolumeManager.activeSafeTimeSle_ = fdp.ConsumeIntegral<int64_t>();
    audioVolumeManager.CheckNearlinkActiveMusicTime(safeVolume);
}

void AudioVolumeManagerCheckRestoreDeviceVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    audioVolumeManager.CheckRestoreDeviceVolume(deviceType);
    audioVolumeManager.RefreshActiveDeviceVolume();
}

void AudioVolumeManagerCheckRestoreDeviceVolumeNearlinkFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    int32_t btRestoreVolume = fdp.ConsumeIntegral<int32_t>();
    int32_t wiredRestoreVolume = fdp.ConsumeIntegral<int32_t>();
    int32_t sleRestoreVolume = fdp.ConsumeIntegral<int32_t>();
    int32_t safeVolume = 0;
    audioVolumeManager.CheckRestoreDeviceVolumeNearlink(btRestoreVolume, wiredRestoreVolume,
        sleRestoreVolume, safeVolume);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
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
    AudioVolumeManagerGetSystemVolumeDegreeFuzzTest,
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
    AudioVolumeManagerSetAppRingMutedFuzzTest,
    AudioVolumeManagerHandleA2dpAbsVolumeFuzzTest,
    AudioVolumeManagerCheckNearlinkActiveMusicTimeFuzzTest,
    AudioVolumeManagerCheckRestoreDeviceVolumeFuzzTest,
    AudioVolumeManagerCheckRestoreDeviceVolumeNearlinkFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}