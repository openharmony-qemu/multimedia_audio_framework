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

#ifndef HPAE_OFFLOAD_RENDER_MANAGER_H
#define HPAE_OFFLOAD_RENDER_MANAGER_H
#include <unordered_map>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <shared_mutex>
#include "hpae_signal_process_thread.h"
#include "hpae_sink_input_node.h"
#include "hpae_process_cluster.h"
#include "hpae_offload_sinkoutput_node.h"
#include "hpae_msg_channel.h"
#include "hpae_no_lock_queue.h"
#include "i_hpae_renderer_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeOffloadRendererManager : public IHpaeRendererManager, public INodeFormatInfoCallback {
public:
    HpaeOffloadRendererManager(HpaeSinkInfo& sinkInfo);
    virtual ~HpaeOffloadRendererManager();
    int32_t CreateStream(const HpaeStreamInfo &streamInfo) override;
    int32_t DestroyStream(uint32_t sessionId) override;
 
    int32_t Start(uint32_t sessionId) override;
    int32_t Pause(uint32_t sessionId) override;
    int32_t Flush(uint32_t sessionId) override;
    int32_t Drain(uint32_t sessionId) override;
    int32_t Stop(uint32_t sessionId) override;
    int32_t Release(uint32_t sessionId) override;
    int32_t MoveStream(uint32_t sessionId, const std::string& sinkName) override;
    int32_t MoveAllStream(const std::string& sinkName, const std::vector<uint32_t>& sessionIds,
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
    int32_t RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback) override;

    int32_t SetOffloadPolicy(uint32_t sessionId, int32_t state) override;
    size_t GetWritableSize(uint32_t sessionId) override;
    int32_t UpdateSpatializationState(uint32_t sessionId, bool spatializationEnabled,
        bool headTrackingEnabled) override;
    int32_t UpdateMaxLength(uint32_t sessionId, uint32_t maxLength) override;
    int32_t SetOffloadRenderCallbackType(uint32_t sessionId, int32_t type) override;
    void SetSpeed(uint32_t sessionId, float speed) override;
    std::vector<SinkInput> GetAllSinkInputsInfo() override;
    int32_t GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo) override;
    int32_t RefreshProcessClusterByDevice() override;
    HpaeSinkInfo GetSinkInfo() override;

    int32_t AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node) override;
    int32_t AddAllNodesToSink(
        const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect) override;

    void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) override;
    void OnRequestLatency(uint32_t sessionId, uint64_t &latency) override;
    void OnRewindAndFlush(uint64_t rewindTime, uint64_t hdiFramePosition = 0) override;
    void OnNotifyQueue() override;
    std::string GetThreadName() override;
    int32_t DumpSinkInfo() override;
    int32_t ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload = false) override;
    std::string GetDeviceHDFDumpInfo() override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    int32_t GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat) override;
private:
    void SendRequest(Request &&request, const std::string &funcName, bool isInit = false);
    int32_t StartRenderSink();
    int32_t CreateInputSession(const HpaeStreamInfo &streamInfo);
    int32_t ConnectInputSession();
    int32_t DisConnectInputSession();
    void DeleteInputSession();
    void AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect = true);
    void MoveAllStreamToNewSink(const std::string &sinkName, const std::vector<uint32_t> &moveIds,
        MoveSessionType moveType);
    void InitSinkInner(bool isReload = false);
    void UpdateAppsUid();

    HpaeRenderSessionInfo sessionInfo_;
    std::shared_ptr<HpaeSinkInputNode> sinkInputNode_ = nullptr;
    std::shared_ptr<HpaeAudioFormatConverterNode> converterForLoudness_ = nullptr;
    std::shared_ptr<HpaeAudioFormatConverterNode> converterForOutput_ = nullptr;
    std::shared_ptr<HpaeLoudnessGainNode> loudnessGainNode_ = nullptr;
    std::unique_ptr<HpaeOffloadSinkOutputNode> sinkOutputNode_ = nullptr;
    HpaeNoLockQueue hpaeNoLockQueue_;
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
#endif // HPAE_OFFLOAD_RENDER_MANAGER_H