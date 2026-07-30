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

#include "default_router_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void DefaultRouterUnitTest::SetUpTestCase(void) {}
void DefaultRouterUnitTest::TearDownTestCase(void) {}
void DefaultRouterUnitTest::SetUp(void) {}
void DefaultRouterUnitTest::TearDown(void) {}

/**
 * @tc.name  : GetMediaRenderDevice_001
 * @tc.desc  : Test GetMediaRenderDevice with STREAM_USAGE_MEDIA
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetMediaRenderDevice_001, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetMediaRenderDevice(STREAM_USAGE_MEDIA, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetMediaRenderDevice_002
 * @tc.desc  : Test GetMediaRenderDevice with STREAM_USAGE_MUSIC
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetMediaRenderDevice_002, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetMediaRenderDevice(STREAM_USAGE_MUSIC, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallRenderDevice_001
 * @tc.desc  : Test GetCallRenderDevice with VOICE_COMMUNICATION
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetCallRenderDevice_001, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallRenderDevice_002
 * @tc.desc  : Test GetCallRenderDevice with VOICE_MODEM_COMMUNICATION
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetCallRenderDevice_002, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_MODEM_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallCaptureDevice_001
 * @tc.desc  : Test GetCallCaptureDevice with SOURCE_TYPE_MIC
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetCallCaptureDevice_001, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_MIC, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetRingRenderDevices_001
 * @tc.desc  : Test GetRingRenderDevices with RINGTONE
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetRingRenderDevices_001, TestSize.Level1)
{
    DefaultRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_RINGTONE, 0);
    EXPECT_GE(descs.size(), 1u);
}

/**
 * @tc.name  : GetRecordCaptureDevice_001
 * @tc.desc  : Test GetRecordCaptureDevice with SOURCE_TYPE_MIC
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetRecordCaptureDevice_001, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetRecordCaptureDevice(SOURCE_TYPE_MIC, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetToneRenderDevice_001
 * @tc.desc  : Test GetToneRenderDevice with STREAM_USAGE_MEDIA
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetToneRenderDevice_001, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetToneRenderDevice(STREAM_USAGE_MEDIA, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetRouterType_001
 * @tc.desc  : Test GetRouterType returns ROUTER_TYPE_DEFAULT
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetRouterType_001, TestSize.Level1)
{
    DefaultRouter router;
    EXPECT_EQ(router.GetRouterType(), ROUTER_TYPE_DEFAULT);
}

/**
 * @tc.name  : GetClassName_001
 * @tc.desc  : Test GetClassName returns correct name
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetClassName_001, TestSize.Level1)
{
    DefaultRouter router;
    EXPECT_EQ(router.GetClassName(), "default_router");
}

/**
 * @tc.name  : GetMediaRenderDevice_003
 * @tc.desc  : Test GetMediaRenderDevice with STREAM_USAGE_VOICE_COMMUNICATION
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetMediaRenderDevice_003, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetMediaRenderDevice(STREAM_USAGE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallRenderDevice_003
 * @tc.desc  : Test GetCallRenderDevice with STREAM_USAGE_VOICE_MESSAGE
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetCallRenderDevice_003, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetCallRenderDevice(STREAM_USAGE_VOICE_MESSAGE, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetCallCaptureDevice_002
 * @tc.desc  : Test GetCallCaptureDevice with SOURCE_TYPE_VOICE_COMMUNICATION
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetCallCaptureDevice_002, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetCallCaptureDevice(SOURCE_TYPE_VOICE_COMMUNICATION, 0);
    ASSERT_NE(dev, nullptr);
}

/**
 * @tc.name  : GetRingRenderDevices_002
 * @tc.desc  : Test GetRingRenderDevices with STREAM_USAGE_VOICE_RINGTONE
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetRingRenderDevices_002, TestSize.Level1)
{
    DefaultRouter router;
    auto descs = router.GetRingRenderDevices(STREAM_USAGE_VOICE_RINGTONE, 0);
    EXPECT_GE(descs.size(), 1u);
}

/**
 * @tc.name  : GetRecordCaptureDevice_002
 * @tc.desc  : Test GetRecordCaptureDevice with SOURCE_TYPE_VOICE_RECOGNITION
 * @tc.type  : FUNC
 */
HWTEST_F(DefaultRouterUnitTest, GetRecordCaptureDevice_002, TestSize.Level1)
{
    DefaultRouter router;
    auto dev = router.GetRecordCaptureDevice(SOURCE_TYPE_VOICE_RECOGNITION, 0);
    ASSERT_NE(dev, nullptr);
}

} // namespace AudioStandard
} // namespace OHOS
