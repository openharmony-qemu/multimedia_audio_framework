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
* @tc.number: AudioSpatializationService_002
* @tc.desc  : Test SetCollaborativePlaybackEnabledForDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_002, TestSize.Level0)
{
    const std::shared_ptr<AudioDeviceDescriptor> audioDevice1 = std::make_shared<AudioDeviceDescriptor>();
    audioDevice1->macAddress_ = testAddr1;
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
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, false);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice1);
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, true);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice3);
    EXPECT_EQ(audioCollaborativeService_.isCollaborativeStateEnabled_, false);
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
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR], COLLABORATIVE_RESERVED);

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
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = true;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), ERR_OPERATION_FAILED);

    // current device is not in addressToCollaborativeEnabledMap_ but collaborative state is false
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = false;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), SUCCESS);
    
    // current device address is in addressToCollaborativeEnabledMap_ and collaborative state should be changed
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = false;
    audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_[TEST_MAC_ADDR] = COLLABORATIVE_OPENED;
    audioCollaborativeServicePtr_->curDeviceAddress_ = TEST_MAC_ADDR;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), ERR_OPERATION_FAILED);
    EXPECT_EQ(audioCollaborativeServicePtr_->GetRealCollaborativeState(), true);

    // current device address is in addressToCollaborativeEnabledMap_ and collaborative state should not be changed
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = true;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), SUCCESS);
}
} // AudioStandard
} // OHOS