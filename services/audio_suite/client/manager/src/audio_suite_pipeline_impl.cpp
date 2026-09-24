/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSuitePipelineImpl"
#endif

#include <string>
#include <atomic>
#include <limits>
#include <unordered_map>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <unistd.h>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "media_monitor_manager.h"
#include "media_monitor_info.h"
#include "audio_system_manager.h"
#include "audio_workgroup_client_manager.h"
#include "audio_suite_common.h"
#include "audio_suite_capabilities.h"
#include "audio_suite_eq_node.h"
#include "audio_suite_env_node.h"
#include "audio_suite_pipeline_impl.h"
#include "audio_suite_input_node.h"
#include "audio_suite_aiss_node.h"
#include "audio_suite_output_node.h"
#include "audio_suite_nr_node.h"
#include "audio_suite_soundfield_node.h"
#include "audio_suite_mixer_node.h"
#include "audio_suite_voice_beautifier_node.h"
#include "audio_suite_general_voice_change_node.h"
#include "audio_suite_pure_voice_change_node.h"
#include "audio_suite_tempo_pitch_node.h"
#include "audio_suite_space_render_node.h"
#include "audio_suite_hoa_render_node.h"
#include "audio_suite_system_dialogue_enhance_node.h"
#include "audio_suite_system_music_master_node.h"
#include "stream_dfx_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
namespace {
static constexpr int32_t EQ_FREQUENCY_BAND_GAINS_MIN = -10;
static constexpr int32_t EQ_FREQUENCY_BAND_GAINS_MAX = 10;
static constexpr float SPACE_RENDER_MIN_CART_POINT_DISTANCE = -5.0f;
static constexpr float SPACE_RENDER_MAX_CART_POINT_DISTANCE = 5.0f;
static constexpr float SPACE_RENDER_MIN_TIME = 2.0f;
static constexpr float SPACE_RENDER_MAX_TIME = 40.0f;
static constexpr float SPACE_RENDER_MIN_EXPAND_RADIUS = 1.0f;
static constexpr float SPACE_RENDER_MAX_EXPAND_RADIUS = 5.0f;
static constexpr int32_t SPACE_RENDER_MIN_EXPAND_ANGLE = 0;
static constexpr int32_t SPACE_RENDER_MAX_EXPAND_ANGLE = 360;
static constexpr float AUDIO_VOICE_MORPHING_PITCH_MIN = 0.3f;
static constexpr float AUDIO_VOICE_MORPHING_PITCH_MAX = 3.0f;
static constexpr float AUDIO_VOICE_MORPHING_PITCH_DEFAULT = 0.0f;
static constexpr float SPEED_RATE_MIN = 0.5f;
static constexpr float SPEED_RATE_MAX = 10.0f;
static constexpr float PITCH_RATE_MIN = 0.1f;
static constexpr float PITCH_RATE_MAX = 5.0f;

static constexpr uint64_t FRAME_DURATION_MS = 20;
static constexpr uint32_t MAX_PIPELINE_NUM = 10;

static constexpr AudioSuitePipelineCfg PIPELINE_NODE_CFG;
}  // namespace

std::mutex AudioSuitePipelineImpl::createMutex_;
bool AudioSuitePipelineImpl::globalInit_ = false;
uint32_t AudioSuitePipelineImpl::allocateId_ = 0;
int32_t AudioSuitePipelineImpl::workGroupId_ = -1;
uint32_t AudioSuitePipelineImpl::pipelineCount_ = 0;

static const std::unordered_map<AudioNodeType,
    std::function<std::shared_ptr<AudioNode>()>> EFFECT_NODE_FACTORY = {
    {NODE_TYPE_EQUALIZER, []() { return std::make_shared<AudioSuiteEqNode>(); }},
    {NODE_TYPE_ENVIRONMENT_EFFECT, []() { return std::make_shared<AudioSuiteEnvNode>(); }},
    {NODE_TYPE_GENERAL_VOICE_CHANGE, []() { return std::make_shared<AudioSuiteGeneralVoiceChangeNode>(); }},
    {NODE_TYPE_PURE_VOICE_CHANGE, []() { return std::make_shared<AudioSuitePureVoiceChangeNode>(); }},
    {NODE_TYPE_VOICE_BEAUTIFIER, []() { return std::make_shared<AudioSuiteVoiceBeautifierNode>(); }},
    {NODE_TYPE_NOISE_REDUCTION, []() { return std::make_shared<AudioSuiteNrNode>(); }},
    {NODE_TYPE_SOUND_FIELD, []() { return std::make_shared<AudioSuiteSoundFieldNode>(); }},
    {NODE_TYPE_AUDIO_MIXER, []() { return std::make_shared<AudioSuiteMixerNode>(); }},
    {NODE_TYPE_AUDIO_SEPARATION, []() { return std::make_shared<AudioSuiteAissNode>(); }},
    {NODE_TYPE_TEMPO_PITCH, []() { return std::make_shared<AudioSuiteTempoPitchNode>(); }},
    {NODE_TYPE_SPACE_RENDER, []() { return std::make_shared<AudioSuiteSpaceRenderNode>(); }},
    {NODE_TYPE_HOA_RENDER, []() { return std::make_shared<AudioSuiteHoaRenderNode>(); }},
    {NODE_TYPE_SYSTEM_DIALOGUE_ENHANCE, []() { return std::make_shared<AudioSuiteSystemDialogueEnhanceNode>(); }},
    {NODE_TYPE_SYSTEM_MUSIC_MASTER, []() { return std::make_shared<AudioSuiteSystemMusicMasterNode>(); }}
};

int32_t AudioSuitePipeline::Create(PipelineWorkMode workMode, std::shared_ptr<AudioSuitePipeline> &pipeline)
{
    return AudioSuitePipelineImpl::Create(workMode, pipeline);
}

int32_t AudioSuitePipeline::IsNodeTypeSupported(AudioNodeType nodeType, bool &isSupported)
{
    return AudioSuitePipelineImpl::IsNodeTypeSupported(nodeType, isSupported);
}

void AudioSuitePipelineImpl::InitGlobal()
{
    CHECK_AND_RETURN_LOG(!globalInit_, "AudioSuitePipelineImpl::InitGlobal already inited");

    auto manager = AudioSystemManager::GetInstance();
    CHECK_AND_RETURN_LOG(manager != nullptr, "AudioSystemManager::GetInstance failed!");
    workGroupId_ = manager->CreateAudioWorkgroup();
    CHECK_AND_RETURN_LOG(workGroupId_ > 0, "CreateAudioWorkgroup failed.");
    AUDIO_INFO_LOG("CreateAudioWorkgroup success, group id:%{public}d", workGroupId_);

    globalInit_ = true;
}

void AudioSuitePipelineImpl::DeInitGlobal()
{
    CHECK_AND_RETURN_LOG(globalInit_, "AudioSuitePipelineImpl::InitGlobal already deInited");

    auto manager = AudioSystemManager::GetInstance();
    if (workGroupId_ != -1) {
        CHECK_AND_RETURN_LOG(manager != nullptr, "AudioSystemManager::GetInstance failed!");
        auto ret = manager->ReleaseAudioWorkgroup(workGroupId_);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "ReleaseAudioWorkgroup failed.");
        AUDIO_INFO_LOG("ReleaseAudioWorkgroup success, group id:%{public}d", workGroupId_);
        workGroupId_ = -1;
    }

    globalInit_ = false;
}

uint32_t AudioSuitePipelineImpl::AllocatePipeLineId()
{
    uint32_t id = ++allocateId_;
    if (allocateId_ == std::numeric_limits<uint32_t>::max()) {
        allocateId_ = 0;
    }

    AUDIO_INFO_LOG("AllocatePipeLineId suc ess. id_ = %{public}u", id);
    return id;
}

int32_t AudioSuitePipelineImpl::Create(PipelineWorkMode workMode, std::shared_ptr<AudioSuitePipeline> &pipeline)
{
    std::lock_guard<std::mutex> lock(createMutex_);
    
    InitGlobal();
    if (pipelineCount_ >= MAX_PIPELINE_NUM) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::CREATE,
                           ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS),
            "create pipeline failed, more than max pipeline num.", true);
        AUDIO_ERR_LOG("create pipeline failed, more than max pipeline num.");
        return ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS;
    }

    auto p = std::make_shared<AudioSuitePipelineImpl>(workMode, AllocatePipeLineId());
    if (p == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::CREATE,
                           ERR_MEMORY_ALLOC_FAILED),
            "create pipeline failed, malloc error.", true);
        AUDIO_ERR_LOG("create pipeline failed, malloc error.");
        return ERR_MEMORY_ALLOC_FAILED;
    }

    pipelineCount_++;
    pipeline = p;
    return SUCCESS;
}

AudioSuitePipelineImpl::AudioSuitePipelineImpl(PipelineWorkMode mode, uint32_t id)
    : pipelineWorkMode_(mode), id_(id), nodeCounts_()
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    AUDIO_INFO_LOG("create pipeline mode:%{public}d, id:%{public}u", static_cast<int32_t>(pipelineWorkMode_), id_);
}

AudioSuitePipelineImpl::~AudioSuitePipelineImpl()
{
    DeInit();
    {
        std::lock_guard<std::mutex> lock(createMutex_);
        if (pipelineCount_ > 0) {
            pipelineCount_--;
            AUDIO_INFO_LOG("AudioSuitePipelineImpl finish pipelineCount_ = %{public}u", pipelineCount_);
        }
        if (pipelineCount_ == 0) {
            DeInitGlobal();
        }
    }
}

int32_t AudioSuitePipelineImpl::DeInit()
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    AUDIO_INFO_LOG("AudioSuitePipelineImpl::DeInit start id_ = %{public}u", id_);

    for (auto& [id, node] : nodeMap_) {
        if (node != nullptr) {
            node->DeInit();
        }
    }
    nodeMap_.clear();
    connections_.clear();
    reverseConnections_.clear();
    outputNode_ = nullptr;
    AUDIO_INFO_LOG("AudioSuitePipelineImpl::DeInit end");
    return SUCCESS;
}

uint32_t AudioSuitePipelineImpl::GetPipelineId()
{
    return id_;
}

PipelineWorkMode AudioSuitePipelineImpl::GetWorkMode()
{
    return pipelineWorkMode_;
}

void AudioSuitePipelineImpl::StartGroup()
{
    CHECK_AND_RETURN(pipelineWorkMode_ == PIPELINE_REALTIME_MODE && workGroupId_ > 0);
    auto manager = AudioSystemManager::GetInstance();
    CHECK_AND_RETURN_LOG(manager != nullptr, "AudioSystemManager::GetInstance() failed!");
    uint64_t startTime = 0;
    std::unordered_map<int32_t, bool> threads{{gettid(), true}};
    bool needUpdatePrio = true;
    manager->StartGroup(workGroupId_, startTime, FRAME_DURATION_MS, threads, needUpdatePrio);
}

void AudioSuitePipelineImpl::StopGroup()
{
    CHECK_AND_RETURN(pipelineWorkMode_ == PIPELINE_REALTIME_MODE && workGroupId_ > 0);
    auto manager = AudioSystemManager::GetInstance();
    CHECK_AND_RETURN_LOG(manager != nullptr, "AudioSystemManager::GetInstance() failed!");
    manager->StopGroup(workGroupId_);
}

int32_t AudioSuitePipelineImpl::Start()
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    if (pipelineState_ == PIPELINE_RUNNING) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::START, ERR_ILLEGAL_STATE),
            "Current pipeline already running", true);
        AUDIO_INFO_LOG("Current pipeline already running, id is %{public}d", id_);
        return ERR_ILLEGAL_STATE;
    }

    if (outputNode_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::START, ERR_ILLEGAL_STATE),
            "Current pipeline not have output node", true);
        AUDIO_INFO_LOG("Current pipeline not have output node");
        return ERR_ILLEGAL_STATE;
    }

    SetMixerNodeSampleRate();

    if (!CheckPipelineNode(outputNode_->GetAudioNodeId())) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::START, ERR_ILLEGAL_STATE),
            "Current pipeline node connet status error", true);
        AUDIO_INFO_LOG("Current pipeline node connet status error, id is %{public}d", id_);
        return ERR_ILLEGAL_STATE;
    }

    pipelineState_ = PIPELINE_RUNNING;
    AUDIO_INFO_LOG("Start pipeline, id is %{public}d", id_);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::Stop()
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    AUDIO_INFO_LOG("Stop pipeline, id is %{public}d", id_);

    if (pipelineState_ == PIPELINE_STOPPED) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::STOP, ERR_ILLEGAL_STATE),
            "Current pipeline already stop", true);
        AUDIO_INFO_LOG("Current pipeline already stop, id is %{public}d", id_);
        return ERR_ILLEGAL_STATE;
    }

    if (outputNode_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::STOP, ERR_ILLEGAL_STATE),
            "Current pipeline not output node", true);
        AUDIO_INFO_LOG("Current pipeline not output node, id is %{public}d", id_);
        return ERR_ILLEGAL_STATE;
    }

    for (const auto& [nodeId, node] : nodeMap_) {
        if (node != nullptr) {
            node->Flush();
            node->SetAudioNodeDataFinishedFlag(false);
        }
    }
    pipelineState_ = PIPELINE_STOPPED;

    // for dfx
    CheckRenderFrameOverTimeCount();

    return SUCCESS;
}

void AudioSuitePipelineImpl::SetMixerNodeSampleRate()
{
    for (const auto& [nodeId, node] : nodeMap_) {
        if (node == nullptr) {
            continue;
        }

        if (node->GetNodeType() != NODE_TYPE_AUDIO_MIXER) {
            continue;
        }

        auto it = reverseConnections_.find(nodeId);
        if (it == reverseConnections_.end() || it->second.empty()) {
            continue;
        }

        AudioSamplingRate firstSampleRate = static_cast<AudioSamplingRate>(0);
        bool allSameRate = true;

        for (const auto& prevNodeId : it->second) {
            auto prevNodeIt = nodeMap_.find(prevNodeId);
            if (prevNodeIt == nodeMap_.end() || prevNodeIt->second == nullptr) {
                continue;
            }

            AudioFormat prevNodeFormat = prevNodeIt->second->GetAudioNodeFormat();
            if (firstSampleRate == static_cast<AudioSamplingRate>(0)) {
                firstSampleRate = prevNodeFormat.rate;
            } else if (prevNodeFormat.rate != firstSampleRate) {
                allSameRate = false;
                break;
            }
        }

        if (allSameRate && firstSampleRate != static_cast<AudioSamplingRate>(0)) {
            AudioFormat mixerFormat = node->GetAudioNodeFormat();
            mixerFormat.rate = firstSampleRate;
            node->SetAudioNodeFormat(mixerFormat);
            AUDIO_INFO_LOG("Set mixer node %{public}d sample rate to %{public}d", nodeId, firstSampleRate);
        }
    }
}

int32_t AudioSuitePipelineImpl::GetPipelineState(AudioSuitePipelineState &state)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    state = pipelineState_;
    AUDIO_INFO_LOG("GetPipelineState is %{public}d", static_cast<int32_t>(pipelineState_));
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::CreateNode(AudioNodeBuilder builder, uint32_t &nodeId)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    AUDIO_DEBUG_LOG("CreateNode enter");
    nodeId = INVALID_NODE_ID;

    int32_t ret = CreateNodeCheckParam(builder);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "CreateNodeCheckParam failed");

    std::shared_ptr<AudioNode> node = CreateNodeForType(builder);
    if (node == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::CREATE, ERR_MEMORY_ALLOC_FAILED),
            "create node failed, malloc error.", true);
        AUDIO_ERR_LOG("create node failed, malloc error.");
        return ERR_MEMORY_ALLOC_FAILED;
    }
    ret = node->Init();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "node Init failed");

    node->SetAudioNodeWorkMode(pipelineWorkMode_);

    nodeMap_[node->GetAudioNodeId()] = node;
    nodeCounts_[builder.nodeType]++;
    nodeId = node->GetAudioNodeId();
    WriteSuiteEngineUtilizationStatsEvent(builder.nodeType);
    AUDIO_INFO_LOG("CreateNode finish, nodeId = %{public}u", nodeId);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::CreateNodeCheckParam(AudioNodeBuilder builder)
{
    if (nodeCounts_[builder.nodeType] >= GetMaxNodeNumsForType(builder.nodeType)) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::CREATE, ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS),
            "The number of nodes created exceeds the system limit", true);
        AUDIO_ERR_LOG("node create node failed, current type node max num is %{public}u.",
            nodeCounts_[builder.nodeType]);
        return ERR_AUDIO_SUITE_CREATED_EXCEED_SYSTEM_LIMITS;
    }

    if (builder.nodeType == NODE_TYPE_SYSTEM_DIALOGUE_ENHANCE || builder.nodeType == NODE_TYPE_SYSTEM_MUSIC_MASTER) {
        AUDIO_DEBUG_LOG("node create node type:%{public}d.", builder.nodeType);
        return SUCCESS;
    }

    bool isSupported;
    AudioSuiteCapabilities &audioSuiteCapabilities = AudioSuiteCapabilities::GetInstance();
    int32_t error = audioSuiteCapabilities.IsNodeTypeSupported(builder.nodeType, isSupported);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        (error == SUCCESS && isSupported), ERR_NOT_SUPPORTED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::CREATE, ERR_NOT_SUPPORTED),
            "node type is not supported.", true),
        "node type: %{public}d is not supported.", builder.nodeType);
    return SUCCESS;
}

uint32_t AudioSuitePipelineImpl::GetMaxNodeNumsForType(AudioNodeType type)
{
    if (type == NODE_TYPE_INPUT) {
        return PIPELINE_NODE_CFG.maxInputNodeNum_;
    }

    if (type == NODE_TYPE_OUTPUT) {
        return PIPELINE_NODE_CFG.maxOutputNodeNum_;
    }

    if (type == NODE_TYPE_AUDIO_MIXER) {
        return PIPELINE_NODE_CFG.maxMixNodeNum_;
    }

    if (type == NODE_TYPE_AUDIO_SEPARATION) {
        return PIPELINE_NODE_CFG.maxAudioSeparationNodeNum_;
    }

    return PIPELINE_NODE_CFG.maxEffectNodeNum_;
}

std::shared_ptr<AudioNode> AudioSuitePipelineImpl::CreateNodeForType(AudioNodeBuilder builder)
{
    AudioFormat audioFormat = builder.nodeFormat;

    if (builder.nodeType == NODE_TYPE_INPUT) {
        AUDIO_INFO_LOG("Create AudioInputNode");
        return std::make_shared<AudioInputNode>(audioFormat);
    }
    if (builder.nodeType == NODE_TYPE_OUTPUT) {
        AUDIO_INFO_LOG("Create AudioOutputNode");
        outputNode_ = std::make_shared<AudioOutputNode>(audioFormat);
        return std::static_pointer_cast<AudioNode>(outputNode_);
    }

    auto it = EFFECT_NODE_FACTORY.find(builder.nodeType);
    if (it != EFFECT_NODE_FACTORY.end()) {
        AUDIO_INFO_LOG("Create node type = %{public}d", static_cast<int32_t>(builder.nodeType));
        return it->second();
    }

    AUDIO_ERR_LOG("Create node failed, current type = %{public}d not support.",
        static_cast<int32_t>(builder.nodeType));
    return nullptr;
}

int32_t AudioSuitePipelineImpl::DestroyNode(uint32_t nodeId)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    if (nodeMap_.find(nodeId) == nodeMap_.end()) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::RELEASE, ERR_INVALID_PARAM),
            "DestroyNode node failed, node id is invailed.", true);
        AUDIO_ERR_LOG("DestroyNode node failed, node id is invailed.");
        return ERR_INVALID_PARAM;
    }

    auto node = nodeMap_[nodeId];
    if (node == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::RELEASE, ERR_AUDIO_SUITE_NODE_NOT_EXIST),
            "DestroyNode failed, node ptr nullptr.", true);
        AUDIO_ERR_LOG("DestroyNode failed, node ptr nullptr.");
        return ERR_AUDIO_SUITE_NODE_NOT_EXIST;
    }

    int32_t ret = SUCCESS;
    if (pipelineState_ == PIPELINE_RUNNING) {
        ret = DestroyNodeForRun(nodeId, node);
    } else {
        ret = DestroyNodeForStop(nodeId, node);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DestroyNode failed");

    AUDIO_INFO_LOG("DestroyNode success. nodeId = %{public}d.", nodeId);
    if (nodeCounts_[node->GetNodeType()] > 0) {
        nodeCounts_[node->GetNodeType()]--;
    }
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::DestroyNodeForStop(uint32_t nodeId, std::shared_ptr<AudioNode> node)
{
    RemovceForwardConnet(nodeId, node);
    RemovceBackwardConnet(nodeId, node);

    if (node.get() == outputNode_.get()) {
        outputNode_ = nullptr;
    }

    node->DeInit();
    nodeMap_.erase(nodeId);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::DestroyNodeForRun(uint32_t nodeId, std::shared_ptr<AudioNode> node)
{
    if (outputNode_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::RELEASE, ERR_ILLEGAL_STATE),
            "DestroyNode failed, pipeline running, can not find output node.", true);
        AUDIO_ERR_LOG("DestroyNode failed, pipeline running, can not find output node, nodeId = %{public}d.", nodeId);
        return ERR_ILLEGAL_STATE;
    }

    if (IsConnected(outputNode_->GetAudioNodeId(), nodeId)) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::RELEASE, ERR_ILLEGAL_STATE),
            "DestroyNode failed, pipeline running, can not destroy used node.", true);
        AUDIO_ERR_LOG("DestroyNode failed, pipeline running, can not destroy used node, nodeId = %{public}d.", nodeId);
        return ERR_ILLEGAL_STATE;
    }

    RemovceForwardConnet(nodeId, node);
    RemovceBackwardConnet(nodeId, node);

    node->DeInit();
    nodeMap_.erase(nodeId);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::BypassEffectNode(uint32_t nodeId, bool bypass)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_INVALID_PARAM,
        "BypassEffectNode: node id invalid");

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "BypassEffectNode: node is nullptr");

    CHECK_AND_RETURN_RET_LOG(
        (node->GetNodeType() != NODE_TYPE_INPUT) && (node->GetNodeType() != NODE_TYPE_OUTPUT),
        ERR_INVALID_OPERATION,
        "BypassEffectNode: input/output node not supported, nodeId=%{public}d", nodeId);

    node->SetBypassEffectNode(bypass);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::GetNodeBypassStatus(uint32_t nodeId, bool &bypass)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetNodeBypassStatus: node id invalid");

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetNodeBypassStatus: node is nullptr");

    bypass = node->GetNodeBypassStatus();
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetAudioFormat(uint32_t nodeId, AudioFormat audioFormat)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_INVALID_PARAM,
        "SetAudioFormat: node id invalid");

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "SetAudioFormat: node is nullptr");

    if ((pipelineState_ == PIPELINE_RUNNING) &&
        ((outputNode_ == nullptr) || (IsConnected(outputNode_->GetAudioNodeId(), nodeId)))) {
        AUDIO_ERR_LOG("SetAudioFormat failed, pipeline status is running, nodeId:%{public}u is using.", nodeId);
        return ERR_ILLEGAL_STATE;
    }

    node->SetAudioNodeFormat(audioFormat);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetRequestDataCallback(uint32_t nodeId,
    std::shared_ptr<InputNodeRequestDataCallBack> callback)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_INVALID_PARAM,
        "SetRequestDataCallback: node id invalid");

    if ((pipelineState_ == PIPELINE_RUNNING) &&
        ((outputNode_ == nullptr) || (IsConnected(outputNode_->GetAudioNodeId(), nodeId)))) {
        AUDIO_ERR_LOG("SetRequestDataCallback failed, "
            "pipeline status is running, nodeId:%{public}u is using.", nodeId);
        return ERR_ILLEGAL_STATE;
    }

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_INPUT, ERR_INVALID_PARAM,
        "SetRequestDataCallback: node type must be input");

    int32_t ret = node->SetRequestDataCallback(callback);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetRequestDataCallback failed");
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::ConnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    if (srcNodeId == destNodeId) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::CONFIG,
                ERR_AUDIO_SUITE_UNSUPPORT_CONNECT), "ConnectNodes failed, srcNodeId can not same destNodeId.", true);
        AUDIO_ERR_LOG("ConnectNodes failed, srcNodeId can not same destNodeId.");
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }

    if ((nodeMap_.find(srcNodeId) == nodeMap_.end()) || (nodeMap_.find(destNodeId) == nodeMap_.end())) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                BusinessScenario::CONFIG, ERR_INVALID_PARAM), "ConnectNodes failed, node id is invailed.", true);
        AUDIO_ERR_LOG("ConnectNodes failed, node id is invailed.");
        return ERR_INVALID_PARAM;
    }

    auto srcNode = nodeMap_[srcNodeId];
    auto destNode = nodeMap_[destNodeId];
    CHECK_AND_RETURN_RET_LOG(srcNode != nullptr && destNode != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "ConnectNodes failed, node ptr is nullptr.");

    if ((srcNode->GetNodeType() == NODE_TYPE_OUTPUT) || (destNode->GetNodeType() == NODE_TYPE_INPUT) ||
        ((srcNode->GetNodeType() == NODE_TYPE_AUDIO_SEPARATION) && (destNode->GetNodeType() != NODE_TYPE_OUTPUT))) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT, BusinessScenario::CONFIG,
                ERR_AUDIO_SUITE_UNSUPPORT_CONNECT), "Current node type does not support connection method.", true);
        AUDIO_ERR_LOG("ConnectNodes failed, srcType:%{public}d, destType: %{public}d error.",
            srcNode->GetNodeType(), destNode->GetNodeType());
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }

    if (IsDirectConnected(srcNodeId, destNodeId)) {
        AUDIO_INFO_LOG("srcNodeId = %{public}d and destNodeId = %{public}d already connet", srcNodeId, destNodeId);
        return SUCCESS;
    }

    int32_t ret = SUCCESS;
    if (pipelineState_ == PIPELINE_STOPPED) {
        ret = ConnectNodesForStop(srcNodeId, destNodeId, srcNode, destNode);
    } else {
        ret = ConnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ConnectNodes failed, srcNodeId=%{public}d, destNodeId=%{public}d",
        srcNodeId, destNodeId);

    if (srcNode->GetNodeType() == NODE_TYPE_AUDIO_MIXER) {
        srcNode->SetAudioNodeFormat(destNode->GetAudioNodeFormat());
    }

    AddNodeConnections(srcNodeId, destNodeId);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::ConnectNodesForStop(uint32_t srcNodeId, uint32_t destNodeId,
    std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode)
{
    RemovceBackwardConnet(srcNodeId, srcNode);
    if (destNode->GetNodeType() != NODE_TYPE_AUDIO_MIXER) {
        RemovceForwardConnet(destNodeId, destNode);
    }

    return destNode->Connect(srcNode);
}

int32_t AudioSuitePipelineImpl::ConnectNodesForRun(uint32_t srcNodeId, uint32_t destNodeId,
    std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode)
{
    if (outputNode_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::CONFIG, ERR_ILLEGAL_STATE),
            "ConnectNodes failed, pipeline running, can not find output node.", true);
        AUDIO_ERR_LOG("ConnectNodes failed, pipeline running, can not find output node.");
        return ERR_ILLEGAL_STATE;
    }

    if (IsConnected(outputNode_->GetAudioNodeId(), srcNodeId)) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::CONFIG, ERR_AUDIO_SUITE_UNSUPPORT_CONNECT),
            "ConnectNodes failed, pipeline running", true);
        AUDIO_ERR_LOG("ConnectNodes failed, pipeline running srcNode = %{public}d can not is used node.", srcNodeId);
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }

    if (!IsConnected(outputNode_->GetAudioNodeId(), destNodeId)) {
        RemovceBackwardConnet(srcNodeId, srcNode);
        if (destNode->GetNodeType() != NODE_TYPE_AUDIO_MIXER) {
            RemovceForwardConnet(destNodeId, destNode);
        }
        return destNode->Connect(srcNode);
    }

    if (destNode->GetNodeType() != NODE_TYPE_AUDIO_MIXER) {
        AUDIO_ERR_LOG("Pipeline status is running, destNodeId = %{public}d type must mix node", destNodeId);
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }
    if (!CheckPipelineNode(srcNodeId)) {
        AUDIO_ERR_LOG("Pipeline status is running, srcNodeId = %{public}d must connet from inputnode", srcNodeId);
        return ERR_AUDIO_SUITE_UNSUPPORT_CONNECT;
    }

    RemovceBackwardConnet(srcNodeId, srcNode);
    return destNode->Connect(srcNode);
}

int32_t AudioSuitePipelineImpl::DisconnectNodes(uint32_t srcNodeId, uint32_t destNodeId)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(srcNodeId != destNodeId, ERR_NOT_SUPPORTED,
        "DisconnectNodes: srcNodeId same as destNodeId");

    CHECK_AND_RETURN_RET_LOG(
        (nodeMap_.find(srcNodeId) != nodeMap_.end()) && (nodeMap_.find(destNodeId) != nodeMap_.end()),
        ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "DisconnectNodes: node id invalid");

    auto srcNode = nodeMap_[srcNodeId];
    auto destNode = nodeMap_[destNodeId];
    CHECK_AND_RETURN_RET_LOG((srcNode != nullptr) && (destNode != nullptr), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "DisconnectNodes: node is nullptr");

    CHECK_AND_RETURN_RET_LOG(
        (srcNode->GetNodeType() != NODE_TYPE_OUTPUT) && (destNode->GetNodeType() != NODE_TYPE_INPUT),
        ERR_NOT_SUPPORTED,
        "DisconnectNodes: node type error");

    if (!IsDirectConnected(srcNodeId, destNodeId)) {
        AUDIO_INFO_LOG("srcNodeId = %{public}d not connet destNodeId = %{public}d.", srcNodeId, destNodeId);
        return SUCCESS;
    }

    int32_t ret = SUCCESS;
    if (pipelineState_ == PIPELINE_STOPPED) {
        ret = destNode->DisConnect(srcNode);
    } else {
        ret = DisconnectNodesForRun(srcNodeId, destNodeId, srcNode, destNode);
    }
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "DisconnectNodes failed, srcNodeId=%{public}d, destNodeId=%{public}d",
        srcNodeId, destNodeId);

    ClearNodeConnections(srcNodeId, destNodeId);
    AUDIO_INFO_LOG("DisconnectNodes success.");
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::DisconnectNodesForRun(uint32_t srcNodeId, uint32_t destNodeId,
    std::shared_ptr<AudioNode> srcNode, std::shared_ptr<AudioNode> destNode)
{
    if (outputNode_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), ERR_PIPELINE_STATE_INVALID,
            "DisconnectNodes failed, pipeline running, can not find output node.", true);
        AUDIO_ERR_LOG("DisconnectNodes failed, pipeline running, can not find output node.");
        return ERR_ILLEGAL_STATE;
    }
    if (!IsConnected(outputNode_->GetAudioNodeId(), destNodeId)) {
        return destNode->DisConnect(srcNode);
    }

    if (destNode->GetNodeType() != NODE_TYPE_AUDIO_MIXER) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_DISCONNECT_NOT_SUPPORTED, "destNode not midx node.", true);
        return ERR_NOT_SUPPORTED;
    }

    if (reverseConnections_.find(destNodeId) == reverseConnections_.end()) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::CONFIG, ERR_ILLEGAL_STATE),
            "srcNode not connet destNode", true);
        return ERR_ILLEGAL_STATE;
    }

    if (reverseConnections_[destNodeId].size() <= 1) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_DISCONNECT_NOT_SUPPORTED,
            "midx node only one connected", true);
        return ERR_NOT_SUPPORTED;
    }

    return destNode->DisConnect(srcNode);
}

void AudioSuitePipelineImpl::RemovceForwardConnet(uint32_t nodeId, std::shared_ptr<AudioNode> node)
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

void AudioSuitePipelineImpl::RemovceBackwardConnet(uint32_t nodeId, std::shared_ptr<AudioNode> node)
{
    if (connections_.find(nodeId) == connections_.end()) {
        return;
    }

    auto destNodeId = connections_[nodeId];
    if (nodeMap_.find(destNodeId) == nodeMap_.end()) {
        return;
    }

    auto destNode = nodeMap_[destNodeId];
    if (destNode == nullptr) {
        return;
    }

    destNode->DisConnect(node);
    ClearNodeConnections(nodeId, destNodeId);
}

void AudioSuitePipelineImpl::AddNodeConnections(uint32_t srcNodeId, uint32_t destNodeId)
{
    connections_[srcNodeId] = destNodeId;
    reverseConnections_[destNodeId].push_back(srcNodeId);
}

void AudioSuitePipelineImpl::ClearNodeConnections(uint32_t srcNodeId, uint32_t destNodeId)
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

int32_t AudioSuitePipelineImpl::RenderFrame(
    uint8_t *audioData, int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    if (pipelineState_ != PIPELINE_RUNNING) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::RENDER, ERR_ILLEGAL_STATE),
            "RenderFrame failed, pipelineState state is not running.", true);
        AUDIO_ERR_LOG("RenderFrame failed, pipelineState state is not running.");
        return ERR_ILLEGAL_STATE;
    }

    if (outputNode_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::RENDER, ERR_ILLEGAL_STATE),
            "RenderFrame failed, outputNode is nullptr.", true);
        AUDIO_ERR_LOG("RenderFrame failed, outputNode is nullptr.");
        return ERR_ILLEGAL_STATE;
    }

    StartGroup();
    int32_t frameDuration = GetFrameDuration(requestFrameSize, outputNode_->GetAudioNodeFormat());
    auto startTime = std::chrono::steady_clock::now();
    int32_t ret = outputNode_->DoProcess(audioData, requestFrameSize, responseSize, finishedFlag);
    auto endTime = std::chrono::steady_clock::now();
    auto processDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    CheckRenderFrameTime(frameDuration, static_cast<uint64_t>(processDuration));
    StopGroup();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RenderFrame failed");

    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::MultiRenderFrame(
    uint8_t **audioDataArray, int32_t arraySize,
    int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);

    if (pipelineState_ != PIPELINE_RUNNING) {
        AUDIO_ERR_LOG("MultiRenderFrame failed, pipelineState state is not running.");
        return ERR_ILLEGAL_STATE;
    }

    if (outputNode_ == nullptr) {
        AUDIO_ERR_LOG("MultiRenderFrame failed, outputNode_ is nullptr.");
        return ERR_ILLEGAL_STATE;
    }

    StartGroup();
    int32_t frameDuration = GetFrameDuration(requestFrameSize, outputNode_->GetAudioNodeFormat());
    auto startTime = std::chrono::steady_clock::now();
    int32_t ret = outputNode_->DoProcess(audioDataArray, arraySize, requestFrameSize, responseSize, finishedFlag);
    auto endTime = std::chrono::steady_clock::now();
    auto processDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    CheckRenderFrameTime(frameDuration, static_cast<uint64_t>(processDuration));
    StopGroup();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "MultiRenderFrame failed");

    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetOptions(uint32_t nodeId, std::string name, std::string value)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);

    StartGroup();
    if (nodeMap_.find(nodeId) == nodeMap_.end()) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::CONFIG, ERR_INVALID_PARAM),
            "SetOptions failed, node id is invaild.", true);
        AUDIO_ERR_LOG("SetOptions failed, node id is invaild.");
        StopGroup();
        return ERR_INVALID_PARAM;
    }

    auto node = nodeMap_[nodeId];
    if (node == nullptr) {
        AUDIO_ERR_LOG("SetOptions failed, node is nullptr.");
        StopGroup();
        return ERR_INVALID_PARAM;
    }
    int32_t ret = node->SetOptions(name, value);
    StopGroup();

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetOptions failed");
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::GetOptions(uint32_t nodeId, std::string name, std::string &value)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);

    CHECK_AND_RETURN_RET_LOG(pipelineState_ == PIPELINE_STOPPED, ERR_ILLEGAL_STATE,
        "GetOptions: pipeline not stopped");

    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_INVALID_PARAM,
        "GetOptions: node id invalid");

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "GetOptions: node is nullptr");

    int32_t ret = node->GetOptions(name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetOptions failed");
    return SUCCESS;
}

bool AudioSuitePipelineImpl::CheckHoaInputNodeConnection(uint32_t nodeId, const AudioFormat &format)
{
    auto nextConnIter = connections_.find(nodeId);
    CHECK_AND_RETURN_RET_LOG(nextConnIter != connections_.end(), false, "Node not connected");

    auto nextNodeIter = nodeMap_.find(nextConnIter->second);
    CHECK_AND_RETURN_RET_LOG(nextNodeIter != nodeMap_.end() && nextNodeIter->second != nullptr,
        false, "HOA InputNode's nextNode not found");

    auto nextNode = nextNodeIter->second;
    AudioNodeType nextNodeType = nextNode->GetNodeType();
    CHECK_AND_RETURN_RET_LOG(nextNodeType == NODE_TYPE_HOA_RENDER, false,
        "HOA InputNode must connect to HoaRenderNode, but got type=%{public}d", nextNodeType);

    AudioFormat hoaNodeFormat = nextNode->GetAudioNodeFormat();
    hoaNodeFormat.audioChannelInfo = format.audioChannelInfo;
    nextNode->SetAudioNodeFormat(hoaNodeFormat);
    std::string layoutStr = std::to_string(static_cast<uint64_t>(format.audioChannelInfo.channelLayout));
    nextNode->SetOptions("hoaLayout", layoutStr);
    return true;
}

bool AudioSuitePipelineImpl::CheckHoaRenderPreNode(uint32_t nodeId)
{
    auto connIter = reverseConnections_.find(nodeId);
    CHECK_AND_RETURN_RET_LOG(connIter != reverseConnections_.end() && connIter->second.size() == 1, false,
        "HoaRenderNode must have exactly one preNode");
    auto preNodeIter = nodeMap_.find(connIter->second[0]);
    CHECK_AND_RETURN_RET_LOG(preNodeIter != nodeMap_.end() && preNodeIter->second != nullptr,
        false, "HoaRenderNode's preNode not found");

    AudioNodeType preNodeType = preNodeIter->second->GetNodeType();
    AudioFormat preNodeFormat = preNodeIter->second->GetAudioNodeFormat();
    CHECK_AND_RETURN_RET_LOG(preNodeType == NODE_TYPE_INPUT &&
        AudioSuiteUtil::IsHoaLayout(preNodeFormat.audioChannelInfo.channelLayout),
        false, "HoaRenderNode requires InputNode of HOA layout");
    return true;
}

bool AudioSuitePipelineImpl::CheckSystemNodeConnection(uint32_t nodeId)
{
    auto nextConnIter = connections_.find(nodeId);
    CHECK_AND_RETURN_RET_LOG(nextConnIter != connections_.end(), false, "Node not connected");

    auto nextNodeIter = nodeMap_.find(nextConnIter->second);
    CHECK_AND_RETURN_RET_LOG(nextNodeIter != nodeMap_.end() && nextNodeIter->second != nullptr,
        false, "systemNode nextNode not found");

    auto nextNode = nextNodeIter->second;
    AudioNodeType nextNodeType = nextNode->GetNodeType();
    CHECK_AND_RETURN_RET_LOG(nextNodeType == NODE_TYPE_OUTPUT, false,
        "system node must connect to outputNode, but got type=%{public}d", nextNodeType);

    auto connIter = reverseConnections_.find(nodeId);
    CHECK_AND_RETURN_RET_LOG(connIter != reverseConnections_.end() && connIter->second.size() == 1, false,
        "systemNode must have exactly one preNode");
    auto preNodeIter = nodeMap_.find(connIter->second[0]);
    CHECK_AND_RETURN_RET_LOG(preNodeIter != nodeMap_.end() && preNodeIter->second != nullptr,
        false, "systemNode preNode not found");

    AudioNodeType preNodeType = preNodeIter->second->GetNodeType();
    CHECK_AND_RETURN_RET_LOG(preNodeType == NODE_TYPE_INPUT, false, "systemNode requires InputNode");
    return true;
}

bool AudioSuitePipelineImpl::CheckPipelineNode(uint32_t startNodeId)
{
    std::queue<uint32_t> nodeQueue;
    std::unordered_set<uint32_t> visitedNodes;

    nodeQueue.push(startNodeId);
    while (!nodeQueue.empty()) {
        uint32_t currentNodeId = nodeQueue.front();
        nodeQueue.pop();

        if (visitedNodes.find(currentNodeId) != visitedNodes.end()) {
            return false;
        }

        auto nodeIter = nodeMap_.find(currentNodeId);
        CHECK_AND_RETURN_RET_LOG(nodeIter != nodeMap_.end() && nodeIter->second != nullptr,
            false, "current node is nullptr");
        AudioNodeType currentNodeType = nodeIter->second->GetNodeType();

        visitedNodes.insert(currentNodeId);

        auto connIter = reverseConnections_.find(currentNodeId);
        if (connIter == reverseConnections_.end()) {
            if (currentNodeType != NODE_TYPE_INPUT) {
                return false;
            }

            if (!nodeIter->second->IsSetReadDataCallback()) {
                return false;
            }

            AudioFormat currentNodeFormat = nodeIter->second->GetAudioNodeFormat();
            if (AudioSuiteUtil::IsHoaLayout(currentNodeFormat.audioChannelInfo.channelLayout)) {
                CHECK_AND_RETURN_RET_LOG(CheckHoaInputNodeConnection(currentNodeId, currentNodeFormat),
                    false, "HOA InputNode check failed");
            }
            continue;
        } else if (currentNodeType == NODE_TYPE_HOA_RENDER) {
            CHECK_AND_RETURN_RET_LOG(CheckHoaRenderPreNode(currentNodeId), false,
                "HoaRenderNode preNode check failed");
        } else if (nodeIter->second->IsSystemNodeType()) {
            CHECK_AND_RETURN_RET_LOG(CheckSystemNodeConnection(currentNodeId), false,
                "SystemNode check failed, nodeId:%{public}u.", currentNodeId);
        }

        for (uint32_t nextNodeId : connIter->second) {
            nodeQueue.push(nextNodeId);
        }
    }

    return true;
}

bool AudioSuitePipelineImpl::IsConnected(uint32_t srcNodeId, uint32_t destNodeId)
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

bool AudioSuitePipelineImpl::IsDirectConnected(uint32_t srcNodeId, uint32_t destNodeId)
{
    if (connections_.find(srcNodeId) == connections_.end()) {
        return false;
    }

    return connections_[srcNodeId] == destNodeId;
}

int32_t AudioSuitePipelineImpl::GetFrameDuration(int32_t frameSize, const AudioFormat &nodeFormat)
{
    int32_t bytesPerSecond = static_cast<int32_t>(nodeFormat.rate) *
                            static_cast<int32_t>(nodeFormat.audioChannelInfo.numChannels) *
                            static_cast<int32_t>(AudioSuiteUtil::GetSampleSize(nodeFormat.format));

    CHECK_AND_RETURN_RET_LOG(bytesPerSecond != 0, 0, "Invalid AudioFormat.");
    double frameDurationMS = std::ceil(static_cast<double>(frameSize) * SECONDS_TO_MS / bytesPerSecond);

    return static_cast<int32_t>(frameDurationMS);
}

void AudioSuitePipelineImpl::CheckRenderFrameTime(int32_t frameDurationMS, uint64_t processDurationUS)
{
    if (frameDurationMS == 0) {
        AUDIO_WARNING_LOG("Invalid para, frame duration is 0.");
        return;
    }

    renderFrameTotalCount_++;

    uint64_t frameDurationUS = static_cast<uint64_t>(frameDurationMS) * MILLISECONDS_TO_MICROSECONDS;
    for (size_t i = 0; i < RTF_OVERTIME_LEVELS; ++i) {
        if (processDurationUS >= static_cast<uint64_t>(frameDurationUS * RTF_OVERTIME_THRESHOLDS[i])) {
            renderFrameOvertimeCounters_[i]++;
        }
    }
}

void AudioSuitePipelineImpl::CheckRenderFrameOverTimeCount()
{
    std::string pipelineWorkMode = (pipelineWorkMode_ == PIPELINE_REALTIME_MODE) ? "Realtime mode" : "Edit mode";
    AUDIO_INFO_LOG("[%{public}s] pipeline renderFrame realtimeFactor overtime counters(1.0, 1.1, 1.2): %{public}d, "
                   "%{public}d, %{public}d, renderFrame total count: %{public}d",
        pipelineWorkMode.c_str(),
        renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_BASE],
        renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_110BASE],
        renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_120BASE],
        renderFrameTotalCount_);

    bool allOvertimeCounterZero = std::all_of(
        std::begin(renderFrameOvertimeCounters_),
        std::end(renderFrameOvertimeCounters_),
        [](int32_t count) { return count == 0; }
    );
    if (!allOvertimeCounterZero) {
        std::shared_ptr<Media::MediaMonitor::EventBean> bean =
            std::make_shared<Media::MediaMonitor::EventBean>(Media::MediaMonitor::ModuleId::AUDIO,
                Media::MediaMonitor::EventId::SUITE_ENGINE_UTILIZATION_STATS,
                Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);

        bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
        bean->Add("AUDIO_NODE_TYPE", "PIPELINE");
        if (pipelineWorkMode_ == PIPELINE_REALTIME_MODE) {
            bean->Add("RT_MODE_RENDER_COUNT", renderFrameTotalCount_);
            bean->Add("RT_MODE_RTF_OVER_BASE_COUNT", renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_BASE]);
            bean->Add("RT_MODE_RTF_OVER_110BASE_COUNT", renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_110BASE]);
            bean->Add("RT_MODE_RTF_OVER_120BASE_COUNT", renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_120BASE]);
            bean->Add("RT_MODE_RTF_OVER_100_COUNT", renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_BASE]);
        } else {
            bean->Add("EDIT_MODE_RENDER_COUNT", renderFrameTotalCount_);
            bean->Add("EDIT_MODE_RTF_OVER_BASE_COUNT", renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_BASE]);
            bean->Add("EDIT_MODE_RTF_OVER_110BASE_COUNT", renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_110BASE]);
            bean->Add("EDIT_MODE_RTF_OVER_120BASE_COUNT", renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_120BASE]);
            bean->Add("EDIT_MODE_RTF_OVER_100_COUNT", renderFrameOvertimeCounters_[RtfOvertimeLevel::OVER_BASE]);
        }
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);

        AUDIO_WARNING_LOG("pipeline run renderFrame timeout, report SuiteEngineUtilizationStats event.");
    }

    renderFrameTotalCount_ = 0;
    std::fill(renderFrameOvertimeCounters_.begin(), renderFrameOvertimeCounters_.end(), 0);
}

const std::unordered_map<uint32_t, std::shared_ptr<AudioNode>> &AudioSuitePipelineImpl::GetNodeMap() const
{
    return nodeMap_;
}
const std::unordered_map<uint32_t, uint32_t> &AudioSuitePipelineImpl::GetConnections() const
{
    return connections_;
}
const std::unordered_map<uint32_t, std::vector<uint32_t>> &AudioSuitePipelineImpl::GetReverseConnections() const
{
    return reverseConnections_;
}

AudioSuitePipelineState AudioSuitePipelineImpl::GetCurrentPipelineState() const
{
    return pipelineState_;
}

int32_t AudioSuitePipelineImpl::MetaRenderFrame(AudioMetaFrame &metaFrame,
    int32_t *responseAudioSize, int32_t *responseMetaSize, bool *finishedFlag)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    if (pipelineState_ != PIPELINE_RUNNING) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::RENDER, ERR_ILLEGAL_STATE),
            "MetaRenderFrame failed, pipelineState state is not running.", true);
        AUDIO_ERR_LOG("MetaRenderFrame failed, pipelineState state is not running.");
        return ERR_ILLEGAL_STATE;
    }

    if (outputNode_ == nullptr) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_WRONG_AUDIO_EFFECT, OperationType::EDIT,
                           BusinessScenario::RENDER, ERR_ILLEGAL_STATE),
            "MetaRenderFrame failed, outputNode is nullptr.", true);
        AUDIO_ERR_LOG("MetaRenderFrame failed, outputNode is nullptr.");
        return ERR_ILLEGAL_STATE;
    }

    StartGroup();
    int32_t frameDuration = GetFrameDuration(metaFrame.audioDataSize, outputNode_->GetAudioNodeFormat());
    auto startTime = std::chrono::steady_clock::now();
    int32_t ret = outputNode_->DoProcess(metaFrame, responseAudioSize, responseMetaSize, finishedFlag);
    auto endTime = std::chrono::steady_clock::now();
    auto processDuration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
    CheckRenderFrameTime(frameDuration, static_cast<uint64_t>(processDuration));
    StopGroup();

    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "MetaRenderFrame failed");
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetSystemNodeParam(uint32_t nodeId, std::vector<uint8_t> &param)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(pipelineState_ == PIPELINE_STOPPED, ERR_ILLEGAL_STATE,
        "SetSystemNodeParam: pipeline not stopped");

    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetSystemNodeParam: node id invalid");

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "SetSystemNodeParam: node is nullptr");

    AudioNodeType nodeType = node->GetNodeType();
    CHECK_AND_RETURN_RET_LOG(
        (nodeType == NODE_TYPE_SYSTEM_DIALOGUE_ENHANCE || nodeType == NODE_TYPE_SYSTEM_MUSIC_MASTER),
        ERR_NOT_SUPPORTED,
        "SetSystemNodeParam: node type not supported, type=%{public}d", nodeType);

    CHECK_AND_RETURN_RET_LOG(!param.empty(), ERR_INVALID_PARAM,
        "SetSystemNodeParam: param is empty");

    int32_t ret = node->SetSystemParam(const_cast<std::vector<uint8_t>&>(param));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "SetSystemNodeParam failed");
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::GetSystemNodeParam(uint32_t nodeId, std::vector<uint8_t> &param)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(pipelineState_ == PIPELINE_STOPPED, ERR_ILLEGAL_STATE,
        "GetSystemNodeParam: pipeline not stopped");

    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSystemNodeParam: node id invalid");

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSystemNodeParam: node is nullptr");

    AudioNodeType nodeType = node->GetNodeType();
    CHECK_AND_RETURN_RET_LOG(
        (nodeType == NODE_TYPE_SYSTEM_DIALOGUE_ENHANCE || nodeType == NODE_TYPE_SYSTEM_MUSIC_MASTER),
        ERR_NOT_SUPPORTED,
        "GetSystemNodeParam: node type not supported, type=%{public}d", nodeType);

    int32_t ret = node->GetSystemParam(param);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSystemNodeParam failed");
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::GetSystemNodeInOutSize(uint32_t nodeId, uint32_t &inSize, uint32_t &outSize)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(pipelineState_ == PIPELINE_STOPPED, ERR_ILLEGAL_STATE,
        "GetSystemNodeInOutSize: pipeline not stopped");

    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSystemNodeInOutSize: node id invalid");

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_AUDIO_SUITE_NODE_NOT_EXIST,
        "GetSystemNodeInOutSize: node is nullptr");

    AudioNodeType nodeType = node->GetNodeType();
    CHECK_AND_RETURN_RET_LOG(
        (nodeType == NODE_TYPE_SYSTEM_DIALOGUE_ENHANCE || nodeType == NODE_TYPE_SYSTEM_MUSIC_MASTER),
        ERR_NOT_SUPPORTED,
        "GetSystemNodeInOutSize: node type not supported, type=%{public}d", nodeType);

    int32_t ret = node->GetInOutSize(inSize, outSize);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSystemNodeInOutSize failed");
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetRequestMetaDataCallback(uint32_t nodeId,
    std::shared_ptr<InputNodeRequestMetaDataCallBack> callback)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    CHECK_AND_RETURN_RET_LOG(nodeMap_.find(nodeId) != nodeMap_.end(), ERR_INVALID_PARAM,
        "SetRequestMetaDataCallback: node id invalid");

    if ((pipelineState_ == PIPELINE_RUNNING) &&
        ((outputNode_ == nullptr) || (IsConnected(outputNode_->GetAudioNodeId(), nodeId)))) {
        AUDIO_ERR_LOG("SetRequestMetaDataCallback failed, "
            "pipeline status is running, nodeId:%{public}u is using.", nodeId);
        return ERR_ILLEGAL_STATE;
    }

    auto node = nodeMap_[nodeId];
    CHECK_AND_RETURN_RET_LOG(node != nullptr, ERR_INVALID_PARAM,
        "SetRequestMetaDataCallback: node is nullptr");

    CHECK_AND_RETURN_RET_LOG(node->GetNodeType() == NODE_TYPE_INPUT, ERR_INVALID_PARAM,
        "SetRequestMetaDataCallback: node type must be input");

    node->SetRequestMetaDataCallback(callback);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::IsNodeTypeSupported(AudioNodeType nodeType, bool &isSupported)
{
    if (nodeType == NODE_TYPE_AUDIO_MIXER) {
        AUDIO_INFO_LOG("MixerNode is supported on all device.");
        isSupported = true;
        return SUCCESS;
    }
    AudioSuiteCapabilities &audioSuiteCapabilities = AudioSuiteCapabilities::GetInstance();
    int32_t ret = audioSuiteCapabilities.IsNodeTypeSupported(nodeType, isSupported);
    if (ret == SUCCESS) {
        AUDIO_INFO_LOG("nodeType: %{public}d is supported on this device.", nodeType);
    } else {
        AUDIO_ERR_LOG("IsNodeTypeSupported failed, Wrong effect nodeType: %{public}d", nodeType);
        isSupported = false;
    }
    return SUCCESS;
}

bool AudioSuitePipelineImpl::ValidateEqualizerGains(const AudioEqualizerFrequencyBandGains &gains)
{
    for (size_t idx = 0; idx < sizeof(gains.gains) / sizeof(gains.gains[0]); idx++) {
        if (gains.gains[idx] < EQ_FREQUENCY_BAND_GAINS_MIN || gains.gains[idx] > EQ_FREQUENCY_BAND_GAINS_MAX) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(
                static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
                "SetEqualizerFrequencyBandGains failed, input value not in -10, 10.", true);
            AUDIO_ERR_LOG("SetEqualizerFrequencyBandGains failed, input value %{public}d not in -10, 10.",
                gains.gains[idx]);
            return false;
        }
    }
    return true;
}

bool AudioSuitePipelineImpl::ValidateSpaceRenderPosition(const AudioSpaceRenderPositionParams &position)
{
    bool valid = (position.x >= SPACE_RENDER_MIN_CART_POINT_DISTANCE) &&
                 (position.x <= SPACE_RENDER_MAX_CART_POINT_DISTANCE) &&
                 (position.y >= SPACE_RENDER_MIN_CART_POINT_DISTANCE) &&
                 (position.y <= SPACE_RENDER_MAX_CART_POINT_DISTANCE) &&
                 (position.z >= SPACE_RENDER_MIN_CART_POINT_DISTANCE) &&
                 (position.z <= SPACE_RENDER_MAX_CART_POINT_DISTANCE);
    if (!valid) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderPositionParams failed, point distance must be in the -5.0f~5.0f", true);
        AUDIO_ERR_LOG("SetSpaceRenderPositionParams failed, point distance must be in the -5.0f~5.0f");
    }
    return valid;
}

bool AudioSuitePipelineImpl::ValidateSpaceRenderRotation(const AudioSpaceRenderRotationParams &rotation)
{
    bool coordValid = (rotation.x >= SPACE_RENDER_MIN_CART_POINT_DISTANCE) &&
                      (rotation.x <= SPACE_RENDER_MAX_CART_POINT_DISTANCE) &&
                      (rotation.y >= SPACE_RENDER_MIN_CART_POINT_DISTANCE) &&
                      (rotation.y <= SPACE_RENDER_MAX_CART_POINT_DISTANCE) &&
                      (rotation.z >= SPACE_RENDER_MIN_CART_POINT_DISTANCE) &&
                      (rotation.z <= SPACE_RENDER_MAX_CART_POINT_DISTANCE);
    if (!coordValid) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderRotationParams failed, point distance must be in the -5.0f~5.0f", true);
        AUDIO_ERR_LOG("SetSpaceRenderRotationParams failed, point distance must be in the -5.0f~5.0f");
        return false;
    }
    bool timeValid = (rotation.surroundTime >= SPACE_RENDER_MIN_TIME) &&
                     (rotation.surroundTime <= SPACE_RENDER_MAX_TIME);
    if (!timeValid) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderRotationParams failed, time must be in the 2.0f~40.0f", true);
        AUDIO_ERR_LOG("SetSpaceRenderRotationParams failed, time must be in the 2.0f~40.0f");
        return false;
    }
    bool dirValid = (rotation.surroundDirection == AudioSurroundDirection::SPACE_RENDER_CCW ||
                     rotation.surroundDirection == AudioSurroundDirection::SPACE_RENDER_CW);
    if (!dirValid) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderRotationParams failed, surround direction must be 0 or 1", true);
        AUDIO_ERR_LOG("SetSpaceRenderRotationParams failed, surround direction must be 0 or 1");
        return false;
    }
    return true;
}

bool AudioSuitePipelineImpl::ValidateSpaceRenderExtension(const AudioSpaceRenderExtensionParams &extension)
{
    if (extension.extRadius < SPACE_RENDER_MIN_EXPAND_RADIUS ||
        extension.extRadius > SPACE_RENDER_MAX_EXPAND_RADIUS) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderExtensionParams failed, Radius must be in the 1.0f~5.0f", true);
        AUDIO_ERR_LOG("SetSpaceRenderExtensionParams failed, Radius must be in the 1.0f~5.0f");
        return false;
    }
    if (extension.extAngle <= SPACE_RENDER_MIN_EXPAND_ANGLE ||
        extension.extAngle >= SPACE_RENDER_MAX_EXPAND_ANGLE) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetSpaceRenderExtensionParams failed, Angle must be in the (0, 360)", true);
        AUDIO_ERR_LOG("SetSpaceRenderExtensionParams failed, Angle must be in the (0, 360)");
        return false;
    }
    return true;
}

bool AudioSuitePipelineImpl::ValidateTempoPitch(float speed, float pitch)
{
    bool valid = (speed >= SPEED_RATE_MIN && speed <= SPEED_RATE_MAX) &&
                 (pitch >= PITCH_RATE_MIN && pitch <= PITCH_RATE_MAX);
    if (!valid) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetTempoAndPitch failed, speed must be in the 0.5~10.0 and pitch must be in the 0.1~5.0.", true);
        AUDIO_ERR_LOG("SetTempoAndPitch failed, speed %{public}f must be in the 0.5~10.0 and pitch %{public}f "
            "must be in the 0.1~5.0.", speed, pitch);
    }
    return valid;
}

bool AudioSuitePipelineImpl::ValidatePureVoiceChange(const AudioPureVoiceChangeOption &option)
{
    bool valid = (option.pitch >= AUDIO_VOICE_MORPHING_PITCH_MIN &&
                  option.pitch <= AUDIO_VOICE_MORPHING_PITCH_MAX) ||
                 option.pitch == AUDIO_VOICE_MORPHING_PITCH_DEFAULT;
    if (!valid) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(
            static_cast<int32_t>(getuid()), EDIT_NODE_PARAMETERS_INVALID,
            "SetPureVoiceChangeOption failed, pitch must be in the 0.3f~3.0f or 0.0f.", true);
        AUDIO_ERR_LOG("SetPureVoiceChangeOption failed, pitch %{public}f must be in the 0.3f~3.0f or 0.0f.",
            option.pitch);
    }
    return valid;
}

int32_t AudioSuitePipelineImpl::SetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains gains)
{
    if (!ValidateEqualizerGains(gains)) {
        return ERR_INVALID_PARAM;
    }
    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "";
    for (size_t idx = 0; idx < sizeof(gains.gains) / sizeof(gains.gains[0]); idx++) {
        value += std::to_string(gains.gains[idx]);
        value += ":";
    }
    return SetOptions(nodeId, name, value);
}

int32_t AudioSuitePipelineImpl::GetEqualizerFrequencyBandGains(uint32_t nodeId, AudioEqualizerFrequencyBandGains &gains)
{
    std::string name = "AudioEqualizerFrequencyBandGains";
    std::string value = "";
    int32_t ret = GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEqualizerFrequencyBandGains failed");
    std::vector<int32_t> parseValue;
    parseValue.reserve(EQUALIZER_BAND_NUM);
    std::istringstream iss(value);
    std::string token;
    while (std::getline(iss, token, ':')) {
        token.erase(0, token.find_first_not_of(' '));
        token.erase(token.find_last_not_of(' ') + 1);
        if (!token.empty()) {
            char* end = nullptr;
            errno = 0;
            long val = std::strtol(token.c_str(), &end, 10);
            if (end != token.c_str() + token.size() || errno == ERANGE ||
                val < INT32_MIN || val > INT32_MAX) {
                break;
            }
            parseValue.push_back(static_cast<int32_t>(val));
        }
    }
    if (parseValue.size() > EQUALIZER_BAND_NUM) {
        AUDIO_ERR_LOG("GetEqualizerFrequencyBandGains ParseValue error");
        return ERROR;
    }
    for (size_t idx = 0; idx < parseValue.size(); idx++) {
        gains.gains[idx] = parseValue[idx];
    }
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetSoundFieldType(uint32_t nodeId, SoundFieldType soundFieldType)
{
    std::string name = "SoundFieldType";
    std::string value = std::to_string(static_cast<int32_t>(soundFieldType));
    return SetOptions(nodeId, name, value);
}

int32_t AudioSuitePipelineImpl::GetSoundFieldType(uint32_t nodeId, SoundFieldType &soundFieldType)
{
    std::string name = "SoundFieldType";
    std::string value = "";
    int32_t ret = GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetSoundFieldType failed");

    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(SoundFieldType::AUDIO_SUITE_SOUND_FIELD_CLOSE) ||
        parseValue > static_cast<int32_t>(SoundFieldType::AUDIO_SUITE_SOUND_FIELD_WIDE)) {
        return ERROR;
    }
    soundFieldType = static_cast<SoundFieldType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetEnvironmentType(uint32_t nodeId, EnvironmentType environmentType)
{
    std::string name = "EnvironmentType";
    std::string value = std::to_string(static_cast<int32_t>(environmentType));
    return SetOptions(nodeId, name, value);
}

int32_t AudioSuitePipelineImpl::GetEnvironmentType(uint32_t nodeId, EnvironmentType &environmentType)
{
    std::string name = "EnvironmentType";
    std::string value = "";
    int32_t ret = GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetEnvironmentType failed");

    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(EnvironmentType::AUDIO_SUITE_ENVIRONMENT_TYPE_CLOSE) ||
        parseValue > static_cast<int32_t>(EnvironmentType::AUDIO_SUITE_ENVIRONMENT_TYPE_GRAMOPHONE)) {
        return ERROR;
    }
    environmentType = static_cast<EnvironmentType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType voiceBeautifierType)
{
    std::string name = "VoiceBeautifierType";
    std::string value = std::to_string(static_cast<int32_t>(voiceBeautifierType));
    return SetOptions(nodeId, name, value);
}

int32_t AudioSuitePipelineImpl::GetVoiceBeautifierType(uint32_t nodeId, VoiceBeautifierType &voiceBeautifierType)
{
    std::string name = "VoiceBeautifierType";
    std::string value = "";
    int32_t ret = GetOptions(nodeId, name, value);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetVoiceBeautifierType failed");

    int32_t parseValue = StringToInt32(value);
    if (parseValue < static_cast<int32_t>(VoiceBeautifierType::AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_CLEAR) ||
        parseValue > static_cast<int32_t>(VoiceBeautifierType::AUDIO_SUITE_VOICE_BEAUTIFIER_TYPE_STUDIO)) {
        return ERROR;
    }
    voiceBeautifierType = static_cast<VoiceBeautifierType>(parseValue);
    return SUCCESS;
}

int32_t AudioSuitePipelineImpl::SetSpaceRenderPositionParams(uint32_t nodeId, AudioSpaceRenderPositionParams position)
{
    if (!ValidateSpaceRenderPosition(position)) {
        return ERR_INVALID_PARAM;
    }
    return SetParamsTemplate(nodeId, "AudioSpaceRenderPositionParams", position);
}

int32_t AudioSuitePipelineImpl::GetSpaceRenderPositionParams(uint32_t nodeId, AudioSpaceRenderPositionParams &position)
{
    return GetParamsTemplate(nodeId, "AudioSpaceRenderPositionParams", position);
}

int32_t AudioSuitePipelineImpl::SetSpaceRenderRotationParams(uint32_t nodeId, AudioSpaceRenderRotationParams rotation)
{
    if (!ValidateSpaceRenderRotation(rotation)) {
        return ERR_INVALID_PARAM;
    }
    return SetParamsTemplate(nodeId, "AudioSpaceRenderRotationParams", rotation);
}

int32_t AudioSuitePipelineImpl::GetSpaceRenderRotationParams(uint32_t nodeId, AudioSpaceRenderRotationParams &rotation)
{
    return GetParamsTemplate(nodeId, "AudioSpaceRenderRotationParams", rotation);
}

int32_t AudioSuitePipelineImpl::SetSpaceRenderExtensionParams(
    uint32_t nodeId, AudioSpaceRenderExtensionParams extension)
{
    if (!ValidateSpaceRenderExtension(extension)) {
        return ERR_INVALID_PARAM;
    }
    return SetParamsTemplate(nodeId, "AudioSpaceRenderExtensionParams", extension);
}

int32_t AudioSuitePipelineImpl::GetSpaceRenderExtensionParams(
    uint32_t nodeId, AudioSpaceRenderExtensionParams &extension)
{
    return GetParamsTemplate(nodeId, "AudioSpaceRenderExtensionParams", extension);
}

int32_t AudioSuitePipelineImpl::SetTempoAndPitch(uint32_t nodeId, float speed, float pitch)
{
    if (!ValidateTempoPitch(speed, pitch)) {
        return ERR_INVALID_PARAM;
    }
    TempoPitchParams params;
    params.speed = speed;
    params.pitch = pitch;
    return SetParamsTemplate(nodeId, "speedAndPitch", params);
}

int32_t AudioSuitePipelineImpl::GetTempoAndPitch(uint32_t nodeId, float &speed, float &pitch)
{
    TempoPitchParams params;
    int32_t ret = GetParamsTemplate(nodeId, "speedAndPitch", params);
    if (ret == SUCCESS) {
        speed = params.speed;
        pitch = params.pitch;
    }
    return ret;
}

int32_t AudioSuitePipelineImpl::SetPureVoiceChangeOption(uint32_t nodeId, AudioPureVoiceChangeOption option)
{
    if (!ValidatePureVoiceChange(option)) {
        return ERR_INVALID_PARAM;
    }
    return SetParamsTemplate(nodeId, "AudioPureVoiceChangeOption", option);
}

int32_t AudioSuitePipelineImpl::GetPureVoiceChangeOption(uint32_t nodeId, AudioPureVoiceChangeOption &option)
{
    return GetParamsTemplate(nodeId, "AudioPureVoiceChangeOption", option);
}

int32_t AudioSuitePipelineImpl::SetGeneralVoiceChangeType(uint32_t nodeId, AudioGeneralVoiceChangeType type)
{
    return SetParamsTemplate(nodeId, "AudioGeneralVoiceChangeType", type);
}

int32_t AudioSuitePipelineImpl::GetGeneralVoiceChangeType(uint32_t nodeId, AudioGeneralVoiceChangeType &type)
{
    return GetParamsTemplate(nodeId, "AudioGeneralVoiceChangeType", type);
}

void AudioSuitePipelineImpl::WriteSuiteEngineUtilizationStatsEvent(AudioNodeType nodeType)
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

void AudioSuitePipelineImpl::WriteSuiteEngineExceptionEvent(uint32_t scene, uint32_t result,
    const std::string& description)
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

int32_t AudioSuitePipelineImpl::StringToInt32(std::string &str)
{
    char* end;
    errno = 0;
    long value = std::strtol(str.c_str(), &end, 10);
    if (end == str.c_str()) {
        return INT32_MAX;
    }
    while (*end != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*end))) {
            return INT32_MAX;
        }
        ++end;
    }
    if (errno == ERANGE || value < INT32_MIN || value > INT32_MAX) {
        return INT32_MAX;
    }
    return static_cast<int32_t>(value);
}

int32_t AudioSuitePipelineImpl::PrintInfo(int32_t fd)
{
    std::lock_guard<std::mutex> lock(pipelineMutex_);
    std::string content = GenerateTextSnapshot();
    return WriteOutput(content, fd);
}

std::string AudioSuitePipelineImpl::GenerateTextSnapshot()
{
    std::stringstream ss;
    std::string timestamp = AudioSuiteUtil::GetCurrentTimestamp();

    ss << "========================================\n";
    ss << "Audio Suite Pipeline Snapshot\n";
    ss << "Timestamp: " << timestamp << "\n";
    ss << "========================================\n\n";

    PrintPipelineSnapshotText(ss);

    ss << "========================================\n";
    return ss.str();
}

void AudioSuitePipelineImpl::PrintPipelineSnapshotText(std::stringstream &ss)
{
    AudioSuitePipelineState state = GetCurrentPipelineState();
    PipelineWorkMode workMode = GetWorkMode();
    const auto& nodeMap = GetNodeMap();
    const auto& connections = GetConnections();
    const auto& reverseConnections = GetReverseConnections();

    ss << "Pipeline [ID: " << id_ << "]\n";
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

void AudioSuitePipelineImpl::PrintNodeSnapshotText(std::shared_ptr<AudioNode> node, std::stringstream &ss)
{
    AudioNodeInfo &info = node->GetAudioNodeInfo();

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

int32_t AudioSuitePipelineImpl::WriteOutput(const std::string &content, int32_t fd)
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

template<typename T>
int32_t AudioSuitePipelineImpl::SetParamsTemplate(uint32_t nodeId, const std::string& name, const T& param)
{
    std::string value = Serializer<T>::Serialize(param);
    return SetOptions(nodeId, name, value);
}

template<typename T>
int32_t AudioSuitePipelineImpl::GetParamsTemplate(uint32_t nodeId, const std::string& name, T& param)
{
    std::string value;
    int32_t ret = GetOptions(nodeId, name, value);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetParamsTemplate: GetOptions failed, ret %{public}d", ret);
        return ret;
    }
    if (!Serializer<T>::Deserialize(value, param)) {
        AUDIO_ERR_LOG("GetParamsTemplate: Deserialize failed");
        return ERROR;
    }
    return SUCCESS;
}

template int32_t AudioSuitePipelineImpl::SetParamsTemplate<AudioSpaceRenderPositionParams>(
    uint32_t nodeId, const std::string& name, const AudioSpaceRenderPositionParams& param);
template int32_t AudioSuitePipelineImpl::GetParamsTemplate<AudioSpaceRenderPositionParams>(
    uint32_t nodeId, const std::string& name, AudioSpaceRenderPositionParams& param);
template int32_t AudioSuitePipelineImpl::SetParamsTemplate<AudioSpaceRenderRotationParams>(
    uint32_t nodeId, const std::string& name, const AudioSpaceRenderRotationParams& param);
template int32_t AudioSuitePipelineImpl::GetParamsTemplate<AudioSpaceRenderRotationParams>(
    uint32_t nodeId, const std::string& name, AudioSpaceRenderRotationParams& param);
template int32_t AudioSuitePipelineImpl::SetParamsTemplate<AudioSpaceRenderExtensionParams>(
    uint32_t nodeId, const std::string& name, const AudioSpaceRenderExtensionParams& param);
template int32_t AudioSuitePipelineImpl::GetParamsTemplate<AudioSpaceRenderExtensionParams>(
    uint32_t nodeId, const std::string& name, AudioSpaceRenderExtensionParams& param);
template int32_t AudioSuitePipelineImpl::SetParamsTemplate<TempoPitchParams>(
    uint32_t nodeId, const std::string& name, const TempoPitchParams& param);
template int32_t AudioSuitePipelineImpl::GetParamsTemplate<TempoPitchParams>(
    uint32_t nodeId, const std::string& name, TempoPitchParams& param);
template int32_t AudioSuitePipelineImpl::SetParamsTemplate<AudioPureVoiceChangeOption>(
    uint32_t nodeId, const std::string& name, const AudioPureVoiceChangeOption& param);
template int32_t AudioSuitePipelineImpl::GetParamsTemplate<AudioPureVoiceChangeOption>(
    uint32_t nodeId, const std::string& name, AudioPureVoiceChangeOption& param);
template int32_t AudioSuitePipelineImpl::SetParamsTemplate<AudioGeneralVoiceChangeType>(
    uint32_t nodeId, const std::string& name, const AudioGeneralVoiceChangeType& param);
template int32_t AudioSuitePipelineImpl::GetParamsTemplate<AudioGeneralVoiceChangeType>(
    uint32_t nodeId, const std::string& name, AudioGeneralVoiceChangeType& param);

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
