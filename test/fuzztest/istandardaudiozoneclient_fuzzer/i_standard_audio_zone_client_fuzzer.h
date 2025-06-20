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

#ifndef I_STANDARD_AUDIO_ZONE_CLIENT_FUZZER_H
#define I_STANDARD_AUDIO_ZONE_CLIENT_FUZZER_H

#include "i_standard_audio_zone_client.h"
#include "i_audio_zone_event_dispatcher.h"

namespace OHOS {
namespace AudioStandard {
class IStandardAudioZoneClientFuzz : public IStandardAudioZoneClient {
public:
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }

    void OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_ADD_EVENT;
        recvEvent_.zoneId = zoneDescriptor.zoneId_;
        Notify();
    }

    void OnAudioZoneRemove(int32_t zoneId) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_REMOVE_EVENT;
        recvEvent_.zoneId = zoneId;
        Notify();
    }

    void OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor &zoneDescriptor,
        AudioZoneChangeReason reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_CHANGE_EVENT;
        recvEvent_.zoneId = zoneId;
        recvEvent_.zoneChangeReason = reason;
        Notify();
    }

    void OnInterruptEvent(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
        recvEvent_.zoneId = zoneId;
        recvEvent_.interrupts = interrupts;
        recvEvent_.zoneInterruptReason = reason;
        Notify();
    }

    void OnInterruptEvent(int32_t zoneId, const std::string &deviceTag,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override
    {
        recvEvent_.type = AudioZoneEventType::AUDIO_ZONE_INTERRUPT_EVENT;
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
} // namespace AudioStandard
} // namesapce OHOS

#endif // I_STANDARD_AUDIO_ZONE_CLIENT_FUZZER_H