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

#include <chrono>
#include <functional>
#include <iostream>
#include <thread>
#include <vector>
#include "hpae_delayed_task_queue.h"
#include "../fuzz_utils.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
typedef void (*TestFuncs)();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

// Delay (ms) values used to drive specific queue branches.
const uint32_t MAX_EXTRA_TASK_COUNT = 5;      // % N + 1 -> 1..5 tasks per iteration
const uint64_t MAX_RANDOM_DELAY_MS = 1000;    // % N -> 0..999 ms random delay
const uint64_t DELAY_MS_SHORT = 100;          // short distinct delay (cancel/insert)
const uint64_t DELAY_MS_MEDIUM = 200;         // medium distinct delay (cancel/insert)
const uint64_t LONG_DELAY_MS = 60000;         // 60s, always "not expired" within a test
const uint64_t INVALID_TASK_ID = 999999;      // sentinel id that was never posted
const uint64_t EXPIRY_SETTLE_MS = 2;          // sleep so a 0-ms task is expired by ProcessExpiredTasks

// Post several tasks with varied delays. Covers PostDelayedTask's insert-position
// branches: insert-at-front (new deadline <= existing) and insert-after (loop
// advances while it->deadline <= deadline), plus nextId_ increment.
void PostMultipleDelaysFuzzTest()
{
    HpaeDelayedTaskQueue q;
    int n = static_cast<int>(g_fuzzUtils.GetData<uint32_t>() % MAX_EXTRA_TASK_COUNT) + 1; // 1..5 tasks
    for (int i = 0; i < n; i++) {
        uint64_t delay = g_fuzzUtils.GetData<uint64_t>() % MAX_RANDOM_DELAY_MS; // 0..999 ms
        (void)q.PostDelayedTask(delay, []() {});
    }
    (void)q.GetNextDelayMs();
}

// Cancel: a present id (found -> erase+return), an absent id (loop completes, no
// match), and double-cancel of the same id (second is not-found).
void CancelTaskFuzzTest()
{
    HpaeDelayedTaskQueue q;
    uint64_t id1 = q.PostDelayedTask(DELAY_MS_SHORT, []() {});
    uint64_t id2 = q.PostDelayedTask(DELAY_MS_MEDIUM, []() {});
    q.CancelTask(id1);      // found -> erase + return
    q.CancelTask(id2);      // found -> erase + return
    q.CancelTask(id1);      // already cancelled -> not found
    q.CancelTask(INVALID_TASK_ID);   // never existed -> not found
}

// ProcessExpiredTasks: empty queue (while skipped), expired task with a real
// callback (entry.task true -> runs), expired task with a null callback
// (entry.task false -> skipped), and a not-yet-expired front (while exits).
void ProcessExpiredTasksFuzzTest()
{
    HpaeDelayedTaskQueue q;
    q.ProcessExpiredTasks(); // empty -> while body skipped

    q.PostDelayedTask(0, []() {}); // expired, real task
    std::this_thread::sleep_for(std::chrono::milliseconds(EXPIRY_SETTLE_MS));
    q.ProcessExpiredTasks();       // runs the task

    q.PostDelayedTask(0, std::function<void()>()); // expired, null task
    std::this_thread::sleep_for(std::chrono::milliseconds(EXPIRY_SETTLE_MS));
    q.ProcessExpiredTasks();       // entry.task false -> skip call

    q.PostDelayedTask(LONG_DELAY_MS, []() {}); // not expired
    q.ProcessExpiredTasks();           // front deadline > now -> while exits
}

// GetNextDelayMs: empty queue -> 0; expired front (remaining <= 0) -> 1;
// future front -> remaining.
void GetNextDelayMsFuzzTest()
{
    HpaeDelayedTaskQueue q;
    (void)q.GetNextDelayMs(); // empty -> 0

    q.PostDelayedTask(0, []() {});
    std::this_thread::sleep_for(std::chrono::milliseconds(EXPIRY_SETTLE_MS));
    (void)q.GetNextDelayMs(); // expired -> 1

    q.CancelAll();
    q.PostDelayedTask(LONG_DELAY_MS, []() {});
    (void)q.GetNextDelayMs(); // future -> remaining
}

// CancelAll clears tasks_; destructor also calls CancelAll (covered when each q
// above goes out of scope).
void CancelAllFuzzTest()
{
    HpaeDelayedTaskQueue q;
    q.PostDelayedTask(DELAY_MS_SHORT, []() {});
    q.PostDelayedTask(DELAY_MS_MEDIUM, []() {});
    q.CancelAll();            // tasks_.clear()
    (void)q.GetNextDelayMs(); // -> 0 (empty)
}

vector<TestFuncs> g_testFuncs = {
    PostMultipleDelaysFuzzTest,
    CancelTaskFuzzTest,
    ProcessExpiredTasksFuzzTest,
    GetNextDelayMsFuzzTest,
    CancelAllFuzzTest,
};

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::HPAE::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }
    OHOS::AudioStandard::HPAE::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::HPAE::g_testFuncs);
    return 0;
}
