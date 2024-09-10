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

#include "audio_info.h"
#include "audio_policy_service.h"
#include "audio_stream_removed_callback.h"
#include "audio_interrupt_callback.h"
#include "audio_policy_manager_stub.h"
#include "audio_policy_client_proxy.h"
#include "audio_server_death_recipient.h"
#include "session_processor.h"
#include "audio_spatialization_service.h"
#include "audio_policy_server_handler.h"
#include "audio_interrupt_service.h"

namespace OHOS {
namespace AudioStandard {

constexpr uint64_t DSTATUS_SESSION_ID = 4294967296;
constexpr uint32_t DSTATUS_DEFAULT_RATE = 48000;
constexpr int32_t LOCAL_USER_ID = 100;

class AudioPolicyService;
class AudioInterruptService;
class AudioPolicyServerHandler;
class AudioSessionService;
class BluetoothEventSubscriber;

class AudioPolicyServer : public SystemAbility,
                          public AudioPolicyManagerStub,
                          public AudioStreamRemovedCallback {
    DECLARE_SYSTEM_ABILITY(AudioPolicyServer);

public:
    DISALLOW_COPY_AND_MOVE(AudioPolicyServer);

    enum DeathRecipientId {
        TRACKER_CLIENT = 0,
        LISTENER_CLIENT
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
        AUDIO_WARNING_LOG("dtor should not happen");
    };

    void OnDump() override;
    void OnStart() override;
    void OnStop() override;

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) override;

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType) override;

    int32_t SetSystemVolumeLevelLegacy(AudioVolumeType volumeType, int32_t volumeLevel) override;

    int32_t SetSystemVolumeLevel(AudioVolumeType volumeType, int32_t volumeLevel, int32_t volumeFlag = 0) override;

    int32_t GetSystemVolumeLevel(AudioStreamType streamType) override;

    int32_t SetLowPowerVolume(int32_t streamId, float volume) override;

    float GetLowPowerVolume(int32_t streamId) override;

    float GetSingleStreamVolume(int32_t streamId) override;

    int32_t SetStreamMuteLegacy(AudioStreamType streamType, bool mute) override;

    int32_t SetStreamMute(AudioStreamType streamType, bool mute) override;

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

    std::vector<sptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag) override;

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint32_t sessionId) override;

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active) override;

    bool IsDeviceActive(InternalDeviceType deviceType) override;

    InternalDeviceType GetActiveOutputDevice() override;

    InternalDeviceType GetActiveInputDevice() override;

    int32_t SetRingerModeLegacy(AudioRingerMode ringMode) override;

    int32_t SetRingerMode(AudioRingerMode ringMode) override;

#ifdef FEATURE_DTMF_TONE
    std::vector<int32_t> GetSupportedTones() override;

    std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype) override;
#endif

    AudioRingerMode GetRingerMode() override;

    int32_t SetAudioScene(AudioScene audioScene) override;

    int32_t SetMicrophoneMuteCommon(bool isMute, bool isLegacy);

    int32_t SetMicrophoneMute(bool isMute) override;

    int32_t SetMicrophoneMuteAudioConfig(bool isMute) override;

    int32_t SetMicrophoneMutePersistent(const bool isMute, const PolicyType type) override;

    bool GetPersistentMicMuteState() override;

    bool IsMicrophoneMuteLegacy() override;

    bool IsMicrophoneMute() override;

    AudioScene GetAudioScene() override;

    int32_t ActivateAudioSession(const AudioSessionStrategy &strategy) override;

    int32_t DeactivateAudioSession() override;

    bool IsAudioSessionActivated() override;

    int32_t SetAudioInterruptCallback(const uint32_t sessionID,
        const sptr<IRemoteObject> &object, const int32_t zoneId = 0) override;

    int32_t UnsetAudioInterruptCallback(const uint32_t sessionID, const int32_t zoneId = 0) override;

    int32_t ActivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneId = 0) override;

    int32_t DeactivateAudioInterrupt(const AudioInterrupt &audioInterrupt, const int32_t zoneId = 0) override;

    int32_t SetAudioManagerInterruptCallback(const int32_t clientId, const sptr<IRemoteObject> &object) override;

    int32_t UnsetAudioManagerInterruptCallback(const int32_t clientId) override;

    int32_t RequestAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt) override;

    int32_t AbandonAudioFocus(const int32_t clientId, const AudioInterrupt &audioInterrupt) override;

    AudioStreamType GetStreamInFocus(const int32_t zoneId = 0) override;

    int32_t GetSessionInfoInFocus(AudioInterrupt &audioInterrupt, const int32_t zoneId = 0) override;

    void OnAudioStreamRemoved(const uint64_t sessionID) override;

    void ProcessSessionRemoved(const uint64_t sessionID, const int32_t zoneId = 0);

    void ProcessSessionAdded(SessionEvent sessionEvent);

    void ProcessorCloseWakeupSource(const uint64_t sessionID);

    int32_t Dump(int32_t fd, const std::vector<std::u16string> &args) override;

    bool CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        SourceType sourceType = SOURCE_TYPE_MIC) override;

    bool CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
        AudioPermissionState state) override;

    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType) override;

    int32_t GetAudioLatencyFromXml() override;

    uint32_t GetSinkLatencyFromXml() override;

    int32_t GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo) override;

    int32_t GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo) override;

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
        StreamUsage streamUsage) override;

    int32_t GetVolumeGroupInfos(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos) override;

    int32_t GetNetworkIdByGroupId(int32_t groupId, std::string &networkId) override;

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
        AudioRendererInfo &rendererInfo) override;

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo) override;

    int32_t SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable) override;

    int32_t GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList,
        const int32_t zoneId = 0) override;

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

    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId) override;

    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones() override;

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support) override;

    bool IsAbsVolumeScene() override;

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, const bool updateUi) override;

    std::vector<std::unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage) override;

    int32_t SetAvailableDeviceChangeCallback(const int32_t /*clientId*/, const AudioDeviceUsage usage,
        const sptr<IRemoteObject> &object) override;

    int32_t UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage) override;

    bool IsSpatializationEnabled() override;

    bool IsSpatializationEnabled(const std::string address) override;

    int32_t SetSpatializationEnabled(const bool enable) override;

    int32_t SetSpatializationEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice,
        const bool enable) override;

    bool IsHeadTrackingEnabled() override;

    bool IsHeadTrackingEnabled(const std::string address) override;

    int32_t SetHeadTrackingEnabled(const bool enable) override;

    int32_t SetHeadTrackingEnabled(const sptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable) override;

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

    int32_t RegisterPolicyCallbackClient(const sptr<IRemoteObject> &object, const int32_t zoneId = 0) override;

    int32_t CreateAudioInterruptZone(const std::set<int32_t> &pids, const int32_t zoneId) override;

    int32_t AddAudioInterruptZonePids(const std::set<int32_t> &pids, const int32_t zoneId) override;

    int32_t RemoveAudioInterruptZonePids(const std::set<int32_t> &pids, const int32_t zoneId) override;

    int32_t ReleaseAudioInterruptZone(const int32_t zoneId) override;

    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address) override;

    std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice() override;

    ConverterConfig GetConverterConfig() override;

    void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason) override;

    void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo) override;

    AudioSpatializationSceneType GetSpatializationSceneType() override;

    int32_t SetSpatializationSceneType(const AudioSpatializationSceneType spatializationSceneType) override;

    float GetMaxAmplitude(const int32_t deviceId) override;

    int32_t DisableSafeMediaVolume() override;

    bool IsHeadTrackingDataRequested(const std::string &macAddress) override;

    int32_t SetAudioDeviceRefinerCallback(const sptr<IRemoteObject> &object) override;

    int32_t UnsetAudioDeviceRefinerCallback() override;

    int32_t TriggerFetchDevice(
        AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN) override;

    int32_t MoveToNewPipe(const uint32_t sessionId, const AudioPipeType pipeType) override;

    int32_t SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object) override;

    int32_t UnsetAudioConcurrencyCallback(const uint32_t sessionID) override;

    int32_t ActivateAudioConcurrency(const AudioPipeType &pipeType) override;

    int32_t InjectInterruption(const std::string networkId, InterruptEvent &event) override;

    int32_t SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
        const StreamUsage streamUsage, bool isRunning) override;

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

    bool IsHighResolutionExist() override;

    int32_t SetHighResolutionExist(bool highResExist) override;

    void NotifyAccountsChanged(const int &id);

    void OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName);
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

protected:
    void OnAddSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

    void RegisterParamCallback();

    void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string& deviceId) override;

private:
    friend class AudioInterruptService;

    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t VOLUME_CHANGE_FACTOR = 1;
    static constexpr int32_t VOLUME_KEY_DURATION = 0;
    static constexpr int32_t VOLUME_MUTE_KEY_DURATION = 1;
    static constexpr int32_t MEDIA_SERVICE_UID = 1013;
    static constexpr int32_t EDM_SERVICE_UID = 3057;
    static constexpr char DAUDIO_DEV_TYPE_SPK = '1';
    static constexpr char DAUDIO_DEV_TYPE_MIC = '2';
    static constexpr int32_t AUDIO_UID = 1041;
    static constexpr uint32_t MICPHONE_CALLER = 0;

    static const std::list<uid_t> RECORD_ALLOW_BACKGROUND_LIST;
    static const std::list<uid_t> RECORD_PASS_APPINFO_LIST;

    class AudioPolicyServerPowerStateCallback : public PowerMgr::PowerStateCallbackStub {
    public:
        AudioPolicyServerPowerStateCallback(AudioPolicyServer *policyServer);
        void OnPowerStateChanged(PowerMgr::PowerState state) override;

    private:
        AudioPolicyServer *policyServer_;
    };

    int32_t VerifyVoiceCallPermission(uint64_t fullTokenId, Security::AccessToken::AccessTokenID tokenId);

    // offload session
    void OffloadStreamCheck(int64_t activateSessionId, int64_t deactivateSessionId);
    void CheckSubscribePowerStateChange();

    void CheckStreamMode(const int64_t activateSessionId);
    bool CheckAudioSessionStrategy(const AudioSessionStrategy &sessionStrategy);

    // for audio volume and mute status
    int32_t SetRingerModeInternal(AudioRingerMode ringMode, bool hasUpdatedVolume = false);
    int32_t SetSystemVolumeLevelInternal(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi);
    int32_t SetSingleStreamVolume(AudioStreamType streamType, int32_t volumeLevel, bool isUpdateUi);
    int32_t GetSystemVolumeLevelInternal(AudioStreamType streamType);
    float GetSystemVolumeDb(AudioStreamType streamType);
    int32_t SetStreamMuteInternal(AudioStreamType streamType, bool mute, bool isUpdateUi);
    int32_t SetSingleStreamMute(AudioStreamType streamType, bool mute, bool isUpdateUi);
    bool GetStreamMuteInternal(AudioStreamType streamType);
    AudioVolumeType GetVolumeTypeFromStreamType(AudioStreamType streamType);
    bool IsVolumeTypeValid(AudioStreamType streamType);
    bool IsVolumeLevelValid(AudioStreamType streamType, int32_t volumeLevel);

    // Permission and privacy
    bool VerifyPermission(const std::string &permission, uint32_t tokenId = 0, bool isRecording = false);
    bool VerifyBluetoothPermission();
    int32_t OffloadStopPlaying(const AudioInterrupt &audioInterrupt);
    int32_t SetAudioSceneInternal(AudioScene audioScene);

    AppExecFwk::BundleInfo GetBundleInfoFromUid();
    int32_t GetApiTargerVersion();

    // externel function call
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    bool MaxOrMinVolumeOption(const int32_t &volLevel, const int32_t keyType, const AudioStreamType &streamInFocus);
    int32_t RegisterVolumeKeyEvents(const int32_t keyType);
    int32_t RegisterVolumeKeyMuteEvents();
    void SubscribeVolumeKeyEvents();
#endif
    void AddAudioServiceOnStart();
    void SubscribeOsAccountChangeEvents();
    void SubscribePowerStateChangeEvents();
    void SubscribeCommonEvent(const std::string event);
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData);
    void HandleKvDataShareEvent();
    void InitMicrophoneMute();
    void InitKVStore();
    void ConnectServiceAdapter();
    void LoadEffectLibrary();
    void RegisterBluetoothListener();
    void SubscribeAccessibilityConfigObserver();
    void RegisterDataObserver();
    void RegisterPowerStateListener();
    void UnRegisterPowerStateListener();
    void RegisterSyncHibernateListener();
    void UnRegisterSyncHibernateListener();
    void RegisterCommonEventReceiver();
    void UnregisterCommonEventReceiver();
    void OnDistributedRoutingRoleChange(const sptr<AudioDeviceDescriptor> descriptor, const CastType type);

    void InitPolicyDumpMap();
    void PolicyDataDump(std::string &dumpString);
    void ArgInfoDump(std::string &dumpString, std::queue<std::u16string> &argQue);
    void InfoDumpHelp(std::string &dumpString);

    int32_t SetRingerModeInner(AudioRingerMode ringMode);

    AudioPolicyService& audioPolicyService_;
    std::shared_ptr<AudioInterruptService> interruptService_;

    int32_t volumeStep_;
    std::atomic<bool> isFirstAudioServiceStart_ = false;
    std::atomic<bool> isInitMuteState_ = false;
#ifdef FEATURE_MULTIMODALINPUT_INPUT
    std::atomic<bool> hasSubscribedVolumeKeyEvents_ = false;
#endif
    std::vector<pid_t> clientDiedListenerState_;
    sptr<PowerStateListener> powerStateListener_;
    sptr<SyncHibernateListener> syncHibernateListener_;
    bool powerStateCallbackRegister_;

    std::mutex keyEventMutex_;
    std::mutex micStateChangeMutex_;
    std::mutex clientDiedListenerStateMutex_;

    SessionProcessor sessionProcessor_{std::bind(&AudioPolicyServer::ProcessSessionRemoved,
        this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&AudioPolicyServer::ProcessSessionAdded,
            this, std::placeholders::_1),
        std::bind(&AudioPolicyServer::ProcessorCloseWakeupSource, this, std::placeholders::_1)};

    AudioSpatializationService& audioSpatializationService_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    std::shared_ptr<BluetoothEventSubscriber> bluetoothEventSubscriberOb_ = nullptr;
    bool volumeApplyToAll_ = false;

    bool isHighResolutionExist_ = false;
    std::mutex descLock_;
    AudioRouterCenter &audioRouterCenter_;
    using DumpFunc = void(AudioPolicyServer::*)(std::string &dumpString);
    std::map<std::u16string, DumpFunc> dumpFuncMap;
    pid_t lastMicMuteSettingPid_ = 0;
    std::string GetBundleName();
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
    AudioPolicyServer *audioPolicyServer_;
};

class BluetoothEventSubscriber : public EventFwk::CommonEventSubscriber,
                                 public std::enable_shared_from_this<BluetoothEventSubscriber> {
public:
    explicit BluetoothEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        sptr<AudioPolicyServer> audioPolicyServer) : EventFwk::CommonEventSubscriber(subscribeInfo),
        audioPolicyServer_(audioPolicyServer) {}
    ~BluetoothEventSubscriber() {}
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
private:
    BluetoothEventSubscriber() = default;
    sptr<AudioPolicyServer> audioPolicyServer_;
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
