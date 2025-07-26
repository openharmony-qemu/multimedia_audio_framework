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
using namespace std;

namespace OHOS {
namespace AudioStandard {

const int32_t NUM_2 = 2;
const int32_t SAFE_VOLUME_LIMIT = 100;
const int32_t RESTORE_VOLUME_NOTIFICATION_ID = 116000;
const int32_t INCREASE_VOLUME_NOTIFICATION_ID = 116001;
const uint32_t NOTIFICATION_BANNER_FLAG = 1 << 9;
typedef void (*TestPtr)(const uint8_t *, size_t);

const vector<AudioStreamType> g_testStreamTypes = {
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
    DEVICE_TYPE_MAX
};

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void AudioVolumeManagerInitSharedVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    std::shared_ptr<AudioSharedMemory> buffer;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.InitSharedVolume(buffer);
}

void AudioVolumeManagerSetVoiceRingtoneMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    bool isMute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.SetVoiceRingtoneMute(isMute);
}

void AudioVolumeManagerHandleAbsBluetoothVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    std::string macAddress = "11:22:33:44:55:66";
    int32_t volumeLevel = static_cast<int32_t>(size);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    vector<DeviceCategory> testDeviceCategory = {
        CATEGORY_DEFAULT,
        BT_HEADPHONE,
        BT_SOUNDBOX,
        BT_CAR,
        BT_GLASSES,
        BT_WATCH,
        BT_HEARAID,
        BT_UNWEAR_HEADPHONE,
    };

    audioVolumeManager.isBtFirstBoot_ = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    uint32_t index = static_cast<uint32_t>(size) % testDeviceCategory.size();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = testDeviceCategory[index];

    audioVolumeManager.HandleAbsBluetoothVolume(macAddress, volumeLevel);
}

void AudioVolumeManagerIsWiredHeadSetFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testDeviceTypes.size();
    DeviceType deviceType = g_testDeviceTypes[index];
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.IsWiredHeadSet(deviceType);
}

void AudioVolumeManagerIsBlueToothFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testDeviceTypes.size();
    DeviceType deviceType = g_testDeviceTypes[index];
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.IsBlueTooth(deviceType);
}

void AudioVolumeManagerCheckMixActiveMusicTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t safeVolume = static_cast<int32_t>(size) % SAFE_VOLUME_LIMIT;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.activeSafeTimeBt_ = static_cast<int64_t>(size);
    audioVolumeManager.activeSafeTime_ = static_cast<int64_t>(size);
    audioVolumeManager.CheckMixActiveMusicTime(safeVolume);
}

void AudioVolumeManagerCheckBlueToothActiveMusicTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t safeVolume = static_cast<int32_t>(size) % SAFE_VOLUME_LIMIT;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = static_cast<int64_t>(size);
    audioVolumeManager.activeSafeTime_ = static_cast<int64_t>(size);
    audioVolumeManager.CheckBlueToothActiveMusicTime(safeVolume);
}

void AudioVolumeManagerCheckWiredActiveMusicTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t safeVolume = static_cast<int32_t>(size) % SAFE_VOLUME_LIMIT;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.startSafeTimeBt_ = 0;
    audioVolumeManager.activeSafeTimeBt_ = static_cast<int64_t>(size);
    audioVolumeManager.activeSafeTime_ = static_cast<int64_t>(size);
    audioVolumeManager.CheckWiredActiveMusicTime(safeVolume);
}

void AudioVolumeManagerRestoreSafeVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testStreamTypes.size();
    AudioStreamType streamType = g_testStreamTypes[index];
    int32_t safeVolume = static_cast<int32_t>(size) % SAFE_VOLUME_LIMIT;
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.RestoreSafeVolume(streamType, safeVolume);
}

void AudioVolumeManagerSetSafeVolumeCallbackFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testStreamTypes.size();
    AudioStreamType streamType = g_testStreamTypes[index];
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    audioVolumeManager.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();

    audioVolumeManager.SetSafeVolumeCallback(streamType);
}

void AudioVolumeManagerChangeDeviceSafeStatusFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<SafeStatus> testSafeStatus = {
        SAFE_UNKNOWN,
        SAFE_INACTIVE,
        SAFE_ACTIVE,
    };

    uint32_t index = static_cast<uint32_t>(size) % testSafeStatus.size();
    SafeStatus safeStatus = testSafeStatus[index];
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.ChangeDeviceSafeStatus(safeStatus);
}

void AudioVolumeManagerSetAbsVolumeSceneAsyncFuzzTest(const uint8_t *rawData, size_t size)
{
    std::string macAddress = "11:22:33:44:55:66";
    bool support = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioActiveDevice_.SetActiveBtDeviceMac(macAddress);
    audioVolumeManager.SetAbsVolumeSceneAsync(macAddress, support);
}

void AudioVolumeManagerDealWithEventVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<int32_t> testNotificationIds = {
        RESTORE_VOLUME_NOTIFICATION_ID,
        INCREASE_VOLUME_NOTIFICATION_ID,
        NOTIFICATION_BANNER_FLAG,
    };
    uint32_t index = static_cast<uint32_t>(size) % testNotificationIds.size();
    int32_t notificationId = testNotificationIds[index];
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    vector<DeviceCategory> testDeviceCategory = {
        CATEGORY_DEFAULT,
        BT_HEADPHONE,
        BT_SOUNDBOX,
        BT_CAR,
        BT_GLASSES,
        BT_WATCH,
        BT_HEARAID,
        BT_UNWEAR_HEADPHONE,
    };
    index = static_cast<uint32_t>(size) % testDeviceCategory.size();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceCategory_ = testDeviceCategory[index];

    index = static_cast<uint32_t>(size) % g_testDeviceTypes.size();
    audioVolumeManager.audioActiveDevice_.currentActiveDevice_.deviceType_ = g_testDeviceTypes[index];

    audioVolumeManager.DealWithEventVolume(notificationId);
}

void AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest(const uint8_t *rawData, size_t size)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.UpdateSafeVolumeByS4();
}

void AudioVolumeManagerSetDeviceAbsVolumeSupportedFuzzTest(const uint8_t *rawData, size_t size)
{
    std::string macAddress = "11:22:33:44:55:66";
    bool support = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.audioActiveDevice_.SetActiveBtDeviceMac(macAddress);
    audioVolumeManager.SetDeviceAbsVolumeSupported(macAddress, support);
}

void AudioVolumeManagerSetStreamMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<StreamUsage> testStreamUsages = {
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
        STREAM_USAGE_ACCESSIBILITY,
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
    uint32_t index = static_cast<uint32_t>(size) % g_testStreamTypes.size();
    AudioVolumeType streamType = g_testStreamTypes[index];
    bool mute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    index = static_cast<uint32_t>(size) % testStreamUsages.size();
    StreamUsage streamUsage = testStreamUsages[index];
    index = static_cast<uint32_t>(size) % g_testDeviceTypes.size();
    DeviceType deviceType = g_testDeviceTypes[index];
    audioVolumeManager.SetStreamMute(streamType, mute, streamUsage, deviceType);
}

void AudioVolumeManagerGetMaxVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testStreamTypes.size();
    AudioVolumeType streamType = g_testStreamTypes[index];
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.GetMaxVolumeLevel(streamType);
}

void AudioVolumeManagerGetMinVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size) % g_testStreamTypes.size();
    AudioVolumeType streamType = g_testStreamTypes[index];
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.GetMinVolumeLevel(streamType);
}

void AudioVolumeManagerGetAllDeviceVolumeInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<DeviceRole> testDeviceRoles = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    auto audioVolumeManager = std::make_shared<AudioVolumeManager>();
    uint32_t index = static_cast<uint32_t>(size) % g_testDeviceTypes.size();
    DeviceType deviceType = g_testDeviceTypes[index];
    index = static_cast<uint32_t>(size) % testDeviceRoles.size();
    DeviceRole deviceRole = testDeviceRoles[index];

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>(
        deviceType, deviceRole);
    audioVolumeManager->audioConnectedDevice_.AddConnectedDevice(remoteDeviceDescriptor);
    audioVolumeManager->GetAllDeviceVolumeInfo();
}

void AudioVolumeManagerInitFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler = std::make_shared<AudioPolicyServerHandler>();

    audioVolumeManager.DeInit();
    audioVolumeManager.Init(audioPolicyServerHandler);
}

void AudioVolumeManagerInitKVStoreFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.InitKVStore();
}

void AudioVolumeManagerForceVolumeKeyControlTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioVolumeManager& audioVolumeManager(AudioVolumeManager::GetInstance());

    audioVolumeManager.ForceVolumeKeyControlType(static_cast<AudioVolumeType>(size), size);
}

} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[] = {
    OHOS::AudioStandard::AudioVolumeManagerInitSharedVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetVoiceRingtoneMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerHandleAbsBluetoothVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerIsWiredHeadSetFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerIsBlueToothFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerCheckMixActiveMusicTimeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerCheckBlueToothActiveMusicTimeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerCheckWiredActiveMusicTimeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerRestoreSafeVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetSafeVolumeCallbackFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerChangeDeviceSafeStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAbsVolumeSceneAsyncFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerDealWithEventVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetDeviceAbsVolumeSupportedFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetStreamMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetMaxVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetMinVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetAllDeviceVolumeInfoFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitKVStoreFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerForceVolumeKeyControlTypeFuzzTest,
};

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    /* Run your code on data */
    if (data == nullptr || size <= 1) {
        return 0;
    }
    uint32_t len = OHOS::AudioStandard::GetArrLength(g_testPtrs);
    if (len > 0) {
        uint8_t firstByte = *data % len;
        if (firstByte >= len) {
            return 0;
        }
        data = data + 1;
        size = size - 1;
        g_testPtrs[firstByte](data, size);
    }
    return 0;
}