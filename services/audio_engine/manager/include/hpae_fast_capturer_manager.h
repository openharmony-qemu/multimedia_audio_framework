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
#ifndef HPAE_FAST_CAPTURER_MANAGER_H
#define HPAE_FAST_CAPTURER_MANAGER_H

#include <atomic>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "hpae_audio_format_converter_node.h"
#include "hpae_fast_source_input_node.h"
#include "hpae_no_lock_queue.h"
#include "hpae_signal_process_thread.h"
#include "hpae_source_output_node.h"
#include "hpae_mixer_node.h"
#include "i_hpae_capturer_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeFastCapturerManager : public IHpaeCapturerManager {
public:
    explicit HpaeFastCapturerManager(HpaeSourceInfo &sourceInfo);
    ~HpaeFastCapturerManager() override;

    int32_t CreateStream(const HpaeStreamInfo& streamInfo) override;
    int32_t DestroyStream(uint32_t sessionId) override;
    int32_t Start(uint32_t sessionId) override;
    int32_t Pause(uint32_t sessionId, bool isStandby = false) override;
    int32_t Flush(uint32_t sessionId) override;
    int32_t Drain(uint32_t sessionId) override;
    int32_t Stop(uint32_t sessionId) override;
    int32_t Release(uint32_t sessionId) override;
    int32_t SetStreamMute(uint32_t sessionId, bool isMute) override;
    int32_t MoveStream(uint32_t sessionId, const std::string& sourceName) override;
    int32_t MoveAllStream(const std::string& sourceName, const std::vector<uint32_t>& sessionIds,
        MoveSessionType moveType = MOVE_ALL) override;
    int32_t SetMute(bool isMute) override;
    void Process() override;
    void HandleMsg() override;
    int32_t Init(bool isReload = false) override;
    int32_t DeInit(bool isMoveDefault = false) override;
    bool IsInit() override;
    bool IsRunning(void) override;
    bool IsMsgProcessing() override;
    bool DeactivateThread() override;
    int32_t StopManager() override;

    int32_t RegisterReadCallback(uint32_t sessionId,
        const std::weak_ptr<ICapturerStreamCallback> &callback) override;
    int32_t GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo) override;
    HpaeSourceInfo GetSourceInfo() override;
    std::vector<SourceOutput> GetAllSourceOutputsInfo() override;

    void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) override;
    void OnNotifyQueue() override;
    void OnRequestLatency(uint32_t sessionId, uint64_t &latency) override;

    int32_t AddNodeToSource(const HpaeCaptureMoveInfo &moveInfo) override;
    int32_t AddAllNodesToSource(const std::vector<HpaeCaptureMoveInfo> &moveInfos, bool isConnect) override;
    std::string GetThreadName() override;
    int32_t ReloadCaptureManager(const HpaeSourceInfo &sourceInfo, bool isReload = false) override;
    int32_t DumpSourceInfo() override;
    std::string GetDeviceHDFDumpInfo() override;

    int32_t AddCaptureInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode,
        const SourceType &sourceType) override;
    int32_t RemoveCaptureInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode,
        const SourceType &sourceType) override;
    void TriggerAppsUidUpdate(uint32_t sessionId) override;
    int32_t SetAppsEnhanceMuteState(const uint32_t &sessionId, bool isMute) override;

private:
    void SendRequest(Request &&request, const std::string &funcName, bool isInit = false);
    int32_t InitCapturerManager();
    void CreateSourceAttr(IAudioSourceAttr &attr);
    int32_t CreateOutputSession(const HpaeStreamInfo &streamInfo);
    int32_t CreateSessionNodes(const HpaeStreamInfo &streamInfo);
    int32_t DeleteSessionNodes(uint32_t sessionId);
    int32_t ConnectOutputSession(uint32_t sessionId);
    int32_t DisConnectOutputSession(uint32_t sessionId);
    int32_t DeleteOutputSession(uint32_t sessionId);
    void AddSingleNodeToSource(const HpaeCaptureMoveInfo &moveInfo, bool isConnect = true);
    void MoveAllStreamToNewSource(const std::string &sourceName,
        const std::vector<uint32_t>& moveIds, MoveSessionType moveType = MOVE_ALL);
    void CheckIfAnyStreamRunning();
    int32_t CapturerSourceStart();
    int32_t CapturerSourceStop();
    void UpdateAppsUidAndSessionId();
    void NotifyStreamChangeToSource(StreamChangeType change, uint32_t sessionId, CapturerState state,
        uint32_t appUid = INVALID_UID, bool mute = false);
    void StopOutputNode();

    HpaeNoLockQueue hpaeNoLockQueue_;
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread_ = nullptr;
    std::unordered_map<uint32_t, HpaeCapturerSessionInfo> sessionNodeMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSourceOutputNode>> sourceOutputNodeMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeAudioFormatConverterNode>> converterNodeMap_;
    std::unordered_map<std::shared_ptr<OutputNode<HpaePcmBuffer*>>,
        std::shared_ptr<HpaeAudioFormatConverterNode>> injectorFmtConverterNodeMap_;
    std::shared_ptr<HpaeFastSourceInputNode> sourceInputNode_ = nullptr;
    std::shared_ptr<HpaeMixerNode> mixerNode_ = nullptr;

    std::atomic<bool> isInit_ = false;
    std::atomic<bool> isMute_ = false;
    HpaeSourceInfo sourceInfo_;
    uint32_t captureId_ = 0;
    std::vector<int32_t> appsUid_;
    std::vector<int32_t> sessionsId_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif
