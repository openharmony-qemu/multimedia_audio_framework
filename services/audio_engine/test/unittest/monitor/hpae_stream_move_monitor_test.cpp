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
#include "hpae_stream_move_monitor.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeStreamMoveMonitorTest : public ::testing::Test {
protected:
    void SetUp();
    void TearDown();
};

void HpaeStreamMoveMonitorTest::SetUp()
{}

void HpaeStreamMoveMonitorTest::TearDown()
{}

/**
 * @tc.name    : ReportStreamMoveException_001
 * @tc.type    : FUNC
 * @tc.number  : ReportStreamMoveException_001
 * @tc.desc    : Test ReportStreamMoveException with normal parameters.
 */
HWTEST_F(HpaeStreamMoveMonitorTest, ReportStreamMoveException_001, TestSize.Level0)
{
    int32_t clientId = 1000;
    uint32_t sessionId = 2000;
    uint32_t streamType = 1;
    std::string srcName = "source_device";
    std::string desName = "dest_device";
    std::string error = "normal error";

    HpaeStreamMoveMonitor::ReportStreamMoveException(
        clientId, sessionId, streamType, srcName, desName, error);

    // No crash means success
    EXPECT_TRUE(true);
}

/**
 * @tc.name    : ReportStreamMoveException_002
 * @tc.type    : FUNC
 * @tc.number  : ReportStreamMoveException_002
 * @tc.desc    : Test ReportStreamMoveException with zero values.
 */
HWTEST_F(HpaeStreamMoveMonitorTest, ReportStreamMoveException_002, TestSize.Level0)
{
    int32_t clientId = 0;
    uint32_t sessionId = 0;
    uint32_t streamType = 0;
    std::string srcName = "src";
    std::string desName = "des";
    std::string error = "error";

    HpaeStreamMoveMonitor::ReportStreamMoveException(
        clientId, sessionId, streamType, srcName, desName, error);

    // No crash means success
    EXPECT_TRUE(true);
}

/**
 * @tc.name    : ReportStreamMoveException_003
 * @tc.type    : FUNC
 * @tc.number  : ReportStreamMoveException_003
 * @tc.desc    : Test ReportStreamMoveException with UINT32_MAX values.
 */
HWTEST_F(HpaeStreamMoveMonitorTest, ReportStreamMoveException_003, TestSize.Level0)
{
    int32_t clientId = INT32_MAX;
    uint32_t sessionId = UINT32_MAX;
    uint32_t streamType = UINT32_MAX;
    std::string srcName = "src";
    std::string desName = "des";
    std::string error = "error";

    HpaeStreamMoveMonitor::ReportStreamMoveException(
        clientId, sessionId, streamType, srcName, desName, error);

    // No crash means success
    EXPECT_TRUE(true);
}

/**
 * @tc.name    : ReportStreamMoveException_004
 * @tc.type    : FUNC
 * @tc.number  : ReportStreamMoveException_004
 * @tc.desc    : Test ReportStreamMoveException with empty strings.
 */
HWTEST_F(HpaeStreamMoveMonitorTest, ReportStreamMoveException_004, TestSize.Level0)
{
    int32_t clientId = 100;
    uint32_t sessionId = 200;
    uint32_t streamType = 1;
    std::string srcName = "";
    std::string desName = "";
    std::string error = "";

    HpaeStreamMoveMonitor::ReportStreamMoveException(
        clientId, sessionId, streamType, srcName, desName, error);

    // No crash means success
    EXPECT_TRUE(true);
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
