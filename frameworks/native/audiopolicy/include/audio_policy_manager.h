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

#ifndef ST_AUDIO_POLICY_MANAGER_H
#define ST_AUDIO_POLICY_MANAGER_H

#include <cstdint>
#include "audio_client_tracker_callback_stub.h"
#include "audio_effect.h"
#include "audio_interrupt_callback.h"
#include "audio_policy_base.h"
#include "audio_policy_manager_listener_stub.h"
#include "audio_policy_client_stub_impl.h"
#include "audio_routing_manager.h"
#include "audio_routing_manager_listener_stub.h"
#include "audio_system_manager.h"
#include "audio_system_manager.h"
#include "i_standard_client_tracker.h"
#include "audio_log.h"
#include "microphone_descriptor.h"
#include "audio_spatialization_manager.h"
#include "audio_spatialization_state_change_listener_stub.h"
#include "i_standard_spatialization_state_change_listener.h"

namespace OHOS {
namespace AudioStandard {
using InternalDeviceType = DeviceType;
using InternalAudioCapturerOptions = AudioCapturerOptions;

class AudioPolicyManager {
public:
    static AudioPolicyManager& GetInstance()
    {
        static AudioPolicyManager policyManager;
        return policyManager;
    }

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType);

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType);

    int32_t SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel, API_VERSION api_v = API_9);

    int32_t GetSystemVolumeLevel(AudioVolumeType volumeType);

    int32_t SetLowPowerVolume(int32_t streamId, float volume);

    float GetLowPowerVolume(int32_t streamId);

    float GetSingleStreamVolume(int32_t streamId);

    int32_t SetStreamMute(AudioVolumeType volumeType, bool mute, API_VERSION api_v = API_9);

    bool GetStreamMute(AudioVolumeType volumeType);

    bool IsStreamActive(AudioVolumeType volumeType);

    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);

    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType);

    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);

    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active);

    bool IsDeviceActive(InternalDeviceType deviceType);

    DeviceType GetActiveOutputDevice();

    DeviceType GetActiveInputDevice();

    int32_t SetRingerMode(AudioRingerMode ringMode, API_VERSION api_v = API_9);

#ifdef FEATURE_DTMF_TONE
    std::vector<int32_t> GetSupportedTones();

    std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype);
#endif

    AudioRingerMode GetRingerMode();

    int32_t SetAudioScene(AudioScene scene);

    int32_t SetMicrophoneMute(bool isMute);

    int32_t SetMicrophoneMuteAudioConfig(bool isMute);

    bool IsMicrophoneMute(API_VERSION api_v = API_9);

    AudioScene GetAudioScene();

    int32_t SetDeviceChangeCallback(const int32_t clientId, const DeviceFlag flag,
        const std::shared_ptr<AudioManagerDeviceChangeCallback> &callback);

    int32_t UnsetDeviceChangeCallback(const int32_t clientId, DeviceFlag flag);

    int32_t SetRingerModeCallback(const int32_t clientId,
        const std::shared_ptr<AudioRingerModeCallback> &callback, API_VERSION api_v = API_9);

    int32_t UnsetRingerModeCallback(const int32_t clientId);

    int32_t UnsetRingerModeCallback(const int32_t clientId,
        const std::shared_ptr<AudioRingerModeCallback> &callback);

    int32_t SetMicStateChangeCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback);

    int32_t SetAudioInterruptCallback(const uint32_t sessionID,
        const std::shared_ptr<AudioInterruptCallback> &callback, const int32_t zoneID = 0);

    int32_t UnsetAudioInterruptCallback(const uint32_t sessionID, const int32_t zoneID = 0);

    int32_t ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID = 0);

    int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID = 0);

    int32_t SetAudioManagerInterruptCallback(const int32_t clientId,
        const std::shared_ptr<AudioInterruptCallback> &callback);

    int32_t UnsetAudioManagerInterruptCallback(const int32_t clientId);

    int32_t RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt);

    int32_t AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt);

    AudioStreamType GetStreamInFocus(const int32_t zoneID = 0);

    int32_t GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneID = 0);

    int32_t SetVolumeKeyEventCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v = API_9);

    int32_t UnsetVolumeKeyEventCallback(const int32_t clientPid);

    bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        SourceType sourceType = SOURCE_TYPE_MIC);

    bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state);

    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType);

    int32_t GetAudioLatencyFromXml();

    uint32_t GetSinkLatencyFromXml();

    int32_t RegisterAudioRendererEventListener(const int32_t clientPid,
        const std::shared_ptr<AudioRendererStateChangeCallback> &callback);

    int32_t UnregisterAudioRendererEventListener(const int32_t clientPid);

    int32_t RegisterAudioCapturerEventListener(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerStateChangeCallback> &callback);

    int32_t UnregisterAudioCapturerEventListener(const int32_t clientPid);

    int32_t RegisterOutputDeviceChangeWithInfoCallback(
        const uint32_t sessionID, const std::shared_ptr<OutputDeviceChangeWithInfoCallback> &callback);
    
    int32_t UnregisterOutputDeviceChangeWithInfoCallback(const uint32_t sessionID);

    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const std::shared_ptr<AudioClientTracker> &clientTrackerObj);

    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);

    int32_t GetCurrentRendererChangeInfos(
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos);

    int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);

    int32_t UpdateStreamState(const int32_t clientUid, StreamSetState streamSetState,
                                    AudioStreamType audioStreamType);

    int32_t GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos);

    int32_t GetNetworkIdByGroupId(int32_t groupId, std::string &networkId);

    bool IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo);

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(AudioRendererInfo &rendererInfo);

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(AudioCapturerInfo &captureInfo);

    int32_t SetPreferredOutputDeviceChangeCallback(const int32_t clientId,
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback);

    int32_t SetPreferredInputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback);

    int32_t UnsetPreferredOutputDeviceChangeCallback(const int32_t clientId);

    int32_t UnsetPreferredInputDeviceChangeCallback();

    int32_t GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList,
        const int32_t zoneID = 0);

    int32_t RegisterFocusInfoChangeCallback(const int32_t clientId,
        const std::shared_ptr<AudioFocusInfoChangeCallback> &callback);

    int32_t UnregisterFocusInfoChangeCallback(const int32_t clientId);

    static void AudioPolicyServerDied(pid_t pid);

    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri);

    std::string GetSystemSoundUri(const std::string &key);

    float GetMinStreamVolume(void);

    float GetMaxStreamVolume(void);
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::weak_ptr<AudioRendererPolicyServiceDiedCallback> &callback);
    int32_t UnregisterAudioPolicyServerDiedCb(const int32_t clientPid);

    int32_t RegisterAudioStreamPolicyServerDiedCb(
        const std::shared_ptr<AudioStreamPolicyServiceDiedCallback> &callback);
    int32_t UnregisterAudioStreamPolicyServerDiedCb(
        const std::shared_ptr<AudioStreamPolicyServiceDiedCallback> &callback);

    bool IsVolumeUnadjustable();

    int32_t AdjustVolumeByStep(VolumeAdjustType adjustType);

    int32_t AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType);

    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType);

    int32_t GetMaxRendererInstances();

    int32_t QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig);

    int32_t SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config, uint32_t appTokenId);

    int32_t SetCaptureSilentState(bool state);

    int32_t GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc);

    static void RecoverAudioPolicyCallbackClient();

    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionID);

    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support);

    bool IsAbsVolumeScene();

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, const bool updateUi);

    std::vector<std::unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);

    int32_t SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
        const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback);

    int32_t UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage);

    bool IsSpatializationEnabled();

    int32_t SetSpatializationEnabled(const bool enable);

    bool IsHeadTrackingEnabled();

    int32_t SetHeadTrackingEnabled(const bool enable);

    int32_t RegisterSpatializationEnabledEventListener(
        const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &callback);

    int32_t RegisterHeadTrackingEnabledEventListener(
        const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &callback);

    int32_t UnregisterSpatializationEnabledEventListener();

    int32_t UnregisterHeadTrackingEnabledEventListener();

    AudioSpatializationState GetSpatializationState(const StreamUsage streamUsage);

    bool IsSpatializationSupported();

    bool IsSpatializationSupportedForDevice(const std::string address);

    bool IsHeadTrackingSupported();

    bool IsHeadTrackingSupportedForDevice(const std::string address);

    int32_t UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState);

    int32_t RegisterSpatializationStateEventListener(const uint32_t sessionID, const StreamUsage streamUsage,
        const std::shared_ptr<AudioSpatializationStateChangeCallback> &callback);

    int32_t ConfigDistributedRoutingRole(sptr<AudioDeviceDescriptor> descriptor, CastType type);

    int32_t SetDistributedRoutingRoleCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback);

    int32_t UnsetDistributedRoutingRoleCallback();

    int32_t UnregisterSpatializationStateEventListener(const uint32_t sessionID);

    int32_t CreateAudioInterruptZone(const std::set<int32_t> pids, const int32_t zoneID);

    int32_t AddAudioInterruptZonePids(const std::set<int32_t> pids, const int32_t zoneID);

    int32_t RemoveAudioInterruptZonePids(const std::set<int32_t> pids, const int32_t zoneID);

    int32_t ReleaseAudioInterruptZone(const int32_t zoneID);

    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address);

    std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo, uint32_t sessionId);

private:
    AudioPolicyManager()
    {
        AUDIO_INFO_LOG("Enter AudioPolicyManager::AudioPolicyManager");
    }
    ~AudioPolicyManager() {}

    int32_t RegisterPolicyCallbackClientFunc(const sptr<IAudioPolicy> &gsp);

    std::mutex listenerStubMutex_;
    std::mutex registerCallbackMutex_;
    std::mutex stateChangelistenerStubMutex_;
    std::mutex clientTrackerStubMutex_;
    sptr<AudioPolicyClientStubImpl> audioPolicyClientStubCB_;
    static std::unordered_map<int32_t, std::weak_ptr<AudioRendererPolicyServiceDiedCallback>> rendererCBMap_;
    static sptr<AudioPolicyClientStubImpl> audioStaticPolicyClientStubCB_;
    static std::vector<std::shared_ptr<AudioStreamPolicyServiceDiedCallback>> audioStreamCBMap_;

    bool isAudioRendererEventListenerRegistered = false;
    bool isAudioCapturerEventListenerRegistered = false;
};
} // namespce AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_MANAGER_H
