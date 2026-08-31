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

#ifndef HPAE_RENDER_MANAGER_H
#define HPAE_RENDER_MANAGER_H
#include <unordered_map>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include "hpae_signal_process_thread.h"
#include "hpae_sink_input_node.h"
#include "hpae_process_cluster.h"
#include "i_hpae_output_cluster.h"
#include "hpae_msg_channel.h"
#include "hpae_no_lock_queue.h"
#include "i_hpae_renderer_manager.h"
#include "hpae_co_buffer_node.h"
#include "effect_instance.h"
#include "i_effect_instance.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeRendererManager : public IHpaeRendererManager {
public:
    HpaeRendererManager(HpaeSinkInfo &sinkInfo);
    virtual ~HpaeRendererManager();
    int32_t CreateStream(const HpaeStreamInfo &streamInfo) override;
    int32_t DestroyStream(uint32_t sessionId) override;

    int32_t Start(uint32_t sessionId) override;
    int32_t StartWithSyncId(uint32_t sessionId, int32_t syncId) override;
    int32_t Pause(uint32_t sessionId, bool isStandby = false) override;
    int32_t Flush(uint32_t sessionId) override;
    int32_t Drain(uint32_t sessionId) override;
    int32_t Stop(uint32_t sessionId) override;
    int32_t Release(uint32_t sessionId) override;
    int32_t MoveStream(uint32_t sessionId, const std::string &sinkName) override;
    int32_t MoveAllStream(const std::string &sinkName, const std::vector<uint32_t>& sessionIds,
        MoveSessionType moveType = MOVE_ALL) override;
    int32_t SuspendStreamManager(bool isSuspend) override;
    int32_t SetMute(bool isMute) override;
    int32_t StopManager() override;
    void Process() override;
    void HandleMsg() override;
    int32_t Init(bool isReload = false) override;
    int32_t DeInit(bool isMoveDefault = false) override;
    bool IsInit() override;
    bool IsRunning(void) override;
    bool IsMsgProcessing() override;
    bool DeactivateThread() override;
    int32_t SetClientVolume(uint32_t sessionId, float volume) override;
    int32_t SetRate(uint32_t sessionId, int32_t rate) override;
    int32_t SetAudioEffectMode(uint32_t sessionId, int32_t effectMode) override;
    int32_t GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode) override;
    int32_t SetPrivacyType(uint32_t sessionId, int32_t privacyType) override;
    int32_t GetPrivacyType(uint32_t sessionId, int32_t &privacyType) override;
    int32_t RegisterWriteCallback(uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback) override;

    size_t GetWritableSize(uint32_t sessionId) override;
    int32_t UpdateSpatializationState(
        uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled) override;
    int32_t UpdateMaxLength(uint32_t sessionId, uint32_t maxLength) override;
    void SetSpeed(uint32_t sessionId, float speed) override;
    std::vector<SinkInput> GetAllSinkInputsInfo() override;
    int32_t GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo) override;
    int32_t RefreshProcessClusterByDevice() override;
    HpaeSinkInfo GetSinkInfo() override;

    int32_t AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node) override;
    int32_t AddAllNodesToSink(
        const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect) override;

    int32_t RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback) override;
    void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) override;
    void OnFadeDone(uint32_t sessionId) override;
    void OnCloseCrossfadeDone(uint32_t sessionId) override;
    void OnRequestLatency(uint32_t sessionId, uint64_t &latency) override;
    void OnRewindBuffer(uint32_t sessionId, uint64_t rewindTimeUs) override;
    void OnSessionEffectPreheatStateChanged(uint32_t sessionId, bool active) override;
    void OnNotifyQueue() override;
    std::string GetThreadName() override;
    int32_t DumpSinkInfo() override;
    int32_t ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload = false) override;
    int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state) override;
    std::string GetDeviceHDFDumpInfo() override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    void OnDisConnectProcessCluster(HpaeProcessorType sceneType) override;
    int32_t UpdateCollaborativeState(CollaborationState isCollaborationEnabled,
        CollaborationRouteMode collaborationRouteMode) override;
    int32_t ConnectCoBufferNode(const std::shared_ptr<HpaeCoBufferNode> &coBufferNode) override;
    int32_t SetAuxiliarySinkEnable(bool isEnabled) override;
    int32_t DisConnectCoBufferNode(const std::shared_ptr<HpaeCoBufferNode> &coBufferNode) override;
    void SetCollDelayCount() override;
    void TriggerAppsUidUpdate(uint32_t sessionId) override;
    void OnCollborationFadeDone(uint32_t sessionId) override;
    void OnDisConnectCoBufferWaitFadeOutDone() override;

    // Session Effect (synchronous — must be called from SignalProcessThread)
    int32_t CreateSessionEffect(uint32_t sessionId, const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false) override;
    int32_t DestroySessionEffect(uint32_t sessionId, bool finalizeNow = false) override;
    // Session Effect (async — dispatches to SignalProcessThread, safe to call from any thread)
    void PostCreateSessionEffect(uint32_t sessionId, const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false) override;
    int32_t PostDestroySessionEffect(uint32_t sessionId, bool finalizeNow = false) override;
    bool HasSessionEffect(uint32_t sessionId) const override;
    bool IsValidSession(uint32_t sessionId) const override;
    // Session Effect parameter routing
    int32_t SetSessionEffectParameter(uint32_t sessionId, int32_t type, float value) override;
    void PostSetSessionEffectParameter(uint32_t sessionId, int32_t type, float value) override;
    void SetCollabDoFade(HpaeProcessorType sceneType);

private:
    void SendRequest(Request &&request, const std::string &funcName, bool isInit = false) override;
    int32_t StartRenderSink();
    bool IsMchDevice();
    bool IsRemoteDevice();
    int32_t CreateInputSession(const HpaeStreamInfo &streamInfo);
    int32_t DeleteInputSession(uint32_t sessionId,
        SessionEffectMoveMode moveMode = SessionEffectMoveMode::FULL_TEARDOWN);
    bool isSplitProcessorType(HpaeProcessorType sceneType) const;
    int32_t ConnectInputSession(uint32_t sessionId);
    int32_t DisConnectInputSession(uint32_t sessionId);
    void SetSessionState(uint32_t sessionId, HpaeSessionState renderState);
    void AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect = true);
    void CreateProcessClusterAndConnect(HpaeNodeInfo &nodeInfo, bool isConnect = true);
    void MoveAllStreamToNewSink(const std::string &sinkName, const std::vector<uint32_t>& moveIds,
        MoveSessionType moveType);
    void UpdateProcessClusterConnection(uint32_t sessionId, int32_t effectMode);
    void ConnectProcessCluster(uint32_t sessionId, HpaeProcessorType sceneType);
    void ConnectInputCluster(uint32_t sessionId, HpaeProcessorType sceneType);
    void ConnectOutputCluster(uint32_t sessionId, HpaeProcessorType sceneType);
    void DisConnectInputCluster(uint32_t sessionId, HpaeProcessorType sceneType);
    void DisConnectOutputCluster(HpaeProcessorType sceneType);
    void CreateProcessCluster(HpaeNodeInfo &nodeInfo);
    void CreateProcessClusterInner(HpaeNodeInfo &nodeInfo, int32_t processClusterDecision);
    void DereferenceInputCluster(uint32_t sessionId);
    bool SetSessionFade(uint32_t sessionId, IOperation operation, bool isStandby = false);
    void CreateDefaultProcessCluster(HpaeNodeInfo &nodeInfo);
    void CreateOutputClusterNodeInfo(HpaeNodeInfo &nodeInfo);
    int32_t InitManager(bool isReload = false, bool isNeedCallBack = true);
    void InitDefaultNodeInfo();
    void MoveStreamSync(uint32_t sessionId, const std::string &sinkName);
    void UpdateAppsUid();
    int32_t HandlePriPaPower(uint32_t sessionId);
    bool CheckIsStreamRunning();
    HpaeProcessorType GetProcessorType(uint32_t sessionId) const;
    HpaeProcessorType GetConnectedProcessorType(uint32_t sessionId) const;
    std::optional<ChannelOverrideRoute> BuildChannelOverrideRoute(uint32_t sessionId) const;
    std::optional<ChannelOverrideRoute> BuildChannelOverrideRouteFromProcessorType(
        HpaeProcessorType sceneType) const;
    void SetChannelOverrideForRoute(const ChannelOverrideRoute &route);
    void ClearChannelOverrideForRoute(const ChannelOverrideRoute &route);
    void OnSessionEffectMaterialized(uint32_t sessionId);
    HpaeProcessorType TransToProperSceneType(StreamUsage streamUsage, AudioEffectScene effectScene);
    void ReConnectNodeForCollaboration(uint32_t sessionID);
    void EnableCollaboration();
    void DisableCollaboration();
    int32_t HandleSyncId(uint32_t sessionId, int32_t syncId);
    int32_t DeleteProcessCluster(uint32_t sessionId, bool needInitEffect = true,
        SessionEffectCleanupMode mode = SessionEffectCleanupMode::DROP_AND_DISABLE);
    int32_t DeleteProcessClusterInner(uint32_t sessionId, HpaeProcessorType sceneType);
    void RefreshProcessClusterByDeviceInner(const std::shared_ptr<HpaeSinkInputNode> &node);
    void SwitchBypassState(const std::shared_ptr<HpaeSinkInputNode> &node,
        HpaeNodeInfo &nodeInfo, int32_t processClusterDecision);
    void DisconnectNodeFromEffectChain(HpaeNodeInfo &nodeInfo);
    void TriggerStreamState(uint32_t sessionId, const std::shared_ptr<HpaeSinkInputNode> &node);
    /**
     * @brief Refresh the usage and type of the cluster corresponding to the given scene type.
     * @param sceneType scene type of the target cluster.
     * @return errCode
     */
    int32_t UpdateClusterStreamInfo(HpaeProcessorType sceneType);
    bool IsClusterDisConnected(HpaeProcessorType sceneType);
    bool QueryOneStreamUnderrun();
    void DeleteNodesByTraversal(uint32_t sessionId,
        SessionEffectCleanupMode mode = SessionEffectCleanupMode::DROP_AND_DISABLE);
    void StopOuputNode();
    void NotifyStreamChangeToSink(StreamChangeType change, uint32_t sessionId,
        RendererState state, uint32_t appUid = INVALID_UID);
    void DisConnectCoBufferFromDeleteProcessCluster(HpaeProcessorType sceneType);
    void ConnectCoBufferFromConnectOutputCluster(HpaeProcessorType sceneType);
    void DisConnectCoBufferFromOnDisConnectProcessCluster(HpaeProcessorType sceneType);
    void ReloadRenderManagerInner(const HpaeSinkInfo &sinkInfo, bool isReload, bool isNeedCallBack);
    void InitDefaultNodeInfoFromInitManager(bool isReload);
    void StopInterphoneSinkIfNeeded();
    void DetachSessionEffectNodeForRefresh(uint32_t sessionId, HpaeProcessorType oldSceneType);
    void MaterializeSessionEffectFromAttachState(uint32_t sessionId, HpaeProcessorType sceneType);
    void HandleSessionEffectMaterializeFailure(uint32_t sessionId, int32_t ret,
        const std::shared_ptr<HpaeSinkInputNode> &sinkInputNode);
    // STOPPING + finalizeNow: finalize the close a prior BeginClose left waiting on its crossfade
    // callback right now, so the old session is fully closed before the new one is created.
    void FinalizeStoppingSessionEffect(uint32_t sessionId, std::optional<ChannelOverrideRoute> route,
        HpaeSessionState state, const std::shared_ptr<HpaeProcessCluster> &processCluster);
    // Rewind upstream history by the combined latency; clear the logical binding.
    void RewindAndClearSessionEffectAttach(uint32_t sessionId, uint64_t latency);
    // Rescue/delivery: a close-crossfade is (or was) waiting on its completion callback —
    // finalize now instead of waiting. Identity-checked against the pending-close token.
    // Returns true when this call finalized the close.
    bool FinalizePendingSessionEffectClose(uint32_t sessionId);
    // Finalize close: driven by the OnCloseCrossfadeDone callback (smooth path), run directly
    // inside the destroy request (finalizeNow, or non-ACTIVE node with no crossfade pending),
    // or run by a Pause/Stop/OnFadeDone rescue. Identity-checked: clears channel override +
    // emits DISABLED only if the cluster actually finalized.
    void FinalizeSessionEffectClose(uint32_t sessionId, std::optional<ChannelOverrideRoute> route,
        HpaeSessionState state, const std::weak_ptr<HpaeSessionEffectNode> &closingNode);
    uint64_t SetPendingSessionEffectCreate(uint32_t sessionId,
        const std::shared_ptr<IEffectInstance> &instance);
    bool IsPendingSessionEffectCreate(uint32_t sessionId, uint64_t generation,
        const std::shared_ptr<IEffectInstance> &instance) const;
    void CancelPendingSessionEffectCreate(uint32_t sessionId);
    // Returns true (and requeues a retry) when the cluster's SE node for this session is
    // STOPPING; false otherwise.
    bool TryDeferSessionEffectCreate(uint32_t sessionId, const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false);
    int32_t InitOutputCluster();

private:

    std::unordered_map<uint32_t, HpaeRenderSessionInfo> sessionNodeMap_;
    std::unordered_map<HpaeProcessorType, std::shared_ptr<HpaeProcessCluster>> sceneClusterMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSinkInputNode>> sinkInputNodeMap_;
    std::shared_ptr<IHpaeOutputCluster> outputCluster_ = nullptr;
    HpaeNoLockQueue hpaeNoLockQueue_;
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread_ = nullptr;
    std::atomic<bool> isInit_ = false;
    std::atomic<bool> isMute_ = false;
    std::atomic<bool> isSuspend_ = false;
    HpaeSinkInfo sinkInfo_;
    std::unordered_map<HpaeProcessorType, int32_t> sceneTypeToProcessClusterCountMap_;
    std::vector<int32_t> appsUid_;
    std::shared_ptr<HpaeCoBufferNode> hpaeCoBufferNode_ = nullptr;
    std::shared_ptr<HpaeCoBufferNode> defaultCoBufferNode_ = nullptr;
    CollaborationState isCollaborationEnabled_ = COLLABORATION_CLOSE;
    CollaborationRouteMode collaborationRouteMode_ = COLLABORATION_INVALID_ROUTE_MODE;
    int64_t noneStreamTime_ = 0; // if no stream, 3s time out to stop rendersink
    std::unordered_map<uint32_t, bool> isNeedInitEffectBufferFlagMap_;

    // Pending SessionEffect create deferred behind a STOPPING node (rapid recreate), per session:
    // sessionId -> {monotonic generation, weak instance}. Single-stream SE in practice, but
    // per-session keyed so concurrent close+recreate on different sessions cannot drop a request.
    uint64_t pendingSeCreateGeneration_ = 0;
    std::unordered_map<uint32_t, std::pair<uint64_t, std::weak_ptr<IEffectInstance>>> pendingSeCreateMap_;

    // Pending close-crossfade finalize waiting on OnCloseCrossfadeDone, per session:
    // sessionId -> identity token of the closing node (guards late callbacks against
    // finalizing a newer node after a rescue + recreate).
    std::unordered_map<uint32_t, std::weak_ptr<HpaeSessionEffectNode>> pendingSeFinalizeMap_;

    int64_t lastOnUnderrunTime_ = 0;
    int64_t lastSessionStateChangeTime_ = 0;
    bool coBufferNodeIsConnected_ = false;
    bool isReloadManagerFromCollStateChange_ = false;
    uint32_t renderId_ = HDI_INVALID_ID;
    DeviceType renderDeviceType_ = DEVICE_TYPE_INVALID;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif
