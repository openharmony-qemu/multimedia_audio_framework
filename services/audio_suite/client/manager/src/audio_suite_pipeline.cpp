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
#define LOG_TAG "AudioSuitePipeline"
#endif

#include <string>
#include <atomic>
#include <limits>
#include <unordered_map>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_pipeline.h"
#include "audio_suite_input_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_mixer_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

using namespace OHOS::AudioStandard::HPAE;

std::mutex AudioSuitePipeline::allocateIdLock;
uint32_t AudioSuitePipeline::allocateId = 0;

AudioSuitePipeline::AudioSuitePipeline(PipelineWorkMode mode)
    : pipelineWorkMode_(mode), nodeCounts_(NODE_TYPE_AUDIO_MIXER + 1, 0), pipelineNoLockQueue_(CURRENT_REQUEST_COUNT)
{
    std::lock_guard<std::mutex> lock(allocateIdLock);
    id_ = ++allocateId;

    if (allocateId == std::numeric_limits<uint32_t>::max()) {
        allocateId = 0;
    }
    AUDIO_INFO_LOG("Create AudioSuitePipeline class sucess. id_ = %{public}u", id_);
}

AudioSuitePipeline::~AudioSuitePipeline()
{
    if (IsInit()) {
        DeInit();
    }
    AUDIO_INFO_LOG("Destroy AudioSuitePipeline class finish. id_ = %{public}u", id_);
}

int32_t AudioSuitePipeline::Init()
{
    if (IsInit()) {
        AUDIO_INFO_LOG("AudioSuitePipeline::Init failed, alreay inited");
        return ERR_ILLEGAL_STATE;
    }
    pipelineThread_ = std::make_unique<AudioSuiteManagerThread>();
    pipelineThread_->ActivateThread(this);
    isInit_.store(true);
    AUDIO_INFO_LOG("AudioSuitePipeline::Init end");
    return SUCCESS;
}

int32_t AudioSuitePipeline::DeInit()
{
    if (pipelineThread_ != nullptr) {
        pipelineThread_->DeactivateThread();
        pipelineThread_ = nullptr;
    }
    pipelineNoLockQueue_.HandleRequests();

    isInit_.store(false);
    AUDIO_INFO_LOG("AudioSuitePipeline::DeInit end");
    return SUCCESS;
}

bool AudioSuitePipeline::IsInit()
{
    return isInit_.load();
}

bool AudioSuitePipeline::IsRunning(void)
{
    if (pipelineThread_ == nullptr) {
        return false;
    }
    return pipelineThread_->IsRunning();
}

bool AudioSuitePipeline::IsMsgProcessing()
{
    return !pipelineNoLockQueue_.IsFinishProcess();
}

void AudioSuitePipeline::HandleMsg()
{
    pipelineNoLockQueue_.HandleRequests();
}

void AudioSuitePipeline::SendRequest(Request &&request, std::string funcName)
{
    Trace trace("sendrequest::" + funcName);
    pipelineNoLockQueue_.PushRequest(std::move(request));
    CHECK_AND_RETURN_LOG(pipelineThread_, "pipelineThread_ is nullptr");
    pipelineThread_->Notify();
}

uint32_t AudioSuitePipeline::GetPipelineId()
{
    return id_;
}

int32_t AudioSuitePipeline::Start()
{
    auto request = [this]() {
        if (pipelineState_ == PIPELINE_RUNNING) {
            AUDIO_INFO_LOG("Current pipeline alreay running, id is %{public}d", id_);
            TriggerCallback(START_PIPELINE, ERR_ILLEGAL_STATE);
        }

        if (outputNode_ == nullptr) {
            AUDIO_INFO_LOG("Current pipeline not have output node");
            TriggerCallback(START_PIPELINE, ERR_ILLEGAL_STATE);
        }

        if (!CheckPipelineNode(outputNode_->GetAudioNodeId())) {
            AUDIO_INFO_LOG("Current pipeline node connet status error, id is %{public}d", id_);
            TriggerCallback(START_PIPELINE, ERR_ILLEGAL_STATE);
        }

        AUDIO_INFO_LOG("Start pipeline, id is %{public}d", id_);
        pipelineState_ = PIPELINE_RUNNING;
        TriggerCallback(START_PIPELINE, SUCCESS);
    };
    SendRequest(request, __func__);

    return SUCCESS;
}

int32_t AudioSuitePipeline::Stop()
{
    auto request = [this]() {
        AUDIO_INFO_LOG("Stop pipeline, id is %{public}d", id_);

        if (pipelineState_ == PIPELINE_STOPPED) {
            AUDIO_INFO_LOG("Current pipeline alreay stop, id is %{public}d", id_);
            TriggerCallback(STOP_PIPELINE, ERR_ILLEGAL_STATE);
        }

        for (const auto& [nodeId, node] : nodeMap_) {
            if (node != nullptr) {
                node->Flush();
                node->SetAudioNodeDataFinishedFlag(false);
            }
        }
        pipelineState_ = PIPELINE_STOPPED;

        TriggerCallback(STOP_PIPELINE, SUCCESS);
    };
    SendRequest(request, __func__);

    return SUCCESS;
}

int32_t AudioSuitePipeline::GetPipelineState()
{
    auto request = [this]() {
        AUDIO_INFO_LOG("GetPipelineState is %{public}d", static_cast<int32_t>(pipelineState_));
        TriggerCallback(GET_PIPELINE_STATE, pipelineState_);
    };
    SendRequest(request, __func__);

    return SUCCESS;
}

int32_t AudioSuitePipeline::CreateNode(AudioNodeBuilder builder)
{
    auto request = [this, builder]() {
        AUDIO_INFO_LOG("CreateNode enter");
        std::shared_ptr<AudioNode> node = nullptr;

        if (CreateNodeCheckParme(builder) != SUCCESS) {
            AUDIO_ERR_LOG("create node check parme failed.");
            TriggerCallback(CREATE_NODE, INVALID_NODE_ID, id_);
            return;
        }

        node = CreateNodeForType(builder);
        if (node == nullptr) {
            AUDIO_ERR_LOG("create node failed, malloc error.");
            TriggerCallback(CREATE_NODE, INVALID_NODE_ID, id_);
            return;
        }
        int32_t ret = node->Init();
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("create node failed, init node error, ret = %{public}d.", ret);
            TriggerCallback(CREATE_NODE, INVALID_NODE_ID, id_);
            return;
        }

        nodeMap_[node->GetAudioNodeId()] = node;
        nodeCounts_[static_cast<std::size_t>(builder.nodeType)]++;
        AUDIO_INFO_LOG("CreateNode finish");
        TriggerCallback(CREATE_NODE, node->GetAudioNodeId(), id_);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::CreateNodeCheckParme(AudioNodeBuilder builder)
{
    if (pipelineWorkMode_ == PIPELINE_REALTIME_MODE) {
        if ((builder.nodeType != NODE_TYPE_INPUT) &&
            (builder.nodeType != NODE_TYPE_OUTPUT) &&
            (builder.nodeType != NODE_TYPE_EQUALIZER)) {
            AUDIO_ERR_LOG("pipline in REALTIME mode, only can craet input, output and equalizer node.");
            return ERR_NOT_SUPPORTED;
        }
    }

    if (nodeCounts_[static_cast<std::size_t>(builder.nodeType)] >= GetMaxNodeNumsForType(builder.nodeType)) {
        AUDIO_ERR_LOG("node create node failed, current type node max num is %{public}u.",
            nodeCounts_[static_cast<std::size_t>(builder.nodeType)]);
        return ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS;
    }

    return SUCCESS;
}

uint32_t AudioSuitePipeline::GetMaxNodeNumsForType(AudioNodeType type)
{
    if (type == NODE_TYPE_INPUT) {
        return pipelineCfg_.maxInputNodeNum_;
    }

    if (type == NODE_TYPE_OUTPUT) {
        return pipelineCfg_.maxOutputNodeNum_;
    }

    if (type == NODE_TYPE_AUDIO_MIXER) {
        return pipelineCfg_.maxMixNodeNum_;
    }

    return pipelineCfg_.maxEffectNodeNum_;
}

std::shared_ptr<AudioNode> AudioSuitePipeline::CreateNodeForType(AudioNodeBuilder builder)
{
    std::shared_ptr<AudioNode> node = nullptr;
    AudioFormat audioFormat = builder.nodeFormat;

    if (builder.nodeType == NODE_TYPE_INPUT) {
        AUDIO_INFO_LOG("Create AudioInputNode");
        node = std::make_shared<AudioInputNode>(audioFormat);
    } else if (builder.nodeType == NODE_TYPE_OUTPUT) {
        AUDIO_INFO_LOG("Create AudioOutputNode");
        outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
        node = std::static_pointer_cast<AudioNode>(outputNode_);
    } else if (builder.nodeType == NODE_TYPE_AUDIO_MIXER) {
        AUDIO_INFO_LOG("Create AudioSuiteMixerNode");
        node = std::make_shared<AudioSuiteMixerNode>(NODE_TYPE_AUDIO_MIXER, audioFormat);
    }

    return node;
}

int32_t AudioSuitePipeline::DestroyNode(uint32_t nodeId)
{
    auto request = [this, nodeId]() {
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("DestroyNode node failed, node id is invailed.");
            TriggerCallback(DESTROY_NODE, ERR_INVALID_PARAM, id_);
            return;
        }

        auto node = nodeMap_[nodeId];
        if (node == nullptr) {
            AUDIO_ERR_LOG("DestroyNode failed, node ptr nullptr.");
            TriggerCallback(DESTROY_NODE, ERR_AUDIO_SUITE_NODE_NOT_EXIST, id_);
            return;
        }

        int32_t ret = SUCCESS;
        if (pipelineState_ == PIPELINE_RUNNING) {
            ret = DestroyNodeForRun(nodeId, node);
        } else {
            ret = DestroyNodeForStop(nodeId, node);
        }
        if (ret != SUCCESS) {
            TriggerCallback(DESTROY_NODE, ret, id_);
            return;
        }

        AUDIO_INFO_LOG("DestroyNode success. nodeId = %{public}d.", nodeId);
        nodeCounts_[static_cast<std::size_t>(node->GetNodeType())]--;
        TriggerCallback(DESTROY_NODE, SUCCESS, id_);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::DestroyNodeForStop(uint32_t nodeId, std::shared_ptr<AudioNode> node)
{
    RemovceForwardConnet(nodeId, node);
    RemovceBackwardConnet(nodeId, node);

    node->DeInit();
    nodeMap_.erase(nodeId);
    return SUCCESS;
}

int32_t AudioSuitePipeline::DestroyNodeForRun(uint32_t nodeId, std::shared_ptr<AudioNode> node)
{
    if (outputNode_ == nullptr) {
        AUDIO_ERR_LOG("DestroyNode failed, pipeline running, can not find output node, nodeId = %{public}d.", nodeId);
        return ERR_ILLEGAL_STATE;
    }

    // In the running state, the node is connected to the output node and cannot be deleted.
    if (IsConnected(outputNode_->GetAudioNodeId(), nodeId)) {
        AUDIO_ERR_LOG("DestroyNode failed, pipeline running, can not destroy used node, nodeId = %{public}d.", nodeId);
        return ERR_ILLEGAL_STATE;
    }

    RemovceForwardConnet(nodeId, node);
    RemovceBackwardConnet(nodeId, node);

    node->DeInit();
    nodeMap_.erase(nodeId);
    return SUCCESS;
}

int32_t AudioSuitePipeline::EnableNode(uint32_t nodeId, AudioNodeEnable audioNoedEnable)
{
    auto request = [this, nodeId, audioNoedEnable]() {
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("EnableNode node failed, node id is invailed.");
            TriggerCallback(SET_ENABLE_NODE, ERR_INVALID_PARAM);
            return;
        }

        auto node = nodeMap_[nodeId];
        if (node == nullptr) {
            AUDIO_ERR_LOG("EnableNode failed, node ptr nullptr.");
            TriggerCallback(SET_ENABLE_NODE, ERR_INVALID_PARAM);
            return;
        }

        if ((node->GetNodeType() == NODE_TYPE_INPUT) || (node->GetNodeType() == NODE_TYPE_OUTPUT)) {
            AUDIO_ERR_LOG("input or output node not support set enable, nodeId = %{public}d.", nodeId);
            TriggerCallback(SET_ENABLE_NODE, ERR_INVALID_OPERATION);
            return;
        }

        node->SetNodeEnableStatus(audioNoedEnable);
        TriggerCallback(SET_ENABLE_NODE, SUCCESS);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::GetNodeEnableStatus(uint32_t nodeId)
{
    auto request = [this, nodeId]() {
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("GetNodeEnableStatus node failed, node id is invailed.");
            TriggerCallback(GET_ENABLE_NODE, NODE_DISABLE);
            return;
        }

        auto node = nodeMap_[nodeId];
        if (node == nullptr) {
            AUDIO_ERR_LOG("GetNodeEnableStatus failed, node ptr nullptr.");
            TriggerCallback(GET_ENABLE_NODE, NODE_DISABLE);
            return;
        }

        AudioNodeEnable enable = node->GetNodeEnableStatus();
        TriggerCallback(GET_ENABLE_NODE, enable);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat)
{
    auto request = [this, nodeId, audioFormat]() {
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("SetAudioFormat node failed, node id is invailed.");
            TriggerCallback(SET_AUDIO_FORMAT, ERR_INVALID_PARAM);
            return;
        }

        auto node = nodeMap_[nodeId];
        if (node == nullptr) {
            AUDIO_ERR_LOG("SetAudioFormat failed, node ptr nullptr.");
            TriggerCallback(SET_AUDIO_FORMAT, ERR_INVALID_PARAM);
            return;
        }

        node->SetAudioNodeFormat(audioFormat);
        TriggerCallback(SET_AUDIO_FORMAT, SUCCESS);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}


int32_t AudioSuitePipeline::SetWriteDataCallback(uint32_t nodeId,
    std::shared_ptr<SuiteInputNodeWriteDataCallBack> callback)
{
    auto request = [this, nodeId, callback]() {
        if (pipelineState_ != PIPELINE_STOPPED) {
            AUDIO_ERR_LOG("SetWriteDataCallback failed, pipelineState status is not stopped.");
            TriggerCallback(SET_WRITEDATA_CALLBACK, ERR_ILLEGAL_STATE);
            return;
        }

        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("SetWriteDataCallback failed, node id is invailed.");
            TriggerCallback(SET_WRITEDATA_CALLBACK, ERR_INVALID_PARAM);
            return;
        }

        auto node = nodeMap_[nodeId];
        if (node->GetNodeType() != NODE_TYPE_INPUT) {
            AUDIO_ERR_LOG("SetWriteDataCallback failed, node type must input type.");
            TriggerCallback(SET_WRITEDATA_CALLBACK, ERR_INVALID_PARAM);
            return;
        }

        int32_t ret = node->SetOnWriteDataCallback(callback);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("SetOnWriteDataCallback, ret = %{public}d.", ret);
            TriggerCallback(SET_WRITEDATA_CALLBACK, ret);
            return;
        }
        TriggerCallback(SET_WRITEDATA_CALLBACK, SUCCESS);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId,
    AudioNodePortType srcPortType, AudioNodePortType destPortType)
{
    auto request = [this, srcNodeId, destNodeId, srcPortType]() {
        if (srcNodeId == destNodeId) {
            AUDIO_ERR_LOG("ConnectNodes failed, srcNodeId can not same destNodeId.");
            TriggerCallback(CONNECT_NODES, ERR_INVALID_PARAM);
            return;
        }

        if ((nodeMap_.find(srcNodeId) == nodeMap_.end()) || (nodeMap_.find(destNodeId) == nodeMap_.end())) {
            AUDIO_ERR_LOG("ConnectNodes failed, node id is invailed.");
            TriggerCallback(CONNECT_NODES, ERR_INVALID_PARAM);
            return;
        }

        auto srcNode = nodeMap_[srcNodeId];
        auto destNode = nodeMap_[destNodeId];
        if ((srcNode == nullptr) || (destNode == nullptr)) {
            AUDIO_ERR_LOG("ConnectNodes failed, node ptr is nullptr.");
            TriggerCallback(CONNECT_NODES, ERR_AUDIO_SUITE_NODE_NOT_EXIST);
            return;
        }

        if ((srcNode->GetNodeType() == NODE_TYPE_OUTPUT) || (destNode->GetNodeType() == NODE_TYPE_INPUT)) {
            AUDIO_ERR_LOG("ConnectNodes failed, node type error.");
            TriggerCallback(CONNECT_NODES, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT);
            return;
        }

        if (IsDirectConnected(srcNodeId, destNodeId)) {
            AUDIO_INFO_LOG("srcNodeId = %{public}d and destNodeId = %{public}d already connet", srcNodeId, destNodeId);
            TriggerCallback(CONNECT_NODES, SUCCESS);
        }

        int32_t ret = SUCCESS;
        if (pipelineState_ == PIPELINE_STOPPED) {
            ret = ConnectNodesForStop(srcNodeId, destNodeId, srcNode, destNode, srcPortType);
        } else {
            ret = ConnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode, srcPortType);
        }
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("ConnectNodes failed, ret = %{public}d, srcNodeId = %{public}d, "
                "destNodeId = %{public}d.", ret, srcNodeId, destNodeId);
            TriggerCallback(SET_WRITEDATA_CALLBACK, ret);
            return;
        }

        AddNodeConnections(srcNodeId, destNodeId);

        AUDIO_INFO_LOG("ConnectNodes success.");
        TriggerCallback(CONNECT_NODES, SUCCESS);
        return;
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::ConnectNodesForStop(uint32_t srcNodeId, uint32_t destNodeId,
    std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode, AudioNodePortType srcPortType)
{
    RemovceBackwardConnet(srcNodeId, srcNode);
    if (destNode->GetNodeType() != NODE_TYPE_AUDIO_MIXER) {
        RemovceForwardConnet(destNodeId, destNode);
    }

    return destNode->Connect(srcNode, srcPortType);
}

int32_t AudioSuitePipeline::ConnectNodesForRun(uint32_t srcNodeId, uint32_t destNodeId,
    std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode, AudioNodePortType srcPortType)
{
    if (outputNode_ == nullptr) {
        AUDIO_ERR_LOG("ConnectNodes failed, pipeline running, can not find output node.");
        return ERR_ILLEGAL_STATE;
    }

    // srcNodeId in pipline running nodes
    if (IsConnected(outputNode_->GetAudioNodeId(), srcNodeId)) {
        AUDIO_ERR_LOG("ConnectNodes failed, pipeline running srcNode = %{public}d can not is used node.", srcNodeId);
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }

    // srcNodeId and destNodeId are not in pipline running nodes
    if (!IsConnected(outputNode_->GetAudioNodeId(), destNodeId)) {
        RemovceBackwardConnet(srcNodeId, srcNode);
        RemovceForwardConnet(destNodeId, destNode);
        return destNode->Connect(srcNode, srcPortType);
    }

    // destNodeId in pipline running nodes
    if (destNode->GetNodeType() != NODE_TYPE_AUDIO_MIXER) {
        AUDIO_ERR_LOG("Pipeline status is running, destNodeId = %{public}d type must mix node", destNodeId);
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }
    // srcNodeId must connet from inputNode and not rings
    if (!CheckPipelineNode(srcNodeId)) {
        AUDIO_ERR_LOG("Pipeline status is running, srcNodeId = %{public}d must connet from inputnode", srcNodeId);
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }

    RemovceBackwardConnet(srcNodeId, srcNode);
    return destNode->Connect(srcNode, srcPortType);
}

int32_t AudioSuitePipeline::DisConnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    auto request = [this, srcNodeId, destNodeId]() {
        if (srcNodeId == destNodeId) {
            AUDIO_ERR_LOG("DisConnectNodes failed, srcNodeId same destNodeId.");
            TriggerCallback(DISCONNECT_NODES, ERR_ILLEGAL_STATE);
            return;
        }

        if ((nodeMap_.find(srcNodeId) == nodeMap_.end()) || (nodeMap_.find(destNodeId) == nodeMap_.end())) {
            AUDIO_ERR_LOG("DisConnectNodes failed, node id is invailed.");
            TriggerCallback(DISCONNECT_NODES, ERR_INVALID_PARAM);
            return;
        }

        auto srcNode = nodeMap_[srcNodeId];
        auto destNode = nodeMap_[destNodeId];
        if ((srcNode == nullptr) || (destNode == nullptr)) {
            AUDIO_ERR_LOG("DisConnectNodes failed, node ptr is nullptr.");
            TriggerCallback(DISCONNECT_NODES, ERR_INVALID_PARAM);
            return;
        }

        if (!IsDirectConnected(srcNodeId, destNodeId)) {
            AUDIO_ERR_LOG("DisConnectNodes failed, srcNodeId = %{public}d not connet destNodeId = %{public}d.",
                srcNodeId, destNodeId);
            TriggerCallback(DISCONNECT_NODES, ERR_INVALID_PARAM);
        }

        int32_t ret = SUCCESS;
        if (pipelineState_ == PIPELINE_STOPPED) {
            ret = destNode->DisConnect(srcNode);
        } else {
            ret = DisConnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
        }
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("DisConnectNodes failed, ret = %{public}d, srcNodeId = %{public}d, "
                "destNodeId = %{public}d.", ret, srcNodeId, destNodeId);
            TriggerCallback(SET_WRITEDATA_CALLBACK, ret);
            return;
        }

        ClearNodeConnections(srcNodeId, destNodeId);
        AUDIO_INFO_LOG("DisConnectNodes success.");
        TriggerCallback(DISCONNECT_NODES, SUCCESS);
        return;
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::DisConnectNodesForRun(uint32_t srcNodeId, uint32_t destNodeId,
    std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode)
{
    if (outputNode_ == nullptr) {
        AUDIO_ERR_LOG("DisConnectNodes failed, pipeline running, can not find output node.");
        return ERR_ILLEGAL_STATE;
    }

    // destNodeId not in pipline running nodes
    if (!IsConnected(outputNode_->GetAudioNodeId(), destNodeId)) {
        return destNode->DisConnect(srcNode);
    }

    if (destNode->GetNodeType() != NODE_TYPE_AUDIO_MIXER) {
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }

    if (reverseConnections_.find(destNodeId) == reverseConnections_.end()) {
        return ERR_ILLEGAL_STATE;
    }

    if (reverseConnections_[destNodeId].size() <= 1) {
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }

    return destNode->DisConnect(srcNode);
}


void AudioSuitePipeline::RemovceForwardConnet(uint32_t nodeId, std::shared_ptr<AudioNode> node)
{
    if (reverseConnections_.find(nodeId) == reverseConnections_.end()) {
        return;
    }

    auto vec = reverseConnections_[nodeId];
    for (const auto& srcNodeId : vec) {
        if (nodeMap_.find(srcNodeId) == nodeMap_.end()) {
            continue;
        }

        auto srcNode = nodeMap_[srcNodeId];
        if (srcNode == nullptr) {
            return;
        }

        node->DisConnect(srcNode);
        ClearNodeConnections(srcNodeId, nodeId);
    }
}

void AudioSuitePipeline::RemovceBackwardConnet(uint32_t nodeId, std::shared_ptr<AudioNode> node)
{
    if (connections_.find(nodeId) == connections_.end()) {
        return;
    }

    auto destNodeId = connections_[nodeId];
    if (nodeMap_.find(destNodeId) ==  nodeMap_.end()) {
        return;
    }

    auto destNode = nodeMap_[destNodeId];
    if (destNode == nullptr) {
        return;
    }

    destNode->DisConnect(node);
    ClearNodeConnections(nodeId, destNodeId);
}

void AudioSuitePipeline::AddNodeConnections(uint32_t srcNodeId, uint32_t destNodeId)
{
    connections_[srcNodeId] = destNodeId;
    reverseConnections_[destNodeId].push_back(srcNodeId);
}

void AudioSuitePipeline::ClearNodeConnections(uint32_t srcNodeId, uint32_t destNodeId)
{
    connections_.erase(srcNodeId);

    auto it = reverseConnections_.find(destNodeId);
    if (it == reverseConnections_.end()) {
        return;
    }

    std::vector<uint32_t>& vec = it->second;
    vec.erase(std::remove(vec.begin(), vec.end(), srcNodeId), vec.end());
    if (vec.empty()) {
        reverseConnections_.erase(destNodeId);
    }
}

int32_t AudioSuitePipeline::InstallTap(uint32_t nodeId, AudioNodePortType portType,
    std::shared_ptr<SuiteNodeReadTapDataCallback> callback)
{
    auto request = [this, nodeId, portType, callback]() {
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("InstallTap failed, node id is invailed.");
            TriggerCallback(INSTALL_NODE_TAP, ERR_INVALID_PARAM);
            return;
        }

        auto node = nodeMap_[nodeId];
        if (node == nullptr) {
            AUDIO_ERR_LOG("InstallTap failed, node ptr is nullptr.");
            TriggerCallback(INSTALL_NODE_TAP, ERR_INVALID_PARAM);
            return;
        }

        int32_t ret = node->InstallTap(portType, callback);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("InstallTap failed, ret = %{public}d, nodeId = %{public}d, portType = %{public}d.",
                ret, nodeId, static_cast<int32_t>(portType));
            TriggerCallback(INSTALL_NODE_TAP, ret);
            return;
        }

        AUDIO_INFO_LOG("InstallTap success.");
        TriggerCallback(INSTALL_NODE_TAP, SUCCESS);
        return;
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::RemoveTap(uint32_t nodeId, AudioNodePortType portType)
{
    auto request = [this, nodeId, portType]() {
        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("RemoveTap failed, node id is invailed.");
            TriggerCallback(REMOVE_NODE_TAP, ERR_INVALID_PARAM);
            return;
        }

        auto node = nodeMap_[nodeId];
        if (node == nullptr) {
            AUDIO_ERR_LOG("RemoveTap failed, node ptr is nullptr.");
            TriggerCallback(REMOVE_NODE_TAP, ERR_INVALID_PARAM);
            return;
        }

        int32_t ret = node->RemoveTap(portType);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("RemoveTap failed, ret = %{public}d, nodeId = %{public}d, portType = %{public}d.",
                ret, nodeId, static_cast<int32_t>(portType));
            TriggerCallback(REMOVE_NODE_TAP, ret);
            return;
        }

        AUDIO_INFO_LOG("RemoveTap success.");
        TriggerCallback(INSTALL_NODE_TAP, SUCCESS);
        return;
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::RenderFrame(uint8_t *audioData, int32_t frameSize, int32_t *writeLen, bool *finishedFlag)
{
    AUDIO_INFO_LOG("AudioSuitePipeline::RenderFrame enter");
    auto request = [this, audioData, frameSize, writeLen, finishedFlag]() {
        AUDIO_INFO_LOG("AudioSuitePipeline::RenderFrame enter request");
        if (pipelineState_ != PIPELINE_RUNNING) {
            AUDIO_ERR_LOG("RenderFrame failed, pipelineState state is not running.");
            TriggerCallback(RENDER_FRAME, ERR_ILLEGAL_STATE);
            return;
        }

        if (outputNode_ == nullptr) {
            AUDIO_ERR_LOG("RenderFrame failed, outputNode_ is nullptr.");
            TriggerCallback(RENDER_FRAME, ERR_ILLEGAL_STATE);
            return;
        }

        int32_t ret = outputNode_->DoProcess(audioData, frameSize, writeLen, finishedFlag);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("RenderFrame, ret = %{public}d.", ret);
            TriggerCallback(RENDER_FRAME, ret);
            return;
        }

        TriggerCallback(RENDER_FRAME, SUCCESS);
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t AudioSuitePipeline::SetOptions(uint32_t nodeId, std::string name, std::string value)
{
    auto request = [this, nodeId, name, value]() {
        if (pipelineState_ != PIPELINE_STOPPED) {
            AUDIO_ERR_LOG("SetOptions failed, pipelineState status is not stopped.");
            return;
        }

        if (nodeMap_.find(nodeId) == nodeMap_.end()) {
            AUDIO_ERR_LOG("SetOptions failed, node id is invailed.");
            return;
        }

        auto node = nodeMap_[nodeId];
        int32_t ret = node->SetOptions(name, value);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("SetOptions, ret = %{public}d.", ret);
            return;
        }
    };

    SendRequest(request, __func__);
    return SUCCESS;
}

bool AudioSuitePipeline::CheckPipelineNode(uint32_t startNodeId)
{
    std::queue<uint32_t> nodeQueue;
    std::unordered_set<uint32_t> visitedNodes;

    nodeQueue.push(startNodeId);
    while (!nodeQueue.empty()) {
        uint32_t currentNodeId = nodeQueue.front();
        nodeQueue.pop();

        if (visitedNodes.find(currentNodeId) != visitedNodes.end()) {
            return false; // ring
        }

        visitedNodes.insert(currentNodeId);

        auto connIter = reverseConnections_.find(currentNodeId);
        if (connIter == reverseConnections_.end()) {
            auto nodeIter = nodeMap_.find(currentNodeId);
            if (nodeIter == nodeMap_.end()) {
                return false;
            }

            if (nodeIter->second == nullptr) {
                return false;
            }

            AudioNodeType currentNodeType = nodeIter->second->GetNodeType();
            if (currentNodeType != NODE_TYPE_INPUT) {
                return false;
            }

            if (!nodeIter->second->IsSetReadDataCallback()) {
                return false;
            }
            continue;
        }

        for (uint32_t nextNodeId : connIter->second) {
            nodeQueue.push(nextNodeId);
        }
    }

    return true;
}

bool AudioSuitePipeline::IsConnected(uint32_t srcNodeId, uint32_t destNodeId)
{
    if (srcNodeId == destNodeId) {
        return true;
    }

    std::unordered_set<uint32_t> visited;
    std::queue<uint32_t> queue;
    queue.push(srcNodeId);
    visited.insert(srcNodeId);

    while (!queue.empty()) {
        uint32_t currentNodeId = queue.front();
        queue.pop();

        if (reverseConnections_.find(currentNodeId) == reverseConnections_.end()) {
            continue;
        }

        for (uint32_t prevNodeId : reverseConnections_[currentNodeId]) {
            if (prevNodeId == destNodeId) {
                return true;
            }

            if (visited.find(prevNodeId) == visited.end()) {
                visited.insert(prevNodeId);
                queue.push(prevNodeId);
            }
        }
    }

    return false;
}

bool AudioSuitePipeline::IsDirectConnected(uint32_t srcNodeId, uint32_t destNodeId)
{
    if (connections_.find(srcNodeId) == connections_.end()) {
        return false;
    }

    return connections_[srcNodeId] == destNodeId;
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
