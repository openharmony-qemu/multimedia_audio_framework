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
#include <atomic>
#include <array>
#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "hpae_no_lock_queue.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

static constexpr size_t TEST_QUEUE_SIZE = 5;
static constexpr size_t TEST_QUEUE_SIZE_THREE = 3;
static constexpr uint32_t NUM_TWO = 2;
static constexpr uint32_t NUM_THREE = 3;
static constexpr uint32_t INVALID_REQUEST_ID_TEST = std::numeric_limits<uint32_t>::max();

class HpaeNoLockQueueTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        queue_ = std::make_unique<HpaeNoLockQueue>(CURRENT_REQUEST_COUNT);
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
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
    EXPECT_TRUE(queue.IsFinishProcess());
}

HWTEST_F(HpaeNoLockQueueTest, queueConstructorFallbacksToCurrentRequestCount, TestSize.Level0)
{
    HpaeNoLockQueue queue(TEST_QUEUE_SIZE);
    EXPECT_EQ(queue.requestCount_.load(), CURRENT_REQUEST_COUNT);
    EXPECT_NE(queue.requestQueue_[0].get(), nullptr);
}

HWTEST_F(HpaeNoLockQueueTest, pushRequestNormalOperation, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
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
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
    for (size_t i = 0; i < MAX_REQUEST_COUNT; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.PushRequest(countingRequest);
    queue.HandleRequests();
    EXPECT_EQ(static_cast<size_t>(gCount), MAX_REQUEST_COUNT);
}

HWTEST_F(HpaeNoLockQueueTest, queueResetFunction, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
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
    
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
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
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
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
    
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
    for (size_t i = 0; i < MAX_REQUEST_COUNT; ++i) {
        queue.PushRequest(countingRequest);
    }
    queue.PushRequest(countingRequest);
    queue.HandleRequests();
    EXPECT_EQ(static_cast<size_t>(gCount), MAX_REQUEST_COUNT);
    queue.PushRequest(countingRequest);
    EXPECT_FALSE(queue.IsFinishProcess());
    queue.HandleRequests();
    EXPECT_EQ(static_cast<size_t>(gCount), MAX_REQUEST_COUNT + 1);
}

HWTEST_F(HpaeNoLockQueueTest, multiThreadedConcurrency, TestSize.Level0)
{
    constexpr int threadCount = 4;
    constexpr int requestPerThread = 400;
    
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
    
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
    constexpr size_t largeSize = MAX_REQUEST_COUNT;
    HpaeNoLockQueue large_queue(CURRENT_REQUEST_COUNT);
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
    
    HpaeNoLockQueue queue(CURRENT_REQUEST_COUNT);
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

HWTEST_F(HpaeNoLockQueueTest, getExpandThresholdReturnsConfiguredPercentage, TestSize.Level0)
{
    EXPECT_EQ(queue_->GetExpandThreshold(CURRENT_REQUEST_COUNT),
        CURRENT_REQUEST_COUNT * EXPAND_THRESHOLD_PERCENT / PERCENTAGE_BASE);
    EXPECT_EQ(queue_->GetExpandThreshold(MAX_REQUEST_COUNT), MAX_REQUEST_COUNT * EXPAND_THRESHOLD_PERCENT /
        PERCENTAGE_BASE);
}

HWTEST_F(HpaeNoLockQueueTest, tryExpandQueueSkipsExpansionBelowThreshold, TestSize.Level0)
{
    queue_->usedRequestCount_ = queue_->GetExpandThreshold(queue_->requestCount_.load()) - 1;
    EXPECT_FALSE(queue_->TryExpandQueue(false));
    EXPECT_EQ(queue_->requestCount_.load(), CURRENT_REQUEST_COUNT);
    EXPECT_EQ(queue_->requestQueue_[1].get(), nullptr);
}

HWTEST_F(HpaeNoLockQueueTest, tryExpandQueueReturnsFalseAtMaximumRequestCount, TestSize.Level0)
{
    queue_->requestCount_ = MAX_REQUEST_COUNT;
    queue_->usedRequestCount_ = MAX_REQUEST_COUNT;
    EXPECT_FALSE(queue_->TryExpandQueue(false));
    EXPECT_FALSE(queue_->TryExpandQueue(true));
}

HWTEST_F(HpaeNoLockQueueTest, tryExpandQueueForceReturnsTrueWhenFreeNodeStillExists, TestSize.Level0)
{
    EXPECT_TRUE(queue_->TryExpandQueue(true));
    EXPECT_EQ(queue_->requestCount_.load(), CURRENT_REQUEST_COUNT);
    EXPECT_EQ(queue_->requestQueue_[1].get(), nullptr);
}

HWTEST_F(HpaeNoLockQueueTest, tryExpandQueueExpandsAtThreshold, TestSize.Level0)
{
    queue_->usedRequestCount_ = queue_->GetExpandThreshold(queue_->requestCount_.load());
    EXPECT_TRUE(queue_->TryExpandQueue(false));
    EXPECT_EQ(queue_->requestCount_.load(), CURRENT_REQUEST_COUNT + EXPAND_REQUEST_COUNT);
    EXPECT_NE(queue_->requestQueue_[1].get(), nullptr);
    EXPECT_NE(queue_->GetRequestNodePtr(CURRENT_REQUEST_COUNT), nullptr);
    EXPECT_NE(queue_->GetRequestNodePtr(CURRENT_REQUEST_COUNT + EXPAND_REQUEST_COUNT - 1), nullptr);
    EXPECT_EQ(queue_->GetRequestNodePtr(CURRENT_REQUEST_COUNT + EXPAND_REQUEST_COUNT), nullptr);
}

HWTEST_F(HpaeNoLockQueueTest, tryExpandQueueForceExpandsAfterFreeListIsExhausted, TestSize.Level0)
{
    for (size_t i = 0; i < CURRENT_REQUEST_COUNT; ++i) {
        EXPECT_NE(queue_->GetRequsetIndex(queue_->GetRequestNode(&queue_->freeRequestHeadIndex_)),
            INVALID_REQUEST_ID_TEST);
    }
    EXPECT_EQ(queue_->GetRequsetIndex(queue_->freeRequestHeadIndex_.load()), INVALID_REQUEST_ID_TEST);

    EXPECT_TRUE(queue_->TryExpandQueue(true));
    EXPECT_EQ(queue_->requestCount_.load(), CURRENT_REQUEST_COUNT + EXPAND_REQUEST_COUNT);
    EXPECT_EQ(queue_->GetRequsetIndex(queue_->freeRequestHeadIndex_.load()), CURRENT_REQUEST_COUNT);
    EXPECT_NE(queue_->GetRequestNodePtr(CURRENT_REQUEST_COUNT), nullptr);
}

HWTEST_F(HpaeNoLockQueueTest, getRequestNodePtrHandlesInvalidAndUnallocatedIndex, TestSize.Level0)
{
    EXPECT_NE(queue_->GetRequestNodePtr(0), nullptr);
    EXPECT_EQ(queue_->GetRequestNodePtr(INVALID_REQUEST_ID_TEST), nullptr);

    queue_->requestCount_ = CURRENT_REQUEST_COUNT + EXPAND_REQUEST_COUNT;
    EXPECT_EQ(queue_->requestQueue_[1].get(), nullptr);
    EXPECT_EQ(queue_->GetRequestNodePtr(CURRENT_REQUEST_COUNT), nullptr);
}

HWTEST_F(HpaeNoLockQueueTest, getRequestNodeHandlesNullAndInvalidHead, TestSize.Level0)
{
    EXPECT_EQ(queue_->GetRequestNode(nullptr), std::numeric_limits<uint64_t>::max());

    std::atomic<uint64_t> emptyHead(INVALID_REQUEST_ID_TEST);
    EXPECT_EQ(queue_->GetRequestNode(&emptyHead), INVALID_REQUEST_ID_TEST);

    std::atomic<uint64_t> invalidHead(CURRENT_REQUEST_COUNT);
    EXPECT_EQ(queue_->GetRequestNode(&invalidHead), INVALID_REQUEST_ID_TEST);
}

HWTEST_F(HpaeNoLockQueueTest, pushRequestNodeHelpersIgnoreInvalidInputs, TestSize.Level0)
{
    std::atomic<uint64_t> requestHead(INVALID_REQUEST_ID_TEST);
    queue_->PushRequestNode(nullptr, 0);
    queue_->PushRequestNode(&requestHead, CURRENT_REQUEST_COUNT);
    EXPECT_EQ(requestHead.load(), INVALID_REQUEST_ID_TEST);

    queue_->PushRequestNodeChain(nullptr, 0, 0);
    queue_->PushRequestNodeChain(&requestHead, 0, CURRENT_REQUEST_COUNT);
    EXPECT_EQ(requestHead.load(), INVALID_REQUEST_ID_TEST);
}

HWTEST_F(HpaeNoLockQueueTest, pushRequestExpandsQueueWhenThresholdReached, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    const size_t threshold = queue_->GetExpandThreshold(queue_->requestCount_.load());

    for (size_t i = 0; i < threshold; ++i) {
        queue_->PushRequest(countingRequest);
    }

    EXPECT_EQ(queue_->requestCount_.load(), CURRENT_REQUEST_COUNT + EXPAND_REQUEST_COUNT);
    EXPECT_NE(queue_->requestQueue_[1].get(), nullptr);
    queue_->HandleRequests();
    EXPECT_EQ(static_cast<size_t>(gCount), threshold);
}

HWTEST_F(HpaeNoLockQueueTest, resetAfterExpansionClearsRequestsWithoutExecution, TestSize.Level0)
{
    std::atomic<int> gCount = 0;
    auto countingRequest = [&gCount]() { gCount++; };
    const size_t threshold = queue_->GetExpandThreshold(queue_->requestCount_.load());

    for (size_t i = 0; i < threshold; ++i) {
        queue_->PushRequest(countingRequest);
    }

    EXPECT_LT(queue_->tempRequestQueue_.capacity(), queue_->requestCount_.load());
    EXPECT_EQ(queue_->usedRequestCount_.load(), threshold);
    queue_->Reset();
    EXPECT_EQ(gCount, 0);
    EXPECT_EQ(queue_->usedRequestCount_.load(), 0U);
    EXPECT_GE(queue_->tempRequestQueue_.capacity(), queue_->requestCount_.load());
}

HWTEST_F(HpaeNoLockQueueTest, pushRequestNodeChainPrependsExpandedBlockToFreeList, TestSize.Level0)
{
    queue_->usedRequestCount_ = queue_->GetExpandThreshold(queue_->requestCount_.load());
    ASSERT_TRUE(queue_->TryExpandQueue(false));

    for (size_t i = 0; i < EXPAND_REQUEST_COUNT; ++i) {
        EXPECT_EQ(queue_->GetRequsetIndex(queue_->GetRequestNode(&queue_->freeRequestHeadIndex_)),
            CURRENT_REQUEST_COUNT + i);
    }
    EXPECT_EQ(queue_->GetRequsetIndex(queue_->GetRequestNode(&queue_->freeRequestHeadIndex_)), 0U);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
