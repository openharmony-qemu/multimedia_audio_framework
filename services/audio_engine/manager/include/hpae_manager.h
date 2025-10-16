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
#include "audio_module_info.h"
#include "hpae_capturer_manager.h"
#include "hpae_renderer_manager.h"
#include "hpae_inner_capturer_manager.h"
#include "hpae_msg_channel.h"
#include "i_hpae_manager.h"
#include "i_hpae_renderer_manager.h"
#include "hpae_policy_manager.h"
#include "high_resolution_timer.h"

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
    void DumpAllAvailableDevice(HpaeDeviceInfo &devicesInfo) override;
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
    int32_t SetDefaultSink(std::string name) override;
    int32_t SetDefaultSource(std::string name) override;
    int32_t SuspendAudioDevice(std::string &audioPortName, bool isSuspend) override;
    bool SetSinkMute(const std::string &sinkName, bool isMute, bool isSync = false) override;
    int32_t SetSourceOutputMute(int32_t uid, bool setMute) override;
    int32_t GetAllSinks() override;

    int32_t GetMsgCount();

    void Invoke(HpaeMsgCode cmdID, const std::any &args) override;
    // play and record stream interface
    int32_t CreateStream(const HpaeStreamInfo &streamInfo) override;
    int32_t DestroyStream(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Start(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t StartWithSyncId(HpaeStreamClassType streamClassType, uint32_t sessionId, int32_t syncId) override;
    int32_t Pause(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Flush(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Drain(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Stop(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t Release(HpaeStreamClassType streamClassType, uint32_t sessionId) override;
    int32_t RegisterStatusCallback(HpaeStreamClassType streamClassType, uint32_t sessionId,
        const std::weak_ptr<IStreamStatusCallback> &callback) override;
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
    // only interface for unit test
    int32_t GetSessionInfo(HpaeStreamClassType streamClassType, uint32_t sessionId, HpaeSessionInfo &sessionInfo);

    // interfaces for render effect
    void InitAudioEffectChainManager(const std::vector<EffectChain> &effectChains,
        const EffectChainManagerParam &effectChainManagerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &effectLibraryList) override;
    void SetOutputDeviceSink(int32_t device, const std::string &sinkName) override;
    int32_t UpdateSpatializationState(AudioSpatializationState spatializationState) override;
    int32_t UpdateSpatialDeviceType(AudioSpatialDeviceType spatialDeviceType) override;
    int32_t SetSpatializationSceneType(AudioSpatializationSceneType spatializationSceneType) override;
    int32_t EffectRotationUpdate(const uint32_t rotationState) override;
    int32_t SetEffectSystemVolume(const int32_t systemVolumeType, const float systemVolume) override;
    int32_t SetAbsVolumeStateToEffect(const bool absVolumeState) override;
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArrayV3 &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArrayV3 &propertyArray) override;
    int32_t SetAudioEffectProperty(const AudioEffectPropertyArray &propertyArray) override;
    int32_t GetAudioEffectProperty(AudioEffectPropertyArray &propertyArray) override;
    void InitHdiState() override;
    void UpdateEffectBtOffloadSupported(const bool &isSupported) override;
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
        const AudioEffectPropertyArrayV3 &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t GetAudioEnhanceProperty(
        AudioEffectPropertyArrayV3 &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t SetAudioEnhanceProperty(
        const AudioEnhancePropertyArray &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    int32_t GetAudioEnhanceProperty(
        AudioEnhancePropertyArray &propertyArray, DeviceType deviceType = DEVICE_TYPE_NONE) override;
    void UpdateExtraSceneType(
        const std::string &mainkey, const std::string &subkey, const std::string &extraSceneType) override;
    void NotifySettingsDataReady() override;
    void NotifyAccountsChanged() override;
    bool IsAcousticEchoCancelerSupported(SourceType sourceType) override;
    bool SetEffectLiveParameter(const std::vector<std::pair<std::string, std::string>> &params) override;
    bool GetEffectLiveParameter(const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result) override;
    int32_t UpdateCollaborativeState(bool isCollaborationEnabled) override;
    void AddStreamVolumeToEffect(const std::string stringSessionID, const float streamVolume) override;
    void DeleteStreamVolumeToEffect(const std::string stringSessionID) override;
    uint64_t ProcessPendingTransitionsAndGetNextDelay();
private:
    int32_t CloseOutAudioPort(std::string sinkName);
    int32_t CloseInAudioPort(std::string sourceName);
    template <typename... Args>
    void RegisterHandler(HpaeMsgCode cmdID, void (HpaeManager::*func)(Args...));
    void HandleUpdateStatus(
        HpaeStreamClassType streamClassType, uint32_t sessionId, HpaeSessionState status, IOperation operation);
    void HandleReloadDeviceResult(std::string deviceName, int32_t result);
    void HandleInitDeviceResult(std::string deviceName, int32_t result);
    void HandleMoveSinkInput(const std::shared_ptr<HpaeSinkInputNode> sinkInputNode, std::string sinkName);
    void HandleMoveAllSinkInputs(std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs, std::string sinkName,
        MoveSessionType moveType);
    void HandleMoveSourceOutput(HpaeCaptureMoveInfo moveInfo, std::string sourceName);
    void HandleMoveAllSourceOutputs(const std::vector<HpaeCaptureMoveInfo> moveInfos, std::string sourceName);
    void HandleMoveSessionFailed(HpaeStreamClassType streamClassType, uint32_t sessionId, MoveSessionType moveType,
        std::string name);
    void HandleDumpSinkInfo(std::string deviceName, std::string dumpStr);
    void HandleDumpSourceInfo(std::string deviceName, std::string dumpStr);
    void HandleConnectCoBufferNode(std::shared_ptr<HpaeCoBufferNode> hpaeCobufferNode);
    void HandleDisConnectCoBufferNode(std::shared_ptr<HpaeCoBufferNode> hpaeCobufferNode);
    void HandleInitSourceResult(SourceType sourceType);

    void SendRequest(Request &&request, std::string funcName);
    int32_t OpenAudioPortInner(const AudioModuleInfo &audioModuleInfo);
    int32_t OpenOutputAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t sinkSourceIndex);
    int32_t OpenInputAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t sinkSourceIndex);
    int32_t OpenVirtualAudioPort(const AudioModuleInfo &audioModuleInfo, uint32_t sinkSourceIndex);
    void HandleRendererManager(const std::string& sinkName, const HpaeStreamInfo &streamInfo);
    void CreateStreamForCapInner(const HpaeStreamInfo &streamInfo);
    int32_t CreateRendererManager(const AudioModuleInfo &audioModuleInfo, uint32_t sinkSourceIndex,
        bool isReload = false);
    void UpdateStatus(const std::weak_ptr<IStreamStatusCallback> &callback, IOperation operation, uint32_t sessionId);

    std::shared_ptr<IHpaeRendererManager> GetRendererManagerById(uint32_t sessionId);
    std::shared_ptr<IHpaeCapturerManager> GetCapturerManagerById(uint32_t sessionId);
    std::shared_ptr<IHpaeRendererManager> GetRendererManagerByName(const std::string &sinkName);
    std::shared_ptr<IHpaeCapturerManager> GetCapturerManagerByName(const std::string &sourceName);
    void AddStreamToCollection(const HpaeStreamInfo &streamInfo, const std::string &name);

    void MoveToPreferSink(const std::string& name, std::shared_ptr<AudioServiceHpaeCallback> &serviceCallback);
    int32_t ReloadRenderManager(const AudioModuleInfo &audioModuleInfo, bool isReload = false);
    void DestroyCapture(uint32_t sessionId);
    void LoadEffectLive();

    bool MovingSinkStateChange(uint32_t sessionId, const std::shared_ptr<HpaeSinkInputNode>& sinkInput);
    bool SetMovingStreamState(HpaeStreamClassType streamType, uint32_t sessionId,
        HpaeSessionState status, HpaeSessionState state, IOperation operation);
    void AddPreferSinkForDefaultChange(bool isAdd, const std::string &sinkName);
    void OnCallbackOpenOrReloadFailed(bool isReload);
    bool ShouldNotSkipProcess(const HpaeStreamClassType &streamType, const uint32_t &sessionId);
    bool CheckMoveSinkInput(uint32_t sinkInputId, const std::string &sinkName);
    bool CheckMoveSourceOutput(uint32_t sourceOutputId, const std::string &sourceName);
    void DequeuePendingTransition(uint32_t sessionId);
    void EnqueuePendingTransition(uint32_t sessionId, HpaeSessionState state, IOperation operation);
    bool IsValidUpdateStatus(IOperation operation, HpaeSessionState currentState);

private:
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
    std::string defaultSink_ = "";
    std::string coreSink_ = "";
    std::unordered_map<std::string, uint32_t> sourceNameSourceIdMap_;
    std::unordered_map<uint32_t, std::string> sourceIdSourceNameMap_;
    std::string defaultSource_ = "Built_in_mic";
    std::atomic<int32_t> sinkSourceIndex_ = 0;
    std::atomic<bool> isInit_ = false;
    std::list<PendingStateTransition> pendingTransitionsTracker_;

    HpaeNoLockQueue hpaeNoLockQueue_;

    std::atomic<int32_t> receiveMsgCount_ = 0;
    std::weak_ptr<AudioServiceHpaeCallback> serviceCallback_;
    std::weak_ptr<AudioServiceHpaeDumpCallback> dumpCallback_;
    std::unordered_map<std::string, std::string> deviceDumpSinkInfoMap_;
    std::unordered_map<HpaeMsgCode, std::function<void(const std::any &)>> handlers_;
    std::string effectLiveState_ = "";
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // HPAE_HDI_MANAGER_H