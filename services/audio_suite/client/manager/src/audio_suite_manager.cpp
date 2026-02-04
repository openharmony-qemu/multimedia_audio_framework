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
#define LOG_TAG "AudioSuiteManager"
#endif

#include <mutex>
#include <string>
#include <atomic>
#include <memory>
#include <sstream>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_engine.h"
#include "audio_suite_manager_private.h"
#include "audio_suite_manager_callback.h"
#include "media_monitor_manager.h"
#include "media_monitor_info.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

namespace {
static const int32_t OPERATION_TIMEOUT_IN_MS = 10000;  // 10s
enum ErrorScene : uint32_t {
    PIPELINE_SCENE = 0,
    NODE_SCENE = 1,
};
enum PipelineErrorCase : uint32_t {
    CREATE_PIPELINE_ERROR = 0,
    DESTROY_PIPELINE_ERROR = 1,
    RENDER_PIPELINE_ERROR = 2,
};
enum NodeErrorCase : uint32_t {
    CREATE_NODE_ERROR = 0,
    DESTROY_NODE_ERROR = 1,
    CONNECT_NODE_ERROR = 2,
    DISCONNECT_NODE_ERROR = 3,
};
}

IAudioSuiteManager& IAudioSuiteManager::GetAudioSuiteManager()
{
    static AudioSuiteManager audioSuiteManager;
    return audioSuiteManager;
}

int32_t AudioSuiteManager::Init()
{
    AUDIO_DEBUG_LOG("Init enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ == nullptr, ERR_ILLEGAL_STATE, "suite engine already inited");

    suiteEngine_ = std::make_shared<AudioSuiteEngine>(*this);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr,
        ERR_MEMORY_ALLOC_FAILED, "Create suite engine failed, malloc error.");

    int32_t ret = suiteEngine_->Init();
    if (ret != SUCCESS) {
        suiteEngine_ = nullptr;
        HILOG_COMM_ERROR("[Init]engine init failed, ret = %{public}d", ret);
        return ret;
    }

    AUDIO_DEBUG_LOG("Init leave");
    return ret;
}

int32_t AudioSuiteManager::DeInit()
{
    AUDIO_DEBUG_LOG("DeInit enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_ILLEGAL_STATE, "suite engine not inited");
    std::vector<std::unique_lock<std::mutex>> pipelineLocks;
    for (auto& [id, lock] : pipelineLockMap_) {
        pipelineLocks.emplace_back(*lock);
    }
    int32_t ret = suiteEngine_->DeInit();
    CHECK_AND_CALL_FUNC_RETURN_RET(ret == SUCCESS, ret,
        HILOG_COMM_ERROR("[DeInit]engine deinit failed, ret = %{public}d", ret));

    suiteEngine_ = nullptr;
    AUDIO_DEBUG_LOG("DeInit leave");
    return ret;
}

int32_t AudioSuiteManager::CreatePipeline(uint32_t &pipelineId, PipelineWorkMode workMode)
{
    AUDIO_DEBUG_LOG("CreatePipeline enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishCreatePipeline_ = false;
    engineCreateResult_ = 0;
    int32_t ret = suiteEngine_->CreatePipeline(workMode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine CreatePipeline failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishCreatePipeline_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, CREATE_PIPELINE_ERROR, "CreatePipeline timeout");
        AUDIO_ERR_LOG("CreatePipeline timeout");
        return ERR_AUDIO_SUITE_TIMEOUT;
    }

    AUDIO_DEBUG_LOG("CreatePipeline leave");
    pipelineId = engineCreatePipelineId_;
    engineCreatePipelineId_ = INVALID_PIPELINE_ID;

    pipelineLockMap_[pipelineId] = std::make_unique<std::mutex>();
    pipelineCallbackMutexMap_[pipelineId] = std::make_unique<std::mutex>();
    pipelineCallbackCVMap_[pipelineId] = std::make_unique<std::condition_variable>();
    CHECK_AND_CALL_FUNC_RETURN_RET(engineCreateResult_ == SUCCESS, engineCreateResult_,
        HILOG_COMM_ERROR("[CreatePipeline]engine failed, engineCreateResult_ = %{public}d", engineCreateResult_));
    return engineCreateResult_;
}

int32_t AudioSuiteManager::DestroyPipeline(uint32_t pipelineId)
{
    AUDIO_DEBUG_LOG("DestroyPipeline enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    auto it = pipelineLockMap_.find(pipelineId);
    CHECK_AND_RETURN_RET_LOG(it != pipelineLockMap_.end(), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                             "pipeline lock not exist");
    auto &pipelineLock = it->second;
    CHECK_AND_RETURN_RET_LOG(pipelineLock != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                             "pipeline lock is null");
    std::lock_guard<std::mutex> pipelineLockGuard(*pipelineLock);

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishDestroyPipeline_ = false;
    int32_t ret = suiteEngine_->DestroyPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine DestroyPipeline failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishDestroyPipeline_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, DESTROY_PIPELINE_ERROR, "DestroyPipeline timeout");
        AUDIO_ERR_LOG("DestroyPipeline timeout");
        return ERR_AUDIO_SUITE_TIMEOUT;
    }
    isFinishRenderFrameMap_.erase(pipelineId);
    renderFrameResultMap_.erase(pipelineId);
    isFinishMultiRenderFrameMap_.erase(pipelineId);
    multiRenderFrameResultMap_.erase(pipelineId);
    pipelineLockMap_.erase(pipelineId);
    pipelineCallbackMutexMap_.erase(pipelineId);
    pipelineCallbackCVMap_.erase(pipelineId);

    AUDIO_DEBUG_LOG("DestroyPipeline leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(destroyPipelineResult_ == SUCCESS, destroyPipelineResult_,
        HILOG_COMM_ERROR("[DestroyPipeline]engine failed, destroyPipelineResult_ = %{public}d",
        destroyPipelineResult_));
    return destroyPipelineResult_;
}

int32_t AudioSuiteManager::StartPipeline(uint32_t pipelineId)
{
    AUDIO_DEBUG_LOG("StartPipeline enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishStartPipeline_ = false;
    int32_t ret = suiteEngine_->StartPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine StartPipeline failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishStartPipeline_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "StartPipeline timeout");

    AUDIO_DEBUG_LOG("StartPipeline leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(startPipelineResult_ == SUCCESS, startPipelineResult_,
        HILOG_COMM_ERROR("[StartPipeline]engine failed, startPipelineResult_ = %{public}d", startPipelineResult_));
    return startPipelineResult_;
}

int32_t AudioSuiteManager::StopPipeline(uint32_t pipelineId)
{
    AUDIO_DEBUG_LOG("StopPipeline enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishStopPipeline_ = false;
    int32_t ret = suiteEngine_->StopPipeline(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine StopPipeline failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishStopPipeline_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "StopPipeline timeout");

    AUDIO_DEBUG_LOG("StopPipeline leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(stopPipelineResult_ == SUCCESS, stopPipelineResult_,
        HILOG_COMM_ERROR("[StopPipeline]engine failed, stopPipelineResult_ = %{public}d", stopPipelineResult_));
    return stopPipelineResult_;
}

int32_t AudioSuiteManager::GetPipelineState(uint32_t pipelineId, AudioSuitePipelineState &state)
{
    AUDIO_DEBUG_LOG("GetPipelineState enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetPipelineState_ = false;
    getPipelineState_ = PIPELINE_STOPPED;
    int32_t ret = suiteEngine_->GetPipelineState(pipelineId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetPipelineState failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetPipelineState_;  // will be true when got notified.
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetPipelineState timeout");

    AUDIO_DEBUG_LOG("GetPipelineState leave");
    state = getPipelineState_;
    return SUCCESS;
}

int32_t AudioSuiteManager::CreateNode(uint32_t pipelineId, AudioNodeBuilder &builder, uint32_t &nodeId)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishCreateNode_ = false;
    engineCreateNodeResult_ = 0;
    int32_t ret = suiteEngine_->CreateNode(pipelineId, builder);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, INVALID_NODE_ID, "engine CreateNode failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishCreateNode_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(NODE_SCENE, CREATE_NODE_ERROR, "CreateNode timeout");
        AUDIO_ERR_LOG("CreateNode timeout");
        nodeId = INVALID_NODE_ID;
        return ERR_AUDIO_SUITE_TIMEOUT;
    }

    AUDIO_DEBUG_LOG("CreateNode leave");
    WriteSuiteEngineUtilizationStatsEvent(builder.nodeType);
    nodeId = engineCreateNodeId_;
    engineCreateNodeId_ = INVALID_NODE_ID;
    CHECK_AND_CALL_FUNC_RETURN_RET(engineCreateNodeResult_ == SUCCESS, engineCreateNodeResult_,
        HILOG_COMM_ERROR("[CreateNode]engine failed, engineCreateNodeResult_ = %{public}d",
        engineCreateNodeResult_));
    return engineCreateNodeResult_;
}

int32_t AudioSuiteManager::DestroyNode(uint32_t nodeId)
{
    AUDIO_DEBUG_LOG("DestroyNode enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishDestroyNode_ = false;
    int32_t ret = suiteEngine_->DestroyNode(nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine DestroyNode failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishDestroyNode_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(NODE_SCENE, DESTROY_NODE_ERROR, "DestroyNode timeout");
        AUDIO_ERR_LOG("DestroyNode timeout");
        return ERR_AUDIO_SUITE_TIMEOUT;
    }

    AUDIO_DEBUG_LOG("DestroyNode leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(destroyNodeResult_ == SUCCESS, destroyNodeResult_,
        HILOG_COMM_ERROR("[DestroyNode]engine failed, destroyNodeResult_ = %{public}d", destroyNodeResult_));
    return destroyNodeResult_;
}

int32_t AudioSuiteManager::BypassEffectNode(uint32_t nodeId, bool bypass)
{
    AUDIO_DEBUG_LOG("BypassEffectNode enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishBypassEffectNode_ = false;
    int32_t ret = suiteEngine_->BypassEffectNode(nodeId, bypass);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine BypassEffectNode failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishBypassEffectNode_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "BypassEffectNode timeout");

    AUDIO_DEBUG_LOG("BypassEffectNode leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(bypassEffectNodeResult_ == SUCCESS, bypassEffectNodeResult_,
        HILOG_COMM_ERROR("[BypassEffectNode]engine failed, bypassEffectNodeResult_ = %{public}d",
        bypassEffectNodeResult_));
    return bypassEffectNodeResult_;
}

int32_t AudioSuiteManager::GetNodeBypassStatus(uint32_t nodeId, bool &bypass)
{
    AUDIO_DEBUG_LOG("GetNodeBypassStatus enter.");

    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetNodeBypassStatus_ = false;
    getNodeBypassResult_ = false;
    int32_t ret = suiteEngine_->GetNodeBypassStatus(nodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetNodeBypassStatus failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetNodeBypassStatus_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetNodeBypassStatus timeout");

    AUDIO_DEBUG_LOG("GetNodeBypassStatus leave");
    bypass = getNodeBypassResult_;
    return SUCCESS;
}

int32_t AudioSuiteManager::SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat)
{
    AUDIO_DEBUG_LOG("SetAudioFormat enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetFormat_ = false;
    int32_t ret = suiteEngine_->SetAudioFormat(nodeId, audioFormat);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetAudioFormat failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishSetFormat_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetAudioFormat timeout");

    AUDIO_DEBUG_LOG("SetAudioFormat leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(setFormatResult_ == SUCCESS, setFormatResult_,
        HILOG_COMM_ERROR("[SetAudioFormat]engine failed, setFormatResult_ = %{public}d", setFormatResult_));
    return setFormatResult_;
}

int32_t AudioSuiteManager::SetRequestDataCallback(uint32_t nodeId,
    std::shared_ptr<InputNodeRequestDataCallBack> callback)
{
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetWriteData_ = false;
    int32_t ret = suiteEngine_->SetRequestDataCallback(nodeId, callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetRequestDataCallback failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishSetWriteData_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetRequestDataCallback timeout");

    AUDIO_DEBUG_LOG("SetRequestDataCallback leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(setWriteDataResult_ == SUCCESS, setWriteDataResult_,
        HILOG_COMM_ERROR("[SetRequestDataCallback]engine failed, setWriteDataResult_ = %{public}d",
        setWriteDataResult_));
    return setWriteDataResult_;
}

int32_t AudioSuiteManager::ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    AUDIO_DEBUG_LOG("ConnectNodes enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishConnectNodes_ = false;
    int32_t ret = suiteEngine_->ConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine ConnectNodes failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishConnectNodes_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(NODE_SCENE, CONNECT_NODE_ERROR, "ConnectNodes timeout");
        AUDIO_ERR_LOG("ConnectNodes timeout");
        return ERR_AUDIO_SUITE_TIMEOUT;
    }

    AUDIO_DEBUG_LOG("ConnectNodes leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(connectNodesResult_ == SUCCESS, connectNodesResult_,
        HILOG_COMM_ERROR("[ConnectNodes]engine failed, connectNodesResult_ = %{public}d", connectNodesResult_));
    return connectNodesResult_;
}

int32_t AudioSuiteManager::DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    AUDIO_DEBUG_LOG("DisConnectNodes enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishDisConnectNodes_ = false;
    int32_t ret = suiteEngine_->DisConnectNodes(srcNodeId, destNodeId);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine DisConnectNodes failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishDisConnectNodes_;
    });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(NODE_SCENE, DISCONNECT_NODE_ERROR, "DisConnectNodes timeout");
        AUDIO_ERR_LOG("DisConnectNodes timeout");
        return ERR_AUDIO_SUITE_TIMEOUT;
    }

    AUDIO_DEBUG_LOG("DisConnectNodes leave");
    CHECK_AND_CALL_FUNC_RETURN_RET(disConnectNodesResult_ == SUCCESS, disConnectNodesResult_,
        HILOG_COMM_ERROR("[DisConnectNodes]engine failed, disConnectNodesResult_ = %{public}d",
        disConnectNodesResult_));
    return disConnectNodesResult_;
}

int32_t AudioSuiteManager::SetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains gains)
{
    AUDIO_DEBUG_LOG("SetEqualizerFrequencyBandGains enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "";
    for (size_t idx = 0; idx < sizeof(gains.gains) / sizeof(gains.gains[0]); idx++) {
        value += std::to_string(gains.gains[idx]);
        value += ":";
    }
    HILOG_COMM_INFO("[SetEqualizerFrequencyBandGains]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "engine SetEqualizerFrequencyBandGains failed, ret = %{public}d", ret);
    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetEqualizerFrequencyBandGains timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetEqualizerFrequencyBandGains]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));

    return ret;
}

int32_t AudioSuiteManager::SetSpaceRenderPositionParams(uint32_t nodeId, AudioSpaceRenderPositionParams position)
{
    AUDIO_DEBUG_LOG("SetSpaceRenderPositionParams enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioSpaceRenderPositionParams";
    std::string value =
        std::to_string(position.x) + "," + std::to_string(position.y) + "," + std::to_string(position.z);
    HILOG_COMM_INFO("[SetSpaceRenderPositionParams]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetSpaceRenderPositionParams failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetSpaceRenderPositionParams timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetSpaceRenderPositionParams]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::GetSpaceRenderPositionParams(uint32_t nodeId, AudioSpaceRenderPositionParams &position)
{
    AUDIO_DEBUG_LOG("GetSpaceRenderPositionParams enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioSpaceRenderPositionParams";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetSpaceRenderPositionParams failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishGetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetSpaceRenderPositionParams timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetSpaceRenderPositionParams]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));

    ParseValue(value, position);
    return SUCCESS;
}

int32_t AudioSuiteManager::SetSpaceRenderRotationParams(uint32_t nodeId, AudioSpaceRenderRotationParams rotation)
{
    AUDIO_DEBUG_LOG("SetSpaceRenderRotationParams enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioSpaceRenderRotationParams";
    std::string value = std::to_string(rotation.x) + "," + std::to_string(rotation.y) + "," +
                        std::to_string(rotation.z) + "," + std::to_string(rotation.surroundTime) + "," +
                        std::to_string(static_cast<int32_t>(rotation.surroundDirection));
    HILOG_COMM_INFO("[SetSpaceRenderRotationParams]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(
        ret == SUCCESS, ret, "engine AudioSpaceRenderRotationParams failed, ret = %{public}d", ret);
    
    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetSpaceRenderRotationParams timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetSpaceRenderRotationParams]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::GetSpaceRenderRotationParams(uint32_t nodeId, AudioSpaceRenderRotationParams &rotation)
{
    AUDIO_DEBUG_LOG("GetSpaceRenderRotationParams enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioSpaceRenderRotationParams";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetSpaceRenderRotationParams failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishGetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetSpaceRenderRotationParams timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetSpaceRenderRotationParams]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));

    ParseValue(value, rotation);
    return SUCCESS;
}

int32_t AudioSuiteManager::SetSpaceRenderExtensionParams(uint32_t nodeId, AudioSpaceRenderExtensionParams extension)
{
    AUDIO_DEBUG_LOG("SetSpaceRenderExtensionParams enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioSpaceRenderExtensionParams";
    std::string value = std::to_string(extension.extRadius) + "," + std::to_string(extension.extAngle);
    HILOG_COMM_INFO("[SetSpaceRenderExtensionParams]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(
        ret == SUCCESS, ret, "engine AudioSpaceRenderExtensionParams failed, ret = %{public}d", ret);
        
    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetSpaceRenderExtensionParams timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetSpaceRenderExtensionParams]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::GetSpaceRenderExtensionParams(uint32_t nodeId, AudioSpaceRenderExtensionParams &extension)
{
    AUDIO_DEBUG_LOG("GetSpaceRenderExtensionParams enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioSpaceRenderExtensionParams";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetSpaceRenderExtensionParams failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishGetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetSpaceRenderExtensionParams timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetSpaceRenderExtensionParams]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));

    ParseValue(value, extension);
    return SUCCESS;
}

int32_t AudioSuiteManager::SetTempoAndPitch(uint32_t nodeId, float speed, float pitch)
{
    AUDIO_DEBUG_LOG("SetTempoAndPitch enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "speedAndPitch";
    std::string value = std::to_string(speed) + "," + std::to_string(pitch);
    HILOG_COMM_INFO("[SetTempoAndPitch]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetTempoAndPitch failed, ret = %{public}d", ret);
    
    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetTempoAndPitch timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetTempoAndPitch]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::GetTempoAndPitch(uint32_t nodeId, float &speed, float &pitch)
{
    AUDIO_DEBUG_LOG("GetTempoAndPitch enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "speedAndPitch";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetTempoAndPitch failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishGetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetTempoAndPitch timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetTempoAndPitch]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));

    ParseValue(value, speed, pitch);
    return SUCCESS;
}

int32_t AudioSuiteManager::SetPureVoiceChangeOption(uint32_t nodeId, AudioPureVoiceChangeOption option)
{
    AUDIO_DEBUG_LOG("SetPureVoiceChangeOption enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioPureVoiceChangeOption";
    std::string value = std::to_string(static_cast<int32_t>(option.optionGender)) + "," +
                        std::to_string(static_cast<int32_t>(option.optionType)) + "," +
                        std::to_string(static_cast<float>(option.pitch));
    HILOG_COMM_INFO("[SetPureVoiceChangeOption]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetPureVoiceChangeOption failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetPureVoiceChangeOption timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetPureVoiceChangeOption]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::GetPureVoiceChangeOption(uint32_t nodeId, AudioPureVoiceChangeOption &option)
{
    AUDIO_DEBUG_LOG("GetPureVoiceChangeOption enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioPureVoiceChangeOption";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine AudioPureVoiceChangeOption failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishGetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetPureVoiceChangeOption timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetPureVoiceChangeOption]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));

    ParseValue(value, option);
    return SUCCESS;
}

int32_t AudioSuiteManager::SetGeneralVoiceChangeType(uint32_t nodeId, AudioGeneralVoiceChangeType type)
{
    AUDIO_DEBUG_LOG("SetGeneralVoiceChangeType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioGeneralVoiceChangeType";
    std::string value = std::to_string(static_cast<int32_t>(type));
    HILOG_COMM_INFO("[SetGeneralVoiceChangeType]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetGeneralVoiceChangeType failed, ret = %{public}d", ret);
    
    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetGeneralVoiceChangeType timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetGeneralVoiceChangeType]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::GetGeneralVoiceChangeType(uint32_t nodeId, AudioGeneralVoiceChangeType &type)
{
    AUDIO_DEBUG_LOG("GetGeneralVoiceChangeType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioGeneralVoiceChangeType";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine AudioGeneralVoiceChangeType failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishGetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetGeneralVoiceChangeType timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetGeneralVoiceChangeType]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));

    int32_t parseValue = StringToInt32(value);
    type = static_cast<AudioGeneralVoiceChangeType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuiteManager::SetSoundFieldType(uint32_t nodeId, SoundFieldType soundFieldType)
{
    AUDIO_DEBUG_LOG("SetSoundFieldType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "SoundFieldType";
    std::string value = std::to_string(static_cast<int32_t>(soundFieldType));
    HILOG_COMM_INFO("[SetSoundFieldType]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetSoundFieldType failed, ret = %{public}d", ret);
    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetSoundFieldType timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetSoundFieldType]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::SetEnvironmentType(uint32_t nodeId, EnvironmentType environmentType)
{
    AUDIO_DEBUG_LOG("EnvironmentType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "EnvironmentType";
    std::string value = std::to_string(static_cast<int32_t>(environmentType));
    HILOG_COMM_INFO("[SetEnvironmentType]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine EnvironmentType failed, ret = %{public}d", ret);
    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetEnvironmentType timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetEnvironmentType]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType)
{
    AUDIO_DEBUG_LOG("SetVoiceBeautifierType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST, "suite engine not inited");

    std::string name = "VoiceBeautifierType";
    std::string value = std::to_string(static_cast<int32_t>(voiceBeautifierType));
    HILOG_COMM_INFO("[SetVoiceBeautifierType]engine set name: %{public}s value: %{public}s",
        name.c_str(), value.c_str());

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = false;
    int32_t ret = suiteEngine_->SetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine SetVoiceBeautifierType failed, ret = %{public}d", ret);
    bool stopWaiting = callbackCV_.wait_for(
        waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] { return isFinishSetOptions_; });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "SetVoiceBeautifierType timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(setOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetVoiceBeautifierType]engine SetOptions failed, setOptionsResult_ = %{public}d",
        setOptionsResult_));
    return ret;
}

int32_t AudioSuiteManager::GetEnvironmentType(uint32_t nodeId, EnvironmentType &environmentType)
{
    AUDIO_DEBUG_LOG("GetEnvironmentType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "EnvironmentType";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine EnvironmentType failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetOptions_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetEnvironmentType timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetEnvironmentType]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));
    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(EnvironmentType::AUDIO_SUITE_ENVIRONMENT_TYPE_CLOSE)
        || parseValue > static_cast<int32_t>(EnvironmentType::AUDIO_SUITE_ENVIRONMENT_TYPE_GRAMOPHONE)) {
        return ERROR;
    }
    environmentType = static_cast<EnvironmentType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuiteManager::GetSoundFieldType(uint32_t nodeId, SoundFieldType &soundFieldType)
{
    AUDIO_DEBUG_LOG("GetSoundFieldType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "SoundFieldType";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine GetSoundFieldType failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetOptions_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetSoundFieldType timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetSoundFieldType]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));
    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(SoundFieldType::AUDIO_SUITE_SOUND_FIELD_CLOSE)
        || parseValue > static_cast<int32_t>(SoundFieldType::AUDIO_SUITE_SOUND_FIELD_WIDE)) {
        return ERROR;
    }
    soundFieldType = static_cast<SoundFieldType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuiteManager::GetEqualizerFrequencyBandGains(uint32_t nodeId,
    AudioEqualizerFrequencyBandGains &frequencyBandGains)
{
    AUDIO_DEBUG_LOG("GetEqualizerFrequencyBandGains enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "engine GetEqualizerFrequencyBandGains failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetOptions_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetEqualizerFrequencyBandGains timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetEqualizerFrequencyBandGains]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));
    int32_t parseValue[EQUALIZER_BAND_NUM] = {0};
    ParseValue(value, parseValue);
    for (size_t idx = 0; idx < sizeof(parseValue) / sizeof(parseValue[0]); idx++) {
        frequencyBandGains.gains[idx] = parseValue[idx];
    }
    return SUCCESS;
}

int32_t AudioSuiteManager::GetVoiceBeautifierType(uint32_t nodeId,
    VoiceBeautifierType &voiceBeautifierType)
{
    AUDIO_DEBUG_LOG("GetVoiceBeautifierType enter.");
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr, ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

    std::string name = "VoiceBeautifierType";
    std::string value = "";

    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = false;
    int32_t ret = suiteEngine_->GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret,
        "engine GetVoiceBeautifierType failed, ret = %{public}d", ret);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetOptions_;
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, ERR_AUDIO_SUITE_TIMEOUT, "GetVoiceBeautifierType timeout");
    CHECK_AND_CALL_FUNC_RETURN_RET(getOptionsResult_ == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[GetVoiceBeautifierType]engine GetOptions failed, getOptionsResult_ = %{public}d",
        getOptionsResult_));
    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(VoiceBeautifierType::AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR)
        || parseValue > static_cast<int32_t>(VoiceBeautifierType::AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_STUDIO)) {
        return ERROR;
    }
    voiceBeautifierType = static_cast<VoiceBeautifierType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuiteManager::RenderFrame(uint32_t pipelineId,
    uint8_t *audioData, int32_t frameSize, int32_t *writeLen, bool *finishedFlag)
{
    std::mutex* pipelineLock = nullptr;
    {
        std::lock_guard<std::mutex> lock(lock_);
        CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr,
            ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, "suite engine not inited");

        auto it = pipelineLockMap_.find(pipelineId);
        CHECK_AND_RETURN_RET_LOG(it != pipelineLockMap_.end(), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                                 "pipeline lock not exist");
        pipelineLock = it->second.get();
        CHECK_AND_RETURN_RET_LOG(pipelineLock != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                                 "pipeline lock is null");
    }

    std::lock_guard<std::mutex> lock(*pipelineLock);
    auto &callbackMutex = pipelineCallbackMutexMap_[pipelineId];

    std::unique_lock<std::mutex> waitLock(*callbackMutex);
    isFinishRenderFrameMap_[pipelineId] = false;
    int32_t ret = suiteEngine_->RenderFrame(pipelineId, audioData, frameSize, writeLen, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine RenderFrame failed, ret = %{public}d", ret);

    auto &callbackCV = pipelineCallbackCVMap_[pipelineId];
    bool stopWaiting = callbackCV->wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS),
        [this, pipelineId] { return isFinishRenderFrameMap_[pipelineId]; });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, RENDER_PIPELINE_ERROR, "RenderFrame timeout");
        AUDIO_ERR_LOG("RenderFrame timeout");
        return ERR_AUDIO_SUITE_TIMEOUT;
    }

    return renderFrameResultMap_[pipelineId];
}

int32_t AudioSuiteManager::MultiRenderFrame(uint32_t pipelineId,
    AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag)
{
    std::mutex* pipelineLock = nullptr;
    {
        std::lock_guard<std::mutex> lock(lock_);
        CHECK_AND_RETURN_RET_LOG(suiteEngine_ != nullptr,
            ERR_AUDIO_SUITE_ENGINE_NOT_EXIST, "suite engine not inited");

        auto it = pipelineLockMap_.find(pipelineId);
        CHECK_AND_RETURN_RET_LOG(it != pipelineLockMap_.end(), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                                 "pipeline lock not exist");
        pipelineLock = it->second.get();
        CHECK_AND_RETURN_RET_LOG(pipelineLock != nullptr, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
                                 "pipeline lock is null");
    }
    std::lock_guard<std::mutex> lock(*pipelineLock);
    auto &callbackMutex = pipelineCallbackMutexMap_[pipelineId];

    std::unique_lock<std::mutex> waitLock(*callbackMutex);
    isFinishMultiRenderFrameMap_[pipelineId] = false;
    int32_t ret = suiteEngine_->MultiRenderFrame(
        pipelineId, audioDataArray, responseSize, finishedFlag);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "engine RenderFrame failed, ret = %{public}d", ret);

    auto &callbackCV = pipelineCallbackCVMap_[pipelineId];
    bool stopWaiting = callbackCV->wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS),
        [this, pipelineId] { return isFinishMultiRenderFrameMap_[pipelineId]; });
    if (!stopWaiting) {
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, RENDER_PIPELINE_ERROR, "MultiRenderFrame timeout");
        AUDIO_ERR_LOG("MultiRenderFrame timeout");
        return ERR_AUDIO_SUITE_TIMEOUT;
    }

    return multiRenderFrameResultMap_[pipelineId];
}

int32_t AudioSuiteManager::IsNodeTypeSupported(AudioNodeType  nodeType, bool *isSupported)
{
    AUDIO_DEBUG_LOG("isNodeTypeSupported enter.");
    if (nodeType == NODE_TYPE_AUDIO_MIXER) {
        AUDIO_INFO_LOG("MixerNode is supported on all device.");
        *isSupported = true;
        return SUCCESS;
    }

    AudioSuiteCapabilities &audioSuiteCapabilities = AudioSuiteCapabilities::GetInstance();
    int32_t ret = audioSuiteCapabilities.IsNodeTypeSupported(nodeType, isSupported);
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("nodeType: %{public}d is supported  on this device.", nodeType);
    } else {
        HILOG_COMM_ERROR("[IsNodeTypeSupported]engine failed, Wrong effect nodeType: %{public}d", nodeType);
        *isSupported = false;
    }
    return SUCCESS;
}

void AudioSuiteManager::OnCreatePipeline(int32_t result, uint32_t pipelineId)
{
    if (result != SUCCESS &&
        result != ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS) {
        std::ostringstream errorDescription;
        errorDescription << "engine CreatePipeline failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, CREATE_PIPELINE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnCreatePipeline enter");
    isFinishCreatePipeline_ = true;
    engineCreateResult_ = result;
    engineCreatePipelineId_ = pipelineId;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnDestroyPipeline(int32_t result)
{
    if (result != SUCCESS &&
        result != ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST) {
        std::ostringstream errorDescription;
        errorDescription << "engine DestroyPipeline failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, DESTROY_PIPELINE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_INFO_LOG("OnDestroyPipeline result: %{public}d", result);
    isFinishDestroyPipeline_ = true;
    destroyPipelineResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnStartPipeline(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnStartPipeline enter");
    isFinishStartPipeline_ = true;
    startPipelineResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnStopPipeline(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnStopPipeline enter");
    isFinishStopPipeline_ = true;
    stopPipelineResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnGetPipelineState(AudioSuitePipelineState state)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnGetPipelineState enter");
    isFinishGetPipelineState_ = true;
    getPipelineState_ = state;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnCreateNode(int32_t result, uint32_t nodeId)
{
    if (result != SUCCESS) {
        std::ostringstream errorDescription;
        errorDescription << "engine CreateNode failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(NODE_SCENE, CREATE_NODE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnCreateNode enter");
    isFinishCreateNode_ = true;
    engineCreateNodeResult_ = result;
    engineCreateNodeId_ = nodeId;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnDestroyNode(int32_t result)
{
    if (result != SUCCESS) {
        std::ostringstream errorDescription;
        errorDescription << "engine DestroyNode failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(NODE_SCENE, DESTROY_NODE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnDestroyNode enter");
    isFinishDestroyNode_ = true;
    destroyNodeResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnBypassEffectNode(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnEnableNode enter");
    isFinishBypassEffectNode_ = true;
    bypassEffectNodeResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnGetNodeBypass(int32_t result, bool bypassStatus)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnGetNodeBypass enter");
    isFinishGetNodeBypassStatus_ = true;
    getNodeBypassResult_ = bypassStatus;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnSetAudioFormat(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnSetAudioFormat enter");
    isFinishSetFormat_ = true;
    setFormatResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnWriteDataCallback(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnWriteDataCallback enter");
    isFinishSetWriteData_ = true;
    setWriteDataResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnConnectNodes(int32_t result)
{
    if (result != SUCCESS &&
        result != ERR_AUDIO_SUITE_UNSUPPORT_CONNECT) {
        std::ostringstream errorDescription;
        errorDescription << "engine ConnectNodes failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(NODE_SCENE, CONNECT_NODE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnConnectNodes enter");
    isFinishConnectNodes_ = true;
    connectNodesResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnDisConnectNodes(int32_t result)
{
    if (result != SUCCESS &&
        result != ERR_NOT_SUPPORTED) {
        std::ostringstream errorDescription;
        errorDescription << "engine DisConnectNodes failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(NODE_SCENE, DISCONNECT_NODE_ERROR, errorDescription.str());
    }
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    AUDIO_DEBUG_LOG("OnDisConnectNodes enter");
    isFinishDisConnectNodes_ = true;
    disConnectNodesResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnRenderFrame(int32_t result, uint32_t pipelineId)
{
    if (result != SUCCESS &&
        result != ERR_NOT_SUPPORTED &&
        result != ERR_ILLEGAL_STATE) {
        std::ostringstream errorDescription;
        errorDescription << "engine RenderFrame failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, RENDER_PIPELINE_ERROR, errorDescription.str());
    }
    auto &callbackMutex = pipelineCallbackMutexMap_[pipelineId];
    auto &callbackCV = pipelineCallbackCVMap_[pipelineId];
    std::unique_lock<std::mutex> waitLock(*callbackMutex);
    isFinishRenderFrameMap_[pipelineId] = true;
    renderFrameResultMap_[pipelineId] = result;
    callbackCV->notify_all();
}

void AudioSuiteManager::OnMultiRenderFrame(int32_t result, uint32_t pipelineId)
{
    if (result != SUCCESS &&
        result != ERR_NOT_SUPPORTED &&
        result != ERR_ILLEGAL_STATE) {
        std::ostringstream errorDescription;
        errorDescription << "engine MultiRenderFrame failed, ret = " << result;
        WriteSuiteEngineExceptionEvent(PIPELINE_SCENE, RENDER_PIPELINE_ERROR, errorDescription.str());
    }
    auto &callbackMutex = pipelineCallbackMutexMap_[pipelineId];
    auto &callbackCV = pipelineCallbackCVMap_[pipelineId];
    std::unique_lock<std::mutex> waitLock(*callbackMutex);
    isFinishMultiRenderFrameMap_[pipelineId] = true;
    multiRenderFrameResultMap_[pipelineId] = result;
    callbackCV->notify_all();
}

void AudioSuiteManager::OnGetOptions(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetOptions_ = true;
    getOptionsResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::OnSetOptions(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishSetOptions_ = true;
    setOptionsResult_ = result;
    callbackCV_.notify_all();
}

void AudioSuiteManager::WriteSuiteEngineUtilizationStatsEvent(AudioNodeType nodeType)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::SUITE_ENGINE_UTILIZATION_STATS,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    std::string nodeTypeStr = "";
    auto it = NODETYPE_TOSTRING_MAP.find(nodeType);
    if (it != NODETYPE_TOSTRING_MAP.end()) {
        nodeTypeStr = it->second;
    }
    bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
    bean->Add("AUDIO_NODE_TYPE", nodeTypeStr);
    bean->Add("AUDIO_NODE_COUNT", static_cast<int32_t>(1));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioSuiteManager::WriteSuiteEngineExceptionEvent(uint32_t scene, uint32_t result, std::string description)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::SUITE_ENGINE_EXCEPTION,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
    bean->Add("ERROR_SCENE", static_cast<int32_t>(scene));
    bean->Add("ERROR_CASE", static_cast<int32_t>(result));
    bean->Add("ERROR_DESCRIPTION", description);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
