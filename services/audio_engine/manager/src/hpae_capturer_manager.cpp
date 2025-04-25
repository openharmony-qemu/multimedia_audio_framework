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
#include "audio_engine_log.h"
#include "audio_errors.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#include "audio_effect_map.h"

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

void HpaeCapturerManager::SetCaptureId(uint32_t captureId)
{
    captureId_ = captureId;
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
    AUDIO_INFO_LOG("Create output node:%{public}d", streamInfo.sessionId);
    HpaeNodeInfo nodeInfo;
    nodeInfo.nodeId = OnGetNodeId();
    nodeInfo.nodeName = "HpaeSourceOutputNode";
    nodeInfo.channels = streamInfo.channels;
    nodeInfo.format = streamInfo.format;
    nodeInfo.frameLen = streamInfo.frameLen;
    nodeInfo.streamType = streamInfo.streamType;
    nodeInfo.sessionId = streamInfo.sessionId;
    nodeInfo.samplingRate = (AudioSamplingRate)streamInfo.samplingRate;
    HpaeProcessorType sceneType = TransSourceTypeToSceneType(streamInfo.sourceType);
    nodeInfo.sceneType = sceneType;
    nodeInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    nodeInfo.statusCallback = weak_from_this();

    // todo: sourceType->processorType->sceneType => sourceType->sceneType
    AudioEnhanceScene enhanceScene = TransProcessType2EnhanceScene(sceneType);
    nodeInfo.effectInfo.enhanceScene = enhanceScene;
    sourceOutputNodeMap_[streamInfo.sessionId] = std::make_shared<HpaeSourceOutputNode>(nodeInfo);
    sessionNodeMap_[streamInfo.sessionId].sceneType = sceneType;
    
    if (sceneType != HPAE_SCENE_EFFECT_NONE && !SafeGetMap(sceneClusterMap_, sceneType)) {
        // todo: algorithm instance count control
        sceneClusterMap_[sceneType] = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
        if (CaptureEffectCreate(sceneType, enhanceScene) != SUCCESS) {
            sceneClusterMap_.erase(sceneType);
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
    if (sceneClusterMap_[sceneType]->GetOutputPortNum() != 0) {
        return;
    }
    // need to disconnect sceneCluster and sourceInputCluster
    HpaeNodeInfo ecNodeInfo;
    if (CheckSceneTypeNeedEc(sceneType) &&
        sceneClusterMap_[sceneType]->GetCapturerEffectConfig(ecNodeInfo, HPAE_SOURCE_BUFFER_TYPE_EC)) {
        if (sourceInfo_.ecType == HPAE_EC_TYPE_SAME_ADAPTER) {
            sceneClusterMap_[sceneType]->DisConnectWithInfo(
                sourceInputClusterMap_[mainMicType_], ecNodeInfo); // ec from mic
        } else if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER) {
            sceneClusterMap_[sceneType]->DisConnectWithInfo(
                sourceInputClusterMap_[HPAE_SOURCE_EC], ecNodeInfo); // ec
        }
    }

    HpaeNodeInfo micRefNodeInfo;
    if (CheckSceneTypeNeedMicRef(sceneType) &&
        sceneClusterMap_[sceneType]->GetCapturerEffectConfig(micRefNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MICREF) &&
        sourceInfo_.micRef == HPAE_REF_ON) {
        sceneClusterMap_[sceneType]->DisConnectWithInfo(
            sourceInputClusterMap_[HPAE_SOURCE_MICREF], micRefNodeInfo); // micref
    }

    HpaeNodeInfo micNodeInfo;
    if (sceneClusterMap_[sceneType]->GetCapturerEffectConfig(micNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MIC)) {
        sceneClusterMap_[sceneType]->DisConnectWithInfo(
            sourceInputClusterMap_[mainMicType_], micNodeInfo); // mic
    }
    return;
}

int32_t HpaeCapturerManager::DeleteOutputSession(uint32_t sessionId)
{
    AUDIO_INFO_LOG("delete output node:%{public}d, source name:%{public}s", sessionId, sourceInfo_.deviceClass.c_str());
    if (!SafeGetMap(sourceOutputNodeMap_, sessionId)) {
        return SUCCESS;
    }

    if (!sourceOutputNodeMap_[sessionId]) {
        sourceOutputNodeMap_.erase(sessionId);
        sessionNodeMap_.erase(sessionId);
        return SUCCESS;
    }

    HpaeProcessorType sceneType = sessionNodeMap_[sessionId].sceneType;
    if (sceneType != HPAE_SCENE_EFFECT_NONE && SafeGetMap(sceneClusterMap_, sceneType)) {
        sourceOutputNodeMap_[sessionId]->DisConnectWithInfo(
            sceneClusterMap_[sceneType], sourceOutputNodeMap_[sessionId]->GetNodeInfo());
        DisConnectSceneClusterFromSourceInputCluster(sceneType);
        if (sceneClusterMap_[sceneType]->GetOutputPortNum() == 0) {
            CaptureEffectRelease(sceneType);
            sceneClusterMap_.erase(sceneType);
        }
    } else {
        sourceOutputNodeMap_[sessionId]->DisConnect(sourceInputClusterMap_[mainMicType_]);
    }
    sourceOutputNodeMap_.erase(sessionId);
    sessionNodeMap_.erase(sessionId);
    return SUCCESS;
}

void HpaeCapturerManager::SetSessionState(uint32_t sessionId,  CapturerState capturerState)
{
    sessionNodeMap_[sessionId].state = capturerState;
}

int32_t HpaeCapturerManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, streamInfo]() {
        AUDIO_INFO_LOG("CreateStream sessionId %{public}u deviceName %{public}s",
            streamInfo.sessionId,
            sourceInfo_.deviceName.c_str());
        CreateOutputSession(streamInfo);
        SetSessionState(streamInfo.sessionId, CAPTURER_NEW);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeCapturerManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        DeleteOutputSession(sessionId);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeCapturerManager::ConnectProcessClusterWithEc(HpaeProcessorType &sceneType)
{
    HpaeNodeInfo ecNodeInfo;
    if (CheckSceneTypeNeedEc(sceneType) &&
        sceneClusterMap_[sceneType]->GetCapturerEffectConfig(ecNodeInfo, HPAE_SOURCE_BUFFER_TYPE_EC)) {
        ecNodeInfo.statusCallback = weak_from_this();
        if (sourceInfo_.ecType == HPAE_EC_TYPE_SAME_ADAPTER) {
            sceneClusterMap_[sceneType]->ConnectWithInfo(
                sourceInputClusterMap_[mainMicType_], ecNodeInfo); // ec from mic
        } else if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER) {
            sceneClusterMap_[sceneType]->ConnectWithInfo(
                sourceInputClusterMap_[HPAE_SOURCE_EC], ecNodeInfo); // ec
        }
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::ConnectProcessClusterWithMicRef(HpaeProcessorType &sceneType)
{
    HpaeNodeInfo micRefNodeInfo;
    if (CheckSceneTypeNeedMicRef(sceneType) &&
        sceneClusterMap_[sceneType]->GetCapturerEffectConfig(micRefNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MICREF)&&
        sourceInfo_.micRef == HPAE_REF_ON) {
        micRefNodeInfo.statusCallback = weak_from_this();
        sceneClusterMap_[sceneType]->ConnectWithInfo(
            sourceInputClusterMap_[HPAE_SOURCE_MICREF], micRefNodeInfo); // micref
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::ConnectOutputSession(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId), ERR_INVALID_PARAM,
        "ConnectOutputSession error, sessionId %{public}u can not find in sourceOutputNodeMap.\n", sessionId);
    
    HpaeProcessorType sceneType = sessionNodeMap_[sessionId].sceneType;
    if (sceneType != HPAE_SCENE_EFFECT_NONE && SafeGetMap(sceneClusterMap_, sceneType)) {
        // 1. Determine if the ResampleNode needs to be created
        // 2. If ResampleNode needs to be created, it should be connected to the UpEffectNode after creation
        // 3. Connect the SourceOutputNode to the ResampleNode
        sourceOutputNodeMap_[sessionId]->ConnectWithInfo(sceneClusterMap_[sceneType],
            sourceOutputNodeMap_[sessionId]->GetNodeInfo());
        HpaeNodeInfo micNodeInfo;
        micNodeInfo.statusCallback = weak_from_this();
        if (sceneClusterMap_[sceneType]->GetCapturerEffectConfig(micNodeInfo, HPAE_SOURCE_BUFFER_TYPE_MIC)) {
            sceneClusterMap_[sceneType]->ConnectWithInfo(sourceInputClusterMap_[mainMicType_], micNodeInfo); // mic
        }
        ConnectProcessClusterWithEc(sceneType);
        ConnectProcessClusterWithMicRef(sceneType);
    } else {
        sourceOutputNodeMap_[sessionId]->ConnectWithInfo(sourceInputClusterMap_[mainMicType_],
            sourceOutputNodeMap_[sessionId]->GetNodeInfo());
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::CapturerSourceStart()
{
    CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[mainMicType_], ERR_ILLEGAL_STATE,
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);
    CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[mainMicType_]->GetSourceState() != CAPTURER_RUNNING,
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
    Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Start");
    auto request = [this, sessionId]() {
        AUDIO_INFO_LOG("Start sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(ConnectOutputSession(sessionId) == SUCCESS, "Connect node error.");
        SetSessionState(sessionId, CAPTURER_RUNNING);
        CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
            sessionNodeMap_[sessionId].state, OPERATION_STARTED);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeCapturerManager::DisConnectOutputSession(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(SafeGetMap(sourceOutputNodeMap_, sessionId), SUCCESS,
        "sessionId %{public}u can not find in sourceOutputNodeMap.", sessionId);
    HpaeProcessorType sceneType = sessionNodeMap_[sessionId].sceneType;
    if (sceneType != HPAE_SCENE_EFFECT_NONE && SafeGetMap(sceneClusterMap_, sceneType)) {
        // 1. Disconnect SourceOutputNode and ResampleNode
        // 2. Disconnect the ResampleNode and UpEffectNode
        // 3. If the ResampleNode has no output, it needs to be deleted
        sourceOutputNodeMap_[sessionId]->DisConnectWithInfo(
            sceneClusterMap_[sceneType], sourceOutputNodeMap_[sessionId]->GetNodeInfo());
        DisConnectSceneClusterFromSourceInputCluster(sceneType);
    } else {
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
    Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Pause");
    auto request = [this, sessionId]() {
        AUDIO_INFO_LOG("Pause sessionId %{public}u", sessionId);
        DisConnectOutputSession(sessionId);
        SetSessionState(sessionId, CAPTURER_PAUSED);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
            sessionNodeMap_[sessionId].state, OPERATION_PAUSED);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeCapturerManager::Flush(uint32_t sessionId)
{
    if (sessionNodeMap_.find(sessionId) == sessionNodeMap_.end()) {
        return ERR_INVALID_OPERATION;
    }
    // to do
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
        sessionNodeMap_[sessionId].state, OPERATION_FLUSHED);
    return SUCCESS;
}

int32_t HpaeCapturerManager::Drain(uint32_t sessionId)
{
    if (sessionNodeMap_.find(sessionId) == sessionNodeMap_.end()) {
        return ERR_INVALID_OPERATION;
    }
    // to do
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
        sessionNodeMap_[sessionId].state, OPERATION_DRAINED);
    return SUCCESS;
}

int32_t HpaeCapturerManager::CapturerSourceStop()
{
    CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[mainMicType_], ERR_ILLEGAL_STATE,
        "sourceInputClusterMap_[%{public}d] is nullptr", mainMicType_);
    CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[mainMicType_]->GetSourceState() != CAPTURER_STOPPED,
        SUCCESS, "capturer source is already stopped");
    int32_t ret = sourceInputClusterMap_[mainMicType_]->CapturerSourceStop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "capturer source stop error, ret = %{public}d.\n", ret);
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER) {
        CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[HPAE_SOURCE_EC], ERR_ILLEGAL_STATE,
            "sourceInputClusterMap_[%{public}d] is nullptr", HPAE_SOURCE_EC);
        ret = sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceStop();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ec capturer source stop error, ret = %{public}d.\n", ret);
    }
    if (sourceInfo_.micRef == HPAE_REF_ON) {
        CHECK_AND_RETURN_RET_LOG(sourceInputClusterMap_[HPAE_SOURCE_MICREF], ERR_ILLEGAL_STATE,
            "sourceInputClusterMap_[%{public}d] is nullptr", HPAE_SOURCE_MICREF);
        ret = sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceStop();
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ec capturer source stop error, ret = %{public}d.\n", ret);
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::Stop(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Stop");
    auto request = [this, sessionId]() {
        DisConnectOutputSession(sessionId);
        SetSessionState(sessionId, CAPTURER_STOPPED);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_RECORD, sessionId,
            sessionNodeMap_[sessionId].state, OPERATION_STOPPED);
    };
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeCapturerManager::Release(uint32_t sessionId)
{
    Trace trace("[" + std::to_string(sessionId) + "]HpaeCapturerManager::Release");
    return DestroyStream(sessionId);
}

int32_t HpaeCapturerManager::SetMute(bool isMute)
{
    // to do check pulseaudio
    auto request = [this, isMute]() {
        if (isMute_ != isMute) {
            isMute_ = isMute;  // todo: fadein and fadeout and mute feature
        }
    };
    SendRequest(request);
    return SUCCESS;
}

void HpaeCapturerManager::Process()
{
    Trace trace("HpaeCapturerManager::Process");
    if (!sourceOutputNodeMap_.empty() && IsRunning()) {
        for (const auto &sourceOutputNodePair : sourceOutputNodeMap_) {
            sourceOutputNodePair.second->DoProcess();
        }
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
        int32_t ret = sourceInputClusterMap_[HPAE_SOURCE_MICREF]->GetCapturerSourceInstance(
            DEFAULT_DEVICE_CLASS, DEFAULT_DEVICE_NETWORKID, SOURCE_TYPE_INVALID, HDI_ID_INFO_EC);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_OPERATION,
            "get ec capturer soruce instance error, ret = %{public}d.\n", ret);
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
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_OPERATION,
            "get micRef capturer soruce instance error, ret = %{public}d.\n", ret);
    }
    return SUCCESS;
}

void HpaeCapturerManager::CreateSourceAttr(IAudioSourceAttr &attr)
{
    attr.adapterName = sourceInfo_.adapterName.c_str();
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
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER &&
        sourceInputClusterMap_.find(HPAE_SOURCE_EC) != sourceInputClusterMap_.end()) {
        IAudioSourceAttr attrEc;
        attrEc.sourceType = SOURCE_TYPE_EC;
        attrEc.adapterName = sourceInfo_.ecAdapterName.c_str();
        attrEc.deviceType = DEVICE_TYPE_MIC;
        attrEc.sampleRate = sourceInfo_.ecSamplingRate;
        attrEc.channel = sourceInfo_.ecChannels;
        attrEc.format = sourceInfo_.ecFormat;
        attrEc.isBigEndian = false;
        attrEc.openMicSpeaker = sourceInfo_.openMicSpeaker;
        ret = sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceInit(attrEc);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_OPERATION, "init ec source input node err");
    }
    if (sourceInfo_.micRef == HPAE_REF_ON &&
        SafeGetMap(sourceInputClusterMap_, HPAE_SOURCE_MICREF)) {
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
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_OPERATION, "init micRef source input node err");
    }
    return SUCCESS;
}

int32_t HpaeCapturerManager::ReloadCaptureManager(const HpaeSourceInfo &sourceInfo)
{
    if (IsInit()) {
        DeInit();
    }
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, sourceInfo] {
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
            AUDIO_INFO_LOG("re-Init HpaeCapturerManager failed");
            return;
        }
        AUDIO_INFO_LOG("re-Init HpaeCapturerManager success");
        // connect
        for (const auto &moveInfo : moveInfos) {
            AddSingleNodeToSource(moveInfo, true);
        }
        TriggerCallback(INIT_DEVICE_RESULT, sourceInfo_.deviceName, ret);
    };
    SendRequest(request, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeCapturerManager::InitCapturerManager()
{
    HpaeNodeInfo nodeInfo;
    HpaeNodeInfo ecNodeInfo;
    HpaeNodeInfo micRefNodeInfo;
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
    int32_t ret = sourceInputClusterMap_[mainMicType_]->GetCapturerSourceInstance(
        sourceInfo_.deviceClass, sourceInfo_.deviceNetId, sourceInfo_.sourceType, sourceInfo_.sourceName);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "get mic capturer soruce instance error, ret = %{public}d.\n", ret);
    CHECK_AND_RETURN_RET_LOG(PrepareCapturerEc(ecNodeInfo) == SUCCESS, ret, "PrepareCapturerEc error");
    CHECK_AND_RETURN_RET_LOG(PrepareCapturerMicRef(micRefNodeInfo) == SUCCESS, ret, "PrepareCapturerMicRef error");
    CHECK_AND_RETURN_RET_LOG(InitCapturer() == SUCCESS, ret, "init main capturer error");
    isInit_.store(true);
    return SUCCESS;
}


int32_t HpaeCapturerManager::Init()
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this] {
        int32_t ret = InitCapturerManager();
        if (ret == SUCCESS) {
            AUDIO_INFO_LOG("Init HpaeCapturerManager success");
            TriggerCallback(INIT_DEVICE_RESULT, sourceInfo_.deviceName, ret);
        }
    };
    SendRequest(request, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeCapturerManager::DeInit(bool isMoveDefault)
{
    AUDIO_INFO_LOG("DeInit device:%{public}s", sourceInfo_.deviceName.c_str());
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    int32_t ret = CapturerSourceStop();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_OPERATION,
        "capturerSource stop error, ret = %{public}d.\n", ret);
    ret = sourceInputClusterMap_[mainMicType_]->CapturerSourceDeInit();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_OPERATION,
        "capturerSourceDeInit error, ret = %{public}d.\n", ret);
    if (sourceInfo_.ecType == HPAE_EC_TYPE_DIFF_ADAPTER) {
        ret = sourceInputClusterMap_[HPAE_SOURCE_EC]->CapturerSourceDeInit();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_INVALID_OPERATION,
            "deinit ec source input node err.ret = %d.\n", ret);
    }
    if (sourceInfo_.micRef == HPAE_REF_ON) {
        ret = sourceInputClusterMap_[HPAE_SOURCE_MICREF]->CapturerSourceDeInit();
        CHECK_AND_RETURN_RET_LOG(ret == 0, ERR_INVALID_OPERATION,
            "deinit micref source input node err.ret = %d.\n", ret);
    }
    isInit_.store(false);
    
    if (isMoveDefault) {
        std::string name = "";
        std::vector<uint32_t> ids;
        AUDIO_INFO_LOG("move all source to default sink");
        MoveAllStreamToNewSource(name, ids, true);
    }
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

int32_t HpaeCapturerManager::RegisterReadCallback(uint32_t sessionId, const std::weak_ptr<IReadCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        sourceOutputNodeMap_[sessionId]->RegisterReadCallback(callback);
    };
    SendRequest(request);
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
        return sourceInputClusterMap_[mainMicType_]->GetSourceState() == CAPTURER_RUNNING &&
            hpaeSignalProcessThread_->IsRunning();
    } else {
        return false;
    }
}

void HpaeCapturerManager::SendRequest(Request &&request, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_INFO_LOG("HpaeCapturerManager not init");
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ capturer is nullptr");
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
    SendRequest(request);
    return SUCCESS;
}

int32_t HpaeCapturerManager::AddNodeToSource(const HpaeCaptureMoveInfo &moveInfo)
{
    auto request = [this, moveInfo]() { AddSingleNodeToSource(moveInfo); };
    SendRequest(request);
    return SUCCESS;
}

void HpaeCapturerManager::AddSingleNodeToSource(const HpaeCaptureMoveInfo &moveInfo, bool isConnect)
{
    uint32_t sessionId = moveInfo.sessionId;
    AUDIO_INFO_LOG("[FinishMove] session :%{public}u to sink:[%{public}s].", sessionId, sourceInfo_.sourceName.c_str());
    sourceOutputNodeMap_[sessionId] = moveInfo.sourceOutputNode;
    sessionNodeMap_[sessionId] = moveInfo.sessionInfo;
    HpaeProcessorType sceneType = sessionNodeMap_[sessionId].sceneType;
    AudioEnhanceScene enhanceScene = TransProcessType2EnhanceScene(sceneType);
    if (sceneType != HPAE_SCENE_EFFECT_NONE) {
        // todo: algorithm instance count control
        HpaeNodeInfo nodeInfo = moveInfo.sourceOutputNode->GetNodeInfo();
        if (!SafeGetMap(sceneClusterMap_, sceneType)) {
            sceneClusterMap_[sceneType] = std::make_shared<HpaeSourceProcessCluster>(nodeInfo);
        }
    }
    if (CaptureEffectCreate(sceneType, enhanceScene) != SUCCESS) {
        AUDIO_WARNING_LOG("[FinishMove] session :%{public}u,create effect failed.", sessionId);
        sceneClusterMap_.erase(sceneType);
    }
    ConnectOutputSession(sessionId);
    if (moveInfo.sessionInfo.state == CAPTURER_RUNNING) {
        CHECK_AND_RETURN_LOG(CapturerSourceStart() == SUCCESS, "CapturerSourceStart error.");
        hpaeSignalProcessThread_->Notify();
    }
}

int32_t HpaeCapturerManager::MoveAllStream(const std::string &sourceName, const std::vector<uint32_t>& sessionIds,
    bool isMoveAll)
{
    if (!IsInit()) {
        AUDIO_INFO_LOG("source is not init ,use sync mode move to: %{public}s", sourceName.c_str());
        MoveAllStreamToNewSource(sourceName, sessionIds, isMoveAll);
    } else {
        AUDIO_INFO_LOG("source is init ,use async mode move to: %{public}s", sourceName.c_str());
        auto request = [this, sourceName, sessionIds, isMoveAll]() {
            MoveAllStreamToNewSource(sourceName, sessionIds, isMoveAll);
        };
        SendRequest(request);
    }
    return SUCCESS;
}

void HpaeCapturerManager::MoveAllStreamToNewSource(const std::string &sourceName,
    const std::vector<uint32_t>& moveIds, bool isMoveAll = true)
{
    std::string name = sourceName;
    std::vector<HpaeCaptureMoveInfo> moveInfos;
    std::string idStr;
    for (const auto &it : sourceOutputNodeMap_) {
        if (isMoveAll || std::find(moveIds.begin(), moveIds.end(), it.first) != moveIds.end()) {
            HpaeCaptureMoveInfo moveInfo;
            moveInfo.sessionId = it.first;
            moveInfo.sourceOutputNode = it.second;
            idStr.append("[");
            idStr.append(std::to_string(it.first));
            idStr.append("],");
            if (sessionNodeMap_.find(it.first) != sessionNodeMap_.end()) {
                moveInfo.sessionInfo = sessionNodeMap_[it.first];
                moveInfos.emplace_back(moveInfo);
            }
        }
    }

    for (const auto &it : moveInfos) {
        DeleteOutputSession(it.sessionId);
    }
    AUDIO_INFO_LOG("[StartMove] session:%{public}s to source name:%{public}s, isMoveAll:%{public}d",
        idStr.c_str(), name.c_str(), isMoveAll);
    TriggerCallback(MOVE_ALL_SOURCE_OUTPUT, moveInfos, name);
}

int32_t HpaeCapturerManager::MoveStream(uint32_t sessionId, const std::string& sourceName)
{
    auto request = [this, sessionId, sourceName]() {
        if (!SafeGetMap(sourceOutputNodeMap_, sessionId)) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,not find session,move %{public}s --> %{public}s",
                sessionId, sourceInfo_.sourceName.c_str(), sourceName.c_str());
            return;
        }
        std::shared_ptr<HpaeSourceOutputNode> sourceNode = sourceOutputNodeMap_[sessionId];
        if (sessionNodeMap_.find(sessionId)==sessionNodeMap_.end()) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,not find session node,move %{public}s --> %{public}s",
                sessionId, sourceInfo_.sourceName.c_str(), sourceName.c_str());
            return;
        }
        if(sourceName.empty()) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,sourceName is empty", sessionId);
            return;
        }
        AUDIO_INFO_LOG("[StartMove] session: %{public}u,sink [%{public}s] --> [%{public}s]",
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
    SendRequest(request);
    return SUCCESS;
}

void HpaeCapturerManager::OnNotifyQueue()
{
    hpaeSignalProcessThread_->Notify();
}

std::string HpaeCapturerManager::GetThreadName()
{
    return sourceInfo_.deviceName;
}

void HpaeCapturerManager::DumpSourceInfo()
{
    SendRequest([this]() {
        AUDIO_INFO_LOG("DumpSourceInfo deviceName %{public}s", sourceInfo_.deviceName.c_str());
        UploadDumpSourceInfo(sourceInfo_.deviceName);
    });
}

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS