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
#include "hpae_signal_process_thread.h"
#include "hpae_sink_input_node.h"
#include "hpae_process_cluster.h"
#include "i_hpae_output_cluster.h"
#include "hpae_msg_channel.h"
#include "hpae_no_lock_queue.h"
#include "i_hpae_renderer_manager.h"
#include "hpae_co_buffer_node.h"

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
    int32_t Pause(uint32_t sessionId) override;
    int32_t Flush(uint32_t sessionId) override;
    int32_t Drain(uint32_t sessionId) override;
    int32_t Stop(uint32_t sessionId) override;
    int32_t Release(uint32_t sessionId) override;
    int32_t MoveStream(uint32_t sessionId, const std::string &sinkName) override;
    int32_t MoveAllStream(const std::string &sinkName, const std::vector<uint32_t>& sessionIds,
        MoveSessionType moveType = MOVE_ALL) override;
    int32_t SuspendStreamManager(bool isSuspend) override;
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
    void OnRequestLatency(uint32_t sessionId, uint64_t &latency) override;
    void OnNotifyQueue() override;
    std::string GetThreadName() override;
    int32_t DumpSinkInfo() override;
    int32_t ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload = false) override;
    int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state) override;
    std::string GetDeviceHDFDumpInfo() override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    void OnDisConnectProcessCluster(HpaeProcessorType sceneType) override;
    int32_t UpdateCollaborativeState(bool isCollaborationEnabled) override;
    int32_t ConnectCoBufferNode(const std::shared_ptr<HpaeCoBufferNode> &coBufferNode) override;
    int32_t DisConnectCoBufferNode(const std::shared_ptr<HpaeCoBufferNode> &coBufferNode) override;

private:
    void SendRequest(Request &&request, const std::string &funcName, bool isInit = false);
    int32_t StartRenderSink();
    bool IsMchDevice();
    int32_t CreateInputSession(const HpaeStreamInfo &streamInfo);
    int32_t DeleteInputSession(uint32_t sessionId);
    int32_t DeleteInputSessionForMove(uint32_t sessionId);
    bool isSplitProcessorType(HpaeProcessorType sceneType);
    int32_t ConnectInputSession(uint32_t sessionId);
    int32_t DisConnectInputSession(uint32_t sessionId);
    int32_t DeleteConnectInputProcessor(const std::shared_ptr<HpaeSinkInputNode> &sinkInputNode);
    void SetSessionState(uint32_t sessionId, HpaeSessionState renderState);
    void AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect = true);
    void CreateEffectAndConnect(HpaeNodeInfo &nodeInfo, bool isConnect = true);
    void MoveAllStreamToNewSink(const std::string &sinkName, const std::vector<uint32_t>& moveIds,
        MoveSessionType moveType);
    void UpdateProcessClusterConnection(uint32_t sessionId, int32_t effectMode);
    void ConnectProcessCluster(uint32_t sessionId, HpaeProcessorType sceneType);
    void DisConnectInputCluster(uint32_t sessionId, HpaeProcessorType sceneType);
    void DisConnectProcessCluster(const HpaeNodeInfo &nodeInfo, HpaeProcessorType sceneType, uint32_t sessionId);
    void CreateProcessCluster(HpaeNodeInfo &nodeInfo);
    void CreateProcessClusterInner(HpaeNodeInfo &nodeInfo, int32_t processClusterDecision);
    bool SetSessionFade(uint32_t sessionId, IOperation operation);
    void CreateDefaultProcessCluster(HpaeNodeInfo &nodeInfo);
    void CreateOutputClusterNodeInfo(HpaeNodeInfo &nodeInfo);
    void InitManager(bool isReload = false);
    void MoveStreamSync(uint32_t sessionId, const std::string &sinkName);
    void UpdateAppsUid();
    int32_t HandlePriPaPower(uint32_t sessionId);
    bool CheckIsStreamRunning();
    HpaeProcessorType GetProcessorType(uint32_t sessionId);
    HpaeProcessorType TransToProperSceneType(StreamUsage streamUsage, AudioEffectScene effectScene);
    void ReConnectNodeForCollaboration(uint32_t sessionID);
    void EnableCollaboration();
    void DisableCollaboration();
    int32_t HandleSyncId(uint32_t sessionId, int32_t syncId);
    int32_t DeleteProcessCluster(HpaeProcessorType sceneType);
    void RefreshProcessClusterByDeviceInner(const std::shared_ptr<HpaeSinkInputNode> &node);

private:
    std::unordered_map<uint32_t, HpaeRenderSessionInfo> sessionNodeMap_;
    std::unordered_map<HpaeProcessorType, std::shared_ptr<HpaeProcessCluster>> sceneClusterMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSinkInputNode>> sinkInputNodeMap_;
    std::unordered_map<HpaeProcessorType, uint32_t> toBeStoppedSceneTypeToSessionMap_;
    std::unique_ptr<IHpaeOutputCluster> outputCluster_ = nullptr;
    HpaeNoLockQueue hpaeNoLockQueue_;
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread_ = nullptr;
    std::atomic<bool> isInit_ = false;
    std::atomic<bool> isMute_ = false;
    std::atomic<bool> isSuspend_ = false;
    HpaeSinkInfo sinkInfo_;
    std::unordered_map<HpaeProcessorType, int32_t> sceneTypeToProcessClusterCountMap_;
    std::vector<int32_t> appsUid_;
    std::shared_ptr<HpaeCoBufferNode> hpaeCoBufferNode_;
    bool isCollaborationEnabled_ = false;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif