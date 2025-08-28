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

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static const uint32_t START_FAKE_STREAM_ID = 888;

class MockSessionTimeOutCallback : public SessionTimeOutCallback {
public:
    ~MockSessionTimeOutCallback() = default;

    void OnSessionTimeout(const int32_t pid) override {}
};

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
    AudioInterrupt audioInterrupt = {};
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
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 0;
    audioSession->streamsInSession_.push_back(audioInterrupt);

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

/**
* @tc.name  : Test SetAudioSessionScene
* @tc.number: SetAudioSessionSceneTest
* @tc.desc  : Test SetAudioSessionScene
*/
HWTEST_F(AudioSessionServiceUnitTest, SetAudioSessionSceneTest, TestSize.Level1)
{
    int32_t fakePid = 123;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = sessionService->ActivateAudioSession(fakePid, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_TRUE(sessionService->IsAudioSessionActivated(fakePid));
    EXPECT_TRUE(sessionService->IsAudioSessionFocusMode(fakePid));
}

/**
* @tc.name  : Test GetAudioSessionStreamUsage
* @tc.number: GetAudioSessionStreamUsage
* @tc.desc  : Test GetAudioSessionStreamUsage
*/
HWTEST_F(AudioSessionServiceUnitTest, GetAudioSessionStreamUsage, TestSize.Level1)
{
    int32_t fakePid = 123;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    StreamUsage usage = sessionService->GetAudioSessionStreamUsage(fakePid);
    EXPECT_EQ(STREAM_USAGE_INVALID, usage);
}

/**
* @tc.name  : Test GetAudioSessionStreamUsage
* @tc.number: GetAudioSessionStreamUsage_001
* @tc.desc  : Test GetAudioSessionStreamUsage
*/
HWTEST_F(AudioSessionServiceUnitTest, GetAudioSessionStreamUsage_001, TestSize.Level1)
{
    int32_t fakePid = 123;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    StreamUsage usage = sessionService->GetAudioSessionStreamUsage(fakePid);
    EXPECT_EQ(STREAM_USAGE_MEDIA, usage);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_010.
* @tc.desc  : Test ActivateAudioSession.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_010, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    sessionService->sessionMap_.emplace(callerPid, nullptr);
    EXPECT_EQ(sessionService->ActivateAudioSession(callerPid, strategy), ERROR);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_011.
* @tc.desc  : Test DeactivateAudioSessionInternal.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_011, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    bool isSessionTimeout = false;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, nullptr));
    EXPECT_EQ(sessionService->DeactivateAudioSessionInternal(callerPid, isSessionTimeout), ERR_ILLEGAL_STATE);

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    uint32_t fakeStreamId = 1000;
    audioSession->SaveFakeStreamId(fakeStreamId);
    sessionService->sessionMap_[callerPid] = audioSession;
    EXPECT_EQ(sessionService->DeactivateAudioSessionInternal(callerPid, isSessionTimeout), SUCCESS);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_012.
* @tc.desc  : Test OnAudioSessionTimeOut.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_012, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    auto mockTimeOutCallback = std::make_shared<MockSessionTimeOutCallback>();
    sessionService->SetSessionTimeOutCallback(mockTimeOutCallback);
    EXPECT_NO_THROW(sessionService->OnAudioSessionTimeOut(callerPid));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_013.
* @tc.desc  : Test SetAudioSessionScene.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_013, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    AudioSessionScene scene = AudioSessionScene::MEDIA;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, nullptr));
    EXPECT_EQ(sessionService->SetAudioSessionScene(callerPid, scene), SUCCESS);

    sessionService->sessionMap_.clear();
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    EXPECT_EQ(sessionService->SetAudioSessionScene(callerPid, scene), SUCCESS);

    callerPid = 2;
    EXPECT_EQ(sessionService->SetAudioSessionScene(callerPid, scene), SUCCESS);
};

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_014.
* @tc.desc  : Test ShouldExcludeStreamType.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_014, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.audioFocusType.streamType = STREAM_NOTIFICATION;
    EXPECT_TRUE(sessionService->ShouldExcludeStreamType(audioInterrupt));

    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    EXPECT_TRUE(sessionService->ShouldExcludeStreamType(audioInterrupt));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_015.
* @tc.desc  : Test ShouldBypassFocusForStream.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_015, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = callerPid;
    audioInterrupt.audioFocusType.streamType = STREAM_NOTIFICATION;
    EXPECT_FALSE(sessionService->ShouldBypassFocusForStream(audioInterrupt));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_016.
* @tc.desc  : Test GenerateFakeAudioInterrupt.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_016, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, nullptr));

    sessionService->GenerateFakeStreamId(callerPid);
    AudioInterrupt fakeAudioInterrupt = sessionService->GenerateFakeAudioInterrupt(callerPid);
    EXPECT_EQ(fakeAudioInterrupt.streamId, 0);

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    audioSession->fakeStreamId_ = 1;
    sessionService->sessionMap_[callerPid] = audioSession;
    fakeAudioInterrupt = sessionService->GenerateFakeAudioInterrupt(callerPid);
    EXPECT_EQ(fakeAudioInterrupt.streamId, 1);

    callerPid = 2;
    sessionService->GenerateFakeStreamId(callerPid);
    fakeAudioInterrupt = sessionService->GenerateFakeAudioInterrupt(callerPid);
    EXPECT_EQ(fakeAudioInterrupt.streamId, 0);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_017.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_017, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = callerPid;
    audioInterrupt.isAudioSessionInterrupt = true;
    EXPECT_NO_THROW(sessionService->RemoveStreamInfo(audioInterrupt));

    audioInterrupt.isAudioSessionInterrupt = false;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    EXPECT_NO_THROW(sessionService->RemoveStreamInfo(audioInterrupt));

    callerPid = 2;
    EXPECT_NO_THROW(sessionService->RemoveStreamInfo(audioInterrupt));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_018.
* @tc.desc  : Test ClearStreamInfo.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_018, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    EXPECT_NO_THROW(sessionService->ClearStreamInfo(callerPid));

    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, nullptr));
    EXPECT_NO_THROW(sessionService->ClearStreamInfo(callerPid));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_019.
* @tc.desc  : Test SetSessionDefaultOutputDevice.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_019, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    EXPECT_EQ(sessionService->SetSessionDefaultOutputDevice(callerPid, deviceType), SUCCESS);

    callerPid = 2;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    EXPECT_EQ(sessionService->SetSessionDefaultOutputDevice(callerPid, deviceType), SUCCESS);

    sessionService->sessionMap_[callerPid].reset();
    EXPECT_EQ(sessionService->SetSessionDefaultOutputDevice(callerPid, deviceType), SUCCESS);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_020.
* @tc.desc  : Test GetSessionDefaultOutputDevice.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_020, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    EXPECT_EQ(sessionService->GetSessionDefaultOutputDevice(callerPid), DEVICE_TYPE_INVALID);

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    audioSession->defaultDeviceType_ = DEVICE_TYPE_DEFAULT;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    EXPECT_EQ(sessionService->GetSessionDefaultOutputDevice(callerPid), DEVICE_TYPE_DEFAULT);

    sessionService->sessionMap_[callerPid].reset();
    EXPECT_EQ(sessionService->GetSessionDefaultOutputDevice(callerPid), DEVICE_TYPE_INVALID);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_021.
* @tc.desc  : Test IsStreamAllowedToSetDevice.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_021, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    uint32_t streamId = 1001;
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = streamId;

    std::shared_ptr<AudioSession> audioSession1 = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    audioSession1->streamsInSession_.push_back(audioInterrupt);
    audioSession1->state_ == AudioSessionState::SESSION_ACTIVE;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession1));
    callerPid = 2;
    std::shared_ptr<AudioSession> audioSession2 = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    audioSession2->streamsInSession_.push_back(audioInterrupt);
    audioSession2->state_ == AudioSessionState::SESSION_NEW;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession2));
    callerPid = 3;
    std::shared_ptr<AudioSession> audioSession3 = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession3));
    callerPid = 4;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, nullptr));

    EXPECT_TRUE(sessionService->IsStreamAllowedToSetDevice(streamId));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_022.
* @tc.desc  : Test IsSessionNeedToFetchOutputDevice.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_022, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t callerPid = 1;
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, nullptr));
    EXPECT_FALSE(sessionService->IsSessionNeedToFetchOutputDevice(callerPid));

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(callerPid, strategy, nullptr);
    audioSession->needToFetch_ = true;
    sessionService->sessionMap_[callerPid] = audioSession;
    EXPECT_TRUE(sessionService->IsSessionNeedToFetchOutputDevice(callerPid));

    callerPid = 2;
    EXPECT_FALSE(sessionService->IsSessionNeedToFetchOutputDevice(callerPid));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_023.
* @tc.desc  : Test NotifyAppStateChange.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_023, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t pid = 1;
    bool isBackState = false;
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, isBackState));

    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(pid, nullptr));
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, isBackState));

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(pid, strategy, nullptr);
    audioSession->audioSessionScene_ = AudioSessionScene::INVALID;
    sessionService->sessionMap_[pid] = audioSession;
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, false));
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, true));

    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, false));
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, true));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_024.
* @tc.desc  : Test NotifyAppStateChange.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_024, TestSize.Level1)
{
    auto sessionService = std::make_shared<AudioSessionService>();
    int32_t pid = 1;
    AudioSessionStrategy strategy;
    AudioInterrupt audioInterrupt = {};
    std::shared_ptr<AudioSession> audioSession = std::make_shared<AudioSession>(pid, strategy, nullptr);
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioSession->streamsInSession_.clear();
    sessionService->sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(pid, audioSession));
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, true));

    audioSession->streamsInSession_.push_back(audioInterrupt);
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, true));

    audioSession->audioSessionScene_ = AudioSessionScene::INVALID;
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, true));

    audioSession->state_ = AudioSessionState::SESSION_NEW;
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, true));

    audioSession->streamsInSession_.clear();
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, true));

    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    EXPECT_NO_THROW(sessionService->NotifyAppStateChange(pid, true));
}

/*
* @tc.name  : Test RemoveStreamInfo
* @tc.number: RemoveStreamInfoTest
* @tc.desc  : Test RemoveStreamInfo
*/
HWTEST_F(AudioSessionServiceUnitTest, RemoveStreamInfoTest, TestSize.Level1)
{
    int32_t fakePid = 123;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    sessionService->sessionMap_.clear();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = 0;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterrupt.streamId = 0;
    sessionService->RemoveStreamInfo(audioInterrupt);

    audioInterrupt.isAudioSessionInterrupt = false;
    sessionService->RemoveStreamInfo(audioInterrupt);

    audioInterrupt.pid = fakePid;
    sessionService->RemoveStreamInfo(audioInterrupt);
}

/**
* @tc.name  : Test ClearStreamInfo
* @tc.number: ClearStreamInfoTest
* @tc.desc  : Test ClearStreamInfo
*/
HWTEST_F(AudioSessionServiceUnitTest, ClearStreamInfoTest, TestSize.Level1)
{
    int32_t fakePid = 123;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    sessionService->sessionMap_.clear();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    sessionService->ClearStreamInfo(0);

    sessionService->sessionMap_[0] = nullptr;
    sessionService->ClearStreamInfo(0);

    sessionService->ClearStreamInfo(fakePid);
}

/**
* @tc.name  : Test SetSessionDefaultOutputDevice
* @tc.number: SetSessionDefaultOutputDeviceTest
* @tc.desc  : Test SetSessionDefaultOutputDevice
*/
HWTEST_F(AudioSessionServiceUnitTest, SetSessionDefaultOutputDeviceTest, TestSize.Level1)
{
    int32_t fakePid = 123;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    sessionService->sessionMap_.clear();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_EQ(sessionService->SetSessionDefaultOutputDevice(0, DEVICE_TYPE_INVALID), ERROR_INVALID_PARAM);

    sessionService->sessionMap_[1] = nullptr;
    EXPECT_EQ(sessionService->SetSessionDefaultOutputDevice(1, DEVICE_TYPE_INVALID), ERROR_INVALID_PARAM);

    EXPECT_EQ(sessionService->SetSessionDefaultOutputDevice(fakePid, DEVICE_TYPE_INVALID), ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test GetSessionDefaultOutputDevice
* @tc.number: GetSessionDefaultOutputDeviceTest
* @tc.desc  : Test GetSessionDefaultOutputDevice
*/
HWTEST_F(AudioSessionServiceUnitTest, GetSessionDefaultOutputDeviceTest, TestSize.Level1)
{
    int32_t fakePid = 100;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    sessionService->sessionMap_.clear();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_EQ(sessionService->GetSessionDefaultOutputDevice(0), DEVICE_TYPE_INVALID);

    sessionService->sessionMap_[1] = nullptr;
    EXPECT_EQ(sessionService->GetSessionDefaultOutputDevice(1), DEVICE_TYPE_INVALID);

    EXPECT_EQ(sessionService->GetSessionDefaultOutputDevice(fakePid), DEVICE_TYPE_INVALID);
}

/**
* @tc.name  : Test IsStreamAllowedToSetDevice
* @tc.number: IsStreamAllowedToSetDeviceTest
* @tc.desc  : Test IsStreamAllowedToSetDevice
*/
HWTEST_F(AudioSessionServiceUnitTest, IsStreamAllowedToSetDeviceTest, TestSize.Level1)
{
    int32_t fakePid = 100;
    int32_t fakeSessionId = 100;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    sessionService->sessionMap_.clear();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_TRUE(sessionService->IsStreamAllowedToSetDevice(0));

    sessionService->sessionMap_[1] = nullptr;
    EXPECT_TRUE(sessionService->IsStreamAllowedToSetDevice(1));

    AudioInterrupt incomingInterrupt = {};
    incomingInterrupt.streamId = fakeSessionId;
    sessionService->sessionMap_[fakePid]->streamsInSession_.push_back(incomingInterrupt);
    EXPECT_TRUE(sessionService->IsStreamAllowedToSetDevice(fakeSessionId));

    sessionService->sessionMap_[fakePid]->state_ = AudioSessionState::SESSION_ACTIVE;
    EXPECT_TRUE(sessionService->IsStreamAllowedToSetDevice(fakeSessionId));
}

/**
* @tc.name  : Test IsSessionNeedToFetchOutputDevice
* @tc.number: IsSessionNeedToFetchOutputDeviceTest
* @tc.desc  : Test IsSessionNeedToFetchOutputDevice
*/
HWTEST_F(AudioSessionServiceUnitTest, IsSessionNeedToFetchOutputDeviceTest, TestSize.Level1)
{
    int32_t fakePid = 100;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    sessionService->sessionMap_.clear();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_FALSE(sessionService->IsSessionNeedToFetchOutputDevice(0));

    sessionService->sessionMap_[1] = nullptr;
    EXPECT_FALSE(sessionService->IsSessionNeedToFetchOutputDevice(1));

    EXPECT_FALSE(sessionService->IsSessionNeedToFetchOutputDevice(fakePid));
}

/**
* @tc.name  : Test NotifyAppStateChange
* @tc.number: NotifyAppStateChangeTest
* @tc.desc  : Test NotifyAppStateChange
*/
HWTEST_F(AudioSessionServiceUnitTest, NotifyAppStateChangeTest, TestSize.Level1)
{
    int32_t fakePid = 100;
    int32_t fakeSessionId = 100;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    ASSERT_TRUE(sessionService != nullptr);
    sessionService->sessionMap_.clear();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    sessionService->NotifyAppStateChange(0, true);

    sessionService->sessionMap_[1] = nullptr;
    sessionService->NotifyAppStateChange(1, true);

    sessionService->NotifyAppStateChange(fakePid, false);

    sessionService->sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    sessionService->NotifyAppStateChange(fakePid, false);

    sessionService->sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::INVALID;
    sessionService->NotifyAppStateChange(fakePid, true);

    sessionService->sessionMap_[fakePid]->state_ = AudioSessionState::SESSION_ACTIVE;
    sessionService->NotifyAppStateChange(fakePid, true);

    sessionService->sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    AudioInterrupt incomingInterrupt = {};
    incomingInterrupt.streamId = fakeSessionId;
    sessionService->sessionMap_[fakePid]->streamsInSession_.push_back(incomingInterrupt);
    sessionService->NotifyAppStateChange(fakeSessionId, false);

    sessionService->sessionMap_[fakePid]->streamsInSession_.clear();
    sessionService->NotifyAppStateChange(fakeSessionId, false);
}

} // namespace AudioStandard
} // namespace OHOS
