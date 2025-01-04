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
} // namespace AudioStandard
} // namespace OHOS