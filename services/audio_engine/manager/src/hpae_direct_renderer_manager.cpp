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

void HpaeDirectRendererManager::DeleteInputSession(SessionEffectMoveMode moveMode)
{
    // Invalidate any deferred Create so a stale retry cannot (re)materialize a node after teardown.
    CancelPendingSessionEffectCreate(sessionEffectSessionId_);
    ClearPendingSessionEffectClose();
    if (sessionEffectNode_ != nullptr) {
        sessionEffectNode_->DropEffect();  // Don't destroy algorithm — Lazy Release manages it
    }
    // MoveStream: pass isStandby=true so StopDirectOutputNode skips the 160ms
    // (PERIOD_NS * DIRECT_SINK_STANDBY_TIMES) standby wait — the stream is being
    // handed to another sink, not parked; waiting here only delays the move.
    DisConnectInputSession(moveMode == SessionEffectMoveMode::KEEP_ATTACH_STATE_FOR_MOVE);

    // Session Effect: clean up SE node. DisConnectInputSession already disconnected
    // the SE node from the chain (no bypass reconnect), so just clean up the physical node.
    if (sessionEffectNode_ != nullptr) {
        uint32_t sid = sessionEffectSessionId_;
        HpaeSessionState state = curNode_ != nullptr ? curNode_->GetState() : HPAE_SESSION_NEW;
        sessionEffectNode_.reset();
        sessionEffectSessionId_ = 0;
        if (moveMode == SessionEffectMoveMode::FULL_TEARDOWN) {
            TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sid, state,
                OPERATION_SESSION_EFFECT_DISABLED);
        }
    }
    // Clear attach state on the SinkInputNode only for full teardown (not for move)
    if (curNode_ != nullptr && moveMode == SessionEffectMoveMode::FULL_TEARDOWN) {
        curNode_->ClearSessionEffectAttachState();
    }

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
            // Clear attach state for non-curNode stream being destroyed
            if (node->HasSessionEffectAttachState()) {
                node->ClearSessionEffectAttachState();
                TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, node->GetState(),
                    OPERATION_SESSION_EFFECT_DISABLED);
            }
            RemoveNodeFromMap(sessionId);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::RecreateSinkOutputNodeIfNeeded()
{
    // A failed reload/init leaves sinkOutputNode_ null while curNode_ stays
    // alive; dereferencing it here would crash the worker thread.
    CHECK_AND_RETURN_RET_LOG(sinkOutputNode_ != nullptr, ERROR, "sinkOutputNode_ is null, sink init/reload failed");
    bool isVoip = (sinkInfo_.deviceClass == VOIP_DIRECT_DEVICE_CLASS);
    AudioSamplingRate newSampleRate = GetDirectSampleRate(curNode_->GetSampleRate(), isVoip);
    AudioSampleFormat newFormat = GetDirectFormat(curNode_->GetBitWidth(), isVoip);
    // Southbound (HDI) direct path does not support mono, only stereo is supported.
    // Force the sink output to stereo regardless of the input channel count.
    AudioChannel newChannels = AudioChannel::STEREO;
    HpaeNodeInfo outputNodeInfo = sinkOutputNode_->GetNodeInfo();
    if (outputNodeInfo.samplingRate == newSampleRate && outputNodeInfo.format == newFormat &&
        outputNodeInfo.channels == newChannels && sinkOutputNode_->IsRenderInit()) {
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
    sinkInfo_.channelLayout = STEREO;
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
        AUDIO_INFO_LOG("SessionId %{public}u, using HpaeVolumeSyncNode for VOIP device", sessionId);
    } else {
        nodeInfo.fadeType = DEFAULT_FADE;
        gainNode_ = std::make_shared<HpaeGainNode>(nodeInfo);
        loudnessGainNode_ = std::make_shared<HpaeLoudnessGainNode>(nodeInfo);
    }
    converterForGain_ = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, nodeInfo);
    converterForGain_->SetDownmixNormalization(false);
    converterForGain_->RegisterCallback(this);
}

int32_t HpaeDirectRendererManager::CreateDirectNodes()
{
    CHECK_AND_RETURN_RET_LOG(curNode_ != nullptr, ERROR, "curNode_ not exist, fail to create direct nodes");

    int32_t ret = RecreateSinkOutputNodeIfNeeded();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "RecreateSinkOutputNodeIfNeeded failed");

    CreateProcessNodes(curNode_->GetSessionId());

    AUDIO_INFO_LOG("SessionId %{public}u, Success create direct nodes: "
        "converterForGainId %{public}u, gainNodeId %{public}u, volumeSyncNodeId %{public}u, limiterNodeId %{public}u,"
        " converterForOutputNodeId %{public}u, loudnessGainNodeId %{public}u", curNode_->GetSessionId(),
        converterForGain_->GetNodeId(),
        gainNode_ ? gainNode_->GetNodeId() : 0, volumeSyncNode_ ? volumeSyncNode_->GetNodeId() : 0,
        limiterNode_ ? limiterNode_->GetNodeId() : 0, converterForOutput_->GetNodeId(),
        loudnessGainNode_ ? loudnessGainNode_->GetNodeId() : 0);
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
        " converterForOutputNodeId %{public}u, loudnessGainNodeId %{public}u",
        curNode_->GetSessionId(), converterForGain_->GetNodeId(),
        gainNode_ ? gainNode_->GetNodeId() : 0,
        volumeSyncNode_ ? volumeSyncNode_->GetNodeId() : 0,
        limiterNode_ ? limiterNode_->GetNodeId() : 0, converterForOutput_->GetNodeId(),
        loudnessGainNode_ ? loudnessGainNode_->GetNodeId() : 0);
    converterForGain_ = nullptr;
    gainNode_ = nullptr;
    volumeSyncNode_ = nullptr;
    converterForOutput_ = nullptr;
    limiterNode_ = nullptr;
    loudnessGainNode_ = nullptr;
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::ConnectInputSession()
{
    CHECK_AND_RETURN_RET_LOG(curNode_ != nullptr, ERROR, "curNode_ is null");
    CHECK_AND_RETURN_RET(curNode_->GetState() == HPAE_SESSION_RUNNING, SUCCESS);
    CHECK_AND_RETURN_RET_LOG(converterForOutput_ != nullptr && converterForGain_ != nullptr, ERROR,
        "direct nodes not ready (converterForOutput_/converterForGain_ is null): "
        "CreateDirectNodes failed, sink init/reload may have failed");
    CHECK_AND_RETURN_RET_LOG(sinkOutputNode_ != nullptr, ERROR, "sinkOutputNode_ is null");
    CHECK_AND_RETURN_RET_LOG(sinkOutputNode_->GetSinkState() != STREAM_MANAGER_NEW, ERROR,
        "sink output node is not init");
    CHECK_AND_RETURN_RET_LOG(gainNode_ != nullptr || volumeSyncNode_ != nullptr, ERROR,
        "neither gainNode_ nor volumeSyncNode_ exists");
    // Connect nodes: sinkOutputNode <- converterForOutput <- [limiterNode]
    // <- gainNode/volumeSyncNode <- converterForGain <- curNode
    // Note: limiterNode is only present when channels > STEREO
    sinkOutputNode_->Connect(converterForOutput_);
    if (limiterNode_ != nullptr) {
        converterForOutput_->Connect(limiterNode_);
        if (volumeSyncNode_ != nullptr) {
            limiterNode_->Connect(volumeSyncNode_);
            volumeSyncNode_->Connect(converterForGain_);
            volumeSyncNode_->ResetVolume();
        } else {
            limiterNode_->Connect(gainNode_);
            gainNode_->Connect(loudnessGainNode_);
            // Session Effect: insert SE between loudnessGainNode_ and converterForGain_ when active
            uint32_t curSessionId = curNode_->GetSessionId();
            if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == curSessionId) {
                loudnessGainNode_->Connect(sessionEffectNode_);
                sessionEffectNode_->Connect(converterForGain_);
            } else {
                loudnessGainNode_->Connect(converterForGain_);
            }
            gainNode_->ResetVolume();
            loudnessGainNode_->SetLoudnessGain(curNode_->GetLoudnessGain());
        }
    } else {
        if (volumeSyncNode_ != nullptr) {
            converterForOutput_->Connect(volumeSyncNode_);
            volumeSyncNode_->Connect(converterForGain_);
            volumeSyncNode_->ResetVolume();
        } else {
            converterForOutput_->Connect(gainNode_);
            gainNode_->Connect(loudnessGainNode_);
            // Session Effect: insert SE between loudnessGainNode_ and converterForGain_ when active
            uint32_t curSessionId = curNode_->GetSessionId();
            if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == curSessionId) {
                loudnessGainNode_->Connect(sessionEffectNode_);
                sessionEffectNode_->Connect(converterForGain_);
            } else {
                loudnessGainNode_->Connect(converterForGain_);
            }
            gainNode_->ResetVolume();
            loudnessGainNode_->SetLoudnessGain(curNode_->GetLoudnessGain());
        }
    }

    // Materialize SE from attach state if curNode has it and physical node not yet created
    MaterializeSessionEffectFromAttachState(curNode_->GetSessionId());

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
            int32_t ret = ConnectInputSession();
            if (ret != SUCCESS) {
                AUDIO_ERR_LOG("ConnectInputSession failed %{public}d, roll back session %{public}u to PREPARED",
                    ret, sessionId);
                node->SetState(HPAE_SESSION_PREPARED);
                return;
            }
            // Start SE if active for this session
            if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId) {
                sessionEffectNode_->StartEffect();
            }
            SetSessionFade(sessionId, OPERATION_STARTED);
        }
    };
    SendRequest(request, __func__);
    return SUCCESS;
}

int32_t HpaeDirectRendererManager::DisConnectInputSession(bool isStandby)
{
    CHECK_AND_RETURN_RET(converterForGain_, SUCCESS);

    // Session Effect: disconnect SE node from chain (no bypass reconnect).
    // Do NOT DropEffect here — that happens in DeleteInputSession after this returns.
    if (sessionEffectNode_ != nullptr && loudnessGainNode_ != nullptr
        && converterForGain_ != nullptr) {
        sessionEffectNode_->PauseEffect();
        loudnessGainNode_->DisConnect(sessionEffectNode_);
        sessionEffectNode_->DisConnect(converterForGain_);
    } else if (loudnessGainNode_ != nullptr && converterForGain_ != nullptr) {
        loudnessGainNode_->DisConnect(converterForGain_);
    }

    converterForGain_->DisConnect(curNode_);
    if (limiterNode_ != nullptr) {
        if (volumeSyncNode_ != nullptr) {
            volumeSyncNode_->DisConnect(converterForGain_);
            limiterNode_->DisConnect(volumeSyncNode_);
        } else {
            loudnessGainNode_->DisConnect(converterForGain_);
            gainNode_->DisConnect(loudnessGainNode_);
            limiterNode_->DisConnect(gainNode_);
        }
        converterForOutput_->DisConnect(limiterNode_);
    } else {
        if (volumeSyncNode_ != nullptr) {
            volumeSyncNode_->DisConnect(converterForGain_);
            converterForOutput_->DisConnect(volumeSyncNode_);
        } else {
            loudnessGainNode_->DisConnect(converterForGain_);
            gainNode_->DisConnect(loudnessGainNode_);
            converterForOutput_->DisConnect(gainNode_);
        }
    }
    if (sinkOutputNode_ != nullptr) {
        sinkOutputNode_->DisConnect(converterForOutput_);
        CHECK_AND_RETURN_RET(sinkOutputNode_->GetSinkState() == STREAM_MANAGER_RUNNING, SUCCESS);
        StopDirectOutputNodeSync(isStandby);
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
        // SE close-crossfade waiting on its callback: finalize now — the chain is about to stop
        // being pulled (fade/teardown); the close cannot be guaranteed to complete before then.
        FinalizePendingSessionEffectClose(sessionId);
        if (curNode_ != nullptr && sessionId == curNode_->GetSessionId()) {
            if (!SetSessionFade(sessionId, OPERATION_PAUSED, isStandby)) {
                DisConnectInputSession(isStandby);
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

        // flush history buffer
        node->Flush();
        CHECK_AND_RETURN(curNode_ != nullptr && sessionId == curNode_->GetSessionId());
        // flush converter nodes' dual buffers
        if (converterForGain_ != nullptr) {
            converterForGain_->FlushBuffers();
        }
        if (converterForOutput_ != nullptr) {
            converterForOutput_->FlushBuffers();
        }

        sinkOutputNode_->RenderSinkFlush();

        // flush SE
        if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId) {
            sessionEffectNode_->FlushEffect();
        }
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
        // SE close-crossfade waiting on its callback: finalize now — the chain is about to stop
        // being pulled (fade/teardown); the close cannot be guaranteed to complete before then.
        FinalizePendingSessionEffectClose(sessionId);
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
        // Prepare SE for move
        if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId) {
            sessionEffectNode_->PrepareMoveStream();
        }
        if (sessionId == curNode_->GetSessionId()) {
            DeleteInputSession(SessionEffectMoveMode::KEEP_ATTACH_STATE_FOR_MOVE);
            SetCurrentNode();
        } else {
            // Attach state follows the stream to the new sink — do NOT clear it or emit DISABLED.
            // The SE will re-materialize on the new sink when ConnectInputSession runs.
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
            DisConnectInputSession(true);
            DetachSessionEffectNodeForReload();  // Detach SE but keep attach state for reload
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
        isInit_.store(ret == SUCCESS);
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
    if (hpaeSignalProcessThread_ == nullptr) {
        AUDIO_ERR_LOG("hpaeSignalProcessThread_ is nullptr, %{public}s excute failed", funcName.c_str());
        HpaeMessageQueueMonitor::ReportMessageQueueException(HPAE_DIRECT_MANAGER_TYPE, funcName, "thread is nullptr");
        return;
    }
    hpaeNoLockQueue_.PushRequest(std::move(request));
    hpaeSignalProcessThread_->Notify();
}

void HpaeDirectRendererManager::OnFadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::OnFadeDone");
        CHECK_AND_RETURN_LOG(curNode_ != nullptr && curNode_->GetSessionId() == sessionId,
            "Fade done, sessionId %{public}u not match curNode_", sessionId);
        AUDIO_INFO_LOG("Fade done, callback at DirectRendererManager");
        // Fade completed while an SE close was still waiting: finalize before the chain stops
        // being pulled.
        FinalizePendingSessionEffectClose(sessionId);
        DisConnectInputSession();
        IOperation operation = curNode_->GetState() == HPAE_SESSION_STOPPING ?
            OPERATION_STOPPED : OPERATION_PAUSED;
        HpaeSessionState state = operation == OPERATION_STOPPED ? HPAE_SESSION_STOPPED : HPAE_SESSION_PAUSED;
        curNode_->SetState(state);
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state, operation);
    };
    SendRequest(request, __func__);
}

bool HpaeDirectRendererManager::SetSessionFade(uint32_t sessionId, IOperation operation, bool isStandby)
{
    CHECK_AND_RETURN_RET_LOG(curNode_ != nullptr && curNode_->GetSessionId() == sessionId, false,
        "sessionId %{public}u not match curNode_", sessionId);
    if (curNode_->GetState() == HPAE_SESSION_STOPPED || curNode_->GetState() == HPAE_SESSION_PAUSED ||
        gainNode_ == nullptr || !IsRunning() || isStandby) {
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
    CHECK_AND_RETURN_LOG(curNode_, "curNode_ directrenderer is nullptr");
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

    if (loudnessGainNode_) {
        processLatency += loudnessGainNode_->GetLatency(sessionId);
    }

    // Session Effect latency
    if (sessionEffectNode_) {
        processLatency += sessionEffectNode_->GetLatency(sessionId);
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

void HpaeDirectRendererManager::OnRewindBuffer(uint32_t sessionId, uint64_t rewindTimeUs)
{
    CHECK_AND_RETURN_LOG(curNode_ != nullptr,
        "HpaeDirectRendererManager::OnRewindBuffer curNode_ is null");
    curNode_->RewindHistoryBuffer(rewindTimeUs);
    AUDIO_INFO_LOG("OnRewindBuffer: session %{public}u rewound %{public}" PRIu64 "us",
        sessionId, rewindTimeUs);
}

int32_t HpaeDirectRendererManager::GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat)
{
    CHECK_AND_RETURN_RET_LOG(loudnessGainNode_, ERROR,
        "sessionId %{public}u, loudnessGainNode does not exist", sessionId);
    CHECK_AND_RETURN_RET_LOG(loudnessGainNode_->GetSessionId() == sessionId, ERROR,
        "loudness node sessionId does not match, sessionId %{public}u", sessionId);

    basicFormat.audioChannelInfo.channelLayout = (AudioChannelLayout)sinkInfo_.channelLayout;
    basicFormat.audioChannelInfo.numChannels = (uint32_t)sinkInfo_.channels;
    basicFormat.rate = sinkInfo_.samplingRate;
    // When sessionEffect is enabled, force 48kHz dual channel
    if (sessionEffectNode_ != nullptr && sessionEffectSessionId_ == sessionId) {
        basicFormat.rate = SAMPLE_RATE_48000;
        basicFormat.audioChannelInfo.numChannels = STEREO;
        basicFormat.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    }
    return SUCCESS;
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
    auto request = [this, sessionId, loudnessGain]() {
        Trace trace("[" + std::to_string(sessionId) + "]HpaeDirectRendererManager::SetLoudnessGain[" +
            std::to_string(loudnessGain) + "]");
        auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
        CHECK_AND_RETURN_LOG(node, "sessionId %{public}d, sinkInputNode is nullptr", sessionId);
        node->SetLoudnessGain(loudnessGain);
        if (curNode_ && curNode_->GetSessionId() == sessionId && loudnessGainNode_) {
            loudnessGainNode_->SetLoudnessGain(loudnessGain);
        }
    };
    SendRequest(request, __func__);
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

void HpaeDirectRendererManager::StopDirectOutputNodeSync(bool isStandby)
{
    auto request = [this, isStandby]() { StopDirectOutputNode(isStandby); };
    SendRequest(request, __func__);
}

void HpaeDirectRendererManager::StopDirectOutputNode(bool isStandby)
{
    CHECK_AND_RETURN_LOG(sinkOutputNode_ != nullptr, "sinkoutput is null");
    bool needSleep = !isStandby && sinkOutputNode_->GetSinkState() == STREAM_MANAGER_RUNNING;
    if (needSleep) {
        ClockTime::RelativeSleep(PERIOD_NS * DIRECT_SINK_STANDBY_TIMES);
    }
    sinkOutputNode_->RenderSinkStop();
}

// === Session Effect ===

int32_t HpaeDirectRendererManager::InsertSessionEffectNode(const std::string &effectName,
    std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
    bool aissSwitchEnabled)
{
    CHECK_AND_RETURN_RET_LOG(sessionEffectNode_ == nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND),
        "InsertSessionEffectNode: SE node already exists");
    CHECK_AND_RETURN_RET_LOG(converterForGain_ != nullptr && loudnessGainNode_ != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND),
        "InsertSessionEffectNode: chain not created yet");

    HpaeNodeInfo nodeInfo = converterForGain_->GetNodeInfo();
    sessionEffectNode_ = std::make_shared<HpaeSessionEffectNode>(nodeInfo);
    int32_t ret = sessionEffectNode_->CreateEffectWithInstance(effectName, instance, config, aissSwitchEnabled);
    if (ret != 0) {
        AUDIO_ERR_LOG("InsertSessionEffectNode: CreateEffectWithInstance failed, ret=%{public}d", ret);
        sessionEffectNode_->DropEffect();
        sessionEffectNode_.reset();
        sessionEffectSessionId_ = 0;
        return ret;
    }

    // Insert between loudnessGainNode_ and converterForGain_
    loudnessGainNode_->DisConnect(converterForGain_);
    loudnessGainNode_->Connect(sessionEffectNode_);
    sessionEffectNode_->Connect(converterForGain_);

    AUDIO_INFO_LOG("InsertSessionEffectNode: SE node inserted for session %{public}u",
        sessionEffectSessionId_);
    return 0;
}

int32_t HpaeDirectRendererManager::RemoveSessionEffectNode(bool finalizeNow)
{
    CHECK_AND_RETURN_RET_LOG(sessionEffectNode_ != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND),
        "RemoveSessionEffectNode: no SE node to remove");

    auto closingNode = sessionEffectNode_;
    uint32_t sessionId = sessionEffectSessionId_;
    closingNode->StopPreheatThread();

    // Pipeline latency the wet output lags behind upstream dry = SE + the converter immediately
    // upstream of the SE insert point (converterForGain_). Flush it, then rewind curNode_ history
    // so the bypass dry re-pulls from the aligned cursor. Downstream queues are left untouched.
    uint64_t latency = closingNode->GetLatency(sessionId);
    if (converterForGain_ != nullptr) {
        latency += converterForGain_->GetLatency();
        converterForGain_->FlushBuffers();
    }
    if (latency > 0 && curNode_ != nullptr && curNode_->GetSessionId() == sessionId) {
        curNode_->RewindHistoryBuffer(latency);
    }

    // Enter STOPPING: the next Process() (between this and the next HandleMsg) gets one aligned
    // wet->dry crossfade opportunity if the chain is pulled. FinalizeClose — direct when
    // finalizeNow or no crossfade pending, else deferred to the OnCloseCrossfadeDone callback —
    // owns the actual node deletion (never inside DoProcess).
    closingNode->StartCloseCrossfade();
    std::weak_ptr<HpaeSessionEffectNode> weakClosingNode = closingNode;
    bool crossfadePending = closingNode->IsCloseCrossfadePending();
    if (finalizeNow || !crossfadePending) {
        // Switch path ("open new"), or a non-ACTIVE node with no crossfade armed: finalize
        // inside this same request so a Create queued right behind us sees no leftover node.
        FinalizeSessionEffectClose(sessionId, weakClosingNode);
    } else {
        // Smooth close: ProcessCloseCrossfade completes on the next pull and calls back
        // OnCloseCrossfadeDone, which finalizes on the HandleMsg thread. Record the identity
        // token so a late callback cannot finalize a different (newer) node.
        pendingCloseNode_ = weakClosingNode;
        pendingCloseSessionId_ = sessionId;
    }
    AUDIO_INFO_LOG("RemoveSessionEffectNode: session %{public}u BeginClose (finalizeNow=%{public}d, "
        "crossfadePending=%{public}d), latency=%" PRIu64,
        sessionId, static_cast<int32_t>(finalizeNow), static_cast<int32_t>(crossfadePending), latency);
    return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
}

void HpaeDirectRendererManager::FinalizeSessionEffectClose(uint32_t sessionId,
    const std::weak_ptr<HpaeSessionEffectNode> &closingNode)
{
    // Any finalize attempt ends the pending-close window for this session.
    if (pendingCloseSessionId_ == sessionId) {
        ClearPendingSessionEffectClose();
    }
    auto node = closingNode.lock();
    // Identity check: no-op if the closing node was already replaced/destroyed
    // (DestroyStream, DeleteInputSession, Reload, or a replayed/recreated node).
    CHECK_AND_RETURN(node != nullptr && sessionEffectNode_ == node && sessionEffectSessionId_ == sessionId);

    // Only reconnect the bypass if the stream is still running; if pause/stop already tore the
    // chain down, leave it disconnected — ConnectInputSession rebuilds on Start.
    bool restoreBypass = curNode_ != nullptr && curNode_->GetSessionId() == sessionId &&
        curNode_->GetState() == HPAE_SESSION_RUNNING &&
        loudnessGainNode_ != nullptr && converterForGain_ != nullptr;
    if (loudnessGainNode_ != nullptr && converterForGain_ != nullptr) {
        loudnessGainNode_->DisConnect(node);
        node->DisConnect(converterForGain_);
        if (restoreBypass) {
            loudnessGainNode_->Connect(converterForGain_);
        }
    }

    // If a pending rapid-recreate will reuse this exact instance, do NOT Flush/Stop it — the new
    // binding takes the instance over (no matching ENABLE otherwise). Otherwise release normally.
    auto pendingInst = pendingSeCreateInstance_.lock();
    auto closingInst = node->GetEffectInstance();
    bool sameInstanceReuse = pendingInst != nullptr && closingInst != nullptr &&
        pendingSeCreateSessionId_ == sessionId && pendingInst.get() == closingInst.get();
    if (sameInstanceReuse) {
        node->CloseSessionEffectCore();
    } else {
        node->DropEffect();
    }

    sessionEffectNode_.reset();
    sessionEffectSessionId_ = 0;

    auto streamNode = SafeGetMap(sinkInputNodeMap_, sessionId);
    HpaeSessionState state = streamNode != nullptr ? streamNode->GetState() : HPAE_SESSION_NEW;
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state,
        OPERATION_SESSION_EFFECT_DISABLED);
    AUDIO_INFO_LOG("FinalizeSessionEffectClose: session %{public}u finalized (sameInstance=%{public}d)",
        sessionId, static_cast<int32_t>(sameInstanceReuse));
}

void HpaeDirectRendererManager::ClearPendingSessionEffectClose()
{
    pendingCloseNode_.reset();
    pendingCloseSessionId_ = 0;
}

bool HpaeDirectRendererManager::FinalizePendingSessionEffectClose(uint32_t sessionId)
{
    CHECK_AND_RETURN_RET(pendingCloseSessionId_ == sessionId && !pendingCloseNode_.expired(), false);
    auto closingNode = pendingCloseNode_.lock();
    ClearPendingSessionEffectClose();
    if (closingNode == nullptr || sessionEffectNode_ != closingNode) {
        AUDIO_WARNING_LOG("PendingClose identity mismatch: session %{public}u", sessionId);
        return false;
    }
    FinalizeSessionEffectClose(sessionId, closingNode);
    return true;
}

void HpaeDirectRendererManager::OnCloseCrossfadeDone(uint32_t sessionId)
{
    auto request = [this, sessionId]() {
        bool finalized = FinalizePendingSessionEffectClose(sessionId);
        AUDIO_INFO_LOG("CloseCrossfadeDone: session %{public}u finalized=%{public}d",
            sessionId, static_cast<int32_t>(finalized));
    };
    SendRequest(request, __func__);
}

uint64_t HpaeDirectRendererManager::SetPendingSessionEffectCreate(uint32_t sessionId,
    const std::shared_ptr<IEffectInstance> &instance)
{
    ++pendingSeCreateGeneration_;
    pendingSeCreateSessionId_ = sessionId;
    pendingSeCreateInstance_ = instance;
    return pendingSeCreateGeneration_;
}

bool HpaeDirectRendererManager::IsPendingSessionEffectCreate(uint32_t sessionId,
    uint64_t generation, const std::shared_ptr<IEffectInstance> &instance) const
{
    return pendingSeCreateSessionId_ == sessionId && pendingSeCreateGeneration_ == generation &&
        instance != nullptr && pendingSeCreateInstance_.lock().get() == instance.get();
}

void HpaeDirectRendererManager::CancelPendingSessionEffectCreate(uint32_t sessionId)
{
    if (pendingSeCreateSessionId_ == sessionId) {
        pendingSeCreateSessionId_ = 0;
    }
}

void HpaeDirectRendererManager::MaterializeSessionEffectFromAttachState(uint32_t curSessionId)
{
    // VOIP scene does not support SE
    CHECK_AND_RETURN(volumeSyncNode_ == nullptr);

    // Only when the physical SE node has not been created yet.
    if (sessionEffectNode_ != nullptr) {
        return;
    }
    auto curSinkInput = SafeGetMap(sinkInputNodeMap_, curSessionId);
    if (curSinkInput == nullptr || !curSinkInput->HasSessionEffectAttachState()) {
        return;
    }
    auto stOpt = curSinkInput->GetSessionEffectAttachState();
    CHECK_AND_RETURN(stOpt.has_value() && stOpt->enabled);
    auto inst = stOpt->instance.lock();
    HpaeSessionState state = curNode_->GetState();
    if (inst == nullptr) {
        curSinkInput->ClearSessionEffectAttachState();
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, curSessionId, state,
            OPERATION_SESSION_EFFECT_DISABLED);
        AUDIO_WARNING_LOG("MaterializeSessionEffectFromAttachState: SE instance expired for session %{public}u, "
            "attach-state cleared", curSessionId);
        return;
    }
    sessionEffectSessionId_ = curSessionId;
    int32_t seRet = InsertSessionEffectNode(stOpt->effectName, inst, stOpt->config);
    if (seRet != static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        sessionEffectSessionId_ = 0;
        curSinkInput->ClearSessionEffectAttachState();
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, curSessionId, state,
            OPERATION_SESSION_EFFECT_DISABLED);
        AUDIO_WARNING_LOG("MaterializeSessionEffectFromAttachState: materialize SE failed for session %{public}u, "
            "attach-state cleared, ret=%{public}d", curSessionId, seRet);
        return;
    }

    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, curSessionId, state,
        OPERATION_SESSION_EFFECT_ENABLED);
    AUDIO_INFO_LOG("MaterializeSessionEffectFromAttachState: SE materialized+ENABLED for session %{public}u",
        curSessionId);
}

void HpaeDirectRendererManager::DetachSessionEffectNodeForReload()
{
    // Do NOT clear attach state — it must survive reload so ConnectInputSession can re-materialize.
    // Only reset the physical cache (node + sessionId).
    CancelPendingSessionEffectCreate(sessionEffectSessionId_);
    ClearPendingSessionEffectClose();
    if (sessionEffectNode_ != nullptr) {
        // Reload runs on SignalProcessThread after DisConnectInputSession has removed this node
        // from the processing chain, so CloseSessionEffectCore only resets local runtime state.
        sessionEffectNode_->CloseSessionEffectCore();
        sessionEffectNode_.reset();
    }
    sessionEffectSessionId_ = 0;
}

bool HpaeDirectRendererManager::TryDeferSessionEffectCreate(uint32_t sessionId,
    const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    // The curNode's physical SE node is still finalizing its deferred close (STOPPING). Defer this
    // Create until that node is gone: record a cancelable/ownership token and requeue. Do NOT write
    // attach state, insert a node, or send ENABLE here.
    if (curNode_ == nullptr || curNode_->GetSessionId() != sessionId ||
        sessionEffectNode_ == nullptr ||
        sessionEffectNode_->GetStatus() != HpaeSessionEffectNode::STATUS::STOPPING) {
        return false;
    }
    uint64_t generation = SetPendingSessionEffectCreate(sessionId, instance);
    Request retry = [this, sessionId, effectName, instance, config, generation, aissSwitchEnabled]() {
        CHECK_AND_RETURN(IsPendingSessionEffectCreate(sessionId, generation, instance));
        CancelPendingSessionEffectCreate(sessionId);
        if (SafeGetMap(sinkInputNodeMap_, sessionId) != nullptr) {
            CreateSessionEffect(sessionId, effectName, instance, config, aissSwitchEnabled);
        }
    };
    hpaeNoLockQueue_.PushRequest(std::move(retry));
    if (hpaeSignalProcessThread_ != nullptr) {
        hpaeSignalProcessThread_->Notify();
    }
    AUDIO_INFO_LOG("CreateSessionEffect: session %{public}u deferred behind STOPPING node", sessionId);
    return true;
}

int32_t HpaeDirectRendererManager::MaterializeCurrentSessionEffect(uint32_t sessionId,
    const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    // Caller guarantees curNode_ != nullptr && curNode_->GetSessionId() == sessionId.
    if (converterForGain_ == nullptr || loudnessGainNode_ == nullptr) {
        // Chain not ready; attach state sits on node, will materialize in ConnectInputSession
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, curNode_->GetState(),
            OPERATION_SESSION_EFFECT_ENABLED);
        AUDIO_INFO_LOG("CreateSessionEffect: deferred for current session %{public}u (chain not created)",
            sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
    }
    sessionEffectSessionId_ = sessionId;
    int32_t ret = InsertSessionEffectNode(effectName, instance, config, aissSwitchEnabled);
    if (ret == static_cast<int32_t>(SessionEffectErrCode::SUCCESS)) {
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, curNode_->GetState(),
            OPERATION_SESSION_EFFECT_ENABLED);
    } else {
        sessionEffectSessionId_ = 0;
    }
    return ret;
}

int32_t HpaeDirectRendererManager::CreateSessionEffect(uint32_t sessionId,
    const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    CHECK_AND_RETURN_RET_LOG(instance != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_INVALID_PARAMETER),
        "CreateSessionEffect: effect instance is null for session %{public}u", sessionId);
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);

    if (TryDeferSessionEffectCreate(sessionId, effectName, instance, config, aissSwitchEnabled)) {
        return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
    }

    // Reject a duplicate bind BEFORE touching attach state
    CHECK_AND_RETURN_RET_LOG(
        !(curNode_ != nullptr && curNode_->GetSessionId() == sessionId && sessionEffectNode_ != nullptr),
        static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND),
        "CreateSessionEffect: curNode already has materialized SE for session %{public}u", sessionId);
    CHECK_AND_RETURN_RET_LOG(volumeSyncNode_ == nullptr, static_cast<int32_t>(SessionEffectErrCode::ERR_NOT_SUPPORTED),
        "CreateSessionEffect: VOIP mode does not support SE for session %{public}u", sessionId);
    // Create accepted — persist the logical binding on the SinkInputNode
    if (node != nullptr) {
        node->EnsureHistoryBufferForAiss(instance->GetPreheatFrames());
        SessionEffectAttachState st;
        st.enabled = true;
        st.effectName = effectName;
        st.instance = instance;
        st.config = config;
        node->SetSessionEffectAttachState(st);
    }

    // If session is curNode_, materialize immediately (chain readiness checked inside)
    if (curNode_ != nullptr && curNode_->GetSessionId() == sessionId) {
        return MaterializeCurrentSessionEffect(sessionId, effectName, instance, config, aissSwitchEnabled);
    }
    // Not curNode_ -> attach state sits on SinkInputNode, waits for ConnectInputSession
    HpaeSessionState state = (node != nullptr) ? node->GetState() : HPAE_SESSION_NEW;
    TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state,
        OPERATION_SESSION_EFFECT_ENABLED);
    AUDIO_INFO_LOG("CreateSessionEffect: deferred for session %{public}u (not curNode)", sessionId);
    return 0;
}

int32_t HpaeDirectRendererManager::DestroySessionEffect(uint32_t sessionId, bool finalizeNow)
{
    // Clear attach state on SinkInputNode (logical unbinding)
    auto node = SafeGetMap(sinkInputNodeMap_, sessionId);
    bool hadAttachState = (node != nullptr) && node->HasSessionEffectAttachState();
    if (node != nullptr) {
        node->ClearSessionEffectAttachState();
    }

    // Remove active physical SE node if materialized for this session
    if (sessionEffectSessionId_ == sessionId && sessionEffectNode_ != nullptr) {
        if (sessionEffectNode_->GetStatus() == HpaeSessionEffectNode::STATUS::STOPPING) {
            // Close already begun (BeginClose is waiting on the crossfade callback). Re-BeginClose
            // would double-rewind and duplicate the Finalize; just cancel any deferred recreate.
            CancelPendingSessionEffectCreate(sessionId);
            if (finalizeNow) {
                // Earlier BeginClose is waiting on its callback; run the Finalize now so the
                // teardown completes before the new session's Create (the late callback no-ops
                // on the cleared token).
                FinalizeSessionEffectClose(sessionId, sessionEffectNode_);
            }
            AUDIO_INFO_LOG("DestroySessionEffect: session %{public}u already STOPPING, skip re-Begin",
                sessionId);
            return static_cast<int32_t>(SessionEffectErrCode::SUCCESS);
        }
        return RemoveSessionEffectNode(finalizeNow);
    }

    // If there was attach state but no physical node, emit DISABLED for the logical unbind
    if (hadAttachState) {
        HpaeSessionState state = (node != nullptr) ? node->GetState() : HPAE_SESSION_NEW;
        TriggerCallback(UPDATE_STATUS, HPAE_STREAM_CLASS_TYPE_PLAY, sessionId, state,
            OPERATION_SESSION_EFFECT_DISABLED);
        AUDIO_INFO_LOG("DestroySessionEffect: cleared attach state for session %{public}u", sessionId);
        return 0;
    }

    AUDIO_WARNING_LOG("DestroySessionEffect: session %{public}u has no SE", sessionId);
    return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
}

int32_t HpaeDirectRendererManager::SetSessionEffectParameter(uint32_t sessionId,
    int32_t type, float value)
{
    if (sessionEffectSessionId_ == sessionId && sessionEffectNode_ != nullptr) {
        return sessionEffectNode_->SetParameter(type, value);
    }
    AUDIO_WARNING_LOG("SetSessionEffectParameter: session %{public}u has no active SE node", sessionId);
    return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
}

void HpaeDirectRendererManager::PostCreateSessionEffect(uint32_t sessionId,
    const std::string &effectName, std::shared_ptr<IEffectInstance> instance,
    const AudioEffectConfig &config, bool aissSwitchEnabled)
{
    auto request = [this, sessionId, effectName, instance, config, aissSwitchEnabled]() {
        CreateSessionEffect(sessionId, effectName, instance, config, aissSwitchEnabled);
    };
    SendRequest(std::move(request), __func__);
}

int32_t HpaeDirectRendererManager::PostDestroySessionEffect(uint32_t sessionId, bool finalizeNow)
{
    auto request = [this, sessionId, finalizeNow]() -> int32_t {
        return DestroySessionEffect(sessionId, finalizeNow);
    };
    int32_t ret = SendRequestSync<int32_t>(std::move(request), __func__, ERR_OPERATION_FAILED);
    CHECK_AND_RETURN_RET_LOG(ret == static_cast<int32_t>(SessionEffectErrCode::SUCCESS), ret,
        "PostDestroySessionEffect: session %{public}u destroy failed ret=%{public}d", sessionId, ret);
    return ret;
}

void HpaeDirectRendererManager::PostSetSessionEffectParameter(uint32_t sessionId,
    int32_t type, float value)
{
    auto request = [this, sessionId, type, value]() {
        SetSessionEffectParameter(sessionId, type, value);
    };
    SendRequest(std::move(request), __func__);
}

bool HpaeDirectRendererManager::IsValidSession(uint32_t sessionId) const
{
    return sinkInputNodeMap_.find(sessionId) != sinkInputNodeMap_.end();
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
