/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "audio_hearing_aid_device_behavior_unit_test.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioHearingAidDeviceBehaviorUnitTest::SetUpTestCase(void) {}
void AudioHearingAidDeviceBehaviorUnitTest::TearDownTestCase(void) {}
void AudioHearingAidDeviceBehaviorUnitTest::SetUp(void) {}
void AudioHearingAidDeviceBehaviorUnitTest::TearDown(void) {}

/**
 * @tc.name  : ActivateDevice_001
 * @tc.desc  : Test ActivateDevice with nullptr streamDesc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioHearingAidDeviceBehaviorUnitTest, ActivateDevice_001, TestSize.Level1)
{
    AudioHearingAidDeviceBehavior behavior;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(nullptr, reason);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : ActivateDevice_002
 * @tc.desc  : Test ActivateDevice with empty newDeviceDescs_
 * @tc.type  : FUNC
 */
HWTEST_F(AudioHearingAidDeviceBehaviorUnitTest, ActivateDevice_002, TestSize.Level1)
{
    AudioHearingAidDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : ActivateDevice_003
 * @tc.desc  : Test ActivateDevice with nullptr deviceDesc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioHearingAidDeviceBehaviorUnitTest, ActivateDevice_003, TestSize.Level1)
{
    AudioHearingAidDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : ActivateDevice_004
 * @tc.desc  : Test ActivateDevice with non-existent hearing aid device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioHearingAidDeviceBehaviorUnitTest, ActivateDevice_004, TestSize.Level1)
{
    AudioHearingAidDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_HEARING_AID;
    deviceDesc->macAddress_ = "non_existent_mac";
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : DeactivateDevice_001
 * @tc.desc  : Test DeactivateDevice without parameter
 * @tc.type  : FUNC
 */
HWTEST_F(AudioHearingAidDeviceBehaviorUnitTest, DeactivateDevice_001, TestSize.Level1)
{
    AudioHearingAidDeviceBehavior behavior;
    int32_t result = behavior.DeactivateDevice();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : DeactivateDevice_002
 * @tc.desc  : Test DeactivateDevice with device descriptor
 * @tc.type  : FUNC
 */
HWTEST_F(AudioHearingAidDeviceBehaviorUnitTest, DeactivateDevice_002, TestSize.Level1)
{
    AudioHearingAidDeviceBehavior behavior;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    int32_t result = behavior.DeactivateDevice(deviceDesc);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : ActivateDevice_005
 * @tc.desc  : Test ActivateDevice with HEARING_AID device type that exists in A2DP map
 * @tc.type  : FUNC
 */
HWTEST_F(AudioHearingAidDeviceBehaviorUnitTest, ActivateDevice_005, TestSize.Level1)
{
    AudioHearingAidDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_HEARING_AID;
    deviceDesc->macAddress_ = "00:11:22:33:44:55";
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : ActivateDevice_006
 * @tc.desc  : Test ActivateDevice with empty stream info (samplingRate, channelLayout, channels)
 * @tc.type  : FUNC
 */
HWTEST_F(AudioHearingAidDeviceBehaviorUnitTest, ActivateDevice_006, TestSize.Level1)
{
    AudioHearingAidDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_HEARING_AID;
    deviceDesc->macAddress_ = "00:11:22:33:44:55";
    DeviceStreamInfo emptyStreamInfo;
    emptyStreamInfo.samplingRate = {};
    emptyStreamInfo.channelLayout = {};
    deviceDesc->audioStreamInfo_.push_back(emptyStreamInfo);
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, ERR_NULL_POINTER);
}

} // namespace AudioStandard
} // namespace OHOS
