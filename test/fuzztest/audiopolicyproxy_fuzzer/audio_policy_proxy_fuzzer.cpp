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
const std::u16string FORMMGR_INTERFACE_TOKEN = u"OHOS.AudioStandard.IAudioPolicy";
const int32_t SYSTEM_ABILITY_ID = 3009;
const float FLOAT_VOLUME = 1.0f;
const bool RUN_ON_CREATE = false;
bool g_hasPermission = false;
bool g_hasServerInit = false;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

sptr<AudioPolicyServer> GetServerPtr()
{
    static sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit && server != nullptr) {
        server->OnStart();
        server->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server->OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server->OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server->OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server->audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return server;
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

    bool resIsMute = GetData<bool>();
    audioPolicyProxy->IsMicrophoneMuteLegacy(resIsMute);
    audioPolicyProxy->IsMicrophoneMute(resIsMute);
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

    int32_t volumeType = GetData<int32_t>();
    int32_t volumeLevel = GetData<int32_t>();
    int32_t volumeFlag = GetData<int32_t>();
    int32_t appUid = GetData<int32_t>();
    bool owned = GetData<bool>();
    bool isMute = GetData<bool>();

    audioPolicyProxy->SetSelfAppVolumeLevel(volumeLevel, volumeFlag);
    audioPolicyProxy->IsAppVolumeMute(appUid, owned, isMute);
    audioPolicyProxy->SetAppVolumeMuted(appUid, isMute, volumeFlag);
    audioPolicyProxy->SetAppVolumeLevel(appUid, volumeLevel, volumeFlag);
    audioPolicyProxy->SetSystemVolumeLevel(volumeType, volumeLevel, volumeFlag, appUid);

    audioPolicyProxy->SetSystemVolumeLevelWithDevice(STREAM_MUSIC, volumeLevel, DEVICE_TYPE_SPEAKER, volumeFlag);

    int32_t clientUid = GetData<int32_t>();
    int32_t audioStreamType = GetData<int32_t>();
    audioPolicyProxy->GetSystemActiveVolumeType(clientUid, audioStreamType);

    audioPolicyProxy->GetSystemVolumeLevel(volumeType, appUid, volumeLevel);
    audioPolicyProxy->GetSelfAppVolumeLevel(volumeLevel);
    audioPolicyProxy->GetAppVolumeLevel(appUid, volumeLevel);

    int32_t streamId = GetData<int32_t>();
    float volume = FLOAT_VOLUME;
    float retVolume = GetData<float>();
    bool retIsUnadjustable = GetData<bool>();
    audioPolicyProxy->SetLowPowerVolume(streamId, volume);
    audioPolicyProxy->GetLowPowerVolume(streamId, retVolume);
    audioPolicyProxy->GetSingleStreamVolume(streamId, retVolume);
    audioPolicyProxy->IsVolumeUnadjustable(retIsUnadjustable);

    audioPolicyProxy->AdjustVolumeByStep(VOLUME_UP);
    audioPolicyProxy->AdjustSystemVolumeByStep(STREAM_MUSIC, VOLUME_UP);
    audioPolicyProxy->GetSystemVolumeInDb(STREAM_MUSIC, volumeLevel, DEVICE_TYPE_SPEAKER, retVolume);
    audioPolicyProxy->GetMinStreamVolume(retVolume);
    audioPolicyProxy->GetMaxStreamVolume(retVolume);

    std::string macAddress = "macAddress";
    audioPolicyProxy->SetDeviceAbsVolumeSupported(macAddress, true);
    audioPolicyProxy->IsAbsVolumeScene(retIsUnadjustable);

    int32_t volumeSetA2dpDevice = GetData<int32_t>();
    audioPolicyProxy->SetA2dpDeviceVolume(macAddress, volumeSetA2dpDevice, true);

    bool updateUi = GetData<bool>();
    audioPolicyProxy->SetNearlinkDeviceVolume(macAddress, STREAM_MUSIC, volumeLevel, updateUi);
    audioPolicyProxy->DisableSageMediaVolume();
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

    bool retMuteState = GetData<bool>();
    int32_t retRingMode = GetData<int32_t>();
    int32_t retScene = GetData<int32_t>();
    audioPolicyProxy->SetRingerModeLegacy(RINGER_MODE_NORMAL);
    audioPolicyProxy->SetRingerMode(RINGER_MODE_NORMAL);
    audioPolicyProxy->GetPersistentMicMuteState(retMuteState);
    audioPolicyProxy->GetRingerMode(retRingMode);
    audioPolicyProxy->SetAudioScene(AUDIO_SCENE_DEFAULT);
    audioPolicyProxy->GetAudioScene(retScene);

    bool mute = GetData<bool>();
    int32_t volumeType = GetData<int32_t>();
    bool retMute = GetData<bool>();
    bool retActive = GetData<bool>();
    audioPolicyProxy->SetStreamMuteLegacy(STREAM_MUSIC, mute, DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->SetStreamMute(STREAM_MUSIC, mute, DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->GetStreamMute(STREAM_MUSIC, retMute);
    audioPolicyProxy->IsStreamActive(STREAM_MUSIC, retActive);

    bool active = GetData<bool>();
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    bool isAllowed = GetData<bool>();
    audioPolicyProxy->SetDeviceActive(DEVICE_TYPE_SPEAKER, active, uid);
    audioPolicyProxy->IsAllowedPlayback(uid, pid, isAllowed);
    uint32_t sessionID = GetData<uint32_t>();
    bool isRunning = GetData<bool>();
    audioPolicyProxy->SetInputDevice(DEVICE_TYPE_SPEAKER, sessionID, SOURCE_TYPE_MIC, isRunning);
    audioPolicyProxy->SetVoiceRingtoneMute(mute);
    bool hasSession = GetData<bool>();
    audioPolicyProxy->NotifySessionStateChange(uid, pid, hasSession);
    audioPolicyProxy->ResetAllProxy();
    bool isVirtual = GetData<bool>();
    audioPolicyProxy->SetVirtualCall(isVirtual);

    int32_t retDeviceType = GetData<int32_t>();
    uint16_t uRetDeviceType = GetData<uint16_t>();
    audioPolicyProxy->IsDeviceActive(DEVICE_TYPE_SPEAKER, retActive);
    audioPolicyProxy->GetActiveOutputDevice(retDeviceType);
    audioPolicyProxy->GetDmDeviceType(uRetDeviceType);
    audioPolicyProxy->GetActiveInputDevice(retDeviceType);
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
    int32_t streamType = GetData<int32_t>();
    string retStr = "retStr";
    audioPolicyProxy->GetSelectedDeviceInfo(uid, pid, streamType, retStr);

    int32_t strategy = GetData<int32_t>();
    audioPolicyProxy->ActivateAudioSession(strategy);
    audioPolicyProxy->DeactivateAudioSession();
    bool retIsActivated = GetData<bool>();
    audioPolicyProxy->IsAudioSessionActivated(retIsActivated);

    int32_t zoneID = GetData<int32_t>();
    int32_t clientId = GetData<int32_t>();
    int32_t retStreamType = GetData<int32_t>();
    bool isUpdatedAudioStrategy = GetData<bool>();
    audioPolicyProxy->ActivatePreemptMode();
    audioPolicyProxy->DeactivatePreemptMode();
    audioPolicyProxy->GetStreamInFocus(zoneID, retStreamType);
    audioPolicyProxy->GetStreamInFocusByUid(uid, zoneID, retStreamType);

    uint32_t count = GetData<uint32_t>();
    audioPolicyProxy->ReconfigureAudioChannel(count, DEVICE_TYPE_SPEAKER);
    audioPolicyProxy->UpdateStreamState(clientId, STREAM_PAUSE, STREAM_USAGE_MUSIC);

    int32_t groupId = GetData<int32_t>();
    int32_t ret = GetData<int32_t>();
    std::string networkId = "networkId";
    std::string key = "key";
    std::string uri = "uri";
    std::string retUri = "retUri";
    audioPolicyProxy->GetNetworkIdByGroupId(groupId, networkId);
    audioPolicyProxy->SetSystemSoundUri(key, uri);
    audioPolicyProxy->GetSystemSoundUri(key, retUri);
    audioPolicyProxy->GetMaxRendererInstances(ret);
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

    bool ret = GetData<bool>();
    audioPolicyProxy->IsSpatializationEnabled(ret);
    std::string address = "address";
    audioPolicyProxy->IsSpatializationEnabled(address, ret);
    audioPolicyProxy->IsSpatializationEnabledForCurrentDevice(ret);
    bool enable = GetData<bool>();
    audioPolicyProxy->SetSpatializationEnabled(enable);
    audioPolicyProxy->IsHeadTrackingEnabled(ret);
    audioPolicyProxy->IsHeadTrackingEnabled(address, ret);
    audioPolicyProxy->SetHeadTrackingEnabled(enable);
    audioPolicyProxy->IsSpatializationSupported(ret);
    audioPolicyProxy->IsSpatializationSupportedForDevice(address, ret);
    AudioSpatialDeviceState audioSpatialDeviceState = {
        "1234",
        true,
        true,
        AudioSpatialDeviceType::EARPHONE_TYPE_NONE
    }
    audioPolicyProxy->UpdateSpatialDeviceState(audioSpatialDeviceState);
    int32_t zoneID = GetData<int32_t>();
    audioPolicyProxy->ReleaseAudioInterruptZone(zoneID);
    bool active = GetData<bool>();
    int32_t uid = GetData<int32_t>();
    int32_t audioSessionScene = GetData<int32_t>();
    audioPolicyProxy->SetCallDeviceActive(DEVICE_TYPE_SPEAKER, active, address, uid);
    audioPolicyProxy->SetAudioSessionScene(audioSessionScene);
    audioPolicyProxy->SetDefaultOutputDevice(DeviceType::DEVICE_TYPE_DEFAULT);

    int32_t deviceType = GetData<int32_t>();
    audioPolicyProxy->GetDefaultOutputDevice(DeviceType);
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
    bool ret = GetData<bool>();
    audioPolicyProxy->IsHighResolutionExist(ret);
    audioPolicyProxy->SetHighResolutionExist(highResExist);

    int32_t deviceId = GetData<int32_t>();
    std::string macAddress = "macAddress";
    std::string networkId = "networkId";

    audioPolicyProxy->SetAudioDeviceAnahsCallback(object);
    audioPolicyProxy->UnsetAudioDeviceAnahsCallback();

    int32_t sourceType = GetData<int32_t>();
    int32_t streamUsage = GetData<int32_t>();
    int32_t retMinVolumeLevel = GetData<int32_t>();
    int32_t retMaxVolumeLevel = GetData<int32_t>();
    int32_t retVolumeLevel = GetData<int32_t>();
    bool isMute = GetData<bool>();
    audioPolicyProxy->IsAcousticEchoCancelerSupported(sourceType, ret);
    audioPolicyProxy->GetMaxVolumeLevelByUsage(sourceType, retMaxVolumeLevel);
    audioPolicyProxy->GetMinVolumeLevelByUsage(streamUsage, retMinVolumeLevel);
    audioPolicyProxy->GetVolumeLevelByUsage(streamUsage, retVolumeLevel);
    audioPolicyProxy->GetStreamMuteByUsage(streamUsage, isMute);
}

void AudioPolicyProxyZoneFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    std::string name = "name";
    int32_t zoneId = GetData<int32_t>();
    bool enable = GetData<bool>();
    int32_t uid = GetData<int32_t>();
    std::string deviceTag = "deviceTag";

    audioPolicyProxy->RegisterAudioZoneClient(object);
    audioPolicyProxy->ReleaseAudioZone(zoneId);
    audioPolicyProxy->EnableAudioZoneReport(enable);
    audioPolicyProxy->EnableAudioZoneChangeReport(zoneId, enable);
    audioPolicyProxy->AddUidToAudioZone(zoneId, uid);
    audioPolicyProxy->RemoveUidFromAudioZone(zoneId, uid);
    audioPolicyProxy->EnableSystemVolumeProxy(zoneId, enable);
    audioPolicyProxy->EnableAudioZoneInterruptReport(zoneId, deviceTag, enable);
}

void AudioPolicyProxyGetMaxVolumeLevelFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t volumnType = GetData<int32_t>();
    int32_t volumnLevel = GetData<int32_t>();

    audioPolicyProxy->GetMaxVolumeLevel(volumnType, volumnLevel);
}

void AudioPolicyProxyGetMinVolumeLevelFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t volumnType = GetData<int32_t>();
    int32_t volumnLevel = GetData<int32_t>();

    audioPolicyProxy->GetMinVolumeLevel(volumnType, volumnLevel);
}

void AudioPolicyProxySetSystemVolumeLevelLegacyFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t volumnType = GetData<int32_t>();
    int32_t volumnLevel = GetData<int32_t>();

    audioPolicyProxy->SetSystemVolumeLevelLegacy(volumnType, volumnLevel);
}

void AudioPolicyProxySetAdjustVolumeForZoneFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t zoneId = GetData<int32_t>();

    audioPolicyProxy->SetAdjustVolumeForZone(zoneId);
}

void AudioPolicyProxyIsStreamActiveByStreamUsageFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t streamUsage = GetData<int32_t>();
    bool active = GetData<bool>();

    audioPolicyProxy->IsStreamActiveByStreamUsage(streamUsage, active);
}

void AudioPolicyProxySetLowPowerVolumeFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t streamId = GetData<int32_t>();
    float volume = GetData<float>();

    audioPolicyProxy->SetLowPowerVolume(streamId, volume);
}

void AudioPolicyProxySetClientCallbacksEnableFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t callbackchange = GetData<int32_t>();
    bool enable = GetData<bool>();

    audioPolicyProxy->SetClientCallbacksEnable(callbackchange, enable);
}

void AudioPolicyProxyGetMaxRendererInstancesFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t ret = GetData<int32_t>();

    audioPolicyProxy->GetMaxRendererInstances(ret);
}

void AudioPolicyProxyIsVgsVolumeSupportedFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    bool supported = GetData<bool>();

    audioPolicyProxy->IsVgsVolumeSupported(supported);
}

void AudioPolicyProxyIsSpatializationEnabledFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    std::string address = "address";
    bool ret = GetData<bool>();

    audioPolicyProxy->IsSpatializationEnabled(address, ret);
}

void AudioPolicyProxyIsHeadTrackingEnabledFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    std::string address = "address";
    bool ret = GetData<bool>();

    audioPolicyProxy->IsHeadTrackingEnabled(ret);
    audioPolicyProxy->IsHeadTrackingEnabled(address, ret);
}

void AudioPolicyProxyIsSpatializationSupportedFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    bool ret = GetData<bool>();

    audioPolicyProxy->IsSpatializationSupported(ret);
}

void AudioPolicyProxyIsSpatializationSupportedForDeviceFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    std::string address = "address";
    bool ret = GetData<bool>();

    audioPolicyProxy->IsSpatializationSupportedForDevice(address, ret);
}

void AudioPolicyProxyIsHeadTrackingSupportedFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    bool ret = GetData<bool>();

    audioPolicyProxy->IsHeadTrackingSupported(ret);
}

void AudioPolicyProxyIsHeadTrackingSupportedForDeviceFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    std::string address = "address";
    bool ret = GetData<bool>();

    audioPolicyProxy->IsHeadTrackingSupportedForDevice(address, ret);
}

void AudioPolicyProxySetZoneDeviceVisibleFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    bool visible = GetData<bool>();

    audioPolicyProxy->SetZoneDeviceVisible(visible);
}

void AudioPolicyProxyGetSpatializationSceneTypeFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t spatializationSceneType = GetData<int32_t>();

    audioPolicyProxy->SetZoneDeviceVisible(spatializationSceneType);
}

void AudioPolicyProxyGetMaxAmplitudeFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    int32_t deviceId = GetData<int32_t>();
    float retMaxAmplitude = GetData<float>();

    audioPolicyProxy->GetMaxAmplitude(deviceId, retMaxAmplitude);
}

void AudioPolicyProxyIsHeadTrackingDataRequestedFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    std::string macAddress = "macAddress";
    bool ret = GetData<bool>();

    audioPolicyProxy->IsHeadTrackingDataRequested(macAddress, ret);
}

void AudioPolicyProxyLoadSplitModuleFuzzTest()
{
    GetServerPtr();
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_POLICY_SERVICE_ID);

    std::shared_ptr<AudioPolicyProxy> audioPolicyProxy = std::make_shared<AudioPolicyProxy>(object);
    if (audioPolicyProxy == nullptr) {
        return;
    }

    std::string splitArgs = "splitArgs";
    std::string networkId = "networkId";

    audioPolicyProxy->LoadSplitModule(splitArgs, networkId);
}

typedef void (*TestFuncs[28])();

TestFuncs g_testFuncs = {
    AudioPolicyCallbackFuzzTest,
    AudioPolicyMicrophoneFuzzTest,
    AudioPolicyVolumeFuzzTest,
    AudioPolicyProxyOneFuzzTest,
    AudioPolicyProxyTwoFuzzTest,
    AudioPolicyProxyThreeFuzzTest,
    AudioPolicyProxyFourFuzzTest,
    AudioPolicyProxyZoneFuzzTest,
    AudioPolicyProxyGetMaxVolumeLevelFuzzTest,
    AudioPolicyProxyGetMinVolumeLevelFuzzTest,
    AudioPolicyProxySetSystemVolumeLevelLegacyFuzzTest,
    AudioPolicyProxySetAdjustVolumeForZoneFuzzTest,
    AudioPolicyProxyIsStreamActiveByStreamUsageFuzzTest,
    AudioPolicyProxySetLowPowerVolumeFuzzTest,
    AudioPolicyProxySetClientCallbacksEnableFuzzTest,
    AudioPolicyProxyGetMaxRendererInstancesFuzzTest,
    AudioPolicyProxyIsVgsVolumeSupportedFuzzTest,
    AudioPolicyProxyIsSpatializationEnabledFuzzTest,
    AudioPolicyProxyIsHeadTrackingEnabledFuzzTest,
    AudioPolicyProxyIsSpatializationSupportedFuzzTest,
    AudioPolicyProxyIsSpatializationSupportedForDeviceFuzzTest,
    AudioPolicyProxyIsHeadTrackingSupportedFuzzTest,
    AudioPolicyProxyIsHeadTrackingSupportedForDeviceFuzzTest,
    AudioPolicyProxySetZoneDeviceVisibleFuzzTest,
    AudioPolicyProxyGetSpatializationSceneTypeFuzzTest,
    AudioPolicyProxyGetMaxAmplitudeFuzzTest,
    AudioPolicyProxyIsHeadTrackingDataRequestedFuzzTest,
    AudioPolicyProxyLoadSplitModuleFuzzTest
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