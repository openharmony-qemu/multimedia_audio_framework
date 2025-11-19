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
#define LOG_TAG "HpaeInjectorRendererManager"
#endif

#include "hpae_injector_renderer_manager.h"
#include "audio_errors.h"
#include "audio_stream_info.h"
#include "audio_utils.h"
#include "hpae_node_common.h"
#include "hpae_message_queue_monitor.h"
#include "hpae_stream_move_monitor.h"
#include "audio_engine_log.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const std::string INJECTOR_THREADNAME = "Injector";
HpaeInjectorRendererManager::HpaeInjectorRendererManager(HpaeSinkInfo &sinkInfo)
    : hpaeNoLockQueue_(CURRENT_REQUEST_COUNT), sinkInfo_(sinkInfo)
{}

HpaeInjectorRendererManager::~HpaeInjectorRendererManager()
{
    AUDIO_INFO_LOG("destructor injector renderer");
    if (isInit_.load()) {
        DeInit();
    }
    sceneCluster_ = nullptr;
    sinkOutputNode_ = nullptr;
}

int32_t HpaeInjectorRendererManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, streamInfo]() {
        Trace trace("HpaeInjectorRendererManager::CreateStream id[" + std::to_string(streamInfo.sessionId) + "]");
        AUDIO_INFO_LOG("CreateStream sessionId %{public}u deviceName %{public}s", streamInfo.sessionId,
            sinkInfo_.deviceName.c_str());
        CreateInputSession(streamInfo);
        SetSessionState(streamInfo.sessionId, HPAE_SESSION_PREPARED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        Trace trace("HpaeInjectorRendererManager::DestroyStream id[" + std::to_string(sessionId) + "]");
        AUDIO_INFO_LOG("DestroyStream sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "DestroyStream not find sessionId %{public}u", sessionId);
        SetSessionState(sessionId, HPAE_SESSION_RELEASED);
        DeleteInputSession(sessionId);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::Start");
        AUDIO_INFO_LOG("Start sessionId %{public}u, deviceName %{public}s", sessionId, sinkInfo_.deviceName.c_str());
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), "sessionId %{public}u not found", sessionId);
        sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_RUNNING);
        ConnectInputSession(sessionId);
        SetSessionState(sessionId, HPAE_SESSION_RUNNING);
        SetSessionFade(sessionId, OPERATION_STARTED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::Pause(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::Pause");
        AUDIO_INFO_LOG("Pause sessionId %{public}u, deviceName %{public}s", sessionId, sinkInfo_.deviceName.c_str());
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), "sessionId %{public}u not found", sessionId);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, HPAE_SESSION_PAUSED, OPERATION_PAUSED);
        SetSessionState(sessionId, HPAE_SESSION_PAUSED);
        DisConnectInputSession(sessionId);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::Flush(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::Flush");
        AUDIO_INFO_LOG("Flush sessionId %{public}u deviceName %{public}s", sessionId, sinkInfo_.deviceName.c_str());
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Flush not find sessionId %{public}u", sessionId);
        sinkInputNodeMap_[sessionId]->Flush();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::Drain(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::Drain");
        AUDIO_INFO_LOG("Drain sessionId %{public}u deviceName %{public}s ", sessionId, sinkInfo_.deviceName.c_str());
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Drain not find sessionId %{public}u", sessionId);
        sinkInputNodeMap_[sessionId]->Drain();
        if (sessionNodeMap_[sessionId].state != HPAE_SESSION_RUNNING) {
            AUDIO_INFO_LOG("TriggerCallback Drain sessionId %{public}u", sessionId);
            TriggerCallback(UPDATE_STATUS,
                HPAE_STREAM_CLASS_TYPE_PLAY,
                sessionId,
                sessionNodeMap_[sessionId].state,
                OPERATION_DRAINED);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::Stop");
        AUDIO_INFO_LOG("Stop sessionId %{public}u, deviceName %{public}s", sessionId, sinkInfo_.deviceName.c_str());
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), "Stop not find sessionId %{public}u", sessionId);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, HPAE_SESSION_STOPPED, OPERATION_STOPPED);
        SetSessionState(sessionId, HPAE_SESSION_STOPPED);
        DisConnectInputSession(sessionId);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::Release(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::Release");
    AUDIO_INFO_LOG("Release sessionId %{public}u, deviceName %{public}s", sessionId, sinkInfo_.deviceName.c_str());
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), ERROR,
        "Release not find sessionId %{public}u", sessionId);
    return DestroyStream(sessionId);
}

int32_t HpaeInjectorRendererManager::MoveStream(uint32_t sessionId, const std::string& sinkName)
{
    if (!IsInit()) {
        MoveStreamSync(sessionId, sinkName);
    } else {
        auto request = [this, sessionId, sinkName]() { MoveStreamSync(sessionId, sinkName); };
        SendRequest(request, __func__);
    }
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::MoveAllStream(const std::string& sinkName, const std::vector<uint32_t>& sessionIds,
    MoveSessionType moveType)
{
    if (!IsInit()) {
        AUDIO_INFO_LOG("sink is not init ,use sync mode move to:%{public}s.", sinkName.c_str());
        MoveAllStreamToNewSink(sinkName, sessionIds, moveType);
    } else {
        AUDIO_INFO_LOG("sink is init ,use async mode move to:%{public}s.", sinkName.c_str());
        auto request = [this, sinkName, sessionIds, moveType]() {
            MoveAllStreamToNewSink(sinkName, sessionIds, moveType);
        };
        SendRequest(request, __func__);
    }
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::SuspendStreamManager(bool isSuspend)
{
    Trace trace("HpaeRendererManager::SuspendStreamManager: " + std::to_string(isSuspend));
    auto request = [this, isSuspend]() {
        if (isSuspend_ == isSuspend) {
            return;
        }
        AUDIO_INFO_LOG("suspend audio device: %{public}s, isSuspend: %{public}d",
            sinkInfo_.deviceName.c_str(), isSuspend);
        isSuspend_ = isSuspend;
        if (isSuspend_) {
            if (sinkOutputNode_ != nullptr) {
                sinkOutputNode_->RenderSinkStop();
            }
        } else if (sinkOutputNode_ != nullptr && sinkOutputNode_->GetState() != STREAM_MANAGER_RUNNING &&
            CheckIsStreamRunning()) {
            sinkOutputNode_->RenderSinkStart();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::SetMute(bool isMute)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

void HpaeInjectorRendererManager::Process()
{
    Trace trace("HpaeInjectorRendererManager::Process");
    if (sinkOutputNode_ != nullptr && IsRunning()) {
        sinkOutputNode_->DoRenderProcess();
    }
}

void HpaeInjectorRendererManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

int32_t HpaeInjectorRendererManager::Init(bool isReload)
{
    // after set sinkoutputnode
    if (sinkOutputNode_ == nullptr) {
        AUDIO_ERR_LOG("init fail, sinkoutputnode is null");
        TriggerCallback(INIT_DEVICE_RESULT, sinkInfo_.deviceName, ERROR);
        return ERR_ILLEGAL_STATE;
    }
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, isReload] {
        Trace trace("HpaeInjectorRendererManager::Init");
        InitManager(isReload);
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::DeInit(bool isMoveDefault)
{
    Trace trace("HpaeInjectorRendererManager::DeInit");
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    if (isMoveDefault) {
        std::string sinkName = "";
        std::vector<uint32_t> ids;
        AUDIO_INFO_LOG("move all sink to default sink");
        MoveAllStreamToNewSink(sinkName, ids, MOVE_ALL);
    }
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->RenderSinkStop();
        sinkOutputNode_->RenderSinkDeInit();
        sinkOutputNode_->ResetAll();
    }
    isInit_.store(false);
    return SUCCESS;
}

bool HpaeInjectorRendererManager::IsInit()
{
    return isInit_.load();
}

bool HpaeInjectorRendererManager::IsRunning(void)
{
    if (sinkOutputNode_ != nullptr && hpaeSignalProcessThread_ != nullptr) {
        return sinkOutputNode_->GetState() == STREAM_MANAGER_RUNNING && sinkOutputNode_->GetIsReadFinished() &&
            hpaeSignalProcessThread_->IsRunning();
    }
    return false;
}

bool HpaeInjectorRendererManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

bool HpaeInjectorRendererManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

int32_t HpaeInjectorRendererManager::SetClientVolume(uint32_t sessionId, float volume)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::SetRate(uint32_t sessionId, int32_t rate)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::SetAudioEffectMode(uint32_t sessionId, int32_t effectMode)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::SetPrivacyType(uint32_t sessionId, int32_t privacyType)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::GetPrivacyType(uint32_t sessionId, int32_t &privacyType)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::RegisterWriteCallback(
    uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        AUDIO_INFO_LOG("RegisterWriteCallback sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "RegisterWriteCallback not find sessionId %{public}u", sessionId);
        sinkInputNodeMap_[sessionId]->RegisterWriteCallback(callback);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::RegisterReadCallback(
    uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

size_t HpaeInjectorRendererManager::GetWritableSize(uint32_t sessionId)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return 0;
}

int32_t HpaeInjectorRendererManager::UpdateSpatializationState(uint32_t sessionId, bool spatializationEnabled,
    bool headTrackingEnabled)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::UpdateMaxLength(uint32_t sessionId, uint32_t maxLength)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

std::vector<SinkInput> HpaeInjectorRendererManager::GetAllSinkInputsInfo()
{
    return {};
}

int32_t HpaeInjectorRendererManager::GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo)
{
    if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
        return ERR_INVALID_OPERATION;
    }
    sinkInputInfo.nodeInfo = sinkInputNodeMap_[sessionId]->GetNodeInfo();
    sinkInputInfo.rendererSessionInfo = sessionNodeMap_[sessionId];
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::RefreshProcessClusterByDevice()
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

HpaeSinkInfo HpaeInjectorRendererManager::GetSinkInfo()
{
    return sinkInfo_;
}

int32_t HpaeInjectorRendererManager::AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    auto request = [this, node]() {
        AddSingleNodeToSink(node);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeInjectorRendererManager::AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect)
{
    Trace trace("HpaeInjectorRendererManager::AddSingleNodeToSink");
    HpaeNodeInfo nodeInfo = node->GetNodeInfo();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.historyFrameCount = 0;
    nodeInfo.statusCallback = weak_from_this();
    node->SetNodeInfo(nodeInfo);
    uint32_t sessionId = nodeInfo.sessionId;
    
    sinkInputNodeMap_[sessionId] = node;
    SetSessionState(sessionId, node->GetState());
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(true, nodeInfo);
#endif
    if (node->GetState() == HPAE_SESSION_RUNNING) {
        ConnectInputSession(sessionId);
    }
    AUDIO_INFO_LOG("[FinishMove] session:%{public}u to sink:injector", sessionId);
}

int32_t HpaeInjectorRendererManager::AddAllNodesToSink(
    const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect)
{
    auto request = [this, sinkInputs, isConnect]() {
        for (const auto &it : sinkInputs) {
            AddSingleNodeToSink(it, isConnect);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeInjectorRendererManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    // from sinkinputnode, maybe underflow, but underflow maybe not arise in this situation
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId,
        sessionNodeMap_[sessionId].state, operation);
}

void HpaeInjectorRendererManager::OnFadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Fade done, not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Fade done, callback at injectorRendererManager");
        DisConnectInputSession(sessionId);
        IOperation operation = sinkInputNodeMap_[sessionId]->GetState() == HPAE_SESSION_STOPPING ?
            OPERATION_STOPPED : OPERATION_PAUSED;
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
        SetSessionState(sessionId, state);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
    };
    SendRequest(request, __func__);
}

void HpaeInjectorRendererManager::OnNotifyQueue()
{
    Trace trace("HpaeInjectorRendererManager::OnNotifyQueue");
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "HpaeInjectorRendererManager hpaeSignalProcessThread_ is nullptr");
    hpaeSignalProcessThread_->Notify();
}

std::string HpaeInjectorRendererManager::GetThreadName()
{
    return INJECTOR_THREADNAME;
}

int32_t HpaeInjectorRendererManager::DumpSinkInfo()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERROR_ILLEGAL_STATE, "HpaeInjectorRendererManager not init");
    auto request = [this]() {
        AUDIO_INFO_LOG("DumpSinkInfo deviceName %{public}s", sinkInfo_.deviceName.c_str());
        UploadDumpSinkInfo(sinkInfo_.deviceName);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload)
{
    if (sinkOutputNode_ == nullptr) { // init after set sinkoutput
        TriggerCallback(INIT_DEVICE_RESULT, sinkInfo_.deviceName, ERROR);
        return ERR_ILLEGAL_STATE;
    }

    if (IsInit()) {
        AUDIO_INFO_LOG("deinit:%{public}s before reload injector renderer manager", sinkInfo.deviceName.c_str());
        DeInit();
    }
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, sinkInfo, isReload]() {
        AUDIO_INFO_LOG("reload injector renderer manager, deviceName %{public}s", sinkInfo.deviceName.c_str());
        for (const auto &it : sinkInputNodeMap_) {
            TriggerStreamState(it.first, it.second);
            DisConnectInputSession(it.first);
        }
        sinkInfo_ = sinkInfo;
        InitManager(isReload);
        AUDIO_INFO_LOG("init device:%{public}s manager end", sinkInfo.deviceName.c_str());
        for (const auto &it : sinkInputNodeMap_) {
            ConnectInputSession(it.first);
        }
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

std::string HpaeInjectorRendererManager::GetDeviceHDFDumpInfo()
{
    std::string config;
    TransDeviceInfoToString(sinkInfo_, config);
    return config;
}

int32_t HpaeInjectorRendererManager::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    AUDIO_ERR_LOG("Unsupported operation");
    return SUCCESS;
}

void HpaeInjectorRendererManager::SendRequest(Request &&request, const std::string &funcName, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_ERR_LOG("HpaeInjectorRendererManager not init, %{public}s excute failed", funcName.c_str());
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    hpaeSignalProcessThread_->Notify();
}

int32_t HpaeInjectorRendererManager::SetSinkVirtualOutputNode(
    const std::shared_ptr<HpaeSinkVirtualOutputNode> &sinkVirtualOutputNode)
{
    // should set sinkoutputnode before init
    sinkOutputNode_ = sinkVirtualOutputNode;
    return SUCCESS;
}

void HpaeInjectorRendererManager::InitManager(bool isReload)
{
    AUDIO_INFO_LOG("init devicename:%{public}s", sinkInfo_.deviceName.c_str());
    HpaeNodeInfo nodeInfo = sinkOutputNode_ ->GetNodeInfo();
    sceneCluster_ = std::make_shared<HpaeVirtualProcessCluster>(nodeInfo);
    int32_t ret = sinkOutputNode_->RenderSinkInit();
    isInit_.store(ret == SUCCESS);
    AUDIO_INFO_LOG("HpaeInjectorRendererManager init %{public}s", ret == SUCCESS ? "success" : "fail");
    TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sinkInfo_.deviceName, ret);
}

int32_t HpaeInjectorRendererManager::CreateInputSession(const HpaeStreamInfo &streamInfo)
{
    Trace trace("[" + std::to_string(streamInfo.sessionId) + "]HpaeInjectorRendererManager::CreateInputSession");
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = streamInfo.channels;
    nodeInfo.format = streamInfo.format;
    nodeInfo.frameLen = streamInfo.frameLen;
    nodeInfo.streamType = streamInfo.streamType;
    nodeInfo.sessionId = streamInfo.sessionId;
    nodeInfo.samplingRate = static_cast<AudioSamplingRate>(streamInfo.samplingRate);
    nodeInfo.customSampleRate = streamInfo.customSampleRate;
    nodeInfo.sceneType = TransStreamTypeToSceneType(streamInfo.streamType);
    nodeInfo.effectInfo = streamInfo.effectInfo;
    nodeInfo.historyFrameCount = 0;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.deviceName = sinkInfo_.deviceName;
    sinkInputNodeMap_[streamInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    AUDIO_INFO_LOG("streamType %{public}u, sessionId = %{public}u, current sceneType is %{public}d",
        nodeInfo.streamType,
        nodeInfo.sessionId,
        nodeInfo.sceneType);
    return SUCCESS;
}

void HpaeInjectorRendererManager::DeleteInputSession(const uint32_t &sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::DeleteInputSession");
    DisConnectInputSession(sessionId);
#ifdef ENABLE_HIDUMP_DFX
    if (auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId)) {
        OnNotifyDfxNodeAdmin(false, sinkInputNode->GetNodeInfo());
    }
#endif
    sinkInputNodeMap_.erase(sessionId);
    sessionNodeMap_.erase(sessionId);
}

int32_t HpaeInjectorRendererManager::ConnectInputSession(const uint32_t &sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::ConnectInputSession");
    AUDIO_INFO_LOG("connect input session:%{public}d", sessionId);
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), ERR_INVALID_PARAM,
        "connect fail, session %{public}u not found", sessionId);
    CHECK_AND_RETURN_RET(sinkInputNodeMap_[sessionId]->GetState() == HPAE_SESSION_RUNNING, SUCCESS);
    CHECK_AND_RETURN_RET_LOG(sinkOutputNode_ != nullptr && sceneCluster_ != nullptr, ERROR, "manager maybe not init");
    sinkOutputNode_->Connect(sceneCluster_);
    sceneCluster_->Connect(sinkInputNodeMap_[sessionId]);
    if (sinkOutputNode_->GetState() != STREAM_MANAGER_RUNNING && !isSuspend_) {
        sinkOutputNode_->RenderSinkStart();
    }
    return SUCCESS;
}

int32_t HpaeInjectorRendererManager::DisConnectInputSession(const uint32_t &sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeInjectorRendererManager::DisConnectInputSession");
    AUDIO_INFO_LOG("disconnect input session:%{public}d", sessionId);
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), ERR_INVALID_PARAM,
        "disconnect fail, session %{public}u not found", sessionId);
    CHECK_AND_RETURN_RET(sinkOutputNode_ != nullptr && sceneCluster_ != nullptr, SUCCESS);
    sceneCluster_->DisConnect(sinkInputNodeMap_[sessionId]);

    if (sceneCluster_->GetConnectSinkInputNum() == 0) {
        sinkOutputNode_->DisConnect(sceneCluster_);
        sinkOutputNode_->RenderSinkStop();
    }
    return SUCCESS;
}

void HpaeInjectorRendererManager::MoveStreamSync(uint32_t sessionId, const std::string &sinkName)
{
    if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
        AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,can not find session,move %{public}s --> %{public}s",
            sessionId, sinkInfo_.deviceName.c_str(), sinkName.c_str());
        TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
        HpaeStreamMoveMonitor::ReportStreamMoveException(0, sessionId, HPAE_STREAM_CLASS_TYPE_PLAY,
            sinkInfo_.deviceName, sinkName, "not find session node");
        return;
    }

    if (sinkName.empty()) {
        AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,sinkName is empty", sessionId);
        TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
        HpaeStreamMoveMonitor::ReportStreamMoveException(sinkInputNodeMap_[sessionId]->GetAppUid(), sessionId,
            HPAE_STREAM_CLASS_TYPE_PLAY, sinkInfo_.deviceName, sinkName, "sinkName is empty");
        return;
    }

    AUDIO_INFO_LOG("[StartMove] session: %{public}u,sink [%{public}s] --> [%{public}s]",
        sessionId, sinkInfo_.deviceName.c_str(), sinkName.c_str());
    std::shared_ptr<HpaeSinkInputNode> inputNode = sinkInputNodeMap_[sessionId];
    DeleteInputSession(sessionId);
    std::string name = sinkName;
    TriggerCallback(MOVE_SINK_INPUT, inputNode, name);
}

void HpaeInjectorRendererManager::MoveAllStreamToNewSink(const std::string &sinkName,
    const std::vector<uint32_t>& moveIds, MoveSessionType moveType)
{
    Trace trace("HpaeInjectorRendererManager::MoveAllStreamToNewSink[" + sinkName + "]");
    AUDIO_INFO_LOG("MoveAllStreamToNewSink[%{public}s]", sinkName.c_str());
    std::string name = sinkName;
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    std::vector<uint32_t> sessionIds;
    std::string idStr;
    for (const auto &it : sinkInputNodeMap_) {
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), it.first) != moveIds.end()) {
            sinkInputs.emplace_back(it.second);
            sessionIds.emplace_back(it.first);
            idStr.append("[").append(std::to_string(it.first)).append("],");
        }
    }
    for (const auto &it : sessionIds) {
        DeleteInputSession(it);
    }
    HILOG_COMM_INFO("[StartMove] session:%{public}s to sink name:%{public}s, move type:%{public}d",
        idStr.c_str(), name.c_str(), moveType);

    if (moveType == MOVE_ALL) {
        TriggerSyncCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
    } else {
        TriggerCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
    }
}

bool HpaeInjectorRendererManager::SetSessionFade(uint32_t sessionId, IOperation operation)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), false,
        "SetSessionFade not find session %{public}u", sessionId);
    std::shared_ptr<HpaeGainNode> sessionGainNode = sceneCluster_->GetGainNodeById(sessionId);
    if (sessionGainNode == nullptr) {
        AUDIO_ERR_LOG("SetSessionFade not find gain node for session %{public}u", sessionId);
        if (operation != OPERATION_STARTED) {
            HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
            SetSessionState(sessionId, state);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
        }
        return false;
    }
    AUDIO_INFO_LOG("get gain node of session %{public}d operation %{public}d.", sessionId, operation);
    if (sinkInputNodeMap_[sessionId]->GetState() != HPAE_SESSION_STOPPING &&
        sinkInputNodeMap_[sessionId]->GetState() != HPAE_SESSION_PAUSING) {
        sessionGainNode->SetFadeState(operation);
    }

    if (operation != OPERATION_STARTED) {
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPING : HPAE_SESSION_PAUSING;
        SetSessionState(sessionId, state);
    }
    return true;
}

void HpaeInjectorRendererManager::SetSessionState(uint32_t sessionId, HpaeSessionState state)
{
    sessionNodeMap_[sessionId].state = state;
    sinkInputNodeMap_[sessionId]->SetState(state);
}

void HpaeInjectorRendererManager::TriggerStreamState(uint32_t sessionId,
    const std::shared_ptr<HpaeSinkInputNode> &inputNode)
{
    HpaeSessionState inputState = inputNode->GetState();
    if (inputState == HPAE_SESSION_STOPPING || inputState == HPAE_SESSION_PAUSING) {
        HpaeSessionState state = inputState == HPAE_SESSION_PAUSING ? HPAE_SESSION_PAUSED : HPAE_SESSION_STOPPED;
        IOperation operation = inputState == HPAE_SESSION_PAUSING ? OPERATION_PAUSED : OPERATION_STOPPED;
        SetSessionState(sessionId, state);
        inputNode->SetState(state);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
    }
}

bool HpaeInjectorRendererManager::CheckIsStreamRunning()
{
    bool isRunning = false;
    for (const auto& it : sessionNodeMap_) {
        if (it.second.state == HPAE_SESSION_RUNNING) {
            isRunning = true;
            break;
        }
    }
    return isRunning;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
