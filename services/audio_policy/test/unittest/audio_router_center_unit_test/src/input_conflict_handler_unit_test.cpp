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

#include "audio_router_center_unit_test.h"
#include "input_conflict_handler.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include <gtest/gtest.h>
#include <memory>
#include <vector>

using namespace testing::ext;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {

/**
 * @tc.name  : Detect_DeviceNullptr
 * @tc.number: InputConflictHandler_Detect_001
 * @tc.desc  : Test Detect returns false when device is nullptr
 */
HWTEST(AudioRouterCenterUnitTest, InputConflictHandler_Detect_001, TestSize.Level1)
{
    InputConflictHandler handler;
    auto highestDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    handler.highestPriorityDevice_ = highestDevice;

    EXPECT_FALSE(handler.Detect(nullptr, ROUTER_TYPE_NONE));
}

/**
 * @tc.name  : Detect_HighestPriorityDeviceNullptr
 * @tc.number: InputConflictHandler_Detect_002
 * @tc.desc  : Test Detect returns false when highestPriorityDevice_ is nullptr
 */
HWTEST(AudioRouterCenterUnitTest, InputConflictHandler_Detect_002, TestSize.Level1)
{
    InputConflictHandler handler;
    handler.highestPriorityDevice_ = nullptr;

    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    EXPECT_FALSE(handler.Detect(device, ROUTER_TYPE_NONE));
}

/**
 * @tc.name  : Detect_OutputDevice
 * @tc.number: InputConflictHandler_Detect_003
 * @tc.desc  : Test Detect returns false when device role is OUTPUT_DEVICE
 */
HWTEST(AudioRouterCenterUnitTest, InputConflictHandler_Detect_003, TestSize.Level1)
{
    InputConflictHandler handler;
    auto highestDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    handler.highestPriorityDevice_ = highestDevice;

    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    EXPECT_FALSE(handler.Detect(device, ROUTER_TYPE_NONE));
}

/**
 * @tc.name  : Detect_CanUseConcurrentlyWithOtherDevice
 * @tc.number: InputConflictHandler_Detect_004
 * @tc.desc  : Test Detect returns false when device can use concurrently (USB_ARM_HEADSET)
 */
HWTEST(AudioRouterCenterUnitTest, InputConflictHandler_Detect_004, TestSize.Level1)
{
    InputConflictHandler handler;
    auto highestDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    handler.highestPriorityDevice_ = highestDevice;

    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_USB_ARM_HEADSET, INPUT_DEVICE);
    EXPECT_FALSE(handler.Detect(device, ROUTER_TYPE_NONE));
}

/**
 * @tc.name  : Detect_SameDeviceTypeAsHighest
 * @tc.number: InputConflictHandler_Detect_005
 * @tc.desc  : Test Detect returns false when device type is same as highestPriorityDevice
 */
HWTEST(AudioRouterCenterUnitTest, InputConflictHandler_Detect_005, TestSize.Level1)
{
    InputConflictHandler handler;
    auto highestDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    handler.highestPriorityDevice_ = highestDevice;

    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    EXPECT_FALSE(handler.Detect(device, ROUTER_TYPE_NONE));
}

/**
 * @tc.name  : Detect_DifferentDeviceTypeConflict
 * @tc.number: InputConflictHandler_Detect_006
 * @tc.desc  : Test Detect returns true when device type differs from highestPriorityDevice
 */
HWTEST(AudioRouterCenterUnitTest, InputConflictHandler_Detect_006, TestSize.Level1)
{
    InputConflictHandler handler;
    auto highestDevice = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    handler.highestPriorityDevice_ = highestDevice;

    auto device = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_WIRED_HEADSET, INPUT_DEVICE);
    EXPECT_TRUE(handler.Detect(device, ROUTER_TYPE_NONE));
}

} // namespace AudioStandard
} // namespace OHOS
