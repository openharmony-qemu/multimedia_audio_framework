/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef HPAE_DIRECT_RENDER_MANAGER_H
#define HPAE_DIRECT_RENDER_MANAGER_H
#include <unordered_map>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <shared_mutex>
#include "hpae_signal_process_thread.h"
#include "hpae_sink_input_node.h"
#include "hpae_process_cluster.h"
#include "hpae_direct_sinkoutput_node.h"
#include "hpae_msg_channel.h"
#include "hpae_no_lock_queue.h"
#include "i_hpae_renderer_manager.h"
#include "hpae_gain_node.h"
#include "hpae_limiter_node.h"
#include "hpae_session_effect_node.h"
#include "hpae_volume_sync_node.h"
#include "hpae_loudness_gain_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeDirectRendererManager : public IHpaeRendererManager, public INodeFormatInfoCallback {
public:
    HpaeDirectRendererManager(HpaeSinkInfo& sinkInfo);
    virtual ~HpaeDirectRendererManager();
    int32_t CreateStream(const HpaeStreamInfo &streamInfo) override;
    int32_t DestroyStream(uint32_t sessionId) override;

    int32_t Start(uint32_t sessionId) override;
    int32_t Pause(uint32_t sessionId, bool isStandby = false) override;
    int32_t Flush(uint32_t sessionId) override;
    int32_t Drain(uint32_t sessionId) override;
    int32_t Stop(uint32_t sessionId) override;
    int32_t Release(uint32_t sessionId) override;
    int32_t MoveStream(uint32_t sessionId, const std::string& sinkName) override;
    int32_t MoveAllStream(const std::string& sinkName, const std::vector<uint32_t>& sessionIds,
        MoveSessionType moveType = MOVE_ALL) override;
    int32_t SuspendStreamManager(bool isSuspend) override;
    int32_t StopManager() override;
    int32_t SetMute(bool isMute) override;
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
    int32_t RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback) override;

    int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state) override;
    size_t GetWritableSize(uint32_t sessionId) override;
    int32_t UpdateSpatializationState(uint32_t sessionId, bool spatializationEnabled,
        bool headTrackingEnabled) override;
    int32_t UpdateMaxLength(uint32_t sessionId, uint32_t maxLength) override;
    void SetSpeed(uint32_t sessionId, float speed) override;
    std::vector<SinkInput> GetAllSinkInputsInfo() override;
    int32_t GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat) override;
    int32_t GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo) override;
    int32_t RefreshProcessClusterByDevice() override;
    HpaeSinkInfo GetSinkInfo() override;

    int32_t AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node) override;
    int32_t AddAllNodesToSink(
        const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect) override;

    void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) override;
    void OnFadeDone(uint32_t sessionId) override;
    void OnCloseCrossfadeDone(uint32_t sessionId) override;
    void OnRequestLatency(uint32_t sessionId, uint64_t &latency) override;
    void OnRewindBuffer(uint32_t sessionId, uint64_t rewindTimeUs) override;
    void OnNotifyQueue() override;
    std::string GetThreadName() override;
    int32_t DumpSinkInfo() override;
    int32_t ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload = false) override;
    std::string GetDeviceHDFDumpInfo() override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    void TriggerAppsUidUpdate(uint32_t sessionId) override;

    // Session Effect (synchronous — must be called from SignalProcessThread)
    int32_t CreateSessionEffect(uint32_t sessionId, const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false) override;
    int32_t DestroySessionEffect(uint32_t sessionId, bool finalizeNow = false) override;
    // Session Effect (async — dispatches to SignalProcessThread)
    void PostCreateSessionEffect(uint32_t sessionId, const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false) override;
    int32_t PostDestroySessionEffect(uint32_t sessionId, bool finalizeNow = false) override;
    bool IsValidSession(uint32_t sessionId) const override;
    // Session Effect parameter routing
    int32_t SetSessionEffectParameter(uint32_t sessionId, int32_t type, float value) override;
    void PostSetSessionEffectParameter(uint32_t sessionId, int32_t type, float value) override;

private:
    void SendRequest(Request &&request, const std::string &funcName, bool isInit = false) override;
    int32_t StartRenderSink();
    std::shared_ptr<HpaeSinkInputNode> CreateInputSession(const HpaeStreamInfo &streamInfo);
    int32_t CreateDirectNodes();
    int32_t DestroyDirectNodes();
    int32_t ConnectInputSession();
    int32_t DisConnectInputSession(bool isStandby = false);
    void DeleteInputSession(SessionEffectMoveMode moveMode = SessionEffectMoveMode::FULL_TEARDOWN);
    void AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect = true);
    void MoveAllStreamToNewSink(const std::string &sinkName, const std::vector<uint32_t> &moveIds,
        MoveSessionType moveType);
    int32_t InitSinkInner(bool isReload = false, bool isCallback = true);
    void UpdateAppsUid();
    void AddNodeToMap(std::shared_ptr<HpaeSinkInputNode> node);
    void RemoveNodeFromMap(uint32_t sessionId);
    void SetCurrentNode();
    void StopOuputNode();
    void NotifyStreamChangeToSink(StreamChangeType change, uint32_t sessionId, RendererState state,
        uint32_t appUid = INVALID_UID);
    void SetStreamSilentMode(uint32_t sessionId, bool silentMode) override;
    int32_t RecreateSinkOutputNodeIfNeeded();
    void CreateProcessNodes(uint32_t sessionId);
    bool SetSessionFade(uint32_t sessionId, IOperation operation, bool isStandby = false);
    AudioSamplingRate GetDirectSampleRate(AudioSamplingRate sampleRate, bool isVoip);
    AudioSampleFormat GetDirectFormat(AudioSampleFormat format, bool isVoip);
    void StopDirectOutputNodeSync(bool isStandby);
    void StopDirectOutputNode(bool isStandby);

private:
    std::shared_ptr<HpaeSinkInputNode> curNode_ = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSinkInputNode>> sinkInputNodeMap_;
    std::shared_ptr<HpaeAudioFormatConverterNode> converterForGain_ = nullptr;
    std::shared_ptr<HpaeAudioFormatConverterNode> converterForOutput_ = nullptr;
    std::shared_ptr<HpaeGainNode> gainNode_ = nullptr;
    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode_ = nullptr;
    std::shared_ptr<HpaeLimiterNode> limiterNode_ = nullptr;
    std::shared_ptr<HpaeLoudnessGainNode> loudnessGainNode_ = nullptr;
    std::unique_ptr<HpaeDirectSinkOutputNode> sinkOutputNode_ = nullptr;
    HpaeNoLockQueue hpaeNoLockQueue_;

    // Session Effect state: sessionEffectNode_/sessionEffectSessionId_ cache the
    // currently materialized physical SE node. Logical binding (attach state) lives
    // on each HpaeSinkInputNode via SetSessionEffectAttachState.
    std::shared_ptr<HpaeSessionEffectNode> sessionEffectNode_ = nullptr;
    uint32_t sessionEffectSessionId_ = 0;

    // Pending close-crossfade finalize waiting on OnCloseCrossfadeDone: identity token of the
    // closing node (guards late callbacks against finalizing a newer node after rescue+recreate).
    std::weak_ptr<HpaeSessionEffectNode> pendingCloseNode_;
    uint32_t pendingCloseSessionId_ = 0;

    // Pending SessionEffect create deferred behind a STOPPING node (rapid recreate).
    uint32_t pendingSeCreateSessionId_ = 0;
    uint64_t pendingSeCreateGeneration_ = 0;
    std::weak_ptr<IEffectInstance> pendingSeCreateInstance_;

    // Session Effect helpers (called on SignalProcessThread)
    int32_t InsertSessionEffectNode(const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false);
    int32_t RemoveSessionEffectNode(bool finalizeNow = false);
    // BeginClose: rewind + enter STOPPING; Finalize direct when finalizeNow or no crossfade
    // pending, else deferred to the OnCloseCrossfadeDone callback
    void FinalizeSessionEffectClose(uint32_t sessionId,
        const std::weak_ptr<HpaeSessionEffectNode> &closingNode);
    // Erase the pending-close token (single slot — at most one materialized SE node / one SE
    // close at a time).
    void ClearPendingSessionEffectClose();
    // Rescue/delivery: finalize the tokened closing node if it is still the current node.
    // Returns true when this call finalized the close.
    bool FinalizePendingSessionEffectClose(uint32_t sessionId);
    uint64_t SetPendingSessionEffectCreate(uint32_t sessionId,
        const std::shared_ptr<IEffectInstance> &instance);
    bool IsPendingSessionEffectCreate(uint32_t sessionId, uint64_t generation,
        const std::shared_ptr<IEffectInstance> &instance) const;
    void CancelPendingSessionEffectCreate(uint32_t sessionId);
    // Returns true (and requeues a retry) when curNode's SE node is STOPPING; false otherwise.
    bool TryDeferSessionEffectCreate(uint32_t sessionId, const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false);
    // Materialize SE for the current session (curNode_). Chain readiness is checked inside.
    int32_t MaterializeCurrentSessionEffect(uint32_t sessionId, const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false);
    void MaterializeSessionEffectFromAttachState(uint32_t curSessionId);
    void DetachSessionEffectNodeForReload();
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread_ = nullptr;
    std::atomic<bool> isInit_ = false;
    std::vector<int32_t> appsUid_;
    HpaeSinkInfo sinkInfo_;
    bool isMute_ = false;
    std::atomic<bool> isSuspend_ = false;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_DIRECT_RENDER_MANAGER_H
