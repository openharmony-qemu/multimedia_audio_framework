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

#ifndef ST_AUDIO_ADAPTER_MANAGER_H
#define ST_AUDIO_ADAPTER_MANAGER_H

#include <list>
#include <unordered_map>
#include <unordered_set>
#include <cinttypes>

#include "audio_adapter_manager_handler.h"
#include "audio_service_adapter.h"
#include "distributed_kv_data_manager.h"
#include "iaudio_policy_interface.h"
#include "types.h"
#include "audio_policy_log.h"
#include "audio_policy_server_handler.h"
#include "audio_volume_config.h"
#include "volume_data_maintainer.h"
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "hdi_adapter_type.h"
#include "audio_device_manager.h"
#include "istandard_audio_service.h"
#include "audio_active_device.h"
#include "audio_connected_device.h"
#include "boot_animation_state_info.h"
#include "audio_iohandle_map.h"
#include "audio_anco_playback_info.h"

namespace OHOS {
namespace AudioStandard {
using namespace OHOS::DistributedKv;

class AudioOsAccountInfo;

struct AppConfigVolume {
    int32_t defaultVolume;
    int32_t maxVolume;
    int32_t minVolume;
};

const int32_t MAX_CACHE_AMOUNT = 10;
static constexpr int32_t MAX_VOLUME_DEGREE = 100;
class AudioAdapterManager : public IAudioPolicyInterface {
public:
    static constexpr std::string_view SPLIT_STREAM_SINK = "libmodule-split-stream-sink.z.so";
    static constexpr std::string_view HDI_SINK = "libmodule-hdi-sink.z.so";
    static constexpr std::string_view HDI_SOURCE = "libmodule-hdi-source.z.so";
    static constexpr std::string_view PIPE_SINK = "libmodule-pipe-sink.z.so";
    static constexpr std::string_view PIPE_SOURCE = "libmodule-pipe-source.z.so";
    static constexpr std::string_view CLUSTER_SINK = "libmodule-cluster-sink.z.so";
    static constexpr std::string_view EFFECT_SINK = "libmodule-effect-sink.z.so";
    static constexpr std::string_view INNER_CAPTURER_SINK = "libmodule-inner-capturer-sink.z.so";
    static constexpr std::string_view RECEIVER_SINK = "libmodule-receiver-sink.z.so";
    static constexpr uint32_t KVSTORE_CONNECT_RETRY_COUNT = 5;
    static constexpr uint32_t KVSTORE_CONNECT_RETRY_DELAY_TIME = 200000;
    static constexpr float MIN_VOLUME = 0.0f;
    static constexpr uint32_t NUMBER_TWO = 2;
    static constexpr float HDI_MAX_SINK_VOLUME_LEVEL = 1.0f;
    static constexpr uint32_t HDI_DEFAULT_MULTICHANNEL_CHANNELLAYOUT = 1551;
    static constexpr uint32_t HDI_EC_SAME_ADAPTER = 1;
    static constexpr std::string_view HDI_AUDIO_PORT_SINK_ROLE = "sink";
    static constexpr std::string_view HDI_AUDIO_PORT_SOURCE_ROLE = "source";
    bool Init();
    void Deinit(void);
    void InitKVStore();
    bool ConnectServiceAdapter();

    static IAudioPolicyInterface& GetInstance()
    {
        static AudioAdapterManager audioAdapterManager;
        return audioAdapterManager;
    }

    virtual ~AudioAdapterManager() {}

    // Shared 3da-direct bus-port predicate: public so pipe-domain code
    // (AudioIOHandleMap / AudioPipeManager) reuses the single implementation.
    static bool Is3daDirectBusPort(const AudioModuleInfo &audioModuleInfo);

    int32_t SetSystemVolumeLevel(AudioStreamType streamType, const VolumeScale &volume,
        std::shared_ptr<AudioDeviceDescriptor> &volDeviceDesc, int32_t zoneId = 0);

    int32_t SetAppVolumeLevel(int32_t appUid, int32_t volumeLevel, DeviceType deviceType);

    int32_t SetAppVolumeMuted(int32_t appUid, bool muted);

    int32_t SetAppVolumeMuteAndHideVolumeBar(int32_t appUid, bool muted);

    bool IsAppMuteAndHideVolumeBar(int32_t appUid);

    int32_t ClearAppMuteAndHideByOwnerUid(int32_t callerUid);

    int32_t SetAppVolumeMutedDB(int32_t appUid, bool muted);

    int32_t SetVoipRendererMute(int32_t callerUid, int32_t appUid, int64_t streamId, bool muted);

    int32_t SetVoipCapturerMute(int32_t callerUid, int32_t appUid, int64_t streamId, bool muted);

    int32_t ClearVoipMuteByCallerUid(int32_t callerUid);

    int32_t ClearVoipMuteByStreamId(int64_t streamId);

    int32_t SetUnmuteWhitelist(const std::vector<int32_t> &uids);

    int32_t ClearUnmuteWhitelist();

    int32_t IsAppVolumeMute(int32_t appUid, bool owned, bool &isMute);

    int32_t GetSystemAppVolumeLevel(int32_t appUid, int32_t &volumeLevel);

    int32_t SetSystemAppVolumeLevel(int32_t appUid, int32_t volumeLevel, DeviceType deviceType);

    int32_t SetSystemAppVolumeMuted(int32_t appUid, bool muted, DeviceType deviceType);

    int32_t IsSystemAppVolumeMuted(int32_t appUid, bool &isMute);

    int32_t SetAppRingMuted(int32_t appUid, bool muted);

    bool IsAppRingMuted(int32_t appUid);

    int32_t GetSystemVolumeLevel(std::shared_ptr<AudioDeviceDescriptor> desc, AudioStreamType streamType,
        int32_t zoneId = 0);

    int32_t GetAppVolumeLevel(int32_t appUid, int32_t &volumeLevel);

    int32_t SetZoneVolumeLevel(int32_t zoneId, AudioStreamType streamType, const VolumeScale &volume,
        std::shared_ptr<AudioDeviceDescriptor> &volDeviceDesc);

    int32_t GetZoneVolumeLevel(int32_t zoneId, AudioStreamType streamType);

    int32_t SetZoneMute(int32_t zoneId, AudioStreamType streamType, bool mute,
        StreamUsage streamUsage = STREAM_USAGE_UNKNOWN,
        const DeviceType &deviceType = DEVICE_TYPE_NONE);

    bool GetZoneMute(int32_t zoneId, AudioStreamType streamType);

    int32_t SetAdjustVolumeForZone(int32_t zoneId);

    int32_t GetVolumeAdjustZoneId();

    int32_t GetSystemVolumeLevelNoMuteState(AudioStreamType streamType,
        std::shared_ptr<AudioDeviceDescriptor> desc);

    float GetSystemVolumeDb(AudioStreamType streamType);

    int32_t SetStreamMute(std::shared_ptr<AudioDeviceDescriptor> &device,
        AudioStreamType streamType, bool mute, StreamUsage streamUsage = STREAM_USAGE_UNKNOWN, int32_t zoneId = 0);

    void SetDeviceNoMuteForRinger(std::shared_ptr<AudioDeviceDescriptor> device);

    void ClearDeviceNoMuteForRinger();

    int32_t SetSourceOutputStreamMute(int32_t uid, bool setMute);

    int32_t SetSourceOutputStreamMuteByStreamId(int32_t sessionId, bool setMute);

    bool GetStreamMute(AudioStreamType streamType,
        std::shared_ptr<AudioDeviceDescriptor> deviceDesc, int32_t zoneId = 0);

    bool GetAppMute(int32_t appUid);

    std::vector<SinkInfo> GetAllSinks();

    void GetAllSinkInputs(std::vector<SinkInput> &sinkInputs);

    std::vector<SourceOutput> GetAllSourceOutputs();

    AudioIOHandle OpenAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex);

    AudioIOHandle OpenAudioPort(const AudioModuleInfo &audioPortInfo, uint32_t &paIndex);

    AudioIOHandle ReloadA2dpAudioPort(const AudioModuleInfo &audioPortInfo, uint32_t &paIndex);

    void ReloadAudioPort(const AudioModuleInfo &audioPortInfo, uint32_t &paIndex);

    int32_t CloseAudioPort(AudioIOHandle ioHandle, uint32_t paIndex = HDI_INVALID_ID);

    int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName);

    int32_t MoveSourceOutputByIndexOrName(uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName);

    int32_t SetRingerMode(AudioRingerMode ringerMode);

    AudioRingerMode GetRingerMode(void) const;

    int32_t SetAudioStreamRemovedCallback(AudioStreamRemovedCallback *callback);

    int32_t SuspendAudioDevice(std::string &name, bool isSuspend);

    bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false);

    float CalculateVolumeDb(int32_t volumeLevel);

    int32_t SetSystemSoundUri(const std::string &key, const std::string &uri);

    std::string GetSystemSoundUri(const std::string &key);

    float GetMinStreamVolume(void) const;

    float GetMaxStreamVolume(void) const;

    bool IsVolumeUnadjustable();

    float CalculateVolumeDbNonlinear(AudioStreamType streamType, DeviceType deviceType,
        int32_t volumeLevel, bool isSupportWearDetection = false);

    void GetStreamVolumeInfoMap(StreamVolumeInfoMap &streamVolumeInfos);

    DeviceVolumeType GetDeviceCategory(DeviceType deviceType, AudioStreamType streamType = STREAM_MUSIC);

    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType);

    bool IsUseNonlinearAlgo() { return useNonlinearAlgo_; }

    void SetAbsVolumeScene(const std::string &macAddress, bool isAbsVolumeScene, int32_t volume);

    int32_t SetNearlinkDeviceVolume(AudioVolumeType volumeType, int32_t volume);

    bool IsAbsVolumeScene(const std::string &macAddress) const;

    void SetAbsVolumeMute(const std::string &macAddress, bool mute);

    void SetAbsVolumeMuteNearlink(bool mute);

    void SetDataShareReady(std::atomic<bool> isDataShareReady);

    bool IsAbsVolumeMute(const std::string &macAddress) const;

    std::string GetModuleArgs(const AudioModuleInfo &audioModuleInfo) const;

    std::string GetHdiSinkIdInfo(const AudioModuleInfo &audioModuleInfo) const;

    std::string GetSinkIdInfo(std::shared_ptr<AudioPipeInfo> pipeInfo) const;

    std::string GetHdiSourceIdInfo(const AudioModuleInfo &audioModuleInfo) const;

    IAudioSinkAttr GetAudioSinkAttr(const AudioModuleInfo &audioModuleInfo) const;
    void FillSinkLatencyAttr(IAudioSinkAttr &attr, const AudioModuleInfo &audioModuleInfo) const;
    void FillUltraFastPeriod(IAudioSinkAttr &attr, const AudioModuleInfo &audioModuleInfo) const;

    IAudioSourceAttr GetAudioSourceAttr(const AudioModuleInfo &audioModuleInfo) const;

    void HandleDpConnection();

    void RefreshVolumeWhenDpReConnect();

    int32_t GetStreamVolume(std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, AudioStreamType streamType);

    void NotifyAccountsChanged(const int &id);
    void NotifySubProfileChanged();
    void UpdateVolumeDataForUserSwitch();
    void UpdateSubProfileId();

    void MuteMediaWhenAccountsChanged();

    void SafeVolumeDump(std::string &dumpString);

    int32_t DoRestoreData();
    SafeStatus GetCurrentDeviceSafeStatus(DeviceType deviceType);

    int64_t GetCurentDeviceSafeTime(DeviceType deviceType);

    int32_t SetDeviceSafeStatus(DeviceType deviceType, SafeStatus status);

    int32_t SetDeviceSafeTime(DeviceType deviceType, int64_t time);

    int32_t SetRestoreVolumeLevel(DeviceType deviceType, int32_t volume);

    int32_t GetRestoreVolumeLevel(DeviceType deviceType);

    int32_t GetSafeVolumeLevel() const;

    int32_t GetSafeVolumeDegree() const;

    int32_t GetSafeVolumeTimeout() const;

    int32_t GetCurActivateCount(void) const;

    void HandleKvData(bool isFirstBoot);

    int32_t SetPersistMicMuteState(const bool isMute);

    int32_t GetPersistMicMuteState(bool &isMute);

    void HandleSaveVolume(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel,
        std::string networkId);

    void SaveRingerMode(AudioRingerMode ringerMode);

    void SetAudioServerProxy(sptr<IStandardAudioService> gsp);

    void SetOffloadSessionId(uint32_t sessionId, OffloadAdapter offloadAdapter, uint32_t ioHandle);

    void ResetOffloadSessionId(uint32_t sessionId);

    int32_t SetDoubleRingVolumeDb(const AudioStreamType &streamType, const int32_t &volumeLevel, DeviceType deviceType);

    void SaveRingerModeInfo(AudioRingerMode ringMode, std::string callerName, std::string invocationTime);

    void GetRingerModeInfo(std::vector<RingerModeAdjustInfo> &ringerModeInfo);

    std::shared_ptr<AllDeviceVolumeInfo> GetAllDeviceVolumeInfo(DeviceType deviceType, AudioStreamType streamType);

    std::vector<AdjustStreamVolumeInfo> GetStreamVolumeInfo(AdjustStreamVolume volumeType);

    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) const;

    int32_t GetDeviceVolume(DeviceType deviceType, AudioStreamType streamType);

    void UpdateSafeVolumeByS4();
    void SetVgsVolumeSupported(bool isVgsSupported);
    bool IsVgsVolumeSupported() const;

    int32_t SaveSpecifiedDeviceVolume(AudioStreamType streamType, int32_t volumeLevel, DeviceType deviceType,
        int32_t zoneId = 0);
    int32_t UpdateCollaborativeState(CollaborationState isCollaborationEnabled);
    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState);
    int32_t UpdateDeviceSpatialEnabled(const std::string &address, const AudioSpatializationState &state);
    int32_t RemoveDeviceSpatialEnabled(const std::string &address);
    int32_t ClearDeviceSpatialEnabled();
    void RegisterDoNotDisturbStatus();
    void RegisterDoNotDisturbStatusWhiteList();
    void RegisterMdmMuteSwitchCallback();
    void MdmMuteSwitchCallback(bool isMute);
    void RegisterAppIndividualVolumeEnableObserver();
    void HandleAppIndividualVolumeSettingChanged();
    void RegisterCloneStateObserver();
    int32_t SetQueryDeviceVolumeBehaviorCallback(const sptr<IRemoteObject> &object);
    void SetSleVoiceStatusFlag(bool isSleVoiceStatus);
    void DisableVolumeLimitWhenVoiceCall(bool disableVolumeLimit);
    void SendLoudVolumeModeToDsp(LoudVolumeHoldType funcHoldType, bool state);
    void SaveSystemVolumeForEffect(DeviceType deviceType, AudioStreamType streamType, int32_t volumeLevel);
    int32_t GetSystemVolumeForEffect(DeviceType deviceType, AudioStreamType streamType);
    int32_t SetSystemVolumeToEffect(AudioStreamType streamType, float volume);
    int32_t SetSystemVolumeToEffectWithDeviceType(AudioStreamType streamType, float volume, DeviceType deviceType);
    void SaveSystemVolumeForSwitchDevice(std::shared_ptr<AudioDeviceDescriptor> &device,
        AudioStreamType streamType, int32_t volumeLevel);
    void AddCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        const SourceType &sourceType);
    void RemoveCaptureInjector(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        const SourceType &sourceType);
    void AddCaptureInjectorByTargetStream(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        uint32_t targetStreamId);
    void RemoveCaptureInjectorByTargetStream(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        uint32_t targetStreamId);
    void UpdateAudioPortInfo(const uint32_t &sinkPortIndex, const AudioModuleInfo &audioPortInfo);
    int32_t AddCaptureInjector();
    int32_t RemoveCaptureInjector();
    void UpdateVolumeWhenDeviceConnect(std::shared_ptr<AudioDeviceDescriptor> &device);
    void UpdateVolumeWhenDeviceDisconnect(std::shared_ptr<AudioDeviceDescriptor> &device);
    void QueryDeviceVolumeBehavior(std::shared_ptr<AudioDeviceDescriptor> &device);
    int32_t GetMaxVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType, int32_t zoneId = 0);
    int32_t GetMinVolumeLevel(AudioVolumeType volumeType, DeviceType deviceType, int32_t zoneId = 0);
    bool IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout);
    bool IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout, DeviceType deviceType);
    void UpdateOtherStreamVolume(AudioStreamType streamType);
    void SetVolumeLimit(float volume);
    void SetMaxVolumeForDpBoardcast();
    void HandleCastingConnection();
    void HandleCastingDisconnection();
    bool IsDPCastingConnect();
    int32_t GetSystemVolumeDegree(std::shared_ptr<AudioDeviceDescriptor> desc, AudioStreamType streamType,
        int32_t zoneId = 0, bool checkMuteState = true);
    int32_t GetMinVolumeDegree(AudioVolumeType volumeType, DeviceType deviceType, int32_t zoneId = 0);
    float GetSystemVolumeInDbByDegree(AudioVolumeType volumeType,
        std::shared_ptr<AudioDeviceDescriptor> deviceDesc, bool mute);
    int32_t GetZoneVolumeDegree(int32_t zoneId, AudioStreamType streamType);
    float CalculateVolumeDbByDegree(DeviceType deviceType, AudioStreamType streamType,
        int32_t volumeDegree, bool isSupportWearDetection = false);
    int32_t UpdateCollaborativeProductId(const std::string &productId);
    bool IsCollaborationSupportedForStreamUsage(DeviceType deviceType, StreamUsage streamUsage);
    int32_t UpdatePersonalizedHRTFBin(const int32_t &fd, const long &length);
    int32_t UpdatePersonalizedSpatializationState(const bool personalizedSpatializationEnabled);
    void LoadCollaborationConfig();
    void SetDualStreamVolumeMute(int32_t sessionId, bool isDualMute);
    void SetVolumeFromRemote(std::string networkId, int32_t volumeDegress);
    void SetMuteFromRemote(std::string networkId, bool mute);
    void RecordBootStateTime(AudioPolicyServerBootState state, int64_t curTime);
    void RecordBootStateTime(BootAnimationState state, int32_t uid, int64_t curTime);
    int32_t SetLegacyHighVolumeCheckTime(int64_t checkTime);
    int64_t GetLegacyHighVolumeCheckTime();
    void InitLegacyHighVolumeCheckTime(bool isFirstBoot);
    int32_t SetAudioZoneVolumeConfig(const std::string &zoneName,
        const std::vector<AudioZoneVolumeConfig> &configs);
    void TriggerAudioZoneVolumeSync(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc = nullptr);
    void SendAudioZoneVolumeForDevice(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc);
    void SendAudioZoneVolumeForZone(int32_t zoneId);
    void InitAudioZoneVolume(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> desc);

    void UpdateAudioPipeVolume(std::vector<PipeDeviceVolumeGroup> pipeDeviceVolumeGroups);
    void RemoveAudioPipeVolume(uint32_t IoHandle);
    void UpdateVolumeForAllPipes();
    void UpdateTypeVolumeForAllPipes(AudioVolumeType volumeType);

    class RemoteVolumeCallback : public AudioParameterCallback {
        void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
            const std::string& condition, const std::string& value) override;

        void OnHdiRouteStateChange(const std::string& networkId, bool enable) override {};
    };
    std::string GetModuleArgsLog(const AudioModuleInfo &audioModuleInfo);
    bool IsRemoteAdapter(const std::string &adapterName);

private:
    friend class PolicyCallbackImpl;

    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t DEFAULT_VOLUME_LEVEL = 7;
    static constexpr int32_t DP_DEFAULT_VOLUME_LEVEL = 25;
    static constexpr int32_t APP_MAX_VOLUME_LEVEL = 100;
    static constexpr int32_t APP_MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t APP_DEFAULT_VOLUME_LEVEL = 100;
    static constexpr int32_t CONST_FACTOR = 100;
    static constexpr int32_t DEFAULT_SAFE_VOLUME_TIMEOUT = 1080;
    static constexpr int32_t CONVERT_FROM_MS_TO_SECONDS = 1000;
    static constexpr float MIN_STREAM_VOLUME = 0.0f;
    static constexpr float MAX_STREAM_VOLUME = 1.0f;
    static constexpr int32_t MIN_VALID_VOLUME_DEGREE = 1;

    struct UserData {
        AudioAdapterManager *thiz;
        AudioStreamType streamType;
        float volume;
        bool mute;
        bool isCorked;
        uint32_t idx;
    };

    AudioAdapterManager()
        : ringerMode_(RINGER_MODE_NORMAL),
          audioPolicyKvStore_(nullptr),
          audioPolicyServerHandler_(DelayedSingleton<AudioPolicyServerHandler>::GetInstance()),
          audioDeviceManager_(AudioDeviceManager::GetAudioDeviceManager()),
          volumeDataMaintainer_(),
          audioActiveDevice_(AudioActiveDevice::GetInstance()),
          audioConnectedDevice_(AudioConnectedDevice::GetInstance()),
          audioIOHandleMap_(AudioIOHandleMap::GetInstance())
    {}

    AudioStreamType GetStreamIDByType(std::string streamType);
    int32_t ReInitKVStore();
    bool InitAudioPolicyKvStore(bool& isFirstBoot);
    void InitVolumeMap(bool isFirstBoot);
    bool LoadVolumeMap(void);
    void LoadVolumeValueMapFromDb();
    std::string GetVolumeKeyForKvStore(DeviceType deviceType, AudioStreamType streamType);
    void InitRingerMode(bool isFirstBoot);
    void InitMuteStatusMap(bool isFirstBoot);
    void InitSystemAppVolume(bool isFirstBoot);
    bool LoadMuteStatusMap(void);
    bool LoadMuteStatusMap(std::shared_ptr<AudioDeviceDescriptor> &device);
    std::string GetMuteKeyForKvStore(DeviceType deviceType, AudioStreamType streamType);
    std::string GetMuteKeyForDeviceType(DeviceType deviceType, std::string &type);
    void InitSystemSoundUriMap();
    void InitBootAnimationVolume();
    void InitSafeVolumeParams();
    void UpdateVolumeMapIndex();
    void GetVolumePoints(AudioVolumeType streamType, DeviceVolumeType deviceType,
        std::vector<VolumePoint> &volumePoints);
    uint32_t GetPositionInVolumePoints(std::vector<VolumePoint> &volumePoints, int32_t idx);
    void SaveBootAnimationVolumeToLocal(std::shared_ptr<AudioDeviceDescriptor> &device,
        AudioVolumeType volumeType);
    int32_t SetSystemVolumeToEffect(std::shared_ptr<AudioDeviceDescriptor> &device, AudioStreamType streamType);
    int32_t SetAppVolumeDb(int32_t appUid, DeviceType deviceType);
    void SetAppAudioVolume(int32_t appUid, float volumeDb);
    void SetAppAudioVolume(std::shared_ptr<AudioDeviceDescriptor> &device, int32_t appUid, float volumeDb);
    int32_t SetSystemAppVolumeDb(int32_t appUid, DeviceType deviceType);
    void SetSystemAppAudioVolume(int32_t appUid, float volumeDb);
    int32_t ClearVoipMuteByCallerUidInner(std::unordered_map<int32_t,
        std::unordered_map<int64_t, std::unordered_set<int32_t>>> &muteMap, int32_t uid, bool isRenderer);
    void SendOffloadVolume();
    bool GetStreamMuteInternal(std::shared_ptr<AudioDeviceDescriptor> &device, AudioStreamType streamType,
        int32_t zoneId = 0);
    bool GetStreamMuteForVolumeSet(std::shared_ptr<AudioDeviceDescriptor> device, AudioStreamType streamType,
        int32_t zoneId = 0);
    int32_t GetStreamVolumeInternal(std::shared_ptr<AudioDeviceDescriptor> &device, AudioStreamType streamType,
        int32_t zoneId = 0);
    int32_t SetRingerModeInternal(AudioRingerMode ringerMode);
    int32_t GetDefaultVolumeLevel(std::unordered_map<AudioStreamType, int32_t> &volumeLevelMapTemp,
        AudioVolumeType volumeType, DeviceType deviceType) const;
    void InitKVStoreInternal(void);
    void DeleteAudioPolicyKvStore();
    void TransferMuteStatus(void);
    void CloneMuteStatusMap(void);
    void CloneVolumeMap(void);
    void CloneSystemSoundUrl(void);
    void InitSafeStatus(bool isFirstBoot);
    void InitSafeTime(bool isFirstBoot);
    void ConvertSafeTime(void);
    void UpdateSafeVolume();
    void UpdateUsbSafeVolume(std::shared_ptr<AudioDeviceDescriptor> &device);
    void CheckAndDealMuteStatus(const DeviceType &deviceType, const AudioStreamType &streamType);
    void SetVolumeCallbackAfterClone();
    void SetFirstBoot(bool isFirst);
    bool IsPaRoute(uint32_t routeFlag);
    bool IsLowLatencyRoute(uint32_t routeFlag);
    void DepressVolume(float &volume, int32_t volumeLevel,
        AudioStreamType streamType, std::shared_ptr<AudioDeviceDescriptor> &device);
    float DepressMediaVolumeForHeadSet(float mediaVolume, float volumeReduction, float volumeLimit);
    float GetVolumeReductionRatio(AudioStreamType streamUsage);
    int32_t CreateHdiPort(std::shared_ptr<AudioPipeInfo> pipeInfo, AudioIOHandle &ioHandle);
    AudioIOHandle OpenPaAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex, std::string moduleArgs);
    AudioIOHandle OpenNotPaAudioPort(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex);
    void GetSinkIdInfoAndIdType(std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &idInfo, HdiIdType &idType);
    void GetSourceIdInfoAndIdType(std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &idInfo, HdiIdType &idType);
    int32_t IsHandleStreamMute(AudioStreamType streamType, bool mute, StreamUsage streamUsage);
    static void UpdateSinkArgs(const AudioModuleInfo &audioModuleInfo, std::string &args);
    static bool Match3daBusToken(const std::string &portName);
    static void ConvergeTo3daSink(const AudioModuleInfo &audioModuleInfo, std::string &deviceClass,
        std::string &idInfo);
    void UpdateVolumeForLowLatency(std::shared_ptr<AudioDeviceDescriptor> device, AudioVolumeType volumeType);
    bool IsDistributedVolumeType(AudioStreamType streamType);
    void GetHdiSourceTypeToAudioSourceAttr(IAudioSourceAttr &attr, int32_t sourceType) const;
    void UpdateSafeVolumeInner(std::shared_ptr<AudioDeviceDescriptor> &device);
    int32_t GetVolumeLevel(std::shared_ptr<AudioDeviceDescriptor> &device, AudioStreamType streamType);
    void SaveVolumeToDbAsync(std::shared_ptr<AudioDeviceDescriptor> desc,
        AudioStreamType streamType, const VolumeScale &volume, int32_t zoneId = 0);
    void SaveMuteToDbAsync(std::shared_ptr<AudioDeviceDescriptor> desc,
        AudioStreamType streamType, bool mute, int32_t zoneId = 0);
    int32_t SetVolumeDbForDeviceInPipe(std::shared_ptr<AudioDeviceDescriptor> desc,
        AudioStreamType streamType, int32_t zoneId = 0);
    float CalculateVolumeDbExt(int32_t volumeInt, int32_t limit = MAX_VOLUME_DEGREE);
    float CalculateVolumeDbNonlinearExt(AudioStreamType streamType, DeviceType deviceType,
        int32_t volumeDegree, bool isSupportWearDetection = false);
    void SaveVolumeData(std::shared_ptr<AudioDeviceDescriptor> device, AudioStreamType streamType,
        int32_t volumeLevel, bool updateDb = false, bool updateMem = false, int32_t zoneId = 0);
    int32_t GetStreamVolumeDegreeInternal(std::shared_ptr<AudioDeviceDescriptor> &device,
        AudioStreamType streamType, int32_t zoneId = 0);
    void SetPrimarySinkExist(bool isPrimarySinkExist);
    int32_t StopAudioPort(std::string oldSinkName);
    void DealDoNotDisturbStatus();
    void DealDoNotDisturbStatusWhiteList();
    void HandleCloneStateChanged();
    void UpdateRingerMuteByRingerMode(std::shared_ptr<AudioDeviceDescriptor> device);
    void SendVolumeKeyEventCbWithUpdateUi(AudioStreamType streamType,
        std::shared_ptr<AudioDeviceDescriptor> device);
    void RegistAdapterManagerCallback(std::string networkId);
    void SetRemoteVolumeForPassThroughDevice(std::shared_ptr<AudioDeviceDescriptor> device, int32_t volumeLevel);
    void UpdateVolumeWhenPassThroughDeviceConnect(std::shared_ptr<AudioDeviceDescriptor> device);
    void RedirectVolumeType(std::shared_ptr<AudioStreamDescriptor> streamDescriptor,
        AudioVolumeType &volumeType);
    void SaveSpecifiedDeviceVolumeInner(AudioStreamType streamType, int32_t volumeLevel,
        DeviceType deviceType, int32_t zoneId);
    void SaveSystemAppVolumeLevelToDb();
    void SaveSystemAppVolumeMuteStatusToDb(bool isAsync);
    int32_t RestoreSystemAppVolumeLevel();
    int32_t RestoreSystemAppVolumeMuteStatus();
    bool IsStreamTypeInUse(AudioStreamType streamType, std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsDirectRoute(uint32_t routeFlag) const;
    void UpdatePipeVolumeWhenVgsStatusChanged();
    void SetPipeVolume(AudioVolumeType volumeType, std::shared_ptr<AudioDeviceDescriptor> device,
        const std::unordered_set<AudioIOHandle>& pipeIds, VolumeValue volumeValue, float volumeDb);
    void UpdateTypeVolumeForPipe(AudioVolumeType volumeType,
        std::shared_ptr<AudioDeviceDescriptor> device, const std::unordered_set<AudioIOHandle>& pipeIds,
        int32_t zoneId = 0);
    void AdjustVolumeForSpecialDevice(std::shared_ptr<AudioDeviceDescriptor> device,
        AudioVolumeType volumeType, bool &isMuted, int32_t &volumeLevel, float &volumeDb);
    void AjustVolumeForSpecialType(AudioStreamType streamType, float &volumeDb);
    void ApplySystemVolumeProxy(int32_t zoneId, std::shared_ptr<AudioDeviceDescriptor> device,
        bool &isMuted, float &volumeDb);

    template<typename T>
    std::vector<uint8_t> TransferTypeToByteArray(const T &t)
    {
        return std::vector<uint8_t>(reinterpret_cast<uint8_t *>(const_cast<T *>(&t)),
            reinterpret_cast<uint8_t *>(const_cast<T *>(&t)) + sizeof(T));
    }

    template<typename T>
    T TransferByteArrayToType(const std::vector<uint8_t> &data)
    {
        if (data.size() != sizeof(T) || data.size() == 0) {
            constexpr int tSize = sizeof(T);
            uint8_t tContent[tSize] = { 0 };
            return *reinterpret_cast<T *>(tContent);
        }
        return *reinterpret_cast<T *>(const_cast<uint8_t *>(&data[0]));
    }

    std::shared_ptr<AudioServiceAdapter> audioServiceAdapter_;
    std::vector<AudioStreamType> defaultVolumeTypeList_;
    std::unordered_map<AudioStreamType, int> minVolumeIndexMap_;
    std::unordered_map<AudioStreamType, int> maxVolumeIndexMap_;
    std::mutex systemSoundMutex_;
    std::unordered_map<std::string, std::string> systemSoundUriMap_;
    StreamVolumeInfoMap streamVolumeInfos_;
    LowerVolumeInfoMap lowerVolumeInfos_;
    std::atomic<AudioRingerMode> ringerMode_ = RINGER_MODE_NORMAL;
    int32_t safeVolume_ = 0;
    int32_t safeVolumeDegree_ = 0;
    SafeStatus safeStatus_ = SAFE_ACTIVE;
    SafeStatus safeStatusBt_ = SAFE_ACTIVE;
    SafeStatus safeStatusSle_ = SAFE_ACTIVE;
    int64_t safeActiveTime_ = 0;
    int64_t safeActiveBtTime_ = 0;
    int64_t safeActiveSleTime_ = 0;
    int32_t safeVolumeTimeout_ = DEFAULT_SAFE_VOLUME_TIMEOUT;
    int32_t safeActiveVolume_ = 0;
    int32_t safeActiveBtVolume_ = 0;
    int32_t safeActiveSleVolume_ = 0;
    std::unordered_map<int32_t, std::unordered_map<int64_t, std::unordered_set<int32_t>>> voipRendererMuteMap_;
    std::unordered_map<int32_t, std::unordered_map<int64_t, std::unordered_set<int32_t>>> voipCapturerMuteMap_;
    bool isWiredBoot_ = true;
    bool isBtBoot_ = true;
    bool isSleBoot_ = true;
    int32_t curActiveCount_ = 0;
    int32_t volumeAdjustZoneId_ = 0;
    bool isSafeBoot_ = true;
    bool isVgsVolumeSupported_ = false;
    std::shared_ptr<AudioAdapterManagerHandler> handler_ = nullptr;

    std::shared_ptr<SingleKvStore> audioPolicyKvStore_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_;
    AudioStreamRemovedCallback *sessionCallback_ = nullptr;
    AudioDeviceManager &audioDeviceManager_;
    VolumeDataMaintainer volumeDataMaintainer_;
    AudioActiveDevice &audioActiveDevice_;
    AudioConnectedDevice &audioConnectedDevice_;
    AudioIOHandleMap &audioIOHandleMap_;
    std::atomic<bool> isPrimarySinkExist_ {true};
    std::atomic<int64_t> legacyHighVolumeCheckTime_ = 0;

    bool isVolumeUnadjustable_ = false;
    bool testModeOn_ {false};
    std::atomic<float> getSystemVolumeInDb_  {0.0f};
    std::atomic<bool> isSleVoiceStatus_ {false};
    // Abs volume scene/mute states of A2DP devices are keyed by the device mac address.
    mutable std::mutex absVolumeStateMutex_;
    std::unordered_map<std::string, bool> absVolumeSceneMap_;
    std::unordered_map<std::string, bool> absVolumeMuteMap_;
    std::atomic<bool> disableVolumeLimit_{false};
    std::atomic<bool> isAbsVolumeMuteNearlink_ {false};
    bool useNonlinearAlgo_ = false;
    bool isNeedCopyVolumeData_ = false;
    bool isNeedCopyMuteData_ = false;
    bool isNeedCopyRingerModeData_ = false;
    bool isNeedCopySystemUrlData_ = false;
    bool isLoaded_ = false;
    bool isAllCopyDone_ = false;
    bool isNeedConvertSafeTime_ = false;
    bool isDataShareReady_ = false;
    sptr<IStandardAudioService> audioServerProxy_ = nullptr;
    std::unordered_map<uint32_t, uint32_t> ioHandleSessionMap_;
    std::unordered_map<uint32_t, float> ioHandleVolumeMap_;
    std::mutex audioVolumeMutex_;
    std::mutex activeDeviceMutex_;
    std::mutex setVoiceStatusMutex_;
    std::mutex setMaxVolumeMutex_;
    AppConfigVolume appConfigVolume_;
    std::shared_ptr<FixedSizeList<RingerModeAdjustInfo>> saveRingerModeInfo_ =
        std::make_shared<FixedSizeList<RingerModeAdjustInfo>>(MAX_CACHE_AMOUNT);
    bool isDpReConnect_ = false;
    sptr<IStandardAudioPolicyManagerListener> deviceVolumeBehaviorListener_;
    bool isA2DPPreActive_ = false;
    std::atomic<float> volumeLimit_ = MAX_STREAM_VOLUME;
    std::atomic<bool> isCastingConnect_ = false;
    std::mutex ringerNoMuteDeviceMutex_;
    std::shared_ptr<AudioDeviceDescriptor> ringerNoMuteDevice_ = nullptr;
    std::shared_ptr<RemoteVolumeCallback> remoteVolumeCallback_ = nullptr;
    std::mutex deviceConnectMutex_;
    std::mutex offloadSessionMutex_;
    int32_t cloneState_ = 0;
};

class PolicyCallbackImpl : public AudioServiceAdapterCallback {
public:
    explicit PolicyCallbackImpl(AudioAdapterManager *audioAdapterManager)
    {
        audioAdapterManager_ = audioAdapterManager;
    }

    ~PolicyCallbackImpl()
    {
        AUDIO_WARNING_LOG("Destructor PolicyCallbackImpl");
    }

    void OnAudioStreamRemoved(const uint64_t sessionID)
    {
        AUDIO_DEBUG_LOG("PolicyCallbackImpl OnAudioStreamRemoved: Session ID %{public}" PRIu64"", sessionID);
        if (audioAdapterManager_->sessionCallback_ == nullptr) {
            AUDIO_DEBUG_LOG("PolicyCallbackImpl audioAdapterManager_->sessionCallback_ == nullptr"
                "not firing OnAudioStreamRemoved");
        } else {
            audioAdapterManager_->sessionCallback_->OnAudioStreamRemoved(sessionID);
        }
    }

private:
    AudioAdapterManager *audioAdapterManager_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_PULSEAUDIO_ADAPTER_MANAGER_H
