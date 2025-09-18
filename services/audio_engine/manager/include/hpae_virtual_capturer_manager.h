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
#ifndef HPAE_VIRTUAL_CAPTURER_MANAGER_H
#define HPAE_VIRTUAL_CAPTURER_MANAGER_H

#include "hpae_capture_move_info.h"
#include "i_hpae_capturer_manager.h"
#include "hpae_info.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeVirtualCapturerManager : public IHpaeCapturerManager {
public:
    HpaeVirtualCapturerManager();
    virtual ~HpaeVirtualCapturerManager();
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
    int32_t ReloadCaptureManager(const HpaeSourceInfo &sourceInfo, bool isReload = false) override;
    int32_t DumpSourceInfo() override;
    std::string GetDeviceHDFDumpInfo() override;

    int32_t AddCaptureInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode,
        const SourceType &sourceType) override;
    int32_t RemoveCaptureInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode,
        const SourceType &sourceType) override;

private:
    void SetSessionState(HpaeCaptureMoveInfo &streamInfo, HpaeSessionState capturerState);

private:
    std::mutex captureMutex_;
    std::unordered_map<uint32_t, HpaeCaptureMoveInfo> captureStream_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif