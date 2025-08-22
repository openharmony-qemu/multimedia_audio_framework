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

#include "pair_device_router_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <memory>
#include <thread>
#include <vector>

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void PairDeviceRouterUnitTest::SetUpTestCase(void) {}
void PairDeviceRouterUnitTest::TearDownTestCase(void) {}
void PairDeviceRouterUnitTest::SetUp(void) {}
void PairDeviceRouterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_001
 * @tc.desc  : Test PairDeviceRouter GetMediaRenderDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_001, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetMediaRenderDevice(STREAM_USAGE_MEDIA, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_002
 * @tc.desc  : Test PairDeviceRouter GetCallCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_002, TestSize.Level4)
{
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::CONNECTED;
    pairDev->exceptionFlag_ = true;
    pairDev->isEnable_ = true;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 101, 201);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_003
 * @tc.desc  : Test PairDeviceRouter GetCallCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_003, TestSize.Level4)
{
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::CONNECTED;
    pairDev->exceptionFlag_ = true;
    pairDev->isEnable_ = false;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 102, 202);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_004
 * @tc.desc  : Test PairDeviceRouter GetCallCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_004, TestSize.Level4)
{
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::CONNECTED;
    pairDev->exceptionFlag_ = false;
    pairDev->isEnable_ = true;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 103, 203);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_005
 * @tc.desc  : Test PairDeviceRouter GetCallCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_005, TestSize.Level4)
{
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::CONNECTED;
    pairDev->exceptionFlag_ = false;
    pairDev->isEnable_ = false;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 104, 104);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_006
 * @tc.desc  : Test PairDeviceRouter GetCallCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_006, TestSize.Level4)
{
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::SUSPEND_CONNECTED;
    pairDev->exceptionFlag_ = true;
    pairDev->isEnable_ = true;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 105, 205);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_007
 * @tc.desc  : Test PairDeviceRouter GetCallCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_007, TestSize.Level4)
{
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::SUSPEND_CONNECTED;
    pairDev->exceptionFlag_ = true;
    pairDev->isEnable_ = false;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 106, 206);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_008
 * @tc.desc  : Test PairDeviceRouter GetCallCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_008, TestSize.Level4)
{
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::SUSPEND_CONNECTED;
    pairDev->exceptionFlag_ = false;
    pairDev->isEnable_ = true;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 107, 207);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_009
 * @tc.desc  : Test PairDeviceRouter GetCallCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_009, TestSize.Level4)
{
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::SUSPEND_CONNECTED;
    pairDev->exceptionFlag_ = false;
    pairDev->isEnable_ = false;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_USB_ARM_HEADSET;

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 108, 208);
    ASSERT_NE(dev, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS
