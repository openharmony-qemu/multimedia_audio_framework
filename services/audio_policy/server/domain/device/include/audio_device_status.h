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
#ifndef ST_AUDIO_DEVICE_STATUS_H
#define ST_AUDIO_DEVICE_STATUS_H

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
#include "audio_errors.h"

#include "audio_stream_collector.h"
#include "audio_device_manager.h"
#include "audio_policy_manager_factory.h"
#include "audio_effect_service.h"

#include "audio_active_device.h"
#include "audio_scene_manager.h"
#include "audio_volume_manager.h"
#include "audio_connected_device.h"
#include "audio_microphone_descriptor.h"
#include "audio_offload_stream.h"
#include "audio_device_common.h"
#include "audio_capturer_session.h"
#include "audio_iohandle_map.h"
#include "audio_a2dp_device.h"
#include "audio_ec_manager.h"
#include "audio_a2dp_offload_flag.h"
#include "audio_policy_config_manager.h"
#include "audio_router_map.h"
#include "audio_a2dp_offload_manager.h"
#include "audio_spatialization_service.h"

namespace OHOS {
namespace AudioStandard {

class AudioDeviceStatus {
public:
    static AudioDeviceStatus& GetInstance()
    {
        static AudioDeviceStatus instance;
        return instance;
    }
    void Init(std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager,
        std::shared_ptr<AudioPolicyServerHandler> handler);
    void DeInit();
    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false);
    void OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status);
    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);
    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status);
    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    std::shared_ptr<AudioDeviceDescriptor> GetDeviceByStatusInfo(const DStatusInfo &statusInfo);
    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);
    int32_t OnServiceConnected(AudioServiceIndex serviceIndex);
    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter = nullptr);
    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, DeviceType devType,
        std::string macAddress, std::string deviceName, bool isActualConnection, AudioStreamInfo streamInfo,
        bool isConnected);
    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);
    uint16_t GetDmDeviceType();
    void RemoveDeviceFromGlobalOnly(std::shared_ptr<AudioDeviceDescriptor> desc);
    void AddDeviceBackToGlobalOnly(std::shared_ptr<AudioDeviceDescriptor> desc);
    uint32_t GetPaIndexByPortName(const std::string &portName);
    void TriggerDeviceInfoUpdatedCallback(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devChangeDesc);
private:
    AudioDeviceStatus() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioEffectService_(AudioEffectService::GetAudioEffectService()),
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
        audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
        audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
        audioEcManager_(AudioEcManager::GetInstance()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
        audioConfigManager_(AudioPolicyConfigManager::GetInstance()),
        audioRouteMap_(AudioRouteMap::GetInstance()) {}
    ~AudioDeviceStatus() {}

    void UpdateLocalGroupInfo(bool isConnected, const std::string& macAddress,
        const std::string& deviceName, const DeviceStreamInfo& streamInfo, AudioDeviceDescriptor& deviceDesc);
    int32_t HandleLocalDeviceConnected(AudioDeviceDescriptor &updatedDesc);
    int32_t HandleLocalDeviceDisconnected(const AudioDeviceDescriptor &updatedDesc);
    void UpdateActiveA2dpDeviceWhenDisconnecting(const std::string& macAddress);
    int32_t RehandlePnpDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address);
    int32_t HandleArmUsbDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address);
    int32_t HandleDpDevice(DeviceType deviceType, const std::string &address);
    int32_t HandleAccessoryDevice(DeviceType deviceType, const std::string &address);
    int32_t LoadAccessoryModule(std::string deviceInfo);
    int32_t HandleSpecialDeviceType(DeviceType &devType, bool &isConnected,
        const std::string &address, DeviceRole role);
    void TriggerAvailableDeviceChangedCallback(
        const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc, bool isConnected);
    void TriggerDeviceChangedCallback(
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devChangeDesc, bool connection);
    void TriggerMicrophoneBlockedCallback(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc,
        DeviceBlockStatus status);
    int32_t HandleDistributedDeviceUpdate(DStatusInfo &statusInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb, AudioStreamDeviceChangeReasonExt &reason);
    void OnPreferredDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor, DeviceType activeInputDevice);
    void UpdateDeviceList(AudioDeviceDescriptor &updatedDesc, bool isConnected,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb,
        AudioStreamDeviceChangeReasonExt &reason);
    int32_t UpdateNearlinkDeviceVolume(AudioDeviceDescriptor &updatedDesc);
#ifdef BLUETOOTH_ENABLE
    void CheckAndActiveHfpDevice(AudioDeviceDescriptor &desc);
#endif
    void CheckForA2dpSuspend(AudioDeviceDescriptor &desc);
    void UpdateAllUserSelectDevice(vector<shared_ptr<AudioDeviceDescriptor>> &userSelectDeviceMap,
        AudioDeviceDescriptor &desc, const std::shared_ptr<AudioDeviceDescriptor> &selectDesc);
    bool IsConfigurationUpdated(DeviceType deviceType, const AudioStreamInfo &streamInfo);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> UserSelectDeviceMapInit();
    void ClearActiveHfpDevice(AudioDeviceDescriptor &desc,
        const DeviceInfoUpdateCommand updateCommand, AudioStreamDeviceChangeReasonExt &reason);
    void OnPreferredStateUpdated(AudioDeviceDescriptor &desc,
        const DeviceInfoUpdateCommand updateCommand, AudioStreamDeviceChangeReasonExt &reason);
    void AddEarpiece();
    void ReloadA2dpOffloadOnDeviceChanged(DeviceType deviceType, const std::string &macAddress,
        const std::string &deviceName, const AudioStreamInfo &streamInfo);
    void AddAudioDevice(AudioModuleInfo& moduleInfo, DeviceType devType);
    bool OpenPortAndAddDeviceOnServiceConnected(AudioModuleInfo &moduleInfo);
    int32_t GetModuleInfo(ClassType classType, std::string &moduleInfoStr);
    int32_t LoadDpModule(std::string deviceInfo);
    int32_t ActivateNewDevice(std::string networkId, DeviceType deviceType, bool isRemote);
    int32_t RestoreNewA2dpPort(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
        AudioModuleInfo &moduleInfo, std::string &currentActivePort);

    void HandleOfflineDistributedDevice();
    DeviceType GetDeviceTypeFromPin(AudioPin pin);
    string GetModuleNameByType(ClassType type);
    bool NoNeedChangeUsbDevice(const string &address);
    void WriteAllDeviceSysEvents(
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc, bool isConnected);
    void WriteHeadsetSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &desc, bool isConnected);
    void WriteDeviceChangeSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &desc);
    void WriteOutputDeviceChangedSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const SinkInput &sinkInput);
    void WriteInputDeviceChangedSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const SourceOutput &sourceOutput);
private:
    IAudioPolicyInterface& audioPolicyManager_;
    AudioStreamCollector& streamCollector_;
    AudioEffectService& audioEffectService_;
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
    AudioIOHandleMap& audioIOHandleMap_;
    AudioA2dpDevice& audioA2dpDevice_;
    AudioEcManager& audioEcManager_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;
    AudioPolicyConfigManager& audioConfigManager_;
    AudioRouteMap& audioRouteMap_;

    bool remoteCapturerSwitch_ = false;
    std::vector<std::pair<AudioDeviceDescriptor, bool>> pnpDeviceList_;

    static std::map<std::string, AudioSampleFormat> formatStrToEnum;
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_ = nullptr;
    bool hasModulesLoaded = false;
    uint16_t dmDeviceType_ = 0;
};

}
}

#endif
