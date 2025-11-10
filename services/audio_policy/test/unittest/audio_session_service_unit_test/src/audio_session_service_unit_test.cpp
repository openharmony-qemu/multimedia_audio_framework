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
void AudioSessionServiceUnitTest::TearDown(void)
{
    audioSessionService_.sessionMap_.clear();
    audioSessionService_.timeOutCallback_.reset();
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_001.
* @tc.desc  : Test IsSameTypeForAudioSession.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_001, TestSize.Level1)
{
    auto ret = audioSessionService_.IsSameTypeForAudioSession(AudioStreamType::STREAM_MUSIC,
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

    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);

    audioSessionService_.sessionMap_[callerPid] = audioSession;
    auto ret = audioSessionService_.DeactivateAudioSessionInternal(callerPid, true);
    EXPECT_EQ(ret, SUCCESS);

    audioSessionService_.sessionMap_[callerPid] = audioSession;
    ret = audioSessionService_.DeactivateAudioSessionInternal(callerPid, false);
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
    std::shared_ptr<SessionTimeOutCallback> timeOutCallback = nullptr;
    auto ret = audioSessionService_.SetSessionTimeOutCallback(timeOutCallback);
    EXPECT_EQ(ret, -1);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_005.
* @tc.desc  : Test OnAudioSessionTimeOut.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_005, TestSize.Level1)
{
    int32_t callerPid = 0;
    audioSessionService_.OnAudioSessionTimeOut(callerPid);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_006.
* @tc.desc  : Test AudioSessionInfoDump.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_006, TestSize.Level1)
{
    std::string dumpString = "test";
    audioSessionService_.AudioSessionInfoDump(dumpString);
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

    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    AudioInterrupt audioInterrupt = {};
    auto interruptPair = std::pair<AudioInterrupt, AudioFocuState>(audioInterrupt, AudioFocuState::DUCK);

    audioSessionService_.sessionMap_[callerPid] = audioSession;
    audioSessionService_.AudioSessionInfoDump(dumpString);
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

    AudioSessionStrategy strategy;
    auto audioSession = std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = 0;
    audioSession->streamsInSession_.push_back(audioInterrupt);

    audioSessionService_.sessionMap_[callerPid] = audioSession;
    audioSessionService_.AudioSessionInfoDump(dumpString);
}

/**
* @tc.name  : Test SetAudioSessionScene
* @tc.number: SetAudioSessionSceneTest
* @tc.desc  : Test SetAudioSessionScene
*/
HWTEST_F(AudioSessionServiceUnitTest, SetAudioSessionSceneTest, TestSize.Level1)
{
    int32_t fakePid = 123;
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = audioSessionService_.ActivateAudioSession(fakePid, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_TRUE(audioSessionService_.IsAudioSessionActivated(fakePid));
    EXPECT_TRUE(audioSessionService_.IsAudioSessionFocusMode(fakePid));
}

/**
* @tc.name  : Test GetAudioSessionStreamUsage
* @tc.number: GetAudioSessionStreamUsage
* @tc.desc  : Test GetAudioSessionStreamUsage
*/
HWTEST_F(AudioSessionServiceUnitTest, GetAudioSessionStreamUsage, TestSize.Level1)
{
    int32_t fakePid = 123;
    StreamUsage usage = audioSessionService_.GetAudioSessionStreamUsage(fakePid);
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
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    StreamUsage usage = audioSessionService_.GetAudioSessionStreamUsage(fakePid);
    EXPECT_EQ(STREAM_USAGE_MEDIA, usage);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_010.
* @tc.desc  : Test ActivateAudioSession.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_010, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    audioSessionService_.sessionMap_.emplace(callerPid, nullptr);
    EXPECT_EQ(audioSessionService_.ActivateAudioSession(callerPid, strategy), ERROR);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_011.
* @tc.desc  : Test DeactivateAudioSessionInternal.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_011, TestSize.Level1)
{
    int32_t callerPid = 1;
    bool isSessionTimeout = false;
    EXPECT_EQ(audioSessionService_.DeactivateAudioSessionInternal(callerPid, isSessionTimeout), ERR_ILLEGAL_STATE);

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    uint32_t fakeStreamId = 1000;
    audioSession->SaveFakeStreamId(fakeStreamId);
    audioSessionService_.sessionMap_[callerPid] = audioSession;
    EXPECT_EQ(audioSessionService_.DeactivateAudioSessionInternal(callerPid, isSessionTimeout), SUCCESS);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_012.
* @tc.desc  : Test OnAudioSessionTimeOut.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_012, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    auto mockTimeOutCallback = std::make_shared<MockSessionTimeOutCallback>();
    audioSessionService_.SetSessionTimeOutCallback(mockTimeOutCallback);
    EXPECT_NO_THROW(audioSessionService_.OnAudioSessionTimeOut(callerPid));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_013.
* @tc.desc  : Test SetAudioSessionScene.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_013, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionScene scene = AudioSessionScene::MEDIA;
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, nullptr));
    EXPECT_EQ(ERROR, audioSessionService_.SetAudioSessionScene(callerPid, scene));

    callerPid = 2;
    EXPECT_EQ(SUCCESS, audioSessionService_.SetAudioSessionScene(callerPid, scene));
};

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_014.
* @tc.desc  : Test ShouldExcludeStreamType.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_014, TestSize.Level1)
{
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.audioFocusType.streamType = STREAM_NOTIFICATION;
    EXPECT_TRUE(audioSessionService_.ShouldExcludeStreamType(audioInterrupt));

    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    EXPECT_TRUE(audioSessionService_.ShouldExcludeStreamType(audioInterrupt));
}

/**
* @tc.name  : Test ShouldExcludeStreamTypeInner.
* @tc.number: ShouldExcludeStreamTypeInnerTest.
* @tc.desc  : Test ShouldExcludeStreamTypeInner.
*/
HWTEST_F(AudioSessionServiceUnitTest, ShouldExcludeStreamTypeInnerTest, TestSize.Level1)
{
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.audioFocusType.streamType = STREAM_NOTIFICATION;
    EXPECT_TRUE(audioSessionService_.ShouldExcludeStreamTypeInner(audioInterrupt));

    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_PLAYBACK_CAPTURE;
    EXPECT_TRUE(audioSessionService_.ShouldExcludeStreamTypeInner(audioInterrupt));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_015.
* @tc.desc  : Test ShouldBypassFocusForStream.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_015, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = callerPid;
    audioInterrupt.audioFocusType.streamType = STREAM_NOTIFICATION;
    EXPECT_FALSE(audioSessionService_.ShouldBypassFocusForStream(audioInterrupt));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_016.
* @tc.desc  : Test GenerateFakeAudioInterrupt.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_016, TestSize.Level1)
{
    int32_t callerPid = 1;
    audioSessionService_.GenerateFakeStreamId(callerPid);
    AudioInterrupt fakeAudioInterrupt = audioSessionService_.GenerateFakeAudioInterrupt(callerPid);
    EXPECT_EQ(fakeAudioInterrupt.streamId, 0);

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSession->fakeStreamId_ = 1;
    audioSessionService_.sessionMap_[callerPid] = audioSession;
    fakeAudioInterrupt = audioSessionService_.GenerateFakeAudioInterrupt(callerPid);
    EXPECT_EQ(fakeAudioInterrupt.streamId, 1);

    callerPid = 2;
    audioSessionService_.GenerateFakeStreamId(callerPid);
    fakeAudioInterrupt = audioSessionService_.GenerateFakeAudioInterrupt(callerPid);
    EXPECT_EQ(fakeAudioInterrupt.streamId, 0);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_017.
* @tc.desc  : Test RemoveStreamInfo.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_017, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = callerPid;
    audioInterrupt.isAudioSessionInterrupt = true;
    EXPECT_NO_THROW(audioSessionService_.RemoveStreamInfo(audioInterrupt.pid, audioInterrupt.streamId));

    audioInterrupt.isAudioSessionInterrupt = false;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    EXPECT_NO_THROW(audioSessionService_.RemoveStreamInfo(audioInterrupt.pid, audioInterrupt.streamId));

    callerPid = 2;
    EXPECT_NO_THROW(audioSessionService_.RemoveStreamInfo(audioInterrupt.pid, audioInterrupt.streamId));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_018.
* @tc.desc  : Test ClearStreamInfo.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_018, TestSize.Level1)
{
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    EXPECT_NO_THROW(audioSessionService_.ClearStreamInfo(callerPid));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_019.
* @tc.desc  : Test SetSessionDefaultOutputDevice.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_019, TestSize.Level1)
{
    int32_t callerPid = 1;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    EXPECT_EQ(audioSessionService_.SetSessionDefaultOutputDevice(callerPid, deviceType), SUCCESS);

    callerPid = 2;
    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    EXPECT_EQ(audioSessionService_.SetSessionDefaultOutputDevice(callerPid, deviceType), SUCCESS);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_020.
* @tc.desc  : Test GetSessionDefaultOutputDevice.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_020, TestSize.Level1)
{
    int32_t callerPid = 1;

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession));
    EXPECT_EQ(audioSessionService_.GetSessionDefaultOutputDevice(callerPid), DEVICE_TYPE_INVALID);

    callerPid = 2;
    EXPECT_EQ(audioSessionService_.GetSessionDefaultOutputDevice(callerPid), DEVICE_TYPE_INVALID);
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_021.
* @tc.desc  : Test IsStreamAllowedToSetDevice.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_021, TestSize.Level1)
{
    uint32_t streamId = 1001;
    int32_t callerPid = 1;
    AudioSessionStrategy strategy;
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.streamId = streamId;

    std::shared_ptr<AudioSession> audioSession1 =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSession1->streamsInSession_.push_back(audioInterrupt);
    audioSession1->state_ == AudioSessionState::SESSION_ACTIVE;
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession1));
    callerPid = 2;
    std::shared_ptr<AudioSession> audioSession2 =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSession2->streamsInSession_.push_back(audioInterrupt);
    audioSession2->state_ == AudioSessionState::SESSION_NEW;
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession2));
    callerPid = 3;
    std::shared_ptr<AudioSession> audioSession3 =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSessionService_.sessionMap_.insert(
        std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, audioSession3));
    callerPid = 4;
    audioSessionService_.sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(callerPid, nullptr));

    EXPECT_TRUE(audioSessionService_.IsStreamAllowedToSetDevice(streamId));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_022.
* @tc.desc  : Test IsSessionNeedToFetchOutputDevice.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_022, TestSize.Level1)
{
    int32_t callerPid = 1;

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSession->needToFetch_ = true;
    audioSessionService_.sessionMap_[callerPid] = audioSession;
    EXPECT_TRUE(audioSessionService_.IsSessionNeedToFetchOutputDevice(callerPid));

    callerPid = 2;
    EXPECT_FALSE(audioSessionService_.IsSessionNeedToFetchOutputDevice(callerPid));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_023.
* @tc.desc  : Test NotifyAppStateChange.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_023, TestSize.Level1)
{
    int32_t pid = 1;
    bool isBackState = false;
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, isBackState));

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(pid, strategy, audioSessionStateMonitor_);
    audioSession->audioSessionScene_ = AudioSessionScene::INVALID;
    audioSessionService_.sessionMap_[pid] = audioSession;
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, false));
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, true));

    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, false));
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, true));
}

/**
* @tc.name  : Test AudioSessionService.
* @tc.number: AudioSessionServiceUnitTest_024.
* @tc.desc  : Test NotifyAppStateChange.
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_024, TestSize.Level1)
{
    int32_t pid = 1;
    AudioSessionStrategy strategy;
    AudioInterrupt audioInterrupt = {};
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(pid, strategy, audioSessionStateMonitor_);
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioSession->streamsInSession_.clear();
    audioSessionService_.sessionMap_.insert(std::pair<int32_t, std::shared_ptr<AudioSession>>(pid, audioSession));
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, true));

    audioSession->streamsInSession_.push_back(audioInterrupt);
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, true));

    audioSession->audioSessionScene_ = AudioSessionScene::INVALID;
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, true));

    audioSession->state_ = AudioSessionState::SESSION_NEW;
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, true));

    audioSession->streamsInSession_.clear();
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, true));

    audioSession->audioSessionScene_ = AudioSessionScene::MEDIA;
    EXPECT_NO_THROW(audioSessionService_.NotifyAppStateChange(pid, true));
}

/*
* @tc.name  : Test RemoveStreamInfo
* @tc.number: RemoveStreamInfoTest
* @tc.desc  : Test RemoveStreamInfo
*/
HWTEST_F(AudioSessionServiceUnitTest, RemoveStreamInfoTest, TestSize.Level1)
{
    int32_t fakePid = 123;
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = 0;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterrupt.streamId = 0;
    audioSessionService_.RemoveStreamInfo(audioInterrupt.pid, audioInterrupt.streamId);

    audioInterrupt.isAudioSessionInterrupt = false;
    audioSessionService_.RemoveStreamInfo(audioInterrupt.pid, audioInterrupt.streamId);

    audioInterrupt.pid = fakePid;
    audioSessionService_.RemoveStreamInfo(audioInterrupt.pid, audioInterrupt.streamId);
}

/**
* @tc.name  : Test ClearStreamInfo
* @tc.number: ClearStreamInfoTest
* @tc.desc  : Test ClearStreamInfo
*/
HWTEST_F(AudioSessionServiceUnitTest, ClearStreamInfoTest, TestSize.Level1)
{
    int32_t fakePid = 123;
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    audioSessionService_.ClearStreamInfo(0);
    audioSessionService_.ClearStreamInfo(fakePid);
}

/**
* @tc.name  : Test SetSessionDefaultOutputDevice
* @tc.number: SetSessionDefaultOutputDeviceTest
* @tc.desc  : Test SetSessionDefaultOutputDevice
*/
HWTEST_F(AudioSessionServiceUnitTest, SetSessionDefaultOutputDeviceTest, TestSize.Level1)
{
    int32_t fakePid = 123;
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_EQ(audioSessionService_.SetSessionDefaultOutputDevice(0, DEVICE_TYPE_INVALID), ERROR_INVALID_PARAM);
    EXPECT_EQ(audioSessionService_.SetSessionDefaultOutputDevice(fakePid, DEVICE_TYPE_INVALID), ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test GetSessionDefaultOutputDevice
* @tc.number: GetSessionDefaultOutputDeviceTest
* @tc.desc  : Test GetSessionDefaultOutputDevice
*/
HWTEST_F(AudioSessionServiceUnitTest, GetSessionDefaultOutputDeviceTest, TestSize.Level1)
{
    int32_t fakePid = 100;
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_EQ(audioSessionService_.GetSessionDefaultOutputDevice(0), DEVICE_TYPE_INVALID);
    EXPECT_EQ(audioSessionService_.GetSessionDefaultOutputDevice(fakePid), DEVICE_TYPE_INVALID);
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
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_TRUE(audioSessionService_.IsStreamAllowedToSetDevice(0));

    AudioInterrupt incomingInterrupt = {};
    incomingInterrupt.streamId = fakeSessionId;
    audioSessionService_.sessionMap_[fakePid]->streamsInSession_.push_back(incomingInterrupt);
    EXPECT_TRUE(audioSessionService_.IsStreamAllowedToSetDevice(fakeSessionId));

    audioSessionService_.sessionMap_[fakePid]->state_ = AudioSessionState::SESSION_ACTIVE;
    EXPECT_TRUE(audioSessionService_.IsStreamAllowedToSetDevice(fakeSessionId));
}

/**
* @tc.name  : Test IsSessionNeedToFetchOutputDevice
* @tc.number: IsSessionNeedToFetchOutputDeviceTest
* @tc.desc  : Test IsSessionNeedToFetchOutputDevice
*/
HWTEST_F(AudioSessionServiceUnitTest, IsSessionNeedToFetchOutputDeviceTest, TestSize.Level1)
{
    int32_t fakePid = 100;
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    EXPECT_FALSE(audioSessionService_.IsSessionNeedToFetchOutputDevice(0));
    EXPECT_FALSE(audioSessionService_.IsSessionNeedToFetchOutputDevice(fakePid));
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
    int ret = audioSessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);

    audioSessionService_.NotifyAppStateChange(0, true);
    audioSessionService_.NotifyAppStateChange(fakePid, false);

    audioSessionService_.sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioSessionService_.NotifyAppStateChange(fakePid, false);

    audioSessionService_.sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::INVALID;
    audioSessionService_.NotifyAppStateChange(fakePid, true);

    audioSessionService_.sessionMap_[fakePid]->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSessionService_.NotifyAppStateChange(fakePid, true);

    audioSessionService_.sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    AudioInterrupt incomingInterrupt = {};
    incomingInterrupt.streamId = fakeSessionId;
    audioSessionService_.sessionMap_[fakePid]->streamsInSession_.push_back(incomingInterrupt);
    audioSessionService_.NotifyAppStateChange(fakeSessionId, false);

    audioSessionService_.sessionMap_[fakePid]->streamsInSession_.clear();
    audioSessionService_.NotifyAppStateChange(fakeSessionId, false);
}

/**
* @tc.name  : Test IsSystemApp
* @tc.number: IsSystemAppTest
* @tc.desc  : Test IsSystemApp
*/
HWTEST_F(AudioSessionServiceUnitTest, IsSystemAppTest, TestSize.Level1)
{
    AudioInterrupt audioInterrupt = {};
    audioInterrupt.pid = 1;
    audioSessionService_.MarkSystemApp(audioInterrupt.pid);
    EXPECT_FALSE(audioSessionService_.IsSystemApp(audioInterrupt.pid));

    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    auto audioSession = std::make_shared<AudioSession>(audioInterrupt.pid, strategy, audioSessionStateMonitor_);
    ASSERT_NE(nullptr, audioSession);
    audioSession->state_ = AudioSessionState::SESSION_ACTIVE;
    audioSessionService_.sessionMap_[audioInterrupt.pid] = audioSession;
    EXPECT_FALSE(audioSessionService_.IsSystemApp(audioInterrupt.pid));
    audioSessionService_.MarkSystemApp(audioInterrupt.pid);
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    EXPECT_TRUE(audioSessionService_.IsSystemApp(audioInterrupt.pid));
    EXPECT_TRUE(audioSessionService_.IsSystemAppWithMixStrategy(audioInterrupt));
}

/**
* @tc.name  : Test FillCurrentOutputDeviceChangedEvent
* @tc.number: AudioSessionServiceUnitTest_027
* @tc.desc  : Test FillCurrentOutputDeviceChangedEvent
*/
HWTEST_F(AudioSessionServiceUnitTest, AudioSessionServiceUnitTest_027, TestSize.Level1)
{
    int32_t callerPid = 1;
    CurrentOutputDeviceChangedEvent deviceChangeEvent;
    AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;

    int32_t ret = audioSessionService_.FillCurrentOutputDeviceChangedEvent(callerPid, reason, deviceChangeEvent);
    EXPECT_EQ(ret, ERROR);

    callerPid = 888;
    audioSessionService_.sessionMap_[callerPid] = nullptr;
    ret = audioSessionService_.FillCurrentOutputDeviceChangedEvent(callerPid, reason, deviceChangeEvent);
    EXPECT_EQ(ret, ERROR);

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSession> audioSession =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    audioSessionService_.sessionMap_[callerPid] = audioSession;

    std::shared_ptr<AudioDeviceDescriptor> ptr = std::make_shared<AudioDeviceDescriptor>();
    deviceChangeEvent.devices.push_back(ptr);
    ret = audioSessionService_.FillCurrentOutputDeviceChangedEvent(callerPid, reason, deviceChangeEvent);
    EXPECT_EQ(ret, ERROR);

    reason = AudioStreamDeviceChangeReason::AUDIO_SESSION_ACTIVATE;
    ret = audioSessionService_.FillCurrentOutputDeviceChangedEvent(callerPid, reason, deviceChangeEvent);
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
