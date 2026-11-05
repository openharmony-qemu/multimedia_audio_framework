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
#ifndef ST_AUDIO_ACTIVE_DEVICE_H
#define ST_AUDIO_ACTIVE_DEVICE_H

#include <bitset>
#include <list>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_module_info.h"
#include "audio_errors.h"
#include "audio_device_manager.h"
#include "audio_a2dp_offload_flag.h"
#include "audio_a2dp_device.h"
#include "audio_iohandle_map.h"
#include "audio_connected_device.h"
#include "audio_stream_change_info.h"

namespace OHOS {
namespace AudioStandard {

using InternalDeviceType = DeviceType;

class AudioActiveDevice {
public:
    static AudioActiveDevice& GetInstance()
    {
        static AudioActiveDevice instance;
        return instance;
    }
    bool IsDirectSupportedDevice();
    void NotifyUserSelectionEventToBt(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
        StreamUsage streamUsage = STREAM_USAGE_UNKNOWN);
    void NotifyUserDisSelectionEventToBt(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
        bool isSameDevice = false, StreamUsage streamUsage = STREAM_USAGE_INVALID);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> BuildWirelessDeviceNeedsBothDirections(
        const std::map<uint32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> &outputPreviewResults,
        const std::map<uint32_t, std::shared_ptr<AudioDeviceDescriptor>> &inputPreviewResults);
    bool IsDeviceInWirelessNeeds(std::shared_ptr<AudioDeviceDescriptor> device,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &needs);
    void DecideWirelessDeviceActionBasedOnPreview(
        const std::map<uint32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> &outputPreviewResults,
        const std::map<uint32_t, std::shared_ptr<AudioDeviceDescriptor>> &inputPreviewResults,
        std::shared_ptr<AudioDeviceDescriptor> newSelectedDevice,
        StreamUsage streamUsage);
    void DecideActionForSingleWirelessOutputDevice(
        std::shared_ptr<AudioDeviceDescriptor> deviceDesc,
        StreamUsage streamUsage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &wirelessDeviceNeeds);
    void DecideWirelessDeviceActionForInputBasedOnPreview(
        const std::map<uint32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> &outputPreviewResults,
        const std::map<uint32_t, std::shared_ptr<AudioDeviceDescriptor>> &inputPreviewResults,
        std::shared_ptr<AudioDeviceDescriptor> newSelectedDevice,
        SourceType sourceType);
    void DecideActionForSingleWirelessInputDevice(
        std::shared_ptr<AudioDeviceDescriptor> deviceDesc,
        SourceType sourceType,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &wirelessDeviceNeeds);
    void NotifyNewInputDeviceSelected(
        std::shared_ptr<AudioDeviceDescriptor> newSelectedDevice,
        SourceType sourceType);
    void NotifyNewDeviceSelected(
        std::shared_ptr<AudioDeviceDescriptor> newSelectedDevice,
        StreamUsage streamUsage);
    void NotifyUserSelectionEventForInput(std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor,
        SourceType sourceType = SOURCE_TYPE_INVALID);
    void SendUserSelectionEventAsync(DeviceType devType, const std::string &macAddress, int32_t eventType);
    void NotifyUserSelectionEventToRemote(std::shared_ptr<AudioDeviceDescriptor> desc);
    void NotifyUserDisSelectionEventToRemote(std::shared_ptr<AudioDeviceDescriptor> desc);

    bool IsDeviceActive(DeviceType deviceType);
    float GetMaxAmplitude(const int32_t deviceId, const AudioInterrupt audioInterrupt);
    std::set<std::string> GetActiveBtDeviceMac() const;
    // Active bluetooth devices form a mac address set to support dual bluetooth connections.
    void AddActiveBtDevice(const std::string &macAddress);
    void RemoveActiveBtDevice(const std::string &macAddress);
    bool IsActiveBtDevice(const std::string &macAddress) const;
    void SetActiveBtInDeviceMac(const std::string macAddress);
    // Stream info queries are split by bluetooth direction: the A2DP output query keys on the
    // target mac address and fails when no mac is provided; the A2DP_IN query follows the single
    // active input device.
    bool GetActiveA2dpDeviceStreamInfo(const std::string &macAddress, AudioStreamInfo &streamInfo);
    bool GetActiveA2dpInDeviceStreamInfo(AudioStreamInfo &streamInfo);
    DeviceType GetCurrentInputDeviceType(const int32_t uid = SYSTEM_UID);
    std::string GetCurrentInputDeviceMacAddr(const int32_t uid = SYSTEM_UID);
    DeviceType GetCurrentOutputDeviceType(const int32_t uid = SYSTEM_UID);
    DeviceCategory GetCurrentOutputDeviceCategory(const int32_t uid = SYSTEM_UID);
    std::string GetCurrentOutputDeviceNetworkId(const int32_t uid = SYSTEM_UID);
    std::string GetCurrentOutputDeviceMacAddr(const int32_t uid = SYSTEM_UID);
    // pipeId == renderId == ioHandle
    void UpdateActiveDeviceRoute(InternalDeviceType deviceType, DeviceFlag deviceFlag, const uint32_t pipeId,
        const std::string &networkId = LOCAL_NETWORK_ID);
    void UpdateActiveDevicesRoute(std::vector<std::pair<InternalDeviceType, DeviceFlag>> &activeDevices,
        const uint32_t pipeId, const std::string &networkId = LOCAL_NETWORK_ID);
    void ReleaseActiveDeviceRoute(InternalDeviceType deviceType, DeviceFlag deviceFlag, const std::string &networkId);
    bool IsDeviceInVector(std::shared_ptr<AudioDeviceDescriptor> desc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs);
    void UpdateStreamDeviceMapForOutput(std::string source);
    void UpdateStreamDeviceMapForInput(std::string source);
    bool IsDeviceInActiveOutputDevices(DeviceType type, bool isRemote);
    bool IsDeviceInActiveOutputDevices(std::shared_ptr<AudioDeviceDescriptor> desc);
    void SetAdjustVolumeForZone(int32_t zoneId);
    int32_t GetAdjustVolumeZoneId();
    int32_t GetRealUid(std::shared_ptr<AudioStreamDescriptor> streamDesc);

    std::shared_ptr<AudioDeviceDescriptor> GetDeviceForVolume(StreamUsage streamUsage);
    std::shared_ptr<AudioDeviceDescriptor> GetDeviceForVolume(AudioStreamType streamType);
    std::shared_ptr<AudioDeviceDescriptor> GetDeviceForVolume();
    std::shared_ptr<AudioDeviceDescriptor> GetDeviceForVolume(int32_t appUid);
    std::shared_ptr<AudioDeviceDescriptor> GetActiveDeviceForVolume(int32_t appUid);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetActiveOutputDevices();
    void UpdateUidVolumeDeviceMapForEnhancedRouting();

private:
    AudioActiveDevice()
        : audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()) {}
    ~AudioActiveDevice() {}
    void WriteOutputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason);
    void UpdateVolumeTypeDeviceMap(std::shared_ptr<AudioStreamDescriptor> desc);
    void UpdateStreamUsageDeviceMap(std::shared_ptr<AudioStreamDescriptor> desc);
    void SortDevicesByPriority(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs);
    bool IsAvailableFrontDeviceInVector(
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs);
    void UpdateDeviceListWithStreamState(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs);
    std::shared_ptr<AudioDeviceDescriptor> GetDeviceForVolumeEnhanced(int32_t appUid = INVALID_UID);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetUidVolumeDevicesForEnhancedRouting(int32_t uid);
    void UpdateSystemPreferredVolumeDevice(
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void UpdateAppPreferredVolumeDevice(
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);

private:
    int32_t volumeAdjustZoneId_ = 0;
    std::mutex deviceForVolumeMutex_;

    std::set<std::string> activeBTDevices_;
    mutable std::mutex activeBtDevicesMutex_;
    std::string activeBTInDevice_;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> activeOutputDevices_;
    std::unordered_map<AudioVolumeType, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> volumeTypeDeviceMap_;
    std::unordered_map<StreamUsage, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> streamUsageDeviceMap_;
    std::unordered_map<int32_t, std::vector<std::shared_ptr<AudioDeviceDescriptor>>> uidVolumeDeviceMap_;
    std::shared_ptr<AudioDeviceDescriptor> defaultOutputDevice_ =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);

    AudioDeviceManager &audioDeviceManager_;
    AudioA2dpDevice& audioA2dpDevice_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;
    AudioConnectedDevice& audioConnectedDevice_;
};

}
}

#endif
