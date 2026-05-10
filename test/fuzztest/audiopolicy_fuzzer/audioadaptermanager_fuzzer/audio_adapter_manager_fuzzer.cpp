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
#include <atomic>
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
const int32_t NUM_2 = 2;
typedef void (*TestPtr)();

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const std::vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
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

void AudioVolumeManagerSaveSpecifiedDeviceVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    int32_t randIntValue = fdp.ConsumeIntegral<int32_t>();
    audioAdapterManager->Init();
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t volumeLevel = randIntValue;
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    audioAdapterManager->GetMinVolumeLevel(streamType);
    audioAdapterManager->GetMaxVolumeLevel(streamType);
    audioAdapterManager->SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
}

void AudioVolumeManagerIsAppVolumeMuteFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    bool owned = fdp.ConsumeBool();
    bool isMute = false;
    AudioAdapterManager::GetInstance().IsAppVolumeMute(appUid, owned, isMute);
}

void AudioVolumeManagerKvDataFuzzTest(FuzzedDataProvider& fdp)
{
    bool isFirstBoot = fdp.ConsumeBool();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitAudioPolicyKvStore(isFirstBoot);
    audioAdapterManager->DeleteAudioPolicyKvStore();
    audioAdapterManager->isNeedCopySystemUrlData_ = fdp.ConsumeBool();
    audioAdapterManager->isNeedCopyVolumeData_ = fdp.ConsumeBool();
    audioAdapterManager->isNeedCopyMuteData_ = fdp.ConsumeBool();
    audioAdapterManager->isNeedCopyRingerModeData_ = fdp.ConsumeBool();
    audioAdapterManager->HandleKvData(isFirstBoot);
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    audioAdapterManager->GetMuteKeyForKvStore(deviceType, streamType);
    audioAdapterManager->GetVolumeKeyForKvStore(deviceType, streamType);
}

void AudioVolumeManagerHandleStreamMuteStatusFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    bool mute = fdp.ConsumeBool();
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    AudioAdapterManager::GetInstance().HandleStreamMuteStatus(streamType, mute, deviceType);
}

void AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->Init();
    (void)fdp;
    audioAdapterManager->UpdateSafeVolumeByS4();
}

void AudioVolumeManagerSaveNotificationVolumeToLocalFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    AudioVolumeType volumeType = PickValue(fdp, g_testAudioStreamTypes);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    audioAdapterManager->SaveNotificationVolumeToLocal(desc, volumeType, volumeLevel);
}

void AudioVolumeManagerSaveRingerModeInfoFuzzTest(FuzzedDataProvider& fdp)
{
    vector<AudioRingerMode> testAudioRingerModers = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    AudioRingerMode ringMode = PickValue(fdp, testAudioRingerModers);

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->SaveRingerModeInfo(ringMode, "test", "invocationTimeTest");
}

void AudioVolumeManagerSelectDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    vector<DeviceRole> testDeviceRoles = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    DeviceRole deviceRole = PickValue(fdp, testDeviceRoles);
    InternalDeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->Init();
    audioAdapterManager->SelectDevice(deviceRole, deviceType, "test");
}

void AudioVolumeManagerNotifyAccountsChangedFuzzTest(FuzzedDataProvider& fdp)
{
    int id = fdp.ConsumeIntegral<int>();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->NotifyAccountsChanged(id);
}

void AudioVolumeManagerOpenNotPaAudioPortFuzzTest(FuzzedDataProvider& fdp)
{
    vector<AudioPipeRole> testAudioPipeRoles = {
        PIPE_ROLE_OUTPUT,
        PIPE_ROLE_INPUT,
        PIPE_ROLE_NONE,
    };
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->pipeRole_ = PickValue(fdp, testAudioPipeRoles);
    pipeInfo->routeFlag_ = fdp.ConsumeIntegral<uint32_t>();
    uint32_t paIndex = 0;

    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->audioServerProxy_ = AudioServerProxy::GetInstance().GetAudioServerProxy();
    audioAdapterManager->OpenNotPaAudioPort(pipeInfo, paIndex);
}

void AudioVolumeManagerUpdateVolumeForLowLatencyFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    audioAdapterManager->UpdateVolumeForLowLatency(desc, PickValue(fdp, g_testAudioStreamTypes));
}

void AudioVolumeManagerSafeVolumeDumpFuzzTest(FuzzedDataProvider& fdp)
{
    std::string dumpString = "test";
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->isSafeBoot_ = fdp.ConsumeBool();
    audioAdapterManager->SafeVolumeDump(dumpString);
}

void AudioVolumeManagerUpdateSinkArgsFuzzTest(FuzzedDataProvider& fdp)
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
    info.needEmptyChunk = fdp.ConsumeBool();
    std::string ret {};
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->UpdateSinkArgs(info, ret);
}

void AudioVolumeManagerInitVolumeMapFuzzTest(FuzzedDataProvider& fdp)
{
    bool isFirstBoot = fdp.ConsumeBool();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitVolumeMap(isFirstBoot);
}

void AudioVolumeManagerInitRingerModeFuzzTest(FuzzedDataProvider& fdp)
{
    bool isFirstBoot = fdp.ConsumeBool();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->isNeedCopyRingerModeData_ = !isFirstBoot;
    audioAdapterManager->ReInitKVStore();
    audioAdapterManager->InitRingerMode(isFirstBoot);
}

void AudioVolumeManagerUpdateSafeVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->UpdateSafeVolume();
}

void AudioVolumeManagerInitMuteStatusMapFuzzTest(FuzzedDataProvider& fdp)
{
    bool isFirstBoot = fdp.ConsumeBool();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitMuteStatusMap(isFirstBoot);
}

void AudioVolumeManagerCheckAndDealMuteStatusFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->CheckAndDealMuteStatus(deviceType, streamType);
}

void AudioVolumeManagerOpenPaAudioPortFuzzTest(FuzzedDataProvider& fdp)
{
    vector<AudioPipeRole> testAudioPipeRoles = {
        PIPE_ROLE_OUTPUT,
        PIPE_ROLE_INPUT,
        PIPE_ROLE_NONE,
    };
    std::shared_ptr<AudioPipeInfo> pipeInfo = std::make_shared<AudioPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    pipeInfo->pipeRole_ = PickValue(fdp, testAudioPipeRoles);
    uint32_t paIndex = 0;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->OpenPaAudioPort(pipeInfo, paIndex, "test");
}

void AudioVolumeManagerCloneMuteStatusMapFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->CloneMuteStatusMap();
}

void AudioVolumeManagerSafeTimeFuzzTest(FuzzedDataProvider& fdp)
{
    bool isFirstBoot = fdp.ConsumeBool();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitSafeTime(isFirstBoot);
    audioAdapterManager->safeActiveTime_ = fdp.ConsumeIntegral<int64_t>();
    audioAdapterManager->safeActiveBtTime_ = fdp.ConsumeIntegral<int64_t>() / NUM_2;
    audioAdapterManager->ConvertSafeTime();
    int64_t time = fdp.ConsumeIntegral<int64_t>();
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    audioAdapterManager->SetDeviceSafeTime(deviceType, time);
    audioAdapterManager->GetCurentDeviceSafeTime(deviceType);
}

void AudioVolumeManagerSafeStatusFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<SafeStatus> testSafeStatus = {
        SAFE_UNKNOWN,
        SAFE_INACTIVE,
        SAFE_ACTIVE,
    };
    bool isFirstBoot = fdp.ConsumeBool();
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->InitSafeStatus(isFirstBoot);
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    SafeStatus status = PickValue(fdp, testSafeStatus);
    audioAdapterManager->SetDeviceSafeStatus(deviceType, status);
    audioAdapterManager->GetCurrentDeviceSafeStatus(deviceType);
}

void AudioVolumeManagerHandleRingerModeFuzzTest(FuzzedDataProvider& fdp)
{
    vector<AudioRingerMode> testAudioRingerModers = {
        RINGER_MODE_SILENT,
        RINGER_MODE_VIBRATE,
        RINGER_MODE_NORMAL,
    };
    AudioRingerMode ringMode = PickValue(fdp, testAudioRingerModers);
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->HandleRingerMode(ringMode);
}

void AudioVolumeManagerUpdateVolumeMapIndexFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<DeviceVolumeType> testDeviceVolumeTypes = {
        EARPIECE_VOLUME_TYPE,
        SPEAKER_VOLUME_TYPE,
        HEADSET_VOLUME_TYPE,
    };
    VolumePoint volumePoint;
    volumePoint.index = fdp.ConsumeIntegral<uint32_t>();
    volumePoint.dbValue = fdp.ConsumeIntegral<int32_t>() / NUM_2;
    std::vector<VolumePoint> volumePoints;
    volumePoints.push_back(volumePoint);
    std::shared_ptr<DeviceVolumeInfo> deviceVolumeInfoPtr = std::make_shared<DeviceVolumeInfo>();
    CHECK_AND_RETURN(deviceVolumeInfoPtr != nullptr);
    deviceVolumeInfoPtr->deviceType = PickValue(fdp, testDeviceVolumeTypes);
    deviceVolumeInfoPtr->volumePoints = volumePoints;
    DeviceVolumeInfoMap deviceVolumeInfoMap;
    deviceVolumeInfoMap.insert({deviceVolumeInfoPtr->deviceType, deviceVolumeInfoPtr});

    std::shared_ptr<StreamVolumeInfo> streamVolumeInfoPtr = std::make_shared<StreamVolumeInfo>();
    CHECK_AND_RETURN(streamVolumeInfoPtr != nullptr);
    streamVolumeInfoPtr->streamType = PickValue(fdp, g_testAudioStreamTypes);
    streamVolumeInfoPtr->maxLevel = static_cast<int>(fdp.ConsumeIntegral<uint32_t>()) | 1;
    streamVolumeInfoPtr->minLevel = static_cast<int>(fdp.ConsumeIntegral<uint32_t>());
    streamVolumeInfoPtr->defaultLevel = static_cast<int>(fdp.ConsumeIntegral<uint32_t>()) / NUM_2;
    streamVolumeInfoPtr->deviceVolumeInfos = deviceVolumeInfoMap;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->streamVolumeInfos_.insert({streamVolumeInfoPtr->streamType, streamVolumeInfoPtr});
    audioAdapterManager->UpdateVolumeMapIndex();
}

void AudioVolumeManagerInitializeFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
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

void AudioVolumeManagerSetDataShareReadyFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    std::atomic<bool> isDataShareReady = fdp.ConsumeBool();
    audioAdapterManager->SetDataShareReady(isDataShareReady.load());
}

void AudioVolumeManagerSendLoudVolumeModeToDspFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    LoudVolumeHoldType funcHoldType = ConsumeEnum<LoudVolumeHoldType>(fdp);
    bool state = fdp.ConsumeBool();
    audioAdapterManager->SendLoudVolumeModeToDsp(funcHoldType, state);
}

void AudioVolumeManagerSetAppRingMutedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    bool muted = fdp.ConsumeBool();
    audioAdapterManager->SetAppRingMuted(appUid, muted);
}

void AudioVolumeManagerGetZoneVolumeDegreeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    int32_t zoneId = fdp.ConsumeIntegral<int32_t>();
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    audioAdapterManager->GetZoneVolumeDegree(zoneId, streamType);
}

void AudioVolumeManagerSetZoneVolumeDegreeToMapFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    int32_t zoneId = fdp.ConsumeIntegral<int32_t>();
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    int32_t volumeDegree = fdp.ConsumeIntegral<int32_t>();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    audioAdapterManager->SetZoneVolumeLevel(zoneId, streamType, {volumeLevel, volumeDegree}, desc);
}

void AudioVolumeManagerSetDeviceNoMuteForRingerFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    audioAdapterManager->SetDeviceNoMuteForRinger(device);
}

void AudioVolumeManagerUpdateOtherStreamVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->Init();
    AudioStreamType streamType = PickValue(fdp, g_testAudioStreamTypes);
    audioAdapterManager->UpdateOtherStreamVolume(streamType);
}

void AudioVolumeManagerIsChannelLayoutSupportedForDspEffectFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    AudioChannelLayout channelLayout = ConsumeEnum<AudioChannelLayout>(fdp);
    audioAdapterManager->IsChannelLayoutSupportedForDspEffect(channelLayout);
}

void AudioVolumeManagerGetMinVolumeDegreeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    AudioVolumeType volumeType = PickValue(fdp, g_testAudioStreamTypes);
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    audioAdapterManager->GetMinVolumeDegree(volumeType, deviceType);
}

void AudioVolumeManagerHandleCastingConnectionFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->HandleCastingConnection();
}

void AudioVolumeManagerHandleCastingDisconnectionFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->HandleCastingDisconnection();
}

void AudioVolumeManagerIsDPCastingConnectFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->IsDPCastingConnect();
}

void AudioVolumeManagerSetMaxVolumeForDpBoardcastFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    audioAdapterManager->isCastingConnect_ = fdp.ConsumeBool();
    audioAdapterManager->SetMaxVolumeForDpBoardcast();
}

void AudioVolumeManagerUpdateRingerMuteByRingerModeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->deviceType_ = PickValue(fdp, g_testDeviceTypes);
    device->networkId_ = LOCAL_NETWORK_ID;
    audioAdapterManager->ringerMode_ = ConsumeEnum<AudioRingerMode>(fdp);
    audioAdapterManager->UpdateRingerMuteByRingerMode(device);
}

void AudioVolumeManagerSetDualStreamVolumeMuteFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    bool isDualMute = fdp.ConsumeBool();
    audioAdapterManager->SetDualStreamVolumeMute(sessionId, isDualMute);
}

void AudioVolumeManagerSetVolumeFromRemoteFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    std::string networkId = LOCAL_NETWORK_ID;
    int32_t volumeDegree = fdp.ConsumeIntegral<int32_t>();
    audioAdapterManager->SetVolumeFromRemote(networkId, volumeDegree);
}

void AudioVolumeManagerSendVolumeKeyEventCbWithUpdateUiFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    AudioVolumeType volumeType = PickValue(fdp, g_testAudioStreamTypes);
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->networkId_ = LOCAL_NETWORK_ID;
    device->deviceType_ = PickValue(fdp, g_testDeviceTypes);
    audioAdapterManager->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    audioAdapterManager->SendVolumeKeyEventCbWithUpdateUi(volumeType, device);
}

void AudioVolumeManagerRegistAdapterManagerCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    std::string networkId = LOCAL_NETWORK_ID;
    audioAdapterManager->RegistAdapterManagerCallback(networkId);
}

void AudioVolumeManagerOnAudioParameterChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioAdapterManager::RemoteVolumeCallback>();
    CHECK_AND_RETURN(callback != nullptr);
    std::string networkId = LOCAL_NETWORK_ID;
    AudioParamKey key = ConsumeEnum<AudioParamKey>(fdp);
    std::string condition = "testCondition";
    std::string value = "testValue";
    callback->OnAudioParameterChange(networkId, key, condition, value);
}

void AudioVolumeManagerUpdateVolumeWhenPassThroughDeviceConnectFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioAdapterManager = std::make_shared<AudioAdapterManager>();
    CHECK_AND_RETURN(audioAdapterManager != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    device->volumeBehavior_.controlMode = ConsumeEnum<VolumeControlMode>(fdp);
    audioAdapterManager->UpdateVolumeWhenPassThroughDeviceConnect(device);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioVolumeManagerIsAppVolumeMuteFuzzTest,
    AudioVolumeManagerSaveSpecifiedDeviceVolumeFuzzTest,
    AudioVolumeManagerHandleStreamMuteStatusFuzzTest,
    AudioVolumeManagerKvDataFuzzTest,
    AudioVolumeManagerSaveNotificationVolumeToLocalFuzzTest,
    AudioVolumeManagerUpdateSafeVolumeByS4FuzzTest,
    AudioVolumeManagerSelectDeviceFuzzTest,
    AudioVolumeManagerSaveRingerModeInfoFuzzTest,
    AudioVolumeManagerOpenNotPaAudioPortFuzzTest,
    AudioVolumeManagerUpdateVolumeForLowLatencyFuzzTest,
    AudioVolumeManagerUpdateSinkArgsFuzzTest,
    AudioVolumeManagerUpdateSafeVolumeFuzzTest,
    AudioVolumeManagerInitVolumeMapFuzzTest,
    AudioVolumeManagerInitRingerModeFuzzTest,
    AudioVolumeManagerInitMuteStatusMapFuzzTest,
    AudioVolumeManagerCheckAndDealMuteStatusFuzzTest,
    AudioVolumeManagerOpenPaAudioPortFuzzTest,
    AudioVolumeManagerCloneMuteStatusMapFuzzTest,
    AudioVolumeManagerSafeStatusFuzzTest,
    AudioVolumeManagerSafeTimeFuzzTest,
    AudioVolumeManagerUpdateVolumeMapIndexFuzzTest,
    AudioVolumeManagerNotifyAccountsChangedFuzzTest,
    AudioVolumeManagerSafeVolumeDumpFuzzTest,
    AudioVolumeManagerHandleRingerModeFuzzTest,
    AudioVolumeManagerSetDataShareReadyFuzzTest,
    AudioVolumeManagerSendLoudVolumeModeToDspFuzzTest,
    AudioVolumeManagerSetAppRingMutedFuzzTest,
    AudioVolumeManagerGetZoneVolumeDegreeFuzzTest,
    AudioVolumeManagerSetZoneVolumeDegreeToMapFuzzTest,
    AudioVolumeManagerSetDeviceNoMuteForRingerFuzzTest,
    AudioVolumeManagerUpdateOtherStreamVolumeFuzzTest,
    AudioVolumeManagerIsChannelLayoutSupportedForDspEffectFuzzTest,
    AudioVolumeManagerGetMinVolumeDegreeFuzzTest,
    AudioVolumeManagerHandleCastingConnectionFuzzTest,
    AudioVolumeManagerHandleCastingDisconnectionFuzzTest,
    AudioVolumeManagerIsDPCastingConnectFuzzTest,
    AudioVolumeManagerSetMaxVolumeForDpBoardcastFuzzTest,
    AudioVolumeManagerUpdateRingerMuteByRingerModeFuzzTest,
    AudioVolumeManagerSetDualStreamVolumeMuteFuzzTest,
    AudioVolumeManagerSetVolumeFromRemoteFuzzTest,
    AudioVolumeManagerSendVolumeKeyEventCbWithUpdateUiFuzzTest,
    AudioVolumeManagerRegistAdapterManagerCallbackFuzzTest,
    AudioVolumeManagerOnAudioParameterChangeFuzzTest,
    AudioVolumeManagerUpdateVolumeWhenPassThroughDeviceConnectFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
