/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef I_AUDIO_POLICY_BASE_H
#define I_AUDIO_POLICY_BASE_H

#include "audio_interrupt_callback.h"
#include "audio_policy_ipc_interface_code.h"
#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"
#include "audio_system_manager.h"
#include "audio_effect.h"
#include "microphone_descriptor.h"

namespace OHOS {
namespace AudioStandard {
using InternalDeviceType = DeviceType;
using InternalAudioCapturerOptions = AudioCapturerOptions;

class IAudioPolicy : public IRemoteBroker {
public:

    virtual int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) = 0;

    virtual int32_t GetMinVolumeLevel(AudioVolumeType volumeType) = 0;

    virtual int32_t SetSystemVolumeLevelLegacy(AudioVolumeType volumeType, int32_t volumeLevel) = 0;

    virtual int32_t SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel, int32_t volumeFlag = 0) = 0;

    virtual int32_t SetSystemVolumeLevelWithDevice(AudioVolumeType volumeType, int32_t volumeLevel,
        DeviceType deviceType, int32_t volumeFlag = 0) = 0;

    virtual int32_t GetAppVolumeLevel(int32_t appUid) = 0;

    virtual int32_t GetSelfAppVolumeLevel() = 0;

    virtual int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel, int32_t volumeFlag = 0) = 0;

    virtual bool IsAppVolumeMute(int32_t appUid, bool muted) = 0;

    virtual int32_t SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag = 0) = 0;

    virtual int32_t SetSelfAppVolumeLevel(int32_t volumeLevel, int32_t volumeFlag = 0) = 0;

    virtual AudioStreamType GetSystemActiveVolumeType(const int32_t clientUid) = 0;

    virtual int32_t GetSystemVolumeLevel(AudioVolumeType volumeType) = 0;

    virtual int32_t SetLowPowerVolume(int32_t streamId, float volume) = 0;

    virtual float GetLowPowerVolume(int32_t streamId) = 0;

    virtual float GetSingleStreamVolume(int32_t streamId) = 0;

    virtual int32_t SetStreamMuteLegacy(AudioVolumeType volumeType, bool mute,
        const DeviceType &deviceType = DEVICE_TYPE_NONE) = 0;

    virtual int32_t SetStreamMute(AudioVolumeType volumeType, bool mute,
        const DeviceType &deviceType = DEVICE_TYPE_NONE) = 0;

    virtual bool GetStreamMute(AudioVolumeType volumeType) = 0;

    virtual bool IsStreamActive(AudioVolumeType volumeType) = 0;

    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag) = 0;

    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag) = 0;

    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetOutputDevice(
        sptr<AudioRendererFilter> audioRendererFilter) = 0;

    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetInputDevice(
        sptr<AudioCapturerFilter> audioCapturerFilter) = 0;

    virtual int32_t SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid = INVALID_UID) = 0;

    virtual bool IsDeviceActive(InternalDeviceType deviceType) = 0;

    virtual DeviceType GetActiveOutputDevice() = 0;

    virtual DeviceType GetActiveInputDevice() = 0;

#ifdef FEATURE_DTMF_TONE
    virtual std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype, const std::string &countryCode) = 0;

    virtual std::vector<int32_t> GetSupportedTones(const std::string &countryCode) = 0;
#endif

    virtual int32_t SetRingerModeLegacy(AudioRingerMode ringMode) = 0;

    virtual int32_t SetRingerMode(AudioRingerMode ringMode) = 0;

    virtual AudioRingerMode GetRingerMode() = 0;

    virtual int32_t SetAudioScene(AudioScene scene) = 0;

    virtual int32_t SetMicrophoneMute(bool isMute) = 0;

    virtual int32_t SetMicrophoneMuteAudioConfig(bool isMute) = 0;

    virtual int32_t SetMicrophoneMutePersistent(const bool isMute, const PolicyType type) = 0;

    virtual bool GetPersistentMicMuteState() = 0;

    virtual bool IsMicrophoneMuteLegacy() = 0;

    virtual bool IsMicrophoneMute() = 0;

    virtual AudioScene GetAudioScene() = 0;

    virtual int32_t ActivateAudioSession(const AudioSessionStrategy &strategy) = 0;

    virtual int32_t DeactivateAudioSession() = 0;

    virtual bool IsAudioSessionActivated() = 0;

    virtual int32_t SetAudioInterruptCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object,
        uint32_t clientUid, const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t UnsetAudioInterruptCallback(const uint32_t sessionID,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t ActivateAudioInterrupt(AudioInterrupt &audioInterrupt,
        const int32_t zoneID = 0 /* default value: 0 -- local device */,
        const bool isUpdatedAudioStrategy = false /* default value: false -- is update audio strategy */) = 0;

    virtual int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t SetAudioManagerInterruptCallback(const int32_t clientId, const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetAudioManagerInterruptCallback(const int32_t clientId) = 0;

    virtual int32_t SetQueryClientTypeCallback(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t SetAudioClientInfoMgrCallback(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt) = 0;

    virtual int32_t AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt) = 0;

    virtual AudioStreamType GetStreamInFocus(const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual AudioStreamType GetStreamInFocusByUid(
        const int32_t uid, const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t GetSessionInfoInFocus(AudioInterrupt &audioInterrupt,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType) = 0;

    virtual int32_t GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo) = 0;

    virtual int32_t GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo) = 0;

    virtual int32_t RegisterTracker(AudioMode &mode,
        AudioStreamChangeInfo &streamChangeInfo, const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo) = 0;

    virtual int32_t GetCurrentRendererChangeInfos(
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) = 0;

    virtual int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) = 0;

    virtual int32_t UpdateStreamState(const int32_t clientUid, StreamSetState streamSetState,
                                            StreamUsage streamUsage) = 0;

    virtual int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) = 0;

    virtual std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) = 0;

    virtual int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) = 0;

    virtual int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) = 0;

    virtual int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) = 0;

    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(
        AudioDeviceUsage audioDevUsage) = 0;

    virtual int32_t GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos) = 0;

    virtual int32_t GetNetworkIdByGroupId(int32_t groupId, std::string &networkId) = 0;

    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
        AudioRendererInfo &rendererInfo, bool forceNoBTPermission) = 0;

    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo) = 0;

    virtual int32_t SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable) = 0;

    virtual int32_t SetCallbackRendererInfo(const AudioRendererInfo &rendererInfo) = 0;

    virtual int32_t SetCallbackCapturerInfo(const AudioCapturerInfo &capturerInfo) = 0;

    virtual int32_t GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t SetSystemSoundUri(const std::string &key, const std::string &uri) = 0;

    virtual std::string GetSystemSoundUri(const std::string &key) = 0;

    virtual float GetMinStreamVolume(void) = 0;

    virtual float GetMaxStreamVolume(void) = 0;

    virtual int32_t GetMaxRendererInstances() = 0;

    virtual bool IsVolumeUnadjustable(void) = 0;

    virtual int32_t AdjustVolumeByStep(VolumeAdjustType adjustType) = 0;

    virtual int32_t AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType) = 0;

    virtual float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) = 0;

    virtual int32_t QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig) = 0;

    virtual int32_t GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc) = 0;

    virtual std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId) = 0;

    virtual std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones() = 0;

    virtual int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support) = 0;

    virtual bool IsAbsVolumeScene() = 0;

    virtual int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, bool updateUi) = 0;

    virtual std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage) = 0;

    virtual int32_t SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
        const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage) = 0;

    virtual int32_t ConfigDistributedRoutingRole(
        const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type) = 0;

    virtual int32_t SetDistributedRoutingRoleCallback(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetDistributedRoutingRoleCallback() = 0;

    virtual bool IsSpatializationEnabled() = 0;

    virtual bool IsSpatializationEnabled(const std::string address) = 0;

    virtual bool IsSpatializationEnabledForCurrentDevice() = 0;

    virtual int32_t SetSpatializationEnabled(const bool enable) = 0;

    virtual int32_t SetSpatializationEnabled(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice,
        const bool enable) = 0;

    virtual bool IsHeadTrackingEnabled() = 0;

    virtual bool IsHeadTrackingEnabled(const std::string address) = 0;

    virtual int32_t SetHeadTrackingEnabled(const bool enable) = 0;

    virtual int32_t SetHeadTrackingEnabled(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice,
        const bool enable) = 0;

    virtual AudioSpatializationState GetSpatializationState(const StreamUsage streamUsage) = 0;

    virtual bool IsSpatializationSupported() = 0;

    virtual bool IsSpatializationSupportedForDevice(const std::string address) = 0;

    virtual bool IsHeadTrackingSupported() = 0;

    virtual bool IsHeadTrackingSupportedForDevice(const std::string address) = 0;

    virtual int32_t UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState) = 0;

    virtual int32_t RegisterSpatializationStateEventListener(const uint32_t sessionID, const StreamUsage streamUsage,
        const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnregisterSpatializationStateEventListener(const uint32_t sessionID) = 0;

    virtual int32_t RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t CreateAudioInterruptZone(const std::set<int32_t> &pids,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t AddAudioInterruptZonePids(const std::set<int32_t> &pids,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t RemoveAudioInterruptZonePids(const std::set<int32_t> &pids,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t ReleaseAudioInterruptZone(const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
        const int32_t uid = INVALID_UID) = 0;

    virtual std::shared_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice() = 0;

    virtual ConverterConfig GetConverterConfig() = 0;

    virtual void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason) = 0;

    virtual void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo) = 0;

    virtual bool IsHighResolutionExist(void) = 0;

    virtual int32_t SetHighResolutionExist(bool highResExist) = 0;

    virtual AudioSpatializationSceneType GetSpatializationSceneType() = 0;

    virtual int32_t SetSpatializationSceneType(const AudioSpatializationSceneType spatializationSceneType) = 0;

    virtual float GetMaxAmplitude(const int32_t deviceId) = 0;

    virtual int32_t DisableSafeMediaVolume() = 0;

    virtual bool IsHeadTrackingDataRequested(const std::string &macAddress) = 0;

    virtual int32_t SetAudioDeviceRefinerCallback(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetAudioDeviceRefinerCallback() = 0;

    virtual int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason) = 0;

    virtual int32_t SetPreferredDevice(const PreferredType preferredType,
        const std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid = INVALID_UID) = 0;

    virtual int32_t SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetAudioDeviceAnahsCallback() = 0;

    virtual int32_t SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetAudioConcurrencyCallback(const uint32_t sessionID) = 0;

    virtual int32_t ActivateAudioConcurrency(const AudioPipeType &pipeType) = 0;

    virtual int32_t MoveToNewPipe(const uint32_t sessionId, const AudioPipeType pipeType) = 0;

    virtual int32_t InjectInterruption(const std::string networkId, InterruptEvent &event) = 0;

    virtual int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId) = 0;

    virtual bool IsAllowedPlayback(const int32_t &uid, const int32_t &pid) = 0;

    virtual int32_t SetVoiceRingtoneMute(bool isMute) = 0;

    virtual void SaveRemoteInfo(const std::string &networkId, DeviceType deviceType) = 0;

    virtual int32_t SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        const bool isConnected) = 0;

    virtual int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) = 0;

    virtual int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray) = 0;

    virtual int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) = 0;

    virtual int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray) = 0;

    virtual int32_t GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray) = 0;

    virtual int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray) = 0;

    virtual int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) = 0;

    virtual int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray) = 0;

    virtual int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray) = 0;

    virtual int32_t SetVirtualCall(const bool isVirtual) = 0;

    virtual int32_t SetQueryAllowedPlaybackCallback(const sptr<IRemoteObject> &object) = 0;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IAudioPolicy");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_AUDIO_POLICY_BASE_H
