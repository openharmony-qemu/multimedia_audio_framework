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
    std::vector<EffectChain> effectChains = {effectChain1, effectChain2, effectChain3, effectChain4};
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
    sSpatializationService_->addressToSpatialEnabledMap_.insert(std::make_pair(encryptedAddress, spatializationState));
    sSpatializationService_->currentDeviceAddress_ = selectedAudioDevice->macAddress_;
    bool enable = true;
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
    std::string macAddress = TEST_MAC_ADDRESS;
    sSpatializationService_->currSpatialDeviceType_ = EARPHONE_TYPE_OTHERS;
    sSpatializationService_->UpdateCurrentDevice(macAddress);
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
    sSpatializationService_->currentDeviceAddress_ = TEST_MAC_ADDRESS;
    EXPECT_NO_THROW(sSpatializationService_->UpdateHeadTrackingDeviceState(outputDeviceChange, preDeviceAddress));
}
} // namespace AudioStandard
} // namespace OHOS
