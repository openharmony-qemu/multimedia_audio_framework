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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

vector<AudioSpatializationSceneType> AudioSpatializationSceneTypeVec = {
    SPATIALIZATION_SCENE_TYPE_DEFAULT,
    SPATIALIZATION_SCENE_TYPE_MUSIC,
    SPATIALIZATION_SCENE_TYPE_MOVIE,
    SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
    SPATIALIZATION_SCENE_TYPE_MAX,
};

vector<StreamUsage> StreamUsageVec = {
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

void UpdateCurrentDeviceFuzzTest()
{
    AudioSpatializationService service;
     // Update with a new device
    std::string newMacAddress = "00:11:22:33:44:55";
    service.UpdateCurrentDevice(newMacAddress);
    // Update with the same device (no change expected)
    service.UpdateCurrentDevice(newMacAddress);
    // Update with an empty address (should not change the current device)
    std::string originalAddress = service.GetCurrentDeviceAddress();
    service.UpdateCurrentDevice("");
    // Update with a new device that has spatial capabilities
    std::string spatialDeviceAddress = "AA:BB:CC:DD:EE:FF";
    int32_t audioSpatialDeviceTypeCount = static_cast<int32_t>(AudioSpatialDeviceType::EARPHONE_TYPE_OTHERS) + 1;
    AudioSpatialDeviceType audioSpatialDeviceType =
        static_cast<AudioSpatialDeviceType>(GetData<int32_t>() % audioSpatialDeviceTypeCount);
    service.addressToSpatialDeviceStateMap_[service.GetSha256EncryptAddress(spatialDeviceAddress)] = {
        spatialDeviceAddress,            // address
        GetData<uint32_t>() % NUM_2,     // isSpatializationSupported
        GetData<uint32_t>() % NUM_2,     // isHeadTrackingSupported
        audioSpatialDeviceType           // spatialDeviceType
    };
    service.UpdateCurrentDevice(spatialDeviceAddress);
    // Update with a device that doesn't have spatial capabilities
    std::string nonSpatialDeviceAddress = "11:22:33:44:55:66";
    service.UpdateCurrentDevice(nonSpatialDeviceAddress);
}

void RemoveOldestDeviceFuzzTest()
{
    AudioSpatializationService service;
    service.addressToDeviceSpatialInfoMap_ = {
        {"device1", "info1|1000"},
        {"device2", "info2|2000"},
        {"device3", "info3|1500"}
    };
    service.addressToSpatialEnabledMap_ = {
        {"device1", AudioSpatializationState{GetData<uint32_t>() % NUM_2, GetData<uint32_t>() % NUM_2}},
        {"device2", AudioSpatializationState{GetData<uint32_t>() % NUM_2, GetData<uint32_t>() % NUM_2}},
        {"device3", AudioSpatializationState{GetData<uint32_t>() % NUM_2, GetData<uint32_t>() % NUM_2}}
    };
    int32_t audioSpatialDeviceTypeCount = static_cast<int32_t>(AudioSpatialDeviceType::EARPHONE_TYPE_OTHERS) + 1;
    int32_t audioSpatialDeviceTypeCount1 = static_cast<int32_t>(AudioSpatialDeviceType::EARPHONE_TYPE_OTHERS) + 1;
    AudioSpatialDeviceType audioSpatialDeviceType =
        static_cast<AudioSpatialDeviceType>(GetData<int32_t>() % audioSpatialDeviceTypeCount);
    AudioSpatialDeviceType audioSpatialDeviceType1 =
        static_cast<AudioSpatialDeviceType>(GetData<int32_t>() % audioSpatialDeviceTypeCount1);
    service.addressToSpatialDeviceStateMap_ = {
        {"device1", AudioSpatialDeviceState{"device1", GetData<uint32_t>() % NUM_2,
                                            GetData<uint32_t>() % NUM_2, audioSpatialDeviceType}},
        {"device2", AudioSpatialDeviceState{"device2", GetData<uint32_t>() % NUM_2,
                                            GetData<uint32_t>() % NUM_2, audioSpatialDeviceType1}},
        {"device3", AudioSpatialDeviceState{"device3", GetData<uint32_t>() % NUM_2,
                                            GetData<uint32_t>() % NUM_2, audioSpatialDeviceType}}
    };
    service.RemoveOldestDevice();
}

void UpdateDeviceSpatialMapInfoFuzzTest()
{
    AudioSpatializationService service;
    for (uint32_t i = 1; i <= AudioSpatializationService::MAX_DEVICE_NUM; ++i) {
        std::string device = "device" + std::to_string(i);
        std::string info = "info" + std::to_string(i);
        service.UpdateDeviceSpatialMapInfo(device, info);
        service.GetSha256EncryptAddress(device);
    }
    // Test updating existing devices
    auto exitNum = GetData<uint32_t>() % AudioSpatializationService::MAX_DEVICE_NUM;
    service.UpdateDeviceSpatialMapInfo("device" + to_string(exitNum), "info" + to_string(exitNum));
    // Test adding more than the maximum number of devices
    service.UpdateDeviceSpatialMapInfo("device" + to_string(exitNum + AudioSpatializationService::MAX_DEVICE_NUM),
                                       "info" + to_string(exitNum + AudioSpatializationService::MAX_DEVICE_NUM));
}

void WriteSpatializationStateToDbFuzzTest()
{
    AudioSpatializationService service;
    uint32_t testSceneCount = GetData<uint32_t>() % AudioSpatializationSceneTypeVec.size();
    AudioSpatializationSceneType testScene = AudioSpatializationSceneTypeVec[testSceneCount];
    service.spatializationSceneType_ = testScene;
    int32_t operationCount =
        static_cast<int32_t>(AudioSpatializationService::WriteToDbOperation::WRITE_DEVICESPATIAL_INFO) + 1;
    AudioSpatializationService::WriteToDbOperation operation =
        static_cast<AudioSpatializationService::WriteToDbOperation>(GetData<uint8_t>() % operationCount);
    service.WriteSpatializationStateToDb(operation, "");
}

void GetSpatializationStateFuzzTest()
{
    AudioSpatializationService service;
    service.spatializationEnabledReal_ = GetData<uint32_t>() % NUM_2;
    service.headTrackingEnabledReal_ = GetData<uint32_t>() % NUM_2;
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    StreamUsage supportedUsage = StreamUsageVec[streamUsageCount];
    service.GetSpatializationState(supportedUsage);
}

void SetHeadTrackingEnabledFuzzTest()
{
    AudioSpatializationService service;
    bool enable = GetData<uint32_t>() % NUM_2;
    service.SetHeadTrackingEnabled(enable);
}

void AudioSpatializationServiceInitFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    vector<EffectChain> effectChains = {
        {"EFFECTCHAIN_BT_MUSIC", {"apply1_1", "apply1_2"}, "SPATIALIZATION_AND_HEADTRACKING"},
        {"Effect1", {"apply1_1", "apply1_2"}, "SPATIALIZATION_AND_HEADTRACKING"},
        {"Effect2", {"apply2_1"}, "SPATIALIZATION"},
        {"Effect3", {}, "HEADTRACKING"}
    };
    ptrAudioSpatializationService->Init(effectChains);
}

void IsSpatializationEnabledFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    vector<string> preSettingSpatialAddressList = { "NO_PREVIOUS_SET_DEVICE", "1234", "test_address"};
    uint32_t preSettingSpatialAddressCount = GetData<uint32_t>() % preSettingSpatialAddressList.size();
    ptrAudioSpatializationService->preSettingSpatialAddress_ =
        preSettingSpatialAddressList[preSettingSpatialAddressCount];
    ptrAudioSpatializationService->IsSpatializationEnabled();
}

void SetSpatializationEnabledFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    const bool enable = GetData<uint32_t>() % NUM_2;
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    ptrAudioSpatializationService->spatializationStateFlag_.spatializationEnabled = GetData<uint32_t>() % NUM_2;
    ptrAudioSpatializationService->SetSpatializationEnabled(enable);
}

void IsHeadTrackingEnabledFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    vector<string> preSettingSpatialAddressList = { "NO_PREVIOUS_SET_DEVICE", "1234"};
    uint32_t preSettingSpatialAddressCount = GetData<uint32_t>() % preSettingSpatialAddressList.size();
    ptrAudioSpatializationService->preSettingSpatialAddress_ =
        preSettingSpatialAddressList[preSettingSpatialAddressCount];
    ptrAudioSpatializationService->IsHeadTrackingEnabled();
}

void HandleHeadTrackingDeviceChangeFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    std::unordered_map<std::string, bool> changeInfo;
    changeInfo.insert({"abc", GetData<uint32_t>() % NUM_2});
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleHeadTrackingDeviceChange(changeInfo);
}

void AudioSpatializationServiceIsSpatializationEnabledFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    std::string address = "test_address";
    ptrAudioSpatializationService->IsSpatializationEnabled(address);
}

void AudioSpatializationServiceIsSpatializationEnabledForCurrentDeviceFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    ptrAudioSpatializationService->currentDeviceAddress_ = "test_address";
    ptrAudioSpatializationService->IsSpatializationEnabledForCurrentDevice();
}

void AudioSpatializationServiceSetSpatializationEnabledFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    const bool enable = GetData<bool>();

    ptrAudioSpatializationService->SetSpatializationEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceIsHeadTrackingEnabledFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    AudioSpatializationState spatializationState;
    std::string address = "test_address";
    ptrAudioSpatializationService->addressToSpatialEnabledMap_.insert({address, spatializationState});
    ptrAudioSpatializationService->IsHeadTrackingEnabled(address);
}

void AudioSpatializationServiceSetHeadTrackingEnabledFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    const bool enable = GetData<bool>();
    ptrAudioSpatializationService->SetHeadTrackingEnabled(selectedAudioDevice, enable);
}

void AudioSpatializationServiceHandleSpatializationEnabledChangeFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const bool enable = GetData<bool>();
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleSpatializationEnabledChange(selectedAudioDevice, enable);
}

void AudioSpatializationServiceHandleSpatializationEnabledChangeForCurrentDeviceFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const bool enable = GetData<bool>();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleSpatializationEnabledChangeForCurrentDevice(enable);
}

void AudioSpatializationServiceHandleHeadTrackingEnabledChangeFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const bool enable = GetData<bool>();
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();
    ptrAudioSpatializationService->HandleHeadTrackingEnabledChange(selectedAudioDevice, enable);
}

void AudioSpatializationServiceIsHeadTrackingSupportedForDeviceFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    const std::string address = "1234";
    ptrAudioSpatializationService->IsHeadTrackingSupportedForDevice(address);
}

void AudioSpatializationServiceUpdateSpatialDeviceStateFuzzTest()
{
    static const vector<AudioSpatialDeviceType> audioSpatialDeviceTypeVec = {
        EARPHONE_TYPE_NONE,
        EARPHONE_TYPE_INEAR,
        EARPHONE_TYPE_HALF_INEAR,
        EARPHONE_TYPE_HEADPHONE,
        EARPHONE_TYPE_GLASSES,
        EARPHONE_TYPE_OTHERS,
    };
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr || audioSpatialDeviceTypeVec.empty()) {
        return;
    }

    const AudioSpatialDeviceState audioSpatialDeviceState = {
        "1234",
        GetData<bool>(),
        GetData<bool>(),
        audioSpatialDeviceTypeVec[GetData<uint32_t>() % audioSpatialDeviceTypeVec.size()],
    };
    ptrAudioSpatializationService->UpdateSpatialDeviceState(audioSpatialDeviceState);
}

void AudioSpatializationServiceGetSpatializationSceneTypeFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    ptrAudioSpatializationService->GetSpatializationSceneType();
}

void AudioSpatializationServiceUnregisterSpatializationStateEventListenerFuzzTest()
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr) {
        return;
    }

    uint32_t sessionID = GetData<uint32_t>();
    ptrAudioSpatializationService->UnregisterSpatializationStateEventListener(sessionID);
}

void AudioSpatializationServiceSetSpatializationSceneTypeFuzzTest()
{
    static const vector<AudioSpatializationSceneType> testAudioSpatializationSceneType = {
        SPATIALIZATION_SCENE_TYPE_DEFAULT,
        SPATIALIZATION_SCENE_TYPE_MUSIC,
        SPATIALIZATION_SCENE_TYPE_MOVIE,
        SPATIALIZATION_SCENE_TYPE_AUDIOBOOK,
        SPATIALIZATION_SCENE_TYPE_MAX,
    };
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();
    if (ptrAudioSpatializationService == nullptr || testAudioSpatializationSceneType.empty()) {
        return;
    }

    AudioSpatializationSceneType spatializationSceneType =
        testAudioSpatializationSceneType[GetData<uint32_t>() % testAudioSpatializationSceneType.size()];
    ptrAudioSpatializationService->SetSpatializationSceneType(spatializationSceneType);
}

TestFuncs g_testFuncs[] = {
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

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
