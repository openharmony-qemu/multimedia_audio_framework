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
#include <atomic>
#include <unordered_map>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_engine.h"
#include "audio_suite_pipeline.h"

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
    RegisterHandler(SET_ENABLE_NODE, &AudioSuiteEngine::HandleEnableNode);
    RegisterHandler(GET_ENABLE_NODE, &AudioSuiteEngine::HandleGetEnableNode);
    RegisterHandler(SET_AUDIO_FORMAT, &AudioSuiteEngine::HandleSetAudioFormat);
    RegisterHandler(SET_WRITEDATA_CALLBACK, &AudioSuiteEngine::HandleSetWriteDataCallback);
    RegisterHandler(CONNECT_NODES, &AudioSuiteEngine::HandleConnectNodes);
    RegisterHandler(DISCONNECT_NODES, &AudioSuiteEngine::HandleDisConnectNodes);
    RegisterHandler(INSTALL_NODE_TAP, &AudioSuiteEngine::HandleInstallTap);
    RegisterHandler(REMOVE_NODE_TAP, &AudioSuiteEngine::HandleRemoveTap);
    RegisterHandler(RENDER_FRAME, &AudioSuiteEngine::HandleRenderFrame);

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
    engineThread_->ActivateThread(this);
    isInit_.store(true);
    AUDIO_INFO_LOG("AudioSuiteEngine::Init end");
    return SUCCESS;
}

int32_t AudioSuiteEngine::DeInit()
{
    if (engineThread_ != nullptr) {
        engineThread_->DeactivateThread();
        engineThread_ = nullptr;
    }
    engineNoLockQueue_.HandleRequests();

    isInit_.store(false);
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
    AUDIO_INFO_LOG("enter HandleMsg.");
    engineNoLockQueue_.HandleRequests();
}

void AudioSuiteEngine::SendRequest(Request &&request, std::string funcName)
{
    Trace trace("sendrequest::" + funcName);
    AUDIO_INFO_LOG("sendrequest:: funcName = %{public}s.", funcName.c_str());
    engineNoLockQueue_.PushRequest(std::move(request));
    CHECK_AND_RETURN_LOG(engineThread_, "engineThread_ is nullptr");
    engineThread_->Notify();
    AUDIO_INFO_LOG("sendrequest exit :: funcName = %{public}s.", funcName.c_str());
}

int32_t AudioSuiteEngine::CreatePipeline()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not create pipeline.");

    auto request = [this]() {
        AUDIO_INFO_LOG("CreatePipeline enter");

        if (pipelineMap_.size() >= engineCfg_.maxPipelineNum_) {
            AUDIO_ERR_LOG("engine create pipeline failed, more than max pipeline num.");
            managerCallback_.OnCreatePipeline(ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS, INVALID_PIPELINE_ID);
            return;
        }

        std::shared_ptr<AudioSuitePipeline> pipeline = std::make_shared<AudioSuitePipeline>(PIPELINE_EDIT_MODE);
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine create pipeline failed, malloc error.");
            managerCallback_.OnCreatePipeline(ERR_MEMORY_ALLOC_FAILED, INVALID_PIPELINE_ID);
            return;
        }

        auto ret = pipeline->Init();
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("engine create pipeline failed, init error, ret = %{public}d.", ret);
            managerCallback_.OnCreatePipeline(ret, INVALID_PIPELINE_ID);
            return;
        }

        pipeline->RegisterSendMsgCallback(weak_from_this());
        pipelineMap_[pipeline->GetPipelineId()] = pipeline;
        managerCallback_.OnCreatePipeline(SUCCESS, pipeline->GetPipelineId());
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::DestroyPipeline(uint32_t pipelineId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not create pipeline.");

    auto request = [this, pipelineId]() {
        AUDIO_INFO_LOG("DestroyPipeline enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine destroy pipeline failed, pipeline id is invailed.");
            managerCallback_.OnDestoryPipeline(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline != nullptr) {
            auto ret = pipeline->DeInit();
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("engine Destroy pipeline failed, pipeline deinit failed, ret = %{public}d.", ret);
                managerCallback_.OnDestoryPipeline(ret);
                return;
            }
        }

        pipelineMap_.erase(pipelineId);
        managerCallback_.OnDestoryPipeline(SUCCESS);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::StartPipeline(uint32_t pipelineId)
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "engine not init, can not create pipeline.");

    auto request = [this, pipelineId]() {
        AUDIO_INFO_LOG("StartPipeline enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
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
    auto request = [this, pipelineId]() {
        AUDIO_INFO_LOG("StopPipeline enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
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
    auto request = [this, pipelineId]() {
        AUDIO_INFO_LOG("GetPipelineState enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine GetPipelineState failed, pipeline id is invailed.");
            managerCallback_.OnGetPipelineState(PIPELINE_STOPPED);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine GetPipelineState pipeline failed, pipeline is nullptr.");
            managerCallback_.OnGetPipelineState(PIPELINE_STOPPED);
            return;
        }

        pipeline->GetPipelineState();
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::CreateNode(uint32_t pipelineId, AudioNodeBuilder& builder)
{
    auto request = [this, pipelineId, builder]() {
        AUDIO_INFO_LOG("CreateNode enter");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine CreateNode node failed, pipeline id is invailed.");
            managerCallback_.OnCreateNode(INVALID_NODE_ID);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine CreateNode failed, pipeline is nullptr.");
            managerCallback_.OnCreateNode(INVALID_NODE_ID);
            return;
        }

        int32_t ret = pipeline->CreateNode(builder);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline CreateNode node failed, ret = %{public}d.", ret);
            managerCallback_.OnCreateNode(INVALID_NODE_ID);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::DestroyNode(uint32_t nodeId)
{
    auto request = [this, nodeId]() {
        AUDIO_INFO_LOG("DestroyNode enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine DestroyNode node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnDestroyNode(ERR_INVALID_PARAM);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine DestroyNode node failed, pipelineId id=%{public}d is invailed.", pipelineId);
            managerCallback_.OnDestroyNode(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline DestroyNode node failed, pipeline is nullptr.");
            managerCallback_.OnDestroyNode(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
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

int32_t AudioSuiteEngine::EnableNode(uint32_t nodeId, AudioNodeEnable audioNoedEnable)
{
    auto request = [this, nodeId, audioNoedEnable]() {
        AUDIO_INFO_LOG("EnableNode enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine EnableNode node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnEnableNode(ERR_INVALID_PARAM);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine EnableNode node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnEnableNode(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline EnableNode node failed, pipeline is nullptr.");
            managerCallback_.OnEnableNode(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->EnableNode(nodeId, audioNoedEnable);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline EnableNode node failed, ret = %{public}d.", ret);
            managerCallback_.OnEnableNode(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::GetNodeEnableStatus(uint32_t nodeId)
{
    auto request = [this, nodeId]() {
        AUDIO_INFO_LOG("GetNodeEnableStatus enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine GetNodeEnableStatus node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnGetNodeEnable(NODE_DISABLE);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine GetNodeEnableStatus node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnGetNodeEnable(NODE_DISABLE);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline GetNodeEnableStatus node failed, pipeline is nullptr.");
            managerCallback_.OnGetNodeEnable(NODE_DISABLE);
            return;
        }

        int32_t ret = pipeline->GetNodeEnableStatus(nodeId);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline GetNodeEnableStatus node failed, ret = %{public}d.", ret);
            managerCallback_.OnGetNodeEnable(NODE_DISABLE);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat)
{
    auto request = [this, nodeId, audioFormat]() {
        AUDIO_INFO_LOG("SetAudioFormat enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine SetAudioFormat node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnSetAudioFormat(ERR_INVALID_PARAM);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine SetAudioFormat node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnSetAudioFormat(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline SetAudioFormat node failed, pipeline is nullptr.");
            managerCallback_.OnSetAudioFormat(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
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

int32_t AudioSuiteEngine::SetWriteDataCallback(uint32_t nodeId,
    std::shared_ptr<SuiteInputNodeWriteDataCallBack> callback)
{
    auto request = [this, nodeId, callback]() {
        AUDIO_INFO_LOG("SetWriteDataCallback enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine SetWriteDataCallback node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnWriteDataCallback(ERR_INVALID_PARAM);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine SetWriteDataCallback node failed, node id=%{public}d is invailed.", nodeId);
            managerCallback_.OnWriteDataCallback(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline SetWriteDataCallback node failed, pipeline is nullptr.");
            managerCallback_.OnSetAudioFormat(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->SetWriteDataCallback(nodeId, callback);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline SetWriteDataCallback node failed, ret = %{public}d.", ret);
            managerCallback_.OnWriteDataCallback(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId,
    AudioNodePortType srcPortType, AudioNodePortType destPortType)
{
    auto request = [this, srcNodeId, destNodeId, srcPortType, destPortType]() {
        AUDIO_INFO_LOG("ConnectNodes enter");
        if ((nodeMap_.find(srcNodeId) == nodeMap_.end()) || (nodeMap_.find(destNodeId) == nodeMap_.end())) {
            AUDIO_ERR_LOG("ConnectNodes, srcNodeId %{public}d or destNodeId %{public}d is invail.",
                srcNodeId, destNodeId);
            managerCallback_.OnConnectNodes(ERR_INVALID_PARAM);
            return;
        }

        if (nodeMap_[srcNodeId] != nodeMap_[destNodeId]) {
            AUDIO_ERR_LOG("ConnectNodes failed, not in one pipeline");
            managerCallback_.OnConnectNodes(ERR_INVALID_PARAM);
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

        int32_t ret = pipeline->ConnectNodes(srcNodeId, destNodeId, srcPortType, destPortType);
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
    auto request = [this, srcNodeId, destNodeId]() {
        AUDIO_INFO_LOG("DisConnectNodes enter");
        if ((nodeMap_.find(srcNodeId) == nodeMap_.end()) || (nodeMap_.find(destNodeId) == nodeMap_.end())) {
            AUDIO_ERR_LOG("DisConnectNodes, srcNodeId %{public}d or destNodeId %{public}d is invail.",
                srcNodeId, destNodeId);
            managerCallback_.OnDisConnectNodes(ERR_INVALID_PARAM);
            return;
        }

        if (nodeMap_[srcNodeId] != nodeMap_[destNodeId]) {
            AUDIO_ERR_LOG("DisConnectNodes failed, not in one pipeline");
            managerCallback_.OnDisConnectNodes(ERR_INVALID_PARAM);
            return;
        }

        auto pipelineId = nodeMap_[destNodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("DisConnectNodes failed, pipelineId=%{public}d is invailed.", pipelineId);
            managerCallback_.OnDisConnectNodes(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline DisConnectNodes node failed, pipeline is nullptr.");
            managerCallback_.OnDisConnectNodes(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
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

int32_t AudioSuiteEngine::InstallTap(uint32_t nodeId, AudioNodePortType portType,
    std::shared_ptr<SuiteNodeReadTapDataCallback> callback)
{
    auto request = [this, nodeId, portType, callback]() {
        AUDIO_INFO_LOG("InstallTap enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("InstallTap failed, nodeId %{public}d is invail.", nodeId);
            managerCallback_.OnInstallTap(ERR_INVALID_PARAM);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("InstallTap failed, pipelineId=%{public}d is invailed.", pipelineId);
            managerCallback_.OnInstallTap(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline InstallTap node failed, pipeline is nullptr.");
            managerCallback_.OnInstallTap(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->InstallTap(nodeId, portType, callback);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("InstallTap failed, ret = %{public}d.", ret);
            managerCallback_.OnInstallTap(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::RemoveTap(uint32_t nodeId, AudioNodePortType portType)
{
    auto request = [this, nodeId, portType]() {
        AUDIO_INFO_LOG("RemoveTap enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("RemoveTap failed, nodeId %{public}d is invail.", nodeId);
            managerCallback_.OnRemoveTap(ERR_INVALID_PARAM);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("RemoveTap failed, pipelineId=%{public}d is invailed.", pipelineId);
            managerCallback_.OnRemoveTap(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline RemoveTap node failed, pipeline is nullptr.");
            managerCallback_.OnRemoveTap(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        int32_t ret = pipeline->RemoveTap(nodeId, portType);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("RemoveTap failed, ret = %{public}d.", ret);
            managerCallback_.OnRemoveTap(ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::RenderFrame(uint32_t pipelineId,
    uint8_t *audioData, int32_t frameSize, int32_t *writeLen, bool *finishedFlag)
{
    auto request = [this, pipelineId, audioData, frameSize, writeLen, finishedFlag]() {
        AUDIO_INFO_LOG("AudioSuiteEngine::RenderFrame enter request");

        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine RenderFrame failed, pipeline id is invailed.");
            managerCallback_.OnRenderFrame(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        std::shared_ptr<IAudioSuitePipeline> pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("engine CreateNode failed, pipeline is nullptr.");
            managerCallback_.OnRenderFrame(ERR_AUDIO_SUITE_PIPELINE_NOT_EXIST);
            return;
        }

        pipeline->RenderFrame(audioData, frameSize, writeLen, finishedFlag);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuiteEngine::SetOptions(uint32_t nodeId, std::string name, std::string value)
{
    auto request = [this, nodeId, name, value]() {
        AUDIO_INFO_LOG("SetOptions enter");
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("engine SetOptions node failed, node id=%{public}d is invailed.", nodeId);
            return;
        }

        auto pipelineId = nodeMap_[nodeId];
        if (pipelineMap_.find(pipelineId) == pipelineMap_.end()) {
            AUDIO_ERR_LOG("engine SetOptions node failed, node id=%{public}d is invailed.", nodeId);
            return;
        }

        auto pipeline = pipelineMap_[pipelineId];
        if (pipeline == nullptr) {
            AUDIO_ERR_LOG("pipeline InstallTap node failed, pipeline is nullptr.");
            return;
        }

        int32_t ret = pipeline->SetOptions(nodeId, name, value);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("pipeline SetOptions node failed, ret = %{public}d.", ret);
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

void AudioSuiteEngine::HandleGetPipelineState(AudioSuitePipelineState state)
{
    managerCallback_.OnGetPipelineState(state);
}

void AudioSuiteEngine::HandleCreateNode(uint32_t nodeId, uint32_t pipelineId)
{
    if (nodeId != INVALID_NODE_ID) {
        nodeMap_[nodeId] = pipelineId;
    }
    managerCallback_.OnCreateNode(nodeId);
}

void AudioSuiteEngine::HandleDestroyNode(int32_t result, uint32_t nodeId)
{
    if (result == SUCCESS) {
        nodeMap_.erase(nodeId);
    }
    managerCallback_.OnDestroyNode(result);
}

void AudioSuiteEngine::HandleEnableNode(int32_t result)
{
    managerCallback_.OnEnableNode(result);
}

void AudioSuiteEngine::HandleGetEnableNode(AudioNodeEnable enable)
{
    managerCallback_.OnGetNodeEnable(enable);
}

void AudioSuiteEngine::HandleSetAudioFormat(int32_t result)
{
    managerCallback_.OnSetAudioFormat(result);
}

void AudioSuiteEngine::HandleSetWriteDataCallback(int32_t result)
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

void AudioSuiteEngine::HandleInstallTap(int32_t result)
{
    managerCallback_.OnInstallTap(result);
}
void AudioSuiteEngine::HandleRemoveTap(int32_t result)
{
    managerCallback_.OnRemoveTap(result);
}

void AudioSuiteEngine::HandleRenderFrame(int32_t result)
{
    managerCallback_.OnRenderFrame(result);
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
