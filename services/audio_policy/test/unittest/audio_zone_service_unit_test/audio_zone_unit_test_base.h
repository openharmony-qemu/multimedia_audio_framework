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

#ifndef AUDIO_ZONE_UNIT_TEST_BASE_H
#define AUDIO_ZONE_UNIT_TEST_BASE_H

#include <condition_variable>
#include <mutex>
#include "gtest/gtest.h"
#include "audio_zone.h"
#include "audio_zone_client_manager.h"
#include "audio_policy_server_handler.h"
#include "audio_zone_service.h"
#include "audio_interrupt_service.h"
#include "i_audio_zone_event_dispatcher.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneUnitTestBase : public testing::Test {
public:
    void SetUp(void) override
    {
        std::shared_ptr<AudioPolicyServerHandler> handler = std::make_shared<AudioPolicyServerHandler>();
        EXPECT_NE(handler, nullptr);
        std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
        EXPECT_NE(interruptService, nullptr);
        AudioZoneService::GetInstance().Init(handler, interruptService);
    }

    void TearDown(void) override
    {
        AudioZoneService::GetInstance().DeInit();
    }
};

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

sptr<AudioZoneUnitTestClient> RegisterTestClient(pid_t clientPid)
{
    sptr<AudioZoneUnitTestClient> client = new AudioZoneUnitTestClient();
    EXPECT_NE(client, nullptr);
    AudioZoneClientManager::GetInstance().RegisterAudioZoneClient(clientPid, client);
    return client;
}

#define TEST_PID_1000 1000
#define TEST_PID_2000 2000

class AudioZoneUnitTestPreset : public AudioZoneUnitTestBase {
public:
    void SetUp() override
    {
        AudioZoneUnitTestBase::SetUp();
        client1000_ = RegisterTestClient(TEST_PID_1000);
        client2000_ = RegisterTestClient(TEST_PID_2000);
        EXPECT_NE(client1000_, nullptr);
        EXPECT_NE(client2000_, nullptr);

        AudioZoneContext context;
        auto zoneId1_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone1", context);
        EXPECT_NE(zoneId1_, 0);
        auto zoneId2_ = AudioZoneService::GetInstance().CreateAudioZone("TestZone2", context);
        EXPECT_NE(zoneId2_, 0);
    }

    void TearDown() override
    {
        zoneId1_ = 0;
        zoneId2_ = 0;
        client1000_ = nullptr;
        client2000_ = nullptr;
        AudioZoneUnitTestBase::TearDown();
    }

    int32_t zoneId1_ = 0;
    int32_t zoneId2_ = 0;
    sptr<AudioZoneUnitTestClient> client1000_;
    sptr<AudioZoneUnitTestClient> client2000_;
};

std::shared_ptr<AudioDeviceDescriptor> CreateDevice(DeviceType type, DeviceRole role,
    const std::string &macAddress, const std::string &networkId)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(type, role);
    EXPECT_NE(desc, nullptr);
    desc->macAddress_ = macAddress;
    desc->networkId_ = networkId;
    return desc;
}
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ZONE_UNIT_TEST_BASE_H