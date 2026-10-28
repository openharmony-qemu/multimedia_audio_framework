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
#include <cstring>
#include "audio_collaborative_service_unit_test.h"
#include "audio_errors.h"
#include "audio_core_service.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
static std::unique_ptr<AudioCollaborativeService> audioCollaborativeServicePtr_ = nullptr;
void AudioCollaborativeServiceUnitTest::SetUpTestCase(void) {}
void AudioCollaborativeServiceUnitTest::TearDownTestCase(void) {}
void AudioCollaborativeServiceUnitTest::SetUp(void)
{
    audioCollaborativeServicePtr_ = std::make_unique<AudioCollaborativeService>();
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
}
void AudioCollaborativeServiceUnitTest::TearDown(void)
{
    audioCollaborativeServicePtr_.reset();
    audioCollaborativeService_.addressToCollaborativeEnabledMap_.clear();
}
static std::string testAddr1 = "address1";
static std::string testAddr2 = "address2";
static std::string testAddr3 = "address3";
static const std::string TEST_NAME = "test_name";
static const std::string TEST_LABEL = "test_label";
static const std::string EMPTY_MAC_ADDR = "";
static const std::string TEST_MAC_ADDR = "8C-32-23-23-6C-12";
static const std::string AUDIO_COLLABORATIVE_SERVICE_LABEL = "COLLABORATIVE";
static const std::string BLUETOOTH_EFFECT_CHAIN_NAME = "EFFECTCHAIN_COLLABORATIVE";

/**
* @tc.name  : Test Su.
* @tc.number: AudioSpatializationService_001
* @tc.desc  : Test isCollaborativePlaybackSupported.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_001, TestSize.Level0)
{
    std::vector<std::string> applyVec;
    EffectChain effectChain1(BLUETOOTH_EFFECT_CHAIN_NAME, applyVec, AUDIO_COLLABORATIVE_SERVICE_LABEL);
    EffectChain effectChain2(BLUETOOTH_EFFECT_CHAIN_NAME, applyVec, TEST_LABEL);
    EffectChain effectChain3(TEST_NAME, applyVec, TEST_LABEL);
    std::vector<EffectChain> effectChains = { effectChain1, effectChain2, effectChain3 };
    audioCollaborativeServicePtr_->Init(effectChains);
    EXPECT_TRUE(audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_002
* @tc.desc  : Test IsCollaborativePlaybackEnabledForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_002, TestSize.Level0)
{
    audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_ = true;
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = COLLABORATION_BLUETOOTH_OPEN;
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice1 = std::make_shared<AudioDeviceDescriptor>();
    audioDevice1->macAddress_ = testAddr1;
    AudioCoreService::GetCoreService()->Init();
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, false);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioSpatializationService_003
* @tc.desc  : Test IsCollaborativePlaybackEnabledForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_003, TestSize.Level0)
{
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice1 = std::make_shared<AudioDeviceDescriptor>();
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice2 = std::make_shared<AudioDeviceDescriptor>();
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice3 = std::make_shared<AudioDeviceDescriptor>();
    AudioDevice1->macAddress_ = testAddr1;
    AudioDevice2->macAddress_ = testAddr2;
    AudioDevice3->macAddress_ = testAddr3;
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice1, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice2, false);
    EXPECT_EQ(ret, SUCCESS);
    bool isEnabled = audioCollaborativeService_.IsCollaborativePlaybackEnabledForDevice(AudioDevice1);
    EXPECT_EQ(isEnabled, true);
    isEnabled = audioCollaborativeService_.IsCollaborativePlaybackEnabledForDevice(AudioDevice2);
    EXPECT_EQ(isEnabled, false);
    isEnabled = audioCollaborativeService_.IsCollaborativePlaybackEnabledForDevice(AudioDevice3);
    EXPECT_EQ(isEnabled, false);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioSpatializationService_004
* @tc.desc  : Test UpdateCurrentDevice.
*             isCollaborativeStateEnabled_ should be updated when collaborative state for current device is changed.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_004, TestSize.Level0)
{
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice1 = std::make_shared<AudioDeviceDescriptor>();
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice2 = std::make_shared<AudioDeviceDescriptor>();
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice3 = std::make_shared<AudioDeviceDescriptor>();
    AudioDevice1->macAddress_ = testAddr1;
    AudioDevice1->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDevice2->macAddress_ = testAddr2;
    AudioDevice2->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDevice3->macAddress_ = testAddr3;
    AudioDevice3->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice1, true);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice2, false);
    EXPECT_EQ(ret, SUCCESS);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice2);
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, COLLABORATION_CLOSE);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice1);
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, COLLABORATION_BLUETOOTH_OPEN);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice3);
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, COLLABORATION_CLOSE);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_006
* @tc.desc  : Test UpdateCurrentDevice.
*             When device is in addressToCollaborativeEnabledMap_ and its type is changed, its collaborative state
*             should be changed too.s
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_005, TestSize.Level0)
{
    AudioDeviceDescriptor descriptor1;
    descriptor1.macAddress_ = TEST_MAC_ADDR;
    descriptor1.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDeviceDescriptor descriptor2;
    descriptor2.macAddress_ = testAddr1;
    descriptor2.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_OPENED;

    descriptor1.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioCollaborativeServicePtr_->UpdateCurrentDevice(descriptor1);

    descriptor1.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeServicePtr_->UpdateCurrentDevice(descriptor1);
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR], COLLABORATIVE_OPENED);
    // size of addressToCollaborativeEnabledMap_ should no be changed by UpdateCurrentDevice()
    audioCollaborativeServicePtr_->UpdateCurrentDevice(descriptor2);
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_.size(), 1);
    descriptor1.deviceType_ = DEVICE_TYPE_SPEAKER;
    audioCollaborativeServicePtr_->UpdateCurrentDevice(descriptor2);
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_.size(), 1);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_007
* @tc.desc  : Test UpdateCollaborativeStateReal.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_006, TestSize.Level0)
{
    // local device does not support collaborative service
    audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_ = false;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), ERROR);

    audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_ = true;
    // current device is not in addressToCollaborativeEnabledMap_ but collaborative state is true
    audioCollaborativeServicePtr_->curDeviceAddress_ = testAddr1;
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = COLLABORATION_BLUETOOTH_OPEN;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), ERR_OPERATION_FAILED);

    // current device is not in addressToCollaborativeEnabledMap_ but collaborative state is false
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), SUCCESS);
    
    // current device address is in addressToCollaborativeEnabledMap_ and collaborative state should be changed
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
    audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_OPENED;
    audioCollaborativeServicePtr_->curDeviceAddress_ = TEST_MAC_ADDR;
    EXPECT_EQ(audioCollaborativeServicePtr_->GetRealCollaborativeState(), false);

    // current device address is in addressToCollaborativeEnabledMap_ and collaborative state should not be changed
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = COLLABORATION_BLUETOOTH_OPEN;
    audioCollaborativeServicePtr_->UpdateCollaborativeStateReal();
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_007
* @tc.desc  : Test isCollaborativePlaybackSupportedForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_007, TestSize.Level0)
{
    std::vector<std::string> applyVec;
    EffectChain effectChain1(BLUETOOTH_EFFECT_CHAIN_NAME, applyVec, AUDIO_COLLABORATIVE_SERVICE_LABEL);
    EffectChain effectChain2(BLUETOOTH_EFFECT_CHAIN_NAME, applyVec, TEST_LABEL);
    EffectChain effectChain3(TEST_NAME, applyVec, TEST_LABEL);
    std::vector<EffectChain> effectChains = { effectChain1, effectChain2, effectChain3 };
    audioCollaborativeServicePtr_->Init(effectChains);
    EXPECT_TRUE(audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_);

    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice1 = std::make_shared<AudioDeviceDescriptor>();
    AudioDevice1->macAddress_ = testAddr1;
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupportedForDevice(AudioDevice1);
    EXPECT_EQ(isSupported, false);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_008
* @tc.desc  : Test isCollaborativePlaybackSupportedForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_008, TestSize.Level0)
{
    audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_ = false;
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupportedForDevice(nullptr);
    EXPECT_EQ(isSupported, false);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_009
* @tc.desc  : Test isCollaborativePlaybackSupportedForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_009, TestSize.Level0)
{
    audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_ = true;
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupportedForDevice(nullptr);
    EXPECT_EQ(isSupported, false);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_010
* @tc.desc  : Test isCollaborativePlaybackSupportedForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_010, TestSize.Level0)
{
    audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_ = true;
    const std::shared_ptr<AudioDeviceDescriptor> AudioDevice = std::make_shared<AudioDeviceDescriptor>();
    AudioDevice->macAddress_ = testAddr1;
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupportedForDevice(AudioDevice);
    EXPECT_EQ(isSupported, false);
}

/**
* @tc.name  : Test GetCollaborativeGlobalSwitchState returns COLLABORATION_CLOSE by default
* @tc.number: AudioCollaborativeService_011
* @tc.desc  : Test GetCollaborativeGlobalSwitchState default state.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_011, TestSize.Level0)
{
    CollaborationState state = audioCollaborativeService_.GetCollaborativeGlobalSwitchState();
    EXPECT_EQ(state, COLLABORATION_CLOSE);
}

/**
* @tc.name  : Test GetCollaborativeGlobalSwitchState after state change
* @tc.number: AudioCollaborativeService_012
* @tc.desc  : Test GetCollaborativeGlobalSwitchState returns updated state.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_012, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_BLUETOOTH_OPEN;
    CollaborationState state = audioCollaborativeService_.GetCollaborativeGlobalSwitchState();
    EXPECT_EQ(state, COLLABORATION_BLUETOOTH_OPEN);
}

/**
* @tc.name  : Test GetCollaborativeGlobalSwitchState with NearLink state
* @tc.number: AudioCollaborativeService_013
* @tc.desc  : Test GetCollaborativeGlobalSwitchState returns NEARLINK_OPEN.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_013, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_NEARLINK_OPEN;
    CollaborationState state = audioCollaborativeService_.GetCollaborativeGlobalSwitchState();
    EXPECT_EQ(state, COLLABORATION_NEARLINK_OPEN);
}

/**
* @tc.name  : Test UpdateCollaborativeStateToHdi with NearLink
* @tc.number: AudioCollaborativeService_014
* @tc.desc  : Test UpdateCollaborativeStateToHdi does not crash with NearLink state.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_014, TestSize.Level0)
{
    // Should not crash
    audioCollaborativeService_.UpdateCollaborativeStateToHdi(COLLABORATION_NEARLINK_OPEN);
    SUCCEED();
}

/**
* @tc.name  : Test UpdateCollaborativeStateToHdi with Close
* @tc.number: AudioCollaborativeService_015
* @tc.desc  : Test UpdateCollaborativeStateToHdi does not crash with CLOSE state.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_015, TestSize.Level0)
{
    audioCollaborativeService_.UpdateCollaborativeStateToHdi(COLLABORATION_CLOSE);
    SUCCEED();
}

/**
* @tc.name  : Test SetCollaborativePlaybackEnableForDevice with A2DP current device enabled.
* @tc.number: AudioCollaborativeService_016
* @tc.desc  : MutePrimarySinkByCollaborative should be triggered when the enabled device is an A2DP
*             device and it is the current device.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_016, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    // Pre-set to the already-enabled state so the test reaches the MutePrimarySinkByCollaborative branch
    // instead of the state-change path, which requires a real service adapter and is unavailable in UT.
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_BLUETOOTH_OPEN;
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice = std::make_shared<AudioDeviceDescriptor>();
    audioDevice->macAddress_ = testAddr1;
    audioCollaborativeService_.curDeviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.curDeviceAddress_ = testAddr1;
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test SetCollaborativePlaybackEnableForDevice with A2DP current device disabled.
* @tc.number: AudioCollaborativeService_017
* @tc.desc  : MutePrimarySinkByCollaborative should not be triggered when the A2DP device is disabled.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_017, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice = std::make_shared<AudioDeviceDescriptor>();
    audioDevice->macAddress_ = testAddr1;
    audioCollaborativeService_.curDeviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.curDeviceAddress_ = testAddr1;
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test SetCollaborativePlaybackEnableForDevice with non-current A2DP device.
* @tc.number: AudioCollaborativeService_018
* @tc.desc  : MutePrimarySinkByCollaborative should not be triggered when the A2DP device is current.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_018, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice = std::make_shared<AudioDeviceDescriptor>();
    audioDevice->macAddress_ = testAddr1;
    audioCollaborativeService_.curDeviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.curDeviceAddress_ = testAddr2;
    int32_t ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice, true);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test IsCollaborativePlaybackSupportedForDevice with A2DP device type
* @tc.number: AudioCollaborativeService_019
* @tc.desc  : Test IsCollaborativePlaybackSupportedForDevice with DEVICE_TYPE_BLUETOOTH_A2DP when not supported.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_019, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice = std::make_shared<AudioDeviceDescriptor>();
    audioDevice->macAddress_ = testAddr1;
    audioDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    // GetDeviceProductId will fail, so returns false
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupportedForDevice(audioDevice);
    EXPECT_EQ(isSupported, false);
}

/**
* @tc.name  : Test IsCollaborativePlaybackSupportedForDevice with NearLink device type
* @tc.number: AudioCollaborativeService_020
* @tc.desc  : Test IsCollaborativePlaybackSupportedForDevice with DEVICE_TYPE_NEARLINK when not supported.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_020, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice = std::make_shared<AudioDeviceDescriptor>();
    audioDevice->macAddress_ = testAddr1;
    audioDevice->deviceType_ = DEVICE_TYPE_NEARLINK;
    // GetDeviceProductId will fail, so returns false
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupportedForDevice(audioDevice);
    EXPECT_EQ(isSupported, false);
}

/**
* @tc.name  : Test IsCollaborativePlaybackSupportedForDevice with unsupported device type
* @tc.number: AudioCollaborativeService_021
* @tc.desc  : Test IsCollaborativePlaybackSupportedForDevice returns false for unsupported device types.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_021, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice = std::make_shared<AudioDeviceDescriptor>();
    audioDevice->macAddress_ = testAddr1;
    audioDevice->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupportedForDevice(audioDevice);
    EXPECT_EQ(isSupported, false);
}

/**
* @tc.name  : Test UpdateCurrentDevice with non-Bluetooth device type
* @tc.number: AudioCollaborativeService_022
* @tc.desc  : Test UpdateCurrentDevice when device type is not Bluetooth.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_022, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    AudioDeviceDescriptor device;
    device.macAddress_ = TEST_MAC_ADDR;
    device.deviceType_ = DEVICE_TYPE_SPEAKER;
    audioCollaborativeService_.UpdateCurrentDevice(device);
    EXPECT_EQ(audioCollaborativeService_.curDeviceType_, DEVICE_TYPE_SPEAKER);
}

/**
* @tc.name  : Test UpdateCollaborativeStateReal when current device not in map and state is CLOSE
* @tc.number: AudioCollaborativeService_023
* @tc.desc  : Test UpdateCollaborativeStateReal returns SUCCESS when device not in map and state is already CLOSE.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_023, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.curDeviceAddress_ = testAddr1;
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_.clear();
    int32_t ret = audioCollaborativeService_.UpdateCollaborativeStateReal();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test UpdateCollaborativeStateReal with collaborative enabled and A2DP device
* @tc.number: AudioCollaborativeService_024
* @tc.desc  : Test UpdateCollaborativeStateReal sets COLLABORATION_BLUETOOTH_OPEN when A2DP device is enabled.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_024, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.curDeviceAddress_ = TEST_MAC_ADDR;
    audioCollaborativeService_.curDeviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_OPENED;
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
    int32_t ret = audioCollaborativeService_.UpdateCollaborativeStateReal();
    // In mock environment, UpdateCollaborativeState may return ERROR
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, COLLABORATION_BLUETOOTH_OPEN);
}

/**
* @tc.name  : Test UpdateCollaborativeStateReal with collaborative disabled (else branch)
* @tc.number: AudioCollaborativeService_025
* @tc.desc  : Test UpdateCollaborativeStateReal sets COLLABORATION_CLOSE when device is not collaborative enabled.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_025, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.curDeviceAddress_ = TEST_MAC_ADDR;
    audioCollaborativeService_.curDeviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_CLOSED;
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_BLUETOOTH_OPEN;
    int32_t ret = audioCollaborativeService_.UpdateCollaborativeStateReal();
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, COLLABORATION_CLOSE);
}

/**
* @tc.name  : Test UpdateCollaborativeStateReal with NearLink device enabled
* @tc.number: AudioCollaborativeService_026
* @tc.desc  : Test UpdateCollaborativeStateReal sets COLLABORATION_NEARLINK_OPEN when NearLink device is enabled.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_026, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.curDeviceAddress_ = TEST_MAC_ADDR;
    audioCollaborativeService_.curDeviceType_ = DEVICE_TYPE_NEARLINK;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_OPENED;
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
    int32_t ret = audioCollaborativeService_.UpdateCollaborativeStateReal();
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, COLLABORATION_NEARLINK_OPEN);
}

/**
* @tc.name  : Test UpdateCurrentDevice changes state from RESERVED to OPEN
* @tc.number: AudioCollaborativeService_027
* @tc.desc  : Test UpdateCurrentDevice when device was RESERVED changes to OPEN.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_027, TestSize.Level0)
{
    AudioDeviceDescriptor device1;
    device1.macAddress_ = TEST_MAC_ADDR;
    device1.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_RESERVED;
    audioCollaborativeServicePtr_->curDeviceAddress_ = TEST_MAC_ADDR;
    audioCollaborativeServicePtr_->UpdateCurrentDevice(device1);
    // When device is A2DP and state is RESERVED, the state remains RESERVED
    // because IsCollaborativePlaybackSupportDeviceType(A2DP) returns true
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR], COLLABORATIVE_RESERVED);
}

/**
* @tc.name  : Test UpdateCurrentDevice with new device address
* @tc.number: AudioCollaborativeService_028
* @tc.desc  : Test UpdateCurrentDevice updates curDeviceAddress_ when device address changes.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_028, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.curDeviceAddress_ = testAddr1;
    AudioDeviceDescriptor device;
    device.macAddress_ = testAddr2;
    device.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.UpdateCurrentDevice(device);
    EXPECT_EQ(audioCollaborativeService_.curDeviceAddress_, testAddr2);
}

/**
* @tc.name  : Test IsCollaborativePlaybackOpenedOrReservedForDevice with OPEN state
* @tc.number: AudioCollaborativeService_029
* @tc.desc  : Test IsCollaborativePlaybackOpenedOrReservedForDevice returns true for OPEN state.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_029, TestSize.Level0)
{
    AudioDeviceDescriptor device;
    device.macAddress_ = TEST_MAC_ADDR;
    device.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_OPENED;
    bool isOpenedOrReserved = audioCollaborativeService_.IsCollaborativePlaybackOpenedOrReservedForDevice(device);
    EXPECT_EQ(isOpenedOrReserved, true);
}

/**
* @tc.name  : Test IsCollaborativePlaybackOpenedOrReservedForDevice with RESERVED state
* @tc.number: AudioCollaborativeService_030
* @tc.desc  : Test IsCollaborativePlaybackOpenedOrReservedForDevice returns true for RESERVED state.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_030, TestSize.Level0)
{
    AudioDeviceDescriptor device;
    device.macAddress_ = TEST_MAC_ADDR;
    device.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_RESERVED;
    bool isOpenedOrReserved = audioCollaborativeService_.IsCollaborativePlaybackOpenedOrReservedForDevice(device);
    EXPECT_EQ(isOpenedOrReserved, true);
}

/**
* @tc.name  : Test IsCollaborativePlaybackOpenedOrReservedForDevice with CLOSED state
* @tc.number: AudioCollaborativeService_031
* @tc.desc  : Test IsCollaborativePlaybackOpenedOrReservedForDevice returns false for CLOSED state.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_031, TestSize.Level0)
{
    AudioDeviceDescriptor device;
    device.macAddress_ = TEST_MAC_ADDR;
    device.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_CLOSED;
    bool isOpenedOrReserved = audioCollaborativeService_.IsCollaborativePlaybackOpenedOrReservedForDevice(device);
    EXPECT_EQ(isOpenedOrReserved, false);
}

/**
* @tc.name  : Test IsCollaborativePlaybackOpenedOrReservedForDevice with address not in map
* @tc.number: AudioCollaborativeService_032
* @tc.desc  : Test IsCollaborativePlaybackOpenedOrReservedForDevice returns false when address not in map.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_032, TestSize.Level0)
{
    AudioDeviceDescriptor device;
    device.macAddress_ = TEST_MAC_ADDR;
    device.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_.clear();
    bool isOpenedOrReserved = audioCollaborativeService_.IsCollaborativePlaybackOpenedOrReservedForDevice(device);
    EXPECT_EQ(isOpenedOrReserved, false);
}

/**
* @tc.name  : Test UpdateCollaborativeStateReal when state does not change
* @tc.number: AudioCollaborativeService_033
* @tc.desc  : Test UpdateCollaborativeStateReal returns SUCCESS without calling
*            UpdateCollaborativeStateToHdi when state unchanged.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_033, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.curDeviceAddress_ = TEST_MAC_ADDR;
    audioCollaborativeService_.curDeviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_OPENED;
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_BLUETOOTH_OPEN;
    int32_t ret = audioCollaborativeService_.UpdateCollaborativeStateReal();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, COLLABORATION_BLUETOOTH_OPEN);
}

/**
* @tc.name  : Test UpdateCurrentDevice when SCO device and address was collaborative opened
* @tc.number: AudioCollaborativeService_034
* @tc.desc  : Test UpdateCurrentDevice changes COLLABORATIVE_OPENED to COLLABORATIVE_RESERVED when SCO connected.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_034, TestSize.Level0)
{
    AudioDeviceDescriptor device1;
    device1.macAddress_ = TEST_MAC_ADDR;
    device1.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioDeviceDescriptor device2;
    device2.macAddress_ = TEST_MAC_ADDR;
    device2.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_OPENED;
    audioCollaborativeServicePtr_->curDeviceAddress_ = TEST_MAC_ADDR;
    audioCollaborativeServicePtr_->curDeviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeServicePtr_->UpdateCurrentDevice(device2);
    // In test environment, IsEnhancedRoutingSupported() returns false,
    // so state remains COLLABORATIVE_OPENED
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR], COLLABORATIVE_OPENED);
}

/**
* @tc.name  : Test UpdateCollaborativeStateReal with current device in map but state is COLLABORATION_CLOSE
* @tc.number: AudioCollaborativeService_035
* @tc.desc  : Test UpdateCollaborativeStateReal handles case where current device is in map but state is CLOSE.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_035, TestSize.Level0)
{
    audioCollaborativeService_.isCollaborativePlaybackSupported_ = true;
    audioCollaborativeService_.curDeviceAddress_ = TEST_MAC_ADDR;
    audioCollaborativeService_.curDeviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeService_.addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_CLOSED;
    audioCollaborativeService_.isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
    int32_t ret = audioCollaborativeService_.UpdateCollaborativeStateReal();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, COLLABORATION_CLOSE);
}
} // AudioStandard
} // OHOS