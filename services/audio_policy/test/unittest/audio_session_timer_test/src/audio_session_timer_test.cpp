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

#include <iostream>
#include "audio_session_timer_test.h"
#include "ipc_skeleton.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSessionTimerTest::SetUpTestCase(void) {}
void AudioSessionTimerTest::TearDownTestCase(void) {}
void AudioSessionTimerTest::SetUp(void) {}
void AudioSessionTimerTest::TearDown(void) {}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_001.
* @tc.desc  : Test StartTimer.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_001, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    audioSession_->StartTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
    audioSession_->StopTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_002.
* @tc.desc  : Test StartTimer.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_002, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    audioSession_->isThreadRunning_.store(false);
    audioSession_->timerThread_ = std::make_shared<std::thread>();
    audioSession_->timerThread_ = nullptr;
    audioSession_->StartTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
    audioSession_->StopTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_003.
* @tc.desc  : Test StartTimer.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_003, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    audioSession_->isThreadRunning_.store(true);
    audioSession_->timerThread_ = std::make_shared<std::thread>();
    audioSession_->timerThread_ = nullptr;
    audioSession_->StartTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
    audioSession_->StopTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_004.
* @tc.desc  : Test StartTimer.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_004, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    audioSession_->isThreadRunning_.store(true);
    audioSession_->timerThread_ = std::make_shared<std::thread>();
    audioSession_->StartTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
    audioSession_->StopTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_005.
* @tc.desc  : Test StartTimer.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_005, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    audioSession_->timerMap_.insert({callerPid, std::time_t(nullptr)});
    audioSession_->StartTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
    audioSession_->StopTimer(callerPid);
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_006.
* @tc.desc  : Test IsSessionTimerRunning.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_006, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    audioSession_->IsSessionTimerRunning(callerPid);
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_007.
* @tc.desc  : Test TimerLoopFunc.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_007, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    audioSession_->TimerLoopFunc();
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_008.
* @tc.desc  : Test TimerLoopFunc.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_008, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    audioSession_->isThreadRunning_.store(true);
    audioSession_->timerMap_.insert({callerPid, std::time_t(nullptr)});
    audioSession_->TimerLoopFunc();
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_009.
* @tc.desc  : Test SendSessionTimeOutCallback.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_009, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    audioSession_->SendSessionTimeOutCallback(callerPid);
    EXPECT_NE(audioSession_, nullptr);
}

/**
* @tc.name  : Test AudioSessionTimer.
* @tc.number: AudioSessionTimer_010.
* @tc.desc  : Test SendSessionTimeOutCallback.
*/
HWTEST(AudioSessionTimerTest, AudioSessionTimer_010, TestSize.Level1)
{
    auto audioSession_ = std::make_shared<AudioSessionTimer>();
    auto sessionTimerCallback = std::make_shared<ConcreteAudioSessionTimerCallback>();
    int32_t result = audioSession_->SetAudioSessionTimerCallback(sessionTimerCallback);
    EXPECT_EQ(result, SUCCESS);

    sessionTimerCallback = nullptr;
    result = audioSession_->SetAudioSessionTimerCallback(sessionTimerCallback);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}
} // AudioStandardnamespace
} // OHOSnamespace