/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioA2dpDevice"
#endif

#include "audio_a2dp_device.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "audio_policy_manager_factory.h"

#include "audio_policy_utils.h"
#include "audio_policy_service.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static std::string GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

bool AudioA2dpDevice::GetA2dpDeviceInfo(const std::string& device, A2dpDeviceConfigInfo& info)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpDeviceMap_.end()) {
        info.streamInfo = configInfoPos->second.streamInfo;
        info.absVolumeSupport = configInfoPos->second.absVolumeSupport;
        info.volumeLevel = configInfoPos->second.volumeLevel;
        info.mute = configInfoPos->second.mute;
        return true;
    }
    return false;
}

bool AudioA2dpDevice::GetA2dpInDeviceInfo(const std::string& device, A2dpDeviceConfigInfo& info)
{
    std::lock_guard<std::mutex> lock(a2dpInDeviceMapMutex_);
    auto configInfoPos = connectedA2dpInDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpInDeviceMap_.end()) {
        info.streamInfo = configInfoPos->second.streamInfo;
        info.absVolumeSupport = configInfoPos->second.absVolumeSupport;
        info.volumeLevel = configInfoPos->second.volumeLevel;
        info.mute = configInfoPos->second.mute;
        return true;
    }
    return false;
}

bool AudioA2dpDevice::GetA2dpDeviceVolumeLevel(const std::string& device, int32_t& volumeLevel)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpDeviceMap_.end()) {
        volumeLevel = configInfoPos->second.volumeLevel;
        return true;
    }
    return false;
}

bool AudioA2dpDevice::CheckA2dpDeviceExist(const std::string& device)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpDeviceMap_.end()) {
        return true;
    }
    return false;
}

bool AudioA2dpDevice::SetA2dpDeviceMute(const std::string& device, bool mute)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos == connectedA2dpDeviceMap_.end() || !configInfoPos->second.absVolumeSupport) {
        AUDIO_WARNING_LOG("Set Mute failed for macAddress:[%{public}s]", GetEncryptAddr(device).c_str());
        return false;
    }
    configInfoPos->second.mute = mute;
    return true;
}

bool AudioA2dpDevice::GetA2dpDeviceMute(const std::string& device, bool& isMute)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos == connectedA2dpDeviceMap_.end() || !configInfoPos->second.absVolumeSupport) {
        return false;
    }
    isMute = configInfoPos->second.mute;
    return true;
}

void AudioA2dpDevice::SetA2dpDeviceStreamInfo(const std::string& device, const DeviceStreamInfo& streamInfo)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    connectedA2dpDeviceMap_[device].streamInfo = streamInfo;
}

void AudioA2dpDevice::AddA2dpDevice(const std::string& device, const A2dpDeviceConfigInfo& config)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    connectedA2dpDeviceMap_[device] = config;
}

void AudioA2dpDevice::AddA2dpInDevice(const std::string& device, const A2dpDeviceConfigInfo& config)
{
    std::lock_guard<std::mutex> lock(a2dpInDeviceMapMutex_);
    connectedA2dpInDeviceMap_[device] = config;
}

size_t AudioA2dpDevice::DelA2dpInDevice(const std::string& device)
{
    std::lock_guard<std::mutex> lock(a2dpInDeviceMapMutex_);
    connectedA2dpInDeviceMap_.erase(device);
    return connectedA2dpInDeviceMap_.size();
}

size_t AudioA2dpDevice::DelA2dpDevice(const std::string& device)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    connectedA2dpDeviceMap_.erase(device);
    return connectedA2dpDeviceMap_.size();
}

bool AudioA2dpDevice::SetA2dpDeviceAbsVolumeSupport(const std::string& device, const bool support,
    int32_t volume, bool mute)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos != connectedA2dpDeviceMap_.end()) {
        configInfoPos->second.absVolumeSupport = support;
        if (support && configInfoPos->second.volumeLevel == -1) {
            configInfoPos->second.volumeLevel = volume;
            configInfoPos->second.mute = mute;
        }
        return true;
    }
    return false;
}

bool AudioA2dpDevice::SetA2dpDeviceVolumeLevel(const std::string& device, const int32_t volumeLevel)
{
    std::lock_guard<std::mutex> lock(a2dpDeviceMapMutex_);
    auto configInfoPos = connectedA2dpDeviceMap_.find(device);
    if (configInfoPos == connectedA2dpDeviceMap_.end() || !configInfoPos->second.absVolumeSupport) {
        AUDIO_WARNING_LOG("Set VolumeLevel failed for macAddress:[%{public}s]", GetEncryptAddr(device).c_str());
        return false;
    }
    configInfoPos->second.volumeLevel = volumeLevel;
    return true;
}

void AudioA2dpDevice::AddHearingAidDevice(const std::string& device, const A2dpDeviceConfigInfo& config)
{
    std::lock_guard<std::mutex> lock(hearingAidDeviceMapMutex_);
    connectedHearingAidDeviceMap_.insert_or_assign(device, config);
}

size_t AudioA2dpDevice::DelHearingAidDevice(const std::string& device)
{
    std::lock_guard<std::mutex> lock(hearingAidDeviceMapMutex_);
    connectedHearingAidDeviceMap_.erase(device);
    return connectedHearingAidDeviceMap_.size();
}

bool AudioA2dpDevice::CheckHearingAidDeviceExist(const std::string& device)
{
    std::lock_guard<std::mutex> lock(hearingAidDeviceMapMutex_);
    auto configInfoPos = connectedHearingAidDeviceMap_.find(device);
    return configInfoPos != connectedHearingAidDeviceMap_.end();
}

}
}