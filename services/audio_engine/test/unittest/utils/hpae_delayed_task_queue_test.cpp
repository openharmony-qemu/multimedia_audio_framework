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

#include "gtest/gtest.h"
#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <thread>
#include <vector>

#include "hpae_delayed_task_queue.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeDelayedTaskQueueTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        queue_ = std::make_unique<HpaeDelayedTaskQueue>();
    }

    void TearDown() override
    {
        queue_.reset();
    }

    std::unique_ptr<HpaeDelayedTaskQueue> queue_;
};

// Basic: post a task, it executes after the delay
HWTEST_F(HpaeDelayedTaskQueueTest, postDelayedTask_executesAfterDelay, TestSize.Level0)
{
    std::atomic<int> counter{0};
    queue_->PostDelayedTask(50, [&counter]() { counter++; });

    // Not yet expired
    queue_->ProcessExpiredTasks();
    EXPECT_EQ(counter.load(), 0);

    // Wait for expiry
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    queue_->ProcessExpiredTasks();
    EXPECT_EQ(counter.load(), 1);
}

// Task not executed before deadline
HWTEST_F(HpaeDelayedTaskQueueTest, processExpiredTasks_skipsNotExpired, TestSize.Level0)
{
    std::atomic<int> counter{0};
    queue_->PostDelayedTask(1000, [&counter]() { counter++; });

    queue_->ProcessExpiredTasks();
    EXPECT_EQ(counter.load(), 0);

    uint64_t delay = queue_->GetNextDelayMs();
    EXPECT_GT(delay, 0u);
}

// CancelTask prevents execution
HWTEST_F(HpaeDelayedTaskQueueTest, cancelTask_preventsExecution, TestSize.Level0)
{
    std::atomic<int> counter{0};
    uint64_t taskId = queue_->PostDelayedTask(50, [&counter]() { counter++; });

    queue_->CancelTask(taskId);

    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    queue_->ProcessExpiredTasks();
    EXPECT_EQ(counter.load(), 0);
}

// CancelAll prevents all execution
HWTEST_F(HpaeDelayedTaskQueueTest, cancelAll_preventsAllExecution, TestSize.Level0)
{
    std::atomic<int> counter{0};
    queue_->PostDelayedTask(10, [&counter]() { counter++; });
    queue_->PostDelayedTask(20, [&counter]() { counter++; });
    queue_->PostDelayedTask(30, [&counter]() { counter++; });

    queue_->CancelAll();

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue_->ProcessExpiredTasks();
    EXPECT_EQ(counter.load(), 0);
}

// GetNextDelayMs returns 0 when no tasks
HWTEST_F(HpaeDelayedTaskQueueTest, getNextDelayMs_returnsZeroWhenEmpty, TestSize.Level0)
{
    EXPECT_EQ(queue_->GetNextDelayMs(), 0u);
}

// GetNextDelayMs returns approximate remaining time
HWTEST_F(HpaeDelayedTaskQueueTest, getNextDelayMs_returnsRemainingTime, TestSize.Level0)
{
    queue_->PostDelayedTask(200, []() {});

    uint64_t delay = queue_->GetNextDelayMs();
    EXPECT_GT(delay, 150u);
    EXPECT_LE(delay, 200u);
}

// GetNextDelayMs skips cancelled tasks
HWTEST_F(HpaeDelayedTaskQueueTest, getNextDelayMs_skipsCancelledTasks, TestSize.Level0)
{
    uint64_t id1 = queue_->PostDelayedTask(100, []() {});
    queue_->PostDelayedTask(200, []() {});

    queue_->CancelTask(id1);

    uint64_t delay = queue_->GetNextDelayMs();
    EXPECT_GT(delay, 150u);
}

// Multiple tasks execute in deadline order
HWTEST_F(HpaeDelayedTaskQueueTest, multipleTasks_executeInDeadlineOrder, TestSize.Level0)
{
    std::vector<int> order;
    queue_->PostDelayedTask(60, [&order]() { order.push_back(3); });
    queue_->PostDelayedTask(20, [&order]() { order.push_back(1); });
    queue_->PostDelayedTask(40, [&order]() { order.push_back(2); });

    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    queue_->ProcessExpiredTasks();

    ASSERT_EQ(order.size(), 3u);
    EXPECT_EQ(order[0], 1);
    EXPECT_EQ(order[1], 2);
    EXPECT_EQ(order[2], 3);
}

// PostDelayedTask returns unique IDs
HWTEST_F(HpaeDelayedTaskQueueTest, postDelayedTask_returnsUniqueIds, TestSize.Level0)
{
    uint64_t id1 = queue_->PostDelayedTask(100, []() {});
    uint64_t id2 = queue_->PostDelayedTask(100, []() {});
    uint64_t id3 = queue_->PostDelayedTask(100, []() {});

    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
}

// ProcessExpiredTasks only processes expired tasks, leaves future ones
HWTEST_F(HpaeDelayedTaskQueueTest, processExpiredTasks_partialExpiry, TestSize.Level0)
{
    std::atomic<int> counter{0};
    queue_->PostDelayedTask(30, [&counter]() { counter++; });
    queue_->PostDelayedTask(300, [&counter]() { counter++; });

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue_->ProcessExpiredTasks();
    EXPECT_EQ(counter.load(), 1);

    uint64_t delay = queue_->GetNextDelayMs();
    EXPECT_GT(delay, 0u);
}

// CancelTask for non-existent taskId is safe (no crash)
HWTEST_F(HpaeDelayedTaskQueueTest, cancelTask_nonExistentId_noop, TestSize.Level0)
{
    queue_->CancelTask(99999u);
    // No crash
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
