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

#include "audio_interrupt_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioInterruptServiceUnitTest::SetUpTestCase(void) {}
void AudioInterruptServiceUnitTest::TearDownTestCase(void) {}
void AudioInterruptServiceUnitTest::SetUp(void) {}
void AudioInterruptServiceUnitTest::TearDown(void) {}

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
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_001, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    bool ret = audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_002
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_002, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive->second, PAUSEDBYREMOTE);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_003
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_003, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::DUCK;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive->second, PAUSEDBYREMOTE);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_004
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_004, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::STOP;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive->second, STOP);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_005
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_005, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::PAUSEDBYREMOTE;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_RESUME;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    bool ret = audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_006
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_006, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::STOP;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_RESUME;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive, tmpFocusInfoList.begin());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_007
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_007, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::STOP;
    tmpFocusInfoList.emplace_back(audioInterrupt, audioFocuState);

    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = tmpFocusInfoList.begin();

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive, tmpFocusInfoList.begin());
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_008
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_008, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    audioInterruptService->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
    ASSERT_NE(audioInterruptService->dfxCollector_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = nullptr;
    audioInterruptService->zonesMap_.insert({0, audioInterruptZone});
    int32_t pid = 0;

    audioInterruptService->WriteSessionTimeoutDfxEvent(pid);
    EXPECT_EQ(audioInterruptService->zonesMap_.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_009
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_009, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    audioInterruptService->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
    ASSERT_NE(audioInterruptService->dfxCollector_, nullptr);
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 5;
    AudioFocuState audioFocuState;
    audioInterruptZone->audioFocusInfoList.emplace_back(audioInterrupt, audioFocuState);
    audioInterruptService->zonesMap_.insert({0, audioInterruptZone});
    int32_t pid = 5;

    audioInterruptService->WriteSessionTimeoutDfxEvent(pid);
    EXPECT_EQ(audioInterruptService->zonesMap_.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_010
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_010, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 5;
    audioInterrupt.streamId = 1;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    AudioInterrupt interrupt;
    interrupt.pid = 5;
    interrupt.audioFocusType.streamType = STREAM_VOICE_COMMUNICATION;
    interrupt.streamId = 0;

    AudioFocuState focusState;
    focusState = PLACEHOLDER;
    audioFocusInfoList.emplace_back(interrupt, focusState);

    auto ret = audioInterruptService->HadVoipStatus(audioInterrupt, audioFocusInfoList);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_011
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_011, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 5;
    audioInterrupt.streamId = 1;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    AudioInterrupt interrupt;
    interrupt.pid = 5;
    interrupt.audioFocusType.streamType = STREAM_VOICE_COMMUNICATION;
    interrupt.streamId = 1;

    AudioFocuState focusState;
    focusState = PLACEHOLDER;
    audioFocusInfoList.emplace_back(interrupt, focusState);

    auto ret = audioInterruptService->HadVoipStatus(audioInterrupt, audioFocusInfoList);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_012
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_012, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 5;
    audioInterrupt.streamId = 1;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    AudioInterrupt interrupt;
    interrupt.pid = 5;
    interrupt.audioFocusType.streamType = STREAM_VOICE_RING;
    interrupt.streamId = 1;

    AudioFocuState focusState;
    focusState = PLACEHOLDER;
    audioFocusInfoList.emplace_back(interrupt, focusState);

    auto ret = audioInterruptService->HadVoipStatus(audioInterrupt, audioFocusInfoList);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_013
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_013, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 5;
    audioInterrupt.streamId = 1;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    AudioInterrupt interrupt;
    interrupt.pid = 5;
    interrupt.audioFocusType.streamType = STREAM_VOICE_RING;
    interrupt.streamId = 1;

    AudioFocuState focusState;
    focusState = STOP;
    audioFocusInfoList.emplace_back(interrupt, focusState);

    auto ret = audioInterruptService->HadVoipStatus(audioInterrupt, audioFocusInfoList);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_014
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_014, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 5;
    audioInterrupt.streamId = 1;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    AudioInterrupt interrupt;
    interrupt.pid = 0;
    interrupt.audioFocusType.streamType = STREAM_VOICE_RING;
    interrupt.streamId = 1;

    AudioFocuState focusState;
    focusState = STOP;
    audioFocusInfoList.emplace_back(interrupt, focusState);

    auto ret = audioInterruptService->HadVoipStatus(audioInterrupt, audioFocusInfoList);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_015
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_015, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    uint32_t streamId = 100005;
    InterruptEventInternal interruptEvent;

    audioInterruptService->DispatchInterruptEventWithStreamId(streamId, interruptEvent);
    ASSERT_NE(audioInterruptService, nullptr);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_016
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_016, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;

    uint32_t uid2 = 0;
    ClientType clientType = CLIENT_TYPE_OTHERS;
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid2, clientType);
    auto ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_017
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_017, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_DUCK;

    uint32_t uid2 = 0;
    ClientType clientType = CLIENT_TYPE_GAME;
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid2, clientType);
    auto ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(interruptEvent.callbackToApp, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_018
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_018, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_UNDUCK;

    uint32_t uid2 = 0;
    ClientType clientType = CLIENT_TYPE_GAME;
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid2, clientType);
    auto ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(interruptEvent.callbackToApp, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_019
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_019, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;

    uint32_t uid2 = 0;
    ClientType clientType = CLIENT_TYPE_GAME;
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid2, clientType);
    audioInterruptService->policyServer_ = new AudioPolicyServer(0);
    ASSERT_NE(audioInterruptService->policyServer_, nullptr);
    auto ret = audioInterruptService->ShouldCallbackToClient(uid, streamId, interruptEvent);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_020
* @tc.desc  : Test SetAudioManagerInterruptCallback
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_020, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    const sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = audioInterruptService->SetAudioManagerInterruptCallback(object);
    EXPECT_EQ(ret, SUCCESS);

    audioInterruptService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    ret = audioInterruptService->SetAudioManagerInterruptCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_021
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_021, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 5;

    int32_t map = audioInterruptService->ZONEID_DEFAULT;
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 5;
    AudioFocuState audioFocuState = ACTIVE;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, audioFocuState});
    audioInterruptService->zonesMap_.insert({map, audioInterruptZone});

    audioInterruptService->RemoveClient(zoneId, streamId);
    EXPECT_EQ(audioInterruptService->zonesMap_.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_022
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_022, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 5;

    int32_t map = audioInterruptService->ZONEID_DEFAULT;
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 0;
    AudioFocuState audioFocuState = ACTIVE;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, audioFocuState});

    uint32_t cb = 5;
    auto audioInterruptCallbackTest = std::make_shared<AudioInterruptCallbackTest>();
    audioInterruptZone->interruptCbsMap.insert({cb, audioInterruptCallbackTest});

    audioInterruptService->zonesMap_.insert({map, audioInterruptZone});
    EXPECT_EQ(audioInterruptService->zonesMap_.find(zoneId)->second->interruptCbsMap.size(), 1);

    audioInterruptService->RemoveClient(zoneId, streamId);
    EXPECT_EQ(audioInterruptService->zonesMap_.size(), 1);
    EXPECT_EQ(audioInterruptService->zonesMap_.find(zoneId)->second->interruptCbsMap.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_023
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_023, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 5;

    int32_t map = audioInterruptService->ZONEID_DEFAULT;
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 0;
    AudioFocuState audioFocuState = ACTIVE;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, audioFocuState});

    uint32_t cb = 0;
    auto audioInterruptCallbackTest = std::make_shared<AudioInterruptCallbackTest>();
    audioInterruptZone->interruptCbsMap.insert({cb, audioInterruptCallbackTest});

    audioInterruptService->zonesMap_.insert({map, audioInterruptZone});
    EXPECT_EQ(audioInterruptService->zonesMap_.find(zoneId)->second->interruptCbsMap.size(), 1);

    audioInterruptService->RemoveClient(zoneId, streamId);
    EXPECT_EQ(audioInterruptService->zonesMap_.size(), 1);
    EXPECT_EQ(audioInterruptService->zonesMap_.find(zoneId)->second->interruptCbsMap.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_024
* @tc.desc  : Test AudioInterruptService
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_024, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 1;
    uint32_t streamId = 5;

    int32_t map = audioInterruptService->ZONEID_DEFAULT;
    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 0;
    AudioFocuState audioFocuState = ACTIVE;
    audioInterruptZone->audioFocusInfoList.push_back({audioInterrupt, audioFocuState});

    uint32_t cb = 0;
    auto audioInterruptCallbackTest = std::make_shared<AudioInterruptCallbackTest>();
    audioInterruptZone->interruptCbsMap.insert({cb, audioInterruptCallbackTest});

    audioInterruptService->zonesMap_.insert({map, audioInterruptZone});

    audioInterruptService->RemoveClient(zoneId, streamId);
    EXPECT_EQ(audioInterruptService->zonesMap_.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_034
* @tc.desc  : Test NotifyFocusGranted
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_034, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t clientId = 0;
    AudioInterrupt audioInterrupt;
    audioInterruptService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_TRUE(audioInterruptService->handler_ != nullptr);
    audioInterruptService->NotifyFocusGranted(clientId, audioInterrupt);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_035
* @tc.desc  : Test NotifyFocusAbandoned
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_035, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t clientId = 0;
    AudioInterrupt audioInterrupt;
    audioInterruptService->handler_ = std::make_shared<AudioPolicyServerHandler>();
    ASSERT_TRUE(audioInterruptService->handler_ != nullptr);
    auto ret = audioInterruptService->NotifyFocusAbandoned(clientId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_036
* @tc.desc  : Test AbandonAudioFocusInternal
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_036, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t clientId = 0;
    AudioInterrupt audioInterrupt;
    auto ret = audioInterruptService->AbandonAudioFocusInternal(clientId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);

    clientId = 10;
    audioInterruptService->AbandonAudioFocusInternal(clientId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_037
* @tc.desc  : Test CheckAudioSessionExistence
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_037, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.pid = 0;
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;

    AudioSessionStrategy strategy;
    std::shared_ptr<AudioSessionStateMonitor> audioSessionStateMonitor = nullptr;
    auto audioSession = std::make_shared<AudioSession>(incomingInterrupt.pid, strategy, audioSessionStateMonitor);
    audioInterruptService->sessionService_ = std::make_shared<AudioSessionService>();
    audioInterruptService->sessionService_->sessionMap_[incomingInterrupt.pid] = audioSession;
    auto ret = audioInterruptService->CheckAudioSessionExistence(incomingInterrupt, focusEntry);
    EXPECT_FALSE(ret);
    audioInterruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);

    focusEntry.actionOn = INCOMING;
    ret = audioInterruptService->CheckAudioSessionExistence(incomingInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_038
* @tc.desc  : Test CheckAudioSessionExistence
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_038, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.pid = 0;
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;

    audioInterruptService->sessionService_ = std::make_shared<AudioSessionService>();
    audioInterruptService->sessionService_->sessionMap_[incomingInterrupt.pid] = nullptr;
    auto ret = audioInterruptService->CheckAudioSessionExistence(incomingInterrupt, focusEntry);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_039
* @tc.desc  : Test UpdateHintTypeForExistingSession
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_039, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_DUCK;
    incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::DUCK_OTHERS;
    audioInterruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);
    EXPECT_EQ(focusEntry.hintType, INTERRUPT_HINT_DUCK);

    focusEntry.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);
    EXPECT_EQ(focusEntry.hintType, INTERRUPT_HINT_DUCK);

    focusEntry.hintType = INTERRUPT_HINT_NONE;
    audioInterruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_040
* @tc.desc  : Test UpdateHintTypeForExistingSession
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_040, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt;
    AudioFocusEntry focusEntry;
    incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::PAUSE_OTHERS;

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);
    EXPECT_EQ(focusEntry.hintType, INTERRUPT_HINT_PAUSE);

    focusEntry.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);
    EXPECT_EQ(focusEntry.hintType, INTERRUPT_HINT_PAUSE);

    focusEntry.hintType = INTERRUPT_HINT_NONE;
    audioInterruptService->UpdateHintTypeForExistingSession(incomingInterrupt, focusEntry);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_041
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_041, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioFocusEntry focusEntry;
    AudioInterrupt incomingInterrupt;
    bool removeFocusInfo = true;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioList;
    audioList.push_back(std::make_pair(incomingInterrupt, ACTIVE));
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = audioList.begin();

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt,
        removeFocusInfo, interruptEvent);
    EXPECT_EQ(interruptEvent.hintType, focusEntry.hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_042
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_042, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioFocusEntry focusEntry;
    AudioInterrupt incomingInterrupt;
    bool removeFocusInfo = true;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioList;
    audioList.push_back(std::make_pair(incomingInterrupt, DUCK));
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = audioList.begin();

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt,
        removeFocusInfo, interruptEvent);
    EXPECT_EQ(interruptEvent.hintType, focusEntry.hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_043
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_043, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioFocusEntry focusEntry;
    AudioInterrupt incomingInterrupt;
    bool removeFocusInfo = true;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioList;
    audioList.push_back(std::make_pair(incomingInterrupt, STOP));
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = audioList.begin();

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt,
        removeFocusInfo, interruptEvent);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_044
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_044, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioFocusEntry focusEntry;
    AudioInterrupt incomingInterrupt;
    bool removeFocusInfo = true;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioList;
    audioList.push_back(std::make_pair(incomingInterrupt, ACTIVE));
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = audioList.begin();

    focusEntry.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt,
        removeFocusInfo, interruptEvent);
    EXPECT_EQ(interruptEvent.hintType, focusEntry.hintType);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_045
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_045, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioFocusEntry focusEntry;
    AudioInterrupt incomingInterrupt;
    bool removeFocusInfo = true;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioList;
    audioList.push_back(std::make_pair(incomingInterrupt, DUCK));
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = audioList.begin();

    focusEntry.hintType = INTERRUPT_HINT_DUCK;
    audioInterruptService->ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt,
        removeFocusInfo, interruptEvent);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_046
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_046, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioFocusEntry focusEntry;
    AudioInterrupt incomingInterrupt;
    bool removeFocusInfo = true;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> audioList;
    audioList.push_back(std::make_pair(incomingInterrupt, DUCK));
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive = audioList.begin();

    focusEntry.hintType = INTERRUPT_HINT_UNDUCK;
    audioInterruptService->ProcessExistInterrupt(iterActive, focusEntry, incomingInterrupt,
        removeFocusInfo, interruptEvent);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_047
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_047, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;

    tmpFocusInfoList.push_back(std::make_pair<>(audioInterrupt, ACTIVE));
    iterActive = tmpFocusInfoList.begin();

    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_048
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_048, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;

    tmpFocusInfoList.push_back(std::make_pair<>(audioInterrupt, ACTIVE));
    iterActive = tmpFocusInfoList.begin();

    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive->second, PAUSEDBYREMOTE);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_049
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_049, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;

    tmpFocusInfoList.push_back(std::make_pair<>(audioInterrupt, DUCK));
    iterActive = tmpFocusInfoList.begin();

    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive->second, PAUSEDBYREMOTE);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_050
* @tc.desc  : Test ProcessExistInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_050, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt audioInterrupt;
    InterruptEventInternal interruptEvent;
    std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator iterActive;
    std::list<std::pair<AudioInterrupt, AudioFocuState>> tmpFocusInfoList;

    tmpFocusInfoList.push_back(std::make_pair<>(audioInterrupt, PLACEHOLDER));
    iterActive = tmpFocusInfoList.begin();

    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_051
* @tc.desc  : Test ReportRecordGetFocusFail
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_051, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    AudioInterrupt incomingInterrupt = {};
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    AudioInterrupt activeInterrupt = {};
    activeInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    audioInterruptService->ReportRecordGetFocusFail(incomingInterrupt, activeInterrupt, RECORD_ERROR_GET_FOCUS_FAIL);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_052
* @tc.desc  : Test UpdateMicFocusStrategy
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_052, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    SourceType existSourceType;
    SourceType incomingSourceType;
    std::string bundleName = "";
    AudioFocusEntry focusEntry;

    existSourceType = SOURCE_TYPE_INVALID;
    incomingSourceType = SOURCE_TYPE_MIC;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService->UpdateMicFocusStrategy(existSourceType, incomingSourceType, bundleName, focusEntry);
    EXPECT_NE(focusEntry.hintType, INTERRUPT_HINT_NONE);
}

/**
* @tc.name  : Test AudioSessionFocusMode
* @tc.number: AudioInterruptService_053
* @tc.desc  : Test AudioSessionFocusMode
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_053, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t fakePid = 123;
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.pid = fakePid;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    incomingInterrupt.streamId = 888; // 888 is a fake stream id.

    std::shared_ptr<AudioSessionService> sessionService = std::make_shared<AudioSessionService>();
    audioInterruptService->sessionService_ = sessionService;
    int ret = sessionService->SetAudioSessionScene(fakePid, AudioSessionScene::MEDIA);
    EXPECT_EQ(SUCCESS, ret);
    AudioSessionStrategy audioSessionStrategy;
    audioSessionStrategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    ret = sessionService->ActivateAudioSession(fakePid, audioSessionStrategy);
    EXPECT_EQ(SUCCESS, ret);
    ret = audioInterruptService->ActivateAudioInterrupt(0, incomingInterrupt, false);
    EXPECT_EQ(ERR_FOCUS_DENIED, ret);
}

} // namespace AudioStandard
} // namespace OHOS