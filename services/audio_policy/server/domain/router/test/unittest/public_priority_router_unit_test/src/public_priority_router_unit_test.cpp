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

#include "public_priority_router_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <memory>
#include <thread>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const static int32_t TEST_CLIENT_UID = 1000;

void PublicPriorityRouterUnitTest::SetUpTestCase(void) {}
void PublicPriorityRouterUnitTest::TearDownTestCase(void) {}
void PublicPriorityRouterUnitTest::SetUp(void) {}
void PublicPriorityRouterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_001
 * @tc.desc  : Test PublicPriorityRouter GetMediaRenderDevice interface.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_001, TestSize.Level4)
{
    PublicPriorityRouter router;
    auto ret = router.GetMediaRenderDevice(STREAM_USAGE_RINGTONE, 101);
    EXPECT_NE(ret, nullptr);
    ret = router.GetMediaRenderDevice(STREAM_USAGE_VOICE_RINGTONE, 102);
    EXPECT_NE(ret, nullptr);
    ret = router.GetMediaRenderDevice(STREAM_USAGE_MEDIA, 103);
    EXPECT_NE(ret, nullptr);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_002
 * @tc.desc  : Test PublicPriorityRouter GetMediaRenderDevice interface.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_002, TestSize.Level4)
{
    PublicPriorityRouter router;
    auto ret = router.GetRingRenderDevices(STREAM_USAGE_MEDIA, 100);
    ASSERT_EQ(ret.size(), 1u);
    ASSERT_NE(ret[0], nullptr);
    ret = router.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 101);
    ASSERT_EQ(ret.size(), 1u);
    ASSERT_NE(ret[0], nullptr);
    ret = router.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, 102);
    ASSERT_EQ(ret.size(), 1u);
    ASSERT_NE(ret[0], nullptr);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_003
 * @tc.desc  : Test GetMediaRenderDevice interface via descs.size() != 0.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_003, TestSize.Level4)
{
    PublicPriorityRouter router;
    StreamUsage streamUsage  = STREAM_USAGE_RINGTONE;
    int32_t clientUID = TEST_CLIENT_UID;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    AudioDeviceManager::GetAudioDeviceManager().commRenderPublicDevices_ = { desc };
    auto descriptor = router.GetMediaRenderDevice(streamUsage, clientUID);
    EXPECT_NE(descriptor, nullptr);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_004
 * @tc.desc  : Test GetMediaRenderDevice interface via latestConnDesc->getType() != DEVICE_TYPE_NONE.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_004, TestSize.Level4)
{
    PublicPriorityRouter router;
    StreamUsage streamUsage  = STREAM_USAGE_UNKNOWN;
    int32_t clientUID = TEST_CLIENT_UID;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->deviceCategory_ = BT_SOUNDBOX;
    AudioDeviceManager::GetAudioDeviceManager().mediaRenderPublicDevices_ = { desc };
    auto descriptors = router.GetRingRenderDevices(streamUsage, clientUID);
    EXPECT_EQ(descriptors.front()->deviceType_, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_005
 * @tc.desc  : Test GetMediaRenderDevice interface via
 *             NeedLatestConnectWithDefaultDevices(latestConnDesc->getType()) == false.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_005, TestSize.Level4)
{
    PublicPriorityRouter router;
    StreamUsage streamUsage  = STREAM_USAGE_UNKNOWN;
    int32_t clientUID = TEST_CLIENT_UID;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_INVALID;
    desc->deviceCategory_ = BT_SOUNDBOX;
    AudioDeviceManager::GetAudioDeviceManager().mediaRenderPublicDevices_ = { desc };
    auto descriptors = router.GetRingRenderDevices(streamUsage, clientUID);
    EXPECT_EQ(descriptors.front()->deviceType_, DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_006
 * @tc.desc  : Test GetMediaRenderDevice interface via streamUsage == STREAM_USAGE_ALARM.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_006, TestSize.Level4)
{
    PublicPriorityRouter router;
    StreamUsage streamUsage  = STREAM_USAGE_ALARM;
    int32_t clientUID = TEST_CLIENT_UID;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->deviceCategory_ = CATEGORY_DEFAULT;
    AudioDeviceManager::GetAudioDeviceManager().mediaRenderPublicDevices_ = { desc };
    auto descriptors = router.GetRingRenderDevices(streamUsage, clientUID);
    EXPECT_EQ(descriptors.size(), 2);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_007
 * @tc.desc  : Test GetMediaRenderDevice interface via streamUsage == STREAM_USAGE_RINGTONE.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_007, TestSize.Level4)
{
    PublicPriorityRouter router;
    StreamUsage streamUsage  = STREAM_USAGE_RINGTONE;
    int32_t clientUID = TEST_CLIENT_UID;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    desc->deviceCategory_ = BT_CAR;
    AudioDeviceManager::GetAudioDeviceManager().commRenderPublicDevices_ = { desc };
    router.audioPolicyManager_.SetRingerMode(RINGER_MODE_NORMAL);
    auto descriptors = router.GetRingRenderDevices(streamUsage, clientUID);
    EXPECT_EQ(descriptors.size(), 2);

    router.audioPolicyManager_.SetRingerMode(RINGER_MODE_SILENT);
    descriptors = router.GetRingRenderDevices(streamUsage, clientUID);
    EXPECT_EQ(descriptors.size(), 1);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_008
 * @tc.desc  : Test GetMediaRenderDevice interface via streamUsage == default.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_008, TestSize.Level4)
{
    PublicPriorityRouter router;
    StreamUsage streamUsage  = STREAM_USAGE_UNKNOWN;
    int32_t clientUID = TEST_CLIENT_UID;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_INVALID;
    AudioDeviceManager::GetAudioDeviceManager().mediaRenderPublicDevices_ = { desc };
    auto descriptors = router.GetRingRenderDevices(streamUsage, clientUID);
    EXPECT_EQ(descriptors.front()->deviceType_, DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_009
 * @tc.desc  : Test GetMediaRenderDevice interface via streamUsage == default.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_009, TestSize.Level4)
{
    PublicPriorityRouter router;
    router.audioPolicyManager_.HandleCastingConnection();
    StreamUsage streamUsage  = STREAM_USAGE_ALARM;
    int32_t clientUID = TEST_CLIENT_UID;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_DP;
    AudioDeviceManager::GetAudioDeviceManager().mediaRenderPublicDevices_ = { desc };
    auto descriptors = router.GetRingRenderDevices(streamUsage, clientUID);
    EXPECT_EQ(descriptors.empty(), true);
}
} // namespace AudioStandard
} // namespace OHOS
