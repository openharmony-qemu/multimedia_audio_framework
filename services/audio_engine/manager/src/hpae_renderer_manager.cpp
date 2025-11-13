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
#include "audio_stream_info.h"
#include "audio_errors.h"
#include "hpae_node_common.h"
#include "audio_effect_chain_manager.h"
#include "audio_utils.h"
#include "audio_volume.h"
#include "audio_engine_log.h"
#include "hpae_output_cluster.h"
#include "hpae_remote_output_cluster.h"
#include "hpae_message_queue_monitor.h"
#include "hpae_stream_move_monitor.h"

constexpr int32_t DEFAULT_EFFECT_RATE = 48000;
constexpr int32_t DEFAULT_EFFECT_FRAME_LEN = 960;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
    constexpr float SUSPEND_TIME_OUT_S = 3.5; // prevent stop not success
    constexpr int64_t AUDIO_NS_PER_US = 1000;
    constexpr int64_t BUFFER_DURATION_US = 10 * 1000; // 10ms
    constexpr int64_t UNDERRUN_BYPASS_DURATION_NS = 60 * 1000 * 1000; // 60ms
    const std::string REMOTE_DEVICE_CLASS = "remote";
}
HpaeRendererManager::HpaeRendererManager(HpaeSinkInfo &sinkInfo)
    : hpaeNoLockQueue_(CURRENT_REQUEST_COUNT), sinkInfo_(sinkInfo)
{}

HpaeRendererManager::~HpaeRendererManager()
{
    AUDIO_INFO_LOG("destructor renderer");
    if (isInit_.load()) {
        DeInit();
    }
}

bool HpaeRendererManager::IsMchDevice()
{
    return sinkInfo_.deviceName == "MCH_Speaker" || sinkInfo_.deviceName == "DP_MCH_speaker";
}

bool HpaeRendererManager::IsRemoteDevice()
{
    return sinkInfo_.deviceClass == REMOTE_DEVICE_CLASS;
}

int32_t HpaeRendererManager::CreateInputSession(const HpaeStreamInfo &streamInfo)
{
    Trace trace("[" + std::to_string(streamInfo.sessionId) + "]HpaeRendererManager::CreateInputSession");
    HpaeNodeInfo nodeInfo;
    ConfigNodeInfo(nodeInfo, streamInfo);
    nodeInfo.sceneType = TransToProperSceneType(streamInfo.effectInfo.streamUsage, streamInfo.effectInfo.effectScene);
    TransNodeInfoForCollaboration(nodeInfo, isCollaborationEnabled_);
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    sinkInputNodeMap_[streamInfo.sessionId] = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNodeMap_[streamInfo.sessionId]->SetAppUid(streamInfo.uid);
    AUDIO_INFO_LOG("streamType %{public}u, sessionId = %{public}u, current sceneType is %{public}d",
        nodeInfo.streamType,
        nodeInfo.sessionId,
        nodeInfo.sceneType);
    CreateProcessCluster(nodeInfo);
    return SUCCESS;
}

int32_t HpaeRendererManager::AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    auto request = [this, node]() { AddSingleNodeToSink(node); };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeRendererManager::AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect)
{
    Trace trace("HpaeRendererManager::AddSingleNodeToSink");
    HpaeNodeInfo nodeInfo = node->GetNodeInfo();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    // no need history buffer in not offload sink
    nodeInfo.historyFrameCount = 0;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.sceneType = TransToProperSceneType(nodeInfo.effectInfo.streamUsage, nodeInfo.effectInfo.effectScene);
    // for collaboration
    TransNodeInfoForCollaboration(nodeInfo, isCollaborationEnabled_);
    node->SetNodeInfo(nodeInfo);
    uint32_t sessionId = nodeInfo.sessionId;
    
    sinkInputNodeMap_[sessionId] = node;
    SetSessionState(sessionId, node->GetState());
    sessionNodeMap_[sessionId].sceneType = nodeInfo.sceneType;
    UpdateClusterStreamInfo(nodeInfo.sceneType);
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(true, nodeInfo);
#endif

    HILOG_COMM_INFO("[FinishMove] session :%{public}u to sink:%{public}s, sceneType is %{public}d",
        sessionId, sinkInfo_.deviceClass.c_str(), nodeInfo.sceneType);
    CreateProcessClusterAndConnect(nodeInfo, isConnect);
}

void HpaeRendererManager::CreateProcessClusterAndConnect(HpaeNodeInfo &nodeInfo, bool isConnect)
{
    uint32_t sessionId = nodeInfo.sessionId;
    HpaeNodeInfo processNodeInfo = nodeInfo;
    CreateProcessCluster(processNodeInfo);

    CHECK_AND_RETURN_LOG(isConnect == true, "not need connect session:%{public}d", sessionId);
    if (sessionNodeMap_[sessionId].state == HPAE_SESSION_RUNNING) {
        AUDIO_INFO_LOG("session:%{public}u connect to sink:%{public}s",
            sessionId, sinkInfo_.deviceClass.c_str());
        ConnectInputSession(sessionId);
    }
}

void HpaeRendererManager::CreateDefaultProcessCluster(HpaeNodeInfo &nodeInfo)
{
    HILOG_COMM_INFO("use default processCluster");
    if (!SafeGetMap(sceneClusterMap_, HPAE_SCENE_DEFAULT)) {
        AUDIO_INFO_LOG("default processCluster is null, create default processCluster");
        HpaeNodeInfo temp = nodeInfo;
        temp.sceneType = HPAE_SCENE_DEFAULT;
        auto hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(temp, sinkInfo_);
        sceneClusterMap_[HPAE_SCENE_DEFAULT] = hpaeProcessCluster;
        sceneClusterMap_[nodeInfo.sceneType] = hpaeProcessCluster;
        sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT]++;
    } else {
        sceneClusterMap_[nodeInfo.sceneType] = sceneClusterMap_[HPAE_SCENE_DEFAULT];
        sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT]++;
    }
    return;
}

void HpaeRendererManager::CreateProcessClusterInner(HpaeNodeInfo &nodeInfo, int32_t processClusterDecision)
{
    HpaeNodeInfo temp = nodeInfo;
    std::shared_ptr<HpaeProcessCluster> hpaeProcessCluster = nullptr;
    switch (processClusterDecision) {
        case NO_NEED_TO_CREATE_PROCESSCLUSTER:
            HILOG_COMM_INFO("no need to create processCluster");
            CHECK_AND_RETURN(!SafeGetMap(sceneClusterMap_, nodeInfo.sceneType));
            AUDIO_INFO_LOG("processCluster is null, create a new processCluster");
            sceneClusterMap_[nodeInfo.sceneType] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo_);
            break;
        case CREATE_NEW_PROCESSCLUSTER:
            CHECK_AND_RETURN(!SafeGetMap(sceneClusterMap_, nodeInfo.sceneType));
            HILOG_COMM_INFO("create new processCluster");
            sceneClusterMap_[nodeInfo.sceneType] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo_);
            break;
        case CREATE_DEFAULT_PROCESSCLUSTER:
            temp.sceneType = HPAE_SCENE_DEFAULT;
            if (!SafeGetMap(sceneClusterMap_, HPAE_SCENE_DEFAULT)) {
                HILOG_COMM_INFO("begin control, create default processCluster");
                hpaeProcessCluster = std::make_shared<HpaeProcessCluster>(temp, sinkInfo_);
                sceneClusterMap_[HPAE_SCENE_DEFAULT] = hpaeProcessCluster;
                sceneClusterMap_[nodeInfo.sceneType] = hpaeProcessCluster;
            } else {
                sceneClusterMap_[nodeInfo.sceneType] = sceneClusterMap_[HPAE_SCENE_DEFAULT];
            }
            sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT]++;
            break;
        case USE_DEFAULT_PROCESSCLUSTER:
            CreateDefaultProcessCluster(nodeInfo);
            break;
        case USE_NONE_PROCESSCLUSTER:
            HILOG_COMM_INFO("use none processCluster");
            break;
        case CREATE_EXTRA_PROCESSCLUSTER:
            HILOG_COMM_INFO("out of control");
            CHECK_AND_RETURN(!SafeGetMap(sceneClusterMap_, nodeInfo.sceneType));
            AUDIO_INFO_LOG("out of control, create a new processCluster");
            sceneClusterMap_[nodeInfo.sceneType] = std::make_shared<HpaeProcessCluster>(nodeInfo, sinkInfo_);
            break;
        default:
            break;
    }
    sessionNodeMap_[nodeInfo.sessionId].bypass = (processClusterDecision == USE_NONE_PROCESSCLUSTER) ? true : false;
    return;
}

void HpaeRendererManager::CreateProcessCluster(HpaeNodeInfo &nodeInfo)
{
    Trace trace("HpaeRendererManager::CreateProcessCluster");
    std::string sceneType = TransProcessorTypeToSceneType(nodeInfo.sceneType);
    int32_t processClusterDecision = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    CreateProcessClusterInner(nodeInfo, processClusterDecision);

    HpaeProcessorType sceneTypeConnect = GetProcessorType(nodeInfo.sessionId);
    CHECK_AND_RETURN_LOG(SafeGetMap(sceneClusterMap_, sceneTypeConnect),
        "could not find processorType %{public}d", sceneTypeConnect);
    if (!sessionNodeMap_[nodeInfo.sessionId].bypass) {
        sceneTypeToProcessClusterCountMap_[nodeInfo.sceneType]++;
    }
    sceneClusterMap_[sceneTypeConnect]->CreateNodes(sinkInputNodeMap_[nodeInfo.sessionId]);
    AUDIO_INFO_LOG("SessionId %{public}d, Create nodes in sceneCluster processorType %{public}d",
        nodeInfo.sessionId, sceneTypeConnect);
    int32_t ret = sceneClusterMap_[sceneTypeConnect]->AudioRendererCreate(nodeInfo, sinkInfo_);
    if (ret != SUCCESS) {
        AUDIO_WARNING_LOG("update audio effect when creating failed, ret = %{public}d", ret);
    }
}

int32_t HpaeRendererManager::AddAllNodesToSink(
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

HpaeProcessorType HpaeRendererManager::TransToProperSceneType(StreamUsage streamUsage, AudioEffectScene effectScene)
{
    if (sinkInfo_.lib == "libmodule-split-stream-sink.z.so") {
        return TransStreamUsageToSplitSceneType(streamUsage, sinkInfo_.splitMode);
    } else if (IsRemoteDevice() || IsMchDevice()) {
        return HPAE_SCENE_EFFECT_NONE;
    } else {
        return TransEffectSceneToSceneType(effectScene);
    }
}

HpaeProcessorType HpaeRendererManager::GetProcessorType(uint32_t sessionId)
{
    HpaeNodeInfo nodeInfo = sinkInputNodeMap_[sessionId]->GetNodeInfo();
    std::string sceneType = TransProcessorTypeToSceneType(nodeInfo.sceneType);
    if ((!isSplitProcessorType(nodeInfo.sceneType)) && (sessionNodeMap_[sessionId].bypass ||
        !AudioEffectChainManager::GetInstance()->ExistAudioEffectChainArm(sceneType, nodeInfo.effectInfo.effectMode))) {
        return HPAE_SCENE_EFFECT_NONE;
    }
    return nodeInfo.sceneType;
}

void HpaeRendererManager::RefreshProcessClusterByDeviceInner(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    CHECK_AND_RETURN_LOG(node != nullptr, "sinkInputNode is nullptr");
    HpaeNodeInfo nodeInfo = node->GetNodeInfo();
    std::string sceneType = TransProcessorTypeToSceneType(nodeInfo.sceneType);
    int32_t processClusterDecision = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    if ((processClusterDecision != USE_NONE_PROCESSCLUSTER && sessionNodeMap_[nodeInfo.sessionId].bypass) ||
        (processClusterDecision == USE_NONE_PROCESSCLUSTER && !sessionNodeMap_[nodeInfo.sessionId].bypass)) {
        if (node->connectedProcessorType_ == HPAE_SCENE_EFFECT_NONE &&
            processClusterDecision == USE_NONE_PROCESSCLUSTER) {
            AUDIO_INFO_LOG("no need to refresh");
            CHECK_AND_RETURN_LOG(SafeGetMap(sceneClusterMap_, nodeInfo.sceneType),
                "could not find processorType %{public}d", nodeInfo.sceneType);
            sceneClusterMap_[nodeInfo.sceneType]->AudioRendererRelease(nodeInfo, sinkInfo_);
            sceneTypeToProcessClusterCountMap_[nodeInfo.sceneType]--;
            AUDIO_INFO_LOG("disconnected from sceneType %{public}d, current count is %{public}d",
                nodeInfo.sceneType, sceneTypeToProcessClusterCountMap_[nodeInfo.sceneType]);
            if (sceneClusterMap_[nodeInfo.sceneType] == sceneClusterMap_[HPAE_SCENE_DEFAULT]) {
                sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT]--;
                AUDIO_INFO_LOG("sceneType default count is %{public}d",
                    sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT]);
            }
            DeleteProcessClusterInner(nodeInfo.sessionId, nodeInfo.sceneType);
            CreateProcessCluster(nodeInfo);
        } else {
            AUDIO_INFO_LOG("refresh to %{public}d", processClusterDecision);
            TriggerStreamState(nodeInfo.sessionId, node);
            DeleteProcessCluster(nodeInfo.sessionId);
            CreateProcessClusterAndConnect(nodeInfo);
        }
    }
}

int32_t HpaeRendererManager::RefreshProcessClusterByDevice()
{
    auto request = [this]() {
        for (const auto &it : sinkInputNodeMap_) {
            RefreshProcessClusterByDeviceInner(it.second);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t checkRet = CheckStreamInfo(streamInfo);
    if (checkRet != SUCCESS) {
        return checkRet;
    }
    auto request = [this, streamInfo]() {
        Trace trace("HpaeRendererManager::CreateStream id[" +
            std::to_string(streamInfo.sessionId) + "]");
        AUDIO_INFO_LOG("CreateStream sessionId %{public}u deviceName %{public}s",
            streamInfo.sessionId,
            sinkInfo_.deviceName.c_str());
        CreateInputSession(streamInfo);
        SetSessionState(streamInfo.sessionId, HPAE_SESSION_PREPARED);
        sessionNodeMap_[streamInfo.sessionId].isMoveAble = streamInfo.isMoveAble;
        sinkInputNodeMap_[streamInfo.sessionId]->SetState(HPAE_SESSION_PREPARED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        Trace trace("HpaeRendererManager::DestroyStream id[" +
            std::to_string(sessionId) + "]");
        AUDIO_INFO_LOG("DestroyStream sessionId %{public}u", sessionId);
        HpaeProcessorType sceneType = GetProcessorType(sessionId);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Release not find sessionId %{public}u", sessionId);
        SetSessionState(sessionId, HPAE_SESSION_RELEASED);
        sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_RELEASED);
        DeleteInputSession(sessionId);
        isNeedInitEffectBufferFlagMap_.erase(sessionId);
        UpdateClusterStreamInfo(sceneType);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::DeleteInputSession(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::DeleteInputSession");
    DeleteProcessCluster(sessionId);
#ifdef ENABLE_HIDUMP_DFX
    if (auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId)) {
        OnNotifyDfxNodeAdmin(false, sinkInputNode->GetNodeInfo());
    }
#endif
    sinkInputNodeMap_.erase(sessionId);
    sessionNodeMap_.erase(sessionId);
    return SUCCESS;
}

int32_t HpaeRendererManager::DeleteProcessClusterInner(uint32_t sessionId, HpaeProcessorType sceneType)
{
    if (sessionNodeMap_[sessionId].bypass) {
        AUDIO_INFO_LOG("none processCluster no need to delete processCluster");
        return ERROR;
    }
    if (sceneTypeToProcessClusterCountMap_.count(sceneType) && sceneTypeToProcessClusterCountMap_[sceneType] == 0) {
        if (sceneClusterMap_[sceneType] == sceneClusterMap_[HPAE_SCENE_DEFAULT] || IsClusterDisConnected(sceneType)) {
            sceneClusterMap_.erase(sceneType);
            sceneTypeToProcessClusterCountMap_.erase(sceneType);
            AUDIO_INFO_LOG("sessionId %{public}u, processCluster %{public}d has been erased", sessionId, sceneType);
        }
    }

    if (sceneTypeToProcessClusterCountMap_.count(HPAE_SCENE_DEFAULT) &&
        sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT] == 0) {
        if (IsClusterDisConnected(HPAE_SCENE_DEFAULT)) {
            sceneClusterMap_.erase(HPAE_SCENE_DEFAULT);
            sceneTypeToProcessClusterCountMap_.erase(HPAE_SCENE_DEFAULT);
            AUDIO_INFO_LOG("processCluster default has been erased");
        }
    }
    return SUCCESS;
}

int32_t HpaeRendererManager::DeleteProcessCluster(uint32_t sessionId)
{
    if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
        AUDIO_INFO_LOG("could not find session:%{public}d", sessionId);
        return SUCCESS;
    }
    HpaeNodeInfo nodeInfo = sinkInputNodeMap_[sessionId]->GetNodeInfo();
    HpaeProcessorType sceneType = sinkInputNodeMap_[sessionId]->connectedProcessorType_;
    DereferenceInputCluster(sessionId);
    DisConnectOutputCluster(sceneType);
    
    HpaeProcessorType sceneTypeToDestroyNodes = GetProcessorType(sessionId);
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sceneClusterMap_, sceneTypeToDestroyNodes), ERROR,
        "SessionId %{public}u, sceneCluster processorType %{public}d not exist, cant destroy nodes",
        sessionId, sceneTypeToDestroyNodes);
    if (sceneClusterMap_[sceneTypeToDestroyNodes]->DestroyNodes(sessionId) != SUCCESS) {
        AUDIO_INFO_LOG("SessionId %{public}d, Nodes not found in sceneCluster processorType %{public}d,"
            "try to destroy nodes by traverse sceneCluster", sessionId, sceneTypeToDestroyNodes);
        DeleteNodesByTraversal(sessionId);
    }
    DeleteProcessClusterInner(sessionId, nodeInfo.sceneType);
    return SUCCESS;
}

bool HpaeRendererManager::isSplitProcessorType(HpaeProcessorType sceneType)
{
    if (sceneType == HPAE_SCENE_SPLIT_MEDIA || sceneType == HPAE_SCENE_SPLIT_NAVIGATION ||
        sceneType == HPAE_SCENE_SPLIT_COMMUNICATION) {
        return true;
    }
    return false;
}

int32_t HpaeRendererManager::ConnectInputSession(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::ConnectInputSession");
    AUDIO_INFO_LOG("connect input session:%{public}d", sessionId);
    if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
        AUDIO_ERR_LOG("could not input node by sessionid:%{public}d", sessionId);
        return ERR_INVALID_PARAM;
    }
    if (sinkInputNodeMap_[sessionId]->GetState() != HPAE_SESSION_RUNNING) {
        return SUCCESS;
    }
    HpaeProcessorType sceneType = GetProcessorType(sessionId);
    if (SafeGetMap(sceneClusterMap_, sceneType)) {
        ConnectProcessCluster(sessionId, sceneType);
    }
    if (outputCluster_->GetState() != STREAM_MANAGER_RUNNING && !isSuspend_) {
        noneStreamTime_ = 0;
        outputCluster_->Start();
    }
    return SUCCESS;
}

int32_t HpaeRendererManager::UpdateClusterStreamInfo(HpaeProcessorType sceneType)
{
    uint32_t minSessionId = UINT32_MAX;
    std::shared_ptr<HpaeSinkInputNode> updateNode = nullptr;
    for (const auto &[mapSessionId, mapNodePtr] : sinkInputNodeMap_) {
        CHECK_AND_CONTINUE(mapNodePtr->connectedProcessorType_ == sceneType);
        CHECK_AND_CONTINUE(mapNodePtr->GetState() == HPAE_SESSION_RUNNING);
        if (mapSessionId < minSessionId) {
            updateNode = mapNodePtr;
            minSessionId = mapSessionId;
        }
    }
    if (updateNode == nullptr) {
        AUDIO_INFO_LOG("no need to update the stream info");
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(outputCluster_ != nullptr, ERR_NULL_POINTER, "outputCluster_ is nullptr");
    outputCluster_->UpdateStreamInfo(updateNode);
    return SUCCESS;
}

void HpaeRendererManager::ConnectProcessCluster(uint32_t sessionId, HpaeProcessorType sceneType)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::ConnectProcessCluster sceneType:"
        + std::to_string(sceneType));
    HpaeProcessorType tmpSceneType = (sceneClusterMap_[sceneType] == SafeGetMap(sceneClusterMap_, HPAE_SCENE_DEFAULT))
        ? HPAE_SCENE_DEFAULT : sceneType;
    if (toBeStoppedSceneTypeToSessionMap_.count(tmpSceneType) > 0) {
        uint32_t sessionIdToStop = toBeStoppedSceneTypeToSessionMap_[tmpSceneType];
        if (sessionIdToStop == sessionId) {
            toBeStoppedSceneTypeToSessionMap_.erase(tmpSceneType);
        } else {
            if (SafeGetMap(sinkInputNodeMap_, sessionIdToStop)) {
                sceneClusterMap_[sceneType]->AudioRendererStop(sinkInputNodeMap_[sessionIdToStop]->GetNodeInfo(),
                    sinkInfo_);
            }
            toBeStoppedSceneTypeToSessionMap_.erase(tmpSceneType);
            int32_t ret = sceneClusterMap_[sceneType]->AudioRendererStart(sinkInputNodeMap_[sessionId]->GetNodeInfo(),
                sinkInfo_);
            if (ret != SUCCESS) {
                AUDIO_WARNING_LOG("update audio effect when starting failed, ret = %{public}d", ret);
            }
        }
    } else {
        int32_t ret = sceneClusterMap_[sceneType]->AudioRendererStart(sinkInputNodeMap_[sessionId]->GetNodeInfo(),
            sinkInfo_);
        if (ret != SUCCESS) {
            AUDIO_WARNING_LOG("update audio effect when starting failed, ret = %{public}d", ret);
        }
    }
    std::shared_ptr<HpaeSinkInputNode> sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_LOG(sinkInputNode != nullptr, "sinkInputNode is nullptr");
    ConnectOutputCluster(sessionId, sceneType);
    ConnectInputCluster(sessionId, sceneType);
    sceneClusterMap_[sceneType]->SetLoudnessGain(sessionId, sinkInputNode->GetLoudnessGain());
}

void HpaeRendererManager::ConnectInputCluster(uint32_t sessionId, HpaeProcessorType sceneType)
{
    CHECK_AND_RETURN_LOG(SafeGetMap(sceneClusterMap_, sceneType),
        "SessionId %{public}u, sceneCluster processorType %{public}d not exist, cant connect nodes",
        sessionId, sceneType);
    if (sceneClusterMap_[sceneType]->CheckNodes(sessionId) != SUCCESS) {
        AUDIO_INFO_LOG("SessionId %{public}d, Nodes not found in sceneCluster processorType %{public}d, "
            "try to destroy nodes by traverse sceneCluster and recreate nodes", sessionId, sceneType);
        DeleteNodesByTraversal(sessionId);
        sceneClusterMap_[sceneType]->CreateNodes(sinkInputNodeMap_[sessionId]);
        AUDIO_INFO_LOG("SessionId %{public}d, recreate nodes in sceneCluster processorType %{public}d",
            sessionId, sceneType);
    }
    sceneClusterMap_[sceneType]->Connect(sinkInputNodeMap_[sessionId]);
    sinkInputNodeMap_[sessionId]->connectedProcessorType_ =
        (sceneClusterMap_[sceneType] == SafeGetMap(sceneClusterMap_, HPAE_SCENE_DEFAULT)) ?
        HPAE_SCENE_DEFAULT : sceneType;
}

void HpaeRendererManager::ConnectOutputCluster(uint32_t sessionId, HpaeProcessorType sceneType)
{
    if (!outputCluster_->IsProcessClusterConnected(sceneType) && !sceneClusterMap_[sceneType]->GetConnectedFlag()) {
        outputCluster_->Connect(sceneClusterMap_[sceneType]);
        sceneClusterMap_[sceneType]->SetConnectedFlag(true);
    } else {
        outputCluster_->UpdateStreamInfo(sceneClusterMap_[sceneType]);
    }
    if (sceneType == HPAE_SCENE_COLLABORATIVE && hpaeCoBufferNode_ != nullptr) {
        uint32_t latency = outputCluster_->GetHdiLatency();
        hpaeCoBufferNode_->SetLatency(latency);
        hpaeCoBufferNode_->Connect(sceneClusterMap_[sceneType]);
        TriggerCallback(CONNECT_CO_BUFFER_NODE, hpaeCoBufferNode_);
    }
}

void HpaeRendererManager::MoveAllStreamToNewSink(const std::string &sinkName,
    const std::vector<uint32_t>& moveIds, MoveSessionType moveType)
{
    Trace trace("HpaeRendererManager::MoveAllStreamToNewSink[" + sinkName + "]");
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
        CHECK_AND_CONTINUE_LOG(SafeGetMap(sinkInputNodeMap_, it),
            "sessionid: %{public}u can not found in sinkInputNodeMap", it);
        TriggerStreamState(it, sinkInputNodeMap_[it]);
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

int32_t HpaeRendererManager::MoveAllStream(const std::string &sinkName, const std::vector<uint32_t>& sessionIds,
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

void HpaeRendererManager::MoveStreamSync(uint32_t sessionId, const std::string &sinkName)
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

    HpaeProcessorType sceneType = GetProcessorType(sessionId);
    AUDIO_INFO_LOG("[StartMove] session: %{public}u,sink [%{public}s] --> [%{public}s]",
        sessionId, sinkInfo_.deviceName.c_str(), sinkName.c_str());
    std::shared_ptr<HpaeSinkInputNode> inputNode = sinkInputNodeMap_[sessionId];
    TriggerStreamState(sessionId, inputNode);
    DeleteInputSession(sessionId);
    UpdateClusterStreamInfo(sceneType);
    std::string name = sinkName;
    TriggerCallback(MOVE_SINK_INPUT, inputNode, name);
}

int32_t HpaeRendererManager::MoveStream(uint32_t sessionId, const std::string &sinkName)
{
    if (!IsInit()) {
        MoveStreamSync(sessionId, sinkName);
    } else {
        auto request = [this, sessionId, sinkName]() { MoveStreamSync(sessionId, sinkName); };
        SendRequest(request, __func__);
    }
    return SUCCESS;
}

int32_t HpaeRendererManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::Start");
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Start not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Start sessionId %{public}u, deviceName %{public}s", sessionId, sinkInfo_.deviceName.c_str());
        HpaeProcessorType sceneType = GetProcessorType(sessionId);
        sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_RUNNING);
        HandlePriPaPower(sessionId);
        ConnectInputSession(sessionId);
        SetSessionState(sessionId, HPAE_SESSION_RUNNING);
        SetSessionFade(sessionId, OPERATION_STARTED);
        UpdateClusterStreamInfo(sceneType);
        isNeedInitEffectBufferFlagMap_[sessionId] = true;
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::StartWithSyncId(uint32_t sessionId, int32_t syncId)
{
    auto request = [this, sessionId, syncId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::StartWithSyncId");
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "StartWithSyncId not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("StartWithSyncId sessionId %{public}u, deviceName %{public}s",
            sessionId, sinkInfo_.deviceName.c_str());
        HpaeProcessorType sceneType = GetProcessorType(sessionId);
        sinkInputNodeMap_[sessionId]->SetState(HPAE_SESSION_RUNNING);
        HandlePriPaPower(sessionId);
        ConnectInputSession(sessionId);
        SetSessionState(sessionId, HPAE_SESSION_RUNNING);
        SetSessionFade(sessionId, OPERATION_STARTED);
        UpdateClusterStreamInfo(sceneType);
        if (syncId >= 0) {
            HandleSyncId(sessionId, syncId);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::DisConnectInputSession(uint32_t sessionId)
{
    if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
        AUDIO_INFO_LOG("DisConnectInputSession sessionId %{public}u", sessionId);
        return SUCCESS;
    }
    HpaeProcessorType sceneType = sinkInputNodeMap_[sessionId]->connectedProcessorType_;
    if (SafeGetMap(sceneClusterMap_, sceneType)) {
        DisConnectInputCluster(sessionId, sceneType);
    }
    return SUCCESS;
}

void HpaeRendererManager::OnDisConnectProcessCluster(HpaeProcessorType sceneType)
{
    auto request = [this, sceneType]() {
        AUDIO_INFO_LOG("mixerNode trigger callback, sceneType %{public}d", sceneType);
        if (SafeGetMap(sceneClusterMap_, sceneType) && sceneClusterMap_[sceneType]->GetPreOutNum() == 0) {
            DisConnectOutputCluster(sceneType);
            // for collaboration
            if (sceneType == HPAE_SCENE_COLLABORATIVE && hpaeCoBufferNode_ != nullptr) {
                hpaeCoBufferNode_->DisConnect(sceneClusterMap_[sceneType]);
                TriggerCallback(DISCONNECT_CO_BUFFER_NODE, hpaeCoBufferNode_);
            }
            if (toBeStoppedSceneTypeToSessionMap_.count(sceneType) &&
                SafeGetMap(sinkInputNodeMap_, toBeStoppedSceneTypeToSessionMap_[sceneType])) {
                sceneClusterMap_[sceneType]->
                    AudioRendererStop(sinkInputNodeMap_[toBeStoppedSceneTypeToSessionMap_[sceneType]]->GetNodeInfo(),
                        sinkInfo_);
            }
            toBeStoppedSceneTypeToSessionMap_.erase(sceneType);
        }
    };
    SendRequest(request, __func__);
}

void HpaeRendererManager::DisConnectInputCluster(uint32_t sessionId, HpaeProcessorType sceneType)
{
    sceneClusterMap_[sceneType]->DisConnect(sinkInputNodeMap_[sessionId]);
    sinkInputNodeMap_[sessionId]->connectedProcessorType_ = HPAE_SCENE_UNCONNECTED;

    if (sessionNodeMap_[sessionId].bypass) {
        CHECK_AND_RETURN_LOG(SafeGetMap(sceneClusterMap_, HPAE_SCENE_EFFECT_NONE),
            "could not find processorType HPAE_SCENE_EFFECT_NONE");
        AUDIO_INFO_LOG("none processCluster need send message to effectNode");
        sceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->AudioRendererStop(sinkInputNodeMap_[sessionId]->GetNodeInfo(),
            sinkInfo_);
        return;
    }

    if (sceneClusterMap_[sceneType]->GetPreOutNum() > 0) {
        sceneClusterMap_[sceneType]->AudioRendererStop(sinkInputNodeMap_[sessionId]->GetNodeInfo(), sinkInfo_);
    } else {
        HpaeProcessorType tmpSceneType = (sceneClusterMap_[sceneType] ==
            SafeGetMap(sceneClusterMap_, HPAE_SCENE_DEFAULT)) ? HPAE_SCENE_DEFAULT : sceneType;
        CHECK_AND_RETURN_LOG(toBeStoppedSceneTypeToSessionMap_.count(sceneType) == 0,
            "sessionId %{public}d to stop already existed", sessionId);
        toBeStoppedSceneTypeToSessionMap_[tmpSceneType] = sessionId;
        AUDIO_INFO_LOG("sessionId %{public}u will be stop", sessionId);
    }
}

void HpaeRendererManager::DisConnectOutputCluster(HpaeProcessorType sceneType)
{
    if (SafeGetMap(sceneClusterMap_, sceneType) && sceneClusterMap_[sceneType]->GetPreOutNum() == 0 &&
        sceneClusterMap_[sceneType]->GetConnectedFlag()) {
        sceneClusterMap_[sceneType]->DisConnectMixerNode();
        if (outputCluster_ != nullptr) {
            outputCluster_->DisConnect(sceneClusterMap_[sceneType]);
        }
        sceneClusterMap_[sceneType]->SetConnectedFlag(false);
    }
}

void HpaeRendererManager::DereferenceInputCluster(uint32_t sessionId)
{
    HpaeProcessorType sceneType = sinkInputNodeMap_[sessionId]->connectedProcessorType_;
    if (SafeGetMap(sceneClusterMap_, sceneType)) {
        sceneClusterMap_[sceneType]->DisConnect(sinkInputNodeMap_[sessionId]);
        sinkInputNodeMap_[sessionId]->connectedProcessorType_ = HPAE_SCENE_UNCONNECTED;
    }

    if (sessionNodeMap_[sessionId].bypass) {
        CHECK_AND_RETURN_LOG(SafeGetMap(sceneClusterMap_, HPAE_SCENE_EFFECT_NONE),
            "could not find processorType HPAE_SCENE_EFFECT_NONE");
        AUDIO_INFO_LOG("none processCluster need send message to effectNode");
        sceneClusterMap_[HPAE_SCENE_EFFECT_NONE]->AudioRendererRelease(sinkInputNodeMap_[sessionId]->GetNodeInfo(),
            sinkInfo_);
        return;
    }
    HpaeNodeInfo nodeInfo = sinkInputNodeMap_[sessionId]->GetNodeInfo();
    CHECK_AND_RETURN_LOG(SafeGetMap(sceneClusterMap_, nodeInfo.sceneType),
        "could not find processorType %{public}d", nodeInfo.sceneType);
    sceneClusterMap_[nodeInfo.sceneType]->AudioRendererRelease(nodeInfo, sinkInfo_);
    sceneTypeToProcessClusterCountMap_[nodeInfo.sceneType]--;
    AUDIO_INFO_LOG("sessionId %{public}u is disconnected from sceneType %{public}d, current count is %{public}d",
        sessionId, nodeInfo.sceneType, sceneTypeToProcessClusterCountMap_[nodeInfo.sceneType]);
    if (sceneClusterMap_[nodeInfo.sceneType] == sceneClusterMap_[HPAE_SCENE_DEFAULT]) {
        sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT]--;
        AUDIO_INFO_LOG("sceneType default count is %{public}d", sceneTypeToProcessClusterCountMap_[HPAE_SCENE_DEFAULT]);
    }
}

void HpaeRendererManager::SetSessionState(uint32_t sessionId, HpaeSessionState renderState)
{
    sessionNodeMap_[sessionId].state = renderState;
}

int32_t HpaeRendererManager::Pause(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::Pause");
        AUDIO_INFO_LOG("Pause sessionId %{public}u deviceName %{public}s", sessionId, sinkInfo_.deviceName.c_str());
        HpaeProcessorType sceneType = GetProcessorType(sessionId);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Pause not find sessionId %{public}u", sessionId);
        if (!SetSessionFade(sessionId, OPERATION_PAUSED)) {
            DisConnectInputSession(sessionId);
        }
        UpdateClusterStreamInfo(sceneType);
        isNeedInitEffectBufferFlagMap_[sessionId] = false;
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::Flush(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::Flush");
        AUDIO_INFO_LOG("Flush sessionId %{public}u deviceName %{public}s", sessionId, sinkInfo_.deviceName.c_str());
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Flush not find sessionId %{public}u", sessionId);
        // flush history buffer
        sinkInputNodeMap_[sessionId]->Flush();
        HpaeProcessorType sceneType = (sinkInputNodeMap_[sessionId]->connectedProcessorType_ != HPAE_SCENE_UNCONNECTED)
            ? sinkInputNodeMap_[sessionId]->connectedProcessorType_ : GetProcessorType(sessionId);
        CHECK_AND_RETURN_LOG(SafeGetMap(sceneClusterMap_, sceneType),
            "Flush not find sceneType: %{public}d in sceneClusterMap", static_cast<int32_t>(sceneType));
        if (isNeedInitEffectBufferFlagMap_[sessionId] == true) {
            sceneClusterMap_[sceneType]->InitEffectBuffer(sessionId);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::Drain(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::Drain");
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

int32_t HpaeRendererManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::Stop");
        AUDIO_INFO_LOG("Stop sessionId %{public}u deviceName %{public}s ", sessionId, sinkInfo_.deviceName.c_str());
        HpaeProcessorType sceneType = GetProcessorType(sessionId);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Stop not find sessionId %{public}u", sessionId);
        if (!SetSessionFade(sessionId, OPERATION_STOPPED)) {
            DisConnectInputSession(sessionId);
        }
        UpdateClusterStreamInfo(sceneType);
        isNeedInitEffectBufferFlagMap_[sessionId] = false;
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::Release(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::Release");
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), ERROR,
        "Release not find sessionId %{public}u", sessionId);
    return DestroyStream(sessionId);
}

bool HpaeRendererManager::CheckIsStreamRunning()
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

int32_t HpaeRendererManager::SuspendStreamManager(bool isSuspend)
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
            if (outputCluster_ != nullptr) {
                outputCluster_->Stop();
            }
        } else if (outputCluster_ != nullptr && outputCluster_->GetState() != STREAM_MANAGER_RUNNING &&
            CheckIsStreamRunning()) {
            outputCluster_->Start();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::SetMute(bool isMute)
{
    // to do check pulseaudio
    auto request = [this, isMute]() {
        if (isMute_ != isMute) {
            isMute_ = isMute;  // todo: fadein and fadeout and mute feature
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeRendererManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

int32_t HpaeRendererManager::ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload)
{
    if (IsInit()) {
        AUDIO_INFO_LOG("deinit:%{public}s renderer first.", sinkInfo.deviceName.c_str());
        DeInit();
    }
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, sinkInfo, isReload]() {
        AUDIO_INFO_LOG("ReloadRenderManager deviceName %{public}s", sinkInfo.deviceName.c_str());
        
        for (const auto &it : sinkInputNodeMap_) {
            TriggerStreamState(it.first, it.second);
            DeleteProcessCluster(it.first);
        }
        AUDIO_INFO_LOG("delete device:%{public}s all input processor end", sinkInfo.deviceName.c_str());
        sinkInfo_ = sinkInfo;
        InitManager(isReload);
        AUDIO_INFO_LOG("init device:%{public}s manager end", sinkInfo.deviceName.c_str());
        for (const auto &it : sinkInputNodeMap_) {
            HpaeNodeInfo nodeInfo = it.second->GetNodeInfo();
            CreateProcessClusterAndConnect(nodeInfo);
        }
        AUDIO_INFO_LOG("connect device:%{public}s all processor end", sinkInfo.deviceName.c_str());
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeRendererManager::InitManager(bool isReload)
{
    AUDIO_INFO_LOG("init devicename:%{public}s", sinkInfo_.deviceName.c_str());
    HpaeNodeInfo nodeInfo;
    int32_t checkRet = CheckFramelen(sinkInfo_);
    if (checkRet != SUCCESS) {
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT,
                        sinkInfo_.deviceName, ERR_INVALID_PARAM);
        return checkRet;
    }
    nodeInfo.channels = sinkInfo_.channels;
    nodeInfo.channelLayout = static_cast<AudioChannelLayout>(sinkInfo_.channelLayout);
    nodeInfo.format = sinkInfo_.format;
    nodeInfo.frameLen = sinkInfo_.frameLen;
    nodeInfo.nodeId = 0;
    nodeInfo.samplingRate = sinkInfo_.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_OUT;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.statusCallback = weak_from_this();
    if (sinkInfo_.lib == "libmodule-split-stream-sink.z.so") {
        outputCluster_ = std::make_unique<HpaeRemoteOutputCluster>(nodeInfo, sinkInfo_);
    } else {
        outputCluster_ = std::make_unique<HpaeOutputCluster>(nodeInfo);
    }
    outputCluster_->SetTimeoutStopThd(sinkInfo_.suspendTime);
    int32_t ret = outputCluster_->GetInstance(sinkInfo_.deviceClass, sinkInfo_.deviceNetId);
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
    attr.aux = sinkInfo_.splitMode.c_str();
    if (!sceneClusterMap_.count(HPAE_SCENE_EFFECT_NONE)) {
        InitDefaultNodeInfo();
    }
    ret = outputCluster_->Init(attr);
    isInit_.store(ret == SUCCESS);
    TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT :INIT_DEVICE_RESULT, sinkInfo_.deviceName, ret);
    return SUCCESS;
}

void HpaeRendererManager::InitDefaultNodeInfo()
{
    HpaeNodeInfo defaultNodeInfo;
    defaultNodeInfo.frameLen = (uint32_t)DEFAULT_EFFECT_FRAME_LEN;
    defaultNodeInfo.samplingRate = (AudioSamplingRate)DEFAULT_EFFECT_RATE;
    defaultNodeInfo.format = AudioSampleFormat::INVALID_WIDTH;
    defaultNodeInfo.channels = STEREO;
    defaultNodeInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    defaultNodeInfo.streamType = STREAM_DEFAULT;
    defaultNodeInfo.sceneType = HPAE_SCENE_EFFECT_NONE;
    defaultNodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    defaultNodeInfo.deviceClass = sinkInfo_.deviceClass;
    defaultNodeInfo.statusCallback = weak_from_this();
    sceneClusterMap_[HPAE_SCENE_EFFECT_NONE] = std::make_shared<HpaeProcessCluster>(defaultNodeInfo, sinkInfo_);
    sceneTypeToProcessClusterCountMap_[HPAE_SCENE_EFFECT_NONE] = 1;
}

void HpaeRendererManager::CreateOutputClusterNodeInfo(HpaeNodeInfo &nodeInfo)
{
    nodeInfo.channels = sinkInfo_.channels;
    nodeInfo.format = sinkInfo_.format;
    nodeInfo.frameLen = sinkInfo_.frameLen;
    nodeInfo.nodeId = 0;
    nodeInfo.samplingRate = sinkInfo_.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_OUT;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.statusCallback = weak_from_this();
    return;
}

int32_t HpaeRendererManager::Init(bool isReload)
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, isReload] {
        Trace trace("HpaeRendererManager::Init");
        InitManager(isReload);
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

bool HpaeRendererManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

int32_t HpaeRendererManager::DeInit(bool isMoveDefault)
{
    Trace trace("HpaeRendererManager::DeInit");
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
    if (outputCluster_ != nullptr) {
        outputCluster_->Stop();
        outputCluster_->DeInit();
    }
    for (const auto &item : sceneClusterMap_) {
        if (item.second) {
            item.second->SetConnectedFlag(false);
        }
    }
    if (outputCluster_ != nullptr) {
        outputCluster_->ResetAll();
        outputCluster_ = nullptr;
    }
    isInit_.store(false);
    return SUCCESS;
}

int32_t HpaeRendererManager::StartRenderSink()
{
    return SUCCESS;
}

int32_t HpaeRendererManager::SetClientVolume(uint32_t sessionId, float volume)
{
    return SUCCESS;
}

int32_t HpaeRendererManager::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    auto request = [this, sessionId, loudnessGain] {
        AUDIO_INFO_LOG("set loudnessGain %{public}f to sessionId %{public}d", loudnessGain, sessionId);
        std::shared_ptr<HpaeSinkInputNode> sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(sinkInputNode != nullptr,
            "session with Id %{public}d not in sinkInputNodeMap_", sessionId);
        sinkInputNode->SetLoudnessGain(loudnessGain);

        HpaeProcessorType sceneType = (sinkInputNodeMap_[sessionId]->connectedProcessorType_ != HPAE_SCENE_UNCONNECTED)
            ? sinkInputNodeMap_[sessionId]->connectedProcessorType_ : GetProcessorType(sessionId);
        std::shared_ptr<HpaeProcessCluster> processCluster = SafeGetMap(sceneClusterMap_, sceneType);
        CHECK_AND_RETURN_LOG(processCluster != nullptr,
            "session with Id %{public}d not in sceneClusterMap_", sessionId);
        processCluster->SetLoudnessGain(sessionId, loudnessGain);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::SetRate(uint32_t sessionId, int32_t rate)
{
    return SUCCESS;
}

int32_t HpaeRendererManager::SetAudioEffectMode(uint32_t sessionId, int32_t effectMode)
{
    if (effectMode < EFFECT_NONE || effectMode > EFFECT_DEFAULT) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId, effectMode]() {
        if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
            AUDIO_WARNING_LOG("miss corresponding sinkInputNode for sessionId %{public}d", sessionId);
            return ;
        }
        HpaeNodeInfo &nodeInfo = sinkInputNodeMap_[sessionId]->GetNodeInfo();
        if (nodeInfo.effectInfo.effectMode != static_cast<AudioEffectMode>(effectMode)) {
            nodeInfo.effectInfo.effectMode = static_cast<AudioEffectMode>(effectMode);
            size_t sinkInputNodeConnectNum = sinkInputNodeMap_[sessionId]->GetOutputPort()->GetInputNum();
            HpaeSessionState inputState = sinkInputNodeMap_[sessionId]->GetState();
            if (sinkInputNodeConnectNum != 0 && inputState == HPAE_SESSION_RUNNING) {
                HILOG_COMM_INFO("UpdateProcessClusterConnection because effectMode to be %{public}d", effectMode);
                UpdateProcessClusterConnection(sessionId, effectMode);
            } else {
                AUDIO_INFO_LOG("no need to ProcessClusterConnection, sinkInputNodeConnectNum is %{public}zu, "
                    "inputState is %{public}d", sinkInputNodeConnectNum, inputState);
            }
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode)
{
    return SUCCESS;
}

int32_t HpaeRendererManager::SetPrivacyType(uint32_t sessionId, int32_t privacyType)
{
    return SUCCESS;
}

int32_t HpaeRendererManager::GetPrivacyType(uint32_t sessionId, int32_t &privacyType)
{
    return SUCCESS;
}

int32_t HpaeRendererManager::RegisterWriteCallback(uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        AUDIO_INFO_LOG("RegisterWriteCallback sessionId %{public}u", sessionId);
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            sinkInputNodeMap_[sessionId]->RegisterWriteCallback(callback);
        }
    };
    hpaeNoLockQueue_.PushRequest(request);
    return SUCCESS;
}

void HpaeRendererManager::Process()
{
    Trace trace("HpaeRendererManager::Process");
    if (outputCluster_ != nullptr && IsRunning()) {
        UpdateAppsUid();
        // no stream running & over 3s need stop
        if (appsUid_.empty()) {
            int64_t now = ClockTime::GetCurNano();
            noneStreamTime_ = noneStreamTime_ == 0 ? now : noneStreamTime_;
            if (now - noneStreamTime_ > SUSPEND_TIME_OUT_S * AUDIO_NS_PER_SECOND) {
                outputCluster_->Stop();
                return;
            }
        } else {
            noneStreamTime_ = 0;
        }
        if (QueryOneStreamUnderrun()) {
            return;
        }
        outputCluster_->DoProcess();
    }
}

void HpaeRendererManager::UpdateAppsUid()
{
    appsUid_.clear();
    for (const auto &sinkInputNodePair : sinkInputNodeMap_) {
        if (sinkInputNodePair.second->GetState() == HPAE_SESSION_RUNNING) {
            appsUid_.emplace_back(sinkInputNodePair.second->GetAppUid());
        }
    }
    outputCluster_->UpdateAppsUid(appsUid_);
}

size_t HpaeRendererManager::GetWritableSize(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeRendererManager::UpdateSpatializationState(
    uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

int32_t HpaeRendererManager::UpdateMaxLength(uint32_t sessionId, uint32_t maxLength)
{
    return SUCCESS;
}

void HpaeRendererManager::SetSpeed(uint32_t sessionId, float speed)
{
    auto request = [this, sessionId, speed]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::SetSpeed");
        AUDIO_INFO_LOG("SetSpeed sessionId %{public}u, deviceName %{public}s, speed %{public}f", sessionId,
            sinkInfo_.deviceName.c_str(), speed);
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), "not find sessionId %{public}u", sessionId);
        sinkInputNodeMap_[sessionId]->SetSpeed(speed);
    };
    SendRequest(request, __func__);
}

std::vector<SinkInput> HpaeRendererManager::GetAllSinkInputsInfo()
{
    return {};
}

int32_t HpaeRendererManager::GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo)
{
    if (!SafeGetMap(sinkInputNodeMap_, sessionId)) {
        return ERR_INVALID_OPERATION;
    }
    sinkInputInfo.nodeInfo = sinkInputNodeMap_[sessionId]->GetNodeInfo();
    sinkInputInfo.rendererSessionInfo = sessionNodeMap_[sessionId];
    return SUCCESS;
}

HpaeSinkInfo HpaeRendererManager::GetSinkInfo()
{
    return sinkInfo_;
}

bool HpaeRendererManager::IsInit()
{
    return isInit_.load();
}

bool HpaeRendererManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

bool HpaeRendererManager::IsRunning(void)
{
    if (outputCluster_ != nullptr && hpaeSignalProcessThread_ != nullptr) {
        return outputCluster_->GetState() == STREAM_MANAGER_RUNNING && hpaeSignalProcessThread_->IsRunning();
    } else {
        return false;
    }
}

void HpaeRendererManager::SendRequest(Request &&request, const std::string &funcName, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_ERR_LOG("HpaeRendererManager not init, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_RENDERER_MANAGER_TYPE, funcName,
            "HpaeRendererManager not init");
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    if (hpaeSignalProcessThread_ == nullptr) {
        AUDIO_ERR_LOG("hpaeSignalProcessThread_ is nullptr, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_RENDERER_MANAGER_TYPE, funcName,
            "thread is nullptr");
        return;
    }
    hpaeSignalProcessThread_->Notify();
}

void HpaeRendererManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, sessionNodeMap_[sessionId].state, operation);
}

void HpaeRendererManager::OnFadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::OnFadeDone");
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Fade done, not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Fade done, call back at RendererManager");
        DisConnectInputSession(sessionId);
        IOperation operation = sinkInputNodeMap_[sessionId]->GetState() == HPAE_SESSION_STOPPING ?
            OPERATION_STOPPED : OPERATION_PAUSED;
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
        SetSessionState(sessionId, state);
        sinkInputNodeMap_[sessionId]->SetState(state);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
    };
    SendRequest(request, __func__);
}

int32_t HpaeRendererManager::RegisterReadCallback(uint32_t sessionId,
    const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    return SUCCESS;
}

void HpaeRendererManager::OnRequestLatency(uint32_t sessionId, uint64_t &latency)
{
    CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), "not find sessionId %{public}u", sessionId);
    HpaeProcessorType sceneType = (sinkInputNodeMap_[sessionId]->connectedProcessorType_ != HPAE_SCENE_UNCONNECTED) ?
        sinkInputNodeMap_[sessionId]->connectedProcessorType_ : GetProcessorType(sessionId);

    uint64_t processLatency = 0;

    if (SafeGetMap(sceneClusterMap_, sceneType)) {
        processLatency += sceneClusterMap_[sceneType]->GetLatency(sessionId);
        if (outputCluster_) {
            processLatency += outputCluster_->GetLatency(sceneType);
        }
    }

    latency += processLatency;
    return;
}

void HpaeRendererManager::OnNotifyQueue()
{
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ is nullptr");
    hpaeSignalProcessThread_->Notify();
}

void HpaeRendererManager::UpdateProcessClusterConnection(uint32_t sessionId, int32_t effectMode)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::UpdateProcessClusterConnection" +
        "effectMode[" + std::to_string(effectMode) + "]");
    HpaeProcessorType sceneType = sinkInputNodeMap_[sessionId]->GetSceneType();
    if (!SafeGetMap(sceneClusterMap_, sceneType)) {
        AUDIO_WARNING_LOG("miss corresponding process cluster for scene type %{public}d", sceneType);
        return;
    }
    if (isSplitProcessorType(sceneType) || sessionNodeMap_[sessionId].bypass) {
        AUDIO_INFO_LOG("no need to update the sceneType %{public}d", sceneType);
        return;
    }
    DisConnectInputSession(sessionId);
    ConnectInputSession(sessionId);
}

std::string HpaeRendererManager::GetThreadName()
{
    return sinkInfo_.deviceName;
}

bool HpaeRendererManager::SetSessionFade(uint32_t sessionId, IOperation operation)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sinkInputNodeMap_, sessionId), false,
        "can not get input node of session %{public}u", sessionId);
    HpaeProcessorType sceneType = (sinkInputNodeMap_[sessionId]->connectedProcessorType_ != HPAE_SCENE_UNCONNECTED)
        ? sinkInputNodeMap_[sessionId]->connectedProcessorType_ : GetProcessorType(sessionId);
    AUDIO_INFO_LOG("sessionId is %{public}d, sceneType is %{public}d", sessionId, sceneType);
    std::shared_ptr<HpaeGainNode> sessionGainNode = nullptr;
    if (SafeGetMap(sceneClusterMap_, sceneType)) {
        sessionGainNode = sceneClusterMap_[sceneType]->GetGainNodeById(sessionId);
    }
    if (sinkInputNodeMap_[sessionId]->GetState() == HPAE_SESSION_STOPPED ||
        sinkInputNodeMap_[sessionId]->GetState() == HPAE_SESSION_PAUSED ||
        sessionGainNode == nullptr || !IsRunning()) {
        AUDIO_WARNING_LOG("session %{public}d do not have gain node or sink is not running!", sessionId);
        if (operation != OPERATION_STARTED) {
            HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
            SetSessionState(sessionId, state);
            sinkInputNodeMap_[sessionId]->SetState(state);
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
        sinkInputNodeMap_[sessionId]->SetState(state);
    }

    return true;
}

int32_t HpaeRendererManager::DumpSinkInfo()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "HpaeRendererManager not init");
    auto request = [this]() {
        AUDIO_INFO_LOG("DumpSinkInfo deviceName %{public}s", sinkInfo_.deviceName.c_str());
        UploadDumpSinkInfo(sinkInfo_.deviceName);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::SetOffloadPolicy(uint32_t sessionId, int32_t state)
{
    auto request = [this, sessionId, state]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::SetOffloadPolicy");
        AUDIO_INFO_LOG("SetOffloadPolicy sessionId %{public}u, deviceName %{public}s, state %{public}d", sessionId,
            sinkInfo_.deviceName.c_str(), state);
        if (SafeGetMap(sinkInputNodeMap_, sessionId)) {
            sinkInputNodeMap_[sessionId]->SetOffloadEnabled(state != OFFLOAD_DEFAULT);
        } else {
            AUDIO_ERR_LOG("not find sessionId %{public}u", sessionId);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::UpdateCollaborativeState(bool isCollaborationEnabled)
{
    auto request = [this, isCollaborationEnabled]() {
        if (isCollaborationEnabled_ == isCollaborationEnabled) {
            AUDIO_INFO_LOG("collaboration state not changed, isCollaborationEnabled_ %{public}d",
                isCollaborationEnabled_);
            return;
        }
        AUDIO_INFO_LOG("collaborativeState change from %{public}d to %{public}d",
            isCollaborationEnabled_, isCollaborationEnabled);
        isCollaborationEnabled_ = isCollaborationEnabled;
        if (isCollaborationEnabled_) {
            // for collaboration enabled
            EnableCollaboration();
        } else {
            // for collaboration disabled
            DisableCollaboration();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::HandlePriPaPower(uint32_t sessionId)
{
    if (!SafeGetMap(sinkInputNodeMap_, sessionId) || sinkInfo_.deviceClass != "primary") {
        return ERR_INVALID_OPERATION;
    }
    auto &nodeInfo = sinkInputNodeMap_[sessionId]->GetNodeInfo();
    struct VolumeValues volumes;
    auto audioVolume = AudioVolume::GetInstance();
    float curVolume = audioVolume->GetVolume(sessionId, nodeInfo.streamType, sinkInfo_.deviceClass, &volumes);
    auto isZeroVolume = audioVolume->IsSameVolume(0.0f, curVolume);
    AUDIO_INFO_LOG("session %{public}u, stream %{public}d, is zero volume %{public}d",
        sessionId, nodeInfo.streamType, isZeroVolume);
    if (!isZeroVolume) {
        return outputCluster_->SetPriPaPower();
    }
    return SUCCESS;
}

std::string HpaeRendererManager::GetDeviceHDFDumpInfo()
{
    std::string config;
    TransDeviceInfoToString(sinkInfo_, config);
    return config;
}

int32_t HpaeRendererManager::ConnectCoBufferNode(const std::shared_ptr<HpaeCoBufferNode> &coBufferNode)
{
    auto request = [this, coBufferNode]() {
        CHECK_AND_RETURN_LOG((outputCluster_ != nullptr) && (coBufferNode != nullptr),
            "outputCluster or coBufferNode is nullptr");
        if (!coBufferNode->IsOutputClusterConnected()) {
            outputCluster_->Connect(coBufferNode);
            coBufferNode->SetOutputClusterConnected(true);
        }
        if (outputCluster_->GetState() != STREAM_MANAGER_RUNNING && !isSuspend_) {
            outputCluster_->Start();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeRendererManager::DisConnectCoBufferNode(const std::shared_ptr<HpaeCoBufferNode> &coBufferNode)
{
    auto request = [this, coBufferNode]() {
        CHECK_AND_RETURN_LOG((outputCluster_ != nullptr) && (coBufferNode != nullptr),
            "outputCluster or coBufferNode is nullptr");
        if (coBufferNode->IsOutputClusterConnected()) {
            outputCluster_->DisConnect(coBufferNode);
            coBufferNode->SetOutputClusterConnected(false);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeRendererManager::ReConnectNodeForCollaboration(uint32_t sessionId)
{
    // todo fade out
    CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
        "sinkInputNodeMap_ not find sessionId %{public}u", sessionId);
    DeleteProcessCluster(sessionId);
    AUDIO_INFO_LOG("AddSingleNodeToSink sessionId %{public}u", sessionId);
    AddSingleNodeToSink(sinkInputNodeMap_[sessionId]);
}

void HpaeRendererManager::EnableCollaboration()
{
    if (hpaeCoBufferNode_ == nullptr) {
        hpaeCoBufferNode_ = std::make_shared<HpaeCoBufferNode>();
    }

    std::vector<uint32_t> sinkInputNodeMapKeys;
    for (auto& [key, node] : sinkInputNodeMap_) {
        HpaeNodeInfo nodeInfo = node->GetNodeInfo();
        if (nodeInfo.effectInfo.effectScene == SCENE_MUSIC || nodeInfo.effectInfo.effectScene == SCENE_MOVIE) {
            sinkInputNodeMapKeys.push_back(key);
        }
    }

    for (auto& key : sinkInputNodeMapKeys) {
        ReConnectNodeForCollaboration(key);
    }
}

void HpaeRendererManager::DisableCollaboration()
{
    std::vector<uint32_t> sinkInputNodeMapKeys;
    for (auto& [key, node] : sinkInputNodeMap_) {
        HpaeNodeInfo nodeInfo = node->GetNodeInfo();
        if (nodeInfo.effectInfo.effectScene == SCENE_COLLABORATIVE) {
            sinkInputNodeMapKeys.push_back(key);
        }
    }
    for (auto& key : sinkInputNodeMapKeys) {
        ReConnectNodeForCollaboration(key);
    }
}

int32_t HpaeRendererManager::HandleSyncId(uint32_t sessionId, int32_t syncId)
{
    if (!SafeGetMap(sinkInputNodeMap_, sessionId) || sinkInfo_.deviceClass != "primary") {
        return ERR_INVALID_OPERATION;
    }
    return outputCluster_->SetSyncId(syncId);
}

void HpaeRendererManager::TriggerStreamState(uint32_t sessionId, const std::shared_ptr<HpaeSinkInputNode> &inputNode)
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

bool HpaeRendererManager::IsClusterDisConnected(HpaeProcessorType sceneType)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sceneClusterMap_, sceneType), true,
        "sceneType %{public}d not in sceneClusterMap", sceneType);
    return (sceneClusterMap_[sceneType]->GetPreOutNum() == 0 && !sceneClusterMap_[sceneType]->GetConnectedFlag());
}

// if one stream underrun, and need to sleep, return true
// if one stream underrun, and need to doprocess instead of sleep, return false, do not refresh lastOnUnderrunTime_
// if not one stream underrun, return false, refresh lastOnUnderrunTime_
bool HpaeRendererManager::QueryOneStreamUnderrun()
{
    CHECK_AND_RETURN_RET(!IsRemoteDevice() && appsUid_.size() == 1 && hpaeSignalProcessThread_, false);
    auto underrunFlag = false;
    for (const auto &[id, node] : sinkInputNodeMap_) {
        CHECK_AND_RETURN_RET_LOG(node, false, "nullptr in map");
        if (node->GetState() == HPAE_SESSION_RUNNING) {
            underrunFlag = node->QueryUnderrun();
            break;
        }
    }
    if (underrunFlag) {
        lastOnUnderrunTime_ = lastOnUnderrunTime_ == 0 ? ClockTime::GetCurNano() : lastOnUnderrunTime_;
        int64_t sleepTimeInNs = lastOnUnderrunTime_ + UNDERRUN_BYPASS_DURATION_NS - ClockTime::GetCurNano();
        Trace trace("HpaeRendererManager::sleep " + std::to_string(sleepTimeInNs) + "ns underrun");
        CHECK_AND_RETURN_RET(sleepTimeInNs > 0, false);
        // sleep atmost 10ms
        hpaeSignalProcessThread_->SleepUntilNotify(std::min(BUFFER_DURATION_US, sleepTimeInNs / AUDIO_NS_PER_US));
        return true;
    } else {
        lastOnUnderrunTime_ = 0;
    }
    return false;
}

void HpaeRendererManager::DeleteNodesByTraversal(uint32_t sessionId)
{
    auto it = sceneClusterMap_.begin();
    for (; it != sceneClusterMap_.end(); ++it) {
        if (SafeGetMap(sceneClusterMap_, it->first) && it->second->DestroyNodes(sessionId) == SUCCESS) {
            AUDIO_INFO_LOG("SessionId %{public}d, Nodes found in processorType %{public}d and destroyed",
                it->first, sessionId);
            break;
        }
    }
    if (it == sceneClusterMap_.end()) {
        AUDIO_WARNING_LOG("SessionId %{public}u, Nodes not found in any sceneCluster", sessionId);
    }
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS