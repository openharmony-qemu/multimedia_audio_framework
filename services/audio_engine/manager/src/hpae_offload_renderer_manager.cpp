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
#define LOG_TAG "HpaeOffloadRendererManager"
#endif

#include "hpae_offload_renderer_manager.h"
#include "audio_stream_info.h"
#include "audio_errors.h"
#include "audio_engine_log.h"
#include "hpae_node_common.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t HISTORY_INTERVAL_S = 7;  // 7s buffer for rewind
}

HpaeOffloadRendererManager::HpaeOffloadRendererManager(HpaeSinkInfo &sinkInfo)
    : hpaeNoLockQueue_(CURRENT_REQUEST_COUNT), sinkInfo_(sinkInfo)
{}

HpaeOffloadRendererManager::~HpaeOffloadRendererManager()
{
    AUDIO_INFO_LOG("destructor offload renderer");
    if (isInit_.load()) {
        DeInit();
    }
}

// private method
int32_t HpaeOffloadRendererManager::CreateInputSession(const HpaeStreamInfo &streamInfo)
{
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = streamInfo.channels;
    nodeInfo.format = streamInfo.format;
    nodeInfo.frameLen = streamInfo.frameLen;
    nodeInfo.streamType = streamInfo.streamType;
    nodeInfo.sessionId = streamInfo.sessionId;
    nodeInfo.samplingRate = static_cast<AudioSamplingRate>(streamInfo.samplingRate);
    nodeInfo.sceneType = TransStreamTypeToSceneType(streamInfo.streamType);
    nodeInfo.historyFrameCount = HISTORY_INTERVAL_S * nodeInfo.samplingRate / nodeInfo.frameLen;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.nodeId = OnGetNodeId();
    nodeInfo.nodeName = "HpaeSinkInputNode";
    sinkInputNode_ = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode_->SetAppUid(streamInfo.uid);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    auto request = [this, node]() { AddSingleNodeToSink(node); };
    SendRequest(request);
    return SUCCESS;
}

void HpaeOffloadRendererManager::AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect)
{
    HpaeNodeInfo nodeInfo = node->GetNodeInfo();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    // 7s history buffer to rewind
    nodeInfo.historyFrameCount = HISTORY_INTERVAL_S * nodeInfo.samplingRate / nodeInfo.frameLen;
    nodeInfo.nodeId = OnGetNodeId();
    nodeInfo.statusCallback = weak_from_this();
    node->SetNodeInfo(nodeInfo);
    uint32_t sessionId = nodeInfo.sessionId;
    AUDIO_INFO_LOG("[FinishMove] session:%{public}u to sink:offload", sessionId);
    sinkInputNode_ = node;
    sessionInfo_.state = node->GetState();

    if (!isConnect || sinkInputNode_->GetState() != HPAE_SESSION_RUNNING) {
        AUDIO_INFO_LOG("[FinishMove] session:%{public}u not need connect session", sessionId);
        return;
    }

    if (node->GetState() == HPAE_SESSION_RUNNING) {
        AUDIO_INFO_LOG("[FinishMove] session:%{public}u connect to sink:offload", sessionId);
        ConnectInputSession();
        if (sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING && !isSuspend_) {
            sinkOutputNode_->RenderSinkStart();
        }
    }
}

int32_t HpaeOffloadRendererManager::AddAllNodesToSink(
    const std::vector<std::shared_ptr<HpaeSinkInputNode>> &sinkInputs, bool isConnect)
{
    auto request = [this, sinkInputs, isConnect]() {
        for (const auto &it : sinkInputs) {
            AddSingleNodeToSink(it, isConnect);
        }
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, streamInfo]() {
        CreateInputSession(streamInfo);
        sessionInfo_.state = HPAE_SESSION_PREPARED;
        sinkInputNode_->SetState(HPAE_SESSION_PREPARED);
    };
    SendRequest(request);
    return SUCCESS;
}

void HpaeOffloadRendererManager::DeleteInputSession()
{
    DisConnectInputSession();
    if (sinkInputNode_->GetState() == HPAE_SESSION_RUNNING) {
        sinkOutputNode_->StopStream();
    }
    sinkInputNode_ = nullptr;
    formatConverterNode_ = nullptr;
}

int32_t HpaeOffloadRendererManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(sinkInputNode_ && sessionId == sinkInputNode_->GetSessionId(),
            "DestroyStream not find sessionId %{public}u",
            sessionId);
        AUDIO_INFO_LOG("DestroyStream sessionId %{public}u", sessionId);
        DeleteInputSession();
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::ConnectInputSession()
{
    if (sinkInputNode_->GetState() != HPAE_SESSION_RUNNING) {
        return SUCCESS;
    }

    if (CheckHpaeNodeInfoIsSame(sinkInputNode_->GetNodeInfo(), sinkOutputNode_->GetNodeInfo())) {
        formatConverterNode_ = nullptr;
        sinkOutputNode_->Connect(sinkInputNode_);
        OnNotifyDfxNodeInfo(true, sinkOutputNode_->GetNodeId(), sinkInputNode_->GetNodeInfo());
    } else {
        HpaeNodeInfo nodeInfo = sinkOutputNode_->GetNodeInfo();
        nodeInfo.nodeName = "HpaeAudioFormatConverterNode";
        nodeInfo.nodeId = OnGetNodeId();
        nodeInfo.sessionId = sinkInputNode_->GetSessionId();
        formatConverterNode_ = std::make_shared<HpaeAudioFormatConverterNode>(
            sinkInputNode_->GetNodeInfo(), nodeInfo);
        formatConverterNode_->Connect(sinkInputNode_);
        sinkOutputNode_->Connect(formatConverterNode_);
        OnNotifyDfxNodeInfo(true, sinkOutputNode_->GetNodeId(), formatConverterNode_->GetNodeInfo());
        OnNotifyDfxNodeInfo(true, formatConverterNode_->GetNodeId(), sinkInputNode_->GetNodeInfo());
    }
    // single stream manager
    HpaeNodeInfo nodeInfo = sinkOutputNode_->GetNodeInfo();
    nodeInfo.sessionId = sinkInputNode_->GetSessionId();
    nodeInfo.streamType = sinkInputNode_->GetStreamType();
    sinkOutputNode_->SetNodeInfo(nodeInfo);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(sinkInputNode_ &&
            sessionId == sinkInputNode_->GetSessionId(), "Start not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Start sessionId %{public}u", sessionId);
        sinkInputNode_->SetState(HPAE_SESSION_RUNNING);
        ConnectInputSession();
        if (sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING && !isSuspend_) {
            sinkOutputNode_->RenderSinkStart();
        }
        sessionInfo_.state = HPAE_SESSION_RUNNING;
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, sessionInfo_.state, OPERATION_STARTED);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::DisConnectInputSession()
{
    if (formatConverterNode_ == nullptr) {
        sinkOutputNode_->DisConnect(sinkInputNode_);
        OnNotifyDfxNodeInfo(false, sinkOutputNode_->GetNodeId(), sinkInputNode_->GetNodeInfo());
    } else {
        formatConverterNode_->DisConnect(sinkInputNode_);
        sinkOutputNode_->DisConnect(formatConverterNode_);
        OnNotifyDfxNodeInfo(false, formatConverterNode_->GetNodeId(), sinkInputNode_->GetNodeInfo());
        OnNotifyDfxNodeInfo(false, sinkOutputNode_->GetNodeId(), formatConverterNode_->GetNodeInfo());
    }
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Pause(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(sinkInputNode_ &&
            sessionId == sinkInputNode_->GetSessionId(), "Pause not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Pause sessionId %{public}u", sessionId);
        DisConnectInputSession();
        auto state = sinkInputNode_->GetState();
        sinkInputNode_->SetState(HPAE_SESSION_PAUSED);
        if (state == HPAE_SESSION_RUNNING) {
            sinkOutputNode_->StopStream();
        }
        sessionInfo_.state = HPAE_SESSION_PAUSED;
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, sessionInfo_.state, OPERATION_PAUSED);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Flush(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(sinkInputNode_ &&
            sessionId == sinkInputNode_->GetSessionId(), "Pause not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Flush sessionId %{public}u", sessionId);
        // flush history buffer
        sinkInputNode_->Flush();
        // flush sinkoutput cache
        sinkOutputNode_->FlushStream();
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, sessionInfo_.state, OPERATION_FLUSHED);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Drain(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(sinkInputNode_ &&
            sessionId == sinkInputNode_->GetSessionId(), "Drain not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Drain sessionId %{public}u", sessionId);
        sinkInputNode_->Drain();
        if (sessionInfo_.state != HPAE_SESSION_RUNNING) {
            TriggerCallback(
                UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, sessionInfo_.state, OPERATION_DRAINED);
        }
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        CHECK_AND_RETURN_LOG(sinkInputNode_ &&
            sessionId == sinkInputNode_->GetSessionId(), "Stop not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Stop sessionId %{public}u", sessionId);
        DisConnectInputSession();
        auto state = sinkInputNode_->GetState();
        sinkInputNode_->SetState(HPAE_SESSION_STOPPED);
        sessionInfo_.state = HPAE_SESSION_STOPPED;
        if (state == HPAE_SESSION_RUNNING) {
            sinkOutputNode_->StopStream();
        }
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, sessionInfo_.state, OPERATION_STOPPED);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Release(uint32_t sessionId)
{
    return DestroyStream(sessionId);
}

void HpaeOffloadRendererManager::MoveAllStreamToNewSink(const std::string &sinkName,
    const std::vector<uint32_t>& moveIds, MoveSessionType moveType)
{
    std::string name = sinkName;
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;
    if (sinkInputNode_) {
        uint32_t sessionId = sinkInputNode_->GetSessionId();
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), sessionId) != moveIds.end()) {
            sinkInputs.emplace_back(sinkInputNode_);
            DeleteInputSession();
            AUDIO_INFO_LOG("[StartMove] session: %{public}u,sink [offload] --> [%{public}s]",
                sessionId, sinkName.c_str());
        }
    }
    if (sinkInputs.size() == 0) {
        AUDIO_WARNING_LOG("sink count is 0,no need move session");
    }
    TriggerCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
}

int32_t HpaeOffloadRendererManager::MoveAllStream(const std::string &sinkName, const std::vector<uint32_t>& sessionIds,
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
        SendRequest(request);
    }
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::MoveStream(uint32_t sessionId, const std::string &sinkName)
{
    auto request = [this, sessionId, sinkName]() {
        if (sinkInputNode_ == nullptr || sessionId != sinkInputNode_->GetSessionId()) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}d failed,sink [offload] --> [%{public}s]",
                sessionId, sinkName.c_str());
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
            return;
        }

        if (sinkName.empty()) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,sinkName is empty", sessionId);
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
            return;
        }

        std::shared_ptr<HpaeSinkInputNode> inputNode = sinkInputNode_;
        AUDIO_INFO_LOG("move session:%{public}d,sink [offload] --> [%{public}s]", sessionId, sinkName.c_str());
        DeleteInputSession();
        std::string name = sinkName;
        TriggerCallback(MOVE_SINK_INPUT, inputNode, name);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::SuspendStreamManager(bool isSuspend)
{
    auto request = [this, isSuspend]() {
        if (isSuspend_ == isSuspend) {
            return;
        }
        isSuspend_ = isSuspend;
        if (isSuspend_) {
            sinkOutputNode_->RenderSinkStop();
        } else if (sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING && sinkInputNode_ &&
            sinkInputNode_->GetState() == HPAE_SESSION_RUNNING) {
            sinkOutputNode_->RenderSinkStart();
        }
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::SetMute(bool isMute)
{
    auto request = [this, isMute]() {
        isMute_ = isMute;  // todo: set to sinkoutputnode
    };
    SendRequest(request);
    return SUCCESS;
}

void HpaeOffloadRendererManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

int32_t HpaeOffloadRendererManager::ReloadRenderManager(const HpaeSinkInfo &sinkInfo)
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, sinkInfo]() {
        sinkInfo_ = sinkInfo;
        InitSinkInner();
    };
    SendRequest(request, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Init()
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this] {
        InitSinkInner();
    };
    SendRequest(request, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

void HpaeOffloadRendererManager::InitSinkInner()
{
    AUDIO_INFO_LOG("HpaeOffloadRendererManager::init");
    HpaeNodeInfo nodeInfo;
    nodeInfo.channels = sinkInfo_.channels;
    nodeInfo.format = sinkInfo_.format;
    nodeInfo.frameLen = sinkInfo_.frameLen;
    nodeInfo.nodeId = 0;
    nodeInfo.samplingRate = sinkInfo_.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_OUT;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.nodeName = "HpaeOffloadSinkOutputNode";
    nodeInfo.nodeId = OnGetNodeId();
    sinkOutputNode_ = std::make_unique<HpaeOffloadSinkOutputNode>(nodeInfo);
    OnNotifyDfxNodeInfo(true, 0, nodeInfo);
    sinkOutputNode_->SetTimeoutStopThd(sinkInfo_.suspendTime);
    // if failed, RenderSinkInit will failed either, so no need to deal ret
    AUDIO_INFO_LOG("HpaeOffloadRendererManager::GetRenderSinkInstance");
    sinkOutputNode_->GetRenderSinkInstance(sinkInfo_.deviceClass, sinkInfo_.deviceNetId);
    IAudioSinkAttr attr;
    attr.adapterName = sinkInfo_.adapterName.c_str();
    attr.sampleRate = sinkInfo_.samplingRate;
    attr.channel = sinkInfo_.channels;
    attr.format = sinkInfo_.format;
    attr.channelLayout = sinkInfo_.channelLayout;
    attr.deviceType = sinkInfo_.deviceType;
    attr.volume = sinkInfo_.volume;
    attr.openMicSpeaker = sinkInfo_.openMicSpeaker;
    attr.deviceNetworkId = sinkInfo_.deviceNetId.c_str();
    attr.filePath = sinkInfo_.filePath.c_str();
    int32_t ret = sinkOutputNode_->RenderSinkInit(attr);
    isInit_.store(true);
    TriggerCallback(INIT_DEVICE_RESULT, sinkInfo_.deviceName, ret);
    AUDIO_INFO_LOG("HpaeOffloadRendererManager::inited");
}

bool HpaeOffloadRendererManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

int32_t HpaeOffloadRendererManager::DeInit(bool isMoveDefault)
{
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
    sinkOutputNode_->RenderSinkStop();
    int32_t ret = sinkOutputNode_->RenderSinkDeInit();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RenderSinkDeInit error, ret %{public}d.", ret);
    sinkOutputNode_->ResetAll();
    isInit_.store(false);
    return SUCCESS;
}

bool HpaeOffloadRendererManager::IsInit()
{
    return isInit_.load();
}

bool HpaeOffloadRendererManager::IsRunning(void)
{
    if (sinkOutputNode_ != nullptr && hpaeSignalProcessThread_ != nullptr) {
        return sinkOutputNode_->GetSinkState() == STREAM_MANAGER_RUNNING && hpaeSignalProcessThread_->IsRunning();
    }
    return false;
}

bool HpaeOffloadRendererManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

int32_t HpaeOffloadRendererManager::SetClientVolume(uint32_t sessionId, float volume)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::SetRate(uint32_t sessionId, int32_t rate)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::SetAudioEffectMode(uint32_t sessionId, int32_t effectMode)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::SetPrivacyType(uint32_t sessionId, int32_t privacyType)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::GetPrivacyType(uint32_t sessionId, int32_t &privacyType)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::RegisterWriteCallback(
    uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        CHECK_AND_RETURN_LOG(sessionId == sinkInputNode_->GetSessionId(),
            "RegisterWriteCallback not find sessionId %{public}u",
            sessionId);
        sinkInputNode_->RegisterWriteCallback(callback);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::RegisterReadCallback(
    uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    return ERR_NOT_SUPPORTED;
}

void HpaeOffloadRendererManager::Process()
{
    if (sinkOutputNode_ != nullptr && IsRunning()) {
        UpdateAppsUid();
        sinkOutputNode_->DoProcess();
    }
}

void HpaeOffloadRendererManager::UpdateAppsUid()
{
    appsUid_.clear();
    if (sinkInputNode_ != nullptr && sinkInputNode_->GetState() == HPAE_SESSION_RUNNING) {
        appsUid_.emplace_back(sinkInputNode_->GetAppUid());
    }
    sinkOutputNode_->UpdateAppsUid(appsUid_);
}

int32_t HpaeOffloadRendererManager::SetOffloadPolicy(uint32_t sessionId, int32_t state)
{
    auto request = [this, sessionId, state]() {
        CHECK_AND_RETURN_LOG(sinkInputNode_ && sessionId == sinkInputNode_->GetSessionId(),
            "SetOffloadPolicy not find sessionId %{public}u",
            sessionId);
        sinkInputNode_->SetOffloadEnabled(state != OFFLOAD_DEFAULT);
        // OFFLOAD_DEFAULT do not need set buffersize
        if (state != OFFLOAD_DEFAULT && sinkOutputNode_) {
            sinkOutputNode_->SetPolicyState(state);
        }
    };
    SendRequest(request);
    return SUCCESS;
}

size_t HpaeOffloadRendererManager::GetWritableSize(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::UpdateSpatializationState(
    uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::UpdateMaxLength(uint32_t sessionId, uint32_t maxLength)
{
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::SetOffloadRenderCallbackType(uint32_t sessionId, int32_t type)
{
    auto request = [this, sessionId, type]() {
        CHECK_AND_RETURN_LOG(sinkInputNode_ && sessionId == sinkInputNode_->GetSessionId(),
            "SetOffloadRenderCallbackType not find sessionId %{public}u",
            sessionId);
        if (sinkOutputNode_) {
            sinkOutputNode_->SetOffloadRenderCallbackType(type);
        }
    };
    SendRequest(request);
    return SUCCESS;
}

std::vector<SinkInput> HpaeOffloadRendererManager::GetAllSinkInputsInfo()
{
    std::vector<SinkInput> sinkInputs;
    return sinkInputs;
}

int32_t HpaeOffloadRendererManager::GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo)
{
    CHECK_AND_RETURN_RET_LOG(sinkInputNode_ && sessionId == sinkInputNode_->GetSessionId(),
        ERR_INVALID_OPERATION,
        "RegisterWriteCallback not find sessionId %{public}u",
        sessionId);
    sinkInputInfo.nodeInfo = sinkInputNode_->GetNodeInfo();
    sinkInputInfo.rendererSessionInfo = sessionInfo_;
    return SUCCESS;
}

HpaeSinkInfo HpaeOffloadRendererManager::GetSinkInfo()
{
    return sinkInfo_;
}

void HpaeOffloadRendererManager::SendRequest(Request &&request, bool isInit)
{
    CHECK_AND_RETURN_LOG(isInit || IsInit(), "HpaeOffloadRendererManager not init");
    hpaeNoLockQueue_.PushRequest(std::move(request));
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ offloadrenderer is nullptr");
    hpaeSignalProcessThread_->Notify();
}

void HpaeOffloadRendererManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, sessionInfo_.state, operation);
}

void HpaeOffloadRendererManager::OnRequestLatency(uint32_t sessionId, uint64_t &latency)
{
    latency = sinkOutputNode_->GetLatency();
}

void HpaeOffloadRendererManager::OnRewindAndFlush(uint64_t rewindTime)
{
    sinkInputNode_->RewindHistoryBuffer(rewindTime);
}

void HpaeOffloadRendererManager::OnNotifyQueue()
{
    hpaeSignalProcessThread_->Notify();
}

std::string HpaeOffloadRendererManager::GetThreadName()
{
    return sinkInfo_.deviceName;
}

void HpaeOffloadRendererManager::DumpSinkInfo()
{
    auto request = [this]() {
        AUDIO_INFO_LOG("DumpSinkInfo deviceName %{public}s", sinkInfo_.deviceName.c_str());
        UploadDumpSinkInfo(sinkInfo_.deviceName);
    };
    SendRequest(request);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS