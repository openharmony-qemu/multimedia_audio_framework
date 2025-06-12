#ifndef LOG_TAG
#define LOG_TAG "AudioCollaborativeService"
#endif
#include <string.h>
#include "audio_collaborative_service.h"
namespace OHOS {
namespace AudioStandard {
static const std::string AUDIO_COLLABORATIVE_SERVICE_LABEL = "COLLABORATIVE";
static constexpr uint32_t ENCRYPTED_ADDRESS_LENGTH = 6;
static constexpr uint32_t ENCRYPTED_ADDRESS_LENGTH_HALF = 3;
static constexpr std::string ENCRYPTED_ADDRESS_CONTENT = "***";
static inline std::string EncryptMacAddress(const std::string &macAdress)
{
    CHECK_AND_RETURN_RET(macAdress.size() > ENCRYPTED_ADDRESS_LENGTH, "");
    std::string encryptedMacAddress = macAdress.substr(0, ENCRYPTED_ADDRESS_LENGTH_HALF) +
        ENCRYPTED_ADDRESS_CONTENT +
        macAdress.substr(macAdress.size() - ENCRYPTED_ADDRESS_LENGTH_HALF, ENCRYPTED_ADDRESS_LENGTH_HALF);
    return encryptedMacAddress;
}

void AudioCollaborativeService::Init(const std::vector<EffectChain> &effectChains)
{
    AUDIO_INFO_LOG("AudioCollaborative service initialized!");
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
    // add: check device type: only bluetooth_a2dp
    AUDIO_INFO_LOG("UpdateCurrentDevice Entered!");
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    
    if (selectedAudioDevice.macAddress_ != curDeviceAddress_) {
        AUDIO_INFO_LOG("Update current device macAddress %{public}s for AudioCollaborativeSerivce",
            EncryptMacAddress(curDeviceAddress_).c_str());
        curDeviceAddress_ = selectedAudioDevice.macAddress_;
    }
    // current device is not A2DP but already in map. May change from A2DP to SCO
    // remember enable state for the address temporarily in memory map
    if ((curDeviceAddress_ != DEVICE_TYPE_BLUETOOTH_A2DP) &&
        addressToCollaborativeEnabledMap_.find(curDeviceAddress_) != addressToCollaborativeEnabledMap_.end()) {
        addressToCollaborativeMemoryMap_[curDeviceAddress_] = addressToCollaborativeEnabledMap_[curDeviceAddress_];
        addressToCollaborativeEnabledMap_.erase(curDeviceAddress_);
    }
    // current device is A2DP but not in map, may be remembered in memory map, put it back to enable map
    if ((curDeviceAddress_ == DEVICE_TYPE_BLUETOOTH_A2DP) &&
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
    if (addressToCollaborativeEnabledMap_.find(selectedAudioDevice->macAddress_) != addressToCollaborativeEnabledMap_.end()) {
        AUDIO_INFO_LOG("selected device address %{public}s is in addressToCollaborativeEnabledMap_, state %{public}d",
            EncryptMacAddress(selectedAudioDevice->macAddress_).c_str(),
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
            AUDIO_INFO_LOG("current device %{public}s is not in addressToCollaborativeEnabledMap_, close collaborative service",
                EncryptMacAddress(curDeviceAddress_).c_str());
            return audioPolicyManager_.UpdateCollaborativeState(isCollaborativeStateEnabled_);
        }
        return SUCCESS;
    }
    if (addressToCollaborativeEnabledMap_[curDeviceAddress_] != isCollaborativeStateEnabled_) {
        isCollaborativeStateEnabled_ = addressToCollaborativeEnabledMap_[curDeviceAddress_];
        AUDIO_INFO_LOG("current collaborative enabled state changed to %{public}d for Mac address %{public}s",
            isCollaborativeStateEnabled_, EncryptMacAddress(curDeviceAddress_).c_str());
        return audioPolicyManager_.UpdateCollaborativeState(isCollaborativeStateEnabled_); // send to HpaeManager
    }
    AUDIO_INFO_LOG("No need to real collaborative state: %{public}d", isCollaborativeStateEnabled_);
    return SUCCESS;
}

AudioCollaborativeService::~AudioCollaborativeService()
{
    AUDIO_ERR_LOG("~AudioCollaborativeService");
}


} // AudioStandard
} // OHOS