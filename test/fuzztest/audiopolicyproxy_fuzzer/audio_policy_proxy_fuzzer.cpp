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
    GetServerPtr();
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
    GetServerPtr();
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
    GetServerPtr();
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

    audioPolicyProxy->GetMaxVolumeLevel(STREAM_MUSIC);
    audioPolicyProxy->GetMinVolumeLevel(STREAM_MUSIC);
    audioPolicyProxy->SetSystemVolumeLevelLegacy(STREAM_MUSIC, volumeLevel);
    audioPolicyProxy->SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
    audioPolicyProxy->IsAppVolumeMute(appUid, owned, isMute);
    audioPolicyProxy->SetAppVolumeMuted(appUid, isMute, volumeFlag);
    audioPolicyProxy->SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    audioPolicyProxy->SetSystemVolumeLevel(STREAM_MUSIC, volumeLevel, volumeFlag);

    audioPolicyProxy->SetSystemVolumeLevelWithDevice(STREAM_MUSIC, volumeLevel, DEVICE_TYPE_SPEAKER, volumeFlag);

    int32_t clientUid = GetData<int32_t>();
    audioPolicyProxy->GetSystemActiveVolumeType(clientUid);

    audioPolicyProxy->GetSystemVolumeLevel(STREAM_MUSIC);
    audioPolicyProxy->GetSelfAppVolumeLevel(volumeLevel);
    audioPolicyProxy->GetAppVolumeLevel(appUid, volumeLevel);

    int32_t streamId = GetData<int32_t>();
    float volume = FLOAT_VOLUME;
    audioPolicyProxy->SetLowPowerVolume(streamId, volume);
    audioPolicyProxy->GetLowPowerVolume(streamId);
    audioPolicyProxy->GetSingleStreamVolume(streamId);
    audioPolicyProxy->IsVolumeUnadjustable();

    audioPolicyProxy->AdjustVolumeByStep(VOLUME_UP);
    audioPolicyProxy->AdjustSystemVolumeByStep(STREAM_MUSIC, VOLUME_UP);
    audioPolicyProxy->GetSystemVolumeInDb(STREAM_MUSIC, volumeLevel, DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->GetMinStreamVolume();
    audioPolicyProxy->GetMaxStreamVolume();

    std::string macAddress = "macAddress";
    audioPolicyProxy->SetDeviceAbsVolumeSupported(macAddress, true);
    audioPolicyProxy->IsAbsVolumeScene();

    int32_t volumeSetA2dpDevice = GetData<int32_t>();
    audioPolicyProxy->SetA2dpDeviceVolume(macAddress, volumeSetA2dpDevice, true);

    bool updateUi = GetData<bool>();
    audioPolicyProxy->SetNearlinkDeviceVolume(macAddress, STREAM_MUSIC, volumeLevel, updateUi);
    audioPolicyProxy->DisableSafeMediaVolume();
}

void AudioPolicyProxyOneFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    audioPolicyProxy->SetRingerModeLegacy(RINGER_MODE_NORMAL);
    audioPolicyProxy->SetRingerMode(RINGER_MODE_NORMAL);
    audioPolicyProxy->GetPersistentMicMuteState();
    audioPolicyProxy->GetRingerMode();
    audioPolicyProxy->SetAudioScene(AUDIO_SCENE_DEFAULT);
    audioPolicyProxy->GetAudioScene();

    bool mute = GetData<bool>();
    audioPolicyProxy->SetStreamMuteLegacy(STREAM_MUSIC, mute, DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->SetStreamMute(STREAM_MUSIC, mute, DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->GetStreamMute(STREAM_MUSIC);
    audioPolicyProxy->IsStreamActive(STREAM_MUSIC);
    audioPolicyProxy->GetDevices(OUTPUT_DEVICES_FLAG);
    audioPolicyProxy->GetDevicesInner(OUTPUT_DEVICES_FLAG);

    bool active = GetData<bool>();
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    audioPolicyProxy->SetDeviceActive(DEVICE_TYPE_SPEAKER, active, uid);
    audioPolicyProxy->IsAllowedPlayback(uid, pid);
    uint32_t sessionID = GetData<uint32_t>();
    bool isRunning = GetData<bool>();
    audioPolicyProxy->SetInputDevice(DEVICE_TYPE_SPEAKER, sessionID, SOURCE_TYPE_MIC, isRunning);
    audioPolicyProxy->SetVoiceRingtoneMute(mute);
    bool hasSession = GetData<bool>();
    audioPolicyProxy->NotifySessionStateChange(uid, pid, hasSession);
    audioPolicyProxy->ResetAllProxy();
    bool isVirtual = GetData<bool>();
    audioPolicyProxy->SetVirtualCall(isVirtual);
    audioPolicyProxy->IsDeviceActive(DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->GetActiveOutputDevice();
    audioPolicyProxy->GetDmDeviceType();
    audioPolicyProxy->GetActiveInputDevice();
}

void AudioPolicyProxyTwoFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    audioPolicyProxy->GetSelectedDeviceInfo(uid, pid, STREAM_MUSIC);
    audioPolicyProxy->GetExcludedDevices(MEDIA_OUTPUT_DEVICES);
    AudioSessionStrategy strategy = {AudioConcurrencyMode::DEFAULT};
    audioPolicyProxy->ActivateAudioSession(strategy);
    audioPolicyProxy->DeactivateAudioSession();
    audioPolicyProxy->IsAudioSessionActivated();

    AudioInterrupt audioInterrupt;
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
    audioPolicyProxy->ReconfigureAudioChannel(count, DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->UpdateStreamState(clientId, STREAM_PAUSE, STREAM_USAGE_MUSIC);

    int32_t groupId = GetData<int32_t>();
    std::string networkId = "networkId";
    std::string key = "key";
    std::string uri = "uri";
    audioPolicyProxy->GetNetworkIdByGroupId(groupId, networkId);
    audioPolicyProxy->SetSystemSoundUri(key, uri);
    audioPolicyProxy->GetSystemSoundUri(key);
    audioPolicyProxy->GetMaxRendererInstances();
}

void AudioPolicyProxyThreeFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    SupportedEffectConfig supportedEffectConfig;
    audioPolicyProxy->QueryEffectSceneMode(supportedEffectConfig);
    audioPolicyProxy->GetAvailableDevices(MEDIA_OUTPUT_DEVICES);
    audioPolicyProxy->IsSpatializationEnabled();
    std::string address = "address";
    audioPolicyProxy->IsSpatializationEnabled(address);
    audioPolicyProxy->IsSpatializationEnabledForCurrentDevice();
    bool enable = GetData<bool>();
    audioPolicyProxy->SetSpatializationEnabled(enable);
    audioPolicyProxy->IsHeadTrackingEnabled();
    audioPolicyProxy->IsHeadTrackingEnabled(address);
    audioPolicyProxy->SetHeadTrackingEnabled(enable);
    audioPolicyProxy->GetSpatializationState(STREAM_USAGE_MUSIC);
    audioPolicyProxy->IsSpatializationSupported();
    audioPolicyProxy->IsSpatializationSupportedForDevice(address);
    audioPolicyProxy->IsHeadTrackingSupported();
    AudioSpatialDeviceState audioSpatialDeviceState = {
        "1234",
        true,
        true,
        AudioSpatialDeviceType::EARPHONE_TYPE_NONE,
    };
    audioPolicyProxy->UpdateSpatialDeviceState(audioSpatialDeviceState);
    int32_t zoneID = GetData<int32_t>();
    audioPolicyProxy->ReleaseAudioInterruptZone(zoneID);
    bool active = GetData<bool>();
    int32_t uid = GetData<int32_t>();
    audioPolicyProxy->SetCallDeviceActive(DEVICE_TYPE_SPEAKER, active, address, uid);
    audioPolicyProxy->GetActiveBluetoothDevice();
    audioPolicyProxy->GetConverterConfig();
}

void AudioPolicyProxyFourFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    bool highResExist = GetData<bool>();
    audioPolicyProxy->IsHighResolutionExist();
    audioPolicyProxy->SetHighResolutionExist(highResExist);
    audioPolicyProxy->GetSpatializationSceneType();
    AudioSpatializationSceneType spatializationSceneType =
        AudioSpatializationSceneType::SPATIALIZATION_SCENE_TYPE_DEFAULT;
    audioPolicyProxy->SetSpatializationSceneType(spatializationSceneType);
    int32_t deviceId = GetData<int32_t>();
    audioPolicyProxy->GetMaxAmplitude(deviceId);
    std::string macAddress = "macAddress";
    audioPolicyProxy->IsHeadTrackingDataRequested(macAddress);
    std::string networkId = "networkId";
    audioPolicyProxy->SaveRemoteInfo(networkId, DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->SetAudioDeviceAnahsCallback(object);
    audioPolicyProxy->UnsetAudioDeviceAnahsCallback();
    uint32_t sessionID = GetData<uint32_t>();
    AudioPipeType pipeType = PIPE_TYPE_OFFLOAD;
    audioPolicyProxy->MoveToNewPipe(sessionID, pipeType);
    audioPolicyProxy->ActivateAudioConcurrency(pipeType);

    AudioEffectPropertyArrayV3 propertyArray;
    audioPolicyProxy->GetSupportedAudioEffectProperty(propertyArray);
    audioPolicyProxy->GetAudioEffectProperty(propertyArray);
    audioPolicyProxy->SetAudioEffectProperty(propertyArray);
    AudioEnhancePropertyArray audioEnhancePropertyArray;
    audioPolicyProxy->GetSupportedAudioEnhanceProperty(audioEnhancePropertyArray);
    AudioEffectPropertyArray audioEffectPropertyArray;
    audioPolicyProxy->GetSupportedAudioEffectProperty(audioEffectPropertyArray);
    audioPolicyProxy->GetAudioEnhanceProperty(audioEnhancePropertyArray);
    audioPolicyProxy->GetAudioEffectProperty(audioEffectPropertyArray);
    audioPolicyProxy->SetAudioEnhanceProperty(audioEnhancePropertyArray);
    audioPolicyProxy->SetAudioEffectProperty(audioEffectPropertyArray);

    InterruptEvent event;
    audioPolicyProxy->InjectInterruption(networkId, event);
    audioPolicyProxy->IsAcousticEchoCancelerSupported(SOURCE_TYPE_MIC);
    audioPolicyProxy->GetMaxVolumeLevelByUsage(STREAM_USAGE_MUSIC);
    audioPolicyProxy->GetMinVolumeLevelByUsage(STREAM_USAGE_MUSIC);
    audioPolicyProxy->GetVolumeLevelByUsage(STREAM_USAGE_MUSIC);
    audioPolicyProxy->GetStreamMuteByUsage(STREAM_USAGE_MUSIC);
}

void AudioPolicyProxyFiveFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    audioPolicyProxy->GetCurrentRendererChangeInfos(audioRendererChangeInfos);

    AudioStreamChangeInfo streamChangeInfo;
    AudioMode mode = AUDIO_MODE_PLAYBACK;
    audioPolicyProxy->RegisterTracker(mode, streamChangeInfo, object);
    audioPolicyProxy->UpdateTracker(mode, streamChangeInfo);

    AudioInterrupt audioInterrupt;
    int32_t zoneID = GetData<int32_t>();
    audioPolicyProxy->DeactivateAudioInterrupt(audioInterrupt, zoneID);

    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;
    audioDeviceDescriptors.push_back(fuzzAudioDeviceDescriptorSptr);
    audioPolicyProxy->SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);

    bool forceNoBTPermission = GetData<bool>();
    AudioRendererInfo rendererInfo;
    rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    AudioCapturerInfo captureInfo;
    captureInfo.sourceType = SOURCE_TYPE_MIC;
    audioPolicyProxy->GetPreferredOutputDeviceDescriptors(rendererInfo, forceNoBTPermission);
    audioPolicyProxy->GetPreferredInputDeviceDescriptors(captureInfo);
    
    int32_t ltonetype = GetData<int32_t>();
    std::string countryCode = "countryCode";
    audioPolicyProxy->GetToneConfig(ltonetype, countryCode);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    audioPolicyProxy->GetAudioFocusInfoList(focusInfoList, zoneID);

    sptr <AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    audioPolicyProxy->SelectOutputDevice(audioRendererFilter, audioDeviceDescriptors);
    audioPolicyProxy->GetSupportedTones(countryCode);

    std::set<StreamUsage> streamUsages;
    audioPolicyProxy->SetCallbackStreamUsageInfo(streamUsages);
    audioPolicyProxy->ForceStopAudioStream(STOP_RENDER);
    AudioCapturerInfo capturerInfo;
    audioPolicyProxy->IsCapturerFocusAvailable(capturerInfo);

    AudioStreamInfo streamInfo;
    audioPolicyProxy->IsFastPlaybackSupported(streamInfo, STREAM_USAGE_MUSIC);
    audioPolicyProxy->IsFastRecordingSupported(streamInfo, SOURCE_TYPE_MIC);
}

void AudioPolicyZoneFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    std::string name = "name";
    AudioZoneContext context;
    int32_t zoneId = GetData<int32_t>();
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(fuzzAudioDeviceDescriptorSptr);
    bool enable = GetData<bool>();
    int32_t uid = GetData<int32_t>();
    std::string deviceTag = "deviceTag";
    std::list<std::pair<AudioInterrupt, AudioFocuState>> interrupts;

    audioPolicyProxy->RegisterAudioZoneClient(object);
    audioPolicyProxy->CreateAudioZone(name, context);
    audioPolicyProxy->ReleaseAudioZone(zoneId);
    audioPolicyProxy->GetAllAudioZone();
    audioPolicyProxy->GetAudioZone(zoneId);
    audioPolicyProxy->BindDeviceToAudioZone(zoneId, devices);
    audioPolicyProxy->UnBindDeviceToAudioZone(zoneId, devices);
    audioPolicyProxy->EnableAudioZoneReport(enable);
    audioPolicyProxy->EnableAudioZoneChangeReport(zoneId, enable);
    audioPolicyProxy->AddUidToAudioZone(zoneId, uid);
    audioPolicyProxy->RemoveUidFromAudioZone(zoneId, uid);
    audioPolicyProxy->EnableSystemVolumeProxy(zoneId, enable);
    audioPolicyProxy->GetAudioInterruptForZone(zoneId);
    audioPolicyProxy->GetAudioInterruptForZone(zoneId, deviceTag);
    audioPolicyProxy->EnableAudioZoneInterruptReport(zoneId, deviceTag, enable);
    audioPolicyProxy->InjectInterruptToAudioZone(zoneId, interrupts);
    audioPolicyProxy->InjectInterruptToAudioZone(zoneId, interrupts);
}

typedef void (*TestFuncs[9])();

TestFuncs g_testFuncs = {
    AudioPolicyCallbackFuzzTest,
    AudioPolicyMicrophoneFuzzTest,
    AudioPolicyVolumeFuzzTest,
    AudioPolicyProxyOneFuzzTest,
    AudioPolicyProxyTwoFuzzTest,
    AudioPolicyProxyThreeFuzzTest,
    AudioPolicyProxyFourFuzzTest,
    AudioPolicyProxyFiveFuzzTest,
    AudioPolicyZoneFuzzTest,
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