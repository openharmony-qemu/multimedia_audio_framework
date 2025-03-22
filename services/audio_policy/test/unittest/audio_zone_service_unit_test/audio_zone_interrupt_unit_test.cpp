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

#include "audio_zone_unit_test_base.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioZoneInterruptUnitTest : public AudioZoneUnitTestBase {
};

static void ActivateInterrupt(int32_t zoneId, StreamUsage usage. AudioStreamType type,
    uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = usage;
    interrupt.audioFocusType.streamType = type;
    interrupt.sessionId = sessionId;
    interrupt.pid = pid;
    interrupt.uid = uid;
    interrupt.deviceId = deviceId;
    AudioZoneService::GetInstance().ActivateInterrupt(zoneId, interrupt);
}

static void DeActivateInterrupt(int32_t zoneId, StreamUsage usage. AudioStreamType type,
    uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = usage;
    interrupt.audioFocusType.streamType = type;
    interrupt.sessionId = sessionId;
    interrupt.pid = pid;
    interrupt.uid = uid;
    interrupt.deviceId = deviceId;
    AudioZoneService::GetInstance().DeActivateInterrupt(zoneId, interrupt);
}

static void ActivateMusicInterrupt(int32_t zoneId, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    ActivateInterrupt(zoneId, STREAM_USAGE_MUSIC, STREAM_MUSIC, sessionId, pid, uid, deviceId);
}

static void DeActivateMusicInterrupt(int32_t zoneId, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    DeActivateInterrupt(zoneId, STREAM_USAGE_MUSIC, STREAM_MUSIC, sessionId, pid, uid, deviceId);
}

static void ActivateVoipInterrupt(int32_t zoneId, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    ActivateInterrupt(zoneId, STREAM_USAGE_VOICE_COMMUNICATION, STREAM_VOICE_CALL, sessionId, pid, uid, deviceId);
}

static void DeActivateVoipInterrupt(int32_t zoneId, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    DeActivateInterrupt(zoneId, STREAM_USAGE_VOICE_COMMUNICATION, STREAM_VOICE_CALL, sessionId, pid, uid, deviceId);
}

static void ActivateMovieInterrupt(int32_t zoneId, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    ActivateInterrupt(zoneId, STREAM_USAGE_MOVIE, STREAM_MOVIE, sessionId, pid, uid, deviceId);
}

static void DeActivateMovieInterrupt(int32_t zoneId, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    DeActivateInterrupt(zoneId, STREAM_USAGE_MOVIE, STREAM_MOVIE, sessionId, pid, uid, deviceId);
}

static void ActivateGamecInterrupt(int32_t zoneId, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    ActivateInterrupt(zoneId, STREAM_USAGE_GAME, STREAM_GAME, sessionId, pid, uid, deviceId);
}

static void DeActivateGameInterrupt(int32_t zoneId, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId)
{
    DeActivateInterrupt(zoneId, STREAM_USAGE_GAME, STREAM_GAME, sessionId, pid, uid, deviceId);
}

static void AddInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    StreamUsage usage, AudioStreamType type, uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId,
    AudioFocuState state)
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = usage;
    interrupt.audioFocusType.streamType = type;
    interrupt.sessionId = sessionId;
    interrupt.pid = pid;
    interrupt.uid = uid;
    interrupt.deviceId = deviceId;

    interrupts.emplace_back(std::make_pair(interrupt, state));
}

static void AddMusicInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId,
    AudioFocuState state)
{
    AddInterruptToList(interrupts, STREAM_USAGE_MUSIC, STREAM_MUSIC, sessionId, pid, uid, deviceId, state);
}

static void AddVoipInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId,
    AudioFocuState state)
{
    AddInterruptToList(interrupts, STREAM_USAGE_VOICE_COMMUNICATION,
        STREAM_VOICE_CALL, sessionId, pid, uid, deviceId, state);
}

static void AddMovieInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId,
    AudioFocuState state)
{
    AddInterruptToList(interrupts, STREAM_USAGE_MOVIE, STREAM_MOVIE, sessionId, pid, uid, deviceId, state);
}

static void AddGameInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    uint_32_t sessionId, int32_t pid, int32_t uid, int32_t deviceId,
    AudioFocuState state)
{
    AddInterruptToList(interrupts, STREAM_USAGE_GAME, STREAM_GAME, sessionId, pid, uid, deviceId, state);
}

/**
* @tc.name  : Test AudioZoneInterrupt.
* @tc.number: AudioZoneInterrupt_001
* @tc.desc  : Test audio zone interrupt.
*/
HWTEST_F(AudioZoneInterruptUnitTest, AudioZoneInterrupt_001, TestSize.Level1)
{
    ActivateMusicInterrupt(0, 10, 10, 10, -1);
    ActivateVoipInterrupt(0, 20, 20, 20, -1);
    auto interruptList = AudioZoneService::GetInstance().GetAudioInterruptForZone(0);
    EXPECT_EQ(interruptList.size(), 2);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId1_, 10);
    interruptList = AudioZoneService::GetInstance().GetAudioInterruptForZone(zoneId1_);
    EXPECT_EQ(interruptList.size(), 1);
}

/**
* @tc.name  : Test AudioZoneInterrupt.
* @tc.number: AudioZoneInterrupt_002
* @tc.desc  : Test audio zone interrupt.
*/
HWTEST_F(AudioZoneInterruptUnitTest, AudioZoneInterrupt_002, TestSize.Level1)
{
    ActivateMusicInterrupt(0, 10, 10, 10, -1);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId1_, 20);
    ActivateVoipInterrupt(zoneId1_, 20, 20, 20, -1);
    AudioZoneService::GetInstance().RemoveUidFromAudioZone(zoneId1_, 20);
    auto interruptList = AudioZoneService::GetInstance().GetAudioInterruptForZone(zoneId1_);
    EXPECT_EQ(interruptList.size(), 0);
    auto interruptList = AudioZoneService::GetInstance().GetAudioInterruptForZone(0);
    EXPECT_EQ(interruptList.size(), 2);
}

/**
* @tc.name  : Test AudioZoneInterrupt.
* @tc.number: AudioZoneInterrupt_003
* @tc.desc  : Test audio zone interrupt.
*/
HWTEST_F(AudioZoneInterruptUnitTest, AudioZoneInterrupt_003, TestSize.Level1)
{
    AudioZoneService::GetInstance().EnableAudioZoneInterruptReport(1000, zoneId1_, -1, true);
    ActivateMusicInterrupt(0, 10, 10, 10, -1);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId1_, 20);
    ActivateVoipInterrupt(zoneId1_, 20, 20, 20, -1);
    client1000_->Wait();
    EXPECT_EQ(client1000_->recvEvent_.type, AUDIO_ZONE_INTERRUPT_EVENT);
    AudioZoneService::GetInstance().EnableAudioZoneInterruptReport(1000, zoneId1_, -1, false);
}
} // namespace AudioStandard
} // namespace OHOS