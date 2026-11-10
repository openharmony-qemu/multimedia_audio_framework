/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#ifndef AUDIO_PIPE_MANAGER_H
#define AUDIO_PIPE_MANAGER_H

#include <functional>
#include <memory>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "audio_stream_descriptor.h"
#include "audio_module_info.h"
#include "audio_pipe_info.h"
#include "audio_core_config_manager.h"
#include "audio_concurrency_manager.h"
#include "audio_policy_utils.h"
#include "core_service_handler.h"
#include "iaudio_policy_interface.h"


namespace OHOS {
namespace AudioStandard {
static const std::unordered_set<DeviceType> g_ultraFastDevicesSet = {
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_USB_ARM_HEADSET
};

class AudioPipeManager {
public:
    using RequiredPipesReadyCallback = std::function<void()>;

    AudioPipeManager();
    virtual ~AudioPipeManager();

    bool Init();
    bool GetUltraFastFlag();
    bool GetUltraFastFlag(const std::string &adapterName, AudioPipeRole role);
    bool GetArmUsbUltraFastFlag();
    bool GetTrueUltraFastFlag();
    bool PreferMultiChannelPipe(std::shared_ptr<AudioStreamDescriptor> &desc);
    uint32_t GetStreamPropInfoSize(const std::string &adapterName, const std::string &pipeName,
        const std::string &networkId);
    bool GetUpdateRouteSupport();
    bool GetHasEarpiece();
    bool GetAdapterInfoFlag();
    bool GetAdapterInfoByType(AudioAdapterType type, std::shared_ptr<PolicyAdapterInfo> &info);
    bool GetModuleListByType(ClassType type, std::list<AudioModuleInfo>& moduleList);
    void UpdateDynamicCapturerConfig(ClassType type, const AudioModuleInfo moduleInfo);
    DirectPlaybackMode GetDirectPlaybackSupport(std::shared_ptr<AudioDeviceDescriptor> desc,
        const AudioStreamInfo &streamInfo);
    void UpdateStreamPropInfo(const std::string &adapterName, const std::string &pipeName,
        const std::list<DeviceStreamInfo> &deviceStreamInfo, const std::list<std::string> &supportDevices,
        const std::string &networkId);
    void ClearStreamPropInfo(const std::string &adapterName, const std::string &pipeName,
        const std::string &networkId);
    uint32_t GetRouteFlag(std::shared_ptr<AudioStreamDescriptor> &desc);
    void GetStreamPropInfo(std::shared_ptr<AudioStreamDescriptor> &desc, std::shared_ptr<PipeStreamPropInfo> &info);
#ifdef MULTI_BUS_ENABLE
    void GetStreamPropInfo(std::shared_ptr<AudioStreamDescriptor> &desc, std::shared_ptr<PipeStreamPropInfo> &info,
        const std::vector<std::string> &busAddresses);
#endif
    void GetGlobalConfigs(PolicyGlobalConfigs &globalConfigs);
    void SetEcEnableState(bool ecEnableState);
    AudioSampleFormat GetFastFormat() const;
    bool IsSupportInnerCaptureOffload();
    bool IsSupportIndependentPipe();
    bool NeedUidCheckForOffloadPipe(const std::shared_ptr<AudioStreamDescriptor> &desc);
    int32_t GetMaxRendererInstances();
    void GetDeviceClassInfo(std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo);

    static std::shared_ptr<AudioPipeManager> GetPipeManager()
    {
        static std::shared_ptr<AudioPipeManager> instance = std::make_shared<AudioPipeManager>();
        return instance;
    }

    void AddAudioPipeInfo(std::shared_ptr<AudioPipeInfo> info);
    void SetRequiredPipesReadyCallback(RequiredPipesReadyCallback callback);
    void RemoveAudioPipeInfo(std::shared_ptr<AudioPipeInfo> info);
    void RemoveAudioPipeInfo(AudioIOHandle id);
    void UpdateAudioPipeInfo(std::shared_ptr<AudioPipeInfo> newPipe);
    void Assign(std::shared_ptr<AudioPipeInfo> dst, std::shared_ptr<AudioPipeInfo> src);
    bool IsSamePipe(std::shared_ptr<AudioPipeInfo> info, std::shared_ptr<AudioPipeInfo> cmpInfo);

    virtual const std::vector<std::shared_ptr<AudioPipeInfo>> GetPipeList();
    std::vector<std::shared_ptr<AudioPipeInfo>> GetUnusedPipe();
    std::vector<std::shared_ptr<AudioPipeInfo>> GetUnusedPipeForCapturer(
        const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    std::vector<std::shared_ptr<AudioPipeInfo>> GetUnusedRecordPipe();
    std::shared_ptr<AudioPipeInfo> GetPipeinfoByNameAndFlag(const std::string adapterName, const uint32_t routeFlag);
    std::shared_ptr<AudioPipeInfo> FindDirectOutputPipe();
    std::shared_ptr<AudioPipeInfo> GetPipeinfoById(AudioIOHandle id);
    std::string GetAdapterNameBySessionId(uint32_t sessionId);
    std::string GetModuleNameBySessionId(uint32_t sessionId);
    std::string GetClassNameBySessionId(uint32_t sessionId);
    std::shared_ptr<AudioDeviceDescriptor> GetProcessDeviceInfoBySessionId(uint32_t sessionId,
        AudioStreamInfo &streamInfo);
    // Bus address of the 3da direct bus port pipe (module name when busAddress is empty);
    // empty when this device has no 3da pipe.
    std::string Get3daDirectBusAddress();

    void StartClient(uint32_t sessionId);
    void PauseClient(uint32_t sessionId);
    void StandbyClient(uint32_t sessionId);
    void ClearStandbyFlag(uint32_t sessionId);
    // Set mute silent flag, return whether isSilent_ changed
    bool SetMuteSilentState(uint32_t sessionId, bool isMuteSilent);
    // Set data transfer silent flag, return whether isSilent_ changed
    bool SetDataTransSilentState(uint32_t sessionId, bool isDataTransSilent);
    // Set notification volume silent flag
    void NotificationVolumeSilentClient(uint32_t sessionId);
    // Clear notification volume silent flag, return whether isSilent_ changed
    bool ClearNotificationVolumeSilentFlag();
    // Check whether stream is silent (isSilent_)
    bool IsStreamSilent(uint32_t sessionId);
    void StopClient(uint32_t sessionId);
    void RemoveClient(uint32_t sessionId);

    void AddStreamToPipe(std::shared_ptr<AudioPipeInfo> pipeInfo,
        std::shared_ptr<AudioStreamDescriptor> streamDesc, AudioPipeAction action);
    void ClearPipeStreams(std::shared_ptr<AudioPipeInfo> pipeInfo);

    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllOutputStreamDescs();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllOutputStreamDescsIncludingModem();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllOutputStreamDescsIncludingModemCopy();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllOutputStreamDescsInfo();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllInputStreamDescs();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllInputStreamDescsCopy();
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetStreamDescsByIoHandle(AudioIOHandle id);
    std::shared_ptr<AudioStreamDescriptor> GetStreamDescById(uint32_t sessionId);
    std::shared_ptr<AudioStreamDescriptor> GetStreamDescByIdInner(uint32_t sessionId);
    int32_t GetStreamCount(const std::string adapterName, const uint32_t routeFlag);
    uint32_t GetPaIndexByIoHandle(AudioIOHandle id);
    uint32_t QueryPipeIdBySessionId(uint32_t sessionId);
    void UpdateRendererPipeInfos(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    void UpdateCapturerPipeInfos(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    uint32_t PcmOffloadSessionCount();
    int32_t GetClientUidBySessionId(uint32_t sessionId);

    void Dump(std::string &dumpString);
    bool IsModemCommunicationIdExist();
    bool IsModemCommunicationIdExist(uint32_t sessionId);
    void AddModemCommunicationId(uint32_t sessionId, std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void RemoveModemCommunicationId(uint32_t sessionId);
    std::shared_ptr<AudioStreamDescriptor> GetModemCommunicationStreamDescById(uint32_t sessionId);
    std::shared_ptr<AudioStreamDescriptor> GetModemCommunicationStreamDesc();
    std::shared_ptr<AudioStreamDescriptor> GetModemCommunicationStreamDescCopy();
    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> GetModemCommunicationMap();
    void UpdateModemStreamStatus(AudioStreamStatus streamStatus);
    void UpdateModemStreamDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &deviceDescs);
    
    bool IsTransferCallOrRingtoneScene();
    bool HasTransferCallOrRingtoneStream();
    void UpdateRingAndVoipStreamStatus(const AudioScene audioScene);
    void UpdateRingAndVoipStreamDevice(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &ringDeviceDescs,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &voipDeviceDescs);
    bool CheckRingAndVoipStreamRunning();
    std::shared_ptr<AudioStreamDescriptor> GetStreamDescForAudioScene(const AudioScene audioScene);
    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> GetRingAndVoipDescMap();
    bool IsModemStreamDeviceChanged(std::shared_ptr<AudioDeviceDescriptor> &deviceDescs);
    std::shared_ptr<AudioPipeInfo> GetNormalSourceInfo(bool isEcFeatureEnable);
    std::vector<uint32_t> GetStreamIdsByUidAndPid(int32_t uid, int32_t pid);
    std::vector<uint32_t> GetStreamIdsByPid(int32_t pid);
    void UpdateOutputStreamDescsByIoHandle(AudioIOHandle id,
        std::vector<std::shared_ptr<AudioStreamDescriptor>> &descs);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetAllCapturerStreamDescs();
    std::shared_ptr<AudioPipeInfo> FindPipeBySessionId(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList,
        uint32_t sessionId);
    bool IsStreamUsageActive(const StreamUsage &usage);
    bool HasRingerOrAlarmStreamRunning();
    bool IsInterphoneStreamActive();
    bool IsInterphoneCapturerActive();
    bool IsVoipOrCellularStreamActive();
    int32_t IsCaptureVoipCall();
    uint32_t GetPaIndexByName(std::string portName);
    bool HasPrimarySink();
    bool HasRunningStream(uint32_t sessionId = 0);
    bool IsA2dpUsed();
    bool HasFastOutputPipe();
    bool HasUltraFastStreamRequest();
    bool IsStreamUseUltraFastRoute(uint32_t sessionId);
    bool IsOnPrimaryAdapter(uint32_t sessionId);
    bool HasActiveStreamOnPrimary(uint32_t inPastMs);
    void AddSingleSwitchStream(uint32_t sessionId, std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void RemoveSwitchStream(uint32_t sessionId);
    bool IsSwitchStreamExist(uint32_t sessionId);
    void AddSwitchStreams(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void UpdateNewDeviceDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDeviceDesc);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetStreamDescsByStreamUsage(StreamUsage streamUsage);
    std::vector<std::shared_ptr<AudioStreamDescriptor>> GetStreamDescsBySourceType(SourceType sourceType);
    void UpdateNewDeviceDescWithCheck(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> newDeviceDesc);
    void UpdateOldDeviceDesc(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> oldDeviceDesc);
    StreamUsage GetLastestRunningCallStreamUsage();
    bool HasUltraFastPipe();
    bool IsUltraFastSupportedDevice(DeviceType deviceType);
    bool IsTrueUltraFastSupportedDevice(DeviceType deviceType);
    uint32_t GetUltraFastStreamCount();
    bool IsOnlyAppLevelHdPlaySupported(std::shared_ptr<AudioPipeInfo> pipeInfo);
    std::vector<std::string> GetNoRunningPipeModuleName();
    bool CheckNoRunningCondition(std::shared_ptr<AudioPipeInfo> pipeInfo);
    bool CheckRemoteNoRunningCondition(std::shared_ptr<AudioPipeInfo> pipeInfo);
    bool IsPipeNeedStop(std::shared_ptr<AudioPipeInfo> pipeInfo);
    std::string GetModuleName(std::shared_ptr<AudioPipeInfo> pipeInfo);
    bool IsValidRenderSessionId(const uint32_t sessionId);

    std::vector<PipeDeviceVolumeGroup> GetAllPipeDeviceVolumeInfo();
    std::vector<PipeDeviceVolumeGroup> GetPipeDeviceVolumeInfoForPipe(AudioIOHandle id);
    std::vector<PipeDeviceVolumeGroup> GetPipeDeviceVolumeInfoForPipe(std::shared_ptr<AudioPipeInfo> pipeInfo);
    std::vector<PipeDeviceVolumeGroup> GetPipeDeviceVolumeInfoForDevice(
        std::shared_ptr<AudioDeviceDescriptor> deviceDesc);
    
    bool HasPausedFastPipeInDelayedSwitch();
    std::string GetPausedFastPipeModuleNameInDelayedSwitch();

    void AddFocusStandby(uint32_t sessionId);
    void RemoveFocusStandby(uint32_t sessionId);
    bool IsFocusStandby(uint32_t sessionId);

private:
    // Filter unused pipes shared by renderer and capturer pipe queries.
    std::vector<std::shared_ptr<AudioPipeInfo>> GetUnusedPipeInternal(
        const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    bool IsSpecialPipe(uint32_t routeFlag);
    bool IsNormalRecordPipe(std::shared_ptr<AudioPipeInfo> pipeInfo);
    std::shared_ptr<AudioPipeInfo> GetPipeByModuleAndFlag(const std::string moduleName, const uint32_t routeFlag);
    AudioStreamInfo DecideStreamInfo(const std::shared_ptr<AudioPipeInfo> pipeInfo,
        const std::shared_ptr<AudioDeviceDescriptor> deviceDesc);
    void ProcessPipeForVolumeInfo(std::vector<PipeDeviceVolumeGroup>& result,
        std::shared_ptr<AudioPipeInfo> pipeInfo);
    AudioVolumeType GetVolumeTypeFromStreamDesc(
        std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void AddOrUpdatePipeDeviceVolumeGroup(std::vector<PipeDeviceVolumeGroup>& result,
        AudioIOHandle pipeId, std::shared_ptr<AudioDeviceDescriptor> device, AudioVolumeType volumeType);
    bool AddAudioPipeInfoInner(std::shared_ptr<AudioPipeInfo> info);
    bool UpdateRendererPipeInfosInner(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    bool UpdateCapturerPipeInfosInner(std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    void InitRequiredPipes();
    std::string GetRequiredPipeKey(const std::string &adapterName, uint32_t routeFlag);
    bool IsRequiredPipesReadyLocked();
    bool UpdateRequiredPipesReadyStateLocked(const std::shared_ptr<AudioPipeInfo> &pipeInfo);
    bool UpdateRequiredPipesReadyStateLocked(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    bool UpdateRequiredPipeAddedStateLocked(const std::shared_ptr<AudioPipeInfo> &pipeInfo);
    bool UpdateRequiredPipeAddedStateLocked(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeInfos);
    void NotifyRequiredPipesReady();

    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> modemCommunicationIdMap_{};
    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> ringAndVoipDescMap_{};
    std::vector<std::shared_ptr<AudioPipeInfo>> curPipeList_{};
    std::shared_mutex pipeListLock_;
    std::atomic<int32_t> transferCallOrRingtoneCount_{0};
    std::unordered_map<uint32_t, std::shared_ptr<AudioStreamDescriptor>> switchStreamMap_{};
    mutable std::shared_mutex switchStreamMapLock_;
    std::set<uint32_t> focusStandbySet_;
    mutable std::shared_mutex focusStandbySetLock_;
    AudioCoreConfigManager& audioConfigManager_;
    AudioConcurrencyManager &audioConcurrencyManager_;
    bool requiredPipesInited_ = false;
    bool requiredPipesReady_ = false;
    std::unordered_set<std::string> requiredPipeKeys_;
    std::unordered_set<std::string> addedRequiredPipeKeys_;
    std::mutex requiredPipesReadyCallbackMutex_;
    RequiredPipesReadyCallback requiredPipesReadyCallback_;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PIPE_MANAGER_H
