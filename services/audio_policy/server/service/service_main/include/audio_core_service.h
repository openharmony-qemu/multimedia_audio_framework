/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_CORE_SERVICE_H
#define AUDIO_CORE_SERVICE_H
#include <mutex>

#include "audio_policy_server_handler.h"
#include "i_core_service_provider.h"
#include "idevice_status_observer.h"
#include "audio_stream_descriptor.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "microphone_descriptor.h"
#include "audio_stream_change_info.h"
#include "audio_active_device.h"
#include "audio_scene_manager.h"
#include "audio_usr_select_manager.h"
#include "audio_volume_manager.h"
#include "audio_capturer_session.h"
#include "audio_device_manager.h"
#include "audio_connected_device.h"
#include "audio_device_status.h"
#include "audio_effect_service.h"
#include "audio_microphone_descriptor.h"
#include "audio_recovery_device.h"
#include "device_status_listener.h"
#include "core_service_provider_stub.h"
#include "audio_pipe_info.h"
#include "audio_service_enum.h"
#include "audio_pipe_manager.h"
#include "audio_session_service.h"
#include "audio_pipe_selector.h"
#include "audio_policy_config_manager.h"
#include "audio_core_service_utils.h"
#include "sle_audio_device_manager.h"
#include "audio_event_utils.h"
#include "audio_stream_id_allocator.h"
#include "i_hpae_soft_link.h"
#include "audio_injector_policy.h"
#include "client_type_manager.h"
namespace OHOS {
namespace AudioStandard {
enum OffloadType {
    LOCAL_OFFLOAD,
    REMOTE_OFFLOAD,
    OFFLOAD_TYPE_NUM,
};

static constexpr uint32_t CONCURRENT_CAPTURE_DFX_THRESHOLD = 2;
static constexpr uint32_t CONCURRENT_CAPTURE_DFX_MSG_ARRAY_MAX = 5;
static constexpr uint32_t CONCURRENT_CAPTURE_DFX_HDI_SEGMENTS = 2;
struct ConcurrentCaptureDfxResult {
    std::vector<std::string> existingAppName{};
    std::vector<uint8_t> existingAppState{};
    std::vector<uint8_t> existingSourceType{};
    std::vector<uint8_t> existingCaptureState{};
    std::vector<uint32_t> existingCreateDuration{};
    std::vector<uint32_t> existingStartDuration{};
    std::vector<bool> existingFastFlag{};
    uint8_t hdiSourceType{};
    std::string hdiSourceAlg{};
    uint8_t deviceType{};
};

class AudioA2dpOffloadManager;
class AudioCoreService : public enable_shared_from_this<AudioCoreService> {
public:
    class EventEntry : public ICoreServiceProvider, public IDeviceStatusObserver {
    public:
        EventEntry(std::shared_ptr<AudioCoreService> coreService);
        void RegistCoreService();

        // Stream operations
        int32_t CreateRendererClient(
            std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId,
            std::string &networkId);
        int32_t CreateCapturerClient(
            std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId);

        // ICoreServiceProvider
        int32_t UpdateSessionOperation(uint32_t sessionId, SessionOperation operation,
            SessionOperationMsg opMsg = SESSION_OP_MSG_DEFAULT) override;
        int32_t ReloadCaptureSession(uint32_t sessionId, SessionOperation operation) override;
        int32_t SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionId,
            const StreamUsage streamUsage, bool isRunning, bool skipForce = false) override;
        std::string GetAdapterNameBySessionId(uint32_t sessionId) override;
        int32_t GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo,
            AudioStreamInfo &streamInfo, bool isReloadProcess = false) override;
        uint32_t GenerateSessionId() override;
        int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId);
        void OnCheckActiveMusicTime(const std::string &reason) override;

        // IDeviceStatusObserver
        void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command) override;
        void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
            const std::string &macAddress, const std::string &deviceName,
            const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false) override;
        void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override;
        void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false) override;
        void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status) override;
        void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected) override;
        void OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
            const std::string &deviceName, const AudioStreamInfo &streamInfo) override;
        void OnServiceConnected(AudioServiceIndex serviceIndex) override;
        void OnServiceDisconnected(AudioServiceIndex serviceIndex) override;
        void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
            sptr<AudioRendererFilter> filter = nullptr) override;
        void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress) override;
        uint32_t GetPaIndexByPortName(const std::string &portName) override;

        // Functions related to assignment operations - device related
        int32_t SetAudioScene(AudioScene audioScene, const int32_t uid = INVALID_UID, const int32_t pid = INVALID_PID);
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);
        int32_t SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid = INVALID_UID);
        int32_t SetInputDevice(const DeviceType deviceType, const uint32_t sessionID,
            const SourceType sourceType, bool isRunning);
        int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
            const int32_t uid = INVALID_UID);
        int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
            const sptr<IRemoteObject> &object, const int32_t apiVersion);
        int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
        void RegisteredTrackerClientDied(pid_t uid, pid_t pid);
        bool ConnectServiceAdapter();
        void OnReceiveUpdateDeviceNameEvent(const std::string macAddress, const std::string deviceName);
        int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode = 0);
        int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc);
        int32_t SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &audioDeviceDescriptor,
            int32_t uid);
        std::shared_ptr<AudioDeviceDescriptor> GetSelectedInputDeviceByUid(int32_t uid);
        int32_t ClearSelectedInputDeviceByUid(int32_t uid);
        int32_t PreferBluetoothAndNearlinkRecordByUid(int32_t uid,
            BluetoothAndNearlinkPreferredRecordCategory category);
        BluetoothAndNearlinkPreferredRecordCategory GetPreferBluetoothAndNearlinkRecordByUid(int32_t uid);
        void NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value);
        int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo, AudioStreamInfo streamInfo);
        void CloseWakeUpAudioCapturer();
        void OnCapturerSessionRemoved(uint64_t sessionID);
        int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason);
        void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
            const AudioStreamDeviceChangeReasonExt reason);
        void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo);
        int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
        int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
        int32_t SetSessionDefaultOutputDevice(const int32_t callerPid, const DeviceType &deviceType);

        // Functions related to get operations - device related
        bool IsArmUsbDevice(const AudioDeviceDescriptor &deviceDesc);
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
            AudioRendererInfo &rendererInfo, std::string networkId = LOCAL_NETWORK_ID);
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
            AudioCapturerInfo &captureInfo, int32_t uid, std::string networkId = LOCAL_NETWORK_ID);
        std::shared_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();
        std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();
        std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);
        int32_t GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
            &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission);
        int32_t GetCurrentCapturerChangeInfos(vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
            bool hasBTPermission, bool hasSystemPermission);
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(AudioDeviceUsage audioDevUsage);
        int32_t FetchOutputDeviceAndRoute(std::string caller,
            const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
        int32_t FetchInputDeviceAndRoute(std::string caller,
            const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
        int32_t GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo, const std::string &bundleName);
        int32_t GetSessionDefaultOutputDevice(const int32_t callerPid, DeviceType &deviceType);
        int32_t GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo);
        std::vector<sptr<VolumeGroupInfo>> GetVolumeGroupInfos();
        int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config) override;
        int32_t ReleaseOffloadPipe(AudioIOHandle id, uint32_t paIndex, OffloadType type);
        int32_t SetRendererTarget(RenderTarget target, RenderTarget lastTarget, uint32_t sessionId) override;
        int32_t StartInjection(uint32_t sessionId) override;
        void RemoveIdForInjector(uint32_t streamId) override;
        void ReleaseCaptureInjector() override;
        void RebuildCaptureInjector(uint32_t streamId) override;
        int32_t A2dpOffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize,
            uint32_t &timeStamp) override;
        int32_t CaptureConcurrentCheck(uint32_t sessionId) override;
private:
        std::shared_ptr<AudioCoreService> coreService_;
        std::shared_mutex eventMutex_;
    };

    // Ctor & dtor
    AudioCoreService();
    ~AudioCoreService();

    // Called by AudioPolicyServer
    static std::shared_ptr<AudioCoreService> GetCoreService();
    void Init();
    void DeInit();
    void SetCallbackHandler(std::shared_ptr<AudioPolicyServerHandler> handler);
    std::shared_ptr<EventEntry> GetEventEntry();
    bool IsStreamBelongToUid(const uid_t uid, const uint32_t sessionId);
    void DumpPipeManager(std::string &dumpString);
    void DumpSelectHistory(std::string &dumpString);
    void SetAudioRouteCallback(uint32_t sessionId, const sptr<IRemoteObject> &object);
    void UnsetAudioRouteCallback(uint32_t sessionId);

    // Called by EventEntry - with lock
    // Stream operations
    int32_t CreateRendererClient(
        std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId,
        std::string &networkId);
    void SetPreferredInputDeviceIfValid(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void WriteDesignateAudioCaptureDeviceEvent(SourceType sourceType, int32_t deviceType, bool isNormalSelection);
    int32_t CreateCapturerClient(
        std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId);
    int32_t StartClient(uint32_t sessionId);
    int32_t PauseClient(uint32_t sessionId);
    int32_t StopClient(uint32_t sessionId);
    int32_t ReleaseClient(uint32_t sessionId, SessionOperationMsg opMsg = SESSION_OP_MSG_DEFAULT);

    // ICoreServiceProvider from EventEntry
    int32_t SetDefaultOutputDevice(
        const DeviceType deviceType, const uint32_t sessionID, const StreamUsage streamUsage, bool isRunning,
        bool skipForce = false);
    std::string GetAdapterNameBySessionId(uint32_t sessionId);
    int32_t GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo,
        AudioStreamInfo &streamInfo);
    uint32_t GenerateSessionId();
    int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId);
    void OnCheckActiveMusicTime(const std::string &reason);

    // IDeviceStatusObserver from EventEntry
    void OnDeviceInfoUpdated(AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command);
    void OnDeviceStatusUpdated(DeviceType devType, bool isConnected,
        const std::string &macAddress, const std::string &deviceName,
        const AudioStreamInfo &streamInfo, DeviceRole role = DEVICE_ROLE_NONE, bool hasPair = false);
    void OnDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);
    void OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop = false);
    void OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status);
    void OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected);
    void OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
        const std::string &deviceName, const AudioStreamInfo &streamInfo);
    int32_t OnServiceConnected(AudioServiceIndex serviceIndex);
    uint32_t GetPaIndexByPortName(const std::string &portName);
    void OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
        sptr<AudioRendererFilter> filter = nullptr);
    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress);

    // Functions related to assignment operations - device related
    int32_t SetAudioScene(AudioScene audioScene, const int32_t uid = INVALID_UID, const int32_t pid = INVALID_PID);
    bool IsArmUsbDevice(const AudioDeviceDescriptor &deviceDesc);
    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid = INVALID_UID);
    int32_t SetInputDevice(const DeviceType deviceType, const uint32_t sessionID,
            const SourceType sourceType, bool isRunning);
    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
        const int32_t uid = INVALID_UID);
    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object, const int32_t apiVersion);
    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void RegisteredTrackerClientDied(pid_t uid, pid_t pid);
    bool ConnectServiceAdapter();
    void OnReceiveUpdateDeviceNameEvent(const std::string macAddress, const std::string deviceName);
    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode = 0);
    void NotifyDistributedOutputChange(const AudioDeviceDescriptor &deviceDesc);
    int32_t SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc);
    int32_t SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &selectedDesc, int32_t uid);
    std::shared_ptr<AudioDeviceDescriptor> GetSelectedInputDeviceByUid(int32_t uid);
    int32_t ClearSelectedInputDeviceByUid(int32_t uid);
    int32_t PreferBluetoothAndNearlinkRecordByUid(int32_t uid, BluetoothAndNearlinkPreferredRecordCategory category);
    BluetoothAndNearlinkPreferredRecordCategory GetPreferBluetoothAndNearlinkRecordByUid(int32_t uid);
    void NotifyRemoteRenderState(std::string networkId, std::string condition, std::string value);
    int32_t OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo, AudioStreamInfo streamInfo);
    void CloseWakeUpAudioCapturer();
    int32_t SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config);
    void OnCapturerSessionRemoved(uint64_t sessionID);
    int32_t TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason);
    void FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
        const AudioStreamDeviceChangeReasonExt reason);
    void FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo);
    int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    int32_t SetSessionDefaultOutputDevice(const int32_t callerPid, const DeviceType &deviceType);

    // Functions related to get operations - device related
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescInner(
        AudioRendererInfo &rendererInfo, std::string networkId);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescInner(
        AudioCapturerInfo &captureInfo, int32_t uid, std::string networkId);
    std::shared_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();
    std::vector<shared_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);
    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();
    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);
    int32_t GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
        &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission);
    int32_t GetCurrentCapturerChangeInfos(vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(AudioDeviceUsage audioDevUsage);
    int32_t GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo, const std::string &bundleName);
    int32_t GetSessionDefaultOutputDevice(const int32_t callerPid, DeviceType &deviceType);
    int32_t GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo);
    bool GetVolumeGroupInfos(std::vector<sptr<VolumeGroupInfo>> &infos);
    DirectPlaybackMode GetDirectPlaybackSupport(const AudioStreamInfo &streamInfo, const StreamUsage &streamUsage);
    void RestoreDistributedDeviceInfo();
    bool IsDistributeServiceOnline();

    // Called by Others - without lock
    int32_t SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object);
    int32_t UnsetAudioDeviceAnahsCallback();
    void OnUpdateAnahsSupport(std::string anahsShowType);
    void RegisterBluetoothListener();
    void UnregisterBluetoothListener();
    void ConfigDistributedRoutingRole(const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type);
    int32_t SetRingerMode(AudioRingerMode ringMode);
    int32_t FetchOutputDeviceAndRoute(std::string caller,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    int32_t FetchInputDeviceAndRoute(std::string caller,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    void SetAudioServerProxy();
    bool GetDisableFastStreamParam();
    bool IsFastAllowed(std::string &bundleName);
    void UpdateStreamPropInfo(const std::string &adapterName, const std::string &pipeName,
        const std::list<DeviceStreamInfo> &deviceStreamInfo, const std::list<std::string> &supportDevices);
    void ClearStreamPropInfo(const std::string &adapterName, const std::string &pipeName);
    uint32_t GetStreamPropInfoSize(const std::string &adapterName, const std::string &pipeName);
    int32_t CaptureConcurrentCheck(uint32_t sessionId);
    void SetFirstScreenOn();
    bool IsDupDeviceChange(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void FetchOutputDupDevice(std::string caller, uint32_t sessionId,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool IsA2dpOffloadStream(uint sessionId);
    int32_t SwitchActiveA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    int32_t SetRendererTarget(RenderTarget target, RenderTarget lastTarget, uint32_t sessionId);
    int32_t StartInjection(uint32_t sessionId);
    void RemoveIdForInjector(uint32_t sessionId);
    void ReleaseCaptureInjector();
    void RebuildCaptureInjector(uint32_t sessionId);
    int32_t A2dpOffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp);
    bool InVideoCommFastBlockList(const std::string &bundleName);
    int32_t SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object);

private:
    static std::string GetEncryptAddr(const std::string &addr);
    int32_t FetchRendererPipesAndExecute(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    int32_t FetchCapturerPipesAndExecute(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    int32_t ScoInputDeviceFetchedForRecongnition(
        bool handleFlag, const std::string &address, ConnectState connectState);
    void BluetoothScoFetch(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void CheckModemScene(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
         const AudioStreamDeviceChangeReasonExt reason);
    int32_t UpdateModemRoute(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs);
    uint32_t GetVoiceCallMuteDuration(AudioDeviceDescriptor &curDesc, AudioDeviceDescriptor &newDesc);
    void UnmuteVoiceCallAfterMuteDuration(uint32_t muteDuration, std::shared_ptr<AudioDeviceDescriptor> desc);
    void NotifyUnmuteVoiceCall();
    void SetUpdateModemRouteFinished(bool flag);
    void HandleAudioCaptureState(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void UpdateDefaultOutputDeviceWhenStopping(int32_t uid);
    void UpdateInputDeviceWhenStopping(int32_t uid);
    int32_t BluetoothDeviceFetchOutputHandle(shared_ptr<AudioStreamDescriptor> &desc,
        const AudioStreamDeviceChangeReasonExt reason, std::string encryptMacAddr);
    int32_t ActivateA2dpDeviceWhenDescEnabled(shared_ptr<AudioDeviceDescriptor> desc,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t ActivateA2dpDevice(std::shared_ptr<AudioDeviceDescriptor> desc,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t ActivateNearlinkDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    void HandleNearlinkErrResult(int32_t result, shared_ptr<AudioDeviceDescriptor> devDesc, bool isVoiceType);
    int32_t LoadA2dpModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo,
        std::string networkId, std::string sinkName, SourceType sourceType);
    int32_t ReloadA2dpAudioPort(AudioModuleInfo &moduleInfo, DeviceType deviceType,
        const AudioStreamInfo& audioStreamInfo, std::string networkId, std::string sinkName,
        SourceType sourceType);
    void ProcessOutputPipeReload(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag,
        const AudioStreamDeviceChangeReasonExt reason);
    AudioIOHandle ReloadOrOpenAudioPort(int32_t engineFlag, AudioModuleInfo &moduleInfo,
        uint32_t &paIndex);
    void GetA2dpModuleInfo(AudioModuleInfo &moduleInfo, const AudioStreamInfo& audioStreamInfo,
        SourceType sourceType);
    void RecordSelectDevice(const std::string &history);
    std::string ParsePreferredInputDeviceHistory(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsSameDevice(shared_ptr<AudioDeviceDescriptor> &desc, const AudioDeviceDescriptor &deviceInfo);
    int32_t SwitchActiveHearingAidDevice(std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor);
    int32_t LoadHearingAidModule(DeviceType deviceType, const AudioStreamInfo &audioStreamInfo,
        std::string networkId, std::string sinkName, SourceType sourceType);
    int32_t SetSleVoiceStatusFlag(AudioScene audioScene);
#ifdef BLUETOOTH_ENABLE
    void RegisterBluetoothDeathCallback();
    static void BluetoothServiceCrashedCallback(pid_t pid, pid_t uid);
#endif
    int32_t FetchDeviceAndRoute(std::string caller,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    int32_t FetchRendererPipeAndExecute(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        uint32_t &sessionId, uint32_t &audioFlag,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    void ProcessOutputPipeNew(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag,
        const AudioStreamDeviceChangeReasonExt reason);
    void ProcessOutputPipeUpdate(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t FetchCapturerPipeAndExecute(
        std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId);
    void ProcessInputPipeNew(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag);
    void ProcessInputPipeUpdate(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag);
    void RemoveUnusedPipe();
    void RemoveUnusedRecordPipe();
    void MoveStreamSink(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::shared_ptr<AudioPipeInfo> pipeInfo, const AudioStreamDeviceChangeReasonExt reason);
    void MoveToNewOutputDevice(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::shared_ptr<AudioPipeInfo> pipeInfo,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    int32_t MoveToRemoteOutputDevice(
        std::vector<SinkInput> sinkInputIds, std::shared_ptr<AudioPipeInfo> pipeInfo,
        std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor);
    void MoveStreamSource(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        const std::vector<SourceOutput>& sourceOutputs);
    void MoveToNewInputDevice(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        const std::vector<SourceOutput>& sourceOutputs);
    int32_t MoveToLocalInputDevice(std::vector<SourceOutput> sourceOutputs,
        std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor, uint32_t routeFlag = AUDIO_FLAG_NONE);
    int32_t MoveToRemoteInputDevice(
        std::vector<SourceOutput> sourceInputs, std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor);
    int32_t OpenRemoteAudioDevice(std::string networkId, DeviceRole deviceRole, DeviceType deviceType,
        std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor);
    bool GetRingerOrAlarmerDualDevices(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        std::vector<std::pair<InternalDeviceType, DeviceFlag>> &activeDevices);
    bool SelectRingerOrAlarmDevices(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void UpdateDualToneState(const bool &enable, const int32_t &sessionId, const std::string &dupSinkName = "Speaker");
    int32_t MoveToLocalOutputDevice(std::vector<SinkInput> sinkInputIds,
        std::shared_ptr<AudioPipeInfo> pipeInfo, std::shared_ptr<AudioDeviceDescriptor> localDeviceDescriptor);
    bool HasLowLatencyCapability(DeviceType deviceType, bool isRemote);
    void TriggerRecreateRendererStreamCallback(shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    void TriggerRecreateRendererStreamCallbackEntry(shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    void TriggerRecreateCapturerStreamCallback(shared_ptr<AudioStreamDescriptor> &streamDesc);
    CapturerState HandleStreamStatusToCapturerState(AudioStreamStatus status);
    uint32_t OpenNewAudioPortAndRoute(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &paIndex);
    static int32_t GetRealUid(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    static int32_t GetRealPid(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    static void UpdateRendererInfoWhenNoPermission(const shared_ptr<AudioRendererChangeInfo> &audioRendererChangeInfos,
        bool hasSystemPermission);
    static void UpdateCapturerInfoWhenNoPermission(const shared_ptr<AudioCapturerChangeInfo> &audioCapturerChangeInfos,
        bool hasSystemPermission);
    void SendA2dpConnectedWhileRunning(const RendererState &rendererState, const uint32_t &sessionId);
    void UpdateSessionConnectionState(const int32_t &sessionID, const int32_t &state);
    void UpdateTrackerDeviceChange(const vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    bool IsForcedNormal(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void UpdatePlaybackStreamFlag(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isCreateProcess);
    AudioFlag SetFlagForMmapStream(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    AudioFlag SetFlagForSpecialStream(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isCreateProcess);
    void UpdateRecordStreamInfo(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    std::vector<SourceOutput> FilterSourceOutputs(int32_t sessionId,
        const std::vector<SourceOutput>& sourceOutputs);
    std::vector<SourceOutput> GetSourceOutputs();
    void UpdateOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void UpdateRingerOrAlarmerDualDeviceOutputRouter(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void UpdateDupDeviceOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void OnPreferredOutputDeviceUpdated(const AudioDeviceDescriptor &deviceDescriptor,
        const AudioStreamDeviceChangeReason reason);
    void OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId,
        const AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::UNKNOWN);
    bool IsRingerOrAlarmerDualDevicesRange(const InternalDeviceType &deviceType);
    bool GetFastControlParam();
    void StoreDistributedRoutingRoleInfo(const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type);
    int32_t GetSystemVolumeLevel(AudioStreamType streamType);
    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) const;
    bool IsStreamSupportLowpower(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsStreamSupportDirect(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsStreamSupportMultiChannel(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsNewDevicePlaybackSupported(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsDeviceSwitching(const AudioStreamDeviceChangeReasonExt reason);

    void AddSessionId(const uint32_t sessionId);
    void DeleteSessionId(const uint32_t sessionId);

    bool IsPaRoute(uint32_t routeFlag);
    bool RecoverFetchedDescs(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    int32_t HandleScoOutputDeviceFetched(
        shared_ptr<AudioDeviceDescriptor> &desc, const AudioStreamDeviceChangeReasonExt reason);
    int32_t HandleScoOutputDeviceFetched(
        shared_ptr<AudioStreamDescriptor> &streamDesc, const AudioStreamDeviceChangeReasonExt reason);
    int32_t HandleFetchOutputWhenNoRunningStream(const AudioStreamDeviceChangeReasonExt reason);
    int32_t HandleFetchInputWhenNoRunningStream();
    bool UpdateOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid,
        const AudioStreamDeviceChangeReasonExt reason);
    bool UpdateInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    void WriteOutputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason);
    void WriteInputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason);
    int32_t HandleDeviceChangeForFetchOutputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    int32_t HandleDeviceChangeForFetchInputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool NeedRehandleA2DPDevice(std::shared_ptr<AudioDeviceDescriptor> &desc);
    void UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo, RendererState rendererState);
    void HandleCommonSourceOpened(std::shared_ptr<AudioPipeInfo> &pipeInfo);
    void DelayReleaseOffloadPipe(AudioIOHandle id, uint32_t paIndex, OffloadType type);
    int32_t ReleaseOffloadPipe(AudioIOHandle id, uint32_t paIndex, OffloadType type);
    void PrepareMoveAttrs(std::shared_ptr<AudioStreamDescriptor> &streamDesc, DeviceType &oldDeviceType,
        bool &isNeedTriggerCallback, std::string &oldSinkName, const AudioStreamDeviceChangeReasonExt reason);
    bool HandleMuteBeforeDeviceSwitch(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
        const AudioStreamDeviceChangeReasonExt reason);
    void MuteSinkPortForSwitchDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    void CheckAndSleepBeforeVoiceCallDeviceSet(const AudioStreamDeviceChangeReasonExt reason);
    void CheckAndSleepBeforeRingDualDeviceSet(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void SleepForSwitchDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    bool IsHeadsetToSpkOrEp(const std::shared_ptr<AudioDeviceDescriptor> &oldDesc,
        const std::shared_ptr<AudioDeviceDescriptor> &newDesc);
    bool IsSceneRequireMuteAndSleep();
    void SetVoiceCallMuteForSwitchDevice();
    void MuteSinkPort(const std::string &oldSinkName, const std::string &newSinkName,
        AudioStreamDeviceChangeReasonExt reason);
    void MutePrimaryOrOffloadSink(const std::string &sinkName, int64_t muteTime);
    void MuteSinkPortLogic(const std::string &oldSinkName, const std::string &newSinkName,
        AudioStreamDeviceChangeReasonExt reason);
    int32_t ActivateOutputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    int32_t ActivateInputDevice(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    void OnAudioSceneChange(const AudioScene& audioScene);
    bool HandleOutputStreamInRunning(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        AudioStreamDeviceChangeReasonExt reason);
    bool HandleInputStreamInRunning(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void HandleDualStartClient(std::vector<std::pair<DeviceType, DeviceFlag>> &activeDevices,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void SelectA2dpType(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isCreateProcess);
    void UpdateStreamDevicesForCreate(std::shared_ptr<AudioStreamDescriptor> &streamDesc, std::string caller);
    void UpdateStreamDevicesForStart(std::shared_ptr<AudioStreamDescriptor> &streamDesc, std::string caller);
    bool IsNoRunningStream(std::vector<std::shared_ptr<AudioStreamDescriptor>> outputStreamDescs);
    void UpdateActiveDeviceAndVolumeBeforeMoveSession(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    void CheckAndSetCurrentOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t sessionId);
    void CheckAndSetCurrentInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc);
    void ClearRingMuteWhenCallStart(bool pre, bool after, std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void CheckForRemoteDeviceState(std::shared_ptr<AudioDeviceDescriptor> desc);
    void UpdateRemoteOffloadModuleName(std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &moduleName);
    void UpdateOffloadState(std::shared_ptr<AudioPipeInfo> pipeInfo);
    void NotifyRouteUpdate(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    void ResetNearlinkDeviceState(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, bool isRunning = true);
    int32_t ForceRemoveSleStreamType(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void WriteScoStateFaultEvent(const std::shared_ptr<AudioDeviceDescriptor> &devDesc);
    void FetchOutputDevicesForDescs(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &outputDescs);

    // For offload
    void CheckAndUpdateOffloadEnableForStream(
        OffloadAction action, std::shared_ptr<AudioStreamDescriptor> &streamDesc);

    void WriteCapturerConcurrentEvent(const std::unique_ptr<ConcurrentCaptureDfxResult> &result);
    void LogCapturerConcurrentResult(const std::unique_ptr<ConcurrentCaptureDfxResult> &result);
    bool WriteCapturerConcurrentMsg(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        const std::unique_ptr<ConcurrentCaptureDfxResult> &result);
    // for collaboration
    void UpdateRouteForCollaboration(InternalDeviceType deviceType);
    void CheckAndUpdateHearingAidCall(const DeviceType deviceType);
    int32_t CheckModuleForHearingAid(uint32_t &paIndex);
    void CheckCloseHearingAidCall(const bool isModemCallRunning, const DeviceType type);
    void CheckOpenHearingAidCall(const bool isModemCallRunning, const DeviceType type);
    std::shared_ptr<AudioDeviceDescriptor> GetCaptureClientDevice(
        std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t sessionId);
    int32_t PlayBackToInjection(uint32_t sessionId);
    int32_t InjectionToPlayBack(uint32_t sessionId);

    // for remote
    void ResetOriginalFlagForRemote(std::shared_ptr<AudioStreamDescriptor> &streamDesc);

private:
    std::shared_ptr<EventEntry> eventEntry_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_ = nullptr;
    AudioActiveDevice& audioActiveDevice_;
    AudioSceneManager& audioSceneManager_;
    AudioVolumeManager& audioVolumeManager_;
    AudioCapturerSession& audioCapturerSession_;
    AudioDeviceManager &audioDeviceManager_;
    AudioConnectedDevice& audioConnectedDevice_;
    AudioDeviceStatus& audioDeviceStatus_;
    AudioEffectService& audioEffectService_;
    AudioMicrophoneDescriptor& audioMicrophoneDescriptor_;
    AudioRecoveryDevice& audioRecoveryDevice_;
    AudioRouterCenter& audioRouterCenter_;
    AudioStreamCollector& streamCollector_;
    AudioStateManager &audioStateManager_;
    AudioDeviceCommon& audioDeviceCommon_;
    AudioOffloadStream& audioOffloadStream_;
    AudioA2dpOffloadFlag& audioA2dpOffloadFlag_;
    IAudioPolicyInterface& audioPolicyManager_;
    AudioRouteMap& audioRouteMap_;
    AudioIOHandleMap& audioIOHandleMap_;
    AudioA2dpDevice& audioA2dpDevice_;
    AudioEcManager& audioEcManager_;
    AudioPolicyConfigManager& policyConfigMananger_;
    AudioAffinityManager &audioAffinityManager_;
    SleAudioDeviceManager &sleAudioDeviceManager_;
    AudioUsrSelectManager &audioUsrSelectManager_;
    std::shared_ptr<AudioPipeSelector> audioPipeSelector_;
    AudioSessionService &audioSessionService_;

    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;
    std::shared_ptr<DeviceStatusListener> deviceStatusListener_;
    std::shared_ptr<AudioPipeManager> pipeManager_ = nullptr;

    bool hearingAidCallFlag_ = false;
    std::shared_ptr<HPAE::IHpaeSoftLink> softLink_ = nullptr;

    // select device history
    std::mutex hisQueueMutex_;
    std::deque<std::string> selectDeviceHistory_;

    // dual tone for same sinks
    std::vector<std::pair<uint32_t, AudioStreamType>> streamsWhenRingDualOnPrimarySpeaker_;
    bool isRingDualToneOnPrimarySpeaker_ = false;

    // Save the relationship of uid and session id.
    std::map<uint32_t, uid_t> sessionIdMap_;
    std::mutex sessionIdMutex_;

    std::unordered_map<std::string, DeviceType> spatialDeviceMap_;
    static bool isBtListenerRegistered;
    static bool isBtCrashed;
    static constexpr int32_t MIN_SERVICE_COUNT = 2;
    std::bitset<MIN_SERVICE_COUNT> serviceFlag_;
    bool isCurrentRemoteRenderer_ = false;
    bool isOpenRemoteDevice = false;
    int32_t enableDualHalToneSessionId_ = -1;
    bool enableDualHalToneState_ = false;
    int32_t shouldUpdateDeviceDueToDualTone_ = false;
    bool isFastControlled_ = true;
    std::mutex serviceFlagMutex_;

    // offload delay release
    // isOffloadOpened_ check whether offload is need open
    std::atomic<bool> isOffloadOpened_[OFFLOAD_TYPE_NUM] = {};
    // isOffloadInRelease_ check whether delayRelease thread is running
    std::atomic<bool> isOffloadInRelease_[OFFLOAD_TYPE_NUM] = {};
    std::condition_variable offloadCloseCondition_[OFFLOAD_TYPE_NUM];
    std::mutex offloadCloseMutex_;

    // route update callback
    std::unordered_map<uint32_t, sptr<IStandardAudioPolicyManagerListener>> routeUpdateCallback_;
    std::mutex routeUpdateCallbackMutex_;

    std::mutex updateModemRouteMutex_;
    std::condition_variable updateModemRouteCV_;
    bool updateModemRouteFinished_ = false;
    bool needUnmuteVoiceCall_ = false;

    DistributedRoutingInfo distributedRoutingInfo_ = {
        .descriptor = nullptr,
        .type = CAST_TYPE_NULL
    };
    bool isFirstScreenOn_ = false;
    bool isCreateProcess_ = false;
    bool isActivateA2dpDeviceForLog_ = false;

    AudioInjectorPolicy &audioInjectorPolicy_;

    sptr<IStandardAudioPolicyManagerListener> queryBundleNameListCallback_ = nullptr;
};
}
}
#endif // AUDIO_CORE_SERVICE_H
