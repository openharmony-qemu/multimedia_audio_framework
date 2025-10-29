/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioDeviceLock"
#endif

#include "audio_device_lock.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_policy_log.h"
#include "media_monitor_manager.h"
#include "audio_state_manager.h"

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {
const int32_t DATA_LINK_CONNECTED = 11;
static constexpr int64_t WAIT_LOAD_DEFAULT_DEVICE_TIME_MS = 200; // 200ms
static constexpr int32_t RETRY_TIMES = 25;

void AudioDeviceLock::DeInit()
{
    audioA2dpOffloadManager_ = nullptr;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetDevices(DeviceFlag deviceFlag)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioConnectedDevice_.GetDevicesInner(deviceFlag);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioDeviceCommon_.GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioDeviceCommon_.GetPreferredInputDeviceDescInner(captureInfo, networkId);
}

void AudioDeviceLock::UpdateAppVolume(int32_t appUid, int32_t volume)
{
    AUDIO_INFO_LOG("appUid = %{public}d, volume = %{public}d", appUid, volume);
    streamCollector_.UpdateAppVolume(appUid, volume);
}

void AudioDeviceLock::OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceInfoUpdated(desc, command);
}

void AudioDeviceLock::OnDeviceStatusUpdated(DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    // Pnp device status update
    Trace trace("KeyAction AudioDeviceLock::OnDeviceStatusUpdated");
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioDeviceLock::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    // Bluetooth device status updated
    Trace trace("KeyAction AudioDeviceLock::OnDeviceStatusUpdated");
    DeviceType devType = updatedDesc.deviceType_;
    string macAddress = updatedDesc.macAddress_;
    string deviceName = updatedDesc.deviceName_;
    bool isActualConnection = (updatedDesc.connectState_ != VIRTUAL_CONNECTED);
    AUDIO_INFO_LOG("Device connection is actual connection: %{public}d", isActualConnection);

    AudioStreamInfo streamInfo = {};
#ifdef BLUETOOTH_ENABLE
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP && isActualConnection && isConnected) {
        int32_t ret = Bluetooth::AudioA2dpManager::GetA2dpDeviceStreamInfo(macAddress, streamInfo);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Get a2dp device stream info failed!");
    }
    if (devType == DEVICE_TYPE_BLUETOOTH_A2DP_IN && isActualConnection && isConnected) {
        int32_t ret = Bluetooth::AudioA2dpManager::GetA2dpInDeviceStreamInfo(macAddress, streamInfo);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Get a2dp input device stream info failed!");
    }
    if (isConnected && isActualConnection
        && devType == DEVICE_TYPE_BLUETOOTH_SCO
        && updatedDesc.deviceCategory_ != BT_UNWEAR_HEADPHONE
        && !audioDeviceManager_.GetScoState()) {
        Bluetooth::AudioHfpManager::SetActiveHfpDevice(macAddress);
    }
#endif
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceStatusUpdated(updatedDesc, devType,
        macAddress, deviceName, isActualConnection, streamInfo, isConnected);
}

void AudioDeviceLock::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioDeviceLock::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    // Distributed devices status update
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioDeviceLock::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnForcedDeviceSelected(devType, macAddress, filter);
}

void AudioDeviceLock::OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnPrivacyDeviceSelected(devType, macAddress);
}

int32_t AudioDeviceLock::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    return audioRecoveryDevice_.UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioDeviceLock::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    std::shared_lock deviceLock(deviceStatusUpdateSharedMutex_);
    return audioStateManager_.GetExcludedDevices(audioDevUsage);
}

// new lock
void AudioDeviceLock::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioDeviceLock::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioDeviceStatus_.OnMicrophoneBlockedUpdate(devType, status);
}

void AudioDeviceLock::UpdateSpatializationSupported(const std::string macAddress, const bool support)
{
    std::lock_guard<std::shared_mutex> deviceLock(deviceStatusUpdateSharedMutex_);
    audioConnectedDevice_.UpdateSpatializationSupported(macAddress, support);
}

}
}
