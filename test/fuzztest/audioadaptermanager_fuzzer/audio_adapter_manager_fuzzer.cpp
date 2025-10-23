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
#include "../fuzz_utils.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();

const int32_t NUM_2 = 2;
typedef void (*TestPtr)();

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

void AudioVolumeManagerIsAppVolumeMuteFuzzTest()
{
    int32_t appUid = g_fuzzUtils.GetData<int32_t>();
    bool owned = g_fuzzUtils.GetData<bool>();
    bool isMute = false;
    AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
}

void AudioVolumeManagerSaveSpecifiedDeviceVolumeFuzzTest()
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    int32_t randIntValue = g_fuzzUtils.GetData<int32_t>();
    audioAdapterManager->Init();
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    int32_t volumeLevel = randIntValue;
    DeviceType deviceType = g_testDeviceTypes[(index + g_fuzzUtils.GetData<uint32_t>()) % g_testDeviceTypes.size()];
    audioAdapterManager->GetMinVolumeLevel(streamType);
    audioAdapterManager->GetMaxVolumeLevel(streamType);
    audioAdapterManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
}

void AudioVolumeManagerHandleStreamMuteStatusFuzzTest()
{
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    bool mute = g_fuzzUtils.GetData<bool>();
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, deviceType);
}

void AudioVolumeManagerKvDataFuzzTest()
{
    bool isFirstBoot = g_fuzzUtils.GetData<bool>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitAudioPolicyKvStore(isFirstBoot);
    audioAdapterManager->DeleteAudioPolicyKvStore();
    audioAdapterManager->isNeedCopySystemUrlData_ = g_fuzzUtils.GetData<bool>();
    audioAdapterManager->isNeedCopyVolumeData_ = g_fuzzUtils.GetData<bool>();
    audioAdapterManager->isNeedCopyMuteData_ = g_fuzzUtils.GetData<bool>();
    audioAdapterManager->isNeedCopyRingerModeData_ = g_fuzzUtils.GetData<bool>();
    audioAdapterManager->HandleKvData(isFirstBoot);
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    audioAdapterManager->GetMuteKeyForKvStore(deviceType, streamType);
    audioAdapterManager->GetVolumeKeyForKvStore(deviceType, streamType);
}

void AudioVolumeManagerSaveRingtoneVolumeToLocalFuzzTest()
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    int32_t volumeLevel = g_fuzzUtils.GetData<int32_t>();
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    AudioVolumeType volumeType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    audioAdapterManager->SaveRingtoneVolumeToLocal(volumeType, volumeLevel);
}

void AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest()
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->Init();
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    audioAdapterManager->UpdateSafeVolumeByS4();
}

void AudioVolumeManagerSelectDeviceFuzzTest()
{
    vector<DeviceRole> testDeviceRoles = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    DeviceRole deviceRole = testDeviceRoles[index % testDeviceRoles.size()];
    InternalDeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->Init();
    audioAdapterManager->SelectDevice(deviceRole, deviceType, "test");
}

void AudioVolumeManagerSaveRingerModeInfoFuzzTest()
{
    vector<AudioRingerMode> testAudioRingerModers = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    AudioRingerMode ringMode = testAudioRingerModers[index % testAudioRingerModers.size()];

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->SaveRingerModeInfo(ringMode, "test", "invocationTimeTest");
}

void AudioVolumeManagerOpenNotPaAudioPortFuzzTest()
{
    vector<AudioPipeRole> testAudioPipeRoles = {
        PIPE_ROLE_OUTPUT,
        PIPE_ROLE_INPUT,
        PIPE_ROLE_NONE,
    };
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = testAudioPipeRoles[index % testAudioPipeRoles.size()];
    pipeInfo->routeFlag_ = g_fuzzUtils.GetData<uint32_t>();
    uint32_t paIndex = 0;

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->audioServerProxy_ = AudioServerProxy::GetInstance().GetAudioServerProxy();
    audioAdapterManager->OpenNotPaAudioPort(pipeInfo, paIndex);
}

void AudioVolumeManagerNotifyAccountsChangedFuzzTest()
{
    int id =  g_fuzzUtils.GetData<int>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->NotifyAccountsChanged(id);
}

void AudioVolumeManagerSafeVolumeDumpFuzzTest()
{
    std::string dumpString = "test";
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->isSafeBoot_ = g_fuzzUtils.GetData<bool>();
    audioAdapterManager->SafeVolumeDump(dumpString);
}

void AudioVolumeManagerUpdateVolumeForLowLatencyFuzzTest()
{
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->UpdateVolumeForLowLatency();
}

void AudioVolumeManagerUpdateSinkArgsFuzzTest()
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
    info.needEmptyChunk = g_fuzzUtils.GetData<bool>();
    std::string ret {};
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->UpdateSinkArgs(info, ret);
}

void AudioVolumeManagerUpdateSafeVolumeFuzzTest()
{
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->UpdateSafeVolume();
}

void AudioVolumeManagerInitVolumeMapFuzzTest()
{
    bool isFirstBoot = g_fuzzUtils.GetData<bool>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitVolumeMap(isFirstBoot);
}

void AudioVolumeManagerInitRingerModeFuzzTest()
{
    bool isFirstBoot = g_fuzzUtils.GetData<bool>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->isNeedCopyRingerModeData_ = !isFirstBoot;
    audioAdapterManager->ReInitKVStore();
    audioAdapterManager->InitRingerMode(isFirstBoot);
}

void AudioVolumeManagerInitMuteStatusMapFuzzTest()
{
    bool isFirstBoot = g_fuzzUtils.GetData<bool>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitMuteStatusMap(isFirstBoot);
}

void AudioVolumeManagerCheckAndDealMuteStatusFuzzTest()
{
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    AudioStreamType streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->CheckAndDealMuteStatus(deviceType, streamType);
}

void AudioVolumeManagerOpenPaAudioPortFuzzTest()
{
    vector<AudioPipeRole> testAudioPipeRoles = {
        PIPE_ROLE_OUTPUT,
        PIPE_ROLE_INPUT,
        PIPE_ROLE_NONE,
    };
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    pipeInfo->pipeRole_ = testAudioPipeRoles[index % testAudioPipeRoles.size()];
    uint32_t paIndex = 0;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->OpenPaAudioPort(pipeInfo, paIndex, "test");
}

void AudioVolumeManagerCloneMuteStatusMapFuzzTest()
{
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->CloneMuteStatusMap();
}

void AudioVolumeManagerSafeStatusFuzzTest()
{
    static const vector<SafeStatus> testSafeStatus = {
        SAFE_UNKNOWN,
        SAFE_INACTIVE,
        SAFE_ACTIVE,
    };
    bool isFirstBoot = g_fuzzUtils.GetData<bool>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitSafeStatus(isFirstBoot);
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    SafeStatus status = testSafeStatus[index % testSafeStatus.size()];
    audioAdapterManager->SetDeviceSafeStatus(deviceType, status);
    audioAdapterManager->GetCurrentDeviceSafeStatus(deviceType);
}

void AudioVolumeManagerSafeTimeFuzzTest()
{
    bool isFirstBoot = g_fuzzUtils.GetData<bool>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitSafeTime(isFirstBoot);
    audioAdapterManager->safeActiveTime_ = g_fuzzUtils.GetData<int64_t>();
    audioAdapterManager->safeActiveBtTime_ = g_fuzzUtils.GetData<int64_t>() / NUM_2;
    audioAdapterManager->ConvertSafeTime();
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    int64_t time = g_fuzzUtils.GetData<int64_t>();
    DeviceType deviceType = g_testDeviceTypes[index % g_testDeviceTypes.size()];
    audioAdapterManager->SetDeviceSafeTime(deviceType, time);
    audioAdapterManager->GetCurentDeviceSafeTime(deviceType);
}

void AudioVolumeManagerUpdateVolumeMapIndexFuzzTest()
{
    static const vector<DeviceVolumeType> testDeviceVolumeTypes = {
        EARPIECE_VOLUME_TYPE,
        SPEAKER_VOLUME_TYPE,
        HEADSET_VOLUME_TYPE,
    };
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    VolumePoint volumePoint;
    volumePoint.index = g_fuzzUtils.GetData<uint32_t>();
    volumePoint.dbValue = g_fuzzUtils.GetData<int32_t>() / NUM_2;
    std::vector<VolumePoint> volumePoints;
    volumePoints.push_back(volumePoint);
    std::shared_ptr<DeviceVolumeInfo> deviceVolumeInfoPtr = std::make_shared<DeviceVolumeInfo>();
    CHECK_AND_RETURN(deviceVolumeInfoPtr != nullptr);
    deviceVolumeInfoPtr->deviceType = testDeviceVolumeTypes[index % testDeviceVolumeTypes.size()];
    deviceVolumeInfoPtr->volumePoints = volumePoints;
    DeviceVolumeInfoMap deviceVolumeInfoMap;
    deviceVolumeInfoMap.insert({deviceVolumeInfoPtr->deviceType, deviceVolumeInfoPtr});

    std::shared_ptr<StreamVolumeInfo> streamVolumeInfoPtr = std::make_shared<StreamVolumeInfo>();
    CHECK_AND_RETURN(streamVolumeInfoPtr != nullptr);
    streamVolumeInfoPtr->streamType = g_testAudioStreamTypes[index % g_testAudioStreamTypes.size()];
    streamVolumeInfoPtr->maxLevel = static_cast<int>(g_fuzzUtils.GetData<uint32_t>()) | 1;
    streamVolumeInfoPtr->minLevel = static_cast<int>(g_fuzzUtils.GetData<uint32_t>());
    streamVolumeInfoPtr->defaultLevel = static_cast<int>(g_fuzzUtils.GetData<uint32_t>()) / NUM_2;
    streamVolumeInfoPtr->deviceVolumeInfos = deviceVolumeInfoMap;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->streamVolumeInfos_.insert({streamVolumeInfoPtr->streamType, streamVolumeInfoPtr});
    audioAdapterManager->UpdateVolumeMapIndex();
}

void AudioVolumeManagerHandleRingerModeFuzzTest()
{
    vector<AudioRingerMode> testAudioRingerModers = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    uint32_t index = g_fuzzUtils.GetData<uint32_t>();
    AudioRingerMode ringMode = testAudioRingerModers[index % testAudioRingerModers.size()];
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->HandleRingerMode(ringMode);
}

void AudioVolumeManagerInitializeFuzzTest()
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->Init();
    audioAdapterManager->ConnectServiceAdapter();
    audioAdapterManager->InitKVStore();
    audioAdapterManager->ReInitKVStore();
    audioAdapterManager->DoRestoreData();
    audioAdapterManager->GetSafeVolumeLevel();
    audioAdapterManager->GetSafeVolumeTimeout();
    audioAdapterManager->SetVolumeCallbackAfterClone();
    audioAdapterManager->LoadMuteStatusMap();
}
} // namespace AudioStandard
} // namespace OHOS

std::vector<OHOS::AudioStandard::TestPtr> g_testPtrs = {
    OHOS::AudioStandard::AudioVolumeManagerIsAppVolumeMuteFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSaveSpecifiedDeviceVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerHandleStreamMuteStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerKvDataFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSaveRingtoneVolumeToLocalFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSelectDeviceFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSaveRingerModeInfoFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerOpenNotPaAudioPortFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateVolumeForLowLatencyFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateSinkArgsFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateSafeVolumeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitVolumeMapFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitRingerModeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerInitMuteStatusMapFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerCheckAndDealMuteStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerOpenPaAudioPortFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerCloneMuteStatusMapFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSafeStatusFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSafeTimeFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerUpdateVolumeMapIndexFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerNotifyAccountsChangedFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerSafeVolumeDumpFuzzTest,
    OHOS::AudioStandard::AudioVolumeManagerHandleRingerModeFuzzTest,
};

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::AudioVolumeManagerInitializeFuzzTest();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, g_testPtrs);
    return 0;
}