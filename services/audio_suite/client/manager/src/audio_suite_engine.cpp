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
#define LOG_TAG "AudioSuiteEngine"
#endif

#include <string>
#include <memory>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <set>
#include <sstream>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_engine.h"
#include "audio_suite_pipeline.h"
#include "stream_dfx_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using namespace OHOS::AudioStandard::HPAE;

AudioSuiteEngine::AudioSuiteEngine(AudioSuiteManagerCallback& callback)
    : managerCallback_(callback), engineNoLockQueue_(CURRENT_REQUEST_COUNT)
{
    RegisterHandler(START_PIPELINE, &AudioSuiteEngine::HandleStartPipeline);
    RegisterHandler(STOP_PIPELINE, &AudioSuiteEngine::HandleStopPipeline);
    RegisterHandler(GET_PIPELINE_STATE, &AudioSuiteEngine::HandleGetPipelineState);
    RegisterHandler(CREATE_NODE, &AudioSuiteEngine::HandleCreateNode);
    RegisterHandler(DESTROY_NODE, &AudioSuiteEngine::HandleDestroyNode);
    RegisterHandler(SET_BYPASS_STATUS, &AudioSuiteEngine::HandleBypassEffectNode);
    RegisterHandler(GET_BYPASS_STATUS, &AudioSuiteEngine::HandleGetNodeBypassStatus);
    RegisterHandler(SET_AUDIO_FORMAT, &AudioSuiteEngine::HandleSetAudioFormat);
    RegisterHandler(SET_WRITEDATA_CALLBACK, &AudioSuiteEngine::HandleSetRequestDataCallback);
    RegisterHandler(CONNECT_NODES, &AudioSuiteEngine::HandleConnectNodes);
    RegisterHandler(DISCONNECT_NODES, &AudioSuiteEngine::HandleDisConnectNodes);
    RegisterHandler(RENDER_FRAME, &AudioSuiteEngine::HandleRenderFrame);
    RegisterHandler(MULTI_RENDER_FRAME, &AudioSuiteEngine::HandleMultiRenderFrame);
    RegisterHandler(GET_OPTIONS, &AudioSuiteEngine::HandleGetOptions);
    RegisterHandler(SET_OPTIONS, &AudioSuiteEngine::HandleSetOptions);
    RegisterHandler(PRINT_INFO, &AudioSuiteEngine::HandlePrintInfo);

    AUDIO_INFO_LOG("AudioEditEngine Create");
}

AudioSuiteEngine::~AudioSuiteEngine()
{
    if (IsInit()) {
        DeInit();
    }
    AUDIO_INFO_LOG("AudioEditEngine Destroy");
}

int32_t AudioSuiteEngine::Init()
{
    if (IsInit()) {
        AUDIO_INFO_LOG("AudioSuiteEngine::Init failed, alreay inited");
        return ERR_ILLEGAL_STATE;
    }
    engineThread_ = std::make_unique<AudioSuiteManagerThread>();
    workGroupId_ = engineThread_->CreateGroup();
    engineThread_->ActivateThread(this, "SuiteEngine");
    isInit_.store(true);
    AUDIO_INFO_LOG("AudioSuiteEngine::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEngine::DeInit()
{
    isInit_.store(false);
    if (engineThread_ != nullptr) {
        engineThread_->ReleaseGroup();
        engineThread_->DeactivateThread();
        engineThread_ = nullptr;
    }
    engineNoLockQueue_.HandleRequests();

    nodeMap_.clear();
    for (auto& [id, pipeline] : pipelineMap_) {
        if (pipeline != nullptr) {
            pipeline->DeInit();
        }
    }
    pipelineMap_.clear();

    AUDIO_INFO_LOG("AudioSuiteEngine::DeInit end");
    return SUCCESS;
}

bool AudioSuiteEngine::IsInit()
{
    return isInit_.load();
}

bool AudioSuiteEngine::IsRunning(void)
{
    if (engineThread_ == nullptr) {
        return false;
    }
    return engineThread_->IsRunning();
}

bool AudioSuiteEngine::IsMsgProcessing()
{
    return !engineNoLockQueue_.IsFinishProcess();
}

void AudioSuiteEngine::HandleMsg()
{
    engineNoLockQueue_.HandleRequests();
}

void AudioSuiteEngine::SendRequest(Request &&request, std::string funcName)
{
    Trace trace("sendrequest::" + funcName);
    engineNoLockQueue_.PushRequest(std::move(request));
    CHECK_AND_RETURN_LOG(engineThread_, "engineThread_ is nullptr");
    engineThread_->Notify();
}

int32_t AudioSuiteEngine::CreatePipeline(PipelineWorkMode workMode)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not create pipeline.");
    auto request = [this, workMode]() {
        AUDIO_DEBUG_LOG("CreatePipeline enter");

        if (pipelineMap_.size() >= engineCfg_.maxPipelineNum_ ||
            (isExistRealtime_ == true && workMode == PIPELINE_REALTIME_MODE)) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::CREATE,
                               ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS),
                "engine create pipeline failed, more than max pipeline num.", true);
            AUDIO_ERR_LOG("engine create pipeline failed, more than max pipeline num.");
            managerCallback_.OnCreatePipeline(ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS, INVALID_PIPELINE_ID);
            return;
        }

        std::shared_ptr<AudioSuitePipeline> pipeline = std::make_shared<AudioSuitePipeline>(workMode);
        if (pipeline == nullptr) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::CREATE,
                               ERR_MEMORY_ALLOC_FAILED),
                "engine create pipeline failed, malloc error.", true);
            AUDIO_ERR_LOG("engine create pipeline failed, malloc error.");
            managerCallback_.OnCreatePipeline(ERR_MEMORY_ALLOC_FAILED, INVALID_PIPELINE_ID);
            return;
        }

        pipeline->SetWorkGroupId(workGroupId_);
        auto ret = pipeline->Init();
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("engine create pipeline failed, init error, ret = %{public}d.", ret);
            managerCallback_.OnCreatePipeline(ret, INVALID_PIPELINE_ID);
            return;
        }

        pipeline->RegisterSendMsgCallback(weak_from_this());
        pipelineMap_[pipeline->GetPipelineId()] = pipeline;
        if (workMode == PIPELINE_REALTIME_MODE) {
            isExistRealtime_ = true;
        }
        managerCallback_.OnCreatePipeline(SUCCESS, pipeline->GetPipelineId());
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::DestroyPipeline(uint32_t pipelineId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not Destroy pipeline.");

    auto request = [this, pipelineId]() {
        AUDIO_DEBUG_LOG("DestroyPipeline enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                               BusinessScenario::RELEASE, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST),
                "engine destroy pipeline failed, pipeline id is invailed.", true);
            AUDIO_ERR_LOG("engine destroy pipeline failed, pipeline id is invailed.");
            managerCallback_.OnDestroyPipeline(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        PipelineWorkMode workMode = PIPELINE_EDIT_MODE;
        if (pipeline != nullptr) {
            workMode = pipeline->GetWorkMode();
            auto ret = pipeline->DeInit();
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("engine Destroy pipeline failed, pipeline deinit failed, ret = %{public}d.", ret);
                managerCallback_.OnDestroyPipeline(ret);
                return;
            }
        }

        pipelineMap_.erase(pipelineId);
        isExistRealtime_ = (workMode == PIPELINE_REALTIME_MODE) ? false : isExistRealtime_;
        managerCallback_.OnDestroyPipeline(SUCCESS);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::StartPipeline(uint32_t pipelineId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not Start pipeline.");

    auto request = [this, pipelineId]() {
        AUDIO_DEBUG_LOG("StartPipeline enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::START,
                               ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST),
                "engine start pipeline failed, pipeline id is invailed.", true);
            AUDIO_ERR_LOG("engine start pipeline failed, pipeline id is invailed.");
            managerCallback_.OnStartPipeline(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine start pipeline failed, pipeline is nullptr.");
            managerCallback_.OnStartPipeline(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        pipeline->Start();
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::StopPipeline(uint32_t pipelineId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not stop pipeline.");

    auto request = [this, pipelineId]() {
        AUDIO_DEBUG_LOG("StopPipeline enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::STOP,
                               ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST),
                "engine stop pipeline failed, pipeline id is invailed.", true);
            AUDIO_ERR_LOG("engine stop pipeline failed, pipeline id is invailed.");
            managerCallback_.OnStopPipeline(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine stop pipeline failed, pipeline is nullptr.");
            managerCallback_.OnStopPipeline(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        pipeline->Stop();
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::GetPipelineState(uint32_t pipelineId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not get pipeline state.");

    auto request = [this, pipelineId]() {
        AUDIO_DEBUG_LOG("GetPipelineState enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine GetPipelineState failed, pipeline id is invailed.");
            managerCallback_.OnGetPipelineState(PIPELINE_STOPPED, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine GetPipelineState pipeline failed, pipeline is nullptr.");
            managerCallback_.OnGetPipelineState(PIPELINE_STOPPED, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        pipeline->GetPipelineState();
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::CreateNode(uint32_t pipelineId, AudioNodeBuilder& builder)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not CreateNode.");

    auto request = [this, pipelineId, builder]() {
        AUDIO_DEBUG_LOG("CreateNode enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine CreateNode node failed, pipeline id is invailed.");
            managerCallback_.OnCreateNode(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, INVALID_NODE_ID);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine CreateNode failed, pipeline is nullptr.");
            managerCallback_.OnCreateNode(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, INVALID_NODE_ID);
            return;
        }

        int32_t ret = pipeline->CreateNode(builder);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline CreateNode node failed, ret = %{public}d.", ret);
            managerCallback_.OnCreateNode(ret, INVALID_NODE_ID);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::DestroyNode(uint32_t nodeId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not DestroyNode.");

    auto request = [this, nodeId]() {
        AUDIO_DEBUG_LOG("DestroyNode enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine DestroyNode node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnDestroyNode(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine DestroyNode node failed, pipelineId id=%{public}d is invailed.", pipelineId);
            managerCallback_.OnDestroyNode(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline DestroyNode node failed, pipeline is nullptr.");
            managerCallback_.OnDestroyNode(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->DestroyNode(nodeId);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline DestroyNode node failed, ret = %{public}d.", ret);
            managerCallback_.OnDestroyNode(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::BypassEffectNode(uint32_t nodeId, bool bypass)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not enableNode.");

    auto request = [this, nodeId, bypass]() {
        AUDIO_DEBUG_LOG("BypassEffectNode enter");

        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine BypassEffectNode node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnBypassEffectNode(ERR_INVALID_PARAM);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine BypassEffectNode node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnBypassEffectNode(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline BypassEffectNode node failed, pipeline is nullptr.");
            managerCallback_.OnBypassEffectNode(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->BypassEffectNode(nodeId, bypass);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline BypassEffectNode node failed, ret = %{public}d.", ret);
            managerCallback_.OnBypassEffectNode(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::GetNodeBypassStatus(uint32_t nodeId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not get node status.");

    auto request = [this, nodeId]() {
        AUDIO_DEBUG_LOG("GetNodeBypassStatus enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine GetNodeBypassStatus node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnGetNodeBypass(ERR_AUDIO_SUITE_NODE_NOT_EXIST, false);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine GetNodeBypassStatus node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnGetNodeBypass(ERR_AUDIO_SUITE_NODE_NOT_EXIST, false);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline GetNodeBypassStatus node failed, pipeline is nullptr.");
            managerCallback_.OnGetNodeBypass(ERR_AUDIO_SUITE_NODE_NOT_EXIST, false);
            return;
        }

        int32_t ret = pipeline->GetNodeBypassStatus(nodeId);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline GetNodeBypassStatus node failed, ret = %{public}d.", ret);
            managerCallback_.OnGetNodeBypass(ret, false);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not SetAudioFormat.");

    auto request = [this, nodeId, audioFormat]() {
        AUDIO_DEBUG_LOG("SetAudioFormat enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine SetAudioFormat node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnSetAudioFormat(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine SetAudioFormat node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnSetAudioFormat(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline SetAudioFormat node failed, pipeline is nullptr.");
            managerCallback_.OnSetAudioFormat(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->SetAudioFormat(nodeId, audioFormat);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline SetAudioFormat node failed, ret = %{public}d.", ret);
            managerCallback_.OnSetAudioFormat(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::SetRequestDataCallback(uint32_t nodeId,
    std::shared_ptr<InputNodeRequestDataCallBack> callback)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not SetRequestDataCallback.");

    auto request = [this, nodeId, callback]() {
        AUDIO_DEBUG_LOG("SetRequestDataCallback enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine SetRequestDataCallback node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnWriteDataCallback(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine SetRequestDataCallback node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnWriteDataCallback(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline SetRequestDataCallback node failed, pipeline is nullptr.");
            managerCallback_.OnWriteDataCallback(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->SetRequestDataCallback(nodeId, callback);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline SetRequestDataCallback node failed, ret = %{public}d.", ret);
            managerCallback_.OnWriteDataCallback(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not ConnectNodes.");

    auto request = [this, srcNodeId, destNodeId]() {
        AUDIO_DEBUG_LOG("ConnectNodes enter");
        if ((nodeMap_.find(srcNodeId) == nodeMap_.end()) || (nodeMap_.find(destNodeId) == nodeMap_.end())) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                    BusinessScenario::CONFIG,
                    ERR_INVALID_PARAM),
                "The node does not exist.", true);
            AUDIO_ERR_LOG("ConnectNodes, srcNodeId %{public}d or destNodeId %{public}d is invail.", srcNodeId,
                          destNodeId);
            managerCallback_.OnConnectNodes(ERR_INVALID_PARAM);
            return;
        }

        if (nodeMap_[srcNodeId] != nodeMap_[destNodeId]) {
            AUDIO_ERR_LOG("ConnectNodes failed, not in one pipeline");
            managerCallback_.OnConnectNodes(ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);
            return;
        }

        auto pipelineId = nodeMap_[destNodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("ConnectNodes failed, pipelineId=%{public}d is invailed.", pipelineId);
            managerCallback_.OnConnectNodes(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline ConnectNodes node failed, pipeline is nullptr.");
            managerCallback_.OnConnectNodes(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->ConnectNodes(srcNodeId, destNodeId);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("ConnectNodes failed, ret = %{public}d.", ret);
            managerCallback_.OnConnectNodes(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not DisConnectNodes.");

    auto request = [this, srcNodeId, destNodeId]() {
        AUDIO_DEBUG_LOG("DisConnectNodes enter");
        if ((nodeMap_.find(srcNodeId) == nodeMap_.end()) || (nodeMap_.find(destNodeId) == nodeMap_.end())) {
            AUDIO_ERR_LOG("DisConnectNodes, srcNodeId %{public}d or destNodeId %{public}d is invail.",
                srcNodeId, destNodeId);
            managerCallback_.OnDisConnectNodes(ERR_NOT_SUPPORTED);
            return;
        }

        if (nodeMap_[srcNodeId] != nodeMap_[destNodeId]) {
            AUDIO_ERR_LOG("DisConnectNodes failed, not in one pipeline");
            managerCallback_.OnDisConnectNodes(ERR_NOT_SUPPORTED);
            return;
        }

        auto pipelineId = nodeMap_[destNodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("DisConnectNodes failed, pipelineId=%{public}d is invailed.", pipelineId);
            managerCallback_.OnDisConnectNodes(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline DisConnectNodes node failed, pipeline is nullptr.");
            managerCallback_.OnDisConnectNodes(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->DisConnectNodes(srcNodeId, destNodeId);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("DisConnectNodes failed, ret = %{public}d.", ret);
            managerCallback_.OnDisConnectNodes(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::RenderFrame(uint32_t pipelineId,
    uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not RenderFrame.");
    auto request = [this, pipelineId, audioData, requestFrameSize, responseSize, finishedFlag]() {
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                               BusinessScenario::RENDER, ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST),
                "engine RenderFrame failed, pipeline id is invailed.", true);
            AUDIO_ERR_LOG("engine RenderFrame failed, pipeline id is invailed.");
            managerCallback_.OnRenderFrame(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, pipelineId);
            return;
        }
        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine RenderFrame failed, pipeline is nullptr.");
            managerCallback_.OnRenderFrame(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, pipelineId);
            return;
        }
        pipeline->RenderFrame(audioData, requestFrameSize, responseSize, finishedFlag);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::MultiRenderFrame(uint32_t pipelineId,
    AudioDataArray *audioDataArray, int32_t *responseSize, bool *finishedFlag)
{
    auto request = [
        this, pipelineId, audioDataArray, responseSize, finishedFlag]() {
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine MultiRenderFrame failed, pipeline id is invailed.");
            managerCallback_.OnMultiRenderFrame(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, pipelineId);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine CreateNode failed, pipeline is nullptr.");
            managerCallback_.OnMultiRenderFrame(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST, pipelineId);
            return;
        }

        pipeline->MultiRenderFrame(reinterpret_cast<uint8_t **>(audioDataArray->audioDataArray),
            audioDataArray->arraySize, audioDataArray->requestFrameSize, responseSize, finishedFlag);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::SetOptions(uint32_t nodeId, std::string name, std::string value)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not SetOptions.");

    auto request = [this, nodeId, name, value]() {
        AUDIO_DEBUG_LOG("SetOptions enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine SetOptions node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnSetOptions(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine SetOptions node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnSetOptions(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline SetOptions node failed, pipeline is nullptr.");
            managerCallback_.OnSetOptions(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->SetOptions(nodeId, name, value);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline SetOptions node failed, ret = %{public}d.", ret);
            managerCallback_.OnSetOptions(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::GetOptions(uint32_t nodeId, std::string name, std::string &value)
{
    auto request = [this, nodeId, name, &value]() {
        AUDIO_DEBUG_LOG("GetOptions enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine GetOptions node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnGetOptions(ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine GetOptions node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnGetOptions(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline GetOptions node failed, pipeline is nullptr.");
            managerCallback_.OnGetOptions(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->GetOptions(nodeId, name, value);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline GetOptions node failed, ret = %{public}d.", ret);
            managerCallback_.OnGetOptions(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

void AudioSuiteEngine::Invoke(PipelineMsgCode cmdID, const std::any &args)
{
    auto it = handlers_.find(cmdID);
    if (it != handlers_.end()) {
        auto request = [it, args]() { it->second(args); };
        SendRequest(request, __func__);
        return;
    };
    AUDIO_ERR_LOG("AudioSuiteEngine::Invoke cmdID %{public}d not found", (int32_t)cmdID);
}

template <typename... Args>
void AudioSuiteEngine::RegisterHandler(PipelineMsgCode cmdID, void (AudioSuiteEngine::*func)(Args...))
{
    handlers_[cmdID] = [this, cmdID, func](const std::any &packedArgs) {
        // unpack args
        auto args = std::any_cast<std::tuple<Args...>>(&packedArgs);
        // print log if args parse error
        CHECK_AND_RETURN_LOG(args != nullptr, "cmdId %{public}d type mismatched", cmdID);
        std::apply(
            [this, func](
                auto &&...unpackedArgs) { (this->*func)(std::forward<decltype(unpackedArgs)>(unpackedArgs)...); },
            *args);
    };
}

void AudioSuiteEngine::HandleStartPipeline(int32_t result)
{
    managerCallback_.OnStartPipeline(result);
}

void AudioSuiteEngine::HandleStopPipeline(int32_t result)
{
    managerCallback_.OnStopPipeline(result);
}

void AudioSuiteEngine::HandleGetPipelineState(AudioSuitePipelineState state, int32_t result)
{
    managerCallback_.OnGetPipelineState(state, result);
}

void AudioSuiteEngine::HandleCreateNode(int32_t result, uint32_t nodeId, uint32_t pipelineId)
{
    if (nodeId != INVALID_NODE_ID) {
        nodeMap_[nodeId] = pipelineId;
    }
    managerCallback_.OnCreateNode(result, nodeId);
}

void AudioSuiteEngine::HandleDestroyNode(int32_t result, uint32_t nodeId)
{
    if (result == SUCCESS) {
        nodeMap_.erase(nodeId);
    }
    managerCallback_.OnDestroyNode(result);
}

void AudioSuiteEngine::HandleBypassEffectNode(int32_t result)
{
    managerCallback_.OnBypassEffectNode(result);
}

void AudioSuiteEngine::HandleGetNodeBypassStatus(int32_t result, bool bypassStatus)
{
    managerCallback_.OnGetNodeBypass(result, bypassStatus);
}

void AudioSuiteEngine::HandleSetAudioFormat(int32_t result)
{
    managerCallback_.OnSetAudioFormat(result);
}

void AudioSuiteEngine::HandleSetRequestDataCallback(int32_t result)
{
    managerCallback_.OnWriteDataCallback(result);
}

void AudioSuiteEngine::HandleConnectNodes(int32_t result)
{
    managerCallback_.OnConnectNodes(result);
}

void AudioSuiteEngine::HandleDisConnectNodes(int32_t result)
{
    managerCallback_.OnDisConnectNodes(result);
}

void AudioSuiteEngine::HandleRenderFrame(int32_t result, uint32_t pipelineId)
{
    managerCallback_.OnRenderFrame(result, pipelineId);
}

void AudioSuiteEngine::HandleMultiRenderFrame(int32_t result, uint32_t pipelineId)
{
    managerCallback_.OnMultiRenderFrame(result, pipelineId);
}

void AudioSuiteEngine::HandleGetOptions(int32_t result)
{
    managerCallback_.OnGetOptions(result);
}

void AudioSuiteEngine::HandleSetOptions(int32_t result)
{
    managerCallback_.OnSetOptions(result);
}

int32_t AudioSuiteEngine::PrintInfo(uint32_t pipelineId, int32_t fd)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not print info.");

    std::shared_ptr<IAudioSuitePipeline> targetPipeline = nullptr;
    if (pipelineId != INVALID_PIPELINE_ID) {
        auto it = pipelineMap_.find(pipelineId);
        CHECK_AND_RETURN_RET_LOG(it != pipelineMap_.end(), ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST,
            "PrintInfo pipeline not found, id = %{public}u",
            pipelineId);
        targetPipeline = it->second;
    }

    auto request = [this, pipelineId, fd, targetPipeline]() {
        AUDIO_DEBUG_LOG("PrintInfo enter");
        
        std::string content = GenerateTextSnapshot(pipelineId, targetPipeline);
        int32_t writeResult = WriteOutput(content, fd);
        managerCallback_.OnPrintInfo(writeResult);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

std::string AudioSuiteEngine::GenerateTextSnapshot(uint32_t pipelineId,
    std::shared_ptr<IAudioSuitePipeline> targetPipeline)
{
    std::stringstream ss;
    std::string timestamp = AudioSuiteUtil::GetCurrentTimestamp();

    ss << "========================================\n";
    ss << "Audio Suite Engine Snapshot\n";
    ss << "Timestamp: " << timestamp << "\n";
    ss << "========================================\n\n";

    if (pipelineId == INVALID_PIPELINE_ID) {
        ss << "Total Pipelines: " << pipelineMap_.size() << "\n\n";
        for (auto &[id, pipeline] : pipelineMap_) {
            PrintPipelineSnapshotText(pipeline, ss);
        }
    } else {
        PrintPipelineSnapshotText(targetPipeline, ss);
    }

    ss << "========================================\n";
    return ss.str();
}

void AudioSuiteEngine::PrintPipelineSnapshotText(std::shared_ptr<IAudioSuitePipeline> pipeline, std::stringstream& ss)
{
    uint32_t pipelineId = pipeline->GetPipelineId();
    AudioSuitePipelineState state = pipeline->GetCurrentPipelineState();
    PipelineWorkMode workMode = pipeline->GetWorkMode();
    const auto& nodeMap = pipeline->GetNodeMap();
    const auto& connections = pipeline->GetConnections();
    const auto& reverseConnections = pipeline->GetReverseConnections();

    ss << "Pipeline [ID: " << pipelineId << "]\n";
    ss << "  Work Mode: " << AudioSuiteUtil::GetWorkModeString(workMode) << "\n";
    ss << "  State: " << AudioSuiteUtil::GetStateString(state) << "\n";
    ss << "  Nodes: " << nodeMap.size() << "\n";
    ss << "  Connections: " << connections.size() << "\n\n";

    for (auto& [nodeId, node] : nodeMap) {
        PrintNodeSnapshotText(node, ss);
    }

    if (!connections.empty()) {
        ss << "  Connections:\n";
        for (auto& [srcId, destId] : connections) {
            ss << "    " << srcId << " -> " << destId << "\n";
        }
        ss << "\n";
    }

    if (!reverseConnections.empty()) {
        ss << "  Reverse Connections:\n";
        for (auto& [srcId, destIds] : reverseConnections) {
            for (auto destId : destIds) {
                ss << "    " << srcId << " -> " << destId << "\n";
            }
        }
        ss << "\n";
    }
}

void AudioSuiteEngine::PrintNodeSnapshotText(std::shared_ptr<AudioNode> node, std::stringstream & ss)
{
    AudioNodeInfo &info = node->GetAudioNodeInfo();   // 获取节点关键属性audioNodeInfo_

    ss << "  Node [ID: " << info.nodeId << ", Type: " << node->GetNodeTypeString() << "]\n";

    if (info.nodeType == NODE_TYPE_INPUT || info.nodeType == NODE_TYPE_OUTPUT ||
        info.nodeType == NODE_TYPE_AUDIO_MIXER) {
        ss << "    Format: sampleRate=" << info.audioFormat.rate
            << ", channels=" << info.audioFormat.audioChannelInfo.numChannels
            << ", format=" << AudioSuiteUtil::GetSampleFormatString(info.audioFormat.format) << "\n";
    }

    ss << "    Volume: " << info.volume << "\n";

    if (info.nodeType == NODE_TYPE_INPUT) {
        ss << "    Finished: " << (info.finishedFlag ? "true" : "false") << "\n";
    }

    std::string nodeInfo = "";
    int32_t result = SUCCESS;
    if (info.nodeType != NODE_TYPE_INPUT && info.nodeType != NODE_TYPE_OUTPUT) {
        ss << "    Bypass: " << (info.bypassStatus ? "true" : "false") << "\n";
        result = node->PrintNodeInfo(nodeInfo);
    }

    if (result == SUCCESS) {
        ss << nodeInfo;
    }
    ss << "\n";
}

int32_t AudioSuiteEngine::WriteOutput(const std::string &content, int32_t fd)
{
    if (fd < 0) {
        AUDIO_INFO_LOG("%{public}s", content.c_str());
        return SUCCESS;
    }

    const char *data = content.c_str();
    size_t totalSize = content.length();
    size_t writtenSize = 0;

    while (writtenSize < totalSize) {
        ssize_t bytesWritten = write(fd, data + writtenSize, totalSize - writtenSize);
        if (bytesWritten < 0) {
            if (errno == EINTR) {
                continue;
            }
            AUDIO_ERR_LOG("WriteOutput write failed, errno = %{public}d", errno);
            return ERR_AUDIO_SUITE_WRITE_FAILED;
        }
        writtenSize += static_cast<size_t>(bytesWritten);
    }
    return SUCCESS;
}

void AudioSuiteEngine::HandlePrintInfo(int32_t result, int32_t fd)
{
    managerCallback_.OnPrintInfo(result);
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
