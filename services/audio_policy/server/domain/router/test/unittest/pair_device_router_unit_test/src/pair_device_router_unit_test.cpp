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

void PairDeviceRouterUnitTest::SetUp(void)
{
    systemOutputs_ = AudioRouterSelectStrategy::GetInstance().GetCurrentOutputDevice(SYSTEM_UID);
    connectedDevices_ = AudioDeviceManager::GetAudioDeviceManager().connectedDevices_;
}

void PairDeviceRouterUnitTest::TearDown(void)
{
    auto &selectStrategy = AudioRouterSelectStrategy::GetInstance();
    selectStrategy.OnAppDied(UID);
    selectStrategy.UpdateCurrentOutputDevice(SYSTEM_UID, systemOutputs_);

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = connectedDevices_;
}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetMediaRenderDevice_001
 * @tc.desc  : Test GetMediaRenderDevice with STREAM_USAGE_MEDIA.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetMediaRenderDevice_001, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetMediaRenderDevice(STREAM_USAGE_MEDIA, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetMediaRenderDevice.
 * @tc.number: GetMediaRenderDevice_002
 * @tc.desc  : Test GetMediaRenderDevice with STREAM_USAGE_MUSIC.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetMediaRenderDevice_002, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetMediaRenderDevice(STREAM_USAGE_MUSIC, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallRenderDevice.
 * @tc.number: GetCallRenderDevice_001
 * @tc.desc  : Test GetCallRenderDevice interface.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallRenderDevice_001, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_001
 * @tc.desc  : Test GetRingRenderDevices interface.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetRingRenderDevices_001, TestSize.Level4)
{
    PairDeviceRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 0);
    EXPECT_EQ(descs.size(), 0u);
}

/**
 * @tc.name  : Test GetRecordCaptureDevice.
 * @tc.number: GetRecordCaptureDevice_001
 * @tc.desc  : Test GetRecordCaptureDevice interface.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetRecordCaptureDevice_001, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetRecordCaptureDevice(SOURCE_TYPE_MIC, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetToneRenderDevice.
 * @tc.number: GetToneRenderDevice_001
 * @tc.desc  : Test GetToneRenderDevice interface.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetToneRenderDevice_001, TestSize.Level4)
{
    PairDeviceRouter router;
    auto dev = router.GetToneRenderDevice(STREAM_USAGE_MEDIA, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetRouterType.
 * @tc.number: GetRouterType_001
 * @tc.desc  : Test GetRouterType interface.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetRouterType_001, TestSize.Level4)
{
    PairDeviceRouter router;
    EXPECT_EQ(router.GetRouterType(), ROUTER_TYPE_PAIR_DEVICE);
}

/**
 * @tc.name  : Test GetClassName.
 * @tc.number: GetClassName_001
 * @tc.desc  : Test GetClassName interface.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetClassName_001, TestSize.Level4)
{
    PairDeviceRouter router;
    EXPECT_EQ(router.GetClassName(), "pair_device_router");
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_001
 * @tc.desc  : Test GetCallCaptureDevice when connected device not found, returns empty.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallCaptureDevice_001, TestSize.Level4)
{
    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = {};

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, UID, SESSION);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_002
 * @tc.desc  : Test GetCallCaptureDevice with nullptr pairDevice returns empty.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallCaptureDevice_002, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = 0;
    desc->pairDeviceDescriptor_ = nullptr;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = { desc };
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(UID, { desc });

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, UID, SESSION);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_003
 * @tc.desc  : Test GetCallCaptureDevice with pairDevice SIMULATED_CONNECTED returns empty.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallCaptureDevice_003, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = 0;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_MIC, DeviceRole::INPUT_DEVICE);
    pairDev->connectState_ = ConnectState::SIMULATED_CONNECTED;
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = { desc };
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(UID, { desc });

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, UID, SESSION);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_004
 * @tc.desc  : Test GetCallCaptureDevice with pairDevice SUSPEND_CONNECTED returns empty.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallCaptureDevice_004, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = 0;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::INPUT_DEVICE);
    pairDev->connectState_ = ConnectState::SUSPEND_CONNECTED;
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = { desc };
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(UID, { desc });

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, UID, SESSION);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_005
 * @tc.desc  : Test GetCallCaptureDevice with pairDevice exceptionFlag=true returns empty.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallCaptureDevice_005, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = 0;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::INPUT_DEVICE);
    pairDev->exceptionFlag_ = true;
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = { desc };
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(UID, { desc });

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, UID, SESSION);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_006
 * @tc.desc  : Test GetCallCaptureDevice with pairDevice disabled, returns empty.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallCaptureDevice_006, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = 0;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::INPUT_DEVICE);
    pairDev->isEnable_ = false;
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = { desc };
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(UID, { desc });

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, UID, SESSION);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_007
 * @tc.desc  : Test GetCallCaptureDevice with BLUETOOTH_SCO pairDevice.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallCaptureDevice_007, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = 0;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::INPUT_DEVICE);
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = { desc };
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(UID, { desc });

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, UID, SESSION);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(Bluetooth::AudioHfpManager::IsAudioScoStateConnect(),
        dev->getType() == DeviceType::DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
 * @tc.name  : Test GetCallCaptureDevice.
 * @tc.number: GetCallCaptureDevice_008
 * @tc.desc  : Test GetCallCaptureDevice with available non-SCO pairDevice returns pairDevice.
 */
HWTEST_F(PairDeviceRouterUnitTest, GetCallCaptureDevice_008, TestSize.Level4)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_WIRED_HEADSET, DeviceRole::OUTPUT_DEVICE);
    desc->deviceId_ = 0;
    auto pairDev = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_WIRED_HEADSET, DeviceRole::INPUT_DEVICE);
    desc->pairDeviceDescriptor_ = pairDev;

    AudioDeviceManager::GetAudioDeviceManager().connectedDevices_ = { desc };
    AudioRouterSelectStrategy::GetInstance().UpdateCurrentOutputDevice(UID, { desc });

    PairDeviceRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, UID, SESSION);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_WIRED_HEADSET);
}

} // namespace AudioStandard
} // namespace OHOS
