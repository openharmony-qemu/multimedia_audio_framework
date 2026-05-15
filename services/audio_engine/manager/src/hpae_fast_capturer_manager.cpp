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
#ifndef LOG_TAG
#define LOG_TAG "HpaeFastCapturerManager"
#endif

#include "hpae_fast_capturer_manager.h"

#include <algorithm>

#include "audio_engine_log.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "hpae_message_queue_monitor.h"
#include "hpae_node_common.h"
#include "hpae_stream_move_monitor.h"
#include "safe_map.h"
#include "audio_stream_enum.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr size_t FAST_CAPTURE_REQUEST_COUNT = 5000;
}

HpaeFastCapturerManager::HpaeFastCapturerManager(HpaeSourceInfo &sourceInfo)
    : hpaeNoLockQueue_(FAST_CAPTURE_REQUEST_COUNT), sourceInfo_(sourceInfo)
{
}

HpaeFastCapturerManager::~HpaeFastCapturerManager()
{
    if (isInit_.load()) {
        DeInit();
    }
}

void HpaeFastCapturerManager::SendRequest(Request &&request, const std::string &funcName, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_INFO_LOG("not init, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_CAPTURE_MANAGER_TYPE, funcName,
            "HpaeFastCapturerManager not init");
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ is nullptr");
    hpaeSignalProcessThread_->Notify();
}

int32_t HpaeFastCapturerManager::CreateSessionNodes(const HpaeStreamInfo &streamInfo)
{
    HpaeNodeInfo nodeInfo;
    ConfigNodeInfo(nodeInfo, streamInfo);
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sourceInfo_.deviceClass;
    nodeInfo.deviceNetId = sourceInfo_.deviceNetId;
    auto sourceOutputNode = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    sourceOutputNode->SetAppUid(streamInfo.uid);
    sourceOutputNodeMap_[streamInfo.sessionId] = sourceOutputNode;

    HpaeNodeInfo sourceNodeInfo = sourceInputNode_->GetNodeInfo();
    auto converter = std::make_shared<HpaeAudioFormatConverterNode>(sourceNodeInfo, nodeInfo);
    converter->SetSourceNode(true);
    converterNodeMap_[streamInfo.sessionId] = converter;
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::CreateOutputSession(const HpaeStreamInfo &streamInfo)
{
    AUDIO_INFO_LOG("CreateStream sessionId %{public}u sourceName %{public}s, channel:%{public}u, rate:%{public}u",
        streamInfo.sessionId, sourceInfo_.sourceName.c_str(), streamInfo.channels, streamInfo.samplingRate);
    int32_t ret = CreateSessionNodes(streamInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreateSessionNodes failed");
    sessionNodeMap_[streamInfo.sessionId].sceneType = TransSourceTypeToSceneType(streamInfo.sourceType);
    sessionNodeMap_[streamInfo.sessionId].isMoveAble = streamInfo.isMoveAble;
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::DeleteSessionNodes(uint32_t sessionId)
{
    converterNodeMap_.erase(sessionId);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::ConnectOutputSession(uint32_t sessionId)
{
    auto sourceOutputNode = SafeGetMap(sourceOutputNodeMap_, sessionId);
    auto converter = SafeGetMap(converterNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(sourceOutputNode != nullptr && converter != nullptr && mixerNode_ != nullptr, ERROR,
        "node not found");
    converter->Connect(mixerNode_);
    sourceOutputNode->Connect(converter);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::DisConnectOutputSession(uint32_t sessionId)
{
    auto sourceOutputNode = SafeGetMap(sourceOutputNodeMap_, sessionId);
    auto converter = SafeGetMap(converterNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(sourceOutputNode != nullptr && converter != nullptr && mixerNode_ != nullptr, ERROR,
        "node not found");
    sourceOutputNode->DisConnect(converter);
    converter->DisConnect(mixerNode_);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::DeleteOutputSession(uint32_t sessionId)
{
    auto sourceOutputNode = SafeGetMap(sourceOutputNodeMap_, sessionId);
    if (!sourceOutputNode) {
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_REMOVE, sessionId, CAPTURER_INVALID);
        sourceOutputNodeMap_.erase(sessionId);
        sessionNodeMap_.erase(sessionId);
        converterNodeMap_.erase(sessionId);
        return SUCCESS;
    }
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(false, sourceOutputNode->GetNodeInfo());
#endif
    if (sourceOutputNode->GetState() == HPAE_SESSION_RUNNING) {
        DisConnectOutputSession(sessionId);
    }
    if (sourceInputNode_ != nullptr && sourceInputNode_->GetOutputPortNum() == 0) {
        CapturerSourceStop();
    }
    DeleteSessionNodes(sessionId);
    HpaeSessionState outputState = sourceOutputNode->GetState();
    CapturerState state = outputState == HPAE_SESSION_RELEASED ? CAPTURER_INVALID : CAPTURER_RELEASED;
    NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_REMOVE, sessionId, state);
    sourceOutputNodeMap_.erase(sessionId);
    sessionNodeMap_.erase(sessionId);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::CapturerSourceStart()
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_ != nullptr, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_->GetSourceState() != STREAM_MANAGER_RUNNING, SUCCESS,
        "capturer source is already opened");
    UpdateAppsUidAndSessionId();
    return sourceInputNode_->CapturerSourceStart();
}

int32_t HpaeFastCapturerManager::CapturerSourceStop()
{
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_ != nullptr, ERR_ILLEGAL_STATE, "sourceInputNode_ is nullptr");
    CHECK_AND_RETURN_RET_LOG(sourceInputNode_->GetSourceState() != STREAM_MANAGER_SUSPENDED, SUCCESS,
        "capturer source is already stopped");
    return sourceInputNode_->CapturerSourceStop();
}

void HpaeFastCapturerManager::CreateSourceAttr(IAudioSourceAttr &attr)
{
    attr.adapterName = sourceInfo_.adapterName;
    attr.sampleRate = sourceInfo_.samplingRate;
    attr.channel = sourceInfo_.channels;
    attr.format = sourceInfo_.format;
    attr.channelLayout = sourceInfo_.channelLayout;
    attr.deviceType = sourceInfo_.deviceType;
    attr.volume = sourceInfo_.volume;
    attr.deviceNetworkId = sourceInfo_.deviceNetId.c_str();
    attr.filePath = sourceInfo_.filePath.c_str();
    attr.isBigEndian = false;
    attr.sourceType = static_cast<int32_t>(sourceInfo_.sourceType);
    attr.openMicSpeaker = sourceInfo_.openMicSpeaker;
    attr.macAddress = sourceInfo_.macAddress;
    bool isVoipFast = (sourceInfo_.routeFlag & AUDIO_INPUT_FLAG_VOIP_FAST) != 0 ||
        ((sourceInfo_.routeFlag & AUDIO_INPUT_FLAG_FAST) != 0 && (sourceInfo_.routeFlag & AUDIO_INPUT_FLAG_VOIP) != 0);
    attr.audioStreamFlag = isVoipFast ? AUDIO_FLAG_VOIP_FAST : AUDIO_FLAG_MMAP;
}

int32_t HpaeFastCapturerManager::InitCapturerManager()
{
    AUDIO_INFO_LOG("sourceName:%{public}s, channel:%{public}u, rate:%{public}u, routeFlag:%{public}u",
        sourceInfo_.sourceName.c_str(), sourceInfo_.channels, sourceInfo_.samplingRate, sourceInfo_.routeFlag);
    int32_t checkRet = CheckSourceInfoFramelen(sourceInfo_);
    CHECK_AND_RETURN_RET(checkRet == SUCCESS, checkRet);

    HpaeNodeInfo nodeInfo;
    nodeInfo.deviceClass = sourceInfo_.deviceClass;
    nodeInfo.deviceNetId = sourceInfo_.deviceNetId;
    nodeInfo.deviceName = sourceInfo_.sourceName;
    nodeInfo.channels = sourceInfo_.channels;
    nodeInfo.channelLayout = static_cast<AudioChannelLayout>(sourceInfo_.channelLayout);
    nodeInfo.format = sourceInfo_.format;
    nodeInfo.frameLen = sourceInfo_.frameLen;
    nodeInfo.samplingRate = sourceInfo_.samplingRate;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.sourceType = sourceInfo_.sourceType;
    nodeInfo.routeFlag = sourceInfo_.routeFlag;
    sourceInputNode_ = std::make_shared<HpaeFastSourceInputNode>(nodeInfo);
    int32_t ret = sourceInputNode_->GetCapturerSourceInstance(sourceInfo_.deviceClass, sourceInfo_.deviceNetId,
        sourceInfo_.sourceType, sourceInfo_.sourceName, sourceInfo_.busAddress);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetCapturerSourceInstance failed");
    captureId_ = sourceInputNode_->GetCaptureId();

    IAudioSourceAttr attr;
    CreateSourceAttr(attr);
    ret = sourceInputNode_->CapturerSourceInit(attr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CapturerSourceInit failed");
    isInit_.store(true);
    mixerNode_ = std::make_shared<HpaeMixerNode>(nodeInfo);
    mixerNode_->Connect(sourceInputNode_);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::Init(bool isReload)
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    hpaeSignalProcessThread_->SetFastThread(true);
    auto request = [this, isReload] {
        int32_t ret = InitCapturerManager();
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sourceInfo_.deviceName, ret);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Init failed");
        TriggerCallback(INIT_SOURCE_RESULT, sourceInfo_.sourceType);
        CheckIfAnyStreamRunning();
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::DeInit(bool isMoveDefault)
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    for (auto &[sessionId, sourceOutputNode] : sourceOutputNodeMap_) {
        sourceOutputNode->ResetAll();
    }
    StopOutputNode();
    isInit_.store(false);

    if (isMoveDefault) {
        std::string name = "";
        std::vector<uint32_t> ids;
        MoveAllStreamToNewSource(name, ids, MOVE_ALL);
    }
    return SUCCESS;
}

bool HpaeFastCapturerManager::IsInit()
{
    return isInit_.load();
}

bool HpaeFastCapturerManager::IsRunning(void)
{
    return sourceInputNode_ != nullptr && hpaeSignalProcessThread_ != nullptr &&
        sourceInputNode_->GetSourceState() == STREAM_MANAGER_RUNNING && hpaeSignalProcessThread_->IsRunning();
}

bool HpaeFastCapturerManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

bool HpaeFastCapturerManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

int32_t HpaeFastCapturerManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t checkRet = CheckStreamInfo(streamInfo);
    if (checkRet != SUCCESS) {
        return checkRet;
    }
    auto request = [this, streamInfo]() {
        CreateOutputSession(streamInfo);
        sourceOutputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_PREPARED);
        sessionNodeMap_[streamInfo.sessionId].state = HPAE_SESSION_PREPARED;
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, streamInfo.sessionId, CAPTURER_PREPARED,
            sourceOutputNodeMap_[streamInfo.sessionId]->GetAppUid());
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        auto node = SafeGetMap(sourceOutputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "DestroyStream not find sessionId %{public}u", sessionId);
        node->SetState(HPAE_SESSION_RELEASED);
        DeleteOutputSession(sessionId);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        auto node = SafeGetMap(sourceOutputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Start not find sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(ConnectOutputSession(sessionId) == SUCCESS, "Connect node error.");
        node->SetState(HPAE_SESSION_RUNNING);
        sessionNodeMap_[sessionId].state = HPAE_SESSION_RUNNING;
        CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, CAPTURER_RUNNING);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::Pause(uint32_t sessionId, bool isStandby)
{
    auto request = [this, sessionId]() {
        auto node = SafeGetMap(sourceOutputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Pause not find sessionId %{public}u", sessionId);
        DisConnectOutputSession(sessionId);
        node->SetState(HPAE_SESSION_PAUSED);
        sessionNodeMap_[sessionId].state = HPAE_SESSION_PAUSED;
        if (sourceInputNode_ != nullptr && sourceInputNode_->GetOutputPortNum() == 0) {
            (void)CapturerSourceStop();
        }
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId, HPAE_SESSION_PAUSED, OPERATION_PAUSED);
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, CAPTURER_PAUSED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::Flush(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        auto node = SafeGetMap(sourceOutputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Flush not find sessionId %{public}u", sessionId);
        if (sourceInputNode_ != nullptr) {
            (void)sourceInputNode_->CapturerSourceFlush();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::Drain(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "Drain not find sessionId %{public}u", sessionId);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId, sessionNodeMap_[sessionId].state,
            OPERATION_DRAINED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        auto node = SafeGetMap(sourceOutputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Stop not find sessionId %{public}u", sessionId);
        DisConnectOutputSession(sessionId);
        node->SetState(HPAE_SESSION_STOPPED);
        sessionNodeMap_[sessionId].state = HPAE_SESSION_STOPPED;
        if (sourceInputNode_ != nullptr && sourceInputNode_->GetOutputPortNum() == 0) {
            (void)CapturerSourceStop();
        }
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, CAPTURER_STOPPED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::Release(uint32_t sessionId)
{
    return DestroyStream(sessionId);
}

int32_t HpaeFastCapturerManager::SetStreamMute(uint32_t sessionId, bool isMute)
{
    auto request = [this, sessionId, isMute]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "Mute not find sessionId %{public}u", sessionId);
        sourceOutputNodeMap_[sessionId]->SetMute(isMute);
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, CAPTURER_RUNNING,
            sourceOutputNodeMap_[sessionId]->GetAppUid(), isMute);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::MoveStream(uint32_t sessionId, const std::string& sourceName)
{
    auto request = [this, sessionId, sourceName]() {
        if (!SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId, MOVE_SINGLE, sourceName);
            return;
        }
        CHECK_AND_RETURN_LOG(!sourceName.empty(), "sourceName is empty");
        HpaeCaptureMoveInfo moveInfo;
        moveInfo.sessionId = sessionId;
        moveInfo.sourceOutputNode = sourceOutputNodeMap_[sessionId];
        moveInfo.sessionInfo = sessionNodeMap_[sessionId];
        DeleteOutputSession(sessionId);
        std::string name = sourceName;
        TriggerCallback(MOVE_SOURCE_OUTPUT, moveInfo, name);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeFastCapturerManager::MoveAllStreamToNewSource(const std::string &sourceName,
    const std::vector<uint32_t>& moveIds, MoveSessionType moveType)
{
    std::string name = sourceName;
    std::vector<HpaeCaptureMoveInfo> moveInfos;
    for (const auto &[sessionId, node] : sourceOutputNodeMap_) {
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), sessionId) != moveIds.end()) {
            HpaeCaptureMoveInfo moveInfo;
            moveInfo.sessionId = sessionId;
            moveInfo.sourceOutputNode = node;
            moveInfo.sessionInfo = sessionNodeMap_[sessionId];
            moveInfos.emplace_back(moveInfo);
        }
    }
    for (const auto &it : moveInfos) {
        DeleteOutputSession(it.sessionId);
    }
    if (moveType == MOVE_ALL) {
        TriggerSyncCallback(MOVE_ALL_SOURCE_OUTPUT, moveInfos, name, moveType);
    } else {
        TriggerCallback(MOVE_ALL_SOURCE_OUTPUT, moveInfos, name, moveType);
    }
}

int32_t HpaeFastCapturerManager::MoveAllStream(const std::string& sourceName,
    const std::vector<uint32_t>& sessionIds, MoveSessionType moveType)
{
    if (!IsInit()) {
        MoveAllStreamToNewSource(sourceName, sessionIds, moveType);
    } else {
        auto request = [this, sourceName, sessionIds, moveType]() {
            MoveAllStreamToNewSource(sourceName, sessionIds, moveType);
        };
        SendRequest(request, __func__);
    }
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::SetMute(bool isMute)
{
    auto request = [this, isMute]() {
        if (isMute_ != isMute) {
            isMute_ = isMute;
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeFastCapturerManager::Process()
{
    if (!IsRunning()) {
        return;
    }
    UpdateAppsUidAndSessionId();
    if (appsUid_.empty()) {
        CapturerSourceStop();
        return;
    }
    for (const auto &[sessionId, sourceOutputNode] : sourceOutputNodeMap_) {
        if (sourceOutputNode->GetState() == HPAE_SESSION_RUNNING) {
            sourceOutputNode->DoProcess();
        }
    }
    int64_t wakeUpTime = ClockTime::GetCurNano();
    CHECK_AND_RETURN_LOG(sourceInputNode_ && sourceInputNode_->PrepareNextLoop(wakeUpTime),
        "PrepareNextLoop failed");
    ClockTime::AbsoluteSleep(wakeUpTime);
}

void HpaeFastCapturerManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

int32_t HpaeFastCapturerManager::RegisterReadCallback(uint32_t sessionId,
    const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        if (SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            sourceOutputNodeMap_[sessionId]->RegisterReadCallback(callback);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo)
{
    if (!SafeGetMap(sourceOutputNodeMap_, sessionId)) {
        return ERR_INVALID_OPERATION;
    }
    sourceOutputInfo.nodeInfo = sourceOutputNodeMap_[sessionId]->GetNodeInfo();
    sourceOutputInfo.capturerSessionInfo = sessionNodeMap_[sessionId];
    return SUCCESS;
}

HpaeSourceInfo HpaeFastCapturerManager::GetSourceInfo()
{
    return sourceInfo_;
}

std::vector<SourceOutput> HpaeFastCapturerManager::GetAllSourceOutputsInfo()
{
    return {};
}

void HpaeFastCapturerManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
        sessionNodeMap_[sessionId].state, operation);
}

void HpaeFastCapturerManager::OnNotifyQueue()
{
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ is nullptr");
    hpaeSignalProcessThread_->Notify();
}

void HpaeFastCapturerManager::OnRequestLatency(uint32_t sessionId, uint64_t &latency)
{
    (void)sessionId;
    latency = 0;
}

int32_t HpaeFastCapturerManager::AddNodeToSource(const HpaeCaptureMoveInfo &moveInfo)
{
    auto request = [this, moveInfo]() { AddSingleNodeToSource(moveInfo); };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::AddAllNodesToSource(const std::vector<HpaeCaptureMoveInfo> &moveInfos, bool isConnect)
{
    auto request = [this, moveInfos, isConnect]() {
        for (const auto &moveInfo : moveInfos) {
            AddSingleNodeToSource(moveInfo, isConnect);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeFastCapturerManager::AddSingleNodeToSource(const HpaeCaptureMoveInfo &moveInfo, bool isConnect)
{
    CHECK_AND_RETURN_LOG(moveInfo.sourceOutputNode != nullptr, "move fail, sourceOutputNode is null");
    HpaeNodeInfo nodeInfo = moveInfo.sourceOutputNode->GetNodeInfo();
    uint32_t sessionId = nodeInfo.sessionId;
    Trace trace("HpaeFastCapturerManager::AddSingleNodeToSource[" + std::to_string(sessionId) + "]");
    AUDIO_INFO_LOG("[FinishMove] session:%{public}u to source:fast", sessionId);
    TriggerCallback(UPDATE_SPAN_SIZE, sessionId, static_cast<uint32_t>(nodeInfo.frameLen),
        HPAE_STREAM_CLASS_TYPE_RECORD);

    sourceOutputNodeMap_[sessionId] = moveInfo.sourceOutputNode;
    sessionNodeMap_[sessionId] = moveInfo.sessionInfo;
    if (!SafeGetMap(converterNodeMap_, sessionId) && sourceInputNode_ != nullptr) {
        HpaeNodeInfo sourceNodeInfo = sourceInputNode_->GetNodeInfo();
        HpaeNodeInfo outputNodeInfo = moveInfo.sourceOutputNode->GetNodeInfo();
        auto converter = std::make_shared<HpaeAudioFormatConverterNode>(sourceNodeInfo, outputNodeInfo);
        converter->SetSourceNode(true);
        converterNodeMap_[sessionId] = converter;
    }
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(true, moveInfo.sourceOutputNode->GetNodeInfo());
#endif
    if (!isConnect || moveInfo.sourceOutputNode->GetState() != HPAE_SESSION_RUNNING) {
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, sessionId,
            ConvertHpaeToCapturerState(moveInfo.sourceOutputNode->GetState()), moveInfo.sourceOutputNode->GetAppUid());
        return;
    }
    CHECK_AND_RETURN_LOG(ConnectOutputSession(sessionId) == SUCCESS, "Connect node error.");
    CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
    NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, sessionId,
        ConvertHpaeToCapturerState(moveInfo.sourceOutputNode->GetState()), moveInfo.sourceOutputNode->GetAppUid());
}

std::string HpaeFastCapturerManager::GetThreadName()
{
    return sourceInfo_.deviceName;
}

int32_t HpaeFastCapturerManager::ReloadCaptureManager(const HpaeSourceInfo &sourceInfo, bool isReload)
{
    if (!IsInit()) {
        hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
        hpaeSignalProcessThread_->SetFastThread(true);
    }
    auto request = [this, sourceInfo, isReload] {
        for (const auto &[sessionId, node] : sourceOutputNodeMap_) {
            (void)sessionId;
            if (node != nullptr) {
                node->ResetAll();
            }
        }
        StopOutputNode();
        std::vector<HpaeCaptureMoveInfo> moveInfos;
        for (const auto &[sessionId, node] : sourceOutputNodeMap_) {
            HpaeCaptureMoveInfo moveInfo;
            moveInfo.sessionId = sessionId;
            moveInfo.sourceOutputNode = node;
            moveInfo.sessionInfo = sessionNodeMap_[sessionId];
            moveInfos.emplace_back(moveInfo);
        }
        for (const auto &it : moveInfos) {
            DeleteSessionNodes(it.sessionId);
        }
        sourceInfo_ = sourceInfo;
        int32_t ret = InitCapturerManager();
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sourceInfo_.deviceName, ret);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "re-Init failed");
        for (const auto &moveInfo : moveInfos) {
            AddSingleNodeToSource(moveInfo, true);
        }
        TriggerCallback(INIT_SOURCE_RESULT, sourceInfo_.sourceType);
    };
    SendRequest(request, __func__, true);
    if (!IsInit()) {
        hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    }
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::DumpSourceInfo()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "not init");
    SendRequest([this]() {
        UploadDumpSourceInfo(sourceInfo_.deviceName);
    }, __func__);
    return SUCCESS;
}

void HpaeFastCapturerManager::CheckIfAnyStreamRunning()
{
    CHECK_AND_RETURN_LOG(!sessionNodeMap_.empty(), "no stream need start");
    for (auto &[sessionId, sessionInfo] : sessionNodeMap_) {
        if (sessionInfo.state == HPAE_SESSION_RUNNING) {
            ConnectOutputSession(sessionId);
            CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
        }
    }
}

std::string HpaeFastCapturerManager::GetDeviceHDFDumpInfo()
{
    std::string config;
    TransDeviceInfoToString(sourceInfo_, config);
    return config;
}

int32_t HpaeFastCapturerManager::StopManager()
{
    auto request = [this] {
        CapturerSourceStop();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::AddCaptureInjector(
    const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode, const SourceType &sourceType)
{
    auto request = [this, sinkOutputNode] {
        Trace trace("HpaeFastCapturerManager::AddCaptureInjector");
        AUDIO_INFO_LOG("add capture injector");
        CHECK_AND_RETURN_LOG(sinkOutputNode != nullptr, "sinkOutputNode is nullptr");
        auto converter = std::make_shared<HpaeAudioFormatConverterNode>(
            sinkOutputNode->GetNodeInfo(), mixerNode_->GetNodeInfo());
        injectorFmtConverterNodeMap_[sinkOutputNode] = converter;
        mixerNode_->Connect(converter);
        converter->Connect(sinkOutputNode);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastCapturerManager::RemoveCaptureInjector(
    const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode, const SourceType &sourceType)
{
    auto request = [this, sinkOutputNode] {
        Trace trace("HpaeFastCapturerManager::RemoveCaptureInjector");
        AUDIO_INFO_LOG("remove capture injector");
        CHECK_AND_RETURN_LOG(sinkOutputNode != nullptr, "sinkOutputNode is nullptr");
        CHECK_AND_RETURN_LOG(injectorFmtConverterNodeMap_.find(sinkOutputNode) != injectorFmtConverterNodeMap_.end() &&
            injectorFmtConverterNodeMap_[sinkOutputNode] != nullptr, "sinkOutputNode not in map");
        auto converter = injectorFmtConverterNodeMap_[sinkOutputNode];
        injectorFmtConverterNodeMap_[sinkOutputNode] = converter;
        mixerNode_->DisConnect(converter);
        converter->DisConnect(sinkOutputNode);
        injectorFmtConverterNodeMap_.erase(sinkOutputNode);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeFastCapturerManager::TriggerAppsUidUpdate(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        appsUid_.clear();
        sessionsId_.clear();
        for (const auto &[streamId, sourceOutputNode] : sourceOutputNodeMap_) {
            if (sourceOutputNode->GetState() == HPAE_SESSION_RUNNING || streamId == sessionId) {
                appsUid_.emplace_back(sourceOutputNode->GetAppUid());
                sessionsId_.emplace_back(static_cast<int32_t>(streamId));
            }
        }
        if (sourceInputNode_ != nullptr) {
            sourceInputNode_->UpdateAppsUidAndSessionId(appsUid_, sessionsId_);
        }
    };
    SendRequest(request, __func__);
}

int32_t HpaeFastCapturerManager::SetAppsEnhanceMuteState(const uint32_t &sessionId, bool isMute)
{
    (void)sessionId;
    (void)isMute;
    return SUCCESS;
}

void HpaeFastCapturerManager::UpdateAppsUidAndSessionId()
{
    appsUid_.clear();
    sessionsId_.clear();
    for (const auto &[sessionId, sourceOutputNode] : sourceOutputNodeMap_) {
        if (sourceOutputNode->GetState() == HPAE_SESSION_RUNNING) {
            appsUid_.emplace_back(sourceOutputNode->GetAppUid());
            sessionsId_.emplace_back(static_cast<int32_t>(sessionId));
        }
    }
    if (sourceInputNode_ != nullptr) {
        sourceInputNode_->UpdateAppsUidAndSessionId(appsUid_, sessionsId_);
    }
}

void HpaeFastCapturerManager::NotifyStreamChangeToSource(
    StreamChangeType change, uint32_t sessionId, CapturerState state, uint32_t appUid, bool mute)
{
    SourceType source = SOURCE_TYPE_INVALID;
    if (sourceOutputNodeMap_.find(sessionId) != sourceOutputNodeMap_.end()) {
        source = sourceOutputNodeMap_[sessionId]->GetSourceType();
    }
    if (sourceInputNode_ != nullptr) {
        sourceInputNode_->NotifyStreamChangeToSource(change, sessionId, source, state, appUid, mute);
    }
}

void HpaeFastCapturerManager::StopOutputNode()
{
    if (sourceInputNode_ != nullptr) {
        (void)CapturerSourceStop();
        (void)sourceInputNode_->CapturerSourceDeInit();
        sourceInputNode_ = nullptr;
    }
    converterNodeMap_.clear();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
