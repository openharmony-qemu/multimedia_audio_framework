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

#include "user_select_router_ext_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "router_base.h"
#include "audio_router_select_strategy.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void UserSelectRouterExtUnitTest::SetUpTestCase(void) {}
void UserSelectRouterExtUnitTest::TearDownTestCase(void) {}
void UserSelectRouterExtUnitTest::SetUp(void) {}
void UserSelectRouterExtUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_004
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRingRenderDevices_004, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    preferredDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        userSelectRouter.GetRingRenderDevices(STREAM_USAGE_RINGTONE, clientUID);
    EXPECT_EQ(descs.size(), 1);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_005
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRingRenderDevices_005, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    preferredDevice->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        userSelectRouter.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, clientUID);
    EXPECT_EQ(descs.size(), 1);
}

/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_006
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRingRenderDevices_006, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    preferredDevice->deviceType_ = DEVICE_TYPE_NONE;
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        userSelectRouter.GetRingRenderDevices(STREAM_USAGE_ALARM, clientUID);
    EXPECT_EQ(descs.size(), 1);
}
/**
 * @tc.name  : Test GetRingRenderDevices.
 * @tc.number: GetRingRenderDevices_007
 * @tc.desc  : GetRingRenderDevices.
 */
HWTEST(UserSelectRouterExtUnitTest, GetRingRenderDevices_007, TestSize.Level3)
{
    UserSelectRouter userSelectRouter;
    SourceType sourceType = SourceType::SOURCE_TYPE_VOICE_RECOGNITION;
    StreamUsage streamUsage = StreamUsage::STREAM_USAGE_VOICE_MESSAGE;
    int32_t clientUID = 1;
    uint32_t sessionID = 678;
    auto preferredDevice = std::make_shared<AudioDeviceDescriptor>();
    preferredDevice->deviceId_ = 0;
    preferredDevice->deviceUsage_ = VOICE;
    preferredDevice->deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        userSelectRouter.GetRingRenderDevices(STREAM_USAGE_ALARM, clientUID);
    EXPECT_EQ(descs.size(), 1);
}
} // namespace AudioStandard
} // namespace OHOS
