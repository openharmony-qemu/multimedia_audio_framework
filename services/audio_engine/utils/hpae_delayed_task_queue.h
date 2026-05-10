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

#ifndef HPAE_DELAYED_TASK_QUEUE_H
#define HPAE_DELAYED_TASK_QUEUE_H
#include <chrono>
#include <cstdint>
#include <functional>
#include <list>
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeDelayedTaskQueue {
public:
    HpaeDelayedTaskQueue();
    ~HpaeDelayedTaskQueue();

    uint64_t PostDelayedTask(uint64_t delayMs, std::function<void()> task);
    void CancelTask(uint64_t taskId);
    void ProcessExpiredTasks();
    uint64_t GetNextDelayMs() const;
    void CancelAll();

private:
    struct DelayedTaskEntry {
        uint64_t taskId;
        std::chrono::steady_clock::time_point deadline;
        std::function<void()> task;
        bool cancelled = false;
    };
    std::list<DelayedTaskEntry> tasks_;
    uint64_t nextId_ = 1;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif
