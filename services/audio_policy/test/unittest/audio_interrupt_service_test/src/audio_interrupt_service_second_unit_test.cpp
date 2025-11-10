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

const int32_t DEFAULT_ZONE_ID = 0;
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

class IStandardAudioPolicyManagerListenerStub : public IStandardAudioPolicyManagerListener {
public:
    sptr<IRemoteObject> AsObject() override { return nullptr; }

    ~IStandardAudioPolicyManagerListenerStub() {}

    ErrCode OnInterrupt(const InterruptEventInternal& interruptEvent) override { return SUCCESS; }

    ErrCode OnRouteUpdate(uint32_t routeFlag, const std::string& networkId) override { return SUCCESS; }

    ErrCode OnAvailableDeviceChange(uint32_t usage, const DeviceChangeAction& deviceChangeAction) override
    {
        return SUCCESS;
    }

    ErrCode OnQueryClientType(const std::string& bundleName, uint32_t uid, bool& ret) override
    {
        return SUCCESS;
    }

    ErrCode OnCheckClientInfo(const std::string& bundleName, int32_t& uid, int32_t pid, bool& ret) override
    {
        return SUCCESS;
    }

    ErrCode OnCheckVKBInfo(const std::string& bundleName, bool& isValid) override
    {
        return SUCCESS;
    }

    ErrCode OnQueryAllowedPlayback(int32_t uid, int32_t pid, bool& ret) override
    {
        return SUCCESS;
    }

    ErrCode OnBackgroundMute(int32_t uid) override
    {
        return SUCCESS;
    }

    ErrCode OnQueryBundleNameIsInList(const std::string& bundleName, const std::string& listType, bool& ret) override
    {
        ret = true;
        return SUCCESS;
    }

    ErrCode OnQueryDeviceVolumeBehavior(VolumeBehavior &volumeBehavior) override
    {
        volumeBehavior.isReady = false;
        volumeBehavior.isVolumeControlDisabled = false;
        volumeBehavior.databaseVolumeName = "";
        return SUCCESS;
    }
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

    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    auto ret = audioInterruptService->ActivateAudioSession(AudioInterruptService::ZONEID_DEFAULT,
        fakePid, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);
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
    incomingInterrupt.streamId = 888;
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    auto ret = audioInterruptService->ActivateAudioSession(1, fakePid, audioSessionStrategy);
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
    auto ret = audioInterruptService->sessionService_.IsSessionNeedToFetchOutputDevice(fakePid);
    EXPECT_EQ(false, ret);

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
    audioInterruptService->DeactivateAudioSessionInFakeFocusMode(pid, hintType);
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;

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
    int32_t zoneId = -1;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.pid = fakePid;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    incomingInterrupt.streamId = 888; // 888 is a fake stream id.
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;

    int ret = audioInterruptService->sessionService_.SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    audioInterruptService->sessionService_.sessionMap_[fakePid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioInterruptService->sessionService_.sessionMap_[fakePid]->state_ = AudioSessionState::SESSION_ACTIVE;
    audioInterruptService->HandleAppStreamType(zoneId, incomingInterrupt);
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

    int ret =
        audioInterruptService->sessionService_.SetAudioSessionScene(audioInterrupt.pid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    audioInterruptService->sessionService_.sessionMap_[audioInterrupt.pid]->audioSessionScene_ =
        AudioSessionScene::MEDIA;
    audioInterruptService->sessionService_.sessionMap_[audioInterrupt.pid]->state_ =
        AudioSessionState::SESSION_NEW;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);

    audioInterruptService->sessionService_.sessionMap_[audioInterrupt.pid]->state_ =
        AudioSessionState::SESSION_ACTIVE;
    audioInterruptService->PrintLogsOfFocusStrategyBaseMusic(audioInterrupt);
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

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_018
* @tc.desc  : Test GetStreamInFocusInternal_003
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_018, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 1003;
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.uid = 1003;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_ASSISTANT;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, ACTIVE});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, 0);
    EXPECT_EQ(STREAM_VOICE_ASSISTANT, ret);

    uid = 123;
    audioInterrupt.uid = uid;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, ACTIVE});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    ret = audioInterruptService->GetStreamInFocusInternal(uid, 0);
    EXPECT_EQ(audioInterruptService->defaultVolumeType_, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_019
* @tc.desc  : Test GetStreamInFocusInternal_004
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_019, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 1003;
    int32_t pid = 123;
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.uid = uid;
    audioInterrupt.pid = pid;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_ASSISTANT;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, ACTIVE});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    int ret = audioInterruptService->sessionService_.SetAudioSessionScene(pid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    audioInterruptService->sessionService_.sessionMap_[pid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioInterruptService->sessionService_.sessionMap_[pid]->state_ = AudioSessionState::SESSION_ACTIVE;
    ret = audioInterruptService->GetStreamInFocusInternal(uid, 0);
    EXPECT_EQ(STREAM_MUSIC, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_020
* @tc.desc  : Test NotifyFocusGranted
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_020, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t clientId = 123;
    AudioInterrupt audioInterrupt;
    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    audioInterruptService->NotifyFocusGranted(clientId, audioInterrupt);
    EXPECT_NE(nullptr, audioInterruptService->handler_);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_021
* @tc.desc  : Test NotifyFocusAbandoned
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_021, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t clientId = 123;
    AudioInterrupt audioInterrupt;
    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    auto ret = audioInterruptService->NotifyFocusAbandoned(clientId, audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_022
* @tc.desc  : Test AbandonAudioFocusInternal
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_022, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t clientId = 123;
    AudioInterrupt audioInterrupt;
    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    auto ret = audioInterruptService->AbandonAudioFocusInternal(clientId, audioInterrupt);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_024
* @tc.desc  : Test UpdateHintTypeForExistingSession_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_024, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    audioInterruptService->sessionService_.sessionMap_.clear();
    int32_t pid = 123;
    AudioInterrupt audioInterrupt;
    AudioFocusEntry audioFocusEntry;
    audioInterrupt.pid = pid;
    auto ret = audioInterruptService->sessionService_.SetAudioSessionScene(pid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    audioInterruptService->sessionService_.sessionMap_[pid]->audioSessionScene_ = AudioSessionScene::MEDIA;
    audioInterruptService->sessionService_.sessionMap_[pid]->state_ = AudioSessionState::SESSION_ACTIVE;
    audioFocusEntry.actionOn = CURRENT;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);

    audioFocusEntry.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->sessionService_.sessionMap_[pid]->strategy_.concurrencyMode =
        AudioConcurrencyMode::PAUSE_OTHERS;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_DUCK, audioFocusEntry.hintType);

    audioFocusEntry.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->sessionService_.sessionMap_[pid]->strategy_.concurrencyMode =
        AudioConcurrencyMode::PAUSE_OTHERS;
    audioInterruptService->sessionService_.sessionMap_[pid]->isSystemApp_ = true;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_PAUSE, audioFocusEntry.hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_025
* @tc.desc  : Test UpdateHintTypeForExistingSession_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_025, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    AudioFocusEntry audioFocusEntry;
    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::DUCK_OTHERS;
    audioFocusEntry.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_DUCK, audioFocusEntry.hintType);

    audioFocusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_DUCK, audioFocusEntry.hintType);

    audioFocusEntry.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_DUCK, audioFocusEntry.hintType);

    audioFocusEntry.hintType = INTERRUPT_HINT_RESUME;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_RESUME, audioFocusEntry.hintType);

    audioInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::PAUSE_OTHERS;
    audioFocusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_PAUSE, audioFocusEntry.hintType);

    audioFocusEntry.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_PAUSE, audioFocusEntry.hintType);

    audioFocusEntry.hintType = INTERRUPT_HINT_RESUME;
    audioInterruptService->UpdateHintTypeForExistingSession(audioInterrupt, audioFocusEntry);
    EXPECT_EQ(INTERRUPT_HINT_RESUME, audioFocusEntry.hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_026
* @tc.desc  : Test ProcessExistInterrupt_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_026, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    AudioFocusEntry audioFocusEntry;
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE,
        INTERRUPT_HINT_NONE, 1.0f};
    bool removeFocusInfo = false;
    auto iterActive = focusInfoList.begin();
    AudioInterrupt incomingInterrupt;
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_FALSE(removeFocusInfo);
    
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    incomingInterrupt.currencySources.sourcesTypes.push_back(SOURCE_TYPE_MIC);
    audioFocusEntry.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_FALSE(removeFocusInfo);

    incomingInterrupt.currencySources.sourcesTypes.clear();
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_TRUE(removeFocusInfo);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_027
* @tc.desc  : Test ProcessExistInterrupt_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_027, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    AudioFocusEntry audioFocusEntry;
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE,
        INTERRUPT_HINT_NONE, 1.0f};
    bool removeFocusInfo = false;
    auto iterActive = focusInfoList.begin();
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioFocusEntry.hintType = INTERRUPT_HINT_PAUSE;
    incomingInterrupt.currencySources.sourcesTypes.push_back(SOURCE_TYPE_MIC);
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_FALSE(removeFocusInfo);

    incomingInterrupt.currencySources.sourcesTypes.clear();
    focusInfoList.clear();
    focusInfoList.push_back({audioInterrupt, STOP});
    iterActive = focusInfoList.begin();
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_FALSE(removeFocusInfo);

    focusInfoList.clear();
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    iterActive = focusInfoList.begin();
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_EQ(interruptEvent.hintType, audioFocusEntry.hintType);

    focusInfoList.clear();
    focusInfoList.push_back({audioInterrupt, DUCK});
    iterActive = focusInfoList.begin();
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_EQ(interruptEvent.hintType, audioFocusEntry.hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_028
* @tc.desc  : Test ProcessExistInterrupt_003
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_028, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    AudioFocusEntry audioFocusEntry;
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE,
        INTERRUPT_HINT_NONE, 1.0f};
    bool removeFocusInfo = false;
    auto iterActive = focusInfoList.begin();
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioFocusEntry.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_EQ(DUCK, iterActive->second);

    focusInfoList.clear();
    focusInfoList.push_back({audioInterrupt, DUCK});
    iterActive = focusInfoList.begin();
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_EQ(DUCK, iterActive->second);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_029
* @tc.desc  : Test ProcessExistInterrupt_004
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_029, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    AudioFocusEntry audioFocusEntry;
    InterruptEventInternal interruptEvent {INTERRUPT_TYPE_BEGIN, INTERRUPT_FORCE,
        INTERRUPT_HINT_NONE, 1.0f};
    bool removeFocusInfo = false;
    auto iterActive = focusInfoList.begin();
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioFocusEntry.hintType = INTERRUPT_HINT_MUTE;
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_EQ(MUTED, iterActive->second);

    focusInfoList.clear();
    focusInfoList.push_back({audioInterrupt, MUTED});
    iterActive = focusInfoList.begin();
    audioInterruptService->ProcessExistInterrupt(iterActive, audioFocusEntry,
        incomingInterrupt, removeFocusInfo, interruptEvent);
    EXPECT_EQ(MUTED, iterActive->second);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_030
* @tc.desc  : Test SwitchHintType_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_030, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    auto iterActive = focusInfoList.begin();
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tempfocusInfoList;
    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    auto ret = audioInterruptService->SwitchHintType(iterActive, interruptEvent, tempfocusInfoList);
    EXPECT_EQ(true, ret);

    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tempfocusInfoList);
    EXPECT_EQ(PAUSEDBYREMOTE, iterActive->second);

    focusInfoList.clear();
    focusInfoList.push_back({audioInterrupt, DUCK});
    iterActive = focusInfoList.begin();
    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tempfocusInfoList);
    EXPECT_EQ(PAUSEDBYREMOTE, iterActive->second);

    focusInfoList.clear();
    focusInfoList.push_back({audioInterrupt, STOP});
    iterActive = focusInfoList.begin();
    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tempfocusInfoList);
    EXPECT_EQ(STOP, iterActive->second);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_031
* @tc.desc  : Test SwitchHintType_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_031, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    auto iterActive = focusInfoList.begin();
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tempfocusInfoList;
    interruptEvent.hintType = INTERRUPT_HINT_RESUME;
    auto ret = audioInterruptService->SwitchHintType(iterActive, interruptEvent, tempfocusInfoList);
    EXPECT_EQ(false, ret);

    focusInfoList.clear();
    focusInfoList.push_back({audioInterrupt, PAUSEDBYREMOTE});
    iterActive = focusInfoList.begin();
    ret = audioInterruptService->SwitchHintType(iterActive, interruptEvent, tempfocusInfoList);
    EXPECT_EQ(true, ret);

    interruptEvent.hintType = INTERRUPT_HINT_DUCK;
    ret = audioInterruptService->SwitchHintType(iterActive, interruptEvent, tempfocusInfoList);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_032
* @tc.desc  : Test ProcessRemoteInterrupt_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_032, TestSize.Level1)
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
    std::set<int32_t> streamIds {123};
    InterruptEventInternal interruptEvent;
    audioInterruptService->ProcessRemoteInterrupt(streamIds, interruptEvent);
    EXPECT_EQ(0, audioInterruptZone->zoneId);
}


/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_033
* @tc.desc  : Test ProcessRemoteInterrupt_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_033, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    auto pid = getpid();
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterrupt.streamId = 123;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    std::set<int32_t> streamIds {123};
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->ProcessRemoteInterrupt(streamIds, interruptEvent);
    EXPECT_EQ(0, audioInterruptZone->zoneId);

    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->ProcessRemoteInterrupt(streamIds, interruptEvent);
    EXPECT_EQ(0, audioInterruptZone->zoneId);

    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->ProcessRemoteInterrupt(streamIds, interruptEvent);
    EXPECT_EQ(0, audioInterruptZone->zoneId);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_034
* @tc.desc  : Test RemoveFocusInfo_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_034, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 123;
    audioInterrupt.streamId = 1234;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    auto iterActive = focusInfoList.begin();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    tmpFocusInfoList.push_back(*iterActive);
    auto zoneInfo = make_shared<AudioInterruptZone>();
    std::list<int32_t> removeFocusInfoPidList;
    audioInterruptService->RemoveFocusInfo(iterActive, tmpFocusInfoList, zoneInfo, removeFocusInfoPidList);
    EXPECT_TRUE(tmpFocusInfoList.empty());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_035
* @tc.desc  : Test RemoveFocusInfo_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_035, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 12;
    audioInterrupt.streamId = 1234;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
    focusInfoList.push_back({audioInterrupt, ACTIVE});
    auto iterActive = focusInfoList.begin();
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    tmpFocusInfoList.push_back(*iterActive);
    auto zoneInfo = make_shared<AudioInterruptZone>();
    std::list<int32_t> removeFocusInfoPidList;
    int32_t pid = 12;
    int ret = audioInterruptService->SetAudioSessionScene(pid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    audioInterruptService->RemoveFocusInfo(iterActive, tmpFocusInfoList, zoneInfo, removeFocusInfoPidList);
    EXPECT_TRUE(tmpFocusInfoList.empty());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_036
* @tc.desc  : Test HandleLowPriorityEvent
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_036, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t pid = 123;
    int32_t streamId = 123;
    auto ret = audioInterruptService->HandleLowPriorityEvent(pid, streamId);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_037
* @tc.desc  : Test SendActiveInterruptEvent
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_037, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    uint32_t streamId = 123;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    audioInterruptService->SendActiveInterruptEvent(streamId, interruptEvent, incomingInterrupt, activeInterrupt);

    interruptEvent.hintType = INTERRUPT_HINT_NONE;
    audioInterruptService->SendActiveInterruptEvent(streamId, interruptEvent, incomingInterrupt, activeInterrupt);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_038
* @tc.desc  : Test IsAudioSourceConcurrency、IsMediaStream
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_038, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    SourceType existSourceType = SOURCE_TYPE_WAKEUP;
    SourceType incomingSourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    std::vector<SourceType> existConcurrentSources, incomingConcurrentSources;
    incomingConcurrentSources.push_back(existSourceType);
    existConcurrentSources.push_back(incomingSourceType);
    auto ret = audioInterruptService->IsAudioSourceConcurrency(existSourceType,
        incomingSourceType, existConcurrentSources, incomingConcurrentSources);
    EXPECT_TRUE(ret);

    AudioStreamType audioStreamType = STREAM_ALARM;
    ret = audioInterruptService->IsMediaStream(audioStreamType);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_039
* @tc.desc  : Test UpdateFocusStrategy
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_039, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    string bundleName = "test";
    AudioFocusEntry focusEntry;
    bool isExistMediaStream = true;
    bool isIncomingMediaStream = true;
    sptr<IStandardAudioPolicyManagerListener> listener(new IStandardAudioPolicyManagerListenerStub());
    audioInterruptService->queryBundleNameListCallback_ = listener;
    audioInterruptService->UpdateFocusStrategy(bundleName, focusEntry, isExistMediaStream, isIncomingMediaStream);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_040
* @tc.desc  : Test UpdateMicFocusStrategy
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_040, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    SourceType existSourceType, incomingSourceType;
    incomingSourceType = SOURCE_TYPE_INVALID;
    existSourceType = SOURCE_TYPE_INVALID;
    AudioStreamType existStreamType = STREAM_ALARM;
    AudioStreamType incomingStreamType = STREAM_ALARM;
    string bundleName = "test";
    std::string currentBundleName = "currentTest";
    AudioFocusEntry focusEntry;
    audioInterruptService->UpdateMicFocusStrategy(existSourceType, incomingSourceType, existStreamType,
        incomingStreamType, currentBundleName, bundleName, focusEntry);
    incomingSourceType = SOURCE_TYPE_VOICE_CALL;
    audioInterruptService->UpdateMicFocusStrategy(existSourceType, incomingSourceType, existStreamType,
        incomingStreamType, currentBundleName, bundleName, focusEntry);
    existSourceType = SOURCE_TYPE_MIC;
    audioInterruptService->UpdateMicFocusStrategy(existSourceType, incomingSourceType, existStreamType,
        incomingStreamType, currentBundleName, bundleName, focusEntry);
    sptr<IStandardAudioPolicyManagerListener> listener(new IStandardAudioPolicyManagerListenerStub());
    audioInterruptService->queryBundleNameListCallback_ = listener;
    audioInterruptService->UpdateMicFocusStrategy(existSourceType, incomingSourceType, existStreamType,
        incomingStreamType, currentBundleName, bundleName, focusEntry);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_041
* @tc.desc  : Test FocusEntryContinue
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_041, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION;
    audioInterrupt.uid = AUDIO_ID;
    audioInterrupt.currencySources.sourcesTypes.push_back(SOURCE_TYPE_INVALID);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> list;
    list.push_back({audioInterrupt, ACTIVE});
    auto iterActive = list.begin();
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = INCOMING;
    focusEntry.isReject = false;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.streamUsage = STREAM_USAGE_NOTIFICATION;
    auto ret = audioInterruptService->FocusEntryContinue(iterActive, focusEntry, incomingInterrupt);
    EXPECT_EQ(false, ret);
    incomingInterrupt.streamUsage = STREAM_USAGE_RINGTONE;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    ret = audioInterruptService->FocusEntryContinue(iterActive, focusEntry, incomingInterrupt);
    EXPECT_EQ(true, ret);
    focusEntry.hintType = INTERRUPT_HINT_DUCK;
    ret = audioInterruptService->FocusEntryContinue(iterActive, focusEntry, incomingInterrupt);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_042
* @tc.desc  : Test IsLowestPriorityRecording
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_042, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.currencySources.sourcesTypes.push_back(SOURCE_TYPE_MIC);
    auto ret = audioInterruptService->IsLowestPriorityRecording(audioInterrupt);
    EXPECT_FALSE(ret);
    audioInterrupt.currencySources.sourcesTypes[0] = SOURCE_TYPE_INVALID;
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_043
* @tc.desc  : Test HadVoipStatus
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_043, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 123;
    audioInterrupt.streamId = 123;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_COMMUNICATION;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    audioFocusInfoList.push_back({audioInterrupt, PLACEHOLDER});
    auto ret = audioInterruptService->HadVoipStatus(audioInterrupt, audioFocusInfoList);
    EXPECT_FALSE(ret);
    audioInterrupt.streamId = 111;
    ret = audioInterruptService->HadVoipStatus(audioInterrupt, audioFocusInfoList);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_044
* @tc.desc  : Test EvaluateWhetherContinue
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_044, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt incoming, inprocessing;
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_MUTE;
    auto ret = audioInterruptService->EvaluateWhetherContinue(incoming, inprocessing,
        focusEntry, false);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_045
* @tc.desc  : Test GetAudioSessionUidList
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_GetAudioSessionUidList, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    AudioInterrupt audioInterrupt;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterrupt.uid = 1000;
    AudioFocuState audioFocuState = AudioFocuState::DUCK;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetAudioSessionUidList(zoneId);

    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret.at(0), 1000);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_046
* @tc.desc  : Test CanMixForSession
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_046, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt, activeInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = false;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    auto ret = audioInterruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(false, ret);

    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    ret = audioInterruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(false, ret);

    focusEntry.isReject = true;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    ret = audioInterruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(false, ret);

    focusEntry.isReject = false;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    ret = audioInterruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(false, ret);

    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    incomingInterrupt.audioFocusType.streamType = STREAM_INTERNAL_FORCE_STOP;
    ret = audioInterruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(false, ret);
    
    incomingInterrupt.audioFocusType.streamType = STREAM_GAME;
    activeInterrupt.audioFocusType.streamType == STREAM_INTERNAL_FORCE_STOP;
    ret = audioInterruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(false, ret);

    incomingInterrupt.audioFocusType.streamType = STREAM_INTERNAL_FORCE_STOP;
    ret = audioInterruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_047
* @tc.desc  : Test CanMixForIncomingSession
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_047, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt incomingInterrupt, activeInterrupt;
    AudioFocusEntry focusEntry;
    incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    auto ret = audioInterruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(true, ret);
    incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    ret = audioInterruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_048
* @tc.desc  : Test CanMixForActiveSession
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_048, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t pid = 123;
    AudioInterrupt incomingInterrupt, activeInterrupt;
    AudioFocusEntry focusEntry;
    activeInterrupt.pid = pid;
    audioInterruptService->SetAudioSessionScene(pid, AudioSessionScene::MEDIA);
    focusEntry.actionOn = CURRENT;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    auto ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(false, ret);

    activeInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(true, ret);
    activeInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    ret = audioInterruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_049
* @tc.desc  : Test SetAudioManagerInterruptCallback_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_049, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
   
    auto object = new RemoteObjectTestStub();
    auto ret = audioInterruptService->SetAudioManagerInterruptCallback(object);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_050
* @tc.desc  : Test SetAudioManagerInterruptCallback_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_050, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
   
    auto object = new RemoteObjectTestStub();
    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    auto ret = audioInterruptService->SetAudioManagerInterruptCallback(object);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_051
* @tc.desc  : Test SetAudioInterruptCallback
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_051, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
   
    int32_t zoneId = 123;
    int32_t streamId = 123;
    int32_t uid = 123;
    auto object = new RemoteObjectTestStub();
    auto ret = audioInterruptService->SetAudioInterruptCallback(zoneId, streamId, object, uid);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_052
* @tc.desc  : Test UnsetAudioInterruptCallback
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_052, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
   
    int32_t zoneId = 123;
    int32_t streamId = 123;
    auto ret = audioInterruptService->UnsetAudioInterruptCallback(zoneId, streamId);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_053
* @tc.desc  : Test GetSessionInfoInFocus
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_053, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    int32_t zoneId = 123;
    auto ret = audioInterruptService->GetSessionInfoInFocus(audioInterrupt, zoneId);
    EXPECT_EQ(SUCCESS, ret);

    audioInterruptService->zonesMap_[zoneId] = nullptr;
    ret = audioInterruptService->GetSessionInfoInFocus(audioInterrupt, zoneId);
    EXPECT_EQ(SUCCESS, ret);

    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, ACTIVE});
    AudioInterrupt audioInterrupt2;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt2, STOP});
    audioInterruptService->zonesMap_[zoneId] = audioInterruptZone;
    ret = audioInterruptService->GetSessionInfoInFocus(audioInterrupt, zoneId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_054
* @tc.desc  : Test GetAudioSessionUidList_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_054, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    AudioInterrupt audioInterrupt;
    audioInterrupt.isAudioSessionInterrupt = false;
    audioInterrupt.uid = 1000;
    AudioFocuState audioFocuState = AudioFocuState::DUCK;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetAudioSessionUidList(zoneId);
    EXPECT_EQ(ret.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_055
* @tc.desc  : Test HandleLowPriorityEvent
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_055, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t pid = 123;
    uint32_t streamId = 123;
    auto ret = audioInterruptService->sessionService_.SetAudioSessionScene(pid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    ret = audioInterruptService->HandleLowPriorityEvent(pid, streamId);
    EXPECT_TRUE(ret);
    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    ret = audioInterruptService->HandleLowPriorityEvent(pid, streamId);
    EXPECT_TRUE(ret);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.streamId = streamId;
    audioInterrupt.streamUsage = STREAM_USAGE_MUSIC;
    audioInterrupt.audioFocusType.isPlay = true;
    audioInterruptService->sessionService_.SetAudioSessionScene(pid, AudioSessionScene::MEDIA);
    EXPECT_NE(nullptr, audioInterruptService->sessionService_.sessionMap_[pid]);
    audioInterruptService->sessionService_.sessionMap_[pid]->AddStreamInfo(audioInterrupt);
    ret = audioInterruptService->HandleLowPriorityEvent(pid, streamId + 1);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_056
* @tc.desc  : Test ProcessAudioScene
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_056, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    AudioInterrupt audioInterrupt;
    uint32_t incomingStreamId = 123;
    int32_t zoneId = 123;
    bool shouldReturnSuccess = false;
    audioInterruptService->ProcessAudioScene(audioInterrupt, incomingStreamId, zoneId, shouldReturnSuccess);
    audioInterruptService->zonesMap_[zoneId] = nullptr;
    audioInterruptService->ProcessAudioScene(audioInterrupt, incomingStreamId, zoneId, shouldReturnSuccess);
    EXPECT_EQ(true, shouldReturnSuccess);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_057
* @tc.desc  : Test SetQueryBundleNameListCallback
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_057, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = audioInterruptService->SetQueryBundleNameListCallback(object);
    EXPECT_EQ(SUCCESS, ret);
    object = nullptr;
    ret = audioInterruptService->SetQueryBundleNameListCallback(object);
    EXPECT_EQ(ERR_CALLBACK_NOT_REGISTERED, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_058
* @tc.desc  : Test ShouldBypassAudioSessionFocus_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_058, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t zoneId = 0;
    auto pid = getpid();
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = true;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[zoneId] = audioInterruptZone;
    auto ret = audioInterruptService->ShouldBypassAudioSessionFocus(zoneId, audioInterrupt);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_059
* @tc.desc  : Test ShouldBypassAudioSessionFocus_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_059, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t zoneId = 0;
    auto pid = getpid();
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    audioInterrupt.isAudioSessionInterrupt = false;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[zoneId] = audioInterruptZone;
    auto ret = audioInterruptService->ShouldBypassAudioSessionFocus(zoneId, audioInterrupt);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_060
* @tc.desc  : Test AddToAudioFocusInfoList
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_060, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    auto audioInterruptZone = std::make_shared<AudioInterruptZone>();
    int32_t zoneId = 123;
    AudioInterrupt incomingInterrupt;
    AudioFocuState incomingState = STOP;
    audioInterruptService->AddToAudioFocusInfoList(audioInterruptZone, zoneId, incomingInterrupt, incomingState);
    EXPECT_EQ(false, incomingInterrupt.isAudioSessionInterrupt);
    incomingState = ACTIVE;
    audioInterruptService->AddToAudioFocusInfoList(audioInterruptZone, zoneId, incomingInterrupt, incomingState);
    EXPECT_EQ(false, incomingInterrupt.isAudioSessionInterrupt);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_061
* @tc.desc  : Test HandleIncomingState
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_061, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t zoneId = 123;
    AudioFocuState incomingState;
    InterruptEventInternal interruptEvent;
    AudioInterrupt incomingInterrupt;
    incomingState = STOP;
    audioInterruptService->HandleIncomingState(zoneId, incomingState, interruptEvent, incomingInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_STOP, interruptEvent.hintType);
    incomingState = PAUSE;
    audioInterruptService->HandleIncomingState(zoneId, incomingState, interruptEvent, incomingInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_PAUSE, interruptEvent.hintType);
    incomingState = MUTED;
    audioInterruptService->HandleIncomingState(zoneId, incomingState, interruptEvent, incomingInterrupt);
    EXPECT_EQ(INTERRUPT_HINT_MUTE, interruptEvent.hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_062
* @tc.desc  : Test SendFocusChangeEvent
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_062, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t zoneId = 0;
    int32_t callbackCategory = static_cast<int32_t>(AudioPolicyServerHandler::REQUEST_CALLBACK_CATEGORY);
    AudioInterrupt audioInterrupt;
    audioInterruptService->handler_ = make_shared<AudioPolicyServerHandler>();
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterruptService->SendFocusChangeEvent(zoneId, callbackCategory, audioInterrupt);
    EXPECT_NE(nullptr, audioInterruptService->handler_);

    callbackCategory = static_cast<int32_t>(AudioPolicyServerHandler::ABANDON_CALLBACK_CATEGORY);
    audioInterruptService->SendFocusChangeEvent(zoneId, callbackCategory, audioInterrupt);
    EXPECT_NE(nullptr, audioInterruptService->handler_);

    callbackCategory = static_cast<int32_t>(AudioPolicyServerHandler::NONE_CALLBACK_CATEGORY);
    audioInterruptService->SendFocusChangeEvent(zoneId, callbackCategory, audioInterrupt);
    EXPECT_NE(nullptr, audioInterruptService->handler_);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_063
* @tc.desc  : Test DispatchInterruptEventWithStreamId
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_063, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    uint32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    audioInterruptService->DispatchInterruptEventWithStreamId(streamId, interruptEvent);
    streamId = 100001;
    audioInterruptService->DispatchInterruptEventWithStreamId(streamId, interruptEvent);
    EXPECT_EQ(true, interruptEvent.callbackToApp);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_064
* @tc.desc  : Test ShouldCallbackToClient
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_064, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    uint32_t streamId = 0;
    uint32_t uid = 123;
    InterruptEventInternal interruptEvent;
    auto ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(true, ret);

    ClientTypeManager::GetInstance()->clientTypeMap_[uid] = CLIENT_TYPE_GAME;
    interruptEvent.hintType = INTERRUPT_HINT_DUCK;
    ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(true, ret);

    interruptEvent.hintType = INTERRUPT_HINT_UNDUCK;
    ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(true, ret);

    interruptEvent.hintType = INTERRUPT_HINT_RESUME;
    sptr<AudioPolicyServer> server = new AudioPolicyServer(0);
    audioInterruptService->Init(server);
    ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(false, ret);

    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(false, ret);

    interruptEvent.hintType = INTERRUPT_HINT_RESUME;
    ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(false, ret);

    interruptEvent.hintType = INTERRUPT_HINT_MUTE;
    ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_065
* @tc.desc  : Test RemoveClient_001
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_065, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t zoneId = 123;
    uint32_t streamId = 123;
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = streamId;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    audioInterruptService->RemoveClient(zoneId, streamId);
    EXPECT_EQ(false, audioInterrupt.isAudioSessionInterrupt);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_066
* @tc.desc  : Test RemoveClient_002
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_066, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    int32_t zoneId = 123;
    uint32_t streamId = 123;
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 0;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->zonesMap_[zoneId] = nullptr;
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    audioInterruptService->RemoveClient(zoneId, streamId);
    EXPECT_EQ(false, audioInterrupt.isAudioSessionInterrupt);

    audioInterruptService->zonesMap_[zoneId] = audioInterruptZone;
    audioInterruptService->RemoveClient(zoneId, streamId);
    EXPECT_EQ(false, audioInterrupt.isAudioSessionInterrupt);

    audioInterruptZone->interruptCbsMap[streamId] = std::make_shared<AudioInterruptCallbackTest>();
    audioInterruptService->zonesMap_[zoneId] = audioInterruptZone;
    audioInterruptService->RemoveClient(zoneId, streamId);
    EXPECT_EQ(false, audioInterrupt.isAudioSessionInterrupt);
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: AudioInterruptService_067
 * @tc.desc  : Test AudioInterruptIsActiveInFocusList
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_067, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);
    uint32_t streamId = 1;
    int32_t zoneId = 0;
    auto ret = audioInterruptService->AudioInterruptIsActiveInFocusList(zoneId, streamId);
    EXPECT_EQ(ret, false);
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = streamId;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, ACTIVE});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    ret = audioInterruptService->AudioInterruptIsActiveInFocusList(zoneId, streamId);
    EXPECT_EQ(ret, true);
    audioInterruptService->mutedGameSessionId_.insert(streamId);
    audioInterruptZone->audioFocusInfoList.clear();
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, PAUSE});
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    ret = audioInterruptService->AudioInterruptIsActiveInFocusList(zoneId, streamId);
    EXPECT_EQ(ret, true);
    audioInterruptZone->audioFocusInfoList.clear();
    audioInterruptService->mutedGameSessionId_.clear();
    audioInterruptService->zonesMap_[0] = audioInterruptZone;
    ret = audioInterruptService->AudioInterruptIsActiveInFocusList(zoneId, streamId);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test ReactivateAudioInterrupts
* @tc.number: AudioInterruptService_068
* @tc.desc  : Test ReactivateAudioInterrupts
*/
HWTEST(AudioInterruptServiceSecondUnitTest, AudioInterruptService_068, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(nullptr, audioInterruptService);
    int32_t zoneId = 0;
    uint32_t streamId = 123;
    int32_t pid = 0;
    bool updateScene = false;
    audioInterruptService->ReactivateAudioInterrupts(zoneId, pid, updateScene);
    EXPECT_FALSE(updateScene);
    audioInterruptService->zonesMap_[zoneId] = nullptr;
    audioInterruptService->ReactivateAudioInterrupts(zoneId, pid, updateScene);
    EXPECT_FALSE(updateScene);
    auto audioInterruptZone = make_shared<AudioInterruptZone>();
    audioInterruptService->zonesMap_[zoneId] = audioInterruptZone;
    audioInterruptService->ReactivateAudioInterrupts(zoneId, pid, updateScene);
    EXPECT_FALSE(updateScene);
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 0;
    audioInterrupt.pid = pid;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, STOP});
    audioInterruptService->ReactivateAudioInterrupts(zoneId, pid, updateScene);
    EXPECT_TRUE(updateScene);
}

/**
 * @tc.name  : Test ActivateAudioInterruptCoreProcedure
 * @tc.number: ActivateAudioInterruptCoreProcedure01
 * @tc.desc  : Test ActivateAudioInterruptCoreProcedure
 */
HWTEST_F(AudioInterruptServiceSecondUnitTest, ActivateAudioInterruptCoreProcedure01, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();

    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;
    audioInterrupt.audioFocusType.isPlay = true;
    audioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;

    bool updateScene = false;
    int32_t ret = audioInterruptService->ActivateAudioInterruptCoreProcedure(DEFAULT_ZONE_ID,
        audioInterrupt, false, updateScene);
    EXPECT_EQ(ret, ERR_FOCUS_DENIED);
}

/**
 * @tc.name  : Test ActivateAudioInterruptInternal
 * @tc.number: ActivateAudioInterruptInternal01
 * @tc.desc  : Test ActivateAudioInterruptInternal
 */
HWTEST(AudioInterruptServiceSecondUnitTest, ActivateAudioInterruptInternal01, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();

    uint32_t streamId = 0;
    uint32_t uid = 123;
    int32_t pid = 0;

    AudioInterrupt audioInterrupt;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_TRANSCRIPTION;
    audioInterrupt.callbackType = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    audioInterrupt.pid = pid;
    audioInterrupt.streamId = streamId;
    audioInterrupt.streamUsage = STREAM_USAGE_GAME;
    audioInterrupt.audioFocusType.isPlay = true;

    bool updateScene = false;

    InterruptEventInternal interruptEvent;
    auto ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(true, ret);
    ClientTypeManager::GetInstance()->clientTypeMap_[uid] = CLIENT_TYPE_GAME;

    audioInterruptService->GameRecogSetParam(CLIENT_TYPE_GAME, SOURCE_TYPE_VOICE_RECOGNITION, true);
    ret = audioInterruptService->ActivateAudioInterruptInternal(DEFAULT_ZONE_ID,
        audioInterrupt, false, updateScene);
    EXPECT_EQ(ret, ERR_FOCUS_DENIED);
    audioInterruptService->GameRecogSetParam(CLIENT_TYPE_GAME, SOURCE_TYPE_VOICE_RECOGNITION, false);
    ret = audioInterruptService->DeactivateAudioInterrupt(DEFAULT_ZONE_ID, audioInterrupt);
    EXPECT_EQ(ret, ERR_FOCUS_DENIED);
}

} // namespace AudioStandard
} // namespace OHOS
