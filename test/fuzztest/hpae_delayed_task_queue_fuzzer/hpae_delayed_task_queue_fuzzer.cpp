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

#include <functional>
#include <iostream>
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
// A 0-delay task's deadline == post-time steady_clock now; the very next
// ProcessExpiredTasks/GetNextDelayMs already observes now >= deadline (tens of
// ns elapse across the calls at nanosecond clock resolution), so no sleep is
// needed to reach the expired branch. Keeping the fuzzer sleep-free preserves
// libFuzzer execs/s throughput (sleep_for(2ms) x N dominated each dispatch).
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
    q.ProcessExpiredTasks();       // runs the task

    q.PostDelayedTask(0, std::function<void()>()); // expired, null task
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

// Deterministically cover PostDelayedTask's while-loop advance counts:
// zero advances (insert at front), one advance (insert in middle), and
// multiple advances (insert at end). The 4th post (another LONG_DELAY_MS)
// advances past every existing entry: the <= advance branch is hit via the
// strictly-later (<) sub-case (steady_clock advances between calls, so exact
// == is unreachable without clock mocking, but < and == share the same advance
// branch). Random delays in PostMultipleDelaysFuzzTest do not reliably hit
// every insert position.
void PostInsertPositionFuzzTest()
{
    HpaeDelayedTaskQueue q;
    // Seed queue with one far-future task -> [LONG].
    (void)q.PostDelayedTask(LONG_DELAY_MS, []() {});

    // Zero advances: SHORT deadline < existing LONG deadline, while body never
    // runs, insert at front -> [SHORT, LONG].
    (void)q.PostDelayedTask(DELAY_MS_SHORT, []() {});

    // One advance: MEDIUM deadline >= SHORT (advance once) but < LONG (stop),
    // insert in middle -> [SHORT, MEDIUM, LONG].
    (void)q.PostDelayedTask(DELAY_MS_MEDIUM, []() {});

    // Multiple advances: another LONG deadline is later than every existing
    // deadline, so the while advances past SHORT, MEDIUM and the first LONG,
    // insert at end -> [SHORT, MEDIUM, LONG, LONG2].
    (void)q.PostDelayedTask(LONG_DELAY_MS, []() {});

    // Exercise GetNextDelayMs on the populated ordered queue.
    (void)q.GetNextDelayMs();
}

// Deterministically cover ProcessExpiredTasks's while loop iterating multiple
// times in a single call: post three 0-delay tasks, let them expire, then one
// ProcessExpiredTasks call pops and executes all three (front.deadline <= now
// is true for each iteration). Then a not-yet-expired task makes the while
// condition go false with a non-empty queue.
void ProcessMultipleExpiredFuzzTest()
{
    HpaeDelayedTaskQueue q;
    // Three expired tasks queued back-to-back; all three must pop in one
    // ProcessExpiredTasks call (while loop runs three iterations).
    (void)q.PostDelayedTask(0, []() {});
    (void)q.PostDelayedTask(0, []() {});
    (void)q.PostDelayedTask(0, []() {});
    q.ProcessExpiredTasks(); // while pops all 3 expired entries in one pass

    // While condition false path: a long-delay task stays at the front, so the
    // while body is skipped entirely on the next ProcessExpiredTasks call.
    (void)q.PostDelayedTask(LONG_DELAY_MS, []() {});
    q.ProcessExpiredTasks(); // front.deadline > now -> while exits immediately
}

// Safety-net: within a single dispatch, chain every public method and every
// branch so coverage does not depend on the dispatcher evenly selecting each
// narrow-case function above. Exercises: empty GetNextDelayMs->0; real task
// expiry (entry.task true -> task() runs); null-task expiry (entry.task false
// -> call skipped); future GetNextDelayMs->remaining; just-passed
// GetNextDelayMs->1 (remaining <= 0 branch); cancel present + cancel absent;
// CancelAll + empty GetNextDelayMs->0.
void FullLifecycleAllBranchesFuzzTest()
{
    HpaeDelayedTaskQueue q;
    // Empty queue -> GetNextDelayMs returns 0.
    (void)q.GetNextDelayMs();

    // Expired real task: entry.task true -> task() runs.
    (void)q.PostDelayedTask(0, []() {});
    q.ProcessExpiredTasks();

    // Expired null task: entry.task false -> call skipped, no crash.
    (void)q.PostDelayedTask(0, std::function<void()>());
    q.ProcessExpiredTasks();

    // Future task -> GetNextDelayMs returns remaining (> 1).
    uint64_t futureId = q.PostDelayedTask(LONG_DELAY_MS, []() {});
    (void)q.GetNextDelayMs();

    // Just-passed task -> GetNextDelayMs returns 1 (remaining <= 0 branch).
    (void)q.PostDelayedTask(0, []() {});
    (void)q.GetNextDelayMs();

    // Cancel a present id (found -> erase + return) and an absent id (loop ends).
    q.CancelTask(futureId);        // found
    q.CancelTask(INVALID_TASK_ID); // not found

    // CancelAll -> tasks_.clear(); then GetNextDelayMs on empty -> 0.
    q.CancelAll();
    (void)q.GetNextDelayMs();
}

vector<TestFuncs> g_testFuncs = {
    PostMultipleDelaysFuzzTest,
    CancelTaskFuzzTest,
    ProcessExpiredTasksFuzzTest,
    GetNextDelayMsFuzzTest,
    CancelAllFuzzTest,
    PostInsertPositionFuzzTest,
    ProcessMultipleExpiredFuzzTest,
    FullLifecycleAllBranchesFuzzTest,
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
