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
#ifndef ST_AUDIO_DEVICE_LOCK_H
#define ST_AUDIO_DEVICE_LOCK_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_manager_base.h"
#include "audio_module_info.h"
#include "audio_volume_config.h"
#include "audio_errors.h"
#include "microphone_descriptor.h"
#include "audio_system_manager.h"

#include "audio_policy_manager_factory.h"
#include "audio_device_manager.h"
#include "audio_stream_collector.h"

#include "audio_active_device.h"
#include "audio_scene_manager.h"
#include "audio_volume_manager.h"
#include "audio_connected_device.h"
#include "audio_microphone_descriptor.h"
#include "audio_offload_stream.h"
#include "audio_device_common.h"
#include "audio_capturer_session.h"
#include "audio_device_status.h"
#include "audio_recovery_device.h"
#include "audio_a2dp_offload_flag.h"
#include "audio_a2dp_offload_manager.h"

namespace OHOS {
namespace AudioStandard {

class AudioDeviceLock {
public:
    static AudioDeviceLock& GetInstance()
    {
        static AudioDeviceLock instance;
        return instance;
    }
    void DeInit();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
        AudioRendererInfo &rendererInfo, std::string networkId = LOCAL_NETWORK_ID);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo, std::string networkId = LOCAL_NETWORK_ID);
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(
        AudioDeviceUsage audioDevUsage);
    void UpdateSpatializationSupported(const std::string macAddress, const bool support);
    /*****IDeviceStatusObserver*****/
    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status);
    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);
    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);
    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);
    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress);
    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);
    void UpdateAppVolume(int32_t appUid, int32_t volume);
    /*****IDeviceStatusObserver*****/
private:
    AudioDeviceLock() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioStateManager_(AudioStateManager::GetAudioStateManager()),
        audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioSceneManager_(AudioSceneManager::GetInstance()),
        audioVolumeManager_(AudioVolumeManager::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
        audioMicrophoneDescriptor_(AudioMicrophoneDescriptor::GetInstance()),
        audioOffloadStream_(AudioOffloadStream::GetInstance()),
        audioDeviceCommon_(AudioDeviceCommon::GetInstance()),
        audioCapturerSession_(AudioCapturerSession::GetInstance()),
        audioDeviceStatus_(AudioDeviceStatus::GetInstance()),
        audioRecoveryDevice_(AudioRecoveryDevice::GetInstance()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()) {}
    ~AudioDeviceLock() {}
private:
    IAudioPolicyInterface& audioPolicyManager_;
    AudioStreamCollector& streamCollector_;
    AudioStateManager &audioStateManager_;
    AudioDeviceManager &audioDeviceManager_;
    AudioActiveDevice& audioActiveDevice_;
    AudioSceneManager& audioSceneManager_;
    AudioVolumeManager& audioVolumeManager_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioMicrophoneDescriptor& audioMicrophoneDescriptor_;
    AudioOffloadStream& audioOffloadStream_;
    AudioDeviceCommon& audioDeviceCommon_;
    AudioCapturerSession& audioCapturerSession_;
    AudioDeviceStatus& audioDeviceStatus_;
    AudioRecoveryDevice& audioRecoveryDevice_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;

    mutable std::shared_mutex deviceStatusUpdateSharedMutex_;
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;
};

}
}

#endif
