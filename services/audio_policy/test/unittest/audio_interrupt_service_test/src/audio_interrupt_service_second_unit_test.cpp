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

#include "audio_interrupt_service_second_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInterruptServiceSecondUnitTest::SetUpTestCase(void) {}
void AudioInterruptServiceSecondUnitTest::TearDownTestCase(void) {}
void AudioInterruptServiceSecondUnitTest::SetUp(void) {}
void AudioInterruptServiceSecondUnitTest::TearDown(void) {}

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

class AudioInterruptCallbackTest : public AudioInterruptCallback {
public:
    void OnInterrupt(const InterruptEventInternal &interruptEvent) override {};
};

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_001
* @tc.desc  : Test OnSessionTimeout and HandleSessionTimeOutEvent
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_001, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    auto pid = getpid();
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    audioInterruptService->sessionService_ = sessionService;
    audioInterruptService->OnSessionTimeout(pid);
    EXPECT_EQ(nullptr, audioInterruptService->handler_);

    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    audioInterruptService->OnSessionTimeout(pid);
    EXPECT_NE(nullptr, audioInterruptService->handler_);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_002
* @tc.desc  : Test ActivateAudioSession_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_002, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t fakePid = 123;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.pid = fakePid;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    incomingInterrupt.streamId = 888; // 888 is a fake stream id.

    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    sessionService->sessionMap_[fakePid] = nullptr;
    audioInterruptService->sessionService_ = sessionService;
    
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    auto ret = audioInterruptService->ActivateAudioSession(AudioInterruptService::ZONEID_DEFAULT,
        fakePid, audioSessionStrategy);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_003
* @tc.desc  : Test ActivateAudioSession_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_003, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t fakePid = 123;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.pid = fakePid;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    incomingInterrupt.streamId = 888; // 888 is a fake stream id.
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;

    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    sessionService->sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    sessionService->sessionMap_[fakePid]->state_ = AudioSessionState::SESSION_ACTIVE;
    audioInterruptService->sessionService_ = sessionService;
    
    ret = audioInterruptService->ActivateAudioSession(1, fakePid, audioSessionStrategy);
    EXPECT_EQ(ERROR, ret);
    
    ret = audioInterruptService->ActivateAudioSession(0, fakePid, audioSessionStrategy);
    EXPECT_EQ(ERROR, ret);

    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = fakePid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    ret = audioInterruptService->ActivateAudioSession(0, fakePid, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_004
* @tc.desc  : Test IsSessionNeedToFetchOutputDevice、SetAudioSessionScene
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_004, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t fakePid = 123;
    auto ret = audioInterruptService->IsSessionNeedToFetchOutputDevice(fakePid);
    EXPECT_EQ(false, ret);
    ret = audioInterruptService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(true, ret);

    sptr<AudioPolicyServer> server(new AudioPolicyServer(0));
    audioInterruptService->Init(server);
    ret = audioInterruptService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_005
* @tc.desc  : Test DeactivateAudioSession
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_005, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    auto pid = getpid();
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    audioInterruptService->sessionService_ = sessionService;
    EXPECT_EQ(nullptr, audioInterruptService->handler_);
    auto ret = audioInterruptService->DeactivateAudioSession(0, pid);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    ret = audioInterruptService->DeactivateAudioSession(0, pid);
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_006
* @tc.desc  : Test DeactivateAudioSessionInFakeFocusMode
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_006, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    auto pid = getpid();
    InterruptHint hintType = INTERRUPT_HINT_PAUSE;
    EXPECT_EQ(nullptr, audioInterruptService->sessionService_);
    audioInterruptService->DeactivateAudioSessionInFakeFocusMode(pid, hintType);
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    audioInterruptService->sessionService_ = sessionService;

    audioInterruptService->handler_ = nullptr;
    audioInterruptService->DeactivateAudioSessionInFakeFocusMode(pid, hintType);

    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    audioInterruptService->DeactivateAudioSessionInFakeFocusMode(pid, hintType);

    hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->DeactivateAudioSessionInFakeFocusMode(pid, hintType);
    EXPECT_NE(nullptr, audioInterruptService->handler_);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_008
* @tc.desc  : Test UnsetAudioInterruptCallback
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_008, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    pid_t pid = 123;
    uint32_t streamId = 123;
    audioInterruptService->interruptClients_[streamId] = nullptr;
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptZone->interruptCbsMap[streamId] = nullptr;
    audioInterruptService->zonesMap_[0] = audioInterruptZone;

    auto ret = audioInterruptService->UnsetAudioInterruptCallback(0, streamId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_009
* @tc.desc  : Test HandleAppStreamType
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_009, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t fakePid = 123;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.pid = fakePid;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    incomingInterrupt.streamId = 888; // 888 is a fake stream id.
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;

    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    sessionService->sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    sessionService->sessionMap_[fakePid]->state_ = AudioSessionState::SESSION_ACTIVE;
    audioInterruptService->sessionService_ = sessionService;
    audioInterruptService->HandleAppStreamType(incomingInterrupt);

    audioInterruptService->sessionService_ = nullptr;
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = fakePid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    audioInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    audioInterruptService->HandleAppStreamType(incomingInterrupt);
    EXPECT_EQ(nullptr, audioInterruptService->sessionService_);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_010
* @tc.desc  : Test PrintLogsOfFocusStrategyBaseMusic_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_010, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    AudioFocusType audioFocusType;
    audioFocusType.streamType = AudioStreamType::STREAM_MUSIC;
    std::pair<AudioFocusType, AudioFocusType> focusPair =
        std::make_pair(audioFocusType, audioInterrupt.audioFocusType);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = INCOMING;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(nullptr, audioInterruptService->sessionService_);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_011
* @tc.desc  : Test PrintLogsOfFocusStrategyBaseMusic_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_011, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 123;
    AudioFocusType audioFocusType;
    audioFocusType.streamType = AudioStreamType::STREAM_MUSIC;
    std::pair<AudioFocusType, AudioFocusType> focusPair =
        std::make_pair(audioFocusType, audioInterrupt.audioFocusType);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;

    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    int ret = sessionService->SetAudioSessionScene(audioInterrupt.pid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    sessionService->sessionMap_[audioInterrupt.pid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    sessionService->sessionMap_[audioInterrupt.pid]->state_ = AudioSessionState::SESSION_NEW;
    audioInterruptService->sessionService_ = sessionService;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_NE(nullptr, audioInterruptService->sessionService_);

    sessionService->sessionMap_[audioInterrupt.pid]->state_ = AudioSessionState::SESSION_ACTIVE;
    audioInterruptService->sessionService_ = sessionService;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_NE(nullptr, audioInterruptService->sessionService_);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_012
* @tc.desc  : Test PrintLogsOfFocusStrategyBaseMusic_003
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_012, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    AudioFocusType audioFocusType;
    audioFocusType.streamType = AudioStreamType::STREAM_MUSIC;
    std::pair<AudioFocusType, AudioFocusType> focusPair =
        std::make_pair(audioFocusType, audioInterrupt.audioFocusType);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(nullptr, audioInterruptService->sessionService_);

    focusEntry.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_DUCK, audioInterruptService->focusCfgMap_[focusPair].hintType);

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_PAUSE, audioInterruptService->focusCfgMap_[focusPair].hintType);

    focusEntry.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_STOP, audioInterruptService->focusCfgMap_[focusPair].hintType);

    focusEntry.hintType = INTERRUPT_HINT_MUTE;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_MUTE, audioInterruptService->focusCfgMap_[focusPair].hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_013
* @tc.desc  : Test PrintLogsOfFocusStrategyBaseMusic_004
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_013, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    AudioFocusType audioFocusType;
    audioFocusType.streamType = AudioStreamType::STREAM_MUSIC;
    std::pair<AudioFocusType, AudioFocusType> focusPair =
        std::make_pair(audioFocusType, audioInterrupt.audioFocusType);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::DUCK_OTHERS;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(nullptr, audioInterruptService->sessionService_);

    focusEntry.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_DUCK, audioInterruptService->focusCfgMap_[focusPair].hintType);

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_PAUSE, audioInterruptService->focusCfgMap_[focusPair].hintType);

    focusEntry.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_STOP, audioInterruptService->focusCfgMap_[focusPair].hintType);

    focusEntry.hintType = INTERRUPT_HINT_MUTE;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_MUTE, audioInterruptService->focusCfgMap_[focusPair].hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_014
* @tc.desc  : Test PrintLogsOfFocusStrategyBaseMusic_005
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_014, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    AudioFocusType audioFocusType;
    audioFocusType.streamType = AudioStreamType::STREAM_MUSIC;
    std::pair<AudioFocusType, AudioFocusType> focusPair =
        std::make_pair(audioFocusType, audioInterrupt.audioFocusType);
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::PAUSE_OTHERS;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(nullptr, audioInterruptService->sessionService_);

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_PAUSE, audioInterruptService->focusCfgMap_[focusPair].hintType);

    focusEntry.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_STOP, audioInterruptService->focusCfgMap_[focusPair].hintType);

    focusEntry.hintType = INTERRUPT_HINT_MUTE;
    audioInterruptService->focusCfgMap_[focusPair] = focusEntry;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_MUTE, audioInterruptService->focusCfgMap_[focusPair].hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_015
* @tc.desc  : Test ActivatePreemptMode
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_015, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 123;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterrupt.streamUsage = STREAM_USAGE_GAME;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    audioInterruptService->handler_ = nullptr;
    auto ret = audioInterruptService->ActivatePreemptMode();
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_016
* @tc.desc  : Test GetStreamInFocusInternal_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_016, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 123;
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, 0);
    EXPECT_EQ(audioInterruptService->defaultVolumeType_, ret);

    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, ACTIVE});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    ret = audioInterruptService->GetStreamInFocusInternal(uid, 0);
    EXPECT_EQ(audioInterruptService->defaultVolumeType_, ret);

    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    ret = audioInterruptService->GetStreamInFocusInternal(uid, 0);
    EXPECT_EQ(audioInterruptService->defaultVolumeType_, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_017
* @tc.desc  : Test GetStreamInFocusInternal_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_017, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 111;
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.uid = 111;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, ACTIVE});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, 0);
    EXPECT_EQ(audioInterruptService->defaultVolumeType_, ret);
    uid = 123;
    ret = audioInterruptService->GetStreamInFocusInternal(uid, 0);
    EXPECT_EQ(audioInterruptService->defaultVolumeType_, ret);
}
} // namespace AudioStandard
} // namespace OHOS
