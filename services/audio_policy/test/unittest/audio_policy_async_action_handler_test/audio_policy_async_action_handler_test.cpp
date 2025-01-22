/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#include "audio_policy_async_action_handler.h"
#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static std::vector<int32_t> g_expectedParaVec;
static std::mutex actionMutex;
static const int32_t WAIT_FOR_ASYNC_ACTION_TIME_US = 100000; // 100ms
static const int32_t ASYNC_ACTION_EXEC_TIME_US = 5000; // 5ms

class AudioPolicyAsyncActionHandlerTest : public testing::Test {
public:
    // SetUp: Called before each test cases
    void SetUp(void) override
    {
        std::lock_guard<std::mutex> lock(actionMutex);
        g_expectedParaVec.clear();
    }
};

class PolicyAsyncTestAction : public PolicyAsyncAction {
public:
    explicit PolicyAsyncTestAction(int32_t para) : para_(para)
    {}

    void Exec() override
    {
        usleep(ASYNC_ACTION_EXEC_TIME_US);
        std::lock_guard<std::mutex> lock(actionMutex);
        g_expectedParaVec.push_back(para_);
    }

private:
    int32_t para_;
};

/**
 * @tc.name  : Test AudioPolicyAsyncActionHandler API
 * @tc.type  : FUNC
 * @tc.number: LowPriorityActionTest
 * @tc.desc  : Low priority action test
 */
HWTEST_F(AudioPolicyAsyncActionHandlerTest, LowPriorityActionTest, TestSize.Level1)
{
    int32_t realPara = 1;
    std::shared_ptr<PolicyAsyncTestAction> action = std::make_shared<PolicyAsyncTestAction>(realPara);
    ASSERT_TRUE(action != nullptr);
    AsyncActionDesc desc;
    desc.action = std::static_pointer_cast<PolicyAsyncAction>(action);
    bool ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc);
    EXPECT_TRUE(ret);
    usleep(WAIT_FOR_ASYNC_ACTION_TIME_US);
    std::lock_guard<std::mutex> lock(actionMutex);
    ASSERT_EQ(g_expectedParaVec.size(), 1);
    EXPECT_EQ(g_expectedParaVec[0], realPara);
}

/**
 * @tc.name  : Test AudioPolicyAsyncActionHandler API
 * @tc.type  : FUNC
 * @tc.number: HighPriorityActionTest
 * @tc.desc  : High priority action test
 */
HWTEST_F(AudioPolicyAsyncActionHandlerTest, HighPriorityActionTest, TestSize.Level1)
{
    int32_t realPara = 2;
    std::shared_ptr<PolicyAsyncTestAction> action = std::make_shared<PolicyAsyncTestAction>(realPara);
    ASSERT_TRUE(action != nullptr);
    AsyncActionDesc desc;
    desc.priority = ActionPriority::HIGH;
    desc.action = std::static_pointer_cast<PolicyAsyncAction>(action);
    bool ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc);
    EXPECT_TRUE(ret);
    usleep(WAIT_FOR_ASYNC_ACTION_TIME_US);
    std::lock_guard<std::mutex> lock(actionMutex);
    ASSERT_EQ(g_expectedParaVec.size(), 1);
    EXPECT_EQ(g_expectedParaVec[0], realPara);
}

/**
 * @tc.name  : Test AudioPolicyAsyncActionHandler API
 * @tc.type  : FUNC
 * @tc.number: ImmediatePriorityActionTest
 * @tc.desc  : Immediate priority action test
 */
HWTEST_F(AudioPolicyAsyncActionHandlerTest, ImmediatePriorityActionTest, TestSize.Level1)
{
    int32_t realPara = 3;
    std::shared_ptr<PolicyAsyncTestAction> action = std::make_shared<PolicyAsyncTestAction>(realPara);
    ASSERT_TRUE(action != nullptr);
    AsyncActionDesc desc;
    desc.priority = ActionPriority::IMMEDIATE;
    desc.action = std::static_pointer_cast<PolicyAsyncAction>(action);
    bool ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc);
    EXPECT_TRUE(ret);
    usleep(WAIT_FOR_ASYNC_ACTION_TIME_US);
    std::lock_guard<std::mutex> lock(actionMutex);
    ASSERT_EQ(g_expectedParaVec.size(), 1);
    EXPECT_EQ(g_expectedParaVec[0], realPara);
}

/**
 * @tc.name  : Test AudioPolicyAsyncActionHandler API
 * @tc.type  : FUNC
 * @tc.number: UnknownPriorityActionTest
 * @tc.desc  : Unknown priority action test
 */
HWTEST_F(AudioPolicyAsyncActionHandlerTest, UnknownPriorityActionTest, TestSize.Level1)
{
    int32_t realPara = 3;
    std::shared_ptr<PolicyAsyncTestAction> action = std::make_shared<PolicyAsyncTestAction>(realPara);
    ASSERT_TRUE(action != nullptr);
    AsyncActionDesc desc;
    desc.priority = static_cast<ActionPriority>(3);
    desc.action = std::static_pointer_cast<PolicyAsyncAction>(action);
    bool ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc);
    EXPECT_FALSE(ret);
    usleep(WAIT_FOR_ASYNC_ACTION_TIME_US);
    std::lock_guard<std::mutex> lock(actionMutex);
    ASSERT_EQ(g_expectedParaVec.size(), 0);
}

/**
 * @tc.name  : Test AudioPolicyAsyncActionHandler API
 * @tc.type  : FUNC
 * @tc.number: MultiPriorityActionsTest
 * @tc.desc  : Multi priority action test
 */
HWTEST_F(AudioPolicyAsyncActionHandlerTest, MultiPriorityActionsTest, TestSize.Level1)
{
    int32_t realPara1 = 1;
    std::shared_ptr<PolicyAsyncTestAction> lowPriorityAction = std::make_shared<PolicyAsyncTestAction>(realPara1);
    ASSERT_TRUE(lowPriorityAction != nullptr);
    AsyncActionDesc lowPriorityDesc;
    lowPriorityDesc.delayTimeMs = 2; // delay 2 ms
    lowPriorityDesc.action = std::static_pointer_cast<PolicyAsyncAction>(lowPriorityAction);

    int32_t realPara2 = 2;
    std::shared_ptr<PolicyAsyncTestAction> highPriorityAction = std::make_shared<PolicyAsyncTestAction>(realPara2);
    ASSERT_TRUE(highPriorityAction != nullptr);
    AsyncActionDesc highPriorityDesc;
    highPriorityDesc.delayTimeMs = 2; // delay 2 ms
    highPriorityDesc.priority = ActionPriority::HIGH;
    highPriorityDesc.action = std::static_pointer_cast<PolicyAsyncAction>(highPriorityAction);

    int32_t realPara3 = 3;
    std::shared_ptr<PolicyAsyncTestAction> immediatePriorityAction = std::make_shared<PolicyAsyncTestAction>(realPara3);
    ASSERT_TRUE(immediatePriorityAction != nullptr);
    AsyncActionDesc immediatePriorityDesc;
    immediatePriorityDesc.delayTimeMs = 2; // delay 2 ms
    immediatePriorityDesc.priority = ActionPriority::IMMEDIATE;
    immediatePriorityDesc.action = std::static_pointer_cast<PolicyAsyncAction>(immediatePriorityAction);

    bool ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(lowPriorityDesc);
    EXPECT_TRUE(ret);
    ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(highPriorityDesc);
    EXPECT_TRUE(ret);
    ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(immediatePriorityDesc);
    EXPECT_TRUE(ret);
    usleep(WAIT_FOR_ASYNC_ACTION_TIME_US);
    std::lock_guard<std::mutex> lock(actionMutex);
    ASSERT_EQ(g_expectedParaVec.size(), 3);
    EXPECT_EQ(g_expectedParaVec[0], realPara3);
    EXPECT_EQ(g_expectedParaVec[1], realPara2);
    EXPECT_EQ(g_expectedParaVec[2], realPara1);
}

/**
 * @tc.name  : Test AudioPolicyAsyncActionHandler API
 * @tc.type  : FUNC
 * @tc.number: MultiActionsOrderTest
 * @tc.desc  : Multi action with same priority should sorted by handle time
 */
HWTEST_F(AudioPolicyAsyncActionHandlerTest, MultiActionsOrderTest, TestSize.Level1)
{
    int32_t realPara1 = 1;
    std::shared_ptr<PolicyAsyncTestAction> action1 = std::make_shared<PolicyAsyncTestAction>(realPara1);
    ASSERT_TRUE(action1 != nullptr);
    AsyncActionDesc desc1;
    desc1.action = std::static_pointer_cast<PolicyAsyncAction>(action1);

    int32_t realPara2 = 2;
    std::shared_ptr<PolicyAsyncTestAction> action2 = std::make_shared<PolicyAsyncTestAction>(realPara2);
    ASSERT_TRUE(action2 != nullptr);
    AsyncActionDesc desc2;
    desc2.action = std::static_pointer_cast<PolicyAsyncAction>(action2);

    int32_t realPara3 = 3;
    std::shared_ptr<PolicyAsyncTestAction> action3 = std::make_shared<PolicyAsyncTestAction>(realPara3);
    ASSERT_TRUE(action3 != nullptr);
    AsyncActionDesc desc3;
    desc3.action = std::static_pointer_cast<PolicyAsyncAction>(action3);

    bool ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc1);
    EXPECT_TRUE(ret);
    ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc2);
    EXPECT_TRUE(ret);
    ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc3);
    EXPECT_TRUE(ret);
    usleep(WAIT_FOR_ASYNC_ACTION_TIME_US);
    std::lock_guard<std::mutex> lock(actionMutex);
    ASSERT_EQ(g_expectedParaVec.size(), 3);
    EXPECT_EQ(g_expectedParaVec[0], realPara1);
    EXPECT_EQ(g_expectedParaVec[1], realPara2);
    EXPECT_EQ(g_expectedParaVec[2], realPara3);
}

/**
 * @tc.name  : Test AudioPolicyAsyncActionHandler API
 * @tc.type  : FUNC
 * @tc.number: MultiDelayTimeActionsTest
 * @tc.desc  : Multi delayTimeMs action test
 */
HWTEST_F(AudioPolicyAsyncActionHandlerTest, MultiDelayTimeActionsTest, TestSize.Level1)
{
    int32_t realPara1 = 1;
    std::shared_ptr<PolicyAsyncTestAction> action1 = std::make_shared<PolicyAsyncTestAction>(realPara1);
    ASSERT_TRUE(action1 != nullptr);
    AsyncActionDesc desc1;
    desc1.delayTimeMs = 5; // delay 5 ms
    desc1.action = std::static_pointer_cast<PolicyAsyncAction>(action1);

    int32_t realPara2 = 2;
    std::shared_ptr<PolicyAsyncTestAction> action2 = std::make_shared<PolicyAsyncTestAction>(realPara2);
    ASSERT_TRUE(action2 != nullptr);
    AsyncActionDesc desc2;
    desc2.delayTimeMs = 3; // delay 3 ms
    desc2.action = std::static_pointer_cast<PolicyAsyncAction>(action2);

    int32_t realPara3 = 3;
    std::shared_ptr<PolicyAsyncTestAction> action3 = std::make_shared<PolicyAsyncTestAction>(realPara3);
    ASSERT_TRUE(action3 != nullptr);
    AsyncActionDesc desc3;
    desc3.action = std::static_pointer_cast<PolicyAsyncAction>(action3);

    bool ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc1);
    EXPECT_TRUE(ret);
    ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc2);
    EXPECT_TRUE(ret);
    ret = DelayedSingleton<AudioPolicyAsyncActionHandler>::GetInstance()->PostAsyncAction(desc3);
    EXPECT_TRUE(ret);
    usleep(WAIT_FOR_ASYNC_ACTION_TIME_US);
    std::lock_guard<std::mutex> lock(actionMutex);
    ASSERT_EQ(g_expectedParaVec.size(), 3);
    EXPECT_EQ(g_expectedParaVec[0], realPara3);
    EXPECT_EQ(g_expectedParaVec[1], realPara2);
    EXPECT_EQ(g_expectedParaVec[2], realPara1);
}

} // namespace AudioStandard
} // namespace OHOS
