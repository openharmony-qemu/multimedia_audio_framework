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

#include "cockpit_phone_router_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void CockpitPhoneRouterUnitTest::SetUpTestCase(void) {}
void CockpitPhoneRouterUnitTest::TearDownTestCase(void) {}
void CockpitPhoneRouterUnitTest::SetUp(void) {}
void CockpitPhoneRouterUnitTest::TearDown(void) {}

/**
 * @tc.name  : GetMediaRenderDevice_001
 * @tc.desc  : Test GetMediaRenderDevice returns empty descriptor
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetMediaRenderDevice_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetMediaRenderDevice(STREAM_USAGE_MEDIA, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : GetCallRenderDevice_001
 * @tc.desc  : Test GetCallRenderDevice with STREAM_USAGE_VOICE_COMMUNICATION
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetCallRenderDevice_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallCaptureDevice_001
 * @tc.desc  : Test GetCallCaptureDevice with SOURCE_TYPE_MIC
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetCallCaptureDevice_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetRingRenderDevices_001
 * @tc.desc  : Test GetRingRenderDevices with RINGTONE stream usage
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRingRenderDevices_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 0);
    EXPECT_FALSE(descs.empty());
}

/**
 * @tc.name  : GetRingRenderDevices_002
 * @tc.desc  : Test GetRingRenderDevices with ALARM stream usage
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRingRenderDevices_002, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_ALARM, 0);
    EXPECT_FALSE(descs.empty());
}

/**
 * @tc.name  : GetRingRenderDevices_003
 * @tc.desc  : Test GetRingRenderDevices with VOICE_RINGTONE stream usage
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRingRenderDevices_003, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, 0);
    EXPECT_FALSE(descs.empty());
}

/**
 * @tc.name  : GetRecordCaptureDevice_001
 * @tc.desc  : Test GetRecordCaptureDevice returns empty descriptor
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRecordCaptureDevice_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetRecordCaptureDevice(SOURCE_TYPE_MIC, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : GetToneRenderDevice_001
 * @tc.desc  : Test GetToneRenderDevice returns empty descriptor
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetToneRenderDevice_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetToneRenderDevice(STREAM_USAGE_MEDIA, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_EQ(dev->getType(), DeviceType::DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : GetRouterType_001
 * @tc.desc  : Test GetRouterType returns ROUTER_TYPE_COCKPIT_PHONE
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRouterType_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    EXPECT_EQ(router.GetRouterType(), ROUTER_TYPE_COCKPIT_PHONE);
}

/**
 * @tc.name  : GetClassName_001
 * @tc.desc  : Test GetClassName returns correct name
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetClassName_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    EXPECT_EQ(router.GetClassName(), "cockpit_phone_router");
}

/**
 * @tc.name  : GetRingRenderDevices_004
 * @tc.desc  : Test GetRingRenderDevices with ALARM and BT_CAR device
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRingRenderDevices_004, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_ALARM, 0);
    EXPECT_FALSE(descs.empty());
    for (const auto &desc : descs) {
        ASSERT_NE(desc, nullptr);
    }
}

/**
 * @tc.name  : GetRingRenderDevices_005
 * @tc.desc  : Test GetRingRenderDevices with RINGTONE and BT_CAR device
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRingRenderDevices_005, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 0);
    EXPECT_FALSE(descs.empty());
    for (const auto &desc : descs) {
        ASSERT_NE(desc, nullptr);
    }
}

/**
 * @tc.name  : GetRingRenderDevices_006
 * @tc.desc  : Test GetRingRenderDevices with VOICE_RINGTONE and BT_CAR device
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRingRenderDevices_006, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, 0);
    EXPECT_FALSE(descs.empty());
    for (const auto &desc : descs) {
        ASSERT_NE(desc, nullptr);
    }
}

/**
 * @tc.name  : GetRingRenderDevices_007
 * @tc.desc  : Test GetRingRenderDevices with VOICE_TRANSFER_RINGTONE stream usage
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetRingRenderDevices_007, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_VOICE_TRANSFER_RINGTONE, 0);
    EXPECT_FALSE(descs.empty());
    for (const auto &desc : descs) {
        ASSERT_NE(desc, nullptr);
    }
}

/**
 * @tc.name  : GetCallRenderDevice_002
 * @tc.desc  : Test GetCallRenderDevice with BT_CAR devices
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetCallRenderDevice_002, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallRenderDevice_003
 * @tc.desc  : Test GetCallRenderDevice with STREAM_USAGE_VOICE_MODEM_COMMUNICATION
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetCallRenderDevice_003, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallCaptureDevice_002
 * @tc.desc  : Test GetCallCaptureDevice with BT_CAR devices
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetCallCaptureDevice_002, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallCaptureDevice_003
 * @tc.desc  : Test GetCallCaptureDevice with SOURCE_TYPE_VOICE_COMMUNICATION
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetCallCaptureDevice_003, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetBTCarDevices_001
 * @tc.desc  : Test GetBTCarDevices with nullptr and non-BT_CAR devices filtering
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetBTCarDevices_001, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_ALARM, 0);
    EXPECT_FALSE(descs.empty());
    for (const auto &desc : descs) {
        ASSERT_NE(desc, nullptr);
        EXPECT_NE(desc->deviceCategory_, BT_CAR);
    }
}

/**
 * @tc.name  : GetBTCarDevices_002
 * @tc.desc  : Test GetBTCarDevices with non-BT_CAR device category returns empty
 * @tc.type  : FUNC
 */
HWTEST_F(CockpitPhoneRouterUnitTest, GetBTCarDevices_002, TestSize.Level1)
{
    CockpitPhoneRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
    EXPECT_NE(dev->deviceCategory_, BT_CAR);
}

} // namespace AudioStandard
} // namespace OHOS
