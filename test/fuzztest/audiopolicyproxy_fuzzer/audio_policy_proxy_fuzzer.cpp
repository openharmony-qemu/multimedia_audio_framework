/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <atomic>
#include <thread>
#include "audio_policy_server.h"
#include "audio_policy_proxy.h"
#include "audio_device_info.h"
#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "audio_routing_manager.h"
#include "audio_stream_manager.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3009;
const float FLOAT_VOLUME = 1.0f;
const bool RUN_ON_CREATE = false;
bool g_hasPermission = false;
bool g_hasServerInit = false;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

AudioPolicyServer* GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        server.OnStart();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return &server;
}

void AudioFuzzTestGetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void AudioPolicyCallbackFuzzTest()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    uint32_t sessionID = GetData<uint32_t>();
    uint32_t clientUid = GetData<uint32_t>();
    int32_t zoneID = GetData<int32_t>();
    audioPolicyProxy->SetAudioInterruptCallback(sessionID, object, clientUid, zoneID);
    audioPolicyProxy->UnsetAudioInterruptCallback(sessionID, zoneID);

    int32_t clientId = GetData<int32_t>();
    audioPolicyProxy->SetAudioManagerInterruptCallback(clientId, object);
    audioPolicyProxy->UnsetAudioManagerInterruptCallback(clientId);
    audioPolicyProxy->SetQueryClientTypeCallback(object);
    audioPolicyProxy->SetAudioClientInfoMgrCallback(object);
    audioPolicyProxy->SetQueryBundleNameListCallback(object);
    audioPolicyProxy->SetAvailableDeviceChangeCallback(clientId, MEDIA_OUTPUT_DEVICES, object);
    audioPolicyProxy->UnsetAvailableDeviceChangeCallback(clientId, MEDIA_OUTPUT_DEVICES);
    audioPolicyProxy->SetAudioConcurrencyCallback(sessionID, object);
    audioPolicyProxy->UnsetAudioConcurrencyCallback(sessionID);
    audioPolicyProxy->SetDistributedRoutingRoleCallback(object);
    audioPolicyProxy->UnsetDistributedRoutingRoleCallback();
    audioPolicyProxy->RegisterPolicyCallbackClient(object, zoneID);
    audioPolicyProxy->SetAudioDeviceRefinerCallback(object);
    audioPolicyProxy->UnsetAudioDeviceRefinerCallback();
    audioPolicyProxy->RegisterSpatializationStateEventListener(sessionID, STREAM_USAGE_MUSIC, object);
    audioPolicyProxy->UnregisterSpatializationStateEventListener(sessionID);
    audioPolicyProxy->SetQueryAllowedPlaybackCallback(object);
    audioPolicyProxy->SetBackgroundMuteCallback(object);
    audioPolicyProxy->SetSleAudioOperationCallback(object);
}

void AudioPolicyMicrophoneFuzzTest()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    audioPolicyProxy->SetMicrophoneMute(true);
    audioPolicyProxy->SetMicrophoneMuteAudioConfig(true);
    audioPolicyProxy->SetMicrophoneMutePersistent(true, PRIVACY_POLCIY_TYPE);
    audioPolicyProxy->IsMicrophoneMuteLegacy();
    audioPolicyProxy->IsMicrophoneMute();

    int32_t sessionId = GetData<int32_t>();
    audioPolicyProxy->GetAudioCapturerMicrophoneDescriptors(sessionId);
    audioPolicyProxy->GetAvailableMicrophones();
}

void AudioPolicyVolumeFuzzTest()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t volumeLevel = GetData<int32_t>();
    int32_t volumeFlag = GetData<int32_t>();
    int32_t appUid = GetData<int32_t>();
    bool owned = GetData<bool>();
    bool isMute = GetData<bool>();
    AudioVolumeType volumeType = GetData<AudioVolumeType>();

    audioPolicyProxy->GetMaxVolumeLevel(volumeType);
    audioPolicyProxy->GetMinVolumeLevel(volumeType);
    audioPolicyProxy->SetSystemVolumeLevelLegacy(volumeType, volumeLevel);
    audioPolicyProxy->SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
    audioPolicyProxy->IsAppVolumeMute(appUid, owned, isMute);
    audioPolicyProxy->SetAppVolumeMuted(appUid, isMute, volumeFlag);
    audioPolicyProxy->SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    audioPolicyProxy->SetSystemVolumeLevel(STREAM_MUSIC, volumeLevel, volumeFlag);

    DeviceType deviceType = GetData<DeviceType>();
    audioPolicyProxy->SetSystemVolumeLevelWithDevice(STREAM_MUSIC, volumeLevel, deviceType, volumeFlag);

    int32_t clientUid = GetData<int32_t>();
    audioPolicyProxy->GetSystemActiveVolumeType(clientUid);

    audioPolicyProxy->GetSystemVolumeLevel(volumeType);
    audioPolicyProxy->GetSelfAppVolumeLevel(volumeLevel);
    audioPolicyProxy->GetAppVolumeLevel(appUid, volumeLevel);
    

    int32_t streamId = GetData<int32_t>();
    float volume = FLOAT_VOLUME;
    audioPolicyProxy->SetLowPowerVolume(streamId, volume);
    audioPolicyProxy->GetLowPowerVolume(streamId);
    audioPolicyProxy->GetSingleStreamVolume(streamId);
    audioPolicyProxy->IsVolumeUnadjustable();

    VolumeAdjustType adjustType = GetData<VolumeAdjustType>()
    audioPolicyProxy->AdjustVolumeByStep(VolumeAdjustType adjustType);
    audioPolicyProxy->AdjustSystemVolumeByStep(volumeType, adjustType)
    audioPolicyProxy->GetSystemVolumeInDb(volumeType, volumeLevel, deviceType)
    audioPolicyProxy->GetMinStreamVolume();
    audioPolicyProxy->GetMaxStreamVolume();

    std::string macAddress = "macAddress";
    audioPolicyProxy->SetDeviceAbsVolumeSupported(macAddress, true);
    audioPolicyProxy->IsAbsVolumeScene();

    int32_t volumeSetA2dpDevice = GetData<int32_t>();
    audioPolicyProxy->SetA2dpDeviceVolume(macAddress, volumeSetA2dpDevice, true);

    bool updateUi = GetData<bool>();
    audioPolicyProxy->SetNearlinkDeviceVolume(macAddress, volumeType, volumeLevel, updateUi);
    audioPolicyProxy->DisableSafeMediaVolume();
}

void AudioPolicyProxyFuzzTest()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    AudioRingerMode ringMode = GetData<AudioRingerMode>();
    audioPolicyProxy->SetRingerModeLegacy(ringMode);
    audioPolicyProxy->SetRingerMode(ringMode);
    audioPolicyProxy->GetPersistentMicMuteState();
    audioPolicyProxy->GetRingerMode();
    AudioScene scene = GetData<AudioScene>();
    audioPolicyProxy->SetAudioScene(scene);
    audioPolicyProxy->GetAudioScene();

    AudioVolumeType volumeType = GetData<AudioVolumeType>();
    bool mute = GetData<bool>();
    DeviceType deviceType = GetData<DeviceType>();
    audioPolicyProxy->SetStreamMuteLegacy(volumeType, mute, deviceType);
    audioPolicyProxy->SetStreamMute(volumeType, mute, deviceType);
    audioPolicyProxy->GetStreamMute(volumeType);
    audioPolicyProxy->IsStreamActive(volumeType);
    DeviceFlag deviceFlag = GetData<DeviceFlag>();
    audioPolicyProxy->GetDevices(deviceFlag);
    audioPolicyProxy->GetDevicesInner(deviceFlag);

    bool active = GetData<bool>();
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    audioPolicyProxy->SetDeviceActive(deviceType, active, uid);
    audioPolicyProxy->IsAllowedPlayback(uid, pid);
    uint32_t sessionID = GetData<uint32_t>();
    SourceType sourceType = GetData<SourceType>();
    bool isRunning = GetData<bool>();
    audioPolicyProxy->SetInputDevice(deviceType, sessionID, sourceType, isRunning);
    audioPolicyProxy->SetVoiceRingtoneMute(mute);
    bool hasSession = GetData<bool>();
    audioPolicyProxy->NotifySessionStateChange(uid, pid, hasSession);
    audioPolicyProxy->ResetAllProxy();
    bool isVirtual = GetData<bool>();
    audioPolicyProxy->SetVirtualCall(isVirtual);
    InternalDeviceType internalDeviceType = GetData<InternalDeviceType>();
    audioPolicyProxy->IsDeviceActive(internalDeviceType);
    audioPolicyProxy->GetActiveOutputDevice();
    audioPolicyProxy->GetDmDeviceType();
    audioPolicyProxy->GetActiveInputDevice();

    AudioStreamType streamType = GetData<AudioStreamType>();
    audioPolicyProxy->GetSelectedDeviceInfo(uid, pid, streamType);
    AudioDeviceUsage audioDevUsage = GetData<AudioDeviceUsage>();
    audioPolicyProxy->GetExcludedDevices(audioDevUsage);
    AudioSessionStrategy strategy = GetData<AudioSessionStrategy>();
    audioPolicyProxy->ActivateAudioSession(strategy);
    audioPolicyProxy->DeactivateAudioSession();
    audioPolicyProxy->IsAudioSessionActivated();

    AudioInterrupt audioInterrupt = GetData<AudioInterrupt>();
    int32_t zoneID = GetData<int32_t>();
    int32_t clientId = GetData<int32_t>();
    bool isUpdatedAudioStrategy = GetData<bool>();
    audioPolicyProxy->ActivateAudioInterrupt(audioInterrupt, zoneID, isUpdatedAudioStrategy);
    audioPolicyProxy->DeactivateAudioInterrupt(audioInterrupt, zoneID);
    audioPolicyProxy->ActivatePreemptMode();
    audioPolicyProxy->DeactivatePreemptMode();
    audioPolicyProxy->RequestAudioFocus(clientId, audioInterrupt);
    audioPolicyProxy->AbandonAudioFocus(clientId, audioInterrupt);
    audioPolicyProxy->GetStreamInFocus(zoneID);
    audioPolicyProxy->GetStreamInFocusByUid(uid, zoneID);
    audioPolicyProxy->GetSessionInfoInFocus(audioInterrupt, zoneID);

    uint32_t count = GetData<uint32_t>();
    StreamSetState streamSetState = GetData<StreamSetState>();
    StreamUsage streamUsage = GetData<StreamUsage>();
    audioPolicyProxy->ReconfigureAudioChannel(count, deviceType);
    audioPolicyProxy->UpdateStreamState(clientId, streamSetState, streamUsage);

    int32_t groupId = GetData<int32_t>();
    std::string networkId = GetData<std::string>();
    std::string key = GetData<std::string>();
    std::string uri = GetData<std::string>();
    audioPolicyProxy->GetNetworkIdByGroupId(groupId, networkId);
    audioPolicyProxy->SetSystemSoundUri(key, uri);
    audioPolicyProxy->GetSystemSoundUri(key);
    audioPolicyProxy->GetMaxRendererInstances();

    SupportedEffectConfig supportedEffectConfig = GetData<SupportedEffectConfig>();
    audioPolicyProxy->QueryEffectSceneMode(supportedEffectConfig);
    audioPolicyProxy->GetAvailableDevices(audioDevUsage);
    audioPolicyProxy->IsSpatializationEnabled();
    std::string address = GetData<std::string>();
    audioPolicyProxy->IsSpatializationEnabled(address);
    audioPolicyProxy->IsSpatializationEnabledForCurrentDevice();
    bool enable = GetData<bool>();
    audioPolicyProxy->SetSpatializationEnabled(enable);
    audioPolicyProxy->IsHeadTrackingEnabled();
    audioPolicyProxy->IsHeadTrackingEnabled(address);
    audioPolicyProxy->SetHeadTrackingEnabled(enable);
    audioPolicyProxy->GetSpatializationState(streamUsage);
    audioPolicyProxy->IsSpatializationSupported();
    audioPolicyProxy->IsSpatializationSupportedForDevice(address);
    audioPolicyProxy->IsHeadTrackingSupported();
    AudioSpatialDeviceState audioSpatialDeviceState = GetData<AudioSpatialDeviceState>();
    audioPolicyProxy->UpdateSpatialDeviceState(audioSpatialDeviceState);
    audioPolicyProxy->ReleaseAudioInterruptZone(zoneID);
    audioPolicyProxy->SetCallDeviceActive(deviceType, active, address, uid);
    audioPolicyProxy->GetActiveBluetoothDevice();
    audioPolicyProxy->GetConverterConfig();

    bool highResExist = GetData<bool>();
    audioPolicyProxy->IsHighResolutionExist();
    audioPolicyProxy->SetHighResolutionExist(highResExist);
    audioPolicyProxy->GetSpatializationSceneType();
    AudioSpatializationSceneType spatializationSceneType = GetData<AudioSpatializationSceneType>();
    audioPolicyProxy->SetSpatializationSceneType(spatializationSceneType);
    int32_t deviceId = GetData<int32_t>();
    audioPolicyProxy->GetMaxAmplitude(deviceId);
    std::string macAddress = GetData<std::string>();
    audioPolicyProxy->IsHeadTrackingDataRequested(macAddress);
    audioPolicyProxy->SaveRemoteInfo(networkId, deviceType);
    audioPolicyProxy->SetAudioDeviceAnahsCallback(object);
    audioPolicyProxy->UnsetAudioDeviceAnahsCallback();
    AudioPipeType pipeType = GetData<AudioPipeType>();
    audioPolicyProxy->MoveToNewPipe(sessionID, pipeType);
    audioPolicyProxy->ActivateAudioConcurrency(pipeType);

    AudioEffectPropertyArrayV3 propertyArray = GetData<AudioEffectPropertyArrayV3>();
    audioPolicyProxy->GetSupportedAudioEffectProperty(propertyArray);
    audioPolicyProxy->GetAudioEffectProperty(propertyArray);
    audioPolicyProxy->SetAudioEffectProperty(propertyArray);
    AudioEnhancePropertyArray audioEnhancePropertyArray = GetData<AudioEnhancePropertyArray>();
    audioPolicyProxy->GetSupportedAudioEnhanceProperty(audioEnhancePropertyArray);
    AudioEffectPropertyArray audioEffectPropertyArray = GetData<AudioEffectPropertyArray>();
    audioPolicyProxy->GetSupportedAudioEffectProperty(audioEffectPropertyArray);
    audioPolicyProxy->GetAudioEnhanceProperty(audioEnhancePropertyArray);
    audioPolicyProxy->GetAudioEffectProperty(audioEffectPropertyArray);
    audioPolicyProxy->SetAudioEnhanceProperty(audioEnhancePropertyArray);
    audioPolicyProxy->SetAudioEffectProperty(audioEffectPropertyArray);

    InterruptEvent event = GetData<InterruptEvent>();
    audioPolicyProxy->InjectInterruption(networkId, event);
    audioPolicyProxy->IsAcousticEchoCancelerSupported(sourceType);
    audioPolicyProxy->GetMaxVolumeLevelByUsage(streamUsage);
    audioPolicyProxy->GetMinVolumeLevelByUsage(streamUsage);
    audioPolicyProxy->GetVolumeLevelByUsage(streamUsage);
    audioPolicyProxy->GetStreamMuteByUsage(streamUsage);
}

typedef void (*TestFuncs[4])();

TestFuncs g_testFuncs = {
    AudioPolicyCallbackFuzzTest,
    AudioPolicyMicrophoneFuzzTest,
    AudioPolicyVolumeFuzzTest,
    AudioPolicyProxyFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}