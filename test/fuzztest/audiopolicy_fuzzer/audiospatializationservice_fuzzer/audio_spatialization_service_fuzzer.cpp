/*
* Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_spatialization_service.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
static int32_t NUM_3 = 3;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
typedef void (*TestPtr)(FuzzedDataProvider&);

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

const vector<AudioSpatializationSceneType> g_testSpatializationSceneTypes = {
    SPATIALIZATION_SCENE_TYPE_DEFAULT,
    SPATIALIZATION_SCENE_TYPE_MUSIC,
    SPATIALIZATION_SCENE_TYPE_MOVIE,
    SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
    SPATIALIZATION_SCENE_TYPE_MAX,
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

const vector<AudioSpatialDeviceType> g_testSpatialDeviceTypes = {
    EARPHONE_TYPE_NONE,
    EARPHONE_TYPE_INEAR,
    EARPHONE_TYPE_HALF_INEAR,
    EARPHONE_TYPE_HEADPHONE,
    EARPHONE_TYPE_GLASSES,
    EARPHONE_TYPE_OTHERS,
};

const vector<RendererState> g_testRendererStates = {
    RENDERER_NEW,
    RENDERER_PREPARED,
    RENDERER_RUNNING,
    RENDERER_STOPPED,
    RENDERER_RELEASED,
    RENDERER_PAUSED,
};

const vector<AudioSpatializationService::WriteToDbOperation> g_testWriteToDbOperations = {
    AudioSpatializationService::WriteToDbOperation::WRITE_SPATIALIZATION_STATE,
    AudioSpatializationService::WriteToDbOperation::WRITE_SPATIALIZATION_SCENE,
    AudioSpatializationService::WriteToDbOperation::WRITE_ALLDEVICESPATIAL_INFO,
};

const vector<DeviceType> g_testOutputDeviceTypes = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_NEARLINK,
};

const vector<DeviceCategory> g_testDeviceCategories = {
    CATEGORY_DEFAULT,
    BT_HEADPHONE,
    BT_SOUNDBOX,
    BT_CAR,
    BT_GLASSES,
    BT_WATCH,
    BT_HEARAID,
    BT_UNWEAR_HEADPHONE,
};

const vector<std::string> g_testSpatializationLabels = {
    "SPATIALIZATION_AND_HEADTRACKING",
    "SPATIALIZATION",
    "HEADTRACKING",
    "PERSONALIZED_SPATIALIZATION_AND_HEADTRACKING",
    "PERSONALIZED_SPATIALIZATION",
};

std::shared_ptr<AudioDeviceDescriptor> MakeDeviceDescriptor(FuzzedDataProvider& fdp, DeviceRole role)
{
    const auto& deviceTypes = (role == INPUT_DEVICE) ? g_testOutputDeviceTypes : g_testOutputDeviceTypes;
    if (deviceTypes.size() == 0 || g_testDeviceCategories.size() == 0) {
        return std::make_shared<AudioDeviceDescriptor>();
    }
    auto desc = std::make_shared<AudioDeviceDescriptor>(PickValue(fdp, deviceTypes), role);
    desc->networkId_ = ConsumeString(fdp, LOCAL_NETWORK_ID);
    desc->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    desc->deviceName_ = ConsumeString(fdp, "audio_fuzz_device");
    desc->deviceCategory_ = PickValue(fdp, g_testDeviceCategories);
    desc->deviceId_ = fdp.ConsumeIntegral<int32_t>();
    return desc;
}

void ResetSpatializationServiceState(AudioSpatializationService& service)
{
    service.addressToDeviceSpatialInfoMap_.clear();
    service.addressToSpatialEnabledMap_.clear();
    service.addressToSpatialDeviceStateMap_.clear();
    service.currentDevice_.macAddress_ = "";
    service.preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    service.spatializationEnabledReal_ = false;
    service.headTrackingEnabledReal_ = false;
    service.isHeadTrackingDataRequested_ = false;
    service.spatializationSceneType_ = SPATIALIZATION_SCENE_TYPE_MUSIC;
    service.spatializationStateFlag_ = {};
    service.currSpatialDeviceType_ = EARPHONE_TYPE_OTHERS;
}

void UpdateCurrentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    AudioDeviceDescriptor selectedAudioDevice;
    selectedAudioDevice.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    service.UpdateCurrentDevice(selectedAudioDevice);
    service.UpdateCurrentDevice(selectedAudioDevice);
    selectedAudioDevice.macAddress_ = "";
    service.UpdateCurrentDevice(selectedAudioDevice);
    std::string spatialDeviceAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    selectedAudioDevice.macAddress_ = spatialDeviceAddress;
    AudioSpatialDeviceType audioSpatialDeviceType = PickValue(fdp, g_testSpatialDeviceTypes);
    service.addressToSpatialDeviceStateMap_[service.GetSha256EncryptAddress(spatialDeviceAddress)] = {
        spatialDeviceAddress,
        fdp.ConsumeBool(),
        fdp.ConsumeBool(),
        audioSpatialDeviceType
    };
    service.UpdateCurrentDevice(selectedAudioDevice);
    selectedAudioDevice.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    service.UpdateCurrentDevice(selectedAudioDevice);
}

void RemoveOldestDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.addressToDeviceSpatialInfoMap_ = {
        {"device1", "info1|1000"},
        {"device2", "info2|2000"},
        {"device3", "info3|1500"}
    };
    service.addressToSpatialEnabledMap_ = {
        {"device1", AudioSpatializationState{fdp.ConsumeBool(), fdp.ConsumeBool()}},
        {"device2", AudioSpatializationState{fdp.ConsumeBool(), fdp.ConsumeBool()}},
        {"device3", AudioSpatializationState{fdp.ConsumeBool(), fdp.ConsumeBool()}}
    };
    AudioSpatialDeviceType audioSpatialDeviceType = PickValue(fdp, g_testSpatialDeviceTypes);
    AudioSpatialDeviceType audioSpatialDeviceType1 = PickValue(fdp, g_testSpatialDeviceTypes);
    service.addressToSpatialDeviceStateMap_ = {
        {"device1", AudioSpatialDeviceState{"device1", fdp.ConsumeBool(),
                                            fdp.ConsumeBool(), audioSpatialDeviceType}},
        {"device2", AudioSpatialDeviceState{"device2", fdp.ConsumeBool(),
                                            fdp.ConsumeBool(), audioSpatialDeviceType1}},
        {"device3", AudioSpatialDeviceState{"device3", fdp.ConsumeBool(),
                                            fdp.ConsumeBool(), audioSpatialDeviceType}}
    };
    service.RemoveOldestDevice();
}

void UpdateDeviceSpatialMapInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    for (uint32_t i = 1; i <= AudioSpatializationService::MAX_DEVICE_NUM; ++i) {
        std::string device = "device" + std::to_string(i);
        std::string info = "info" + std::to_string(i);
        service.UpdateDeviceSpatialMapInfo(device, info);
        service.GetSha256EncryptAddress(device);
    }
    uint32_t exitNum = fdp.ConsumeIntegralInRange<uint32_t>(0, AudioSpatializationService::MAX_DEVICE_NUM);
    service.UpdateDeviceSpatialMapInfo("device" + to_string(exitNum), "info" + to_string(exitNum));
    service.UpdateDeviceSpatialMapInfo("device" + to_string(exitNum + AudioSpatializationService::MAX_DEVICE_NUM),
                                       "info" + to_string(exitNum + AudioSpatializationService::MAX_DEVICE_NUM));
}

void WriteSpatializationStateToDbFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    AudioSpatializationSceneType testScene = PickValue(fdp, g_testSpatializationSceneTypes);
    service.spatializationSceneType_ = testScene;
    AudioSpatializationService::WriteToDbOperation operation = PickValue(fdp, g_testWriteToDbOperations);
    service.WriteSpatializationStateToDb(operation);
}

void GetSpatializationStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.spatializationEnabledReal_ = fdp.ConsumeBool();
    service.headTrackingEnabledReal_ = fdp.ConsumeBool();
    StreamUsage supportedUsage = PickValue(fdp, g_testStreamUsages);
    service.GetSpatializationState(supportedUsage);
}

void SetHeadTrackingEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    bool enable = fdp.ConsumeBool();
    service.SetHeadTrackingEnabled(enable);
}

void AudioSpatializationServiceInitFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    vector<EffectChain> effectChains = {
        {"EFFECTCHAIN_BT_MUSIC", {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")},
            PickValue(fdp, g_testSpatializationLabels)},
        {"Effect1", {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")},
            PickValue(fdp, g_testSpatializationLabels)},
        {"Effect2", {ConsumeString(fdp, "apply2_1")}, PickValue(fdp, g_testSpatializationLabels)},
        {"Effect3", {}, PickValue(fdp, g_testSpatializationLabels)}
    };
    service.Init(effectChains);
}

void IsSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    vector<string> preSettingSpatialAddressList = { "NO_PREVIOUS_SET_DEVICE", "1234", "test_address"};
    service.preSettingSpatialAddress_ = PickValue(fdp, preSettingSpatialAddressList);
    service.IsSpatializationEnabled();
}

void SetSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const bool enable = fdp.ConsumeBool();
    service.preSettingSpatialAddress_ = ConsumeString(fdp, "NO_PREVIOUS_SET_DEVICE");
    service.spatializationStateFlag_.spatializationEnabled = fdp.ConsumeBool();
    service.SetSpatializationEnabled(enable);
}

void IsHeadTrackingEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    vector<string> preSettingSpatialAddressList = { "NO_PREVIOUS_SET_DEVICE", "1234"};
    service.preSettingSpatialAddress_ = PickValue(fdp, preSettingSpatialAddressList);
    service.IsHeadTrackingEnabled();
}

void HandleHeadTrackingDeviceChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::unordered_map<std::string, bool> changeInfo;
    changeInfo.insert({ConsumeString(fdp, "abc"), fdp.ConsumeBool()});
    service.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    service.HandleHeadTrackingDeviceChange(changeInfo);
}

void AudioSpatializationServiceIsSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::string address = ConsumeString(fdp, "test_address");
    service.IsSpatializationEnabled(address);
}

void AudioSpatializationServiceIsSpatializationEnabledForCurrentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.currentDevice_.macAddress_ = ConsumeString(fdp, "test_address");
    service.IsSpatializationEnabledForCurrentDevice();
}

void AudioSpatializationServiceSetSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    service.SetSpatializationEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceIsHeadTrackingEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    AudioSpatializationState spatializationState;
    std::string address = ConsumeString(fdp, "test_address");
    service.addressToSpatialEnabledMap_.insert({address, spatializationState});
    service.IsHeadTrackingEnabled(address);
}

void AudioSpatializationServiceSetHeadTrackingEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    service.SetHeadTrackingEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceHandleSpatializationEnabledChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const bool enable = fdp.ConsumeBool();
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    service.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    service.HandleSpatializationEnabledChange(selectedAudioDevice, enable);
}

void AudioSpatializationServiceHandleSpatializationEnabledChangeForCurrentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const bool enable = fdp.ConsumeBool();
    service.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    service.HandleSpatializationEnabledChangeForCurrentDevice(enable);
}

void AudioSpatializationServiceHandleHeadTrackingEnabledChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const bool enable = fdp.ConsumeBool();
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    service.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    service.HandleHeadTrackingEnabledChange(selectedAudioDevice, enable);
}

void AudioSpatializationServiceIsHeadTrackingSupportedForDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const std::string address = ConsumeString(fdp, "1234");
    service.IsHeadTrackingSupportedForDevice(address);
}

void AudioSpatializationServiceUpdateSpatialDeviceStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const AudioSpatialDeviceState audioSpatialDeviceState = {
        ConsumeString(fdp, "1234"),
        fdp.ConsumeBool(),
        fdp.ConsumeBool(),
        PickValue(fdp, g_testSpatialDeviceTypes),
    };
    service.UpdateSpatialDeviceState(audioSpatialDeviceState);
}

void AudioSpatializationServiceGetSpatializationSceneTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.spatializationSceneType_ = PickValue(fdp, g_testSpatializationSceneTypes);
    service.GetSpatializationSceneType();
}

void AudioSpatializationServiceSetSpatializationSceneTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    AudioSpatializationSceneType spatializationSceneType = PickValue(fdp, g_testSpatializationSceneTypes);
    service.SetSpatializationSceneType(spatializationSceneType);
}

void AudioSpatializationServiceDeinitFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    vector<EffectChain> effectChains = {
        {"EFFECTCHAIN_BT_MUSIC", {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")},
            PickValue(fdp, g_testSpatializationLabels)},
        {"Effect1", {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")},
            PickValue(fdp, g_testSpatializationLabels)},
        {"Effect2", {ConsumeString(fdp, "apply2_1")}, PickValue(fdp, g_testSpatializationLabels)},
        {"Effect3", {}, PickValue(fdp, g_testSpatializationLabels)}
    };
    service.Init(effectChains);
    service.Deinit();
}

void AudioSpatializationServiceGetAudioServerProxyFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    vector<EffectChain> effectChains = {
        {"EFFECTCHAIN_BT_MUSIC", {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")},
            PickValue(fdp, g_testSpatializationLabels)},
        {"Effect1", {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")},
            PickValue(fdp, g_testSpatializationLabels)},
        {"Effect2", {ConsumeString(fdp, "apply2_1")}, PickValue(fdp, g_testSpatializationLabels)},
        {"Effect3", {}, PickValue(fdp, g_testSpatializationLabels)}
    };
    service.Init(effectChains);
    service.GetAudioServerProxy();
}

void AudioSpatializationServiceIsSpatializationSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.spatializationEnabledReal_ = fdp.ConsumeBool();
    service.IsSpatializationSupported();
}

void AudioSpatializationServiceIsHeadTrackingSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.headTrackingEnabledReal_ = fdp.ConsumeBool();
    service.IsHeadTrackingSupported();
}

void AudioSpatializationServiceIsSpatializationSupportedForDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::string address = ConsumeString(fdp, "test_address");
    service.IsSpatializationSupportedForDevice(address);
}

void AudioSpatializationServiceIsHeadTrackingDataRequestedFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    service.currentDevice_.macAddress_ = macAddress;
    service.isHeadTrackingDataRequested_ = fdp.ConsumeBool();
    service.IsHeadTrackingDataRequested(macAddress);
}

void AudioSpatializationServiceUpdateRendererInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfo;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    if (changeInfo != nullptr) {
        changeInfo->rendererState = PickValue(fdp, g_testRendererStates);
        changeInfo->outputDeviceInfo.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
        changeInfo->rendererInfo.streamUsage = PickValue(fdp, g_testStreamUsages);
        rendererChangeInfo.push_back(changeInfo);
    }
    service.UpdateRendererInfo(rendererChangeInfo);
}

void AudioSpatializationServiceInitSpatializationStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.spatializationEnabledReal_ = fdp.ConsumeBool();
    service.InitSpatializationState();
}

void AudioSpatializationServiceGetCurrentDeviceAddressFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.currentDevice_.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    service.GetCurrentDeviceAddress();
}

void AudioSpatializationServiceUpdateSpatializationSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::string encryptedAddress = ConsumeString(fdp, "encrypted_test_address");
    service.UpdateSpatializationSupported(encryptedAddress);
}

void AudioSpatializationServiceHandleDeviceConfigChangedFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice =
        MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    service.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    service.HandleDeviceConfigChanged(selectedAudioDevice);
}

void AudioSpatializationServiceHandleSpatializationEnabledChangeBoolFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const bool enable = fdp.ConsumeBool();
    service.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    service.HandleSpatializationEnabledChange(enable);
}

void AudioSpatializationServiceHandleHeadTrackingEnabledChangeBoolFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const bool enable = fdp.ConsumeBool();
    service.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    service.HandleHeadTrackingEnabledChange(enable);
}

void AudioSpatializationServiceGetSpatializationStateDefaultFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    service.spatializationEnabledReal_ = fdp.ConsumeBool();
    service.headTrackingEnabledReal_ = fdp.ConsumeBool();
    service.GetSpatializationState();
}

void AudioSpatializationServiceUpdateSpatialDeviceStateMultipleFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    for (size_t i = 0; i < g_testSpatialDeviceTypes.size(); ++i) {
        const AudioSpatialDeviceState audioSpatialDeviceState = {
            ConsumeString(fdp, "device_" + std::to_string(i)),
            fdp.ConsumeBool(),
            fdp.ConsumeBool(),
            g_testSpatialDeviceTypes[i],
        };
        service.UpdateSpatialDeviceState(audioSpatialDeviceState);
    }
}

void AudioSpatializationServiceSetSpatializationEnabledWithStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice =
        MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    service.spatializationStateFlag_.spatializationEnabled = !enable;
    service.SetSpatializationEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceSetHeadTrackingEnabledWithStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice =
        MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    service.spatializationStateFlag_.headTrackingEnabled = !enable;
    service.SetHeadTrackingEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceUpdateRendererInfoMultipleFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfo;
    for (size_t i = 0; i < NUM_3; ++i) {
        auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
        if (changeInfo != nullptr) {
            changeInfo->rendererState = PickValue(fdp, g_testRendererStates);
            changeInfo->outputDeviceInfo.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
            changeInfo->rendererInfo.streamUsage = PickValue(fdp, g_testStreamUsages);
            rendererChangeInfo.push_back(changeInfo);
        }
    }
    service.UpdateRendererInfo(rendererChangeInfo);
}

void AudioSpatializationServiceIsHeadTrackingDataRequestedDifferentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    service.currentDevice_.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    service.isHeadTrackingDataRequested_ = fdp.ConsumeBool();
    service.IsHeadTrackingDataRequested(macAddress);
}

void AudioSpatializationServiceSetSpatializationSceneTypeAllTypesFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    AudioSpatializationSceneType sceneType = PickValue(fdp, g_testSpatializationSceneTypes);
    service.SetSpatializationSceneType(sceneType);
}

void AudioSpatializationServiceIsPersonalizedSpatializationSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    vector<EffectChain> effectChains = {
        {"EFFECTCHAIN_BT_MUSIC", {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")},
            PickValue(fdp, g_testSpatializationLabels)},
        {"Effect1", {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")},
            PickValue(fdp, g_testSpatializationLabels)},
        {"Effect2", {ConsumeString(fdp, "apply2_1")}, PickValue(fdp, g_testSpatializationLabels)},
        {"Effect3", {}, PickValue(fdp, g_testSpatializationLabels)}
    };
    service.Init(effectChains);
    service.IsPersonalizedSpatializationSupported();
}

void AudioSpatializationServiceIsPersonalizedSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    std::string address = ConsumeString(fdp, "test_address");
    service.IsPersonalizedSpatializationEnabled(address);
}

void AudioSpatializationServiceSetPersonalizedSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice =
        MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    service.SetPersonalizedSpatializationEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceHandlePersonalizedSpatializationEnabledChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const bool enable = fdp.ConsumeBool();
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice =
        MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    service.audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    service.HandlePersonalizedSpatializationEnabledChange(selectedAudioDevice, enable);
}

void AudioSpatializationServiceSetPersonalizedSpatializationEnabledWithStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto& service = AudioSpatializationService::GetAudioSpatializationService();
    ResetSpatializationServiceState(service);
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice =
        MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    service.spatializationStateFlag_.personalizedSpatializationEnabled = !enable;
    service.SetPersonalizedSpatializationEnabled(selectedAudioDevice, enable);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    UpdateCurrentDeviceFuzzTest,
    RemoveOldestDeviceFuzzTest,
    UpdateDeviceSpatialMapInfoFuzzTest,
    WriteSpatializationStateToDbFuzzTest,
    GetSpatializationStateFuzzTest,
    SetHeadTrackingEnabledFuzzTest,
    AudioSpatializationServiceInitFuzzTest,
    IsSpatializationEnabledFuzzTest,
    SetSpatializationEnabledFuzzTest,
    IsHeadTrackingEnabledFuzzTest,
    HandleHeadTrackingDeviceChangeFuzzTest,
    AudioSpatializationServiceIsSpatializationEnabledFuzzTest,
    AudioSpatializationServiceIsSpatializationEnabledForCurrentDeviceFuzzTest,
    AudioSpatializationServiceSetSpatializationEnabledFuzzTest,
    AudioSpatializationServiceIsHeadTrackingEnabledFuzzTest,
    AudioSpatializationServiceSetHeadTrackingEnabledFuzzTest,
    AudioSpatializationServiceHandleSpatializationEnabledChangeFuzzTest,
    AudioSpatializationServiceHandleSpatializationEnabledChangeForCurrentDeviceFuzzTest,
    AudioSpatializationServiceHandleHeadTrackingEnabledChangeFuzzTest,
    AudioSpatializationServiceIsHeadTrackingSupportedForDeviceFuzzTest,
    AudioSpatializationServiceUpdateSpatialDeviceStateFuzzTest,
    AudioSpatializationServiceGetSpatializationSceneTypeFuzzTest,
    AudioSpatializationServiceSetSpatializationSceneTypeFuzzTest,
    AudioSpatializationServiceDeinitFuzzTest,
    AudioSpatializationServiceGetAudioServerProxyFuzzTest,
    AudioSpatializationServiceIsSpatializationSupportedFuzzTest,
    AudioSpatializationServiceIsHeadTrackingSupportedFuzzTest,
    AudioSpatializationServiceIsSpatializationSupportedForDeviceFuzzTest,
    AudioSpatializationServiceIsHeadTrackingDataRequestedFuzzTest,
    AudioSpatializationServiceUpdateRendererInfoFuzzTest,
    AudioSpatializationServiceInitSpatializationStateFuzzTest,
    AudioSpatializationServiceGetCurrentDeviceAddressFuzzTest,
    AudioSpatializationServiceUpdateSpatializationSupportedFuzzTest,
    AudioSpatializationServiceHandleDeviceConfigChangedFuzzTest,
    AudioSpatializationServiceHandleSpatializationEnabledChangeBoolFuzzTest,
    AudioSpatializationServiceHandleHeadTrackingEnabledChangeBoolFuzzTest,
    AudioSpatializationServiceGetSpatializationStateDefaultFuzzTest,
    AudioSpatializationServiceUpdateSpatialDeviceStateMultipleFuzzTest,
    AudioSpatializationServiceSetSpatializationEnabledWithStateFuzzTest,
    AudioSpatializationServiceSetHeadTrackingEnabledWithStateFuzzTest,
    AudioSpatializationServiceUpdateRendererInfoMultipleFuzzTest,
    AudioSpatializationServiceIsHeadTrackingDataRequestedDifferentDeviceFuzzTest,
    AudioSpatializationServiceSetSpatializationSceneTypeAllTypesFuzzTest,
    AudioSpatializationServiceIsPersonalizedSpatializationSupportedFuzzTest,
    AudioSpatializationServiceIsPersonalizedSpatializationEnabledFuzzTest,
    AudioSpatializationServiceSetPersonalizedSpatializationEnabledFuzzTest,
    AudioSpatializationServiceHandlePersonalizedSpatializationEnabledChangeFuzzTest,
    AudioSpatializationServiceSetPersonalizedSpatializationEnabledWithStateFuzzTest,
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
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}
