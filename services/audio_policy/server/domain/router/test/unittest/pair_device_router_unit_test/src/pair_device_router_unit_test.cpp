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
    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(static_cast<SourceType>(0), 108, 208);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_010
 * @tc.desc  : Test PairDeviceRouter GetCallRenderDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_010, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_011
 * @tc.desc  : Test PairDeviceRouter GetRingRenderDevices interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_011, TestSize.Level4)
{
    PairDeviceRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 0);
    EXPECT_EQ(descs.size(), 0u);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_012
 * @tc.desc  : Test PairDeviceRouter GetRecordCaptureDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_012, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetRecordCaptureDevice(SOURCE_TYPE_MIC, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_013
 * @tc.desc  : Test PairDeviceRouter GetToneRenderDevice interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_013, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetToneRenderDevice(STREAM_USAGE_MEDIA, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_014
 * @tc.desc  : Test PairDeviceRouter GetRouterType interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_014, TestSize.Level4)
{
    PairDeviceRouter router;
    EXPECT_EQ(router.GetRouterType(), ROUTER_TYPE_PAIR_DEVICE);
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_015
 * @tc.desc  : Test PairDeviceRouter GetClassName interface.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_015, TestSize.Level4)
{
    PairDeviceRouter router;
    EXPECT_EQ(router.GetClassName(), "pair_device_router");
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_016
 * @tc.desc  : Test GetCallCaptureDevice with valid pairDevice, isEnable=true, CONNECTED, no exception.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_016, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = 1;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::CONNECTED;
    pairDev->exceptionFlag_ = false;
    pairDev->isEnable_ = true;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.push_back(desc);

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, 200, 300);
    ASSERT_NE(dev, nullptr);
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_017
 * @tc.desc  : Test GetCallCaptureDevice with pairDevice SIMULATED_CONNECTED returns empty.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_017, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = 2;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::SIMULATED_CONNECTED;
    pairDev->exceptionFlag_ = false;
    pairDev->isEnable_ = true;
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.push_back(desc);

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, 201, 301);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_018
 * @tc.desc  : Test GetCallCaptureDevice with pairDevice disabled but isScoStateConnect=true.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_018, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = 3;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>();
    pairDev->connectState_ = ConnectState::CONNECTED;
    pairDev->exceptionFlag_ = false;
    pairDev->isEnable_ = false;
    pairDev->deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.push_back(desc);

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, 202, 302);
    ASSERT_NE(dev, nullptr);
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_019
 * @tc.desc  : Test GetCallCaptureDevice with nullptr pairDevice returns empty.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_019, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceId_ = 4;
    desc->pairDeviceDescriptor_ = nullptr;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.push_back(desc);

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, 203, 303);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_.clear();
}

/**
 * @tc.name  : Test PairDeviceRouter.
 * @tc.number: PairDeviceRouter_020
 * @tc.desc  : Test GetMediaRenderDevice with STREAM_USAGE_MUSIC.
 */
HWTEST(PairDeviceRouterUnitTest, PairDeviceRouter_020, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetMediaRenderDevice(STREAM_USAGE_MUSIC, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

} // namespace AudioStandard
} // namespace OHOS
