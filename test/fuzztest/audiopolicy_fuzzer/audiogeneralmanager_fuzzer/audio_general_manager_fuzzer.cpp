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

#include "audio_general_manager.h"
#include "audio_policy_manager.h"
#include "audio_info.h"
#include "audio_device_info.h"
#include <atomic>
#include <fuzzer/FuzzedDataProvider.h>
#include <string>
#include <vector>

using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;

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

class AudioDeviceRefinerFuzzTest : public AudioDeviceRefiner {
public:
    AudioDeviceRefinerFuzzTest() {}
    int32_t OnAudioOutputDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const FetchDeviceInfo &fetchDeviceInfo) override { return 0; }
    int32_t OnAudioDupDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const FetchDeviceInfo &fetchDeviceInfo) override { return 0; }
    int32_t OnAudioInputDeviceRefined(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        RouterType routerType, SourceType sourceType, int32_t clientUid, AudioPipeType audioPipeType) override { return 0; }
    int32_t GetSplitInfoRefined(std::string &splitInfo) override { return 0; }
    int32_t OnDistributedOutputChange(bool isRemote) override { return 0; }
    int32_t OnDistributedServiceOnline() override { return 0; }
    int32_t SetSplitModeReadyRefined() override { return 0; }
};

class AudioClientInfoMgrCallbackFuzzTest : public AudioClientInfoMgrCallback {
public:
    AudioClientInfoMgrCallbackFuzzTest() {}
    bool OnCheckClientInfo(const std::string &bundleName, int32_t &uid, int32_t pid) override { return true; }
    bool OnCheckMediaControllerBundle(const std::string &bundleName) override { return true; }
    bool OnQueryIsForceGetZoneDevice(const std::string &bundleName) override { return false; }
    bool OnQueryIsForceGetDevByVolumeType(const std::string &bundleName) override { return false; }
    bool OnQueryUseSystemPreferredDeviceForVolume(const std::string &bundleName) override { return false; }
};

class AudioFocusInfoChangeCallbackFuzzTest : public AudioFocusInfoChangeCallback {
public:
    AudioFocusInfoChangeCallbackFuzzTest() {}
    void OnAudioFocusInfoChange(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList) override {}
    void OnAudioFocusRequested(const AudioInterrupt &) override {}
    void OnAudioFocusAbandoned(const AudioInterrupt &) override {}
};

class AudioPreferredOutputDeviceChangeCallbackFuzzTest : public AudioPreferredOutputDeviceChangeCallback {
public:
    AudioPreferredOutputDeviceChangeCallbackFuzzTest() {}
    void OnPreferredOutputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override {}
};

class AudioPreferredInputDeviceChangeCallbackFuzzTest : public AudioPreferredInputDeviceChangeCallback {
public:
    AudioPreferredInputDeviceChangeCallbackFuzzTest() {}
    void OnPreferredInputDeviceUpdated(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc) override {}
};

class AudioManagerDeviceChangeCallbackFuzzTest : public AudioManagerDeviceChangeCallback {
public:
    AudioManagerDeviceChangeCallbackFuzzTest() {}
    void OnDeviceChange(const DeviceChangeAction &deviceChangeAction) override {}
};

class AudioManagerDeviceInfoUpdateCallbackFuzzTest : public AudioManagerDeviceInfoUpdateCallback {
public:
    AudioManagerDeviceInfoUpdateCallbackFuzzTest() {}
    void OnDeviceInfoUpdate(const DeviceChangeAction &deviceChangeAction) override {}
};

class AudioQueryClientTypeCallbackFuzzTest : public AudioQueryClientTypeCallback {
public:
    AudioQueryClientTypeCallbackFuzzTest() {}
    bool OnQueryClientType(const std::string &bundleName, uint32_t uid) override { return false; }
};

class AudioQueryDeviceVolumeBehaviorCallbackFuzzTest : public AudioQueryDeviceVolumeBehaviorCallback {
public:
    AudioQueryDeviceVolumeBehaviorCallbackFuzzTest() {}
    VolumeBehavior OnQueryDeviceVolumeBehavior() override { return {}; }
};

class VolumeKeyEventCallbackFuzzTest : public VolumeKeyEventCallback {
public:
    VolumeKeyEventCallbackFuzzTest() {}
    void OnVolumeKeyEvent(VolumeEvent volumeEvent) override {}
    void OnVolumeDegreeEvent(VolumeEvent volumeEvent) override {}
};

class AudioManagerAudioSceneChangedCallbackFuzzTest : public AudioManagerAudioSceneChangedCallback {
public:
    AudioManagerAudioSceneChangedCallbackFuzzTest() {}
    void OnAudioSceneChange(const AudioScene audioScene) override {}
};

class AudioCapturerStateChangeCallbackFuzzTest : public AudioCapturerStateChangeCallback {
public:
    AudioCapturerStateChangeCallbackFuzzTest() {}
    void OnCapturerStateChange(const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override {}
};

class AudioRendererStateChangeCallbackFuzzTest : public AudioRendererStateChangeCallback {
public:
    AudioRendererStateChangeCallbackFuzzTest() {}
    void OnRendererStateChange(const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override {}
};

class PreferredDeviceSetCallbackFuzzTest : public PreferredDeviceSetCallback {
public:
    PreferredDeviceSetCallbackFuzzTest() {}
    void OnPreferredDeviceSet(PreferredType preferredType,
        const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, int32_t uid, const std::string &caller) override {}
};

class SleAudioOperationCallbackFuzzTest : public SleAudioOperationCallback {
public:
    SleAudioOperationCallbackFuzzTest() {}
    virtual ~SleAudioOperationCallbackFuzzTest() {}
    void GetSleAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override {}
    void GetSleVirtualAudioDeviceList(std::vector<AudioDeviceDescriptor> &devices) override {}
    bool IsInBandRingOpen(const std::string &device) const override { return false; }
    uint32_t GetSupportStreamType(const std::string &device) const override { return 0; }
    int32_t SetActiveSinkDevice(const std::string &device, uint32_t streamType) override { return 0; }
    int32_t StartPlaying(const std::string &device, uint32_t streamType, int32_t timeoutMs) override { return 0; }
    int32_t StopPlaying(const std::string &device, uint32_t streamType) override { return 0; }
    int32_t ConnectAllowedProfiles(const std::string &remoteAddr) const override { return 0; }
    int32_t SetDeviceAbsVolume(const std::string &remoteAddr, uint32_t volume, uint32_t streamType) override { return 0; }
    int32_t SendUserSelection(const std::string &device, uint32_t streamType, int32_t eventType) override { return 0; }
    int32_t GetRenderPosition(const std::string &device, uint32_t &delayValue) override { return 0; }
    int32_t GetDeviceProductId(const std::string &remoteAddr, std::string &productId) override { return 0; }
    bool IsAudioLoopbackSupported(const std::string &remoteAddr) const override { return false; }
};

const vector<AudioVolumeType> g_testVolumeTypes = {
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_NOTIFICATION,
    STREAM_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_SYSTEM,
    STREAM_ACCESSIBILITY,
    STREAM_VOICE_RING,
    STREAM_ULTRASONIC,
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

const vector<DeviceFlag> g_testDeviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
};

const vector<DeviceRole> g_testDeviceRoles = {
    DeviceRole::OUTPUT_DEVICE,
    DeviceRole::INPUT_DEVICE,
    DeviceRole::DEVICE_ROLE_MAX,
};

const vector<AsrVoiceMuteMode> g_testAsrVoiceMuteModes = {
    AsrVoiceMuteMode::OUTPUT_MUTE,
    AsrVoiceMuteMode::INPUT_MUTE,
    AsrVoiceMuteMode::TTS_MUTE,
    AsrVoiceMuteMode::CALL_MUTE,
    AsrVoiceMuteMode::OUTPUT_MUTE_EX,
};

const vector<PreferredType> g_testPreferredTypes = {
    PreferredType::AUDIO_MEDIA_RENDER,
    PreferredType::AUDIO_CALL_RENDER,
    PreferredType::AUDIO_CALL_CAPTURE,
    PreferredType::AUDIO_RING_RENDER,
    PreferredType::AUDIO_RECORD_CAPTURE,
    PreferredType::AUDIO_TONE_RENDER,
    PreferredType::AUDIO_RECOGNITION_CAPTURE,
};

const vector<AudioStreamDeviceChangeReasonExt> g_testReasonExts = {
    AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN),
    AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReasonExt::ExtEnum::NEW_DEVICE_AVAILABLE),
    AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReasonExt::ExtEnum::OLD_DEVICE_UNAVALIABLE),
    AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReasonExt::ExtEnum::OVERRODE),
    AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReasonExt::ExtEnum::AUDIO_SESSION_ACTIVATE),
    AudioStreamDeviceChangeReasonExt(AudioStreamDeviceChangeReasonExt::ExtEnum::STREAM_PRIORITY_CHANGED),
};

const vector<VolumeBehavior> g_testVolumeBehaviors = {
    VolumeBehavior(true, false, "testVolume"),
    VolumeBehavior(false, true, "testVolume2"),
    VolumeBehavior(false, false, ""),
    VolumeBehavior(true, true, "testVolume3"),
};

const vector<DeviceInfoUpdateCommand> g_testDeviceInfoUpdateCommands = {
    DeviceInfoUpdateCommand::CATEGORY_UPDATE,
    DeviceInfoUpdateCommand::CONNECTSTATE_UPDATE,
    DeviceInfoUpdateCommand::ENABLE_UPDATE,
    DeviceInfoUpdateCommand::USAGE_UPDATE,
    DeviceInfoUpdateCommand::EXCEPTION_FLAG_UPDATE,
};

const vector<AudioScene> g_testAudioScenes = {
    AUDIO_SCENE_INVALID,
    AUDIO_SCENE_DEFAULT,
    AUDIO_SCENE_RINGING,
    AUDIO_SCENE_PHONE_CALL,
    AUDIO_SCENE_PHONE_CHAT,
    AUDIO_SCENE_CALL_START,
    AUDIO_SCENE_CALL_END,
    AUDIO_SCENE_VOICE_RINGING,
};

std::shared_ptr<AudioDeviceDescriptor> CreateAudioDeviceDescriptor(FuzzedDataProvider& fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    if (desc == nullptr) {
        return nullptr;
    }
    desc->deviceType_ = PickValue(fdp, g_testDeviceTypes);
    desc->deviceRole_ = PickValue(fdp, g_testDeviceRoles);
    desc->networkId_ = "LocalDevice";
    desc->displayName_ = "TestDevice";
    desc->macAddress_ = "00:00:00:00:00:00";
    desc->channelMasks_ = 1;
    return desc;
}

sptr<AudioRendererFilter> CreateAudioRendererFilter(FuzzedDataProvider& fdp)
{
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    if (filter == nullptr) {
        return nullptr;
    }
    filter->uid = fdp.ConsumeIntegral<int32_t>();
    filter->streamType = PickValue(fdp, g_testVolumeTypes);
    return filter;
}

AudioRendererInfo CreateAudioRendererInfo(FuzzedDataProvider& fdp)
{
    AudioRendererInfo rendererInfo;
    rendererInfo.contentType = static_cast<ContentType>(fdp.ConsumeIntegral<int32_t>() % 10);
    rendererInfo.streamUsage = static_cast<StreamUsage>(fdp.ConsumeIntegral<int32_t>() % 20);
    rendererInfo.rendererFlags = fdp.ConsumeIntegral<int32_t>();
    return rendererInfo;
}

AudioCapturerInfo CreateAudioCapturerInfo(FuzzedDataProvider& fdp)
{
    AudioCapturerInfo capturerInfo;
    capturerInfo.sourceType = static_cast<SourceType>(fdp.ConsumeIntegral<int32_t>() % 10);
    capturerInfo.capturerFlags = fdp.ConsumeIntegral<int32_t>();
    return capturerInfo;
}

void AudioGeneralManagerGetInstanceFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    AudioGeneralManager* instance = AudioGeneralManager::GetInstance();
    if (instance != nullptr) {
        instance->GetCallingPid();
    }
}

void AudioGeneralManagerSetAudioDeviceRefinerCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioDeviceRefinerFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetAudioDeviceRefinerCallback(callback);
}

void AudioGeneralManagerSetAudioClientInfoMgrCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioClientInfoMgrCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetAudioClientInfoMgrCallback(callback);
}

void AudioGeneralManagerGetPreferredOutputDeviceForRendererInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRendererInfo rendererInfo = CreateAudioRendererInfo(fdp);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AudioGeneralManager::GetInstance()->GetPreferredOutputDeviceForRendererInfo(rendererInfo, desc);
}

void AudioGeneralManagerUnsetAudioDeviceRefinerCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    AudioGeneralManager::GetInstance()->UnsetAudioDeviceRefinerCallback();
}

void AudioGeneralManagerSetDeviceVolumeBehaviorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string networkId = "LocalNetworkId";
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    VolumeBehavior volumeBehavior = PickValue(fdp, g_testVolumeBehaviors);
    AudioGeneralManager::GetInstance()->SetDeviceVolumeBehavior(networkId, deviceType, volumeBehavior);
}

void AudioGeneralManagerTriggerFetchDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamDeviceChangeReasonExt reason = PickValue(fdp, g_testReasonExts);
    AudioGeneralManager::GetInstance()->TriggerFetchDevice(reason);
}

void AudioGeneralManagerSetPreferredDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    PreferredType preferredType = PickValue(fdp, g_testPreferredTypes);
    auto desc = CreateAudioDeviceDescriptor(fdp);
    CHECK_AND_RETURN(desc != nullptr);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioGeneralManager::GetInstance()->SetPreferredDevice(preferredType, desc, uid);
}

void AudioGeneralManagerSetAsrVoiceMuteModeFuzzTest(FuzzedDataProvider& fdp)
{
    AsrVoiceMuteMode asrVoiceMuteMode = PickValue(fdp, g_testAsrVoiceMuteModes);
    bool on = fdp.ConsumeBool();
    AudioGeneralManager::GetInstance()->SetAsrVoiceMuteMode(asrVoiceMuteMode, on);
}

void AudioGeneralManagerSetPreferredOutputDeviceChangeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioRendererInfo rendererInfo = CreateAudioRendererInfo(fdp);
    auto callback = std::make_shared<AudioPreferredOutputDeviceChangeCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetPreferredOutputDeviceChangeCallback(rendererInfo, callback);
}

void AudioGeneralManagerRegisterFocusInfoChangeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioFocusInfoChangeCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->RegisterFocusInfoChangeCallback(callback);
}

void AudioGeneralManagerGetDevicesInnerFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceFlag flag = PickValue(fdp, g_testDeviceFlags);
    AudioGeneralManager::GetInstance()->GetDevicesInner(flag);
}

void AudioGeneralManagerSetDeviceChangeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceFlag flag = PickValue(fdp, g_testDeviceFlags);
    auto callback = std::make_shared<AudioManagerDeviceChangeCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetDeviceChangeCallback(flag, callback);
}

void AudioGeneralManagerSetDeviceInfoUpdateCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioManagerDeviceInfoUpdateCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetDeviceInfoUpdateCallback(callback);
}

void AudioGeneralManagerSetQueryClientTypeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioQueryClientTypeCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetQueryClientTypeCallback(callback);
}

void AudioGeneralManagerSetQueryDeviceVolumeBehaviorCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioQueryDeviceVolumeBehaviorCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetQueryDeviceVolumeBehaviorCallback(callback);
}

void AudioGeneralManagerGetAudioGeneralManagerProxyFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    AudioGeneralManager::GetInstance()->GetAudioGeneralManagerProxy();
}

void AudioGeneralManagerSetExtraParametersFuzzTest(FuzzedDataProvider& fdp)
{
    std::string key = fdp.ConsumeRandomLengthString(32);
    std::vector<std::pair<std::string, std::string>> kvpairs;
    int32_t pairCount = fdp.ConsumeIntegralInRange<int32_t>(0, 5);
    for (int32_t i = 0; i < pairCount; i++) {
        kvpairs.push_back({fdp.ConsumeRandomLengthString(16), fdp.ConsumeRandomLengthString(16)});
    }
    AudioGeneralManager::GetInstance()->SetExtraParameters(key, kvpairs);
}

void AudioGeneralManagerGetVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeType volumeType = PickValue(fdp, g_testVolumeTypes);
    AudioGeneralManager::GetInstance()->GetVolume(volumeType);
}

void AudioGeneralManagerUnregisterVolumeKeyEventCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t clientPid = fdp.ConsumeIntegral<int32_t>();
    auto callback = std::make_shared<VolumeKeyEventCallbackFuzzTest>();
    AudioGeneralManager::GetInstance()->UnregisterVolumeKeyEventCallback(clientPid, callback);
}

void AudioGeneralManagerGetActiveOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    AudioGeneralManager::GetInstance()->GetActiveOutputDevice();
}

void AudioGeneralManagerGetAudioSceneFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    AudioGeneralManager::GetInstance()->GetAudioScene();
}

void AudioGeneralManagerSetAudioSceneChangeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioManagerAudioSceneChangedCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetAudioSceneChangeCallback(callback);
}

void AudioGeneralManagerGetMaxVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioVolumeType volumeType = PickValue(fdp, g_testVolumeTypes);
    AudioGeneralManager::GetInstance()->GetMaxVolume(volumeType);
}

void AudioGeneralManagerUnregisterFocusInfoChangeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioFocusInfoChangeCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->UnregisterFocusInfoChangeCallback(callback);
}

void AudioGeneralManagerGetAudioFocusInfoListFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    AudioGeneralManager::GetInstance()->GetAudioFocusInfoList(focusInfoList);
}

void AudioGeneralManagerSelectOutputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto desc = CreateAudioDeviceDescriptor(fdp);
    CHECK_AND_RETURN(desc != nullptr);
    if (desc->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors;
        descriptors.push_back(desc);
        AudioGeneralManager::GetInstance()->SelectOutputDevice(descriptors);
    }
}

void AudioGeneralManagerRegisterVolumeKeyEventCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t clientPid = fdp.ConsumeIntegral<int32_t>();
    auto callback = std::make_shared<VolumeKeyEventCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    API_VERSION api_v = API_VERSION::API_9;
    AudioGeneralManager::GetInstance()->RegisterVolumeKeyEventCallback(clientPid, callback, api_v);
}

void AudioGeneralManagerRegisterAudioCapturerEventListenerFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t clientPid = fdp.ConsumeIntegral<int32_t>();
    auto callback = std::make_shared<AudioCapturerStateChangeCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->RegisterAudioCapturerEventListener(clientPid, callback);
}

void AudioGeneralManagerGetCurrentRendererChangeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    AudioGeneralManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

void AudioGeneralManagerRegisterAudioRendererEventListenerFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<AudioRendererStateChangeCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->RegisterAudioRendererEventListener(callback);
}

void AudioGeneralManagerGetPreferredInputDeviceForCapturerInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerInfo captureInfo = CreateAudioCapturerInfo(fdp);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AudioGeneralManager::GetInstance()->GetPreferredInputDeviceForCapturerInfo(captureInfo, desc);
}

void AudioGeneralManagerSetPreferredInputDeviceChangeCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCapturerInfo capturerInfo = CreateAudioCapturerInfo(fdp);
    auto callback = std::make_shared<AudioPreferredInputDeviceChangeCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetPreferredInputDeviceChangeCallback(capturerInfo, callback);
}

void AudioGeneralManagerGetCurrentCapturerChangeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    AudioGeneralManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
}

void AudioGeneralManagerSetDeviceConnectionStatusFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceDesc = CreateAudioDeviceDescriptor(fdp);
    CHECK_AND_RETURN(deviceDesc != nullptr);
    bool isConnected = fdp.ConsumeBool();
    AudioGeneralManager::GetInstance()->SetDeviceConnectionStatus(deviceDesc, isConnected);
}

void AudioGeneralManagerUpdateDeviceInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto deviceDesc = CreateAudioDeviceDescriptor(fdp);
    CHECK_AND_RETURN(deviceDesc != nullptr);
    DeviceInfoUpdateCommand command = PickValue(fdp, g_testDeviceInfoUpdateCommands);
    AudioGeneralManager::GetInstance()->UpdateDeviceInfo(deviceDesc, command);
}

void AudioGeneralManagerSelectOutputDeviceWithFilterFuzzTest(FuzzedDataProvider& fdp)
{
    auto filter = CreateAudioRendererFilter(fdp);
    CHECK_AND_RETURN(filter != nullptr);
    auto desc = CreateAudioDeviceDescriptor(fdp);
    CHECK_AND_RETURN(desc != nullptr);
    if (desc->deviceRole_ == DeviceRole::OUTPUT_DEVICE) {
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors;
        descriptors.push_back(desc);
        AudioGeneralManager::GetInstance()->SelectOutputDevice(filter, descriptors);
    }
}

void AudioGeneralManagerSelectPrivateDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType devType = PickValue(fdp, g_testDeviceTypes);
    std::string macAddress = fdp.ConsumeRandomLengthString(32);
    AudioGeneralManager::GetInstance()->SelectPrivateDevice(devType, macAddress);
}

void AudioGeneralManagerForceSelectDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    DeviceType devType = PickValue(fdp, g_testDeviceTypes);
    std::string macAddress = fdp.ConsumeRandomLengthString(32);
    auto filter = CreateAudioRendererFilter(fdp);
    AudioGeneralManager::GetInstance()->ForceSelectDevice(devType, macAddress, filter);
}

void AudioGeneralManagerSetActiveHfpDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string macAddress = fdp.ConsumeRandomLengthString(32);
    AudioGeneralManager::GetInstance()->SetActiveHfpDevice(macAddress);
}

void AudioGeneralManagerSetSleAudioOperationCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<SleAudioOperationCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->SetSleAudioOperationCallback(callback);
}

void AudioGeneralManagerRestoreDistributedDeviceInfoFuzzTest(FuzzedDataProvider& fdp)
{
    (void)fdp;
    AudioGeneralManager::GetInstance()->RestoreDistributedDeviceInfo();
}

void AudioGeneralManagerRegisterPreferredDeviceSetCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<PreferredDeviceSetCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->RegisterPreferredDeviceSetCallback(callback);
}

void AudioGeneralManagerUnregisterPreferredDeviceSetCallbackFuzzTest(FuzzedDataProvider& fdp)
{
    auto callback = std::make_shared<PreferredDeviceSetCallbackFuzzTest>();
    CHECK_AND_RETURN(callback != nullptr);
    AudioGeneralManager::GetInstance()->UnregisterPreferredDeviceSetCallback(callback);
}

void AudioGeneralManagerNotifySensitiveRecordPermitFuzzTest(FuzzedDataProvider& fdp)
{
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    bool isPermitted = fdp.ConsumeBool();
    AudioGeneralManager::GetInstance()->NotifySensitiveRecordPermit(sessionId, isPermitted);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
        AudioGeneralManagerGetInstanceFuzzTest,
        AudioGeneralManagerSetAudioDeviceRefinerCallbackFuzzTest,
        AudioGeneralManagerSetAudioClientInfoMgrCallbackFuzzTest,
        AudioGeneralManagerGetPreferredOutputDeviceForRendererInfoFuzzTest,
        AudioGeneralManagerUnsetAudioDeviceRefinerCallbackFuzzTest,
        AudioGeneralManagerSetDeviceVolumeBehaviorFuzzTest,
        AudioGeneralManagerTriggerFetchDeviceFuzzTest,
        AudioGeneralManagerSetPreferredDeviceFuzzTest,
        AudioGeneralManagerSetAsrVoiceMuteModeFuzzTest,
        AudioGeneralManagerSetPreferredOutputDeviceChangeCallbackFuzzTest,
        AudioGeneralManagerRegisterFocusInfoChangeCallbackFuzzTest,
        AudioGeneralManagerGetDevicesInnerFuzzTest,
        AudioGeneralManagerSetDeviceChangeCallbackFuzzTest,
        AudioGeneralManagerSetDeviceInfoUpdateCallbackFuzzTest,
        AudioGeneralManagerSetQueryClientTypeCallbackFuzzTest,
        AudioGeneralManagerSetQueryDeviceVolumeBehaviorCallbackFuzzTest,
        AudioGeneralManagerGetAudioGeneralManagerProxyFuzzTest,
        AudioGeneralManagerSetExtraParametersFuzzTest,
        AudioGeneralManagerGetVolumeFuzzTest,
        AudioGeneralManagerUnregisterVolumeKeyEventCallbackFuzzTest,
        AudioGeneralManagerGetActiveOutputDeviceFuzzTest,
        AudioGeneralManagerGetAudioSceneFuzzTest,
        AudioGeneralManagerSetAudioSceneChangeCallbackFuzzTest,
        AudioGeneralManagerGetMaxVolumeFuzzTest,
        AudioGeneralManagerUnregisterFocusInfoChangeCallbackFuzzTest,
        AudioGeneralManagerGetAudioFocusInfoListFuzzTest,
        AudioGeneralManagerSelectOutputDeviceFuzzTest,
        AudioGeneralManagerRegisterVolumeKeyEventCallbackFuzzTest,
        AudioGeneralManagerRegisterAudioCapturerEventListenerFuzzTest,
        AudioGeneralManagerGetCurrentRendererChangeInfosFuzzTest,
        AudioGeneralManagerRegisterAudioRendererEventListenerFuzzTest,
        AudioGeneralManagerGetPreferredInputDeviceForCapturerInfoFuzzTest,
        AudioGeneralManagerSetPreferredInputDeviceChangeCallbackFuzzTest,
        AudioGeneralManagerGetCurrentCapturerChangeInfosFuzzTest,
        AudioGeneralManagerSetDeviceConnectionStatusFuzzTest,
        AudioGeneralManagerUpdateDeviceInfoFuzzTest,
        AudioGeneralManagerSelectOutputDeviceWithFilterFuzzTest,
        AudioGeneralManagerSelectPrivateDeviceFuzzTest,
        AudioGeneralManagerForceSelectDeviceFuzzTest,
        AudioGeneralManagerSetActiveHfpDeviceFuzzTest,
        AudioGeneralManagerSetSleAudioOperationCallbackFuzzTest,
        AudioGeneralManagerRestoreDistributedDeviceInfoFuzzTest,
        AudioGeneralManagerRegisterPreferredDeviceSetCallbackFuzzTest,
        AudioGeneralManagerUnregisterPreferredDeviceSetCallbackFuzzTest,
        AudioGeneralManagerNotifySensitiveRecordPermitFuzzTest,
    });
    func(fdp);
}

void Init()
{
}

} // namespace AudioStandard
} // namespace OHOS

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
