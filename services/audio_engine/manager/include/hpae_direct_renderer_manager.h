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
#include "hpae_volume_sync_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeDirectRendererManager : public IHpaeRendererManager {
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
    int32_t GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo) override;
    int32_t RefreshProcessClusterByDevice() override;
    HpaeSinkInfo GetSinkInfo() override;

    int32_t AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node) override;
    int32_t AddAllNodesToSink(
        const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect) override;

    void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) override;
    void OnFadeDone(uint32_t sessionId) override;
    void OnRequestLatency(uint32_t sessionId, uint64_t &latency) override;
    void OnNotifyQueue() override;
    std::string GetThreadName() override;
    int32_t DumpSinkInfo() override;
    int32_t ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload = false) override;
    std::string GetDeviceHDFDumpInfo() override;
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain) override;
    void TriggerAppsUidUpdate(uint32_t sessionId) override;

private:
    void SendRequest(Request &&request, const std::string &funcName, bool isInit = false);
    int32_t StartRenderSink();
    std::shared_ptr<HpaeSinkInputNode> CreateInputSession(const HpaeStreamInfo &streamInfo);
    int32_t CreateDirectNodes();
    int32_t DestroyDirectNodes();
    int32_t ConnectInputSession();
    int32_t DisConnectInputSession(bool isStandby = false);
    void DeleteInputSession();
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
    int32_t RecreateSinkOutputNodeIfNeeded();
    void CreateProcessNodes(uint32_t sessionId);
    bool SetSessionFade(uint32_t sessionId, IOperation operation);
    AudioSamplingRate GetDirectSampleRate(AudioSamplingRate sampleRate, bool isVoip);
    AudioSampleFormat GetDirectFormat(AudioSampleFormat format, bool isVoip);

private:
    std::shared_ptr<HpaeSinkInputNode> curNode_ = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSinkInputNode>> sinkInputNodeMap_;
    std::shared_ptr<HpaeAudioFormatConverterNode> converterForGain_ = nullptr;
    std::shared_ptr<HpaeAudioFormatConverterNode> converterForOutput_ = nullptr;
    std::shared_ptr<HpaeGainNode> gainNode_ = nullptr;
    std::shared_ptr<HpaeVolumeSyncNode> volumeSyncNode_ = nullptr;
    std::shared_ptr<HpaeLimiterNode> limiterNode_ = nullptr;
    std::unique_ptr<HpaeDirectSinkOutputNode> sinkOutputNode_ = nullptr;
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
#endif // HPAE_DIRECT_RENDER_MANAGER_H
