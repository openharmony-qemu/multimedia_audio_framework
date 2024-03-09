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

#ifndef ST_AUDIO_POLICY_PROXY_H
#define ST_AUDIO_POLICY_PROXY_H

#include <memory>
#include "iremote_proxy.h"
#include "audio_policy_base.h"
#include "audio_info.h"
#include "audio_errors.h"
#include "microphone_descriptor.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyProxy : public IRemoteProxy<IAudioPolicy> {
public:
    explicit AudioPolicyProxy(const sptr<IRemoteObject> &impl);
    virtual ~AudioPolicyProxy() = default;

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) override;

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType) override;

    int32_t SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel, API_VERSION api_v) override;

    int32_t GetSystemVolumeLevel(AudioVolumeType volumeType) override;

    int32_t SetLowPowerVolume(int32_t streamId, float volume) override;

    float GetLowPowerVolume(int32_t streamId) override;

    float GetSingleStreamVolume(int32_t streamId) override;

    int32_t SetStreamMute(AudioVolumeType volumeType, bool mute, API_VERSION api_v) override;

    bool GetStreamMute(AudioVolumeType volumeType) override;

    bool IsStreamActive(AudioVolumeType volumeType) override;

    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag) override;

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint32_t sessionId) override;

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active) override;

    bool IsDeviceActive(InternalDeviceType deviceType) override;

    DeviceType GetActiveOutputDevice() override;

    DeviceType GetActiveInputDevice() override;

    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) override;

    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) override;

    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) override;

    int32_t SetRingerMode(AudioRingerMode ringMode, API_VERSION api_v) override;

    int32_t ConfigDistributedRoutingRole(const sptr<AudioDeviceDescriptor> descriptor, CastType type) override;

    int32_t SetDistributedRoutingRoleCallback(const sptr<IRemoteObject> &object) override;

    int32_t UnsetDistributedRoutingRoleCallback() override;

#ifdef FEATURE_DTMF_TONE
    std::vector<int32_t> GetSupportedTones() override;

    std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype) override;
#endif

    AudioRingerMode GetRingerMode() override;

    int32_t SetAudioScene(AudioScene scene) override;

    int32_t SetMicrophoneMute(bool isMute) override;

    int32_t SetMicrophoneMuteAudioConfig(bool isMute) override;

    bool IsMicrophoneMute(API_VERSION api_v) override;

    AudioScene GetAudioScene() override;

    int32_t SetAudioInterruptCallback(const uint32_t sessionID,
        const sptr<IRemoteObject> &object, const int32_t zoneID = 0) override;

    int32_t UnsetAudioInterruptCallback(const uint32_t sessionID, const int32_t zoneID = 0) override;

    int32_t ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID = 0) override;

    int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID = 0) override;

    int32_t SetAudioManagerInterruptCallback(const int32_t clientId, const sptr<IRemoteObject> &object) override;

    int32_t UnsetAudioManagerInterruptCallback(const int32_t clientId) override;

    int32_t RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt) override;

    int32_t AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt) override;

    AudioStreamType GetStreamInFocus(const int32_t zoneID = 0) override;

    int32_t GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneID = 0) override;

    bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        SourceType sourceType = SOURCE_TYPE_MIC) override;

    bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state) override;

    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType) override;

    int32_t GetAudioLatencyFromXml() override;

    uint32_t GetSinkLatencyFromXml() override;

    int32_t RegisterTracker(AudioMode &mode,
        AudioStreamChangeInfo &streamChangeInfo, const sptr<IRemoteObject> &object) override;

    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo) override;

    int32_t GetCurrentRendererChangeInfos(
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;

    int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;

    int32_t UpdateStreamState(const int32_t clientUid, StreamSetState streamSetState,
        AudioStreamType audioStreamType) override;

    int32_t GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos) override;

    int32_t GetNetworkIdByGroupId(int32_t groupId, std::string &networkId) override;

    bool IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo) override;

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
        AudioRendererInfo &rendererInfo) override;

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo) override;

    int32_t GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList,
        const int32_t zoneID = 0) override;

    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri) override;

    std::string GetSystemSoundUri(const std::string &key) override;

    float GetMinStreamVolume(void) override;

    float GetMaxStreamVolume(void) override;

    int32_t GetMaxRendererInstances() override;

    bool IsVolumeUnadjustable() override;

    int32_t AdjustVolumeByStep(VolumeAdjustType adjustType) override;

    int32_t AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType) override;

    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) override;

    int32_t QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig) override;

    int32_t SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config, uint32_t appTokenId) override;

    int32_t  SetCaptureSilentState(bool state) override;

    int32_t GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc) override;

    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId) override;

    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones() override;

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support) override;

    bool IsAbsVolumeScene() override;

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, const bool updateUi) override;

    std::vector<std::unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage) override;

    int32_t SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
        const sptr<IRemoteObject> &object) override;

    int32_t UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage) override;

    bool IsSpatializationEnabled() override;

    int32_t SetSpatializationEnabled(const bool enable) override;

    bool IsHeadTrackingEnabled() override;

    int32_t SetHeadTrackingEnabled(const bool enable) override;

    int32_t RegisterSpatializationEnabledEventListener(const sptr<IRemoteObject> &object) override;

    int32_t RegisterHeadTrackingEnabledEventListener(const sptr<IRemoteObject> &object) override;

    int32_t UnregisterSpatializationEnabledEventListener() override;

    int32_t UnregisterHeadTrackingEnabledEventListener() override;

    AudioSpatializationState GetSpatializationState(const StreamUsage streamUsage) override;

    bool IsSpatializationSupported() override;

    bool IsSpatializationSupportedForDevice(const std::string address) override;

    bool IsHeadTrackingSupported() override;

    bool IsHeadTrackingSupportedForDevice(const std::string address) override;

    int32_t UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState) override;

    int32_t RegisterSpatializationStateEventListener(const uint32_t sessionID, const StreamUsage streamUsage,
        const sptr<IRemoteObject> &object) override;

    int32_t UnregisterSpatializationStateEventListener(const uint32_t sessionID) override;

    int32_t RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const int32_t zoneID = 0) override;

    int32_t CreateAudioInterruptZone(const std::set<int32_t> pids, const int32_t zoneID) override;

    int32_t AddAudioInterruptZonePids(const std::set<int32_t> pids, const int32_t zoneID) override;

    int32_t RemoveAudioInterruptZonePids(const std::set<int32_t> pids, const int32_t zoneID) override;

    int32_t ReleaseAudioInterruptZone(const int32_t zoneID) override;

    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address) override;

    std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice() override;

private:
    static inline BrokerDelegator<AudioPolicyProxy> mDdelegator;
    void WriteStreamChangeInfo(MessageParcel &data, const AudioMode &mode,
        const AudioStreamChangeInfo &streamChangeInfo);
    void ReadAudioFocusInfo(MessageParcel &reply, std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_PROXY_H
