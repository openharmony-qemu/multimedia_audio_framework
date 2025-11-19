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

#ifndef LOG_TAG
#define LOG_TAG "HpaeVirtualCapturerManager"
#endif

#include "hpae_virtual_capturer_manager.h"
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_engine_log.h"
#include "audio_log.h"
#include "hpae_node_common.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeVirtualCapturerManager::HpaeVirtualCapturerManager() {}

HpaeVirtualCapturerManager::~HpaeVirtualCapturerManager() {}

int32_t HpaeVirtualCapturerManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    AUDIO_INFO_LOG("Create output node:%{public}d", streamInfo.sessionId);
    HpaeNodeInfo nodeInfo;
    ConfigNodeInfo(nodeInfo, streamInfo);
    HpaeProcessorType sceneType = TransSourceTypeToSceneType(streamInfo.sourceType);
    nodeInfo.sceneType = sceneType;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.statusCallback = weak_from_this();

    AudioEnhanceScene enhanceScene = TransProcessType2EnhanceScene(sceneType);
    nodeInfo.effectInfo.enhanceScene = enhanceScene;
    auto sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    sourceOutputNode->SetAppUid(streamInfo.uid);
    HpaeCapturerSessionInfo sessionInfo;
    sessionInfo.sceneType = sceneType;
    HpaeCaptureMoveInfo moveInfo = {streamInfo.sessionId, sourceOutputNode, sessionInfo};
    captureStream_.emplace(streamInfo.sessionId, moveInfo);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::DestroyStream(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    captureStream_.erase(sessionId);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::Start(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeVirtualCapturerManager::Start");
    std::lock_guard<std::mutex> lock(captureMutex_);
    CHECK_AND_RETURN_RET_LOG(captureStream_.find(sessionId) != captureStream_.end(), SUCCESS,
        "sessionId %{public}u is not exist", sessionId);
    SetSessionState(captureStream_[sessionId], HPAE_SESSION_RUNNING);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::Pause(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeVirtualCapturerManager::Pause");
    std::lock_guard<std::mutex> lock(captureMutex_);
    CHECK_AND_RETURN_RET_LOG(captureStream_.find(sessionId) != captureStream_.end(), SUCCESS,
        "sessionId %{public}u is not exist", sessionId);
    SetSessionState(captureStream_[sessionId], HPAE_SESSION_PAUSED);
    TriggerSyncCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
        HPAE_SESSION_PAUSED, OPERATION_PAUSED);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::Flush(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeVirtualCapturerManager::Flush");
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::Drain(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeVirtualCapturerManager::Drain");
    std::lock_guard<std::mutex> lock(captureMutex_);
    CHECK_AND_RETURN_RET_LOG(captureStream_.find(sessionId) != captureStream_.end(), SUCCESS,
        "sessionId %{public}u is not exist", sessionId);
    auto captureInfo = captureStream_[sessionId];
    TriggerSyncCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
        captureInfo.sessionInfo.state, OPERATION_DRAINED);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::Stop(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeVirtualCapturerManager::Stop");
    std::lock_guard<std::mutex> lock(captureMutex_);
    CHECK_AND_RETURN_RET_LOG(captureStream_.find(sessionId) != captureStream_.end(), SUCCESS,
        "sessionId %{public}u is not exist", sessionId);
    SetSessionState(captureStream_[sessionId], HPAE_SESSION_STOPPED);
    TriggerSyncCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
        HPAE_SESSION_STOPPED, OPERATION_STOPPED);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::Release(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeVirtualCapturerManager::Release");
    return DestroyStream(sessionId);
}

int32_t HpaeVirtualCapturerManager::MoveStream(uint32_t sessionId, const std::string &sourceName)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    CHECK_AND_RETURN_RET_LOG(captureStream_.find(sessionId) != captureStream_.end(), SUCCESS,
        "sessionId %{public}u is not exist", sessionId);
    auto captureInfo = captureStream_[sessionId];
    std::string name = sourceName;
    AUDIO_INFO_LOG("[StartMove] session: %{public}u, source [virtual] ---> [%{public}s]",
        sessionId, sourceName.c_str());
    TriggerSyncCallback(MOVE_SOURCE_OUTPUT, captureInfo, name);
    captureStream_.erase(sessionId);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::MoveAllStream(const std::string &sourceName, const std::vector<uint32_t> &moveIds,
    MoveSessionType moveType)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    std::string name = sourceName;
    std::vector<HpaeCaptureMoveInfo> moveInfos;
    std::string idStr;
    for (auto itr = captureStream_.begin(); itr != captureStream_.end();) {
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), itr->first) != moveIds.end()) {
            moveInfos.emplace_back(itr->second);
            idStr.append("[").append(std::to_string(itr->first)).append("],");
            itr = captureStream_.erase(itr);
        } else {
            ++itr;
        }
    }
    AUDIO_INFO_LOG("[StartMove] session: %{public}s to source name:%{public}s, move type:%{public}d",
        idStr.c_str(), name.c_str(), moveType);
    TriggerSyncCallback(MOVE_ALL_SOURCE_OUTPUT, moveInfos, name);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::SetStreamMute(uint32_t sessionId, bool isMute)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    CHECK_AND_RETURN_RET_LOG(captureStream_.find(sessionId) != captureStream_.end(), SUCCESS,
        "sessionId %{public}u is not exist", sessionId);
    auto captureInfo = captureStream_[sessionId];
    CHECK_AND_RETURN_RET_LOG(captureInfo.sourceOutputNode, SUCCESS, "captureInfo.sourceOutputNode is nullptr");
    captureStream_[sessionId].sourceOutputNode->SetMute(isMute);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::SetMute(bool isMute)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

void HpaeVirtualCapturerManager::Process()
{
    AUDIO_ERR_LOG("Unsupported operation");
}

void HpaeVirtualCapturerManager::HandleMsg()
{
    AUDIO_ERR_LOG("Unsupported operation");
}

int32_t HpaeVirtualCapturerManager::Init(bool isReload)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::DeInit(bool isMoveDefault)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

bool HpaeVirtualCapturerManager::IsInit()
{
    return true;
}

bool HpaeVirtualCapturerManager::IsRunning(void)
{
    return true;
}

bool HpaeVirtualCapturerManager::IsMsgProcessing()
{
    return true;
}

bool HpaeVirtualCapturerManager::DeactivateThread()
{
    return true;
}

int32_t HpaeVirtualCapturerManager::RegisterReadCallback(uint32_t sessionId,
    const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    CHECK_AND_RETURN_RET_LOG(captureStream_.find(sessionId) != captureStream_.end(), SUCCESS,
        "sessionId %{public}u is not exist", sessionId);
    auto captureInfo = captureStream_[sessionId];
    CHECK_AND_RETURN_RET_LOG(captureInfo.sourceOutputNode, SUCCESS, "captureInfo.sourceOutputNode is nullptr");
    sourceOutputInfo.nodeInfo = captureInfo.sourceOutputNode->GetNodeInfo();
    sourceOutputInfo.capturerSessionInfo = captureInfo.sessionInfo;
    return SUCCESS;
}

HpaeSourceInfo HpaeVirtualCapturerManager::GetSourceInfo()
{
    HpaeSourceInfo sourceInfo;
    return sourceInfo;
}

std::vector<SourceOutput> HpaeVirtualCapturerManager::GetAllSourceOutputsInfo()
{
    return {};
}

void HpaeVirtualCapturerManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    CHECK_AND_RETURN_LOG(captureStream_.find(sessionId) != captureStream_.end(),
        "sessionId %{public}u is not exist", sessionId);
    auto captureInfo = captureStream_[sessionId];
    TriggerSyncCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
        captureInfo.sessionInfo.state, operation);
}

void HpaeVirtualCapturerManager::OnNotifyQueue()
{
    AUDIO_ERR_LOG("Unsupported operation");
}

void HpaeVirtualCapturerManager::OnRequestLatency(uint32_t sessionId, uint64_t &latency)
{
    latency = 0;
}

int32_t HpaeVirtualCapturerManager::AddNodeToSource(const HpaeCaptureMoveInfo &moveInfo)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    AUDIO_INFO_LOG("[FinishMove] session: %{public}u to source:[virtual]", moveInfo.sessionId);
    captureStream_.emplace(moveInfo.sessionId, moveInfo);
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::AddAllNodesToSource(const std::vector<HpaeCaptureMoveInfo> &moveInfos,
    bool isConnect)
{
    std::lock_guard<std::mutex> lock(captureMutex_);
    for (auto moveInfo : moveInfos) {
        AUDIO_INFO_LOG("[FinishMove] session: %{public}u to source:[virtual]", moveInfo.sessionId);
        captureStream_.emplace(moveInfo.sessionId, moveInfo);
    }
    return SUCCESS;
}

std::string HpaeVirtualCapturerManager::GetThreadName()
{
    return "Virtual_capture";
}

int32_t HpaeVirtualCapturerManager::ReloadCaptureManager(const HpaeSourceInfo &sourceInfo, bool isReload)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::DumpSourceInfo()
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

std::string HpaeVirtualCapturerManager::GetDeviceHDFDumpInfo()
{
    AUDIO_ERR_LOG("Unsupported operation");
    return "";
}

void HpaeVirtualCapturerManager::SetSessionState(HpaeCaptureMoveInfo &streamInfo, HpaeSessionState capturerState)
{
    streamInfo.sessionInfo.state = capturerState;
    CHECK_AND_RETURN_LOG(streamInfo.sourceOutputNode, "streamInfo.sourceOutputNode is nullptr");
    streamInfo.sourceOutputNode->SetState(capturerState);
}

int32_t HpaeVirtualCapturerManager::AddCaptureInjector(
    const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode, const SourceType &sourceType)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeVirtualCapturerManager::RemoveCaptureInjector(
    const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode, const SourceType &sourceType)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS