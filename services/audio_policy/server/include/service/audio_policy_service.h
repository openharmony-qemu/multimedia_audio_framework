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

#ifndef ST_AUDIO_POLICY_SERVICE_H
#define ST_AUDIO_POLICY_SERVICE_H

#include <bitset>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "singleton.h"
#include "audio_group_handle.h"
#include "audio_info.h"
#include "audio_manager_base.h"
#include "audio_policy_client_proxy.h"
#include "audio_policy_manager_factory.h"
#include "audio_stream_collector.h"
#include "audio_router_center.h"
#include "datashare_helper.h"
#include "ipc_skeleton.h"
#include "power_mgr_client.h"
#ifdef FEATURE_DTMF_TONE
#include "audio_tone_parser.h"
#endif

#ifdef ACCESSIBILITY_ENABLE
#include "accessibility_config_listener.h"
#else
#include "iaudio_accessibility_config_observer.h"
#endif
#include "device_status_listener.h"
#include "iaudio_policy_interface.h"
#include "iport_observer.h"
#include "audio_policy_parser_factory.h"
#include "audio_effect_manager.h"
#include "audio_volume_config.h"
#include "policy_provider_stub.h"
#include "audio_device_manager.h"
#include "audio_device_parser.h"
#include "audio_state_manager.h"
#include "audio_pnp_server.h"
#include "audio_policy_server_handler.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

namespace OHOS {
namespace AudioStandard {
enum A2dpOffloadConnectionState : int32_t {
    CONNECTION_STATUS_DISCONNECTED = 0,
    CONNECTION_STATUS_CONNECTING = 1,
    CONNECTION_STATUS_CONNECTED = 2,
    CONNECTION_STATUS_TIMEOUT = 3,
};

class AudioA2dpOffloadManager;

class AudioPolicyService : public IPortObserver, public IDeviceStatusObserver,
    public IAudioAccessibilityConfigObserver, public IPolicyProvider {
public:
    static AudioPolicyService& GetAudioPolicyService()
    {
        static AudioPolicyService audioPolicyService;
        return audioPolicyService;
    }

    bool Init(void);
    void Deinit(void);
    void InitKVStore();
    bool ConnectServiceAdapter();

    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status);

    void OnBlockedStatusUpdated(DeviceType devType, DeviceBlockStatus status);

    void TriggerMicrophoneBlockedCallback(const std::vector<sptr<AudioDeviceDescriptor>> &desc,
        DeviceBlockStatus status);

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) const;

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType) const;

    int32_t SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel);

    int32_t SelectDealSafeVolume(AudioStreamType streamType, int32_t volumeLevel);

    int32_t GetSystemVolumeLevel(AudioStreamType streamType);

    float GetSystemVolumeDb(AudioStreamType streamType) const;

    int32_t SetLowPowerVolume(int32_t streamId, float volume) const;

    float GetLowPowerVolume(int32_t streamId) const;

    void HandlePowerStateChanged(PowerMgr::PowerState state);

    float GetSingleStreamVolume(int32_t streamId) const;

    int32_t SetStreamMute(AudioStreamType streamType, bool mute,
        const StreamUsage &streamUsage = STREAM_USAGE_UNKNOWN);

    int32_t SetSourceOutputStreamMute(int32_t uid, bool setMute) const;

    bool GetStreamMute(AudioStreamType streamType);

    bool IsStreamActive(AudioStreamType streamType) const;

    void NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value);

    void NotifyUserSelectionEventToBt(sptr<AudioDeviceDescriptor> audioDeviceDescriptor);

    bool IsArmUsbDevice(const AudioDeviceDescriptor &desc);

    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
    int32_t SelectFastOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        sptr<AudioDeviceDescriptor> deviceDescriptor);

    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType);

    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);

    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    std::vector<sptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);

    int32_t SetWakeUpAudioCapturer(InternalAudioCapturerOptions options);

    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config);

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo, uint32_t sessionId);

    int32_t CloseWakeUpAudioCapturer();

    int32_t NotifyWakeUpCapturerRemoved();

    bool IsAbsVolumeSupported();

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active);

    bool IsDeviceActive(InternalDeviceType deviceType);

    DeviceType GetActiveOutputDevice();

    unique_ptr<AudioDeviceDescriptor> GetActiveOutputDeviceDescriptor();

    DeviceType GetActiveInputDevice();

    int32_t SetRingerMode(AudioRingerMode ringMode);

    AudioRingerMode GetRingerMode() const;

    int32_t SetMicrophoneMute(bool isMute);

    int32_t SetMicrophoneMutePersistent(const bool isMute);

    int32_t InitPersistentMicrophoneMuteState(bool &isMute);

    bool GetPersistentMicMuteState();

    bool IsMicrophoneMute();

    int32_t SetAudioScene(AudioScene audioScene);

    AudioScene GetAudioScene(bool hasSystemPermission = true) const;

    int32_t GetAudioLatencyFromXml() const;

    uint32_t GetSinkLatencyFromXml() const;

    int32_t GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo, const std::string &bundleName);

    int32_t GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo);

    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri);

    std::string GetSystemSoundUri(const std::string &key);

    void SetNormalVoipFlag(const bool &normalVoipFlag);

    int32_t GetVoipRendererFlag(const std::string &sinkPortName, const std::string &networkId,
        const AudioSamplingRate &samplingRate);

    bool GetVoipConfig();

    // Audio Policy Parser callbacks
    void OnAudioPolicyXmlParsingCompleted(const std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap);

    // Parser callbacks
    void OnXmlParsingCompleted(const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmldata);

    void OnVolumeGroupParsed(std::unordered_map<std::string, std::string>& volumeGroupData);

    void OnInterruptGroupParsed(std::unordered_map<std::string, std::string>& interruptGroupData);

    void OnGlobalConfigsParsed(GlobalConfigs &globalConfigs);

    void OnVoipConfigParsed(bool enableFastVoip);

    void OnUpdateRouteSupport(bool isSupported);

    int32_t GetDeviceNameFromDataShareHelper(std::string &deviceName);

    void SetDisplayName(const std::string &deviceName, bool isLocalDevice);

    bool IsDataShareReady();

    void SetDataShareReady(std::atomic<bool> isDataShareReady);

    int32_t ResumeStreamState();
#ifdef FEATURE_DTMF_TONE
    std::vector<int32_t> GetSupportedTones();

    std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype);
#endif

    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);

    void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected);

    void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &name, const std::string &adderess);

    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);

    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);

    void checkOffloadAvailable(AudioModuleInfo& moduleInfo);

    void OnServiceDisconnected(AudioServiceIndex serviceIndex);

    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress);

    void OnMonoAudioConfigChanged(bool audioMono);

    void OnAudioBalanceChanged(float audioBalance);

    void LoadEffectLibrary();

    int32_t SetAudioStreamRemovedCallback(AudioStreamRemovedCallback *callback);

    void AddAudioPolicyClientProxyMap(int32_t clientPid, const sptr<IAudioPolicyClient>& cb);

    void ReduceAudioPolicyClientProxyMap(pid_t clientPid);

    int32_t SetPreferredDevice(const PreferredType preferredType, const sptr<AudioDeviceDescriptor> &desc);

    int32_t SetPreferredOutputDeviceChangeCallback(const int32_t clientId, const sptr<IRemoteObject> &object,
        bool hasBTPermission);

    int32_t SetPreferredInputDeviceChangeCallback(const int32_t clientId, const sptr<IRemoteObject> &object,
        bool hasBTPermission);

    int32_t UnsetPreferredOutputDeviceChangeCallback(const int32_t clientId);

    int32_t UnsetPreferredInputDeviceChangeCallback(const int32_t clientId);

    int32_t RegisterAudioRendererEventListener(int32_t clientPid, const sptr<IRemoteObject> &object,
        bool hasBTPermission, bool hasSysPermission);

    int32_t UnregisterAudioRendererEventListener(int32_t clientPid);

    int32_t RegisterAudioCapturerEventListener(int32_t clientPid, const sptr<IRemoteObject> &object,
        bool hasBTPermission, bool hasSysPermission);

    int32_t UnregisterAudioCapturerEventListener(int32_t clientPid);

    int32_t SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
        const sptr<IRemoteObject> &object, bool hasBTPermission);

    int32_t UnsetAvailableDeviceChangeCallback(const int32_t clientId, AudioDeviceUsage usage);

    int32_t SetQueryClientTypeCallback(const sptr<IRemoteObject> &object);

    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object, const int32_t apiVersion);

    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);

    int32_t GetCurrentRendererChangeInfos(vector<unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);

    int32_t GetCurrentCapturerChangeInfos(vector<unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);

    void RegisteredTrackerClientDied(pid_t uid);

    void ClientDiedDisconnectScoNormal();

    void ClientDiedDisconnectScoRecognition();

    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType);

    void OnAudioLatencyParsed(uint64_t latency);

    void OnSinkLatencyParsed(uint32_t latency);

    int32_t UpdateStreamState(int32_t clientUid, StreamSetStateEventInternal &streamSetStateEventInternal);

    DeviceType GetDeviceTypeFromPin(AudioPin pin);

    std::vector<sptr<VolumeGroupInfo>> GetVolumeGroupInfos();

    void SetParameterCallback(const std::shared_ptr<AudioParameterCallback>& callback);

    void RegiestPolicy();

    // override for IPolicyProvider
    int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag, AudioDeviceDescriptor &deviceInfo);

    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer);

    bool GetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume &vol);

#ifdef BLUETOOTH_ENABLE
    static void BluetoothServiceCrashedCallback(pid_t pid);
#endif

    void RegisterBluetoothListener();

    void SubscribeAccessibilityConfigObserver();

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(AudioRendererInfo &rendererInfo,
        std::string networkId = LOCAL_NETWORK_ID);

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(AudioCapturerInfo &captureInfo,
        std::string networkId = LOCAL_NETWORK_ID);

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescInner(AudioRendererInfo &rendererInfo,
        std::string networkId = LOCAL_NETWORK_ID);

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescInner(AudioCapturerInfo &captureInfo,
        std::string networkId = LOCAL_NETWORK_ID);

    int32_t SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable);

    float GetMinStreamVolume(void);

    float GetMaxStreamVolume(void);

    int32_t GetMaxRendererInstances();

    int32_t CheckMaxRendererInstances();

    void RegisterDataObserver();

    bool IsVolumeUnadjustable();

    void GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos);

    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) const;

    int32_t QueryEffectManagerSceneMode(SupportedEffectConfig &supportedEffectConfig);

    void UpdateDescWhenNoBTPermission(vector<sptr<AudioDeviceDescriptor>> &desc);

    int32_t SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config);

    int32_t SetCaptureSilentState(bool state);

    int32_t GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc);

    vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);

    vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support);

    bool IsAbsVolumeScene() const;

    bool IsVgsVolumeSupported() const;

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, bool internalCall = false);

    int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo, AudioStreamInfo streamInfo);

    void OnCapturerSessionRemoved(uint64_t sessionID);

    std::vector<unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);

    void TriggerAvailableDeviceChangedCallback(const vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected);

    void OffloadStreamSetCheck(uint32_t sessionId);

    void OffloadStreamReleaseCheck(uint32_t sessionId);

    void RemoteOffloadStreamRelease(uint32_t sessionId);

    int32_t OffloadStopPlaying(const std::vector<int32_t> &sessionIds);

    int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp);

    int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName);

    void GetA2dpOffloadCodecAndSendToDsp();

    int32_t HandleA2dpDeviceInOffload(BluetoothOffloadState a2dpOffloadFlag);

    int32_t HandleA2dpDeviceOutOffload(BluetoothOffloadState a2dpOffloadFlag);

    void ConfigDistributedRoutingRole(const sptr<AudioDeviceDescriptor> descriptor, CastType type);

    DistributedRoutingInfo GetDistributedRoutingRoleInfo();

    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);

    void DeviceUpdateClearRecongnitionStatus(AudioDeviceDescriptor &desc);

    void UpdateA2dpOffloadFlagBySpatialService(
        const std::string& macAddress, std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap);

    std::vector<sptr<AudioDeviceDescriptor>> DeviceFilterByUsageInner(AudioDeviceUsage usage,
        const std::vector<sptr<AudioDeviceDescriptor>>& descs);

    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address);

    std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();

    ConverterConfig GetConverterConfig();

    void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo);

    float GetMaxAmplitude(const int32_t deviceId);

    int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    int32_t DisableSafeMediaVolume();

    int32_t SafeVolumeDialogDisapper();

    void NotifyAccountsChanged(const int &id);

    int32_t ActivateConcurrencyFromServer(AudioPipeType incomingPipe);

    // for hidump
    void DevicesInfoDump(std::string &dumpString);
    void AudioModeDump(std::string &dumpString);
    void AudioPolicyParserDump(std::string &dumpString);
    void XmlParsedDataMapDump(std::string &dumpString);
    void StreamVolumesDump(std::string &dumpString);
    void DeviceVolumeInfosDump(std::string &dumpString, DeviceVolumeInfoMap &deviceVolumeInfos);
    void AudioStreamDump(std::string &dumpString);
    void GetVolumeConfigDump(std::string &dumpString);
    void GetGroupInfoDump(std::string &dumpString);
    void GetCallStatusDump(std::string &dumpString);
    void GetRingerModeDump(std::string &dumpString);
    void GetMicrophoneDescriptorsDump(std::string &dumpString);
    void GetCapturerStreamDump(std::string &dumpString);
    void GetSafeVolumeDump(std::string &dumpString);
    void GetOffloadStatusDump(std::string &dumpString);
    void EffectManagerInfoDump(std::string &dumpString);
    void MicrophoneMuteInfoDump(std::string &dumpString);

    int32_t GetCurActivateCount();
    void CheckStreamMode(const int64_t activateSessionId);

    int32_t MoveToNewPipe(const uint32_t sessionId, const AudioPipeType pipeType);
    int32_t DynamicUnloadModule(const AudioPipeType pipeType);

    int32_t SetAudioConcurrencyCallback(const uint32_t sessionID, const sptr<IRemoteObject> &object);

    int32_t UnsetAudioConcurrencyCallback(const uint32_t sessionID);

    int32_t ActivateAudioConcurrency(const AudioPipeType &pipeType);

    void OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName);

    int32_t ResetRingerModeMute();

    bool IsRingerModeMute();

    AudioScene GetLastAudioScene() const;

    void SetRotationToEffect(const uint32_t rotate);
    void FetchStreamForA2dpOffload(const bool &requireReset);
    void UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state);
    bool getFastControlParam();

    int32_t SetVoiceRingtoneMute(bool isMute);

    int32_t SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
        const StreamUsage streamUsage, bool isRunning);

    int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId);
private:
    AudioPolicyService()
        :audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        audioPolicyConfigParser_(AudioPolicyParserFactory::GetInstance().CreateParser(*this)),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
        audioEffectManager_(AudioEffectManager::GetAudioEffectManager()),
        audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioStateManager_(AudioStateManager::GetAudioStateManager()),
        audioPolicyServerHandler_(DelayedSingleton<AudioPolicyServerHandler>::GetInstance()),
        audioPnpServer_(AudioPnpServer::GetAudioPnpServer())
    {
#ifdef ACCESSIBILITY_ENABLE
        accessibilityConfigListener_ = std::make_shared<AccessibilityConfigListener>(*this);
#endif
        deviceStatusListener_ = std::make_unique<DeviceStatusListener>(*this);
    }

    ~AudioPolicyService();

    void UpdateDeviceInfo(AudioDeviceDescriptor &deviceInfo, const sptr<AudioDeviceDescriptor> &desc,
        bool hasBTPermission, bool hasSystemPermission);

    std::string GetSinkPortName(InternalDeviceType deviceType, AudioPipeType pipeType = PIPE_TYPE_UNKNOWN);

    std::string GetSourcePortName(InternalDeviceType deviceType);

    int32_t RememberRoutingInfo(sptr<AudioRendererFilter> audioRendererFilter,
        sptr<AudioDeviceDescriptor> deviceDescriptor);

    int32_t MoveToLocalOutputDevice(std::vector<SinkInput> sinkInputIds,
        sptr<AudioDeviceDescriptor> localDeviceDescriptor);

    std::vector<SinkInput> FilterSinkInputs(sptr<AudioRendererFilter> audioRendererFilter, bool moveAll);

    std::vector<SinkInput> FilterSinkInputs(int32_t sessionId);

    std::vector<SourceOutput> FilterSourceOutputs(int32_t sessionId);

    int32_t MoveToRemoteOutputDevice(std::vector<SinkInput> sinkInputIds,
        sptr<AudioDeviceDescriptor> remoteDeviceDescriptor);

    int32_t MoveToLocalInputDevice(std::vector<SourceOutput> sourceOutputIds,
        sptr<AudioDeviceDescriptor> localDeviceDescriptor);

    int32_t MoveToRemoteInputDevice(std::vector<SourceOutput> sourceOutputIds,
        sptr<AudioDeviceDescriptor> remoteDeviceDescriptor);

    AudioModuleInfo ConstructRemoteAudioModuleInfo(std::string networkId,
        DeviceRole deviceRole, DeviceType deviceType);

    bool FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo, PipeInfo *pipeInfo,
        AudioModuleInfo &audioModuleInfo);
    bool ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo, AudioModuleInfo &audioModuleInfo);

    AudioIOHandle GetSinkIOHandle(InternalDeviceType deviceType);

    AudioIOHandle GetSourceIOHandle(InternalDeviceType deviceType);

    int32_t OpenRemoteAudioDevice(std::string networkId, DeviceRole deviceRole, DeviceType deviceType,
        sptr<AudioDeviceDescriptor> remoteDeviceDescriptor);

    InternalDeviceType GetDeviceType(const std::string &deviceName);

    std::string GetGroupName(const std::string& deviceName, const GroupType type);

    bool IsDeviceConnected(sptr<AudioDeviceDescriptor> &audioDeviceDescriptors) const;

    int32_t DeviceParamsCheck(DeviceRole targetRole,
        std::vector<sptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) const;

    DeviceRole GetDeviceRole(DeviceType deviceType) const;

    DeviceRole GetDeviceRole(const std::string &role);

    int32_t SwitchActiveA2dpDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor);

    int32_t HandleActiveDevice(DeviceType deviceType);

    int32_t LoadA2dpModule();

    int32_t LoadUsbModule(string deviceInfo, DeviceRole deviceRole);

    int32_t LoadDpModule(string deviceInfo);

    int32_t LoadDefaultUsbModule(DeviceRole deviceRole);

    int32_t RehandlePnpDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address);

    int32_t HandleArmUsbDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address);

    int32_t HandleDpDevice(DeviceType deviceType, const std::string &address);

    int32_t GetModuleInfo(ClassType classType, std::string &moduleInfoStr);

    void MoveToNewOutputDevice(unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        vector<std::unique_ptr<AudioDeviceDescriptor>> &outputDevices,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    void MoveToNewInputDevice(unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
        unique_ptr<AudioDeviceDescriptor> &inputDevice);

    int32_t SetRenderDeviceForUsage(StreamUsage streamUsage, sptr<AudioDeviceDescriptor> desc);

    void SetCaptureDeviceForUsage(AudioScene scene, SourceType srcType, sptr<AudioDeviceDescriptor> desc);

    DeviceRole GetDeviceRole(AudioPin pin) const;

    void UnmutePortAfterMuteDuration(int32_t muteDuration, std::string portName, DeviceType deviceType);

    int32_t ActivateNewDevice(std::string networkId, DeviceType deviceType, bool isRemote);

    int32_t HandleScoOutputDeviceFetched(unique_ptr<AudioDeviceDescriptor> &desc,
        vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    void FetchOutputDevice(vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    bool IsFastFromA2dpToA2dp(const std::unique_ptr<AudioDeviceDescriptor> &desc,
        const std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    void FetchStreamForA2dpMchStream(std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        vector<std::unique_ptr<AudioDeviceDescriptor>> &descs);

    int32_t HandleScoInputDeviceFetched(unique_ptr<AudioDeviceDescriptor> &desc,
        vector<unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos);

    void FetchInputDevice(vector<unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    int32_t HandleDeviceChangeForFetchInputDevice(unique_ptr<AudioDeviceDescriptor> &desc,
        unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo);

    void BluetoothScoFetch(unique_ptr<AudioDeviceDescriptor> &desc,
        vector<unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos, SourceType sourceType);

    void BluetoothScoDisconectForRecongnition();

    void FetchDevice(bool isOutputDevice = true,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    void UpdateConnectedDevicesWhenConnecting(const AudioDeviceDescriptor& updatedDesc,
        std::vector<sptr<AudioDeviceDescriptor>>& descForCb);

    void UpdateConnectedDevicesWhenDisconnecting(const AudioDeviceDescriptor& updatedDesc,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);

    void TriggerDeviceChangedCallback(const std::vector<sptr<AudioDeviceDescriptor>> &devChangeDesc, bool connection);

    void GetAllRunningStreamSession(std::vector<int32_t> &allSessions, bool doStop = false);

    void WriteDeviceChangedSysEvents(const std::vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected);

    void WriteOutDeviceChangedSysEvents(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const SinkInput &sinkInput);

    void WriteInDeviceChangedSysEvents(const sptr<AudioDeviceDescriptor> &deviceDescriptor,
        const SourceOutput &sourceOutput);

    void WriteSelectOutputSysEvents(const std::vector<sptr<AudioDeviceDescriptor>> &selectedDesc,
    StreamUsage strUsage);

    void WriteSelectInputSysEvents(const std::vector<sptr<AudioDeviceDescriptor>> &selectedDesc,
        SourceType srcType, AudioScene scene);

    bool GetActiveDeviceStreamInfo(DeviceType deviceType, AudioStreamInfo &streamInfo);

    bool IsConfigurationUpdated(DeviceType deviceType, const AudioStreamInfo &streamInfo);

    void UpdateInputDeviceInfo(DeviceType deviceType);

    void UpdateTrackerDeviceChange(const vector<sptr<AudioDeviceDescriptor>> &desc);

    void UpdateGroupInfo(GroupType type, std::string groupName, int32_t& groupId, std::string networkId,
        bool connected, int32_t mappingId);

    void AddAudioDevice(AudioModuleInfo& moduleInfo, InternalDeviceType devType);

    void OnPreferredOutputDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor);

    void OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId);

    void OnPreferredDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor, DeviceType activeInputDevice);

    std::vector<sptr<AudioDeviceDescriptor>> GetDevicesForGroup(GroupType type, int32_t groupId);

    void SetVolumeForSwitchDevice(DeviceType deviceType, const std::string &newSinkName = PORT_NONE);

    void UpdateVolumeForLowLatency();

    void SetVoiceCallVolume(int32_t volume);

    std::string GetVolumeGroupType(DeviceType deviceType);

    int32_t ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, const AudioStreamInfo& audioStreamInfo);

    void RemoveDeviceInRouterMap(std::string networkId);

    void RemoveDeviceInFastRouterMap(std::string networkId);

    void UpdateDisplayName(sptr<AudioDeviceDescriptor> deviceDescriptor);

    void RegisterRemoteDevStatusCallback();

    void UpdateLocalGroupInfo(bool isConnected, const std::string& macAddress,
        const std::string& deviceName, const DeviceStreamInfo& streamInfo, AudioDeviceDescriptor& deviceDesc);

    int32_t HandleLocalDeviceConnected(AudioDeviceDescriptor &updatedDesc);

    int32_t HandleLocalDeviceDisconnected(const AudioDeviceDescriptor &updatedDesc);

    void UpdateActiveA2dpDeviceWhenDisconnecting(const std::string& macAddress);

    void UpdateEffectDefaultSink(DeviceType deviceType);

    void LoadSinksForCapturer();

    void LoadInnerCapturerSink(string moduleName, AudioStreamInfo streamInfo);

    DeviceType FindConnectedHeadset();

    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelperInstance();

    void RegisterNameMonitorHelper();

    bool IsConnectedOutputDevice(const sptr<AudioDeviceDescriptor> &desc);

    void AddMicrophoneDescriptor(sptr<AudioDeviceDescriptor> &deviceDescriptor);

    void RemoveMicrophoneDescriptor(sptr<AudioDeviceDescriptor> &deviceDescriptor);

    void AddAudioCapturerMicrophoneDescriptor(int32_t sessionId, DeviceType devType);

    void UpdateAudioCapturerMicrophoneDescriptor(DeviceType devType);

    void RemoveAudioCapturerMicrophoneDescriptor(int32_t uid);

    void SetOffloadMode();

    void ResetOffloadMode(int32_t sessionId);

    bool GetOffloadAvailableFromXml() const;

    void SetOffloadAvailableFromXML(AudioModuleInfo &moduleInfo);

    bool CheckActiveOutputDeviceSupportOffload();

    bool OpenPortAndAddDeviceOnServiceConnected(AudioModuleInfo &moduleInfo);

    int32_t FetchTargetInfoForSessionAdd(const SessionInfo sessionInfo, StreamPropInfo &targetInfo,
        SourceType &targetSourceType);

    void StoreDistributedRoutingRoleInfo(const sptr<AudioDeviceDescriptor> descriptor, CastType type);

    void AddEarpiece();

    void FetchOutputDeviceWhenNoRunningStream();

    void FetchInputDeviceWhenNoRunningStream();

    void UpdateActiveDeviceRoute(InternalDeviceType deviceType, DeviceFlag deviceFlag);

    void UpdateActiveDevicesRoute(std::vector<std::pair<InternalDeviceType, DeviceFlag>> &activeDevices);

    void UpdateDualToneState(const bool &enable, const int32_t &sessionId);

    int32_t ActivateA2dpDevice(unique_ptr<AudioDeviceDescriptor> &desc,
        vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    int32_t ActivateA2dpDeviceWhenDescEnabled(unique_ptr<AudioDeviceDescriptor> &desc,
        vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    void ResetToSpeaker(DeviceType devType);

    void UpdateConnectedDevicesWhenConnectingForOutputDevice(const AudioDeviceDescriptor &updatedDesc,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);

    void UpdateConnectedDevicesWhenConnectingForInputDevice(const AudioDeviceDescriptor &updatedDesc,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);

    bool IsSameDevice(unique_ptr<AudioDeviceDescriptor> &desc, AudioDeviceDescriptor &deviceInfo);

    bool IsSameDevice(unique_ptr<AudioDeviceDescriptor> &desc, const AudioDeviceDescriptor &deviceDesc);

    void UpdateOffloadWhenActiveDeviceSwitchFromA2dp();

    bool IsRendererStreamRunning(unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo);

    bool NeedRehandleA2DPDevice(unique_ptr<AudioDeviceDescriptor> &desc);

    void MuteSinkPort(const std::string &portName, int32_t duration, bool isSync);

    void MuteSinkPort(const std::string &oldSinkname, const std::string &newSinkName,
        AudioStreamDeviceChangeReasonExt reason);

    void MuteDefaultSinkPort();

    void SetVoiceCallMuteForSwitchDevice();

    void MuteSinkPortForSwtichDevice(unique_ptr<AudioRendererChangeInfo>& rendererChangeInfo,
        vector<std::unique_ptr<AudioDeviceDescriptor>>& outputDevices, const AudioStreamDeviceChangeReasonExt reason);

    std::string GetSinkName(const AudioDeviceDescriptor& desc, int32_t sessionId);

    void RectifyModuleInfo(AudioModuleInfo &moduleInfo, std::list<AudioModuleInfo> &moduleInfoList,
        SourceInfo &targetInfo);

    void ClearScoDeviceSuspendState(string macAddress = "");

    PipeInfo& GetPipeInfoByPipeName(std::string &supportPipe, AudioAdapterInfo &adapterInfo);

    int32_t CheckDeviceCapability(AudioAdapterInfo &adapterInfo, int32_t flag, DeviceType deviceType);

    bool IsConfigInfoHasAttribute(std::list<ConfigInfo> &configInfos, std::string value);

    int32_t OpenPortAndInsertIOHandle(const std::string &moduleName, const AudioModuleInfo &moduleInfo);

    int32_t ClosePortAndEraseIOHandle(const std::string &moduleName);

    DeviceUsage GetDeviceUsage(const AudioDeviceDescriptor &desc);

    void UnloadInnerCapturerSink(string moduleName);

    void HandleRemoteCastDevice(bool isConnected, AudioStreamInfo streamInfo = {});

    int32_t HandleDeviceChangeForFetchOutputDevice(unique_ptr<AudioDeviceDescriptor> &desc,
        unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo);

    bool IsWiredHeadSet(const DeviceType &deviceType);

    bool IsBlueTooth(const DeviceType &deviceType);

    int32_t DealWithSafeVolume(const int32_t volumeLevel, bool isA2dpDevice);

    void CreateCheckMusicActiveThread();

    void CreateSafeVolumeDialogThread();

    void SetDeviceSafeVolumeStatus();

    void CheckBlueToothActiveMusicTime(int32_t safeVolume);

    void CheckWiredActiveMusicTime(int32_t safeVolume);

    void RestoreSafeVolume(AudioStreamType streamType, int32_t safeVolume);

    int32_t CheckActiveMusicTime();

    int32_t ShowDialog();

    int32_t GetVoipDeviceInfo(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo, int32_t type,
        std::vector<sptr<AudioDeviceDescriptor>> &preferredDeviceList);

    int32_t GetPreferredOutputStreamTypeInner(StreamUsage streamUsage, DeviceType deviceType, int32_t flags,
        std::string &networkId, AudioSamplingRate &samplingRate);

    int32_t GetPreferredInputStreamTypeInner(SourceType sourceType, DeviceType deviceType, int32_t flags,
        const std::string &networkId, const AudioSamplingRate &samplingRate);

    int32_t GetPreferredInputStreamTypeFromDeviceInfo(AudioAdapterInfo &adapterInfo,
        DeviceType deviceType, int32_t flags);

    bool NotifyRecreateRendererStream(std::unique_ptr<AudioDeviceDescriptor> &desc,
        const std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    void TriggerRecreateRendererStreamCallback(int32_t callerPid, int32_t sessionId, int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);

    bool NotifyRecreateDirectStream(std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    bool IsDirectSupportedDevice(DeviceType deviceType);

    bool UpdateDevice(unique_ptr<AudioDeviceDescriptor> &desc, const AudioStreamDeviceChangeReasonExt reason,
        const std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo);

    bool NotifyRecreateCapturerStream(bool isUpdateActiveDevice,
        const std::unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    void TriggerRecreateCapturerStreamCallback(int32_t callerPid, int32_t sessionId, int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);

    bool HasLowLatencyCapability(DeviceType deviceType, bool isRemote);

    int32_t HandleAbsBluetoothVolume(const std::string &macAddress, const int32_t volumeLevel);

    void WriteServiceStartupError(string reason);

    bool LoadToneDtmfConfig();

    void CreateRecoveryThread();
    void RecoveryPreferredDevices();

    int32_t HandleRecoveryPreferredDevices(int32_t preferredType, int32_t deviceType,
        int32_t usageOrSourceType);

    void WriteOutputRouteChangeEvent(unique_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason);
    void WriteInputRouteChangeEvent(unique_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason);

    bool CheckStreamOffloadMode(int64_t activateSessionId, AudioStreamType streamType);
    AudioModuleInfo ConstructOffloadAudioModuleInfo(DeviceType deviceType);
    int32_t LoadOffloadModule();
    int32_t UnloadOffloadModule();
    int32_t MoveToOutputDevice(uint32_t sessionId, std::string portName);

    bool CheckStreamMultichannelMode(const int64_t activateSessionId);
    AudioModuleInfo ConstructMchAudioModuleInfo(DeviceType deviceType);
    int32_t LoadMchModule();
    int32_t UnloadMchModule();

    int32_t MoveToNewPipeInner(const uint32_t sessionId, const AudioPipeType pipeType);

    void UpdateRoute(unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        vector<std::unique_ptr<AudioDeviceDescriptor>> &outputDevices);

    bool IsRingerOrAlarmerDualDevicesRange(const InternalDeviceType &deviceType);

    bool SelectRingerOrAlarmDevices(const vector<std::unique_ptr<AudioDeviceDescriptor>> &descs,
        const unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo);

    void DealAudioSceneOutputDevices(const AudioScene &audioScene, std::vector<DeviceType> &activeOutputDevices,
        bool &haveArmUsbDevice);

    bool IsA2dpOrArmUsbDevice(const InternalDeviceType &deviceType);

    void UpdateAllUserSelectDevice(vector<unique_ptr<AudioDeviceDescriptor>> &userSelectDeviceMap,
        AudioDeviceDescriptor &desc, const sptr<AudioDeviceDescriptor> &selectDesc);

    void SetAbsVolumeSceneAsync(const std::string &macAddress, const bool support);

    void SetSharedAbsVolumeScene(const bool support);

    void LoadHdiEffectModel();

    void UpdateEffectBtOffloadSupported(const bool &isSupported);

    int32_t ScoInputDeviceFetchedForRecongnition(bool handleFlag, const std::string &address,
        ConnectState connectState);

    bool IsA2dpOffloadConnected();

    void SetCurrenInputDevice(const AudioDeviceDescriptor &desc);

    AudioDeviceDescriptor GetCurrentInputDevice();

    DeviceType GetCurrentInputDeviceType();

    void SetCurrentInputDeviceType(DeviceType deviceType);

    void SetCurrentOutputDevice(const AudioDeviceDescriptor &desc);

    void SetCurrentOutputDeviceType(DeviceType deviceType);

    AudioDeviceDescriptor GetCurrentOutputDevice();

    DeviceType GetCurrentOutputDeviceType();

    DeviceCategory GetCurrentOutputDeviceCategory();

    std::string GetCurrentOutputDeviceNetworkId();

    std::string GetCurrentOutputDeviceMacAddr();

    void SendA2dpConnectedWhileRunning(const RendererState &rendererState, const uint32_t &sessionId);

    int32_t ConnectVirtualDevice(sptr<AudioDeviceDescriptor> &desc);
    void UpdateDeviceList(AudioDeviceDescriptor &updatedDesc, bool isConnected,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb,
        AudioStreamDeviceChangeReasonExt &reason);
    void UpdateDefaultOutputDeviceWhenStopping(int32_t uid);

    void SetDefaultDeviceLoadFlag(bool isLoad);

    int32_t ErasePreferredDeviceByType(const PreferredType preferredType);

    void CheckAndNotifyUserSelectedDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor);

    bool GetAudioEffectOffloadFlag();
    void ResetOffloadModeOnSpatializationChanged(std::vector<int32_t> &allSessions);

    bool CheckSpatializationAndEffectState();

    void FetchStreamForSpkMchStream(std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
    vector<std::unique_ptr<AudioDeviceDescriptor>> &descs);

    void ResetOffloadAndMchMode(std::unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        vector<std::unique_ptr<AudioDeviceDescriptor>> &outputDevices);

    void JudgeIfLoadMchModule();

    const sptr<IStandardAudioService> GetAudioServerProxy();

    int32_t SelectFastInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        sptr<AudioDeviceDescriptor> deviceDescriptor);

    void GetAudioAdapterInfos(std::unordered_map<AdaptersType, AudioAdapterInfo> &adapterInfoMap);

    void GetVolumeGroupData(std::unordered_map<std::string, std::string>& volumeGroupData);

    void GetInterruptGroupData(std::unordered_map<std::string, std::string>& interruptGroupData);

    void GetDeviceClassInfo(std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo);

    void GetGlobalConfigs(GlobalConfigs &globalConfigs);

    int32_t HandleSpecialDeviceType(DeviceType &devType, bool &isConnected, const std::string &address);

    void ReloadA2dpOffloadOnDeviceChanged(DeviceType deviceType, const std::string &macAddress,
        const std::string &deviceName, const AudioStreamInfo &streamInfo);

    void HandleOfflineDistributedDevice();

    int32_t HandleDistributedDeviceUpdate(DStatusInfo &statusInfo,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);

    void OnServiceConnected(AudioServiceIndex serviceIndex);

    void LoadModernInnerCapSink();

    void HandleAudioCaptureState(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);

    AudioStreamType GetStreamType(int32_t sessionId);

    int32_t GetChannelCount(uint32_t sessionId);

    int32_t GetUid(int32_t sessionId);

    bool SetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume vol);

    void UnregisterBluetoothListener();

    void GetEffectManagerInfo();

    std::vector<unique_ptr<AudioDeviceDescriptor>> GetAvailableDevicesInner(AudioDeviceUsage usage);

    void UpdateA2dpOffloadFlagForAllStream(std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap,
        DeviceType deviceType = DEVICE_TYPE_NONE);

    int32_t UpdateA2dpOffloadFlagForAllStream(DeviceType deviceType = DEVICE_TYPE_NONE);

    int32_t OffloadStartPlaying(const std::vector<int32_t> &sessionIds);

#ifdef BLUETOOTH_ENABLE
    void UpdateA2dpOffloadFlag(const std::vector<Bluetooth::A2dpStreamInfo> &allActiveSessions,
        DeviceType deviceType = DEVICE_TYPE_NONE);

    void CheckAndActiveHfpDevice(AudioDeviceDescriptor &desc);
#endif

    void OnPreferredStateUpdated(AudioDeviceDescriptor &desc,
        const DeviceInfoUpdateCommand updateCommand, AudioStreamDeviceChangeReasonExt &reason);

    void CheckForA2dpSuspend(AudioDeviceDescriptor &desc);

    void UnloadA2dpModule();

    std::vector<sptr<AudioDeviceDescriptor>> GetDumpDevices(DeviceFlag deviceFlag);
    std::vector<sptr<AudioDeviceDescriptor>> GetDumpDeviceInfo(std::string &dumpString, DeviceFlag deviceFlag);
    bool IsStreamSupported(AudioStreamType streamType);
    bool IsVoiceCallRelatedScene();
private:
    bool isUpdateRouteSupported_ = true;
    bool isCurrentRemoteRenderer = false;
    bool remoteCapturerSwitch_ = false;
    bool isOpenRemoteDevice = false;
    static bool isBtListenerRegistered;
    bool isPnpDeviceConnected = false;
    bool hasModulesLoaded = false;
    bool hasEarpiece_ = false;
    const int32_t G_UNKNOWN_PID = -1;
    int32_t dAudioClientUid = 3055;
    int32_t maxRendererInstances_ = 128;
    uint64_t audioLatencyInMsec_ = 50;
    uint32_t sinkLatencyInMsec_ {0};
    bool isOffloadAvailable_ = false;
    bool enableFastVoip_ = false;
    bool enableDualHalToneState_ = false;
    int32_t enableDualHalToneSessionId_ = -1;
    int32_t shouldUpdateDeviceDueToDualTone_ = false;
    bool isFastControlled_ = false;
    bool isVoiceRingtoneMute_ = false;

    std::unordered_map<std::string, DeviceType> spatialDeviceMap_;

    BluetoothOffloadState a2dpOffloadFlag_ = NO_A2DP_DEVICE;
    std::mutex switchA2dpOffloadMutex_;

    std::bitset<MIN_SERVICE_COUNT> serviceFlag_;
    std::mutex serviceFlagMutex_;
    DeviceType effectActiveDevice_ = DEVICE_TYPE_NONE;
    std::mutex curOutputDevice_; // lock this mutex to operate currentActiveDevice_
    AudioDeviceDescriptor currentActiveDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_NONE, DEVICE_ROLE_NONE);
    std::mutex curInputDevice_; // lock this mutex to operate currentActiveInputDevice_
    AudioDeviceDescriptor currentActiveInputDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_NONE, DEVICE_ROLE_NONE);
    std::vector<std::pair<DeviceType, bool>> pnpDeviceList_;

    std::mutex routerMapMutex_; // unordered_map is not concurrently-secure
    mutable std::mutex a2dpDeviceMapMutex_;
    std::unordered_map<int32_t, std::pair<std::string, int32_t>> routerMap_;
    std::unordered_map<int32_t, std::pair<std::string, DeviceRole>> fastRouterMap_; // key:uid value:<netWorkId, Role>
    IAudioPolicyInterface& audioPolicyManager_;
    Parser& audioPolicyConfigParser_;
#ifdef FEATURE_DTMF_TONE
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> toneDescriptorMap;
#endif
    AudioStreamCollector& streamCollector_;
    AudioRouterCenter& audioRouterCenter_;
#ifdef ACCESSIBILITY_ENABLE
    std::shared_ptr<AccessibilityConfigListener> accessibilityConfigListener_;
#endif
    std::unique_ptr<DeviceStatusListener> deviceStatusListener_;
    std::vector<sptr<AudioDeviceDescriptor>> connectedDevices_;
    std::vector<sptr<MicrophoneDescriptor>> connectedMicrophones_;
    std::unordered_map<int32_t, sptr<MicrophoneDescriptor>> audioCaptureMicrophoneDescriptor_;
    std::unordered_map<std::string, A2dpDeviceConfigInfo> connectedA2dpDeviceMap_;
    std::string activeBTDevice_;

    AudioScene audioScene_ = AUDIO_SCENE_DEFAULT;
    AudioScene lastAudioScene_ = AUDIO_SCENE_DEFAULT;
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo_ = {};
    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap_ {};

    std::mutex ioHandlesMutex_;
    std::unordered_map<std::string, AudioIOHandle> IOHandles_ = {};

    std::shared_ptr<AudioSharedMemory> policyVolumeMap_ = nullptr;
    volatile Volume *volumeVector_ = nullptr;
    volatile bool *sharedAbsVolumeScene_ = nullptr;

    std::vector<DeviceType> outputPriorityList_ = {
        DEVICE_TYPE_BLUETOOTH_SCO,
        DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_DP,
        DEVICE_TYPE_USB_HEADSET,
        DEVICE_TYPE_WIRED_HEADSET,
        DEVICE_TYPE_SPEAKER
    };
    std::vector<DeviceType> inputPriorityList_ = {
        DEVICE_TYPE_BLUETOOTH_SCO,
        DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_USB_HEADSET,
        DEVICE_TYPE_WIRED_HEADSET,
        DEVICE_TYPE_WAKEUP,
        DEVICE_TYPE_MIC
    };

    std::vector<sptr<VolumeGroupInfo>> volumeGroups_;
    std::vector<sptr<InterruptGroupInfo>> interruptGroups_;
    std::unordered_map<std::string, std::string> volumeGroupData_;
    std::unordered_map<std::string, std::string> interruptGroupData_;
    GlobalConfigs globalConfigs_;
    AudioEffectManager& audioEffectManager_;

    bool isMicrophoneMuteTemporary_ = false;

    bool isMicrophoneMutePersistent_ = false;

    mutable std::shared_mutex deviceStatusUpdateSharedMutex_;

    bool hasArmUsbDevice_ = false;
    bool hasHifiUsbDevice_ = false; // Only the first usb device is supported now, hifi or arm.
    bool hasDpDevice_ = false; // Only the first dp device is supported.

    AudioDeviceManager &audioDeviceManager_;
    AudioStateManager &audioStateManager_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    AudioPnpServer &audioPnpServer_;

    std::optional<uint32_t> offloadSessionID_;
    PowerMgr::PowerState currentPowerState_ = PowerMgr::PowerState::AWAKE;
    bool currentOffloadSessionIsBackground_ = false;
    std::mutex offloadMutex_;

    AudioModuleInfo primaryMicModuleInfo_ = {};
    std::mutex defaultDeviceLoadMutex_;
    std::condition_variable loadDefaultDeviceCV_;
    std::atomic<bool> isPrimaryMicModuleInfoLoaded_ = false;
    std::atomic<bool> isAdapterInfoMap_ = false;

    std::mutex moveDeviceMutex_;
    std::condition_variable moveDeviceCV_;
    std::atomic<bool> moveDeviceFinished_ = false;

    std::unordered_map<uint32_t, SessionInfo> sessionWithNormalSourceType_;

    DistributedRoutingInfo distributedRoutingInfo_ = {
        .descriptor = nullptr,
        .type = CAST_TYPE_NULL
    };

    // sourceType is SOURCE_TYPE_PLAYBACK_CAPTURE, SOURCE_TYPE_WAKEUP or SOURCE_TYPE_VIRTUAL_CAPTURE
    std::unordered_map<uint32_t, SessionInfo> sessionWithSpecialSourceType_;

    static std::map<std::string, std::string> sinkPortStrToClassStrMap_;
    static std::map<std::string, uint32_t> formatStrToEnum;
    static std::map<std::string, ClassType> classStrToEnum;

    std::unordered_set<uint32_t> sessionIdisRemovedSet_;

    SourceType currentSourceType = SOURCE_TYPE_MIC;
    uint32_t currentRate = 0;

    bool updateA2dpOffloadLogFlag = false;
    std::mutex checkSpatializedMutex_;
    SafeStatus safeStatusBt_ = SAFE_UNKNOWN;
    SafeStatus safeStatus_ = SAFE_UNKNOWN;
    int64_t activeSafeTimeBt_ = 0;
    int64_t activeSafeTime_ = 0;
    std::time_t startSafeTimeBt_ = 0;
    std::time_t startSafeTime_ = 0;
    bool userSelect_ = false;
    std::unique_ptr<std::thread> calculateLoopSafeTime_ = nullptr;
    std::mutex checkMusicActiveThreadMutex_; // lock calculateLoopSafeTime_
    bool safeVolumeExit_ = false;
    bool isAbsBtFirstBoot_ = true;
    bool normalVoipFlag_ = false;

    std::mutex dialogMutex_;
    std::atomic<bool> isDialogSelectDestroy_ = false;
    std::condition_variable dialogSelectCondition_;

    std::unique_ptr<std::thread> safeVolumeDialogThrd_ = nullptr;
    std::atomic<bool> isSafeVolumeDialogShowing_ = false;
    std::mutex safeVolumeMutex_;

    DeviceType priorityOutputDevice_ = DEVICE_TYPE_INVALID;
    DeviceType priorityInputDevice_ = DEVICE_TYPE_INVALID;
    ConnectType conneceType_ = CONNECT_TYPE_LOCAL;

    SupportedEffectConfig supportedEffectConfig_;
    ConverterConfig converterConfig_;

    std::unique_ptr<std::thread> RecoveryDevicesThread_ = nullptr;

    std::mutex offloadCloseMutex_;
    std::mutex offloadOpenMutex_;
    std::atomic<bool> isOffloadOpened_ = false;
    std::condition_variable offloadCloseCondition_;

    std::mutex ringerModeMuteMutex_;
    std::atomic<bool> ringerModeMute_ = true;
    std::condition_variable ringerModeMuteCondition_;
    std::atomic<bool> isPolicyConfigParsered_ = false;
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;

    bool isBTReconnecting_ = false;
};

class AudioA2dpOffloadManager final : public Bluetooth::AudioA2dpPlayingStateChangedListener,
    public enable_shared_from_this<AudioA2dpOffloadManager> {
public:
    AudioA2dpOffloadManager(AudioPolicyService *audioPolicyService) : audioPolicyService_(audioPolicyService) {};
    void Init() {Bluetooth::AudioA2dpManager::RegisterA2dpPlayingStateChangedListener(shared_from_this());};
    A2dpOffloadConnectionState GetA2dOffloadConnectionState() {return currentOffloadConnectionState_;};

    void ConnectA2dpOffload(const std::string &deviceAddress, const vector<int32_t> &sessionIds);
    void OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState) override;

    void WaitForConnectionCompleted();
    bool IsA2dpOffloadConnecting(int32_t sessionId);
private:
    A2dpOffloadConnectionState currentOffloadConnectionState_ = CONNECTION_STATUS_DISCONNECTED;
    std::vector<int32_t> connectionTriggerSessionIds_;
    std::string a2dpOffloadDeviceAddress_ = "";
    AudioPolicyService *audioPolicyService_ = nullptr;
    std::mutex connectionMutex_;
    std::condition_variable connectionCV_;
    static const int32_t CONNECTION_TIMEOUT_IN_MS = 1000; // 1000ms
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_SERVICE_H
