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

#ifndef HPAE_INNER_CAPTURER_MANAGER_H
#define HPAE_INNER_CAPTURER_MANAGER_H
#include <unordered_map>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <shared_mutex>
#include "i_renderer_stream.h"
#include "hpae_signal_process_thread.h"
#include "hpae_sink_input_node.h"
#include "hpae_inner_cap_sink_node.h"
#include "hpae_source_output_node.h"
#include "hpae_source_process_cluster.h"
#include "hpae_msg_channel.h"
#include "hpae_no_lock_queue.h"
#include "i_hpae_renderer_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeInnerCapturerManager : public IHpaeRendererManager {
public:
    HpaeInnerCapturerManager(HpaeSinkInfo &sinkInfo);
    ~HpaeInnerCapturerManager();
    int32_t CreateStream(const HpaeStreamInfo &streamInfo) override;
    int32_t DestroyStream(uint32_t sessionId) override;

    int32_t Start(uint32_t sessionId) override;
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

    void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) override;
    void OnFadeDone(uint32_t sessionId) override;

    int32_t AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node) override;
    int32_t AddAllNodesToSink(
        const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect) override;

    int32_t RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback) override;
    int32_t GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo) override;
    std::vector<SourceOutput> GetAllSourceOutputsInfo() override;
    std::string GetThreadName() override;
    int32_t DumpSinkInfo() override;
    int32_t ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload = false) override;
    int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state) override;
    std::string GetDeviceHDFDumpInfo() override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    void OnDisConnectProcessCluster(HpaeProcessorType sceneType) override;
    bool SetSessionFade(uint32_t sessionId, IOperation operation);
    void TriggerStreamState(uint32_t sessionId, const std::shared_ptr<HpaeSinkInputNode> &inputNode);

private:
    void TransStreamInfoToNodeInfoInner(const HpaeStreamInfo &streamInfo, HpaeNodeInfo &nodeInfo);
    int32_t CreateRendererInputSessionInner(const HpaeStreamInfo &streamInfo);
    int32_t CreateCapturerInputSessionInner(const HpaeStreamInfo &streamInfo);
    int32_t DeleteRendererInputSessionInner(uint32_t sessionId);
    void DeleteRendererInputNodeSession(const std::shared_ptr<HpaeSinkInputNode> &sinkInputNode);
    int32_t DeleteCapturerInputSessionInner(uint32_t sessionId);
    int32_t ConnectRendererInputSessionInner(uint32_t sessionId);
    int32_t ConnectCapturerOutputSessionInner(uint32_t sessionId);
    int32_t DisConnectRendererInputSessionInner(uint32_t sessionId);
    int32_t DisConnectCapturerInputSessionInner(uint32_t sessionId);
    void SetSessionStateForRenderer(uint32_t sessionId, HpaeSessionState renderState);
    void SetSessionStateForCapturer(uint32_t sessionId, HpaeSessionState capturerState);
    void SendRequestInner(Request &&request, const std::string &funcName, bool isInit = false);
    uint32_t GetSinkInputNodeIdInner();
    void AddSingleNodeToSinkInner(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect = true);
    void MoveAllStreamToNewSinkInner(const std::string &sinkName, const std::vector<uint32_t> &moveIds,
        MoveSessionType moveType);
    int32_t InitSinkInner(bool isReload = false);
    uint32_t sinkInputNodeCounter_ = 0;
    int32_t sceneTypeToProcessClusterCount_ = 0;
    std::atomic<bool> isInit_ = false;
    HpaeSinkInfo sinkInfo_;
    HpaeNoLockQueue hpaeNoLockQueue_;
    std::shared_ptr<HpaeInnerCapSinkNode> hpaeInnerCapSinkNode_ = nullptr;
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread_ = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSinkInputNode>> sinkInputNodeMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSourceOutputNode>> sourceOutputNodeMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeAudioFormatConverterNode>> capturerAudioFormatConverterNodeMap_;
    std::unordered_map<HpaeProcessorType, std::shared_ptr<HpaeSourceProcessCluster>> capturerSceneClusterMap_;
    std::unordered_map<HpaeProcessorType, std::shared_ptr<HpaeProcessCluster>> rendererSceneClusterMap_;
    std::unordered_map<uint32_t, HpaeCapturerSessionInfo> capturerSessionNodeMap_;
    std::unordered_map<uint32_t, HpaeRenderSessionInfo> rendererSessionNodeMap_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif