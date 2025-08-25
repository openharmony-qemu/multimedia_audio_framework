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

} // namespace AudioStandard
} // namespace OHOS
