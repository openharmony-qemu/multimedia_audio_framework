/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_VOLUME_UTILS_H
#define AUDIO_VOLUME_UTILS_H

#include <list>
#include <string>
#include <mutex>

#include "audio_log.h"
#include "audio_errors.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_utils.h"
#include "audio_volume_config.h"
#include "audio_volume_parser.h"

namespace OHOS {
namespace AudioStandard {

static const std::vector<AudioStreamType> DISTRIBUTED_VOLUME_TYPE_LIST = {
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_MUSIC
};

static const std::vector<AudioStreamType> BASE_VOLUME_TYPE_LIST = {
    // all volume types except STREAM_ALL
    STREAM_RING,
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_ACCESSIBILITY,
    STREAM_ULTRASONIC,
    STREAM_MUSIC
};

static const std::vector<AudioStreamType> PC_VOLUME_TYPE_LIST = {
    // all volume types except STREAM_ALL
    STREAM_RING,
    STREAM_VOICE_CALL,
    STREAM_VOICE_ASSISTANT,
    STREAM_ALARM,
    STREAM_ACCESSIBILITY,
    STREAM_SYSTEM,
    STREAM_ULTRASONIC,
    // adjust the type of music from the head of list to end, make sure music is updated last.
    // avoid interference from ring updates on special platform.
    // when the device is switched to headset,ring and alarm is dualtone type.
    // dualtone type use fixed volume curve of speaker.
    // the ring and alarm are classified into the music group.
    // the music volume becomes abnormal when the db value of music is modified.
    STREAM_MUSIC
};

static const std::unordered_map<DeviceType, DeviceVolumeType> DEVICE_TYPE_TO_DEVICE_VOLUME_TYPE_MAP = {
    {DEVICE_TYPE_EARPIECE, EARPIECE_VOLUME_TYPE},
    {DEVICE_TYPE_SPEAKER, SPEAKER_VOLUME_TYPE},
    {DEVICE_TYPE_WIRED_HEADSET, HEADSET_VOLUME_TYPE}
};

class AudioVolumeUtils {
public:
    static AudioVolumeUtils &GetInstance();

    bool LoadConfig();

    int32_t GetDefaultVolumeLevel(const std::shared_ptr<AudioDeviceDescriptor> &desc, AudioStreamType streamType);
    void GetDefaultVolumeLevelFromConfig(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        AudioStreamType streamType, int32_t &volumeLevel);
    void GetDefaultVolumeLevelForDPsDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        AudioStreamType streamType, int32_t &volumeLevel);
    void GetDefaultVolumeLevelForDistributedDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        AudioStreamType streamType, int32_t &volumeLevel);
    void GetDefaultVolumeLevelForHearingAidDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        AudioStreamType streamType, int32_t &volumeLevel);
    
    int32_t GetMaxVolumeLevel(const std::shared_ptr<AudioDeviceDescriptor> &desc, AudioStreamType streamType);
    void GetMaxVolumeLevelFromConfig(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        AudioStreamType streamType, int32_t &volumeLevel);
    
    int32_t GetMinVolumeLevel(const std::shared_ptr<AudioDeviceDescriptor> &desc, AudioStreamType streamType);
    void GetMinVolumeLevelFromConfig(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        AudioStreamType streamType, int32_t &volumeLevel);
    
    bool IsDistributedDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc);
    bool IsDeviceWithSafeVolume(const std::shared_ptr<AudioDeviceDescriptor> &desc);

private:
    AudioVolumeUtils() {};
    ~AudioVolumeUtils() {};
    std::map<AudioVolumeType, std::shared_ptr<StreamVolumeInfo>> streamVolumeInfos_;
};
}
}
#endif
