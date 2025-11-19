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
#include "audio_manager_base.h"
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
#include "audio_policy_config_manager.h"
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
#include "audio_device_lock.h"
#include "audio_capturer_session.h"
#include "audio_device_status.h"
#include "audio_background_manager.h"
#include "audio_global_config_manager.h"
#include "sle_audio_device_manager.h"
#include "va_device_manager.h"

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

    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status);

    int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel);

    int32_t SetSourceOutputStreamMute(int32_t uid, bool setMute) const;

    std::string GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetOutputDevice(sptr<AudioRendererFilter> audioRendererFilter);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter);

    int32_t NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo, uint32_t sessionId);

    int32_t NotifyWakeUpCapturerRemoved();

    bool IsAbsVolumeSupported();

    shared_ptr<AudioDeviceDescriptor> GetActiveOutputDeviceDescriptor();

    int32_t GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo, const std::string &bundleName);

    void OnUpdateAnahsSupport(std::string anahsShowType);

    void GetAllSinkInputs(std::vector<SinkInput> &sinkInputs);

    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);

    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);

    void OnDeviceConfigurationChanged(DeviceType deviceType,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo);

    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);

    void OnServiceDisconnected(AudioServiceIndex serviceIndex);

    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter = nullptr);

    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress);

    void LoadEffectLibrary();

    int32_t RegisterAudioRendererEventListener(int32_t clientPid, const sptr<IRemoteObject> &object,
        bool hasBTPermission, bool hasSysPermission);

    int32_t UnregisterAudioRendererEventListener(int32_t clientPid);

    int32_t RegisterAudioCapturerEventListener(int32_t clientPid, const sptr<IRemoteObject> &object,
        bool hasBTPermission, bool hasSysPermission);

    int32_t UnregisterAudioCapturerEventListener(int32_t clientPid);

    int32_t SetAvailableDeviceChangeCallback(const int32_t clientId, const AudioDeviceUsage usage,
        const sptr<IRemoteObject> &object, bool hasBTPermission);

    int32_t SetQueryDeviceVolumeBehaviorCallback(const sptr<IRemoteObject> &object);

    int32_t SetQueryClientTypeCallback(const sptr<IRemoteObject> &object);

    int32_t GetCurrentCapturerChangeInfos(vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);

    void SetParameterCallback(const std::shared_ptr<AudioParameterCallback>& callback);

    void RegiestPolicy();

    // override for IPolicyProvider
    int32_t GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag, AudioDeviceDescriptor &deviceInfo);

    int32_t InitSharedVolume(std::shared_ptr<AudioSharedMemory> &buffer);

#ifdef BLUETOOTH_ENABLE
    static void BluetoothServiceCrashedCallback(pid_t pid, pid_t uid);
#endif

    void RegisterBluetoothListener();

    void SubscribeAccessibilityConfigObserver();

    void RegisterRemoteDevStatusCallback();

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
        AudioCapturerInfo &captureInfo, std::string networkId = LOCAL_NETWORK_ID);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescInner(
        AudioRendererInfo &rendererInfo, std::string networkId = LOCAL_NETWORK_ID, const int32_t uid = -1);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescInner(
        AudioCapturerInfo &captureInfo, std::string networkId = LOCAL_NETWORK_ID);

    int32_t GetMaxRendererInstances();

    bool IsSupportInnerCaptureOffload();

    void RegisterDataObserver();

    int32_t QueryEffectManagerSceneMode(SupportedEffectConfig &supportedEffectConfig);

    void UpdateDescWhenNoBTPermission(vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);

    int32_t GetHardwareOutputSamplingRate(const std::shared_ptr<AudioDeviceDescriptor> &desc);

    int32_t OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp);

    int32_t NearlinkGetRenderPosition(uint32_t &delayValue);

    int32_t GetAndSaveClientType(uint32_t uid, const std::string &bundleName);

    DistributedRoutingInfo GetDistributedRoutingRoleInfo();

    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);

    void UpdateA2dpOffloadFlagBySpatialService(
        const std::string& macAddress, std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap);

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> DeviceFilterByUsageInner(AudioDeviceUsage usage,
        const std::vector<std::shared_ptr<AudioDeviceDescriptor>>& descs);

    void NotifyAccountsChanged(const int &id);

    void MuteMediaWhenAccountsChanged();

    // for effect V3
    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray);
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray);
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray);
    // for effect
    int32_t GetSupportedAudioEffectProperty(AudioEffectPropertyArray &propertyArray);
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray);
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray);
    // for enhance
    int32_t GetSupportedAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray);
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray);
    bool IsIntelligentNoiseReductionEnabledForCurrentDevice(SourceType sourceType);
    bool getFastControlParam();

    void OnReceiveEvent(const EventFwk::CommonEventData &eventData);
    void SubscribeSafeVolumeEvent();
    int32_t NotifyCapturerRemoved(uint64_t sessionId);
    void UpdateSpatializationSupported(const std::string macAddress, const bool support);
#ifdef HAS_FEATURE_INNERCAPTURER
    int32_t LoadModernInnerCapSink(int32_t innerCapId);
    int32_t UnloadModernInnerCapSink(int32_t innerCapId);
    int32_t LoadModernOffloadCapSource();
    int32_t UnloadModernOffloadCapSource();
#endif
    void RestoreSession(const uint32_t &sessionID, RestoreInfo restoreInfo);

    int32_t SetSleAudioOperationCallback(const sptr<IRemoteObject> &object);
    int32_t ClearAudioFocusBySessionID(const int32_t &sessionID);
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
#ifdef AUDIO_WIRED_DETECT
        audioPnpServer_(AudioPnpServer::GetAudioPnpServer()),
#endif
        audioGlobalConfigManager_(AudioGlobalConfigManager::GetAudioGlobalConfigManager()),
        audioIOHandleMap_(AudioIOHandleMap::GetInstance()),
        audioRouteMap_(AudioRouteMap::GetInstance()),
        audioConfigManager_(AudioPolicyConfigManager::GetInstance()),
        audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
        audioToneManager_(AudioToneManager::GetInstance()),
        audioMicrophoneDescriptor_(AudioMicrophoneDescriptor::GetInstance()),
        audioActiveDevice_(AudioActiveDevice::GetInstance()),
        audioA2dpDevice_(AudioA2dpDevice::GetInstance()),
        audioSceneManager_(AudioSceneManager::GetInstance()),
        audioBackgroundManager_(AudioBackgroundManager::GetInstance()),
        audioOffloadStream_(AudioOffloadStream::GetInstance()),
        audioVolumeManager_(AudioVolumeManager::GetInstance()),
        audioEcManager_(AudioEcManager::GetInstance()),
        audioDeviceCommon_(AudioDeviceCommon::GetInstance()),
        audioRecoveryDevice_(AudioRecoveryDevice::GetInstance()),
        audioCapturerSession_(AudioCapturerSession::GetInstance()),
        audioDeviceLock_(AudioDeviceLock::GetInstance()),
        audioDeviceStatus_(AudioDeviceStatus::GetInstance()),
        sleAudioDeviceManager_(SleAudioDeviceManager::GetInstance()),
        vaDeviceManager_(VADeviceManager::GetInstance())
    {
        deviceStatusListener_ = std::make_unique<DeviceStatusListener>(*this);
    }

    ~AudioPolicyService();

    void GetSupportedEffectProperty(AudioEffectPropertyArrayV3 &propertyArray);
    void GetSupportedEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray);
    int32_t CheckSupportedAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray, const EffectFlag& flag);
    int32_t GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray);

    void RegisterNameMonitorHelper();

    void RegisterAccessibilityMonitorHelper();

    PipeInfo& GetPipeInfoByPipeName(std::string &supportPipe, AudioAdapterInfo &adapterInfo);

    int32_t CheckDeviceCapability(AudioAdapterInfo &adapterInfo, int32_t flag, DeviceType deviceType);

    bool IsConfigInfoHasAttribute(std::list<ConfigInfo> &configInfos, std::string value);

    int32_t GetVoipDeviceInfo(const AudioProcessConfig &config, AudioDeviceDescriptor &deviceInfo, int32_t type,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &preferredDeviceList);

    bool LoadAudioPolicyConfig();
    void CreateRecoveryThread();

    void LoadHdiEffectModel();

    void UpdateInputDeviceWhenStopping(const bool &isSupported);

    void SetDefaultDeviceLoadFlag(bool isLoad);

    void OnServiceConnected(AudioServiceIndex serviceIndex);

    void UnregisterBluetoothListener();

    BluetoothOffloadState GetA2dpOffloadFlag();
    void SetDefaultAdapterEnable(bool isEnable);
    bool IsDevicePlaybackSupported(const AudioProcessConfig &config, const AudioDeviceDescriptor &deviceInfo);
    bool CheckVoipAnrOn(std::vector<AudioEffectPropertyV3> &property);
private:

    static bool isBtListenerRegistered;
    static bool isBtCrashed;
    bool isPnpDeviceConnected = false;
    const int32_t G_UNKNOWN_PID = -1;
    int32_t dAudioClientUid = 3055;
    int32_t maxRendererInstances_ = 128;
    bool isFastControlled_ = true;
    static constexpr int32_t MIN_SERVICE_COUNT = 2;
    std::bitset<MIN_SERVICE_COUNT> serviceFlag_;
    std::mutex serviceFlagMutex_;

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
        DEVICE_TYPE_SPEAKER,
        DEVICE_TYPE_HDMI,
        DEVICE_TYPE_LINE_DIGITAL
    };
    std::vector<DeviceType> inputPriorityList_ = {
        DEVICE_TYPE_BLUETOOTH_SCO,
        DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_USB_HEADSET,
        DEVICE_TYPE_WIRED_HEADSET,
        DEVICE_TYPE_WAKEUP,
        DEVICE_TYPE_MIC,
        DEVICE_TYPE_ACCESSORY
    };

    AudioEffectService& audioEffectService_;

    bool isMicrophoneMuteTemporary_ = false;

    bool isMicrophoneMutePersistent_ = false;

    AudioDeviceManager &audioDeviceManager_;
    AudioAffinityManager &audioAffinityManager_;
    AudioStateManager &audioStateManager_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
#ifdef AUDIO_WIRED_DETECT
    AudioPnpServer &audioPnpServer_;
#endif
    AudioGlobalConfigManager &audioGlobalConfigManager_;
    DistributedRoutingInfo distributedRoutingInfo_ = {
        .descriptor = nullptr,
        .type = CAST_TYPE_NULL
    };

    static std::map<std::string, ClassType> classStrToEnum;

    SourceType currentSourceType = SOURCE_TYPE_MIC;
    uint32_t currentRate = 0;
    bool updateA2dpOffloadLogFlag = false;
    std::mutex checkSpatializedMutex_;

    std::unique_ptr<std::thread> RecoveryDevicesThread_ = nullptr;

    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;

    AudioIOHandleMap& audioIOHandleMap_;
    AudioRouteMap& audioRouteMap_;
    AudioPolicyConfigManager& audioConfigManager_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioToneManager& audioToneManager_;
    AudioMicrophoneDescriptor& audioMicrophoneDescriptor_;
    AudioActiveDevice& audioActiveDevice_;
    AudioA2dpDevice& audioA2dpDevice_;
    AudioSceneManager& audioSceneManager_;
    AudioBackgroundManager& audioBackgroundManager_;
    AudioOffloadStream& audioOffloadStream_;
    AudioVolumeManager& audioVolumeManager_;
    AudioEcManager& audioEcManager_;
    AudioDeviceCommon& audioDeviceCommon_;
    AudioRecoveryDevice& audioRecoveryDevice_;

    AudioCapturerSession& audioCapturerSession_;
    AudioDeviceLock& audioDeviceLock_;
    AudioDeviceStatus& audioDeviceStatus_;
    SleAudioDeviceManager& sleAudioDeviceManager_;
    VADeviceManager& vaDeviceManager_;
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
