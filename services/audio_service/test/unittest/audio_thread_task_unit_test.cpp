/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "none_mix_engine.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
class AudioThreadTaskUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
* @tc.name  : Test Start API
* @tc.type  : FUNC
* @tc.number: Start_001
* @tc.desc  : Test Start interface.
*/
HWTEST(AudioThreadTaskUnitTest, Start_001, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STARTED;
    audioThreadTask->Start();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STARTED);
}

/**
* @tc.name  : Test StopAsync API
* @tc.type  : FUNC
* @tc.number: StopAsync_001
* @tc.desc  : Test StopAsync interface.
*/
HWTEST(AudioThreadTaskUnitTest, StopAsync_001, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STOPPED;
    audioThreadTask->StopAsync();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STOPPED);
}

/**
* @tc.name  : Test StopAsync API
* @tc.type  : FUNC
* @tc.number: StopAsync_002
* @tc.desc  : Test StopAsync interface.
*/
HWTEST(AudioThreadTaskUnitTest, StopAsync_002, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STOPPING;
    audioThreadTask->StopAsync();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STOPPING);
}

/**
* @tc.name  : Test StopAsync API
* @tc.type  : FUNC
* @tc.number: StopAsync_003
* @tc.desc  : Test StopAsync interface.
*/
HWTEST(AudioThreadTaskUnitTest, StopAsync_003, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STARTED;
    audioThreadTask->StopAsync();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STOPPING);
}

/**
* @tc.name  : Test Pause API
* @tc.type  : FUNC
* @tc.number: Pause_001
* @tc.desc  : Test  Pause interface.
*/
HWTEST(AudioThreadTaskUnitTest, Pause_001, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::PAUSED;
    audioThreadTask->Pause();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::PAUSED);
}

/**
* @tc.name  : Test PauseAsync API
* @tc.type  : FUNC
* @tc.number: PauseAsync_001
* @tc.desc  : Test PauseAsync interface.
*/
HWTEST(AudioThreadTaskUnitTest, PauseAsync_001, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::PAUSED;
    audioThreadTask->PauseAsync();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::PAUSED);
}

/**
* @tc.name  : Test PauseAsync API
* @tc.type  : FUNC
* @tc.number: PauseAsync_002
* @tc.desc  : Test PauseAsync interface.
*/
HWTEST(AudioThreadTaskUnitTest, PauseAsync_002, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STARTED;
    audioThreadTask->PauseAsync();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::PAUSING);
}

/**
* @tc.name  : Test Start API
* @tc.type  : FUNC
* @tc.number: Start_002
* @tc.desc  : Test Start interface.
*/
HWTEST(AudioThreadTaskUnitTest, Start_002, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STOPPED;
    audioThreadTask->Start();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STARTED);
}

/**
* @tc.name  : Test Start API
* @tc.type  : FUNC
* @tc.number: Start_003
* @tc.desc  : Test Start interface.
*/
HWTEST(AudioThreadTaskUnitTest, Start_003, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STOPPED;
    audioThreadTask->loop_ = std::make_unique<std::thread>([&] {});
    audioThreadTask->Start();
    EXPECT_NE(audioThreadTask->loop_, nullptr);
}

/**
* @tc.name  : Test Start API
* @tc.type  : FUNC
* @tc.number: Start_004
* @tc.desc  : Test Start interface.
*/
HWTEST(AudioThreadTaskUnitTest, Start_004, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STOPPED;
    std::thread joinableThread;
    audioThreadTask->loop_ = std::make_unique<std::thread>(std::move(joinableThread));
    audioThreadTask->Start();
    EXPECT_NE(audioThreadTask->loop_, nullptr);
}

/**
* @tc.name  : Test Start API
* @tc.type  : FUNC
* @tc.number: Start_005
* @tc.desc  : Test Start interface.
*/
HWTEST(AudioThreadTaskUnitTest, Start_005, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STOPPED;
    audioThreadTask->loop_ = std::make_unique<std::thread>([&] {});
    audioThreadTask->Start();
    EXPECT_NE(audioThreadTask->loop_, nullptr);
}

/**
* @tc.name  : Test Start API
* @tc.type  : FUNC
* @tc.number: Start_006
* @tc.desc  : Test Start interface.
*/
HWTEST(AudioThreadTaskUnitTest, Start_006, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->state_ = AudioThreadTask::RunningState::STOPPED;
    audioThreadTask->Start();
    EXPECT_NE(audioThreadTask->loop_, nullptr);
}

// Bounded spin-wait and timing constants for the behavioral tests below.
constexpr int SPIN_RETRIES = 200;  // Max retries; ~200ms budget at 1ms each.
constexpr int SPIN_SLEEP_MS = 1;
constexpr int MIN_EXEC_COUNT = 1;
constexpr int PAUSE_FREEZE_WAIT_MS = 300;  // Window to confirm a counter freeze.

/**
* @tc.name  : Test RegisterJob API
* @tc.type  : FUNC
* @tc.number: RegisterJob_001
* @tc.desc  : Test that a registered job actually runs on the worker thread.
*/
HWTEST(AudioThreadTaskUnitTest, RegisterJob_001, TestSize.Level1)
{
    std::atomic<int> counter{0};
    std::thread::id workerTid;
    std::thread::id mainTid = std::this_thread::get_id();
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->RegisterJob([&counter, &workerTid]() {
        workerTid = std::this_thread::get_id();
        counter.fetch_add(1);
    });
    audioThreadTask->Start();
    // Bounded spin-wait until the registered job has executed at least once.
    for (int i = 0; i < SPIN_RETRIES; ++i) {
        if (counter.load() >= MIN_EXEC_COUNT) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_SLEEP_MS));
    }
    audioThreadTask->Stop(); // Joins the worker so workerTid is safe to read.
    ASSERT_GE(counter.load(), MIN_EXEC_COUNT);
    EXPECT_NE(workerTid, mainTid);
}

/**
* @tc.name  : Test Stop API
* @tc.type  : FUNC
* @tc.number: Stop_001
* @tc.desc  : Test that Stop joins the worker, resets state, and halts the job.
*/
HWTEST(AudioThreadTaskUnitTest, Stop_001, TestSize.Level1)
{
    std::atomic<int> counter{0};
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->RegisterJob([&counter]() { counter.fetch_add(1); });
    audioThreadTask->Start();
    for (int i = 0; i < SPIN_RETRIES; ++i) {
        if (counter.load() >= MIN_EXEC_COUNT) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_SLEEP_MS));
    }
    EXPECT_GE(counter.load(), MIN_EXEC_COUNT);
    audioThreadTask->Stop();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STOPPED);
    EXPECT_FALSE(audioThreadTask->CheckThreadIsRunning());
    int countAfterStop = counter.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(PAUSE_FREEZE_WAIT_MS));
    EXPECT_EQ(counter.load(), countAfterStop);
}

/**
* @tc.name  : Test Stop API
* @tc.type  : FUNC
* @tc.number: Stop_002
* @tc.desc  : Test Stop on a never-started task is a no-op and does not deadlock.
*/
HWTEST(AudioThreadTaskUnitTest, Stop_002, TestSize.Level1)
{
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->Stop();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STOPPED);
    EXPECT_EQ(audioThreadTask->loop_, nullptr);
}

/**
* @tc.name  : Test CheckThreadIsRunning API
* @tc.type  : FUNC
* @tc.number: CheckThreadIsRunning_001
* @tc.desc  : Test running flag across fresh, started, and stopped states.
*/
HWTEST(AudioThreadTaskUnitTest, CheckThreadIsRunning_001, TestSize.Level1)
{
    std::atomic<int> counter{0};
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    EXPECT_FALSE(audioThreadTask->CheckThreadIsRunning());
    audioThreadTask->RegisterJob([&counter]() { counter.fetch_add(1); });
    audioThreadTask->Start();
    for (int i = 0; i < SPIN_RETRIES; ++i) {
        if (counter.load() >= MIN_EXEC_COUNT) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_SLEEP_MS));
    }
    EXPECT_GE(counter.load(), MIN_EXEC_COUNT);
    EXPECT_TRUE(audioThreadTask->CheckThreadIsRunning());
    audioThreadTask->Stop();
    EXPECT_FALSE(audioThreadTask->CheckThreadIsRunning());
}

/**
* @tc.name  : Test Destructor
* @tc.type  : FUNC
* @tc.number: Destructor_001
* @tc.desc  : Test that destroying a running task joins the worker without hang.
*/
HWTEST(AudioThreadTaskUnitTest, Destructor_001, TestSize.Level1)
{
    std::atomic<int> counter{0};
    {
        std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
        audioThreadTask->RegisterJob([&counter]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_SLEEP_MS));
            counter.fetch_add(1);
        });
        audioThreadTask->Start();
        for (int i = 0; i < SPIN_RETRIES; ++i) {
            if (counter.load() >= MIN_EXEC_COUNT) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_SLEEP_MS));
        }
        EXPECT_GE(counter.load(), MIN_EXEC_COUNT);
        // Scope exit destroys the task; the dtor sets STOPPED and joins the worker.
    }
    int countAfterDtor = counter.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(PAUSE_FREEZE_WAIT_MS));
    EXPECT_EQ(counter.load(), countAfterDtor);
}

/**
* @tc.name  : Test Start Stop Start cycle
* @tc.type  : FUNC
* @tc.number: Start_Stop_Start_001
* @tc.desc  : Test that a newly registered job runs after a restart.
*/
HWTEST(AudioThreadTaskUnitTest, Start_Stop_Start_001, TestSize.Level1)
{
    std::atomic<int> firstCounter{0};
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->RegisterJob([&firstCounter]() { firstCounter.fetch_add(1); });
    audioThreadTask->Start();
    for (int i = 0; i < SPIN_RETRIES; ++i) {
        if (firstCounter.load() >= MIN_EXEC_COUNT) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_SLEEP_MS));
    }
    EXPECT_GE(firstCounter.load(), MIN_EXEC_COUNT);
    audioThreadTask->Stop();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STOPPED);

    std::atomic<int> secondCounter{0};
    audioThreadTask->RegisterJob([&secondCounter]() { secondCounter.fetch_add(1); });
    audioThreadTask->Start();
    for (int i = 0; i < SPIN_RETRIES; ++i) {
        if (secondCounter.load() >= MIN_EXEC_COUNT) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_SLEEP_MS));
    }
    EXPECT_GE(secondCounter.load(), MIN_EXEC_COUNT);
    audioThreadTask->Stop();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::STOPPED);
}

/**
* @tc.name  : Test Pause API
* @tc.type  : FUNC
* @tc.number: Pause_002
* @tc.desc  : Test that Pause freezes a running job.
*/
HWTEST(AudioThreadTaskUnitTest, Pause_002, TestSize.Level1)
{
    std::atomic<int> counter{0};
    std::unique_ptr<AudioThreadTask> audioThreadTask = std::make_unique<AudioThreadTask>("test");
    audioThreadTask->RegisterJob([&counter]() { counter.fetch_add(1); });
    audioThreadTask->Start();
    for (int i = 0; i < SPIN_RETRIES; ++i) {
        if (counter.load() >= MIN_EXEC_COUNT) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(SPIN_SLEEP_MS));
    }
    EXPECT_GE(counter.load(), MIN_EXEC_COUNT);
    audioThreadTask->Pause();
    EXPECT_EQ(audioThreadTask->state_, AudioThreadTask::RunningState::PAUSED);
    int countAtPause = counter.load();
    std::this_thread::sleep_for(std::chrono::milliseconds(PAUSE_FREEZE_WAIT_MS));
    EXPECT_EQ(counter.load(), countAtPause);
    audioThreadTask->Stop();
}
} // namespace AudioStandard
} // namespace OHOS