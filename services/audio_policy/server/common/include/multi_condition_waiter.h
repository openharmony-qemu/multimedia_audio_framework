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

#ifndef MULTI_CONDITION_WAITER_H
#define MULTI_CONDITION_WAITER_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>

namespace OHOS {
namespace AudioStandard {

struct MultiConditionWaiterData {
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<uint64_t> completedConditionMask_ = 0;
};

class MultiConditionWaiter {
public:
    enum class WaitResult {
        SUCCESS,
        INVALID_STATE,
        WAIT_LIMIT_EXCEEDED,
        TIMEOUT,
    };

    explicit MultiConditionWaiter(std::shared_ptr<MultiConditionWaiterData> data);
    ~MultiConditionWaiter() = default;
    MultiConditionWaiter(const MultiConditionWaiter &other) = delete;
    MultiConditionWaiter(MultiConditionWaiter &&other) = delete;
    MultiConditionWaiter &operator=(const MultiConditionWaiter &other) = delete;
    MultiConditionWaiter &operator=(MultiConditionWaiter &&other) = delete;

    /*
     * Wait until all bits in targetConditionMask are notified.
     * waitLimit limits concurrent waits on this waiter; 0 means no limit.
     */
    WaitResult Wait(uint64_t targetConditionMask, int32_t waitLimit = 0);

    /*
     * Wait until all bits in targetConditionMask are notified or waitTimeout expires.
     * waitLimit limits concurrent waits on this waiter; 0 means no limit.
     */
    WaitResult WaitFor(uint64_t targetConditionMask, std::chrono::milliseconds waitTimeout, int32_t waitLimit = 0);

private:
    class WaitCountGuard {
    public:
        explicit WaitCountGuard(std::atomic<int32_t> &waitCount);
        ~WaitCountGuard();
        WaitCountGuard(const WaitCountGuard &other) = delete;
        WaitCountGuard(WaitCountGuard &&other) = delete;
        WaitCountGuard &operator=(const WaitCountGuard &other) = delete;
        WaitCountGuard &operator=(WaitCountGuard &&other) = delete;
        int32_t GetOldCount() const;

    private:
        std::atomic<int32_t> &waitCount_;
        int32_t oldCount_ = 0;
    };

    bool IsCompleted(uint64_t targetConditionMask) const;
    template<bool waitFor, typename... Args>
    WaitResult WaitInternal(uint64_t targetConditionMask, int32_t waitLimit, Args &&... args);
    template<bool waitFor, typename... Args>
    bool DoWait(uint64_t targetConditionMask, Args &&... args);

    std::shared_ptr<MultiConditionWaiterData> data_;
    std::atomic<int32_t> waitCount_ = 0;
};

class MultiConditionNotifier {
public:
    MultiConditionNotifier();
    ~MultiConditionNotifier() = default;
    MultiConditionNotifier(const MultiConditionNotifier &other) = default;
    MultiConditionNotifier(MultiConditionNotifier &&other) noexcept = default;
    MultiConditionNotifier &operator=(const MultiConditionNotifier &other) = default;
    MultiConditionNotifier &operator=(MultiConditionNotifier &&other) noexcept = default;
    MultiConditionWaiter CreateWaiter() const;
    void NotifySuccess(uint64_t completedConditionMask);

private:
    std::shared_ptr<MultiConditionWaiterData> data_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // MULTI_CONDITION_WAITER_H
