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

#include <condition_variable>
#include <mutex>
#include "gtest/gtest.h"
#include "audio_zone.h"
#include "audio_zone_client_manager.h"
#include "audio_policy_server_handler.h"
#include "audio_zone_service.h"
#include "audio_interrupt_service.h"
#include "i_audio_zone_event_dispatcher.h"

using namespace testing::ext;

#define TEST_PID_1000 1000

namespace OHOS {
namespace AudioStandard {
class AudioZoneUnitTestClient : public IStandardAudioZoneClient {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    void OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) override
    {
        recvEvent_.type = AUDIO_ZONE_ADD_EVENT;
        recvEvent_.zoneId = zoneDescriptor.zoneId_;
        Notify();
    }

    void OnAudioZoneRemove(int32_t zoneId) override
    {
        recvEvent_.type = AUDIO_ZONE_REMOVE_EVENT;
        recvEvent_.zoneId = zoneId;
        Notify();
    }

    void OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor &zoneDescriptor,
        AudioZoneChangeReason reason) override
    {
        recvEvent_.type = AUDIO_ZONE_CHANGE_EVENT;
        recvEvent_.zoneId = zoneId;
        recvEvent_.zoneChangeReason = reason;
        Notify();
    }

    void OnInterruptEvent(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override
    {
        recvEvent_.type = AUDIO_ZONE_INTERRUPT_EVENT;
        recvEvent_.zoneId = zoneId;
        recvEvent_.interrupts = interrupts;
        recvEvent_.zoneInterruptReason = reason;
        Notify();
    }

    void OnInterruptEvent(int32_t zoneId, const std::string &deviceTag,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override
    {
        recvEvent_.type = AUDIO_ZONE_INTERRUPT_EVENT;
        recvEvent_.zoneId = zoneId;
        recvEvent_.deviceTag = deviceTag;
        recvEvent_.interrupts = interrupts;
        recvEvent_.zoneInterruptReason = reason;
        Notify();
    }

    int32_t SetSystemVolume(const int32_t zoneId, const AudioVolumeType volumeType,
        const int32_t volumeLevel, const int32_t volumeFlag) override
    {
        volumeLevel_ = volumeLevel;
        Notify();
        return 0;
    }

    int32_t GetSystemVolume(int32_t zoneId, AudioVolumeType volumeType) override
    {
        Notify();
        return volumeLevel_;
    }

    int Notify()
    {
        std::unique_lock<std::mutex> lock(waitLock_);
        waitStatus_ = 1;
        waiter_.notify_one();
        return 0;
    }

    void Wait()
    {
        std::unique_lock<std::mutex> lock(waitLock_);
        if (waitStatus_ == 0) {
            waiter_.wait(lock, [this] {
                return waitStatus_ != 0;
            });
        }
        waitStatus_ = 0;
    }

    struct AudioZoneEvent recvEvent_;
    std::condition_variable waiter_;
    std::mutex waitLock_;
    int32_t waitStatus_ = 0;
    int32_t volumeLevel_ = 0;
};

class AudioZoneInterruptUnitTest : public testing::Test {
    void SetUp() override
    {
        std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
        std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
        AudioZoneService::GetInstance().Init(handler, interruptService);
        AudioZoneContext context;
        zoneId1_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context);
        zoneId2_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context);
        sptr<AudioZoneUnitTestClient> client = new AudioZoneUnitTestClient();
        AudioZoneClientManager::GetInstance().RegisterAudioZoneClient(TEST_PID_1000, client);
        client1000_ = client;
    }

    void TearDown() override
    {
        zoneId1_ = 0;
        zoneId2_ = 0;
        client1000_ = nullptr;
        AudioZoneService::GetInstance().DeInit();
    }

    int32_t zoneId1_ = 0;
    int32_t zoneId2_ = 0;
    sptr<AudioZoneUnitTestClient> client1000_;
};

struct InterruptInfo {
    StreamUsage usage;
    AudioStreamType type;
    uint32_t streamId;
    int32_t pid;
    int32_t uid;
    std::string deviceTag;
};

static void ActivateInterrupt(int32_t zoneId, InterruptInfo info)
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = info.usage;
    interrupt.audioFocusType.streamType = info.type;
    interrupt.streamId = info.streamId;
    interrupt.pid = info.pid;
    interrupt.uid = info.uid;
    interrupt.deviceId = info.deviceId;
    AudioZoneService::GetInstance().ActivateAudioInterrupt(zoneId, interrupt);
}

static void DeActivateInterrupt(int32_t zoneId, InterruptInfo info)
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = info.usage;
    interrupt.audioFocusType.streamType = info.type;
    interrupt.streamId = info.streamId;
    interrupt.pid = info.pid;
    interrupt.uid = info.uid;
    interrupt.deviceTag = info.deviceTag;
    AudioZoneService::GetInstance().DeactivateAudioInterrupt(zoneId, interrupt);
}

static void ActivateMusicInterrupt(int32_t zoneId, InterruptInfo info)
{
    info.type = STREAM_MUSIC;
    info.usage = STREAM_USAGE_MUSIC;
    ActivateInterrupt(zoneId, info);
}

static void DeActivateMusicInterrupt(int32_t zoneId, InterruptInfo info)
{
    info.type = STREAM_MUSIC;
    info.usage = STREAM_USAGE_MUSIC;
    DeActivateInterrupt(zoneId, info);
}

static void ActivateVoipInterrupt(int32_t zoneId, InterruptInfo info)
{
    info.type = STREAM_VOICE_CALL;
    info.usage = STREAM_USAGE_VOICE_COMMUNICATION;
    ActivateInterrupt(zoneId, info);
}

static void DeActivateVoipInterrupt(int32_t zoneId, InterruptInfo info)
{
    info.type = STREAM_VOICE_CALL;
    info.usage = STREAM_USAGE_VOICE_COMMUNICATION;
    DeActivateInterrupt(zoneId, info);
}

static void ActivateMovieInterrupt(int32_t zoneId, InterruptInfo info)
{
    info.type = STREAM_MOVIE;
    info.usage = STREAM_USAGE_MOVIE;
    ActivateInterrupt(zoneId, info);
}

static void DeActivateMovieInterrupt(int32_t zoneId, InterruptInfo info)
{
    info.type = STREAM_MOVIE;
    info.usage = STREAM_USAGE_MOVIE;
    DeActivateInterrupt(zoneId, info);
}

static void ActivateGamecInterrupt(int32_t zoneId, InterruptInfo info)
{
    info.type = STREAM_GAME;
    info.usage = STREAM_USAGE_GAME;
    ActivateInterrupt(zoneId, info);
}

static void DeActivateGameInterrupt(int32_t zoneId, InterruptInfo info)
{
    info.type = STREAM_GAME;
    info.usage = STREAM_USAGE_GAME;
    DeActivateInterrupt(zoneId, info);
}

static void AddInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    InterruptInfo info, AudioFocuState state)
{
    AudioInterrupt interrupt;
    interrupt.streamUsage = info.usage;
    interrupt.audioFocusType.streamType = info.type;
    interrupt.streamId = info.streamId;
    interrupt.pid = info.pid;
    interrupt.uid = info.uid;
    interrupt.deviceTag = info.deviceTag;

    interrupts.emplace_back(std::make_pair(interrupt, state));
}

static void AddMusicInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    InterruptInfo info, AudioFocuState state)
{
    info.type = STREAM_MUSIC;
    info.usage = STREAM_USAGE_MUSIC;
    AddInterruptToList(interrupts, info, state);
}

static void AddVoipInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    InterruptInfo info, AudioFocuState state)
{
    info.type = STREAM_VOICE_CALL;
    info.usage = STREAM_USAGE_VOICE_COMMUNICATION;
    AddInterruptToList(interrupts, info, state);
}

static void AddMovieInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    InterruptInfo info, AudioFocuState state)
{
    info.type = STREAM_MOVIE;
    info.usage = STREAM_USAGE_MOVIE;
    AddInterruptToList(interrupts, info, state);
}

static void AddGameInterruptToList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
    InterruptInfo info, AudioFocuState state)
{
    info.type = STREAM_GAME;
    info.usage = STREAM_USAGE_GAME;
    AddInterruptToList(interrupts, info, state);
}

/**
* @tc.name  : Test AudioZoneInterrupt.
* @tc.number: AudioZoneInterrupt_001
* @tc.desc  : Test audio zone interrupt.
*/
HWTEST_F(AudioZoneInterruptUnitTest, AudioZoneInterrupt_001, TestSize.Level1)
{
    InterruptInfo info;
    info.streamId = 10;
    info.pid = 10;
    info.uid = 10;
    info.deviceTag = "";
    ActivateMusicInterrupt(0, info);
    info.streamId = 20;
    info.pid = 20;
    info.uid = 20;
    ActivateVoipInterrupt(0, info);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId1_, 10);
    auto interruptList = AudioZoneService::GetInstance().GetAudioInterruptForZone(zoneId1_);
    EXPECT_EQ(interruptList.size(), 0);
}

/**
* @tc.name  : Test AudioZoneInterrupt.
* @tc.number: AudioZoneInterrupt_002
* @tc.desc  : Test audio zone interrupt.
*/
HWTEST_F(AudioZoneInterruptUnitTest, AudioZoneInterrupt_002, TestSize.Level1)
{
    InterruptInfo info;
    info.streamId = 10;
    info.pid = 10;
    info.uid = 10;
    info.deviceTag = "";
    ActivateMusicInterrupt(0, info);
    AudioZoneService::GetInstance().AddUidToAudioZone(zoneId1_, 20);
    info.streamId = 20;
    info.pid = 20;
    info.uid = 20;
    ActivateVoipInterrupt(zoneId1_, info);
    AudioZoneService::GetInstance().RemoveUidFromAudioZone(zoneId1_, 20);
    auto interruptList1 = AudioZoneService::GetInstance().GetAudioInterruptForZone(zoneId1_);
    EXPECT_EQ(interruptList1.size(), 0);
}
} // namespace AudioStandard
} // namespace OHOS