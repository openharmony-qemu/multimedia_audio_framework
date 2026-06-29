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
int32_t HpaeRendererManager::CreateSessionEffect(uint32_t sessionId, const std::string &effectName,
    std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config)
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
    int32_t ret = processCluster->CreateSessionEffectNode(sessionId, effectName, instance, config);
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
    }
    return ret;
}

int32_t HpaeRendererManager::DestroySessionEffect(uint32_t sessionId)
{
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

    uint64_t latency = 0;
    int32_t ret = processCluster->DestroySessionEffectNode(sessionId, latency);
    if (ret != static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        AUDIO_WARNING_LOG("DestroySessionEffect failed for session %{public}u, ret %{public}d", sessionId, ret);
        return ret;
    }

    // RewindHistoryBuffer: return unconsumed data to SinkInputNode
    // Phase 2: latency=0, RewindBuffer(0) is no-op
    // Phase 3: latency>0, RewindBuffer returns data to historyBuffer
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    if (sinkInputNode != nullptr) {
        if (latency > 0) {
            sinkInputNode->RewindHistoryBuffer(latency);
        }
        sinkInputNode->ClearSessionEffectAttachState();
        AUDIO_INFO_LOG("[SE::RendererManager] AttachState cleared sessionId=%{public}u", sessionId);
    }
    AUDIO_INFO_LOG("[SE::RendererManager] DestroySessionEffect sessionId=%{public}u", sessionId);
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, it->second.state,
        OPERATION_SESSION_EFFECT_DISABLED);
    return ret;
}

void HpaeRendererManager::DetachSessionEffectNodeForRefresh(uint32_t sessionId, HpaeProcessorType oldSceneType)
{
    // Remove the physical SE node from the OLD cluster before a refresh-driven disconnect, WITHOUT
    // clearing attach state, so ConnectInputSession re-materializes it in the NEW cluster.
    auto processCluster = SafeGetMap(sceneClusterMap_, oldSceneType);
    CHECK_AND_RETURN_LOG(processCluster != nullptr,
        "DetachSessionEffectNodeForRefresh: old cluster not found sceneType %{public}d",
        static_cast<int32_t>(oldSceneType));
    CHECK_AND_RETURN_LOG(processCluster->HasSessionEffectNode(sessionId),
        "DetachSessionEffectNodeForRefresh: no SE node for session %{public}u", sessionId);
    // Capture lastSeStatus before destroy so materialize can restart preheat.
    auto seNode = processCluster->GetSessionEffectNode(sessionId);
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    if (seNode != nullptr && sinkInputNode != nullptr && sinkInputNode->HasSessionEffectAttachState()) {
        auto st = sinkInputNode->GetSessionEffectAttachState();
        if (st.has_value()) {
            st->lastSeStatus = static_cast<int32_t>(seNode->GetStatus());
            sinkInputNode->SetSessionEffectAttachState(st.value());
        }
    }
    uint64_t latency = 0;
    int32_t ret = processCluster->DestroySessionEffectNode(sessionId, latency);
    CHECK_AND_RETURN_LOG(ret == static_cast<int32_t>(SessionEffectErrCode::SUCCESS),
        "DetachSessionEffectNodeForRefresh: destroy failed session %{public}u ret %{public}d", sessionId, ret);
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
    std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config)
{
    auto request = [this, sessionId, effectName, instance, config]() {
        int32_t ret = CreateSessionEffect(sessionId, effectName, instance, config);
        if (ret != static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
            AUDIO_WARNING_LOG("PostCreateSessionEffect: CreateSessionEffect failed for session %{public}u,"
                " ret=%{public}d (system abnormal state, no rollback)", sessionId, ret);
        }
    };
    SendRequest(std::move(request), __func__);
}

void HpaeRendererManager::PostDestroySessionEffect(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        DestroySessionEffect(sessionId);
    };
    SendRequest(std::move(request), __func__);
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
