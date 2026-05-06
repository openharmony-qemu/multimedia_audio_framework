/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "HpaeDirectRendererManager"
#endif

#include "hpae_direct_renderer_manager.h"
#include "audio_stream_info.h"
#include "audio_errors.h"
#include "hpae_node_common.h"
#include "audio_engine_log.h"
#include "hpae_message_queue_monitor.h"
#include "hpae_stream_move_monitor.h"
#include "audio_utils.h"
#include "hpae_info.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
const std::string VOIP_DIRECT_DEVICE_CLASS = "primary_direct_voip";
constexpr int32_t MILLISECONDS_PER_SECOND = 1000;
constexpr int32_t MILLISECONDS_20MS = 20;
static constexpr uint32_t DIRECT_SINK_STANDBY_TIMES = 8;
static constexpr uint32_t PERIOD_NS = 20000000; // 20ms

AudioSamplingRate HpaeDirectRendererManager::GetDirectSampleRate(AudioSamplingRate sampleRate, bool isVoip)
{
    if (isVoip) {
        // VoIP stream type. Return the special sample rate of direct VoIP mode.
        if (sampleRate <= AudioSamplingRate::SAMPLE_RATE_16000) {
            return AudioSamplingRate::SAMPLE_RATE_16000;
        } else {
            return AudioSamplingRate::SAMPLE_RATE_48000;
        }
    }
    // High resolution for music
    AudioSamplingRate result = sampleRate;
    switch (sampleRate) {
        case AudioSamplingRate::SAMPLE_RATE_44100:
            result = AudioSamplingRate::SAMPLE_RATE_48000;
            break;
        case AudioSamplingRate::SAMPLE_RATE_88200:
            result = AudioSamplingRate::SAMPLE_RATE_96000;
            break;
        case AudioSamplingRate::SAMPLE_RATE_176400:
            result = AudioSamplingRate::SAMPLE_RATE_192000;
            break;
        default:
            break;
    }
    return result;
}

AudioSampleFormat HpaeDirectRendererManager::GetDirectFormat(AudioSampleFormat format, bool isVoip)
{
    if (!isVoip) {
        // Only SAMPLE_S32LE is supported for high resolution stream.
        return AudioSampleFormat::SAMPLE_S32LE;
    }

    // Both SAMPLE_S16LE and SAMPLE_S32LE are supported for direct VoIP stream.
    if (format == SAMPLE_S16LE || format == SAMPLE_S32LE) {
        return format;
    } else if (format == SAMPLE_F32LE) {
        // Direct VoIP not support SAMPLE_F32LE format. It needs to be converted to S16.
        return AudioSampleFormat::SAMPLE_S16LE;
    } else {
        AUDIO_WARNING_LOG("The format %{public}u is unsupported for direct VoIP. Use 32Bit.", format);
        return AudioSampleFormat::SAMPLE_S32LE;
    }
}

HpaeDirectRendererManager::HpaeDirectRendererManager(HpaeSinkInfo &sinkInfo)
    : hpaeNoLockQueue_(CURRENT_REQUEST_COUNT), sinkInfo_(sinkInfo)
{
}

HpaeDirectRendererManager::~HpaeDirectRendererManager()
{
    AUDIO_INFO_LOG("destructor");
    if (isInit_.load()) {
        DeInit();
    }
}

std::shared_ptr<HpaeSinkInputNode> HpaeDirectRendererManager::CreateInputSession(const HpaeStreamInfo &streamInfo)
{
    HpaeNodeInfo nodeInfo;
    ConfigNodeInfo(nodeInfo, streamInfo);
    nodeInfo.sceneType = TransStreamTypeToSceneType(streamInfo.streamType);
    nodeInfo.statusCallback = weak_from_this();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.effectInfo = streamInfo.effectInfo;
    AUDIO_INFO_LOG("streamType %{public}u, sessionId = %{public}u,channels:%{public}u,rate:%{public}u",
        nodeInfo.streamType, nodeInfo.sessionId, nodeInfo.channels,
        nodeInfo.customSampleRate == 0 ? nodeInfo.samplingRate : nodeInfo.customSampleRate);
    auto sinkInputNode = std::make_shared<HpaeSinkInputNode>(nodeInfo);
    sinkInputNode->SetAppUid(streamInfo.uid);
    AddNodeToMap(sinkInputNode);
    return sinkInputNode;
}

int32_t HpaeDirectRendererManager::AddNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node)
{
    auto request = [this, node]() { AddSingleNodeToSink(node); };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeDirectRendererManager::AddNodeToMap(std::shared_ptr<HpaeSinkInputNode> node)
{
    node->SetDirect(true);
    sinkInputNodeMap_[node->GetSessionId()] = node;
    if (curNode_ == nullptr) {
        curNode_ = node;
        CreateDirectNodes();
    }
}

void HpaeDirectRendererManager::RemoveNodeFromMap(uint32_t sessionId)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    HpaeSessionState inputState = HPAE_SESSION_RELEASED;
    if (node != nullptr) {
        inputState = node->GetState();
        node->SetDirect(false);
#ifdef ENABLE_HIDUMP_DFX
        OnNotifyDfxNodeAdmin(false, node->GetNodeInfo());
#endif
    }
    RendererState state = inputState == HPAE_SESSION_RELEASED ? RENDERER_INVALID : RENDERER_RELEASED;
    NotifyStreamChangeToSink(STREAM_CHANGE_TYPE_REMOVE, sessionId, state);
    sinkInputNodeMap_.erase(sessionId);
    if (curNode_ && curNode_->GetSessionId() == sessionId) {
        DestroyDirectNodes();
        curNode_ = nullptr;
    }
}

void HpaeDirectRendererManager::SetCurrentNode()
{
    if (curNode_ != nullptr) {
        AUDIO_WARNING_LOG("curNode_ in exist, no need to set");
        return;
    }
    for (auto [_, node]: sinkInputNodeMap_) {
        curNode_ = node;
        CreateDirectNodes();
        if (curNode_->GetState() == HPAE_SESSION_RUNNING) {
            ConnectInputSession();
        }
        break;
    }
    AUDIO_INFO_LOG("now curNode_ is [%{public}u]", curNode_ ? curNode_->GetSessionId() : 0);
}

void HpaeDirectRendererManager::AddSingleNodeToSink(const std::shared_ptr<HpaeSinkInputNode> &node, bool isConnect)
{
    HpaeNodeInfo nodeInfo = node->GetNodeInfo();
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.statusCallback = weak_from_this();
    node->SetNodeInfo(nodeInfo);
    uint32_t sessionId = nodeInfo.sessionId;
    AUDIO_INFO_LOG("[FinishMove] session:%{public}u to sink:direct", sessionId);
    AddNodeToMap(node);
#ifdef ENABLE_HIDUMP_DFX
    OnNotifyDfxNodeAdmin(true, nodeInfo);
#endif
    if (!isConnect || node->GetState() != HPAE_SESSION_RUNNING) {
        AUDIO_INFO_LOG("[FinishMove] session:%{public}u not need connect session", sessionId);
        return;
    }

    if (node->GetState() == HPAE_SESSION_RUNNING && node->GetSessionId() == curNode_->GetSessionId()) {
        AUDIO_INFO_LOG("[FinishMove] session:%{public}u connect to sink:direct", sessionId);
        ConnectInputSession();
    }
    node->OnStreamInfoChange(false);
}

int32_t HpaeDirectRendererManager::AddAllNodesToSink(
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

int32_t HpaeDirectRendererManager::CreateStream(const HpaeStreamInfo &streamInfo)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    int32_t checkRet = CheckStreamInfo(streamInfo);
    if (checkRet != SUCCESS) {
        return checkRet;
    }
    auto request = [this, streamInfo]() {
        auto node = CreateInputSession(streamInfo);
        node->SetState(HPAE_SESSION_PREPARED);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeDirectRendererManager::DeleteInputSession()
{
    DisConnectInputSession();
    CHECK_AND_RETURN_LOG(curNode_ != nullptr, "curNode_ not exist");
    RemoveNodeFromMap(curNode_->GetSessionId());
}

int32_t HpaeDirectRendererManager::DestroyStream(uint32_t sessionId)
{
    if (!IsInit()) {
        return ERR_INVALID_OPERATION;
    }
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::DestroyStream");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "DestroyStream not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("DestroyStream sessionId %{public}u", sessionId);
        if (sessionId == curNode_->GetSessionId()) {
            DeleteInputSession();
            SetCurrentNode();
        } else {
            RemoveNodeFromMap(sessionId);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::RecreateSinkOutputNodeIfNeeded()
{
    bool isVoip = (sinkInfo_.deviceClass == VOIP_DIRECT_DEVICE_CLASS);
    AudioSamplingRate newSampleRate = GetDirectSampleRate(curNode_->GetSampleRate(), isVoip);
    AudioSampleFormat newFormat = GetDirectFormat(curNode_->GetBitWidth(), isVoip);
    AudioChannel newChannels = (curNode_->GetChannelCount() > STEREO) ?
        AudioChannel::STEREO : curNode_->GetChannelCount();
    HpaeNodeInfo outputNodeInfo = sinkOutputNode_->GetNodeInfo();
    if (outputNodeInfo.samplingRate == newSampleRate && outputNodeInfo.format == newFormat &&
        outputNodeInfo.channels == newChannels) {
        return SUCCESS;
    }
    AUDIO_INFO_LOG("SessionId %{public}u, config changed: rate[%{public}u->%{public}u], "
        "format[%{public}u->%{public}u], channels[%{public}u->%{public}u], recreate sinkOutputNode",
        outputNodeInfo.sessionId, outputNodeInfo.samplingRate, newSampleRate,
        outputNodeInfo.format, newFormat, outputNodeInfo.channels, newChannels);
    StopOuputNode();
    sinkInfo_.samplingRate = newSampleRate;
    sinkInfo_.format = newFormat;
    sinkInfo_.channels = newChannels;
    sinkInfo_.channelLayout = newChannels == STEREO ? STEREO : MONO;
    int32_t ret = InitSinkInner(false, false);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "recreate direct sink failed when create stream:%{public}u.",
        outputNodeInfo.sessionId);
    return ret;
}

void HpaeDirectRendererManager::CreateProcessNodes(uint32_t sessionId)
{
    HpaeNodeInfo nodeInfo = sinkOutputNode_->GetNodeInfo();
    nodeInfo.sessionId = sessionId;
    nodeInfo.streamType = curNode_->GetStreamType();
    HpaeNodeInfo preNodeInfo = curNode_->GetNodeInfo();
    converterForOutput_ = std::make_shared<HpaeAudioFormatConverterNode>(nodeInfo, sinkOutputNode_->GetNodeInfo());
    converterForOutput_->SetDownmixNormalization(false);

    if (curNode_->GetChannelCount() > STEREO) {
        limiterNode_ = std::make_shared<HpaeLimiterNode>(nodeInfo);
        if (limiterNode_->SetupAudioLimiter() != SUCCESS) {
            AUDIO_WARNING_LOG("SessionId %{public}u, limiterNode_ SetupAudioLimiter failed!", sessionId);
        }
    } else {
        limiterNode_ = nullptr;
        AUDIO_INFO_LOG("SessionId %{public}u, skip limiterNode for channels=%{public}u (<= STEREO)",
            sessionId, nodeInfo.channels);
    }
    if (sinkInfo_.deviceClass == VOIP_DIRECT_DEVICE_CLASS) {
        volumeSyncNode_ = std::make_shared<HpaeVolumeSyncNode>(nodeInfo);
        volumeSyncNode_->ResetVolume();
        AUDIO_INFO_LOG("SessionId %{public}u, using HpaeVolumeSyncNode for VOIP device", sessionId);
    } else {
        gainNode_ = std::make_shared<HpaeGainNode>(nodeInfo);
        gainNode_->ResetVolume();
    }
    converterForGain_ = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, nodeInfo);
    converterForGain_->SetDownmixNormalization(false);
}

int32_t HpaeDirectRendererManager::CreateDirectNodes()
{
    CHECK_AND_RETURN_RET_LOG(curNode_ != nullptr, ERROR, "curNode_ not exist, fail to create direct nodes");

    int32_t ret = RecreateSinkOutputNodeIfNeeded();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RecreateSinkOutputNodeIfNeeded failed");

    CreateProcessNodes(curNode_->GetSessionId());

    AUDIO_INFO_LOG("SessionId %{public}u, Success create direct nodes: "
        "converterForGainId %{public}u, gainNodeId %{public}u, volumeSyncNodeId %{public}u, limiterNodeId %{public}u,"
        " converterForOutputNodeId %{public}u", curNode_->GetSessionId(), converterForGain_->GetNodeId(),
        gainNode_ ? gainNode_->GetNodeId() : 0, volumeSyncNode_ ? volumeSyncNode_->GetNodeId() : 0,
        limiterNode_ ? limiterNode_->GetNodeId() : 0, converterForOutput_->GetNodeId());
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::DestroyDirectNodes()
{
    CHECK_AND_RETURN_RET_LOG(curNode_ != nullptr && converterForGain_ != nullptr &&
        (gainNode_ != nullptr || volumeSyncNode_ != nullptr) &&
        converterForOutput_ != nullptr, ERROR,
        "direct nodes not exist, fail to destroy direct nodes");
    AUDIO_INFO_LOG("SessionId %{public}u, Success destroy direct nodes: "
        "converterForGainId %{public}u, gainNodeId %{public}u, volumeSyncNodeId %{public}u, limiterNodeId %{public}u,"
        " converterForOutputNodeId %{public}u ",
        curNode_->GetSessionId(), converterForGain_->GetNodeId(),
        gainNode_ ? gainNode_->GetNodeId() : 0,
        volumeSyncNode_ ? volumeSyncNode_->GetNodeId() : 0,
        limiterNode_ ? limiterNode_->GetNodeId() : 0, converterForOutput_->GetNodeId());
    converterForGain_ = nullptr;
    gainNode_ = nullptr;
    volumeSyncNode_ = nullptr;
    converterForOutput_ = nullptr;
    limiterNode_ = nullptr;
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::ConnectInputSession()
{
    if (curNode_->GetState() != HPAE_SESSION_RUNNING) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(sinkOutputNode_->GetSinkState() != STREAM_MANAGER_NEW, ERROR,
        "sink output node is not init");
    // Connect nodes: sinkOutputNode <- converterForOutput <- [limiterNode]
    // <- gainNode/volumeSyncNode <- converterForGain <- curNode
    // Note: limiterNode is only present when channels > STEREO
    sinkOutputNode_->Connect(converterForOutput_);
    if (limiterNode_ != nullptr) {
        converterForOutput_->Connect(limiterNode_);
        if (volumeSyncNode_ != nullptr) {
            limiterNode_->Connect(volumeSyncNode_);
            volumeSyncNode_->Connect(converterForGain_);
        } else {
            limiterNode_->Connect(gainNode_);
            gainNode_->Connect(converterForGain_);
        }
    } else {
        if (volumeSyncNode_ != nullptr) {
            converterForOutput_->Connect(volumeSyncNode_);
            volumeSyncNode_->Connect(converterForGain_);
        } else {
            converterForOutput_->Connect(gainNode_);
            gainNode_->Connect(converterForGain_);
        }
    }
    converterForGain_->Connect(curNode_);

    if (sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING && !isSuspend_) {
        sinkOutputNode_->RenderSinkStart();
    }
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::Start(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::Start");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Start not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Start sessionId %{public}u", sessionId);
        node->SetState(HPAE_SESSION_RUNNING);
        if (sessionId == curNode_->GetSessionId()) {
            ConnectInputSession();
            SetSessionFade(sessionId, OPERATION_STARTED);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::DisConnectInputSession(bool isStandby)
{
    CHECK_AND_RETURN_RET(converterForGain_, SUCCESS);
    converterForGain_->DisConnect(curNode_);
    if (limiterNode_ != nullptr) {
        if (volumeSyncNode_ != nullptr) {
            volumeSyncNode_->DisConnect(converterForGain_);
            limiterNode_->DisConnect(volumeSyncNode_);
        } else {
            gainNode_->DisConnect(converterForGain_);
            limiterNode_->DisConnect(gainNode_);
        }
        converterForOutput_->DisConnect(limiterNode_);
    } else {
        if (volumeSyncNode_ != nullptr) {
            volumeSyncNode_->DisConnect(converterForGain_);
            converterForOutput_->DisConnect(volumeSyncNode_);
        } else {
            gainNode_->DisConnect(converterForGain_);
            converterForOutput_->DisConnect(gainNode_);
        }
    }
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->DisConnect(converterForOutput_);
        if (!isStandby && sinkOutputNode_->GetSinkState() == STREAM_MANAGER_RUNNING) {
            ClockTime::RelativeSleep(PERIOD_NS * DIRECT_SINK_STANDBY_TIMES);
        }
        sinkOutputNode_->RenderSinkStop();
    }
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::Pause(uint32_t sessionId, bool isStandby)
{
    auto request = [this, sessionId, isStandby]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::Pause");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Pause not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Pause sessionId %{public}u isStandby %{public}d", sessionId, isStandby);
        if (curNode_ != nullptr && sessionId == curNode_->GetSessionId()) {
            if (isStandby) {
                DisConnectInputSession(isStandby);
                node->SetState(HPAE_SESSION_PAUSED);
                TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId,
                    node->GetState(), OPERATION_PAUSED);
            } else if (!SetSessionFade(sessionId, OPERATION_PAUSED)) {
                DisConnectInputSession();
            }
        } else {
            node->SetState(HPAE_SESSION_PAUSED);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, node->GetState(), OPERATION_PAUSED);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::Flush(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::Flush");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Flush not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Flush sessionId %{public}u", sessionId);
        node->Flush();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::Drain(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::Drain");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Drain not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Drain sessionId %{public}u", sessionId);
        node->Drain();
        if (node->GetState() != HPAE_SESSION_RUNNING) {
            TriggerCallback(
                UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, node->GetState(), OPERATION_DRAINED);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::Stop(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::Stop");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "Stop not find sessionId %{public}u", sessionId);
        AUDIO_INFO_LOG("Stop sessionId %{public}u", sessionId);
        if (curNode_ != nullptr && sessionId == curNode_->GetSessionId()) {
            if (!SetSessionFade(sessionId, OPERATION_STOPPED)) {
                DisConnectInputSession();
            }
        } else {
            node->SetState(HPAE_SESSION_STOPPED);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, node->GetState(), OPERATION_STOPPED);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::Release(uint32_t sessionId)
{
    return DestroyStream(sessionId);
}

void HpaeDirectRendererManager::MoveAllStreamToNewSink(const std::string &sinkName,
    const std::vector<uint32_t>& moveIds, MoveSessionType moveType)
{
    Trace trace("HpaeDirectRendererManager::MoveAllStreamToNewSink[" +
        sinkName + "]_moveType[" + std::to_string(moveType) + "]");
    std::string name = sinkName;
    std::vector<std::shared_ptr<HpaeSinkInputNode>> sinkInputs;

    for (auto [sessionId, node]: sinkInputNodeMap_) {
        if (moveType == MOVE_ALL || std::find(moveIds.begin(), moveIds.end(), sessionId) != moveIds.end()) {
            sinkInputs.emplace_back(node);
            AUDIO_INFO_LOG("[StartMove] session: %{public}u,sink [direct] --> [%{public}s]",
                sessionId, sinkName.c_str());
        }
    }
    for (auto node: sinkInputs) {
        if (curNode_ && node->GetSessionId() == curNode_->GetSessionId()) {
            DeleteInputSession();
        } else {
            RemoveNodeFromMap(node->GetSessionId());
        }
    }

    if (sinkInputs.size() == 0) {
        AUDIO_WARNING_LOG("sink count is 0,no need move session");
    }
    if (moveType == MOVE_ALL) {
        TriggerSyncCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
    } else {
        TriggerCallback(MOVE_ALL_SINK_INPUT, sinkInputs, name, moveType);
    }
}

int32_t HpaeDirectRendererManager::MoveAllStream(const std::string &sinkName, const std::vector<uint32_t>& sessionIds,
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

int32_t HpaeDirectRendererManager::MoveStream(uint32_t sessionId, const std::string &sinkName)
{
    auto request = [this, sessionId, sinkName]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::MoveStream to [" +
            sinkName + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        if (node == nullptr) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}d failed,sink [direct] --> [%{public}s]",
                sessionId, sinkName.c_str());
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
            HpaeStreamMoveMonitor::ReportStreamMoveException(0, sessionId, HPAE_STREAM_CLASS_TYPE_PLAY,
                "direct", sinkName, "not find session node");
            return;
        }

        if (sinkName.empty()) {
            AUDIO_ERR_LOG("[StartMove] session:%{public}u failed,sinkName is empty", sessionId);
            TriggerCallback(MOVE_SESSION_FAILED, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, MOVE_SINGLE, sinkName);
            HpaeStreamMoveMonitor::ReportStreamMoveException(node->GetAppUid(), sessionId,
                HPAE_STREAM_CLASS_TYPE_PLAY, "direct", sinkName, "sinkName is empty");
            return;
        }
        AUDIO_INFO_LOG("move session:%{public}d,sink [direct] --> [%{public}s]", sessionId, sinkName.c_str());
        if (sessionId == curNode_->GetSessionId()) {
            DeleteInputSession();
            SetCurrentNode();
        } else {
            RemoveNodeFromMap(sessionId);
        }
        std::string name = sinkName;
        TriggerCallback(MOVE_SINK_INPUT, node, name);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::SuspendStreamManager(bool isSuspend)
{
    auto request = [this, isSuspend]() {
        Trace trace("HpaeDirectRendererManager::SuspendStreamManager[" + std::to_string(isSuspend) + "]");
        if (isSuspend_ == isSuspend) {
            return;
        }
        CHECK_AND_RETURN_LOG(sinkOutputNode_ != nullptr, "sink output node is nullptr");
        isSuspend_ = isSuspend;
        if (isSuspend_) {
            sinkOutputNode_->RenderSinkStop();
        } else if (sinkOutputNode_->GetSinkState() != STREAM_MANAGER_RUNNING && curNode_ &&
            curNode_->GetState() == HPAE_SESSION_RUNNING) {
            UpdateAppsUid();
            sinkOutputNode_->RenderSinkStart();
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::StopManager()
{
    auto request = [this] {
        Trace trace("StopManager");
        CHECK_AND_RETURN_LOG(sinkOutputNode_ != nullptr, "sink output node is nullptr");
        sinkOutputNode_->RenderSinkStop();
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::SetMute(bool isMute)
{
    auto request = [this, isMute]() {
        if (isMute_ != isMute) {
            isMute_ = isMute;
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

void HpaeDirectRendererManager::HandleMsg()
{
    hpaeNoLockQueue_.HandleRequests();
}

void HpaeDirectRendererManager::StopOuputNode()
{
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->RenderSinkStop();
        sinkOutputNode_->RenderSinkDeInit();
        sinkOutputNode_->ResetAll();
        sinkOutputNode_ = nullptr;
    }
}

int32_t HpaeDirectRendererManager::ReloadRenderManager(const HpaeSinkInfo &sinkInfo, bool isReload)
{
    if (!IsInit()) {
        hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    }
    auto request = [this, sinkInfo, isReload]() {
        Trace trace("HpaeDirectRendererManager::ReloadRenderManager[" + std::to_string(isReload) + "]");
        AUDIO_INFO_LOG("reload direct");
        if (curNode_ != nullptr && curNode_->GetState() == HPAE_SESSION_RUNNING) {
            DisConnectInputSession();
            DestroyDirectNodes();
        }
        StopOuputNode();
        sinkInfo_ = sinkInfo;
        InitSinkInner(isReload);

        CHECK_AND_RETURN_LOG(curNode_ != nullptr, "curNode_ is null");
        if (curNode_->GetState() == HPAE_SESSION_RUNNING) {
            CreateDirectNodes();
            ConnectInputSession();
        }
    };
    SendRequest(request, __func__, true);
    if (!IsInit()) {
        hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    }
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::Init(bool isReload)
{
    hpaeSignalProcessThread_ = std::make_unique<HpaeSignalProcessThread>();
    auto request = [this, isReload] {
        Trace trace("HpaeDirectRendererManager::Init[" + std::to_string(isReload) + "]");
        InitSinkInner(isReload);
    };
    SendRequest(request, __func__, true);
    hpaeSignalProcessThread_->ActivateThread(shared_from_this());
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::InitSinkInner(bool isReload, bool isCallback)
{
    AUDIO_INFO_LOG("init devicename:%{public}s,channel:%{public}u,rate:%{public}u",
        GetEncryptStr(sinkInfo_.deviceName).c_str(), sinkInfo_.channels, sinkInfo_.samplingRate);
    HpaeNodeInfo nodeInfo;
    int32_t checkRet = CheckFramelen(sinkInfo_);
    if (checkRet != SUCCESS) {
        if (!isCallback) {
            return checkRet;
        }
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT,
                        sinkInfo_.deviceName, ERR_INVALID_PARAM);
        return checkRet;
    }
    nodeInfo.channels = sinkInfo_.channels;
    nodeInfo.format = sinkInfo_.format;
    nodeInfo.frameLen = (sinkInfo_.samplingRate * MILLISECONDS_20MS) / MILLISECONDS_PER_SECOND;
    nodeInfo.nodeId = 0;
    nodeInfo.samplingRate = sinkInfo_.samplingRate;
    nodeInfo.sceneType = HPAE_SCENE_EFFECT_OUT;
    nodeInfo.deviceNetId = sinkInfo_.deviceNetId;
    nodeInfo.deviceClass = sinkInfo_.deviceClass;
    nodeInfo.statusCallback = weak_from_this();
    sinkOutputNode_ = std::make_unique<HpaeDirectSinkOutputNode>(nodeInfo);
    AUDIO_INFO_LOG("GetRenderSinkInstance");
    sinkOutputNode_->GetRenderSinkInstance(sinkInfo_.deviceClass, sinkInfo_.deviceNetId);
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
    int32_t ret = sinkOutputNode_->RenderSinkInit(attr);
    AUDIO_INFO_LOG("inited");
    if (isCallback) {
        TriggerCallback(isReload ? RELOAD_AUDIO_SINK_RESULT : INIT_DEVICE_RESULT, sinkInfo_.deviceName, ret);
        isInit_.store(true);
    } else {
        return ret;
    }
    return SUCCESS;
}

bool HpaeDirectRendererManager::DeactivateThread()
{
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->DeactivateThread();
        hpaeSignalProcessThread_ = nullptr;
    }
    hpaeNoLockQueue_.HandleRequests();
    return true;
}

int32_t HpaeDirectRendererManager::DeInit(bool isMoveDefault)
{
    Trace trace("HpaeDirectRendererManager::DeInit[" + std::to_string(isMoveDefault) + "]");
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
    StopOuputNode();

    isInit_.store(false);
    return SUCCESS;
}

bool HpaeDirectRendererManager::IsInit()
{
    return isInit_.load();
}

bool HpaeDirectRendererManager::IsRunning(void)
{
    if (sinkOutputNode_ != nullptr && hpaeSignalProcessThread_ != nullptr) {
        return sinkOutputNode_->GetSinkState() == STREAM_MANAGER_RUNNING && hpaeSignalProcessThread_->IsRunning();
    }
    return false;
}

bool HpaeDirectRendererManager::IsMsgProcessing()
{
    return !hpaeNoLockQueue_.IsFinishProcess();
}

int32_t HpaeDirectRendererManager::SetClientVolume(uint32_t sessionId, float volume)
{
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::SetRate(uint32_t sessionId, int32_t rate)
{
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::SetAudioEffectMode(uint32_t sessionId, int32_t effectMode)
{
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::GetAudioEffectMode(uint32_t sessionId, int32_t &effectMode)
{
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::SetPrivacyType(uint32_t sessionId, int32_t privacyType)
{
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::GetPrivacyType(uint32_t sessionId, int32_t &privacyType)
{
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::RegisterWriteCallback(
    uint32_t sessionId, const std::weak_ptr<IStreamCallback> &callback)
{
    auto request = [this, sessionId, callback]() {
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node != nullptr, "RegisterWriteCallback not find sessionId %{public}u",
            sessionId);
        node->RegisterWriteCallback(callback);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::RegisterReadCallback(
    uint32_t sessionId, const std::weak_ptr<ICapturerStreamCallback> &callback)
{
    return SUCCESS;
}

void HpaeDirectRendererManager::Process()
{
    if (sinkOutputNode_ != nullptr && IsRunning()) {
        sinkOutputNode_->DoProcess();
    }
}

int32_t HpaeDirectRendererManager::SetOffloadPolicy(uint32_t sessionId, int32_t state)
{
    auto request = [this, sessionId, state]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeOffloadRendererManager::SetOffloadPolicy[" +
            std::to_string(state) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "SetOffloadPolicy not find sessionId %{public}u", sessionId);
        node->SetOffloadEnabled(state != OFFLOAD_DEFAULT);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

size_t HpaeDirectRendererManager::GetWritableSize(uint32_t sessionId)
{
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::UpdateSpatializationState(
    uint32_t sessionId, bool spatializationEnabled, bool headTrackingEnabled)
{
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::UpdateMaxLength(uint32_t sessionId, uint32_t maxLength)
{
    return SUCCESS;
}

void HpaeDirectRendererManager::SetSpeed(uint32_t sessionId, float speed)
{
    // direct mode does not support speed change
}

std::vector<SinkInput> HpaeDirectRendererManager::GetAllSinkInputsInfo()
{
    std::vector<SinkInput> sinkInputs;
    return sinkInputs;
}

int32_t HpaeDirectRendererManager::GetSinkInputInfo(uint32_t sessionId, HpaeSinkInputInfo &sinkInputInfo)
{
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(node, ERR_INVALID_OPERATION,
        "GetSinkInputInfo not find sessionId %{public}u", sessionId);
    sinkInputInfo.nodeInfo = node->GetNodeInfo();
    sinkInputInfo.rendererSessionInfo.state = node->GetState();
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::RefreshProcessClusterByDevice()
{
    return SUCCESS;
}

HpaeSinkInfo HpaeDirectRendererManager::GetSinkInfo()
{
    return sinkInfo_;
}

void HpaeDirectRendererManager::SendRequest(Request &&request, const std::string &funcName, bool isInit)
{
    if (!isInit && !IsInit()) {
        AUDIO_ERR_LOG("HpaeDirectRendererManager not init, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_DIRECT_MANAGER_TYPE, funcName,
            "HpaeDirectRendererManager not init");
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    if (hpaeSignalProcessThread_ == nullptr) {
        AUDIO_ERR_LOG("hpaeSignalProcessThread_ is nullptr, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_DIRECT_MANAGER_TYPE, funcName, "thread is nullptr");
        return;
    }
    hpaeSignalProcessThread_->Notify();
}

void HpaeDirectRendererManager::OnFadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::OnFadeDone");
        CHECK_AND_RETURN_LOG(curNode_ != nullptr && curNode_->GetSessionId() == sessionId,
            "Fade done, sessionId %{public}u not match curNode_", sessionId);
        AUDIO_INFO_LOG("Fade done, callback at DirectRendererManager");
        DisConnectInputSession();
        IOperation operation = curNode_->GetState() == HPAE_SESSION_STOPPING ?
            OPERATION_STOPPED : OPERATION_PAUSED;
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
        curNode_->SetState(state);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
    };
    SendRequest(request, __func__);
}

bool HpaeDirectRendererManager::SetSessionFade(uint32_t sessionId, IOperation operation)
{
    CHECK_AND_RETURN_RET_LOG(curNode_ != nullptr && curNode_->GetSessionId() == sessionId, false,
        "sessionId %{public}u not match curNode_", sessionId);
    if (curNode_->GetState() == HPAE_SESSION_STOPPED || curNode_->GetState() == HPAE_SESSION_PAUSED ||
        gainNode_ == nullptr || !IsRunning()) {
        AUDIO_WARNING_LOG("session %{public}d do not have gain node or sink is not running!", sessionId);
        if (operation != OPERATION_STARTED) {
            HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
            curNode_->SetState(state);
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
        }
        return false;
    }
    AUDIO_INFO_LOG("get gain node of session %{public}d operation %{public}d.", sessionId, operation);
    if (curNode_->GetState() != HPAE_SESSION_STOPPING &&
        curNode_->GetState() != HPAE_SESSION_PAUSING) {
        gainNode_->SetFadeState(operation);
    }
    if (operation != OPERATION_STARTED) {
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPING : HPAE_SESSION_PAUSING;
        curNode_->SetState(state);
    }
    return true;
}

void HpaeDirectRendererManager::OnNodeStatusUpdate(uint32_t sessionId, IOperation operation)
{
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, curNode_->GetState(), operation);
}

void HpaeDirectRendererManager::OnRequestLatency(uint32_t sessionId, uint64_t &latency)
{
    uint64_t processLatency = 0;

    if (converterForGain_) {
        processLatency += converterForGain_->GetLatency();
    }

    if (gainNode_) {
        processLatency += gainNode_->GetLatency();
    }

    if (volumeSyncNode_) {
        processLatency += volumeSyncNode_->GetLatency();
    }

    if (limiterNode_) {
        processLatency += limiterNode_->GetLatency();
    }

    if (converterForOutput_) {
        processLatency += converterForOutput_->GetLatency();
    }

    if (sinkOutputNode_) {
        processLatency += sinkOutputNode_->GetLatency();
    }

    latency += processLatency;
    return;
}

void HpaeDirectRendererManager::OnNotifyQueue()
{
    CHECK_AND_RETURN_LOG(hpaeSignalProcessThread_, "hpaeSignalProcessThread_ directrenderer is nullptr");
    hpaeSignalProcessThread_->Notify();
}

std::string HpaeDirectRendererManager::GetThreadName()
{
    return sinkInfo_.deviceName;
}

int32_t HpaeDirectRendererManager::DumpSinkInfo()
{
    CHECK_AND_RETURN_RET_LOG(IsInit(), ERR_ILLEGAL_STATE, "HpaeDirectRendererManager not init");
    auto request = [this]() {
        AUDIO_INFO_LOG("DumpSinkInfo deviceName %{public}s", sinkInfo_.deviceName.c_str());
        UploadDumpSinkInfo(sinkInfo_.deviceName);
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

std::string HpaeDirectRendererManager::GetDeviceHDFDumpInfo()
{
    std::string config;
    TransDeviceInfoToString(sinkInfo_, config);
    return config;
}

int32_t HpaeDirectRendererManager::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    // direct mode does not support loudness gain
    return SUCCESS;
}

void HpaeDirectRendererManager::UpdateAppsUid()
{
    appsUid_.clear();
    if (curNode_ != nullptr && curNode_->GetState() == HPAE_SESSION_RUNNING) {
        appsUid_.emplace_back(curNode_->GetAppUid());
    }
    sinkOutputNode_->UpdateAppsUid(appsUid_);
}

void HpaeDirectRendererManager::TriggerAppsUidUpdate(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        AUDIO_INFO_LOG("deviceClass: %{public}s", sinkInfo_.deviceClass.c_str());
        appsUid_.clear();
        if (curNode_ != nullptr &&
            (curNode_->GetState() == HPAE_SESSION_RUNNING ||
            curNode_->GetSessionId() == sessionId)) {
            appsUid_.emplace_back(curNode_->GetAppUid());
        }
        sinkOutputNode_->UpdateAppsUid(appsUid_);
    };
    SendRequest(request, __func__);
}

void HpaeDirectRendererManager::NotifyStreamChangeToSink(
    StreamChangeType change, uint32_t sessionId, RendererState state, uint32_t appUid)
{
    CHECK_AND_RETURN(sinkOutputNode_ != nullptr);
    StreamUsage usage = STREAM_USAGE_UNKNOWN;
    if (sinkInputNodeMap_.find(sessionId) != sinkInputNodeMap_.end()) {
        usage = AudioTypeUtils::GetStreamUsageByStreamType(sinkInputNodeMap_[sessionId]->GetStreamType());
    }
    sinkOutputNode_->NotifyStreamChangeToSink(change, sessionId, usage, state, appUid);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
