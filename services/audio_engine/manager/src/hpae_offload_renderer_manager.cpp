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
#include <cinttypes>
#include "audio_stream_info.h"
#include "audio_errors.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"
#include "hpae_message_queue_monitor.h"
#include "hpae_stream_move_monitor.h"
#include "audio_utils.h"
#include "hpae_define.h"
#include "stream_dfx_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
constexpr uint32_t HISTORY_INTERVAL_S = 7;  // 7s buffer for rewind
const std::string REMOTE_OFFLOAD_DEVICE_CLASS = "remote_offload";
}

HpaeOffloadRendererManager::HpaeOffloadRendererManager(HpaeSinkInfo &sinkInfo)
    : hpaeNoLockQueue_(CURRENT_REQUEST_COUNT), sinkInfo_(sinkInfo)
{
    HpaeNodeInfo nodeInfo;
    renderNoneEffectNode_ = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
}

HpaeOffloadRendererManager::~HpaeOffloadRendererManager()
{
    AUDIO_INFO_LOG("destructor");
    if (isInit_.load()) {
        DeInit();
    }
}

// private method
std::shared_ptr<HpaeSinkInputNode> HpaeOffloadRendererManager::CreateInputSession(const HpaeStreamInfo &streamInfo)
{
    HpaeNodeInfo nodeInfo;
    ConfigNodeInfo(nodeInfo, streamInfo);
    nodeInfo.sceneType = TransStreamTypeToSceneType(streamInfo.streamType);
    nodeInfo.historyFrameCount = nodeInfo.frameLen ?
        HISTORY_INTERVAL_S * (nodeInfo.customSampleRate ? nodeInfo.customSampleRate : nodeInfo.samplingRate)
        / nodeInfo.frameLen : 0;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.effectInfo = streamInfo.effectInfo;
    AUDIO_INFO_LOG("streamType %{public}u, sessionId = %{public}u,channels:%{public}u,rate:%{public}u",
        nodeInfo.streamType, nodeInfo.sessionId, nodeInfo.channels,
        nodeInfo.customSampleRate == 0 ? nodeInfo.samplingRate : nodeInfo.customSampleRate);
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->SetAppUid(streamInfo.uid);
    AddNodeToMap(sinkInputNode);
    return sinkInputNode;
}

int32_t HpaeOffloadRendererManager::AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    auto request = [this, node]() { AddSingleNodeToSink(node); };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeOffloadRendererManager::AddNodeToMap(std::shared_ptr<HpaeSinkInputNode> node)
{
    sinkInputNodeMap_[node->GetSessionId()] = node;
    renderNoneEffectNode_->AudioOffloadRendererCreate(node->GetNodeInfo(), sinkInfo_);
    if (curNode_ == nullptr) {
        curNode_ = node;
        CreateOffloadNodes();
    }
}

void HpaeOffloadRendererManager::RemoveNodeFromMap(uint32_t sessionId)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    HpaeSessionState inputState = HPAE_SESSION_RELEASED;
    if (node != nullptr) {
        renderNoneEffectNode_->AudioOffloadRendererRelease(node->GetNodeInfo(), sinkInfo_);
        inputState = node->GetState();
#ifdef ENABLE_HIDUMP_DFX
        OnNotifyDfxNodeAdmin(false, node->GetNodeInfo());
#endif
    }
    RendererState state = inputState == HPAE_SESSION_RELEASED ? RENDERER_INVALID : RENDERER_RELEASED;
    NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, sessionId, state);
    sinkInputNodeMap_.erase(sessionId);
    if (curNode_ && curNode_->GetSessionId() == sessionId) {
        DestroyOffloadNodes();
        curNode_ = nullptr;
    }
}

void HpaeOffloadRendererManager::SetCurrentNode()
{
    if (curNode_ != nullptr) {
        AUDIO_WARNING_LOG("curNode_ in exist, no need to set");
        return;
    }
    // pick one node from sinkInputNodeMap_ and set to curNode_
    for (auto [_, node]: sinkInputNodeMap_) {
        curNode_ = node;
        CreateOffloadNodes();
        if (curNode_->GetState() == HPAE_SESSION_RUNNING) {
            ConnectInputSession();
        }
        break;
    }
    AUDIO_INFO_LOG("now curNode_ is [%{public}u]", curNode_ ? curNode_->GetSessionId() : 0);
}

void HpaeOffloadRendererManager::AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect)
{
    HpaeNodeInfo nodeInfo = node->GetNodeInfo();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    // 7s history buffer to rewind
    nodeInfo.historyFrameCount = HISTORY_INTERVAL_S * (nodeInfo.customSampleRate ?
        nodeInfo.customSampleRate : nodeInfo.samplingRate) / nodeInfo.frameLen;
    nodeInfo.statusCallback = weak_from_this();
    node->SetNodeInfo(nodeInfo);
    node->SetIsLowLatency(false);
    uint32_t sessionId = nodeInfo.sessionId;
    HILOG_COMM_INFO("[FinishMove] session:%{public}u to sink:offload", sessionId);
    AddNodeToMap(node);
    CHECK_AND_RETURN_LOG(sinkOutputNode_ != nullptr, "sinkOutputNode_ is nullptr");
    const auto outNodeInfo = sinkOutputNode_->GetNodeInfo();
    bool frameLenAdjusted = AdjustFrameLen(nodeInfo, outNodeInfo);
    if (frameLenAdjusted) {
        node->SetNodeInfo(nodeInfo);
        TriggerCallback(UPDATE_SPAN_SIZE, sessionId,
            static_cast<uint32_t>(nodeInfo.frameLen), HPAE_STREAM_CLASS_TYPE_PLAY);
    }
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(true, nodeInfo);
#endif
    if (!isConnect || node->GetState() != HPAE_SESSION_RUNNING) {
        AUDIO_INFO_LOG("[FinishMove] session:%{public}u not need connect session", sessionId);
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, sessionId, ConvertHpaeToRendererState(node->GetState()),
            node->GetAppUid());
        return;
    }

    if (node->GetState() == HPAE_SESSION_RUNNING && node->GetSessionId() == curNode_->GetSessionId()) {
        AUDIO_INFO_LOG("[FinishMove] session:%{public}u connect to sink:offload", sessionId);
        ConnectInputSession();
    }
    NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, sessionId, ConvertHpaeToRendererState(node->GetState()),
        node->GetAppUid());
    node->OnStreamInfoChange(false);
}

int32_t HpaeOffloadRendererManager::AddAllNodesToSink(
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

int32_t HpaeOffloadRendererManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t checkRet = CheckStreamInfo(streamInfo);
    if (checkRet != SUCCESS) {
        return checkRet;
    }
    auto request = [this, streamInfo]() {
        auto node = CreateInputSession(streamInfo);
        node->SetState(HPAE_SESSION_PREPARED);
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, streamInfo.sessionId, RENDERER_PREPARED,
            node->GetAppUid());
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeOffloadRendererManager::DeleteInputSession(SessionEffectMoveMode moveMode)
{
    // Invalidate any deferred Create so a stale retry cannot (re)materialize a node after teardown.
    CancelPendingSessionEffectCreate(sessionEffectSessionId_);
    ClearPendingSessionEffectClose();
    if (sessionEffectNode_ != nullptr) {
        sessionEffectNode_->DropEffect();  // Don't destroy algorithm — Lazy Release manages it
    }
    DisConnectInputSession();

    // Session Effect: clean up SE node. DisConnectInputSession already disconnected
    // the SE node from the chain (no bypass reconnect), so DestroyOffloadNodes
    // (called via RemoveNodeFromMap) will find a clean state.
    if (sessionEffectNode_ != nullptr) {
        uint32_t sid = sessionEffectSessionId_;
        HpaeSessionState state = curNode_ != nullptr ? curNode_->GetState() : HPAE_SESSION_NEW;
        sessionEffectNode_.reset();
        sessionEffectSessionId_ = 0;
        if (moveMode == SessionEffectMoveMode::FULL_TEARDOWN) {
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sid, state,
                OPERATION_SESSION_EFFECT_DISABLED);
        }
    }
    // Clear attach state on the SinkInputNode only for full teardown (not for move)
    if (curNode_ != nullptr && moveMode == SessionEffectMoveMode::FULL_TEARDOWN) {
        curNode_->ClearSessionEffectAttachState();
    }

    // Guard the direct dereferences below: curNode_ can be null here (previous session
    // removed via RemoveNodeFromMap while another entry stays in the map, see :107).
    CHECK_AND_RETURN_LOG(curNode_ != nullptr, "curNode_ not exist");

    if (curNode_->GetState() == HPAE_SESSION_RUNNING && sinkOutputNode_ != nullptr) {
        sinkOutputNode_->StopStream();
    }
    RemoveNodeFromMap(curNode_->GetSessionId());
}

int32_t HpaeOffloadRendererManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::DestroyStream");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "DestroyStream not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("DestroyStream sessionId %{public}u", sessionId);
        if (sessionId == curNode_->GetSessionId()) {
            DeleteInputSession();
            SetCurrentNode();
        } else {
            // Clear attach state for non-curNode stream being destroyed
            if (node->HasSessionEffectAttachState()) {
                node->ClearSessionEffectAttachState();
                TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, node->GetState(),
                    OPERATION_SESSION_EFFECT_DISABLED);
            }
            RemoveNodeFromMap(sessionId);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::CreateOffloadNodes()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(curNode_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::PLAY,
                BusinessScenario::CREATE, ERR_NULL_POINTER),
            "curNode_ not exist, fail to create offload nodes", false),
        "curNode_ not exist, fail to create offload nodes");
    HpaeNodeInfo outputNodeInfo = sinkOutputNode_->GetNodeInfo();
    outputNodeInfo.sessionId = curNode_->GetSessionId();
    outputNodeInfo.streamType = curNode_->GetStreamType();
    sinkOutputNode_->SetNodeInfo(outputNodeInfo);
 
    HpaeNodeInfo nodeInfo = sinkOutputNode_->GetNodeInfo();
    converterForOutput_ = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, outputNodeInfo);
    converterForOutput_->SetDownmixNormalization(false);
    loudnessGainNode_ = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    converterForLoudness_ = std::make_shared<HpaeAudioFormatConverterNode>(curNode_->GetNodeInfo(), nodeInfo);
    converterForLoudness_->SetDownmixNormalization(false);
    // Only create limiter node for remote_offload
    if (sinkInfo_.deviceClass == REMOTE_OFFLOAD_DEVICE_CLASS && curNode_->GetChannelCount() > STEREO) {
        limiterNode_ = std::make_shared<HpaeLimiterNode>(nodeInfo);
        // Setup AudioLimiter
        if (limiterNode_->SetupAudioLimiter() != SUCCESS) {
            AUDIO_WARNING_LOG("SessionId %{public}u, limiterNode_ SetupAudioLimiter failed!", outputNodeInfo.sessionId);
        }
        AUDIO_INFO_LOG("SessionId %{public}u, limiterNodeId %{public}u",
            outputNodeInfo.sessionId, limiterNode_->GetNodeId());
    }
    AUDIO_INFO_LOG("SessionId %{public}u, Success create offload nodes: "
        "converterForLoudnessId %{public}u, loudnessGainNodeId %{public}u,converterForOutputNodeId %{public}u",
        outputNodeInfo.sessionId, converterForLoudness_->GetNodeId(),
        loudnessGainNode_->GetNodeId(), converterForOutput_->GetNodeId());
    return SUCCESS;
}
 
int32_t HpaeOffloadRendererManager::DestroyOffloadNodes()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(curNode_ != nullptr && converterForLoudness_ != nullptr &&
        loudnessGainNode_ != nullptr && converterForOutput_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::PLAY,
                BusinessScenario::RELEASE, ERR_NULL_POINTER),
            "offload nodes not exist, fail to destroy offload nodes", false),
        "offload nodes not exist, fail to destroy offload nodes");
    AUDIO_INFO_LOG("SessionId %{public}u, Success destroy offload nodes: "
        "converterForLoudnessId %{public}u, loudnessGainNodeId %{public}u,limiterNodeId %{public}u,"
        " converterForOutputNodeId %{public}u ",
        curNode_->GetSessionId(), converterForLoudness_->GetNodeId(), loudnessGainNode_->GetNodeId(),
        limiterNode_ ? limiterNode_->GetNodeId() : 0, converterForOutput_->GetNodeId());
    converterForLoudness_ = nullptr;
    loudnessGainNode_ = nullptr;
    converterForOutput_ = nullptr;
    limiterNode_ = nullptr;
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::ConnectInputSession()
{
    if (curNode_->GetState() != HPAE_SESSION_RUNNING) {
        return SUCCESS;
    }
    sinkOutputNode_->SetSpeed(curNode_->GetSpeed());
    auto offloadType = curNode_->GetOffloadType();
    if (offloadType != OFFLOAD_DEFAULT) {
        sinkOutputNode_->SetPolicyState(offloadType);
    }
    // single stream manager
    sinkOutputNode_->Connect(converterForOutput_);
    // if there's no loudness algo, audio format will be converted to output device format at the first converternode
    if (limiterNode_) {
        converterForOutput_->Connect(limiterNode_);
        limiterNode_->Connect(loudnessGainNode_);
    } else {
        converterForOutput_->Connect(loudnessGainNode_);
    }
    // Session Effect: insert SE between loudnessGainNode_ and converterForLoudness_ when active
    uint32_t curSessionId = curNode_->GetSessionId();
    if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == curSessionId) {
        loudnessGainNode_->Connect(sessionEffectNode_);
        sessionEffectNode_->Connect(converterForLoudness_);
    } else {
        loudnessGainNode_->Connect(converterForLoudness_);
    }
    loudnessGainNode_->SetLoudnessGain(curNode_->GetLoudnessGain());

    // Materialize SE from attach state if curNode has it and physical node not yet created
    MaterializeSessionEffectFromAttachState(curSessionId);

    converterForLoudness_->Connect(curNode_);
    converterForLoudness_->RegisterCallback(this);
    renderNoneEffectNode_->AudioOffloadRendererStart(curNode_->GetNodeInfo(), sinkInfo_);
    if (sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING && !isSuspend_) {
        UpdateAppsUid();
        sinkOutputNode_->RenderSinkStart();
    }
    return SUCCESS;
}

void HpaeOffloadRendererManager::MaterializeSessionEffectFromAttachState(uint32_t curSessionId)
{
    // Only when the physical SE node has not been created yet.
    if (sessionEffectNode_ != nullptr) {
        return;
    }
    auto curSinkInput = SafeGetMap(sinkInputNodeMap_, curSessionId);
    if (curSinkInput == nullptr || !curSinkInput->HasSessionEffectAttachState()) {
        return;
    }
    auto stOpt = curSinkInput->GetSessionEffectAttachState();
    if (!stOpt.has_value() || !stOpt->enabled) {
        return;
    }
    auto inst = stOpt->instance.lock();
    HpaeSessionState state = curNode_->GetState();
    if (inst == nullptr) {
        curSinkInput->ClearSessionEffectAttachState();
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, curSessionId, state,
            OPERATION_SESSION_EFFECT_DISABLED);
        AUDIO_WARNING_LOG("ConnectInputSession: SE instance expired for session %{public}u, "
            "attach-state cleared", curSessionId);
        return;
    }
    sessionEffectSessionId_ = curSessionId;
    int32_t seRet = InsertSessionEffectNode(stOpt->effectName, inst, stOpt->config);
    if (seRet != static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        sessionEffectSessionId_ = 0;
        curSinkInput->ClearSessionEffectAttachState();
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, curSessionId, state,
            OPERATION_SESSION_EFFECT_DISABLED);
        AUDIO_WARNING_LOG("ConnectInputSession: materialize SE failed permanently for session %{public}u, "
            "attach-state cleared, ret=%{public}d", curSessionId, seRet);
        return;
    }
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, curSessionId, state,
        OPERATION_SESSION_EFFECT_ENABLED);
    AUDIO_INFO_LOG("ConnectInputSession: SE materialized+ENABLED for session %{public}u", curSessionId);
}

int32_t HpaeOffloadRendererManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::Start");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Start not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Start sessionId %{public}u", sessionId);
        node->SetState(HPAE_SESSION_RUNNING);
        if (sessionId == curNode_->GetSessionId()) {
            ConnectInputSession();
            if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId) {
                sessionEffectNode_->StartEffect();
            }
        }
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, RENDERER_RUNNING);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::DisConnectInputSession()
{
    CHECK_AND_RETURN_RET(converterForLoudness_, SUCCESS);

    // Session Effect: disconnect SE node from chain (no bypass reconnect).
    // Do NOT DropEffect here — that happens in DeleteInputSession after this returns.
    if (sessionEffectNode_ != nullptr && loudnessGainNode_ != nullptr
        && converterForLoudness_ != nullptr) {
        sessionEffectNode_->PauseEffect();
        loudnessGainNode_->DisConnect(sessionEffectNode_);
        sessionEffectNode_->DisConnect(converterForLoudness_);
    } else if (loudnessGainNode_ != nullptr && converterForLoudness_ != nullptr) {
        loudnessGainNode_->DisConnect(converterForLoudness_);
    }

    converterForLoudness_->DisConnect(curNode_);
    if (limiterNode_) {
        limiterNode_->DisConnect(loudnessGainNode_);
        converterForOutput_->DisConnect(limiterNode_);
    } else {
        converterForOutput_->DisConnect(loudnessGainNode_);
    }
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->DisConnect(converterForOutput_);
    }
    renderNoneEffectNode_->AudioOffloadRendererStop(curNode_->GetNodeInfo(), sinkInfo_);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Pause(uint32_t sessionId, bool isStandby)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::Pause");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Pause not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Pause sessionId %{public}u", sessionId);
        // SE close-crossfade waiting on its callback: finalize now — TearDownCurrentNodePipeline
        // stops pulling the chain; the close cannot be guaranteed to complete before then.
        FinalizePendingSessionEffectClose(sessionId);
        auto state = curNode_->GetState();
        bool isSessionEffectActive = sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId;
        node->SetState(HPAE_SESSION_PAUSED);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, node->GetState(), OPERATION_PAUSED);
        TearDownCurrentNodePipeline(sessionId, state, isSessionEffectActive);
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, RENDERER_PAUSED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeOffloadRendererManager::TearDownCurrentNodePipeline(uint32_t sessionId,
    HpaeSessionState state, bool isSessionEffectActive)
{
    if (sessionId != curNode_->GetSessionId()) {
        return;
    }
    DisConnectInputSession();
    bool isRunningWithSink = state == HPAE_SESSION_RUNNING && sinkOutputNode_ != nullptr;
    if (isRunningWithSink) {
        sinkOutputNode_->StopStream(!isSessionEffectActive);
    }
    if (isRunningWithSink && !isSessionEffectActive) {
        uint64_t latency = 0;
        OnRequestLatency(sessionId, latency);
        OnRewindBuffer(sessionId, latency);
        FlushPipelineNodes(sessionId);
    }
}

int32_t HpaeOffloadRendererManager::Flush(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::Flush");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Flush not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Flush sessionId %{public}u", sessionId);
        // flush history buffer
        node->Flush();
        // flush converter nodes' dual buffers to prevent stale resampled data
        if (converterForLoudness_ != nullptr) {
            converterForLoudness_->FlushBuffers();
        }
        if (converterForOutput_ != nullptr) {
            converterForOutput_->FlushBuffers();
        }
        if (sessionId == curNode_->GetSessionId()) {
            // flush sinkoutput cache
            sinkOutputNode_->FlushStream();
        }
        if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId) {
            sessionEffectNode_->FlushEffect();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeOffloadRendererManager::FlushPipelineNodes(uint32_t sessionId)
{
    if (converterForLoudness_ != nullptr) {
        converterForLoudness_->FlushBuffers();
    }
    if (converterForOutput_ != nullptr) {
        converterForOutput_->FlushBuffers();
    }
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->FlushStream();
    }
}

int32_t HpaeOffloadRendererManager::Drain(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::Drain");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Drain not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Drain sessionId %{public}u", sessionId);
        node->Drain();
        if (node->GetState() != HPAE_SESSION_RUNNING) {
            TriggerCallback(
                UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, node->GetState(), OPERATION_DRAINED);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::Stop");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Stop not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Stop sessionId %{public}u", sessionId);
        // SE close-crossfade waiting on its callback: finalize now — TearDownCurrentNodePipeline
        // stops pulling the chain; the close cannot be guaranteed to complete before then.
        FinalizePendingSessionEffectClose(sessionId);
        auto state = curNode_->GetState();
        bool isSessionEffectActive = sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId;
        node->SetState(HPAE_SESSION_STOPPED);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, node->GetState(), OPERATION_STOPPED);
        TearDownCurrentNodePipeline(sessionId, state, isSessionEffectActive);
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, RENDERER_STOPPED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Release(uint32_t sessionId)
{
    return DestroyStream(sessionId);
}

void HpaeOffloadRendererManager::MoveAllStreamToNewSink(const std::string &sinkName,
    const std::vector<uint32_t>& moveIds, MoveSessionType moveType)
{
    Trace trace("HpaeOffloadRendererManager::MoveAllStreamToNewSink[" +
        sinkName + "]_moveType[" + std::to_string(moveType) + "]");
    std::string name = sinkName;
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;

    for (auto [sessionId, node]: sinkInputNodeMap_) {
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), sessionId) != moveIds.end()) {
            sinkInputs.emplace_back(node);
            HILOG_COMM_INFO("[StartMove] session: %{public}u,sink [offload] --> [%{public}s]",
                sessionId, sinkName.c_str());
        }
    }
    for (auto node: sinkInputs) {
        // curNode_ can be null: DeleteInputSession nulls it via RemoveNodeFromMap
        // after the first match, and move-in paths can arrive at an empty manager.
        if (curNode_ && node->GetSessionId() == curNode_->GetSessionId()) {
            DeleteInputSession(SessionEffectMoveMode::KEEP_ATTACH_STATE_FOR_MOVE);
        } else {
            // Attach state follows the stream to the new sink — do NOT clear it or emit DISABLED.
            // The SE will re-materialize on the new sink when ConnectInputSession runs.
            RemoveNodeFromMap(node->GetSessionId());
        }
    }

    if (sinkInputs.size() == 0) {
        AUDIO_WARNING_LOG("sink count is 0,no need move session");
    }
    if (moveType == MOVE_ALL) {
        TriggerSyncCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
    } else {
        TriggerCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
    }
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
        SendRequest(request, __func__);
    }
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::MoveStream(uint32_t sessionId, const std::string &sinkName)
{
    auto request = [this, sessionId, sinkName]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::MoveStream to [" +
            sinkName + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        if (node == nullptr) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}d failed,sink [offload] --> [%{public}s]",
                sessionId, sinkName.c_str());
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
            HpaeStreamMoveMonitor::ReportStreamMoveException(0, sessionId, HPAE_STREAM_CLASS_TYPE_PLAY,
                "offload", sinkName, "not find session node");
            return;
        }

        if (sinkName.empty()) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,sinkName is empty", sessionId);
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
            HpaeStreamMoveMonitor::ReportStreamMoveException(node->GetAppUid(), sessionId,
                HPAE_STREAM_CLASS_TYPE_PLAY, "offload", sinkName, "sinkName is empty");
            return;
        }
        AUDIO_INFO_LOG("move session:%{public}d,sink [offload] --> [%{public}s]", sessionId, sinkName.c_str());
        if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId) {
            sessionEffectNode_->PrepareMoveStream();
        }
        if (sessionId == curNode_->GetSessionId()) {
            DeleteInputSession(SessionEffectMoveMode::KEEP_ATTACH_STATE_FOR_MOVE);
            SetCurrentNode();
        } else {
            // Attach state follows the stream to the new sink — do NOT clear it or emit DISABLED.
            // The SE will re-materialize on the new sink when ConnectInputSession runs.
            RemoveNodeFromMap(sessionId);
        }
        std::string name = sinkName;
        TriggerCallback(MOVE_SINK_INPUT, node, name);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::SuspendStreamManager(bool isSuspend)
{
    auto request = [this, isSuspend]() {
        Trace trace("HpaeOffloadRendererManager::SuspendStreamManager[" + std::to_string(isSuspend) + "]");
        if (isSuspend_ == isSuspend) {
            return;
        }
        isSuspend_ = isSuspend;
        if (isSuspend_) {
            sinkOutputNode_->RenderSinkStop();
        } else if (sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING && curNode_ &&
            curNode_->GetState() == HPAE_SESSION_RUNNING) {
            UpdateAppsUid();
            sinkOutputNode_->RenderSinkStart();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::StopManager()
{
    auto request = [this] {
        Trace trace("StopManager");
        CHECK_AND_RETURN_LOG(sinkOutputNode_ != nullptr, "sink output node is nullptr");
        sinkOutputNode_->RenderSinkStop();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::SetMute(bool isMute)
{
    auto request = [this, isMute]() {
        isMute_ = isMute;  // todo: set to sinkoutputnode
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeOffloadRendererManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

void HpaeOffloadRendererManager::StopOuputNode()
{
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->RenderSinkStop();
        sinkOutputNode_->RenderSinkDeInit();
        sinkOutputNode_->ResetAll();
        sinkOutputNode_ = nullptr;
    }
}

int32_t HpaeOffloadRendererManager::ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload)
{
    if (!IsInit()) {
        hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    }
    auto request = [this, sinkInfo, isReload]() {
        Trace trace("HpaeOffloadRendererManager::ReloadRenderManager[" + std::to_string(isReload) + "]");
        AUDIO_INFO_LOG("reload offload");
        if (curNode_ != nullptr && curNode_->GetState() == HPAE_SESSION_RUNNING) {
            DisConnectInputSession();
            DetachSessionEffectNodeForReload();
            if (sinkOutputNode_ != nullptr) {
                sinkOutputNode_->StopStream();
            }
            DestroyOffloadNodes();
        }
        StopOuputNode();
        sinkInfo_ = sinkInfo;
        InitSinkInner(isReload);

        CHECK_AND_RETURN_LOG(curNode_ != nullptr, "curNode_ is null");
        if (curNode_->GetState() == HPAE_SESSION_RUNNING) {
            CreateOffloadNodes();
            ConnectInputSession();
        }
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, curNode_->GetSessionId(),
            ConvertHpaeToRendererState(curNode_->GetState()));
    };
    SendRequest(request, __func__, true);
    if (!IsInit()) {
        hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    }
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::Init(bool isReload)
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, isReload] {
        Trace trace("HpaeOffloadRendererManager::Init[" + std::to_string(isReload) + "]");
        InitSinkInner(isReload);
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::InitSinkInner(bool isReload)
{
    AUDIO_INFO_LOG("init devicename:%{public}s,channel:%{public}u,rate:%{public}u",
        GetEncryptStr(sinkInfo_.deviceName).c_str(), sinkInfo_.channels, sinkInfo_.samplingRate);
    HpaeNodeInfo nodeInfo;
    int32_t checkRet = CheckFramelen(sinkInfo_);
    if (checkRet != SUCCESS) {
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT,
                        sinkInfo_.deviceName, ERR_INVALID_PARAM);
        return checkRet;
    }
    nodeInfo.channels = sinkInfo_.channels;
    nodeInfo.format = sinkInfo_.format;
    nodeInfo.frameLen = sinkInfo_.frameLen;
    nodeInfo.nodeId = 0;
    nodeInfo.samplingRate = sinkInfo_.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_OUT;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.statusCallback = weak_from_this();
    sinkOutputNode_ = std::make_unique<HpaeOffloadSinkOutputNode>(nodeInfo);
    sinkOutputNode_->SetTimeoutStopThd(sinkInfo_.suspendTime);
    // if failed, RenderSinkInit will failed either, so no need to deal ret
    AUDIO_INFO_LOG("GetRenderSinkInstance");
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
    sinkOutputNode_->RegisterOffloadCallback(this);
    isInit_.store(ret == SUCCESS);
    TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sinkInfo_.deviceName, ret);
    AUDIO_INFO_LOG("inited");
    return SUCCESS;
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
    Trace trace("HpaeOffloadRendererManager::DeInit[" + std::to_string(isMoveDefault) + "]");
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
    StopOuputNode();
    
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
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node != nullptr, "RegisterWriteCallback not find sessionId %{public}u",
            sessionId);
        node->RegisterWriteCallback(callback);
    };
    SendRequest(request, __func__);
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
    if (curNode_ != nullptr && curNode_->GetState() == HPAE_SESSION_RUNNING) {
        appsUid_.emplace_back(curNode_->GetAppUid());
    }
    sinkOutputNode_->UpdateAppsUid(appsUid_);
}

void HpaeOffloadRendererManager::NotifyStreamChangeToSink(
    StreamChangeType change, uint32_t sessionId, RendererState state, uint32_t appUid)
{
    CHECK_AND_RETURN(sinkOutputNode_ != nullptr);
    StreamUsage usage = STREAM_USAGE_UNKNOWN;
    if (sinkInputNodeMap_.find(sessionId) != sinkInputNodeMap_.end()) {
        usage = AudioTypeUtils::GetStreamUsageByStreamType(sinkInputNodeMap_[sessionId]->GetStreamType());
    }
    sinkOutputNode_->NotifyStreamChangeToSink(change, sessionId, usage, state, appUid);
}

int32_t HpaeOffloadRendererManager::SetOffloadPolicy(uint32_t sessionId, int32_t state)
{
    auto request = [this, sessionId, state]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::SetOffloadPolicy[" +
            std::to_string(state) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "SetOffloadPolicy not find sessionId %{public}u", sessionId);
        node->SetOffloadEnabled(state != OFFLOAD_DEFAULT);
        node->SetOffloadType(state);

        lastPolicyState_ = state;

        // AISS active: intercept BACKGROUND to keep 200ms buffer + RunningLock.
        // Real state is saved in lastPolicyState_, restored by RemoveSessionEffectNode when AISS closes.
        if (sessionEffectNode_ != nullptr && state == OFFLOAD_INACTIVE_BACKGROUND) {
            AUDIO_INFO_LOG("SetOffloadPolicy: AISS active, intercept BACKGROUND for session %{public}u",
                sessionId);
            return;
        }

        // Normal path
        // OFFLOAD_DEFAULT do not need set buffersize
        if (state != OFFLOAD_DEFAULT && sinkOutputNode_) {
            sinkOutputNode_->SetPolicyState(state);
        }
    };
    SendRequest(request, __func__);
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
        Trace trace("[" + std::to_string(sessionId) +
            "]HpaeOffloadRendererManager::SetOffloadRenderCallbackType[" + std::to_string(type) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "SetOffloadRenderCallbackType not find sessionId %{public}u",
            sessionId);
        CHECK_AND_RETURN_LOG(sinkOutputNode_, "sinkOutputNode_ is nullptr");
        if (sessionId == curNode_->GetSessionId()) {
            sinkOutputNode_->SetOffloadRenderCallbackType(type);
        } else {
            AUDIO_WARNING_LOG("curNode_ sessionId is %{public}u but set %{public}u",
                curNode_->GetSessionId(), sessionId);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeOffloadRendererManager::SetSpeed(uint32_t sessionId, float speed)
{
    auto request = [this, sessionId, speed]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::SetSpeed[" +
            std::to_string(speed) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "SetSpeed not find sessionId %{public}u", sessionId);
        node->SetSpeed(speed);
        CHECK_AND_RETURN_LOG(sinkOutputNode_, "sinkOutputNode is nullptr");
        if (sessionId == curNode_->GetSessionId()) {
            sinkOutputNode_->SetSpeed(speed);
        } else {
            AUDIO_WARNING_LOG("curNode_ sessionId is %{public}u but set %{public}u",
                curNode_->GetSessionId(), sessionId);
        }
    };
    SendRequest(request, __func__);
}

std::vector<SinkInput> HpaeOffloadRendererManager::GetAllSinkInputsInfo()
{
    std::vector<SinkInput> sinkInputs;
    return sinkInputs;
}

int32_t HpaeOffloadRendererManager::GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(node, ERR_INVALID_OPERATION,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::PLAY,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "GetSinkInputInfo not find sessionId", false),
        "GetSinkInputInfo not find sessionId %{public}u", sessionId);
    sinkInputInfo.nodeInfo = node->GetNodeInfo();
    sinkInputInfo.rendererSessionInfo.state = node->GetState();
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::RefreshProcessClusterByDevice()
{
    return SUCCESS;
}

HpaeSinkInfo HpaeOffloadRendererManager::GetSinkInfo()
{
    return sinkInfo_;
}

void HpaeOffloadRendererManager::SendRequest(Request &&request, const std::string &funcName, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_ERR_LOG("HpaeOffloadRendererManager not init, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_OFFLOAD_MANAGER_TYPE, funcName,
            "HpaeOffloadRendererManager not init");
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    if (hpaeSignalProcessThread_ == nullptr) {
        AUDIO_ERR_LOG("hpaeSignalProcessThread_ is nullptr, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_OFFLOAD_MANAGER_TYPE, funcName, "thread is nullptr");
        return;
    }
    hpaeSignalProcessThread_->Notify();
}

void HpaeOffloadRendererManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    CHECK_AND_RETURN_LOG(curNode_ != nullptr,
        "HpaeOffloadRendererManager::OnNodeStatusUpdate curNode_ is null");
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, curNode_->GetState(), operation);
}

void HpaeOffloadRendererManager::OnRequestLatency(uint32_t sessionId, uint64_t &latency)
{
    uint64_t processLatency = 0;

    if (converterForLoudness_) {
        processLatency += converterForLoudness_->GetLatency();
    }

    // Session Effect latency
    if (sessionEffectNode_) {
        processLatency += sessionEffectNode_->GetLatency(sessionId);
    }

    if (loudnessGainNode_) {
        processLatency += loudnessGainNode_->GetLatency();
    }

    if (limiterNode_) {
        processLatency += limiterNode_->GetLatency();
    }

    if (converterForOutput_) {
        processLatency += converterForOutput_->GetLatency();
    }

    if (sinkOutputNode_) {
        processLatency += sinkOutputNode_->GetLatency();
    }

    latency += processLatency;
    return;
}

void HpaeOffloadRendererManager::OnRewindAndFlush(uint64_t rewindTime, uint64_t hdiFramePosition)
{
    CHECK_AND_RETURN_LOG(curNode_ != nullptr,
        "HpaeOffloadRendererManager::OnRewindAndFlush curNode_ is null");
    curNode_->RewindHistoryBuffer(rewindTime, hdiFramePosition);
}

void HpaeOffloadRendererManager::OnRewindBuffer(uint32_t sessionId, uint64_t rewindTimeUs)
{
    CHECK_AND_RETURN_LOG(curNode_ != nullptr,
        "HpaeOffloadRendererManager::OnRewindBuffer curNode_ is null");
    curNode_->RewindHistoryBuffer(rewindTimeUs);
    AUDIO_INFO_LOG("OnRewindBuffer: session %{public}u rewound %{public}" PRIu64 "us",
        sessionId, rewindTimeUs);
}

void HpaeOffloadRendererManager::OnNotifyHdiData(const std::pair<uint64_t, TimePoint> &hdiPos)
{
    CHECK_AND_RETURN_LOG(curNode_ != nullptr,
        "HpaeOffloadRendererManager::OnNotifyHdiData curNode_ is null");
    curNode_->NotifyOffloadHdiPos(hdiPos);
}

OffloadCallbackData HpaeOffloadRendererManager::GetOffloadCallbackData() noexcept
{
    OffloadCallbackData data;
    if (sinkOutputNode_ != nullptr) {
        data = sinkOutputNode_->GetOffloadCallbackData();
    }
    return data;
}

void HpaeOffloadRendererManager::OnNotifyQueue()
{
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ offloadrenderer is nullptr");
    hpaeSignalProcessThread_->Notify();
}

std::string HpaeOffloadRendererManager::GetThreadName()
{
    return sinkInfo_.deviceName;
}

int32_t HpaeOffloadRendererManager::DumpSinkInfo()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "HpaeOffloadRendererManager not init");
    auto request = [this]() {
        AUDIO_INFO_LOG("DumpSinkInfo deviceName %{public}s", GetEncryptStr(sinkInfo_.deviceName).c_str());
        UploadDumpSinkInfo(sinkInfo_.deviceName);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

std::string HpaeOffloadRendererManager::GetDeviceHDFDumpInfo()
{
    std::string config;
    TransDeviceInfoToString(sinkInfo_, config);
    return config;
}

int32_t HpaeOffloadRendererManager::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    auto request = [this, sessionId, loudnessGain]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::SetLoudnessGain[" +
            std::to_string(loudnessGain) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "sessionId %{public}d, sinkInputNode is nullptr", sessionId);
        node->SetLoudnessGain(loudnessGain);
        if (sessionId == curNode_->GetSessionId()) {
            CHECK_AND_RETURN_LOG(loudnessGainNode_, "session id %{public}d is not connected", sessionId);
            loudnessGainNode_->SetLoudnessGain(loudnessGain);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeOffloadRendererManager::GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(loudnessGainNode_, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::PLAY,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "gainNode does not exist", false),
        "sessionId %{public}d, gainNode does not exist", sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(loudnessGainNode_->GetSessionId() == sessionId, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::PLAY,
                BusinessScenario::QUERY, ERR_INVALID_PARAM),
            "loudness node sessionId does not match", false),
        "loudness node sessionId does not match, sessionId %{public}u", sessionId);
    basicFormat.audioChannelInfo.channelLayout = (AudioChannelLayout)sinkInfo_.channelLayout;
    basicFormat.audioChannelInfo.numChannels = (uint32_t)sinkInfo_.channels;
    basicFormat.rate = sinkInfo_.samplingRate;
    if (loudnessGainNode_->IsLoudnessAlgoOn()) {
        // has loudness gain algorithm, should convert to 48k, channels and chanellayout stay same as input
        basicFormat.rate = SAMPLE_RATE_48000;
    }
    // When sessionEffect is enabled, force 48kHz dual channel
    CHECK_AND_RETURN_RET(sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId, SUCCESS);
    basicFormat.rate = SAMPLE_RATE_48000;
    basicFormat.audioChannelInfo.numChannels = STEREO;
    basicFormat.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    return SUCCESS;
}

void HpaeOffloadRendererManager::TriggerAppsUidUpdate(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        AUDIO_INFO_LOG("deviceClass: %{public}s", sinkInfo_.deviceClass.c_str());
        CHECK_AND_RETURN(sinkInfo_.deviceClass == REMOTE_OFFLOAD_DEVICE_CLASS);
        appsUid_.clear();
        if (curNode_ != nullptr &&
            (curNode_->GetState() == HPAE_SESSION_RUNNING ||
            curNode_->GetSessionId() == sessionId)) {
            appsUid_.emplace_back(curNode_->GetAppUid());
        }
        sinkOutputNode_->UpdateAppsUid(appsUid_);
    };
    SendRequest(request, __func__);
}

// === Session Effect ===

int32_t HpaeOffloadRendererManager::InsertSessionEffectNode(const std::string &effectName,
    std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
    bool aissSwitchEnabled)
{
    CHECK_AND_RETURN_RET_LOG(sessionEffectNode_ == nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND),
        "InsertSessionEffectNode: SE node already exists");
    CHECK_AND_RETURN_RET_LOG(converterForLoudness_ != nullptr && loudnessGainNode_ != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND),
        "InsertSessionEffectNode: chain not created yet");

    HpaeNodeInfo nodeInfo = converterForLoudness_->GetNodeInfo();
    sessionEffectNode_ = std::make_shared<HpaeSessionEffectNode>(nodeInfo);
    int32_t ret = sessionEffectNode_->CreateEffectWithInstance(effectName, instance, config, aissSwitchEnabled);
    if (ret != 0) {
        AUDIO_ERR_LOG("InsertSessionEffectNode: CreateEffectWithInstance failed, ret=%{public}d", ret);
        sessionEffectNode_->DropEffect();
        sessionEffectNode_.reset();
        sessionEffectSessionId_ = 0;
        return ret;
    }

    // Insert between converterForLoudness_ and loudnessGainNode_
    loudnessGainNode_->DisConnect(converterForLoudness_);
    loudnessGainNode_->Connect(sessionEffectNode_);
    sessionEffectNode_->Connect(converterForLoudness_);

    // Force foreground buffer: AISS needs 200ms low-latency data flow.
    // If currently in BACKGROUND (7000ms buffer), this cancels the pending 3s delay
    // and restores 200ms + RunningLock. 0 = OFFLOAD_ACTIVE_FOREGROUND.
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->SetPolicyState(0);
    }

    AUDIO_INFO_LOG("InsertSessionEffectNode: SE node inserted for session %{public}u",
        sessionEffectSessionId_);
    return 0;
}

int32_t HpaeOffloadRendererManager::RemoveSessionEffectNode(bool finalizeNow)
{
    CHECK_AND_RETURN_RET_LOG(sessionEffectNode_ != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND),
        "RemoveSessionEffectNode: no SE node to remove");

    auto closingNode = sessionEffectNode_;
    uint32_t sessionId = sessionEffectSessionId_;
    closingNode->StopPreheatThread();

    // Pipeline latency the wet output lags behind upstream dry = SE + the converter
    // immediately upstream of the SE insert point. Flush that converter's pending data,
    // then rewind curNode_ history so the bypass dry re-pulls from the aligned cursor.
    // Downstream converter/HDI queues are intentionally untouched to keep output continuous.
    uint64_t latency = closingNode->GetLatency(sessionId);
    if (converterForLoudness_ != nullptr) {
        latency += converterForLoudness_->GetLatency();
        converterForLoudness_->FlushBuffers();
    }
    if (latency > 0 && curNode_ != nullptr && curNode_->GetSessionId() == sessionId) {
        curNode_->RewindHistoryBuffer(latency);
    }

    // Enter STOPPING: the next Process() (between this and the next HandleMsg) gets one aligned
    // wet->dry crossfade opportunity if the chain is pulled. FinalizeClose — direct when
    // finalizeNow or no crossfade pending, else deferred to the OnCloseCrossfadeDone callback —
    // owns the actual node deletion (never inside DoProcess).
    closingNode->StartCloseCrossfade();
    std::weak_ptr<HpaeSessionEffectNode> weakClosingNode = closingNode;
    bool crossfadePending = closingNode->IsCloseCrossfadePending();
    if (finalizeNow || !crossfadePending) {
        // Switch path ("open new"), or a non-ACTIVE node with no crossfade armed: finalize
        // inside this same request so a Create queued right behind us sees no leftover node.
        FinalizeSessionEffectClose(sessionId, weakClosingNode);
    } else {
        // Smooth close: ProcessCloseCrossfade completes on the next pull and calls back
        // OnCloseCrossfadeDone, which finalizes on the HandleMsg thread. Record the identity
        // token so a late callback cannot finalize a different (newer) node.
        pendingCloseNode_ = weakClosingNode;
        pendingCloseSessionId_ = sessionId;
    }
    AUDIO_INFO_LOG("RemoveSessionEffectNode: session %{public}u BeginClose (finalizeNow=%{public}d, "
        "crossfadePending=%{public}d), latency=%" PRIu64,
        sessionId, static_cast<int32_t>(finalizeNow), static_cast<int32_t>(crossfadePending), latency);
    return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
}

void HpaeOffloadRendererManager::FinalizeSessionEffectClose(uint32_t sessionId,
    const std::weak_ptr<HpaeSessionEffectNode> &closingNode)
{
    // Any finalize attempt ends the pending-close window for this session.
    if (pendingCloseSessionId_ == sessionId) {
        ClearPendingSessionEffectClose();
    }
    auto node = closingNode.lock();
    // Identity check: no-op if the closing node was already replaced/destroyed
    // (DestroyStream, DeleteInputSession, Reload, or a replayed/recreated node).
    if (node == nullptr || sessionEffectNode_ != node || sessionEffectSessionId_ != sessionId) {
        return;
    }

    // Only reconnect the bypass if the stream is still running; if pause/stop already
    // tore the chain down, leave it disconnected — ConnectInputSession rebuilds on Start.
    bool restoreBypass = curNode_ != nullptr && curNode_->GetSessionId() == sessionId &&
        curNode_->GetState() == HPAE_SESSION_RUNNING &&
        loudnessGainNode_ != nullptr && converterForLoudness_ != nullptr;
    if (loudnessGainNode_ != nullptr && converterForLoudness_ != nullptr) {
        loudnessGainNode_->DisConnect(node);
        node->DisConnect(converterForLoudness_);
        if (restoreBypass) {
            loudnessGainNode_->Connect(converterForLoudness_);
        }
    }

    // If a pending rapid-recreate will reuse this exact instance, do NOT Flush/Stop it —
    // the new binding takes the instance over (and there is no matching ENABLE otherwise).
    // Otherwise release normally (Flush + StopEffect, Lazy Release owns the algorithm).
    auto pendingInst = pendingSeCreateInstance_.lock();
    auto closingInst = node->GetEffectInstance();
    bool sameInstanceReuse = pendingInst != nullptr && closingInst != nullptr &&
        pendingSeCreateSessionId_ == sessionId && pendingInst.get() == closingInst.get();
    if (sameInstanceReuse) {
        node->CloseSessionEffectCore();
    } else {
        node->DropEffect();
    }

    sessionEffectNode_.reset();
    sessionEffectSessionId_ = 0;

    // Restore real offload policy (forced foreground while AISS was active).
    if (sinkOutputNode_ != nullptr && lastPolicyState_ != OFFLOAD_DEFAULT) {
        AUDIO_INFO_LOG("FinalizeSessionEffectClose: restore policy state=%{public}d", lastPolicyState_);
        sinkOutputNode_->SetPolicyState(lastPolicyState_);
    }
    auto streamNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    HpaeSessionState state = streamNode != nullptr ? streamNode->GetState() : HPAE_SESSION_NEW;
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state,
        OPERATION_SESSION_EFFECT_DISABLED);
    AUDIO_INFO_LOG("FinalizeSessionEffectClose: session %{public}u finalized (sameInstance=%{public}d)",
        sessionId, static_cast<int32_t>(sameInstanceReuse));
}

void HpaeOffloadRendererManager::ClearPendingSessionEffectClose()
{
    pendingCloseNode_.reset();
    pendingCloseSessionId_ = 0;
}

bool HpaeOffloadRendererManager::FinalizePendingSessionEffectClose(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET(pendingCloseSessionId_ == sessionId && !pendingCloseNode_.expired(), false);
    auto closingNode = pendingCloseNode_.lock();
    ClearPendingSessionEffectClose();
    if (closingNode == nullptr || sessionEffectNode_ != closingNode) {
        AUDIO_WARNING_LOG("PendingClose identity mismatch: session %{public}u", sessionId);
        return false;
    }
    FinalizeSessionEffectClose(sessionId, closingNode);
    return true;
}

void HpaeOffloadRendererManager::OnCloseCrossfadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        bool finalized = FinalizePendingSessionEffectClose(sessionId);
        AUDIO_INFO_LOG("CloseCrossfadeDone: session %{public}u finalized=%{public}d",
            sessionId, static_cast<int32_t>(finalized));
    };
    SendRequest(request, __func__);
}

uint64_t HpaeOffloadRendererManager::SetPendingSessionEffectCreate(uint32_t sessionId,
    const std::shared_ptr<IEffectInstance> &instance)
{
    ++pendingSeCreateGeneration_;
    pendingSeCreateSessionId_ = sessionId;
    pendingSeCreateInstance_ = instance;
    return pendingSeCreateGeneration_;
}

bool HpaeOffloadRendererManager::IsPendingSessionEffectCreate(uint32_t sessionId,
    uint64_t generation, const std::shared_ptr<IEffectInstance> &instance) const
{
    return pendingSeCreateSessionId_ == sessionId && pendingSeCreateGeneration_ == generation &&
        instance != nullptr && pendingSeCreateInstance_.lock().get() == instance.get();
}

void HpaeOffloadRendererManager::CancelPendingSessionEffectCreate(uint32_t sessionId)
{
    if (pendingSeCreateSessionId_ == sessionId) {
        pendingSeCreateSessionId_ = 0;
    }
}

bool HpaeOffloadRendererManager::StopOffloadSinkIfRunning()
{
    CHECK_AND_RETURN_RET(curNode_ != nullptr, false);
    CHECK_AND_RETURN_RET(curNode_->GetState() == HPAE_SESSION_RUNNING, false);
    CHECK_AND_RETURN_RET(sinkOutputNode_ != nullptr, false);
    // Only flush HDI/DSP cache to prevent pop noise on SE insert/remove.
    // Do NOT rewind the upstream history buffer; the SE pipeline handles buffering itself.
    sinkOutputNode_->StopStream(false);
    return true;
}

void HpaeOffloadRendererManager::DetachSessionEffectNodeForReload()
{
    // Do NOT clear attach state — it must survive reload so ConnectInputSession can re-materialize.
    // Only reset the physical cache (node + sessionId).
    CancelPendingSessionEffectCreate(sessionEffectSessionId_);
    ClearPendingSessionEffectClose();
    if (sessionEffectNode_ != nullptr) {
        // Reload runs on SignalProcessThread after DisConnectInputSession has removed this node
        // from the processing chain, so CloseSessionEffectCore only resets local runtime state.
        sessionEffectNode_->CloseSessionEffectCore();
        sessionEffectNode_.reset();
    }
    sessionEffectSessionId_ = 0;
}

bool HpaeOffloadRendererManager::TryDeferSessionEffectCreate(uint32_t sessionId,
    const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    // The curNode's physical SE node is still finalizing its deferred close (STOPPING).
    // Defer this Create until that node is gone: record a cancelable/ownership token and
    // requeue the original request. Do NOT write attach state, insert a node, or send ENABLE.
    CHECK_AND_RETURN_RET(curNode_ != nullptr && curNode_->GetSessionId() == sessionId &&
        sessionEffectNode_ != nullptr &&
        sessionEffectNode_->GetStatus() == HpaeSessionEffectNode::STATUS::STOPPING, false);
    uint64_t generation = SetPendingSessionEffectCreate(sessionId, instance);
    Request retry = [this, sessionId, effectName, instance, config, generation, aissSwitchEnabled]() {
        // superseded by a newer Create, or cancelled by Destroy/DestroyStream/Move/Reload
        CHECK_AND_RETURN(IsPendingSessionEffectCreate(sessionId, generation, instance));
        CancelPendingSessionEffectCreate(sessionId);
        if (SafeGetMap(sinkInputNodeMap_, sessionId) != nullptr) {
            CreateSessionEffect(sessionId, effectName, instance, config, aissSwitchEnabled);
        }
    };
    hpaeNoLockQueue_.PushRequest(std::move(retry));
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->Notify();
    }
    AUDIO_INFO_LOG("CreateSessionEffect: session %{public}u deferred behind STOPPING node", sessionId);
    return true;
}

int32_t HpaeOffloadRendererManager::MaterializeCurrentSessionEffect(uint32_t sessionId,
    const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    // Caller guarantees curNode_ != nullptr && curNode_->GetSessionId() == sessionId.
    if (converterForLoudness_ == nullptr || loudnessGainNode_ == nullptr) {
        // Chain not ready; attach state sits on node, will materialize in ConnectInputSession
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, curNode_->GetState(),
            OPERATION_SESSION_EFFECT_ENABLED);
        AUDIO_INFO_LOG("CreateSessionEffect: deferred for current session %{public}u (chain not created)",
            sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
    }
    sessionEffectSessionId_ = sessionId;
    int32_t ret = InsertSessionEffectNode(effectName, instance, config, aissSwitchEnabled);
    if (ret == static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, curNode_->GetState(),
            OPERATION_SESSION_EFFECT_ENABLED);
    } else {
        sessionEffectSessionId_ = 0;
    }
    return ret;
}

int32_t HpaeOffloadRendererManager::CreateSessionEffect(uint32_t sessionId,
    const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_RET(!TryDeferSessionEffectCreate(sessionId, effectName, instance, config, aissSwitchEnabled),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));

    // Reject a duplicate bind BEFORE touching attach state, so a rejected Create does not
    // overwrite the existing binding already held on the SinkInputNode.
    CHECK_AND_RETURN_RET_LOG(
        !(curNode_ != nullptr && curNode_->GetSessionId() == sessionId && sessionEffectNode_ != nullptr),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND),
        "CreateSessionEffect: curNode already has materialized SE for session %{public}u",
        sessionId);

    // Create accepted — persist the logical binding on the SinkInputNode. Done after the
    // ALREADY_BOUND check so a rejected Create leaves the prior attach state intact.
    if (node != nullptr) {
        SessionEffectAttachState st;
        st.enabled = true;
        st.effectName = effectName;
        st.instance = instance;
        st.config = config;
        node->SetSessionEffectAttachState(st);
    }

    // If session is curNode_, materialize immediately (chain readiness checked inside)
    if (curNode_ != nullptr && curNode_->GetSessionId() == sessionId) {
        return MaterializeCurrentSessionEffect(sessionId, effectName, instance, config, aissSwitchEnabled);
    }
    // Not curNode_ -> attach state sits on SinkInputNode, waits for ConnectInputSession
    HpaeSessionState state = (node != nullptr) ? node->GetState() : HPAE_SESSION_NEW;
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state,
        OPERATION_SESSION_EFFECT_ENABLED);
    AUDIO_INFO_LOG("CreateSessionEffect: deferred for session %{public}u (not curNode)", sessionId);
    return 0;
}

void HpaeOffloadRendererManager::PostCreateSessionEffect(uint32_t sessionId,
    const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    auto request = [this, sessionId, effectName, instance, config, aissSwitchEnabled]() {
        CreateSessionEffect(sessionId, effectName, instance, config, aissSwitchEnabled);
    };
    SendRequest(std::move(request), __func__);
}

int32_t HpaeOffloadRendererManager::DestroySessionEffect(uint32_t sessionId, bool finalizeNow)
{
    // Clear attach state on SinkInputNode (logical unbinding)
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    bool hadAttachState = (node != nullptr) && node->HasSessionEffectAttachState();
    if (node != nullptr) {
        node->ClearSessionEffectAttachState();
    }

    // Remove active physical SE node if materialized for this session
    if (sessionEffectSessionId_ == sessionId && sessionEffectNode_ != nullptr) {
        if (sessionEffectNode_->GetStatus() == HpaeSessionEffectNode::STATUS::STOPPING) {
            // Close already begun (BeginClose is waiting on the crossfade callback). Re-BeginClose
            // would double-rewind and duplicate the Finalize; just cancel any deferred recreate.
            CancelPendingSessionEffectCreate(sessionId);
            if (finalizeNow) {
                // Earlier BeginClose is waiting on its callback; run the Finalize now so the
                // teardown completes before the new session's Create (the late callback no-ops
                // on the cleared token).
                FinalizeSessionEffectClose(sessionId, sessionEffectNode_);
            }
            AUDIO_INFO_LOG("DestroySessionEffect: session %{public}u already STOPPING, skip re-Begin",
                sessionId);
            return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
        }
        return RemoveSessionEffectNode(finalizeNow);
    }

    // If there was attach state but no physical node, emit DISABLED for the logical unbind
    if (hadAttachState) {
        HpaeSessionState state = (node != nullptr) ? node->GetState() : HPAE_SESSION_NEW;
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state,
            OPERATION_SESSION_EFFECT_DISABLED);
        AUDIO_INFO_LOG("DestroySessionEffect: cleared attach state for session %{public}u", sessionId);
        return 0;
    }

    AUDIO_WARNING_LOG("DestroySessionEffect: session %{public}u has no SE", sessionId);
    return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
}

int32_t HpaeOffloadRendererManager::PostDestroySessionEffect(uint32_t sessionId, bool finalizeNow)
{
    auto request = [this, sessionId, finalizeNow]() -> int32_t {
        return DestroySessionEffect(sessionId, finalizeNow);
    };
    int32_t ret = SendRequestSync<int32_t>(std::move(request), __func__, ERR_OPERATION_FAILED);
    CHECK_AND_RETURN_RET_LOG(ret == static_cast<int32_t>(SessionEffectErrCode::SUCCESS), ret,
        "PostDestroySessionEffect: session %{public}u destroy failed ret=%{public}d", sessionId, ret);
    return ret;
}

bool HpaeOffloadRendererManager::IsValidSession(uint32_t sessionId) const
{
    return sinkInputNodeMap_.find(sessionId) != sinkInputNodeMap_.end();
}

int32_t HpaeOffloadRendererManager::SetSessionEffectParameter(uint32_t sessionId,
    int32_t type, float value)
{
    if (sessionEffectSessionId_ == sessionId && sessionEffectNode_ != nullptr) {
        return sessionEffectNode_->SetParameter(type, value);
    }
    AUDIO_WARNING_LOG("SetSessionEffectParameter: session %{public}u has no active SE node", sessionId);
    return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
}

void HpaeOffloadRendererManager::PostSetSessionEffectParameter(uint32_t sessionId,
    int32_t type, float value)
{
    auto request = [this, sessionId, type, value]() {
        SetSessionEffectParameter(sessionId, type, value);
    };
    SendRequest(std::move(request), __func__);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
