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
}
void AudioCollaborativeServiceUnitTest::TearDown(void)
{
    audioCollaborativeServicePtr_.reset();
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
    bool isSupported = audioCollaborativeService_.IsCollaborativePlaybackSupported();
    EXPECT_EQ(isSupported, false);
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
    EXPECT_EQ(ret, ERROR);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, true);
    EXPECT_EQ(ret, ERROR);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, false);
    EXPECT_EQ(ret, ERROR);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(audioDevice1, false);
    EXPECT_EQ(ret, ERROR);
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
    EXPECT_EQ(ret, ERROR);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice2, false);
    EXPECT_EQ(ret, ERROR);
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
    EXPECT_EQ(ret, ERROR);
    ret = audioCollaborativeService_.SetCollaborativePlaybackEnabledForDevice(AudioDevice2, false);
    EXPECT_EQ(ret, ERROR);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice1);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice2);
    audioCollaborativeService_.UpdateCurrentDevice(*AudioDevice3);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_005
* @tc.desc  : Test Init.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_005, TestSize.Level0)
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
* @tc.number: AudioCollaborativeService_006
* @tc.desc  : Test UpdateCurrentDevice.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_006, TestSize.Level0)
{
    AudioDeviceDescriptor descriptor1;
    descriptor1.macAddress_ = TEST_MAC_ADDR;
    descriptor1.deviceType_ = DEVICE_TYPE_EARPIECE;
    AudioDeviceDescriptor descriptor2;
    descriptor2.macAddress_ = EMPTY_MAC_ADDR;
    descriptor2.deviceType_ = DEVICE_TYPE_EARPIECE;
    audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_
        .insert(std::pair<std::string, bool>(TEST_MAC_ADDR, true));

    audioCollaborativeServicePtr_->UpdateCurrentDevice(descriptor1);
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_.size(), 0);
    descriptor1.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeServicePtr_->UpdateCurrentDevice(descriptor1);
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_.size(), 1);
    audioCollaborativeServicePtr_->UpdateCurrentDevice(descriptor2);
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_.size(), 1);
    descriptor1.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioCollaborativeServicePtr_->UpdateCurrentDevice(descriptor2);
    EXPECT_EQ(audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_.size(), 1);
}

/**
* @tc.name  : Test Su.
* @tc.number: AudioCollaborativeService_007
* @tc.desc  : Test UpdateCollaborativeStateReal.
*/
HWTEST_F(AudioCollaborativeServiceUnitTest, AudioCollaborativeService_007, TestSize.Level0)
{
    audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_ = false;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), ERROR);

    audioCollaborativeServicePtr_->isCollaborativePlaybackSupported_ = true;
    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = true;
    audioCollaborativeServicePtr_->addressToCollaborativeEnabledMap_
        .insert(std::pair<std::string, bool>(TEST_MAC_ADDR, true));
    audioCollaborativeServicePtr_->curDeviceAddress_ = EMPTY_MAC_ADDR;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(),
        ERR_OPERATION_FAILED);

    audioCollaborativeServicePtr_->curDeviceAddress_ = TEST_MAC_ADDR;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), ERR_OPERATION_FAILED);

    audioCollaborativeServicePtr_->isCollaborativeStateEnabled_ = true;
    EXPECT_EQ(audioCollaborativeServicePtr_->UpdateCollaborativeStateReal(), SUCCESS);
    EXPECT_EQ(audioCollaborativeServicePtr_->GetRealCollaborativeState(), true);
}
} // AudioStandard
} // OHOS