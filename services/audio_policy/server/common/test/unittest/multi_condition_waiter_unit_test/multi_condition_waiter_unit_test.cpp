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

#include "multi_condition_waiter.h"

#include <chrono>
#include <future>
#include <gtest/gtest.h>
#include <thread>
#include <utility>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint64_t FIRST_CONDITION = 1ULL;
constexpr uint64_t SECOND_CONDITION = 1ULL << 1;
constexpr uint64_t ALL_CONDITIONS = FIRST_CONDITION | SECOND_CONDITION;
constexpr auto SHORT_WAIT_TIME = std::chrono::milliseconds(20);
constexpr auto LONG_WAIT_TIME = std::chrono::milliseconds(200);
} // namespace

class MultiConditionWaiterUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void) {}
};

/**
 * @tc.name  : WaitFor_Timeout_001
 * @tc.number: WaitFor_Timeout_001
 * @tc.desc  : Test WaitFor returns timeout when no condition is notified.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_Timeout_001, TestSize.Level4)
{
    MultiConditionNotifier notifier;
    MultiConditionWaiter waiter = notifier.CreateWaiter();

    auto result = waiter.WaitFor(FIRST_CONDITION, SHORT_WAIT_TIME);

    EXPECT_EQ(result, MultiConditionWaiter::WaitResult::TIMEOUT);
}

/**
 * @tc.name  : Wait_InvalidState_001
 * @tc.number: Wait_InvalidState_001
 * @tc.desc  : Test Wait returns invalid state when data is null.
 */
HWTEST_F(MultiConditionWaiterUnitTest, Wait_InvalidState_001, TestSize.Level4)
{
    MultiConditionWaiter waiter(nullptr);

    auto result = waiter.Wait(FIRST_CONDITION);

    EXPECT_EQ(result, MultiConditionWaiter::WaitResult::INVALID_STATE);
}

/**
 * @tc.name  : WaitFor_InvalidState_001
 * @tc.number: WaitFor_InvalidState_001
 * @tc.desc  : Test WaitFor returns invalid state when data is null.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_InvalidState_001, TestSize.Level4)
{
    MultiConditionWaiter waiter(nullptr);

    auto result = waiter.WaitFor(FIRST_CONDITION, SHORT_WAIT_TIME);

    EXPECT_EQ(result, MultiConditionWaiter::WaitResult::INVALID_STATE);
}

/**
 * @tc.name  : WaitFor_AlreadyCompleted_001
 * @tc.number: WaitFor_AlreadyCompleted_001
 * @tc.desc  : Test WaitFor returns success immediately when target condition is already completed.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_AlreadyCompleted_001, TestSize.Level4)
{
    MultiConditionNotifier notifier;
    MultiConditionWaiter waiter = notifier.CreateWaiter();

    notifier.NotifySuccess(FIRST_CONDITION);
    auto result = waiter.WaitFor(FIRST_CONDITION, SHORT_WAIT_TIME);

    EXPECT_EQ(result, MultiConditionWaiter::WaitResult::SUCCESS);
}

/**
 * @tc.name  : WaitFor_ZeroCondition_001
 * @tc.number: WaitFor_ZeroCondition_001
 * @tc.desc  : Test zero condition notification does not complete a non-zero target condition.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_ZeroCondition_001, TestSize.Level4)
{
    MultiConditionNotifier notifier;
    MultiConditionWaiter waiter = notifier.CreateWaiter();

    notifier.NotifySuccess(0);
    auto result = waiter.WaitFor(FIRST_CONDITION, SHORT_WAIT_TIME);

    EXPECT_EQ(result, MultiConditionWaiter::WaitResult::TIMEOUT);
}

/**
 * @tc.name  : WaitFor_MoveConstructor_001
 * @tc.number: WaitFor_MoveConstructor_001
 * @tc.desc  : Test moved waiter keeps shared notifier data.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_MoveConstructor_001, TestSize.Level4)
{
    MultiConditionNotifier notifier;
    MultiConditionWaiter waiter = notifier.CreateWaiter();
    MultiConditionWaiter movedWaiter(std::move(waiter));

    notifier.NotifySuccess(FIRST_CONDITION);
    auto result = movedWaiter.WaitFor(FIRST_CONDITION, SHORT_WAIT_TIME);

    EXPECT_EQ(result, MultiConditionWaiter::WaitResult::SUCCESS);
}

/**
 * @tc.name  : WaitFor_MoveAssignment_001
 * @tc.number: WaitFor_MoveAssignment_001
 * @tc.desc  : Test move assignment keeps shared notifier data.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_MoveAssignment_001, TestSize.Level4)
{
    MultiConditionNotifier notifier;
    MultiConditionWaiter waiter = notifier.CreateWaiter();
    MultiConditionNotifier otherNotifier;
    MultiConditionWaiter movedWaiter = otherNotifier.CreateWaiter();

    movedWaiter = std::move(waiter);
    notifier.NotifySuccess(FIRST_CONDITION);
    auto result = movedWaiter.WaitFor(FIRST_CONDITION, SHORT_WAIT_TIME);

    EXPECT_EQ(result, MultiConditionWaiter::WaitResult::SUCCESS);
}

/**
 * @tc.name  : Wait_AllConditionsReady_001
 * @tc.number: Wait_AllConditionsReady_001
 * @tc.desc  : Test Wait returns success after all target condition bits are notified.
 */
HWTEST_F(MultiConditionWaiterUnitTest, Wait_AllConditionsReady_001, TestSize.Level4)
{
    MultiConditionNotifier notifier;
    MultiConditionWaiter waiter = notifier.CreateWaiter();

    auto waitTask = std::async(std::launch::async, [&waiter]() {
        return waiter.Wait(ALL_CONDITIONS);
    });
    std::this_thread::sleep_for(SHORT_WAIT_TIME);

    notifier.NotifySuccess(FIRST_CONDITION);
    EXPECT_EQ(waitTask.wait_for(SHORT_WAIT_TIME), std::future_status::timeout);

    notifier.NotifySuccess(SECOND_CONDITION);
    EXPECT_EQ(waitTask.get(), MultiConditionWaiter::WaitResult::SUCCESS);
}

/**
 * @tc.name  : WaitFor_AllConditionsReady_001
 * @tc.number: WaitFor_AllConditionsReady_001
 * @tc.desc  : Test waiter returns success after all target condition bits are notified.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_AllConditionsReady_001, TestSize.Level4)
{
    MultiConditionNotifier notifier;
    MultiConditionWaiter waiter = notifier.CreateWaiter();

    auto waitTask = std::async(std::launch::async, [&waiter]() {
        return waiter.WaitFor(ALL_CONDITIONS, LONG_WAIT_TIME);
    });
    std::this_thread::sleep_for(SHORT_WAIT_TIME);

    notifier.NotifySuccess(FIRST_CONDITION);
    EXPECT_EQ(waitTask.wait_for(SHORT_WAIT_TIME), std::future_status::timeout);

    notifier.NotifySuccess(SECOND_CONDITION);
    EXPECT_EQ(waitTask.get(), MultiConditionWaiter::WaitResult::SUCCESS);
}

/**
 * @tc.name  : WaitFor_WaiterLimit_001
 * @tc.number: WaitFor_WaiterLimit_001
 * @tc.desc  : Test wait count limit is held by each waiter.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_WaiterLimit_001, TestSize.Level4)
{
    constexpr int32_t waitLimit = 1;
    MultiConditionNotifier notifier;
    MultiConditionWaiter waiter = notifier.CreateWaiter();

    auto firstWaitTask = std::async(std::launch::async, [&waiter]() {
        return waiter.WaitFor(FIRST_CONDITION, LONG_WAIT_TIME, waitLimit);
    });
    std::this_thread::sleep_for(SHORT_WAIT_TIME);

    auto secondResult = waiter.WaitFor(FIRST_CONDITION, SHORT_WAIT_TIME, waitLimit);
    EXPECT_EQ(secondResult, MultiConditionWaiter::WaitResult::WAIT_LIMIT_EXCEEDED);

    notifier.NotifySuccess(FIRST_CONDITION);
    EXPECT_EQ(firstWaitTask.get(), MultiConditionWaiter::WaitResult::SUCCESS);
}

/**
 * @tc.name  : WaitFor_WaitLimitIndependentWaiters_001
 * @tc.number: WaitFor_WaitLimitIndependentWaiters_001
 * @tc.desc  : Test wait count limit is independent for waiters sharing the same notifier data.
 */
HWTEST_F(MultiConditionWaiterUnitTest, WaitFor_WaitLimitIndependentWaiters_001, TestSize.Level4)
{
    constexpr int32_t waitLimit = 1;
    MultiConditionNotifier notifier;
    MultiConditionWaiter firstWaiter = notifier.CreateWaiter();
    MultiConditionWaiter secondWaiter = notifier.CreateWaiter();

    auto firstWaitTask = std::async(std::launch::async, [&firstWaiter]() {
        return firstWaiter.WaitFor(FIRST_CONDITION, LONG_WAIT_TIME, waitLimit);
    });
    auto secondWaitTask = std::async(std::launch::async, [&secondWaiter]() {
        return secondWaiter.WaitFor(FIRST_CONDITION, LONG_WAIT_TIME, waitLimit);
    });
    std::this_thread::sleep_for(SHORT_WAIT_TIME);

    notifier.NotifySuccess(FIRST_CONDITION);
    EXPECT_EQ(firstWaitTask.get(), MultiConditionWaiter::WaitResult::SUCCESS);
    EXPECT_EQ(secondWaitTask.get(), MultiConditionWaiter::WaitResult::SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
