#ifndef LOG_TAG
#define LOG_TAG "AudioCollaborativeService"
#endif
#include <string.h>
#include "audio_collaborative_service.h"
namespace OHOS {
namespace AudioStandard {
static const std::string AUDIO_COLLABORATIVE_SERVICE_LABEL = "COLLABORATIVE";

void AudioCollaborativeService::Init(const std::vector<EffectChain> &effectChains)
{
    AUDIO_INFO_LOG("AudioCollaborative service initialized!");
    isCollaborativePlaybackSupported_ = false;
    // for (auto effectChain: effectChains) {
    //     if (effectChain.name != BLUETOOTH_EFFECT_CHAIN_NAME) { // only support bluebooth effectchain?
    //         continue;
    //     }
    //     if (effectChain.label == AUDIO_COLLABORATIVE_SERVICE_LABEL) {
    //         isCollaborativePlaybackSupported_ = true;
    //     }
    // }
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
    if (selectedAudioDevice.deviceType_ != DEVICE_TYPE_BLUETOOTH_A2DP) {
        AUDIO_INFO_LOG("Change to non-bluetooth_a2dp device, set collaborative disabled");
        if (isCollaborativeStateEnabled_) {
            isCollaborativeStateEnabled_ = false;
            audioPolicyManager_.UpdateCollaborativeState(isCollaborativeStateEnabled_);
        }
    }
    if (selectedAudioDevice.macAddress_ != curDeviceAddress_) {
        AUDIO_INFO_LOG("Update current device for AudioCollaborativeSerivce");
        curDeviceAddress_ = selectedAudioDevice.macAddress_;
        UpdateCollaborativeStateReal();
    }
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
        AUDIO_INFO_LOG("new device added, macaddress %{public}s", selectedAudioDevice->macAddress_.c_str());
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
        AUDIO_INFO_LOG("current device is not in addressToCollaborativeEnabledMap_, no need to update real state");
        return SUCCESS;
    }
    if (addressToCollaborativeEnabledMap_[curDeviceAddress_] != isCollaborativeStateEnabled_) {
        isCollaborativeStateEnabled_ = addressToCollaborativeEnabledMap_[curDeviceAddress_];
        AUDIO_INFO_LOG("current device collaborative enabled state changed to %{public}d", isCollaborativeStateEnabled_);
        int32_t ret = audioPolicyManager_.UpdateCollaborativeState(isCollaborativeStateEnabled_); // send to HpaeManager
        return ret;
    }
    AUDIO_INFO_LOG("current device state unchanged. No need to update current device collaborative state");
    return SUCCESS;
}

AudioCollaborativeService::~AudioCollaborativeService()
{
    AUDIO_ERR_LOG("~AudioCollaborativeService");
}



} // AudioStandard
} // OHOS