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
#include "audio_server_proxy.h"
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
    audioAdapterManager_->SetOffloadVolume(streamType, volumeDb, "offload");
}

void AudioVolumeManagerSetOffloadSessionIdFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t sessionId = static_cast<uint32_t>(size);
    AudioAdapterManager::GetInstance().SetOffloadSessionId(sessionId);
}

void AudioVolumeManagerConnectServiceAdapterFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    audioAdapterManager_->ConnectServiceAdapter();
    audioAdapterManager_->InitKVStore();
}

void AudioVolumeManagerHandleKvDataFuzzTest(const uint8_t *rawData, size_t size)
{
    static uint32_t step = 0;
    audioAdapterManager_->Init();
    step += static_cast<uint32_t>(size);
    audioAdapterManager_->isNeedCopySystemUrlData_ = static_cast<bool>(step % NUM_2);
    audioAdapterManager_->isNeedCopyVolumeData_ = static_cast<bool>(step % NUM_2);
    audioAdapterManager_->isNeedCopyMuteData_ = static_cast<bool>(step % NUM_2);
    audioAdapterManager_->isNeedCopyRingerModeData_ = static_cast<bool>(step % NUM_2);
    audioAdapterManager_->isNeedCopySystemUrlData_ = static_cast<bool>(step % NUM_2);
    bool isFirstBoot = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->HandleKvData(isFirstBoot);
}

void AudioVolumeManagerReInitKVStoreFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    audioAdapterManager_->ReInitKVStore();
}

void AudioVolumeManagerSaveRingtoneVolumeToLocalFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t volumeLevel = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    AudioVolumeType volumeType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    audioAdapterManager_->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager_->SaveRingtoneVolumeToLocal(volumeType, volumeLevel);
}

void AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    audioAdapterManager_->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager_->UpdateSafeVolumeByS4();
}

void AudioVolumeManagerSetAppVolumeMutedFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t randIntValue = static_cast<int32_t>(size);
    bool muted = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->SetAppVolumeLevel(randIntValue, randIntValue / NUM_2);
    audioAdapterManager_->SetAppVolumeMuted(randIntValue, muted);
}

void AudioVolumeManagerSetSystemVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<string> testNetworkIds = {
        "LocalDevice",
        "RemoteDevice",
        "NetworkDevice1",
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    deviceDescriptor.networkId_ = testNetworkIds[index % testNetworkIds.size()];
    int32_t testVolumeLevel = static_cast<int32_t>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetActiveDeviceDescriptor(deviceDescriptor);
    audioAdapterManager->SetSystemVolumeLevel(STREAM_MUSIC, testVolumeLevel);
}

void AudioVolumeManagerSetDoubleRingVolumeDbFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    audioAdapterManager_->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    int32_t volumeLevel = static_cast<int32_t>(size);
    audioAdapterManager_->useNonlinearAlgo_ = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->SetDoubleRingVolumeDb(streamType, volumeLevel);
}

void AudioVolumeManagerSetInnerStreamMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    StreamUsage streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    bool mute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->SetInnerStreamMute(streamType, mute, streamUsage);
}

void AudioVolumeManagerSetStreamMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    StreamUsage streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    bool mute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->SetStreamMute(streamType, mute, streamUsage, deviceType);
}

void AudioVolumeManagerSetSinkMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    static uint32_t step = 0;
    step += static_cast<uint32_t>(size);
    vector<string> sinkNames = {
        "Speaker",
        "Offload_Speaker",
        "Bt_Speaker",
        "MCH_Speaker",
        "Usb_arm_speaker",
        "DP_speaker",
        "file_sink",
        "RemoteCastInnerCapturer",
        "test",
    };
    uint32_t index = static_cast<uint32_t>(size);
    string sinkName = sinkNames[index % sinkNames.size()];
    bool isMute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    bool isSync = static_cast<bool>(step % NUM_2);
    audioAdapterManager_->SetSinkMute(sinkName, isMute, isSync);
}

void AudioVolumeManagerSelectDeviceFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    vector<DeviceRole> testDeviceRoles = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    uint32_t index = static_cast<uint32_t>(size);
    DeviceRole deviceRole = testDeviceRoles[index % testDeviceRoles.size()];
    InternalDeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager_->SelectDevice(deviceRole, deviceType, "test");
}

void AudioVolumeManagerSetDeviceActiveFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    vector<DeviceFlag> testDeviceFlags = {
        NONE_DEVICES_FLAG,
        OUTPUT_DEVICES_FLAG,
        INPUT_DEVICES_FLAG,
        ALL_DEVICES_FLAG,
        DISTRIBUTED_OUTPUT_DEVICES_FLAG,
        DISTRIBUTED_INPUT_DEVICES_FLAG,
        ALL_DISTRIBUTED_DEVICES_FLAG,
        ALL_L_D_DEVICES_FLAG,
        DEVICE_FLAG_MAX,
    };
    uint32_t index = static_cast<uint32_t>(size);
    InternalDeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    bool active = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    DeviceFlag flag = testDeviceFlags[index % testDeviceFlags.size()];
    audioAdapterManager_->SetDeviceActive(deviceType, "test", active, flag);
}

void AudioVolumeManagerAdjustBluetoothVoiceAssistantVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    InternalDeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    bool isA2dpSwitchToSco = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->AdjustBluetoothVoiceAssistantVolume(deviceType, isA2dpSwitchToSco);
}

void AudioVolumeManagerSetVolumeForSwitchDeviceFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<string> testNetworkIds = {
        "LocalDevice",
        "RemoteDevice",
        "NetworkDevice1",
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    deviceDescriptor.networkId_ = testNetworkIds[index % testNetworkIds.size()];

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetVolumeForSwitchDevice(deviceDescriptor);
}

void AudioVolumeManagerSaveRingerModeInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<AudioRingerMode> testAudioRingerModers = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioRingerMode ringMode = testAudioRingerModers[index % testAudioRingerModers.size()];

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SaveRingerModeInfo(ringMode, "test", "invocationTimeTest");
}

void AudioVolumeManagerGetSinkIdInfoAndIdTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<string> testAdapterNames = {
        "primary",
        "a2dp",
        "testAdapter",
    };
    uint32_t index = static_cast<uint32_t>(size);
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = testAdapterNames[index % testAdapterNames.size()];
    pipeInfo->routeFlag_ = static_cast<uint32_t>(size);
    std::string idInfo;
    HdiIdType idType;

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetSinkIdInfoAndIdType(pipeInfo, idInfo, idType);
}

void AudioVolumeManagerOpenNotPaAudioPortFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<AudioPipeRole> testAudioPipeRoles = {
        PIPE_ROLE_OUTPUT,
        PIPE_ROLE_INPUT,
        PIPE_ROLE_NONE,
    };
    uint32_t index = static_cast<uint32_t>(size);
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = testAudioPipeRoles[index % testAudioPipeRoles.size()];
    pipeInfo->routeFlag_ = static_cast<uint32_t>(size);
    uint32_t paIndex = 0;

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->audioServerProxy_ = AudioServerProxy::GetInstance().GetAudioServerProxy();
    audioAdapterManager->OpenNotPaAudioPort(pipeInfo, paIndex);
}

void AudioVolumeManagerSetAudioVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    float volumeDb = static_cast<float>(size);
    audioAdapterManager_->SetAudioVolume(streamType, volumeDb);
}

void AudioVolumeManagerNotifyAccountsChangedFuzzTest(const uint8_t *rawData, size_t size)
{
    int id =  static_cast<int>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->NotifyAccountsChanged(id);
}

void AudioVolumeManagerDoRestoreDataFuzzTest(const uint8_t *rawData, size_t size)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->DoRestoreData();
    audioAdapterManager->GetSafeVolumeLevel();
    audioAdapterManager->GetSafeVolumeTimeout();
}

void AudioVolumeManagerSetFirstBootFuzzTest(const uint8_t *rawData, size_t size)
{
    bool isFirst = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetFirstBoot(isFirst);
}

void AudioVolumeManagerSafeVolumeDumpFuzzTest(const uint8_t *rawData, size_t size)
{
    std::string dumpString = "test";
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->isSafeBoot_ = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager->SafeVolumeDump(dumpString);
}

void AudioVolumeManagerSetVgsVolumeSupportedFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    bool isVgsSupported = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager->SetVgsVolumeSupported(isVgsSupported);
    audioAdapterManager->IsVgsVolumeSupported();
}

void AudioVolumeManagerUpdateVolumeForLowLatencyFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager->UpdateVolumeForLowLatency();
}

void AudioVolumeManagerUpdateSinkArgsFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioModuleInfo info;
    info.name = "hello";
    info.adapterName = "world";
    info.className = "CALSS";
    info.fileName = "sink.so";
    info.sinkLatency = "300ms";
    info.networkId = "ASD**G124";
    info.deviceType = "AE00";
    info.extra = "1:13:2";
    info.needEmptyChunk = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    std::string ret {};
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->UpdateSinkArgs(info, ret);
}

void AudioVolumeManagerGetModuleArgsFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<std::string_view> testLib = {
        AudioAdapterManager::HDI_SINK,
        AudioAdapterManager::SPLIT_STREAM_SINK,
        AudioAdapterManager::HDI_SOURCE,
        AudioAdapterManager::PIPE_SINK,
        AudioAdapterManager::PIPE_SOURCE,
        AudioAdapterManager::CLUSTER_SINK,
        AudioAdapterManager::EFFECT_SINK,
        AudioAdapterManager::INNER_CAPTURER_SINK,
        AudioAdapterManager::RECEIVER_SINK,
    };
    AudioModuleInfo info;
    info.name = "hello";
    info.adapterName = "world";
    info.className = "CALSS";
    info.fileName = "sink.so";
    info.sinkLatency = "300ms";
    info.networkId = "ASD**G124";
    info.deviceType = "AE00";
    info.extra = "1:13:2";
    info.lib = testLib[static_cast<uint32_t>(size) % testLib.size()];
    info.needEmptyChunk = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetModuleArgs(info);
}

void AudioVolumeManagerGetHdiSinkIdInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<string> testClassName = {
        "CALSS",
        "remote",
    };
    AudioModuleInfo info;
    info.name = "hello";
    info.adapterName = "world";
    info.className = testClassName[static_cast<uint32_t>(size) % testClassName.size()];
    info.fileName = "sink.so";
    info.sinkLatency = "300ms";
    info.networkId = "ASD**G124";
    info.deviceType = "AE00";
    info.extra = "1:13:2";
    info.needEmptyChunk = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetHdiSinkIdInfo(info);
}

void AudioVolumeManagerGetHdiSourceIdInfoFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<string> testClassName = {
        "primary",
        "CALSS",
        "remote",
    };
    AudioModuleInfo info;
    info.name = "hello";
    info.adapterName = "world";
    info.className = testClassName[static_cast<uint32_t>(size) % testClassName.size()];
    info.fileName = "sink.so";
    info.sinkLatency = "300ms";
    info.networkId = "ASD**G124";
    info.deviceType = "AE00";
    info.extra = "1:13:2";
    info.sourceType = "SOURCE_TYPE_WAKEUP";
    info.needEmptyChunk = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetHdiSourceIdInfo(info);
}

void AudioVolumeManagerGetVolumeKeyForKvStoreFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetVolumeKeyForKvStore(deviceType, streamType);
}

void AudioVolumeManagerGetStreamIDByTypeFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<string> testStreamTypes = {
        "music",
        "ring",
        "voice_call",
        "system",
        "notification",
        "alarm",
        "voice_assistant",
        "accessibility",
        "ultrasonic",
        "camcorder",
    };
    std::string streamType = testStreamTypes[static_cast<uint32_t>(size) % testStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetStreamIDByType(streamType);
}

void AudioVolumeManagerGetDeviceCategoryFuzzTest(const uint8_t *rawData, size_t size)
{
    DeviceType deviceType = g_testDeviceTypes[static_cast<uint32_t>(size) % g_testDeviceTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetDeviceCategory(deviceType);
}

void AudioVolumeManagerDeleteAudioPolicyKvStoreFuzzTest(const uint8_t *rawData, size_t size)
{
    bool isFirstBoot = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->InitAudioPolicyKvStore(isFirstBoot);
    audioAdapterManager->DeleteAudioPolicyKvStore();
}

void AudioVolumeManagerUpdateSafeVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager->UpdateSafeVolume();
}

void AudioVolumeManagerInitVolumeMapFuzzTest(const uint8_t *rawData, size_t size)
{
    bool isFirstBoot = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->InitVolumeMap(isFirstBoot);
}

void AudioVolumeManagerGetDefaultVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    std::unordered_map<AudioStreamType, int32_t> volumeLevelMapTemp;
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    int32_t volumeLevel = static_cast<int32_t>(size);
    volumeLevelMapTemp.insert({streamType, volumeLevel});
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioVolumeType volumeType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetDefaultVolumeLevel(volumeLevelMapTemp, volumeType, deviceType);
}

void AudioVolumeManagerResetRemoteCastDeviceVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->defaultVolumeTypeList_.push_back(streamType);
    audioAdapterManager->ResetRemoteCastDeviceVolume();
}

void AudioVolumeManagerInitRingerModeFuzzTest(const uint8_t *rawData, size_t size)
{
    bool isFirstBoot = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->isNeedCopyRingerModeData_ = !isFirstBoot;
    audioAdapterManager->ReInitKVStore();
    audioAdapterManager->InitRingerMode(isFirstBoot);
}

void AudioVolumeManagerHandleDistributedVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    deviceDescriptor.networkId_ = "testNetworkId";
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->HandleDistributedVolume(streamType);
}

void AudioVolumeManagerInitMuteStatusMapFuzzTest(const uint8_t *rawData, size_t size)
{
    bool isFirstBoot = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->InitMuteStatusMap(isFirstBoot);
}

void AudioVolumeManagerCheckAndDealMuteStatusFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = deviceType;
    audioAdapterManager->CheckAndDealMuteStatus(deviceType, streamType);
}

void AudioVolumeManagerSetVolumeCallbackAfterCloneFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->defaultVolumeTypeList_.push_back(streamType);
    audioAdapterManager->SetVolumeCallbackAfterClone();
}

void AudioVolumeManagerOpenPaAudioPortFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    audioAdapterManager_->ConnectServiceAdapter();
    audioAdapterManager_->audioServerProxy_ = AudioServerProxy::GetInstance().GetAudioServerProxy();
    vector<AudioPipeRole> testAudioPipeRoles = {
        PIPE_ROLE_OUTPUT,
        PIPE_ROLE_INPUT,
        PIPE_ROLE_NONE,
    };
    uint32_t index = static_cast<uint32_t>(size);
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = testAudioPipeRoles[index % testAudioPipeRoles.size()];
    uint32_t paIndex = 0;
    audioAdapterManager_->OpenPaAudioPort(pipeInfo, paIndex, "test");
}

void AudioVolumeManagerCloneMuteStatusMapFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager->CloneMuteStatusMap();
}

void AudioVolumeManagerLoadMuteStatusMapFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->isNeedCopyMuteData_ = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    audioAdapterManager->defaultVolumeTypeList_.push_back(streamType);
    audioAdapterManager->LoadMuteStatusMap();
}

void AudioVolumeManagerInitSafeStatusFuzzTest(const uint8_t *rawData, size_t size)
{
    bool isFirstBoot = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->InitSafeStatus(isFirstBoot);
}

void AudioVolumeManagerInitSafeTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    bool isFirstBoot = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->InitSafeTime(isFirstBoot);
}

void AudioVolumeManagerConvertSafeTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->safeActiveTime_ = static_cast<int64_t>(size);
    audioAdapterManager->safeActiveBtTime_ = static_cast<int64_t>(size / NUM_2);
    audioAdapterManager->ConvertSafeTime();
}

void AudioVolumeManagerGetCurrentDeviceSafeStatusFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetCurrentDeviceSafeStatus(deviceType);
}

void AudioVolumeManagerGetCurentDeviceSafeTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetCurentDeviceSafeTime(deviceType);
}

void AudioVolumeManagerGetRestoreVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetRestoreVolumeLevel(deviceType);
}

void AudioVolumeManagerSetDeviceSafeStatusFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<SafeStatus> testSafeStatus = {
        SAFE_UNKNOWN,
        SAFE_INACTIVE,
        SAFE_ACTIVE,
    };
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    SafeStatus status = testSafeStatus[index % testSafeStatus.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetDeviceSafeStatus(deviceType, status);
}

void AudioVolumeManagerSetDeviceSafeTimeFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    int64_t time = static_cast<int64_t>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetDeviceSafeTime(deviceType, time);
}

void AudioVolumeManagerSetRestoreVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    int32_t volume = static_cast<int32_t>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetRestoreVolumeLevel(deviceType, volume);
}

void AudioVolumeManagerGetMuteKeyForKvStoreFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetMuteKeyForKvStore(deviceType, streamType);
}

void AudioVolumeManagerSetSystemSoundUriFuzzTest(const uint8_t *rawData, size_t size)
{
    static const std::vector<std::string> testSystemSoundKey = {
        "ringtone_for_sim_card_0",
        "ringtone_for_sim_card_1",
        "system_tone_for_sim_card_0",
        "system_tone_for_sim_card_1",
        "system_tone_for_notification"
    };
    std::string key = testSystemSoundKey[static_cast<uint32_t>(size) % testSystemSoundKey.size()];
    std::string uri = "test";
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetSystemSoundUri(key, uri);
}

void AudioVolumeManagerGetSystemSoundUriFuzzTest(const uint8_t *rawData, size_t size)
{
    static const std::vector<std::string> testSystemSoundKey = {
        "ringtone_for_sim_card_0",
        "ringtone_for_sim_card_1",
        "system_tone_for_sim_card_0",
        "system_tone_for_sim_card_1",
        "system_tone_for_notification"
    };
    std::string key = testSystemSoundKey[static_cast<uint32_t>(size) % testSystemSoundKey.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetSystemSoundUri(key);
}

void AudioVolumeManagerGetSystemVolumeInDbFuzzTest(const uint8_t *rawData, size_t size)
{
    int32_t volumeLevel = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    AudioVolumeType volumeType =  g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->useNonlinearAlgo_ = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager->GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
}

void AudioVolumeManagerGetPositionInVolumePointsFuzzTest(const uint8_t *rawData, size_t size)
{
    static uint32_t step = 0;
    step += static_cast<uint32_t>(size);
    VolumePoint volumePoint1;
    volumePoint1.index = static_cast<uint32_t>(size);
    volumePoint1.dbValue = static_cast<int32_t>(size) / NUM_2;
    VolumePoint volumePoint2;
    volumePoint2.index = step;
    volumePoint2.dbValue = step / NUM_2;
    std::vector<VolumePoint> volumePoints;
    volumePoints.push_back(volumePoint1);
    volumePoints.push_back(volumePoint2);
    int32_t idx = static_cast<int32_t>(size);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->GetPositionInVolumePoints(volumePoints, idx);
}

void AudioVolumeManagerInitVolumeMapIndexFuzzTest(const uint8_t *rawData, size_t size)
{
    AudioStreamType streamType = g_testAudioStreamTypes[static_cast<uint32_t>(size) % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->defaultVolumeTypeList_.push_back(streamType);
    audioAdapterManager->InitVolumeMapIndex();
}

void AudioVolumeManagerUpdateVolumeMapIndexFuzzTest(const uint8_t *rawData, size_t size)
{
    static const vector<DeviceVolumeType> testDeviceVolumeTypes = {
        EARPIECE_VOLUME_TYPE,
        SPEAKER_VOLUME_TYPE,
        HEADSET_VOLUME_TYPE,
    };
    uint32_t index = static_cast<uint32_t>(size);
    VolumePoint volumePoint;
    volumePoint.index = static_cast<uint32_t>(size);
    volumePoint.dbValue = static_cast<int32_t>(size) / NUM_2;
    std::vector<VolumePoint> volumePoints;
    volumePoints.push_back(volumePoint);
    std::shared_ptr<DeviceVolumeInfo> deviceVolumeInfoPtr = std::make_shared<DeviceVolumeInfo>();
    deviceVolumeInfoPtr->deviceType = testDeviceVolumeTypes[index % testDeviceVolumeTypes.size()];
    deviceVolumeInfoPtr->volumePoints = volumePoints;
    DeviceVolumeInfoMap deviceVolumeInfoMap;
    deviceVolumeInfoMap.insert({deviceVolumeInfoPtr->deviceType, deviceVolumeInfoPtr});

    std::shared_ptr<StreamVolumeInfo> streamVolumeInfoPtr = std::make_shared<StreamVolumeInfo>();
    streamVolumeInfoPtr->streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    streamVolumeInfoPtr->maxLevel = static_cast<int>(size) | 1;
    streamVolumeInfoPtr->minLevel = static_cast<int>(size);
    streamVolumeInfoPtr->defaultLevel = static_cast<int>(size) / NUM_2;
    streamVolumeInfoPtr->deviceVolumeInfos = deviceVolumeInfoMap;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->streamVolumeInfos_.insert({streamVolumeInfoPtr->streamType, streamVolumeInfoPtr});
    audioAdapterManager->UpdateVolumeMapIndex();
}

void AudioVolumeManagerSetAbsVolumeSceneFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    bool isAbsVolumeScene = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager->SetAbsVolumeScene(isAbsVolumeScene);
}

void AudioVolumeManagerSetAbsVolumeMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    uint32_t index = static_cast<uint32_t>(size);
    bool mute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager->SetAbsVolumeMute(mute);
}

void AudioVolumeManagerSetAppVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    audioAdapterManager_->currentActiveDevice_.deviceType_ = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    int32_t randIntValue = static_cast<int32_t>(size);
    audioAdapterManager_->SetAppVolumeLevel(randIntValue, randIntValue / NUM_2);
}
 
void AudioVolumeManagerSetAdjustVolumeForZoneFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t zoneId = static_cast<int32_t>(size);
    audioAdapterManager_->SetAdjustVolumeForZone(zoneId);
}
 
void AudioVolumeManagerSetZoneMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t zoneId = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    StreamUsage streamUsage = g_testStreamUsages[index % g_testStreamUsages.size()];
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    bool mute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->SetZoneMute(zoneId, streamType, mute, streamUsage, deviceType);
}
 
void AudioVolumeManagerGetZoneMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t zoneId = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    audioAdapterManager_->GetZoneMute(zoneId, streamType);
}
 
void AudioVolumeManagerGetZoneVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t zoneId = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    audioAdapterManager_->GetZoneVolumeLevel(zoneId, streamType);
}
 
void AudioVolumeManagerSetZoneVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t zoneId = static_cast<int32_t>(size);
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    int32_t volumeLevel = static_cast<int32_t>(size);
    audioAdapterManager_->SetZoneVolumeLevel(zoneId, streamType, volumeLevel);
}
 
void AudioVolumeManagerGetDeviceVolumeFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager_->GetDeviceVolume(deviceType, streamType);
}
 
void AudioVolumeManagerHandleRingerModeFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<AudioRingerMode> testAudioRingerModers = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioRingerMode ringMode = testAudioRingerModers[index % testAudioRingerModers.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->HandleRingerMode(ringMode);
}
 
void AudioVolumeManagerSetAppVolumeDbFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t appUid = static_cast<int32_t>(size);
    audioAdapterManager_->SetAppVolumeDb(appUid);
}
 
void AudioVolumeManagerSetAppVolumeMutedDBFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t appUid = static_cast<int32_t>(size);
    bool muted = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->SetAppVolumeMutedDB(appUid, muted);
}
 
void AudioVolumeManagerGetAppVolumeLevelFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t appUid = static_cast<int32_t>(size);
    int32_t volumeLevel = static_cast<int32_t>(size);
    audioAdapterManager_->GetAppVolumeLevel(appUid, volumeLevel);
}
 
void AudioVolumeManagerSetPersistMicMuteStateFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    bool mute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->SetPersistMicMuteState(mute);
}
 
void AudioVolumeManagerSetSourceOutputStreamMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t uid = static_cast<int32_t>(size);
    bool setMute = static_cast<bool>(static_cast<uint32_t>(size) % NUM_2);
    audioAdapterManager_->SetSourceOutputStreamMute(uid, setMute);
}
 
void AudioVolumeManagerGetStreamMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    uint32_t index = static_cast<uint32_t>(size);
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    audioAdapterManager_->GetStreamMute(streamType);
}
 
void AudioVolumeManagerGetAppMuteFuzzTest(const uint8_t *rawData, size_t size)
{
    audioAdapterManager_->Init();
    int32_t appUid = static_cast<int32_t>(size);
    audioAdapterManager_->GetAppMute(appUid);
}
 
void AudioVolumeManagerSetRingerModeFuzzTest(const uint8_t *rawData, size_t size)
{
    vector<AudioRingerMode> testAudioRingerModers = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    uint32_t index = static_cast<uint32_t>(size);
    AudioRingerMode ringMode = testAudioRingerModers[index % testAudioRingerModers.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    audioAdapterManager->SetRingerMode(ringMode);
}
} // namespace AudioStandard
} // namesapce OHOS

OHOS::AudioStandard::TestPtr g_testPtrs[] = {
    OHOS::AudioStandard::AudioVolumeManagerIsAppVolumeMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSaveSpecifiedDeviceVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerHandleStreamMuteStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetOffloadVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetOffloadSessionIdFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerConnectServiceAdapterFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerHandleKvDataFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerReInitKVStoreFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSaveRingtoneVolumeToLocalFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAppVolumeMutedFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetSystemVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetDoubleRingVolumeDbFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetInnerStreamMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetStreamMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetSinkMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSelectDeviceFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetDeviceActiveFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerAdjustBluetoothVoiceAssistantVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSaveRingerModeInfoFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetSinkIdInfoAndIdTypeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerOpenNotPaAudioPortFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateSinkArgsFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetModuleArgsFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetHdiSinkIdInfoFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetHdiSourceIdInfoFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetVolumeKeyForKvStoreFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetStreamIDByTypeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetDeviceCategoryFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerDeleteAudioPolicyKvStoreFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateSafeVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitVolumeMapFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetDefaultVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerResetRemoteCastDeviceVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitRingerModeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerHandleDistributedVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitMuteStatusMapFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerCheckAndDealMuteStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetVolumeCallbackAfterCloneFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerOpenPaAudioPortFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerCloneMuteStatusMapFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerLoadMuteStatusMapFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitSafeStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitSafeTimeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerConvertSafeTimeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetCurrentDeviceSafeStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetCurentDeviceSafeTimeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetRestoreVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetDeviceSafeStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetDeviceSafeTimeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetRestoreVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetMuteKeyForKvStoreFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetSystemSoundUriFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetSystemSoundUriFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetSystemVolumeInDbFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetPositionInVolumePointsFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitVolumeMapIndexFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateVolumeMapIndexFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAbsVolumeSceneFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAbsVolumeMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAudioVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerNotifyAccountsChangedFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerDoRestoreDataFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetFirstBootFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSafeVolumeDumpFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetVgsVolumeSupportedFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAppVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAdjustVolumeForZoneFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetZoneMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetZoneMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetZoneMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetZoneVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetZoneVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetDeviceVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerHandleRingerModeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAppVolumeDbFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetAppVolumeMutedDBFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetAppVolumeLevelFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetPersistMicMuteStateFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetSourceOutputStreamMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetStreamMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerGetAppMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSetRingerModeFuzzTest,
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