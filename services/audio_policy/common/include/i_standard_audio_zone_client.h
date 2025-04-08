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

#ifndef I_STANDARD_AUDIO_ZONE_CLIENT_H
#define I_STANDARD_AUDIO_ZONE_CLIENT_H

#include <list>
#include <utility>
#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "audio_info.h"
#include "audio_zone_info.h"
#include "audio_interrupt_info.h"

namespace OHOS {
namespace AudioStandard {
enum class AudioZoneClientCode {
    ON_AUDIO_ZONE_ADD = 0,
    ON_AUDIO_ZONE_REMOVE,
    ON_AUDIO_ZONE_CHANGE,
    ON_AUDIO_ZONE_INTERRUPT,
    ON_AUDIO_ZONE_DEVICE_INTERRUPT,
    ON_AUDIO_ZONE_SYSTEM_VOLUME_SET,
    ON_AUDIO_ZONE_SYSTEM_VOLUME_GET,
};

class IStandardAudioZoneClient : public IRemoteBroker {
public:
    virtual ~IStandardAudioZoneClient() = default;
    
    virtual void OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) = 0;

    virtual void OnAudioZoneRemove(int32_t zoneId) = 0;

    virtual void OnAudioZoneChange(int32_t zoneId, const AudioZoneDescriptor &zoneDescriptor,
        AudioZoneChangeReason reason) = 0;

    virtual void OnInterruptEvent(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) = 0;

    virtual void OnInterruptEvent(int32_t zoneId, int32_t deviceId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) = 0;

    virtual int32_t SetSystemVolume(const int32_t zoneId, const AudioVolumeType volumeType,
        const int32_t volumeLevel, const int32_t volumeFlag) = 0;

    virtual int32_t GetSystemVolume(int32_t zoneId, AudioVolumeType volumeType) = 0;

    bool hasBTPermission_ = true;
    bool hasSystemPermission_ = true;

    DECLARE_INTERFACE_DESCRIPTOR(u"IStandardAudioZoneClient");
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // I_STANDARD_AUDIO_ZONE_CLIENT_H