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

#ifndef ST_AUDIO_ZONE_MANAGER_H
#define ST_AUDIO_ZONE_MANAGER_H

#include <vector>
#include <list>
#include <utility>
#include "audio_info.h"
#include "audio_interrupt_info.h"
#include "audio_system_manager.h"
#include "audio_zone_info.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneCallback {
public:
    virtual ~AudioZoneCallback() = default;

    virtual void OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) = 0;

    virtual void OnAudioZoneRemove(int32_t zoneId) = 0;
};

class AudioZoneChangeCallback {
public:
    virtual ~AudioZoneChangeCallback() = default;

    virtual void OnAudioZoneChange(const AudioZoneDescriptor &zoneDescriptor, AudioZoneChangeReason reason) = 0;
};

class AudioZoneVolumeProxy {
public:
    virtual ~AudioZoneVolumeProxy() = default;

    virtual void SetSystemVolume(const AudioVolumeType volumeType, const int32_t volumeLevel) = 0;
    virtual int32_t GetSystemVolume(AudioVolumeType volumeType) = 0;
};

class AudioZoneInterruptCallback {
public:
    virtual ~AudioZoneInterruptCallback() = default;

    virtual void OnInterruptEvent(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) = 0;
};

class AudioZoneManager {
public:
    AudioZoneManager() = default;
    virtual ~AudioZoneManager() = default;

    static AudioZoneManager *GetInstance();

    virtual int32_t CreateAudioZone(const std::string &name, const AudioZoneContext &context) = 0;

    virtual void ReleaseAudioZone(int32_t zoneId) = 0;

    virtual const std::vector<sptr<AudioZoneDescriptor>> GetAllAudioZone() = 0;

    virtual const sptr<AudioZoneDescriptor> GetAudioZone(int32_t zoneId) = 0;

    virtual int32_t BindDeviceToAudioZone(int32_t zoneId, std::vector<sptr<AudioDeviceDescriptor>> devices) = 0;

    virtual int32_t UnBindDeviceToAudioZone(int32_t zoneId, std::vector<sptr<AudioDeviceDescriptor>> devices) = 0;

    virtual int32_t RegisterAudioZoneCallback(const std::shared_ptr<AudioZoneCallback> &callback) = 0;

    virtual int32_t UnRegisterAudioZoneCallback() = 0;

    virtual int32_t RegisterAudioZoneChangeCallback(int32_t zoneId,
        const std::shared_ptr<AudioZoneChangeCallback> &callback) = 0;
    
    virtual int32_t UnRegisterAudioZoneChangeCallback(int32_t zoneId) = 0;

    virtual int32_t AddUidToAudioZone(int32_t zoneId, int32_t uid) = 0;

    virtual int32_t RemoveUidFromAudioZone(int32_t zoneId, int32_t uid) = 0;

    virtual int32_t RegisterSystemVolumeProxy(int32_t zoneId,
        const std::shared_ptr<AudioZoneVolumeProxy> &proxy) = 0;

    virtual int32_t UnRegisterSystemVolumeProxy(int32_t zoneId) = 0;

    virtual int32_t SetSystemVolumeLevelForZone(const int32_t zoneId, const AudioVolumeType volumeType,
        const int32_t volumeLevel, const int32_t volumeFlag = 0) = 0;
    
    virtual int32_t GetSystemVolumeLevelForZone(int32_t zoneId, AudioVolumeType volumeType) = 0;

    virtual std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(
        int32_t zoneId) = 0;
    
    virtual std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(
        int32_t zoneId, int32_t deviceId) = 0;
    
    virtual int32_t RegisterAudioZoneInterruptCallback(int32_t zoneId,
        const std::shared_ptr<AudioZoneInterruptCallback> &callback) = 0;
    
    virtual int32_t UnRegisterAudioZoneInterruptCallback(int32_t zoneId) = 0;

    virtual int32_t RegisterAudioZoneInterruptCallback(int32_t zoneId, int32_t deviceId,
        const std::shared_ptr<AudioZoneInterruptCallback> &callback) = 0;
    
    virtual int32_t UnRegisterAudioZoneInterruptCallback(int32_t zoneId,
        int32_t deviceId) = 0;
    
    virtual int32_t InjectInterruptToAudioZone(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts) = 0;
    
    virtual int32_t InjectInterruptToAudioZone(int32_t zoneId, int32_t deviceId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ZONE_MANAGER_H