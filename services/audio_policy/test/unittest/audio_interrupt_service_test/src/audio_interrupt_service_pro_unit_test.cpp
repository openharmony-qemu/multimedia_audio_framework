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
} // namespace AudioStandard
} // namespace OHOS