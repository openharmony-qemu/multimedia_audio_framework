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
#define LOG_TAG "HpaeCapturerManager"
#endif

#include "hpae_capturer_manager.h"
#include "audio_info.h"
#include "audio_errors.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#include "audio_effect_map.h"
#include "hpae_policy_manager.h"
#include "audio_engine_log.h"
#include "hpae_message_queue_monitor.h"
#include "hpae_stream_move_monitor.h"
#include "stream_dfx_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

const std::string DEFAULT_DEVICE_CLASS = "primary";
const std::string DEFAULT_DEVICE_NETWORKID = "LocalDevice";
const std::string REMOTE_DEVICE_CLASS = "remote";

HpaeCapturerManager::HpaeCapturerManager(HpaeSourceInfo &sourceInfo)
    : hpaeNoLockQueue_(CURRENT_REQUEST_COUNT), sourceInfo_(sourceInfo)
{
    AUDIO_INFO_LOG("Source info: mic[%{public}d_%{public}d_%{public}d] "\
        "ec[%{public}d_%{public}d_%{public}d_%{public}d] "\
        "micref[%{public}d_%{public}d_%{public}d_%{public}d]",
        sourceInfo.samplingRate, sourceInfo.channels, sourceInfo.format,
        sourceInfo.ecType, sourceInfo.ecSamplingRate, sourceInfo.ecChannels, sourceInfo.ecFormat,
        sourceInfo.micRef, sourceInfo.micRefSamplingRate, sourceInfo.micRefChannels, sourceInfo.micRefFormat);
    CHECK_AND_RETURN(sourceInfo_.sourceType == SOURCE_TYPE_VOICE_RECOGNITION ||
        sourceInfo_.sourceType == SOURCE_TYPE_CAMCORDER);
    AUDIO_INFO_LOG("micin info:[%{public}u_%{public}u_%{public}u]",
        sourceInfo_.micInSamplingRate, sourceInfo_.micInFormat, sourceInfo_.micInChannels);
}

HpaeCapturerManager::~HpaeCapturerManager()
{
    if (isInit_.load()) {
        DeInit();
    }
}

int32_t HpaeCapturerManager::CaptureEffectCreate(const HpaeProcessorType &processorType,
    const AudioEnhanceScene &sceneType)
{
    const std::unordered_map<AudioEnhanceScene, std::string> &audioEnhanceSupportedSceneTypes =
        GetEnhanceSupportedSceneType();
    auto item = audioEnhanceSupportedSceneTypes.find(sceneType);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(item != audioEnhanceSupportedSceneTypes.end(), ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD,
                BusinessScenario::CREATE, ERR_INVALID_PARAM),
            "sceneType not supported", false),
        "sceneType %{public}d not supported", sceneType);
    uint64_t sceneCode = static_cast<uint64_t>(sceneType);
    uint64_t sceneKeyCode = 0;
    sceneKeyCode = (sceneCode << SCENE_TYPE_OFFSET) + (captureId_ << CAPTURER_ID_OFFSET) + renderId_;
    AUDIO_INFO_LOG("sceneCode:%{public}" PRIu64 " captureId_ %{public}d sceneKeyCode:%{public}" PRIu64,
        sceneCode, captureId_, sceneKeyCode);
    CaptureEffectAttr attr = {};
    attr.needEc = sourceInfo_.ecType != HPAE_EC_TYPE_NONE;
    attr.needMicRef = sourceInfo_.micRef == HPAE_REF_ON;
    attr.micChannels = static_cast<uint32_t>(sourceInfo_.channels);
    attr.ecChannels = static_cast<uint32_t>(sourceInfo_.ecChannels);
    attr.micRefChannels = static_cast<uint32_t>(sourceInfo_.micRefChannels);
    
    int32_t ret = sceneClusterMap_[processorType]->CaptureEffectCreate(sceneKeyCode, attr);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD,
                BusinessScenario::CREATE, ERR_OPERATION_FAILED),
            "sceneType create failed", false),
        "sceneType[%{public}u] create failed", sceneType);
    return SUCCESS;
}

HpaeProcessorType HpaeCapturerManager::GetEffectiveSceneType(uint32_t sessionId)
{
    auto it = sessionNodeMap_.find(sessionId);
    CHECK_AND_RETURN_RET(it != sessionNodeMap_.end(), HPAE_SCENE_EFFECT_NONE);
    CHECK_AND_RETURN_RET(activeCollabSourceTypes_.count(it->second.sourceType) == 0,
        HPAE_SCENE_COLLABORATIVE_RECORD);
    return it->second.sceneType;
}

int32_t HpaeCapturerManager::CreateOutputSession(const HpaeStreamInfo &streamInfo)
{
    AUDIO_INFO_LOG("CreateStream sessionId %{public}u deviceName %{public}s,channel:%{public}u,rate:%{public}u",
        streamInfo.sessionId, sourceInfo_.deviceName.c_str(), streamInfo.channels, streamInfo.samplingRate);
    HpaeNodeInfo nodeInfo;
    ConfigNodeInfo(nodeInfo, streamInfo);
    HpaeProcessorType sceneType = TransSourceTypeToSceneType(streamInfo.sourceType);
    nodeInfo.sceneType = sceneType;
    if (streamInfo.sourceType == SOURCE_TYPE_OFFLOAD_CAPTURE) {
        nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    } else {
        nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    }
    
    nodeInfo.statusCallback = weak_from_this();

    sessionNodeMap_[streamInfo.sessionId].sceneType = sceneType;
    sessionNodeMap_[streamInfo.sessionId].sourceType = streamInfo.sourceType;

    HpaeProcessorType effectiveSceneType = GetEffectiveSceneType(streamInfo.sessionId);
    AudioEnhanceScene enhanceScene = TransProcessType2EnhanceScene(effectiveSceneType);
    nodeInfo.effectInfo.enhanceScene = enhanceScene;
    sourceOutputNodeMap_[streamInfo.sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    sourceOutputNodeMap_[streamInfo.sessionId]->SetAppUid(streamInfo.uid);
    sessionNodeMap_[streamInfo.sessionId].sceneType = sceneType;
    sessionNodeMap_[streamInfo.sessionId].mixWithWakeUp = streamInfo.mixWithWakeUp;

    CreateSceneCluster(effectiveSceneType, enhanceScene);

    return SUCCESS;
}

void HpaeCapturerManager::UpdateClusterNodeInfoForRecognition(HpaeNodeInfo &clusterNodeInfo)
{
    CHECK_AND_RETURN(sourceInfo_.sourceType == SOURCE_TYPE_VOICE_RECOGNITION ||
        sourceInfo_.sourceType == SOURCE_TYPE_CAMCORDER);
    // for recognition, mic = preprocess(1ch) + micin(4ch)
    uint8_t totalChannels = static_cast<uint8_t>(sourceInfo_.micInChannels) +
        static_cast<uint8_t>(sourceInfo_.channels) + static_cast<uint8_t>(sourceInfo_.ecChannels);
    CHECK_AND_RETURN_LOG(totalChannels <= CHANNEL_16,
        "totalChannels[%{public}u] is not supported", totalChannels);
    clusterNodeInfo.channels = static_cast<AudioChannel>(totalChannels);
}

void HpaeCapturerManager::CreateSceneCluster(HpaeProcessorType sceneType, AudioEnhanceScene enhanceScene)
{
    CHECK_AND_RETURN(sceneType != HPAE_SCENE_EFFECT_NONE && !SafeGetMap(sceneClusterMap_, sceneType));
    // DEVICE_TYPE_ACCESSORY effect move to HAL
    CHECK_AND_RETURN_LOG(sourceInfo_.deviceType != DEVICE_TYPE_ACCESSORY,
        "deviceType %{public}d needn't create effect", sourceInfo_.deviceType);

    // todo: algorithm instance count control
    HpaeNodeInfo clusterNodeInfo;
    clusterNodeInfo.channels = sourceInfo_.channels;
    clusterNodeInfo.format = sourceInfo_.format;
    clusterNodeInfo.samplingRate = sourceInfo_.samplingRate;
    clusterNodeInfo.frameLen = CalculateFrameLenBySampleRate(clusterNodeInfo.samplingRate);
    clusterNodeInfo.statusCallback = weak_from_this();
    clusterNodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    clusterNodeInfo.effectInfo.enhanceScene = enhanceScene;

    UpdateClusterNodeInfoForRecognition(clusterNodeInfo);
    
    sceneClusterMap_[sceneType] = std::make_shared<HpaeSourceProcessCluster>(clusterNodeInfo);
    CHECK_AND_RETURN(CaptureEffectCreate(sceneType, enhanceScene) != SUCCESS);
    // not erase effect processcluster for inject
    AUDIO_WARNING_LOG("sceneType[%{public}u] create failed, not delete sceneCluster", sceneType);
}

int32_t HpaeCapturerManager::CaptureEffectRelease(const HpaeProcessorType &sceneType)
{
    uint64_t sceneCode = static_cast<uint64_t>(TransProcessType2EnhanceScene(sceneType));
    uint64_t sceneKeyCode = 0;
    sceneKeyCode = (sceneCode << SCENE_TYPE_OFFSET) + (captureId_ << CAPTURER_ID_OFFSET) + renderId_;
    int32_t ret = sceneClusterMap_[sceneType]->CaptureEffectRelease(sceneKeyCode);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::RELEASE, ERR_OPERATION_FAILED),
            "sceneType release failed", false),
        "sceneType[%{public}u] release failed", sceneType);
    return SUCCESS;
}

void HpaeCapturerManager::DisConnectSceneClusterFromSourceInputCluster(HpaeProcessorType &sceneType)
{
    auto sceneCluster = SafeGetMap(sceneClusterMap_, sceneType);
    CHECK_AND_RETURN_LOG(sceneCluster != nullptr, "connot find sceneType:%{public}u", sceneType);
    CHECK_AND_RETURN_LOG(sceneCluster->GetOutputPortNum() == 0,
        "sceneType:%{public}u outputNum:%{public}u",
        sceneType, static_cast<uint32_t>(sceneCluster->GetOutputPortNum()));
    // need to disconnect sceneCluster and sourceInputCluster
    if (sceneCluster->IsEffectNodeValid()) {
        HpaeNodeInfo ecNodeInfo;
        HpaeSourceInputNodeType ecNodeType;
        if (CheckEcCondition(sceneType, ecNodeInfo, ecNodeType)) {
            sceneCluster->DisConnectWithInfo(sourceInputClusterMap_[ecNodeType], ecNodeInfo); // ec
        }

        HpaeNodeInfo micRefNodeInfo;
        if (CheckMicRefCondition(sceneType, micRefNodeInfo)) {
            // micref
            sceneCluster->DisConnectWithInfo(sourceInputClusterMap_[HPAE_SOURCE_MICREF], micRefNodeInfo);
        }
    }

    if (sceneType == HPAE_SCENE_COLLABORATIVE_RECORD) {
        HpaeNodeInfo collabNodeInfo;
        auto collabInputCluster = SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_DEFAULT);
        if (collabInputCluster && sceneCluster->GetCapturerEffectConfig(collabNodeInfo,
            HPAE_SOURCE_BUFFER_TYPE_DEFAULT)) {
            collabNodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_DEFAULT;
            sceneCluster->DisConnectWithInfo(collabInputCluster, collabNodeInfo);
        }
    }

    HpaeNodeInfo micNodeInfo;
    if (SafeGetMap(sourceInputClusterMap_, mainMicType_) &&
        sceneCluster->GetCapturerEffectConfig(micNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MIC)) {
        sceneCluster->DisConnectWithInfo(
            sourceInputClusterMap_[mainMicType_], micNodeInfo); // mic
    }
    return;
}

int32_t HpaeCapturerManager::DeleteOutputSession(uint32_t sessionId, bool keepSessionMaps)
{
    AUDIO_INFO_LOG("delete output node:%{public}d, source name:%{public}s", sessionId, sourceInfo_.deviceClass.c_str());
    auto sourceOutputNode = SafeGetMap(sourceOutputNodeMap_, sessionId);
    if (!sourceOutputNode) {
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_REMOVE, sessionId, CAPTURER_INVALID);
        if (!keepSessionMaps) {
            sourceOutputNodeMap_.erase(sessionId);
            sessionNodeMap_.erase(sessionId);
        }
        return SUCCESS;
    }
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(false, sourceOutputNode->GetNodeInfo());
#endif

    HpaeProcessorType sceneType = GetEffectiveSceneType(sessionId);
    if (sceneType != HPAE_SCENE_EFFECT_NONE && SafeGetMap(sceneClusterMap_, sceneType)) {
        sourceOutputNodeMap_[sessionId]->DisConnectWithInfo(
            sceneClusterMap_[sceneType], sourceOutputNodeMap_[sessionId]->GetNodeInfo());
        DisConnectSceneClusterFromSourceInputCluster(sceneType);
        if (sceneClusterMap_[sceneType]->GetOutputPortNum() == 0) {
            CaptureEffectRelease(sceneType);
            sceneClusterMap_.erase(sceneType);
        }
    } else if (SafeGetMap(sourceInputClusterMap_, mainMicType_)) {
        sourceOutputNodeMap_[sessionId]->DisConnectWithInfo(sourceInputClusterMap_[mainMicType_],
            sourceOutputNodeMap_[sessionId]->GetNodeInfo());
    }

    if (SafeGetMap(sourceInputClusterMap_, mainMicType_) &&
        sourceInputClusterMap_[mainMicType_]->GetOutputPortNum() == 0) {
        CapturerSourceStop();
    }
    
    HpaeSessionState outputState = sourceOutputNodeMap_[sessionId]->GetState();
    CapturerState state = outputState == HPAE_SESSION_RELEASED ? CAPTURER_INVALID : CAPTURER_RELEASED;
    NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_REMOVE, sessionId, state);
    if (!keepSessionMaps) {
        sourceOutputNodeMap_.erase(sessionId);
        sessionNodeMap_.erase(sessionId);
    }
    return SUCCESS;
}

void HpaeCapturerManager::SetSessionState(uint32_t sessionId, HpaeSessionState capturerState)
{
    sessionNodeMap_[sessionId].state = capturerState;
    sourceOutputNodeMap_[sessionId]->SetState(capturerState);
}

int32_t HpaeCapturerManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        AUDIO_ERR_LOG("not init");
        return ERR_INVALID_OPERATION;
    }
    int32_t checkRet = CheckStreamInfo(streamInfo);
    if (checkRet != SUCCESS) {
        return checkRet;
    }
    auto request = [this, streamInfo]() {
        CreateOutputSession(streamInfo);
        SetSessionState(streamInfo.sessionId, HPAE_SESSION_PREPARED);
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, streamInfo.sessionId, CAPTURER_PREPARED,
            sourceOutputNodeMap_[streamInfo.sessionId]->GetAppUid());
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        AUDIO_ERR_LOG("not init");
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        // map check in DeleteOutputSession
        DeleteOutputSession(sessionId);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

bool HpaeCapturerManager::CheckEcCondition(const HpaeProcessorType &sceneType, HpaeNodeInfo &ecNodeInfo,
    HpaeSourceInputNodeType &ecNodeType)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sourceInfo_.ecType != HPAE_EC_TYPE_NONE, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM),
            "source not need ec", false),
        "source not need ec");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(CheckSceneTypeNeedEc(sceneType), false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM),
            "scene not need ec", false),
        "scene not need ec");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(SafeGetMap(sceneClusterMap_, sceneType) &&
        sceneClusterMap_[sceneType]->GetCapturerEffectConfig(ecNodeInfo, HPAE_SOURCE_BUFFER_TYPE_EC), false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "capture effect node has no ec config", false),
        "capture effect node has no ec config");
    ecNodeType = sourceInfo_.ecType == HPAE_EC_TYPE_SAME_ADAPTER ? mainMicType_ : HPAE_SOURCE_EC;
    AUDIO_INFO_LOG("resolve connect or disconnect for ecNode type[%{public}u]", ecNodeType);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(SafeGetMap(sourceInputClusterMap_, ecNodeType), false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "ec node is null", false),
        "ec node is null");
    return true;
}

bool HpaeCapturerManager::CheckMicRefCondition(const HpaeProcessorType &sceneType, HpaeNodeInfo &micRefNodeInfo)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sourceInfo_.micRef == HPAE_REF_ON, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM),
            "source not need micref", false),
        "source not need micref");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(CheckSceneTypeNeedMicRef(sceneType), false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_INVALID_PARAM),
            "scene not need micref", false),
        "scene not need micref");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(SafeGetMap(sceneClusterMap_, sceneType) &&
        sceneClusterMap_[sceneType]->GetCapturerEffectConfig(micRefNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MICREF), false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "capture effect node has no micref config", false),
        "capture effect node has no micref config");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_MICREF), false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "micref node is null", false),
        "micref node is null");
    return true;
}

void HpaeCapturerManager::ConnectProcessClusterWithEc(HpaeProcessorType &sceneType)
{
    HpaeNodeInfo ecNodeInfo;
    HpaeSourceInputNodeType ecNodeType;
    CHECK_AND_RETURN_LOG(CheckEcCondition(sceneType, ecNodeInfo, ecNodeType), "connect ec failed");
    sceneClusterMap_[sceneType]->ConnectWithInfo(sourceInputClusterMap_[ecNodeType], ecNodeInfo); // ec
}

void HpaeCapturerManager::ConnectProcessClusterWithMicRef(HpaeProcessorType &sceneType)
{
    HpaeNodeInfo micRefNodeInfo;
    CHECK_AND_RETURN_LOG(CheckMicRefCondition(sceneType, micRefNodeInfo), "connect micref failed");
    sceneClusterMap_[sceneType]->ConnectWithInfo(sourceInputClusterMap_[HPAE_SOURCE_MICREF], micRefNodeInfo); // micref
}

int32_t HpaeCapturerManager::ConnectOutputSession(uint32_t sessionId)
{
    auto sourceOutputNode = SafeGetMap(sourceOutputNodeMap_, sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sourceOutputNode, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD,
                BusinessScenario::CREATE, ERR_INVALID_PARAM),
            "ConnectOutputSession error, sessionId can not find in sourceOutputNodeMap",
            false),
        "ConnectOutputSession error, sessionId can not find in sourceOutputNodeMap, sessionId %{public}u", sessionId);
    
    HpaeProcessorType sceneType = GetEffectiveSceneType(sessionId);
    auto scnenCluster = SafeGetMap(sceneClusterMap_, sceneType);
    if (sceneType != HPAE_SCENE_EFFECT_NONE && scnenCluster != nullptr) {
        HpaeNodeInfo micNodeInfo;
        if (scnenCluster->GetCapturerEffectConfig(micNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MIC)) {
            scnenCluster->ConnectWithInfo(sourceInputClusterMap_[mainMicType_], micNodeInfo); // mic
        }

        if (sceneType == HPAE_SCENE_COLLABORATIVE_RECORD) {
            HpaeNodeInfo collabNodeInfo;
            auto collabInputCluster = SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_DEFAULT);
            if (collabInputCluster && scnenCluster->GetCapturerEffectConfig(collabNodeInfo,
                HPAE_SOURCE_BUFFER_TYPE_DEFAULT)) {
                collabNodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_DEFAULT;
                scnenCluster->ConnectWithInfo(collabInputCluster, collabNodeInfo);
            }
        }
        
        if (scnenCluster->IsEffectNodeValid()) {
            ConnectProcessClusterWithEc(sceneType);
            ConnectProcessClusterWithMicRef(sceneType);
        }
        // 1. Determine if the ResampleNode needs to be created
        // 2. If ResampleNode needs to be created, it should be connected to the UpEffectNode after creation
        // 3. Connect the SourceOutputNode to the ResampleNode
        sourceOutputNode->ConnectWithInfo(scnenCluster,
            sourceOutputNode->GetNodeInfo());
    } else {
        sourceOutputNode->ConnectWithInfo(sourceInputClusterMap_[mainMicType_],
            sourceOutputNode->GetNodeInfo());
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::CapturerSourceStart()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sourceInputClusterMap_[mainMicType_], ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START,
                ERR_NULL_POINTER), "sourceInputClusterMap_ is nullptr", false),
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);
    CHECK_AND_RETURN_RET(sourceInputClusterMap_[mainMicType_]->GetSourceState() != STREAM_MANAGER_RUNNING,
        SUCCESS);
    UpdateAppsUidAndSessionId();
    int32_t ret = sourceInputClusterMap_[mainMicType_]->CapturerSourceStart();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START,
                ERR_OPERATION_FAILED), "capturer source start error", false),
        "capturer source start error, ret = %{public}d.", ret);
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sourceInputClusterMap_[HPAE_SOURCE_EC], ERR_ILLEGAL_STATE,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START,
                    ERR_NULL_POINTER), "sourceInputClusterMap_ EC is nullptr", false),
            "sourceInputClusterMap_[%{public}d] is nullptr", HPAE_SOURCE_EC);
        ret = sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceStart();
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START,
                    ERR_OPERATION_FAILED), "ec capturer source start error", false),
            "ec capturer source start error, ret = %{public}d.", ret);
    }
    if (sourceInfo_.micRef == HPAE_REF_ON) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sourceInputClusterMap_[HPAE_SOURCE_MICREF], ERR_ILLEGAL_STATE,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START,
                    ERR_NULL_POINTER), "sourceInputClusterMap_ MICREF is nullptr", false),
            "sourceInputClusterMap_[%{public}d] is nullptr", HPAE_SOURCE_MICREF);
        ret = sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceStart();
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START,
                    ERR_OPERATION_FAILED), "micref capturer source start error", false),
            "micref capturer source start error, ret = %{public}d.", ret);
    }
    if (!activeCollabSourceTypes_.empty()) {
        auto collabInputCluster = SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_DEFAULT);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(collabInputCluster, ERR_ILLEGAL_STATE,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START,
                    ERR_NULL_POINTER), "sourceInputClusterMap_ DEFAULT is nullptr", false),
            "sourceInputCLusterMap_[%{public}d] is nullptr", HPAE_SOURCE_DEFAULT);
        ret = collabInputCluster->CapturerSourceStart();
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::START,
                    ERR_OPERATION_FAILED), "collaborative capturer source start error", false),
            "collaborative capturer source start error, ret = %{public}d.", ret);
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Start");
        AUDIO_INFO_LOG("Start sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(ConnectOutputSession(sessionId) == SUCCESS, "Connect node error.");
        SetSessionState(sessionId, HPAE_SESSION_RUNNING);
        CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, CAPTURER_RUNNING);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::DisConnectOutputSession(uint32_t sessionId)
{
    HpaeProcessorType sceneType = GetEffectiveSceneType(sessionId);
    if (sceneType != HPAE_SCENE_EFFECT_NONE && SafeGetMap(sceneClusterMap_, sceneType)) {
        // 1. Disconnect SourceOutputNode and ResampleNode
        // 2. Disconnect the ResampleNode and UpEffectNode
        // 3. If the ResampleNode has no output, it needs to be deleted
        sourceOutputNodeMap_[sessionId]->DisConnectWithInfo(
            sceneClusterMap_[sceneType], sourceOutputNodeMap_[sessionId]->GetNodeInfo());
        DisConnectSceneClusterFromSourceInputCluster(sceneType);
    } else if (SafeGetMap(sourceInputClusterMap_, mainMicType_)) {
        AUDIO_INFO_LOG("sceneType[%{public}u] do not exist sceneCluster", sceneType);
        sourceOutputNodeMap_[sessionId]->DisConnectWithInfo(sourceInputClusterMap_[mainMicType_],
            sourceOutputNodeMap_[sessionId]->GetNodeInfo());
    }

    if (sourceInputClusterMap_[mainMicType_]->GetOutputPortNum() == 0) {
        CapturerSourceStop();
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::Pause(uint32_t sessionId, bool isStandby)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Pause");
        AUDIO_INFO_LOG("Pause sessionId %{public}u deviceName %{public}s",
            sessionId, sourceInfo_.deviceName.c_str());
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "Pause not find sessionId %{public}u", sessionId);
        DisConnectOutputSession(sessionId);
        SetSessionState(sessionId, HPAE_SESSION_PAUSED);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
            HPAE_SESSION_PAUSED, OPERATION_PAUSED);
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, CAPTURER_PAUSED);
        
        StopInterphoneSourceIfNeeded();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeCapturerManager::StopInterphoneSourceIfNeeded()
{
    if (sourceInfo_.deviceName.find("interphone") != std::string::npos) {
        AUDIO_INFO_LOG("Interphone capturer paused, stopping source cluster immediately");
        if (SafeGetMap(sourceInputClusterMap_, mainMicType_) && sourceInputClusterMap_[mainMicType_]) {
            sourceInputClusterMap_[mainMicType_]->CapturerSourceStop();
        }
    }
}

int32_t HpaeCapturerManager::Flush(uint32_t sessionId)
{
    if (!IsInit()) {
        AUDIO_ERR_LOG("not init");
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Flush");
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "Flush not find sessionId %{public}u", sessionId);
        // no cache data need to flush
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::Drain(uint32_t sessionId)
{
    if (!IsInit()) {
        AUDIO_ERR_LOG("not init");
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Drain");
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "Drain not find sessionId %{public}u", sessionId);
        // no cache data need to drain
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
            sessionNodeMap_[sessionId].state, OPERATION_DRAINED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeCapturerManager::CapturerSourceStopForRemote()
{
    CHECK_AND_RETURN_LOG(sourceInfo_.deviceClass == "remote", "not remote source");
    CHECK_AND_RETURN_LOG(SafeGetMap(sourceInputClusterMap_, mainMicType_),
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);
    CHECK_AND_RETURN_LOG(sourceInputClusterMap_[mainMicType_]->GetOutputPortNum() == 0, "source has running stream");
    sourceInputClusterMap_[mainMicType_]->CapturerSourceStop();
}

int32_t HpaeCapturerManager::CapturerSourceStop()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(SafeGetMap(sourceInputClusterMap_, mainMicType_), ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::STOP, ERR_NULL_POINTER),
            "sourceInputClusterMap_ is nullptr", false),
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);

    // If remote source has no running stream, stop source
    CapturerSourceStopForRemote();

    CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[mainMicType_]->GetSourceState() != STREAM_MANAGER_SUSPENDED,
        SUCCESS, "capturer source is already stopped");
    sourceInputClusterMap_[mainMicType_]->CapturerSourceStop();

    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_EC)) {
        sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceStop();
    }

    if (sourceInfo_.micRef == HPAE_REF_ON && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_MICREF)) {
        sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceStop();
    }

    if (auto collabInputCluster = SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_DEFAULT)) {
        collabInputCluster->CapturerSourceStop();
    }

    HpaePolicyManager::GetInstance().NotifyAlgoToStopStream();
    return SUCCESS;
}

int32_t HpaeCapturerManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Stop");
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "Stop not find sessionId %{public}u", sessionId);
        DisConnectOutputSession(sessionId);
        SetSessionState(sessionId, HPAE_SESSION_STOPPED);
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, CAPTURER_STOPPED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::Release(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Release");
    return DestroyStream(sessionId);
}

int32_t HpaeCapturerManager::SetStreamMute(uint32_t sessionId, bool isMute)
{
    auto request = [this, sessionId, isMute]() {
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "Mute not find sessionId %{public}u", sessionId);
        sourceOutputNodeMap_[sessionId]->SetMute(isMute);
        std::shared_ptr<HpaeSourceOutputNode> sourceNode = sourceOutputNodeMap_[sessionId];
        NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, CAPTURER_RUNNING,
            sourceNode->GetAppUid(), isMute);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::SetMute(bool isMute)
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

void HpaeCapturerManager::Process()
{
    Trace trace("HpaeCapturerManager::Process");
    if (IsRunning()) {
        UpdateAppsUidAndSessionId();
        if (appsUid_.empty()) {
            CapturerSourceStop();
            return;
        }
        for (const auto &sourceOutputNodePair : sourceOutputNodeMap_) {
            if (sourceOutputNodePair.second->GetState() == HPAE_SESSION_RUNNING) {
                sourceOutputNodePair.second->DoProcess();
            }
        }
    }
}

void HpaeCapturerManager::UpdateAppsUidAndSessionId()
{
    appsUid_.clear();
    sessionsId_.clear();
    for (const auto &sourceOutputNodePair : sourceOutputNodeMap_) {
        if (sourceOutputNodePair.second->GetState() == HPAE_SESSION_RUNNING) {
            appsUid_.emplace_back(sourceOutputNodePair.second->GetAppUid());
            sessionsId_.emplace_back(static_cast<int32_t>(sourceOutputNodePair.first));
        }
    }
    if (SafeGetMap(sourceInputClusterMap_, mainMicType_) && sourceInputClusterMap_[mainMicType_]) {
        sourceInputClusterMap_[mainMicType_]->UpdateAppsUidAndSessionId(appsUid_, sessionsId_);
    }
}
void HpaeCapturerManager::NotifyStreamChangeToSource(
    StreamChangeType change, uint32_t sessionId, CapturerState state, uint32_t appUid, bool mute, bool mixWithWakeUp)
{
    SourceType source = SOURCE_TYPE_INVALID;
    if (sourceOutputNodeMap_.find(sessionId) != sourceOutputNodeMap_.end()) {
        source = sourceOutputNodeMap_[sessionId]->GetSourceType();
    }
    if (sessionNodeMap_.find(sessionId) != sessionNodeMap_.end()) {
        mixWithWakeUp = sessionNodeMap_[sessionId].mixWithWakeUp;
    }
    if (SafeGetMap(sourceInputClusterMap_, mainMicType_) && sourceInputClusterMap_[mainMicType_]) {
        sourceInputClusterMap_[mainMicType_]->NotifyStreamChangeToSource(change, sessionId, source, state,
            appUid, mute, mixWithWakeUp);
    }
}

void HpaeCapturerManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

int32_t HpaeCapturerManager::PrepareCapturerEc(HpaeNodeInfo &ecNodeInfo)
{
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER) {
        ecNodeInfo.frameLen = sourceInfo_.ecFrameLen;
        ecNodeInfo.channels = sourceInfo_.ecChannels;
        ecNodeInfo.format = sourceInfo_.ecFormat;
        ecNodeInfo.samplingRate = sourceInfo_.ecSamplingRate;
        ecNodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
        ecNodeInfo.sourceInputNodeType = HPAE_SOURCE_EC;
        ecNodeInfo.statusCallback = weak_from_this();
        sourceInputClusterMap_[HPAE_SOURCE_EC] = std::make_shared<HpaeSourceInputCluster>(ecNodeInfo);
        int32_t ret = sourceInputClusterMap_[HPAE_SOURCE_EC]->GetCapturerSourceInstance(
            DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_NETWORKID, SOURCE_TYPE_INVALID, HDI_ID_INFO_EC);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("get ec capturer soruce instance error, ret = %{public}d.", ret);
            sourceInputClusterMap_.erase(HPAE_SOURCE_EC);
        }
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::PrepareCapturerMicRef(HpaeNodeInfo &micRefNodeInfo)
{
    if (sourceInfo_.micRef == HPAE_REF_ON) {
        micRefNodeInfo.frameLen = sourceInfo_.micRefFrameLen;
        micRefNodeInfo.channels = sourceInfo_.micRefChannels;
        micRefNodeInfo.format = sourceInfo_.micRefFormat;
        micRefNodeInfo.samplingRate = sourceInfo_.micRefSamplingRate;
        micRefNodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MICREF;
        micRefNodeInfo.sourceInputNodeType = HPAE_SOURCE_MICREF;
        micRefNodeInfo.statusCallback = weak_from_this();
        sourceInputClusterMap_[HPAE_SOURCE_MICREF] = std::make_shared<HpaeSourceInputCluster>(micRefNodeInfo);
        int32_t ret = sourceInputClusterMap_[HPAE_SOURCE_MICREF]->GetCapturerSourceInstance(
            DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_NETWORKID, SOURCE_TYPE_INVALID, HDI_ID_INFO_MIC_REF);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("get micRef capturer soruce instance error, ret = %{public}d.", ret);
            sourceInputClusterMap_.erase(HPAE_SOURCE_MICREF);
        }
    }
    return SUCCESS;
}

void HpaeCapturerManager::CreateSourceAttr(IAudioSourceAttr &attr)
{
    attr.adapterName = sourceInfo_.adapterName;
    attr.sampleRate = sourceInfo_.samplingRate;
    attr.channel = sourceInfo_.channels;
    attr.format = sourceInfo_.format;
    attr.channelLayout = sourceInfo_.channelLayout;
    attr.deviceType = sourceInfo_.deviceType;
    attr.volume = sourceInfo_.volume;
    attr.deviceNetworkId = sourceInfo_.deviceNetId.c_str();
    attr.filePath = sourceInfo_.filePath.c_str();
    attr.isBigEndian = false;
    attr.sourceType = static_cast<int32_t>(sourceInfo_.sourceType);
    attr.openMicSpeaker = sourceInfo_.openMicSpeaker;
    attr.hasEcConfig = mainMicType_ == HPAE_SOURCE_MIC_EC;
    attr.macAddress = sourceInfo_.macAddress;
    return;
}

void HpaeCapturerManager::UpdateSourceAttr(IAudioSourceAttr &attr)
{
    if (attr.hasEcConfig) {
        attr.formatEc = sourceInfo_.ecFormat;
        attr.sampleRateEc = sourceInfo_.ecSamplingRate;
        attr.channelEc = sourceInfo_.ecChannels;
    }
    if (sourceInfo_.sourceType == SOURCE_TYPE_VOICE_RECOGNITION ||
        sourceInfo_.sourceType == SOURCE_TYPE_CAMCORDER) {
        attr.formatMicIn = sourceInfo_.micInFormat;
        attr.sampleRateMicIn = sourceInfo_.micInSamplingRate;
        attr.channelMicIn = sourceInfo_.micInChannels;
    }
}

int32_t HpaeCapturerManager::InitCapturer()
{
    IAudioSourceAttr attr;
    CreateSourceAttr(attr);
    UpdateSourceAttr(attr);
    
    int32_t ret = sourceInputClusterMap_[mainMicType_]->CapturerSourceInit(attr);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_OPERATION,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD,
                BusinessScenario::CREATE, ERR_OPERATION_FAILED),
            "init mic source input node err", false),
        "init mic source input node err, , ret = %{public}d.\n", ret);
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_EC)) {
        IAudioSourceAttr attrEc;
        attrEc.sourceType = SOURCE_TYPE_EC;
        attrEc.adapterName = sourceInfo_.ecAdapterName;
        attrEc.deviceType = DEVICE_TYPE_MIC;
        attrEc.sampleRate = sourceInfo_.ecSamplingRate;
        attrEc.channel = sourceInfo_.ecChannels;
        attrEc.format = sourceInfo_.ecFormat;
        attrEc.isBigEndian = false;
        attrEc.openMicSpeaker = sourceInfo_.openMicSpeaker;
        ret = sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceInit(attrEc);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("init ec source input node err, ret = %{public}d.", ret);
            sourceInputClusterMap_.erase(HPAE_SOURCE_EC);
        }
    }
    if (sourceInfo_.micRef == HPAE_REF_ON && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_MICREF)) {
        IAudioSourceAttr attrMicRef;
        attrMicRef.sourceType = SOURCE_TYPE_MIC_REF;
        attrMicRef.adapterName = "primary";
        attrMicRef.deviceType = DEVICE_TYPE_MIC;
        attrMicRef.sampleRate = sourceInfo_.micRefSamplingRate;
        attrMicRef.channel = sourceInfo_.micRefChannels;
        attrMicRef.format = sourceInfo_.micRefFormat;
        attrMicRef.isBigEndian = false;
        attrMicRef.openMicSpeaker = sourceInfo_.openMicSpeaker;
        ret = sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceInit(attrMicRef);
        if (ret != SUCCESS) {
            AUDIO_ERR_LOG("init micRef source input node err, ret = %{public}d.", ret);
            sourceInputClusterMap_.erase(HPAE_SOURCE_MICREF);
        }
    }
    return SUCCESS;
}

void HpaeCapturerManager::StopOuputNode()
{
    CHECK_AND_RETURN_LOG(SafeGetMap(sourceInputClusterMap_, mainMicType_),
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);
    CapturerSourceStop();
    sourceInputClusterMap_[mainMicType_]->CapturerSourceDeInit();
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_EC)) {
        sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceDeInit();
    }
    if (sourceInfo_.micRef == HPAE_REF_ON && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_MICREF)) {
        sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceDeInit();
    }
    if (auto collabInputCluster = SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_DEFAULT)) {
        collabInputCluster->CapturerSourceDeInit();
    }
    sourceInputClusterMap_.clear();
}

int32_t HpaeCapturerManager::ReloadCaptureManager(const HpaeSourceInfo &sourceInfo, bool isReload)
{
    if (!IsInit()) {
        hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    }
    auto request = [this, sourceInfo, isReload] {
        AUDIO_INFO_LOG("reload capture");
        StopOuputNode();
        // disconnect
        std::vector<HpaeCaptureMoveInfo> moveInfos;
        for (const auto &it : sourceOutputNodeMap_) {
            HpaeCaptureMoveInfo moveInfo;
            moveInfo.sessionId = it.first;
            moveInfo.sourceOutputNode = it.second;
            if (sessionNodeMap_.find(it.first) != sessionNodeMap_.end()) {
                moveInfo.sessionInfo = sessionNodeMap_[it.first];
                moveInfos.emplace_back(moveInfo);
            }
        }
        for (const auto &it : moveInfos) {
            DeleteOutputSession(it.sessionId, true);
        }
        sourceInfo_ = sourceInfo;
        int32_t ret = InitCapturerManager();
        if (ret != SUCCESS) {
            AUDIO_INFO_LOG("re-Init failed");
            TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sourceInfo_.deviceName, ret);
            return;
        }
        AUDIO_INFO_LOG("re-Init success");
        HpaePolicyManager::GetInstance().SetInputDevice(captureId_, static_cast<DeviceType>(sourceInfo_.deviceType));
        // rebuild collaborative source if needed
        if (!activeCollabSourceTypes_.empty()) {
            int32_t ret = OpenCollaborativePrimary();
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("rebuild collaborative source failed");
                activeCollabSourceTypes_.clear();
            }
        }
        // connect
        for (const auto &moveInfo : moveInfos) {
            AddSingleNodeToSource(moveInfo, true);
        }
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sourceInfo_.deviceName, ret);
        TriggerCallback(INIT_SOURCE_RESULT, sourceInfo_.sourceType);
    };
    SendRequest(request, __func__, true);
    if (!IsInit()) {
        hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    }
    return SUCCESS;
}

void HpaeCapturerManager::UpdateSourceInputNodeInfoForRecognition(HpaeNodeInfo &nodeInfo)
{
    CHECK_AND_RETURN(nodeInfo.sourceType == SOURCE_TYPE_VOICE_RECOGNITION ||
        nodeInfo.sourceType == SOURCE_TYPE_CAMCORDER);
    // for recognition, mic = preprocess(1ch) + micin(4ch)
    uint8_t totalChannels = static_cast<uint8_t>(sourceInfo_.micInChannels) +
        static_cast<uint8_t>(sourceInfo_.channels);
    CHECK_AND_RETURN_LOG(totalChannels <= CHANNEL_16,
        "totalChannels[%{public}u] is not supported", totalChannels);
    nodeInfo.channels = static_cast<AudioChannel>(totalChannels);
}

int32_t HpaeCapturerManager::InitCapturerManager()
{
    AUDIO_INFO_LOG("deviceName:%{public}s,channel:%{public}u,rate:%{public}u", sourceInfo_.sourceName.c_str(),
        sourceInfo_.channels, sourceInfo_.samplingRate);
    HpaeNodeInfo nodeInfo;
    HpaeNodeInfo ecNodeInfo;
    HpaeNodeInfo micRefNodeInfo;
    int32_t checkRet = CheckSourceInfoFramelen(sourceInfo_);
    CHECK_AND_RETURN_RET(checkRet == SUCCESS, checkRet);
    nodeInfo.deviceClass = sourceInfo_.deviceClass;
    nodeInfo.channels = sourceInfo_.channels;
    nodeInfo.format = sourceInfo_.format;
    nodeInfo.frameLen = sourceInfo_.frameLen;
    nodeInfo.samplingRate = sourceInfo_.samplingRate;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.sourceType = sourceInfo_.sourceType;
    nodeInfo.routeFlag = sourceInfo_.routeFlag;
    mainMicType_ = sourceInfo_.ecType == HPAE_EC_TYPE_SAME_ADAPTER ? HPAE_SOURCE_MIC_EC : HPAE_SOURCE_MIC;
    
    UpdateSourceInputNodeInfoForRecognition(nodeInfo);

    if (mainMicType_ == HPAE_SOURCE_MIC_EC) {
        ecNodeInfo.channels = sourceInfo_.ecChannels;
        ecNodeInfo.format = sourceInfo_.ecFormat;
        ecNodeInfo.samplingRate = sourceInfo_.ecSamplingRate;
        ecNodeInfo.frameLen = sourceInfo_.ecFrameLen;
        ecNodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
        ecNodeInfo.statusCallback = weak_from_this();
        ecNodeInfo.sourceType = sourceInfo_.sourceType;
        nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC_EC;
        std::vector<HpaeNodeInfo> nodeInfos = {nodeInfo, ecNodeInfo};
        sourceInputClusterMap_[mainMicType_] = std::make_shared<HpaeSourceInputCluster>(nodeInfos);
    } else {
        nodeInfo.sourceInputNodeType = HPAE_SOURCE_MIC;
        sourceInputClusterMap_[mainMicType_] = std::make_shared<HpaeSourceInputCluster>(nodeInfo);
    }

    sourceInputClusterMap_[mainMicType_]->SetSourceInputNodeType(mainMicType_);  // to do rewrite, optimise
    if (sourceInfo_.sourceType == SOURCE_TYPE_OFFLOAD_CAPTURE) {
        sourceInputClusterMap_[mainMicType_]->SetSourceInputNodeType(HPAE_SOURCE_OFFLOAD);
    }
    int32_t ret = sourceInputClusterMap_[mainMicType_]->GetCapturerSourceInstance(
        sourceInfo_.deviceClass, sourceInfo_.deviceNetId, sourceInfo_.sourceType,
        sourceInfo_.sourceName, sourceInfo_.busAddress);
    captureId_ = sourceInputClusterMap_[mainMicType_]->GetCaptureId();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                ERR_OPERATION_FAILED), "get mic capturer source instance error", false),
        "get mic capturer soruce instance error, ret = %{public}d.\n", ret);
    PrepareCapturerEc(ecNodeInfo);
    PrepareCapturerMicRef(micRefNodeInfo);
    ret = InitCapturer();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                ERR_OPERATION_FAILED), "init main capturer error", false),
        "init main capturer error, ret %{public}d", ret);
    isInit_.store(true);
    return SUCCESS;
}

int32_t HpaeCapturerManager::Init(bool isReload)
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, isReload] {
        int32_t ret = InitCapturerManager();
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sourceInfo_.deviceName, ret);
        CHECK_AND_RETURN_LOG(ret == SUCCESS, "Init failed");
        TriggerCallback(INIT_SOURCE_RESULT, sourceInfo_.sourceType);
        AUDIO_INFO_LOG("Init success");
        CheckIfAnyStreamRunning();
        HpaePolicyManager::GetInstance().SetInputDevice(captureId_,
            static_cast<DeviceType>(sourceInfo_.deviceType));
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeCapturerManager::DeInit(bool isMoveDefault)
{
    AUDIO_INFO_LOG("device:%{public}s", sourceInfo_.deviceName.c_str());
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    for (auto outputNode : sourceOutputNodeMap_) {
        outputNode.second->ResetAll();
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(SafeGetMap(sourceInputClusterMap_, mainMicType_), ERR_INVALID_OPERATION,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::RELEASE, ERR_NULL_POINTER),
            "sourceInputClusterMap_ is nullptr", false),
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);
    CapturerSourceStop();
    sourceInputClusterMap_[mainMicType_]->CapturerSourceDeInit();
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_EC)) {
        sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceDeInit();
    }
    if (sourceInfo_.micRef == HPAE_REF_ON && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_MICREF)) {
        sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceDeInit();
    }

    if (auto collabInputCluster = SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_DEFAULT)) {
        collabInputCluster->CapturerSourceDeInit();
    }
    isInit_.store(false);
    
    if (isMoveDefault) {
        std::string name = "";
        std::vector<uint32_t> ids;
        AUDIO_INFO_LOG("move all source to default source");
        MoveAllStreamToNewSource(name, ids, MOVE_ALL);
    }
    sourceInputClusterMap_.clear();
    return SUCCESS;
}

bool HpaeCapturerManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

int32_t HpaeCapturerManager::RegisterReadCallback(uint32_t sessionId,
    const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        if (SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            sourceOutputNodeMap_[sessionId]->RegisterReadCallback(callback);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::GetSourceOutputInfo(uint32_t sessionId, HpaeSourceOutputInfo &sourceOutputInfo)
{
    if (!SafeGetMap(sourceOutputNodeMap_, sessionId)) {
        return ERR_INVALID_OPERATION;
    }
    sourceOutputInfo.nodeInfo = sourceOutputNodeMap_[sessionId]->GetNodeInfo();
    sourceOutputInfo.capturerSessionInfo = sessionNodeMap_[sessionId];
    return SUCCESS;
}

HpaeSourceInfo HpaeCapturerManager::GetSourceInfo()
{
    return sourceInfo_;
}

std::vector<SourceOutput> HpaeCapturerManager::GetAllSourceOutputsInfo()
{
    return {};
}

bool HpaeCapturerManager::IsInit()
{
    return isInit_.load();
}

bool HpaeCapturerManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

bool HpaeCapturerManager::IsRunning(void)
{
    if (SafeGetMap(sourceInputClusterMap_, mainMicType_) &&
        hpaeSignalProcessThread_ != nullptr) {
        return sourceInputClusterMap_[mainMicType_]->GetSourceState() == STREAM_MANAGER_RUNNING &&
            hpaeSignalProcessThread_->IsRunning();
    } else {
        return false;
    }
}

void HpaeCapturerManager::SendRequest(Request &&request, const std::string &funcName, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_INFO_LOG("not init, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_CAPTURE_MANAGER_TYPE, funcName,
            "HpaeCapturerManager not init");
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ capturer is nullptr");
    if (hpaeSignalProcessThread_ == nullptr) {
        AUDIO_INFO_LOG("hpaeSignalProcessThread_ capturer is nullptr, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_CAPTURE_MANAGER_TYPE, funcName, "thread is nullptr");
        return;
    }
    hpaeSignalProcessThread_->Notify();
}

void HpaeCapturerManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
        sessionNodeMap_[sessionId].state, operation);
}

int32_t HpaeCapturerManager::AddAllNodesToSource(const std::vector<HpaeCaptureMoveInfo> &moveInfos, bool isConnect)
{
    auto request = [this, moveInfos, isConnect]() {
        for (const auto &moveInfo : moveInfos) {
            AddSingleNodeToSource(moveInfo, isConnect);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::AddNodeToSource(const HpaeCaptureMoveInfo &moveInfo)
{
    auto request = [this, moveInfo]() { AddSingleNodeToSource(moveInfo); };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeCapturerManager::AddSingleNodeToSource(const HpaeCaptureMoveInfo &moveInfo, bool isConnect)
{
    uint32_t sessionId = moveInfo.sessionId;
    HILOG_COMM_INFO("[FinishMove] session :%{public}u to source:[%{public}s]",
        sessionId, sourceInfo_.sourceName.c_str());
    CHECK_AND_RETURN_LOG(moveInfo.sourceOutputNode != nullptr, "move fail, sourceoutputnode is null");
    HpaeNodeInfo nodeInfo = moveInfo.sourceOutputNode->GetNodeInfo();
    sourceOutputNodeMap_[sessionId] = moveInfo.sourceOutputNode;
    sessionNodeMap_[sessionId] = moveInfo.sessionInfo;
    sessionNodeMap_[sessionId].sourceType = nodeInfo.sourceType;
    TriggerCallback(UPDATE_SPAN_SIZE, sessionId,
        static_cast<uint32_t>(nodeInfo.frameLen), HPAE_STREAM_CLASS_TYPE_RECORD);
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(true, nodeInfo);
#endif
    HpaeProcessorType sceneType = GetEffectiveSceneType(sessionId);
    AudioEnhanceScene enhanceScene = TransProcessType2EnhanceScene(sceneType);
    CreateSceneCluster(sceneType, enhanceScene);

    if (moveInfo.sessionInfo.state == HPAE_SESSION_RUNNING) {
        ConnectOutputSession(sessionId);
        CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
    }
    NotifyStreamChangeToSource(STREAM_CHANGE_TYPE_ADD, sessionId,
        ConvertHpaeToCapturerState(moveInfo.sessionInfo.state), sourceOutputNodeMap_[sessionId]->GetAppUid(),
        sourceOutputNodeMap_[sessionId]->GetMute());
}

int32_t HpaeCapturerManager::MoveAllStream(const std::string &sourceName, const std::vector<uint32_t>& sessionIds,
    MoveSessionType moveType)
{
    if (!IsInit()) {
        AUDIO_INFO_LOG("source is not init ,use sync mode move to: %{public}s", sourceName.c_str());
        MoveAllStreamToNewSource(sourceName, sessionIds, moveType);
    } else {
        AUDIO_INFO_LOG("source is init ,use async mode move to: %{public}s", sourceName.c_str());
        auto request = [this, sourceName, sessionIds, moveType]() {
            MoveAllStreamToNewSource(sourceName, sessionIds, moveType);
        };
        SendRequest(request, __func__);
    }
    return SUCCESS;
}

void HpaeCapturerManager::MoveAllStreamToNewSource(const std::string &sourceName,
    const std::vector<uint32_t>& moveIds, MoveSessionType moveType)
{
    std::string name = sourceName;
    std::vector<HpaeCaptureMoveInfo> moveInfos;
    std::string idStr;
    for (const auto &it : sourceOutputNodeMap_) {
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), it.first) != moveIds.end()) {
            HpaeCaptureMoveInfo moveInfo;
            moveInfo.sessionId = it.first;
            moveInfo.sourceOutputNode = it.second;
            idStr.append("[").append(std::to_string(it.first)).append("],");
            if (sessionNodeMap_.find(it.first) != sessionNodeMap_.end()) {
                moveInfo.sessionInfo = sessionNodeMap_[it.first];
                moveInfos.emplace_back(moveInfo);
            }
        }
    }

    for (const auto &it : moveInfos) {
        DeleteOutputSession(it.sessionId);
    }
    HILOG_COMM_INFO("[StartMove] session:%{public}s to source name:%{public}s, move type:%{public}d",
        idStr.c_str(), name.c_str(), moveType);
    if (moveType == MOVE_ALL) {
        TriggerSyncCallback(MOVE_ALL_SOURCE_OUTPUT, moveInfos, name, moveType);
    } else {
        TriggerCallback(MOVE_ALL_SOURCE_OUTPUT, moveInfos, name, moveType);
    }
}

int32_t HpaeCapturerManager::MoveStream(uint32_t sessionId, const std::string& sourceName)
{
    auto request = [this, sessionId, sourceName]() {
        if (!SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,not find session,move %{public}s --> %{public}s",
                sessionId, sourceInfo_.sourceName.c_str(), sourceName.c_str());
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId, MOVE_SINGLE, sourceName);
            HpaeStreamMoveMonitor::ReportStreamMoveException(0, sessionId, HPAE_STREAM_CLASS_TYPE_RECORD,
                sourceInfo_.sourceName, sourceName, "not find session");
            return;
        }
        std::shared_ptr<HpaeSourceOutputNode> sourceNode = sourceOutputNodeMap_[sessionId];
        if (sessionNodeMap_.find(sessionId)==sessionNodeMap_.end()) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,not find session node,move %{public}s --> %{public}s",
                sessionId, sourceInfo_.sourceName.c_str(), sourceName.c_str());
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId, MOVE_SINGLE, sourceName);
            HpaeStreamMoveMonitor::ReportStreamMoveException(sourceNode->GetAppUid(), sessionId,
                HPAE_STREAM_CLASS_TYPE_RECORD, sourceInfo_.sourceName, sourceName, "not find session node");
            return;
        }
        CHECK_AND_RETURN_LOG(!sourceName.empty(), "[StartMove] session:%{public}u failed,sourceName is empty",
            sessionId);
        AUDIO_INFO_LOG("[StartMove] session: %{public}u, source [%{public}s] --> [%{public}s]",
            sessionId, sourceInfo_.sourceName.c_str(), sourceName.c_str());
        HpaeCapturerSessionInfo sessionInfo = sessionNodeMap_[sessionId];
        HpaeCaptureMoveInfo moveInfo;
        moveInfo.sessionId = sessionId;
        moveInfo.sourceOutputNode = sourceNode;
        moveInfo.sessionInfo = sessionInfo;
        DeleteOutputSession(sessionId);
        std::string name = sourceName;
        TriggerCallback(MOVE_SOURCE_OUTPUT, moveInfo, name);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeCapturerManager::OnNotifyQueue()
{
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ is nullptr");
    hpaeSignalProcessThread_->Notify();
}

void HpaeCapturerManager::OnRequestLatency(uint32_t sessionId, uint64_t &latency)
{
    // todo: add processLatency
    latency = 0;
    return;
}

std::string HpaeCapturerManager::GetThreadName()
{
    return sourceInfo_.deviceName;
}

int32_t HpaeCapturerManager::DumpSourceInfo()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "not init");
    SendRequest([this]() {
        AUDIO_INFO_LOG("DumpSourceInfo deviceName %{public}s", sourceInfo_.deviceName.c_str());
        UploadDumpSourceInfo(sourceInfo_.deviceName);
        }, __func__);
    return SUCCESS;
}

void HpaeCapturerManager::CheckIfAnyStreamRunning()
{
    CHECK_AND_RETURN_LOG(!sessionNodeMap_.empty(), "no stream need start");
    for (auto &sessionPair :sessionNodeMap_) {
        if (sessionPair.second.state == HPAE_SESSION_RUNNING) {
            ConnectOutputSession(sessionPair.first);
            CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
        }
    }
}

std::string HpaeCapturerManager::GetDeviceHDFDumpInfo()
{
    std::string config;
    TransDeviceInfoToString(sourceInfo_, config);
    return config;
}

int32_t HpaeCapturerManager::StopManager()
{
    auto request = [this] {
        CapturerSourceStop();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::AddCaptureInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode,
    const SourceType &sourceType)
{
    auto request = [this, sinkOutputNode, sourceType] {
        Trace trace("HpaeCapturerManager::AddCaptureInjector");
        AUDIO_INFO_LOG("add capture injector");
        HpaeProcessorType sceneType = TransSourceTypeToSceneType(sourceType);
        auto sceneCluster = SafeGetMap(sceneClusterMap_, sceneType);
        CHECK_AND_RETURN_LOG(sceneCluster != nullptr, "sourceType[%{public}d] cluster not exit", sourceType);
        sceneCluster->ConnectInjector(sinkOutputNode);
        auto inputCluster = SafeGetMap(sourceInputClusterMap_, mainMicType_);
        CHECK_AND_RETURN_LOG(inputCluster != nullptr, "mainMic is nullptr, set inject state failed");
        inputCluster->SetInjectState(true);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::RemoveCaptureInjector(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &sinkOutputNode,
    const SourceType &sourceType)
{
    auto request = [this, sinkOutputNode, sourceType] {
        Trace trace("HpaeCapturerManager::RemoveCaptureInjector");
        AUDIO_INFO_LOG("remove capture injector");
        HpaeProcessorType sceneType = TransSourceTypeToSceneType(sourceType);
        auto sceneCluster = SafeGetMap(sceneClusterMap_, sceneType);
        CHECK_AND_RETURN_LOG(sceneCluster != nullptr, "sourceType[%{public}d] cluster not exit", sourceType);
        sceneCluster->DisConnectInjector(sinkOutputNode);
        auto inputCluster = SafeGetMap(sourceInputClusterMap_, mainMicType_);
        CHECK_AND_RETURN_LOG(inputCluster != nullptr, "mainMic is nullptr, remove inject state failed");
        inputCluster->SetInjectState(false);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeCapturerManager::TriggerAppsUidUpdate(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        AUDIO_INFO_LOG("deviceClass: %{public}s", sourceInfo_.deviceClass.c_str());
        CHECK_AND_RETURN(sourceInfo_.deviceClass == REMOTE_DEVICE_CLASS);
        appsUid_.clear();
        sessionsId_.clear();
        for (const auto &sourceOutputNodePair : sourceOutputNodeMap_) {
            if (sourceOutputNodePair.second->GetState() == HPAE_SESSION_RUNNING ||
                sourceOutputNodePair.first == sessionId) {
                appsUid_.emplace_back(sourceOutputNodePair.second->GetAppUid());
                sessionsId_.emplace_back(static_cast<int32_t>(sourceOutputNodePair.first));
            }
        }
        if (SafeGetMap(sourceInputClusterMap_, mainMicType_) && sourceInputClusterMap_[mainMicType_]) {
            sourceInputClusterMap_[mainMicType_]->UpdateAppsUidAndSessionId(appsUid_, sessionsId_);
        }
    };
    SendRequest(request, __func__);
}

int32_t HpaeCapturerManager::SetAppsEnhanceMuteState(const uint32_t &sessionId, bool isMute)
{
    auto request = [this, sessionId, isMute] () {
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "can not find sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(sessionNodeMap_.find(sessionId) != sessionNodeMap_.end(),
            "can not find session [%{public}u] info", sessionId);
        HpaeProcessorType sceneType = GetEffectiveSceneType(sessionId);
        auto sceneCluster = SafeGetMap(sceneClusterMap_, sceneType);
        CHECK_AND_RETURN_LOG(sceneCluster != nullptr, "cluster is nullptr");
        sceneCluster->SetAppsEnhanceMuteState(isMute);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeCapturerManager::EnableCaptureCollaboration(SourceType sourceType)
{
    auto request = [this, sourceType] () {
        CHECK_AND_RETURN_LOG(sourceInfo_.deviceClass != DEFAULT_DEVICE_CLASS, "Collaboration is not support");
        CHECK_AND_RETURN_LOG(IsCollabRecordSupported(sourceType), "sourceType not supported for collaboration");
        CHECK_AND_RETURN_LOG(activeCollabSourceTypes_.count(sourceType) == 0,
            "Collaboration sourceType[%{public}d] already enabled", sourceType);
        if (activeCollabSourceTypes_.empty()) {
            int32_t ret = OpenCollaborativePrimary();
            CHECK_AND_RETURN_LOG(ret == SUCCESS, "OpenCollaborativePrimary fail, abort collaboration");
        }
        OpenCollaboration(sourceType);
    };
    SendRequest(request, __func__);
}

void HpaeCapturerManager::DisableCaptureCollaboration(SourceType sourceType)
{
    auto request = [this, sourceType] () {
        CHECK_AND_RETURN_LOG(sourceInfo_.deviceClass != DEFAULT_DEVICE_CLASS, "Collaboration is not support");
        CHECK_AND_RETURN_LOG(activeCollabSourceTypes_.count(sourceType) > 0,
            "Collaboration sourceType[%{public}d] not enabled", sourceType);
        CloseCollaboration(sourceType);
        if (activeCollabSourceTypes_.empty()) {
            CloseCollaborativePrimary();
        }
    };
    SendRequest(request, __func__);
}

void HpaeCapturerManager::GetCollaborativeSourceInfo(IAudioSourceAttr &attr, HpaeNodeInfo &nodeInfo)
{
    attr.adapterName = DEFAULT_DEVICE_CLASS;
    attr.sampleRate = SAMPLE_RATE_48000;
    attr.channel = STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = 4lu;
    attr.deviceType = DEVICE_TYPE_MIC;
    attr.volume = 1.f;
    attr.deviceNetworkId = "LocalDevice";
    attr.filePath = "";
    attr.isBigEndian = false;
    attr.sourceType = SOURCE_TYPE_UNPROCESSED;
    attr.hasEcConfig = false;
    attr.openMicSpeaker = 1;
    attr.macAddress = "";

    nodeInfo.deviceClass = DEFAULT_DEVICE_CLASS;
    nodeInfo.channels = static_cast<AudioChannel>(attr.channel);
    nodeInfo.format = attr.format;
    nodeInfo.samplingRate = static_cast<AudioSamplingRate>(attr.sampleRate);
    nodeInfo.frameLen = CalculateFrameLenBySampleRate(nodeInfo.samplingRate);
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_DEFAULT;
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.sourceType = static_cast<SourceType>(attr.sourceType);
    nodeInfo.sourceInputNodeType = HPAE_SOURCE_DEFAULT;
}

int32_t HpaeCapturerManager::OpenCollaborativePrimary()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(IsRunning(), ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD,
                BusinessScenario::START, ERR_ILLEGAL_STATE),
            "source not running", false),
        "source not running");
    IAudioSourceAttr primaryAttr;
    HpaeNodeInfo primaryNodeInfo;
    std::string primarySourceName = PRIMARY_MIC;
    GetCollaborativeSourceInfo(primaryAttr, primaryNodeInfo);
    std::shared_ptr<HpaeSourceInputCluster> collabInput = std::make_shared<HpaeSourceInputCluster>(primaryNodeInfo);

    int32_t ret = collabInput->GetCapturerSourceInstance(primaryNodeInfo.deviceClass, primaryAttr.deviceNetworkId,
        static_cast<SourceType>(primaryAttr.sourceType), primarySourceName);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD,
                BusinessScenario::CREATE, ERR_OPERATION_FAILED),
            "get collaborative source instance fail", false),
        "get collaborative source instance fail, ret %{public}d", ret);
    ret = collabInput->CapturerSourceInit(primaryAttr);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD,
                BusinessScenario::CREATE, ERR_OPERATION_FAILED),
            "collaborative source init fail", false),
        "collaborative source init fail, ret %{public}d", ret);
    sourceInputClusterMap_[primaryNodeInfo.sourceInputNodeType] = collabInput;
    AUDIO_INFO_LOG("collaborative source init success");
    return SUCCESS;
}

void HpaeCapturerManager::CloseCollaborativePrimary()
{
    auto collabInput = SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_DEFAULT);
    CHECK_AND_RETURN_LOG(collabInput != nullptr, "collaborative sourceInput is nullptr");
    collabInput->CapturerSourceDeInit();
    sourceInputClusterMap_.erase(HPAE_SOURCE_DEFAULT);
}

void HpaeCapturerManager::OpenCollaboration(SourceType sourceType)
{
    for (auto session : sourceOutputNodeMap_) {
        DisConnectOutputSession(session.first);
    }

    activeCollabSourceTypes_.insert(sourceType);
    CreateSceneCluster(HPAE_SCENE_COLLABORATIVE_RECORD,
        TransProcessType2EnhanceScene(HPAE_SCENE_COLLABORATIVE_RECORD));

    CheckIfAnyStreamRunning();

    AUDIO_INFO_LOG("sourceType[%{public}d] open collaboration", sourceType);
}

void HpaeCapturerManager::CloseCollaboration(SourceType sourceType)
{
    for (auto session : sourceOutputNodeMap_) {
        DisConnectOutputSession(session.first);
    }

    activeCollabSourceTypes_.erase(sourceType);

    HpaeProcessorType normalSceneType = TransSourceTypeToSceneType(sourceType);
    CreateSceneCluster(normalSceneType, TransProcessType2EnhanceScene(normalSceneType));

    if (activeCollabSourceTypes_.empty() && SafeGetMap(sceneClusterMap_, HPAE_SCENE_COLLABORATIVE_RECORD)) {
        CaptureEffectRelease(HPAE_SCENE_COLLABORATIVE_RECORD);
        sceneClusterMap_.erase(HPAE_SCENE_COLLABORATIVE_RECORD);
    }

    CheckIfAnyStreamRunning();

    AUDIO_INFO_LOG("sourceType[%{public}d] close collaboration", sourceType);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
