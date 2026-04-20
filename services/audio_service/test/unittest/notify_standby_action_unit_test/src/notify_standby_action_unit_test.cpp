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

#include "notify_standby_action_unit_test.h"
#include "renderer_in_server.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void NotifyStandbyActionUnitTest::SetUpTestCase(void) {}
void NotifyStandbyActionUnitTest::TearDownTestCase(void) {}
void NotifyStandbyActionUnitTest::SetUp(void) {}
void NotifyStandbyActionUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test NotifyStandbyAction Exec
 * @tc.type  : FUNC
 * @tc.number: NotifyStandbyActionExec_001
 * @tc.desc  : Test NotifyStandbyAction Exec when isStandbyOn_ is true.
 */
HWTEST_F(NotifyStandbyActionUnitTest, NotifyStandbyActionExec_001, TestSize.Level1)
{
    uint32_t sessionId = 123;
    auto action = std::make_shared<NotifyStandbyAction>(sessionId, true);
    ASSERT_NE(nullptr, action);
    
    action->Exec();
}

/**
 * @tc.name  : Test NotifyStandbyAction Exec
 * @tc.type  : FUNC
 * @tc.number: NotifyStandbyActionExec_002
 * @tc.desc  : Test NotifyStandbyAction Exec when isStandbyOn_ is false.
 */
HWTEST_F(NotifyStandbyActionUnitTest, NotifyStandbyActionExec_002, TestSize.Level1)
{
    uint32_t sessionId = 456;
    auto action = std::make_shared<NotifyStandbyAction>(sessionId, false);
    ASSERT_NE(nullptr, action);
    
    action->Exec();
}

} // namespace AudioStandard
} // namespace OHOS