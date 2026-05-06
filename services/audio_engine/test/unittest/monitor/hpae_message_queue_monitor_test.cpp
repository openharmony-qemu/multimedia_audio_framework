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

#include "gtest/gtest.h"
#include "hpae_message_queue_monitor.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeMessageQueueMonitorTest : public ::testing::Test {
protected:
    void SetUp();
    void TearDown();
};

void HpaeMessageQueueMonitorTest::SetUp()
{}

void HpaeMessageQueueMonitorTest::TearDown()
{}

/**
 * @tc.name    : ReportMessageQueueException_001
 * @tc.type    : FUNC
 * @tc.number  : ReportMessageQueueException_001
 * @tc.desc    : Test ReportMessageQueueException with each MessageQueueType value.
 */
HWTEST_F(HpaeMessageQueueMonitorTest, ReportMessageQueueException_001, TestSize.Level0)
{
    std::string func = "TestFunc";
    std::string error = "TestError";

    HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_MANAGER_TYPE, func, error);
    HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_RENDERER_MANAGER_TYPE, func, error);
    HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_CAPTURE_MANAGER_TYPE, func, error);
    HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_INNER_CAPTURE_MANAGER_TYPE, func, error);
    HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_OFFLOAD_MANAGER_TYPE, func, error);
    HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_NO_LOCK_QUEUE_TYPE, func, error);

    // No crash means success
    EXPECT_TRUE(true);
}

/**
 * @tc.name    : ReportMessageQueueException_002
 * @tc.type    : FUNC
 * @tc.number  : ReportMessageQueueException_002
 * @tc.desc    : Test ReportMessageQueueException with empty strings.
 */
HWTEST_F(HpaeMessageQueueMonitorTest, ReportMessageQueueException_002, TestSize.Level0)
{
    std::string func = "";
    std::string error = "";

    HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_MANAGER_TYPE, func, error);

    // No crash means success
    EXPECT_TRUE(true);
}

/**
 * @tc.name    : ReportMessageQueueException_003
 * @tc.type    : FUNC
 * @tc.number  : ReportMessageQueueException_003
 * @tc.desc    : Test ReportMessageQueueException with long strings.
 */
HWTEST_F(HpaeMessageQueueMonitorTest, ReportMessageQueueException_003, TestSize.Level0)
{
    std::string func(1024, 'A');
    std::string error(1024, 'B');

    HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_MANAGER_TYPE, func, error);

    // No crash means success
    EXPECT_TRUE(true);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
