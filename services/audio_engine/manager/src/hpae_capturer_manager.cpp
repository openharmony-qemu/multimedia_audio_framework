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

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const std::string DEFAULT_DEVICE_CLASS = "primary";
const std::string DEFAULT_DEVICE_NETWORKID = "LocalDevice";

HpaeCapturerManager::HpaeCapturerManager(HpaeSourceInfo &sourceInfo)
    : hpaeNoLockQueue_(CURRENT_REQUEST_COUNT), sourceInfo_(sourceInfo)
{
    AUDIO_INFO_LOG("Source info: mic[%{public}d_%{public}d_%{public}d] "\
        "ec[%{public}d_%{public}d_%{public}d_%{public}d] "\
        "micref[%{public}d_%{public}d_%{public}d_%{public}d]",
        sourceInfo.samplingRate, sourceInfo.channels, sourceInfo.format,
        sourceInfo.ecType, sourceInfo.ecSamplingRate, sourceInfo.ecChannels, sourceInfo.ecFormat,
        sourceInfo.micRef, sourceInfo.micRefSamplingRate, sourceInfo.micRefChannels, sourceInfo.micRefFormat);
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
    CHECK_AND_RETURN_RET_LOG(item != audioEnhanceSupportedSceneTypes.end(), ERROR,
        "sceneType %{public}d not supported", sceneType);
    uint64_t sceneCode = static_cast<uint64_t>(sceneType);
    uint64_t sceneKeyCode = 0;
    sceneKeyCode = (sceneCode << SCENE_TYPE_OFFSET) + (captureId_ << CAPTURER_ID_OFFSET) + renderId_;
    AUDIO_INFO_LOG("sceneCode:%{public}" PRIu64 "sceneKeyCode:%{public}" PRIu64, sceneCode, sceneKeyCode);
    CaptureEffectAttr attr = {};
    attr.micChannels = static_cast<uint32_t>(sourceInfo_.channels);
    attr.ecChannels = static_cast<uint32_t>(sourceInfo_.ecChannels);
    attr.micRefChannels = static_cast<uint32_t>(sourceInfo_.micRefChannels);
    
    int32_t ret = sceneClusterMap_[processorType]->CaptureEffectCreate(sceneKeyCode, attr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "sceneType[%{public}u] create failed", sceneType);
    return SUCCESS;
}

int32_t HpaeCapturerManager::CreateOutputSession(const HpaeStreamInfo &streamInfo)
{
    AUDIO_INFO_LOG("CreateStream sessionId %{public}u deviceName %{public}s",
        streamInfo.sessionId, sourceInfo_.deviceName.c_str());
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

    // todo: sourceType->processorType->sceneType => sourceType->sceneType
    AudioEnhanceScene enhanceScene = TransProcessType2EnhanceScene(sceneType);
    nodeInfo.effectInfo.enhanceScene = enhanceScene;
    sourceOutputNodeMap_[streamInfo.sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    sourceOutputNodeMap_[streamInfo.sessionId]->SetAppUid(streamInfo.uid);
    sessionNodeMap_[streamInfo.sessionId].sceneType = sceneType;
    
    if (sceneType != HPAE_SCENE_EFFECT_NONE && !SafeGetMap(sceneClusterMap_, sceneType)) {
        // todo: algorithm instance count control
        HpaeNodeInfo clusterNodeInfo;
        clusterNodeInfo.channels = sourceInfo_.channels;
        clusterNodeInfo.format = sourceInfo_.format;
        clusterNodeInfo.samplingRate = sourceInfo_.samplingRate;
        clusterNodeInfo.frameLen = CalculateFrameLenBySampleRate(clusterNodeInfo.samplingRate);
        clusterNodeInfo.statusCallback = weak_from_this();
        clusterNodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
        sceneClusterMap_[sceneType] = std::make_shared<HpaeSourceProcessCluster>(clusterNodeInfo);
        if (CaptureEffectCreate(sceneType, enhanceScene) != SUCCESS) {
            // not erase effect processcluster for inject
            AUDIO_WARNING_LOG("sceneType[%{public}u] create failed, not delete sceneCluster", sceneType);
        }
    }

    return SUCCESS;
}

int32_t HpaeCapturerManager::CaptureEffectRelease(const HpaeProcessorType &sceneType)
{
    uint64_t sceneCode = static_cast<uint64_t>(TransProcessType2EnhanceScene(sceneType));
    uint64_t sceneKeyCode = 0;
    sceneKeyCode = (sceneCode << SCENE_TYPE_OFFSET) + (captureId_ << CAPTURER_ID_OFFSET) + renderId_;
    int32_t ret = sceneClusterMap_[sceneType]->CaptureEffectRelease(sceneKeyCode);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "sceneType[%{public}u] release failed", sceneType);
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

    HpaeNodeInfo micNodeInfo;
    if (SafeGetMap(sourceInputClusterMap_, mainMicType_) &&
        sceneCluster->GetCapturerEffectConfig(micNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MIC)) {
        sceneCluster->DisConnectWithInfo(
            sourceInputClusterMap_[mainMicType_], micNodeInfo); // mic
    }
    return;
}

int32_t HpaeCapturerManager::DeleteOutputSession(uint32_t sessionId)
{
    AUDIO_INFO_LOG("delete output node:%{public}d, source name:%{public}s", sessionId, sourceInfo_.deviceClass.c_str());
    auto sourceOutputNode = SafeGetMap(sourceOutputNodeMap_, sessionId);
    if (!sourceOutputNode) {
        sourceOutputNodeMap_.erase(sessionId);
        sessionNodeMap_.erase(sessionId);
        return SUCCESS;
    }
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(false, sourceOutputNode->GetNodeInfo());
#endif

    HpaeProcessorType sceneType = sessionNodeMap_[sessionId].sceneType;
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
    sourceOutputNodeMap_.erase(sessionId);
    sessionNodeMap_.erase(sessionId);
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
    CHECK_AND_RETURN_RET_LOG(sourceInfo_.ecType != HPAE_EC_TYPE_NONE, false, "source not need ec");
    CHECK_AND_RETURN_RET_LOG(CheckSceneTypeNeedEc(sceneType), false, "scene not need ec");
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sceneClusterMap_, sceneType) &&
        sceneClusterMap_[sceneType]->GetCapturerEffectConfig(ecNodeInfo, HPAE_SOURCE_BUFFER_TYPE_EC),
        false, "capture effect node has no ec config");
    ecNodeType = sourceInfo_.ecType == HPAE_EC_TYPE_SAME_ADAPTER ? mainMicType_ : HPAE_SOURCE_EC;
    AUDIO_INFO_LOG("resolve connect or disconnect for ecNode type[%{public}u]", ecNodeType);
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceInputClusterMap_, ecNodeType), false, "ec node is null");
    return true;
}

bool HpaeCapturerManager::CheckMicRefCondition(const HpaeProcessorType &sceneType, HpaeNodeInfo &micRefNodeInfo)
{
    CHECK_AND_RETURN_RET_LOG(sourceInfo_.micRef == HPAE_REF_ON, false, "source not need micref");
    CHECK_AND_RETURN_RET_LOG(CheckSceneTypeNeedMicRef(sceneType), false, "scene not need micref");
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sceneClusterMap_, sceneType) &&
        sceneClusterMap_[sceneType]->GetCapturerEffectConfig(micRefNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MICREF),
        false, "capture effect node has no micref config");
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_MICREF), false, "micref node is null");
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
    CHECK_AND_RETURN_RET_LOG(sourceOutputNode, ERR_INVALID_PARAM,
        "ConnectOutputSession error, sessionId %{public}u can not find in sourceOutputNodeMap.\n", sessionId);
    
    HpaeProcessorType sceneType = sessionNodeMap_[sessionId].sceneType;
    auto scnenCluster = SafeGetMap(sceneClusterMap_, sceneType);
    if (sceneType != HPAE_SCENE_EFFECT_NONE && scnenCluster != nullptr) {
        HpaeNodeInfo micNodeInfo;
        if (scnenCluster->GetCapturerEffectConfig(micNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MIC)) {
            scnenCluster->ConnectWithInfo(sourceInputClusterMap_[mainMicType_], micNodeInfo); // mic
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
    CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[mainMicType_], ERR_ILLEGAL_STATE,
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);
    CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[mainMicType_]->GetSourceState() != STREAM_MANAGER_RUNNING,
        SUCCESS, "capturer source is already opened");
    int32_t ret = sourceInputClusterMap_[mainMicType_]->CapturerSourceStart();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "capturer source start error, ret = %{public}d.", ret);
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER) {
        CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[HPAE_SOURCE_EC], ERR_ILLEGAL_STATE,
            "sourceInputClusterMap_[%{public}d] is nullptr", HPAE_SOURCE_EC);
        ret = sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceStart();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ec capturer source start error, ret = %{public}d.", ret);
    }
    if (sourceInfo_.micRef == HPAE_REF_ON) {
        CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[HPAE_SOURCE_MICREF], ERR_ILLEGAL_STATE,
            "sourceInputClusterMap_[%{public}d] is nullptr", HPAE_SOURCE_MICREF);
        ret = sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceStart();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "micref capturer source start error, ret = %{public}d.", ret);
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
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeCapturerManager::DisConnectOutputSession(uint32_t sessionId)
{
    HpaeProcessorType sceneType = sessionNodeMap_[sessionId].sceneType;
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

int32_t HpaeCapturerManager::Pause(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Pause");
        AUDIO_INFO_LOG("Pause sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId),
            "Pause not find sessionId %{public}u", sessionId);
        DisConnectOutputSession(sessionId);
        SetSessionState(sessionId, HPAE_SESSION_PAUSED);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
            HPAE_SESSION_PAUSED, OPERATION_PAUSED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
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
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceInputClusterMap_, mainMicType_), ERR_ILLEGAL_STATE,
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
    HpaePolicyManager::GetInstance().SendInitCommandToAlgo();
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
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
            HPAE_SESSION_STOPPED, OPERATION_STOPPED);
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
            AUDIO_ERR_LOG("get ec capturer soruce instance error, ret = %{public}d.\n", ret);
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
            AUDIO_ERR_LOG("get micRef capturer soruce instance error, ret = %{public}d.\n", ret);
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

int32_t HpaeCapturerManager::InitCapturer()
{
    IAudioSourceAttr attr;
    CreateSourceAttr(attr);
    if (attr.hasEcConfig) {
        attr.formatEc = sourceInfo_.ecFormat;
        attr.sampleRateEc = sourceInfo_.ecSamplingRate;
        attr.channelEc = sourceInfo_.ecChannels;
    }
    int32_t ret = sourceInputClusterMap_[mainMicType_]->CapturerSourceInit(attr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_OPERATION,
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
            AUDIO_ERR_LOG("init ec source input node err, ret = %{public}d.\n", ret);
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
            AUDIO_ERR_LOG("init micRef source input node err, ret = %{public}d.\n", ret);
            sourceInputClusterMap_.erase(HPAE_SOURCE_MICREF);
        }
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::ReloadCaptureManager(const HpaeSourceInfo &sourceInfo, bool isReload)
{
    if (IsInit()) {
        DeInit();
    }
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, sourceInfo, isReload] {
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
            DeleteOutputSession(it.sessionId);
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
        // connect
        for (const auto &moveInfo : moveInfos) {
            AddSingleNodeToSource(moveInfo, true);
        }
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sourceInfo_.deviceName, ret);
        TriggerCallback(INIT_SOURCE_RESULT, sourceInfo_.sourceType);
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeCapturerManager::InitCapturerManager()
{
    HpaeNodeInfo nodeInfo;
    HpaeNodeInfo ecNodeInfo;
    HpaeNodeInfo micRefNodeInfo;
    CHECK_AND_RETURN_RET_LOG(CheckSourceInfoFramelen(sourceInfo_) == SUCCESS, ERROR, "Check SourceInfo ERROR");
    nodeInfo.deviceClass = sourceInfo_.deviceClass;
    nodeInfo.channels = sourceInfo_.channels;
    nodeInfo.format = sourceInfo_.format;
    nodeInfo.frameLen = sourceInfo_.frameLen;
    nodeInfo.samplingRate = sourceInfo_.samplingRate;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.statusCallback = weak_from_this();
    mainMicType_ = sourceInfo_.ecType == HPAE_EC_TYPE_SAME_ADAPTER ? HPAE_SOURCE_MIC_EC : HPAE_SOURCE_MIC;

    if (mainMicType_ == HPAE_SOURCE_MIC_EC) {
        ecNodeInfo.channels = sourceInfo_.ecChannels;
        ecNodeInfo.format = sourceInfo_.ecFormat;
        ecNodeInfo.samplingRate = sourceInfo_.ecSamplingRate;
        ecNodeInfo.frameLen = sourceInfo_.ecFrameLen;
        ecNodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
        ecNodeInfo.statusCallback = weak_from_this();
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
        sourceInfo_.deviceClass, sourceInfo_.deviceNetId, sourceInfo_.sourceType, sourceInfo_.sourceName);
    captureId_ = sourceInputClusterMap_[mainMicType_]->GetCaptureId();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get mic capturer soruce instance error, ret = %{public}d.\n", ret);
    PrepareCapturerEc(ecNodeInfo);
    PrepareCapturerMicRef(micRefNodeInfo);
    ret = InitCapturer();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "init main capturer error");
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
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceInputClusterMap_, mainMicType_), ERR_INVALID_OPERATION,
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);
    CapturerSourceStop();
    sourceInputClusterMap_[mainMicType_]->CapturerSourceDeInit();
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_EC)) {
        sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceDeInit();
    }
    if (sourceInfo_.micRef == HPAE_REF_ON && SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_MICREF)) {
        sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceDeInit();
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
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(true, nodeInfo);
#endif
    HpaeProcessorType sceneType = sessionNodeMap_[sessionId].sceneType;
    AudioEnhanceScene enhanceScene = TransProcessType2EnhanceScene(sceneType);
    if (sceneType != HPAE_SCENE_EFFECT_NONE) {
        // todo: algorithm instance count control
        if (!SafeGetMap(sceneClusterMap_, sceneType)) {
            sceneClusterMap_[sceneType] = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
        }
    }
    if (CaptureEffectCreate(sceneType, enhanceScene) != SUCCESS) {
        AUDIO_WARNING_LOG("[FinishMove] session :%{public}u,create effect failed.", sessionId);
    }

    if (moveInfo.sessionInfo.state == HPAE_SESSION_RUNNING) {
        ConnectOutputSession(sessionId);
        CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
    }
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
        TriggerSyncCallback(MOVE_ALL_SOURCE_OUTPUT, moveInfos, name);
    } else {
        TriggerCallback(MOVE_ALL_SOURCE_OUTPUT, moveInfos, name);
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
    for (auto &sessionPair : sessionNodeMap_) {
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
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS