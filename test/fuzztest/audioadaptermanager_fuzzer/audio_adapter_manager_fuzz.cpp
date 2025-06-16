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

#include "audio_adapter_manager.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

static AudioAdapterManager *audioAdapterManager_;

const int32_t NUM_2 = 2;
typedef void (*TestPtr)(const uint8_t *, size_t);

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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void AudioVolumeManagerIsAppVolumeMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    static uint32_t randomStep = 0;
    int32_t randIntValue = static_cast<int32_t>(size);
    randomStep += randIntValue;
    int32_t appUid = randIntValue;
    bool owned = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    bool isMute = static_cast<bool>(static_cast<uint32_t>(size + randomStep) % NUM_2);
    AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
}

void AudioVolumeManagerSaveSpecifiedDeviceVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    int32_t randIntValue = static_cast<int32_t>(size);
    static uint32_t randomStep = 0;
    randomStep += randIntValue;
    audioAdapterManager_->Init();
    audioAdapterManager_->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    int32_t volumeLevel = randIntValue;
    DeviceType deviceType = g_testDeviceTypes[(index + randomStep) % g_testDeviceTypes.size()];
    audioAdapterManager_->GetMinVolumeLevel(streamType);
    audioAdapterManager_->GetMaxVolumeLevel(streamType);
    audioAdapterManager_->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
}

void AudioVolumeManagerHandleStreamMuteStatusFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    bool mute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    StreamUsage streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, streamUsage, deviceType);
}

void AudioVolumeManagerSetOffloadVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    float volumeDb = static_cast<float>(size);
    audioAdapterManager_->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb);
}

void AudioVolumeManagerSetOffloadSessionIdFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId);
}

} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[] = {
    OHOS::AudioStandard::AudioVolumeManagerIsAppVolumeMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSaveSpecifiedDeviceVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerHandleStreamMuteStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetOffloadVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetOffloadSessionIdFuzzTest,
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