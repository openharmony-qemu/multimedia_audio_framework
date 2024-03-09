/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include "parser_factory.h"
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

    const sptr<IStandardAudioService> GetAudioServerProxy();

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) const;

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType) const;

    int32_t SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel, bool isFromVolumeKey = false);

    int32_t GetSystemVolumeLevel(AudioStreamType streamType, bool isFromVolumeKey = false) const;

    float GetSystemVolumeDb(AudioStreamType streamType) const;

    int32_t SetLowPowerVolume(int32_t streamId, float volume) const;

    float GetLowPowerVolume(int32_t streamId) const;

    void HandlePowerStateChanged(PowerMgr::PowerState state);

    float GetSingleStreamVolume(int32_t streamId) const;

    int32_t SetStreamMute(AudioStreamType streamType, bool mute);

    int32_t SetSourceOutputStreamMute(int32_t uid, bool setMute) const;

    bool GetStreamMute(AudioStreamType streamType) const;

    bool IsStreamActive(AudioStreamType streamType) const;

    void NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value);

    void NotifyUserSelectionEventToBt(sptr<AudioDeviceDescriptor> audioDeviceDescriptor);

    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
    int32_t SelectFastOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        sptr<AudioDeviceDescriptor> deviceDescriptor);

    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType);

    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors);
    int32_t SelectFastInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        sptr<AudioDeviceDescriptor> deviceDescriptor);

    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    int32_t SetWakeUpAudioCapturer(InternalAudioCapturerOptions options);

    int32_t SetWakeUpAudioCapturerFromAudioServer();

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo, uint32_t sessionId);

    int32_t CloseWakeUpAudioCapturer();

    int32_t NotifyWakeUpCapturerRemoved();

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active);

    bool IsDeviceActive(InternalDeviceType deviceType) const;

    DeviceType GetActiveOutputDevice() const;

    unique_ptr<AudioDeviceDescriptor> GetActiveOutputDeviceDescriptor() const;

    DeviceType GetActiveInputDevice() const;

    int32_t SetRingerMode(AudioRingerMode ringMode);

    bool IsAudioInterruptEnabled() const;

    auto& GetAudioFocusMap() const
    {
        return focusMap_;
    }

    AudioRingerMode GetRingerMode() const;

    int32_t SetMicrophoneMute(bool isMute);

    bool IsMicrophoneMute();

    int32_t SetAudioScene(AudioScene audioScene);

    AudioScene GetAudioScene(bool hasSystemPermission = true) const;

    int32_t GetAudioLatencyFromXml() const;

    uint32_t GetSinkLatencyFromXml() const;

    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri);

    std::string GetSystemSoundUri(const std::string &key);

    bool IsSessionIdValid(int32_t callerUid, int32_t sessionId);

    // Parser callbacks
    void OnXmlParsingCompleted(const std::unordered_map<ClassType, std::list<AudioModuleInfo>> &xmldata);

    void OnVolumeGroupParsed(std::unordered_map<std::string, std::string>& volumeGroupData);

    void OnInterruptGroupParsed(std::unordered_map<std::string, std::string>& interruptGroupData);

    void OnAudioInterruptEnable(bool enable);

    void OnUpdateRouteSupport(bool isSupported);

    int32_t GetDeviceNameFromDataShareHelper(std::string &deviceName);

    void SetDisplayName(const std::string &deviceName, bool isLocalDevice);

#ifdef FEATURE_DTMF_TONE
    std::vector<int32_t> GetSupportedTones();

    std::shared_ptr<ToneInfo> GetToneConfig(int32_t ltonetype);
#endif

    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);

    int32_t HandleSpecialDeviceType(DeviceType &devType, bool &isConnected);

    void OnPnpDeviceStatusUpdated(DeviceType devType, bool isConnected);

    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);

    void ReloadA2dpOffloadOnDeviceChanged(DeviceType deviceType, const std::string &macAddress,
        const std::string &deviceName, const AudioStreamInfo &streamInfo);

    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);

    void HandleOfflineDistributedDevice();

    int32_t HandleDistributedDeviceUpdate(DStatusInfo &statusInfo,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);

    void OnServiceConnected(AudioServiceIndex serviceIndex);

    void OnServiceDisconnected(AudioServiceIndex serviceIndex);

    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress);

    void OnMonoAudioConfigChanged(bool audioMono);

    void OnAudioBalanceChanged(float audioBalance);

    void LoadEffectLibrary();

    int32_t SetAudioSessionCallback(AudioSessionCallback *callback);

    void AddAudioPolicyClientProxyMap(int32_t clientPid, const sptr<IAudioPolicyClient>& cb);

    void ReduceAudioPolicyClientProxyMap(pid_t clientPid);

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

    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object);

    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);

    int32_t GetCurrentRendererChangeInfos(vector<unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);

    int32_t GetCurrentCapturerChangeInfos(vector<unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);

    void RegisteredTrackerClientDied(pid_t uid);

    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType);

    void OnAudioLatencyParsed(uint64_t latency);

    void OnSinkLatencyParsed(uint32_t latency);

    int32_t UpdateStreamState(int32_t clientUid, StreamSetStateEventInternal &streamSetStateEventInternal);

    AudioStreamType GetStreamType(int32_t sessionId);
    
    int32_t GetChannelCount(uint32_t sessionId);

    int32_t GetUid(int32_t sessionId);

    DeviceType GetDeviceTypeFromPin(AudioPin pin);

    std::vector<sptr<VolumeGroupInfo>> GetVolumeGroupInfos();

    void SetParameterCallback(const std::shared_ptr<AudioParameterCallback>& callback);

    void RegiestPolicy();

    // override for IPolicyProvider
    int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, DeviceInfo &deviceInfo);

    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer);

    bool GetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume &vol);

    bool SetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume vol);

#ifdef BLUETOOTH_ENABLE
    static void BluetoothServiceCrashedCallback(pid_t pid);
#endif

    void RegisterBluetoothListener();

    void UnregisterBluetoothListener();

    void SubscribeAccessibilityConfigObserver();

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(AudioRendererInfo &rendererInfo,
        std::string networkId = LOCAL_NETWORK_ID);

    std::vector<sptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(AudioCapturerInfo &captureInfo,
        std::string networkId = LOCAL_NETWORK_ID);

    void GetEffectManagerInfo(OriginalEffectConfig& oriEffectConfig, std::vector<Effect>& availableEffects);

    float GetMinStreamVolume(void);

    float GetMaxStreamVolume(void);

    void MaxRenderInstanceInit();

    int32_t GetMaxRendererInstances();

    void RegisterDataObserver();

    bool IsVolumeUnadjustable();

    void GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos);

    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) const;

    std::string GetLocalDevicesType();

    int32_t QueryEffectManagerSceneMode(SupportedEffectConfig &supportedEffectConfig);

    void UpdateDescWhenNoBTPermission(vector<sptr<AudioDeviceDescriptor>> &desc);

    int32_t SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config);

    int32_t SetCaptureSilentState(bool state);

    void UnloadLoopback();

    void UpdateOutputDeviceSelectedByCalling(DeviceType deviceType);

    int32_t GetHardwareOutputSamplingRate(const sptr<AudioDeviceDescriptor> &desc);

    vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);

    vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support);

    bool IsAbsVolumeScene() const;

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume);

    int32_t GetA2dpDeviceVolume(const std::string& macAddress, int32_t& volume);

    int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo);

    void OnCapturerSessionRemoved(uint64_t sessionID);

    std::vector<unique_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);

    void TriggerAvailableDeviceChangedCallback(const vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected);

    void OffloadStreamSetCheck(uint32_t sessionId);

    void OffloadStreamReleaseCheck(uint32_t sessionId);

    void UpdateA2dpOffloadFlagForAllStream(std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap,
        DeviceType deviceType = DEVICE_TYPE_NONE);

    void UpdateA2dpOffloadFlagForAllStream(DeviceType deviceType = DEVICE_TYPE_NONE);

    void OffloadStartPlayingIfOffloadMode(uint64_t sessionId);

    int32_t OffloadStartPlaying(const std::vector<int32_t> &sessionIds);

    int32_t OffloadStopPlaying(const std::vector<int32_t> &sessionIds);
#ifdef BLUETOOTH_ENABLE
    void UpdateA2dpOffloadFlag(const std::vector<Bluetooth::A2dpStreamInfo> &allActiveSessions,
        DeviceType deviceType = DEVICE_TYPE_NONE);

    void UpdateAllActiveSessions(std::vector<Bluetooth::A2dpStreamInfo> &allActiveSessions);
#endif
    void GetA2dpOffloadCodecAndSendToDsp();

    int32_t HandleA2dpDeviceInOffload();

    int32_t HandleA2dpDeviceOutOffload();

    void ConfigDistributedRoutingRole(const sptr<AudioDeviceDescriptor> descriptor, CastType type);

    DistributedRoutingInfo GetDistributedRoutingRoleInfo();

    void OnScoStateChanged(const std::string &macAddress, bool isConnnected);

    void OnPreferredStateUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand updateCommand);

    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand updateCommand);

    void UpdateA2dpOffloadFlagBySpatialService(
        const std::string& macAddress, std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap);

    std::vector<sptr<AudioDeviceDescriptor>> DeviceFilterByUsage(AudioDeviceUsage usage,
        const std::vector<sptr<AudioDeviceDescriptor>>& descs);

    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address);

    std::unique_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();

private:
    AudioPolicyService()
        :audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        configParser_(ParserFactory::GetInstance().CreateParser(*this)),
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

    void UpdateDeviceInfo(DeviceInfo &deviceInfo, const sptr<AudioDeviceDescriptor> &desc, bool hasBTPermission,
        bool hasSystemPermission);

    std::string GetSinkPortName(InternalDeviceType deviceType);

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

    AudioModuleInfo ConstructWakeUpAudioModuleInfo(int32_t wakeupNo);

    AudioIOHandle GetSinkIOHandle(InternalDeviceType deviceType);

    AudioIOHandle GetSourceIOHandle(InternalDeviceType deviceType);

    int32_t OpenRemoteAudioDevice(std::string networkId, DeviceRole deviceRole, DeviceType deviceType,
        sptr<AudioDeviceDescriptor> remoteDeviceDescriptor);

    InternalDeviceType GetDeviceType(const std::string &deviceName);

    std::string GetGroupName(const std::string& deviceName, const GroupType type);

    bool IsDeviceConnected(sptr<AudioDeviceDescriptor> &audioDeviceDescriptors) const;

    int32_t DeviceParamsCheck(DeviceRole targetRole,
        std::vector<sptr<AudioDeviceDescriptor>> &audioDeviceDescriptors) const;

    bool IsInputDevice(DeviceType deviceType) const;

    bool IsOutputDevice(DeviceType deviceType) const;

    DeviceRole GetDeviceRole(DeviceType deviceType) const;

    DeviceRole GetDeviceRole(const std::string &role);

    int32_t SelectNewDevice(DeviceRole deviceRole, const sptr<AudioDeviceDescriptor> &deviceDescriptor);

    int32_t SwitchActiveA2dpDevice(const sptr<AudioDeviceDescriptor> &deviceDescriptor);

    int32_t HandleActiveDevice(DeviceType deviceType);

    int32_t HandleA2dpDevice(DeviceType deviceType);

    int32_t LoadA2dpModule(DeviceType deviceType);

    int32_t LoadUsbModule(string deviceInfo);

    int32_t LoadDefaultUsbModule();

    int32_t HandleArmUsbDevice(DeviceType deviceType);

    int32_t HandleFileDevice(DeviceType deviceType);

    int32_t ActivateNormalNewDevice(DeviceType deviceType, bool isSceneActivation);

    int32_t ActivateNewDevice(DeviceType deviceType, bool isSceneActivation);

    void SelectNewOutputDevice(unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo,
        unique_ptr<AudioDeviceDescriptor> &outputDevice, bool isStreamStatusUpdated = false,
        const AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::UNKNOWN);

    void SelectNewInputDevice(unique_ptr<AudioCapturerChangeInfo> &capturerChangeInfo,
        unique_ptr<AudioDeviceDescriptor> &inputDevice, bool isStreamStatusUpdated);

    DeviceRole GetDeviceRole(AudioPin pin) const;

    void KeepPortMute(int32_t muteDuration, std::string portName, DeviceType deviceType);

    int32_t ActivateNewDevice(std::string networkId, DeviceType deviceType, bool isRemote);

    DeviceType FetchHighPriorityDevice(bool isOutputDevice);

    int32_t HandleScoOutputDeviceFetched(unique_ptr<AudioDeviceDescriptor> &desc,
        vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos, bool &isStreamStatusUpdated);

    void FetchOutputDevice(vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos,
        bool isStreamStatusUpdated = false,
        const AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::UNKNOWN);
    
    void FetchStreamForA2dpOffload(vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos);

    int32_t HandleScoInputDeviceFetched(unique_ptr<AudioDeviceDescriptor> &desc,
        vector<unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos, bool &isStreamStatusUpdated);

    void FetchInputDevice(vector<unique_ptr<AudioCapturerChangeInfo>> &capturerChangeInfos,
        bool isStreamStatusUpdated);

    void FetchDevice(bool isOutputDevice = true,
        const AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::UNKNOWN);

    void UpdateConnectedDevicesWhenConnecting(const AudioDeviceDescriptor& updatedDesc,
        std::vector<sptr<AudioDeviceDescriptor>>& descForCb);

    void UpdateConnectedDevicesWhenDisconnecting(const AudioDeviceDescriptor& updatedDesc,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);

    void TriggerDeviceChangedCallback(const std::vector<sptr<AudioDeviceDescriptor>> &devChangeDesc, bool connection);
    
    void GetAllRunningStreamSession(std::vector<int32_t> &allSessions, bool doStop = false);

    void WriteDeviceChangedSysEvents(const std::vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected);

    bool GetActiveDeviceStreamInfo(DeviceType deviceType, AudioStreamInfo &streamInfo);

    bool IsConfigurationUpdated(DeviceType deviceType, const AudioStreamInfo &streamInfo);

    void UpdateInputDeviceInfo(DeviceType deviceType);

    void UpdateStreamChangeDeviceInfoForPlayback(AudioStreamChangeInfo &streamChangeInfo);

    void UpdateStreamChangeDeviceInfoForRecord(AudioStreamChangeInfo &streamChangeInfo);

    void UpdateTrackerDeviceChange(const vector<sptr<AudioDeviceDescriptor>> &desc);

    void UpdateGroupInfo(GroupType type, std::string groupName, int32_t& groupId, std::string networkId,
        bool connected, int32_t mappingId);

    void AddAudioDevice(AudioModuleInfo& moduleInfo, InternalDeviceType devType);

    void OnPreferredOutputDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor);

    void OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId);

    void OnPreferredDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor, DeviceType activeInputDevice);

    std::vector<sptr<AudioDeviceDescriptor>> GetDevicesForGroup(GroupType type, int32_t groupId);

    void SetVolumeForSwitchDevice(DeviceType deviceType);

    void SetVoiceCallVolume(int32_t volume);

    std::string GetVolumeGroupType(DeviceType deviceType);

    int32_t ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo);

    void SetOffloadVolume(int32_t volume);

    void RemoveDeviceInRouterMap(std::string networkId);

    void RemoveDeviceInFastRouterMap(std::string networkId);

    void UpdateDisplayName(sptr<AudioDeviceDescriptor> deviceDescriptor);

    void RegisterRemoteDevStatusCallback();

    void UpdateLocalGroupInfo(bool isConnected, const std::string& macAddress,
        const std::string& deviceName, const DeviceStreamInfo& streamInfo, AudioDeviceDescriptor& deviceDesc);

    int32_t HandleLocalDeviceConnected(const AudioDeviceDescriptor &updatedDesc);

    int32_t HandleLocalDeviceDisconnected(const AudioDeviceDescriptor &updatedDesc);

    void UpdateActiveA2dpDeviceWhenDisconnecting(const std::string& macAddress);

    void UpdateEffectDefaultSink(DeviceType deviceType);

    void LoadEffectSinks();

    void LoadSinksForCapturer();

    void LoadInnerCapturerSink();

    void LoadReceiverSink();

    void LoadLoopback();

    DeviceType FindConnectedHeadset();

    bool CreateDataShareHelperInstance();

    void RegisterNameMonitorHelper();

    bool IsConnectedOutputDevice(const sptr<AudioDeviceDescriptor> &desc);

    void AddMicrophoneDescriptor(sptr<AudioDeviceDescriptor> &deviceDescriptor);

    void RemoveMicrophoneDescriptor(sptr<AudioDeviceDescriptor> &deviceDescriptor);

    void AddAudioCapturerMicrophoneDescriptor(int32_t sessionId, DeviceType devType);

    void UpdateAudioCapturerMicrophoneDescriptor(DeviceType devType);

    void RemoveAudioCapturerMicrophoneDescriptor(int32_t uid);

    void SetOffloadMode();

    void ResetOffloadMode();

    bool GetOffloadAvailableFromXml() const;

    void SetOffloadAvailableFromXML(AudioModuleInfo &moduleInfo);

    bool CheckActiveOutputDeviceSupportOffload();

    bool OpenPortAndAddDeviceOnServiceConnected(AudioModuleInfo &moduleInfo);

    int32_t FetchTargetInfoForSessionAdd(const SessionInfo sessionInfo, SourceInfo &targetInfo);

    void StoreDistributedRoutingRoleInfo(const sptr<AudioDeviceDescriptor> descriptor, CastType type);

    void AddEarpiece();

    void FetchOutputDeviceWhenNoRunningStream();

    void FetchInputDeviceWhenNoRunningStream();

    void UpdateActiveDeviceRoute(InternalDeviceType deviceType);

    int32_t ActivateA2dpDevice(unique_ptr<AudioDeviceDescriptor> &desc,
        vector<unique_ptr<AudioRendererChangeInfo>> &rendererChangeInfos, bool isStreamStatusUpdated);

    void ResetToSpeaker(DeviceType devType);

    void UpdateConnectedDevicesWhenConnectingForOutputDevice(const AudioDeviceDescriptor &updatedDesc,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);

    void UpdateConnectedDevicesWhenConnectingForInputDevice(const AudioDeviceDescriptor &updatedDesc,
        std::vector<sptr<AudioDeviceDescriptor>> &descForCb);

    bool IsSameDevice(unique_ptr<AudioDeviceDescriptor> &desc, DeviceInfo &deviceInfo);

    bool IsSameDevice(unique_ptr<AudioDeviceDescriptor> &desc, AudioDeviceDescriptor &deviceDesc);
    
    void UpdateOffloadWhenActiveDeviceSwitchFromA2dp();

    bool IsRendererStreamRunning(unique_ptr<AudioRendererChangeInfo> &rendererChangeInfo);

    bool NeedRehandleA2DPDevice(unique_ptr<AudioDeviceDescriptor> &desc);

    void MuteSinkPort(unique_ptr<AudioDeviceDescriptor> &desc);

    bool interruptEnabled_ = true;
    bool isUpdateRouteSupported_ = true;
    bool isCurrentRemoteRenderer = false;
    bool remoteCapturerSwitch_ = false;
    bool isOpenRemoteDevice = false;
    static bool isBtListenerRegistered;
    bool isPnpDeviceConnected = false;
    bool hasModulesLoaded = false;
    const int32_t G_UNKNOWN_PID = -1;
    int32_t dAudioClientUid = 3055;
    int32_t maxRendererInstances_ = 128;
    uint64_t audioLatencyInMsec_ = 50;
    uint32_t sinkLatencyInMsec_ {0};
    bool isOffloadAvailable_ = false;

    std::unordered_map<std::string, DeviceType> spatialDeviceMap_;

    BluetoothOffloadState a2dpOffloadFlag_ = NO_A2DP_DEVICE;
    BluetoothOffloadState preA2dpOffloadFlag_ = NO_A2DP_DEVICE;
    std::mutex switchA2dpOffloadMutex_;
    bool sameDeviceSwitchFlag_ = false;

    std::bitset<MIN_SERVICE_COUNT> serviceFlag_;
    std::mutex serviceFlagMutex_;
    DeviceType effectActiveDevice_ = DEVICE_TYPE_NONE;
    AudioDeviceDescriptor currentActiveDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_NONE, DEVICE_ROLE_NONE);
    AudioDeviceDescriptor currentActiveInputDevice_ = AudioDeviceDescriptor(DEVICE_TYPE_NONE, DEVICE_ROLE_NONE);
    std::vector<std::pair<DeviceType, bool>> pnpDeviceList_;
    std::string localDevicesType_ = "";

    std::mutex routerMapMutex_; // unordered_map is not concurrently-secure
    mutable std::mutex a2dpDeviceMapMutex_;
    std::unordered_map<int32_t, std::pair<std::string, int32_t>> routerMap_;
    std::unordered_map<int32_t, std::pair<std::string, DeviceRole>> fastRouterMap_; // key:uid value:<netWorkId, Role>
    IAudioPolicyInterface& audioPolicyManager_;
    Parser& configParser_;
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
    std::string lastBTDevice_;

    AudioScene audioScene_ = AUDIO_SCENE_DEFAULT;
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> focusMap_ = {};
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo_ = {};

    std::mutex ioHandlesMutex_;
    std::unordered_map<std::string, AudioIOHandle> IOHandles_ = {};

    std::shared_ptr<AudioSharedMemory> policyVolumeMap_ = nullptr;
    volatile Volume *volumeVector_ = nullptr;

    std::vector<DeviceType> outputPriorityList_ = {
        DEVICE_TYPE_BLUETOOTH_SCO,
        DEVICE_TYPE_BLUETOOTH_A2DP,
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
    AudioEffectManager& audioEffectManager_;

    std::mutex outputDeviceSelectedByCallingMutex_;
    std::unordered_map<decltype(IPCSkeleton::GetCallingUid()), DeviceType> outputDeviceSelectedByCalling_;

    bool isMicrophoneMute_ = false;

    int wakeupCount_ = 0;
    std::mutex wakeupCountMutex_;

    std::mutex deviceClassInfoMutex_;

    std::mutex deviceStatusUpdateSharedMutex_;
    std::mutex microphonesMutex_;

    bool isArmUsbDevice_ = false;

    AudioDeviceManager &audioDeviceManager_;
    AudioStateManager &audioStateManager_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    AudioPnpServer &audioPnpServer_;

    std::optional<uint32_t> offloadSessionID_;
    PowerMgr::PowerState currentPowerState_ = PowerMgr::PowerState::AWAKE;
    bool currentOffloadSessionIsBackground_ = false;
    std::mutex offloadMutex_;

    AudioModuleInfo primaryMicModuleInfo_ = {};

    std::unordered_map<uint32_t, SessionInfo> sessionWithNormalSourceType_;

    DistributedRoutingInfo distributedRoutingInfo_;
    
    // sourceType is SOURCE_TYPE_PLAYBACK_CAPTURE, SOURCE_TYPE_WAKEUP or SOURCE_TYPE_VIRTUAL_CAPTURE
    std::unordered_map<uint32_t, SessionInfo> sessionWithSpecialSourceType_;
    static inline const std::unordered_set<SourceType> specialSourceTypeSet_ = {
        SOURCE_TYPE_PLAYBACK_CAPTURE,
        SOURCE_TYPE_WAKEUP,
        SOURCE_TYPE_VIRTUAL_CAPTURE
    };

    std::unordered_set<uint32_t> sessionIdisRemovedSet_;

    SourceType currentSourceType = SOURCE_TYPE_MIC;
    uint32_t currentRate = 0;
    bool updateA2dpOffloadLogFlag = false;
    
    std::unordered_map<uint32_t, bool> sessionHasBeenSpatialized_;
    std::mutex checkSpatializedMutex_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_SERVICE_H
