/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

    virtual int32_t SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel,
        API_VERSION api_v = API_9) = 0;

    virtual int32_t GetSystemVolumeLevel(AudioVolumeType volumeType) = 0;

    virtual int32_t SetLowPowerVolume(int32_t streamId, float volume) = 0;

    virtual float GetLowPowerVolume(int32_t streamId) = 0;

    virtual float GetSingleStreamVolume(int32_t streamId) = 0;

    virtual int32_t SetStreamMute(AudioVolumeType volumeType, bool mute, API_VERSION api_v = API_9) = 0;

    virtual bool GetStreamMute(AudioVolumeType volumeType) = 0;

    virtual bool IsStreamActive(AudioVolumeType volumeType) = 0;

    virtual std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag) = 0;

    virtual int32_t SetDeviceActive(InternalDeviceType deviceType, bool active) = 0;

    virtual int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint32_t sessionId) = 0;

    virtual bool IsDeviceActive(InternalDeviceType deviceType) = 0;

    virtual DeviceType GetActiveOutputDevice() = 0;

    virtual DeviceType GetActiveInputDevice() = 0;

#ifdef FEATURE_DTMF_TONE
    virtual std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype) = 0;

    virtual std::vector<int32_t> GetSupportedTones() = 0;
#endif

    virtual int32_t SetRingerMode(AudioRingerMode ringMode, API_VERSION api_v = API_9) = 0;

    virtual AudioRingerMode GetRingerMode() = 0;

    virtual int32_t SetAudioScene(AudioScene scene) = 0;

    virtual int32_t SetMicrophoneMute(bool isMute) = 0;

    virtual int32_t SetMicrophoneMuteAudioConfig(bool isMute) = 0;

    virtual bool IsMicrophoneMute(API_VERSION api_v = API_9) = 0;

    virtual AudioScene GetAudioScene() = 0;

    virtual int32_t SetAudioInterruptCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t UnsetAudioInterruptCallback(const uint32_t sessionID,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t SetAudioManagerInterruptCallback(const int32_t clientId, const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetAudioManagerInterruptCallback(const int32_t clientId) = 0;

    virtual int32_t RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt) = 0;

    virtual int32_t AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt) = 0;

    virtual AudioStreamType GetStreamInFocus(const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t GetSessionInfoInFocus(AudioInterrupt &audioInterrupt,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        SourceType sourceType = SOURCE_TYPE_MIC) = 0;

    virtual bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state) = 0;

    virtual int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType) = 0;

    virtual int32_t GetAudioLatencyFromXml() = 0;

    virtual uint32_t GetSinkLatencyFromXml() = 0;

    virtual int32_t RegisterTracker(AudioMode &mode,
        AudioStreamChangeInfo &streamChangeInfo, const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo) = 0;

    virtual int32_t GetCurrentRendererChangeInfos(
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) = 0;

    virtual int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) = 0;

    virtual int32_t UpdateStreamState(const int32_t clientUid, StreamSetState streamSetState,
                                            AudioStreamType audioStreamType) = 0;

    virtual int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) = 0;

    virtual std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) = 0;

    virtual int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) = 0;

    virtual int32_t GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos) = 0;

    virtual int32_t GetNetworkIdByGroupId(int32_t groupId, std::string &networkId) = 0;

    virtual bool IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo) = 0;

    virtual std::vector<sptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
        AudioRendererInfo &rendererInfo) = 0;

    virtual std::vector<sptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo) = 0;

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

    virtual int32_t SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config, uint32_t appTokenId) = 0;

    virtual int32_t SetCaptureSilentState(bool state) = 0;

    virtual int32_t GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc) = 0;

    virtual std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId) = 0;

    virtual std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones() = 0;

    virtual int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support) = 0;

    virtual bool IsAbsVolumeScene() = 0;

    virtual int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, bool updateUi) = 0;

    virtual std::vector<std::unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage) = 0;

    virtual int32_t SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
        const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage) = 0;

    virtual int32_t ConfigDistributedRoutingRole(const sptr<AudioDeviceDescriptor> descriptor, CastType type) = 0;

    virtual int32_t SetDistributedRoutingRoleCallback(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnsetDistributedRoutingRoleCallback() = 0;

    virtual bool IsSpatializationEnabled() = 0;

    virtual int32_t SetSpatializationEnabled(const bool enable) = 0;

    virtual bool IsHeadTrackingEnabled() = 0;

    virtual int32_t SetHeadTrackingEnabled(const bool enable) = 0;

    virtual int32_t RegisterSpatializationEnabledEventListener(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t RegisterHeadTrackingEnabledEventListener(const sptr<IRemoteObject> &object) = 0;

    virtual int32_t UnregisterSpatializationEnabledEventListener() = 0;

    virtual int32_t UnregisterHeadTrackingEnabledEventListener() = 0;

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

    virtual int32_t CreateAudioInterruptZone(const std::set<int32_t> pids,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t AddAudioInterruptZonePids(const std::set<int32_t> pids,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t RemoveAudioInterruptZonePids(const std::set<int32_t> pids,
        const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t ReleaseAudioInterruptZone(const int32_t zoneID = 0 /* default value: 0 -- local device */) = 0;

    virtual int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address) = 0;

    virtual std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice() = 0;

public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IAudioPolicy");
};
} // namespace AudioStandard
} // namespace OHOS
#endif // I_AUDIO_POLICY_BASE_H
