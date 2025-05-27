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
#include "audio_policy_utils.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_STOPPED = 1;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
bool g_hasServerInit = false;
bool g_hasPermission = false;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

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

void AudioPolicyServiceEnhanceOneFuzzTest()
{
    PowerMgr::PowerState state = PowerMgr::PowerState::STAND_BY;
    GetServerPtr()->audioPolicyService_.HandlePowerStateChanged(state);

    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    if (audioRendererFilter == nullptr) {return;}
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint32_t sessionId = GetData<uint32_t>();
    GetServerPtr()->audioPolicyService_.NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);

    DeviceType deviceType = GetData<DeviceType>();
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.SetCurrentInputDeviceType(deviceType);
}

void AudioPolicyServiceEnhanceTwoFuzzTest()
{
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.BluetoothScoDisconectForRecongnition();

    AudioStreamInfo audioStreamInfo;
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.LoadA2dpModule(DEVICE_TYPE_BLUETOOTH_A2DP,
        audioStreamInfo, "", "", SOURCE_TYPE_VOICE_RECOGNITION);

    DeviceType deviceType = GetData<DeviceType>();
    std::string networkId = "LocalDevice";
    bool isRemote = GetData<DeviceType>();
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.ActivateNewDevice(networkId, deviceType, isRemote);

    DeviceBlockStatus status = GetData<DeviceBlockStatus>();
    GetServerPtr()->audioPolicyService_.OnMicrophoneBlockedUpdate(deviceType, status);
    GetServerPtr()->audioPolicyService_.audioDeviceLock_.OnBlockedStatusUpdated(deviceType, status);
    GetServerPtr()->audioPolicyService_.ResetToSpeaker(DEVICE_TYPE_BLUETOOTH_SCO);
    GetServerPtr()->audioPolicyService_.ResetToSpeaker(DEVICE_TYPE_NONE);

    AudioDeviceDescriptor updatedDesc;
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(updatedDesc, isRemote);
    AudioStreamDeviceChangeReasonExt reason = GetData<AudioStreamDeviceChangeReasonExt::ExtEnum>();
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.UpdateDeviceList(updatedDesc, isRemote,
        selectedDesc, reason);

    std::string macAddress = "";
    std::unordered_map<uint32_t, bool> sessionIDToSpatialization;
    GetServerPtr()->audioPolicyService_.UpdateA2dpOffloadFlagBySpatialService(macAddress, sessionIDToSpatialization);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInRouterMap(networkId);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInFastRouterMap(networkId);
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleOfflineDistributedDevice();
}

void AudioPolicyServiceEnhanceThreeFuzzTest()
{
    DStatusInfo statusInfo;
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    descForCb.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.OnDeviceStatusUpdated(statusInfo, true);
    GetServerPtr()->audioPolicyService_.OnMonoAudioConfigChanged(true);
    GetServerPtr()->audioPolicyService_.UpdateDescWhenNoBTPermission(descForCb);
    DeviceBlockStatus status = GetData<DeviceBlockStatus>();
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.TriggerMicrophoneBlockedCallback(descForCb, status);
    float audioBalance = GetData<float>();
    GetServerPtr()->audioPolicyService_.OnAudioBalanceChanged(audioBalance);
    std::string macAddress = "";
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.SetAbsVolumeSceneAsync(macAddress, true);
    GetServerPtr()->audioPolicyService_.SetDeviceAbsVolumeSupported(macAddress, true);
    GetServerPtr()->audioPolicyService_.SetNormalVoipFlag(true);

    std::vector<AudioPin> audioPin = {
        AUDIO_PIN_NONE,
        AUDIO_PIN_OUT_SPEAKER,
        AUDIO_PIN_OUT_HEADSET,
        AUDIO_PIN_OUT_LINEOUT,
        AUDIO_PIN_OUT_HDMI,
        AUDIO_PIN_OUT_USB,
        AUDIO_PIN_OUT_USB_EXT,
        AUDIO_PIN_OUT_DAUDIO_DEFAULT,
        AUDIO_PIN_IN_MIC,
        AUDIO_PIN_IN_HS_MIC,
        AUDIO_PIN_IN_LINEIN,
        AUDIO_PIN_IN_USB_EXT,
        AUDIO_PIN_IN_DAUDIO_DEFAULT,
        AUDIO_PIN_OUT_DP,
    };
    uint32_t audioPinInt = GetData<uint32_t>() % audioPin.size();
    AudioPin pin = audioPin[audioPinInt];
    AudioPolicyUtils::GetInstance().GetDeviceRole(pin);

    std::vector<DeviceType> DeviceTypeVec = {
        DEVICE_TYPE_EARPIECE,
        DEVICE_TYPE_SPEAKER,
        DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_FILE_SINK,
        DEVICE_TYPE_USB_ARM_HEADSET,
        DEVICE_TYPE_WIRED_HEADSET,
        DEVICE_TYPE_USB_HEADSET,
        DEVICE_TYPE_BLUETOOTH_SCO,
        DEVICE_TYPE_DEFAULT,
    };
    uint32_t deviceTypeInt = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeInt];
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.UpdateInputDeviceInfo(deviceType);
}

void AudioPolicyServiceEnhanceFourFuzzTest()
{
    GetServerPtr()->audioPolicyService_.ResumeStreamState();

    int32_t clientUid = GetData<int32_t>();
    int32_t sessionId = GetData<int32_t>();
    StreamSetStateEventInternal streamSetStateEventInternal;
    GetServerPtr()->audioPolicyService_.UpdateStreamState(clientUid, streamSetStateEventInternal);
    GetServerPtr()->audioPolicyService_.GetUid(sessionId);

    std::vector<AudioPin> audioPin = {
        AUDIO_PIN_NONE,
        AUDIO_PIN_OUT_SPEAKER,
        AUDIO_PIN_OUT_DAUDIO_DEFAULT,
        AUDIO_PIN_OUT_HEADSET,
        AUDIO_PIN_OUT_LINEOUT,
        AUDIO_PIN_OUT_HDMI,
        AUDIO_PIN_OUT_USB,
        AUDIO_PIN_OUT_USB_EXT,
        AUDIO_PIN_OUT_USB_HEADSET,
        AUDIO_PIN_IN_USB_HEADSET,
        AUDIO_PIN_IN_MIC,
        AUDIO_PIN_IN_DAUDIO_DEFAULT,
        AUDIO_PIN_IN_HS_MIC,
        AUDIO_PIN_IN_LINEIN,
        AUDIO_PIN_IN_USB_EXT,
    };
    uint32_t audioPinInt = GetData<uint32_t>() % audioPin.size();
    AudioPin hdiPin = audioPin[audioPinInt];
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.GetDeviceTypeFromPin(hdiPin);

    AudioDeviceDescriptor deviceInfo;
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_PLAYBACK;
    bool lockFlag = GetData<bool>();
    GetServerPtr()->audioPolicyService_.GetProcessDeviceInfo(config, lockFlag, deviceInfo);

    AudioProcessConfig processInfoConfig;
    processInfoConfig.audioMode = AUDIO_MODE_RECORD;
    GetServerPtr()->audioPolicyService_.GetProcessDeviceInfo(processInfoConfig, lockFlag, deviceInfo);

    AudioProcessConfig processConfig;
    processConfig.audioMode = AUDIO_MODE_RECORD;
    processConfig.capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    GetServerPtr()->audioPolicyService_.GetProcessDeviceInfo(processConfig, lockFlag, deviceInfo);

    int32_t type = GetData<int32_t>();
    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> preferredDeviceList;
    preferredDeviceList.push_back(fuzzAudioDeviceDescriptorSptr);
    GetServerPtr()->audioPolicyService_.GetVoipDeviceInfo(processConfig, deviceInfo, type, preferredDeviceList);

    std::shared_ptr<AudioSharedMemory> buffer;
    GetServerPtr()->audioPolicyService_.InitSharedVolume(buffer);
}

void AudioPolicyServiceEnhanceFiveFuzzTest()
{
    Volume vol;
    AudioVolumeType streamType = GetData<AudioVolumeType>();
    DeviceType deviceType = GetData<DeviceType>();
    GetServerPtr()->audioPolicyService_.GetSharedVolume(streamType, deviceType, vol);

    uint64_t sessionID = GetData<uint32_t>();
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionRemoved(sessionID);
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.HandleRemainingSource();
    AudioDeviceDescriptor inputDevice, outputDevice;
    inputDevice.deviceType_ = DEVICE_TYPE_DEFAULT;
    outputDevice.deviceType_ = deviceType;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.ReloadSourceForDeviceChange(
        inputDevice, outputDevice, "test");
}

void AudioPolicyServiceEnhanceSixFuzzTest()
{
    AudioEnhancePropertyArray oldPropertyArray;
    AudioEnhancePropertyArray newPropertyArray;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.ReloadSourceForEffect(oldPropertyArray, newPropertyArray);

    std::shared_ptr<AudioDeviceDescriptor> fuzzAudioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(fuzzAudioDeviceDescriptorSptr);
    AudioDeviceUsage usage = GetData<AudioDeviceUsage>();
    GetServerPtr()->audioPolicyService_.DeviceFilterByUsageInner(usage, descs);

    uint32_t delayValue = GetData<uint32_t>();
    uint64_t sendDataSize = GetData<uint64_t>();
    uint32_t timeStamp = GetData<uint32_t>();
    GetServerPtr()->audioPolicyService_.OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);

    uint32_t uid = GetData<uint32_t>();
    std::string bundleName = "";
    GetServerPtr()->audioPolicyService_.GetAndSaveClientType(uid, bundleName);
}

void AudioPolicyServiceEnhanceSevenFuzzTest()
{
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.OnPreferredStateUpdated(desc, CATEGORY_UPDATE, reason);

    AudioDeviceDescriptor descUpdated;
    descUpdated.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    descUpdated.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    descUpdated.connectState_ = DEACTIVE_CONNECTED;
    GetServerPtr()->audioPolicyService_.OnDeviceInfoUpdated(desc, ENABLE_UPDATE);
    AudioPolicyUtils::GetInstance().WriteServiceStartupError("Audio Tone Load Configuration failed");

    uint32_t sessionID = GetData<uint32_t>();
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    GetServerPtr()->audioPolicyService_.SetAudioConcurrencyCallback(sessionID, object);

    GetServerPtr()->audioPolicyService_.audioVolumeManager_.ringerModeMute_ = false;
    GetServerPtr()->audioPolicyService_.ResetRingerModeMute();

    std::string deviceAddress = "";
    vector<int32_t> sessionIds = {0};
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->ConnectA2dpOffload(deviceAddress, sessionIds);

    A2dpOffloadConnectionState currentOffloadConnectionState = GetData<A2dpOffloadConnectionState>();
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(currentOffloadConnectionState);

    int32_t sessionId = GetData<uint32_t>();
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->IsA2dpOffloadConnecting(sessionId);

    EventFwk::CommonEventData eventData;
    GetServerPtr()->audioPolicyService_.OnReceiveEvent(eventData);
    GetServerPtr()->audioPolicyService_.UnsetAudioDeviceAnahsCallback();

    int32_t streamId = GetData<int32_t>();
    GetServerPtr()->audioPolicyService_.GetLowPowerVolume(streamId);
    GetServerPtr()->audioPolicyService_.GetSingleStreamVolume(streamId);
}

void AudioPolicyServiceEnhanceEightFuzzTest()
{
    std::shared_ptr<AudioDeviceDescriptor> remote = std::make_shared<AudioDeviceDescriptor>();
    CastType type = GetData<CastType>();
    GetServerPtr()->audioPolicyService_.ConfigDistributedRoutingRole(remote, type);
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.SwitchActiveA2dpDevice(remote);

    AudioProcessConfig config;
    GetServerPtr()->audioPolicyService_.SetWakeUpAudioCapturerFromAudioServer(config);
    GetServerPtr()->audioPolicyService_.NotifyWakeUpCapturerRemoved();
    GetServerPtr()->audioPolicyService_.IsAbsVolumeSupported();
    GetServerPtr()->audioPolicyService_.CloseWakeUpAudioCapturer();

    AudioModuleInfo moduleInfo;
    AudioStreamInfo audioStreamInfo;
    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.ReloadA2dpAudioPort(moduleInfo,
        DEVICE_TYPE_BLUETOOTH_A2DP, audioStreamInfo, "", "", SOURCE_TYPE_MIC);

    InternalDeviceType internalDeviceType = GetData<InternalDeviceType>();
    bool active = GetData<bool>();
    GetServerPtr()->audioPolicyService_.SetDeviceActive(internalDeviceType, active);

    std::string anahsShowType = "";
    GetServerPtr()->audioPolicyService_.OnUpdateAnahsSupport(anahsShowType);

    DeviceType deviceType = GetData<DeviceType>();
    std::string macAddress = "";
    std::string deviceName = "";
    AudioStreamInfo streamInfo;
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.ReloadA2dpOffloadOnDeviceChanged(
        deviceType, macAddress, deviceName, streamInfo);
}

void AudioPolicyServiceEnhanceNineFuzzTest()
{
    DStatusInfo statusInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleDistributedDeviceUpdate(statusInfo, descForCb, reason);
    GetServerPtr()->audioPolicyDump_.GetEffectManagerInfo();
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
    int32_t clientId = GetData<int32_t>();
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    AudioDeviceUsage usage = GetData<AudioDeviceUsage>();
    GetServerPtr()->audioPolicyService_.SetAvailableDeviceChangeCallback(clientId, usage, object, true);
    GetServerPtr()->audioPolicyService_.UnsetAvailableDeviceChangeCallback(clientId, usage);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_FILE_SINK;
    std::vector<AudioChannel> audioChannel = {
        MONO,
        STEREO,
        CHANNEL_3,
        CHANNEL_4,
        CHANNEL_5,
        CHANNEL_6,
    };
    uint32_t audioChannelInt = GetData<uint32_t>() % audioChannel.size();
    uint32_t channelCount = audioChannel[audioChannelInt];
    std::vector<DeviceType> DeviceTypeVec = {
        DEVICE_TYPE_FILE_SINK,
        DEVICE_TYPE_FILE_SOURCE,
        DEVICE_TYPE_BLUETOOTH_A2DP,
    };
    uint32_t deviceTypeInt = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeInt];
    GetServerPtr()->audioPolicyService_.ReconfigureAudioChannel(channelCount, deviceType);
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    SourceOutput sourceOutput;
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.WriteInputDeviceChangedSysEvents(deviceDescriptor,
        sourceOutput);
}

void AudioPolicyServiceEnhanceTenFuzzTest()
{
    OHOS::AudioStandard::AudioPolicyServiceEnhanceNineFuzzTest();
    int32_t notificationId = GetData<int32_t>();
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.CancelSafeVolumeNotification(notificationId);
    int64_t activateSessionId = GetData<int64_t>();
    GetServerPtr()->audioPolicyService_.CheckStreamMode(activateSessionId);

    GetServerPtr()->audioPolicyService_.GetConverterConfig();

    std::string dumpString = "";
    GetServerPtr()->audioPolicyDump_.DevicesInfoDump(dumpString);
    GetServerPtr()->audioPolicyDump_.GetMicrophoneDescriptorsDump(dumpString);
    GetServerPtr()->audioPolicyDump_.AudioPolicyParserDump(dumpString);
    GetServerPtr()->audioPolicyDump_.XmlParsedDataMapDump(dumpString);
    GetServerPtr()->audioPolicyDump_.EffectManagerInfoDump(dumpString);
    GetServerPtr()->audioPolicyDump_.MicrophoneMuteInfoDump(dumpString);

    AudioEffectPropertyArray effectPropertyArray;
    GetServerPtr()->audioPolicyService_.GetSupportedAudioEffectProperty(effectPropertyArray);
    GetServerPtr()->audioPolicyService_.GetAudioEffectProperty(effectPropertyArray);
    GetServerPtr()->audioPolicyService_.SetAudioEffectProperty(effectPropertyArray);

    AudioEnhancePropertyArray enhancePropertyArray;
    GetServerPtr()->audioPolicyService_.GetAudioEnhanceProperty(enhancePropertyArray);
    GetServerPtr()->audioPolicyService_.SetAudioEnhanceProperty(enhancePropertyArray);

    uint32_t rotate = GetData<uint32_t>();
    GetServerPtr()->audioPolicyService_.SetRotationToEffect(rotate);

    uint32_t sessionID = GetData<uint32_t>();
    DeviceType deviceTypeSet = GetData<DeviceType>();
    StreamUsage streamUsage = GetData<StreamUsage>();
    bool isRunning = GetData<bool>();
    GetServerPtr()->audioPolicyService_.
        SetDefaultOutputDevice(deviceTypeSet, sessionID, streamUsage, isRunning);
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->WaitForConnectionCompleted();
}

void AudioPolicyServiceEnhanceElevenFuzzTest()
{
    std::string deviceAddress = "deviceAddress";
    int32_t playingState = GetData<int32_t>();
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
    A2dpOffloadConnectionState currentOffloadConnectionState = GetData<A2dpOffloadConnectionState>();
    GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(currentOffloadConnectionState);
    GetServerPtr()->
        audioPolicyService_.audioA2dpOffloadManager_->OnA2dpPlayingStateChanged(deviceAddressEnmpy, playingStateThree);
}

typedef void (*TestFuncs[11])();

TestFuncs g_testFuncs = {
    AudioPolicyServiceEnhanceOneFuzzTest,
    AudioPolicyServiceEnhanceTwoFuzzTest,
    AudioPolicyServiceEnhanceThreeFuzzTest,
    AudioPolicyServiceEnhanceFourFuzzTest,
    AudioPolicyServiceEnhanceFiveFuzzTest,
    AudioPolicyServiceEnhanceSixFuzzTest,
    AudioPolicyServiceEnhanceSevenFuzzTest,
    AudioPolicyServiceEnhanceEightFuzzTest,
    AudioPolicyServiceEnhanceNineFuzzTest,
    AudioPolicyServiceEnhanceTenFuzzTest,
    AudioPolicyServiceEnhanceElevenFuzzTest,
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
