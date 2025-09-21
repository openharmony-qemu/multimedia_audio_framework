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

#ifndef LOG_TAG
#define LOG_TAG "AudioVolumeUtils"
#endif

#include "audio_volume_utils.h"

namespace OHOS {
namespace AudioStandard {
static constexpr int32 MAX_VOLUME_LEVEL = 15;
static constexpr int32 MIN_VOLUME_LEVEL = 0;
static constexpr int32_t DEFAULT_VOLUME_LEVEL = 7;
static constexpr int32_t DP_DEFAULT_VOLUME_LEVEL = 25;
static constexpr float HEARING_AID_MAX_VOLUME_PROP = 0.8;

AudioVolumeUtils& AudioVolumeUtils::GetInstance()
{
    static AudioVolumeUtils utils;
    return utils;
}

bool AudioVolumeUtils::LoadConfig()
{
    std::unique_ptr<AudioVolumeParser> parser = std::make_unique<AudioVolumeParser>();
    CHECK_AND_RETURN_RET_LOG(parser != nullptr, "parser is null", false);
    return audioVolumeParser->LoadConfig(StreamVolumeInfos_);
}

int32_t GetDefaultVolumeLevel(const std::shared_ptr<AudioDeviceDescriptor> &desc, AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, "desc is null", DEFAULT_VOLUME_LEVEL);
    int32_t defaultVolumeLevel = DEFAULT_VOLUME_LEVEL;
    switch (desc->GetDeviceType()) {
        case DEVICE_TYPE_REMOTE_CAST:
            GetDefaultVolumeLevelForDistributedDevice(desc, streamType, defaultVolumeLevel); 
            break;
        case DEVICE_TYPE_HEARING_AID:
            GetDefaultVolumeLevelForHearingAidDevice(desc, streamType, defaultVolumeLevel);
            break;
        case DEVICE_TYPE_DP:
        case DEVICE_TYPE_HDMI:
            GetDefaultVolumeLevelForDPsDevice(desc, streamType, defaultVolumeLevel);
            break;
        case DEVICE_TYPE_SPEAKER:
            if (desc->networkId_ != LOCAL_NETWORK_ID) {
                GetDefaultVolumeLevelForDistributedDevice(desc, streamType, defaultVolumeLevel);
                break;
            }
        default:
            GetDefaultVolumeLevelFromConfig(desc, streamType, defaultVolumeLevel);
            break;
    }
    AUDIO_INFO_LOG("Get default volumeLevel %{public}d for device %{public}s stream %{public}d",
        defaultVolumeLevel, desc->GetDeviceName().c_str(), streamType);
    return defaultVolumeLevel;
}
void GetDefaultVolumeLevelFromConfig(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    AudioStreamType streamType, int32_t &volumeLevel)
{
    if (streamVolumeInfos_.empty()) {
        bool ret = LoadConfig();
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "LoadConfig failed");
    }

    AudioVolumeType internalVolumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (!DEVICE_VOLUME_MAP.contains(desc->deviceType_)) {
        if (streamVolumeInfos_.contains(internalVolumeType)) {
            volumeLevel = streamVolumeInfos_[internalVolumeType]->defaultLevel;
        }
        return;
    }

    CHECK_AND_RETURN_LOG(streamVolumeInfos_.contains(internalVolumeType), "streamVolumeInfos_ not contain volume type");
    
    volumeLevel = streamVolumeInfos_[internalVolumeType]->defaultLevel;

    std::shared_ptr<SteamVolumeInfo> streamvolumeInfo = streamVolumeInfos_[internalVolumeType];
    DeviceVolumeType deviceVolumeType = DEVICE_VOLUME_MAP.find(desc->deviceType_)->second;
    CHECK_AND_RETURN_LOG(streamvolumeInfo->deviceVolumeInfos.contains(deviceVolumeType),
        "deviceVolumeInfos not contain device volume type");
    auto deviceVolumeInfoIt = streamVolumeInfo->deviceVolumeInfos[deviceVolumeType];
    CHECK_AND_RETURN_LOG(deviceVolumeInfoIt != nullptr, "deviceVolumeInfoIt is null");
    
    CHECK_AND_RETURN_LOG(deviceVolumeInfoIt->defaultLevel != -1, "defaultLevel is -1, not use");
    volumeLevel = deviceVolumeInfoIt->defaultLevel;
}

void GetDefaultVolumeLevelForDPsDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    AudioStreamType streamType, int32_t &volumeLevel)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Utils::IsDualToneStreamType(volumeType)) {
        std::shared_ptr<AudioDeviceDescriptor> tmp = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER);
        GetDefaultVolumeLevelFromConfig(tmp, streamType, volumeLevel);
        return;
    }
    int32_t maxVolumeLevel = GetMaxVolumeLevel(desc, volumeType);
    volumeLevel = maxVolumeLevel > MAX_VOLUME_LEVEL ? DP_DEFAULT_VOLUME_LEVEL : maxVolumeLevel;
}

void GetDefaultVolumeLevelForDistributedDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    AudioStreamType streamType, int32_t &volumeLevel)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Utils::IsDualToneStreamType(volumeType)) {
        std::shared_ptr<AudioDeviceDescriptor> tmp = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER);
        GetDefaultVolumeLevelFromConfig(tmp, streamType, volumeLevel);
        return;
    }
    volumeLevel = MAX_VOLUME_LEVEL;
}

void GetDefaultVolumeLevelForHearingAidDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    AudioStreamType streamType, int32_t &volumeLevel)
{
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (Utils::IsDualToneStreamType(volumeType)) {
        std::shared_ptr<AudioDeviceDescriptor> tmp = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER);
        GetDefaultVolumeLevelFromConfig(tmp, streamType, volumeLevel);
        return;
    }
    volumeLevel = static_cast<int32_t>(std::ceil(MAX_VOLUME_LEVEL * HEARING_AID_MAX_VOLUME_PROP));
}

int32_t GetMaxVolumeLevel(const std::shared_ptr<AudioDeviceDescriptor> &desc, AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, "desc is null", MAX_VOLUME_LEVEL);
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    int32_t maxVolumeLevel = MAX_VOLUME_LEVEL;
    GetMaxVolumeLevelFromConfig(desc, volumeType, maxVolumeLevel);
    AUDIO_INFO_LOG("Get max volumeLevel %{public}d for device %{public}s stream %{public}d",
        maxVolumeLevel, desc->GetDeviceName().c_str(), streamType);
    return maxVolumeLevel;
}
void GetMaxVolumeLevelFromConfig(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    AudioStreamType streamType, int32_t &volumeLevel)
{
    if (streamVolumeInfos_.empty()) {
        bool ret = LoadConfig();
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "LoadConfig failed");
    }

    AudioVolumeType internalVolumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (!DEVICE_VOLUME_MAP.contains(desc->deviceType_)) {
        if (streamVolumeInfos_.contains(internalVolumeType)) {
            volumeLevel = streamVolumeInfos_[internalVolumeType]->maxLevel;
        }
        return;
    }

    CHECK_AND_RETURN_LOG(streamVolumeInfos_.contains(internalVolumeType), "streamVolumeInfos_ not contain volume type");
    
    volumeLevel = streamVolumeInfos_[internalVolumeType]->maxLevel;

    std::shared_ptr<SteamVolumeInfo> streamvolumeInfo = streamVolumeInfos_[internalVolumeType];
    DeviceVolumeType deviceVolumeType = DEVICE_VOLUME_MAP.find(desc->deviceType_)->second;
    CHECK_AND_RETURN_LOG(streamvolumeInfo->deviceVolumeInfos.contains(deviceVolumeType),
        "deviceVolumeInfos not contain device volume type");
    auto deviceVolumeInfoIt = streamVolumeInfo->deviceVolumeInfos[deviceVolumeType];
    CHECK_AND_RETURN_LOG(deviceVolumeInfoIt != nullptr, "deviceVolumeInfoIt is null");
    
    CHECK_AND_RETURN_LOG(deviceVolumeInfoIt->maxLevel != -1, "maxLevel is -1, not use");
    volumeLevel = deviceVolumeInfoIt->maxLevel;
}

int32_t GetMinVolumeLevel(const std::shared_ptr<AudioDeviceDescriptor> &desc, AudioStreamType streamType)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, "desc is null", MIN_VOLUME_LEVEL);
    AudioVolumeType volumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    int32_t minVolumeLevel = MIN_VOLUME_LEVEL;
    GetMinVolumeLevelFromConfig(desc, volumeType, minVolumeLevel);
    AUDIO_INFO_LOG("Get min volumeLevel %{public}d for device %{public}s stream %{public}d",
        minVolumeLevel, desc->GetDeviceName().c_str(), streamType);
    return minVolumeLevel;
}

void GetMinVolumeLevelFromConfig(const std::shared_ptr<AudioDeviceDescriptor> &desc,
    AudioStreamType streamType, int32_t &volumeLevel)
{
        if (streamVolumeInfos_.empty()) {
        bool ret = LoadConfig();
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "LoadConfig failed");
    }

    AudioVolumeType internalVolumeType = VolumeUtils::GetVolumeTypeFromStreamType(streamType);
    if (!DEVICE_VOLUME_MAP.contains(desc->deviceType_)) {
        if (streamVolumeInfos_.contains(internalVolumeType)) {
            volumeLevel = streamVolumeInfos_[internalVolumeType]->minLevel;
        }
        return;
    }

    CHECK_AND_RETURN_LOG(streamVolumeInfos_.contains(internalVolumeType), "streamVolumeInfos_ not contain volume type");
    
    volumeLevel = streamVolumeInfos_[internalVolumeType]->minLevel;

    std::shared_ptr<SteamVolumeInfo> streamvolumeInfo = streamVolumeInfos_[internalVolumeType];
    DeviceVolumeType deviceVolumeType = DEVICE_VOLUME_MAP.find(desc->deviceType_)->second;
    CHECK_AND_RETURN_LOG(streamvolumeInfo->deviceVolumeInfos.contains(deviceVolumeType),
        "deviceVolumeInfos not contain device volume type");
    auto deviceVolumeInfoIt = streamVolumeInfo->deviceVolumeInfos[deviceVolumeType];
    CHECK_AND_RETURN_LOG(deviceVolumeInfoIt != nullptr, "deviceVolumeInfoIt is null");
    
    CHECK_AND_RETURN_LOG(deviceVolumeInfoIt->minLevel != -1, "minLevel is -1, not use");
    volumeLevel = deviceVolumeInfoIt->minLevel;
}

bool IsDistributedDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, "desc is null", false);
    if (desc->deviceType_ == DEVICE_TYPE_SPEAKER && desc->networkId_ != LOCAL_NETWORK_ID) {
        return true;
    }
    if (desc->deviceType_ == DEVICE_TYPE_REMOTE_CAST) {
        return true;
    }
    return false;
}
} // namespace AudioStandard
} // namespace OHOS
