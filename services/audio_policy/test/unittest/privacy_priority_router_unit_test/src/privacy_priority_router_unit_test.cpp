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

#include "../include/privacy_priority_router_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void PrivacyPriorityRouterUnitTest::SetUpTestCase(void) {}
void PrivacyPriorityRouterUnitTest::TearDownTestCase(void) {}
void PrivacyPriorityRouterUnitTest::SetUp(void) {}
void PrivacyPriorityRouterUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: PrivacyPriorityRouter_001
 * @tc.desc  : Test NeedLatestConnectWithDefaultDevices interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, PrivacyPriorityRouter_001, TestSize.Level1)
{
    PrivacyPriorityRouter router;

    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_WIRED_HEADSET));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_WIRED_HEADPHONES));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_BLUETOOTH_SCO));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_USB_HEADSET));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_BLUETOOTH_A2DP));
    EXPECT_TRUE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_USB_ARM_HEADSET));
    EXPECT_FALSE(router.NeedLatestConnectWithDefaultDevices(DEVICE_TYPE_NONE));
}

/**
 * @tc.name  : Test PrivacyPriorityRouter.
 * @tc.number: PrivacyPriorityRouter_002
 * @tc.desc  : Test GetRingRenderDevices interface.
 */
HWTEST(PrivacyPriorityRouterUnitTest, PrivacyPriorityRouter_002, TestSize.Level1)
{
    PrivacyPriorityRouter router;

    EXPECT_EQ(1, router.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, 1).size());
    EXPECT_EQ(1, router.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 1).size());
    EXPECT_EQ(1, router.GetRingRenderDevices(STREAM_USAGE_ALARM, 1).size());
}
} // namespace AudioStandard
} // namespace OHOS
 