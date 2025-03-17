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

#include "audio_errors.h"
#include "audio_session_service.h"
#include "audio_session_service_unit_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSessionServiceUnitTest::SetUpTestCase(void) {}
void AudioSessionServiceUnitTest::TearDownTestCase(void) {}
void AudioSessionServiceUnitTest::SetUp(void) {}
void AudioSessionServiceUnitTest::TearDown(void) {}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_001.
* @tc.desc  : Test IsSameTypeForAudioSession.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_001, TestSize.Level1)
{
    auto audioSessionService = std::make_shared<AudioSessionService>();
    auto ret = audioSessionService->IsSameTypeForAudioSession(AudioStreamType::STREAM_MUSIC,
        AudioStreamType::STREAM_MUSIC);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_002.
* @tc.desc  : Test DeactivateAudioSessionInternal.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_002, TestSize.Level1)
{
    int32_t callerPid = 0;
    auto audioSessionService = std::make_shared<AudioSessionService>();

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);

    audioSessionService->sessionMap_[callerPid] = audioSession;
    auto ret = audioSessionService->DeactivateAudioSessionInternal(callerPid, true);
    EXPECT_EQ(ret, SUCCESS);

    audioSessionService->sessionMap_[callerPid] = audioSession;
    ret = audioSessionService->DeactivateAudioSessionInternal(callerPid, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_003.
* @tc.desc  : Test SetSessionTimeOutCallback.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_003, TestSize.Level1)
{
    int32_t callerPid = 0;
    auto audioSessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(audioSessionService != nullptr);

    std::shared_ptr<SessionTimeOutCallback> timeOutCallback = nullptr;
    auto ret = audioSessionService->SetSessionTimeOutCallback(timeOutCallback);
    EXPECT_EQ(ret, -1);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_004.
* @tc.desc  : Test GetAudioSessionByPid.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_004, TestSize.Level1)
{
    int32_t callerPid = 10;
    auto audioSessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(audioSessionService != nullptr);

    auto ret = audioSessionService->GetAudioSessionByPid(callerPid);
    EXPECT_TRUE(ret == nullptr);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_005.
* @tc.desc  : Test OnAudioSessionTimeOut.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_005, TestSize.Level1)
{
    int32_t callerPid = 0;
    auto audioSessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(audioSessionService != nullptr);

    audioSessionService->OnAudioSessionTimeOut(callerPid);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_006.
* @tc.desc  : Test AudioSessionInfoDump.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_006, TestSize.Level1)
{
    std::string dumpString = "test";
    auto audioSessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(audioSessionService != nullptr);

    audioSessionService->AudioSessionInfoDump(dumpString);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_007.
* @tc.desc  : Test AudioSessionInfoDump.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_007, TestSize.Level1)
{
    int32_t callerPid = 0;
    std::string dumpString = "test";
    auto audioSessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(audioSessionService != nullptr);

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);
    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);

    audioSessionService->sessionMap_[callerPid] = audioSession;
    audioSessionService->AudioSessionInfoDump(dumpString);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_008.
* @tc.desc  : Test AudioSessionInfoDump.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_008, TestSize.Level1)
{
    int32_t callerPid = 0;
    std::string dumpString = "test";
    auto audioSessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(audioSessionService != nullptr);

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor);
    AudioInterrupt audioInterrupt;
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);
    audioSession->interruptMap_[callerPid] = interruptPair;

    audioSessionService->sessionMap_[callerPid] = audioSession;
    audioSessionService->AudioSessionInfoDump(dumpString);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_009.
* @tc.desc  : Test AudioSessionInfoDump.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_009, TestSize.Level1)
{
    int32_t callerPid = 0;
    std::string dumpString = "test";
    auto audioSessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(audioSessionService != nullptr);

    audioSessionService->sessionMap_[callerPid] = nullptr;
    audioSessionService->AudioSessionInfoDump(dumpString);
}
} // namespace AudioStandard
} // namespace OHOS
