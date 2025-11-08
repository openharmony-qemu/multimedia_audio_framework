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

#ifndef I_AUDIO_POLICY_INTERFACE_H
#define I_AUDIO_POLICY_INTERFACE_H
#define HDI_INVALID_ID 0xFFFFFFFF

#include "audio_adapter_info.h"
#include "audio_policy_manager.h"
#include "audio_policy_ipc_interface_code.h"
#include "audio_stream_removed_callback.h"
#include "audio_volume_config.h"
#include "volume_data_maintainer.h"
#include "audio_manager_base.h"
#include "audio_pipe_info.h"
#include "istandard_audio_service.h"

#include <memory>
#include <string>

namespace OHOS {
namespace AudioStandard {

enum LoudVolumeHoldType {
    LOUD_VOLUME_MODE_INVALID = -1,
    LOUD_VOLUME_MODE_MUSIC,
    LOUD_VOLUME_MODE_VOICE,
};

enum OffloadAdapter : uint32_t {
    OFFLOAD_IN_PRIMARY = 0,
    OFFLOAD_IN_REMOTE,
    OFFLOAD_IN_ADAPTER_SIZE
};

class IAudioPolicyInterface {
public:
    virtual ~IAudioPolicyInterface() {}

    virtual bool Init() = 0;

    virtual void Deinit(void) = 0;

    virtual void InitKVStore() = 0;

    virtual bool ConnectServiceAdapter() = 0;

    virtual int32_t GetMaxVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType = DEVICE_TYPE_NONE) = 0;

    virtual int32_t GetMinVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType = DEVICE_TYPE_NONE) = 0;

    virtual int32_t SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel) = 0;

    virtual int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel) = 0;

    virtual int32_t SetAppVolumeMuted(int32_t appUid, bool muted) = 0;

    virtual int32_t IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute) = 0;

    virtual int32_t SetAppRingMuted(int32_t appUid, bool muted) = 0;

    virtual bool IsAppRingMuted(int32_t appUid) = 0;

    virtual int32_t GetSystemVolumeLevel(AudioStreamType streamType) = 0;

    virtual int32_t GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel) = 0;

    virtual int32_t SetAdjustVolumeForZone(int32_t zoneId) = 0;

    virtual int32_t GetVolumeAdjustZoneId() = 0;

    virtual int32_t SetZoneVolumeLevel(int32_t zoneId, AudioStreamType streamType, int32_t volumeLevel) = 0;

    virtual int32_t GetZoneVolumeLevel(int32_t zoneId, AudioStreamType streamType) = 0;

    virtual int32_t SetZoneMute(int32_t zoneId, AudioStreamType streamType, bool mute,
        StreamUsage streamUsage = STREAM_USAGE_UNKNOWN,
        const DeviceType &deviceType = DEVICE_TYPE_NONE) = 0;

    virtual bool GetZoneMute(int32_t zoneId, AudioStreamType streamType) = 0;

    virtual int32_t GetSystemVolumeLevelNoMuteState(AudioStreamType streamType) = 0;

    virtual float GetSystemVolumeDb(AudioStreamType streamType) = 0;

    virtual int32_t SetStreamMute(AudioStreamType streamType, bool mute,
        StreamUsage streamUsage = STREAM_USAGE_UNKNOWN, const DeviceType &deviceType = DEVICE_TYPE_NONE,
        std::string networkId = LOCAL_NETWORK_ID) = 0;

    virtual void SetDeviceNoMuteForRinger(std::shared_ptr<AudioDeviceDescriptor> device) = 0;

    virtual void ClearDeviceNoMuteForRinger() = 0;

    virtual int32_t SetSourceOutputStreamMute(int32_t uid, bool setMute) = 0;

    virtual bool GetStreamMute(AudioStreamType streamType) = 0;

    virtual std::vector<SinkInfo> GetAllSinks() = 0;

    virtual void GetAllSinkInputs(std::vector<SinkInput> &sinkInputs) = 0;

    virtual std::vector<SourceOutput> GetAllSourceOutputs() = 0;

    virtual AudioIOHandle OpenAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex) = 0;

    virtual AudioIOHandle OpenAudioPort(const AudioModuleInfo &audioPortInfo, uint32_t &paIndex) = 0;
    
    virtual AudioIOHandle ReloadA2dpAudioPort(const AudioModuleInfo &audioPortInfo, uint32_t &paIndex) = 0;

    virtual void ReloadAudioPort(const AudioModuleInfo &audioPortInfo, uint32_t &paIndex) = 0;

    virtual int32_t CloseAudioPort(AudioIOHandle ioHandle, uint32_t paIndex = HDI_INVALID_ID) = 0;

    virtual int32_t SelectDevice(DeviceRole deviceRole, InternalDeviceType deviceType, std::string name) = 0;

    virtual int32_t SetDeviceActive(InternalDeviceType deviceType,
                                    std::string name, bool active, DeviceFlag flag = ALL_DEVICES_FLAG) = 0;

    virtual int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName) = 0;

    virtual int32_t MoveSourceOutputByIndexOrName(uint32_t sourceOutputId,
        uint32_t sourceIndex, std::string sourceName) = 0;

    virtual int32_t SetRingerMode(AudioRingerMode ringerMode) = 0;

    virtual AudioRingerMode GetRingerMode() const = 0;

    virtual int32_t SetAudioStreamRemovedCallback(AudioStreamRemovedCallback *callback) = 0;

    virtual int32_t SuspendAudioDevice(std::string &name, bool isSuspend) = 0;

    virtual void UpdateVolumeForStreams() = 0;

    virtual bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false) = 0;

    virtual float CalculateVolumeDb(int32_t volumeLevel) = 0;

    virtual int32_t SetSystemSoundUri(const std::string &key, const std::string &uri) = 0;

    virtual std::string GetSystemSoundUri(const std::string &key) = 0;

    virtual float GetMinStreamVolume() const = 0;

    virtual float GetMaxStreamVolume() const = 0;

    virtual bool IsVolumeUnadjustable() = 0;

    virtual void GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos) = 0;

    virtual void SetAbsVolumeScene(bool isAbsVolumeScene, int32_t volume) = 0;

    virtual bool IsAbsVolumeScene() const = 0;

    virtual void SetAbsVolumeMute(bool mute) = 0;

    virtual void SetAbsVolumeMuteNearlink(bool mute) = 0;

    virtual void SetDataShareReady(std::atomic<bool> isDataShareReady) = 0;

    virtual bool IsAbsVolumeMute() const = 0;

    virtual float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) = 0;

    virtual std::string GetModuleArgs(const AudioModuleInfo &audioModuleInfo) const = 0;

    virtual void HandleDpConnection() = 0;

    virtual int32_t DoRestoreData() = 0;

    virtual SafeStatus GetCurrentDeviceSafeStatus(DeviceType deviceType) = 0;

    virtual int64_t GetCurentDeviceSafeTime(DeviceType deviceType) = 0;

    virtual int32_t SetDeviceSafeStatus(DeviceType deviceType, SafeStatus status) = 0;

    virtual int32_t SetDeviceSafeTime(DeviceType deviceType, int64_t time) = 0;

    virtual int32_t SetRestoreVolumeLevel(DeviceType deviceType, int32_t volume) = 0;

    virtual int32_t GetRestoreVolumeLevel(DeviceType deviceType) = 0;

    virtual int32_t GetSafeVolumeLevel() const = 0;

    virtual int32_t GetSafeVolumeTimeout() const = 0;

    virtual void SafeVolumeDump(std::string &dumpString) = 0;

    virtual void SetActiveDeviceDescriptor(AudioDeviceDescriptor deviceDescriptor) = 0;

    virtual DeviceType GetActiveDevice() = 0;

    virtual DeviceCategory GetCurrentOutputDeviceCategory() = 0;

    virtual AudioDeviceDescriptor GetActiveDeviceDescriptor() = 0;

    virtual void NotifyAccountsChanged(const int &id) = 0;

    virtual void MuteMediaWhenAccountsChanged() = 0;

    virtual int32_t GetCurActivateCount() const = 0;

    virtual void HandleKvData(bool isFirstBoot) = 0;

    virtual int32_t SetPersistMicMuteState(const bool isMute) = 0;

    virtual int32_t GetPersistMicMuteState(bool &isMute) = 0;

    virtual void HandleSaveVolume(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel,
        std::string networkId) = 0;

    virtual void HandleStreamMuteStatus(AudioStreamType streamType, bool mute,
        const DeviceType &deviceType = DEVICE_TYPE_NONE,
        std::string networkId = LOCAL_NETWORK_ID) = 0;

    virtual void HandleRingerMode(AudioRingerMode ringerMode) = 0;

    virtual void SetAudioServerProxy(sptr<IStandardAudioService> gsp) = 0;

    virtual void SetOffloadSessionId(uint32_t sessionId, OffloadAdapter offloadAdapter) = 0;

    virtual void ResetOffloadSessionId(OffloadAdapter offloadAdapter) = 0;

    virtual int32_t SetDoubleRingVolumeDb(const AudioStreamType &streamType, const int32_t &volumeLevel) = 0;

    virtual int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) const = 0;

    virtual int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) const = 0;

    virtual int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) const = 0;

    virtual int32_t GetDeviceVolume(DeviceType deviceType, AudioStreamType streamType) = 0;

    virtual void SaveRingerModeInfo(AudioRingerMode ringMode, std::string callerName, std::string invocationTime) = 0;

    virtual void GetRingerModeInfo(std::vector<RingerModeAdjustInfo> &ringerModeInfo) = 0;

    virtual std::vector<AdjustStreamVolumeInfo> GetStreamVolumeInfo(AdjustStreamVolume volumeType) = 0;

    virtual std::shared_ptr<AllDeviceVolumeInfo> GetAllDeviceVolumeInfo(DeviceType deviceType,
        AudioStreamType streamType) = 0;

    virtual void UpdateSafeVolumeByS4() = 0;

    virtual int32_t SaveSpecifiedDeviceVolume(AudioStreamType streamType, int32_t volumeLevel,
        DeviceType deviceType) = 0;

    virtual int32_t UpdateCollaborativeState(bool isCollaborationEnabled) = 0;
    virtual void RegisterDoNotDisturbStatus() = 0;
    virtual void RegisterDoNotDisturbStatusWhiteList() = 0;
    virtual int32_t SetQueryDeviceVolumeBehaviorCallback(const sptr<IRemoteObject> &object) = 0;

    virtual void SetSleVoiceStatusFlag(bool isSleVoiceStatus) = 0;
    virtual void SendLoudVolumeModeToDsp(LoudVolumeHoldType funcHoldType, bool state) = 0;
    virtual void SaveSystemVolumeForEffect(DeviceType deviceType, AudioStreamType streamType,
        int32_t volumeLevel) = 0;

    virtual int32_t GetSystemVolumeForEffect(DeviceType deviceType, AudioStreamType streamType) = 0;

    virtual int32_t SetSystemVolumeToEffect(AudioStreamType streamType, float volume) = 0;

    virtual float CalculateVolumeDbNonlinear(AudioStreamType streamType, DeviceType deviceType,
        int32_t volumeLevel) = 0;
    
    virtual void AddCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        const SourceType &sourceType) = 0;
    virtual void RemoveCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        const SourceType &sourceType) = 0;
    virtual void UpdateAudioPortInfo(const uint32_t &sinkPortIndex, const AudioModuleInfo &audioPortInfo) = 0;
    virtual int32_t AddCaptureInjector() = 0;
    virtual int32_t RemoveCaptureInjector() = 0;
    virtual void UpdateVolumeWhenDeviceConnect(std::shared_ptr<AudioDeviceDescriptor> &device) = 0;
    virtual void UpdateVolumeWhenDeviceDisconnect(std::shared_ptr<AudioDeviceDescriptor> &device) = 0;
    virtual void QueryDeviceVolumeBehavior(std::shared_ptr<AudioDeviceDescriptor> &device) = 0;
    virtual bool IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout) = 0;
    virtual void UpdateOtherStreamVolume(AudioStreamType streamType) = 0;
    virtual void SetVolumeLimit(float volume) = 0;
    virtual void SetMaxVolumeForDpBoardcast() = 0;
    virtual void HandleCastingConnection() = 0;
    virtual void HandleCastingDisconnection() = 0;
    virtual bool IsDPCastingConnect() = 0;
    virtual int32_t SetSystemVolumeDegree(AudioStreamType streamType, int32_t volumeDegree) = 0;
    virtual int32_t GetSystemVolumeDegree(AudioStreamType streamType, bool checkMuteState = true) = 0;
    virtual int32_t GetMinVolumeDegree(AudioVolumeType volumeType, DeviceType deviceType) = 0;
    virtual float GetSystemVolumeInDbByDegree(AudioVolumeType volumeType, DeviceType deviceType, bool mute) = 0;
    virtual int32_t SetZoneVolumeDegreeToMap(int32_t zoneId, AudioStreamType streamType, int32_t volumeDegree) = 0;
    virtual int32_t GetZoneVolumeDegree(int32_t zoneId, AudioStreamType streamType) = 0;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // I_AUDIO_POLICY_INTERFACE_H
