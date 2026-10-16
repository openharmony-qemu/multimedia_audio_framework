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
#include "media_monitor_manager.h"
#include "audio_core_service.h"
#include "manager/hdi_adapter_manager.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
namespace OHOS {
namespace AudioStandard {
static const std::string AUDIO_COLLABORATIVE_SERVICE_LABEL = "COLLABORATIVE";
static const std::string BLUETOOTH_EFFECT_CHAIN_NAME = "EFFECTCHAIN_COLLABORATIVE";
const int ADDRESS_STR_LEN = 17;
const int START_POS = 6;
const int END_POS = 13;
const int SLE_FLAG = 1;
static constexpr int32_t SET_COLLABORATIVE_PLAYBACK_ENABLED_FOR_DEVICE_TIMEOUT = 15;
static constexpr int32_t COLLABORATIVE_STATE_CHANGE_MUTE_SINK_PORT_US = 200000; // 200ms

static bool IsCollaborativePlaybackSupportedDeviceType(DeviceType deviceType)
{
    return deviceType == DEVICE_TYPE_BLUETOOTH_A2DP || deviceType == DEVICE_TYPE_NEARLINK;
}

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
    RecoverCollaborativeState();
    UpdateCollaborativeStateReal();
    LoadCollaborationConfig();
}

bool AudioCollaborativeService::IsCollaborativePlaybackSupported()
{
    return isCollaborativePlaybackSupported_;
}

bool AudioCollaborativeService::IsCollaborativePlaybackSupportedForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    if (!isCollaborativePlaybackSupported_ || selectedAudioDevice == nullptr) {
        return false;
    }

    std::string productId;
    int ret = -1;
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    if (selectedAudioDevice->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
#ifdef BLUETOOTH_ENABLE
        Bluetooth::BluetoothRemoteDevice device = Bluetooth::BluetoothRemoteDevice(selectedAudioDevice->macAddress_);
        ret = device.GetDeviceProductId(productId);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("DEVICE_TYPE_BLUETOOTH_A2DP GetDeviceProductId fail:%{public}d", ret);
            return false;
        }
#endif
    } else if (selectedAudioDevice->deviceType_ == DEVICE_TYPE_NEARLINK) {
        ret = SleAudioDeviceManager::GetInstance().GetDeviceProductId(selectedAudioDevice->macAddress_, productId);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("DEVICE_TYPE_NEARLINK GetDeviceProductId fail:%{public}d", ret);
            return false;
        }
        productId += "SLE";
    } else {
        AUDIO_INFO_LOG("Not support deviceType:%{public}d", selectedAudioDevice->deviceType_);
        return false;
    }

    ret = audioPolicyManager_.UpdateCollaborativeProductId(productId);
    if (ret != SUCCESS) {
        AUDIO_INFO_LOG("Not support deviceType:%{public}d productId: %{public}s update ret=%{public}d",
            selectedAudioDevice->deviceType_, productId.c_str(), ret);
        return false;
    }
    return true;
}

void AudioCollaborativeService::UpdateCurrentDevice(const AudioDeviceDescriptor &selectedAudioDevice)
{
    AUDIO_INFO_LOG("UpdateCurrentDevice Entered, seletedAudioDevice.deviceType_: %{public}d",
        selectedAudioDevice.deviceType_);
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    if (Util::IsEnhancedRoutingSupported()) {
        if (!(selectedAudioDevice.deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO || selectedAudioDevice.deviceType_ ==
            DEVICE_TYPE_BLUETOOTH_A2DP || selectedAudioDevice.deviceType_ == DEVICE_TYPE_NEARLINK)) {
            return;
        }
    }
    
    if (selectedAudioDevice.macAddress_ != curDeviceAddress_) {
        curDeviceAddress_ = selectedAudioDevice.macAddress_;
        AUDIO_INFO_LOG("Update current device macAddress %{public}s for AudioCollaborativeService",
            GetEncryptAddr(curDeviceAddress_).c_str());
    }

    // Update current device type
    curDeviceType_ = selectedAudioDevice.deviceType_;
    // current device is not collaborative playback devcie type but already in map and opened.
    // May change from A2DP/NearLink to other type.
    // collaborative closed when SCO but reserved, will recover to opened later
    if ((!IsCollaborativePlaybackSupportedDeviceType(selectedAudioDevice.deviceType_)) &&
        addressToCollaborativeEnabledMap_.find(curDeviceAddress_) != addressToCollaborativeEnabledMap_.end() &&
        addressToCollaborativeEnabledMap_[curDeviceAddress_] == COLLABORATIVE_OPENED) {
        addressToCollaborativeEnabledMap_[curDeviceAddress_] = COLLABORATIVE_RESERVED;
        WriteCollaborativeStateSysEvents(curDeviceAddress_, addressToCollaborativeEnabledMap_[curDeviceAddress_]);
    }
    // current device is collaborative playback devcie type and reserved state.
    // which comes back from other type. shoud be opened.
    if ((!IsCollaborativePlaybackSupportedDeviceType(selectedAudioDevice.deviceType_)) &&
        addressToCollaborativeEnabledMap_.find(curDeviceAddress_) != addressToCollaborativeEnabledMap_.end() &&
        addressToCollaborativeEnabledMap_[curDeviceAddress_] == COLLABORATIVE_RESERVED) {
        addressToCollaborativeEnabledMap_[curDeviceAddress_] = COLLABORATIVE_OPENED;
        WriteCollaborativeStateSysEvents(curDeviceAddress_, addressToCollaborativeEnabledMap_[curDeviceAddress_]);
    }
    UpdateCollaborativeStateReal();
}

int32_t AudioCollaborativeService::SetCollaborativePlaybackEnabledForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled)
{
    AUDIO_INFO_LOG("SetCollaborativePlaybackEnabledForDevice Entered!");
    int32_t ret;
    {
        std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
        std::string deviceAddress = selectedAudioDevice->macAddress_;
        AUDIO_INFO_LOG("Device Collaborative Enabled should be set to: %{public}d", enabled);
        addressToCollaborativeEnabledMap_[deviceAddress] = enabled ? COLLABORATIVE_OPENED : COLLABORATIVE_CLOSED;
        WriteCollaborativeStateSysEvents(deviceAddress, addressToCollaborativeEnabledMap_[deviceAddress]);
        ret = UpdateCollaborativeStateReal();
    }
    
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "UpdateCollaborativeStateReal failed!");
    AudioXCollie AudioXCollie("AudioCollaborativeService::SetCollaborativePlaybackEnabledForDevice",
        SET_COLLABORATIVE_PLAYBACK_ENABLED_FOR_DEVICE_TIMEOUT,
        [](void *) {
            AUDIO_ERR_LOG("SetCollaborativePlaybackEnabledForDevice timeout");
        }, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    CHECK_AND_RETURN_RET_LOG(AudioCoreService::GetCoreService() != nullptr &&
        AudioCoreService::GetCoreService()->GetEventEntry() != nullptr,  ERR_OPERATION_FAILED, "nullptr");
    if (selectedAudioDevice->IsA2dpOffload() && enabled && selectedAudioDevice->macAddress_ == curDeviceAddress_) {
        AudioCoreService::GetCoreService()->GetEventEntry()->MutePrimarySinkByCollaborative(
            COLLABORATIVE_STATE_CHANGE_MUTE_SINK_PORT_US);
    }
    AudioCoreService::GetCoreService()->GetEventEntry()->
        FetchOutputDeviceAndRoute("SetCollaborativePlaybackEnabledForDevice");
    return SUCCESS;
}

bool AudioCollaborativeService::IsCollaborativePlaybackEnabledForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    AUDIO_DEBUG_LOG("isCollaborativePlaybackEnabledForDevice Entered!");
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    if (addressToCollaborativeEnabledMap_.find(selectedAudioDevice->macAddress_) !=
        addressToCollaborativeEnabledMap_.end()) {
        AUDIO_INFO_LOG("selected device address %{public}s is in addressToCollaborativeEnabledMap_, state %{public}d",
            GetEncryptAddr(selectedAudioDevice->macAddress_).c_str(),
            addressToCollaborativeEnabledMap_[selectedAudioDevice->macAddress_]);
        return addressToCollaborativeEnabledMap_[selectedAudioDevice->macAddress_] == COLLABORATIVE_OPENED;
    }
    AUDIO_DEBUG_LOG("address %{public}s is not in map", GetEncryptAddr(selectedAudioDevice->macAddress_).c_str());
    return false;
}

int32_t AudioCollaborativeService::UpdateCollaborativeStateReal()
{
    if (!isCollaborativePlaybackSupported_) {
        AUDIO_INFO_LOG("Local device does not support collaborative service!");
        return ERROR;
    }
    if (addressToCollaborativeEnabledMap_.find(curDeviceAddress_) == addressToCollaborativeEnabledMap_.end()) {
        if (isCollaborativeStateEnabled_ == COLLABORATION_BLUETOOTH_OPEN ||
            isCollaborativeStateEnabled_ == COLLABORATION_NEARLINK_OPEN) {
            isCollaborativeStateEnabled_ = COLLABORATION_CLOSE;
            HILOG_COMM_INFO("[UpdateCollaborativeStateReal]current device %{public}s is not in "
                "addressToCollaborativeEnabledMap_, close collaborative service",
                GetEncryptAddr(curDeviceAddress_).c_str());
            UpdateCollaborativeStateToHdi(isCollaborativeStateEnabled_);
            return audioPolicyManager_.UpdateCollaborativeState(COLLABORATION_CLOSE);
        }
        return SUCCESS;
    }

    bool isCurrentCollaborativeEnabledTemp =
        (addressToCollaborativeEnabledMap_[curDeviceAddress_] == COLLABORATIVE_OPENED);
    CollaborationState isCurrentCollaborativeEnabledState;
    if (isCurrentCollaborativeEnabledTemp && curDeviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        isCurrentCollaborativeEnabledState = COLLABORATION_BLUETOOTH_OPEN;
    } else if (isCurrentCollaborativeEnabledTemp && curDeviceType_ == DEVICE_TYPE_NEARLINK) {
        isCurrentCollaborativeEnabledState = COLLABORATION_NEARLINK_OPEN;
    } else {
        isCurrentCollaborativeEnabledState = COLLABORATION_CLOSE;
    }

    if (isCollaborativeStateEnabled_ != isCurrentCollaborativeEnabledState) {
        isCollaborativeStateEnabled_ = isCurrentCollaborativeEnabledState;
        HILOG_COMM_INFO("[UpdateCollaborativeStateReal]current collaborative enabled state changed to %{public}d "
            "for Mac address %{public}s", isCollaborativeStateEnabled_, GetEncryptAddr(curDeviceAddress_).c_str());
        UpdateCollaborativeStateToHdi(isCollaborativeStateEnabled_);
        return audioPolicyManager_.UpdateCollaborativeState(isCollaborativeStateEnabled_);
    }
    AUDIO_INFO_LOG("No need to real collaborative state: %{public}d", isCollaborativeStateEnabled_);
    return SUCCESS;
}

bool AudioCollaborativeService::GetRealCollaborativeState()
{
    AUDIO_DEBUG_LOG("GetRealCollaborativeState Entered!");
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    return isCollaborativeStateEnabled_;
}

void AudioCollaborativeService::WriteCollaborativeStateSysEvents(std::string macAddress_, CollaborativeState state)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::SET_DEVICE_COLLABORATIVE_STATE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("ADDRESS", macAddress_);
    bean->Add("COLLABORATIVE_STATE", static_cast<int32_t>(state));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioCollaborativeService::RecoverCollaborativeState()
{
    std::map<std::string, uint32_t> storedCollaborativeState;
    Media::MediaMonitor::MediaMonitorManager::GetInstance().GetCollaborativeDeviceState(storedCollaborativeState);
    for (auto p: storedCollaborativeState) {
        addressToCollaborativeEnabledMap_[p.first] = static_cast<CollaborativeState>(p.second);
    }
}

AudioCollaborativeService::~AudioCollaborativeService()
{
    AUDIO_ERR_LOG("~AudioCollaborativeService");
}

int32_t AudioCollaborativeService::UpdateCollaborativeProductId(const std::string &productId)
{
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    return audioPolicyManager_.UpdateCollaborativeProductId(productId);
}

void AudioCollaborativeService::LoadCollaborationConfig()
{
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    audioPolicyManager_.LoadCollaborationConfig();
}

bool AudioCollaborativeService::IsCollaborativePlaybackOpenedOrReservedForDevice(
    const AudioDeviceDescriptor &selectedAudioDevice)
{
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    AUDIO_INFO_LOG("IsCollaborativePlaybackOpenedOrReservedForDevice Entered!");
    auto it = addressToCollaborativeEnabledMap_.find(selectedAudioDevice.macAddress_);
    CHECK_AND_RETURN_RET_LOG(it != addressToCollaborativeEnabledMap_.end(), false,
        "address %{public}s is not in map", GetEncryptAddr(selectedAudioDevice.macAddress_).c_str());
    return it->second == COLLABORATIVE_OPENED || it->second == COLLABORATIVE_RESERVED;
}

void AudioCollaborativeService::UpdateCollaborativeStateToHdi(CollaborationState collaborativeState)
{
    HdiAdapterManager &manager = HdiAdapterManager::GetInstance();
    std::shared_ptr<IDeviceManager> deviceManager = manager.GetDeviceManager(HDI_DEVICE_MANAGER_TYPE_LOCAL);
    CHECK_AND_RETURN_LOG(deviceManager != nullptr, "local device manager is nullptr");
    std::string keyValue = std::string("collaborative_state=") +
        std::to_string(static_cast<int32_t>(collaborativeState));
    deviceManager->SetAudioParameter("primary", AudioParamKey::NONE, "", keyValue);
    AUDIO_INFO_LOG("UpdateCollaborativeStateToHdi, value: %{public}s", keyValue.c_str());
}

CollaborationState AudioCollaborativeService::GetCollaborativeGlobalSwitchState()
{
    std::lock_guard<std::mutex> lock(collaborativeServiceMutex_);
    AUDIO_INFO_LOG("GetCollaborativeGlobalSwitchState :%{public}d", isCollaborativeStateEnabled_);
    return isCollaborativeStateEnabled_;
}
} // AudioStandard
} // OHOS