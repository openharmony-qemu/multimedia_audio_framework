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

    void OnInterruptEvent(int32_t zoneId, int32_t deviceId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override
    {
        recvEvent_.type = AUDIO_ZONE_INTERRUPT_EVENT;
        recvEvent_.zoneId = zoneId;
        recvEvent_.deviceId = deviceId;
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

class AudioZoneVolumeUnitTest : public testing::Test {
    void SetUp() override
    {
        std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
        std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
        AudioZoneService::GetInstance().Init(handler, interruptService);
        AudioZoneContext context;
        auto zoneId1_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context);
        auto zoneId2_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context);
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

/**
* @tc.name  : Test AudioZoneVolume.
* @tc.number: AudioZoneVolume_001
* @tc.desc  : Test audio zone volume proxy.
*/
HWTEST_F(AudioZoneVolumeUnitTest, AudioZoneVolume_001, TestSize.Level1)
{
    EXPECT_NE(AudioZoneService::GetInstance().SetSystemVolumeLevelForZone(zoneId1_, STREAM_RING, 33, 0), 0);
    EXPECT_EQ(AudioZoneService::GetInstance().EnableSystemVolumeProxy(TEST_PID_1000, zoneId1_, true), 0);

    EXPECT_EQ(AudioZoneService::GetInstance().SetSystemVolumeLevelForZone(zoneId1_, STREAM_RING, 33, 0), 0);
    client1000_->Wait();
    EXPECT_EQ(client1000_->volumeLevel_, 33);
    EXPECT_EQ(AudioZoneService::GetInstance().GetSystemVolumeLevelForZone(zoneId1_, STREAM_RING), 33);
    EXPECT_EQ(AudioZoneService::GetInstance().EnableSystemVolumeProxy(TEST_PID_1000, zoneId1_, false), 0);
}
} // namespace AudioStandard
} // namespace OHOS