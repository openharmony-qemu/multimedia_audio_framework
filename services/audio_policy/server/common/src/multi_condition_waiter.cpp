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

#ifndef LOG_TAG
#define LOG_TAG "MultiConditionWaiter"
#endif

#include "multi_condition_waiter.h"

#include "audio_policy_log.h"

#include <cinttypes>
#include <utility>

namespace OHOS {
namespace AudioStandard {

MultiConditionWaiter::MultiConditionWaiter(std::shared_ptr<MultiConditionWaiterData> data) : data_(data)
{}

bool MultiConditionWaiter::IsCompleted(uint64_t targetConditionMask) const
{
    CHECK_AND_RETURN_RET(data_ != nullptr, false);
    return (data_->completedConditionMask_.load(std::memory_order_acquire) & targetConditionMask) ==
        targetConditionMask;
}

MultiConditionWaiter::WaitCountGuard::WaitCountGuard(std::atomic<int32_t> &waitCount) : waitCount_(waitCount)
{
    oldCount_ = waitCount_.fetch_add(1, std::memory_order_relaxed);
}

MultiConditionWaiter::WaitCountGuard::~WaitCountGuard()
{
    waitCount_.fetch_sub(1, std::memory_order_relaxed);
}

int32_t MultiConditionWaiter::WaitCountGuard::GetOldCount() const
{
    return oldCount_;
}

template<bool waitFor, typename... Args>
bool MultiConditionWaiter::DoWait(uint64_t targetConditionMask, Args&&... args)
{
    std::unique_lock<std::mutex> lock(data_->mutex_);
    auto condition = [this, targetConditionMask]() { return IsCompleted(targetConditionMask); };
    if constexpr (waitFor) {
        return data_->cv_.wait_for(lock, std::forward<Args>(args)..., condition);
    } else {
        data_->cv_.wait(lock, condition);
        return true;
    }
}

template<bool waitFor, typename... Args>
MultiConditionWaiter::WaitResult MultiConditionWaiter::WaitInternal(uint64_t targetConditionMask,
    int32_t waitLimit, Args&&... args)
{
    CHECK_AND_RETURN_RET_LOG(data_ != nullptr, WaitResult::INVALID_STATE, "waiter data is nullptr");
    if (IsCompleted(targetConditionMask)) {
        return WaitResult::SUCCESS;
    }
    WaitCountGuard guard(waitCount_);
    if (waitLimit > 0 && guard.GetOldCount() >= waitLimit) {
        AUDIO_ERR_LOG("wait count exceeds limit");
        return WaitResult::WAIT_LIMIT_EXCEEDED;
    }
    if (!DoWait<waitFor>(targetConditionMask, std::forward<Args>(args)...)) {
        AUDIO_ERR_LOG("timeout waiting for required conditions");
        return WaitResult::TIMEOUT;
    }
    return WaitResult::SUCCESS;
}

MultiConditionWaiter::WaitResult MultiConditionWaiter::Wait(uint64_t targetConditionMask, int32_t waitLimit)
{
    return WaitInternal<false>(targetConditionMask, waitLimit);
}

MultiConditionWaiter::WaitResult MultiConditionWaiter::WaitFor(uint64_t targetConditionMask,
    std::chrono::milliseconds waitTimeout, int32_t waitLimit)
{
    return WaitInternal<true>(targetConditionMask, waitLimit, waitTimeout);
}

MultiConditionNotifier::MultiConditionNotifier() : data_(std::make_shared<MultiConditionWaiterData>())
{}

MultiConditionWaiter MultiConditionNotifier::CreateWaiter() const
{
    return MultiConditionWaiter(data_);
}

void MultiConditionNotifier::NotifySuccess(uint64_t completedConditionMask)
{
    if (completedConditionMask == 0) {
        return;
    }
    CHECK_AND_RETURN_LOG(data_ != nullptr, "waiter data is nullptr");

    {
        std::lock_guard<std::mutex> lock(data_->mutex_);
        data_->completedConditionMask_.fetch_or(completedConditionMask, std::memory_order_release);
        AUDIO_INFO_LOG("Wait condition mask done: 0x%{public}" PRIx64, completedConditionMask);
    }
    data_->cv_.notify_all();
}

} // namespace AudioStandard
} // namespace OHOS
