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
#define LOG_TAG "HpaeProcessCluster"
#endif

#include <cinttypes>

#include "audio_errors.h"
#include "hpae_process_cluster.h"
#include "hpae_remote_device_class.h"
#include "hpae_node_common.h"
#include "audio_utils.h"
#include "audio_engine_log.h"
#include "stream_dfx_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
HpaeProcessCluster::HpaeProcessCluster(HpaeNodeInfo nodeInfo, HpaeSinkInfo &sinkInfo)
    : HpaeNode(nodeInfo), sinkInfo_(sinkInfo)
{
    uint32_t effectiveRate = nodeInfo.GetEffectiveSampleRate();
    if (effectiveRate == 0 || sinkInfo.samplingRate == 0) {
        AUDIO_ERR_LOG("ProcessCluster invalid rate: effectiveRate=%{public}u sinkRate=%{public}d",
            effectiveRate, sinkInfo.samplingRate);
        nodeInfo.frameLen = sinkInfo.frameLen; // fallback
    } else {
        // mixerNode operates at sink rate, compute frameLen directly from sink rate.
        nodeInfo.frameLen = sinkInfo.samplingRate * FRAME_LEN_20MS / MILLISECOND_PER_SECOND;
    }
    nodeInfo.samplingRate = sinkInfo.samplingRate;
    // nodeInfo is the first streamInfo, but mixerNode need formatConverterOutput's nodeInfo.
    // so we need to make a prediction here on the output of the formatConverter node.
    // don't worry, Nodeinfo will still be modified during DoProcess.
    if (nodeInfo.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE || nodeInfo.sourceType == SOURCE_TYPE_REMOTE_CAST) {
        HpaeNodeInfo tempNodeInfo(nodeInfo);
        tempNodeInfo.channels = sinkInfo.channels;
        tempNodeInfo.channelLayout = static_cast<AudioChannelLayout>(sinkInfo.channelLayout);
        mixerNode_ = std::make_shared<HpaeMixerNode>(tempNodeInfo);
        AUDIO_INFO_LOG("because processCluster is open limiter,so change channels to %{public}u",
            tempNodeInfo.channels);
    } else {
        mixerNode_ = std::make_shared<HpaeMixerNode>(nodeInfo);
    }
    if (TransProcessorTypeToSceneType(nodeInfo.sceneType) != "SCENE_EXTRA") {
        renderEffectNode_ = std::make_shared<HpaeRenderEffectNode>(nodeInfo, sinkInfo.collaborationRouteMode);
        renderNoneEffectNode_ = nullptr;
    } else {
        renderEffectNode_ = nullptr;
        renderNoneEffectNode_ = std::make_shared<HpaeRenderEffectNode>(nodeInfo);
    }
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("HpaeProcessCluster");
#endif
}

HpaeProcessCluster::~HpaeProcessCluster()
{
    AUDIO_INFO_LOG("destroyed, scene type is %{public}d", GetSceneType());
    Reset();
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
#endif
}

void HpaeProcessCluster::DoProcess()
{
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->DoProcess();
        return;
    }
    mixerNode_->DoProcess();
}

bool HpaeProcessCluster::Reset()
{
    mixerNode_->Reset();
    for (auto converterNode : idConverterMap_) {
        converterNode.second->Reset();
    }
    for (auto gainNode : idGainMap_) {
        gainNode.second->Reset();
    }
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->Reset();
        renderEffectNode_ = nullptr;
    }
    return true;
}

bool HpaeProcessCluster::ResetAll()
{
    return renderEffectNode_ != nullptr ? renderEffectNode_->ResetAll() : mixerNode_->ResetAll();
}

std::shared_ptr<HpaeNode> HpaeProcessCluster::GetSharedInstance()
{
    if (renderEffectNode_ != nullptr) {
        return renderEffectNode_;
    }
    return mixerNode_;
}

OutputPort<HpaePcmBuffer *> *HpaeProcessCluster::GetOutputPort()
{
    return renderEffectNode_ != nullptr ? renderEffectNode_->GetOutputPort() : mixerNode_->GetOutputPort();
}

int32_t HpaeProcessCluster::GetGainNodeCount()
{
    return idGainMap_.size();
}

int32_t HpaeProcessCluster::GetConverterNodeCount()
{
    return idConverterMap_.size();
}

int32_t HpaeProcessCluster::GetLoudnessGainNodeCount()
{
    return idLoudnessGainNodeMap_.size();
}

int32_t HpaeProcessCluster::GetPreOutNum()
{
    return mixerNode_->GetPreOutNum();
}

void HpaeProcessCluster::ConnectMixerNode()
{
    if (renderEffectNode_ != nullptr && renderEffectNode_->GetPreOutNum() == 0) {
        renderEffectNode_->Connect(mixerNode_);
        AUDIO_INFO_LOG("Process Connect mixerNode_");
    }
    return;
}

void HpaeProcessCluster::CreateGainNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo)
{
    auto gainNode = SafeGetMap(idGainMap_, sessionId);
    if (!gainNode) {
        HpaeNodeInfo gainNodeInfo = preNodeInfo;
        idGainMap_[sessionId] = std::make_shared<HpaeGainNode>(gainNodeInfo);
    } else {
        // Reuse existing node: clear stale fade state (e.g. DONE_FADEOUT left by a previous
        // Pause) so SignalProcess won't keep silencing data after the reconnect. Arm fade-in
        // only when resuming from a fade lifecycle; keep continuity for a cleanly playing node.
        if (gainNode->HasPendingFade()) {
            gainNode->ResetFadeState();
        } else {
            gainNode->ClearFadeState();
        }
        AUDIO_INFO_LOG("sessionId %{public}d gainNode already exist, reset fade state", sessionId);
    }
}

void HpaeProcessCluster::CreateConverterNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo)
{
    CHECK_AND_RETURN_LOG(!SafeGetMap(idConverterMap_, sessionId),
        "sessionId %{public}d converterNode already exist", sessionId);
    AudioBasicFormat basicFormat;
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->GetExpectedInputChannelInfo(basicFormat);
    }
    uint32_t channels = basicFormat.audioChannelInfo.numChannels;
    AudioChannelLayout channelLayout = basicFormat.audioChannelInfo.channelLayout;
    HpaeNodeInfo outputNodeInfo = preNodeInfo;
    outputNodeInfo.frameLen = mixerNode_->GetFrameLen();
    outputNodeInfo.samplingRate = sinkInfo_.samplingRate;
    outputNodeInfo.format = sinkInfo_.format;
    outputNodeInfo.channels = channels == 0 ? sinkInfo_.channels : static_cast<AudioChannel>(channels);
    outputNodeInfo.channelLayout = channelLayout == 0 ? static_cast<AudioChannelLayout>(sinkInfo_.channelLayout) :
        static_cast<AudioChannelLayout>(channelLayout);
    idConverterMap_[sessionId] = std::make_shared<HpaeAudioFormatConverterNode>(preNodeInfo, outputNodeInfo);
    // if there is no loudness gain or effect, query information will not change
    idConverterMap_[sessionId]->RegisterCallback(this);
    // processCluster comes with outputCluster, which has a limiter, so for converter in process cluster,
    // downmix normalization can be disabled to preserve the loudness of input
    idConverterMap_[sessionId]->SetDownmixNormalization(false);
}

void HpaeProcessCluster::CreateLoudnessGainNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo)
{
    CHECK_AND_RETURN_LOG(!SafeGetMap(idLoudnessGainNodeMap_, sessionId),
        "sessionId %{public}d loudnessGainNode already exist", sessionId);
    HpaeNodeInfo loudnessGainNodeInfo = preNodeInfo;
    loudnessGainNodeInfo.waitValidInput = false;
    idLoudnessGainNodeMap_[sessionId] = std::make_shared<HpaeLoudnessGainNode>(loudnessGainNodeInfo);
}

void HpaeProcessCluster::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaeNodeInfo &preNodeInfo = preNode->GetNodeInfo();
    uint32_t sessionId = preNodeInfo.sessionId;
    AUDIO_INFO_LOG("sessionId is %{public}u, streamType is %{public}d, sceneType is %{public}d, "
        "rate is %{public}u, ch is %{public}u, "
        "preNodeId %{public}u, preNodeName is %{public}s",
        preNodeInfo.sessionId, preNodeInfo.streamType, preNodeInfo.sceneType,
        preNodeInfo.customSampleRate == 0 ? preNodeInfo.samplingRate : preNodeInfo.customSampleRate,
        preNodeInfo.channels, preNodeInfo.nodeId, preNodeInfo.nodeName.c_str());
    
    ConnectMixerNode();
    mixerNode_->Connect(idGainMap_[sessionId]);
    idGainMap_[sessionId]->Connect(idLoudnessGainNodeMap_[sessionId]);
    auto seNode = SafeGetMap(idSessionEffectMap_, sessionId);
    if (seNode) {
        idLoudnessGainNodeMap_[sessionId]->Connect(seNode);
        // SE→converter persists across DisConnect/Connect, no need to reconnect
    } else {
        idLoudnessGainNodeMap_[sessionId]->Connect(idConverterMap_[sessionId]);
    }
    idConverterMap_[sessionId]->Connect(preNode);
    // update mixer node info
    HpaeNodeInfo tmpNodeinfo = mixerNode_->GetNodeInfo();
    tmpNodeinfo.streamType = preNodeInfo.streamType;
    tmpNodeinfo.effectInfo.streamUsage = preNodeInfo.effectInfo.streamUsage;
    mixerNode_->SetNodeInfo(tmpNodeinfo);
    mixerNode_->EnableProcess(true);
}

void HpaeProcessCluster::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    uint32_t sessionId = preNode->GetNodeInfo().sessionId;
    AUDIO_INFO_LOG("sessionId is %{public}u, streamType is %{public}d, sceneType is %{public}d",
        sessionId, preNode->GetNodeInfo().streamType, preNode->GetNodeInfo().sceneType);
    if (SafeGetMap(idConverterMap_, sessionId)) {
        auto seNode = SafeGetMap(idSessionEffectMap_, sessionId);
        if (seNode) {
            seNode->PauseEffect();
            idLoudnessGainNodeMap_[sessionId]->DisConnect(seNode);
        } else {
            idLoudnessGainNodeMap_[sessionId]->DisConnect(idConverterMap_[sessionId]);
        }
        idConverterMap_[sessionId]->DisConnect(preNode);
        idGainMap_[sessionId]->DisConnect(idLoudnessGainNodeMap_[sessionId]);
        mixerNode_->DisConnect(idGainMap_[sessionId]);
        AUDIO_INFO_LOG("Exist converterNode preOutNum is %{public}zu", mixerNode_->GetPreOutNum());
    }
    if (mixerNode_->GetPreOutNum() == 0) {
        mixerNode_->EnableProcess(false);
        AUDIO_DEBUG_LOG("Set mixerNode EnableProcess false");
    }
}

void HpaeProcessCluster::DisConnectMixerNode()
{
    if (renderEffectNode_) {
        renderEffectNode_->DisConnect(mixerNode_);
        renderEffectNode_->InitEffectBufferFromDisConnect();
        AUDIO_INFO_LOG("DisConnect mixerNode_");
    }
}

int32_t HpaeProcessCluster::CreateNodes(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(preNode != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::PLAY,
                BusinessScenario::CREATE, ERR_NULL_POINTER),
            "Fail create all nodes", false),
        "Fail create all nodes");
    HpaeNodeInfo &preNodeInfo = preNode->GetNodeInfo();
    uint32_t sessionId = preNodeInfo.sessionId;
    CreateConverterNode(sessionId, preNodeInfo);
    CreateLoudnessGainNode(sessionId, preNodeInfo);
    CreateGainNode(sessionId, preNodeInfo);
    AUDIO_INFO_LOG("SessionId %{public}u, Success create all nodes: "
        "converterNodeId %{public}u, loudnessGainNodeId %{public}u, gainNodeId %{public}u",
        sessionId, idConverterMap_[sessionId]->GetNodeId(), idLoudnessGainNodeMap_[sessionId]->GetNodeId(),
        idGainMap_[sessionId]->GetNodeId());
    return SUCCESS;
}
 
int32_t HpaeProcessCluster::CheckNodes(uint32_t sessionId)
{
    if (SafeGetMap(idConverterMap_, sessionId) && SafeGetMap(idLoudnessGainNodeMap_, sessionId) &&
        SafeGetMap(idGainMap_, sessionId)) {
        return SUCCESS;
    }
    AUDIO_INFO_LOG("SessionId %{public}u, No nodes in this processcluster, cant connect or destroy", sessionId);
    return ERROR;
}
 
int32_t HpaeProcessCluster::DestroyNodes(uint32_t sessionId, SessionEffectCleanupMode mode)
{
    CHECK_AND_RETURN_RET(CheckNodes(sessionId) == SUCCESS, ERROR);
    auto seNode = SafeGetMap(idSessionEffectMap_, sessionId);
    if (seNode) {
        if (mode == SessionEffectCleanupMode::DETACH_FOR_RELOAD) {
            seNode->CloseSessionEffectCore();
        } else {
            seNode->DropEffect();
        }
        idSessionEffectMap_.erase(sessionId);
    }
    idConverterMap_.erase(sessionId);
    idLoudnessGainNodeMap_.erase(sessionId);
    idGainMap_.erase(sessionId);
    AUDIO_INFO_LOG("SessionId %{public}u, Success destroy all nodes", sessionId);
    return SUCCESS;
}

void HpaeProcessCluster::Flush(uint32_t sessionId)
{
    AUDIO_INFO_LOG("Flush sessionId %{public}u", sessionId);
    auto converter = SafeGetMap(idConverterMap_, sessionId);
    if (converter != nullptr) {
        converter->FlushBuffers();
    }
    if (HasSessionEffectNode(sessionId)) {
        SessionEffectFlush(sessionId);
    }
}

void HpaeProcessCluster::InitEffectBuffer(const uint32_t sessionId)
{
    CHECK_AND_RETURN_LOG(renderEffectNode_ != nullptr, "renderEffectNode is nullptr");
    renderEffectNode_->InitEffectBuffer(sessionId);
    AUDIO_INFO_LOG("init sessionId: %{public}u", sessionId);
}

int32_t HpaeProcessCluster::GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat)
{
    // get format input from loundness gain node
    if (SafeGetMap(idLoudnessGainNodeMap_, sessionId)) {
        if (idLoudnessGainNodeMap_[sessionId]->IsLoudnessAlgoOn()) { // loundess algorithm needs 48k sample rate
            basicFormat.rate = SAMPLE_RATE_48000;
            basicFormat.audioChannelInfo.numChannels = sinkInfo_.channels;
            basicFormat.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(sinkInfo_.channelLayout);
        } else { // if there is no algorithm, stream into loudness node should may need to be sinkoutput format
            basicFormat.rate = sinkInfo_.samplingRate;
            basicFormat.audioChannelInfo.numChannels = sinkInfo_.channels;
            basicFormat.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(sinkInfo_.channelLayout);
        }
    }
    // get format info from effect node
    int32_t ret = SUCCESS;
    if (renderEffectNode_ != nullptr) {
        ret = renderEffectNode_->GetExpectedInputChannelInfo(basicFormat);
    }
    // SessionEffectNode (AISS) requires 48kHz input
    if (SafeGetMap(idSessionEffectMap_, sessionId)) {
        basicFormat.rate = SAMPLE_RATE_48000;
        basicFormat.audioChannelInfo.numChannels = STEREO;
        basicFormat.audioChannelInfo.channelLayout = CH_LAYOUT_STEREO;
    }
    return ret;
}

int32_t HpaeProcessCluster::AudioRendererCreate(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    if (renderEffectNode_ != nullptr) {
        return renderEffectNode_->AudioRendererCreate(nodeInfo);
    }

    if (renderNoneEffectNode_ != nullptr && CheckNeedNotifyEffectNode(sinkInfo)) {
        return renderNoneEffectNode_->AudioRendererCreate(nodeInfo);
    }

    return 0;
}

int32_t HpaeProcessCluster::HandleRendererStart(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    uint32_t sessionId = nodeInfo.sessionId;
    if (hasPendingStop_ && pendingStopSessionId_ == sessionId) {
        ClearDeferredStop();
        AUDIO_INFO_LOG("sessionId %{public}u reconnect, cancel deferred stop", sessionId);
        return SUCCESS;
    }
    if (hasPendingStop_) {
        AUDIO_INFO_LOG("flush deferred stop %{public}u before start %{public}u", pendingStopSessionId_, sessionId);
        ExecuteDeferredStop();
    }
    return AudioRendererStart(nodeInfo, sinkInfo);
}

int32_t HpaeProcessCluster::HandleRendererStop(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    uint32_t sessionId = nodeInfo.sessionId;
    if (renderEffectNode_ != nullptr && GetPreOutNum() == 0) {
        if (hasPendingStop_) {
            AUDIO_WARNING_LOG("overwrite pending stop %{public}u with %{public}u", pendingStopSessionId_, sessionId);
        }
        hasPendingStop_ = true;
        pendingStopSessionId_ = sessionId;
        pendingStopNodeInfo_ = nodeInfo;
        AUDIO_INFO_LOG("sessionId %{public}u will be stop (deferred)", sessionId);
        return SUCCESS;
    }
    return AudioRendererStop(nodeInfo, sinkInfo);
}

int32_t HpaeProcessCluster::AudioRendererStart(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    if (renderEffectNode_ != nullptr) {
        return renderEffectNode_->AudioRendererStart(nodeInfo);
    }

    if (renderNoneEffectNode_ != nullptr && CheckNeedNotifyEffectNode(sinkInfo)) {
        return renderNoneEffectNode_->AudioRendererStart(nodeInfo);
    }

    return 0;
}

int32_t HpaeProcessCluster::AudioRendererStop(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    if (renderEffectNode_ != nullptr) {
        return renderEffectNode_->AudioRendererStop(nodeInfo);
    }

    if (renderNoneEffectNode_ != nullptr && CheckNeedNotifyEffectNode(sinkInfo)) {
        return renderNoneEffectNode_->AudioRendererStop(nodeInfo);
    }

    return 0;
}

void HpaeProcessCluster::ExecuteDeferredStop()
{
    if (!hasPendingStop_) {
        return;
    }
    AUDIO_INFO_LOG("sessionId %{public}u execute deferred stop", pendingStopSessionId_);
    AudioRendererStop(pendingStopNodeInfo_, sinkInfo_);
    ClearDeferredStop();
}

bool HpaeProcessCluster::ExecuteDeferredStopFor(uint32_t sessionId)
{
    if (!hasPendingStop_ || pendingStopSessionId_ != sessionId) {
        return false;
    }
    ExecuteDeferredStop();
    return true;
}

void HpaeProcessCluster::ClearDeferredStop()
{
    hasPendingStop_ = false;
    pendingStopSessionId_ = 0;
}

int32_t HpaeProcessCluster::AudioRendererRelease(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    if (hasPendingStop_ && pendingStopSessionId_ == nodeInfo.sessionId) {
        ClearDeferredStop();
    }
    if (renderEffectNode_ != nullptr) {
        return renderEffectNode_->AudioRendererRelease(nodeInfo);
    }

    if (renderNoneEffectNode_ != nullptr && CheckNeedNotifyEffectNode(sinkInfo)) {
        return renderNoneEffectNode_->AudioRendererRelease(nodeInfo);
    }

    return 0;
}

bool HpaeProcessCluster::CheckNeedNotifyEffectNode(const HpaeSinkInfo &sinkInfo)
{
    if (IsRemoteDeviceClass(sinkInfo.deviceClass)
        || sinkInfo.deviceName == "DP_MCH_speaker") {
        return false;
    }

    return true;
}

std::shared_ptr<HpaeGainNode> HpaeProcessCluster::GetGainNodeById(uint32_t sessionId) const
{
    return SafeGetMap(idGainMap_, sessionId);
}

std::shared_ptr<HpaeAudioFormatConverterNode> HpaeProcessCluster::GetConverterNodeById(uint32_t sessionId) const
{
    return SafeGetMap(idConverterMap_, sessionId);
}

void HpaeProcessCluster::SetConnectedFlag(bool flag)
{
    isConnectedToOutputCluster = flag;
}

bool HpaeProcessCluster::GetConnectedFlag() const
{
    return isConnectedToOutputCluster;
}

int32_t HpaeProcessCluster::SetupAudioLimiter()
{
    if (mixerNode_ != nullptr) {
        return mixerNode_->SetupAudioLimiter();
    }
    AUDIO_ERR_LOG("mixerNode_ is nullptr");
    return ERROR;
}

int32_t HpaeProcessCluster::SetLoudnessGain(uint32_t sessionId, float loudnessGain)
{
    AUDIO_INFO_LOG("set sessionId %{public}d loudness gain to %{public}f", sessionId, loudnessGain);
    std::shared_ptr<HpaeLoudnessGainNode> loudneesGainNode = SafeGetMap(idLoudnessGainNodeMap_, sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(loudneesGainNode, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::PLAY,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "loudnessGainNode does not exist", false),
        "sessionId %{public}d loudnessGainNode doesNodeExists", sessionId);
    return loudneesGainNode->SetLoudnessGain(loudnessGain);
}

uint64_t HpaeProcessCluster::GetLatency(uint32_t sessionId)
{
    uint64_t latency = 0;

    latency += SafeGetMap(idConverterMap_, sessionId) ? idConverterMap_[sessionId]->GetLatency() : 0;

    latency += SafeGetMap(idSessionEffectMap_, sessionId) ? idSessionEffectMap_[sessionId]->GetLatency(sessionId) : 0;

    latency += SafeGetMap(idLoudnessGainNodeMap_, sessionId) ? idLoudnessGainNodeMap_[sessionId]->GetLatency() : 0;

    latency += SafeGetMap(idGainMap_, sessionId) ? idGainMap_[sessionId]->GetLatency() : 0;

    latency += mixerNode_ ? mixerNode_->GetLatency() : 0;

    latency += renderEffectNode_ ? renderEffectNode_->GetLatency() : 0;

    return latency;
}

int32_t HpaeProcessCluster::CreateSessionEffectNode(uint32_t sessionId, const std::string &effectName,
    std::shared_ptr<IEffectInstance> instance, const AudioEffectConfig &config,
    bool aissSwitchEnabled)
{
    if (SafeGetMap(idSessionEffectMap_, sessionId) != nullptr) {
        AUDIO_WARNING_LOG("sessionId %{public}u already has session effect node", sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_ALREADY_BOUND);
    }
    auto converterNode = SafeGetMap(idConverterMap_, sessionId);
    auto loudnessGainNode = SafeGetMap(idLoudnessGainNodeMap_, sessionId);
    if (converterNode == nullptr || loudnessGainNode == nullptr) {
        AUDIO_WARNING_LOG("sessionId %{public}u converter or loudnessGain node not found", sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::ERR_SESSION_NOT_FOUND);
    }

    HpaeNodeInfo nodeInfo = converterNode->GetNodeInfo();
    auto sessionEffectNode = std::make_shared<HpaeSessionEffectNode>(nodeInfo);

    int32_t ret = sessionEffectNode->CreateEffectWithInstance(effectName, instance, config, aissSwitchEnabled);
    if (ret != 0) {
        AUDIO_WARNING_LOG("sessionId %{public}u create effect failed, ret=%{public}d", sessionId, ret);
        return ret;
    }

    // Insert between ConverterNode and LoudnessGainNode:
    // Original: ConverterNode <- LoudnessGainNode
    // After:    ConverterNode <- SessionEffectNode <- LoudnessGainNode
    loudnessGainNode->DisConnect(converterNode);
    sessionEffectNode->Connect(converterNode);
    loudnessGainNode->Connect(sessionEffectNode);

    idSessionEffectMap_[sessionId] = sessionEffectNode;
    AUDIO_INFO_LOG("sessionId %{public}u session effect node created, effect=%{public}s",
        sessionId, effectName.c_str());
    return 0;
}

int32_t HpaeProcessCluster::DestroySessionEffectNode(uint32_t sessionId, uint64_t &outLatency)
{
    // Synchronous immediate destroy: Begin + Finalize in one call (no smooth-close opportunity).
    // Used by non-smooth paths (cluster refresh) and focused tests.
    std::weak_ptr<HpaeSessionEffectNode> weakNode;
    int32_t ret = BeginDestroySessionEffectNode(sessionId, outLatency, weakNode);
    CHECK_AND_RETURN_RET(ret == 0, ret);
    FinalizeDestroySessionEffectNode(sessionId, weakNode, nullptr);
    return 0;
}

int32_t HpaeProcessCluster::BeginDestroySessionEffectNode(uint32_t sessionId, uint64_t &outLatency,
    std::weak_ptr<HpaeSessionEffectNode> &outWeakNode)
{
    outLatency = 0;
    auto node = SafeGetMap(idSessionEffectMap_, sessionId);
    if (node == nullptr) {
        AUDIO_WARNING_LOG("BeginDestroySessionEffectNode: sessionId %{public}u SE node not found",
            sessionId);
        return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
    }

    node->StopPreheatThread();
    // Pipeline latency the wet output lags behind upstream dry = SE + the converter immediately
    // upstream of the SE insert point. Flush it; the manager rewinds SinkInputNode by this total.
    outLatency = node->GetLatency(sessionId);
    auto converterNode = SafeGetMap(idConverterMap_, sessionId);
    if (converterNode != nullptr) {
        outLatency += converterNode->GetLatency();
        converterNode->FlushBuffers();
    }
    // Enter STOPPING; node stays in idSessionEffectMap_ until Finalize (next queue cycle) erases it.
    node->StartCloseCrossfade();
    outWeakNode = node;
    AUDIO_INFO_LOG("BeginDestroySessionEffectNode: session %{public}u BeginClose, latency=%" PRIu64,
        sessionId, outLatency);
    return 0;
}

bool HpaeProcessCluster::FinalizeDestroySessionEffectNode(uint32_t sessionId,
    const std::weak_ptr<HpaeSessionEffectNode> &closingNode,
    std::shared_ptr<IEffectInstance> pendingInstance)
{
    auto node = closingNode.lock();
    auto current = SafeGetMap(idSessionEffectMap_, sessionId);
    // Identity check: no-op if the closing node was already replaced/destroyed (DestroyStream,
    // Reload, DestroyNodes, or a recreated node).
    CHECK_AND_RETURN_RET(node != nullptr && current == node, false);

    auto converterNode = SafeGetMap(idConverterMap_, sessionId);
    auto loudnessGainNode = SafeGetMap(idLoudnessGainNodeMap_, sessionId);
    // Restore original chain: ConverterNode <- LoudnessGainNode
    if (loudnessGainNode != nullptr && converterNode != nullptr) {
        loudnessGainNode->DisConnect(node);
        node->DisConnect(converterNode);
        loudnessGainNode->Connect(converterNode);
    }

    // If a pending rapid-recreate will reuse this exact instance, do NOT Flush/Stop it — the new
    // binding takes the instance over. Otherwise release normally.
    auto closingInst = node->GetEffectInstance();
    bool sameInstanceReuse = pendingInstance != nullptr && closingInst != nullptr &&
        pendingInstance.get() == closingInst.get();
    if (sameInstanceReuse) {
        node->CloseSessionEffectCore();
    } else {
        node->DropEffect();
    }
    idSessionEffectMap_.erase(sessionId);
    AUDIO_INFO_LOG("FinalizeDestroySessionEffectNode: session %{public}u finalized (sameInstance=%{public}d)",
        sessionId, static_cast<int32_t>(sameInstanceReuse));
    return true;
}

bool HpaeProcessCluster::IsSessionEffectNodeStopping(uint32_t sessionId) const
{
    auto node = SafeGetMap(idSessionEffectMap_, sessionId);
    return node != nullptr && node->GetStatus() == HpaeSessionEffectNode::STATUS::STOPPING;
}

bool HpaeProcessCluster::HasSessionEffectNode(uint32_t sessionId) const
{
    return SafeGetMap(idSessionEffectMap_, sessionId) != nullptr;
}

int32_t HpaeProcessCluster::SessionEffectStart(uint32_t sessionId)
{
    auto node = SafeGetMap(idSessionEffectMap_, sessionId);
    if (node == nullptr) {
        return static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND);
    }
    return node->StartEffect();
}

int32_t HpaeProcessCluster::SessionEffectFlush(uint32_t sessionId)
{
    auto node = SafeGetMap(idSessionEffectMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND),
        "SessionEffectFlush: no SE for session %{public}u", sessionId);
    return node->FlushEffect();
}

int32_t HpaeProcessCluster::SessionEffectPause(uint32_t sessionId)
{
    auto node = SafeGetMap(idSessionEffectMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND),
        "SessionEffectPause: no SE for session %{public}u", sessionId);
    return node->PauseEffect();
}

void HpaeProcessCluster::SessionEffectPrepareMoveStream(uint32_t sessionId)
{
    auto node = SafeGetMap(idSessionEffectMap_, sessionId);
    if (node != nullptr) {
        node->PrepareMoveStream();
    }
}

int32_t HpaeProcessCluster::SetSessionEffectParameter(uint32_t sessionId, int32_t type, float value)
{
    auto node = SafeGetMap(idSessionEffectMap_, sessionId);
    CHECK_AND_RETURN_RET_LOG(node != nullptr,
        static_cast<int32_t>(SessionEffectErrCode::ERR_EFFECT_NOT_BOUND),
        "SetSessionEffectParameter: no SE node for session %{public}u", sessionId);
    return node->SetParameter(type, value);
}

std::shared_ptr<HpaeSessionEffectNode> HpaeProcessCluster::GetSessionEffectNode(uint32_t sessionId) const
{
    auto it = idSessionEffectMap_.find(sessionId);
    return it != idSessionEffectMap_.end() ? it->second : nullptr;
}

std::shared_ptr<HpaeLoudnessGainNode> HpaeProcessCluster::GetLoudnessGainNode(uint32_t sessionId) const
{
    return SafeGetMap(idLoudnessGainNodeMap_, sessionId);
}

std::shared_ptr<HpaeLoudnessGainNode> HpaeProcessCluster::GetRenderEffectNode() const
{
    return renderEffectNode_;
}

void HpaeProcessCluster::SetCollaborationStateToMixNode(CollaborationState isCollaborationEnabled)
{
    if (mixerNode_ != nullptr) {
        mixerNode_->SetCollaborationStateToMixNode(isCollaborationEnabled);
    }

    return;
}

void HpaeProcessCluster::SetDelayCountToEffectNode(int32_t delayCount)
{
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->SetDelayCountToEffectNode(delayCount);
    }

    return;
}

void HpaeProcessCluster::SetCollabDoFade(bool fadeOutState)
{
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->SetCollabDoFade(fadeOutState);
    }

    return;
}

void HpaeProcessCluster::SetStopRunningState(bool stopRunning)
{
    if (renderEffectNode_ != nullptr) {
        renderEffectNode_->SetStopRunningState(stopRunning);
    }

    return;
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
