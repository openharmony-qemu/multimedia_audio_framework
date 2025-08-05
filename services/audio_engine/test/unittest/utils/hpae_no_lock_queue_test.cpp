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
#include "hpae_no_lock_queue.h"
#include <atomic>
#include <thread>

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static constexpr size_t TEST_QUEUE_SIZE = 5;
static constexpr size_t TEST_QUEUE_SIZE_THREE = 3;
static constexpr uint32_t NUM_TWO = 2;
static constexpr uint32_t NUM_THREE = 3;

class HpaeNoLockQueueTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        queue_ = std::make_unique<HpaeNoLockQueue>(TEST_QUEUE_SIZE);
        processed_count_ = 0;
    }
    
    std::unique_ptr<HpaeNoLockQueue> queue_;
    std::atomic<int> processed_count_;
};

auto CreateCountingRequest(std::atomic<int>* count)
{
    return [count]() { (*count)++; };
}

HWTEST_F(HpaeNoLockQueueTest, queueConstructorInitialization, TestSize.Level0)
{
    HpaeNoLockQueue queue(TEST_QUEUE_SIZE);
    EXPECT_TRUE(queue.IsFinishProcess());
    HpaeNoLockQueue empty_queue(0);
    EXPECT_FALSE(empty_queue.IsFinishProcess());
}

HWTEST_F(HpaeNoLockQueueTest, pushRequestNormalOperation, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    HpaeNoLockQueue queue(TEST_QUEUE_SIZE);
    for (int i = 0; i < TEST_QUEUE_SIZE; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.HandleRequests();
    EXPECT_EQ(gCount, TEST_QUEUE_SIZE);
}

HWTEST_F(HpaeNoLockQueueTest, pushRequestCapacityLimit, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    HpaeNoLockQueue queue(TEST_QUEUE_SIZE_THREE);
    for (int i = 0; i < TEST_QUEUE_SIZE_THREE; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.PushRequest(countingRequest);
    queue.HandleRequests();
    EXPECT_EQ(gCount, TEST_QUEUE_SIZE_THREE);
}

HWTEST_F(HpaeNoLockQueueTest, queueResetFunction, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    
    HpaeNoLockQueue queue(TEST_QUEUE_SIZE);
    for (int i = 0; i < TEST_QUEUE_SIZE; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.Reset();
    queue.HandleRequests();
    EXPECT_TRUE(queue.IsFinishProcess());
}

HWTEST_F(HpaeNoLockQueueTest, requestExecutionOrder, TestSize.Level0)
{
    std::vector<int> execution_order;
    
    HpaeNoLockQueue queue(TEST_QUEUE_SIZE_THREE);
    queue.PushRequest([&execution_order]() { execution_order.push_back(1); });
    queue.PushRequest([&execution_order]() { execution_order.push_back(NUM_TWO); });
    queue.PushRequest([&execution_order]() { execution_order.push_back(NUM_THREE); });
    queue.HandleRequests();
    ASSERT_EQ(execution_order.size(), NUM_THREE);
    EXPECT_EQ(execution_order[0], 1);
    EXPECT_EQ(execution_order[1], NUM_TWO);
    EXPECT_EQ(execution_order[NUM_TWO], NUM_THREE);
}

HWTEST_F(HpaeNoLockQueueTest, isFinishProcessStatus, TestSize.Level0)
{
    HpaeNoLockQueue queue(NUM_TWO);
    EXPECT_TRUE(queue.IsFinishProcess());
    queue.PushRequest([] () {});
    EXPECT_FALSE(queue.IsFinishProcess());
    queue.HandleRequests();
    EXPECT_TRUE(queue.IsFinishProcess());
    queue.PushRequest([] () {});
    queue.Reset();
    EXPECT_TRUE(queue.IsFinishProcess());
}

HWTEST_F(HpaeNoLockQueueTest, queueExhaustionBehavior, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    
    HpaeNoLockQueue queue(TEST_QUEUE_SIZE_THREE);
    for (int i = 0; i < TEST_QUEUE_SIZE_THREE; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.PushRequest(countingRequest);
    queue.HandleRequests();
    EXPECT_EQ(gCount, TEST_QUEUE_SIZE_THREE);
    queue.PushRequest(countingRequest);
    EXPECT_FALSE(queue.IsFinishProcess());
    queue.HandleRequests();
    EXPECT_EQ(gCount, 4); // 4: expected res
}

HWTEST_F(HpaeNoLockQueueTest, multiThreadedConcurrency, TestSize.Level0)
{
    constexpr int threadCount = 4;
    constexpr int requestPerThread = 100;
    
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    
    HpaeNoLockQueue queue(threadCount * requestPerThread + 10); // 10: additional len
    
    auto pushTask = [&queue, countingRequest]() {
        for (int i = 0; i < requestPerThread; ++i) {
            queue.PushRequest(countingRequest);
        }
    };
    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(pushTask);
    }
    for (auto& thread : threads) {
        thread.join();
    }
    queue.HandleRequests();
    EXPECT_EQ(gCount, threadCount * requestPerThread);
    EXPECT_TRUE(queue.IsFinishProcess());
}

HWTEST_F(HpaeNoLockQueueTest, maximumRequestCountHandling, TestSize.Level0)
{
    constexpr size_t largeSize = 10000;
    HpaeNoLockQueue large_queue(largeSize);
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    
    for (size_t i = 0; i < largeSize; ++i) {
        large_queue.PushRequest(countingRequest);
    }
    large_queue.PushRequest(countingRequest);
    large_queue.HandleRequests();
    EXPECT_EQ(static_cast<size_t>(gCount), largeSize);
}

HWTEST_F(HpaeNoLockQueueTest, mixedOperations, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    
    HpaeNoLockQueue queue(10); // 10: queue size
    for (int i = 0; i < TEST_QUEUE_SIZE; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.HandleRequests();
    EXPECT_EQ(gCount, TEST_QUEUE_SIZE);
    queue.Reset();
    EXPECT_TRUE(queue.IsFinishProcess());
    for (int i = 0; i < TEST_QUEUE_SIZE_THREE; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.HandleRequests();
    EXPECT_EQ(gCount, 8); // 8: expected res
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS