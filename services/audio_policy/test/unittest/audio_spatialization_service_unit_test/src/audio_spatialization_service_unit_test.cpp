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

#include "audio_spatialization_service_unit_test.h"
#include "audio_errors.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "audio_spatialization_state_change_listener_proxy.h"
#include <openssl/sha.h>
#include <thread>
#include <string>
#include <memory>
#include <vector>
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSpatializationServiceUnitTest::SetUpTestCase(void) {}
void AudioSpatializationServiceUnitTest::TearDownTestCase(void) {}
void AudioSpatializationServiceUnitTest::SetUp(void) {}
void AudioSpatializationServiceUnitTest::TearDown(void) {}

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

static const int32_t SPATIALIZATION_SERVICE_OK = 0;
static const std::string SPATIALIZATION_STATE_SETTINGKEY = "spatialization_state";
static const std::string SPATIALIZATION_SCENE_SETTINGKEY = "spatialization_scene";
static const std::string PRE_SETTING_SPATIAL_ADDRESS = "pre_setting_spatial_address";

enum SpatializationStateOffset {
    SPATIALIZATION_OFFSET,
    HEADTRACKING_OFFSET
};

static uint32_t PackSpatializationState(AudioSpatializationState state)
{
    uint32_t spatializationEnabled = state.spatializationEnabled ? 1 : 0;
    uint32_t headTrackingEnabled = state.headTrackingEnabled ? 1 :0;
    return (spatializationEnabled << SPATIALIZATION_OFFSET) | (headTrackingEnabled << HEADTRACKING_OFFSET);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_001
* @tc.desc  : Test UpdateCurrentDevice.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_001, TestSize.Level1)
{
    AudioSpatializationService service;
     // Test case 1: Update with a new device
    std::string newMacAddress = "00:11:22:33:44:55";
    service.UpdateCurrentDevice(newMacAddress);
    EXPECT_EQ(service.GetCurrentDeviceAddress(), newMacAddress);

    // Test case 2: Update with the same device (no change expected)
    service.UpdateCurrentDevice(newMacAddress);
    EXPECT_EQ(service.GetCurrentDeviceAddress(), newMacAddress);

    // Test case 3: Update with an empty address (should not change the current device)
    std::string originalAddress = service.GetCurrentDeviceAddress();
    service.UpdateCurrentDevice("");
    EXPECT_NE(service.GetCurrentDeviceAddress(), originalAddress);

    // Test case 4: Update with a new device that has spatial capabilities
    std::string spatialDeviceAddress = "AA:BB:CC:DD:EE:FF";
    service.addressToSpatialDeviceStateMap_[service.GetSha256EncryptAddress(spatialDeviceAddress)] = {
        spatialDeviceAddress,  // address
        true,                  // isSpatializationSupported
        false,                 // isHeadTrackingSupported
        AudioSpatialDeviceType::EARPHONE_TYPE_HEADPHONE  // spatialDeviceType
    };
    service.UpdateCurrentDevice(spatialDeviceAddress);
    EXPECT_EQ(service.GetCurrentDeviceAddress(), spatialDeviceAddress);
    EXPECT_EQ(service.currSpatialDeviceType_, AudioSpatialDeviceType::EARPHONE_TYPE_HEADPHONE);

    // Test case 5: Update with a device that doesn't have spatial capabilities
    std::string nonSpatialDeviceAddress = "11:22:33:44:55:66";
    service.UpdateCurrentDevice(nonSpatialDeviceAddress);
    EXPECT_EQ(service.GetCurrentDeviceAddress(), nonSpatialDeviceAddress);
    EXPECT_EQ(service.currSpatialDeviceType_, EARPHONE_TYPE_NONE);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_002
* @tc.desc  : Test RemoveOldestDevice.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_002, TestSize.Level1)
{
    //Initialize the data
    service.addressToDeviceSpatialInfoMap_ = {
        {"device1", "info1|1000"},
        {"device2", "info2|2000"},
        {"device3", "info3|1500"}
    };
    service.addressToSpatialEnabledMap_ = {
        {"device1", AudioSpatializationState{true, true}},
        {"device2", AudioSpatializationState{false, false}},
        {"device3", AudioSpatializationState{true, false}}
    };
    service.addressToSpatialDeviceStateMap_ = {
        {"device1", AudioSpatialDeviceState{"device1", true, true, AudioSpatialDeviceType::EARPHONE_TYPE_HEADPHONE}},
        {"device2", AudioSpatialDeviceState{"device2", false, false, AudioSpatialDeviceType::EARPHONE_TYPE_NONE}},
        {"device3", AudioSpatialDeviceState{"device3", true, false, AudioSpatialDeviceType::EARPHONE_TYPE_HEADPHONE}}
    };
    std::string removedDevice = service.RemoveOldestDevice();
    // Verification returns the oldest device address
    EXPECT_EQ(removedDevice, "device1");
    // Verify that the oldest device has been removed from all mappings
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_.count("device1"), 0);
    EXPECT_EQ(service.addressToSpatialEnabledMap_.count("device1"), 0);
    EXPECT_EQ(service.addressToSpatialDeviceStateMap_.count("device1"), 0);
    // Verify that the other devices are still there
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_.count("device2"), 1);
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_.count("device3"), 1);
    EXPECT_EQ(service.addressToSpatialEnabledMap_.count("device2"), 1);
    EXPECT_EQ(service.addressToSpatialEnabledMap_.count("device3"), 1);
    EXPECT_EQ(service.addressToSpatialDeviceStateMap_.count("device2"), 1);
    EXPECT_EQ(service.addressToSpatialDeviceStateMap_.count("device3"), 1);
    // Verify the number of devices remaining
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_.size(), 2);
    EXPECT_EQ(service.addressToSpatialEnabledMap_.size(), 2);
    EXPECT_EQ(service.addressToSpatialDeviceStateMap_.size(), 2);
    // Verify that the information for the remaining devices remains the same
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_["device2"], "info2|2000");
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_["device3"], "info3|1500");
    // Verify the information for the remaining devices in the addressToSpatialDeviceStateMap_
    const auto& device2State = service.addressToSpatialDeviceStateMap_["device2"];
    EXPECT_EQ(device2State.address, "device2");
    EXPECT_FALSE(device2State.isSpatializationSupported);
    EXPECT_FALSE(device2State.isHeadTrackingSupported);

    const auto& device3State = service.addressToSpatialDeviceStateMap_["device3"];
    EXPECT_EQ(device3State.address, "device3");
    EXPECT_TRUE(device3State.isSpatializationSupported);
    EXPECT_FALSE(device3State.isHeadTrackingSupported);
    EXPECT_EQ(device3State.spatialDeviceType, AudioSpatialDeviceType::EARPHONE_TYPE_HEADPHONE);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_003
* @tc.desc  : Test UpdateDeviceSpatialMapInfo.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_003, TestSize.Level1)
{
    // Test adding a new device
    for (uint32_t i = 1; i <= AudioSpatializationService::MAX_DEVICE_NUM; ++i) {
        std::string device = "device" + std::to_string(i);
        std::string info = "info" + std::to_string(i);
        service.UpdateDeviceSpatialMapInfo(device, info);

        auto encryptedDevice = service.GetSha256EncryptAddress(device);
        EXPECT_EQ(service.addressToDeviceSpatialInfoMap_[encryptedDevice], info);
        EXPECT_EQ(service.addressToDeviceIDMap_[encryptedDevice], i);
    }

    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_.size(), AudioSpatializationService::MAX_DEVICE_NUM);
    EXPECT_EQ(service.addressToDeviceIDMap_.size(), AudioSpatializationService::MAX_DEVICE_NUM);
    // Test updating existing devices
    service.UpdateDeviceSpatialMapInfo("device5", "updated_info5");
    auto encryptedDevice5 = service.GetSha256EncryptAddress("device5");
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_[encryptedDevice5], "updated_info5");
    EXPECT_EQ(service.addressToDeviceIDMap_[encryptedDevice5], 5);
    // Test adding more than the maximum number of devices
    service.UpdateDeviceSpatialMapInfo("device11", "info11");
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_.size(), AudioSpatializationService::MAX_DEVICE_NUM);
    EXPECT_NE(service.addressToDeviceIDMap_.size(), AudioSpatializationService::MAX_DEVICE_NUM);
    auto encryptedDevice1 = service.GetSha256EncryptAddress("device1");
    auto encryptedDevice11 = service.GetSha256EncryptAddress("device11");
    // Verify that the oldest device is removed
    EXPECT_NE(service.addressToDeviceSpatialInfoMap_.count(encryptedDevice1), 0);
    EXPECT_NE(service.addressToDeviceIDMap_.count(encryptedDevice1), 0);
    // Verify that the new device is added correctly
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_.count(encryptedDevice11), 1);
    EXPECT_EQ(service.addressToDeviceIDMap_.count(encryptedDevice11), 1);
    // Verify that the new device inherits the ID of the removed device
    EXPECT_NE(service.addressToDeviceIDMap_[encryptedDevice11], 1);
    // Verify that the other device information remains the same
    auto encryptedDevice10 = service.GetSha256EncryptAddress("device10");
    EXPECT_EQ(service.addressToDeviceSpatialInfoMap_[encryptedDevice10], "info10");
    EXPECT_EQ(service.addressToDeviceIDMap_[encryptedDevice10], 10);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_004
* @tc.desc  : Test WriteSpatializationStateToDb_WRITE_SPATIALIZATION_STATE
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_004, TestSize.Level1)
{
    AudioSpatializationState testState = {true, false};
    service.spatializationStateFlag_ = testState;

    service.WriteSpatializationStateToDb(AudioSpatializationService::WRITE_SPATIALIZATION_STATE, "");

    int32_t savedState;
    AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID).GetIntValue(SPATIALIZATION_STATE_SETTINGKEY, savedState);

    uint32_t expectedPackedState = PackSpatializationState(testState);
    EXPECT_NE(savedState, expectedPackedState);

    // Verify the individual bits
    EXPECT_NE(savedState & (1 << SPATIALIZATION_OFFSET), 1 << SPATIALIZATION_OFFSET);  // Spatialization enabled
    EXPECT_EQ(savedState & (1 << HEADTRACKING_OFFSET), 0);  // Head tracking disabled
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_005
* @tc.desc  : Test WriteSpatializationStateToDb_WRITE_SPATIALIZATION_SCENE
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_005, TestSize.Level1)
{
    AudioSpatializationSceneType testScene = SPATIALIZATION_SCENE_TYPE_DEFAULT;
    service.spatializationSceneType_ = testScene;

    service.WriteSpatializationStateToDb(AudioSpatializationService::WRITE_SPATIALIZATION_SCENE, "");
    int32_t savedScene;
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    settingProvider.GetIntValue(SPATIALIZATION_SCENE_SETTINGKEY, savedScene);

    EXPECT_EQ(savedScene, static_cast<int32_t>(testScene));
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_006
* @tc.desc  : Test WriteSpatializationStateToDb_WRITE_DEVICESPATIAL_INFO
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_006, TestSize.Level1)
{
    std::string testAddress = "test_address";
    std::string encryptedAddress = service.GetSha256EncryptAddress(testAddress);
    uint32_t testDeviceId = 1;
    std::string testDeviceInfo = "test_device_info";
    service.addressToDeviceIDMap_[encryptedAddress] = testDeviceId;
    service.addressToDeviceSpatialInfoMap_[encryptedAddress] = testDeviceInfo;
    service.preSettingSpatialAddress_ = testAddress;

    service.WriteSpatializationStateToDb(AudioSpatializationService::WRITE_DEVICESPATIAL_INFO, testAddress);

    std::string savedDeviceInfo;
    AudioSettingProvider &settingProvider = AudioSettingProvider::GetInstance(AUDIO_POLICY_SERVICE_ID);
    ErrCode ret = settingProvider.GetStringValue(
        SPATIALIZATION_STATE_SETTINGKEY + "_device" + std::to_string(testDeviceId), savedDeviceInfo);
    EXPECT_NE(ret, SUCCESS);
    EXPECT_NE(savedDeviceInfo, testDeviceInfo);

    std::string savedPreSettingAddress;
    ret = settingProvider.GetStringValue(PRE_SETTING_SPATIAL_ADDRESS, savedPreSettingAddress);
    EXPECT_NE(ret, SUCCESS);
    EXPECT_NE(savedPreSettingAddress, testAddress);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_007
* @tc.desc  : Test UpdateSpatializationState
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_007, TestSize.Level1)
{
    service.spatializationEnabledReal_ = true;
    service.headTrackingEnabledReal_ = false;
    service.UpdateSpatializationState();

    AudioSpatializationState currentState = service.GetSpatializationState(StreamUsage::STREAM_USAGE_MEDIA);
    EXPECT_EQ(currentState.spatializationEnabled, true);
    EXPECT_EQ(currentState.headTrackingEnabled, false);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_008
* @tc.desc  : Test UpdateSpatializationState
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_008, TestSize.Level1)
{
    service.spatializationEnabledReal_ = true;
    service.headTrackingEnabledReal_ = true;

    StreamUsage supportedUsage = StreamUsage::STREAM_USAGE_MEDIA;

    AudioSpatializationState result = service.GetSpatializationState(supportedUsage);

    EXPECT_EQ(result.spatializationEnabled, true);
    EXPECT_EQ(result.headTrackingEnabled, true);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_009
* @tc.desc  : Test UpdateSpatializationState
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_009, TestSize.Level1)
{
    service.spatializationEnabledReal_ = true;
    service.headTrackingEnabledReal_ = true;

    StreamUsage unsupportedUsage = StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION;
    AudioSpatializationState result = service.GetSpatializationState(unsupportedUsage);
    EXPECT_EQ(result.spatializationEnabled, true);
    EXPECT_EQ(result.headTrackingEnabled, true);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_010
* @tc.desc  : Test UpdateSpatializationState
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_010, TestSize.Level1)
{
    service.spatializationEnabledReal_ = true;
    service.headTrackingEnabledReal_ = true;
    service.UpdateSpatializationState();

    AudioSpatializationState getResult = service.GetSpatializationState(StreamUsage::STREAM_USAGE_MEDIA);
    EXPECT_EQ(getResult.spatializationEnabled, true);
    EXPECT_EQ(getResult.headTrackingEnabled, true);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_011
* @tc.desc  : Test UpdateCurrentDevice
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_011, TestSize.Level1)
{
    AudioSpatializationService spatializationService;
     // Test empty addresses
    spatializationService.UpdateCurrentDevice("");
    EXPECT_EQ(spatializationService.currentDeviceAddress_, "");
    EXPECT_NE(spatializationService.currSpatialDeviceType_, EARPHONE_TYPE_NONE);

    // Test the new device address
    std::string newAddress = "00:11:22:33:44:55";
    spatializationService.UpdateCurrentDevice(newAddress);
    EXPECT_EQ(spatializationService.currentDeviceAddress_, newAddress);

    // Test the same device address
    spatializationService.UpdateCurrentDevice(newAddress);
    EXPECT_EQ(spatializationService.currentDeviceAddress_, newAddress);

    // Test device type updates
    std::string encryptedAddress = spatializationService.GetSha256EncryptAddress(newAddress);
    spatializationService.addressToSpatialDeviceStateMap_[encryptedAddress].spatialDeviceType = EARPHONE_TYPE_INEAR;
    spatializationService.UpdateCurrentDevice(newAddress);
    EXPECT_NE(spatializationService.currSpatialDeviceType_, EARPHONE_TYPE_INEAR);

    // Test for unknown device types
    std::string unknownAddress = "AA:BB:CC:DD:EE:FF";
    spatializationService.UpdateCurrentDevice(unknownAddress);
    EXPECT_EQ(spatializationService.currSpatialDeviceType_, EARPHONE_TYPE_NONE);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_012
* @tc.desc  : Test SetHeadTrackingEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_012, TestSize.Level1)
{
    bool enable = true;
    int32_t result = service.SetHeadTrackingEnabled(enable);
    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);

    EXPECT_FALSE(service.GetSpatializationState().headTrackingEnabled);

    enable = false;
    result = service.SetHeadTrackingEnabled(enable);
    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);
    EXPECT_FALSE(service.GetSpatializationState().headTrackingEnabled);
}

/**
* @tc.name  : Test AudioSocketThread.
* @tc.number: AudioSpatializationService_013
* @tc.desc  : Test SetHeadTrackingEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_013, TestSize.Level1)
{
    bool enable = true;
    service.SetHeadTrackingEnabled(enable);

    int32_t result = service.SetHeadTrackingEnabled(enable);
    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);

    EXPECT_FALSE(service.GetSpatializationState().headTrackingEnabled);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_014
* @tc.desc  : Test AudioSpatializationService::Init
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_014, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const std::vector<EffectChain> effectChains = {
        {"EFFECTCHAIN_BT_MUSIC", {"apply1_1", "apply1_2"}, "SPATIALIZATION_AND_HEADTRACKING"},
        {"Effect1", {"apply1_1", "apply1_2"}, "SPATIALIZATION_AND_HEADTRACKING"},
        {"Effect2", {"apply2_1"}, "SPATIALIZATION"},
        {"Effect3", {}, "HEADTRACKING"}
    };
    ptrAudioSpatializationService->Init(effectChains);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_015
* @tc.desc  : Test AudioSpatializationService::IsSpatializationEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_015, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    auto result = ptrAudioSpatializationService->IsSpatializationEnabled();

    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_016
* @tc.desc  : Test AudioSpatializationService::IsSpatializationEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_016, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    ptrAudioSpatializationService->preSettingSpatialAddress_ = "1234";
    auto result = ptrAudioSpatializationService->IsSpatializationEnabled();

    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_017
* @tc.desc  : Test AudioSpatializationService::SetSpatializationEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_017, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "1234";
    auto result = ptrAudioSpatializationService->SetSpatializationEnabled(enable);

    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_018
* @tc.desc  : Test AudioSpatializationService::SetSpatializationEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_018, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    ptrAudioSpatializationService->spatializationStateFlag_.spatializationEnabled = true;

    auto result = ptrAudioSpatializationService->SetSpatializationEnabled(enable);

    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_019
* @tc.desc  : Test AudioSpatializationService::SetSpatializationEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_019, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    ptrAudioSpatializationService->spatializationStateFlag_.spatializationEnabled = false;

    auto result = ptrAudioSpatializationService->SetSpatializationEnabled(enable);

    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_020
* @tc.desc  : Test AudioSpatializationService::SetSpatializationEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_020, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    const bool enable = true;

    auto result = ptrAudioSpatializationService->SetSpatializationEnabled(selectedAudioDevice, enable);

    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_021
* @tc.desc  : Test AudioSpatializationService::IsHeadTrackingEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_021, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    auto result = ptrAudioSpatializationService->IsHeadTrackingEnabled();

    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_022
* @tc.desc  : Test AudioSpatializationService::IsHeadTrackingEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_022, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    ptrAudioSpatializationService->preSettingSpatialAddress_ = "1234";
    auto result = ptrAudioSpatializationService->IsHeadTrackingEnabled();

    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_023
* @tc.desc  : Test AudioSpatializationService::SetHeadTrackingEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_023, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "1234";
    auto result = ptrAudioSpatializationService->SetHeadTrackingEnabled(enable);

    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_024
* @tc.desc  : Test AudioSpatializationService::SetHeadTrackingEnabled
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_024, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    const bool enable = true;

    auto result = ptrAudioSpatializationService->SetHeadTrackingEnabled(selectedAudioDevice, enable);

    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_025
* @tc.desc  : Test AudioSpatializationService::HandleSpatializationEnabledChange
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_025, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();

    EXPECT_NE(ptrAudioSpatializationService->audioPolicyServerHandler_, nullptr);

    ptrAudioSpatializationService->HandleSpatializationEnabledChange(enable);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_026
* @tc.desc  : Test AudioSpatializationService::HandleSpatializationEnabledChange
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_026, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    ptrAudioSpatializationService->audioPolicyServerHandler_ = nullptr;

    ptrAudioSpatializationService->HandleSpatializationEnabledChange(enable);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_027
* @tc.desc  : Test AudioSpatializationService::HandleSpatializationEnabledChange
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_027, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = nullptr;

    ptrAudioSpatializationService->HandleSpatializationEnabledChange(selectedAudioDevice, enable);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_028
* @tc.desc  : Test AudioSpatializationService::HandleSpatializationEnabledChange
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_028, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();

    EXPECT_NE(ptrAudioSpatializationService->audioPolicyServerHandler_, nullptr);

    ptrAudioSpatializationService->HandleSpatializationEnabledChange(selectedAudioDevice, enable);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_029
* @tc.desc  : Test AudioSpatializationService::HandleHeadTrackingEnabledChange
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_029, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    ptrAudioSpatializationService->audioPolicyServerHandler_ = nullptr;

    ptrAudioSpatializationService->HandleHeadTrackingEnabledChange(enable);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_030
* @tc.desc  : Test AudioSpatializationService::HandleHeadTrackingEnabledChange
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_030, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = std::make_shared<AudioPolicyServerHandler>();

    EXPECT_NE(ptrAudioSpatializationService->audioPolicyServerHandler_, nullptr);

    ptrAudioSpatializationService->HandleHeadTrackingEnabledChange(selectedAudioDevice, enable);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_031
* @tc.desc  : Test AudioSpatializationService::HandleHeadTrackingEnabledChange
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_031, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const bool enable = true;
    const std::shared_ptr<AudioDeviceDescriptor> selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    ptrAudioSpatializationService->audioPolicyServerHandler_ = nullptr;

    ptrAudioSpatializationService->HandleHeadTrackingEnabledChange(selectedAudioDevice, enable);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_032
* @tc.desc  : Test AudioSpatializationService::IsSpatializationSupportedForDevice
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_032, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const std::string address = "1234";

    auto result = ptrAudioSpatializationService->IsSpatializationSupportedForDevice(address);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_033
* @tc.desc  : Test AudioSpatializationService::IsHeadTrackingSupportedForDevice
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_033, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const std::string address = "1234";

    auto result = ptrAudioSpatializationService->IsHeadTrackingSupportedForDevice(address);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_034
* @tc.desc  : Test AudioSpatializationService::UpdateSpatialDeviceState
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_034, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const AudioSpatialDeviceState audioSpatialDeviceState = {
        "1234",
        true,
        true,
        AudioSpatialDeviceType::EARPHONE_TYPE_NONE,
    };

    auto result = ptrAudioSpatializationService->UpdateSpatialDeviceState(audioSpatialDeviceState);
    EXPECT_EQ(result, SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_035
* @tc.desc  : Test AudioSpatializationService::IsHeadTrackingDataRequested
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_035, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const std::string macAddress = "1234";
    ptrAudioSpatializationService->currentDeviceAddress_ = "1234";
    ptrAudioSpatializationService->isHeadTrackingDataRequested_ = true;

    auto result = ptrAudioSpatializationService->IsHeadTrackingDataRequested(macAddress);
    EXPECT_EQ(result, true);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_036
* @tc.desc  : Test AudioSpatializationService::IsHeadTrackingDataRequested
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_036, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    const std::string macAddress = "1234";
    ptrAudioSpatializationService->currentDeviceAddress_ = "abc";
    ptrAudioSpatializationService->isHeadTrackingDataRequested_ = true;

    auto result = ptrAudioSpatializationService->IsHeadTrackingDataRequested(macAddress);
    EXPECT_EQ(result, false);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_037
* @tc.desc  : Test AudioSpatializationService::UpdateSpatializationStateReal
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_037, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    bool outputDeviceChange = true;
    std::string preDeviceAddress = "1234";
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "1234";

    auto result = ptrAudioSpatializationService->UpdateSpatializationStateReal(outputDeviceChange, preDeviceAddress);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_038
* @tc.desc  : Test AudioSpatializationService::UpdateSpatializationStateReal
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_038, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    bool outputDeviceChange = true;
    std::string preDeviceAddress = "1234";
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    ptrAudioSpatializationService->spatializationEnabledReal_ = true;
    ptrAudioSpatializationService->headTrackingEnabledReal_ = true;

    auto result = ptrAudioSpatializationService->UpdateSpatializationStateReal(outputDeviceChange, preDeviceAddress);
    EXPECT_NE(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_039
* @tc.desc  : Test AudioSpatializationService::UpdateSpatializationStateReal
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_039, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    bool outputDeviceChange = true;
    std::string preDeviceAddress = "1234";
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    ptrAudioSpatializationService->spatializationEnabledReal_ = false;
    ptrAudioSpatializationService->headTrackingEnabledReal_ = true;

    auto result = ptrAudioSpatializationService->UpdateSpatializationStateReal(outputDeviceChange, preDeviceAddress);
    EXPECT_NE(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_040
* @tc.desc  : Test AudioSpatializationService::UpdateSpatializationStateReal
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_040, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    bool outputDeviceChange = true;
    std::string preDeviceAddress = "1234";
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    ptrAudioSpatializationService->spatializationEnabledReal_ = false;
    ptrAudioSpatializationService->headTrackingEnabledReal_ = false;

    auto result = ptrAudioSpatializationService->UpdateSpatializationStateReal(outputDeviceChange, preDeviceAddress);
    EXPECT_EQ(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_041
* @tc.desc  : Test AudioSpatializationService::UpdateSpatializationStateReal
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_041, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    bool outputDeviceChange = true;
    std::string preDeviceAddress = "1234";
    ptrAudioSpatializationService->preSettingSpatialAddress_ = "NO_PREVIOUS_SET_DEVICE";
    ptrAudioSpatializationService->spatializationEnabledReal_ = true;
    ptrAudioSpatializationService->headTrackingEnabledReal_ = false;

    auto result = ptrAudioSpatializationService->UpdateSpatializationStateReal(outputDeviceChange, preDeviceAddress);
    EXPECT_NE(result, SUCCESS);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_042
* @tc.desc  : Test AudioSpatializationService::HandleSpatializationStateChange
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_042, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    bool outputDeviceChange = true;
    ptrAudioSpatializationService->HandleSpatializationStateChange(outputDeviceChange);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_043
* @tc.desc  : Test AudioSpatializationService::WriteSpatializationStateToDb
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_043, TestSize.Level1)
{
    auto ptrAudioSpatializationService = std::make_shared<AudioSpatializationService>();

    EXPECT_NE(ptrAudioSpatializationService, nullptr);

    AudioSpatializationService::WriteToDbOperation operation = AudioSpatializationService::WriteToDbOperation(5);
    const std::string address = "1234";
    ptrAudioSpatializationService->WriteSpatializationStateToDb(operation, address);
}
} // namespace AudioStandard
} // namespace OHOS
