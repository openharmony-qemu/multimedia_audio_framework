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

void AudioInterruptServiceUnitTest::SetUp(void)
{
    audioInterruptService_ = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService_, nullptr);
    audioInterruptService_->sessionService_.sessionMap_.clear();
    audioInterruptService_->sessionService_.timeOutCallback_.reset();
}

void AudioInterruptServiceUnitTest::TearDown(void)
{
    audioInterruptService_ = nullptr;
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: AudioInterruptService_001
 * @tc.desc  : Test SetAudioInterruptCallback
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_001, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 0;
    uint32_t uid = 0;
    sptr<AudioPolicyManagerListenerStubImpl> interruptListenerStub =
        new(std::nothrow) AudioPolicyManagerListenerStubImpl();
    sptr<IRemoteObject> object = interruptListenerStub->AsObject();
    auto ret = audioInterruptService->SetAudioInterruptCallback(zoneId, streamId, object, uid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: AudioInterruptService_002
 * @tc.desc  : Test SetAudioInterruptCallback
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_002, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 0;
    uint32_t uid = 0;
    sptr<AudioPolicyManagerListenerStubImpl> interruptListenerStub =
        new(std::nothrow) AudioPolicyManagerListenerStubImpl();
    sptr<IRemoteObject> object = interruptListenerStub->AsObject();

    audioInterruptService->zonesMap_.insert({zoneId, nullptr});
    auto ret = audioInterruptService->SetAudioInterruptCallback(zoneId, streamId, object, uid);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: AudioInterruptService_003
 * @tc.desc  : Test SetAudioInterruptCallback
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_003, TestSize.Level1)
{
    auto audioInterruptService = std::make_shared<AudioInterruptService>();
    ASSERT_NE(audioInterruptService, nullptr);

    int32_t zoneId = 0;
    uint32_t streamId = 0;
    uint32_t uid = 0;
    sptr<AudioPolicyManagerListenerStubImpl> interruptListenerStub =
        new(std::nothrow) AudioPolicyManagerListenerStubImpl();
    sptr<IRemoteObject> object = interruptListenerStub->AsObject();

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
 * @tc.number: AudioInterruptService_005
 * @tc.desc  : Test GetStreamTypePriority
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_005, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_006
 * @tc.desc  : Test GetStreamInFocusInternal
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_006, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_007
 * @tc.desc  : Test GetStreamInFocusInternal
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_007, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_008
 * @tc.desc  : Test GetStreamInFocusInternal
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_008, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_009
 * @tc.desc  : Test GetStreamInFocusInternal
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_009, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_010
 * @tc.desc  : Test GetStreamInFocusInternal
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_010, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_011
 * @tc.desc  : Test GetStreamInFocusInternal
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_011, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_012
 * @tc.desc  : Test GetSessionInfoInFocus
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_012, TestSize.Level1)
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
 * @tc.number: AudioInterruptService_013
 * @tc.desc  : Test GetSessionInfoInFocus
 */
HWTEST(AudioInterruptServiceProUnitTest, AudioInterruptService_013, TestSize.Level1)
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

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: IsSessionNeedToFetchOutputDevice_001
 * @tc.desc  : Test IsSessionNeedToFetchOutputDevice
 */
HWTEST_F(AudioInterruptServiceUnitTest, IsSessionNeedToFetchOutputDevice_001, TestSize.Level4)
{
    int32_t callerPid = 1001;
    bool ret = audioInterruptService_->sessionService_.IsSessionNeedToFetchOutputDevice(callerPid);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: DeactivateAudioSessionFakeInterrupt_001
 * @tc.desc  : Test DeactivateAudioSessionFakeInterrupt iter != audioFocusInfoList.end()
 */
HWTEST_F(AudioInterruptServiceUnitTest, DeactivateAudioSessionFakeInterrupt_001, TestSize.Level4)
{
    int32_t zoneId = 1;
    const int32_t callerPid = 1001;
    bool isSessionTimeout = false;
    std::shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = callerPid;
    audioInterrupt.isAudioSessionInterrupt = true;
    zone->audioFocusInfoList.push_back(
        std::make_pair(audioInterrupt, AudioFocuState::ACTIVE));
    audioInterruptService_->zonesMap_[zoneId] = zone;
    EXPECT_NO_THROW(
        audioInterruptService_->DeactivateAudioSessionFakeInterruptInternal(zoneId, callerPid, isSessionTimeout));
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: CanMixForSession_001
 * @tc.desc  : Test CanMixForSession IsCanMixInterrupt(incomingInterrupt, activeInterrupt) == false
 */
HWTEST_F(AudioInterruptServiceUnitTest, CanMixForSession_001, TestSize.Level4)
{
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_VOICE_CALL;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = false;
    EXPECT_FALSE(audioInterruptService_->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: CanMixForSession_002
 * @tc.desc  : Test CanMixForSession incomingInterrupt.audioFocusType.streamType == STREAM_INTERNAL_FORCE_STOP ||
 *             activeInterrupt.audioFocusType.streamType == STREAM_INTERNAL_FORCE_STOP
 */
HWTEST_F(AudioInterruptServiceUnitTest, CanMixForSession_002, TestSize.Level4)
{
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_INVALID;
    incomingInterrupt.audioFocusType.streamType = STREAM_INTERNAL_FORCE_STOP;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_DEFAULT;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = false;
    EXPECT_FALSE(audioInterruptService_->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry));

    activeInterrupt.audioFocusType.streamType = STREAM_INTERNAL_FORCE_STOP;
    EXPECT_FALSE(audioInterruptService_->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: CanMixForIncomingSession_001
 * @tc.desc  : Test CanMixForIncomingSession concurrencyMode == AudioConcurrencyMode::SILENT ||
 *             concurrencyMode == AudioConcurrencyMode::MIX_WITH_OTHERS
 */
HWTEST_F(AudioInterruptServiceUnitTest, CanMixForIncomingSession_001, TestSize.Level4)
{
    AudioInterrupt incomingInterrupt;
    incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    EXPECT_TRUE(audioInterruptService_->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry));

    incomingInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    EXPECT_TRUE(audioInterruptService_->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: CanMixForActiveSession_001
 * @tc.desc  : Test CanMixForActiveSession IsIncomingStreamLowPriority(focusEntry) == true
 */
HWTEST_F(AudioInterruptServiceUnitTest, CanMixForActiveSession_001, TestSize.Level4)
{
    int32_t callerPid = 1001;
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    AudioFocusEntry focusEntry;
    focusEntry.actionOn = CURRENT;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    std::shared_ptr<AudioSession> session =
        std::make_shared<AudioSession>(callerPid, strategy, audioSessionStateMonitor_);
    session->state_ = AudioSessionState::SESSION_ACTIVE;
    audioInterruptService_->sessionService_.sessionMap_[callerPid] = session;
    EXPECT_FALSE(audioInterruptService_->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: CanMixForActiveSession_002
 * @tc.desc  : Test CanMixForActiveSession concurrencyMode == AudioConcurrencyMode::SILENT ||
 *             concurrencyMode == AudioConcurrencyMode::MIX_WITH_OTHERS
 */
HWTEST_F(AudioInterruptServiceUnitTest, CanMixForActiveSession_002, TestSize.Level4)
{
    AudioInterrupt incomingInterrupt;
    AudioInterrupt activeInterrupt;
    activeInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::SILENT;
    AudioFocusEntry focusEntry;
    EXPECT_TRUE(audioInterruptService_->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry));

    activeInterrupt.sessionStrategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    EXPECT_TRUE(audioInterruptService_->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry));
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: UnsetAudioInterruptCallback_001
 * @tc.desc  : Test UnsetAudioInterruptCallback interruptClients_.erase(streamId) == 0
 */
HWTEST_F(AudioInterruptServiceUnitTest, UnsetAudioInterruptCallback_001, TestSize.Level4)
{
    int32_t zoneId = 1;
    uint32_t streamId = 1;
    audioInterruptService_->interruptClients_.clear();
    EXPECT_EQ(audioInterruptService_->UnsetAudioInterruptCallback(zoneId, streamId), ERR_INVALID_PARAM);
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: DeactivateAudioInterrupt_001
 * @tc.desc  : Test DeactivateAudioInterrupt HasAudioSessionFakeInterrupt(zoneId, currAudioInterrupt.pid) == true
 */
HWTEST_F(AudioInterruptServiceUnitTest, DeactivateAudioInterrupt_001, TestSize.Level4)
{
    int32_t zoneId = 1;
    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = 1;
    audioInterrupt.isAudioSessionInterrupt = true;
    std::shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    zone->audioFocusInfoList.push_back(
        std::make_pair(audioInterrupt, AudioFocuState::ACTIVE));
    audioInterruptService_->zonesMap_[zoneId] = zone;
    EXPECT_EQ(audioInterruptService_->DeactivateAudioInterrupt(zoneId, audioInterrupt), SUCCESS);
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: GetSessionInfoInFocus_001
 * @tc.desc  : Test GetSessionInfoInFocus iter->second == ACTIVE
 */
HWTEST_F(AudioInterruptServiceUnitTest, GetSessionInfoInFocus_001, TestSize.Level4)
{
    AudioInterrupt audioInterrupt;
    int32_t zoneId = 1;
    std::shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    zone->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, AudioFocuState::ACTIVE));
    zone->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, AudioFocuState::MUTED));
    audioInterruptService_->zonesMap_[zoneId] = zone;
    EXPECT_EQ(audioInterruptService_->GetSessionInfoInFocus(audioInterrupt, zoneId), SUCCESS);
}

/**
 * @tc.name  : Test AudioInterruptService
 * @tc.number: ProcessRemoteInterrupt_001
 * @tc.desc  : Test ProcessRemoteInterrupt targetZoneIt == zonesMap_.end()
 */
HWTEST_F(AudioInterruptServiceUnitTest, ProcessRemoteInterrupt_001, TestSize.Level4)
{
    std::set<int32_t> streamIds;
    InterruptEventInternal interruptEvent;
    int32_t zoneMapId = 0;
    std::shared_ptr<AudioInterruptZone> zone = std::make_shared<AudioInterruptZone>();
    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = 1;
    zone->zoneId = 1;
    zone->audioFocusInfoList.push_back(std::make_pair(audioInterrupt, AudioFocuState::ACTIVE));
    audioInterruptService_->zonesMap_[zoneMapId] = zone;
    streamIds.insert(audioInterrupt.streamId);

    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService_->ProcessRemoteInterrupt(streamIds, interruptEvent);

    interruptEvent.hintType = INTERRUPT_HINT_STOP;
    audioInterruptService_->ProcessRemoteInterrupt(streamIds, interruptEvent);

    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService_->ProcessRemoteInterrupt(streamIds, interruptEvent);
    EXPECT_EQ(zone->zoneId, 0);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: WriteSessionTimeoutDfxEvent_001
* @tc.desc  : Test itZone->second == nullptr
*/
HWTEST_F(AudioInterruptServiceUnitTest, WriteSessionTimeoutDfxEvent_001, TestSize.Level4)
{
    int32_t pid = 0;
    audioInterruptService_->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();

    audioInterruptService_->zonesMap_.clear();
    audioInterruptService_->zonesMap_.insert({0, nullptr});

    audioInterruptService_->WriteSessionTimeoutDfxEvent(pid);
    EXPECT_EQ(audioInterruptService_->zonesMap_.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: WriteSessionTimeoutDfxEvent_002
* @tc.desc  : Test iter == audioFocusInfoList.end()
*/
HWTEST_F(AudioInterruptServiceUnitTest, WriteSessionTimeoutDfxEvent_002, TestSize.Level4)
{
    int32_t pid = 1;
    audioInterruptService_->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
    ASSERT_NE(audioInterruptService_->dfxCollector_, nullptr);

    audioInterruptService_->zonesMap_.clear();

    auto zone1 = std::make_shared<AudioInterruptZone>();
    ASSERT_NE(zone1, nullptr);

    AudioInterrupt audioInterrupt;
    audioInterrupt.pid = pid;
    AudioFocuState audioFocuState = ACTIVE;
    zone1->audioFocusInfoList.push_back({audioInterrupt, audioFocuState});
    
    audioInterruptService_->zonesMap_.insert({0, zone1});

    audioInterruptService_->WriteSessionTimeoutDfxEvent(pid);
    EXPECT_EQ(audioInterruptService_->zonesMap_.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: WriteStartDfxMsg_001
* @tc.desc  : Test audioInterrupt.state == State::PREPARED
*/
HWTEST_F(AudioInterruptServiceUnitTest, WriteStartDfxMsg_001, TestSize.Level4)
{
    audioInterruptService_->dfxCollector_ = std::make_unique<AudioInterruptDfxCollector>();
    ASSERT_NE(audioInterruptService_->dfxCollector_, nullptr);

    InterruptDfxBuilder dfxBuilder;
    AudioInterrupt audioInterrupt;
    audioInterrupt.state = State::PREPARED;
    audioInterruptService_->WriteStartDfxMsg(dfxBuilder, audioInterrupt);
    EXPECT_EQ(audioInterruptService_->zonesMap_.size(), 0);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: GetAppState_001
* @tc.desc  : Test state != 0
*/
HWTEST_F(AudioInterruptServiceUnitTest, GetAppState_001, TestSize.Level4)
{
    OHOS::AppExecFwk::AppMgrClient appManager;
    int32_t appPid = 0;
    std::vector<AppExecFwk::RunningProcessInfo> info;
    appManager.GetAllRunningProcesses(info);
    if (!info.empty()) {
        appPid = info[0].pid_;
    }
    std::vector<int32_t> pids;
    pids.push_back (appPid);
    appManager.KillProcessesByPids(pids);

    audioInterruptService_->GetAppState(appPid);
    EXPECT_TRUE(audioInterruptService_->GetAppState(appPid) != 0);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: RemoveClient_004
* @tc.desc  : Test it->second->interruptCbsMap.find(streamId) != it->second->interruptCbsMap.end()
*/
HWTEST_F(AudioInterruptServiceUnitTest, RemoveClient_004, TestSize.Level4)
{
    int32_t zoneId = 0;
    uint32_t streamId = 0;
    audioInterruptService_->zonesMap_.clear();
    auto zone1 = std::make_shared<AudioInterruptZone>();
    ASSERT_NE(zone1, nullptr);

    AudioInterrupt audioInterrupt;
    audioInterrupt.streamId = streamId;
    AudioFocuState audioFocuState = ACTIVE;
    zone1->audioFocusInfoList.push_back({audioInterrupt, audioFocuState});

    zone1->interruptCbsMap.insert({streamId, nullptr});
    audioInterruptService_->zonesMap_.insert({zoneId, zone1});

    audioInterruptService_->RemoveClient(zoneId, streamId);
    EXPECT_EQ(audioInterruptService_->zonesMap_.size(), 1);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: ShouldCallbackToClient_001
* @tc.desc  : Test clientType != CLIENT_TYPE_GAME
*/
HWTEST_F(AudioInterruptServiceUnitTest, ShouldCallbackToClient_001, TestSize.Level4)
{
    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;

    ClientTypeManager::GetInstance()->clientTypeMap_.clear();

    EXPECT_EQ(audioInterruptService_->ShouldCallbackToClient(uid, streamId, interruptEvent), true);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: ShouldCallbackToClient_002
* @tc.desc  : Test interruptEvent.hintType == INTERRUPT_HINT_DUCK
*/
HWTEST_F(AudioInterruptServiceUnitTest, ShouldCallbackToClient_002, TestSize.Level4)
{
    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_DUCK;

    ClientTypeManager::GetInstance()->clientTypeMap_.clear();
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid, ClientType::CLIENT_TYPE_GAME);
    EXPECT_EQ(audioInterruptService_->ShouldCallbackToClient(uid, streamId, interruptEvent), true);
    EXPECT_EQ(interruptEvent.callbackToApp, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: ShouldCallbackToClient_003
* @tc.desc  : Test interruptEvent.hintType == INTERRUPT_HINT_UNDUCK
*/
HWTEST_F(AudioInterruptServiceUnitTest, ShouldCallbackToClient_003, TestSize.Level4)
{
    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_UNDUCK;

    ClientTypeManager::GetInstance()->clientTypeMap_.clear();
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid, ClientType::CLIENT_TYPE_GAME);
    EXPECT_EQ(audioInterruptService_->ShouldCallbackToClient(uid, streamId, interruptEvent), true);
    EXPECT_EQ(interruptEvent.callbackToApp, false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: ShouldCallbackToClient_004
* @tc.desc  : Test interruptEvent.hintType = INTERRUPT_HINT_RESUME
*/
HWTEST_F(AudioInterruptServiceUnitTest, ShouldCallbackToClient_004, TestSize.Level4)
{
    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_RESUME;

    ClientTypeManager::GetInstance()->clientTypeMap_.clear();
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid, ClientType::CLIENT_TYPE_GAME);

    audioInterruptService_->policyServer_ = new AudioPolicyServer(0);
    ASSERT_NE(audioInterruptService_->policyServer_, nullptr);

    EXPECT_EQ(audioInterruptService_->ShouldCallbackToClient(uid, streamId, interruptEvent), false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: ShouldCallbackToClient_005
* @tc.desc  : Test interruptEvent.hintType = INTERRUPT_HINT_PAUSE
*/
HWTEST_F(AudioInterruptServiceUnitTest, ShouldCallbackToClient_005, TestSize.Level4)
{
    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_PAUSE;

    ClientTypeManager::GetInstance()->clientTypeMap_.clear();
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid, ClientType::CLIENT_TYPE_GAME);
    audioInterruptService_->policyServer_ = new AudioPolicyServer(0);
    ASSERT_NE(audioInterruptService_->policyServer_, nullptr);

    EXPECT_EQ(audioInterruptService_->ShouldCallbackToClient(uid, streamId, interruptEvent), false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: ShouldCallbackToClient_006
* @tc.desc  : Test interruptEvent.hintType = INTERRUPT_HINT_STOP
*/
HWTEST_F(AudioInterruptServiceUnitTest, ShouldCallbackToClient_006, TestSize.Level4)
{
    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_STOP;

    ClientTypeManager::GetInstance()->clientTypeMap_.clear();
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid, ClientType::CLIENT_TYPE_GAME);
    audioInterruptService_->policyServer_ = new AudioPolicyServer(0);
    ASSERT_NE(audioInterruptService_->policyServer_, nullptr);

    EXPECT_EQ(audioInterruptService_->ShouldCallbackToClient(uid, streamId, interruptEvent), false);
}

/**
* @tc.name  : Test AudioInterruptService
* @tc.number: ShouldCallbackToClient_007
* @tc.desc  : Test interruptEvent.hintType = INTERRUPT_HINT_NONE
*/
HWTEST_F(AudioInterruptServiceUnitTest, ShouldCallbackToClient_007, TestSize.Level4)
{
    uint32_t uid = 0;
    int32_t streamId = 0;
    InterruptEventInternal interruptEvent;
    interruptEvent.hintType = INTERRUPT_HINT_NONE;

    ClientTypeManager::GetInstance()->clientTypeMap_.clear();
    ClientTypeManager::GetInstance()->OnClientTypeQueryCompleted(uid, ClientType::CLIENT_TYPE_GAME);

    EXPECT_EQ(audioInterruptService_->ShouldCallbackToClient(uid, streamId, interruptEvent), false);
}

/**
* @tc.name  : Test InterruptStrategy Mute
* @tc.number: AudioInterruptStrategy_001
* @tc.desc  : Test InterruptStrategy Mute
*/
HWTEST_F(AudioInterruptServiceUnitTest, AudioInterruptStrategy_001, TestSize.Level1)
{
    int32_t fakePid = 123;
    AudioInterrupt incomingInterrupt1;
    incomingInterrupt1.pid = fakePid;
    incomingInterrupt1.audioFocusType.sourceType = SOURCE_TYPE_MIC;
    incomingInterrupt1.streamId = 888; // 888 is a fake stream id.

    int32_t fakePid2 = 124;
    AudioInterrupt incomingInterrupt2;
    incomingInterrupt2.pid = fakePid2;
    incomingInterrupt2.audioFocusType.sourceType = SOURCE_TYPE_UNPROCESSED;
    incomingInterrupt2.streamId = 889; // 889 is a fake stream id.
    incomingInterrupt1.strategy = InterruptStrategy::MUTE;

    AudioFocusEntry focusEntry;
    focusEntry.hintType = INTERRUPT_HINT_PAUSE;
    audioInterruptService_->UpdateMuteAudioFocusStrategy(incomingInterrupt1, incomingInterrupt2, focusEntry);
    EXPECT_EQ(focusEntry.hintType, INTERRUPT_HINT_MUTE);

    focusEntry.hintType = INTERRUPT_HINT_PAUSE;

    audioInterruptService_->UpdateMuteAudioFocusStrategy(incomingInterrupt2, incomingInterrupt1, focusEntry);
    EXPECT_EQ(focusEntry.hintType, INTERRUPT_HINT_MUTE);
}

} // namespace AudioStandard
} // namespace OHOS