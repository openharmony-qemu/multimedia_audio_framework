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
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "message_parcel.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t LIMITSIZE = 4;
const int32_t MOD_NUM_TWO = 2;
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_STOPPED = 1;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
bool g_hasServerInit = false;
bool g_hasPermission = false;
const float AUDIO_BALANCE = 1.0f;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";

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

void AudioPolicyServiceEnhanceOneFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    PowerMgr::PowerState state = PowerMgr::PowerState::STAND_BY;
    GetServerPtr()->audioPolicyService_.HandlePowerStateChanged(state);

    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    if (audioRendererFilter == nullptr) {return;}
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    sptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = new AudioDeviceDescriptor();
    std::vector<sptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.ConnectVirtualDevice(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.SelectOutputDeviceForFastInner(audioRendererFilter, selectedDesc);
    GetServerPtr()->audioPolicyService_.FilterSinkInputs(audioRendererFilter, true);
    GetServerPtr()->audioPolicyService_.WriteSelectInputSysEvents(
        selectedDesc, SOURCE_TYPE_VOICE_RECOGNITION, AUDIO_SCENE_RINGING);

    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint32_t sessionId = *reinterpret_cast<const uint32_t*>(rawData);
    GetServerPtr()->audioPolicyService_.NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);
    GetServerPtr()->audioPolicyService_.SetCurrentInputDeviceType(DeviceType::DEVICE_TYPE_INVALID);
    GetServerPtr()->audioPolicyService_.UpdateDualToneState(true, sessionId);
    GetServerPtr()->audioPolicyService_.UpdateDualToneState(false, sessionId);

    std::unique_ptr<AudioDeviceDescriptor> desc = std::make_unique<AudioDeviceDescriptor>();
    vector<shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    GetServerPtr()->audioPolicyService_.HandleScoOutputDeviceFetched(desc, rendererChangeInfos);
    GetServerPtr()->audioPolicyService_.NeedRehandleA2DPDevice(desc);

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::NEW_DEVICE_AVAILABLE;
    GetServerPtr()->audioPolicyService_.ActivateA2dpDeviceWhenDescEnabled(desc, rendererChangeInfos, reason);
    GetServerPtr()->audioPolicyService_.ActivateA2dpDevice(desc, rendererChangeInfos, reason);

    int32_t callerPid = *reinterpret_cast<const uint32_t*>(rawData);
    int32_t streamFlag = *reinterpret_cast<const uint32_t*>(rawData);
    GetServerPtr()->audioPolicyService_.TriggerRecreateCapturerStreamCallback(callerPid, sessionId, streamFlag, reason);
    GetServerPtr()->audioPolicyService_.TriggerRecreateRendererStreamCallback(callerPid, sessionId, streamFlag, reason);
}

void AudioPolicyServiceEnhanceTwoFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    vector<std::unique_ptr<AudioDeviceDescriptor>> descs =
        GetServerPtr()->audioPolicyService_.audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1);
    GetServerPtr()->audioPolicyService_.FetchStreamForA2dpMchStream(rendererChangeInfo, descs);

    std::unique_ptr<AudioDeviceDescriptor> desc = std::make_unique<AudioDeviceDescriptor>();
    vector<shared_ptr<AudioCapturerChangeInfo>> capturerChangeInfos;
    GetServerPtr()->audioPolicyService_.HandleScoInputDeviceFetched(desc, capturerChangeInfos);
    GetServerPtr()->audioPolicyService_.BluetoothScoFetch(desc, capturerChangeInfos, SOURCE_TYPE_VOICE_RECOGNITION);
    GetServerPtr()->audioPolicyService_.BluetoothScoDisconectForRecongnition();

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::NEW_DEVICE_AVAILABLE;
    std::shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = std::make_shared<AudioCapturerChangeInfo>();
    GetServerPtr()->audioPolicyService_.NotifyRecreateCapturerStream(true, capturerChangeInfo, reason);
    GetServerPtr()->audioPolicyService_.WriteInputRouteChangeEvent(desc, reason);

    GetServerPtr()->audioPolicyService_.LoadA2dpModule(DEVICE_TYPE_BLUETOOTH_A2DP);
    GetServerPtr()->audioPolicyService_.LoadDefaultUsbModule(INPUT_DEVICE);
    GetServerPtr()->audioPolicyService_.HandleActiveDevice(DEVICE_TYPE_INVALID);

    std::string networkId = "LocalDevice";
    GetServerPtr()->audioPolicyService_.ActivateNewDevice(networkId, DEVICE_TYPE_INVALID, true);

    sptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = new AudioDeviceDescriptor();
    std::vector<sptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.CheckAndNotifyUserSelectedDevice(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.OnMicrophoneBlockedUpdate(DEVICE_TYPE_INVALID, DEVICE_BLOCKED);
    GetServerPtr()->audioPolicyService_.OnBlockedStatusUpdated(DEVICE_TYPE_NONE, DEVICE_BLOCKED);
    GetServerPtr()->audioPolicyService_.ResetToSpeaker(DEVICE_TYPE_BLUETOOTH_SCO);
    GetServerPtr()->audioPolicyService_.ResetToSpeaker(DEVICE_TYPE_NONE);

    AudioDeviceDescriptor updatedDesc;
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(updatedDesc, true);
    GetServerPtr()->audioPolicyService_.UpdateDeviceList(updatedDesc, true, selectedDesc, reason);

    std::string macAddress = "";
    std::unordered_map<uint32_t, bool> sessionIDToSpatialization;
    GetServerPtr()->audioPolicyService_.UpdateA2dpOffloadFlagBySpatialService(macAddress, sessionIDToSpatialization);
    GetServerPtr()->audioPolicyService_.UpdateA2dpOffloadFlagForAllStream(sessionIDToSpatialization, DEVICE_TYPE_NONE);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInRouterMap(networkId);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInFastRouterMap(networkId);
    GetServerPtr()->audioPolicyService_.HandleOfflineDistributedDevice();
}

void AudioPolicyServiceEnhanceThreeFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    DStatusInfo statusInfo;
    sptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = new AudioDeviceDescriptor();
    std::vector<sptr<AudioDeviceDescriptor>> descForCb;
    descForCb.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(statusInfo, true);
    GetServerPtr()->audioPolicyService_.OnMonoAudioConfigChanged(true);
    GetServerPtr()->audioPolicyService_.UpdateDescWhenNoBTPermission(descForCb);
    GetServerPtr()->audioPolicyService_.TriggerMicrophoneBlockedCallback(descForCb, DEVICE_BLOCKED);

    float audioBalance = AUDIO_BALANCE;
    GetServerPtr()->audioPolicyService_.OnAudioBalanceChanged(audioBalance);

    std::string macAddress = "";
    GetServerPtr()->audioPolicyService_.SetAbsVolumeSceneAsync(macAddress, true);
    GetServerPtr()->audioPolicyService_.SetDeviceAbsVolumeSupported(macAddress, true);
    GetServerPtr()->audioPolicyService_.SetNormalVoipFlag(true);

    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_NONE);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_OUT_SPEAKER);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_OUT_HEADSET);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_OUT_LINEOUT);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_OUT_HDMI);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_OUT_USB);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_OUT_USB_EXT);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_OUT_DAUDIO_DEFAULT);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_IN_MIC);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_IN_HS_MIC);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_IN_LINEIN);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_IN_USB_EXT);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_IN_DAUDIO_DEFAULT);
    GetServerPtr()->audioPolicyService_.GetDeviceRole(AUDIO_PIN_OUT_DP);

    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_EARPIECE);
    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_SPEAKER);
    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_BLUETOOTH_A2DP);
    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_FILE_SINK);
    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_USB_ARM_HEADSET);
    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_WIRED_HEADSET);
    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_USB_HEADSET);
    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_BLUETOOTH_SCO);
    GetServerPtr()->audioPolicyService_.UpdateInputDeviceInfo(DEVICE_TYPE_DEFAULT);
}

void AudioPolicyServiceEnhanceFourFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.ResumeStreamState();

    int32_t clientUid = *reinterpret_cast<const int32_t*>(rawData);
    int32_t sessionId = *reinterpret_cast<const int32_t*>(rawData);
    StreamSetStateEventInternal streamSetStateEventInternal;
    GetServerPtr()->audioPolicyService_.UpdateStreamState(clientUid, streamSetStateEventInternal);
    GetServerPtr()->audioPolicyService_.GetUid(sessionId);

    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_NONE);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_OUT_SPEAKER);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_OUT_DAUDIO_DEFAULT);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_OUT_HEADSET);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_OUT_LINEOUT);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_OUT_HDMI);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_OUT_USB);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_OUT_USB_EXT);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_OUT_USB_HEADSET);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_IN_USB_HEADSET);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_IN_MIC);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_IN_DAUDIO_DEFAULT);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_IN_HS_MIC);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_IN_LINEIN);
    GetServerPtr()->audioPolicyService_.GetDeviceTypeFromPin(AUDIO_PIN_IN_USB_EXT);

    AudioDeviceDescriptor deviceInfo;
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_PLAYBACK;
    GetServerPtr()->audioPolicyService_.GetProcessDeviceInfo(config, true, deviceInfo);

    AudioProcessConfig processInfoConfig;
    processInfoConfig.audioMode = AUDIO_MODE_RECORD;
    GetServerPtr()->audioPolicyService_.GetProcessDeviceInfo(processInfoConfig, true, deviceInfo);

    AudioProcessConfig processConfig;
    processConfig.audioMode = AUDIO_MODE_RECORD;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    GetServerPtr()->audioPolicyService_.GetProcessDeviceInfo(processConfig, true, deviceInfo);

    int32_t type = *reinterpret_cast<const int32_t*>(rawData);
    sptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = new AudioDeviceDescriptor();
    std::vector<sptr<AudioDeviceDescriptor>> preferredDeviceList;
    preferredDeviceList.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.GetVoipDeviceInfo(processConfig, deviceInfo, type, preferredDeviceList);

    std::shared_ptr<AudioSharedMemory> buffer;
    GetServerPtr()->audioPolicyService_.InitSharedVolume(buffer);
}

void AudioPolicyServiceEnhanceFiveFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    Volume vol;
    GetServerPtr()->audioPolicyService_.GetSharedVolume(STREAM_MUSIC, DEVICE_TYPE_EARPIECE, vol);

    SourceType targetSource;
    bool useMatchingPropInfo;
    GetServerPtr()->audioPolicyService_.
        GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_RECOGNITION, targetSource, useMatchingPropInfo);
    GetServerPtr()->audioPolicyService_.
        GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_COMMUNICATION, targetSource, useMatchingPropInfo);
    GetServerPtr()->audioPolicyService_.
        GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_TRANSCRIPTION, targetSource, useMatchingPropInfo);
    GetServerPtr()->audioPolicyService_.
        GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_CALL, targetSource, useMatchingPropInfo);
    GetServerPtr()->audioPolicyService_.
        GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_MIC, targetSource, useMatchingPropInfo);

    uint64_t sessionID = SOURCE_TYPE_REMOTE_CAST;
    GetServerPtr()->audioPolicyService_.OnCapturerSessionRemoved(sessionID);
    GetServerPtr()->audioPolicyService_.HandleRemainingSource();
    GetServerPtr()->audioPolicyService_.CloseNormalSource();
    GetServerPtr()->audioPolicyService_.GetEcType(DEVICE_TYPE_MIC, DEVICE_TYPE_SPEAKER);
    GetServerPtr()->audioPolicyService_.ShouldOpenMicRef(SOURCE_TYPE_VOICE_COMMUNICATION);

    std::string role = ROLE_SOURCE;
    PipeInfo pipeInfo;
    GetServerPtr()->audioPolicyService_.GetHalNameForDevice(role, DEVICE_TYPE_MIC);
    GetServerPtr()->audioPolicyService_.GetPipeInfoByDeviceTypeForEc(role, DEVICE_TYPE_MIC, pipeInfo);
    GetServerPtr()->audioPolicyService_.GetPipeNameByDeviceForEc(role, DEVICE_TYPE_SPEAKER);
    GetServerPtr()->audioPolicyService_.GetPipeNameByDeviceForEc(role, DEVICE_TYPE_WIRED_HEADSET);
    GetServerPtr()->audioPolicyService_.GetPipeNameByDeviceForEc(role, DEVICE_TYPE_USB_HEADSET);
    GetServerPtr()->audioPolicyService_.GetPipeNameByDeviceForEc(role, DEVICE_TYPE_BLUETOOTH_SCO);
    GetServerPtr()->audioPolicyService_.GetPipeNameByDeviceForEc(role, DEVICE_TYPE_MIC);
    GetServerPtr()->audioPolicyService_.GetPipeNameByDeviceForEc(role, DEVICE_TYPE_USB_ARM_HEADSET);
    GetServerPtr()->audioPolicyService_.GetPipeNameByDeviceForEc(role, DEVICE_TYPE_DP);
    GetServerPtr()->audioPolicyService_.GetPipeNameByDeviceForEc(role, DEVICE_TYPE_NONE);

    SessionInfo sessionInfo;
    GetServerPtr()->audioPolicyService_.ReloadSourceForSession(sessionInfo);

    GetServerPtr()->audioPolicyService_.isEcFeatureEnable_ = true;
    GetServerPtr()->audioPolicyService_.normalSourceOpened_ = SOURCE_TYPE_VOICE_COMMUNICATION;
    GetServerPtr()->audioPolicyService_.ReloadSourceForDeviceChange(DEVICE_TYPE_DEFAULT, DEVICE_TYPE_DEFAULT, true);
    GetServerPtr()->audioPolicyService_.UpdateAudioEcInfo(DEVICE_TYPE_DEFAULT, DEVICE_TYPE_DEFAULT);
}

void AudioPolicyServiceEnhanceSixFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    GetServerPtr()->audioPolicyService_.isMicRefFeatureEnable_ = true;
    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;
    GetServerPtr()->audioPolicyService_.ReloadSourceForEffect(oldPropertyArray, newPropertyArray);

    std::string halNameDP = DP_CLASS;
    std::string halNameUSB = USB_CLASS;
    std::string halName = "";
    StreamPropInfo outModuleInfo;
    GetServerPtr()->audioPolicyService_.GetEcSamplingRate(halName, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetEcSamplingRate(halNameDP, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetEcSamplingRate(halNameUSB, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetEcFormat(halName, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetEcFormat(halNameDP, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetEcFormat(halNameUSB, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetEcChannels(halName, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetEcChannels(halNameDP, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetEcChannels(halNameUSB, outModuleInfo);
    GetServerPtr()->audioPolicyService_.GetAudioEcInfo();

    AudioModuleInfo moduleInfo;
    StreamPropInfo targetInfo;
    GetServerPtr()->audioPolicyService_.UpdateModuleInfoForEc(moduleInfo);
    GetServerPtr()->audioPolicyService_.UpdateModuleInfoForMicRef(moduleInfo, SOURCE_TYPE_VOICE_COMMUNICATION);
    GetServerPtr()->audioPolicyService_.UpdateStreamEcInfo(moduleInfo, SOURCE_TYPE_VOICE_COMMUNICATION);
    GetServerPtr()->audioPolicyService_.UpdateStreamCommonInfo(moduleInfo, targetInfo, SOURCE_TYPE_VOICE_COMMUNICATION);
    GetServerPtr()->audioPolicyService_.isEcFeatureEnable_ = true;
    GetServerPtr()->audioPolicyService_.UpdateStreamCommonInfo(moduleInfo, targetInfo, SOURCE_TYPE_VOICE_COMMUNICATION);

    sptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = new AudioDeviceDescriptor();
    std::vector<sptr<AudioDeviceDescriptor>> descs;
    descs.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.DeviceFilterByUsageInner(MEDIA_OUTPUT_DEVICES, descs);

    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;
    GetServerPtr()->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);

    uint32_t uid = 0;
    std::string bundleName = "";
    GetServerPtr()->audioPolicyService_.GetAndSaveClientType(uid, bundleName);
}

void AudioPolicyServiceEnhanceSevenFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    GetServerPtr()->audioPolicyService_.OnPreferredStateUpdated(desc, CATEGORY_UPDATE, reason);

    AudioDeviceDescriptor descUpdated;
    descUpdated.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    descUpdated.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    descUpdated.connectState_ = DEACTIVE_CONNECTED;
    GetServerPtr()->audioPolicyService_.OnDeviceInfoUpdated(desc, ENABLE_UPDATE);
    AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Tone Load Configuration failed");

    uint32_t sessionID = *reinterpret_cast<const uint32_t*>(rawData);
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    GetServerPtr()->audioPolicyService_.SetAudioConcurrencyCallback(sessionID, object);

    vector<std::unique_ptr<AudioDeviceDescriptor>> descs =
        GetServerPtr()->audioPolicyService_.audioRouterCenter_.FetchOutputDevices(STREAM_USAGE_MEDIA, -1);
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    GetServerPtr()->audioPolicyService_.SelectRingerOrAlarmDevices(descs, rendererChangeInfo);

    GetServerPtr()->audioPolicyService_.ringerModeMute_ = false;
    GetServerPtr()->audioPolicyService_.ResetRingerModeMute();

    std::string deviceAddress = "";
    vector<int32_t> sessionIds = {0};
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->ConnectA2dpOffload(deviceAddress, sessionIds);

    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->IsA2dpOffloadConnecting(MOD_NUM_TWO);

    EventFwk::CommonEventData eventData;
    GetServerPtr()->audioPolicyService_.OnReceiveEvent(eventData);
    GetServerPtr()->audioPolicyService_.UnsetAudioDeviceAnahsCallback();

    int32_t streamId = *reinterpret_cast<const int32_t*>(rawData);
    GetServerPtr()->audioPolicyService_.GetLowPowerVolume(streamId);
    GetServerPtr()->audioPolicyService_.GetSingleStreamVolume(streamId);
}

void AudioPolicyServiceEnhanceEightFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::string networkId = "RemoteDevice";
    sptr<AudioDeviceDescriptor> remote = new AudioDeviceDescriptor();
    GetServerPtr()->audioPolicyService_.OpenRemoteAudioDevice(networkId, INPUT_DEVICE, DEVICE_TYPE_EARPIECE, remote);
    GetServerPtr()->audioPolicyService_.ConfigDistributedRoutingRole(remote, CAST_TYPE_ALL);
    GetServerPtr()->audioPolicyService_.SwitchActiveA2dpDevice(remote);

    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    GetServerPtr()->audioPolicyService_.SelectFastInputDevice(audioCapturerFilter, remote);

    std::vector<SourceOutput> sourceOutputs;
    GetServerPtr()->audioPolicyService_.MoveToRemoteInputDevice(sourceOutputs, remote);

    AudioProcessConfig config;
    GetServerPtr()->audioPolicyService_.SetWakeUpAudioCapturerFromAudioServer(config);
    GetServerPtr()->audioPolicyService_.NotifyWakeUpCapturerRemoved();
    GetServerPtr()->audioPolicyService_.IsAbsVolumeSupported();
    GetServerPtr()->audioPolicyService_.CloseWakeUpAudioCapturer();

    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE;
    std::unique_ptr<AudioDeviceDescriptor> desc = std::make_unique<AudioDeviceDescriptor>();
    std::shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    GetServerPtr()->audioPolicyService_.IsFastFromA2dpToA2dp(desc, rendererChangeInfo, reason);

    AudioModuleInfo moduleInfo;
    AudioStreamInfo audioStreamInfo;
    GetServerPtr()->audioPolicyService_.ReloadA2dpAudioPort(moduleInfo, DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo);
    GetServerPtr()->audioPolicyService_.SetDeviceActive(DEVICE_TYPE_EARPIECE, false);

    std::string anahsShowType = "";
    GetServerPtr()->audioPolicyService_.OnUpdateAnahsSupport(anahsShowType);

    std::string macAddress = "";
    std::string deviceName = "";
    AudioStreamInfo streamInfo;
    GetServerPtr()->
        audioPolicyService_.ReloadA2dpOffloadOnDeviceChanged(DEVICE_TYPE_EARPIECE, macAddress, deviceName, streamInfo);
}

void AudioPolicyServiceEnhanceNineFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    DStatusInfo statusInfo;
    std::vector<sptr<AudioDeviceDescriptor>> descForCb;
    GetServerPtr()->audioPolicyService_.HandleDistributedDeviceUpdate(statusInfo, descForCb);
    GetServerPtr()->audioPolicyService_.LoadSinksForCapturer();
    GetServerPtr()->audioPolicyService_.GetEffectManagerInfo();
    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnVoipConfigParsed(true);

    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.GetAudioAdapterInfos(adapterInfoMap);

    std::unordered_map<std::string, std::string> volumeGroupData;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.GetVolumeGroupData(volumeGroupData);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.GetInterruptGroupData(volumeGroupData);

    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.GetDeviceClassInfo(deviceClassInfo);

    GlobalConfigs globalConfigs;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.GetGlobalConfigs(globalConfigs);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.GetVoipConfig();

    int32_t clientId = *reinterpret_cast<const int32_t*>(rawData);
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(rawData, size);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    GetServerPtr()->audioPolicyService_.SetAvailableDeviceChangeCallback(clientId, MEDIA_OUTPUT_DEVICES, object, true);
    GetServerPtr()->audioPolicyService_.UnsetAvailableDeviceChangeCallback(clientId, MEDIA_OUTPUT_DEVICES);
    GetServerPtr()->audioPolicyService_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_FILE_SINK;

    uint32_t channelCount = CHANNEL_4;
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelCount, DEVICE_TYPE_FILE_SINK);
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelCount, DEVICE_TYPE_FILE_SOURCE);

    sptr<AudioDeviceDescriptor> deviceDescriptor = new AudioDeviceDescriptor();
    SourceOutput sourceOutput;
    GetServerPtr()->audioPolicyService_.WriteInDeviceChangedSysEvents(deviceDescriptor, sourceOutput);

    int32_t notificationId = *reinterpret_cast<const int32_t*>(rawData);
    GetServerPtr()->audioPolicyService_.CancelSafeVolumeNotification(notificationId);

    int64_t activateSessionId = CHANNEL_16;
    GetServerPtr()->audioPolicyService_.CheckStreamMode(activateSessionId);
    GetServerPtr()->audioPolicyService_.SetCaptureSilentState(true);
    GetServerPtr()->audioPolicyService_.GetConverterConfig();
}

void AudioPolicyServiceEnhanceTenFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    OHOS::AudioStandard::AudioPolicyServiceEnhanceNineFuzzTest(rawData, size);
    std::string dumpString = "";
    GetServerPtr()->audioPolicyService_.DevicesInfoDump(dumpString);
    GetServerPtr()->audioPolicyService_.GetMicrophoneDescriptorsDump(dumpString);
    GetServerPtr()->audioPolicyService_.AudioPolicyParserDump(dumpString);
    GetServerPtr()->audioPolicyService_.XmlParsedDataMapDump(dumpString);
    GetServerPtr()->audioPolicyService_.EffectManagerInfoDump(dumpString);
    GetServerPtr()->audioPolicyService_.MicrophoneMuteInfoDump(dumpString);

    GetServerPtr()->audioPolicyService_.IsA2dpOrArmUsbDevice(DEVICE_TYPE_BLUETOOTH_A2DP);
    GetServerPtr()->audioPolicyService_.IsA2dpOrArmUsbDevice(DEVICE_TYPE_USB_ARM_HEADSET);
    GetServerPtr()->audioPolicyService_.IsA2dpOrArmUsbDevice(DEVICE_TYPE_SPEAKER);

    std::string address = "";
    GetServerPtr()->audioPolicyService_.ScoInputDeviceFetchedForRecongnition(true, address, SUSPEND_CONNECTED);
    GetServerPtr()->audioPolicyService_.ScoInputDeviceFetchedForRecongnition(false, address, DEACTIVE_CONNECTED);

    AudioEffectPropertyArray effectPropertyArray;
    GetServerPtr()->audioPolicyService_.GetSupportedAudioEffectProperty(effectPropertyArray);
    GetServerPtr()->audioPolicyService_.GetAudioEffectProperty(effectPropertyArray);
    GetServerPtr()->audioPolicyService_.SetAudioEffectProperty(effectPropertyArray);

    AudioEnhancePropertyArray enhancePropertyArray;
    GetServerPtr()->audioPolicyService_.GetAudioEnhanceProperty(enhancePropertyArray);
    GetServerPtr()->audioPolicyService_.SetAudioEnhanceProperty(enhancePropertyArray);

    uint32_t rotate = *reinterpret_cast<const uint32_t*>(rawData);
    GetServerPtr()->audioPolicyService_.SetRotationToEffect(rotate);

    uint32_t sessionID = *reinterpret_cast<const uint32_t*>(rawData);
    GetServerPtr()->audioPolicyService_.
        SetDefaultOutputDevice(DEVICE_TYPE_EARPIECE, sessionID, STREAM_USAGE_VOICE_MESSAGE, true);

    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->WaitForConnectionCompleted();
}

void AudioPolicyServiceEnhanceElevenFuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    std::string deviceAddress = "deviceAddress";
    int32_t playingState = *reinterpret_cast<const int32_t*>(rawData);
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged(deviceAddress, playingState);
    std::string deviceAddressEnmpy = "";
    int32_t playingStateOne = 0;
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged(deviceAddressEnmpy, playingStateOne);
    int32_t playingStateTwo = A2DP_STOPPED;
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged(deviceAddressEnmpy, playingStateTwo);
    int32_t playingStateThree = A2DP_PLAYING;
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged(deviceAddressEnmpy, playingStateThree);
}
} // namespace AudioStandard
} // namesapce OHOS

extern "C" int LLVMFuzzerInitialize(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *rawData, size_t size)
{
    /* Run your code on data */
    OHOS::AudioStandard::AudioPolicyServiceEnhanceOneFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceTwoFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceThreeFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceFourFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceFiveFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceSixFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceSevenFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceEightFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceNineFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceTenFuzzTest(rawData, size);
    OHOS::AudioStandard::AudioPolicyServiceEnhanceElevenFuzzTest(rawData, size);
    return 0;
}
