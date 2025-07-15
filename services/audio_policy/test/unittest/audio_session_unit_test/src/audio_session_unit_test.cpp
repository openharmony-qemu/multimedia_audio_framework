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
#include "audio_errors.h"
#include "audio_session.h"
#include "audio_session_service.h"
#include "audio_session_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSessionUnitTest::SetUpTestCase(void) {}
void AudioSessionUnitTest::TearDownTestCase(void) {}
void AudioSessionUnitTest::SetUp(void) {}
void AudioSessionUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_001.
* @tc.desc  : Test AddStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_001, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 1;
    audioSession->AddStreamInfo(audioInterrupt);
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_002.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_002, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 1;
    audioSession->AddStreamInfo(audioInterrupt);
    audioSession->RemoveStreamInfo(0);
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_003.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_003, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 10;
    audioSession->AddStreamInfo(audioInterrupt);
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_004.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_004, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 0;
    audioSession->AddStreamInfo(audioInterrupt);
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
    EXPECT_TRUE(audioSession->IsAudioSessionEmpty());
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_005.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_005, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t i = 1;
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = i;
    audioSession->AddStreamInfo(audioInterrupt);

    AudioInterrupt audioInterrupt2;
    audioInterrupt2.streamId = i + 1;
    audioSession->AddStreamInfo(audioInterrupt2);

    audioSession->RemoveStreamInfo(audioInterrupt2.streamId);
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_006.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_006, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t i = 1;
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = i;

    audioSession->AddStreamInfo(audioInterrupt);
    EXPECT_FALSE(audioSession->IsAudioSessionEmpty());
    audioSession->RemoveStreamInfo(audioInterrupt.streamId);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_007.
* @tc.desc  : Test IsAudioRendererEmpty.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_007, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t streamId = 1;
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = streamId;
    audioInterrupt.audioFocusType.streamType = STREAM_DEFAULT;

    audioSession->AddStreamInfo(audioInterrupt);
    EXPECT_TRUE(audioSession->IsAudioRendererEmpty());
    audioSession->RemoveStreamInfo(streamId);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_008.
* @tc.desc  : Test IsAudioRendererEmpty.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_008, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t streamId = 1;
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = streamId;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;

    audioSession->AddStreamInfo(audioInterrupt);
    EXPECT_FALSE(audioSession->IsAudioRendererEmpty());
}
} // namespace AudioStandard
} // namespace OHOS
