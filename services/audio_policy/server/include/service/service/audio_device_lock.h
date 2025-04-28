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
    void Init(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager);
    int32_t SetAudioScene(AudioScene audioScene, const int32_t uid = INVALID_UID, const int32_t pid = INVALID_PID);
    void DeInit();
    bool IsArmUsbDevice(const AudioDeviceDescriptor &desc);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);
    int32_t SetDeviceActive(DeviceType deviceType, bool active, const int32_t uid = INVALID_UID);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
        AudioRendererInfo &rendererInfo, std::string networkId = LOCAL_NETWORK_ID);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo, std::string networkId = LOCAL_NETWORK_ID);
    std::shared_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();
    int32_t SetCallDeviceActive(DeviceType deviceType, bool active, std::string address,
        const int32_t uid = INVALID_UID);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);

    void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo);
    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object, const int32_t apiVersion);
    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void RegisteredTrackerClientDied(pid_t uid);
    int32_t GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);
    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();
    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);
    void OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName);
    void UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state);
    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
    int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(
        AudioDeviceUsage audioDevUsage);
    void NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value);
    int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo, AudioStreamInfo streamInfo);
    void OnCapturerSessionRemoved(uint64_t sessionID);
    int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    void SetDisplayName(const std::string &deviceName, bool isLocalDevice);
    void SetDmDeviceType(const uint16_t dmDeviceType);
    void UpdateSpatializationSupported(const std::string macAddress, const bool support);
    std::vector<sptr<VolumeGroupInfo>> GetVolumeGroupInfos();
    /*****IDeviceStatusObserver*****/
    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status);
    void OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status);
    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);
    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);
    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);
    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    int32_t OnServiceConnected(AudioServiceIndex serviceIndex);
    void OnServiceDisconnected(AudioServiceIndex serviceIndex);
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
    void UpdateDefaultOutputDeviceWhenStopping(int32_t uid);
    void SendA2dpConnectedWhileRunning(const RendererState &rendererState, const uint32_t &sessionId);
    void UpdateTrackerDeviceChange(const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    void HandleAudioCaptureState(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
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
