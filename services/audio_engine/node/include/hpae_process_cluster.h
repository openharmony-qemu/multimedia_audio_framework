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

#ifndef HPAE_PROCESS_CLUSTER_H
#define HPAE_PROCESS_CLUSTER_H
#include "hpae_mixer_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_gain_node.h"
#include "hpae_render_effect_node.h"
#include "hpae_loudness_gain_node.h"
#include "hpae_session_effect_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

enum class SessionEffectCleanupMode {
    DROP_AND_DISABLE,
    DETACH_FOR_RELOAD,
};

class HpaeProcessCluster : public OutputNode<HpaePcmBuffer*>, public InputNode<HpaePcmBuffer*>,
    public INodeFormatInfoCallback {
public:
    HpaeProcessCluster(HpaeNodeInfo nodeInfo, HpaeSinkInfo &sinkInfo);
    virtual ~HpaeProcessCluster();
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    int32_t GetGainNodeCount();
    int32_t GetConverterNodeCount();
    int32_t GetLoudnessGainNodeCount();
    int32_t GetPreOutNum();
    int32_t AudioRendererCreate(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t HandleRendererStart(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioRendererStart(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t HandleRendererStop(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioRendererStop(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioRendererRelease(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    void ExecuteDeferredStop();
    bool ExecuteDeferredStopFor(uint32_t sessionId);
    bool HasDeferredStop() const { return hasPendingStop_; }
    uint32_t GetDeferredStopSessionId() const { return pendingStopSessionId_; }
    int32_t GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat) override;
    std::shared_ptr<HpaeGainNode> GetGainNodeById(uint32_t id) const;
    std::shared_ptr<HpaeAudioFormatConverterNode> GetConverterNodeById(uint32_t id) const;
    void SetConnectedFlag(bool flag);
    bool GetConnectedFlag() const;
    int32_t SetupAudioLimiter();
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain);
    void DisConnectMixerNode();
    void InitEffectBuffer(const uint32_t sessionId);
    uint64_t GetLatency(uint32_t sessionId);
    int32_t CreateNodes(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode);
    int32_t DestroyNodes(uint32_t sessionId,
        SessionEffectCleanupMode mode = SessionEffectCleanupMode::DROP_AND_DISABLE);
    int32_t CheckNodes(uint32_t sessionId);
    void Flush(uint32_t sessionId);

    // Session Effect management
    int32_t CreateSessionEffectNode(uint32_t sessionId, const std::string &effectName,
        std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
        bool aissSwitchEnabled = false);
    // Synchronous immediate destroy (Begin+Finalize in one call; no smooth-close opportunity).
    int32_t DestroySessionEffectNode(uint32_t sessionId, uint64_t &outLatency);
    // Two-phase smooth close: Begin enters STOPPING (node stays in map) and returns the combined
    // SE+upstream-converter latency + a weak identity handle; Finalize (next queue cycle) does the
    // identity-checked disconnect+drop+erase. pendingInstance drives same-instance handoff.
    int32_t BeginDestroySessionEffectNode(uint32_t sessionId, uint64_t &outLatency,
        std::weak_ptr<HpaeSessionEffectNode> &outWeakNode);
    bool FinalizeDestroySessionEffectNode(uint32_t sessionId,
        const std::weak_ptr<HpaeSessionEffectNode> &closingNode,
        std::shared_ptr<IEffectInstance> pendingInstance);
    bool IsSessionEffectNodeStopping(uint32_t sessionId) const;
    bool HasSessionEffectNode(uint32_t sessionId) const;
    int32_t SessionEffectStart(uint32_t sessionId);
    int32_t SessionEffectFlush(uint32_t sessionId);
    int32_t SessionEffectPause(uint32_t sessionId);
    void SessionEffectPrepareMoveStream(uint32_t sessionId);
    int32_t SetSessionEffectParameter(uint32_t sessionId, int32_t type, float value);
    std::shared_ptr<HpaeSessionEffectNode> GetSessionEffectNode(uint32_t sessionId) const;
    std::shared_ptr<HpaeLoudnessGainNode> GetLoudnessGainNode(uint32_t sessionId) const;
    std::shared_ptr<HpaeRenderEffectNode> GetRenderEffectNode() const;
    void SetCollaborationStateToMixNode(CollaborationState isCollaborationEnabled);
    void SetDelayCountToEffectNode(int32_t delayCount);
    void SetCollabDoFade(bool fadeOutState);
    void SetStopRunningState(bool stopRunning);

private:
    void ConnectMixerNode();
    void ClearDeferredStop();
    void CreateGainNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo);
    void CreateConverterNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo);
    void CreateLoudnessGainNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo);
    bool CheckNeedNotifyEffectNode(const HpaeSinkInfo &sinkInfo);
    std::shared_ptr<HpaeMixerNode> mixerNode_;
    std::shared_ptr<HpaeRenderEffectNode> renderEffectNode_ = nullptr;
    std::shared_ptr<HpaeRenderEffectNode> renderNoneEffectNode_ = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeAudioFormatConverterNode>> idConverterMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeGainNode>> idGainMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeLoudnessGainNode>> idLoudnessGainNodeMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSessionEffectNode>> idSessionEffectMap_;
    HpaeSinkInfo sinkInfo_;
    bool isConnectedToOutputCluster = false;
    bool hasPendingStop_ = false;
    uint32_t pendingStopSessionId_ = 0;
    HpaeNodeInfo pendingStopNodeInfo_ {};
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_PROCESS_CLUSTER_H
