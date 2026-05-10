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

#include "hpae_delayed_task_queue.h"
#include <algorithm>
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeDelayedTaskQueue::HpaeDelayedTaskQueue() {}

HpaeDelayedTaskQueue::~HpaeDelayedTaskQueue()
{
    CancelAll();
}

uint64_t HpaeDelayedTaskQueue::PostDelayedTask(uint64_t delayMs, std::function<void()> task)
{
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(delayMs);
    uint64_t id = nextId_++;
    DelayedTaskEntry entry{id, deadline, std::move(task), false};

    auto it = tasks_.begin();
    while (it != tasks_.end() && it->deadline <= deadline) {
        ++it;
    }
    tasks_.insert(it, std::move(entry));
    return id;
}

void HpaeDelayedTaskQueue::CancelTask(uint64_t taskId)
{
    for (auto it = tasks_.begin(); it != tasks_.end(); ++it) {
        if (it->taskId == taskId) {
            tasks_.erase(it);
            return;
        }
    }
}

void HpaeDelayedTaskQueue::ProcessExpiredTasks()
{
    auto now = std::chrono::steady_clock::now();
    while (!tasks_.empty() && tasks_.front().deadline <= now) {
        auto entry = std::move(tasks_.front());
        tasks_.pop_front();
        if (!entry.cancelled && entry.task) {
            entry.task();
        }
    }
}

uint64_t HpaeDelayedTaskQueue::GetNextDelayMs() const
{
    for (const auto &entry : tasks_) {
        if (!entry.cancelled) {
            auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                entry.deadline - std::chrono::steady_clock::now());
            if (remaining.count() <= 0) {
                return 1;  // Avoid indefinite wait in caller when deadline just passed
            }
            return static_cast<uint64_t>(remaining.count());
        }
    }
    return 0;
}

void HpaeDelayedTaskQueue::CancelAll()
{
    tasks_.clear();
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
