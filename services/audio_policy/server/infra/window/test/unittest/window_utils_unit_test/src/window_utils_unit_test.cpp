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

#include "window_utils_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void WindowUtilsUnitTest::SetUpTestCase(void) {}
void WindowUtilsUnitTest::TearDownTestCase(void) {}
void WindowUtilsUnitTest::SetUp(void) {}
void WindowUtilsUnitTest::TearDown(void) {}

/**
 * @tc.name  : CheckWindowState_001
 * @tc.desc  : Test CheckWindowState with invalid pid (0), should return false.
 * @tc.type  : FUNC
 */
HWTEST_F(WindowUtilsUnitTest, CheckWindowState_001, TestSize.Level1)
{
    bool result = WindowUtils::CheckWindowState(0);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckWindowState_002
 * @tc.desc  : Test CheckWindowState with negative pid, should return false.
 * @tc.type  : FUNC
 */
HWTEST_F(WindowUtilsUnitTest, CheckWindowState_002, TestSize.Level1)
{
    bool result = WindowUtils::CheckWindowState(-1);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckWindowState_003
 * @tc.desc  : Test CheckWindowState with non-existent pid, should return false.
 * @tc.type  : FUNC
 */
HWTEST_F(WindowUtilsUnitTest, CheckWindowState_003, TestSize.Level1)
{
    bool result = WindowUtils::CheckWindowState(99999);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckWindowState_004
 * @tc.desc  : Test CheckWindowState with a typical valid pid value.
 * @tc.type  : FUNC
 */
HWTEST_F(WindowUtilsUnitTest, CheckWindowState_004, TestSize.Level1)
{
    bool result = WindowUtils::CheckWindowState(1000);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckWindowState_005
 * @tc.desc  : Test CheckWindowState with INT32_MAX pid, should return false.
 * @tc.type  : FUNC
 */
HWTEST_F(WindowUtilsUnitTest, CheckWindowState_005, TestSize.Level1)
{
    bool result = WindowUtils::CheckWindowState(INT32_MAX);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckWindowState_006
 * @tc.desc  : Test CheckWindowState with pid 1 (init process), should return false
 *             since init process has no window.
 * @tc.type  : FUNC
 */
HWTEST_F(WindowUtilsUnitTest, CheckWindowState_006, TestSize.Level1)
{
    bool result = WindowUtils::CheckWindowState(1);
    EXPECT_FALSE(result);
}

/**
 * @tc.name  : CheckWindowState_007
 * @tc.desc  : Test CheckWindowState called multiple times with same pid, should return
 *             consistent results.
 * @tc.type  : FUNC
 */
HWTEST_F(WindowUtilsUnitTest, CheckWindowState_007, TestSize.Level1)
{
    int32_t pid = 12345;
    bool result1 = WindowUtils::CheckWindowState(pid);
    bool result2 = WindowUtils::CheckWindowState(pid);
    EXPECT_EQ(result1, result2);
    EXPECT_FALSE(result1);
}

/**
 * @tc.name  : CheckWindowState_008
 * @tc.desc  : Test CheckWindowState with different pids consecutively, should return false
 *             for non-existent window processes.
 * @tc.type  : FUNC
 */
HWTEST_F(WindowUtilsUnitTest, CheckWindowState_008, TestSize.Level1)
{
    bool result1 = WindowUtils::CheckWindowState(100);
    bool result2 = WindowUtils::CheckWindowState(200);
    bool result3 = WindowUtils::CheckWindowState(300);
    EXPECT_FALSE(result1);
    EXPECT_FALSE(result2);
    EXPECT_FALSE(result3);
}

} // namespace AudioStandard
} // namespace OHOS
