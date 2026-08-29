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
#define LOG_TAG "HpaeRendererManager"
#endif

#include "hpae_renderer_manager.h"

#include "audio_engine_log.h"
#include "audio_utils.h"
#include "hpae_define.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
bool HpaeRendererManager::TryDeferSessionEffectCreate(
    uint32_t sessionId, const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    // The physical SE node is still finalizing its deferred close (STOPPING). Defer this Create
    // until that node is gone: record a cancelable/ownership token and requeue. Do NOT write
    // attach state, create a node, or send ENABLE here.
    HpaeProcessorType sceneType = GetConnectedProcessorType(sessionId);
    auto processCluster = SafeGetMap(sceneClusterMap_, sceneType);
    CHECK_AND_RETURN_RET(processCluster != nullptr && processCluster->IsSessionEffectNodeStopping(sessionId), false);
    uint64_t generation = SetPendingSessionEffectCreate(sessionId, instance);
    Request retry = [this, sessionId, effectName, instance, config, generation, aissSwitchEnabled]() {
        // superseded by a newer Create, or cancelled by Destroy/DestroyStream/Move/Reload
        CHECK_AND_RETURN(IsPendingSessionEffectCreate(sessionId, generation, instance));
        CancelPendingSessionEffectCreate(sessionId);
        CreateSessionEffect(sessionId, effectName, instance, config, aissSwitchEnabled);
    };
    hpaeNoLockQueue_.PushRequest(std::move(retry));
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->Notify();
    }
    AUDIO_INFO_LOG("CreateSessionEffect: session %{public}u deferred behind STOPPING node", sessionId);
    return true;
}

int32_t HpaeRendererManager::CreateSessionEffect(uint32_t sessionId, const std::string &effectName,
    std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
    bool aissSwitchEnabled)
{
    auto it = sessionNodeMap_.find(sessionId);
    if (it == sessionNodeMap_.end()) {
        AUDIO_WARNING_LOG("CreateSessionEffect: session %{public}u not found", sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND);
    }
    CHECK_AND_RETURN_RET_LOG(instance != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER),
        "CreateSessionEffect: effect instance is null for session %{public}u", sessionId);
    HpaeProcessorType sceneType = GetConnectedProcessorType(sessionId);
    auto processCluster = SafeGetMap(sceneClusterMap_, sceneType);
    if (!processCluster) {
        AUDIO_WARNING_LOG("CreateSessionEffect: cluster not found for sceneType %{public}d", sceneType);
        return static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND);
    }
    CHECK_AND_RETURN_RET(!TryDeferSessionEffectCreate(sessionId, effectName, instance, config, aissSwitchEnabled),
        static_cast<int32_t>(SessionEffectErrCode::SUCCESS));
    int32_t ret = processCluster->CreateSessionEffectNode(sessionId, effectName, instance, config,
        aissSwitchEnabled);
    if (ret != static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        AUDIO_WARNING_LOG("CreateSessionEffect failed for session %{public}u, ret %{public}d", sessionId, ret);
    } else {
        auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
        if (sinkInputNode != nullptr) {
            sinkInputNode->EnsureHistoryBufferForAiss(instance->GetPreheatFrames());
            SessionEffectAttachState st;
            st.enabled = true;
            st.effectName = effectName;
            st.instance = instance;
            st.config = config;
            sinkInputNode->SetSessionEffectAttachState(st);
            AUDIO_INFO_LOG("[SE::RendererManager] AttachState written sessionId=%{public}u effect=%{public}s",
                sessionId, effectName.c_str());
        } else {
            AUDIO_WARNING_LOG("CreateSessionEffect: SinkInputNode not found for session %{public}u", sessionId);
        }
        AUDIO_INFO_LOG("[SE::RendererManager] CreateSessionEffect sessionId=%{public}u", sessionId);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, it->second.state,
            OPERATION_SESSION_EFFECT_ENABLED);
        OnSessionEffectMaterialized(sessionId);
    }
    return ret;
}

void HpaeRendererManager::OnSessionEffectPreheatStateChanged(uint32_t sessionId, bool active)
{
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_LOG(sinkInputNode != nullptr,
        "OnSessionEffectPreheatStateChanged: sinkInputNode not found, session=%{public}u", sessionId);
    sinkInputNode->SetSessionEffectPreheatActive(active);
}

int32_t HpaeRendererManager::DestroySessionEffect(uint32_t sessionId, bool finalizeNow)
{
    // Route via GetProcessorType (override-set key); cluster lookup below uses GetConnectedProcessorType.
    auto route = BuildChannelOverrideRoute(sessionId);
    auto it = sessionNodeMap_.find(sessionId);
    if (it == sessionNodeMap_.end()) {
        AUDIO_WARNING_LOG("DestroySessionEffect: session %{public}u not found", sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND);
    }
    HpaeProcessorType sceneType = GetConnectedProcessorType(sessionId);
    auto processCluster = SafeGetMap(sceneClusterMap_, sceneType);
    if (!processCluster) {
        AUDIO_WARNING_LOG("DestroySessionEffect: cluster not found for sceneType %{public}d", sceneType);
        return static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND);
    }

    // STOPPING: a prior BeginClose is still waiting on its crossfade callback — skip re-Begin
    // (double-rewind / duplicate Finalize), cancel pending recreate, mirror derived managers.
    if (processCluster->IsSessionEffectNodeStopping(sessionId)) {
        CancelPendingSessionEffectCreate(sessionId);
        if (finalizeNow) {
            FinalizeStoppingSessionEffect(sessionId, route, it->second.state, processCluster);
        }
        AUDIO_INFO_LOG("DestroySessionEffect: session %{public}u already STOPPING, skip re-Begin",
            sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
    }

    // BeginClose: enter STOPPING (node stays in the cluster map); get combined SE+upstream
    // latency + a weak identity handle for Finalize.
    uint64_t latency = 0;
    std::weak_ptr<HpaeSessionEffectNode> weakNode;
    int32_t ret = processCluster->BeginDestroySessionEffectNode(sessionId, latency, weakNode);
    if (ret != static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        // SE may still be processing; preserve channel override to keep the chain at 2ch.
        AUDIO_WARNING_LOG("DestroySessionEffect: BeginClose failed ret=%{public}d; channel override preserved"
            " sessionId=%{public}u", ret, sessionId);
        return ret;
    }

    RewindAndClearSessionEffectAttach(sessionId, latency);
    HpaeSessionState state = it->second.state;
    auto closingNode = weakNode.lock();
    bool crossfadePending = closingNode != nullptr && closingNode->IsCloseCrossfadePending();
    if (finalizeNow || !crossfadePending) {
        // Switch path ("open new"), or a non-ACTIVE node with no crossfade armed: finalize
        // inside this same request so a Create queued right behind us sees no leftover node.
        FinalizeSessionEffectClose(sessionId, route, state, weakNode);
    } else {
        // Smooth close: ProcessCloseCrossfade completes on the next pull and calls back
        // OnCloseCrossfadeDone (see below), which finalizes on the HandleMsg thread.
        pendingSeFinalizeMap_[sessionId] = weakNode;
    }
    AUDIO_INFO_LOG("[SE::RendererManager] DestroySessionEffect BeginClose sessionId=%{public}u "
        "finalizeNow=%{public}d crossfadePending=%{public}d",
        sessionId, static_cast<int32_t>(finalizeNow), static_cast<int32_t>(crossfadePending));
    return ret;
}

// STOPPING + finalizeNow: finalize the close a prior BeginClose left waiting on its crossfade
// callback right now, so the old session is fully closed before the new one is created. A late
// OnCloseCrossfadeDone no-ops on the cleared token / identity check. Reuses the route built at
// destroy entry (single-threaded request, nothing re-routes in between).
void HpaeRendererManager::FinalizeStoppingSessionEffect(uint32_t sessionId,
    std::optional<ChannelOverrideRoute> route, HpaeSessionState state,
    const std::shared_ptr<HpaeProcessCluster> &processCluster)
{
    auto stoppingNode = processCluster->GetSessionEffectNode(sessionId);
    CHECK_AND_RETURN(stoppingNode != nullptr);
    FinalizeSessionEffectClose(sessionId, route, state, stoppingNode);
}

// Rewind upstream history by the combined latency; clear the logical binding now.
void HpaeRendererManager::RewindAndClearSessionEffectAttach(uint32_t sessionId, uint64_t latency)
{
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN(sinkInputNode != nullptr);
    if (latency > 0) {
        sinkInputNode->RewindHistoryBuffer(latency);
    }
    sinkInputNode->ClearSessionEffectAttachState();
    AUDIO_INFO_LOG("[SE::RendererManager] AttachState cleared sessionId=%{public}u", sessionId);
}

// Marshal-only: the finalize must not run inline here (this callback fires inside the node's
// DoProcess stack). Caveat: the INTERPHONE + null-thread SendRequest branch drains inline on the
// caller thread — unreachable today (interphone chains are never pulled); must stay that way, or
// this callback would finalize inside the node's process stack.
void HpaeRendererManager::OnCloseCrossfadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        bool finalized = FinalizePendingSessionEffectClose(sessionId);
        AUDIO_INFO_LOG("[SE::RendererManager] OnCloseCrossfadeDone sessionId=%{public}u finalized=%{public}d",
            sessionId, static_cast<int32_t>(finalized));
    };
    SendRequest(request, __func__);
}

// Rescue/delivery entry: erase the token, then finalize the recorded closing node if it is
// still the cluster's current node for the session (identity gate against rescue-then-recreate).
bool HpaeRendererManager::FinalizePendingSessionEffectClose(uint32_t sessionId)
{
    auto pendingIt = pendingSeFinalizeMap_.find(sessionId);
    CHECK_AND_RETURN_RET(pendingIt != pendingSeFinalizeMap_.end(), false);
    std::weak_ptr<HpaeSessionEffectNode> token = pendingIt->second;
    pendingSeFinalizeMap_.erase(pendingIt);
    auto closingNode = token.lock();
    CHECK_AND_RETURN_RET(closingNode != nullptr, false);
    HpaeProcessorType sceneType = GetConnectedProcessorType(sessionId);
    auto processCluster = SafeGetMap(sceneClusterMap_, sceneType);
    if (processCluster == nullptr || processCluster->GetSessionEffectNode(sessionId) != closingNode) {
        AUDIO_WARNING_LOG("[SE::RendererManager] PendingClose identity mismatch sessionId=%{public}u", sessionId);
        return false;
    }
    auto sessionIt = sessionNodeMap_.find(sessionId);
    HpaeSessionState state = sessionIt != sessionNodeMap_.end() ?
        sessionIt->second.state : HPAE_SESSION_NEW;
    FinalizeStoppingSessionEffect(sessionId, BuildChannelOverrideRoute(sessionId), state, processCluster);
    return true;
}

void HpaeRendererManager::FinalizeSessionEffectClose(uint32_t sessionId,
    std::optional<ChannelOverrideRoute> route, HpaeSessionState state,
    const std::weak_ptr<HpaeSessionEffectNode> &closingNode)
{
    // Any finalize attempt ends the pending-close window for this session: erased deliberately
    // BEFORE the identity checks below (the token window ends at the first finalize attempt;
    // the identity gates stay as the last line of defense for races the token cannot express).
    pendingSeFinalizeMap_.erase(sessionId);
    HpaeProcessorType sceneType = GetConnectedProcessorType(sessionId);
    auto processCluster = SafeGetMap(sceneClusterMap_, sceneType);
    CHECK_AND_RETURN(processCluster);
    // Feed the pending create's instance so the cluster can hand off a reused instance without
    // Flush/Stop; null -> cluster releases normally.
    std::shared_ptr<IEffectInstance> pendingInst;
    auto pendingIt = pendingSeCreateMap_.find(sessionId);
    if (pendingIt != pendingSeCreateMap_.end()) {
        pendingInst = pendingIt->second.second.lock();
    }
    bool finalized = processCluster->FinalizeDestroySessionEffectNode(sessionId, closingNode, pendingInst);
    CHECK_AND_RETURN(finalized);
    if (route.has_value()) {
        ClearChannelOverrideForRoute(route.value());
    }
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state,
        OPERATION_SESSION_EFFECT_DISABLED);
    AUDIO_INFO_LOG("[SE::RendererManager] FinalizeSessionEffectClose sessionId=%{public}u", sessionId);
}

uint64_t HpaeRendererManager::SetPendingSessionEffectCreate(uint32_t sessionId,
    const std::shared_ptr<IEffectInstance> &instance)
{
    ++pendingSeCreateGeneration_;
    pendingSeCreateMap_[sessionId] = {pendingSeCreateGeneration_, instance};
    return pendingSeCreateGeneration_;
}

bool HpaeRendererManager::IsPendingSessionEffectCreate(uint32_t sessionId, uint64_t generation,
    const std::shared_ptr<IEffectInstance> &instance) const
{
    auto it = pendingSeCreateMap_.find(sessionId);
    return it != pendingSeCreateMap_.end() && it->second.first == generation &&
        instance != nullptr && it->second.second.lock().get() == instance.get();
}

void HpaeRendererManager::CancelPendingSessionEffectCreate(uint32_t sessionId)
{
    pendingSeCreateMap_.erase(sessionId);
}

void HpaeRendererManager::DetachSessionEffectNodeForRefresh(uint32_t sessionId, HpaeProcessorType oldSceneType)
{
    // Invalidate any deferred Create so a stale retry cannot (re)materialize after the refresh.
    CancelPendingSessionEffectCreate(sessionId);
    pendingSeFinalizeMap_.erase(sessionId);
    // Remove the physical SE node from the OLD cluster before a refresh-driven disconnect, WITHOUT
    // clearing attach state, so ConnectInputSession re-materializes it in the NEW cluster.
    auto processCluster = SafeGetMap(sceneClusterMap_, oldSceneType);
    CHECK_AND_RETURN_LOG(processCluster != nullptr,
        "DetachSessionEffectNodeForRefresh: old cluster not found sceneType %{public}d",
        static_cast<int32_t>(oldSceneType));
    CHECK_AND_RETURN_LOG(processCluster->HasSessionEffectNode(sessionId),
        "DetachSessionEffectNodeForRefresh: no SE node for session %{public}u", sessionId);
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    uint64_t latency = 0;
    int32_t ret = processCluster->DestroySessionEffectNode(sessionId, latency);
    CHECK_AND_RETURN_LOG(ret == static_cast<int32_t>(SessionEffectErrCode::SUCCESS),
        "DetachSessionEffectNodeForRefresh: destroy failed session %{public}u ret %{public}d", sessionId, ret);
    auto oldRoute = BuildChannelOverrideRouteFromProcessorType(oldSceneType);
    if (oldRoute.has_value()) {
        ClearChannelOverrideForRoute(oldRoute.value());
    }
    AUDIO_INFO_LOG("[SE::RendererManager] Detached SE node for refresh sessionId=%{public}u oldSceneType=%{public}d",
        sessionId, static_cast<int32_t>(oldSceneType));
    CHECK_AND_RETURN(latency > 0 && sinkInputNode != nullptr);
    sinkInputNode->RewindHistoryBuffer(latency);
}

void HpaeRendererManager::HandleSessionEffectMaterializeFailure(uint32_t sessionId, int32_t ret,
    const std::shared_ptr<HpaeSinkInputNode> &sinkInputNode)
{
    // Permanent materialize failure: tear down any partial node, clear attach state, emit DISABLED.
    auto sessionIt = sessionNodeMap_.find(sessionId);
    HpaeSessionState sessionState = (sessionIt != sessionNodeMap_.end())
        ? sessionIt->second.state : HPAE_SESSION_NEW;
    sinkInputNode->ClearSessionEffectAttachState();
    auto route = BuildChannelOverrideRoute(sessionId);
    if (route.has_value()) {
        ClearChannelOverrideForRoute(route.value());
    }
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, sessionState,
        OPERATION_SESSION_EFFECT_DISABLED);
    AUDIO_WARNING_LOG("[SE::RendererManager] Materialize failure permanent, attach-state cleared, "
        "sessionId=%{public}u ret=%{public}d", sessionId, ret);
}

bool HpaeRendererManager::HasSessionEffect(uint32_t sessionId) const
{
    auto it = sessionNodeMap_.find(sessionId);
    if (it == sessionNodeMap_.end()) {
        return false;
    }
    HpaeProcessorType sceneType = GetConnectedProcessorType(sessionId);
    auto processCluster = SafeGetMap(sceneClusterMap_, sceneType);
    if (!processCluster) {
        return false;
    }
    return processCluster->HasSessionEffectNode(sessionId);
}

bool HpaeRendererManager::IsValidSession(uint32_t sessionId) const
{
    return sessionNodeMap_.find(sessionId) != sessionNodeMap_.end();
}

void HpaeRendererManager::PostCreateSessionEffect(uint32_t sessionId, const std::string &effectName,
    std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
    bool aissSwitchEnabled)
{
    auto request = [this, sessionId, effectName, instance, config, aissSwitchEnabled]() {
        int32_t ret = CreateSessionEffect(sessionId, effectName, instance, config, aissSwitchEnabled);
        if (ret != static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
            AUDIO_WARNING_LOG("PostCreateSessionEffect: CreateSessionEffect failed for session %{public}u,"
                " ret=%{public}d (system abnormal state, no rollback)", sessionId, ret);
        }
    };
    SendRequest(std::move(request), __func__);
}

int32_t HpaeRendererManager::PostDestroySessionEffect(uint32_t sessionId, bool finalizeNow)
{
    auto request = [this, sessionId, finalizeNow]() -> int32_t {
        return DestroySessionEffect(sessionId, finalizeNow);
    };
    int32_t ret = SendRequestSync<int32_t>(std::move(request), __func__, ERR_OPERATION_FAILED);
    CHECK_AND_RETURN_RET_LOG(ret == static_cast<int32_t>(SessionEffectErrCode::SUCCESS), ret,
        "PostDestroySessionEffect: session %{public}u destroy failed ret=%{public}d", sessionId, ret);
    return ret;
}

int32_t HpaeRendererManager::SetSessionEffectParameter(uint32_t sessionId, int32_t type, float value)
{
    auto it = sessionNodeMap_.find(sessionId);
    CHECK_AND_RETURN_RET_LOG(it != sessionNodeMap_.end(),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND),
        "SetSessionEffectParameter: session %{public}u not found", sessionId);
    HpaeProcessorType sceneType = GetConnectedProcessorType(sessionId);
    auto processCluster = SafeGetMap(sceneClusterMap_, sceneType);
    CHECK_AND_RETURN_RET_LOG(processCluster,
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND),
        "SetSessionEffectParameter: cluster not found");
    return processCluster->SetSessionEffectParameter(sessionId, type, value);
}

void HpaeRendererManager::PostSetSessionEffectParameter(uint32_t sessionId, int32_t type, float value)
{
    auto request = [this, sessionId, type, value]() {
        SetSessionEffectParameter(sessionId, type, value);
    };
    SendRequest(std::move(request), __func__);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
