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

#include "audio_spatialization_service_unit_test.h"
#include "gmock/gmock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static std::unique_ptr<AudioSpatializationService> sSpatializationService_ = nullptr;
static const int32_t SPATIALIZATION_SERVICE_OK = 0;
static const std::string BLUETOOTH_EFFECT_CHAIN_NAME = "EFFECTCHAIN_BT_MUSIC";
static const std::string SPATIALIZATION_AND_HEAD_TRACKING_SUPPORTED_LABEL = "SPATIALIZATION_AND_HEADTRACKING";
static const std::string SPATIALIZATION_SUPPORTED_LABEL = "SPATIALIZATION";
static const std::string HEAD_TRACKING_SUPPORTED_LABEL = "HEADTRACKING";
static const std::string TEST_SUPPORTED_LABEL = "TEST_LABEL";
static const std::string TEST_MAC_ADDRESS = "00:11:22:33:44:55";
static const std::string PERSONALIZED_SPATIALIZATION_AND_HEAD_TRACKING_SUPPORTED_LABEL =
    "PERSONALIZED_SPATIALIZATION_AND_HEADTRACKING";
static const std::string PERSONALIZED_SPATIALIZATION_SUPPORTED_LABEL = "PERSONALIZED_SPATIALIZATION";

void AudioSpatializationServiceUnitTest::SetUpTestCase(void) {}
void AudioSpatializationServiceUnitTest::TearDownTestCase(void) {}
void AudioSpatializationServiceUnitTest::SetUp(void)
{
    sSpatializationService_ = std::make_unique<AudioSpatializationService>();
}
void AudioSpatializationServiceUnitTest::TearDown(void)
{
    sSpatializationService_.reset();
}

class MockAudioSpatializationStateChangeCallback : public AudioSpatializationStateChangeCallback {
public:
    MockAudioSpatializationStateChangeCallback() = default;
    virtual ~MockAudioSpatializationStateChangeCallback() = default;

    void OnSpatializationStateChange(const AudioSpatializationState &spatializationState) {}
};

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_001
* @tc.desc  : Test Init.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_001, TestSize.Level4)
{
    std::string sName = BLUETOOTH_EFFECT_CHAIN_NAME;
    std::vector<std::string> applyVec;
    std::string sLabel = SPATIALIZATION_AND_HEAD_TRACKING_SUPPORTED_LABEL;
    EffectChain effectChain1(sName, applyVec, sLabel);
    sLabel = SPATIALIZATION_SUPPORTED_LABEL;
    EffectChain effectChain2(sName, applyVec, sLabel);
    sLabel = HEAD_TRACKING_SUPPORTED_LABEL;
    EffectChain effectChain3(sName, applyVec, sLabel);
    sLabel = TEST_SUPPORTED_LABEL;
    EffectChain effectChain4(sName, applyVec, sLabel);
    sLabel = PERSONALIZED_SPATIALIZATION_AND_HEAD_TRACKING_SUPPORTED_LABEL;
    EffectChain effectChain5(sName, applyVec, sLabel);
    sLabel = PERSONALIZED_SPATIALIZATION_SUPPORTED_LABEL;
    EffectChain effectChain6(sName, applyVec, sLabel);
    std::vector<EffectChain> effectChains = {effectChain1, effectChain2, effectChain3, effectChain4, effectChain5,
        effectChain6};
    sSpatializationService_->Init(effectChains);
    EXPECT_TRUE(sSpatializationService_->isSpatializationSupported_);
    EXPECT_TRUE(sSpatializationService_->isHeadTrackingSupported_);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_002
* @tc.desc  : Test SetSpatializationEnabled.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_002, TestSize.Level4)
{
    auto selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    selectedAudioDevice->macAddress_ = TEST_MAC_ADDRESS;
    std::string encryptedAddress = sSpatializationService_->GetSha256EncryptAddress(selectedAudioDevice->macAddress_);
    bool isSpatializationSupported = false;
    bool isHeadTrackingSupported = false;
    AudioSpatializationState spatializationState(isSpatializationSupported, isHeadTrackingSupported);
    sSpatializationService_->addressToSpatialEnabledMap_.insert(std::make_pair(encryptedAddress, spatializationState));
    bool enable = false;
    EXPECT_EQ(sSpatializationService_->SetHeadTrackingEnabled(selectedAudioDevice, enable),
        SPATIALIZATION_SERVICE_OK);
    EXPECT_EQ(sSpatializationService_->SetSpatializationEnabled(selectedAudioDevice, enable),
        SPATIALIZATION_SERVICE_OK);
    EXPECT_EQ(sSpatializationService_->SetAdaptiveSpatialRenderingEnabled(selectedAudioDevice, enable),
        SPATIALIZATION_SERVICE_OK);
    EXPECT_EQ(sSpatializationService_->SetPersonalizedSpatializationEnabled(selectedAudioDevice, enable),
        SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_003
* @tc.desc  : Test SetSpatializationEnabled.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_003, TestSize.Level4)
{
    auto selectedAudioDevice = std::make_shared<AudioDeviceDescriptor>();
    selectedAudioDevice->macAddress_ = TEST_MAC_ADDRESS;
    std::string encryptedAddress = sSpatializationService_->GetSha256EncryptAddress(selectedAudioDevice->macAddress_);
    bool isSpatializationSupported = false;
    bool isHeadTrackingSupported = false;
    AudioSpatializationState spatializationState(isSpatializationSupported, isHeadTrackingSupported);
    sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddress] = spatializationState;
    sSpatializationService_->currentDevice_.macAddress_ = selectedAudioDevice->macAddress_;
    bool enable = true;
    EXPECT_EQ(sSpatializationService_->SetSpatializationEnabled(selectedAudioDevice, enable),
        SPATIALIZATION_SERVICE_OK);
    EXPECT_EQ(sSpatializationService_->SetAdaptiveSpatialRenderingEnabled(selectedAudioDevice, enable),
        SPATIALIZATION_SERVICE_OK);
    EXPECT_EQ(sSpatializationService_->SetPersonalizedSpatializationEnabled(selectedAudioDevice, enable), ERROR);

    sSpatializationService_->currentDevice_.macAddress_ = "00:11:22:33:44:66";
    enable = false;
    EXPECT_EQ(sSpatializationService_->SetSpatializationEnabled(selectedAudioDevice, enable),
        SPATIALIZATION_SERVICE_OK);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_004
* @tc.desc  : Test GetSpatializationState.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_004, TestSize.Level4)
{
    sSpatializationService_->spatializationEnabledReal_ = true;
    StreamUsage streamUsage = STREAM_USAGE_GAME;
    AudioSpatializationState spatializationState = sSpatializationService_->GetSpatializationState(streamUsage);
    EXPECT_FALSE(spatializationState.spatializationEnabled);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_005
* @tc.desc  : Test IsSpatializationSupportedForDevice and IsHeadTrackingSupportedForDevice.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_005, TestSize.Level4)
{
    std::string address = TEST_MAC_ADDRESS;
    std::string encryptedAddress = sSpatializationService_->GetSha256EncryptAddress(address);
    bool isSpatializationSupported = true;
    bool isHeadTrackingSupported = true;
    AudioSpatialDeviceType spatialDeviceType = EARPHONE_TYPE_INEAR;
    AudioSpatialDeviceState spatialDeviceState = {
        address, isSpatializationSupported, isHeadTrackingSupported, spatialDeviceType
    };
    sSpatializationService_->addressToSpatialDeviceStateMap_.insert(
        std::make_pair(encryptedAddress, spatialDeviceState));
    EXPECT_TRUE(sSpatializationService_->IsSpatializationSupportedForDevice(address));
    EXPECT_TRUE(sSpatializationService_->IsHeadTrackingSupportedForDevice(address));
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_006
* @tc.desc  : Test UpdateSpatialDeviceState.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_006, TestSize.Level4)
{
    std::string address = TEST_MAC_ADDRESS;
    bool isSpatializationSupported = true;
    bool isHeadTrackingSupported = true;
    std::string encryptedAddress = sSpatializationService_->GetSha256EncryptAddress(address);
    AudioSpatialDeviceType spatialDeviceType = EARPHONE_TYPE_INEAR;
    AudioSpatialDeviceState spatialDeviceState = {
        address, isSpatializationSupported, isHeadTrackingSupported, spatialDeviceType
    };
    sSpatializationService_->addressToSpatialDeviceStateMap_.insert(
        std::make_pair(encryptedAddress, spatialDeviceState));
    EXPECT_EQ(sSpatializationService_->UpdateSpatialDeviceState(spatialDeviceState), SPATIALIZATION_SERVICE_OK);

    AudioSpatialDeviceState audioSpatialDeviceState = spatialDeviceState;
    audioSpatialDeviceState.spatialDeviceType = EARPHONE_TYPE_HALF_INEAR;
    sSpatializationService_->currSpatialDeviceType_ = EARPHONE_TYPE_OTHERS;
    sSpatializationService_->UpdateSpatialDeviceState(audioSpatialDeviceState);
    EXPECT_EQ(sSpatializationService_->currSpatialDeviceType_, EARPHONE_TYPE_HALF_INEAR);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_007
* @tc.desc  : Test UpdateCurrentDevice.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_007, TestSize.Level4)
{
    AudioDeviceDescriptor selectedAudioDevice;
    selectedAudioDevice.macAddress_ = TEST_MAC_ADDRESS;
    sSpatializationService_->currSpatialDeviceType_ = EARPHONE_TYPE_OTHERS;
    sSpatializationService_->UpdateCurrentDevice(selectedAudioDevice);
    EXPECT_EQ(sSpatializationService_->currSpatialDeviceType_, EARPHONE_TYPE_NONE);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_008
* @tc.desc  : Test HandleSpatializationStateChange.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_008, TestSize.Level4)
{
    uint32_t spatializationStateCBIndex = 1;
    auto mockAudioSpatializationStateChangeCallback1 = std::make_shared<MockAudioSpatializationStateChangeCallback>();
    sSpatializationService_->spatializationStateCBMap_.emplace(spatializationStateCBIndex,
        std::make_pair(mockAudioSpatializationStateChangeCallback1, StreamUsage::STREAM_USAGE_MEDIA));
    spatializationStateCBIndex = 2;
    auto mockAudioSpatializationStateChangeCallback2 = std::make_shared<MockAudioSpatializationStateChangeCallback>();
    sSpatializationService_->spatializationStateCBMap_.emplace(spatializationStateCBIndex,
        std::make_pair(mockAudioSpatializationStateChangeCallback2, StreamUsage::STREAM_USAGE_GAME));
    EXPECT_NO_THROW(sSpatializationService_->HandleSpatializationStateChange(true));
    EXPECT_NO_THROW(sSpatializationService_->HandleSpatializationStateChange(false));
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_009
* @tc.desc  : Test UpdateHeadTrackingDeviceState
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_009, TestSize.Level4)
{
    bool outputDeviceChange = true;
    std::string preDeviceAddress = TEST_MAC_ADDRESS;
    sSpatializationService_->isHeadTrackingDataRequested_ = true;
    EXPECT_NO_THROW(sSpatializationService_->UpdateHeadTrackingDeviceState(outputDeviceChange, preDeviceAddress));
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_010
* @tc.desc  : Test UpdateHeadTrackingDeviceState
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_010, TestSize.Level4)
{
    bool outputDeviceChange = false;
    std::string preDeviceAddress = "";
    sSpatializationService_->isHeadTrackingDataRequested_ = true;
    sSpatializationService_->currentDevice_.macAddress_ = TEST_MAC_ADDRESS;
    EXPECT_NO_THROW(sSpatializationService_->UpdateHeadTrackingDeviceState(outputDeviceChange, preDeviceAddress));
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_010
* @tc.desc  : Test SafeConvertToBool
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_012, TestSize.Level4)
{
    std::string token = "123456";
    std::string paramName = "test";
    EXPECT_EQ(sSpatializationService_->SafeConvertToBool(token, paramName), false);
}

/**
* @tc.name  : Test AudioSpatializationService.
* @tc.number: AudioSpatializationService_010
* @tc.desc  : Test SafeConvertToBool
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_011, TestSize.Level4)
{
    std::string token = "1";
    std::string paramName = "test";
    EXPECT_EQ(sSpatializationService_->SafeConvertToBool(token, paramName), true);
    token = "0";
    EXPECT_EQ(sSpatializationService_->SafeConvertToBool(token, paramName), false);
}

/**
* @tc.name  : Test UpdateCurrentDevice personalized spatialization rollback.
* @tc.number: AudioSpatializationService_013
* @tc.desc  : Test UpdateCurrentDevice with new device having personalizedSpatializationEnabled=true in map.
*             AudioAdapterManager::UpdatePersonalizedSpatializationState fails in test context (adapter is null),
*             so personalizedSpatializationEnabled should be rolled back to false and
*             HandlePersonalizedSpatializationEnabledChange triggered with false.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_013, TestSize.Level4)
{
    AudioDeviceDescriptor device1;
    device1.macAddress_ = "AA:BB:CC:DD:EE:01";
    AudioDeviceDescriptor device2;
    device2.macAddress_ = "AA:BB:CC:DD:EE:02";

    // Set current device to device1
    sSpatializationService_->currentDevice_ = device1;

    // Setup device2 in map with personalizedSpatializationEnabled=true
    std::string encryptedAddr2 = sSpatializationService_->GetSha256EncryptAddress(device2.macAddress_);
    AudioSpatializationState state2(false, false, false, true); // personalized=true
    sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr2] = state2;

    // Switch to device2 → adapter fails → rollback
    sSpatializationService_->UpdateCurrentDevice(device2);

    // Verify rollback: personalizedSpatializationEnabled should be false now
    EXPECT_EQ(sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr2].personalizedSpatializationEnabled,
        false);
}

/**
* @tc.name  : Test UpdateCurrentDevice personalized spatialization no rollback.
* @tc.number: AudioSpatializationService_014
* @tc.desc  : Test UpdateCurrentDevice with new device having personalizedSpatializationEnabled=false in map.
*             AudioAdapterManager::UpdatePersonalizedSpatializationState fails, but since currentPersonalizedState
*             is false, no rollback logic is triggered. personalizedSpatializationEnabled stays false.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_014, TestSize.Level4)
{
    AudioDeviceDescriptor device1;
    device1.macAddress_ = "AA:BB:CC:DD:EE:01";
    AudioDeviceDescriptor device2;
    device2.macAddress_ = "AA:BB:CC:DD:EE:02";

    sSpatializationService_->currentDevice_ = device1;

    // Setup device2 with personalizedSpatializationEnabled=false
    std::string encryptedAddr2 = sSpatializationService_->GetSha256EncryptAddress(device2.macAddress_);
    AudioSpatializationState state2(false, false, false, false); // personalized=false
    sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr2] = state2;

    sSpatializationService_->UpdateCurrentDevice(device2);

    // No rollback: stays false
    EXPECT_EQ(sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr2].personalizedSpatializationEnabled,
        false);
}

/**
* @tc.name  : Test UpdateCurrentDevice with device not in map.
* @tc.number: AudioSpatializationService_015
* @tc.desc  : Test UpdateCurrentDevice switching to a device that is NOT in addressToSpatialEnabledMap_.
*             currentPersonalizedState defaults to false. Adapter fails but no rollback triggered.
*             The new entry should be created in the map during the process.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_015, TestSize.Level4)
{
    AudioDeviceDescriptor device1;
    device1.macAddress_ = "AA:BB:CC:DD:EE:01";
    AudioDeviceDescriptor device2;
    device2.macAddress_ = "AA:BB:CC:DD:EE:02";

    sSpatializationService_->currentDevice_ = device1;
    // device2 is NOT in addressToSpatialEnabledMap_

    sSpatializationService_->UpdateCurrentDevice(device2);

    // Device not in map → currentPersonalizedState=false → no rollback
    std::string encryptedAddr2 = sSpatializationService_->GetSha256EncryptAddress(device2.macAddress_);
    // The map may or may not have the entry, but if it does, personalized should be false
    if (sSpatializationService_->addressToSpatialEnabledMap_.count(encryptedAddr2)) {
        EXPECT_EQ(
            sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr2].personalizedSpatializationEnabled,
            false);
    }
}

/**
* @tc.name  : Test UpdateCurrentDevice same device early return.
* @tc.number: AudioSpatializationService_016
* @tc.desc  : Test UpdateCurrentDevice with the same macAddress as current device.
*             Should hit early return (currentDevice_.macAddress_ == selectedAudioDevice.macAddress_)
*             and the personalized spatialization section is never reached.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_016, TestSize.Level4)
{
    AudioDeviceDescriptor device1;
    device1.macAddress_ = TEST_MAC_ADDRESS;
    sSpatializationService_->currentDevice_ = device1;

    // Setup map with personalized=true
    std::string encryptedAddr = sSpatializationService_->GetSha256EncryptAddress(TEST_MAC_ADDRESS);
    AudioSpatializationState state(false, false, false, true);
    sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr] = state;

    // Update with same device → early return, personalized state unchanged
    sSpatializationService_->UpdateCurrentDevice(device1);
    EXPECT_EQ(sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr].personalizedSpatializationEnabled,
        true);
}

// ==================== Unit tests for NotifyUpdatePersonalizedSpatializationStateResult ====================

/**
* @tc.name  : Test NotifyUpdatePersonalizedSpatializationStateResult with success.
* @tc.number: AudioSpatializationService_017
* @tc.desc  : Test NotifyUpdatePersonalizedSpatializationStateResult with ret=0 (success).
*             Should not change any state. personalizedSpatializationEnabled in map stays true.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_017, TestSize.Level4)
{
    AudioDeviceDescriptor device;
    device.macAddress_ = TEST_MAC_ADDRESS;
    sSpatializationService_->currentDevice_ = device;

    std::string encryptedAddr = sSpatializationService_->GetSha256EncryptAddress(TEST_MAC_ADDRESS);
    AudioSpatializationState state(false, false, false, true); // personalized=true
    sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr] = state;

    // ret=0 → no state change
    sSpatializationService_->NotifyUpdatePersonalizedSpatializationStateResult(0);
    EXPECT_EQ(sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr].personalizedSpatializationEnabled,
        true);
}

/**
* @tc.name  : Test NotifyUpdatePersonalizedSpatializationStateResult rollback on failure.
* @tc.number: AudioSpatializationService_018
* @tc.desc  : Test NotifyUpdatePersonalizedSpatializationStateResult with ret!=0 (failure)
*             and personalizedSpatializationEnabled=true in map.
*             Should roll back personalizedSpatializationEnabled to false and trigger
*             HandlePersonalizedSpatializationEnabledChange(device, false).
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_018, TestSize.Level4)
{
    AudioDeviceDescriptor device;
    device.macAddress_ = TEST_MAC_ADDRESS;
    sSpatializationService_->currentDevice_ = device;

    std::string encryptedAddr = sSpatializationService_->GetSha256EncryptAddress(TEST_MAC_ADDRESS);
    AudioSpatializationState state(false, false, false, true); // personalized=true
    sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr] = state;

    // ret!=0 + personalized=true → rollback to false
    sSpatializationService_->NotifyUpdatePersonalizedSpatializationStateResult(-1);
    EXPECT_EQ(sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr].personalizedSpatializationEnabled,
        false);
}

/**
* @tc.name  : Test NotifyUpdatePersonalizedSpatializationStateResult no rollback when already false.
* @tc.number: AudioSpatializationService_019
* @tc.desc  : Test NotifyUpdatePersonalizedSpatializationStateResult with ret!=0 (failure)
*             but personalizedSpatializationEnabled=false in map.
*             Should not trigger rollback or HandlePersonalizedSpatializationEnabledChange.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_019, TestSize.Level4)
{
    AudioDeviceDescriptor device;
    device.macAddress_ = TEST_MAC_ADDRESS;
    sSpatializationService_->currentDevice_ = device;

    std::string encryptedAddr = sSpatializationService_->GetSha256EncryptAddress(TEST_MAC_ADDRESS);
    AudioSpatializationState state(false, false, false, false); // personalized=false
    sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr] = state;

    // ret!=0 + personalized=false → no rollback
    sSpatializationService_->NotifyUpdatePersonalizedSpatializationStateResult(-1);
    EXPECT_EQ(sSpatializationService_->addressToSpatialEnabledMap_[encryptedAddr].personalizedSpatializationEnabled,
        false);
}

/**
* @tc.name  : Test NotifyUpdatePersonalizedSpatializationStateResult with empty macAddress.
* @tc.number: AudioSpatializationService_020
* @tc.desc  : Test NotifyUpdatePersonalizedSpatializationStateResult with ret!=0 and
*             currentDevice_.macAddress_ is empty. Should not crash, skip db write,
*             and skip rollback since map entry likely does not exist.
*/
HWTEST_F(AudioSpatializationServiceUnitTest, AudioSpatializationService_020, TestSize.Level4)
{
    AudioDeviceDescriptor device;
    device.macAddress_ = ""; // empty mac address
    sSpatializationService_->currentDevice_ = device;

    // ret!=0 + empty mac → should not crash
    EXPECT_NO_THROW(
        sSpatializationService_->NotifyUpdatePersonalizedSpatializationStateResult(-1));
}
} // namespace AudioStandard
} // namespace OHOS
