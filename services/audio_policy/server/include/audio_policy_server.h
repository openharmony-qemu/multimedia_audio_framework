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

#include "bundle_mgr_interface.h"
#include "bundle_mgr_proxy.h"

#include "audio_info.h"
#include "audio_policy_service.h"
#include "audio_session_callback.h"
#include "audio_interrupt_callback.h"
#include "audio_policy_manager_stub.h"
#include "audio_policy_client_proxy.h"
#include "audio_server_death_recipient.h"
#include "audio_service_dump.h"
#include "session_processor.h"
#include "audio_spatialization_service.h"
#include "audio_policy_server_handler.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint64_t DSTATUS_SESSION_ID = 4294967296;
constexpr uint32_t DSTATUS_DEFAULT_RATE = 48000;
class AudioPolicyServer : public SystemAbility, public AudioPolicyManagerStub, public AudioSessionCallback {
    DECLARE_SYSTEM_ABILITY(AudioPolicyServer);
public:
    DISALLOW_COPY_AND_MOVE(AudioPolicyServer);

    enum DeathRecipientId {
        TRACKER_CLIENT = 0,
        LISTENER_CLIENT
    };

    enum SpatializationEventCategory {
        SPATIALIZATION_ENABLED_CHANGE_EVENT,
        HEAD_TRACKING_ENABLED_CHANGE_EVENT,
    };

    const std::vector<AudioStreamType> GET_STREAM_ALL_VOLUME_TYPES {
        STREAM_MUSIC,
        STREAM_VOICE_CALL,
        STREAM_RING,
        STREAM_VOICE_ASSISTANT,
        STREAM_ALARM,
        STREAM_ACCESSIBILITY,
        STREAM_ULTRASONIC
    };

    explicit AudioPolicyServer(int32_t systemAbilityId, bool runOnCreate = true);

    virtual ~AudioPolicyServer()
    {
        AUDIO_WARNING_LOG("AudioPolicyServer destroyed");
    };

    typedef struct {
        int32_t zoneID; // Zone ID value should 0 on local device.
        std::set<int32_t> pids; // When Zone ID is 0, there does not need to be a value.
        std::unordered_map<uint32_t /* sessionID */, std::shared_ptr<AudioInterruptCallback>> interruptCbsMap;
        std::unordered_map<int32_t /* clientPid */, sptr<IAudioPolicyClient>> audioPolicyClientProxyCBMap;
        std::list<std::pair<AudioInterrupt, AudioFocuState>> audioFocusInfoList;
    } AudioInterruptZone;

    void OnDump() override;
    void OnStart() override;
    void OnStop() override;

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) override;

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType) override;

    int32_t SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel, API_VERSION api_v = API_9) override;

    int32_t GetSystemVolumeLevel(AudioStreamType streamType) override;

    int32_t SetLowPowerVolume(int32_t streamId, float volume) override;

    float GetLowPowerVolume(int32_t streamId) override;

    float GetSingleStreamVolume(int32_t streamId) override;

    int32_t SetStreamMute(AudioStreamType streamType, bool mute, API_VERSION api_v = API_9) override;

    bool GetStreamMute(AudioStreamType streamType) override;

    bool IsStreamActive(AudioStreamType streamType) override;

    bool IsVolumeUnadjustable() override;

    int32_t AdjustVolumeByStep(VolumeAdjustType adjustType) override;

    int32_t AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType) override;

    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) override;

    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) override;

    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) override;

    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) override;

    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag) override;

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint32_t sessionId) override;

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active) override;

    bool IsDeviceActive(InternalDeviceType deviceType) override;

    InternalDeviceType GetActiveOutputDevice() override;

    InternalDeviceType GetActiveInputDevice() override;

    int32_t SetRingerMode(AudioRingerMode ringMode, API_VERSION api_v = API_9) override;

#ifdef FEATURE_DTMF_TONE
    std::vector<int32_t> GetSupportedTones() override;

    std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype) override;
#endif

    AudioRingerMode GetRingerMode() override;

    int32_t SetAudioScene(AudioScene audioScene) override;

    int32_t SetMicrophoneMuteCommon(bool isMute, API_VERSION api_v);

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

    void OnSessionRemoved(const uint64_t sessionID) override;

    void ProcessSessionRemoved(const uint64_t sessionID, const int32_t zoneID = 0);

    void ProcessSessionAdded(SessionEvent sessionEvent);

    void OnCapturerSessionAdded(const uint64_t sessionID, SessionInfo sessionInfo) override;

    void OnPlaybackCapturerStop() override;

    void OnWakeupCapturerStop(uint32_t sessionID) override;

    void ProcessorCloseWakeupSource(const uint64_t sessionID);

    void OnDstatusUpdated(bool isConnected) override;

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        SourceType sourceType = SOURCE_TYPE_MIC) override;

    bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state) override;

    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType) override;

    int32_t GetAudioLatencyFromXml() override;

    uint32_t GetSinkLatencyFromXml() override;

    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object) override;

    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo) override;

    int32_t GetCurrentRendererChangeInfos(
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos) override;

    int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos) override;

    void RegisterClientDeathRecipient(const sptr<IRemoteObject> &object, DeathRecipientId id);

    void RegisteredTrackerClientDied(int uid);

    void RegisteredStreamListenerClientDied(int pid);

    bool IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo) override;

    int32_t UpdateStreamState(const int32_t clientUid, StreamSetState streamSetState,
        AudioStreamType audioStreamType) override;

    int32_t GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos) override;

    int32_t GetNetworkIdByGroupId(int32_t groupId, std::string &networkId) override;

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

    void GetStreamVolumeInfoMap(StreamVolumeInfoMap& streamVolumeInfos);

    int32_t QueryEffectSceneMode(SupportedEffectConfig &supportedEffectConfig) override;

    int32_t SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config,
        uint32_t appTokenId) override;

    int32_t SetCaptureSilentState(bool state) override;
    
    int32_t GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc) override;

    vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId) override;

    vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones() override;

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support) override;

    bool IsAbsVolumeScene() override;

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, const bool updateUi) override;

    std::vector<std::unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage) override;

    int32_t SetAvailableDeviceChangeCallback(const int32_t /*clientId*/, const AudioDeviceUsage usage,
        const sptr<IRemoteObject> &object) override;

    int32_t UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage) override;

    bool SpatializationClientDeathRecipientExist(SpatializationEventCategory eventCategory, pid_t uid);

    void RegisterSpatializationClientDeathRecipient(const sptr<IRemoteObject> &object,
        SpatializationEventCategory eventCategory);

    void RegisteredSpatializationEnabledClientDied(pid_t uid);

    void RegisteredHeadTrackingEnabledClientDied(pid_t uid);

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

    int32_t ConfigDistributedRoutingRole(const sptr<AudioDeviceDescriptor> descriptor, CastType type) override;

    int32_t SetDistributedRoutingRoleCallback(const sptr<IRemoteObject> &object) override;

    int32_t UnsetDistributedRoutingRoleCallback() override;

    int32_t UnregisterSpatializationStateEventListener(const uint32_t sessionID) override;

    int32_t RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const int32_t zoneID = 0) override;

    int32_t CreateAudioInterruptZone(const set<int32_t> pids, const int32_t zoneID) override;

    int32_t AddAudioInterruptZonePids(const set<int32_t> pids, const int32_t zoneID) override;

    int32_t RemoveAudioInterruptZonePids(const set<int32_t> pids, const int32_t zoneID) override;

    int32_t ReleaseAudioInterruptZone(const int32_t zoneID) override;
    
    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address) override;

    std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice() override;

    class RemoteParameterCallback : public AudioParameterCallback {
    public:
        RemoteParameterCallback(sptr<AudioPolicyServer> server);
        // AudioParameterCallback
        void OnAudioParameterChange(const std::string networkId, const AudioParamKey key, const std::string& condition,
            const std::string& value) override;
    private:
        sptr<AudioPolicyServer> server_;
        void VolumeOnChange(const std::string networkId, const std::string& condition);
        void InterruptOnChange(const std::string networkId, const std::string& condition);
        void StateOnChange(const std::string networkId, const std::string& condition, const std::string& value);
    };

    std::shared_ptr<RemoteParameterCallback> remoteParameterCallback_;

    class PerStateChangeCbCustomizeCallback : public Security::AccessToken::PermStateChangeCallbackCustomize {
    public:
        explicit PerStateChangeCbCustomizeCallback(const Security::AccessToken::PermStateChangeScope &scopeInfo,
            sptr<AudioPolicyServer> server) : PermStateChangeCallbackCustomize(scopeInfo),
            ready_(false), server_(server) {}
        ~PerStateChangeCbCustomizeCallback() {}

        void PermStateChangeCallback(Security::AccessToken::PermStateChangeInfo& result);
        int32_t getUidByBundleName(std::string bundle_name, int user_id);

        bool ready_;
    private:
        sptr<AudioPolicyServer> server_;
    };

protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    void RegisterParamCallback();

    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t VOLUME_CHANGE_FACTOR = 1;
    static constexpr int32_t VOLUME_KEY_DURATION = 0;
    static constexpr int32_t VOLUME_MUTE_KEY_DURATION = 1;
    static constexpr int32_t MEDIA_SERVICE_UID = 1013;
    static constexpr int32_t EDM_SERVICE_UID = 3057;
    static constexpr int32_t DEFAULT_APP_PID = -1;
    static constexpr char DAUDIO_DEV_TYPE_SPK = '1';
    static constexpr char DAUDIO_DEV_TYPE_MIC = '2';
    static constexpr int32_t DEFAULT_ZONEID = 0;

    static const std::map<InterruptHint, AudioFocuState> HINTSTATEMAP;
    static const std::list<uid_t> RECORD_ALLOW_BACKGROUND_LIST;
    static const std::list<uid_t> RECORD_PASS_APPINFO_LIST;
    static std::map<InterruptHint, AudioFocuState> CreateStateMap();

    int32_t VerifyVoiceCallPermission(uint64_t fullTokenId, Security::AccessToken::AccessTokenID tokenId);

    class AudioPolicyServerPowerStateCallback : public PowerMgr::PowerStateCallbackStub {
    public:
        AudioPolicyServerPowerStateCallback(AudioPolicyServer *policyServer);
        void OnPowerStateChanged(PowerMgr::PowerState state) override;

    private:
        AudioPolicyServer *policyServer_;
    };

    // offload session
    void OffloadStreamCheck(int64_t activateSessionId, AudioStreamType activateStreamType,
        int64_t deactivateSessionId);
    void CheckSubscribePowerStateChange();

    // for audio interrupt
    bool IsSameAppInShareMode(const AudioInterrupt incomingInterrupt, const AudioInterrupt activateInterrupt);
    void ProcessAudioScene(const AudioInterrupt &audioInterrupt, const uint32_t &incomingSessionID,
        const int32_t &zoneID, bool &shouldReturnSuccess);
    int32_t ProcessFocusEntry(const AudioInterrupt &incomingInterrupt, const int32_t zoneID);
    void HandleIncomingState(AudioFocuState incomingState, InterruptEventInternal &interruptEvent,
        const AudioInterrupt &incomingInterrupt, const int32_t zoneID);
    void ProcessCurrentInterrupt(const AudioInterrupt &incomingInterrupt, const int32_t zoneID,
        std::unordered_map<int32_t, std::shared_ptr<AudioInterruptZone>>::iterator &itZone,
        std::list<std::pair<AudioInterrupt, AudioFocuState>> &audioFocusInfoList);
    void ResumeAudioFocusList(const int32_t zoneID);
    std::list<std::pair<AudioInterrupt, AudioFocuState>> SimulateFocusEntry(const int32_t zoneID);
    void NotifyStateChangedEvent(AudioFocuState oldState, AudioFocuState newState,
        std::list<std::pair<AudioInterrupt, AudioFocuState>>::iterator &iterActive);
    void NotifyFocusGranted(const int32_t clientId, const AudioInterrupt &audioInterrupt);
    int32_t NotifyFocusAbandoned(const int32_t clientId, const AudioInterrupt &audioInterrupt);
    void OnAudioFocusInfoChange(int32_t callbackCategory,
        const AudioInterrupt &audioInterrupt, const int32_t zoneID = 0);
    void OnAudioFocusRequested(const AudioInterrupt &audioInterrupt);
    void OnAudioFocusAbandoned(const AudioInterrupt &audioInterrupt);
    void UpdateAudioScene(const AudioScene audioScene, AudioInterruptChangeType changeType);
    void ProcessInterrupt(const InterruptHint& hint);
    AudioScene GetHighestPriorityAudioSceneFromAudioFocusInfoList(const int32_t zoneID) const;
    int32_t DeactivateAudioInterruptEnable(const AudioInterrupt &audioInterrupt, const int32_t zoneID);

    // for audio volume and mute status
    int32_t SetSystemVolumeLevelInternal(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi);
    int32_t SetSingleStreamVolume(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi);
    int32_t GetSystemVolumeLevelInternal(AudioStreamType streamType, bool isFromVolumeKey);
    float GetSystemVolumeDb(AudioStreamType streamType);
    int32_t SetStreamMuteInternal(AudioStreamType streamType, bool mute, bool isUpdateUi);
    int32_t SetSingleStreamMute(AudioStreamType streamType, bool mute, bool isUpdateUi);
    bool GetStreamMuteInternal(AudioStreamType streamType);
    AudioVolumeType GetVolumeTypeFromStreamType(AudioStreamType streamType);
    bool IsVolumeTypeValid(AudioStreamType streamType);
    bool IsVolumeLevelValid(AudioStreamType streamType, int32_t volumeLevel);

    // Permission and privacy
    bool VerifyPermission(const std::string &permission, uint32_t tokenId = 0, bool isRecording = false);
    bool CheckAppBackgroundPermission(uid_t callingUid, uint64_t targetFullTokenId, uint32_t targetTokenId);
    Security::AccessToken::AccessTokenID GetTargetTokenId(uid_t callingUid, uint32_t callingTokenId,
        uint32_t appTokenId);
    uint64_t GetTargetFullTokenId(uid_t callingUid, uint64_t callingFullTokenId, uint64_t appFullTokenId);
    bool CheckRootCalling(uid_t callingUid, int32_t appUid);
    void NotifyPrivacy(uint32_t targetTokenId, AudioPermissionState state);

    // common
    void GetPolicyData(PolicyData &policyData);
    void GetDeviceInfo(PolicyData &policyData);
    void GetGroupInfo(PolicyData &policyData);

    int32_t OffloadStopPlaying(const AudioInterrupt &audioInterrupt);

    int32_t HitZoneID(const set<int32_t> &pids, const std::shared_ptr<AudioInterruptZone> &audioInterruptZone,
        const int32_t &zoneID, int32_t &hitZoneID, bool &haveSamePids);
    int32_t HitZoneIDHaveTheSamePidsZone(const set<int32_t> &pids, int32_t &hitZoneID);

    int32_t DealAudioInterruptZoneData(const int32_t pid,
        const std::shared_ptr<AudioInterruptZone> &audioInterruptZoneTmp,
        std::shared_ptr<AudioInterruptZone> &audioInterruptZone);

    int32_t NewAudioInterruptZoneByPids(std::shared_ptr<AudioInterruptZone> &audioInterruptZone,
        const set<int32_t> &pids, const int32_t &zoneID);

    int32_t ArchiveToNewAudioInterruptZone(const int32_t &fromZoneID, const int32_t &archiveToZoneID);

    bool CheckAudioInterruptZonePermission();

    AppExecFwk::BundleInfo GetBundleInfoFromUid();
    int32_t GetApiTargerVersion();

    // externel function call
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    bool MaxOrMinVolumeOption(const int32_t &volLevel, const int32_t keyType, const AudioStreamType &streamInFocus);
    int32_t RegisterVolumeKeyEvents(const int32_t keyType);
    int32_t RegisterVolumeKeyMuteEvents();
    void SubscribeVolumeKeyEvents();
#endif
    void SubscribePowerStateChangeEvents();
    void InitMicrophoneMute();
    void InitKVStore();
    void ConnectServiceAdapter();
    void LoadEffectLibrary();
    void RegisterBluetoothListener();
    void SubscribeAccessibilityConfigObserver();
    void RegisterDataObserver();
    void RegisterPowerStateListener();
    void UnRegisterPowerStateListener();

    void OnDistributedRoutingRoleChange(const sptr<AudioDeviceDescriptor> descriptor, const CastType type);

    bool powerStateCallbackRegister_;
    AudioPolicyService& audioPolicyService_;
    int32_t clientOnFocus_;
    int32_t volumeStep_;
    std::atomic<bool> isFirstAudioServiceStart_ = false;
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    std::atomic<bool> hasSubscribedVolumeKeyEvents_ = false;
#endif
    std::unique_ptr<AudioInterrupt> focussedAudioInterruptInfo_;
    std::recursive_mutex focussedAudioInterruptInfoMutex_;
    std::vector<pid_t> clientDiedListenerState_;
    std::vector<pid_t> spatializationEnabledListenerState_;
    std::vector<pid_t> headTrackingEnabledListenerState_;
    sptr<PowerStateListener> powerStateListener_;
    std::unordered_map<int32_t /* zone id */, std::shared_ptr<AudioInterruptZone>> audioInterruptZonesMap_;

    std::mutex keyEventMutex_;
    std::mutex audioInterruptZoneMutex_;
    std::mutex micStateChangeMutex_;
    std::mutex clientDiedListenerStateMutex_;
    std::mutex spatializationEnabledListenerStateMutex_;
    std::mutex headTrackingEnabledListenerStateMutex_;

    SessionProcessor sessionProcessor_{std::bind(&AudioPolicyServer::ProcessSessionRemoved,
        this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&AudioPolicyServer::ProcessSessionAdded,
            this, std::placeholders::_1),
        std::bind(&AudioPolicyServer::ProcessorCloseWakeupSource, this, std::placeholders::_1)};

    AudioSpatializationService& audioSpatializationService_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    bool isAvSessionSetVoipStart = false;
    std::set<uint32_t> saveAppCapTokenIdThroughMS;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_SERVER_H
