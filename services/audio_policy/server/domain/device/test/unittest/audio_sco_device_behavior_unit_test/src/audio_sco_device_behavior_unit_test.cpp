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

#include "audio_sco_device_behavior_unit_test.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioScoDeviceBehaviorUnitTest::SetUpTestCase(void) {}
void AudioScoDeviceBehaviorUnitTest::TearDownTestCase(void) {}
void AudioScoDeviceBehaviorUnitTest::SetUp(void) {}
void AudioScoDeviceBehaviorUnitTest::TearDown(void) {}

/**
 * @tc.name  : ActivateDevice_001
 * @tc.desc  : Test ActivateDevice with nullptr streamDesc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, ActivateDevice_001, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(nullptr, reason);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : ActivateDevice_002
 * @tc.desc  : Test ActivateDevice with empty newDeviceDescs_
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, ActivateDevice_002, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : ActivateDevice_003
 * @tc.desc  : Test ActivateDevice with nullptr deviceDesc in newDeviceDescs_
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, ActivateDevice_003, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : ActivateDevice_004
 * @tc.desc  : Test ActivateDevice with playback mode and valid device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, ActivateDevice_004, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "00:11:22:33:44:55";
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : ActivateDevice_005
 * @tc.desc  : Test ActivateDevice with capture mode and valid device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, ActivateDevice_005, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "00:11:22:33:44:55";
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : DeactivateDevice_001
 * @tc.desc  : Test DeactivateDevice without parameter
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, DeactivateDevice_001, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    int32_t result = behavior.DeactivateDevice();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : DeactivateDevice_002
 * @tc.desc  : Test DeactivateDevice with device descriptor
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, DeactivateDevice_002, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "00:11:22:33:44:55";
    int32_t result = behavior.DeactivateDevice(deviceDesc);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : ActivateDevice_006
 * @tc.desc  : Test ActivateDevice with AUDIO_MODE_PLAYBACK success path
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, ActivateDevice_006, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "AA:BB:CC:DD:EE:FF";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, ERR_NULL_POINTER);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : ActivateDevice_007
 * @tc.desc  : Test ActivateDevice with AUDIO_MODE_RECORD success path
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, ActivateDevice_007, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "AA:BB:CC:DD:EE:FF";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    int32_t result = behavior.ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, ERR_NULL_POINTER);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : DeactivateDevice_003
 * @tc.desc  : Test DeactivateDevice() with active HFP device to cover ClearActiveHfpDevice path
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, DeactivateDevice_003, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "11:22:33:44:55:66";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->audioMode_ = AUDIO_MODE_PLAYBACK;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    behavior.ActivateDevice(streamDesc, reason);
    int32_t result = behavior.DeactivateDevice();
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : DeactivateDevice_004
 * @tc.desc  : Test DeactivateDevice(deviceDesc) with mac address
 * @tc.type  : FUNC
 */
HWTEST_F(AudioScoDeviceBehaviorUnitTest, DeactivateDevice_004, TestSize.Level1)
{
    AudioScoDeviceBehavior behavior;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->macAddress_ = "11:22:33:44:55:66";
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    int32_t result = behavior.DeactivateDevice(deviceDesc);
    EXPECT_EQ(result, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
