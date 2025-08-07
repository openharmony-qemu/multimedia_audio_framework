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

#include <thread>
#include <memory>
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
 * @tc.desc  : Test GetMediaRenderDevice interface.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_001, TestSize.Level3)
{
    auto publicPriorityRouter_ = std::make_shared<PublicPriorityRouter>();

    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;
    int32_t clientId = 1;
    auto desc = publicPriorityRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_EQ(desc != nullptr, true);

    streamUsage = STREAM_USAGE_VOICE_RINGTONE;
    desc = publicPriorityRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_EQ(desc != nullptr, true);

    streamUsage = STREAM_USAGE_ALARM;
    desc = publicPriorityRouter_->GetMediaRenderDevice(streamUsage, clientId);
    EXPECT_EQ(desc != nullptr, true);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_002
 * @tc.desc  : Test GetCallRenderDevice interface.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_002, TestSize.Level4)
{
    auto publicPriorityRouter_ = std::make_shared<PublicPriorityRouter>();

    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;
    int32_t clientId = 1;
    auto desc = publicPriorityRouter_->GetCallRenderDevice(streamUsage, clientId);
    EXPECT_EQ(desc != nullptr, true);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_003
 * @tc.desc  : Test GetCallCaptureDevice interface.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_003, TestSize.Level4)
{
    auto publicPriorityRouter_ = std::make_shared<PublicPriorityRouter>();

    SourceType sourceType = SOURCE_TYPE_VOICE_CALL;
    int32_t clientId = 1;
    uint32_t sessionID = 1;
    auto desc = publicPriorityRouter_->GetCallCaptureDevice(sourceType, clientId, sessionID);
    EXPECT_EQ(desc != nullptr, true);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_004
 * @tc.desc  : Test GetRingRenderDevices interface.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_004, TestSize.Level3)
{
    auto publicPriorityRouter_ = std::make_shared<PublicPriorityRouter>();

    StreamUsage streamUsage = STREAM_USAGE_VOICE_RINGTONE;
    int32_t clientId = 1;
    auto desc = publicPriorityRouter_->GetRingRenderDevices(streamUsage, clientId);
    EXPECT_EQ(1, desc.size());

    streamUsage = STREAM_USAGE_RINGTONE;
    desc = publicPriorityRouter_->GetRingRenderDevices(streamUsage, clientId);
    EXPECT_EQ(1, desc.size());

    streamUsage = STREAM_USAGE_ALARM;
    desc = publicPriorityRouter_->GetRingRenderDevices(streamUsage, clientId);
    EXPECT_EQ(1, desc.size());
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_005
 * @tc.desc  : Test GetRecordCaptureDevice interface.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_005, TestSize.Level4)
{
    auto publicPriorityRouter_ = std::make_shared<PublicPriorityRouter>();

    SourceType sourceType = SOURCE_TYPE_VOICE_CALL;
    int32_t clientId = 1;
    uint32_t sessionID = 1;
    auto desc = publicPriorityRouter_->GetRecordCaptureDevice(sourceType, clientId, sessionID);
    EXPECT_EQ(desc != nullptr, true);
}

/**
 * @tc.name  : Test PublicPriorityRouter.
 * @tc.number: PublicPriorityRouter_006
 * @tc.desc  : Test GetToneRenderDevice interface.
 */
HWTEST(PublicPriorityRouterUnitTest, PublicPriorityRouter_006, TestSize.Level3)
{
    auto publicPriorityRouter_ = std::make_shared<PublicPriorityRouter>();

    StreamUsage streamUsage = STREAM_USAGE_RINGTONE;
    int32_t clientId = 1;
    auto desc = publicPriorityRouter_->GetToneRenderDevice(streamUsage, clientId);
    EXPECT_EQ(desc != nullptr, true);
}
} // namespace AudioStandard
} // namespace OHOS
 