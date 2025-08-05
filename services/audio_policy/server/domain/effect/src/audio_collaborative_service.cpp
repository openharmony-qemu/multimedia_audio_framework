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
#ifndef LOG_TAG
#define LOG_TAG "AudioCollaborativeService"
#endif
#include <cstring>
#include "audio_collaborative_service.h"

namespace OHOS {
namespace AudioStandard {
static const std::string AUDIO_COLLABORATIVE_SERVICE_LABEL = "COLLABORATIVE";
static const std::string BLUETOOTH_EFFECT_CHAIN_NAME = "EFFECTCHAIN_COLLABORATIVE";
const int ADDRESS_STR_LEN = 17;
const int START_POS = 6;
const int END_POS = 13;

static std::string GetEncryptAddr(const std::string &addr)
{
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

void AudioCollaborativeService::Init(const std::vector<EffectChain> &effectChains)
{
    AUDIO_INFO_LOG("AudioCollaborative service initialized!");
    isCollaborativePlaybackSupported_ = false;
    for (auto effectChain: effectChains) {
        if (effectChain.name != BLUETOOTH_EFFECT_CHAIN_NAME) { // only support bluebooth effectchain?
            continue;
        }
        if (effectChain.label == AUDIO_COLLABORATIVE_SERVICE_LABEL) {
            isCollaborativePlaybackSupported_ = true;
        }
    }
}

bool AudioCollaborativeService::IsCollaborativePlaybackSupported()
{
    return isCollaborativePlaybackSupported_;
}

void AudioCollaborativeService::UpdateCurrentDevice(const AudioDeviceDescriptor &selectedAudioDevice)
{
    AUDIO_INFO_LOG("UpdateCurrentDevice Entered!");
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    
    if (selectedAudioDevice.macAddress_ != curDeviceAddress_) {
        curDeviceAddress_ = selectedAudioDevice.macAddress_;
        AUDIO_INFO_LOG("Update current device macAddress %{public}s for AudioCollaborativeSerivce",
            GetEncryptAddr(curDeviceAddress_).c_str());
    }
    // current device is not A2DP but already in map. May change from A2DP to SCO
    // remember enable state for the address temporarily in memory map
    if ((selectedAudioDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) &&
        addressToCollaborativeEnabledMap_.find(curDeviceAddress_) != addressToCollaborativeEnabledMap_.end()) {
        addressToCollaborativeMemoryMap_[curDeviceAddress_] = addressToCollaborativeEnabledMap_[curDeviceAddress_];
        addressToCollaborativeEnabledMap_.erase(curDeviceAddress_);
    }
    // current device is A2DP but not in map, may be remembered in memory map, put it back to enable map
    if ((selectedAudioDevice.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) &&
        addressToCollaborativeEnabledMap_.find(curDeviceAddress_) == addressToCollaborativeEnabledMap_.end() &&
        addressToCollaborativeMemoryMap_.find(curDeviceAddress_) != addressToCollaborativeMemoryMap_.end()) {
        addressToCollaborativeEnabledMap_[curDeviceAddress_] = addressToCollaborativeMemoryMap_[curDeviceAddress_];
        addressToCollaborativeMemoryMap_.erase(curDeviceAddress_);
    }
    UpdateCollaborativeStateReal();
}

int32_t AudioCollaborativeService::SetCollaborativePlaybackEnabledForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled)
{
    AUDIO_INFO_LOG("SetCollaborativePlaybackEnabledForDevice Entered!");
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    std::string deviceAddress = selectedAudioDevice->macAddress_;
    AUDIO_INFO_LOG("Device Collaborative Enabled should be set to: %{public}d", enabled);
    addressToCollaborativeEnabledMap_[deviceAddress] = enabled;
    return UpdateCollaborativeStateReal();
}

bool AudioCollaborativeService::IsCollaborativePlaybackEnabledForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    AUDIO_INFO_LOG("isCollaborativePlaybackEnabledForDevice Entered!");
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    if (addressToCollaborativeEnabledMap_.find(selectedAudioDevice->macAddress_) !=
        addressToCollaborativeEnabledMap_.end()) {
        AUDIO_INFO_LOG("selected device address %{public}s is in addressToCollaborativeEnabledMap_, state %{public}d",
            GetEncryptAddr(selectedAudioDevice->macAddress_).c_str(),
            addressToCollaborativeEnabledMap_[selectedAudioDevice->macAddress_]);
        return addressToCollaborativeEnabledMap_[selectedAudioDevice->macAddress_];
    }
    AUDIO_INFO_LOG("address %{public}s is not in map", selectedAudioDevice->macAddress_.c_str());
    return false;
}

int32_t AudioCollaborativeService::UpdateCollaborativeStateReal()
{
    if (!isCollaborativePlaybackSupported_) {
        AUDIO_INFO_LOG("Local device does not support collaborative service!");
        return ERROR;
    }
    if (addressToCollaborativeEnabledMap_.find(curDeviceAddress_) == addressToCollaborativeEnabledMap_.end()) {
        if (isCollaborativeStateEnabled_) {
            isCollaborativeStateEnabled_ = false;
            AUDIO_INFO_LOG("current device %{public}s is not in addressToCollaborativeEnabledMap_, "
                "close collaborative service", GetEncryptAddr(curDeviceAddress_).c_str());
            return audioPolicyManager_.UpdateCollaborativeState(isCollaborativeStateEnabled_);
        }
        return SUCCESS;
    }
    if (addressToCollaborativeEnabledMap_[curDeviceAddress_] != isCollaborativeStateEnabled_) {
        isCollaborativeStateEnabled_ = addressToCollaborativeEnabledMap_[curDeviceAddress_];
        AUDIO_INFO_LOG("current collaborative enabled state changed to %{public}d for Mac address %{public}s",
            isCollaborativeStateEnabled_, GetEncryptAddr(curDeviceAddress_).c_str());
        return audioPolicyManager_.UpdateCollaborativeState(isCollaborativeStateEnabled_); // send to HpaeManager
    }
    AUDIO_INFO_LOG("No need to real collaborative state: %{public}d", isCollaborativeStateEnabled_);
    return SUCCESS;
}

bool AudioCollaborativeService::GetRealCollaborativeState()
{
    AUDIO_INFO_LOG("GetRealCollaborativeState Entered!");
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    return isCollaborativeStateEnabled_;
}

AudioCollaborativeService::~AudioCollaborativeService()
{
    AUDIO_ERR_LOG("~AudioCollaborativeService");
}
} // AudioStandard
} // OHOS