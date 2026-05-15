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
#define LOG_TAG "HpaeFastRendererManager"
#endif

#include "hpae_fast_renderer_manager.h"
#include "audio_stream_info.h"
#include "audio_errors.h"
#include "hpae_define.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"
#include "hpae_message_queue_monitor.h"
#include "hpae_stream_move_monitor.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
namespace {
    constexpr size_t FAST_REQUEST_COUNT = 5000;
    constexpr float SUSPEND_TIME_OUT_S = 3; // prevent stop not success
}

HpaeFastRendererManager::HpaeFastRendererManager(HpaeSinkInfo &sinkInfo)
    : hpaeNoLockQueue_(FAST_REQUEST_COUNT), sinkInfo_(sinkInfo)
{
}

HpaeFastRendererManager::~HpaeFastRendererManager()
{
    AUDIO_INFO_LOG("destructor");
    if (isInit_.load()) {
        DeInit();
    }
}

std::shared_ptr<HpaeSinkInputNode> HpaeFastRendererManager::CreateInputSession(const HpaeStreamInfo &streamInfo)
{
    Trace trace("HpaeFastRendererManager::CreateInputSession id[" + std::to_string(streamInfo.sessionId) + "]");
    HpaeNodeInfo nodeInfo;
    ConfigNodeInfo(nodeInfo, streamInfo);
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    AUDIO_INFO_LOG("streamType %{public}u, sessionId = %{public}u, channels:%{public}u, rate:%{public}u",
        nodeInfo.streamType, nodeInfo.sessionId, nodeInfo.channels, nodeInfo.samplingRate);
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->SetAppUid(streamInfo.uid);
    sinkInputNodeMap_[streamInfo.sessionId] = sinkInputNode;
    CreateFastNodes(nodeInfo);
    return sinkInputNode;
}

void HpaeFastRendererManager::RemoveNodeFromMap(uint32_t sessionId)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    HpaeSessionState inputState = HPAE_SESSION_RELEASED;
    if (node != nullptr) {
        inputState = node->GetState();
#ifdef ENABLE_HIDUMP_DFX
        OnNotifyDfxNodeAdmin(false, node->GetNodeInfo());
#endif
    }
    RendererState state = inputState == HPAE_SESSION_RELEASED ? RENDERER_INVALID : RENDERER_RELEASED;
    NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, sessionId, state);
    sinkInputNodeMap_.erase(sessionId);
}

int32_t HpaeFastRendererManager::CreateFastNodes(const HpaeNodeInfo &nodeInfo)
{
    Trace trace("HpaeFastRendererManager::CreateFastNodes id[" + std::to_string(nodeInfo.sessionId) + "]");
    HpaeNodeInfo outputNodeInfo = mixerNode_->GetNodeInfo();
    outputNodeInfo.streamType = nodeInfo.streamType;
    outputNodeInfo.sessionId = nodeInfo.sessionId;
    outputNodeInfo.fadeType = nodeInfo.fadeType;
    auto converter = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, outputNodeInfo);
    converter->SetDownmixNormalization(false);
    converterNodeMap_[nodeInfo.sessionId] = converter;
    gainNodeMap_[nodeInfo.sessionId] = std::make_shared<HpaeGainNode>(outputNodeInfo);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::DeleteSessionNodes(uint32_t sessionId)
{
    Trace trace("HpaeFastRendererManager::DeleteSessionNodes id[" + std::to_string(sessionId) + "]");
    DisConnectInputCluster(sessionId);
    gainNodeMap_.erase(sessionId);
    converterNodeMap_.erase(sessionId);
    AUDIO_INFO_LOG("SessionId %{public}u, session nodes deleted", sessionId);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::ConnectInputCluster(uint32_t sessionId)
{
    Trace trace("HpaeFastRendererManager::ConnectInputCluster id[" + std::to_string(sessionId) + "]");
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(sinkInputNode != nullptr, ERROR, "node not found");
    auto gainNode = SafeGetMap(gainNodeMap_, sessionId);
    auto converter = SafeGetMap(converterNodeMap_, sessionId);
    mixerNode_->Connect(gainNode);
    gainNode->Connect(converter);
    converter->Connect(sinkInputNode);
    AUDIO_INFO_LOG("SessionId %{public}u, input cluster connected, hasConverter=%{public}d",
        sessionId, converter != nullptr ? 1 : 0);
    // 3s 停流
    if (sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING && !isSuspend_) {
        UpdateAppsUid();
        sinkOutputNode_->RenderSinkStart();
    }
    return SUCCESS;
}

int32_t HpaeFastRendererManager::DisConnectInputCluster(uint32_t sessionId)
{
    Trace trace("HpaeFastRendererManager::DisConnectInputCluster id[" + std::to_string(sessionId) + "]");
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(sinkInputNode != nullptr, ERROR, "node not found");
    auto gainNode = SafeGetMap(gainNodeMap_, sessionId);
    auto converter = SafeGetMap(converterNodeMap_, sessionId);

    converter->DisConnect(sinkInputNode);
    gainNode->DisConnect(converter);
    mixerNode_->DisConnect(gainNode);
    AUDIO_INFO_LOG("SessionId %{public}u, input cluster disconnected", sessionId);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::ConnectInputSession(uint32_t sessionId)
{
    Trace trace("HpaeFastRendererManager::ConnectInputSession id[" + std::to_string(sessionId) + "]");
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(sinkInputNode != nullptr, ERROR, "sinkInputNode not found");
    if (sinkInputNode->GetState() != HPAE_SESSION_RUNNING) {
        return SUCCESS;
    }
    ConnectInputCluster(sessionId);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::DisConnectInputSession(uint32_t sessionId)
{
    Trace trace("HpaeFastRendererManager::DisConnectInputSession id[" + std::to_string(sessionId) + "]");
    DisConnectInputCluster(sessionId);
    return SUCCESS;
}

void HpaeFastRendererManager::DeleteInputSession(uint32_t sessionId)
{
    Trace trace("HpaeFastRendererManager::DeleteInputSession id[" + std::to_string(sessionId) + "]");
    DisConnectInputSession(sessionId);
    RemoveNodeFromMap(sessionId);
}

bool HpaeFastRendererManager::CheckIsStreamRunning()
{
    for (auto &[sessionId, node] : sinkInputNodeMap_) {
        if (node->GetState() == HPAE_SESSION_RUNNING) {
            return true;
        }
    }
    return false;
}

bool HpaeFastRendererManager::SetSessionFade(uint32_t sessionId, IOperation operation)
{
    auto sinkInputNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(sinkInputNode != nullptr, false,
        "can not get input node of session %{public}u", sessionId);
    auto gainNode = SafeGetMap(gainNodeMap_, sessionId);
    if (sinkInputNode->GetState() == HPAE_SESSION_STOPPED ||
        sinkInputNode->GetState() == HPAE_SESSION_PAUSED ||
        gainNode == nullptr || !IsRunning()) {
        AUDIO_WARNING_LOG("session %{public}d do not have gain node or sink is not running!", sessionId);
        if (operation != OPERATION_STARTED) {
            HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
            sinkInputNode->SetState(state);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
        }
        return false;
    }
    AUDIO_INFO_LOG("get gain node of session %{public}d operation %{public}d.", sessionId, operation);
    if (sinkInputNode->GetState() != HPAE_SESSION_STOPPING &&
        sinkInputNode->GetState() != HPAE_SESSION_PAUSING) {
        gainNode->SetFadeState(operation);
    }
    if (operation != OPERATION_STARTED) {
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPING : HPAE_SESSION_PAUSING;
        sinkInputNode->SetState(state);
    }
    return true;
}

void HpaeFastRendererManager::AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect)
{
    Trace trace("HpaeFastRendererManager::AddSingleNodeToSink[" + std::to_string(node->GetNodeInfo().sessionId) + "]");
    HpaeNodeInfo nodeInfo = node->GetNodeInfo();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.historyFrameCount = 0;
    nodeInfo.statusCallback = weak_from_this();
    uint32_t sessionId = nodeInfo.sessionId;
    AUDIO_INFO_LOG("[FinishMove] session:%{public}u to sink:fast", sessionId);
    bool spanSizeUpdated = false;
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->RefreshSpanSize(spanSizeUpdated);
    }
    const auto outNodeInfo = sinkOutputNode_->GetNodeInfo();
    bool frameLenAdjusted = AdjustFrameLen(nodeInfo, outNodeInfo);
    AUDIO_INFO_LOG("[FinishMode] session:%{public}u sinkSpanUpdated:%{public}d inputFrameLenAdjusted:%{public}d "
        "newFrameLen:%{public}u", sessionId, spanSizeUpdated, frameLenAdjusted, nodeInfo.frameLen);
    node->SetNodeInfo(nodeInfo);
    node->SetIsLowLatency(true);
    sinkInputNodeMap_[sessionId] = node;
    TriggerCallback(UPDATE_SPAN_SIZE, sessionId, static_cast<uint32_t>(nodeInfo.frameLen),
        HPAE_STREAM_CLASS_TYPE_PLAY);

#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(true, nodeInfo);
#endif
    CreateFastNodes(node->GetNodeInfo());
    if (!isConnect || node->GetState() != HPAE_SESSION_RUNNING) {
        AUDIO_INFO_LOG("[FinishMove] session:%{public}u not need connect session", sessionId);
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, sessionId,
            ConvertHpaeToRendererState(node->GetState()), node->GetAppUid());
        return;
    }

    ConnectInputSession(sessionId);
    NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, sessionId,
        ConvertHpaeToRendererState(node->GetState()), node->GetAppUid());
}

void HpaeFastRendererManager::MoveAllStreamToNewSink(const std::string &sinkName,
    const std::vector<uint32_t> &moveIds, MoveSessionType moveType)
{
    Trace trace("HpaeFastRendererManager::MoveAllStreamToNewSink[" +
        sinkName + "]_moveType[" + std::to_string(moveType) + "]");
    std::string name = sinkName;
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;

    for (auto &[sessionId, node] : sinkInputNodeMap_) {
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), sessionId) != moveIds.end()) {
            sinkInputs.emplace_back(node);
            AUDIO_INFO_LOG("[StartMove] session: %{public}u, sink [fast] --> [%{public}s]",
                sessionId, sinkName.c_str());
        }
    }
    for (auto &node : sinkInputs) {
        TriggerStreamState(node->GetSessionId(), node);
        DeleteSessionNodes(node->GetSessionId());
        RemoveNodeFromMap(node->GetSessionId());
    }

    if (sinkInputs.empty()) {
        AUDIO_WARNING_LOG("sink count is 0, no need move session");
    }
    if (moveType == MOVE_ALL) {
        TriggerSyncCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
    } else {
        TriggerCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
    }
}

void HpaeFastRendererManager::UpdateAppsUid()
{
    appsUid_.clear();
    bool needCheckZeroVolume = true;
    for (auto &[sessionId, node] : sinkInputNodeMap_) {
        if (node->GetState() == HPAE_SESSION_RUNNING) {
            appsUid_.emplace_back(node->GetAppUid());
            needCheckZeroVolume = node->GetKeepRunning() ? false : needCheckZeroVolume;
        }
    }
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->UpdateAppsUid(appsUid_);
        sinkOutputNode_->SetNeedCheckZeroVolume(needCheckZeroVolume);
    }
    AUDIO_DEBUG_LOG("UpdateAppsUid size:%{public}zu needCheckZeroVolume:%{public}d", appsUid_.size(),
        needCheckZeroVolume);
}

void HpaeFastRendererManager::NotifyStreamChangeToSink(
    StreamChangeType change, uint32_t sessionId, RendererState state, uint32_t appUid)
{
    CHECK_AND_RETURN(sinkOutputNode_ != nullptr);
    StreamUsage usage = STREAM_USAGE_UNKNOWN;
    if (sinkInputNodeMap_.find(sessionId) != sinkInputNodeMap_.end()) {
        usage = AudioTypeUtils::GetStreamUsageByStreamType(sinkInputNodeMap_[sessionId]->GetStreamType());
    }
    sinkOutputNode_->NotifyStreamChangeToSink(change, sessionId, usage, state, appUid);
}

void HpaeFastRendererManager::StopOutputNode()
{
    if (sinkOutputNode_ != nullptr) {
        if (mixerNode_ != nullptr) {
            sinkOutputNode_->DisConnect(mixerNode_);
        }
        sinkOutputNode_->RenderSinkStop();
        sinkOutputNode_->RenderSinkDeInit();
        sinkOutputNode_->ResetAll();
        sinkOutputNode_ = nullptr;
    }
}

int32_t HpaeFastRendererManager::StartRenderSink()
{
    CHECK_AND_RETURN_RET_LOG(sinkOutputNode_ != nullptr, ERROR, "sinkOutputNode_ is nullptr");
    UpdateAppsUid();
    return sinkOutputNode_->RenderSinkStart();
}

IAudioSinkAttr HpaeFastRendererManager::CreateSinkAttr()
{
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
    attr.audioStreamFlag = sinkInfo_.deviceClass == "primary_mmap_voip" ?
        AUDIO_FLAG_VOIP_FAST : AUDIO_FLAG_MMAP;
    attr.isLoopback = sinkInfo_.isLoopback;
    if (sinkInfo_.isUltraFast) {
        attr.period = static_cast<int32_t>(
            static_cast<float>(sinkInfo_.samplingRate) * ULTRA_FAST_PERIOD_TIME_IN_MS /
            static_cast<float>(MILLISECOND_PER_SECOND));
        AUDIO_INFO_LOG("UltraFast period set: %{public}u frames for rate %{public}u",
            attr.period, sinkInfo_.samplingRate);
    }
    return attr;
}

int32_t HpaeFastRendererManager::InitSinkInner(bool isReload)
{
    AUDIO_INFO_LOG("init devicename:%{public}s, channel:%{public}u, rate:%{public}u",
        GetEncryptStr(sinkInfo_.deviceName).c_str(), sinkInfo_.channels, sinkInfo_.samplingRate);
    HpaeNodeInfo nodeInfo;
    int32_t checkRet = CheckFramelen(sinkInfo_);
    if (checkRet != SUCCESS) {
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT,
            sinkInfo_.deviceName, ERR_INVALID_PARAM);
        return checkRet;
    }
    nodeInfo.channels = sinkInfo_.channels;
    nodeInfo.format = sinkInfo_.format;
    nodeInfo.frameLen = sinkInfo_.frameLen;
    nodeInfo.nodeId = 0;
    nodeInfo.samplingRate = sinkInfo_.samplingRate;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.statusCallback = weak_from_this();
    sinkOutputNode_ = std::make_unique<HpaeFastSinkOutputNode>(nodeInfo);
    sinkOutputNode_->SetTimeoutStopThd(sinkInfo_.suspendTime);
    AUDIO_INFO_LOG("GetRenderSinkInstance for fast path");
    sinkOutputNode_->GetRenderSinkInstance(sinkInfo_.deviceClass, sinkInfo_.deviceNetId);
    IAudioSinkAttr attr = CreateSinkAttr();
    AUDIO_INFO_LOG("InitSinkInner attr, deviceClass:%{public}s deviceType:%{public}d flag:%{public}u "
        "channelLayout:%{public}" PRIu64, sinkInfo_.deviceClass.c_str(), sinkInfo_.deviceType,
        attr.audioStreamFlag, sinkInfo_.channelLayout);
    int32_t ret = sinkOutputNode_->RenderSinkInit(attr);
    isInit_.store(ret == SUCCESS);
    TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sinkInfo_.deviceName, ret);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RenderSinkInit failed:%{public}d", ret);
    nodeInfo.frameLen = sinkOutputNode_->GetFrameLen();
    mixerNode_ = std::make_shared<HpaeMixerNode>(nodeInfo);
    sinkOutputNode_->Connect(mixerNode_);
    sinkOutputNode_->SetClientWakeCallback([this]() {
        for (auto &[sessionId, node] : sinkInputNodeMap_) {
            if (node->GetState() == HPAE_SESSION_RUNNING) {
                node->TriggerClientWake();
            }
        }
    });
    return SUCCESS;
}

int32_t HpaeFastRendererManager::Init(bool isReload)
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    hpaeSignalProcessThread_->SetFastThread(true);
    auto request = [this, isReload] {
        Trace trace("HpaeFastRendererManager::Init[" + std::to_string(isReload) + "]");
        InitSinkInner(isReload);
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeFastRendererManager::DeInit(bool isMoveDefault)
{
    Trace trace("HpaeFastRendererManager::DeInit[" + std::to_string(isMoveDefault) + "]");
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
    StopOutputNode();
    isInit_.store(false);
    return SUCCESS;
}

bool HpaeFastRendererManager::IsInit()
{
    return isInit_.load();
}

bool HpaeFastRendererManager::IsRunning(void)
{
    if (sinkOutputNode_ != nullptr && hpaeSignalProcessThread_ != nullptr) {
        return sinkOutputNode_->GetSinkState() == STREAM_MANAGER_RUNNING &&
            hpaeSignalProcessThread_->IsRunning();
    }
    return false;
}

bool HpaeFastRendererManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

bool HpaeFastRendererManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

void HpaeFastRendererManager::Process()
{
    if (sinkOutputNode_ != nullptr && IsRunning()) {
        UpdateAppsUid();
        if (appsUid_.empty()) {
            int64_t now = ClockTime::GetCurNano();
            noneStreamTime_ = noneStreamTime_ == 0 ? now : noneStreamTime_;
            if (now - noneStreamTime_ > SUSPEND_TIME_OUT_S * AUDIO_NS_PER_SECOND) {
                AUDIO_INFO_LOG("Process auto stop sink by empty apps, device:%{public}s timeout:%{public}f",
                    sinkInfo_.deviceName.c_str(), SUSPEND_TIME_OUT_S);
                sinkOutputNode_->RenderSinkStop();
                return;
            }
        } else {
            noneStreamTime_ = 0;
        }
        sinkOutputNode_->DoProcess();
    }
}

void HpaeFastRendererManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

int32_t HpaeFastRendererManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t checkRet = CheckStreamInfo(streamInfo);
    if (checkRet != SUCCESS) {
        return checkRet;
    }
    auto request = [this, streamInfo]() {
        Trace trace("HpaeFastRendererManager::CreateStream id[" + std::to_string(streamInfo.sessionId) + "]");
        auto node = CreateInputSession(streamInfo);
        node->SetState(HPAE_SESSION_PREPARED);
        node->SetIsLowLatency(true);
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, streamInfo.sessionId, RENDERER_PREPARED,
            node->GetAppUid());
        if (streamInfo.isLoopback) {
            loopbackSessionIds_.insert(streamInfo.sessionId);
            UpdateLoopbackState();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        Trace trace("HpaeFastRendererManager::DestroyStream id[" + std::to_string(sessionId) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "DestroyStream not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("DestroyStream sessionId %{public}u", sessionId);
        node->SetState(HPAE_SESSION_RELEASED);
        DeleteSessionNodes(sessionId);
        RemoveNodeFromMap(sessionId);
        loopbackSessionIds_.erase(sessionId);
        UpdateLoopbackState();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("HpaeFastRendererManager::Start id[" + std::to_string(sessionId) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Start not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Start sessionId %{public}u", sessionId);
        node->SetState(HPAE_SESSION_RUNNING);
        ConnectInputCluster(sessionId);
        SetSessionFade(sessionId, OPERATION_STARTED);
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, RENDERER_RUNNING);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::Pause(uint32_t sessionId, bool isStandby)
{
    auto request = [this, sessionId]() {
        Trace trace("HpaeFastRendererManager::Pause id[" + std::to_string(sessionId) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Pause not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Pause sessionId %{public}u", sessionId);
        if (!SetSessionFade(sessionId, OPERATION_PAUSED)) {
            DisConnectInputCluster(sessionId);
        }
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, RENDERER_PAUSED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::Flush(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("HpaeFastRendererManager::Flush id[" + std::to_string(sessionId) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Flush not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Flush sessionId %{public}u", sessionId);
        node->Flush();
        auto converter = SafeGetMap(converterNodeMap_, sessionId);
        if (converter != nullptr) {
            converter->FlushBuffers();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::Drain(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("HpaeFastRendererManager::Drain id[" + std::to_string(sessionId) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Drain not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Drain sessionId %{public}u", sessionId);
        node->Drain();
        if (node->GetState() != HPAE_SESSION_RUNNING) {
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId,
                node->GetState(), OPERATION_DRAINED);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("HpaeFastRendererManager::Stop id[" + std::to_string(sessionId) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Stop not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Stop sessionId %{public}u", sessionId);
        if (!SetSessionFade(sessionId, OPERATION_STOPPED)) {
            DisConnectInputCluster(sessionId);
        }
        NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_STATE_CHANGE, sessionId, RENDERER_STOPPED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::Release(uint32_t sessionId)
{
    return DestroyStream(sessionId);
}

void HpaeFastRendererManager::UpdateLoopbackState()
{
    bool isExistLoopback = !loopbackSessionIds_.empty();
    isExistLoopback_.store(isExistLoopback);
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->SetLoopbackState(isExistLoopback);
    }
}

void HpaeFastRendererManager::MoveStreamSync(uint32_t sessionId, const std::string &sinkName)
{
    Trace trace("HpaeFastRendererManager::MoveStreamSync[" + std::to_string(sessionId) + "->" + sinkName + "]");
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    if (node == nullptr) {
        AUDIO_ERR_LOG("[StartMove] session:%{public}u failed, sink [fast] --> [%{public}s]",
            sessionId, sinkName.c_str());
        TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
        HpaeStreamMoveMonitor::ReportStreamMoveException(0, sessionId, HPAE_STREAM_CLASS_TYPE_PLAY,
            "fast", sinkName, "not find session node");
        return;
    }

    if (sinkName.empty()) {
        AUDIO_ERR_LOG("[StartMove] session:%{public}u failed, sinkName is empty", sessionId);
        TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
        HpaeStreamMoveMonitor::ReportStreamMoveException(node->GetAppUid(), sessionId,
            HPAE_STREAM_CLASS_TYPE_PLAY, "fast", sinkName, "sinkName is empty");
        return;
    }

    AUDIO_INFO_LOG("move session:%{public}d, sink [fast] --> [%{public}s]", sessionId, sinkName.c_str());
    TriggerStreamState(sessionId, node);
    DeleteSessionNodes(sessionId);
    RemoveNodeFromMap(sessionId);
    std::string name = sinkName;
    TriggerCallback(MOVE_SINK_INPUT, node, name);
}

int32_t HpaeFastRendererManager::MoveStream(uint32_t sessionId, const std::string &sinkName)
{
    if (!IsInit()) {
        MoveStreamSync(sessionId, sinkName);
    } else {
        auto request = [this, sessionId, sinkName]() { MoveStreamSync(sessionId, sinkName); };
        SendRequest(request, __func__);
    }
    return SUCCESS;
}

int32_t HpaeFastRendererManager::MoveAllStream(const std::string &sinkName,
    const std::vector<uint32_t> &sessionIds, MoveSessionType moveType)
{
    if (!IsInit()) {
        AUDIO_INFO_LOG("sink is not init, use sync mode move to:%{public}s.", sinkName.c_str());
        MoveAllStreamToNewSink(sinkName, sessionIds, moveType);
    } else {
        AUDIO_INFO_LOG("sink is init, use async mode move to:%{public}s.", sinkName.c_str());
        auto request = [this, sinkName, sessionIds, moveType]() {
            MoveAllStreamToNewSink(sinkName, sessionIds, moveType);
        };
        SendRequest(request, __func__);
    }
    return SUCCESS;
}

int32_t HpaeFastRendererManager::AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    auto request = [this, node]() { AddSingleNodeToSink(node); };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::AddAllNodesToSink(
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

int32_t HpaeFastRendererManager::SuspendStreamManager(bool isSuspend)
{
    auto request = [this, isSuspend]() {
        Trace trace("HpaeFastRendererManager::SuspendStreamManager[" + std::to_string(isSuspend) + "]");
        if (isSuspend_ == isSuspend) {
            return;
        }
        isSuspend_ = isSuspend;
        if (isSuspend_) {
            if (sinkOutputNode_ != nullptr) {
                sinkOutputNode_->RenderSinkStop();
            }
        } else if (sinkOutputNode_ != nullptr && sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING &&
            CheckIsStreamRunning()) {
            UpdateAppsUid();
            sinkOutputNode_->RenderSinkStart();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::SetMute(bool isMute)
{
    auto request = [this, isMute]() {
        isMute_ = isMute;
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeFastRendererManager::SetMuteForSwitchDevice(bool mute)
{
    auto request = [this, mute]() {
        CHECK_AND_RETURN_LOG(sinkOutputNode_ != nullptr, "sinkOutputNode_ is nullptr");
        sinkOutputNode_->SetMuteForSwitchDevice(mute);
    };
    SendRequest(request, __func__);
}

int32_t HpaeFastRendererManager::StopManager()
{
    auto request = [this] {
        Trace trace("StopManager");
        CHECK_AND_RETURN_LOG(sinkOutputNode_ != nullptr, "sink output node is nullptr");
        sinkOutputNode_->RenderSinkStop();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}


int32_t HpaeFastRendererManager::SetClientVolume(uint32_t sessionId, float volume)
{
    auto request = [this, sessionId, volume]() {
        auto gainNode = SafeGetMap(gainNodeMap_, sessionId);
        if (gainNode != nullptr) {
            gainNode->SetClientVolume(volume);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    return SUCCESS;
}

int32_t HpaeFastRendererManager::SetRate(uint32_t sessionId, int32_t rate)
{
    return SUCCESS;
}

int32_t HpaeFastRendererManager::SetAudioEffectMode(uint32_t sessionId, int32_t effectMode)
{
    return SUCCESS;
}

int32_t HpaeFastRendererManager::GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode)
{
    return SUCCESS;
}

int32_t HpaeFastRendererManager::SetPrivacyType(uint32_t sessionId, int32_t privacyType)
{
    return SUCCESS;
}

int32_t HpaeFastRendererManager::GetPrivacyType(uint32_t sessionId, int32_t &privacyType)
{
    return SUCCESS;
}

void HpaeFastRendererManager::SetSpeed(uint32_t sessionId, float speed)
{
    auto request = [this, sessionId, speed]() {
        Trace trace("HpaeFastRendererManager::SetSpeed id[" + std::to_string(sessionId) +
            "] speed[" + std::to_string(speed) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "SetSpeed not find sessionId %{public}u", sessionId);
        node->SetSpeed(speed);
    };
    SendRequest(request, __func__);
}

size_t HpaeFastRendererManager::GetWritableSize(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeFastRendererManager::UpdateSpatializationState(
    uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

int32_t HpaeFastRendererManager::UpdateMaxLength(uint32_t sessionId, uint32_t maxLength)
{
    return SUCCESS;
}

int32_t HpaeFastRendererManager::RegisterWriteCallback(
    uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node != nullptr, "RegisterWriteCallback not find sessionId %{public}u", sessionId);
        node->RegisterWriteCallback(callback);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::RegisterReadCallback(
    uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    return ERR_NOT_SUPPORTED;
}

void HpaeFastRendererManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId,
        node ? node->GetState() : HPAE_SESSION_INVALID, operation);
}

void HpaeFastRendererManager::OnFadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeRendererManager::OnFadeDone");
        CHECK_AND_RETURN_LOG(SafeGetMap(sinkInputNodeMap_, sessionId),
            "Fade done, not find sessionId %{public}u", sessionId);
        CHECK_AND_RETURN_LOG(sinkInputNodeMap_[sessionId]->GetState() != HPAE_SESSION_RUNNING,
            "no need disconnect renderer input session, %{public}u is running", sessionId);
        AUDIO_INFO_LOG("Fade done, call back at RendererManager");
        DisConnectInputCluster(sessionId);
        IOperation operation = sinkInputNodeMap_[sessionId]->GetState() == HPAE_SESSION_STOPPING ?
            OPERATION_STOPPED : OPERATION_PAUSED;
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
        sinkInputNodeMap_[sessionId]->SetState(state);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
    };
    SendRequest(request, __func__);
}
void HpaeFastRendererManager::OnRequestLatency(uint32_t sessionId, uint64_t &latency)
{
    uint64_t processLatency = 0;
    auto converter = SafeGetMap(converterNodeMap_, sessionId);
    CHECK_AND_RETURN_LOG(converter != nullptr, "converter node not found for sessionId %{public}u", sessionId);
    processLatency += converter->GetLatency();
    if (mixerNode_) {
        processLatency += mixerNode_->GetLatency(sessionId);
    }
    // Align with legacy AudioEndpoint behavior:
    // fast playback data path should not query HDI sink latency on each process cycle.
    // Sink/device latency is fetched on explicit latency/position query path in HpaeRendererStreamImpl.
    latency += processLatency;
}

void HpaeFastRendererManager::OnRewindAndFlush(uint64_t rewindTime, uint64_t hdiFramePosition)
{
    for (auto &[sessionId, node] : sinkInputNodeMap_) {
        if (node->GetState() == HPAE_SESSION_RUNNING) {
            node->RewindHistoryBuffer(rewindTime, hdiFramePosition);
        }
    }
}

void HpaeFastRendererManager::OnNotifyQueue()
{
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ fast renderer is nullptr");
    hpaeSignalProcessThread_->Notify();
}

std::vector<SinkInput> HpaeFastRendererManager::GetAllSinkInputsInfo()
{
    std::vector<SinkInput> sinkInputs;
    return sinkInputs;
}

int32_t HpaeFastRendererManager::GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(node, ERR_INVALID_OPERATION,
        "GetSinkInputInfo not find sessionId %{public}u", sessionId);
    sinkInputInfo.nodeInfo = node->GetNodeInfo();
    sinkInputInfo.rendererSessionInfo.state = node->GetState();
    return SUCCESS;
}

int32_t HpaeFastRendererManager::GetSpanSizeInFrame(uint32_t sessionId, uint32_t &spanSizeInFrame)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(node, ERR_INVALID_OPERATION,
        "GetSpanSizeInFrame not find sessionId %{public}u", sessionId);
    spanSizeInFrame = static_cast<uint32_t>(node->GetNodeInfo().frameLen);
    return SUCCESS;
}

int32_t HpaeFastRendererManager::RefreshProcessClusterByDevice()
{
    return SUCCESS;
}

HpaeSinkInfo HpaeFastRendererManager::GetSinkInfo()
{
    return sinkInfo_;
}

std::string HpaeFastRendererManager::GetThreadName()
{
    return sinkInfo_.deviceName;
}

int32_t HpaeFastRendererManager::DumpSinkInfo()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "HpaeFastRendererManager not init");
    auto request = [this]() {
        AUDIO_INFO_LOG("DumpSinkInfo deviceName %{public}s", sinkInfo_.deviceName.c_str());
        UploadDumpSinkInfo(sinkInfo_.deviceName);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

std::string HpaeFastRendererManager::GetDeviceHDFDumpInfo()
{
    std::string config;
    TransDeviceInfoToString(sinkInfo_, config);
    return config;
}

float HpaeFastRendererManager::GetMaxAmplitude()
{
    return sinkOutputNode_ != nullptr ? sinkOutputNode_->GetMaxAmplitude() : 0.0f;
}

int32_t HpaeFastRendererManager::ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload)
{
    if (!IsInit()) {
        hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
        hpaeSignalProcessThread_->SetFastThread(true);
    }
    auto request = [this, sinkInfo, isReload]() {
        Trace trace("HpaeFastRendererManager::ReloadRenderManager[" + std::to_string(isReload) + "]");
        AUDIO_INFO_LOG("reload fast");
        StopOutputNode();
        
        for (const auto &it : sinkInputNodeMap_) {
            TriggerStreamState(it.first, it.second);
            DisConnectInputCluster(it.first);
            DeleteSessionNodes(it.first);
        }
        sinkInfo_ = sinkInfo;
        InitSinkInner(isReload);

        for (auto &[sessionId, node] : sinkInputNodeMap_) {
            CreateFastNodes(node->GetNodeInfo());
            if (node->GetState() == HPAE_SESSION_RUNNING) {
                ConnectInputCluster(sessionId);
            }
            NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_ADD, sessionId,
                ConvertHpaeToRendererState(node->GetState()));
        }
    };
    SendRequest(request, __func__, true);
    if (!IsInit()) {
        hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    }
    return SUCCESS;
}

void HpaeFastRendererManager::TriggerStreamState(uint32_t sessionId,
    const std::shared_ptr<HpaeSinkInputNode> &inputNode)
{
    HpaeSessionState inputState = inputNode->GetState();
    if (inputState == HPAE_SESSION_STOPPING || inputState == HPAE_SESSION_PAUSING) {
        HpaeSessionState state = inputState == HPAE_SESSION_PAUSING ? HPAE_SESSION_PAUSED : HPAE_SESSION_STOPPED;
        IOperation operation = inputState == HPAE_SESSION_PAUSING ? OPERATION_PAUSED : OPERATION_STOPPED;
        inputNode->SetState(state);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
    }
}

void HpaeFastRendererManager::TriggerAppsUidUpdate(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        AUDIO_INFO_LOG("TriggerAppsUidUpdate sessionId: %{public}u", sessionId);
        appsUid_.clear();
        for (auto &[id, node] : sinkInputNodeMap_) {
            if (node->GetState() == HPAE_SESSION_RUNNING || id == sessionId) {
                appsUid_.emplace_back(node->GetAppUid());
            }
        }
        if (sinkOutputNode_ != nullptr) {
            sinkOutputNode_->UpdateAppsUid(appsUid_);
        }
    };
    SendRequest(request, __func__);
}

void HpaeFastRendererManager::SendRequest(Request &&request, const std::string &funcName, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_ERR_LOG("HpaeFastRendererManager not init, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_RENDERER_MANAGER_TYPE, funcName,
            "HpaeFastRendererManager not init");
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

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
