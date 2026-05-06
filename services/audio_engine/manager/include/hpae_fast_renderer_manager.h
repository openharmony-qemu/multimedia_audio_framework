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

#ifndef HPAE_FAST_RENDER_MANAGER_H
#define HPAE_FAST_RENDER_MANAGER_H

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include "hpae_signal_process_thread.h"
#include "hpae_sink_input_node.h"
#include "hpae_msg_channel.h"
#include "hpae_no_lock_queue.h"
#include "i_hpae_renderer_manager.h"
#include "hpae_fast_sink_output_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_mixer_node.h"
#include "hpae_gain_node.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeFastRendererManager : public IHpaeRendererManager {
public:
    HpaeFastRendererManager(HpaeSinkInfo &sinkInfo);
    virtual ~HpaeFastRendererManager();

    // === Core Lifecycle Methods ===
    int32_t CreateStream(const HpaeStreamInfo &streamInfo) override;
    int32_t DestroyStream(uint32_t sessionId) override;
    int32_t Init(bool isReload = false) override;
    int32_t DeInit(bool isMoveDefault = false) override;
    void Process() override;
    void HandleMsg() override;
    bool IsInit() override;
    bool IsRunning(void) override;
    bool IsMsgProcessing() override;
    bool DeactivateThread() override;

    // === Stream Control Methods ===
    int32_t Start(uint32_t sessionId) override;
    int32_t Pause(uint32_t sessionId, bool isStandby = false) override;
    int32_t Flush(uint32_t sessionId) override;
    int32_t Drain(uint32_t sessionId) override;
    int32_t Stop(uint32_t sessionId) override;
    int32_t Release(uint32_t sessionId) override;

    // === Stream Movement Methods ===
    int32_t MoveStream(uint32_t sessionId, const std::string &sinkName) override;
    int32_t MoveAllStream(const std::string &sinkName, const std::vector<uint32_t>& sessionIds,
        MoveSessionType moveType = MOVE_ALL) override;
    int32_t AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node) override;
    int32_t AddAllNodesToSink(
        const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect) override;

    // === Manager Control Methods ===
    int32_t SuspendStreamManager(bool isSuspend) override;
    int32_t SetMute(bool isMute) override;
    int32_t StopManager() override;
    void SetMuteForSwitchDevice(bool mute) override;

    // === Volume/Rate/Effect Methods ===
    int32_t SetClientVolume(uint32_t sessionId, float volume) override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    int32_t SetRate(uint32_t sessionId, int32_t rate) override;
    int32_t SetAudioEffectMode(uint32_t sessionId, int32_t effectMode) override;
    int32_t GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode) override;
    int32_t SetPrivacyType(uint32_t sessionId, int32_t privacyType) override;
    int32_t GetPrivacyType(uint32_t sessionId, int32_t &privacyType) override;
    void SetSpeed(uint32_t sessionId, float speed) override;
    size_t GetWritableSize(uint32_t sessionId) override;
    int32_t UpdateSpatializationState(
        uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled) override;
    int32_t UpdateMaxLength(uint32_t sessionId, uint32_t maxLength) override;

    // === Callback Methods ===
    int32_t RegisterWriteCallback(uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback) override;
    int32_t RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback) override;

    // === INodeCallback Overrides ===
    void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) override;
    void OnFadeDone(uint32_t sessionId) override;
    void OnRequestLatency(uint32_t sessionId, uint64_t &latency) override;
    void OnRewindAndFlush(uint64_t rewindTime, uint64_t hdiFramePosition = 0) override;
    void OnNotifyQueue() override;

    // === Query Methods ===
    std::vector<SinkInput> GetAllSinkInputsInfo() override;
    int32_t GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo) override;
    int32_t RefreshProcessClusterByDevice() override;
    HpaeSinkInfo GetSinkInfo() override;
    int32_t GetSpanSizeInFrame(uint32_t sessionId, uint32_t &spanSizeInFrame) override;
    std::string GetThreadName() override;
    int32_t DumpSinkInfo() override;
    std::string GetDeviceHDFDumpInfo() override;
    float GetMaxAmplitude() override;

    // === Reload ===
    int32_t ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload = false) override;

    // === App UID Tracking ===
    void TriggerAppsUidUpdate(uint32_t sessionId) override;

private:
    void SendRequest(Request &&request, const std::string &funcName, bool isInit = false);
    int32_t StartRenderSink();
    std::shared_ptr<HpaeSinkInputNode> CreateInputSession(const HpaeStreamInfo &streamInfo);
    int32_t CreateFastNodes(const HpaeNodeInfo &nodeInfo);
    int32_t DeleteSessionNodes(uint32_t sessionId);
    int32_t ConnectInputCluster(uint32_t sessionId);
    int32_t DisConnectInputCluster(uint32_t sessionId);
    int32_t ConnectInputSession(uint32_t sessionId);
    int32_t DisConnectInputSession(uint32_t sessionId);
    void DeleteInputSession(uint32_t sessionId);
    void MoveStreamSync(uint32_t sessionId, const std::string &sinkName);
    bool CheckIsStreamRunning();
    void AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect = true);
    void UpdateLoopbackState();
    void MoveAllStreamToNewSink(const std::string &sinkName, const std::vector<uint32_t> &moveIds,
        MoveSessionType moveType);
    int32_t InitSinkInner(bool isReload = false);
    void UpdateAppsUid();
    void RemoveNodeFromMap(uint32_t sessionId);
    bool SetSessionFade(uint32_t sessionId, IOperation operation);
    void StopOutputNode();
    IAudioSinkAttr CreateSinkAttr();
    void NotifyStreamChangeToSink(StreamChangeType change, uint32_t sessionId, RendererState state,
        uint32_t appUid = INVALID_UID);
    void TriggerStreamState(uint32_t sessionId, const std::shared_ptr<HpaeSinkInputNode> &inputNode);

    std::unordered_map<uint32_t, std::shared_ptr<HpaeSinkInputNode>> sinkInputNodeMap_;

    // Per-stream nodes
    std::unordered_map<uint32_t, std::shared_ptr<HpaeGainNode>> gainNodeMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeAudioFormatConverterNode>> converterNodeMap_;

    // Mixer node (shared across all streams)
    std::shared_ptr<HpaeMixerNode> mixerNode_ = nullptr;

    // Fast sink output node (mmap-based)
    std::unique_ptr<HpaeFastSinkOutputNode> sinkOutputNode_ = nullptr;

    // Thread and message queue
    HpaeNoLockQueue hpaeNoLockQueue_;
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread_ = nullptr;

    // State flags
    std::atomic<bool> isInit_ = false;
    std::atomic<bool> isMute_ = false;
    std::atomic<bool> isSuspend_ = false;

    // Configuration
    HpaeSinkInfo sinkInfo_;

    // App UID tracking
    std::vector<int32_t> appsUid_;

    // Loopback tracking
    std::unordered_set<uint32_t> loopbackSessionIds_;
    std::atomic<bool> isExistLoopback_{false};

    int64_t noneStreamTime_ = 0; // if no stream, 3s time out to stop rendersink
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif // HPAE_FAST_RENDER_MANAGER_H
