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
#ifndef HPAE_MANAGER_H
#define HPAE_MANAGER_H
#include <functional>
#include <any>
#include <future>
#include <optional>
#include "audio_module_info.h"
#include "hpae_capturer_manager.h"
#include "hpae_virtual_capturer_manager.h"
#include "hpae_virtual_renderer_manager.h"
#include "hpae_renderer_manager.h"
#include "hpae_inner_capturer_manager.h"
#include "hpae_msg_channel.h"
#include "i_hpae_manager.h"
#include "i_hpae_renderer_manager.h"
#include "hpae_policy_manager.h"
#include "high_resolution_timer.h"
#include "session_effect_manager.h"
#include "hpae_delayed_task_queue.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeManager;

struct PendingStateTransition {
    uint32_t sessionId = 0;
    HpaeSessionState state = HPAE_SESSION_INVALID;
    IOperation operation = OPERATION_INVALID;
    TimePoint time;
};

class HpaeManagerThread {
public:
    HpaeManagerThread() : running_(false)
    {}
    ~HpaeManagerThread();
    void ActivateThread(HpaeManager *hpaeManager);
    void DeactivateThread();
    void Run();
    void Notify();
    bool IsRunning() const
    {
        return running_.load();
    }
    bool IsMsgProcessing() const
    {
        return recvSignal_.load();
    }

private:
    std::atomic<bool> running_;
    std::atomic<bool> recvSignal_;
    HpaeManager *m_hpaeManager = nullptr;
    std::condition_variable condition_;
    std::mutex mutex_;
    std::thread thread_;
};

class HpaeManager : public IHpaeManager, public ISendMsgCallback, public std::enable_shared_from_this<HpaeManager> {
public:
    static constexpr std::string_view SPLIT_STREAM_SINK = "libmodule-split-stream-sink.z.so";
    static constexpr std::string_view HDI_SINK = "libmodule-hdi-sink.z.so";
    static constexpr std::string_view HDI_SOURCE = "libmodule-hdi-source.z.so";
    static constexpr std::string_view INNER_CAPTURER_SINK = "libmodule-inner-capturer-sink.z.so";
    HpaeManager();
    ~HpaeManager();
    // sync interface
    int32_t Init() override;
    int32_t DeInit() override;
    int32_t RegisterSerivceCallback(const std::weak_ptr<AudioServiceHpaeCallback> &callback) override;
    int32_t RegisterHpaeDumpCallback(const std::weak_ptr<AudioServiceHpaeDumpCallback> &callback) override;
    void DumpSinkInfo(std::string deviceName) override;
    void DumpSourceInfo(std::string deviceName) override;
    void DumpAllAvailableDevice() override;
    void DumpSinkInputsInfo() override;
    void DumpSourceOutputsInfo() override;
    uint32_t OpenAudioPort(const AudioModuleInfo &audioModuleInfo) override;
    uint32_t ReloadAudioPort(const AudioModuleInfo &audioModuleInfo) override;
    int32_t CloseAudioPort(int32_t audioHandleIndex) override;
    int32_t GetSinkInfoByIdx(const int32_t &sinkIdx,
        std::function<void(const HpaeSinkInfo &sinkInfo, int32_t result)> callback) override;
    int32_t GetSourceInfoByIdx(const int32_t &sourceIdx,
        std::function<void(const HpaeSourceInfo &sourceInfo, int32_t result)> callback) override;

    int32_t GetAllSinkInputs() override;
    int32_t GetAllSourceOutputs() override;
    int32_t MoveSourceOutputByIndexOrName(
        uint32_t sourceOutputId, uint32_t sourceIndex, std::string sourceName) override;
    int32_t MoveSinkInputByIndexOrName(uint32_t sinkInputId, uint32_t sinkIndex, std::string sinkName) override;
    void HandleMsg() override;
    bool IsInit() override;
    bool IsRunning() override;
    bool IsMsgProcessing() override;
    // async interface
    int32_t SetDefaultSource(std::string name) override;
    int32_t SuspendAudioDevice(std::string &audioPortName, bool isSuspend) override;
    int32_t StopAudioPort(const std::string &audioPortName) override;
    bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false) override;
    int32_t SetSourceOutputMute(int32_t uid, bool setMute) override;
    int32_t SetSourceOutputMuteByStreamId(int32_t sessionId, bool setMute) override;
    int32_t GetAllSinks() override;

    int32_t GetMsgCount();

    void Invoke(HpaeMsgCode cmdID, const std::any &args) override;
    void InvokeSync(HpaeMsgCode cmdID, const std::any &args) override;
    // play and record stream interface
    int32_t CreateStream(const HpaeStreamInfo &streamInfo) override;
    int32_t DestroyStream(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Start(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t StartWithSyncId(HpaeStreamClassType streamClassType, uint32_t sessionId, int32_t syncId) override;
    int32_t Pause(HpaeStreamClassType streamClassType, uint32_t sessionId, bool isStandby = false) override;
    int32_t Flush(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Drain(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Stop(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Release(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t RegisterStatusCallback(HpaeStreamClassType streamClassType, uint32_t sessionId,
        const std::weak_ptr<IStreamStatusCallback> &callback) override;
    int32_t GetStreamSpanSize(HpaeStreamClassType streamClassType, uint32_t sessionId,
        uint32_t &spanSizeInFrame) override;
    int32_t GetStreamRouteFlag(HpaeStreamClassType streamClassType, uint32_t sessionId,
        uint32_t &routeFlag) override;
    // record stream interface
    int32_t RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback) override;
    int32_t GetSourceOutputInfo(uint32_t sessionId, HpaeStreamInfo &streamInfo) override;
    // play stream interface
    int32_t SetClientVolume(uint32_t sessionId, float volume) override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    int32_t SetRate(uint32_t sessionId, int32_t rate) override;
    int32_t SetAudioEffectMode(uint32_t sessionId, int32_t effectMode) override;
    int32_t GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode) override;
    int32_t SetPrivacyType(uint32_t sessionId, int32_t privacyType) override;
    int32_t GetPrivacyType(uint32_t sessionId, int32_t &privacyType) override;
    int32_t RegisterWriteCallback(uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback) override;
    int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state) override;
    size_t GetWritableSize(uint32_t sessionId) override;
    int32_t UpdateSpatializationState(
        uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled) override;
    int32_t UpdateMaxLength(uint32_t sessionId, uint32_t maxLength) override;
    int32_t SetOffloadRenderCallbackType(uint32_t sessionId, int32_t type) override;
    void SetSpeed(uint32_t sessionId, float speed) override;
    bool IsSessionHdiSpeedSupported(uint32_t sessionId) override;
    // only interface for unit test
    int32_t GetSessionInfo(HpaeStreamClassType streamClassType, uint32_t sessionId, HpaeSessionInfo &sessionInfo);

    // interfaces for render effect
    void InitAudioEffectChainManager(const std::vector<EffectChain> &effectChains,
        const EffectChainManagerParam &effectChainManagerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList) override;
    void SetOutputDeviceSink(int32_t device, const std::string &sinkName) override;
    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState) override;
    int32_t UpdateDeviceSpatialEnabled(const std::string &address,
        const AudioSpatializationState &state) override;
    int32_t RemoveDeviceSpatialEnabled(const std::string &address) override;
    int32_t ClearDeviceSpatialEnabled() override;
    int32_t UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType) override;
    int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType) override;
    int32_t EffectRotationUpdate(const uint32_t rotationState) override;
    int32_t SetEffectSystemVolume(const int32_t systemVolumeType, const float systemVolume) override;
    int32_t SetEffectSystemVolumeWithDeviceType(const int32_t systemVolumeType, const float systemVolume,
        DeviceType deviceType) override;
    int32_t SetAbsVolumeStateToEffect(const bool absVolumeState) override;
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) override;
    void InitHdiState() override;
    void UpdateEffectBtOffloadSupported(const bool &isSupported) override;
    void SetPersonalizedSpatializationStateResultCallback(
        std::function<void(int32_t)> callback) override;
    void UpdateParamExtra(const std::string &mainkey, const std::string &subkey, const std::string &value) override;
    // interfaces for capture effect
    void InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
        const EffectChainManagerParam &managerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList) override;
    int32_t SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice) override;
    int32_t SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol) override;
    int32_t SetMicrophoneMuteInfo(const bool &isMute) override;
    int32_t SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol) override;
    int32_t SetAudioEnhanceProperty(
        const AudioEffectPropertyArray &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t GetAudioEnhanceProperty(
        AudioEffectPropertyArray &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    void UpdateExtraSceneType(
        const std::string &mainkey, const std::string &subkey, const std::string &extraSceneType) override;
    void NotifySettingsDataReady() override;
    void NotifyAccountsChanged() override;
    bool IsAcousticEchoCancelerSupported(SourceType sourceType) override;
    bool SetEffectLiveParameter(const std::vector<std::pair<std::string, std::string>> &params) override;
    bool GetEffectLiveParameter(const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result) override;
    int32_t UpdateCollaborativeState(CollaborationState isCollaborationEnabled) override;
    void AddStreamVolumeToEffect(const std::string stringSessionID, const float streamVolume) override;
    void DeleteStreamVolumeToEffect(const std::string stringSessionID) override;
    uint64_t ProcessPendingTransitionsAndGetNextDelay();
    bool IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout) override;
    bool IsChannelLayoutSupportedForDspEffect(AudioChannelLayout channelLayout, DeviceType deviceType) override;
    // interfaces for injector
    void UpdateAudioPortInfo(const uint32_t &sinkPortIndex, const AudioModuleInfo &audioPortInfo) override;
    void AddCaptureInjector(
        const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex, const SourceType &sourceType) override;
    void RemoveCaptureInjector(
        const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex, const SourceType &sourceType) override;
    void AddCaptureInjectorByTargetStream(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        uint32_t targetStreamId) override;
    void RemoveCaptureInjectorByTargetStream(const uint32_t &sinkPortIndex, const uint32_t &sourcePortIndex,
        uint32_t targetStreamId) override;
    void SetCaptureMicMuteByTargetStream(uint32_t targetStreamId, bool isMute) override;
    int32_t UpdateCollaborativeProductId(const std::string &productId) override;
    void UpdatePersonalizedSpatializationState(const bool personalizedSpatializationEnabled) override;
    bool IsCollaborationSupportedForStreamUsage(DeviceType deviceType, StreamUsage streamUsage) override;
    int32_t UpdatePersonalizedHRTFBin(const int32_t &fd, const long &length) override;
    void LoadCollaborationConfig() override;
    void HandleBypassSpatializationForStereo(bool bypass) override;
    int32_t SetAppsEnhanceMuteState(const uint32_t &sessionId, bool isMute) override;

    int32_t SetAissEnabledSync(uint32_t sessionId, bool enabled) override;
    int32_t SetVocalRatioSync(float ratio) override;
    int32_t SetAissEnabledByUidSync(int32_t uid, bool enabled) override;
    int32_t DisableAissByUidWhenStreamInactiveSync(int32_t uid) override;
    uint64_t PostDelayedTask(uint64_t delayMs, std::function<void()> task);
    void CancelDelayedTask(uint64_t taskId);
    HpaeDelayedTaskQueue &GetDelayedTaskQueue();

    int32_t SetAuxiliarySinkEnable(bool isEnabled) override;
    void TriggerAppsUidUpdate(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    void SetStreamSilentMode(HpaeStreamClassType streamClassType, uint32_t sessionId, bool silentMode) override;
    float GetMaxAmplitude() override;
    void SetSinkMuteForSwitchDevice(const std::string &deviceClass, bool mute) override;

private:
    int32_t BindAndCreateAiss(uint32_t sessionId, bool aissSwitchEnabled = false);
    void TryBindSessionEffectOnStart(uint32_t sessionId);
    uint32_t FindRunningMusicSession(int32_t uid);
    int32_t UnbindAissFromOldUid(int32_t oldUid);
    int32_t BindAissToRunningSession(int32_t uid, int32_t oldUid, uint32_t targetSessionId);
    int32_t EnableAissByUid(int32_t uid);
    int32_t DisableAissByUid(int32_t uid);
    int32_t ClearAissSession(uint32_t sessionId, bool clearUidBinding = false, bool finalizeNow = false);
    // Deferred AISS-disable helpers (see DisableAissByUidWhenStreamInactiveSync).
    void FinishPendingAissDisable();
    // Cancel the backstop task, drop the request and bump the control epoch. Central helper for
    // every path that invalidates a pending deferred AISS-disable request.
    void InvalidatePendingAissDisable();
    bool IsPendingAissDisableFor(uint32_t sessionId) const;
    // Engine-thread body of DisableAissByUidWhenStreamInactiveSync: close AISS now when the bound
    // stream is already terminal, otherwise defer.
    int32_t DisableAissByUidWhenStreamInactive(int32_t uid);
    // Register a deferred AISS-disable request for an active/transitioning stream and arm the 1s
    // backstop delayed task.
    int32_t RegisterPendingAissDisable(int32_t uid, uint32_t sessionId, HpaeSessionState state);
    // Body of the 1s backstop delayed task of a registered deferred AISS-disable request.
    void RunAissDisableBackstop(uint32_t sessionId, uint64_t generation);
    int32_t CloseOutAudioPort(std::string sinkName);
    int32_t CloseInAudioPort(std::string sourceName);
    template <typename... Args>
    void RegisterHandler(HpaeMsgCode cmdID, void (HpaeManager::*func)(Args...));
    void HandleUpdateStatus(
        HpaeStreamClassType streamClassType, uint32_t sessionId, HpaeSessionState status, IOperation operation);
    void HandleUpdateSpanSize(uint32_t sessionId, uint32_t spanSizeInFrame, HpaeStreamClassType streamClassType);
    void HandleReloadDeviceResult(std::string deviceName, int32_t result);
    void HandleInitDeviceResult(std::string deviceName, int32_t result);
    void HandleMoveSinkInput(const std::shared_ptr<HpaeSinkInputNode> sinkInputNode, std::string sinkName);
    void HandleMoveAllSinkInputs(std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs, std::string sinkName,
        MoveSessionType moveType);
    void HandleMoveSourceOutput(HpaeCaptureMoveInfo moveInfo, std::string sourceName);
    void HandleMoveAllSourceOutputs(std::vector<HpaeCaptureMoveInfo> moveInfos, std::string sourceName,
        MoveSessionType moveType);
    void HandleMoveSessionFailed(HpaeStreamClassType streamClassType, uint32_t sessionId, MoveSessionType moveType,
        std::string name);
    void HandleDumpSinkInfo(std::string deviceName, std::string dumpStr);
    void HandleDumpSourceInfo(std::string deviceName, std::string dumpStr);
    void HandleConnectCoBufferNode(std::shared_ptr<HpaeCoBufferNode> hpaeCobufferNode);
    void HandleDisConnectCoBufferNode(std::shared_ptr<HpaeCoBufferNode> hpaeCobufferNode);
    void HandleInitSourceResult(SourceType sourceType);

    void SendRequest(Request &&request, std::string funcName);

    template<typename Ret>
    Ret SendRequestSync(std::function<Ret()> request, const std::string &funcName,
                        Ret defaultValue, uint32_t timeoutMs = 2000)
    {
        auto promise = std::make_shared<std::promise<Ret>>();
        auto future = promise->get_future();

        auto wrapper = [req = std::move(request), p = promise]() {
            p->set_value(req());
        };

        SendRequest(std::move(wrapper), funcName);

        auto status = future.wait_for(std::chrono::milliseconds(timeoutMs));
        if (status != std::future_status::ready) {
            AUDIO_ERR_LOG("SendRequestSync %{public}s timed out after %{public}u ms",
                funcName.c_str(), timeoutMs);
            return defaultValue;
        }
        return future.get();
    }
    int32_t OpenAudioPortInner(const AudioModuleInfo &audioModuleInfo);
    int32_t OpenOutputAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t sinkSourceIndex);
    int32_t OpenInputAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t sinkSourceIndex);
    int32_t OpenVirtualAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t sinkSourceIndex);
    bool HandleRendererManager(const std::string &sinkName, const HpaeStreamInfo &streamInfo);
    void CreateStreamForCapInner(const HpaeStreamInfo &streamInfo);
    int32_t CreateRendererManager(const AudioModuleInfo &audioModuleInfo, uint32_t sinkSourceIndex,
        bool isReload = false);
    int32_t CreateCaptureManager(HpaeSourceInfo &sourceInfo, uint32_t sinkSourceIndex, bool isReload = false);
    void UpdateStatus(const std::weak_ptr<IStreamStatusCallback> &callback, IOperation operation, uint32_t sessionId);

    std::shared_ptr<IHpaeRendererManager> GetRendererManagerById(uint32_t sessionId);
    std::shared_ptr<IHpaeCapturerManager> GetCapturerManagerById(uint32_t sessionId);
    std::shared_ptr<IHpaeRendererManager> GetRendererManagerByName(const std::string &sinkName);
    std::shared_ptr<IHpaeRendererManager> GetRendererManagerByDeviceClass(const std::string &deviceClass);
    std::shared_ptr<IHpaeCapturerManager> GetCapturerManagerByName(const std::string &sourceName);
    void AddStreamToCollection(const HpaeStreamInfo &streamInfo, const std::string &name);

    void MoveToPreferSink(const std::string &name, std::shared_ptr<AudioServiceHpaeCallback> &serviceCallback);
    int32_t ReloadRenderManager(const AudioModuleInfo &audioModuleInfo, bool isReload = false);
    int32_t ReloadCaptureManager(HpaeSourceInfo &sourceInfo, bool isReload = false);
    void DestroyCapture(uint32_t sessionId);
    void LoadEffectLive();

    bool MovingSinkStateChange(uint32_t sessionId, const std::shared_ptr<HpaeSinkInputNode> &sinkInput);
    bool SetMovingStreamState(HpaeStreamClassType streamType, uint32_t sessionId,
        HpaeSessionState status, HpaeSessionState state, IOperation operation);
    bool SetMovingPlayStreamState(uint32_t sessionId, HpaeSessionState state, IOperation operation);
    bool SetMovingCaptureStreamState(uint32_t sessionId, HpaeSessionState state, IOperation operation);
    void MarkClosingSinkPrefer(const std::string &sinkName);
    void OnCallbackOpenOrReloadFailed(bool isReload);
    bool ShouldNotSkipProcess(const HpaeStreamClassType &streamType, const uint32_t &sessionId);
    bool CheckMoveSinkInput(uint32_t sinkInputId, const std::string &sinkName);
    bool CheckMoveSourceOutput(uint32_t sourceOutputId, const std::string &sourceName);
    void CreateCoreSourceManager();
    void CreateCoreSinkManager();
    void DequeuePendingTransition(uint32_t sessionId);
    void EnqueuePendingTransition(uint32_t sessionId, HpaeSessionState state, IOperation operation);
    bool IsValidUpdateStatus(IOperation operation, HpaeSessionState currentState);
    void DeleteRendererManager(const std::string &name);
    void DeleteCaptureManager(const std::string &name);
    void DeleteAudioport(const std::string &name);
    std::vector<std::shared_ptr<HpaeSinkInputNode>> GetPerferSinkInputs(
        const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs);
    std::vector<HpaeCaptureMoveInfo> GetUsedMoveInfos(std::vector<HpaeCaptureMoveInfo> &moveInfos);
    std::vector<uint32_t> GetAllRenderSession(const std::string &name);
    std::vector<uint32_t> GetAllCaptureSession(const std::string &name);
    std::shared_ptr<IHpaeRendererManager> GetAuxiliaryRendererManager();
    void UpdateBypassSpatializationForStereo(bool bypass);

private:
    static constexpr uint32_t UNBOUND_SESSION_ID = 0;

    // Deferred AISS-disable request: close AISS once the bound play stream of `uid` reaches a
    // terminal state, or the 1s backstop delayed task expires - whichever comes first.
    // `generation` ties the request to the control epoch (aissControlGeneration_) captured at
    // registration, so any later Enable / immediate Disable / different-uid preempt / session-level
    // AISS control - each bumps aissControlGeneration_ - invalidates it.
    struct PendingAissDisable {
        int32_t uid { -1 };
        uint32_t sessionId { 0 };
        uint64_t generation { 0 };
        uint64_t taskId { 0 };  // backstop task in delayedTaskQueue_; 0 = none (ids start at 1)
    };

    std::unique_ptr<HpaeManagerThread> hpaeManagerThread_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<IHpaeCapturerManager>> capturerManagerMap_;
    std::unordered_map<std::string, std::shared_ptr<IHpaeRendererManager>> rendererManagerMap_;
    std::unordered_map<uint32_t, std::string> capturerIdSourceNameMap_;
    std::unordered_map<uint32_t, std::string> rendererIdSinkNameMap_;
    std::unordered_map<uint32_t, std::string> idPreferSinkNameMap_;
    std::unordered_map<uint32_t, HpaeSessionInfo> rendererIdStreamInfoMap_;
    std::unordered_map<uint32_t, HpaeSessionInfo> capturerIdStreamInfoMap_;
    std::unordered_map<uint32_t, SinkInput> sinkInputs_;
    std::unordered_map<uint32_t, SourceOutput> sourceOutputs_;
    std::unordered_map<std::string, uint32_t> sinkNameSinkIdMap_;  // todo
    std::unordered_map<uint32_t, std::string> sinkIdSinkNameMap_;
    std::unordered_map<uint32_t, HpaeSessionState> movingIds_;
    std::unordered_map<std::string, uint32_t> sourceNameSourceIdMap_;
    std::unordered_map<uint32_t, std::string> sourceIdSourceNameMap_;
    std::string defaultSource_ = "";
    std::string coreSource_ = "";
    std::atomic<int32_t> sinkSourceIndex_ = 0;
    std::atomic<bool> isInit_ = false;
    std::atomic<bool> auxSinkEnable_ = false;
    std::list<PendingStateTransition> pendingTransitionsTracker_;

    HpaeNoLockQueue hpaeNoLockQueue_;

    std::atomic<int32_t> receiveMsgCount_ = 0;
    std::weak_ptr<AudioServiceHpaeCallback> serviceCallback_;
    std::weak_ptr<AudioServiceHpaeDumpCallback> dumpCallback_;
    std::unordered_map<std::string, std::string> deviceDumpSinkInfoMap_;
    std::unordered_map<HpaeMsgCode, std::function<void(const std::any &)>> handlers_;
    std::string effectLiveState_ = "";
    std::mutex mutex_;

    std::unordered_map<uint32_t, std::shared_ptr<HpaeSinkVirtualOutputNode>> sinkVirtualOutputNodeMap_;
    std::mutex sinkVirtualOutputNodeMapMutex_;

    bool bypassSpatializationForStereo_ = false;
    bool adaptiveSpatialRenderingEnabled_ = false;

    // Per-device spatialization state cache, only meaningful under EnhancedRouting.
    // Accessed solely on HpaeSignalProcessThread via SendRequest, no lock required.
    std::unordered_map<std::string, AudioSpatializationState> deviceSpatialEnabledMap_;

    // Notify effect chain of the per-device spatialization state when an output port
    // is opened/reloaded, keyed by audioModuleInfo.macAddress. No-op for input/virtual
    // ports, when mac is empty, or when EnhancedRouting is off.
    void NotifySpatializationStateForPort(const AudioModuleInfo &audioModuleInfo);
    // Pushed when deviceSpatialEnabledMap_ is updated for a mac whose output port is
    // already open. Iterates rendererManagerMap_ to find a matching sink by macAddress.
    void NotifySpatializationStateIfPortOpen(const std::string &encryptedAddress,
        const AudioSpatializationState &state);

    std::unique_ptr<SessionEffectManager> sessionEffectManager_;
    HpaeDelayedTaskQueue delayedTaskQueue_;

    int32_t boundUid_ = -1;
    uint32_t boundSessionId_ = UNBOUND_SESSION_ID;
    std::optional<PendingAissDisable> pendingAissDisable_;
    uint64_t aissControlGeneration_ = 0;
    CollaborationState isCollaborationEnabled_ = COLLABORATION_CLOSE;
    CollaborationRouteMode collaborationRouteMode_ = COLLABORATION_INVALID_ROUTE_MODE;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // HPAE_HDI_MANAGER_H
