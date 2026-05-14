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
#ifndef AUDIO_CORE_SERVICE_H
#define AUDIO_CORE_SERVICE_H
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <unordered_set>

#include "audio_policy_server_handler.h"
#include "i_core_service_provider.h"
#include "idevice_status_observer.h"
#include "audio_stream_descriptor.h"
#include "audio_device_descriptor.h"
#include "audio_info.h"
#include "audio_debug_info.h"
#include "audio_ec_info.h"
#include "microphone_descriptor.h"
#include "audio_stream_change_info.h"
#include "audio_active_device.h"
#include "audio_scene_manager.h"
#include "audio_volume_manager.h"
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
#include "audio_core_config_manager.h"
#include "audio_core_service_utils.h"
#include "sle_audio_device_manager.h"
#include "audio_event_utils.h"
#include "audio_stream_id_allocator.h"
#include "i_hpae_soft_link.h"
#include "audio_injector_policy.h"
#include "client_type_manager.h"
#include "async_action_handler.h"
#include "audio_policy_dump.h"
#include "boot_animation_state_info.h"

namespace OHOS {
namespace AudioStandard {
enum OffloadType {
    LOCAL_OFFLOAD,
    REMOTE_OFFLOAD,
    OFFLOAD_TYPE_NUM,
};

const uint32_t PCM_8_BIT = 8;
const uint32_t SESSION_ID_INVALID = 0;
const int32_t AUDIO_EXT_UID = 1041;
const std::string PIPE_PRIMARY_INPUT = "primary_input";
const std::string PIPE_WAKEUP_INPUT = "wakeup_input";
const std::string PIPE_PRIMARY_INPUT_AI = "primary_input_AI";
static const char* PIPE_PRIMARY_OUTPUT = "primary_output";
static const char* PIPE_USB_ARM_OUTPUT = "usb_arm_output";
static const char* PIPE_USB_ARM_INPUT = "usb_arm_input";
static const char* PIPE_DP_OUTPUT = "dp_output";
static const char* PIPE_ACCESSORY_INPUT = "accessory_input";

static constexpr uint32_t CONCURRENT_CAPTURE_DFX_THRESHOLD = 2;
static constexpr uint32_t CONCURRENT_CAPTURE_DFX_MSG_ARRAY_MAX = 5;
static constexpr uint32_t CONCURRENT_CAPTURE_DFX_HDI_SEGMENTS = 2;
static const std::unordered_set<DeviceType> g_ultraFastDevicesSet = {
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_USB_ARM_HEADSET
};
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

struct RouteUpdateCallbackInfo {
    sptr<IStandardAudioPolicyManagerListener> listener;
    uid_t clientUid = 0;
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
            std::shared_ptr<AudioStreamDescriptor> &streamDesc, uint32_t &flag, uint32_t &sessionId,
            std::string &networkId);
        int32_t CreateCapturerClient(
            std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId);

        // ICoreServiceProvider
        int32_t UpdateSessionOperation(uint32_t sessionId, SessionOperation operation,
            SessionOperationMsg opMsg = SESSION_OP_MSG_DEFAULT, bool needRemoveFromMap = false) override;
        int32_t ReloadCaptureSession(uint32_t sessionId, SessionOperation operation) override;
        int32_t SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionId,
            const StreamUsage streamUsage, bool isRunning, bool skipForce = false) override;
        int32_t RecordBootStateTime(int32_t state, int32_t uid, int64_t curTime) override;
        int32_t SetOffloadAllowedForUid(uint32_t uid, bool allowed);
        std::string GetAdapterNameBySessionId(uint32_t sessionId) override;
        std::string GetModuleNameBySessionId(uint32_t sessionId) override;
        int32_t GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo,
            AudioStreamInfo &streamInfo, bool &isUltraFast, bool isReloadProcess = false) override;
        int32_t GetAudioDebugInfoBySessionId(uint32_t sessionId, AudioDebugInfo &audioDebugInfo);
        uint32_t GenerateSessionId() override;
        int32_t SetAudioRouteSelectorCallback(const AudioPipeSelector::AudioRouteSelectorCallback &callback);
        int32_t UnsetAudioRouteSelectorCallback();
        int32_t SetSplitModeReady();
        int32_t LoadSplitModule(const std::string &splitArgs, const std::string &networkId);
        void OnCheckActiveMusicTime(const std::string &reason) override;
        int32_t SetCapturerMuteHint(uint32_t sessionId, bool mute) override;
        void ReEvaluateVoipPrivacyMuteForCapturers(const std::string &reason);
        void UpdateVoipNoPrivacyFlagBySessionId(uint32_t sessionId, bool enabled, const std::string &reason);
        void UpdateVoipNoPrivacyFlagByPid(int32_t callerPid, bool enabled, const std::string &reason);

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
            sptr<AudioRendererFilter> filter = nullptr, const std::string &caller = "") override;
        void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
            const std::string &caller = "") override;
        void OnConnectFailed(AudioDeviceDescriptor &desc) override;
        uint32_t GetPaIndexByPortName(const std::string &portName) override;

        // Functions related to assignment operations - device related
        int32_t SetAudioScene(AudioScene audioScene, const int32_t uid = INVALID_UID, const int32_t pid = INVALID_PID);
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);
        int32_t SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid = INVALID_UID);
        int32_t SetInputDevice(const DeviceType deviceType, const uint32_t sessionID, int32_t uid);
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
        int32_t SetMediaOutputDeviceByUid(DeviceType deviceType, const int32_t uid);
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
        int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
        int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
            std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
        int32_t SetSessionDefaultOutputDevice(const int32_t callerPid, const DeviceType &deviceType,
            int32_t callerUid = INVALID_UID);

        // Functions related to get operations - device related
        bool IsArmUsbDevice(const AudioDeviceDescriptor &deviceDesc);
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescriptors(
            AudioRendererInfo &rendererInfo, int32_t uid, std::string networkId = LOCAL_NETWORK_ID,
            const uint32_t streamId = INVALID_STREAM_ID);
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescriptors(
            AudioCapturerInfo &captureInfo, int32_t uid, std::string networkId = LOCAL_NETWORK_ID,
            const uint32_t streamId = INVALID_STREAM_ID);
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
        int32_t GetSessionDefaultOutputDevice(const int32_t callerPid, DeviceType &deviceType);
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
        void NotifySessionStandbyStatus(uint32_t sessionId, bool standbyStatus) override;
        void HandleDeviceConfigChanged(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice);
        void NotifyRemoteRouteStateChange(const std::string &networkId, DeviceType deviceType, bool enable);
        void ReloadSourceForEffect(const AudioEffectPropertyArray oldPropertyArray,
            const AudioEffectPropertyArray newPropertyArray);
        void OnVehiclePriorityChanged(bool enable);
        // Trigger reload for VoIP priority change due to mute callback
        void TriggerVoipPriorityReload(uint32_t sessionId, bool lowered);

    private:
        int32_t HandleSetAudioRouteSelectorCallback(const AudioPipeSelector::AudioRouteSelectorCallback &callback);
        int32_t HandleUnsetAudioRouteSelectorCallback();
        int32_t WaitForServiceReady();
        void NotifyServiceReady();

        std::shared_ptr<AudioCoreService> coreService_;
        std::shared_mutex eventMutex_;
        std::mutex serviceReadyMutex_;
        std::condition_variable serviceReadyCV_;
        std::atomic<bool> serviceReady_ = false;
        int32_t serviceReadyWaitCount_ = 0;
        AudioPolicyDump &audioPolicyDump_;
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
    void SetAsyncActionHandler(std::shared_ptr<AsyncActionHandler> &handler);
    std::shared_ptr<AsyncActionHandler> GetAsyncActionHandler() const;
    void SetConfigParserFlag();
    // Pipe capability pass-through interfaces. Synchronization is managed in pipe/domain modules.
    void SetEcEnableState(bool ecEnableState);
    AudioSampleFormat GetFastFormat() const;
    bool IsSupportInnerCaptureOffload();
    bool GetEnhancedRoutingSupported();
    int32_t GetMaxRendererInstances();
    void GetDeviceClassInfo(std::unordered_map<ClassType, std::list<AudioModuleInfo>> &deviceClassInfo);

    void SetEcAndMicRefEnableState(int32_t ecEnableState, int32_t micRefEnableState);

    void CloseNormalSource();
    AudioEcInfo GetAudioEcInfo();
    void ResetAudioEcInfo();

    void PresetArmIdleInput(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc);
    void CloseUsbArmDevice(const AudioDeviceDescriptor &device);
    void GetTargetSourceTypeAndMatchingFlag(SourceType source, SourceType &targetSource, bool &useMatchingPropInfo);

    int32_t FetchTargetInfoForSessionAdd(const SessionInfo sessionInfo, PipeStreamPropInfo &targetInfo,
        SourceType &targetSourceType);

    void ReloadSourceForSession(SessionInfo sessionInfo, uint32_t targetSessionId);
    int32_t ReloadSourceSoftLink(std::shared_ptr<AudioPipeInfo> &pipeInfo, const AudioModuleInfo &moduleInfo);
    int32_t ReloadSourceForInputPipe(std::shared_ptr<AudioPipeInfo> &pipeInfo, uint32_t targetSessionId);

    void SetDpSinkModuleInfo(const AudioModuleInfo &moduleInfo);
    void SetPrimaryMicModuleInfo(const AudioModuleInfo &moduleInfo);
    SourceType GetSourceOpened();
    bool GetEcFeatureEnable();
    bool GetMicRefFeatureEnable();
    void UpdateStreamEcAndMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType);
    void SetOpenedNormalSource(SourceType sourceType);
    int32_t ReloadNormalSource(SessionInfo &sessionInfo, PipeStreamPropInfo &targetInfo, SourceType targetSource,
        const int32_t uid = -1);
    void UpdateStreamEcInfo(AudioModuleInfo &moduleInfo, SourceType sourceType);
    void UpdateStreamMicRefInfo(AudioModuleInfo &moduleInfo, SourceType sourceType);
    void PrepareNormalSource(std::shared_ptr<AudioPipeInfo> &pipeInfo,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void SetOpenedNormalSourceSessionId(uint64_t sessionId);
    uint64_t GetOpenedNormalSourceSessionId();
    void SetUsbSinkModuleInfo(AudioModuleInfo &moduleInfo);
    void SetUsbSourceModuleInfo(AudioModuleInfo &moduleInfo);

    bool IsStreamBelongToUid(const uid_t uid, const uint32_t sessionId);
    bool IsValidRenderSessionId(const uint32_t sessionId);
    void DumpPipeManager(std::string &dumpString);
    void DumpSelectHistory(std::string &dumpString);
    void SetAudioRouteCallback(uint32_t sessionId, const sptr<IRemoteObject> &object);
    void UnsetAudioRouteCallback(uint32_t sessionId);

    // Called by EventEntry - with lock
    // Stream operations
    int32_t CreateRendererClient(
        std::shared_ptr<AudioStreamDescriptor> &streamDesc, uint32_t &audioFlag, uint32_t &sessionId,
        std::string &networkId);
    void SetPreferredInputDeviceIfValid(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    int32_t CreateCapturerClient(
        std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &audioFlag, uint32_t &sessionId);
    int32_t StartClient(uint32_t sessionId);
    int32_t StartClientForPlayback(uint32_t sessionId,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        std::shared_ptr<AudioDeviceDescriptor> &deviceDesc);
    int32_t StartClientForRecord(uint32_t sessionId,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        std::shared_ptr<AudioDeviceDescriptor> &deviceDesc);
    int32_t PauseClient(uint32_t sessionId);
    int32_t StandbyClient(uint32_t sessionId);
    void NotifyStandbyStatus(uint32_t sessionId, bool standbyStatus);
    int32_t StopClient(uint32_t sessionId);
    int32_t ReleaseClient(uint32_t sessionId, SessionOperationMsg opMsg = SESSION_OP_MSG_DEFAULT,
        bool needRemoveFromMap = false);
    void ForceStopInterphoneRenderer();

    // ICoreServiceProvider from EventEntry
    std::string GetAdapterNameBySessionId(uint32_t sessionId);
    std::string GetModuleNameBySessionId(uint32_t sessionId);
    int32_t GetProcessDeviceInfoBySessionId(uint32_t sessionId, AudioDeviceDescriptor &deviceInfo,
        AudioStreamInfo &streamInfo, bool &isUltraFast);
    int32_t GetAudioDebugInfoBySessionId(uint32_t sessionId, AudioDebugInfo &audioDebugInfo);
    uint32_t GenerateSessionId();
    int32_t SetAudioRouteSelectorCallback(const AudioPipeSelector::AudioRouteSelectorCallback &callback);
    int32_t UnsetAudioRouteSelectorCallback();
    int32_t SetSplitModeReady();
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
        sptr<AudioRendererFilter> filter = nullptr, const std::string &caller = "");
    void OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
        const std::string &caller = "");
    void OnConnectFailed(AudioDeviceDescriptor &desc);

    // Functions related to assignment operations - device related
    int32_t SetAudioScene(AudioScene audioScene, const int32_t uid = INVALID_UID, const int32_t pid = INVALID_PID);
    bool IsArmUsbDevice(const AudioDeviceDescriptor &deviceDesc);
    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid = INVALID_UID);
    int32_t SetInputDevice(const DeviceType deviceType, const uint32_t sessionID, int32_t uid);
    int32_t SetCallDeviceActive(InternalDeviceType deviceType, bool active, std::string address,
        const int32_t uid = INVALID_UID);
    int32_t RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
        const sptr<IRemoteObject> &object, const int32_t apiVersion);
    int32_t UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo);
    void RegisteredTrackerClientDied(pid_t uid, pid_t pid);
    bool ConnectServiceAdapter();
    void OnReceiveUpdateDeviceNameEvent(const std::string macAddress, const std::string deviceName);
    int32_t SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode = 0,
        const bool isNeedNotifyBt = true);
    void NotifyDistributedOutputChange(const AudioDeviceDescriptor &deviceDesc);
    int32_t SetMediaOutputDeviceByUid(DeviceType deviceType, const int32_t uid);
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
    int32_t ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    int32_t UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors);
    bool HandleRingToNonRingSceneChange(AudioScene lastAudioScene, AudioScene audioScene);
    bool IsCallOrRingToDefault(AudioScene lastAudioScene, AudioScene audioScene);
    int32_t SetSessionDefaultOutputDevice(
        const int32_t callerPid, const DeviceType &deviceType, bool skipForce = false,
        int32_t callerUid = INVALID_UID);
    int32_t FetchAndActivateOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc);

    // Functions related to get operations - device related
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetDevices(DeviceFlag deviceFlag);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredOutputDeviceDescInner(
        AudioRendererInfo &rendererInfo, std::string networkId = LOCAL_NETWORK_ID, const int32_t uid = INVALID_UID,
        const uint32_t streamId = INVALID_STREAM_ID);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetPreferredInputDeviceDescInner(
        AudioCapturerInfo &captureInfo, std::string networkId = LOCAL_NETWORK_ID, const int32_t uid = INVALID_UID,
        const uint32_t streamId = INVALID_STREAM_ID);
    std::shared_ptr<AudioDeviceDescriptor> GetActiveBluetoothDevice();
    std::vector<shared_ptr<AudioDeviceDescriptor>> GetAvailableDevices(AudioDeviceUsage usage);
    std::vector<sptr<MicrophoneDescriptor>> GetAvailableMicrophones();
    std::vector<sptr<MicrophoneDescriptor>> GetAudioCapturerMicrophoneDescriptors(int32_t sessionId);
    int32_t GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
        &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission);
    int32_t GetCurrentCapturerChangeInfos(vector<shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos,
        bool hasBTPermission, bool hasSystemPermission);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> GetExcludedDevices(AudioDeviceUsage audioDevUsage);
    int32_t GetSessionDefaultOutputDevice(const int32_t callerPid, DeviceType &deviceType);
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
    bool HandleA2dpSuspendWhenLoad();
    void HandleA2dpRestore();
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
    int32_t SetCapturerMuteHint(uint32_t sessionId, bool mute);
    void ReEvaluateVoipBypassByAppMuteHint();
    void ReEvaluateVoipPrivacyMuteForCapturers(const std::string &reason);
    void SetFirstScreenOn();
    bool IsDupDeviceChange(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void FetchOutputDupDevice(std::string caller, uint32_t sessionId,
        std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool IsA2dpOffloadStream(uint sessionId);
    int32_t SetRendererTarget(RenderTarget target, RenderTarget lastTarget, uint32_t sessionId);
    int32_t StartInjection(uint32_t sessionId);
    void RemoveIdForInjector(uint32_t sessionId);
    void ReleaseCaptureInjector();
    void RebuildCaptureInjector(uint32_t sessionId);
    int32_t A2dpOffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp);
    bool InVideoCommFastBlockList(const std::string &bundleName);
    int32_t SetQueryBundleNameListCallback(const sptr<IRemoteObject> &object);
    void HandleDeviceConfigChanged(const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice);
    void DeactivateRemoteDevice(const std::string &networkId, DeviceType deviceType);
    void NotifyRemoteRouteStateChange(const std::string &networkId, DeviceType deviceType, bool enable);
    void NotifyRemoteDeviceStatusUpdate(std::shared_ptr<AudioDeviceDescriptor> desc);
    void NotifyVoipPriorityLoweredByMute(uint32_t streamId, bool lowered);
    bool UpdateVoipNoPrivacyFlagBySessionId(uint32_t sessionId, bool enabled);
    bool UpdateVoipNoPrivacyFlagByPid(int32_t callerPid, bool enabled);
    int32_t ReloadCaptureSession(uint32_t sessionId, SessionOperation operation);
    void TriggerVoipPriorityReload(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool lowered);
    int32_t ReloadCaptureSessionSoftLink();
    bool ReloadCheckForStart(bool findRunningSessionRet, std::shared_ptr<AudioStreamDescriptor> newStream,
        AudioStreamDescriptor &runningStream);
    void ReloadSourceForEffect(const AudioEffectPropertyArray &oldPropertyArray,
        const AudioEffectPropertyArray &newPropertyArray);
    void ReloadSourceForDeviceChange(const AudioDeviceDescriptor &inputDevice,
        const AudioDeviceDescriptor &outputDevice, const std::string &caller);
    int32_t GetActiveStreamsVolumeInfo(std::vector<ActiveStreamVolumeInfo> &activeStreamsVolumeInfo);
    void RecordBootStateTime(AudioPolicyServerBootState state, int64_t curTime);
    void RecordBootStateTime(BootAnimationState state, int32_t uid, int64_t curTime);

    void OnPreferredOutputDeviceUpdated(const AudioDeviceDescriptor &deviceDescriptor,
        const AudioStreamDeviceChangeReason reason);
    void OnPreferredInputDeviceUpdated(DeviceType deviceType, std::string networkId,
        const AudioStreamDeviceChangeReason reason = AudioStreamDeviceChangeReason::UNKNOWN);

private:
    void UpdateEnhanceEffectState(SourceType source);
    void UpdatePrimaryMicModuleInfo(std::shared_ptr<AudioPipeInfo> &pipeInfo, SourceType sourceType);
    void UpdateStreamCommonInfo(AudioModuleInfo &moduleInfo, PipeStreamPropInfo &targetInfo, SourceType sourceType);
    void UpdateModuleInfoDeviceType(AudioModuleInfo &moduleInfo,
        const std::shared_ptr<AudioDeviceDescriptor> &inputDesc);
    void UpdateAudioEcInfo(const AudioDeviceDescriptor &inputDevice, const AudioDeviceDescriptor &outputDevice);
    void UpdateModuleInfoForEc(AudioModuleInfo &moduleInfo);
    void UpdateModuleInfoForMicRef(AudioModuleInfo &moduleInfo, SourceType sourceType);
    std::shared_ptr<PipeStreamPropInfo> GetMicStreamPropInfoForMicRef();
    void ClearModuleInfoForEc(AudioModuleInfo &moduleInfo);
    void ClearModuleInfoForMicRef(AudioModuleInfo &moduleInfo);
    std::string ShouldOpenMicRef(SourceType source);

    EcType GetEcType(const DeviceType inputDevice, const DeviceType outputDevice);
    std::string GetEcSamplingRate(const std::string &halName, std::shared_ptr<PipeStreamPropInfo> &outModuleInfo);
    std::string GetEcFormat(const std::string &halName, std::shared_ptr<PipeStreamPropInfo> &outModuleInfo);
    std::string GetEcChannels(const std::string &halName, std::shared_ptr<PipeStreamPropInfo> &outModuleInfo);

    int32_t GetPipeInfoByDeviceTypeForEc(const std::string &role, const DeviceType deviceType,
        std::shared_ptr<AdapterPipeInfo> &pipeInfo);

    void UpdateArmModuleInfo(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc, AudioModuleInfo &moduleInfo);
    std::string GetHalNameForDevice(const std::string &role, const DeviceType deviceType);
    std::string GetPipeNameByDeviceForEc(const std::string &role, const DeviceType deviceType);

    bool IsHigherPrioritySourceType(SourceType newSource, SourceType currentSource);
    static std::string GetEncryptAddr(const std::string &addr);
    int32_t GetPipeBaseDebugInfo(const std::shared_ptr<AudioPipeInfo> &pipe, AudioDebugInfo &debugInfo);
    int32_t GetStreamDebugInfo(uint32_t sessionId, const std::shared_ptr<AudioPipeInfo> &pipe,
        AudioDebugInfo &debugInfo);
    int32_t GetVolumeDebugInfo(AudioDebugInfo &debugInfo);
    void DecidedAudioFlagForStreams(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    int32_t FetchRendererPipesAndExecute(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    int32_t FetchCapturerPipesAndExecute(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    int32_t ScoInputDeviceFetchedForRecongnition(
        bool handleFlag, const std::string &address, ConnectState connectState, bool isVrSupported = true);
    void CheckModemScene(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
         const AudioStreamDeviceChangeReasonExt reason);
    void CheckRingAndVoipScene(const AudioStreamDeviceChangeReasonExt reason);
    bool CheckRingAndVoipStreamRunning();
    int32_t UpdateModemRoute(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs);
    uint32_t GetVoiceCallMuteDuration(AudioDeviceDescriptor &curDesc, AudioDeviceDescriptor &newDesc);
    void UnmuteVoiceCallAfterMuteDuration(uint32_t muteDuration, std::shared_ptr<AudioDeviceDescriptor> desc);
    void NotifyUnmuteVoiceCall();
    void SetUpdateModemRouteFinished(bool flag);
    void UpdateDefaultOutputDeviceWhenStopping(int32_t uid);
    void ProcessOutputPipeReload(std::shared_ptr<AudioPipeInfo> pipeInfo, uint32_t &flag,
        const AudioStreamDeviceChangeReasonExt reason);
    void RecordSelectDevice(const std::string &history);
    std::string ParsePreferredInputDeviceHistory(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsSameDevice(shared_ptr<AudioDeviceDescriptor> &desc, const AudioDeviceDescriptor &deviceInfo);
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
    void TrackScoStreamAudioScene(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isAdd);
    AudioScene CalculatePipeAudioScene(uint32_t pipeId);
    bool RecordIsForcedNormal(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool IsForcedNormal(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    bool IsHWDecoding(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void UpdatePlaybackStreamFlag(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isCreateProcess);
    AudioFlag GetFlagForMmapStream(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    AudioFlag SetFlagForSpecialStream(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isCreateProcess);
    bool CheckStaticModeAndSelectFlag(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void UpdateRecordStreamInfo(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    std::vector<SourceOutput> FilterSourceOutputs(int32_t sessionId,
        const std::vector<SourceOutput>& sourceOutputs);
    std::vector<SourceOutput> GetSourceOutputs();
    void UpdateOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc, const uint32_t pipeId);
    void UpdateRingerOrAlarmerDualDeviceOutputRouter(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void UpdateDupDeviceOutputRoute(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsRingerOrAlarmerDualDevicesRange(const InternalDeviceType &deviceType);
    void StoreDistributedRoutingRoleInfo(const std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type);
    int32_t GetSystemVolumeLevel(AudioStreamType streamType);
    float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType) const;
    bool CheckOffloadPipeUidAllowed(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, int32_t realUid) const;
    bool IsStreamSupportLowpower(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsOffloadAllowedForUid(uint32_t uid) const;
    bool IsStreamSupportDirect(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsStreamSupportMultiChannel(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsNewDevicePlaybackSupported(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsStreamSupportOutputInterPhone(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    bool IsDeviceSwitching(const AudioStreamDeviceChangeReasonExt reason);

    void AddSessionId(const uint32_t sessionId);
    void DeleteSessionId(const uint32_t sessionId);

    int32_t HandleFetchOutputWhenNoRunningStream(const AudioStreamDeviceChangeReasonExt reason);
    int32_t HandleFetchInputWhenNoRunningStream();
    bool UpdateOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid,
        const AudioStreamDeviceChangeReasonExt reason);
    bool UpdateInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t uid,
        const AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN);
    void WriteOutputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason);
    void WriteInputRouteChangeEvent(std::shared_ptr<AudioDeviceDescriptor> &desc,
        const AudioStreamDeviceChangeReason reason, const int32_t uid);
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
    void HandlePrimaryMediaMuteForDualRing(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
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
    bool HandleA2dpSuspendWhenFetch(const AudioStreamDeviceChangeReasonExt &reason,
        const AudioDeviceDescriptor &actived, const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streams);
    void HandleA2dpSuspend();
    void UpdateActiveDeviceAndVolumeBeforeMoveSession(std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    void CheckAndSetCurrentOutputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, int32_t sessionId);
    void CheckAndSetCurrentInputDevice(std::shared_ptr<AudioDeviceDescriptor> &desc, const int32_t uid);
    void ClearRingMuteWhenCallStart(bool pre, bool after, std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void CheckForRemoteDeviceState(std::shared_ptr<AudioDeviceDescriptor> desc);
    void UpdateRemoteOffloadModuleName(std::shared_ptr<AudioPipeInfo> pipeInfo, std::string &moduleName);
    void UpdateOffloadState(std::shared_ptr<AudioPipeInfo> pipeInfo);
    void NotifyRouteUpdate(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs);
    int32_t ForceRemoveSleStreamType(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void WriteScoStateFaultEvent(const std::shared_ptr<AudioDeviceDescriptor> &devDesc);
    void FetchOutputDevicesForDescs(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &outputDescs);
    void DeactivateBluetoothDevice(bool isRunning);
    bool DeactivateA2dpAfterExclude(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &excludeDeviceDescs,
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &outputStreamDescs);

    // For offload
    void CheckAndUpdateOffloadEnableForStream(
        OffloadAction action, std::shared_ptr<AudioStreamDescriptor> &streamDesc, uint32_t ioHandle);
    int32_t SetOffloadAllowedForUid(uint32_t uid, bool allowed);

    void WriteCapturerConcurrentEvent(const std::unique_ptr<ConcurrentCaptureDfxResult> &result);
    void LogCapturerConcurrentResult(const std::unique_ptr<ConcurrentCaptureDfxResult> &result);
    bool WriteCapturerConcurrentMsg(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        const std::unique_ptr<ConcurrentCaptureDfxResult> &result);
    // for collaboration
    void UpdateRouteForCollaboration(InternalDeviceType deviceType, uint32_t sessionId);
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
    AudioStreamDeviceChangeReasonExt UpdateRemoteDeviceChangeReason(std::shared_ptr<AudioStreamDescriptor> streamDesc,
        const AudioStreamDeviceChangeReasonExt reason);
    void OnRemoteDeviceStatusUpdatedWhenNoRunningStream(std::shared_ptr<AudioDeviceDescriptor> newDesc);
    void OnRemoteDeviceStatusUpdated();
    void StopNoRunningPipe();

    bool IsDescInSourceStrategyMap(std::shared_ptr<AudioStreamDescriptor> desc);
    void GetFlagForUltraFastStream(std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool isCreateProcess);

    void HandlePlaybackStoppingOperations(std::shared_ptr<AudioStreamDescriptor> &streamDesc, std::string caller);
    void CheckScoState(std::shared_ptr<AudioStreamDescriptor> &streamDesc);
    void ForceStopInterphoneStreamsIfNecessary(const std::shared_ptr<AudioStreamDescriptor>& streamDesc);
    int32_t CheckStreamConflicts(const std::shared_ptr<AudioStreamDescriptor>& streamDesc, uint32_t& audioFlag);
    int32_t CheckCapturerStreamConflicts(const std::shared_ptr<AudioStreamDescriptor>& streamDesc);
    void LoadInnerCapturerSink(std::string moduleName, AudioStreamInfo streamInfo);
    void UnloadInnerCapturerSink(std::string moduleName);
    void HandleRemoteCastDevice(bool isConnected, AudioStreamInfo streamInfo = {});
    bool GetTargetSessionIdForInputPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
        uint32_t originSessionId, uint32_t &targetSessionId, SessionOperation operation);
    uint32_t GetMaxPriorityForInputPipe(const std::shared_ptr<AudioPipeInfo> &pipeInfo,
        uint32_t sessionId, AudioStreamDescriptor &maxPriorityDesc, bool onlyRunning);
    bool IsVirtualAudioRecognitionSession(uint32_t sessionId);
    int32_t ReloadCapturerSessionForInputPipe(uint32_t sessionId, SessionOperation operation);
    bool FindRemainingNormalSession(uint32_t sessionId, bool findRunningSessionRet,
        uint32_t runningSessionId, uint32_t &targetSessionId);
    bool IsPipeInSourceStrategyMap(std::shared_ptr<AudioPipeInfo> pipeInfo, uint64_t sessionID);
    bool IsInputPipeRouteFlagSession(uint32_t sessionId);
    bool ConstructWakeupAudioModuleInfo(const AudioStreamInfo &streamInfo,
        AudioModuleInfo &audioModuleInfo);
    bool FillWakeupStreamPropInfo(const AudioStreamInfo &streamInfo, std::shared_ptr<AdapterPipeInfo> pipeInfo,
        AudioModuleInfo &audioModuleInfo);
    int32_t SetWakeUpAudioCapturer(InternalAudioCapturerOptions options);
    bool HandleIndependentInputpipe(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList,
        uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession);
    bool CompareIndependentxmlPriority(const std::shared_ptr<AudioPipeInfo> &pipe,
        uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession);
    bool HandleNormalInputPipes(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList,
        uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo, bool &hasSession);
    bool FindRunningNormalSession(uint32_t sessionId, AudioStreamDescriptor &runningSessionInfo);
    std::pair<SourceType, uint32_t> GetTargetSessionForEc();
    bool HasNormalTypeCapturerSession(const std::vector<std::shared_ptr<AudioPipeInfo>> &pipeList);
    bool IsSourceTypeValidForEc(SourceType sourceType);
    bool IsVoipDeviceChanged(const AudioDeviceDescriptor &inputDevcie, const AudioDeviceDescriptor &outputDevice);
    bool IsEnhancedRoutingAvailableForSelectMode(int32_t audioDeviceSelectMode);
    int32_t ReloadCaptureSoftLink(std::shared_ptr<AudioPipeInfo> &pipeInfo, const AudioModuleInfo &moduleInfo);
    std::string GetEnhancePropByName(const AudioEffectPropertyArray &oldPropertyArray, const std::string &propName);
    bool IsVoipPrivacyMuteCandidateSource(SourceType sourceType) const;
    bool IsVoipPrivacyBypassedByBehavior(const std::shared_ptr<AudioStreamDescriptor> &streamDesc) const;
    bool IsVoipPrivacyMuteBypassedForUltrasonic(SourceType sourceType) const;
    bool ShouldMuteCapturerByVoipPrivacy(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
        bool hasActivePrivacyVoipCapturer) const;
    void EvaluateActivePrivacyVoipPresence(
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &allCapturerStreamDescs,
        bool &hasActivePrivacyVoipCapturer) const;
    void CollectSessionsToMuteByVoipPrivacy(
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &allCapturerStreamDescs,
        bool hasActivePrivacyVoipCapturer,
        std::unordered_set<uint32_t> &targetMuteSessions) const;
    void SyncVoipPrivacyMuteState(
        const std::vector<std::shared_ptr<AudioStreamDescriptor>> &allCapturerStreamDescs,
        const std::unordered_set<uint32_t> &targetMuteSessions);
    bool UpdateVoipPrivacyMuteForSession(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool mute);
    void WriteVoipMutedCaptureStats(const std::shared_ptr<AudioStreamDescriptor> &streamDesc, bool mute) const;

    void CheckAndUpdateEffect(std::shared_ptr<AudioStreamDescriptor> streamDesc);
    void OnVehiclePriorityChanged(bool enable);

private:
    std::shared_ptr<EventEntry> eventEntry_;
    std::shared_ptr<AudioPolicyServerHandler> audioPolicyServerHandler_ = nullptr;
    AudioActiveDevice& audioActiveDevice_;
    AudioSceneManager& audioSceneManager_;
    AudioVolumeManager& audioVolumeManager_;
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
    SleAudioDeviceManager &sleAudioDeviceManager_;
    std::shared_ptr<AudioPipeSelector> audioPipeSelector_;
    AudioSessionService &audioSessionService_;
    AudioRouterSelectStrategy& audioRouterSelectStrategy_;

    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager_ = nullptr;
    std::shared_ptr<DeviceStatusListener> deviceStatusListener_;
    std::shared_ptr<AudioPipeManager> pipeManager_ = nullptr;
    std::shared_ptr<AsyncActionHandler> asyncHandler_ = nullptr;

    bool isEcFeatureEnable_ = false;
    bool isMicRefFeatureEnable_ = false;

    bool hearingAidCallFlag_ = false;
    bool hearingAidReloadFlag_ = false;
    std::shared_ptr<HPAE::IHpaeSoftLink> softLink_ = nullptr;

    bool isPolicyConfigParsered_ = false;
    AudioDeviceDescriptor inputDeviceForReload_;

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
    std::mutex serviceFlagMutex_;

    std::atomic<bool> a2dpNeedSuspend_ = { false };
    std::chrono::steady_clock::time_point a2dpSuspendUntil_;
    std::mutex a2dpSuspendMutex_;
    std::unordered_set<uint32_t> capturerSessionIdisRemovedSet_;

    // offload delay release
    // isOffloadOpened_ check whether offload is need open
    std::atomic<bool> isOffloadOpened_[OFFLOAD_TYPE_NUM] = {};
    // isOffloadInRelease_ check whether delayRelease thread is running
    std::atomic<bool> isOffloadInRelease_[OFFLOAD_TYPE_NUM] = {};
    std::condition_variable offloadCloseCondition_[OFFLOAD_TYPE_NUM];
    std::mutex offloadCloseMutex_;
    std::unordered_set<uint32_t> offloadAllowedUids_;

    // route update callback
    std::unordered_map<uint32_t, RouteUpdateCallbackInfo> routeUpdateCallback_;
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
    std::unordered_map<uint32_t, uint32_t> sessionWithInputPipeRouteFlag_;
    std::unordered_map<uint32_t, SourceType> sessionSourceTypeMap_;

    AudioModuleInfo usbSinkModuleInfo_ = {};
    AudioModuleInfo usbSourceModuleInfo_ = {};
    AudioModuleInfo primaryMicModuleInfo_ = {};
    AudioModuleInfo dpSinkModuleInfo_ = {};
    SourceType normalSourceOpened_ = SOURCE_TYPE_INVALID;
    uint64_t sessionIdUsedToOpenSource_ = 0;

    std::mutex audioEcInfoMutex_;
    AudioEcInfo audioEcInfo_;
    std::string activeArmInputAddr_;
    std::string activeArmOutputAddr_;

    bool isMicRefVoipUpOn_ = false;
    bool isMicRefRecordOn_ = false;

    AudioInjectorPolicy &audioInjectorPolicy_;
    AudioPolicyDump &audioPolicyDump_;

    sptr<IStandardAudioPolicyManagerListener> queryBundleNameListCallback_ = nullptr;
    int32_t isSupportUltraFast_ = 0;
};

class RestoreA2dpSinkAction : public AsyncActionHandler::AsyncAction {
public:
    void Exec() override
    {
        AudioCoreService::GetCoreService()->HandleA2dpRestore();
    }
};
}
}
#endif // AUDIO_CORE_SERVICE_H
