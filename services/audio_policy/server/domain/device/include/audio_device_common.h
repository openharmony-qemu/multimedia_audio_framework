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
#include "audio_manager_base.h"
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
    void UpdateDualToneState(const bool &enable, const int32_t &sessionId);
    void FetchOutputDevice(std::vector<std::shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    void FetchInputDevice(std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    void MoveToNewOutputDevice(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &outputDevices,
        std::vector<SinkInput> sinkInputs,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    void MoveToNewInputDevice(std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
        std::shared_ptr<AudioDeviceDescriptor> &inputDevice);
    DeviceType GetSpatialDeviceType(const std::string& macAddress);

    int32_t ActivateA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    bool IsRendererStreamRunning(std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo);

    int32_t MoveToLocalOutputDevice(std::vector<SinkInput> sinkInputIds,
        std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor);
    void TriggerRecreateRendererStreamCallback(int32_t callerPid, int32_t sessionId, int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t ScoInputDeviceFetchedForRecongnition(bool handleFlag, const std::string &address,
        ConnectState connectState);
    std::vector<SourceOutput> GetSourceOutputs();
    void SetFirstScreenOn();
    void ClientDiedDisconnectScoNormal();
    void ClientDiedDisconnectScoRecognition();
    int32_t SetVirtualCall(pid_t uid, const bool isVirtual);
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
        audioA2dpDevice_(AudioA2dpDevice::GetInstance()) {}
    ~AudioDeviceCommon() {}

    void UpdateConnectedDevicesWhenConnectingForOutputDevice(const AudioDeviceDescriptor &updatedDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb);
    void UpdateConnectedDevicesWhenConnectingForInputDevice(const AudioDeviceDescriptor &updatedDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb);

    void MuteOtherSink(const std::string &sinkName, int64_t muteTime);
    void MuteSinkPort(const std::string &oldSinkName, const std::string &newSinkName,
        AudioStreamDeviceChangeReasonExt reason);
    void MuteSinkPortLogic(const std::string &oldSinkName, const std::string &newSinkName,
        AudioStreamDeviceChangeReasonExt reason);

    void UpdateRoute(shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &outputDevices);
    bool SelectRingerOrAlarmDevices(const vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo);
    bool IsSameDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, AudioDeviceDescriptor &deviceInfo);
    bool IsSameDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, const AudioDeviceDescriptor &deviceDesc);
    void RemoveOfflineDevice(const AudioDeviceDescriptor& updatedDesc);
    bool IsDeviceConnected(std::shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptors) const;
    int32_t HandleDeviceChangeForFetchInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo);
    bool IsFastFromA2dpToA2dp(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);
    void WriteInputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason);
    std::vector<SourceOutput> FilterSourceOutputs(int32_t sessionId);
    bool IsRingerOrAlarmerDualDevicesRange(const InternalDeviceType &deviceType);

    int32_t MoveToLocalInputDevice(std::vector<SourceOutput> sourceOutputIds,
        std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor);
    int32_t MoveToRemoteInputDevice(std::vector<SourceOutput> sourceOutputIds,
        std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor);
    int32_t MoveToRemoteOutputDevice(std::vector<SinkInput> sinkInputIds,
        std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor);

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
    int32_t SwitchActiveA2dpDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor);
    int32_t RingToneVoiceControl(const InternalDeviceType &deviceType);
    void ClearRingMuteWhenCallStart(bool pre, bool after);

    // fetchOutput
    void FetchOutputEnd(const bool isUpdateActiveDevice, const int32_t runningStreamCount,
        const AudioStreamDeviceChangeReason reason);
    void FetchOutputDeviceWhenNoRunningStream(const AudioStreamDeviceChangeReason reason);
    void SetDeviceConnectedFlagWhenFetchOutputDevice();
    int32_t HandleDeviceChangeForFetchOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo, const AudioStreamDeviceChangeReason reason);
    void MuteSinkPortForSwitchDevice(std::shared_ptr<AudioRendererChangeInfo>& rendererChangeInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>>& outputDevices,
        const AudioStreamDeviceChangeReasonExt reason);
    void MuteSinkForSwitchGeneralDevice(std::shared_ptr<AudioRendererChangeInfo>& rendererChangeInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>>& outputDevices,
        const AudioStreamDeviceChangeReasonExt reason);
    void MuteSinkForSwitchBluetoothDevice(std::shared_ptr<AudioRendererChangeInfo>& rendererChangeInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>>& outputDevices,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t ActivateA2dpDeviceWhenDescEnabled(shared_ptr<AudioDeviceDescriptor> &desc,
        vector<shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    int32_t HandleScoOutputDeviceFetched(std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    bool NotifyRecreateRendererStream(std::shared_ptr<AudioDeviceDescriptor> &desc,
        const std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);
    bool NeedRehandleA2DPDevice(std::shared_ptr<AudioDeviceDescriptor> &desc);
    void SetVoiceCallMuteForSwitchDevice();
    bool HasLowLatencyCapability(DeviceType deviceType, bool isRemote);
    vector<std::shared_ptr<AudioDeviceDescriptor>> GetDeviceDescriptorInner(
        std::shared_ptr<AudioRendererChangeInfo> &rendererChangeInfo);
    bool IsRingDualToneOnPrimarySpeaker(const vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
        const int32_t sessionId);
    bool IsRingOverPlayback(AudioMode &mode, RendererState rendererState);
    bool IsDualStreamWhenRingDual(AudioStreamType streamType);

    // fetchInput
    void FetchInputDeviceInner(std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason, bool& needUpdateActiveDevice, bool& isUpdateActiveDevice,
        int32_t& runningStreamCount);
    void FetchInputEnd(const bool isUpdateActiveDevice, const int32_t runningStreamCount);
    void FetchInputDeviceWhenNoRunningStream();
    void HandleBluetoothInputDeviceFetched(std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos, SourceType sourceType);
    bool NotifyRecreateCapturerStream(bool isUpdateActiveDevice,
        const std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);
    void BluetoothScoFetch(std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos, SourceType sourceType);
    void HandleA2dpInputDeviceFetched(std::shared_ptr<AudioDeviceDescriptor> &desc, SourceType sourceType);
    void TriggerRecreateCapturerStreamCallback(const std::shared_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
        int32_t streamFlag, const AudioStreamDeviceChangeReasonExt reason);
    int32_t HandleScoInputDeviceFetched(std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos);
    void SetHeadsetUnpluggedToSpkOrEpFlag(DeviceType oldDeviceType, DeviceType newDeviceType);

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

    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_ = nullptr;
};

}
}

#endif
