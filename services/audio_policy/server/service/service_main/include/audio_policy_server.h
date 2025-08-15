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

#ifndef ST_AUDIO_POLICY_SERVER_H
#define ST_AUDIO_POLICY_SERVER_H

#include <mutex>
#include <pthread.h>

#include "singleton.h"
#include "system_ability_definition.h"
#include "ipc_skeleton.h"
#include "system_ability.h"
#include "iservice_registry.h"

#include "accesstoken_kit.h"
#include "perm_state_change_callback_customize.h"
#include "power_state_callback_stub.h"
#include "power_state_listener.h"
#include "common_event_subscriber.h"
#include "common_event_support.h"

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"

#include "audio_policy_service.h"
#include "audio_policy_utils.h"
#include "audio_stream_removed_callback.h"
#include "audio_interrupt_callback.h"
#include "audio_policy_stub.h"
#include "audio_server_death_recipient.h"
#include "audio_collaborative_service.h"
#include "audio_spatialization_service.h"
#include "audio_policy_server_handler.h"
#include "audio_interrupt_service.h"
#include "audio_device_manager.h"
#include "audio_policy_dump.h"
#include "app_state_listener.h"
#include "audio_core_service.h"
#include "audio_converter_parser.h"

#ifdef USB_ENABLE
#include "audio_usb_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {

class AudioPolicyService;
class AudioInterruptService;
class AudioPolicyServerHandler;
class AudioSessionService;
class BluetoothEventSubscriber;

class AudioPolicyServer : public SystemAbility,
                          public AudioPolicyStub,
                          public AudioStreamRemovedCallback {
    DECLARE_SYSTEM_ABILITY(AudioPolicyServer);

public:
    DISALLOW_COPY_AND_MOVE(AudioPolicyServer);

    enum DeathRecipientId {
        TRACKER_CLIENT = 0,
        LISTENER_CLIENT
    };

    explicit AudioPolicyServer(int32_t systemAbilityId, bool runOnCreate = true);

    virtual ~AudioPolicyServer()
    {
        AUDIO_WARNING_LOG("dtor should not happen");
    };

    void OnDump() override;
    void OnStart() override;
    void OnStop() override;

    int32_t GetMaxVolumeLevel(int32_t volumeType, int32_t &volumeLevel, int32_t deviceType = -1) override;

    int32_t GetMinVolumeLevel(int32_t volumeType, int32_t &volumeLevel, int32_t deviceType = -1) override;

    int32_t SetSystemVolumeLevelLegacy(int32_t streamTypeIn, int32_t volumeLevel) override;

    int32_t SetSystemVolumeLevel(int32_t volumeType, int32_t volumeLevel, int32_t volumeFlag, int32_t uid) override;

    int32_t SetSystemVolumeLevelWithDevice(int32_t volumeType, int32_t volumeLevel, int32_t deviceType,
        int32_t volumeFlag = 0) override;

    int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel, int32_t volumeFlag) override;

    int32_t IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute) override;

    int32_t SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag) override;

    int32_t SetAppRingMuted(int32_t appUid, bool muted) override;

    int32_t SetAdjustVolumeForZone(int32_t zoneId) override;

    int32_t SetSelfAppVolumeLevel(int32_t volumeLevel, int32_t volumeFlag) override;

    int32_t GetSystemActiveVolumeType(int32_t clientUid, int32_t &streamType) override;

    int32_t GetSystemVolumeLevel(int32_t streamType, int32_t uid, int32_t &volumeLevel) override;

    int32_t GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel) override;

    int32_t GetSelfAppVolumeLevel(int32_t &volumeLevel) override;

    int32_t SetLowPowerVolume(int32_t streamId, float volume) override;

    int32_t GetFastStreamInfo(AudioStreamInfo &streamInfo) override;

    int32_t GetLowPowerVolume(int32_t streamId, float &outVolume) override;

    int32_t GetSingleStreamVolume(int32_t streamId, float &outVolume) override;

    int32_t SetStreamMuteLegacy(int32_t streamType, bool mute,
        int32_t deviceType) override;

    int32_t SetStreamMute(int32_t streamType, bool mute,
        int32_t deviceType) override;

    int32_t GetStreamMute(int32_t streamType, bool &mute) override;

    int32_t IsStreamActive(int32_t streamType, bool &active) override;

    int32_t IsStreamActiveByStreamUsage(int32_t streamUsage, bool &active) override;

    int32_t IsFastPlaybackSupported(const AudioStreamInfo &streamInfo, int32_t usage, bool &support) override;
    int32_t IsFastRecordingSupported(const AudioStreamInfo &streamInfo, int32_t source, bool &support) override;

    int32_t IsVolumeUnadjustable(bool &unadjustable) override;

    int32_t AdjustVolumeByStep(int32_t adjustType) override;

    int32_t AdjustSystemVolumeByStep(int32_t volumeType, int32_t adjustType) override;

    int32_t GetSystemVolumeInDb(int32_t volumeType, int32_t volumeLevel,
        int32_t deviceType, float &volume) override;

    bool IsArmUsbDevice(const AudioDeviceDescriptor &desc);

    void MapExternalToInternalDeviceType(AudioDeviceDescriptor &desc);

    int32_t SelectOutputDevice(const sptr<AudioRendererFilter> &audioRendererFilter,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) override;

    int32_t GetSelectedDeviceInfo(int32_t uid, int32_t pid, int32_t streamType, std::string &info) override;

    int32_t SelectInputDevice(const sptr<AudioCapturerFilter> &audioCapturerFilter,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) override;

    int32_t ExcludeOutputDevices(int32_t audioDevUsage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) override;

    int32_t UnexcludeOutputDevices(int32_t audioDevUsage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) override;

    int32_t GetExcludedDevices(int32_t audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) override;

    int32_t GetDevices(int32_t deviceFlag, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs) override;

    int32_t GetDevicesInner(int32_t deviceFlag,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs) override;

    int32_t SetDeviceActive(int32_t deviceType, bool active, int32_t uid) override;

    int32_t IsDeviceActive(int32_t deviceType, bool &active) override;

    int32_t GetActiveOutputDevice(int32_t &deviceType) override;

    int32_t GetDmDeviceType(uint16_t &deviceType) override;

    int32_t GetActiveInputDevice(int32_t &deviceType) override;

    int32_t SetRingerModeLegacy(int32_t ringMode) override;

    int32_t SetRingerMode(int32_t ringMode) override;

#ifdef FEATURE_DTMF_TONE
    int32_t GetSupportedTones(const std::string &countryCode, std::vector<int32_t> &throw_with_nested) override;

    int32_t GetToneConfig(int32_t ltonetype, const std::string &countryCode,
        std::shared_ptr<ToneInfo> &config) override;
#endif

    int32_t GetRingerMode(int32_t &ringMode) override;

    int32_t SetAudioScene(int32_t audioScene) override;

    int32_t SetMicrophoneMuteCommon(bool isMute, bool isLegacy);

    int32_t SetMicrophoneMute(bool isMute) override;

    int32_t SetMicrophoneMuteAudioConfig(bool isMute) override;

    int32_t SetMicrophoneMutePersistent(bool isMute, int32_t type) override;

    int32_t GetPersistentMicMuteState(bool &mute) override;

    int32_t IsMicrophoneMuteLegacy(bool &mute) override;

    int32_t IsMicrophoneMute(bool &mute) override;

    int32_t GetAudioScene(int32_t &scene) override;

    int32_t ActivateAudioSession(int32_t strategy) override;

    int32_t DeactivateAudioSession() override;

    int32_t IsAudioSessionActivated(bool &active) override;

    int32_t SetAudioSessionScene(int32_t audioSessionScene) override;

    int32_t GetDefaultOutputDevice(int32_t &deviceType) override;

    int32_t SetDefaultOutputDevice(int32_t deviceType) override;

    int32_t SetAudioInterruptCallback(uint32_t sessionID,
        const sptr<IRemoteObject> &object, uint32_t clientUid, int32_t zoneId) override;

    int32_t UnsetAudioInterruptCallback(uint32_t sessionID, int32_t zoneId) override;

    int32_t ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt, int32_t zoneId,
        bool isUpdatedAudioStrategy) override;

    int32_t SetAppConcurrencyMode(const int32_t appUid, const int32_t mode = 0) override;

    int32_t SetAppSlientOnDisplay(const int32_t displayId = -1) override;

    int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt, int32_t zoneId) override;

    int32_t SetAudioRouteCallback(uint32_t sessionId, const sptr<IRemoteObject> &object, uint32_t clientUid) override;

    int32_t UnsetAudioRouteCallback(uint32_t sessionId) override;

    int32_t ActivatePreemptMode(void) override;

    int32_t DeactivatePreemptMode(void) override;

    int32_t SetAudioManagerInterruptCallback(int32_t clientId, const sptr<IRemoteObject> &object) override;

    int32_t UnsetAudioManagerInterruptCallback(int32_t clientId) override;

    int32_t SetQueryClientTypeCallback(const sptr<IRemoteObject> &object) override;

    int32_t SetQueryDeviceVolumeBehaviorCallback(const sptr<IRemoteObject> &object) override;

    int32_t SetAudioClientInfoMgrCallback(const sptr<IRemoteObject> &object) override;

    int32_t SetAudioVKBInfoMgrCallback(const sptr<IRemoteObject> &object) override;

    int32_t CheckVKBInfo(const std::string &bundleName, bool &isValid) override;

    int32_t SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object) override;

    int32_t RequestAudioFocus(int32_t clientId, const AudioInterrupt &audioInterrupt) override;

    int32_t AbandonAudioFocus(int32_t clientId, const AudioInterrupt &audioInterrupt) override;

    int32_t GetStreamInFocus(int32_t zoneId, int32_t &streamType) override;

    int32_t GetStreamInFocusByUid(int32_t uid, int32_t zoneId, int32_t &streamType) override;

    int32_t GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, int32_t zoneId) override;

    void OnAudioStreamRemoved(const uint64_t sessionID) override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    int32_t GetPreferredOutputStreamType(const AudioRendererInfo &rendererInfo, int32_t &streamType) override;

    int32_t GetPreferredInputStreamType(const AudioCapturerInfo &capturerInfo, int32_t &streamType) override;

    int32_t CreateRendererClient(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        uint32_t &flag, uint32_t &sessionId, std::string &networkId) override;

    int32_t CreateCapturerClient(
        const std::shared_ptr<AudioStreamDescriptor> &streamDesc, uint32_t &flag, uint32_t &sessionId) override;

    int32_t RegisterTracker(int32_t mode, const AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object) override;

    int32_t UpdateTracker(int32_t mode, const AudioStreamChangeInfo &streamChangeInfo) override;

    int32_t GetCurrentRendererChangeInfos(
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;

    int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;

    void RegisterClientDeathRecipient(const sptr<IRemoteObject> &object, DeathRecipientId id);

    void RegisteredTrackerClientDied(int pid, int uid);

    void RegisteredStreamListenerClientDied(int pid, int uid);

    int32_t ResumeStreamState();

    int32_t UpdateStreamState(int32_t clientUid, int32_t streamSetState,
        int32_t streamUsage) override;

    int32_t GetVolumeGroupInfos(const std::string &networkId, std::vector<sptr<VolumeGroupInfo>> &infos) override;

    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) override;
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) override;

    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray) override;
    int32_t GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray) override;
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) override;
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray) override;
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray) override;

    int32_t IsAcousticEchoCancelerSupported(int32_t sourceType, bool &ret) override;
    int32_t IsAudioLoopbackSupported(int32_t mode, bool &ret) override;
    int32_t SetKaraokeParameters(const std::string &parameters, bool &ret) override;

    int32_t GetNetworkIdByGroupId(int32_t groupId, std::string &networkId) override;

    int32_t GetPreferredOutputDeviceDescriptors(const AudioRendererInfo &rendererInfo, bool forceNoBTPermission,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs) override;

    int32_t GetPreferredInputDeviceDescriptors(const AudioCapturerInfo &captureInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs) override;

    int32_t GetOutputDevice(const sptr<AudioRendererFilter> &audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs) override;

    int32_t GetInputDevice(const sptr<AudioCapturerFilter> &audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs) override;

    int32_t SetClientCallbacksEnable(int32_t callbackchange, bool enable) override;

    int32_t SetCallbackRendererInfo(const AudioRendererInfo &rendererInfo) override;

    int32_t SetCallbackCapturerInfo(const AudioCapturerInfo &capturerInfo) override;

    int32_t GetAudioFocusInfoList(std::vector<std::map<AudioInterrupt, int32_t>> &focusInfoList,
        int32_t zoneId) override;

    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri) override;

    int32_t GetSystemSoundUri(const std::string &key, std::string &uri) override;

    int32_t GetMinStreamVolume(float &volume) override;

    int32_t GetMaxStreamVolume(float &volume) override;

    int32_t GetMaxRendererInstances(int32_t &ret) override;

    void GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos);

    int32_t QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig) override;

    int32_t GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc,
        int32_t &ret) override;

    int32_t GetAudioCapturerMicrophoneDescriptors(int32_t sessionId,
        std::vector<sptr<MicrophoneDescriptor>> &micDescs) override;

    int32_t GetAvailableMicrophones(std::vector<sptr<MicrophoneDescriptor>> &retMicList) override;

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support) override;

    int32_t IsAbsVolumeScene(bool &ret) override;

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, int32_t volume, bool updateUi) override;

    int32_t SetNearlinkDeviceVolume(const std::string &macAddress, int32_t volumeType,
        int32_t volume, bool updateUi) override;

    int32_t GetAvailableDevices(int32_t usage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs) override;

    int32_t SetAvailableDeviceChangeCallback(int32_t /*clientId*/, int32_t usage,
        const sptr<IRemoteObject> &object) override;

    int32_t UnsetAvailableDeviceChangeCallback(int32_t clientId, int32_t usage) override;

    int32_t IsSpatializationEnabled(bool &ret) override;

    int32_t IsSpatializationEnabled(const std::string &address, bool &ret) override;

    int32_t IsSpatializationEnabledForCurrentDevice(bool &ret) override;

    int32_t SetSpatializationEnabled(const bool enable) override;

    int32_t SetSpatializationEnabled(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice,
        const bool enable) override;

    int32_t IsHeadTrackingEnabled(bool &ret) override;

    int32_t IsHeadTrackingEnabled(const std::string &address, bool &ret) override;

    int32_t SetHeadTrackingEnabled(const bool enable) override;

    int32_t SetHeadTrackingEnabled(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable) override;

    int32_t GetSpatializationState(int32_t streamUsage, AudioSpatializationState &state) override;

    int32_t IsSpatializationSupported(bool &ret) override;

    int32_t IsSpatializationSupportedForDevice(const std::string &address, bool &ret) override;

    int32_t IsHeadTrackingSupported(bool &ret) override;

    int32_t IsHeadTrackingSupportedForDevice(const std::string &address, bool &ret) override;

    int32_t UpdateSpatialDeviceState(const AudioSpatialDeviceState &audioSpatialDeviceState) override;

    int32_t RegisterSpatializationStateEventListener(uint32_t sessionID, int32_t streamUsage,
        const sptr<IRemoteObject> &object) override;

    int32_t ConfigDistributedRoutingRole(
        const std::shared_ptr<AudioDeviceDescriptor> &descriptor, int32_t type) override;

    int32_t SetDistributedRoutingRoleCallback(const sptr<IRemoteObject> &object) override;

    int32_t UnsetDistributedRoutingRoleCallback() override;

    int32_t UnregisterSpatializationStateEventListener(uint32_t sessionID) override;

    int32_t RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, int32_t zoneId) override;

    int32_t CreateAudioInterruptZone(const std::set<int32_t> &pids, int32_t zoneId) override;

    int32_t AddAudioInterruptZonePids(const std::set<int32_t> &pids, int32_t zoneId) override;

    int32_t RemoveAudioInterruptZonePids(const std::set<int32_t> &pids, int32_t zoneId) override;

    int32_t ReleaseAudioInterruptZone(int32_t zoneId) override;

    int32_t RegisterAudioZoneClient(const sptr<IRemoteObject> &object) override;

    int32_t CreateAudioZone(const std::string &name, const AudioZoneContext &context, int32_t &zoneId,
        int32_t pid) override;

    int32_t ReleaseAudioZone(int32_t zoneId) override;

    int32_t GetAllAudioZone(std::vector<std::shared_ptr<AudioZoneDescriptor>> &descs) override;

    int32_t GetAudioZone(int32_t zoneId, std::shared_ptr<AudioZoneDescriptor> &desc) override;

    int32_t GetAudioZoneByName(const std::string &name, int32_t &zoneId) override;

    int32_t BindDeviceToAudioZone(int32_t zoneId,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices) override;

    int32_t UnBindDeviceToAudioZone(int32_t zoneId,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devices) override;

    int32_t EnableAudioZoneReport (bool enable) override;

    int32_t EnableAudioZoneChangeReport(int32_t zoneId, bool enable) override;

    int32_t AddUidToAudioZone(int32_t zoneId, int32_t uid) override;

    int32_t RemoveUidFromAudioZone(int32_t zoneId, int32_t uid) override;

    int32_t EnableSystemVolumeProxy(int32_t zoneId, bool enable) override;

    int32_t AddStreamToAudioZone(int32_t zoneId, const AudioZoneStream &stream) override;

    int32_t AddStreamsToAudioZone(int32_t zoneId, const std::vector<AudioZoneStream> &streams) override;

    int32_t RemoveStreamFromAudioZone(int32_t zoneId, const AudioZoneStream &stream) override;

    int32_t RemoveStreamsFromAudioZone(int32_t zoneId, const std::vector<AudioZoneStream> &streams) override;

    int32_t SetZoneDeviceVisible(bool visible) override;

    int32_t GetAudioInterruptForZone(int32_t zoneId,
        std::vector<std::map<AudioInterrupt, int32_t>> &retList) override;

    int32_t GetAudioInterruptForZone(int32_t zoneId, const std::string &deviceTag,
        std::vector<std::map<AudioInterrupt, int32_t>> &retList) override;

    int32_t EnableAudioZoneInterruptReport(int32_t zoneId, const std::string &deviceTag, bool enable) override;

    int32_t InjectInterruptToAudioZone(int32_t zoneId,
        const std::vector<std::map<AudioInterrupt, int32_t>> &interrupts) override;

    int32_t InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
        const std::vector<std::map<AudioInterrupt, int32_t>> &interrupts) override;

    int32_t SetCallDeviceActive(int32_t deviceType, bool active, const std::string &address,
        int32_t uid) override;

    int32_t GetActiveBluetoothDevice(std::shared_ptr<AudioDeviceDescriptor> &descs) override;

    int32_t GetConverterConfig(ConverterConfig &cfg) override;

    int32_t FetchOutputDeviceForTrack(const AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt &reason) override;

    int32_t FetchInputDeviceForTrack(const AudioStreamChangeInfo &streamChangeInfo) override;

    int32_t GetSpatializationSceneType(int32_t &type) override;

    int32_t SetSpatializationSceneType(int32_t spatializationSceneType) override;

    int32_t GetMaxAmplitude(int32_t deviceId, float &ret) override;

    int32_t DisableSafeMediaVolume() override;

    int32_t IsHeadTrackingDataRequested(const std::string &macAddress, bool &ret) override;

    int32_t SetAudioDeviceRefinerCallback(const sptr<IRemoteObject> &object) override;

    int32_t UnsetAudioDeviceRefinerCallback() override;

    int32_t TriggerFetchDevice(
        const AudioStreamDeviceChangeReasonExt &reason) override;

    int32_t SetPreferredDevice(int32_t preferredType,
        const std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid) override;

    int32_t SetDeviceVolumeBehavior(const std::string &networkId, int32_t deviceType,
        const VolumeBehavior &volumeBehavior) override;

    int32_t SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object) override;

    int32_t UnsetAudioDeviceAnahsCallback() override;

    int32_t MoveToNewPipe(uint32_t sessionId, int32_t pipeType) override;

    int32_t InjectInterruption(const std::string &networkId, const InterruptEvent &event) override;

    int32_t SetInputDevice(int32_t deviceType, uint32_t sessionID, int32_t sourceType, bool isRunning) override;

    int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId) override;

    int32_t IsAllowedPlayback(int32_t uid, int32_t pid, bool &isAllowed) override;

    int32_t SetVoiceRingtoneMute(bool isMute) override;

    int32_t NotifySessionStateChange(int32_t uid, int32_t pid, bool hasSession) override;

    int32_t NotifyFreezeStateChange(const std::set<int32_t> &pidList, bool isFreeze) override;

    int32_t ResetAllProxy() override;

    int32_t NotifyProcessBackgroundState(int32_t uid, int32_t pid) override;

    int32_t SetVirtualCall(bool isVirtual) override;

    int32_t SetDeviceConnectionStatus(const std::shared_ptr<AudioDeviceDescriptor> &desc, bool isConnected) override;

    int32_t SetQueryAllowedPlaybackCallback(const sptr<IRemoteObject> &object) override;

    int32_t SetBackgroundMuteCallback(const sptr<IRemoteObject> &object) override;

    int32_t GetDirectPlaybackSupport(const AudioStreamInfo &streamInfo, int32_t streamUsage, int32_t &retMod) override;

    int32_t GetMaxVolumeLevelByUsage(int32_t streamUsage, int32_t &retMaxVolumeLevel) override;

    int32_t GetMinVolumeLevelByUsage(int32_t streamUsage, int32_t &retMinVolumeLevel) override;

    int32_t GetVolumeLevelByUsage(int32_t streamUsage, int32_t &retVolumeLevel) override;

    int32_t GetStreamMuteByUsage(int32_t streamUsage, bool &isMute) override;

    int32_t GetVolumeInDbByStream(int32_t streamUsage, int32_t volumeLevel,
        int32_t deviceType, float &ret) override;

    int32_t GetSupportedAudioVolumeTypes(std::vector<int32_t> &ret) override;

    int32_t GetAudioVolumeTypeByStreamUsage(int32_t streamUsage, int32_t &volumeType) override;

    int32_t GetStreamUsagesByVolumeType(int32_t audioVolumeType, std::vector<int32_t> &ret) override;

    int32_t SetCallbackStreamUsageInfo(const std::set<int32_t> &streamUsages) override;

    int32_t ForceStopAudioStream(int32_t audioType) override;

    int32_t IsCapturerFocusAvailable(const AudioCapturerInfo &capturerInfo, bool &ret) override;

    int32_t ForceVolumeKeyControlType(int32_t volumeType, int32_t duration, int32_t &ret) override;

    void ProcessRemoteInterrupt(std::set<int32_t> sessionIds, InterruptEventInternal interruptEvent);
    std::set<int32_t> GetStreamIdsForAudioSessionByStreamUsage(
        const int32_t zoneId, const std::set<StreamUsage> &streamUsageSet);

    void SendVolumeKeyEventCbWithUpdateUiOrNot(AudioStreamType streamType, const bool& isUpdateUi = false,
        int32_t zoneId = 0);
    void SendMuteKeyEventCbWithUpdateUiOrNot(AudioStreamType streamType, const bool& isUpdateUi = false,
        int32_t zoneId = 0);
    void UpdateMuteStateAccordingToVolLevel(AudioStreamType streamType, int32_t volumeLevel,
        bool mute, const bool& isUpdateUi = false, int32_t zoneId = 0);

    void ProcUpdateRingerMode();
    uint32_t TranslateErrorCode(int32_t result);

    int32_t SetCollaborativePlaybackEnabledForDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool enabled) override;
    
    int32_t IsCollaborativePlaybackEnabledForDevice(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, bool &enabled) override;

    int32_t IsCollaborativePlaybackSupported(bool &ret) override;

    class RemoteParameterCallback : public AudioParameterCallback {
    public:
        RemoteParameterCallback(sptr<AudioPolicyServer> server);
        // AudioParameterCallback
        void OnAudioParameterChange(const std::string networkId, const AudioParamKey key, const std::string &condition,
            const std::string &value) override;
    private:
        sptr<AudioPolicyServer> server_;
        void VolumeOnChange(const std::string networkId, const std::string &condition);
        void InterruptOnChange(const std::string networkId, const std::string &condition);
        void StateOnChange(const std::string networkId, const std::string &condition, const std::string &value);
    };

    std::shared_ptr<RemoteParameterCallback> remoteParameterCallback_;

    class PerStateChangeCbCustomizeCallback : public Security::AccessToken::PermStateChangeCallbackCustomize {
    public:
        explicit PerStateChangeCbCustomizeCallback(const Security::AccessToken::PermStateChangeScope &scopeInfo,
            sptr<AudioPolicyServer> server) : PermStateChangeCallbackCustomize(scopeInfo),
            ready_(false), server_(server) {}
        ~PerStateChangeCbCustomizeCallback() {}

        void PermStateChangeCallback(Security::AccessToken::PermStateChangeInfo &result);
        void UpdateMicPrivacyByCapturerState(bool targetMuteState, uint32_t targetTokenId, int32_t appUid);

        bool ready_;
    private:
        sptr<AudioPolicyServer> server_;
    };

    int32_t IsHighResolutionExist(bool &ret) override;

    int32_t SetHighResolutionExist(bool highResExist) override;

    void NotifyAccountsChanged(const int &id);
    void SendVolumeKeyEventToRssWhenAccountsChanged();

    // for hidump
    void AudioDevicesDump(std::string &dumpString);
    void AudioModeDump(std::string &dumpString);
    void AudioInterruptZoneDump(std::string &dumpString);
    void AudioPolicyParserDump(std::string &dumpString);
    void AudioVolumeDump(std::string &dumpString);
    void AudioStreamDump(std::string &dumpString);
    void OffloadStatusDump(std::string &dumpString);
    void XmlParsedDataMapDump(std::string &dumpString);
    void EffectManagerInfoDump(std::string &dumpString);
    void MicrophoneMuteInfoDump(std::string &dumpString);
    void AudioSessionInfoDump(std::string &dumpString);
    void AudioPipeManagerDump(std::string &dumpString);
    void SelectDeviceDump(std::string &dumpString);

    // for hibernate callback
    void CheckHibernateState(bool hibernate);
    // for S4 reboot update safevolume
    void UpdateSafeVolumeByS4();

    void CheckConnectedDevice();
    void SetDeviceConnectedFlagFalseAfterDuration();

    int32_t UpdateDeviceInfo(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, int32_t command) override;
    int32_t SetSleAudioOperationCallback(const sptr<IRemoteObject> &object) override;
    int32_t CallRingtoneLibrary();
    int32_t SetSystemVolumeDegree(int32_t streamType, int32_t volumeDegree, int32_t volumeFlag, int32_t uid) override;
    int32_t GetSystemVolumeDegree(int32_t streamType, int32_t uid, int32_t &volumeDegree) override;
    int32_t GetMinVolumeDegree(int32_t volumeType, int32_t &volumeDegree) override;
protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    void RegisterParamCallback();

    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) override;
    int32_t GetApiTargetVersion();

private:
    int32_t SetSystemVolumeDegreeInner(AudioStreamType streamType, int32_t volumeDegree,
        bool isUpdateUi, int32_t uid);

    friend class AudioInterruptService;

    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t VOLUME_CHANGE_FACTOR = 1;
    static constexpr int32_t VOLUME_KEY_DURATION = 0;
    static constexpr int32_t VOLUME_MUTE_KEY_DURATION = 0;
    static constexpr int32_t MEDIA_SERVICE_UID = 1013;
    static constexpr int32_t EDM_SERVICE_UID = 3057;
    static constexpr char DAUDIO_DEV_TYPE_SPK = '1';
    static constexpr char DAUDIO_DEV_TYPE_MIC = '2';
    static constexpr int32_t AUDIO_UID = 1041;
    static constexpr uint32_t MICPHONE_CALLER = 0;
    static constexpr int32_t ROOT_UID = 0;
    static constexpr int32_t PREEMPT_UID = 7015;

    static const std::list<uid_t> RECORD_ALLOW_BACKGROUND_LIST;
    static const std::list<uid_t> RECORD_PASS_APPINFO_LIST;
    static constexpr const char* MICROPHONE_CONTROL_PERMISSION = "ohos.permission.MICROPHONE_CONTROL";

    class AudioPolicyServerPowerStateCallback : public PowerMgr::PowerStateCallbackStub {
    public:
        AudioPolicyServerPowerStateCallback(AudioPolicyServer *policyServer);
        void OnAsyncPowerStateChanged(PowerMgr::PowerState state) override;

    private:
        AudioPolicyServer *policyServer_;
    };

    int32_t VerifyVoiceCallPermission(uint64_t fullTokenId, Security::AccessToken::AccessTokenID tokenId);

    // offload session
    void CheckSubscribePowerStateChange();
    bool CheckAudioSessionStrategy(const AudioSessionStrategy &sessionStrategy);

    // for audio volume and mute status
    int32_t SetRingerModeInternal(AudioRingerMode inputRingerMode, bool hasUpdatedVolume = false);
    int32_t SetSystemVolumeLevelInternal(AudioStreamType streamType, int32_t volumeLevel,
        bool isUpdateUi, int32_t zoneId = 0);
    int32_t SetAppVolumeLevelInternal(int32_t appUid, int32_t volumeLevel, bool isUpdateUi);
    int32_t SetAppVolumeMutedInternal(int32_t appUid, bool muted, bool isUpdateUi);
    int32_t SetAppRingMutedInternal(int32_t appUid, bool muted);
    int32_t SetSystemVolumeLevelWithDeviceInternal(AudioStreamType streamType, int32_t volumeLevel,
        bool isUpdateUi, DeviceType deviceType);
    int32_t SetSingleStreamVolume(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi,
        bool mute, int32_t zoneId = 0);
    int32_t SetAppSingleStreamVolume(int32_t streamType, int32_t volumeLevel, bool isUpdateUi);
    int32_t SetSingleStreamVolumeWithDevice(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi,
        DeviceType deviceType);
    AudioStreamType GetSystemActiveVolumeTypeInternal(const int32_t clientUid);
    int32_t GetSystemVolumeLevelInternal(AudioStreamType streamType, int32_t zoneId = 0);
    int32_t GetAppVolumeLevelInternal(int32_t appUid, int32_t &volumeLevel);
    int32_t GetSystemVolumeLevelNoMuteState(AudioStreamType streamType);
    float GetSystemVolumeDb(AudioStreamType streamType);
    int32_t SetStreamMuteInternal(AudioStreamType streamType, bool mute, bool isUpdateUi,
        const DeviceType &deviceType = DEVICE_TYPE_NONE, int32_t zoneId = 0);
    void UpdateSystemMuteStateAccordingMusicState(AudioStreamType streamType, bool mute, bool isUpdateUi);
    void ProcUpdateRingerModeForMute(bool updateRingerMode, bool mute);
    int32_t SetSingleStreamMute(AudioStreamType streamType, bool mute, bool isUpdateUi,
        const DeviceType &deviceType = DEVICE_TYPE_NONE, int32_t zoneId = 0);
    bool GetStreamMuteInternal(AudioStreamType streamType, int32_t zoneId = 0);
    bool IsVolumeTypeValid(AudioStreamType streamType);
    bool IsVolumeLevelValid(AudioStreamType streamType, int32_t volumeLevel);
    bool IsRingerModeValid(AudioRingerMode ringMode);
    bool CheckCanMuteVolumeTypeByStep(AudioVolumeType volumeType, int32_t volumeLevel);

    // Permission and privacy
    bool VerifyPermission(const std::string &permission, uint32_t tokenId = 0, bool isRecording = false);
    bool VerifyBluetoothPermission();
    int32_t OffloadStopPlaying(const AudioInterrupt &audioInterrupt);
    int32_t SetAudioSceneInternal(AudioScene audioScene, const int32_t uid = INVALID_UID,
        const int32_t pid = INVALID_PID);
    bool VerifySessionId(uint32_t sessionId, uint32_t clientUid);

    // externel function call
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    bool MaxOrMinVolumeOption(const int32_t &volLevel, const int32_t keyType, const AudioStreamType &streamInFocus);
    int32_t RegisterVolumeKeyEvents(const int32_t keyType);
    int32_t RegisterVolumeKeyMuteEvents();
    void SubscribeVolumeKeyEvents();
    bool IsContinueAddVol();
    void TriggerMuteCheck();
    int32_t ProcessVolumeKeyEvents(const int32_t keyType);
#endif
    void AddAudioServiceOnStart();
    void SubscribeOsAccountChangeEvents();
    void SubscribePowerStateChangeEvents();
    void SubscribeCommonEvent(const std::string event);
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData);
    void HandleKvDataShareEvent();
    void InitMicrophoneMute();
    void InitKVStore();
    void NotifySettingsDataReady();
    void ConnectServiceAdapter();
    void LoadEffectLibrary();
    void RegisterBluetoothListener();
    void SubscribeAccessibilityConfigObserver();
    void RegisterDataObserver();
    void RegisterPowerStateListener();
    void UnRegisterPowerStateListener();
    void RegisterSyncHibernateListener();
    void UnRegisterSyncHibernateListener();
    void RegisterAppStateListener();
    void AddRemoteDevstatusCallback();
    void OnDistributedRoutingRoleChange(const std::shared_ptr<AudioDeviceDescriptor> descriptor, const CastType type);
    void SubscribeSafeVolumeEvent();
    void SubscribeCommonEventExecute();
    void SubscribeBackgroundTask();
    void SendMonitrtEvent(const int32_t keyType, int32_t resultOfVolumeKey);
    void RegisterDefaultVolumeTypeListener();
    int32_t SetVolumeInternalByKeyEvent(AudioStreamType streamInFocus, int32_t zoneId, const int32_t keyType);

    void InitPolicyDumpMap();
    void PolicyDataDump(std::string &dumpString);
    void ArgInfoDump(std::string &dumpString, std::queue<std::u16string> &argQue);
    void InfoDumpHelp(std::string &dumpString);

    int32_t SetRingerModeInner(AudioRingerMode ringMode);
    void AddSystemAbilityListeners();
    void OnAddSystemAbilityExtract(int32_t systemAbilityId, const std::string &deviceId);

    // for updating default device selection state when game audio stream is muted
    void UpdateDefaultOutputDeviceWhenStarting(const uint32_t sessionID);
    void UpdateDefaultOutputDeviceWhenStopping(const uint32_t sessionID);
    void ChangeVolumeOnVoiceAssistant(AudioStreamType &streamInFocus);
    AudioStreamType GetCurrentStreamInFocus();

    AudioEffectService &audioEffectService_;
    AudioAffinityManager &audioAffinityManager_;
    AudioCapturerSession &audioCapturerSession_;
    AudioStateManager &audioStateManager_;
    AudioToneManager &audioToneManager_;
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor_;
    AudioDeviceStatus &audioDeviceStatus_;
    AudioPolicyConfigManager &audioConfigManager_;
    AudioSceneManager &audioSceneManager_;
    AudioConnectedDevice &audioConnectedDevice_;
    AudioDeviceLock &audioDeviceLock_;
    AudioStreamCollector &streamCollector_;
    AudioOffloadStream &audioOffloadStream_;
    AudioBackgroundManager &audioBackgroundManager_;
    AudioVolumeManager &audioVolumeManager_;
    AudioDeviceCommon &audioDeviceCommon_;
    IAudioPolicyInterface &audioPolicyManager_;
    AudioPolicyConfigManager &audioPolicyConfigManager_;
    AudioPolicyService &audioPolicyService_;
    AudioPolicyUtils &audioPolicyUtils_;
    AudioDeviceManager &audioDeviceManager_;
    AudioSpatializationService &audioSpatializationService_;
    AudioCollaborativeService &audioCollaborativeService_;
    AudioRouterCenter &audioRouterCenter_;
    AudioPolicyDump &audioPolicyDump_;
#ifdef USB_ENABLE
    AudioUsbManager &usbManager_;
#endif
    AudioActiveDevice &audioActiveDevice_;

    std::shared_ptr<AudioInterruptService> interruptService_;
    std::shared_ptr<AudioCoreService> coreService_;
    std::shared_ptr<AudioCoreService::EventEntry> eventEntry_;

    int32_t volumeStep_;
    std::atomic<bool> isFirstAudioServiceStart_ = false;
    std::atomic<bool> isInitMuteState_ = false;
    std::atomic<bool> isInitSettingsData_ = false;
    std::atomic<bool> isScreenOffOrLock_ = false;
    std::atomic<bool> isInitRingtoneReady_ = false;
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    std::mutex volUpHistoryMutex_;
    std::deque<int64_t> volUpHistory_;
    std::atomic<bool> hasSubscribedVolumeKeyEvents_ = false;
#endif
    std::vector<pid_t> clientDiedListenerState_;
    sptr<PowerStateListener> powerStateListener_;
    sptr<SyncHibernateListener> syncHibernateListener_;
    bool powerStateCallbackRegister_;
    AppExecFwk::AppMgrClient appManager_;
    sptr<AppStateListener> appStateListener_;

    std::mutex systemVolumeMutex_;
    std::mutex micStateChangeMutex_;
    std::mutex clientDiedListenerStateMutex_;
    std::mutex subscribeVolumeKey_;

    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    bool volumeApplyToAll_ = false;
    bool screenOffAdjustVolumeEnable_ = false;
    bool supportVibrator_ = false;

    bool isHighResolutionExist_ = false;
    std::mutex descLock_;

    using DumpFunc = void(AudioPolicyServer::*)(std::string &dumpString);
    std::map<std::u16string, DumpFunc> dumpFuncMap;
    pid_t lastMicMuteSettingPid_ = 0;
    std::shared_ptr<AudioOsAccountInfo> accountObserver_ = nullptr;

    int32_t sessionIdByRemote_ = -1;
    sptr<IStandardAudioPolicyManagerListener> queryBundleNameListCallback_ = nullptr;
    bool isAlreadyRegisterCommonEventListener_ = false;
};

class AudioOsAccountInfo : public AccountSA::OsAccountSubscriber {
public:
    explicit AudioOsAccountInfo(const AccountSA::OsAccountSubscribeInfo &subscribeInfo,
        AudioPolicyServer *audioPolicyServer) : AccountSA::OsAccountSubscriber(subscribeInfo),
        audioPolicyServer_(audioPolicyServer) {}

    ~AudioOsAccountInfo()
    {
        AUDIO_WARNING_LOG("Destructor AudioOsAccountInfo");
    }

    void OnAccountsChanged(const int &id) override
    {
        AUDIO_INFO_LOG("OnAccountsChanged received, id: %{public}d", id);
    }

    void OnAccountsSwitch(const int &newId, const int &oldId) override
    {
        CHECK_AND_RETURN_LOG(oldId >= LOCAL_USER_ID, "invalid id");
        AUDIO_INFO_LOG("OnAccountsSwitch received, newid: %{public}d, oldid: %{public}d", newId, oldId);
        if (audioPolicyServer_ != nullptr) {
            audioPolicyServer_->NotifyAccountsChanged(newId);
        }
    }
private:
    static constexpr int32_t LOCAL_USER_ID = 100;
    AudioPolicyServer *audioPolicyServer_;
};

class AudioCommonEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit AudioCommonEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        std::function<void(const EventFwk::CommonEventData&)> receiver)
        : EventFwk::CommonEventSubscriber(subscribeInfo), eventReceiver_(receiver) {}
    ~AudioCommonEventSubscriber() {}
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
private:
    AudioCommonEventSubscriber() = default;
    std::function<void(const EventFwk::CommonEventData&)> eventReceiver_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_SERVER_H
