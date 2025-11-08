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

#ifndef ST_AUDIO_POLICY_MANAGER_H
#define ST_AUDIO_POLICY_MANAGER_H

#include <cstdint>
#include <memory>
#include "audio_client_tracker_callback_service.h"
#include "audio_client_tracker_callback_listener.h"
#include "audio_effect.h"
#include "audio_interrupt_callback.h"
#include "iaudio_policy.h"
#include "audio_policy_manager_listener_stub_impl.h"
#include "audio_policy_client_stub_impl.h"
#include "audio_routing_manager.h"
#include "audio_routing_manager_listener.h"
#include "audio_anahs_manager_listener.h"
#include "audio_policy_interface.h"
#include "audio_system_manager.h"
#include "istandard_client_tracker.h"
#include "audio_policy_log.h"
#include "microphone_descriptor.h"
#include "audio_spatialization_manager.h"
#include "audio_combine_denoising_manager.h"
#include "audio_stream_descriptor.h"
#include "sle_audio_operation_callback_stub_impl.h"
#include "audio_capturer_options.h"
#include "audio_collaborative_manager.h"

namespace OHOS {
namespace AudioStandard {
using InternalDeviceType = DeviceType;
using InternalAudioCapturerOptions = AudioCapturerOptions;
using AudioServerDiedCallBack = std::function<void()>;

struct CallbackChangeInfo {
    std::mutex mutex;
    bool isEnable = false;
};

class AudioPolicyManager {
public:
    static AudioPolicyManager& GetInstance();
    static const sptr<IAudioPolicy> GetAudioPolicyManagerProxy(bool block = true);

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType = DEVICE_TYPE_NONE);

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType = DEVICE_TYPE_NONE);

    int32_t SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel, bool isLegacy = false,
        int32_t volumeFlag = 0, int32_t uid = 0);

    int32_t SetSystemVolumeLevelWithDevice(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType,
        int32_t volumeFlag = 0);
    int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel, int32_t volumeFlag = 0);

    int32_t SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag = 0);

    int32_t SetAppRingMuted(int32_t appUid, bool muted);

    int32_t SetAdjustVolumeForZone(int32_t zoneId);

    int32_t IsAppVolumeMute(int32_t appUid, bool muted, bool &isMute);

    int32_t SetSelfAppVolumeLevel(int32_t volumeLevel, int32_t volumeFlag = 0);

    AudioStreamType GetSystemActiveVolumeType(const int32_t clientUid);

    bool ReloadLoudVolumeMode(AudioStreamType streamType, SetLoudVolMode setVolMode);

    int32_t GetSystemVolumeLevel(AudioVolumeType volumeType, int32_t uid = 0);

    int32_t GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel);

    int32_t GetSelfAppVolumeLevel(int32_t &volumeLevel);

    int32_t SetLowPowerVolume(int32_t streamId, float volume);

    float GetLowPowerVolume(int32_t streamId);

    float GetSingleStreamVolume(int32_t streamId);

    AudioStreamInfo GetFastStreamInfo(uint32_t sessionId);

    int32_t SetStreamMute(AudioVolumeType volumeType, bool mute, bool isLegacy = false,
        const DeviceType &deviceType = DEVICE_TYPE_NONE);

    bool GetStreamMute(AudioVolumeType volumeType);

    bool IsStreamActive(AudioVolumeType volumeType);

    bool IsStreamActiveByStreamUsage(StreamUsage streamUsage);

    bool IsFastPlaybackSupported(AudioStreamInfo &streamInfo, StreamUsage usage);
    bool IsFastRecordingSupported(AudioStreamInfo &streamInfo, SourceType source);

    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors,
        const int32_t audioDeviceSelectMode = 0);

    int32_t SelectPrivateDevice(int32_t devType, const std::string &macAddress);

    int32_t ForceSelectDevice(DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter);

    int32_t RestoreOutputDevice(sptr<AudioRendererFilter> audioRendererFilter);

    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType);

    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors);

    int32_t SelectInputDevice(std::shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptor);

    int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);

    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(
        AudioDeviceUsage audioDevUsage);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetOutputDevice(
        sptr<AudioRendererFilter> audioRendererFilter);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetInputDevice(
        sptr<AudioCapturerFilter> audioCapturerFilter);

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid = INVALID_UID);

    bool IsDeviceActive(InternalDeviceType deviceType);

    DeviceType GetActiveOutputDevice();

    uint16_t GetDmDeviceType();

    DeviceType GetActiveInputDevice();

    int32_t SetRingerModeLegacy(AudioRingerMode ringMode);

    int32_t SetRingerMode(AudioRingerMode ringMode);

    void CleanUpResource();

#ifdef FEATURE_DTMF_TONE
    std::vector<int32_t> GetSupportedTones(const std::string &countryCode);

    std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype, const std::string &countryCode);
#endif

    AudioRingerMode GetRingerMode();

    int32_t SetAudioScene(AudioScene scene);

    int32_t SetMicrophoneMute(bool isMute);

    int32_t SetMicrophoneMuteAudioConfig(bool isMute);

    int32_t SetMicrophoneMutePersistent(const bool isMute, const PolicyType type);

    bool GetPersistentMicMuteState();

    bool IsMicrophoneMuteLegacy();

    bool IsMicrophoneMute();

    AudioScene GetAudioScene();

    int32_t SetDeviceChangeCallback(const int32_t clientId, const DeviceFlag flag,
        const std::shared_ptr<AudioManagerDeviceChangeCallback> &callback);

    int32_t UnsetDeviceChangeCallback(const int32_t clientId, DeviceFlag flag,
        std::shared_ptr<AudioManagerDeviceChangeCallback> &cb);

    int32_t SetDeviceInfoUpdateCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> &callback);

    int32_t UnsetDeviceInfoUpdateCallback(const int32_t clientId,
        std::shared_ptr<AudioManagerDeviceInfoUpdateCallback> &cb);

    int32_t SetRingerModeCallback(const int32_t clientId,
        const std::shared_ptr<AudioRingerModeCallback> &callback, API_VERSION api_v = API_9);

    int32_t SetAppVolumeChangeCallbackForUid(const int32_t appUid,
        const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback);

    int32_t UnsetAppVolumeCallbackForUid(
        const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback = nullptr);

    int32_t SetSelfAppVolumeChangeCallback(const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback);

    int32_t UnsetSelfAppVolumeCallback(const std::shared_ptr<AudioManagerAppVolumeChangeCallback> &callback);

    int32_t SetActiveVolumeTypeCallback(const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback);

    int32_t UnsetActiveVolumeTypeCallback(const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback);

    int32_t UnsetRingerModeCallback(const int32_t clientId);

    int32_t UnsetRingerModeCallback(const int32_t clientId,
        const std::shared_ptr<AudioRingerModeCallback> &callback);

    int32_t SetMicStateChangeCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback);

    int32_t UnsetMicStateChangeCallback(const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback);

    int32_t SetAudioInterruptCallback(const uint32_t sessionID,
        const std::shared_ptr<AudioInterruptCallback> &callback, uint32_t clientUid, const int32_t zoneID = 0);

    int32_t UnsetAudioInterruptCallback(const uint32_t sessionID, const int32_t zoneID = 0);

    int32_t ActivateAudioInterrupt(
        AudioInterrupt &audioInterrupt, const int32_t zoneID = 0, const bool isUpdatedAudioStrategy = false);

    int32_t SetAppConcurrencyMode(const int32_t appUid, const int32_t mode = 0);

    int32_t SetAppSilentOnDisplay(const int32_t displayId = -1);

    int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneID = 0);

    int32_t ActivatePreemptMode(void);

    int32_t DeactivatePreemptMode(void);

    int32_t SetQueryClientTypeCallback(const std::shared_ptr<AudioQueryClientTypeCallback> &callback);

    int32_t SetQueryBundleNameListCallback(const std::shared_ptr<AudioQueryBundleNameListCallback> &callback);

    int32_t SetAudioManagerInterruptCallback(const int32_t clientId,
        const std::shared_ptr<AudioInterruptCallback> &callback);

    int32_t UnsetAudioManagerInterruptCallback(const int32_t clientId);

    int32_t RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt);

    int32_t AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt);

    AudioStreamType GetStreamInFocus(const int32_t zoneID = 0);

    AudioStreamType GetStreamInFocusByUid(const int32_t uid, const int32_t zoneID = 0);

    int32_t GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneID = 0);

    int32_t RegisterAudioPolicyServerDiedCb(std::shared_ptr<AudioSessionManagerPolicyServiceDiedCallback> &callback);

    static void AudioSessionManagerCallback();

    int32_t ActivateAudioSession(const AudioSessionStrategy &strategy);

    int32_t DeactivateAudioSession();

    bool IsAudioSessionActivated();

    int32_t SetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback);

    int32_t UnsetAudioSessionCallback();

    int32_t UnsetAudioSessionCallback(const std::shared_ptr<AudioSessionCallback> &audioSessionCallback);

    int32_t SetAudioSessionScene(const AudioSessionScene audioSessionScene);

    int32_t SetAudioSessionStateChangeCallback(
        const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback);

    int32_t UnsetAudioSessionStateChangeCallback();

    int32_t UnsetAudioSessionStateChangeCallback(
        const std::shared_ptr<AudioSessionStateChangedCallback> &stateChangedCallback);

    int32_t GetDefaultOutputDevice(DeviceType &deviceType);

    int32_t SetDefaultOutputDevice(DeviceType deviceType);

    int32_t SetAudioSessionCurrentDeviceChangeCallback(
        const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback);

    int32_t UnsetAudioSessionCurrentDeviceChangeCallback();

    int32_t UnsetAudioSessionCurrentDeviceChangeCallback(
        const std::shared_ptr<AudioSessionCurrentDeviceChangedCallback> &deviceChangedCallback);
    
    int32_t SetAudioSessionCurrentInputDeviceChangeCallback(
        const std::shared_ptr<AudioSessionCurrentInputDeviceChangedCallback> &deviceChangedCallback);

    int32_t UnsetAudioSessionCurrentInputDeviceChangeCallback(
        const std::optional<std::shared_ptr<AudioSessionCurrentInputDeviceChangedCallback>> &deviceChangedCallback);

    int32_t SetVolumeKeyEventCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v = API_9);

    int32_t UnsetVolumeKeyEventCallback(const std::shared_ptr<VolumeKeyEventCallback> &callback);

    int32_t SetSystemVolumeChangeCallback(const int32_t clientPid,
        const std::shared_ptr<SystemVolumeChangeCallback> &callback);

    int32_t UnsetSystemVolumeChangeCallback(const std::shared_ptr<SystemVolumeChangeCallback> &callback);

    int32_t GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo);

    int32_t GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo);

    int32_t CreateRendererClient(
        std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId, std::string &networkId);

    int32_t CreateCapturerClient(
        std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId);

    int32_t RegisterAudioRendererEventListener(const std::shared_ptr<AudioRendererStateChangeCallback> &callback);

    int32_t UnregisterAudioRendererEventListener(
        const std::vector<std::shared_ptr<AudioRendererStateChangeCallback>> &callbacks);

    int32_t UnregisterAudioRendererEventListener(
        const std::shared_ptr<AudioRendererStateChangeCallback> &callback);

    int32_t RegisterAudioCapturerEventListener(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerStateChangeCallback> &callback);

    int32_t UnregisterAudioCapturerEventListener(const int32_t clientPid);

    int32_t RegisterDeviceChangeWithInfoCallback(
        const uint32_t sessionID, const std::weak_ptr<DeviceChangeWithInfoCallback> &callback);

    int32_t UnregisterDeviceChangeWithInfoCallback(const uint32_t sessionID);

    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const std::shared_ptr<AudioClientTracker> &clientTrackerObj);

    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);

    int32_t GetCurrentRendererChangeInfos(
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos);

    int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);

    int32_t UpdateStreamState(const int32_t clientUid, StreamSetState streamSetState,
                                    StreamUsage streamUsage);

    int32_t GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos);

    int32_t GetNetworkIdByGroupId(int32_t groupId, std::string &networkId);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
        AudioRendererInfo &rendererInfo, bool forceNoBTPermission = false);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo);

    int32_t SetPreferredOutputDeviceChangeCallback(const AudioRendererInfo &rendererInfo,
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback, const int32_t uid = -1);

    int32_t SetPreferredInputDeviceChangeCallback(const AudioCapturerInfo &capturerInfo,
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback);

    int32_t UnsetPreferredOutputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback = nullptr);

    int32_t UnsetPreferredInputDeviceChangeCallback(
        const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback = nullptr);

    int32_t GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList,
        const int32_t zoneID = 0);

    int32_t RegisterFocusInfoChangeCallback(const int32_t clientId,
        const std::shared_ptr<AudioFocusInfoChangeCallback> &callback);

    int32_t UnregisterFocusInfoChangeCallback(const int32_t clientId);

    static void AudioPolicyServerDied(pid_t pid, pid_t uid);

    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri);

    std::string GetSystemSoundUri(const std::string &key);

    float GetMinStreamVolume(void);

    float GetMaxStreamVolume(void);
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback);
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback);
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

    int32_t GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc);

    void RecoverAudioPolicyCallbackClient();

    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionID);

    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support, const int32_t volume);

    bool IsAbsVolumeScene();

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, const bool updateUi);

    int32_t SetNearlinkDeviceVolume(const std::string &macAddress, AudioVolumeType volumeType,
        const int32_t volume, const bool updateUi);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);

    std::shared_ptr<AudioDeviceDescriptor> GetSelectedInputDevice();

    int32_t ClearSelectedInputDevice();

    int32_t PreferBluetoothAndNearlinkRecord(BluetoothAndNearlinkPreferredRecordCategory category);

    BluetoothAndNearlinkPreferredRecordCategory GetPreferBluetoothAndNearlinkRecord();

    int32_t SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
        const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback);

    int32_t UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage);

    bool IsSpatializationEnabled();

    bool IsSpatializationEnabled(const std::string address);

    bool IsSpatializationEnabledForCurrentDevice();

    int32_t SetSpatializationEnabled(const bool enable);

    int32_t SetSpatializationEnabled(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable);

    bool IsHeadTrackingEnabled();

    bool IsHeadTrackingEnabled(const std::string address);

    int32_t SetHeadTrackingEnabled(const bool enable);

    int32_t SetHeadTrackingEnabled(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable);

    int32_t RegisterSpatializationEnabledEventListener(
        const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &callback);

    int32_t RegisterSpatializationEnabledForCurrentDeviceEventListener(
        const std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> &callback);

    int32_t RegisterHeadTrackingEnabledEventListener(
        const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &callback);

    int32_t RegisterNnStateEventListener(const std::shared_ptr<AudioNnStateChangeCallback> &callback);

    int32_t UnregisterSpatializationEnabledEventListener();

    int32_t UnregisterSpatializationEnabledForCurrentDeviceEventListener();

    int32_t UnregisterHeadTrackingEnabledEventListener();

    int32_t UnregisterNnStateEventListener();

    AudioSpatializationState GetSpatializationState(const StreamUsage streamUsage);

    bool IsSpatializationSupported();

    bool IsSpatializationSupportedForDevice(const std::string address);

    bool IsHeadTrackingSupported();

    bool IsHeadTrackingSupportedForDevice(const std::string address);

    int32_t UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState);

    int32_t RegisterSpatializationStateEventListener(const uint32_t sessionID, const StreamUsage streamUsage,
        const std::shared_ptr<AudioSpatializationStateChangeCallback> &callback);

    int32_t ConfigDistributedRoutingRole(std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type);

    int32_t SetDistributedRoutingRoleCallback(const std::shared_ptr<AudioDistributedRoutingRoleCallback> &callback);

    int32_t UnsetDistributedRoutingRoleCallback();

    int32_t UnregisterSpatializationStateEventListener(const uint32_t sessionID);

    int32_t RegisterAudioZoneClient(const sptr<IRemoteObject>& object);

    int32_t CreateAudioZone(const std::string &name, const AudioZoneContext &context);

    void ReleaseAudioZone(int32_t zoneId);

    const std::vector<std::shared_ptr<AudioZoneDescriptor>> GetAllAudioZone();

    const std::shared_ptr<AudioZoneDescriptor> GetAudioZone(int32_t zoneId);

    int32_t GetAudioZoneByName(std::string name);

    int32_t BindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices);

    int32_t UnBindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices);

    int32_t EnableAudioZoneReport(bool enable);

    int32_t EnableAudioZoneChangeReport(int32_t zoneId, bool enable);

    int32_t AddUidToAudioZone(int32_t zoneId, int32_t uid);

    int32_t RemoveUidFromAudioZone(int32_t zoneId, int32_t uid);

    int32_t EnableSystemVolumeProxy(int32_t zoneId, bool enable);

    int32_t AddStreamToAudioZone(int32_t zoneId, AudioZoneStream stream);

    int32_t AddStreamsToAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams);

    int32_t RemoveStreamFromAudioZone(int32_t zoneId, AudioZoneStream stream);

    int32_t RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams);

    void SetZoneDeviceVisible(bool visible);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(int32_t zoneId);

    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(
        int32_t zoneId, const std::string &deviceTag);

    int32_t EnableAudioZoneInterruptReport(int32_t zoneId, const std::string &deviceTag, bool enable);

    int32_t InjectInterruptToAudioZone(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts);

    int32_t InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts);

    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
        const int32_t uid = INVALID_UID);

    std::shared_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();

    ConverterConfig GetConverterConfig();

    void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo);

    bool IsHighResolutionExist();

    int32_t SetHighResolutionExist(bool highResExist);

    AudioSpatializationSceneType GetSpatializationSceneType();

    int32_t SetSpatializationSceneType(const AudioSpatializationSceneType spatializationSceneType);

    float GetMaxAmplitude(const int32_t deviceId);

    int32_t DisableSafeMediaVolume();

    bool IsHeadTrackingDataRequested(const std::string &macAddress);

    int32_t RegisterHeadTrackingDataRequestedEventListener(const std::string &macAddress,
        const std::shared_ptr<HeadTrackingDataRequestedChangeCallback> &callback);

    int32_t UnregisterHeadTrackingDataRequestedEventListener(const std::string &macAddress);

    int32_t SetAudioDeviceRefinerCallback(const std::shared_ptr<AudioDeviceRefiner> &callback);

    int32_t UnsetAudioDeviceRefinerCallback();

    int32_t SetAudioClientInfoMgrCallback(const std::shared_ptr<AudioClientInfoMgrCallback> &callback);

    int32_t SetAudioVKBInfoMgrCallback(const std::shared_ptr<AudioVKBInfoMgrCallback> &callback);
    int32_t CheckVKBInfo(const std::string &bundleName, bool &isValid);

    int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason);

    int32_t SetPreferredDevice(const PreferredType preferredType, const std::shared_ptr<AudioDeviceDescriptor> &desc,
        const int32_t uid = INVALID_UID);

    int32_t SetAudioDeviceAnahsCallback(const std::shared_ptr<AudioDeviceAnahs> &callback);

    int32_t UnsetAudioDeviceAnahsCallback();

    void ResetClientTrackerStubMap();

    void RemoveClientTrackerStub(int32_t sessionId);

    void CheckAndRemoveClientTrackerStub(const AudioMode &mode, const AudioStreamChangeInfo &streamChangeInfo);

    int32_t InjectInterruption(const std::string networkId, InterruptEvent &event);

    int32_t SetMicrophoneBlockedCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> &callback);

    int32_t UnsetMicrophoneBlockedCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> &callback);

    int32_t SetDeviceVolumeBehavior(const std::string &networkId, DeviceType deviceType, VolumeBehavior volumeBehavior);

    int32_t SetQueryDeviceVolumeBehaviorCallback(
        const std::shared_ptr<AudioQueryDeviceVolumeBehaviorCallback> &callback);

    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray);
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray);
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray);

    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray);
    int32_t GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray);
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray);
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray);
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);
    bool IsAcousticEchoCancelerSupported(SourceType sourceType);
    bool IsAudioLoopbackSupported(AudioLoopbackMode mode);
    bool IsSupportInnerCaptureOffload();
    bool IsIntelligentNoiseReductionEnabledForCurrentDevice(SourceType sourceType);
    bool SetKaraokeParameters(const std::string &parameters);
    int32_t SetAudioRouteCallback(uint32_t sessionId, std::shared_ptr<AudioRouteCallback> callback, uint32_t clientUid);
    int32_t UnsetAudioRouteCallback(uint32_t sessionId);

    int32_t SetAudioSceneChangeCallback(const int32_t clientId,
        const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &callback);

    int32_t UnsetAudioSceneChangeCallback(
        const std::shared_ptr<AudioManagerAudioSceneChangedCallback> &callback);

    int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId);

    bool IsAllowedPlayback(const int32_t &uid, const int32_t &pid);

    int32_t SetVoiceRingtoneMute(bool isMute);

    int32_t NotifySessionStateChange(const int32_t uid, const int32_t pid, const bool hasSession);

    int32_t NotifyFreezeStateChange(const std::set<int32_t> &pidList, const bool isFreeze);

    int32_t ResetAllProxy();

    int32_t NotifyProcessBackgroundState(const int32_t uid, const int32_t pid);

    static void RegisterServerDiedCallBack(AudioServerDiedCallBack func);

    int32_t SetVirtualCall(const bool isVirtual);

    int32_t SetInputDevice(const DeviceType deviceType, const uint32_t sessionId,
        const SourceType sourceType, bool isRunning);

    int32_t SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc, const bool isConnected);

    int32_t SetQueryAllowedPlaybackCallback(const std::shared_ptr<AudioQueryAllowedPlaybackCallback> &callback);

    int32_t SetBackgroundMuteCallback(const std::shared_ptr<AudioBackgroundMuteCallback> &callback);

    int32_t SetAudioFormatUnsupportedErrorCallback(
        const std::shared_ptr<AudioFormatUnsupportedErrorCallback> &callback);

    int32_t UnsetAudioFormatUnsupportedErrorCallback();

    DirectPlaybackMode GetDirectPlaybackSupport(const AudioStreamInfo &streamInfo, const StreamUsage &streamUsage);

    int32_t ForceStopAudioStream(StopAudioType audioType);
    bool IsCapturerFocusAvailable(const AudioCapturerInfo &capturerInfo);
    int32_t GetMaxVolumeLevelByUsage(StreamUsage streamUsage);
    int32_t GetMinVolumeLevelByUsage(StreamUsage streamUsage);
    int32_t GetVolumeLevelByUsage(StreamUsage streamUsage);
    bool GetStreamMuteByUsage(StreamUsage streamUsage);
    float GetVolumeInDbByStream(StreamUsage streamUsage, int32_t volumeLevel, DeviceType deviceType);
    std::vector<AudioVolumeType> GetSupportedAudioVolumeTypes();
    AudioVolumeType GetAudioVolumeTypeByStreamUsage(StreamUsage streamUsage);
    std::vector<StreamUsage> GetStreamUsagesByVolumeType(AudioVolumeType audioVolumeType);
    int32_t SetStreamVolumeChangeCallback(const int32_t clientPid, const std::set<StreamUsage> &streamUsages,
        const std::shared_ptr<StreamVolumeChangeCallback> &callback);
    int32_t UnsetStreamVolumeChangeCallback(const std::shared_ptr<StreamVolumeChangeCallback> &callback);
    int32_t SetCallbackStreamUsageInfo(const std::set<StreamUsage> &streamUsages);
    int32_t UpdateDeviceInfo(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
        const DeviceInfoUpdateCommand command);
    int32_t SetSleAudioOperationCallback(const std::shared_ptr<SleAudioOperationCallback> &callback);
    bool IsCollaborativePlaybackSupported();
    int32_t SetCollaborativePlaybackEnabledForDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled);
    bool IsCollaborativePlaybackEnabledForDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice);
    int32_t ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration);
    int32_t RestoreDistributedDeviceInfo();

    int32_t SetVolumeDegreeCallback(const int32_t clientPid,
        const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v = API_9);
    int32_t UnsetVolumeDegreeCallback(const std::shared_ptr<VolumeKeyEventCallback> &callback);
    int32_t SetSystemVolumeDegree(AudioVolumeType volumeType, int32_t volumeDegree,
        int32_t volumeFlag, int32_t uid);
    int32_t GetSystemVolumeDegree(AudioVolumeType volumeType, int32_t uid);
    int32_t GetMinVolumeDegree(AudioVolumeType volumeType, DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t RegisterCollaborationEnabledForCurrentDeviceEventListener(
        const std::shared_ptr<AudioCollaborationEnabledChangeForCurrentDeviceCallback> &callback);
    int32_t UnregisterCollaborationEnabledForCurrentDeviceEventListener();

private:
    AudioPolicyManager() {}
    ~AudioPolicyManager() {}

    int32_t RegisterPolicyCallbackClientFunc(const sptr<IAudioPolicy> &gsp);
    int32_t SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable, bool block = true);
    int32_t SetCallbackStreamInfo(const CallbackChange &callbackChange);
    int32_t SetCallbackRendererInfo(const AudioRendererInfo &rendererInfo, const int32_t uid = -1);
    int32_t SetCallbackCapturerInfo(const AudioCapturerInfo &capturerInfo);
    int32_t CheckAudioPolicyClientRegisted();

    std::mutex listenerStubMutex_;
    std::mutex registerCallbackMutex_;
    std::mutex stateChangelistenerStubMutex_;
    std::mutex clientTrackerStubMutex_;
    sptr<AudioPolicyClientStubImpl> audioPolicyClientStubCB_;
    std::atomic<bool> isAudioPolicyClientRegisted_ = false;

    static std::unordered_map<int32_t, std::weak_ptr<AudioRendererPolicyServiceDiedCallback>> rendererCBMap_;
    static std::weak_ptr<AudioCapturerPolicyServiceDiedCallback> capturerCB_;
    static std::vector<std::weak_ptr<AudioStreamPolicyServiceDiedCallback>> audioStreamCBMap_;
    static std::unordered_map<int32_t, sptr<AudioClientTrackerCallbackService>> clientTrackerStubMap_;

    bool isAudioRendererEventListenerRegistered = false;
    bool isAudioCapturerEventListenerRegistered = false;

    std::array<CallbackChangeInfo, CALLBACK_MAX> callbackChangeInfos_ = {};
    std::vector<AudioRendererInfo> rendererInfos_;
    std::vector<AudioCapturerInfo> capturerInfos_;

    std::mutex handleAvailableDeviceChangeCbsMapMutex_;
    std::map<std::pair<int32_t, AudioDeviceUsage>,
        sptr<IRemoteObject>> availableDeviceChangeCbsMap_;

    static std::vector<AudioServerDiedCallBack> serverDiedCbks_;
    static std::mutex serverDiedCbkMutex_;

    static std::weak_ptr<AudioSessionManagerPolicyServiceDiedCallback> audioSessionManagerCb_;
    static std::mutex serverDiedSessionManagerCbkMutex_;
};
} // namespce AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_MANAGER_H
