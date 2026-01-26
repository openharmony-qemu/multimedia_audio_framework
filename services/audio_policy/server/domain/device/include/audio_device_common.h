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

#ifndef ST_AUDIO_DEVICE_COMMON_H
#define ST_AUDIO_DEVICE_COMMON_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_ec_info.h"
#include "audio_module_info.h"
#include "audio_router_center.h"
#include "audio_policy_manager_factory.h"
#include "audio_device_manager.h"
#include "audio_stream_collector.h"
#include "audio_state_manager.h"
#include "audio_affinity_manager.h"
#include "audio_policy_server_handler.h"

#include "audio_a2dp_device.h"
#include "audio_a2dp_offload_flag.h"
#include "audio_policy_config_manager.h"
#include "audio_active_device.h"
#include "audio_iohandle_map.h"
#include "audio_router_map.h"
#include "audio_connected_device.h"
#include "audio_microphone_descriptor.h"
#include "audio_scene_manager.h"
#include "audio_offload_stream.h"
#include "audio_volume_manager.h"
#include "audio_ec_manager.h"
#include "audio_adapter_manager.h"
#include "audio_pipe_manager.h"
namespace OHOS {
namespace AudioStandard {

class AudioDeviceCommon {
public:
    static AudioDeviceCommon& GetInstance()
    {
        static AudioDeviceCommon instance;
        return instance;
    }
    void Init(std::shared_ptr<AudioPolicyServerHandler> handler);
    void DeInit();
    void OnPreferredOutputDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor,
        const AudioStreamDeviceChangeReason reason);
    void OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescInner(
        AudioRendererInfo &rendererInfo, std::string networkId = LOCAL_NETWORK_ID, const int32_t uid = -1);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescInner(
        AudioCapturerInfo &captureInfo, std::string networkId = LOCAL_NETWORK_ID);
    int32_t GetPreferredOutputStreamTypeInner(StreamUsage streamUsage, DeviceType deviceType, int32_t flags,
        std::string &networkId, AudioSamplingRate &samplingRate, bool isFirstCreate = true);
    int32_t GetPreferredInputStreamTypeInner(SourceType sourceType, DeviceType deviceType, int32_t flags,
        const std::string &networkId, const AudioSamplingRate &samplingRate);
    void UpdateDeviceInfo(AudioDeviceDescriptor &deviceInfo,
        const std::shared_ptr<AudioDeviceDescriptor> &desc,
        bool hasBTPermission, bool hasSystemPermission);
    int32_t DeviceParamsCheck(DeviceRole targetRole,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) const;
    void UpdateConnectedDevicesWhenConnecting(const AudioDeviceDescriptor& updatedDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>>& descForCb);
    void UpdateConnectedDevicesWhenDisconnecting(const AudioDeviceDescriptor& updatedDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb, bool updateVolume = true);
    void ClearPreferredDevices(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb);
    DeviceType GetSpatialDeviceType(const std::string& macAddress);

    bool IsRendererStreamRunning(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo);

    int32_t MoveToLocalOutputDevice(std::vector<SinkInput> sinkInputIds,
        std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor);
    void TriggerRecreateRendererStreamCallback(int32_t callerPid, int32_t sessionId, int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t ScoInputDeviceFetchedForRecongnition(bool handleFlag, const std::string &address,
        ConnectState connectState, bool isVrSupported = true);
    std::vector<SourceOutput> GetSourceOutputs();
    void SetFirstScreenOn();
    void ClientDiedDisconnectScoNormal();
    void ClientDiedDisconnectScoRecognition();
    int32_t SetVirtualCall(pid_t uid, const bool isVirtual);
    bool GetVirtualCall(pid_t uid);
    void NotifyDistributedOutputChange(const AudioDeviceDescriptor &deviceDesc);
private:
    AudioDeviceCommon() : audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
        audioStateManager_(AudioStateManager::GetAudioStateManager()),
        audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioAffinityManager_(AudioAffinityManager::GetAudioAffinityManager()),
        audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioConfigManager_(AudioPolicyConfigManager::GetInstance()),
        audioSceneManager_(AudioSceneManager::GetInstance()),
        audioVolumeManager_(AudioVolumeManager::GetInstance()),
        audioRouteMap_(AudioRouteMap::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
        audioMicrophoneDescriptor_(AudioMicrophoneDescriptor::GetInstance()),
        audioEcManager_(AudioEcManager::GetInstance()),
        audioOffloadStream_(AudioOffloadStream::GetInstance()),
        audioA2dpOffloadFlag_(AudioA2dpOffloadFlag::GetInstance()),
        audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
        pipeManager_(AudioPipeManager::GetPipeManager()) {}
    ~AudioDeviceCommon() {}

    void UpdateConnectedDevicesWhenConnectingForOutputDevice(const AudioDeviceDescriptor &updatedDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb);
    void UpdateConnectedDevicesWhenConnectingForInputDevice(const AudioDeviceDescriptor &updatedDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb);

    bool IsSameDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, AudioDeviceDescriptor &deviceInfo);
    bool IsSameDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, const AudioDeviceDescriptor &deviceDesc);
    void RemoveOfflineDevice(const AudioDeviceDescriptor& updatedDesc);
    bool IsDeviceConnected(std::shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptors) const;
    bool IsFastFromA2dpToA2dp(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);
    void WriteInputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason);
    std::vector<SourceOutput> FilterSourceOutputs(int32_t sessionId);
    bool IsRingerOrAlarmerDualDevicesRange(const InternalDeviceType &deviceType);

    void CheckAndNotifyUserSelectedDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    int32_t OpenRemoteAudioDevice(std::string networkId, DeviceRole deviceRole, DeviceType deviceType,
        std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor);

    int32_t LoadA2dpModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo, std::string networkID,
        std::string sinkName, SourceType sourceType);
    void GetA2dpModuleInfo(AudioModuleInfo &moduleInfo, const AudioStreamInfo& audioStreamInfo,
        SourceType sourceType);
    int32_t ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, DeviceType deviceType,
        const AudioStreamInfo& audioStreamInfo, std::string networkID, std::string sinkName,
        SourceType sourceType);
    int32_t RingToneVoiceControl(const InternalDeviceType &deviceType);
    void ClearRingMuteWhenCallStart(bool pre, bool after);
    bool NeedClearPreferredMediaRenderer(const std::shared_ptr<AudioDeviceDescriptor> &preferred,
        const std::shared_ptr<AudioDeviceDescriptor> &updated,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &fetched, const DeviceUsage usage) const;

    // fetchOutput
    bool HasLowLatencyCapability(DeviceType deviceType, bool isRemote);
    vector<std::shared_ptr<AudioDeviceDescriptor>> GetDeviceDescriptorInner(
        std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo);
    bool IsRingDualToneOnPrimarySpeaker(const vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const int32_t sessionId);
    bool IsRingOverPlayback(AudioMode &mode, RendererState rendererState);

private:
    std::unordered_map<std::string, DeviceType> spatialDeviceMap_;
    bool isCurrentRemoteRenderer = false;
    bool enableDualHalToneState_ = false;
    int32_t enableDualHalToneSessionId_ = -1;
    bool isOpenRemoteDevice = false;
    int32_t shouldUpdateDeviceDueToDualTone_ = false;
    bool isFirstScreenOn_ = false;
    bool isRingDualToneOnPrimarySpeaker_ = false;
    bool isHeadsetUnpluggedToSpkOrEpFlag_ = false;
    std::vector<std::pair<uint32_t, AudioStreamType>> streamsWhenRingDualOnPrimarySpeaker_;

    IAudioPolicyInterface& audioPolicyManager_;
    AudioStreamCollector& streamCollector_;
    AudioRouterCenter& audioRouterCenter_;
    AudioStateManager &audioStateManager_;
    AudioDeviceManager &audioDeviceManager_;
    AudioAffinityManager &audioAffinityManager_;
    AudioIOHandleMap& audioIOHandleMap_;
    AudioActiveDevice& audioActiveDevice_;
    AudioPolicyConfigManager& audioConfigManager_;
    AudioSceneManager& audioSceneManager_;
    AudioVolumeManager& audioVolumeManager_;
    AudioRouteMap& audioRouteMap_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioMicrophoneDescriptor& audioMicrophoneDescriptor_;
    AudioEcManager& audioEcManager_;
    AudioOffloadStream& audioOffloadStream_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;
    AudioA2dpDevice& audioA2dpDevice_;
    std::shared_ptr<AudioPipeManager> pipeManager_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_ = nullptr;
};

}
}

#endif
