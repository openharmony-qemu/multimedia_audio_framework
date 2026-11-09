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

#include "audio_device_factory_unit_test.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioDeviceFactoryUnitTest::SetUpTestCase(void) {}
void AudioDeviceFactoryUnitTest::TearDownTestCase(void) {}
void AudioDeviceFactoryUnitTest::SetUp(void) {}
void AudioDeviceFactoryUnitTest::TearDown(void) {}

/**
 * @tc.name  : GetInstance_001
 * @tc.desc  : Test GetInstance returns same instance
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, GetInstance_001, TestSize.Level1)
{
    auto &instance1 = AudioDeviceFactory::GetInstance();
    auto &instance2 = AudioDeviceFactory::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name  : ActivateDevice_001
 * @tc.desc  : Test ActivateDevice with nullptr streamDesc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_001, TestSize.Level1)
{
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(nullptr, reason);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : ActivateDevice_002
 * @tc.desc  : Test ActivateDevice with empty newDeviceDescs_
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_002, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : ActivateDevice_003
 * @tc.desc  : Test ActivateDevice with nullptr deviceDesc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_003, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->newDeviceDescs_.push_back(nullptr);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, ERR_NULL_POINTER);
}

/**
 * @tc.name  : ActivateDevice_004
 * @tc.desc  : Test ActivateDevice with SPEAKER device (default case returns nullptr -> SUCCESS)
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_004, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : DeactivateDevice_001
 * @tc.desc  : Test DeactivateDevice with DEVICE_TYPE_SPEAKER
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, DeactivateDevice_001, TestSize.Level1)
{
    int32_t result = AudioDeviceFactory::GetInstance().DeactivateDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : DeactivateDevice_002
 * @tc.desc  : Test DeactivateDevice with empty deviceDescs vector
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, DeactivateDevice_002, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    int32_t result = AudioDeviceFactory::GetInstance().DeactivateDevice(deviceDescs);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : DeactivateDevice_003
 * @tc.desc  : Test DeactivateDevice with valid deviceDescs vector
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, DeactivateDevice_003, TestSize.Level1)
{
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDesc->deviceId_ = 1;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs = {deviceDesc};
    int32_t result = AudioDeviceFactory::GetInstance().DeactivateDevice(deviceDescs);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : ActivateDevice_005
 * @tc.desc  : Test ActivateDevice with DEVICE_TYPE_BLUETOOTH_SCO
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_005, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : ActivateDevice_006
 * @tc.desc  : Test ActivateDevice with DEVICE_TYPE_BLUETOOTH_A2DP
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_006, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : ActivateDevice_007
 * @tc.desc  : Test ActivateDevice with DEVICE_TYPE_NEARLINK
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_007, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_NEARLINK;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : ActivateDevice_008
 * @tc.desc  : Test ActivateDevice with DEVICE_TYPE_USB_ARM_HEADSET
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_008, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : ActivateDevice_009
 * @tc.desc  : Test ActivateDevice with DEVICE_TYPE_HEARING_AID
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_009, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_HEARING_AID;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : ActivateDevice_010
 * @tc.desc  : Test ActivateDevice with STREAM_STATUS_STARTED and NEARLINK triggers conflict handling
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, ActivateDevice_010, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_NEARLINK;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    streamDesc->streamStatus_ = STREAM_STATUS_STARTED;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t result = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_NE(result, SUCCESS);
}

/**
 * @tc.name  : DeactivateDevice_004
 * @tc.desc  : Test DeactivateDevice with device type that was activated
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, DeactivateDevice_004, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t activateResult = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_EQ(activateResult, SUCCESS);
    int32_t deactivateResult = AudioDeviceFactory::GetInstance().DeactivateDevice(DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(deactivateResult, SUCCESS);
}

/**
 * @tc.name  : DeactivateDevice_005
 * @tc.desc  : Test DeactivateDevice with vector containing activated device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioDeviceFactoryUnitTest, DeactivateDevice_005, TestSize.Level1)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceDesc->deviceId_ = 100;
    streamDesc->newDeviceDescs_.push_back(deviceDesc);
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    int32_t activateResult = AudioDeviceFactory::GetInstance().ActivateDevice(streamDesc, reason);
    EXPECT_NE(activateResult, SUCCESS);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs = {deviceDesc};
    int32_t deactivateResult = AudioDeviceFactory::GetInstance().DeactivateDevice(deviceDescs);
    EXPECT_EQ(deactivateResult, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
