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

void UpdateCurrentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSpatializationService service;
    std::string newMacAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    service.UpdateCurrentDevice(newMacAddress);
    service.UpdateCurrentDevice(newMacAddress);
    service.UpdateCurrentDevice("");
    std::string spatialDeviceAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    AudioSpatialDeviceType audioSpatialDeviceType = PickValue(fdp, g_testSpatialDeviceTypes);
    service.addressToSpatialDeviceStateMap_[service.GetSha256EncryptAddress(spatialDeviceAddress)] = {
        spatialDeviceAddress,
        fdp.ConsumeBool(),
        fdp.ConsumeBool(),
        audioSpatialDeviceType
    };
    service.UpdateCurrentDevice(spatialDeviceAddress);
    std::string nonSpatialDeviceAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    service.UpdateCurrentDevice(nonSpatialDeviceAddress);
}

void RemoveOldestDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSpatializationService service;
    std::string device1 = ConsumeString(fdp, "device1");
    std::string device2 = ConsumeString(fdp, "device2");
    std::string device3 = ConsumeString(fdp, "device3");
    service.addressToDeviceSpatialInfoMap_ = {
        {device1, ConsumeString(fdp, "info1") + "|1000"},
        {device2, ConsumeString(fdp, "info2") + "|2000"},
        {device3, ConsumeString(fdp, "info3") + "|1500"}
    };
    service.addressToSpatialEnabledMap_ = {
        {device1, AudioSpatializationState{fdp.ConsumeBool(), fdp.ConsumeBool()}},
        {device2, AudioSpatializationState{fdp.ConsumeBool(), fdp.ConsumeBool()}},
        {device3, AudioSpatializationState{fdp.ConsumeBool(), fdp.ConsumeBool()}}
    };
    AudioSpatialDeviceType audioSpatialDeviceType = PickValue(fdp, g_testSpatialDeviceTypes);
    AudioSpatialDeviceType audioSpatialDeviceType1 = PickValue(fdp, g_testSpatialDeviceTypes);
    service.addressToSpatialDeviceStateMap_ = {
        {device1, AudioSpatialDeviceState{device1, fdp.ConsumeBool(),
                                            fdp.ConsumeBool(), audioSpatialDeviceType}},
        {device2, AudioSpatialDeviceState{device2, fdp.ConsumeBool(),
                                            fdp.ConsumeBool(), audioSpatialDeviceType1}},
        {device3, AudioSpatialDeviceState{device3, fdp.ConsumeBool(),
                                            fdp.ConsumeBool(), audioSpatialDeviceType}}
    };
    service.RemoveOldestDevice();
}

void UpdateDeviceSpatialMapInfoFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSpatializationService service;
    for (uint32_t i = 1; i <= AudioSpatializationService::MAX_DEVICE_NUM; ++i) {
        std::string device = ConsumeString(fdp, "device") + std::to_string(i);
        std::string info = ConsumeString(fdp, "info") + std::to_string(i);
        service.UpdateDeviceSpatialMapInfo(device, info);
        service.GetSha256EncryptAddress(device);
    }
    uint32_t exitNum = fdp.ConsumeIntegralInRange<uint32_t>(0, AudioSpatializationService::MAX_DEVICE_NUM);
    service.UpdateDeviceSpatialMapInfo(ConsumeString(fdp, "device") + std::to_string(exitNum),
                                       ConsumeString(fdp, "info") + std::to_string(exitNum));
    service.UpdateDeviceSpatialMapInfo(ConsumeString(fdp, "device") + std::to_string(exitNum + AudioSpatializationService::MAX_DEVICE_NUM),
                                       ConsumeString(fdp, "info") + std::to_string(exitNum + AudioSpatializationService::MAX_DEVICE_NUM));
}

void WriteSpatializationStateToDbFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSpatializationService service;
    AudioSpatializationSceneType testScene = PickValue(fdp, g_testSpatializationSceneTypes);
    service.spatializationSceneType_ = testScene;
    AudioSpatializationService::WriteToDbOperation operation = PickValue(fdp, g_testWriteToDbOperations);
    service.WriteSpatializationStateToDb(operation);
}

void GetSpatializationStateFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSpatializationService service;
    service.spatializationEnabledReal_ = fdp.ConsumeBool();
    service.headTrackingEnabledReal_ = fdp.ConsumeBool();
    StreamUsage supportedUsage = PickValue(fdp, g_testStreamUsages);
    service.GetSpatializationState(supportedUsage);
}

void SetHeadTrackingEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    AudioSpatializationService service;
    bool enable = fdp.ConsumeBool();
    service.SetHeadTrackingEnabled(enable);
}

void AudioSpatializationServiceInitFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    vector<EffectChain> effectChains = {
        {ConsumeString(fdp, "EFFECTCHAIN_BT_MUSIC"), {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")}, PickValue(fdp, g_testSpatializationLabels)},
        {ConsumeString(fdp, "Effect1"), {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")}, PickValue(fdp, g_testSpatializationLabels)},
        {ConsumeString(fdp, "Effect2"), {ConsumeString(fdp, "apply2_1")}, PickValue(fdp, g_testSpatializationLabels)},
        {ConsumeString(fdp, "Effect3"), {}, PickValue(fdp, g_testSpatializationLabels)}
    };
    ptrAudioSpatializationService->Init(effectChains);
}

void IsSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    vector<string> preSettingSpatialAddressList = { "NO_PREVIOUS_SET_DEVICE", "1234", "test_address"};
    ptrAudioSpatializationService->preSettingSpatialAddress_ = PickValue(fdp, preSettingSpatialAddressList);
    ptrAudioSpatializationService->IsSpatializationEnabled();
}

void SetSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    const bool enable = fdp.ConsumeBool();
    ptrAudioSpatializationService->preSettingSpatialAddress_ = ConsumeString(fdp, "NO_PREVIOUS_SET_DEVICE");
    ptrAudioSpatializationService->spatializationStateFlag_.spatializationEnabled = fdp.ConsumeBool();
    ptrAudioSpatializationService->SetSpatializationEnabled(enable);
}

void IsHeadTrackingEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    vector<string> preSettingSpatialAddressList = { "NO_PREVIOUS_SET_DEVICE", "1234"};
    ptrAudioSpatializationService->preSettingSpatialAddress_ = PickValue(fdp, preSettingSpatialAddressList);
    ptrAudioSpatializationService->IsHeadTrackingEnabled();
}

void HandleHeadTrackingDeviceChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    std::unordered_map<std::string, bool> changeInfo;
    changeInfo.insert({ConsumeString(fdp, "abc"), fdp.ConsumeBool()});
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleHeadTrackingDeviceChange(changeInfo);
}

void AudioSpatializationServiceIsSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    std::string address = ConsumeString(fdp, "test_address");
    ptrAudioSpatializationService->IsSpatializationEnabled(address);
}

void AudioSpatializationServiceIsSpatializationEnabledForCurrentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    ptrAudioSpatializationService->currentDeviceAddress_ = ConsumeString(fdp, "test_address");
    ptrAudioSpatializationService->IsSpatializationEnabledForCurrentDevice();
}

void AudioSpatializationServiceSetSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();

    ptrAudioSpatializationService->SetSpatializationEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceIsHeadTrackingEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    AudioSpatializationState spatializationState;
    std::string address = ConsumeString(fdp, "test_address");
    ptrAudioSpatializationService->addressToSpatialEnabledMap_.insert({address, spatializationState});
    ptrAudioSpatializationService->IsHeadTrackingEnabled(address);
}

void AudioSpatializationServiceSetHeadTrackingEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    ptrAudioSpatializationService->SetHeadTrackingEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceHandleSpatializationEnabledChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const bool enable = fdp.ConsumeBool();
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleSpatializationEnabledChange(selectedAudioDevice, enable);
}

void AudioSpatializationServiceHandleSpatializationEnabledChangeForCurrentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const bool enable = fdp.ConsumeBool();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleSpatializationEnabledChangeForCurrentDevice(enable);
}

void AudioSpatializationServiceHandleHeadTrackingEnabledChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const bool enable = fdp.ConsumeBool();
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleHeadTrackingEnabledChange(selectedAudioDevice, enable);
}

void AudioSpatializationServiceIsHeadTrackingSupportedForDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::string address = ConsumeString(fdp, "1234");
    ptrAudioSpatializationService->IsHeadTrackingSupportedForDevice(address);
}

void AudioSpatializationServiceUpdateSpatialDeviceStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const AudioSpatialDeviceState audioSpatialDeviceState = {
        ConsumeString(fdp, "1234"),
        fdp.ConsumeBool(),
        fdp.ConsumeBool(),
        PickValue(fdp, g_testSpatialDeviceTypes),
    };
    ptrAudioSpatializationService->UpdateSpatialDeviceState(audioSpatialDeviceState);
}

void AudioSpatializationServiceGetSpatializationSceneTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    ptrAudioSpatializationService->spatializationSceneType_ = PickValue(fdp, g_testSpatializationSceneTypes);
    ptrAudioSpatializationService->GetSpatializationSceneType();
}

void AudioSpatializationServiceUnregisterSpatializationStateEventListenerFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    uint32_t sessionID = fdp.ConsumeIntegral<uint32_t>();
    ptrAudioSpatializationService->UnregisterSpatializationStateEventListener(sessionID);
}

void AudioSpatializationServiceSetSpatializationSceneTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    AudioSpatializationSceneType spatializationSceneType = PickValue(fdp, g_testSpatializationSceneTypes);
    ptrAudioSpatializationService->SetSpatializationSceneType(spatializationSceneType);
}

void AudioSpatializationServiceDeinitFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    vector<EffectChain> effectChains = {
        {ConsumeString(fdp, "EFFECTCHAIN_BT_MUSIC"), {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")}, PickValue(fdp, g_testSpatializationLabels)},
    };
    ptrAudioSpatializationService->Init(effectChains);
    ptrAudioSpatializationService->Deinit();
}

void AudioSpatializationServiceGetAudioServerProxyFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    vector<EffectChain> effectChains = {
        {ConsumeString(fdp, "EFFECTCHAIN_BT_MUSIC"), {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")}, PickValue(fdp, g_testSpatializationLabels)},
    };
    ptrAudioSpatializationService->Init(effectChains);
    ptrAudioSpatializationService->GetAudioServerProxy();
}

void AudioSpatializationServiceIsSpatializationSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    ptrAudioSpatializationService->spatializationEnabledReal_ = fdp.ConsumeBool();
    ptrAudioSpatializationService->IsSpatializationSupported();
}

void AudioSpatializationServiceIsHeadTrackingSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    ptrAudioSpatializationService->headTrackingEnabledReal_ = fdp.ConsumeBool();
    ptrAudioSpatializationService->IsHeadTrackingSupported();
}

void AudioSpatializationServiceIsSpatializationSupportedForDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    std::string address = ConsumeString(fdp, "test_address");
    ptrAudioSpatializationService->IsSpatializationSupportedForDevice(address);
}

void AudioSpatializationServiceIsHeadTrackingDataRequestedFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    ptrAudioSpatializationService->currentDeviceAddress_ = macAddress;
    ptrAudioSpatializationService->isHeadTrackingDataRequested_ = fdp.ConsumeBool();
    ptrAudioSpatializationService->IsHeadTrackingDataRequested(macAddress);
}

void AudioSpatializationServiceUpdateRendererInfoFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfo;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    if (changeInfo != nullptr) {
        changeInfo->rendererState = PickValue(fdp, g_testRendererStates);
        changeInfo->outputDeviceInfo.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
        changeInfo->rendererInfo.streamUsage = PickValue(fdp, g_testStreamUsages);
        rendererChangeInfo.push_back(changeInfo);
    }
    ptrAudioSpatializationService->UpdateRendererInfo(rendererChangeInfo);
}

void AudioSpatializationServiceInitSpatializationStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    ptrAudioSpatializationService->spatializationEnabledReal_ = fdp.ConsumeBool();
    ptrAudioSpatializationService->InitSpatializationState();
}

void AudioSpatializationServiceGetCurrentDeviceAddressFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    ptrAudioSpatializationService->currentDeviceAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    ptrAudioSpatializationService->GetCurrentDeviceAddress();
}

void AudioSpatializationServiceUpdateSpatializationSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    std::string encryptedAddress = ConsumeString(fdp, "encrypted_test_address");
    ptrAudioSpatializationService->UpdateSpatializationSupported(encryptedAddress);
}

void AudioSpatializationServiceRegisterSpatializationStateEventListenerFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    uint32_t sessionID = fdp.ConsumeIntegral<uint32_t>();
    StreamUsage streamUsage = PickValue(fdp, g_testStreamUsages);
    sptr<IRemoteObject> object = nullptr;
    ptrAudioSpatializationService->RegisterSpatializationStateEventListener(sessionID, streamUsage, object);
}

void AudioSpatializationServiceHandleDeviceConfigChangedFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleDeviceConfigChanged(selectedAudioDevice);
}

void AudioSpatializationServiceHandleSpatializationEnabledChangeBoolFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    const bool enable = fdp.ConsumeBool();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleSpatializationEnabledChange(enable);
}

void AudioSpatializationServiceHandleHeadTrackingEnabledChangeBoolFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    const bool enable = fdp.ConsumeBool();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleHeadTrackingEnabledChange(enable);
}

void AudioSpatializationServiceGetSpatializationStateDefaultFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    ptrAudioSpatializationService->spatializationEnabledReal_ = fdp.ConsumeBool();
    ptrAudioSpatializationService->headTrackingEnabledReal_ = fdp.ConsumeBool();
    ptrAudioSpatializationService->GetSpatializationState();
}

void AudioSpatializationServiceUpdateSpatialDeviceStateMultipleFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    for (size_t i = 0; i < g_testSpatialDeviceTypes.size(); ++i) {
        const AudioSpatialDeviceState audioSpatialDeviceState = {
            ConsumeString(fdp, "device_" + std::to_string(i)),
            fdp.ConsumeBool(),
            fdp.ConsumeBool(),
            g_testSpatialDeviceTypes[i],
        };
        ptrAudioSpatializationService->UpdateSpatialDeviceState(audioSpatialDeviceState);
    }
}

void AudioSpatializationServiceSetSpatializationEnabledWithStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    ptrAudioSpatializationService->spatializationStateFlag_.spatializationEnabled = !enable;
    ptrAudioSpatializationService->SetSpatializationEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceSetHeadTrackingEnabledWithStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    ptrAudioSpatializationService->spatializationStateFlag_.headTrackingEnabled = !enable;
    ptrAudioSpatializationService->SetHeadTrackingEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceUpdateRendererInfoMultipleFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfo;
    for (size_t i = 0; i < 3; ++i) {
        auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
        if (changeInfo != nullptr) {
            changeInfo->rendererState = PickValue(fdp, g_testRendererStates);
            changeInfo->outputDeviceInfo.macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
            changeInfo->rendererInfo.streamUsage = PickValue(fdp, g_testStreamUsages);
            rendererChangeInfo.push_back(changeInfo);
        }
    }
    ptrAudioSpatializationService->UpdateRendererInfo(rendererChangeInfo);
}

void AudioSpatializationServiceIsHeadTrackingDataRequestedDifferentDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    std::string macAddress = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    ptrAudioSpatializationService->currentDeviceAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    ptrAudioSpatializationService->isHeadTrackingDataRequested_ = fdp.ConsumeBool();
    ptrAudioSpatializationService->IsHeadTrackingDataRequested(macAddress);
}

void AudioSpatializationServiceSetSpatializationSceneTypeAllTypesFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    for (const auto& sceneType : g_testSpatializationSceneTypes) {
        ptrAudioSpatializationService->SetSpatializationSceneType(sceneType);
    }
}

void AudioSpatializationServiceIsPersonalizedSpatializationSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }
    vector<EffectChain> effectChains = {
        {ConsumeString(fdp, "EFFECTCHAIN_BT_MUSIC"), {ConsumeString(fdp, "apply1_1"), ConsumeString(fdp, "apply1_2")}, PickValue(fdp, g_testSpatializationLabels)},
    };
    ptrAudioSpatializationService->Init(effectChains);
    ptrAudioSpatializationService->IsPersonalizedSpatializationSupported();
}

void AudioSpatializationServiceIsPersonalizedSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    std::string address = ConsumeString(fdp, "test_address");
    ptrAudioSpatializationService->IsPersonalizedSpatializationEnabled(address);
}

void AudioSpatializationServiceSetPersonalizedSpatializationEnabledFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();

    ptrAudioSpatializationService->SetPersonalizedSpatializationEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceHandlePersonalizedSpatializationEnabledChangeFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const bool enable = fdp.ConsumeBool();
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandlePersonalizedSpatializationEnabledChange(selectedAudioDevice, enable);
}

void AudioSpatializationServiceSetPersonalizedSpatializationEnabledWithStateFuzzTest(FuzzedDataProvider& fdp)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = MakeDeviceDescriptor(fdp, OUTPUT_DEVICE);
    const bool enable = fdp.ConsumeBool();
    ptrAudioSpatializationService->spatializationStateFlag_.personalizedSpatializationEnabled = !enable;
    ptrAudioSpatializationService->SetPersonalizedSpatializationEnabled(selectedAudioDevice, enable);
}

using FuzzTestFunc = void (*)(FuzzedDataProvider& fdp);
static constexpr FuzzTestFunc ALL_FUZZ_TESTS[] = {
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
    AudioSpatializationServiceUnregisterSpatializationStateEventListenerFuzzTest,
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
    AudioSpatializationServiceRegisterSpatializationStateEventListenerFuzzTest,
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
};

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray(ALL_FUZZ_TESTS);
    func(fdp);
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
    return 0;
}