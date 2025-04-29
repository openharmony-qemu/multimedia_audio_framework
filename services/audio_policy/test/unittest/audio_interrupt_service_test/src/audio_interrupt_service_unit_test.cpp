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

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive, tmpFocusInfoList.end());
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

    audioInterruptService->SwitchHintType(iterActive, interruptEvent, tmpFocusInfoList);
    EXPECT_EQ(iterActive, tmpFocusInfoList.end());
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
* @tc.desc  : Test SetAudioInterruptCallback
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_021, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 0;
    uint32_t uid = 0;
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    auto ret = audioInterruptService->SetAudioInterruptCallback(zoneId, streamId, object, uid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_022
* @tc.desc  : Test SetAudioInterruptCallback
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_022, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 0;
    uint32_t uid = 0;
    sptr<IRemoteObject> object = new RemoteObjectTestStub();

    audioInterruptService->zonesMap_.insert({zoneId, nullptr});
    auto ret = audioInterruptService->SetAudioInterruptCallback(zoneId, streamId, object, uid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_023
* @tc.desc  : Test SetAudioInterruptCallback
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_023, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 0;
    uint32_t uid = 0;
    sptr<IRemoteObject> object = new RemoteObjectTestStub();

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->SetAudioInterruptCallback(zoneId, streamId, object, uid);
    EXPECT_EQ(ret, SUCCESS);

    audioInterruptService->interruptClients_.insert({streamId, nullptr});
    ret = audioInterruptService->SetAudioInterruptCallback(zoneId, streamId, object, uid);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_024
* @tc.desc  : Test DeactivateAudioInterrupt
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_024, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    AudioInterrupt audioInterrupt;
    audioInterrupt.parallelPlayFlag = true;
    auto ret = audioInterruptService->DeactivateAudioInterrupt(zoneId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);

    audioInterrupt.parallelPlayFlag = false;
    ret = audioInterruptService->DeactivateAudioInterrupt(zoneId, audioInterrupt);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_025
* @tc.desc  : Test GetStreamTypePriority
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_025, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    auto ret = audioInterruptService->GetStreamTypePriority(AudioStreamType::STREAM_VOICE_CALL);
    EXPECT_EQ(ret, 0);

    ret = audioInterruptService->GetStreamTypePriority(AudioStreamType::STREAM_APP);
    EXPECT_EQ(ret, 100);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_026
* @tc.desc  : Test GetStreamInFocusInternal
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_026, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 0;
    int32_t zoneId = 0;

    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::PAUSE;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, zoneId);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_027
* @tc.desc  : Test GetStreamInFocusInternal
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_027, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 0;
    int32_t zoneId = 0;

    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, zoneId);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_028
* @tc.desc  : Test GetStreamInFocusInternal
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_028, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 0;
    int32_t zoneId = 0;

    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::PAUSE;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, zoneId);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_029
* @tc.desc  : Test GetStreamInFocusInternal
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_029, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 1;
    int32_t zoneId = 0;

    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::DUCK;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.uid = 0;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, zoneId);
    EXPECT_EQ(ret, STREAM_MUSIC);

    uid = 0;
    ret = audioInterruptService->GetStreamInFocusInternal(uid, zoneId);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_030
* @tc.desc  : Test GetStreamInFocusInternal
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_030, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 0;
    int32_t zoneId = 0;

    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::DUCK;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.uid = 1;
    audioInterrupt.audioFocusType.streamType = STREAM_VOICE_ASSISTANT;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, zoneId);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_031
* @tc.desc  : Test GetStreamInFocusInternal
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_031, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t uid = 1;
    int32_t zoneId = 0;

    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::DUCK;
    audioInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    audioInterrupt.uid = 1;
    audioInterrupt.audioFocusType.streamType = STREAM_MEDIA;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetStreamInFocusInternal(uid, zoneId);
    EXPECT_EQ(ret, STREAM_MUSIC);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_032
* @tc.desc  : Test GetSessionInfoInFocus
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_032, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::DUCK;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetSessionInfoInFocus(audioInterrupt, zoneId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: AudioInterruptService_033
* @tc.desc  : Test GetSessionInfoInFocus
*/
HWTEST(AudioInterruptServiceUnitTest, AudioInterruptService_033, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    AudioInterrupt audioInterrupt;
    AudioFocuState audioFocuState = AudioFocuState::ACTIVE;
    std::pair<AudioInterrupt, AudioFocuState> tmpFocusInfoList = std::make_pair(audioInterrupt, audioFocuState);

    std::shared_ptr<AudioInterruptZone> audioInterruptZone = std::make_shared<AudioInterruptZone>();
    audioInterruptZone->audioFocusInfoList.push_back(tmpFocusInfoList);

    audioInterruptService->zonesMap_.insert({zoneId, audioInterruptZone});
    auto ret = audioInterruptService->GetSessionInfoInFocus(audioInterrupt, zoneId);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS