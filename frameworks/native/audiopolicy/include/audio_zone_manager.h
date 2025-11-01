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
    virtual void SetSystemVolumeDegree(AudioVolumeType volumeType, int32_t volumeDegree) {}
    virtual int32_t GetSystemVolumeDegree(AudioVolumeType volumeType) { return -1;}
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

    virtual const std::vector<std::shared_ptr<AudioZoneDescriptor>> GetAllAudioZone() = 0;

    virtual const std::shared_ptr<AudioZoneDescriptor> GetAudioZone(int32_t zoneId) = 0;

    virtual int32_t GetAudioZoneByName(std::string name) = 0;

    virtual int32_t BindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices) = 0;

    virtual int32_t UnBindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices) = 0;

    virtual int32_t RegisterAudioZoneCallback(const std::shared_ptr<AudioZoneCallback> &callback) = 0;

    virtual int32_t UnRegisterAudioZoneCallback() = 0;

    virtual int32_t RegisterAudioZoneChangeCallback(int32_t zoneId,
        const std::shared_ptr<AudioZoneChangeCallback> &callback) = 0;
    
    virtual int32_t UnRegisterAudioZoneChangeCallback(int32_t zoneId) = 0;

    virtual int32_t AddUidToAudioZone(int32_t zoneId, int32_t uid) = 0;

    virtual int32_t RemoveUidFromAudioZone(int32_t zoneId, int32_t uid) = 0;

    virtual int32_t AddStreamToAudioZone(int32_t zoneId, AudioZoneStream stream) = 0;

    virtual int32_t AddStreamsToAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams) = 0;

    virtual int32_t RemoveStreamFromAudioZone(int32_t zoneId, AudioZoneStream stream) = 0;
    
    virtual int32_t RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams) = 0;

    virtual void SetZoneDeviceVisible(bool visible) = 0;

    virtual int32_t RegisterSystemVolumeProxy(int32_t zoneId,
        const std::shared_ptr<AudioZoneVolumeProxy> &proxy) = 0;

    virtual int32_t UnRegisterSystemVolumeProxy(int32_t zoneId) = 0;

    virtual std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(
        int32_t zoneId) = 0;
    
    virtual std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(
        int32_t zoneId, const std::string &deviceTag) = 0;
    
    virtual int32_t RegisterAudioZoneInterruptCallback(int32_t zoneId,
        const std::shared_ptr<AudioZoneInterruptCallback> &callback) = 0;
    
    virtual int32_t UnRegisterAudioZoneInterruptCallback(int32_t zoneId) = 0;

    virtual int32_t RegisterAudioZoneInterruptCallback(int32_t zoneId, const std::string &deviceTag,
        const std::shared_ptr<AudioZoneInterruptCallback> &callback) = 0;
    
    virtual int32_t UnRegisterAudioZoneInterruptCallback(int32_t zoneId,
        const std::string &deviceTag) = 0;
    
    virtual int32_t InjectInterruptToAudioZone(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts) = 0;
    
    virtual int32_t InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ZONE_MANAGER_H