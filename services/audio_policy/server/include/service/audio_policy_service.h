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
#include "common_event_manager.h"
#ifdef FEATURE_DTMF_TONE
#include "audio_tone_parser.h"
#endif

#include "device_status_listener.h"
#include "iaudio_policy_interface.h"
#include "iport_observer.h"
#include "audio_policy_parser_factory.h"
#include "audio_effect_service.h"
#include "audio_volume_config.h"
#include "policy_provider_stub.h"
#include "audio_device_manager.h"
#include "audio_device_parser.h"
#include "audio_state_manager.h"
#include "audio_pnp_server.h"
#include "audio_policy_server_handler.h"
#include "audio_affinity_manager.h"
#include "audio_ec_info.h"

#ifdef BLUETOOTH_ENABLE
#include "audio_server_death_recipient.h"
#include "audio_bluetooth_manager.h"
#include "bluetooth_device_manager.h"
#endif

#include "audio_a2dp_offload_manager.h"
#include "audio_iohandle_map.h"
#include "audio_router_map.h"
#include "audio_config_manager.h"
#include "audio_connected_device.h"
#include "audio_tone_manager.h"
#include "audio_microphone_descriptor.h"
#include "audio_active_device.h"
#include "audio_a2dp_device.h"
#include "audio_scene_manager.h"
#include "audio_offload_stream.h"
#include "audio_volume_manager.h"
#include "audio_ec_manager.h"
#include "audio_device_common.h"
#include "audio_recovery_device.h"

namespace OHOS {
namespace AudioStandard {

class AudioA2dpOffloadManager;

class AudioPolicyService : public IDeviceStatusObserver, public IPolicyProvider {
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

    void TriggerMicrophoneBlockedCallback(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc,
        DeviceBlockStatus status);

    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType) const;

    int32_t GetMinVolumeLevel(AudioVolumeType volumeType) const;

    int32_t SetSystemVolumeLevel(AudioStreamType streamType, int32_t volumeLevel);

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

    bool IsArmUsbDevice(const AudioDeviceDescriptor &desc);

    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors);

    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType);

    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevicesInner(DeviceFlag deviceFlag);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetOutputDevice(sptr<AudioRendererFilter> audioRendererFilter);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter);

    int32_t SetWakeUpAudioCapturer(InternalAudioCapturerOptions options);

    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config);

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo, uint32_t sessionId);

    int32_t CloseWakeUpAudioCapturer();

    int32_t NotifyWakeUpCapturerRemoved();

    bool IsAbsVolumeSupported();

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active);

    bool IsDeviceActive(InternalDeviceType deviceType);

    DeviceType GetActiveOutputDevice();

    shared_ptr<AudioDeviceDescriptor> GetActiveOutputDeviceDescriptor();

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

    void OnUpdateAnahsSupport(std::string anahsShowType);

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
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);

    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);

    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);

    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);

    void OnServiceDisconnected(AudioServiceIndex serviceIndex);

    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress);

    void OnMonoAudioConfigChanged(bool audioMono);

    void OnAudioBalanceChanged(float audioBalance);

    void LoadEffectLibrary();

    int32_t SetAudioStreamRemovedCallback(AudioStreamRemovedCallback *callback);

    void AddAudioPolicyClientProxyMap(int32_t clientPid, const sptr<IAudioPolicyClient>& cb);

    void ReduceAudioPolicyClientProxyMap(pid_t clientPid);

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

    int32_t GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);

    int32_t GetCurrentCapturerChangeInfos(vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);

    void RegisteredTrackerClientDied(pid_t uid);

    int32_t ReconfigureAudioChannel(const uint32_t &count, DeviceType deviceType);

    int32_t UpdateStreamState(int32_t clientUid, StreamSetStateEventInternal &streamSetStateEventInternal);

    void RemoveDeviceForUid(int32_t uid);

    DeviceType GetDeviceTypeFromPin(AudioPin pin);

    std::vector<sptr<VolumeGroupInfo>> GetVolumeGroupInfos();

    void SetParameterCallback(const std::shared_ptr<AudioParameterCallback>& callback);

    void RegiestPolicy();

    // override for IPolicyProvider
    int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag, AudioDeviceDescriptor &deviceInfo);

    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer);

    bool GetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume &vol);

#ifdef BLUETOOTH_ENABLE
    static void BluetoothServiceCrashedCallback(pid_t pid, pid_t uid);
#endif

    void RegisterBluetoothListener();

    void SubscribeAccessibilityConfigObserver();

    void RegisterRemoteDevStatusCallback();

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
        AudioRendererInfo &rendererInfo, std::string networkId = LOCAL_NETWORK_ID);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo, std::string networkId = LOCAL_NETWORK_ID);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescInner(
        AudioRendererInfo &rendererInfo, std::string networkId = LOCAL_NETWORK_ID);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescInner(
        AudioCapturerInfo &captureInfo, std::string networkId = LOCAL_NETWORK_ID);

    int32_t SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable);

    float GetMinStreamVolume(void);

    float GetMaxStreamVolume(void);

    int32_t GetMaxRendererInstances();

    void RegisterDataObserver();

    bool IsVolumeUnadjustable();

    void GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos);

    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) const;

    int32_t QueryEffectManagerSceneMode(SupportedEffectConfig &supportedEffectConfig);

    void UpdateDescWhenNoBTPermission(vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);

    int32_t SetPlaybackCapturerFilterInfos(const AudioPlaybackCaptureConfig &config);

    int32_t SetCaptureSilentState(bool state);

    int32_t GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc);

    vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);

    vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();

    int32_t SetDeviceAbsVolumeSupported(const std::string &macAddress, const bool support);

    bool IsAbsVolumeScene() const;

    int32_t SetA2dpDeviceVolume(const std::string &macAddress, const int32_t volume, bool internalCall = false);

    int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo, AudioStreamInfo streamInfo);

    void OnCapturerSessionRemoved(uint64_t sessionID);

    std::vector<shared_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);

    void TriggerAvailableDeviceChangedCallback(
        const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc, bool isConnected);

    void OffloadStreamSetCheck(uint32_t sessionId);

    void OffloadStreamReleaseCheck(uint32_t sessionId);

    int32_t OffloadStopPlaying(const std::vector<int32_t> &sessionIds);

    int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp);

    int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName);

    void ConfigDistributedRoutingRole(const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type);

    DistributedRoutingInfo GetDistributedRoutingRoleInfo();

    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);

    void DeviceUpdateClearRecongnitionStatus(AudioDeviceDescriptor &desc);

    void UpdateA2dpOffloadFlagBySpatialService(
        const std::string& macAddress, std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> DeviceFilterByUsageInner(AudioDeviceUsage usage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& descs);

    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address);

    std::shared_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();

    ConverterConfig GetConverterConfig();

    void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);

    void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo);

    float GetMaxAmplitude(const int32_t deviceId);

    int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);

    int32_t DisableSafeMediaVolume();

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

    int32_t ResetRingerModeMute();

    bool IsRingerModeMute();

    void OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName);
    // for effect
    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray);
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray);
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray);
    // for enhance
    int32_t GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray);
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);
    int32_t GetAudioEnhancePropertyByDevice(DeviceType deviceType, AudioEnhancePropertyArray &propertyArray);

    AudioScene GetLastAudioScene() const;
    void SetRotationToEffect(const uint32_t rotate);
    void UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state);
    bool getFastControlParam();

    int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId);

    bool IsAllowedPlayback(const int32_t &uid, const int32_t &pid);

    bool IsCurrentActiveDeviceA2dp();

    int32_t SetVoiceRingtoneMute(bool isMute);

    int32_t SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
        const StreamUsage streamUsage, bool isRunning);

    int32_t SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object);

    int32_t UnsetAudioDeviceAnahsCallback();
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData);
    void SubscribeSafeVolumeEvent();
    int32_t NotifyCapturerRemoved(uint64_t sessionId);
    void UpdateStreamEcAndMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType);
    void ReloadSourceForDeviceChange(const DeviceType inputDevice, const DeviceType outputDevice,
        const std::string &caller);
private:
    AudioPolicyService()
        :audioPolicyManager_(AudioPolicyManagerFactory::GetAudioPolicyManager()),
        streamCollector_(AudioStreamCollector::GetAudioStreamCollector()),
        audioRouterCenter_(AudioRouterCenter::GetAudioRouterCenter()),
        audioEffectService_(AudioEffectService::GetAudioEffectService()),
        audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
        audioAffinityManager_(AudioAffinityManager::GetAudioAffinityManager()),
        audioStateManager_(AudioStateManager::GetAudioStateManager()),
        audioPolicyServerHandler_(DelayedSingleton<AudioPolicyServerHandler>::GetInstance()),
        audioPnpServer_(AudioPnpServer::GetAudioPnpServer()),
        audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
        audioRouteMap_(AudioRouteMap::GetInstance()),
        audioConfigManager_(AudioConfigManager::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
        audioToneManager_(AudioToneManager::GetInstance()),
        audioMicrophoneDescriptor_(AudioMicrophoneDescriptor::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
        audioSceneManager_(AudioSceneManager::GetInstance()),
        audioOffloadStream_(AudioOffloadStream::GetInstance()),
        audioVolumeManager_(AudioVolumeManager::GetInstance()),
        audioEcManager_(AudioEcManager::GetInstance()),
        audioDeviceCommon_(AudioDeviceCommon::GetInstance()),
        audioRecoveryDevice_(AudioRecoveryDevice::GetInstance())
    {
        deviceStatusListener_ = std::make_unique<DeviceStatusListener>(*this);
    }

    ~AudioPolicyService();

    bool FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo, PipeInfo *pipeInfo,
        AudioModuleInfo &audioModuleInfo);
    bool ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo, AudioModuleInfo &audioModuleInfo);

    InternalDeviceType GetDeviceType(const std::string &deviceName);

    DeviceRole GetDeviceRole(DeviceType deviceType) const;

    DeviceRole GetDeviceRole(const std::string &role);

    int32_t LoadDpModule(string deviceInfo);

    int32_t RehandlePnpDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address);

    int32_t HandleArmUsbDevice(DeviceType deviceType, DeviceRole deviceRole, const std::string &address);

    int32_t HandleDpDevice(DeviceType deviceType, const std::string &address);

    int32_t GetModuleInfo(ClassType classType, std::string &moduleInfoStr);

    DeviceRole GetDeviceRole(AudioPin pin) const;

    int32_t ActivateNewDevice(std::string networkId, DeviceType deviceType, bool isRemote);

    void RestoreSession(const int32_t &sessionID, bool isOutput);

    void BluetoothScoDisconectForRecongnition();

    void TriggerDeviceChangedCallback(
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &devChangeDesc, bool connection);

    void WriteAllDeviceSysEvents(
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc, bool isConnected);

    void WriteHeadsetSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &desc, bool isConnected);
    
    void WriteDeviceChangeSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &desc);

    void WriteOutputDeviceChangedSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const SinkInput &sinkInput);

    void WriteInputDeviceChangedSysEvents(const std::shared_ptr<AudioDeviceDescriptor> &deviceDescriptor,
        const SourceOutput &sourceOutput);

    bool IsConfigurationUpdated(DeviceType deviceType, const AudioStreamInfo &streamInfo);

    void UpdateTrackerDeviceChange(const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);

    void AddAudioDevice(AudioModuleInfo& moduleInfo, InternalDeviceType devType);

    void OnPreferredDeviceUpdated(const AudioDeviceDescriptor& deviceDescriptor, DeviceType activeInputDevice);

    void UpdateLocalGroupInfo(bool isConnected, const std::string& macAddress,
        const std::string& deviceName, const DeviceStreamInfo& streamInfo, AudioDeviceDescriptor& deviceDesc);

    int32_t HandleLocalDeviceConnected(AudioDeviceDescriptor &updatedDesc);

    int32_t HandleLocalDeviceDisconnected(const AudioDeviceDescriptor &updatedDesc);

    void UpdateActiveA2dpDeviceWhenDisconnecting(const std::string& macAddress);

    void LoadSinksForCapturer();

    void LoadInnerCapturerSink(string moduleName, AudioStreamInfo streamInfo);

    std::shared_ptr<DataShare::DataShareHelper> CreateDataShareHelperInstance();

    void RegisterNameMonitorHelper();

    void RegisterAccessibilityMonitorHelper();

    void RegisterAccessiblilityBalance();

    void RegisterAccessiblilityMono();

    bool OpenPortAndAddDeviceOnServiceConnected(AudioModuleInfo &moduleInfo);

    void StoreDistributedRoutingRoleInfo(const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type);

    void AddEarpiece();

    void ResetToSpeaker(DeviceType devType);

    PipeInfo& GetPipeInfoByPipeName(std::string &supportPipe, AudioAdapterInfo &adapterInfo);

    int32_t CheckDeviceCapability(AudioAdapterInfo &adapterInfo, int32_t flag, DeviceType deviceType);

    bool IsConfigInfoHasAttribute(std::list<ConfigInfo> &configInfos, std::string value);

    void UnloadInnerCapturerSink(string moduleName);

    void HandleRemoteCastDevice(bool isConnected, AudioStreamInfo streamInfo = {});

    int32_t GetVoipDeviceInfo(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo, int32_t type,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &preferredDeviceList);

    bool LoadAudioPolicyConfig();
    void CreateRecoveryThread();
    void RecoveryPreferredDevices();
    bool IsVoipDeviceChanged(const DeviceType inputDevcie, const DeviceType outputDevice);
    void SetInputDeviceTypeForReload(DeviceType deviceType);
    DeviceType GetInputDeviceTypeForReload();

    void ReloadSourceForEffect(const AudioEnhancePropertyArray &oldPropertyArray,
        const AudioEnhancePropertyArray &newPropertyArray);

    void UpdateAllUserSelectDevice(vector<shared_ptr<AudioDeviceDescriptor>> &userSelectDeviceMap,
        AudioDeviceDescriptor &desc, const std::shared_ptr<AudioDeviceDescriptor> &selectDesc);

    void LoadHdiEffectModel();

    void UpdateEffectBtOffloadSupported(const bool &isSupported);

    bool IsA2dpOffloadConnected();

    void SendA2dpConnectedWhileRunning(const RendererState &rendererState, const uint32_t &sessionId);

    void UpdateDeviceList(AudioDeviceDescriptor &updatedDesc, bool isConnected,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb,
        AudioStreamDeviceChangeReasonExt &reason);
    void UpdateDefaultOutputDeviceWhenStopping(int32_t uid);

    void SetDefaultDeviceLoadFlag(bool isLoad);

    void HandleRemainingSource();

    bool GetAudioEffectOffloadFlag();

    int32_t HandleSpecialDeviceType(DeviceType &devType, bool &isConnected,
        const std::string &address, DeviceRole role);
    bool NoNeedChangeUsbDevice(const string &address);

    void ReloadA2dpOffloadOnDeviceChanged(DeviceType deviceType, const std::string &macAddress,
        const std::string &deviceName, const AudioStreamInfo &streamInfo);

    void HandleOfflineDistributedDevice();

    int32_t HandleDistributedDeviceUpdate(DStatusInfo &statusInfo,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descForCb);

    void OnServiceConnected(AudioServiceIndex serviceIndex);

    void LoadModernInnerCapSink();

    void HandleAudioCaptureState(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);

    AudioStreamType GetStreamType(int32_t sessionId);

    int32_t GetChannelCount(uint32_t sessionId);

    int32_t GetUid(int32_t sessionId);

    void UnregisterBluetoothListener();

    void GetEffectManagerInfo();

    int32_t OffloadStartPlaying(const std::vector<int32_t> &sessionIds);

    void SetA2dpOffloadFlag(BluetoothOffloadState state);
    BluetoothOffloadState GetA2dpOffloadFlag();

#ifdef BLUETOOTH_ENABLE
    void CheckAndActiveHfpDevice(AudioDeviceDescriptor &desc);
#endif

    void OnPreferredStateUpdated(AudioDeviceDescriptor &desc,
        const DeviceInfoUpdateCommand updateCommand, AudioStreamDeviceChangeReasonExt &reason);

    void CheckForA2dpSuspend(AudioDeviceDescriptor &desc);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDumpDevices(DeviceFlag deviceFlag);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDumpDeviceInfo(
        std::string &dumpString, DeviceFlag deviceFlag);
    bool IsStreamSupported(AudioStreamType streamType);

    void AudioPolicyParserDumpInner(std::string &dumpString,
        const std::unordered_map<AdaptersType, AudioAdapterInfo>& adapterInfoMap,
        const std::unordered_map<std::string, std::string>& volumeGroupData,
        std::unordered_map<std::string, std::string>& interruptGroupData,
        GlobalConfigs globalConfigs);
private:

    bool remoteCapturerSwitch_ = false;

    static bool isBtListenerRegistered;
    bool isPnpDeviceConnected = false;
    bool hasModulesLoaded = false;
    const int32_t G_UNKNOWN_PID = -1;
    int32_t dAudioClientUid = 3055;
    int32_t maxRendererInstances_ = 128;
    bool isFastControlled_ = true;
    std::bitset<MIN_SERVICE_COUNT> serviceFlag_;
    std::mutex serviceFlagMutex_;
    std::vector<std::pair<AudioDeviceDescriptor, bool>> pnpDeviceList_;

    IAudioPolicyInterface& audioPolicyManager_;

    AudioStreamCollector& streamCollector_;
    AudioRouterCenter& audioRouterCenter_;
    std::unique_ptr<DeviceStatusListener> deviceStatusListener_;

    AudioScene audioScene_ = AUDIO_SCENE_DEFAULT;
    AudioScene lastAudioScene_ = AUDIO_SCENE_DEFAULT;

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

    AudioEffectService& audioEffectService_;

    bool isMicrophoneMuteTemporary_ = false;

    bool isMicrophoneMutePersistent_ = false;

    mutable std::shared_mutex deviceStatusUpdateSharedMutex_;

    AudioDeviceManager &audioDeviceManager_;
    AudioAffinityManager &audioAffinityManager_;
    AudioStateManager &audioStateManager_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    AudioPnpServer &audioPnpServer_;

    std::mutex defaultDeviceLoadMutex_;
    std::condition_variable loadDefaultDeviceCV_;
    std::atomic<bool> isPrimaryMicModuleInfoLoaded_ = false;

    std::unordered_map<uint32_t, SessionInfo> sessionWithNormalSourceType_;
    uint64_t sessionIdUsedToOpenSource_ = 0;

    DistributedRoutingInfo distributedRoutingInfo_ = {
        .descriptor = nullptr,
        .type = CAST_TYPE_NULL
    };

    // sourceType is SOURCE_TYPE_PLAYBACK_CAPTURE, SOURCE_TYPE_WAKEUP or SOURCE_TYPE_VIRTUAL_CAPTURE
    std::unordered_map<uint32_t, SessionInfo> sessionWithSpecialSourceType_;

    static std::map<std::string, ClassType> classStrToEnum;

    std::unordered_set<uint32_t> sessionIdisRemovedSet_;

    SourceType currentSourceType = SOURCE_TYPE_MIC;
    uint32_t currentRate = 0;
    bool updateA2dpOffloadLogFlag = false;
    std::mutex checkSpatializedMutex_;

    std::mutex inputDeviceReloadMutex_;
    DeviceType inputDeviceForReload_ = DEVICE_TYPE_DEFAULT;

    DeviceType priorityOutputDevice_ = DEVICE_TYPE_INVALID;
    DeviceType priorityInputDevice_ = DEVICE_TYPE_INVALID;
    ConnectType conneceType_ = CONNECT_TYPE_LOCAL;

    SupportedEffectConfig supportedEffectConfig_;
    ConverterConfig converterConfig_;

    std::unique_ptr<std::thread> RecoveryDevicesThread_ = nullptr;

    std::atomic<bool> isPolicyConfigParsered_ = false;
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;

    AudioIOHandleMap& audioIOHandleMap_;
    AudioRouteMap& audioRouteMap_;
    AudioConfigManager& audioConfigManager_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioToneManager& audioToneManager_;
    AudioMicrophoneDescriptor& audioMicrophoneDescriptor_;
    AudioActiveDevice& audioActiveDevice_;
    AudioA2dpDevice& audioA2dpDevice_;
    AudioSceneManager& audioSceneManager_;
    AudioOffloadStream& audioOffloadStream_;
    AudioVolumeManager& audioVolumeManager_;
    AudioEcManager& audioEcManager_;
    AudioDeviceCommon& audioDeviceCommon_;
    AudioRecoveryDevice& audioRecoveryDevice_;
};

class SafeVolumeEventSubscriber : public EventFwk::CommonEventSubscriber {
public:
    explicit SafeVolumeEventSubscriber(const EventFwk::CommonEventSubscribeInfo &subscribeInfo,
        std::function<void(const EventFwk::CommonEventData&)> receiver)
        : EventFwk::CommonEventSubscriber(subscribeInfo), eventReceiver_(receiver) {}
    ~SafeVolumeEventSubscriber() {}
    void OnReceiveEvent(const EventFwk::CommonEventData &eventData) override;
private:
    SafeVolumeEventSubscriber() = default;
    std::function<void(const EventFwk::CommonEventData&)> eventReceiver_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_SERVICE_H
