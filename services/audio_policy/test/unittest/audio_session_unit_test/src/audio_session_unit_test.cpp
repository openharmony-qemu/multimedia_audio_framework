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
* @tc.desc  : Test AddAudioInterrpt.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_001, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    auto ret = audioSession->AddAudioInterrpt(interruptPair);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_002.
* @tc.desc  : Test RemoveAudioInterrpt.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_002, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    auto ret = audioSession->RemoveAudioInterrpt(interruptPair);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_003.
* @tc.desc  : Test RemoveAudioInterrpt.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_003, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[audioInterrupt.streamId] = interruptPair;
    auto ret = audioSession->RemoveAudioInterrpt(interruptPair);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_004.
* @tc.desc  : Test RemoveAudioInterrpt.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_004, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[audioInterrupt.streamId] = interruptPair;
    auto ret = audioSession->RemoveAudioInterrpt(interruptPair);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_005.
* @tc.desc  : Test RemoveAudioInterrpt.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_005, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t i = 1;
    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[audioInterrupt.streamId] = interruptPair;
    audioSession->interruptMap_[i] = interruptPair;
    auto ret = audioSession->RemoveAudioInterrpt(interruptPair);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_006.
* @tc.desc  : Test RemoveAudioInterrpt.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_006, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t i = 1;
    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[audioInterrupt.streamId] = interruptPair;
    audioSession->interruptMap_[i] = interruptPair;
    auto ret = audioSession->RemoveAudioInterrpt(interruptPair);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_007.
* @tc.desc  : Test RemoveAudioInterrptByStreamId.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_007, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t streamId = 10;
    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[streamId] = interruptPair;
    auto ret = audioSession->RemoveAudioInterrptByStreamId(streamId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_008.
* @tc.desc  : Test RemoveAudioInterrptByStreamId.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_008, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t streamId = 10;
    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[streamId] = interruptPair;
    auto ret = audioSession->RemoveAudioInterrptByStreamId(streamId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_009.
* @tc.desc  : Test RemoveAudioInterrptByStreamId.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_009, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t streamId0 = 1;
    uint32_t streamId = 10;
    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[streamId] = interruptPair;
    audioSession->interruptMap_[streamId0] = interruptPair;
    auto ret = audioSession->RemoveAudioInterrptByStreamId(streamId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_010.
* @tc.desc  : Test RemoveAudioInterrptByStreamId.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_010, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t streamId0 = 1;
    uint32_t streamId = 10;
    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[streamId] = interruptPair;
    audioSession->interruptMap_[streamId0] = interruptPair;
    auto ret = audioSession->RemoveAudioInterrptByStreamId(streamId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_011.
* @tc.desc  : Test IsAudioRendererEmpty.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_011, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t streamId = 1;
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.streamType = STREAM_DEFAULT;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[streamId] = interruptPair;
    auto ret = audioSession->IsAudioRendererEmpty();
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioSession.
* @tc.number: AudioSessionUnitTest_012.
* @tc.desc  : Test IsAudioRendererEmpty.
*/
HWTEST_F(AudioSessionUnitTest, AudioSessionUnitTest_012, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = std::make_shared<AudioSessionService>();
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    uint32_t streamId = 1;
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[streamId] = interruptPair;
    auto ret = audioSession->IsAudioRendererEmpty();
    EXPECT_FALSE(ret);
}
} // namespace AudioStandard
} // namespace OHOS
