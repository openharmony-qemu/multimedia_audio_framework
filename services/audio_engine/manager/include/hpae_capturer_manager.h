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
#ifndef HPAE_CAPTURER_MANAGER_H
#define HPAE_CAPTURER_MANAGER_H
#include <unordered_map>
#include <memory>
#include <atomic>
#include <string>
#include <mutex>
#include <shared_mutex>
#include "audio_effect.h"
#include "hpae_signal_process_thread.h"
#include "hpae_source_input_node.h"
#include "hpae_source_input_cluster.h"
#include "hpae_source_output_node.h"
#include "hpae_source_process_cluster.h"
#include "hpae_no_lock_queue.h"
#include "i_hpae_capturer_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeCapturerManager : public IHpaeCapturerManager {
public:
    HpaeCapturerManager(HpaeSourceInfo &sourceInfo);
    virtual ~HpaeCapturerManager();
    int32_t CreateStream(const HpaeStreamInfo& streamInfo) override;
    int32_t DestroyStream(uint32_t sessionId) override;

    int32_t Start(uint32_t sessionId) override;
    int32_t Pause(uint32_t sessionId) override;
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

    int32_t RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback) override;
    int32_t GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo) override;
    HpaeSourceInfo GetSourceInfo() override;
    std::vector<SourceOutput> GetAllSourceOutputsInfo() override;

    void OnNodeStatusUpdate(uint32_t sessionId, IOperation operation) override;
    void OnNotifyQueue() override;
    void OnRequestLatency(uint32_t sessionId, uint64_t &latency) override;

    int32_t AddNodeToSource(const HpaeCaptureMoveInfo &moveInfo) override;
    int32_t AddAllNodesToSource(const std::vector<HpaeCaptureMoveInfo> &moveInfos, bool isConnect) override;
    std::string GetThreadName() override;
    void SetCaptureId(uint32_t captureId);
    int32_t ReloadCaptureManager(const HpaeSourceInfo &sourceInfo) override;
    int32_t DumpSourceInfo() override;
    std::string GetDeviceHDFDumpInfo() override;
private:
    void SendRequest(Request &&request, const std::string &funcName, bool isInit = false);
    int32_t CreateOutputSession(const HpaeStreamInfo &streamInfo);
    int32_t DeleteOutputSession(uint32_t sessionId);
    void ConnectProcessClusterWithEc(HpaeProcessorType &sceneType);
    void ConnectProcessClusterWithMicRef(HpaeProcessorType &sceneType);
    int32_t ConnectOutputSession(uint32_t sessionId);
    int32_t DisConnectOutputSession(uint32_t sessionId);
    void DisConnectSceneClusterFromSourceInputCluster(HpaeProcessorType &sceneType);
    void SetSessionState(uint32_t sessionId, HpaeSessionState capturerState);
    int32_t PrepareCapturerEc(HpaeNodeInfo &ecNodeInfo);
    int32_t PrepareCapturerMicRef(HpaeNodeInfo &micRefNodeInfo);
    int32_t InitCapturer();
    void AddSingleNodeToSource(const HpaeCaptureMoveInfo &moveInfo, bool isConnect = true);
    void MoveAllStreamToNewSource(const std::string &sourceName,
        const std::vector<uint32_t>& moveIds, MoveSessionType moveType = MOVE_ALL);
    int32_t CaptureEffectCreate(const HpaeProcessorType &sceneType, const AudioEnhanceScene &enhanceScene);
    int32_t CaptureEffectRelease(const HpaeProcessorType &sceneType);
    int32_t InitCapturerManager();
    void CreateSourceAttr(IAudioSourceAttr &attr);
    int32_t CapturerSourceStart();
    int32_t CapturerSourceStop();
    void CapturerSourceStopForRemote();
    void CheckIfAnyStreamRunning();
    void UpdateAppsUidAndSessionId();
    bool CheckEcCondition(const HpaeProcessorType &sceneType, HpaeNodeInfo &ecNodeInfo,
        HpaeSourceInputNodeType &ecNodeType);
    bool CheckMicRefCondition(const HpaeProcessorType &sceneType, HpaeNodeInfo &micRefNodeInfo);
private:
    HpaeNoLockQueue hpaeNoLockQueue_;
    std::unique_ptr<HpaeSignalProcessThread> hpaeSignalProcessThread_ = nullptr;
    std::unordered_map<uint32_t, HpaeCapturerSessionInfo> sessionNodeMap_;
    std::unordered_map<HpaeProcessorType, std::shared_ptr<HpaeSourceProcessCluster>> sceneClusterMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeSourceOutputNode>> sourceOutputNodeMap_;
    std::unordered_map<HpaeSourceInputNodeType, std::shared_ptr<HpaeSourceInputCluster>> sourceInputClusterMap_;

    HpaeSourceInputNodeType mainMicType_;
    std::atomic<bool> isInit_ = false;
    std::atomic<bool> isMute_ = false;
    HpaeSourceInfo sourceInfo_;
    uint32_t captureId_ = 0;
    uint32_t renderId_ = 0;

    std::vector<int32_t> appsUid_;
    std::vector<int32_t> sessionsId_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif