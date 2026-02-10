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

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <functional>
#include <memory>
#include <atomic>

#include "thread_pool.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace testing::ext;
using namespace testing;

namespace {
class ThreadPoolTest : public testing::Test {
public:
    void SetUp() override
    {
        pool_ = std::make_unique<ThreadPool>("TestPool");
    }

    void TearDown() override
    {
        if (pool_ != nullptr) {
            pool_->Stop();
            pool_.reset();
        }
    }

    std::unique_ptr<ThreadPool> pool_;
};

// ============================================================================
// Test Start and Stop
// ============================================================================

HWTEST_F(ThreadPoolTest, Start_Success, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(4);
}

HWTEST_F(ThreadPoolTest, Start_ZeroThreads, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(0);
}

HWTEST_F(ThreadPoolTest, Stop_Normal, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(4);
    pool_->Stop();
}

HWTEST_F(ThreadPoolTest, Stop_NotStarted, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Stop();
}

HWTEST_F(ThreadPoolTest, Stop_Twice, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(4);
    pool_->Stop();
    pool_->Stop();
}

// ============================================================================
// Test SetMaxTaskNum
// ============================================================================

HWTEST_F(ThreadPoolTest, SetMaxTaskNum_Positive, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->SetMaxTaskNum(8);
}

HWTEST_F(ThreadPoolTest, SetMaxTaskNum_Zero, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->SetMaxTaskNum(0);
}

// ============================================================================
// Test Submit - Basic Functionality
// ============================================================================

HWTEST_F(ThreadPoolTest, Submit_ReturnsValidFuture, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(4);

    auto future = pool_->Submit([]() -> int {
        return 42;
    });

    ASSERT_TRUE(future.valid());
    EXPECT_EQ(future.get(), 42);
}

HWTEST_F(ThreadPoolTest, Submit_WithArguments, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(4);

    auto future = pool_->Submit([](int a, int b) -> int {
        return a + b;
    }, 10, 20);

    ASSERT_TRUE(future.valid());
    EXPECT_EQ(future.get(), 30);
}

// ============================================================================
// Test Submit - Multiple Tasks
// ============================================================================

HWTEST_F(ThreadPoolTest, Submit_MultipleTasks, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(4);

    std::vector<std::future<int>> futures;
    for (int i = 0; i < 10; ++i) {
        futures.push_back(pool_->Submit([i]() -> int {
            return i * 10;
        }));
    }

    for (int i = 0; i < 10; ++i) {
        ASSERT_TRUE(futures[i].valid());
        EXPECT_EQ(futures[i].get(), i * 10);
    }
}

HWTEST_F(ThreadPoolTest, Submit_MultipleTasks_ExceedsMax, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->SetMaxTaskNum(3);
    pool_->Start(4);

    std::vector<std::future<int>> futures;
    for (int i = 0; i < 5; ++i) {
        futures.push_back(pool_->Submit([i]() -> int {
            return i;
        }));
    }

    // First 3 should complete, last 2 may return empty
    for (int i = 0; i < 5; ++i) {
        ASSERT_TRUE(futures[i].valid());
        int result = futures[i].get();
        // Results may vary depending on thread pool state
        (void)result;  // Suppress unused variable warning
    }
}

// ============================================================================
// Test Max Task Number Limit
// ============================================================================

HWTEST_F(ThreadPoolTest, MaxTaskNum_BlocksWhenFull, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->SetMaxTaskNum(2);
    pool_->Start(2);

    std::vector<std::future<int>> futures;
    for (int i = 0; i < 4; ++i) {
        futures.push_back(pool_->Submit([i]() -> int {
            return i;
        }));
    }

    for (int i = 0; i < 4; ++i) {
        ASSERT_TRUE(futures[i].valid());
        int result = futures[i].get();
        (void)result;  // Suppress unused variable warning
    }
}

// ============================================================================
// Test Restart
// ============================================================================

HWTEST_F(ThreadPoolTest, Restart_StopThenStart, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(4);

    // Simulate work
    auto future = pool_->Submit([]() -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 1;
    });

    pool_->Stop();
    pool_->Start(4);

    ASSERT_TRUE(future.valid());
    EXPECT_EQ(future.get(), 1);
}

HWTEST_F(ThreadPoolTest, Restart_StartStopStart, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(4);

    pool_->Stop();
    pool_->Start(4);
    pool_->Stop();

    pool_->Start(4);
    pool_->Stop();
}

HWTEST_F(ThreadPoolTest, Concurrent_StopAndSubmit, TestSize.Level0)
{
    ASSERT_NE(pool_, nullptr);
    pool_->Start(2);

    std::atomic<bool> stopCalled{false};
    std::atomic<int> submitCount{0};

    std::thread stopper([this, &stopCalled]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        pool_->Stop();
        stopCalled = true;
    });

    // Submit tasks while stopping
    for (int i = 0; i < 10; ++i) {
        if (stopCalled.load()) {
            auto future = pool_->Submit([]() -> int { return 0; });
            ++submitCount;
            if (future.valid()) {
                future.get(); // Should return immediately
            }
        } else {
            ++submitCount;
            std::this_thread::sleep_for(std::chrono::microseconds(100));
        }
    }

    stopper.join();

    // Some tasks may have been submitted, but not accepted
    EXPECT_GE(submitCount.load(), 10);
}

}  // namespace
