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

#ifndef AUDIO_ZONE_CLIENT_MANAGER_UNIT_TEST_H
#define AUDIO_ZONE_CLIENT_MANAGER_UNIT_TEST_H
 
#include "gtest/gtest.h"
#include "audio_zone_client_manager.h"
 
namespace OHOS {
namespace AudioStandard {
 
class AudioZoneClientManagerUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

class IStandardAudioZoneClientUnitTest : public IStandardAudioZoneClient {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    ErrCode OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_ADD_EVENT;
        Notify();
        return 0;
    }

    ErrCode OnAudioZoneRemove(int32_t zoneId) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT;
        recvEvent_.zoneId = zoneId;
        Notify();
        return 0;
    }

    ErrCode OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor& zoneDescriptor,
        int32_t reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_CHANGE_EVENT;
        recvEvent_.zoneId = zoneId;
        Notify();
        return 0;
    }

    ErrCode OnInterruptEvent(int32_t zoneId,
        const std::vector<std::map<AudioInterrupt, int32_t>>& ipcInterrupts,
        int32_t reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
        recvEvent_.zoneId = zoneId;
        Notify();
        return 0;
    }

    ErrCode OnInterruptEvent(int32_t zoneId, const std::string& deviceTag,
        const std::vector<std::map<AudioInterrupt, int32_t>>& ipcInterrupts,
        int32_t reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
        recvEvent_.zoneId = zoneId;
        recvEvent_.deviceTag = deviceTag;
        Notify();
        return 0;
    }

    ErrCode SetSystemVolume(int32_t zoneId, int32_t deviceType, int32_t volumeType, int32_t volumeLevel,
        int32_t volumeDegree, int32_t volumeFlag) override
    {
        (void)deviceType;
        (void)volumeType;
        volumeLevel_ = volumeLevel;
        volumeDegree_ = volumeDegree;
        Notify();
        return 0;
    }

    ErrCode GetSystemVolume(int32_t zoneId, int32_t deviceType, int32_t volumeType, int32_t& outVolume) override
    {
        (void)deviceType;
        (void)volumeType;
        outVolume = volumeLevel_;
        Notify();
        return 0;
    }

    ErrCode GetSystemVolumeDegree(int32_t zoneId, int32_t deviceType, int32_t volumeType, int32_t &outVolume) override
    {
        (void)deviceType;
        (void)volumeType;
        outVolume = volumeDegree_;
        Notify();
        return 0;
    }

    void Notify()
    {
        std::unique_lock<std::mutex> lock(waitLock_);
        waitStatus_ = 1;
        waiter_.notify_one();
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
    int32_t volumeDegree_ = -1;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_ZONE_CLIENT_MANAGER_UNIT_TEST_H