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
#define LOG_TAG "HpaeInnerCapturerManager"
#endif
#include "audio_stream_info.h"
#include "audio_errors.h"
#include "audio_utils.h"
#include "hpae_node_common.h"
#include "hpae_inner_capturer_manager.h"
#include "audio_engine_log.h"
#include "hpae_message_queue_monitor.h"
#include "hpae_stream_move_monitor.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
// todo sinkInfo
HpaeInnerCapturerManager::HpaeInnerCapturerManager(HpaeSinkInfo &sinkInfo)
    : sinkInfo_(sinkInfo), hpaeNoLockQueue_(CURRENT_REQUEST_COUNT)
{}

HpaeInnerCapturerManager::~HpaeInnerCapturerManager()
{
    AUDIO_INFO_LOG("destructor");
    if (isInit_.load()) {
        DeInit();
    }
}

int32_t HpaeInnerCapturerManager::AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    auto request = [this, node]() {
        AddSingleNodeToSinkInner(node);
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

void HpaeInnerCapturerManager::AddSingleNodeToSinkInner(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect)
{
    HpaeNodeInfo nodeInfo = node->GetNodeInfo();
    uint32_t sessionId = nodeInfo.sessionId;
    Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::AddSingleNodeToSinkInner");
    HILOG_COMM_INFO("[FinishMove] session :%{public}u to sink:%{public}s, sceneType is %{public}d",
        sessionId, sinkInfo_.deviceClass.c_str(), node->GetSceneType());
    sinkInputNodeMap_[sessionId] = node;
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    nodeInfo.statusCallback = weak_from_this();
    sinkInputNodeMap_[sessionId]->SetNodeInfo(nodeInfo);
    SetSessionStateForRenderer(sessionId, node->GetState());
    rendererSessionNodeMap_[sessionId].sceneType = nodeInfo.sceneType;
    sceneTypeToProcessClusterCount_++;

    if (!SafeGetMap(rendererSceneClusterMap_, nodeInfo.sceneType)) {
        rendererSceneClusterMap_[nodeInfo.sceneType] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo_);
    }

    if (!isConnect) {
        AUDIO_INFO_LOG("[FinishMove] not need connect session:%{public}d", sessionId);
        return;
    }

    if (node->GetState() == HPAE_SESSION_RUNNING) {
        AUDIO_INFO_LOG("[FinishMove] session:%{public}u connect to sink:%{public}s",
            sessionId, sinkInfo_.deviceClass.c_str());
        ConnectRendererInputSessionInner(sessionId);
    }
}

int32_t HpaeInnerCapturerManager::AddAllNodesToSink(
    const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect)
{
    auto request = [this, sinkInputs, isConnect]() {
        for (const auto &it : sinkInputs) {
            AddSingleNodeToSinkInner(it, isConnect);
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

void HpaeInnerCapturerManager::MoveAllStreamToNewSinkInner(const std::string &sinkName,
    const std::vector<uint32_t>& moveIds, MoveSessionType moveType)
{
    Trace trace("HpaeInnerCapturerManager::MoveAllStreamToNewSinkInner[" + sinkName + "]");
    std::string name = sinkName;
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    std::vector<uint32_t> sessionIds;
    std::string idStr;
    for (const auto &it : sinkInputNodeMap_) {
        if (!rendererSessionNodeMap_[it.first].isMoveAble) {
            AUDIO_INFO_LOG("move session:%{public}u failed, session is not moveable.", it.first);
            continue;
        }
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), it.first) != moveIds.end()) {
            sinkInputs.emplace_back(it.second);
            sessionIds.emplace_back(it.first);
            idStr.append("[");
            idStr.append(std::to_string(it.first));
            idStr.append("],");
        }
    }
    for (const auto &it : sessionIds) {
        DeleteRendererInputSessionInner(it);
    }
    HILOG_COMM_INFO("StartMove] session:%{public}s to sink name:%{public}s, move type:%{public}d",
        idStr.c_str(), name.c_str(), moveType);
    TriggerCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
}

int32_t HpaeInnerCapturerManager::MoveAllStream(const std::string &sinkName, const std::vector<uint32_t>& sessionIds,
    MoveSessionType moveType)
{
    if (!IsInit()) {
        AUDIO_INFO_LOG("sink is not init ,use sync mode move to:%{public}s.", sinkName.c_str());
        MoveAllStreamToNewSinkInner(sinkName, sessionIds, moveType);
    } else {
        AUDIO_INFO_LOG("sink is init ,use async mode move to:%{public}s.", sinkName.c_str());
        auto request = [this, sinkName, sessionIds, moveType]() {
            MoveAllStreamToNewSinkInner(sinkName, sessionIds, moveType);
        };
        SendRequestInner(request, __func__);
    }
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::MoveStream(uint32_t sessionId, const std::string &sinkName)
{
    AUDIO_INFO_LOG("move session:%{public}d,sink name:%{public}s", sessionId, sinkName.c_str());
    auto request = [this, sessionId, sinkName]() {
        if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,can not find session,move %{public}s --> %{public}s",
                sessionId, sinkInfo_.deviceName.c_str(), sinkName.c_str());
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
            HpaeStreamMoveMonitor::ReportStreamMoveException(0, sessionId, HPAE_STREAM_CLASS_TYPE_PLAY,
                "innercapture", sinkName, "not find session node");
            return;
        }
        if (sinkName.empty()) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,sinkName is empty", sessionId);
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
            HpaeStreamMoveMonitor::ReportStreamMoveException(sinkInputNodeMap_[sessionId]->GetAppUid(), sessionId,
                HPAE_STREAM_CLASS_TYPE_PLAY, "innercapture", sinkName, "sinkName is empty");
            return;
        }

        AUDIO_INFO_LOG("[StartMove] session: %{public}u,sink [%{public}s] --> [%{public}s]",
            sessionId, sinkInfo_.deviceName.c_str(), sinkName.c_str());
        std::shared_ptr<HpaeSinkInputNode> inputNode = sinkInputNodeMap_[sessionId];
        DeleteRendererInputSessionInner(sessionId);
        std::string name = sinkName;
        TriggerCallback(MOVE_SINK_INPUT, inputNode, name);
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        AUDIO_INFO_LOG("not init");
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, streamInfo]() {
        if (streamInfo.streamClassType == HPAE_STREAM_CLASS_TYPE_PLAY) {
            Trace trace("HpaeInnerCapturerManager::CreateRendererStream id[" +
                std::to_string(streamInfo.sessionId) + "]");
            AUDIO_INFO_LOG("CreateCapRendererStream sessionID: %{public}d", streamInfo.sessionId);
            CreateRendererInputSessionInner(streamInfo);
            SetSessionStateForRenderer(streamInfo.sessionId, HPAE_SESSION_PREPARED);
            sinkInputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_PREPARED);
            rendererSessionNodeMap_[streamInfo.sessionId].isMoveAble = streamInfo.isMoveAble;
        } else if (streamInfo.streamClassType == HPAE_STREAM_CLASS_TYPE_RECORD) {
            Trace trace("HpaeInnerCapturerManager::CreateCapturerStream id[" +
                std::to_string(streamInfo.sessionId) + "]");
            AUDIO_INFO_LOG("CreateCapCapturerStream sessionID: %{public}d", streamInfo.sessionId);
            CreateCapturerInputSessionInner(streamInfo);
            SetSessionStateForCapturer(streamInfo.sessionId, HPAE_SESSION_PREPARED);
            capturerSessionNodeMap_[streamInfo.sessionId].isMoveAble = streamInfo.isMoveAble;
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId) ||
            SafeGetMap(sourceOutputNodeMap_, sessionId),
            "no find sessionId in sinkInputNodeMap and sourceOutputNodeMap");
        AUDIO_INFO_LOG("DestroyStream sessionId %{public}u", sessionId);
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            Trace trace("HpaeInnerCapturerManager::DestroyRendererStream id[" + std::to_string(sessionId) + "]");
            AUDIO_INFO_LOG("DestroyCapRendererStream sessionID: %{public}d", sessionId);
            DeleteRendererInputSessionInner(sessionId);
        } else if (SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            Trace trace("HpaeInnerCapturerManager::DestroyCapturerStream id[" + std::to_string(sessionId) + "]");
            AUDIO_INFO_LOG("DestroyCapCapturerStream sessionID: %{public}d", sessionId);
            DeleteCapturerInputSessionInner(sessionId);
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}
 
int32_t HpaeInnerCapturerManager::ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload)
{
    if (IsInit()) {
        AUDIO_INFO_LOG("deinit inner capture first.");
        DeInit();
    }
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, sinkInfo, isReload]() {
        for (const auto &it: sinkInputNodeMap_) {
            DeleteRendererInputNodeSession(it.second);
        }

        sinkInfo_ = sinkInfo;
        InitSinkInner(isReload);

        for (const auto &it: sinkInputNodeMap_) {
            HpaeProcessorType processorType =  it.second->GetSceneType();
            HpaeNodeInfo nodeInfo = it.second->GetNodeInfo();
            if (!SafeGetMap(rendererSceneClusterMap_, processorType)) {
                rendererSceneClusterMap_[processorType] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo_);
            }
            if (it.second->GetState() == HPAE_SESSION_RUNNING) {
                ConnectRendererInputSessionInner(it.first);
            }
        }
    };
    SendRequestInner(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::Init(bool isReload)
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, isReload] {
        Trace trace("HpaeInnerCapturerManager::Init");
        InitSinkInner(isReload);
    };
    SendRequestInner(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}
 
void HpaeInnerCapturerManager::InitSinkInner(bool isReload)
{
    Trace trace("HpaeInnerCapturerManager::InitSinkInner");
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = sinkInfo_.channels;
    nodeInfo.format = sinkInfo_.format;
    nodeInfo.frameLen = sinkInfo_.frameLen;
    nodeInfo.nodeId = 0;
    nodeInfo.samplingRate = sinkInfo_.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_OUT;
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.statusCallback = weak_from_this();
    hpaeInnerCapSinkNode_ = std::make_unique<HpaeInnerCapSinkNode>(nodeInfo);
    AUDIO_INFO_LOG("Init innerCapSinkNode");
    hpaeInnerCapSinkNode_->InnerCapturerSinkInit();
    isInit_.store(true);
    TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT :INIT_DEVICE_RESULT, sinkInfo_.deviceName, SUCCESS);
}

bool HpaeInnerCapturerManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

int32_t HpaeInnerCapturerManager::DeInit(bool isMoveDefault)
{
    Trace trace("HpaeInnerCapturerManager::DeInit[" + std::to_string(isMoveDefault) + "]");
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    int32_t ret = SUCCESS;
    if (hpaeInnerCapSinkNode_  != nullptr) {
        ret = hpaeInnerCapSinkNode_->InnerCapturerSinkDeInit();
        hpaeInnerCapSinkNode_->ResetAll();
        hpaeInnerCapSinkNode_ = nullptr;
    }
    isInit_.store(false);
    if (isMoveDefault) {
        std::string sinkName = "";
        std::vector<uint32_t> ids;
        AUDIO_INFO_LOG("move all sink to default sink");
        MoveAllStreamToNewSinkInner(sinkName, ids, MOVE_ALL);
    }
    return ret;
}

int32_t HpaeInnerCapturerManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId) ||
            SafeGetMap(sourceOutputNodeMap_, sessionId),\
            "no find sessionId in sinkInputNodeMap and sourceOutputNodeMap");
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::StartRendererStream");
            AUDIO_INFO_LOG("StartCapRendererStream sessionId %{public}u", sessionId);
            sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_RUNNING);
            ConnectRendererInputSessionInner(sessionId);
            SetSessionStateForRenderer(sessionId, HPAE_SESSION_RUNNING);
        } else if (SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::StartCapturerStream");
            AUDIO_INFO_LOG("StartCapCapturerStream sessionId %{public}u", sessionId);
            if (hpaeInnerCapSinkNode_->GetSinkState() != STREAM_MANAGER_RUNNING) {
                hpaeInnerCapSinkNode_->InnerCapturerSinkStart();
            }
            sourceOutputNodeMap_[sessionId]->SetState(HPAE_SESSION_RUNNING);
            ConnectCapturerOutputSessionInner(sessionId);
            SetSessionStateForCapturer(sessionId, HPAE_SESSION_RUNNING);
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::Pause(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId) ||
            SafeGetMap(sourceOutputNodeMap_, sessionId),
            "no find sessionId in sinkInputNodeMap and sourceOutputNodeMap");
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::PauseRendererStream");
            AUDIO_INFO_LOG("PauseCapRendererStream sessionId %{public}u", sessionId);
            DisConnectRendererInputSessionInner(sessionId);
            SetSessionStateForRenderer(sessionId, HPAE_SESSION_PAUSED);
            sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_PAUSED);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId,
                HPAE_SESSION_PAUSED, OPERATION_PAUSED);
        } else if (SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::PauseCapturerStream");
            AUDIO_INFO_LOG("PauseCapCapturerStream sessionId %{public}u", sessionId);
            DisConnectCapturerInputSessionInner(sessionId);
            SetSessionStateForCapturer(sessionId, HPAE_SESSION_PAUSED);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
                HPAE_SESSION_PAUSED, OPERATION_PAUSED);
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::Flush(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId) ||
            SafeGetMap(sourceOutputNodeMap_, sessionId),\
            "no find sessionId in sinkInputNodeMap and sourceOutputNodeMap");
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::FlushRendererStream");
            AUDIO_INFO_LOG("FlushCapRendererStream sessionId %{public}u", sessionId);
            CHECK_AND_RETURN_LOG(rendererSessionNodeMap_.find(sessionId) != rendererSessionNodeMap_.end(),
                "Flush not find sessionId %{public}u", sessionId);
        } else if (SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::FlushCapturerStream");
            AUDIO_INFO_LOG("FlushCapCapturerStream sessionId %{public}u", sessionId);
            CHECK_AND_RETURN_LOG(capturerSessionNodeMap_.find(sessionId) != capturerSessionNodeMap_.end(),
                "Flush not find sessionId %{public}u", sessionId);
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::Drain(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId) ||
            SafeGetMap(sourceOutputNodeMap_, sessionId),
            "no find sessionId in sinkInputNodeMap and sourceOutputNodeMap");
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::DrainRendererStream");
            AUDIO_INFO_LOG("DrainCapRendererStream sessionId %{public}u", sessionId);
            CHECK_AND_RETURN_LOG(rendererSessionNodeMap_.find(sessionId) != rendererSessionNodeMap_.end(),
                "Drain not find sessionId %{public}u", sessionId);
            sinkInputNodeMap_[sessionId]->Drain();
            if (rendererSessionNodeMap_[sessionId].state != HPAE_SESSION_RUNNING) {
                AUDIO_INFO_LOG("TriggerCallback Drain sessionId %{public}u", sessionId);
                TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId,
                    rendererSessionNodeMap_[sessionId].state, OPERATION_DRAINED);
            }
        } else if (SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::DrainCapturerStream");
            AUDIO_INFO_LOG("DrainCapCapturerStream sessionId %{public}u", sessionId);
            CHECK_AND_RETURN_LOG(capturerSessionNodeMap_.find(sessionId) != capturerSessionNodeMap_.end(),
                "Drain not find sessionId %{public}u", sessionId);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
                capturerSessionNodeMap_[sessionId].state, OPERATION_DRAINED);
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId) ||
            SafeGetMap(sourceOutputNodeMap_, sessionId),\
            "no find sessionId in sinkInputNodeMap and sourceOutputNodeMap");
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::StopRendererStream");
            AUDIO_INFO_LOG("StopCapRendererStream sessionId %{public}u", sessionId);
            DisConnectRendererInputSessionInner(sessionId);
            SetSessionStateForRenderer(sessionId, HPAE_SESSION_STOPPED);
            sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_STOPPED);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId,
                HPAE_SESSION_STOPPED, OPERATION_STOPPED);
        } else if (SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::StopCapturerStream");
            AUDIO_INFO_LOG("StopCapCapturerStream sessionId %{public}u", sessionId);
            DisConnectCapturerInputSessionInner(sessionId);
            SetSessionStateForCapturer(sessionId, HPAE_SESSION_STOPPED);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
                HPAE_SESSION_STOPPED, OPERATION_STOPPED);
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::Release(uint32_t sessionId)
{
    return DestroyStream(sessionId);
}

int32_t HpaeInnerCapturerManager::SuspendStreamManager(bool isSuspend)
{
    Trace trace("HpaeInnerCapturerManager::SuspendStreamManager: " + std::to_string(isSuspend));
    auto request = [this, isSuspend]() {
        if (isSuspend) {
            // todo fadout
            hpaeInnerCapSinkNode_->InnerCapturerSinkStop();
        } else {
            // todo fadout
            hpaeInnerCapSinkNode_->InnerCapturerSinkStart();
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::SetMute(bool isMute)
{
    Trace trace("HpaeInnerCapturerManager::SetMute: " + std::to_string(isMute));
    auto request = [this, isMute]() {
        if (hpaeInnerCapSinkNode_ != nullptr) {
            hpaeInnerCapSinkNode_->SetMute(isMute);
        } else {
            AUDIO_INFO_LOG("hapeInnerCapSinkNode_ is nullptr");
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

void HpaeInnerCapturerManager::Process()
{
    Trace trace("HpaeInnerCapturerManager::Process");
    if (hpaeInnerCapSinkNode_ != nullptr && !sourceOutputNodeMap_.empty() && IsRunning()) {
        for (const auto& sourceOutputNodePair : sourceOutputNodeMap_) {
            if (capturerSessionNodeMap_[sourceOutputNodePair.first].state == HPAE_SESSION_RUNNING) {
                sourceOutputNodePair.second->DoProcess();
            }
        }
    }
}

void HpaeInnerCapturerManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

bool HpaeInnerCapturerManager::IsInit()
{
    return isInit_.load();
}

bool HpaeInnerCapturerManager::IsRunning(void)
{
    if (hpaeInnerCapSinkNode_ != nullptr && hpaeSignalProcessThread_ != nullptr) {
        return hpaeSignalProcessThread_->IsRunning() && hpaeInnerCapSinkNode_->GetSinkState() ==
            STREAM_MANAGER_RUNNING;
    } else {
        return false;
    }
}

bool HpaeInnerCapturerManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

int32_t HpaeInnerCapturerManager::SetClientVolume(uint32_t sessionId, float volume)
{
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::SetRate(uint32_t sessionId, int32_t rate)
{
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::SetAudioEffectMode(uint32_t sessionId, int32_t effectMode)
{
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode)
{
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::SetPrivacyType(uint32_t sessionId, int32_t privacyType)
{
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::GetPrivacyType(uint32_t sessionId, int32_t &privacyType)
{
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::RegisterWriteCallback(uint32_t sessionId,
    const std::weak_ptr<IStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        AUDIO_INFO_LOG("RegisterWriteCallback sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId) ||
            SafeGetMap(sourceOutputNodeMap_, sessionId),\
            "no find sessionId in sinkInputNodeMap and sourceOutputNodeMap");
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            sinkInputNodeMap_[sessionId]->RegisterWriteCallback(callback);
        }
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

size_t HpaeInnerCapturerManager::GetWritableSize(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::UpdateSpatializationState(
    uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::UpdateMaxLength(uint32_t sessionId, uint32_t maxLength)
{
    return SUCCESS;
}

std::vector<SinkInput> HpaeInnerCapturerManager::GetAllSinkInputsInfo()
{
    std::vector<SinkInput> sinkInputs;
    return sinkInputs;
}

int32_t HpaeInnerCapturerManager::GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo)
{
    if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
        return ERR_INVALID_OPERATION;
    }
    sinkInputInfo.nodeInfo = sinkInputNodeMap_[sessionId]->GetNodeInfo();
    sinkInputInfo.rendererSessionInfo = rendererSessionNodeMap_[sessionId];
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::RefreshProcessClusterByDevice()
{
    return SUCCESS;
}

HpaeSinkInfo HpaeInnerCapturerManager::GetSinkInfo()
{
    return sinkInfo_;
}

void HpaeInnerCapturerManager::OnFadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::OnFadeDone");
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Fade done, not find sessionId %{public}u", sessionId);
        DisConnectRendererInputSessionInner(sessionId);
        IOperation operation = sinkInputNodeMap_[sessionId]->GetState() == HPAE_SESSION_STOPPING ?
            OPERATION_STOPPED : OPERATION_PAUSED;
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
        SetSessionStateForRenderer(sessionId, state);
        sinkInputNodeMap_[sessionId]->SetState(state);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
    };
    SendRequestInner(request, __func__);
}

void HpaeInnerCapturerManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), "no find sessionId in sinkInputNodeMap");
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId,
        rendererSessionNodeMap_[sessionId].state, operation);
}

int32_t HpaeInnerCapturerManager::RegisterReadCallback(uint32_t sessionId,
    const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId), "no find sessionId in sourceOutputNodeMap");
        AUDIO_INFO_LOG("RegisterReadCallback sessionId %{public}u", sessionId);
        sourceOutputNodeMap_[sessionId]->RegisterReadCallback(callback);
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo)
{
    if (!SafeGetMap(sourceOutputNodeMap_, sessionId)) {
        return ERR_INVALID_OPERATION;
    }
    sourceOutputInfo.nodeInfo = sourceOutputNodeMap_[sessionId]->GetNodeInfo();
    sourceOutputInfo.capturerSessionInfo = capturerSessionNodeMap_[sessionId];
    return SUCCESS;
}

std::vector<SourceOutput> HpaeInnerCapturerManager::GetAllSourceOutputsInfo()
{
    // to do
    std::vector<SourceOutput> sourceOutputs;
    return sourceOutputs;
}

int32_t HpaeInnerCapturerManager::CreateRendererInputSessionInner(const HpaeStreamInfo &streamInfo)
{
    Trace trace("[" + std::to_string(streamInfo.sessionId) +
        "]HpaeInnerCapturerManager::CreateRendererInputSessionInner");
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = streamInfo.channels;
    nodeInfo.format = streamInfo.format;
    nodeInfo.frameLen = streamInfo.frameLen;
    nodeInfo.nodeId = GetSinkInputNodeIdInner();
    nodeInfo.streamType = streamInfo.streamType;
    nodeInfo.sessionId = streamInfo.sessionId;
    nodeInfo.samplingRate = (AudioSamplingRate)streamInfo.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    AUDIO_INFO_LOG("nodeInfo.channels %{public}d, nodeInfo.format %{public}hhu, nodeInfo.frameLen %{public}d",
        nodeInfo.channels, nodeInfo.format, nodeInfo.frameLen);
    sinkInputNodeMap_[streamInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);

    if (!SafeGetMap(rendererSceneClusterMap_, nodeInfo.sceneType)) {
        rendererSceneClusterMap_[nodeInfo.sceneType] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo_);
        if (rendererSceneClusterMap_[nodeInfo.sceneType]->SetupAudioLimiter() != SUCCESS) {
            AUDIO_ERR_LOG("SetupAudioLimiter failed, sessionId %{public}u", nodeInfo.sessionId);
        }
    }
    sceneTypeToProcessClusterCount_++;
    // todo change nodeInfo
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::CreateCapturerInputSessionInner(const HpaeStreamInfo &streamInfo)
{
    Trace trace("[" + std::to_string(streamInfo.sessionId) +
        "]HpaeInnerCapturerManager::CreateCapturerInputSessionInner");
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = streamInfo.channels;
    nodeInfo.format = streamInfo.format;
    nodeInfo.frameLen = streamInfo.frameLen;
    nodeInfo.streamType = streamInfo.streamType;
    nodeInfo.sessionId = streamInfo.sessionId;
    nodeInfo.samplingRate = (AudioSamplingRate)streamInfo.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    nodeInfo.sourceType = streamInfo.sourceType;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    AUDIO_INFO_LOG("nodeInfo.channels %{public}d, nodeInfo.format %{public}hhu, nodeInfo.frameLen %{public}d",
        nodeInfo.channels, nodeInfo.format, nodeInfo.frameLen);
    sourceOutputNodeMap_[streamInfo.sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    HpaeNodeInfo outputNodeInfo = hpaeInnerCapSinkNode_->GetNodeInfo();
    // todo change nodeInfo
    capturerAudioFormatConverterNodeMap_[streamInfo.sessionId] =
        std::make_shared<HpaeAudioFormatConverterNode>(outputNodeInfo, nodeInfo);
    capturerAudioFormatConverterNodeMap_[streamInfo.sessionId]->SetSourceNode(true);
    capturerSessionNodeMap_[streamInfo.sessionId].sceneType = nodeInfo.sceneType;
    return SUCCESS;
}

void HpaeInnerCapturerManager::DeleteRendererInputNodeSession(const std::shared_ptr<HpaeSinkInputNode> &sinkInputNode)
{
    HpaeProcessorType sceneType = sinkInputNode->GetSceneType();
    if (SafeGetMap(rendererSceneClusterMap_, sceneType)) {
        rendererSceneClusterMap_[sceneType]->DisConnect(sinkInputNode);
        sceneTypeToProcessClusterCount_--;
        uint32_t sessionId = sinkInputNode->GetSessionId();
        if (sceneTypeToProcessClusterCount_ == 0) {
            AUDIO_INFO_LOG("need to erase rendererSceneCluster, last stream: %{public}u", sessionId);
        } else {
            AUDIO_INFO_LOG("%{public}u is not last stream, no need erase rendererSceneCluster", sessionId);
        }
    }
}

int32_t HpaeInnerCapturerManager::DeleteRendererInputSessionInner(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::DeleteRendererInputSessionInner");
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), SUCCESS,
        "sessionId %{public}u can not find in sinkInputNodeMap_.", sessionId);
    DeleteRendererInputNodeSession(sinkInputNodeMap_[sessionId]);
    sinkInputNodeMap_.erase(sessionId);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::DeleteCapturerInputSessionInner(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::DeleteCapturerInputSessionInner");
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId), SUCCESS,
        "sessionId %{public}u can not find in sourceOutputNodeMap_.", sessionId);
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(capturerAudioFormatConverterNodeMap_, sessionId), SUCCESS,
        "sessionId %{public}u can not find in capturerAudioFormatConverterNodeMap_.", sessionId);
    // no need process cluster
    sourceOutputNodeMap_[sessionId]->DisConnect(capturerAudioFormatConverterNodeMap_[sessionId]);
    capturerAudioFormatConverterNodeMap_[sessionId]->DisConnect(hpaeInnerCapSinkNode_);
    if (hpaeInnerCapSinkNode_->GetOutputPortNum() == 0) {
        hpaeInnerCapSinkNode_->InnerCapturerSinkStop();
    }
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::ConnectRendererInputSessionInner(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), ERR_INVALID_PARAM,
        "sessionId %{public}u can not find in sinkInputNodeMap_.", sessionId);
    CHECK_AND_RETURN_RET_LOG(sinkInputNodeMap_[sessionId]->GetState() == HPAE_SESSION_RUNNING, SUCCESS,
        "sink input node is running");
    HpaeProcessorType sceneType = sinkInputNodeMap_[sessionId]->GetSceneType();
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(rendererSceneClusterMap_, sceneType), SUCCESS,
        "miss corresponding process cluster for scene type %{public}d", sceneType);
    // todo check if connect process cluster
    hpaeInnerCapSinkNode_->Connect(rendererSceneClusterMap_[sceneType]);
    rendererSceneClusterMap_[sceneType]->Connect(sinkInputNodeMap_[sessionId]);
    rendererSceneClusterMap_[sceneType]->SetLoudnessGain(sessionId, sinkInputNodeMap_[sessionId]->GetLoudnessGain());
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::ConnectCapturerOutputSessionInner(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId), ERR_INVALID_PARAM,
        "sessionId %{public}u can not find in sourceOutputCLusterMap.", sessionId);
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(capturerAudioFormatConverterNodeMap_, sessionId),
        ERR_INVALID_PARAM,
        "sessionId %{public}u can not find in capturerAudioFormatConverterNodeMap_.", sessionId);
    // todo connect gain node
    capturerAudioFormatConverterNodeMap_[sessionId]->Connect(hpaeInnerCapSinkNode_);
    sourceOutputNodeMap_[sessionId]->Connect(capturerAudioFormatConverterNodeMap_[sessionId]);
    return SUCCESS;
}

void HpaeInnerCapturerManager::OnDisConnectProcessCluster(HpaeProcessorType sceneType)
{
    auto request = [this, sceneType]() {
        AUDIO_INFO_LOG("mixerNode trigger callback");
        if (SafeGetMap(rendererSceneClusterMap_, sceneType) &&
            rendererSceneClusterMap_[sceneType]->GetPreOutNum() == 0) {
            rendererSceneClusterMap_[sceneType]->DisConnectMixerNode();
            hpaeInnerCapSinkNode_->DisConnect(rendererSceneClusterMap_[sceneType]);
        }

        if (sceneTypeToProcessClusterCount_ == 0) {
            rendererSceneClusterMap_.erase(sceneType);
            AUDIO_INFO_LOG("erase rendererSceneCluster sceneType[%{public}d]", sceneType);
        } else {
            AUDIO_INFO_LOG("no need erase rendererSceneCluster sceneType[%{public}d]", sceneType);
        }
    };
    SendRequestInner(request, __func__);
}

int32_t HpaeInnerCapturerManager::DisConnectRendererInputSessionInner(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), SUCCESS,
        "sessionId %{public}u can not find in sinkInputNodeMap_.", sessionId);
    HpaeProcessorType sceneType = sinkInputNodeMap_[sessionId]->GetSceneType();
    if (SafeGetMap(rendererSceneClusterMap_, sceneType)) {
        rendererSceneClusterMap_[sceneType]->DisConnect(sinkInputNodeMap_[sessionId]);
    }
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::DisConnectCapturerInputSessionInner(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId), SUCCESS,
        "sessionId %{public}u can not find in sourceOutputNodeMap_.", sessionId);
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(capturerAudioFormatConverterNodeMap_, sessionId), SUCCESS,
        "sessionId %{public}u can not find in capturerAudioFormatConverterNodeMap_.", sessionId);
    sourceOutputNodeMap_[sessionId]->DisConnect(capturerAudioFormatConverterNodeMap_[sessionId]);
    capturerAudioFormatConverterNodeMap_[sessionId]->DisConnect(hpaeInnerCapSinkNode_);
    if (hpaeInnerCapSinkNode_->GetOutputPortNum() == 0) {
        hpaeInnerCapSinkNode_->InnerCapturerSinkStop();
    }
    return SUCCESS;
}

void HpaeInnerCapturerManager::SetSessionStateForRenderer(uint32_t sessionId, HpaeSessionState renderState)
{
    rendererSessionNodeMap_[sessionId].state = renderState;
}

void HpaeInnerCapturerManager::SetSessionStateForCapturer(uint32_t sessionId, HpaeSessionState capturerState)
{
    capturerSessionNodeMap_[sessionId].state = capturerState;
}

void HpaeInnerCapturerManager::SendRequestInner(Request &&request, const std::string &funcName, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_INFO_LOG("not init, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_INNER_CAPTURE_MANAGER_TYPE,
            funcName, "HpaeInnerCapturerManager not init");
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    if (hpaeSignalProcessThread_ == nullptr) {
        AUDIO_INFO_LOG("hpaeSignalProcessThread_  inner capturer sink is nullptr, %{public}s excute failed",
            funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_INNER_CAPTURE_MANAGER_TYPE, funcName,
            "thread is nullptr");
        return;
    }
    hpaeSignalProcessThread_->Notify();
}

uint32_t HpaeInnerCapturerManager::GetSinkInputNodeIdInner()
{
    return sinkInputNodeCounter_++;
}

std::string HpaeInnerCapturerManager::GetThreadName()
{
    if (sinkInfo_.deviceName == "RemoteCastInnerCapturer") {
        AUDIO_INFO_LOG("threadName is RemoteCast");
        return "RemoteCast";
    }
    std::string threadName = "InnerCap";
    for (auto ch : sinkInfo_.deviceName) {
        if (ch >= '0' && ch <= '9') {
            threadName += ch;
        }
    }
    AUDIO_INFO_LOG("threadName change to %{public}s", threadName.c_str());
    return threadName;
}

std::string HpaeInnerCapturerManager::GetDeviceHDFDumpInfo()
{
    std::string config;
    TransDeviceInfoToString(sinkInfo_, config);
    return config;
}

int32_t HpaeInnerCapturerManager::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    auto request = [this, sessionId, loudnessGain]() {
        AUDIO_INFO_LOG("set loudnessGain %{public}f to sessionId %{public}d", loudnessGain, sessionId);
        std::shared_ptr<HpaeSinkInputNode> sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(sinkInputNode != nullptr,
            "session with Id %{public}d not in sinkInputNodeMap_", sessionId);
        sinkInputNode->SetLoudnessGain(loudnessGain);

        HpaeProcessorType processorType = sinkInputNodeMap_[sessionId]->GetSceneType();
        std::shared_ptr<HpaeProcessCluster> processCluster = SafeGetMap(rendererSceneClusterMap_, processorType);
        CHECK_AND_RETURN_LOG(processCluster != nullptr,
            "processCluster with sceneType %{public}d not exists", processorType);
        processCluster->SetLoudnessGain(sessionId, loudnessGain);
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::DumpSinkInfo()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "HpaeInnerCapturerManager not init");
    auto request = [this]() {
        AUDIO_INFO_LOG("DumpSinkInfo deviceName %{public}s", sinkInfo_.deviceName.c_str());
        UploadDumpSinkInfo(sinkInfo_.deviceName);
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

int32_t HpaeInnerCapturerManager::SetOffloadPolicy(uint32_t sessionId, int32_t state)
{
    auto request = [this, sessionId, state]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::SetOffloadPolicy");
        AUDIO_INFO_LOG("SetOffloadPolicy sessionId %{public}u, deviceName %{public}s, state %{public}d", sessionId,
            sinkInfo_.deviceName.c_str(), state);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), "not find sessionId %{public}u", sessionId);
        sinkInputNodeMap_[sessionId]->SetOffloadEnabled(state != OFFLOAD_DEFAULT);
    };
    SendRequestInner(request, __func__);
    return SUCCESS;
}

void HpaeInnerCapturerManager::SetSpeed(uint32_t sessionId, float speed)
{
    auto request = [this, sessionId, speed]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeInnerCapturerManager::SetSpeed");
        AUDIO_INFO_LOG("SetSpeed sessionId %{public}u, deviceName %{public}s, speed %{public}f", sessionId,
            sinkInfo_.deviceName.c_str(), speed);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), "not find sessionId %{public}u", sessionId);
        sinkInputNodeMap_[sessionId]->SetSpeed(speed);
    };
    SendRequestInner(request, __func__);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS