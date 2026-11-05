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
#define LOG_TAG "HpaeCaptureEffectNode"
#endif

#include "hpae_capture_effect_node.h"
#include <iostream>
#include "hpae_pcm_buffer.h"
#include "audio_errors.h"
#include "hpae_format_convert.h"
#include "audio_enhance_chain_manager.h"
#include "audio_effect_map.h"
#include "audio_utils.h"
#include "audio_effect_log.h"
#include "stream_dfx_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeCaptureEffectNode::HpaeCaptureEffectNode(HpaeNodeInfo &nodeInfo)
    : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo)
{
    const std::unordered_map<AudioEnhanceScene, std::string> &audioEnhanceSupportedSceneTypes =
        GetEnhanceSupportedSceneType();
    auto item = audioEnhanceSupportedSceneTypes.find(nodeInfo.effectInfo.enhanceScene);
    if (item != audioEnhanceSupportedSceneTypes.end()) {
        sceneType_ = item->second;
        AUDIO_INFO_LOG("created scenetype: [%{public}s]", sceneType_.c_str());
    } else {
        AUDIO_ERR_LOG("scenetype: %{public}u not supported", nodeInfo.effectInfo.enhanceScene);
    }
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeCaptureEffectNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
    RebuildTraceName();
}

void HpaeCaptureEffectNode::OnTraceInfoChanged()
{
    HpaePluginNode::OnTraceInfoChanged();
    RebuildTraceName();
}

void HpaeCaptureEffectNode::RebuildTraceName()
{
    traceName_ = "[" + sceneType_ + "]HpaeCaptureEffectNode::SignalProcess inputs num[" +
        std::to_string(inputStream_.GetPreOutputNum()) + "]";
}

void HpaeCaptureEffectNode::Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaePluginNode::Connect(preNode);
    RebuildTraceName();
}

void HpaeCaptureEffectNode::DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode)
{
    HpaePluginNode::DisConnect(preNode);
    RebuildTraceName();
}

HpaeCaptureEffectNode::~HpaeCaptureEffectNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

bool HpaeCaptureEffectNode::Reset()
{
    return HpaePluginNode::Reset();
}

HpaePcmBuffer *HpaeCaptureEffectNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    Trace trace(traceName_);
    if (inputs.empty()) {
        if (inputsEmptyGate_.ShouldLog()) {
            AUDIO_WARNING_LOG("inputs size is empty, SessionId:%{public}d", GetSessionId());
        }
        return nullptr;
    }
    if (inputsEmptyGate_.IsActive()) {
        AUDIO_WARNING_LOG("inputs size empty recovered, SessionId:%{public}d, frames:%{public}u",
            GetSessionId(), inputsEmptyGate_.Reset());
    }

    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    EnhanceTransBuffer transBuf = {};
    uint32_t processLength = 0;
    for (uint32_t i = 0; i < inputs.size(); i++) {
        HpaeSourceBufferType sourceBufType = inputs[i]->GetSourceBufferType();
        if (sourceBufType == HPAE_SOURCE_BUFFER_TYPE_MIC) {
            transBuf.micData = static_cast<void *>(inputs[i]->GetPcmDataBuffer());
            transBuf.micDataLen = inputs[i]->DataSize();
        } else if (sourceBufType == HPAE_SOURCE_BUFFER_TYPE_EC) {
            transBuf.ecData = static_cast<void *>(inputs[i]->GetPcmDataBuffer());
            transBuf.ecDataLen = inputs[i]->DataSize();
        } else if (sourceBufType == HPAE_SOURCE_BUFFER_TYPE_MICREF) {
            transBuf.micRefData = static_cast<void *>(inputs[i]->GetPcmDataBuffer());
            transBuf.micRefDataLen = inputs[i]->DataSize();
        }
    }

    outPcmBuffer_->SetBufferValid(processLength != 0);
    if (processLength != 0 && micNullLogActive_) {
        AUDIO_INFO_LOG("main mic data recovered after %{public}u empty frames", micNullLogCount_);
        micNullLogActive_ = false;
        micNullLogCount_ = 0;
    }
    if (processLength == 0) {
        micNullLogCount_++;
        if (!micNullLogActive_ || micNullLogCount_ % captureLogHeartbeatFrames == 0) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                    BusinessScenario::QUERY, ERR_INVALID_PARAM), "main mic data is null", false);
            AUDIO_ERR_LOG("main mic data is null");
        }
        micNullLogActive_ = true;
        return outPcmBuffer_.get();
    }

    int32_t ret = audioEnhanceChainManager->ApplyEnhanceChainById(sceneKeyCode_, transBuf);
    if (ret == SUCCESS && applyFailLogActive_) {
        AUDIO_INFO_LOG("effect apply recovered after %{public}u failed frames", applyFailLogCount_);
        applyFailLogActive_ = false;
        applyFailLogCount_ = 0;
    }
    if (ret != SUCCESS) {
        applyFailLogCount_++;
        if (!applyFailLogActive_ || applyFailLogCount_ % captureLogHeartbeatFrames == 0) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                    ERR_OPERATION_FAILED), "effect apply failed", false);
            AUDIO_ERR_LOG("effect apply failed, ret:%{public}d", ret);
        }
        applyFailLogActive_ = true;
        return outPcmBuffer_.get();
    }
    audioEnhanceChainManager->GetChainOutputDataById(sceneKeyCode_, outPcmBuffer_->GetPcmDataBuffer(),
        outPcmBuffer_->DataSize());
    return outPcmBuffer_.get();
}

void HpaeCaptureEffectNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    std::shared_ptr<HpaeNode> realPreNode = preNode->GetSharedInstance(nodeInfo);
    CHECK_AND_RETURN_LOG(realPreNode != nullptr, "realPreNode is nullptr");
    inputStream_.Connect(realPreNode, preNode->GetOutputPort(nodeInfo));
    RebuildTraceName();
#ifdef ENABLE_HIDUMP_DFX
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(true, realPreNode->GetNodeId(), GetNodeId());
    }
#endif
}

void HpaeCaptureEffectNode::DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode,
    HpaeNodeInfo &nodeInfo)
{
    CHECK_AND_RETURN_LOG(!inputStream_.CheckIfDisConnected(preNode->GetOutputPort(nodeInfo)),
        "%{public}u has disconnected with preNode", GetNodeId());
    const auto port = preNode->GetOutputPort(nodeInfo, true);
    inputStream_.DisConnect(port);
    RebuildTraceName();
#ifdef ENABLE_HIDUMP_DFX
    CHECK_AND_RETURN_LOG(port != nullptr, "port is nullptr");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeInfo(false, port->GetNodeId(), GetNodeId());
    }
#endif
}

bool HpaeCaptureEffectNode::GetCapturerEffectConfig(HpaeNodeInfo& nodeInfo, HpaeSourceBufferType type)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(capturerEffectConfigMap_.find(type) != capturerEffectConfigMap_.end(), false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "config not found", false),
        "not need resample node, type:%{public}u", type);
    nodeInfo = capturerEffectConfigMap_[type];
    return true;
}

void HpaeCaptureEffectNode::GetCaptureEffectMicChannelLayout(uint32_t channels, AudioChannelLayout &channelLayout)
{
    if (channels == 2) { // 2 is stereo
        channelLayout = CH_LAYOUT_STEREO;
    } else if (channels == 4) { // 4 is QUAD_SIDE
        channelLayout = CH_LAYOUT_QUAD_SIDE;
    } else {
        AUDIO_WARNING_LOG("channel is not meet expectations");
    }
}

void HpaeCaptureEffectNode::SetCapturerEffectConfig(const AudioBufferConfig micConfig,
    const AudioBufferConfig ecConfig, const AudioBufferConfig micrefConfig)
{
    HpaeNodeInfo micInfo = GetNodeInfo();
    HpaeNodeInfo ecInfo = GetNodeInfo();
    HpaeNodeInfo micrefInfo = GetNodeInfo();
    micInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    micInfo.frameLen = FRAME_LEN_20MS * (micConfig.samplingRate / MILLISECOND_PER_SECOND);
    micInfo.samplingRate = static_cast<AudioSamplingRate>(micConfig.samplingRate);
    micInfo.channels = static_cast<AudioChannel>(micConfig.channels);
    micInfo.format = SAMPLE_F32LE;
    GetCaptureEffectMicChannelLayout(micConfig.channels, micInfo.channelLayout);
    ecInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    ecInfo.frameLen = FRAME_LEN_20MS * (ecConfig.samplingRate / MILLISECOND_PER_SECOND);
    ecInfo.samplingRate = static_cast<AudioSamplingRate>(ecConfig.samplingRate);
    ecInfo.channels = static_cast<AudioChannel>(ecConfig.channels);
    ecInfo.format = SAMPLE_F32LE;
    micrefInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MICREF;
    micrefInfo.frameLen = FRAME_LEN_20MS * (micrefConfig.samplingRate / MILLISECOND_PER_SECOND);
    micrefInfo.samplingRate = static_cast<AudioSamplingRate>(micrefConfig.samplingRate);
    micrefInfo.channels = static_cast<AudioChannel>(micrefConfig.channels);
    micrefInfo.format = SAMPLE_F32LE;
    capturerEffectConfigMap_.emplace(HPAE_SOURCE_BUFFER_TYPE_MIC, micInfo);
    capturerEffectConfigMap_.emplace(HPAE_SOURCE_BUFFER_TYPE_EC, ecInfo);
    capturerEffectConfigMap_.emplace(HPAE_SOURCE_BUFFER_TYPE_MICREF, micrefInfo);
}

int32_t HpaeCaptureEffectNode::CaptureEffectCreate(uint64_t sceneKeyCode, CaptureEffectAttr attr)
{
    sceneKeyCode_ = sceneKeyCode;
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioEnhanceChainManager, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                ERR_NULL_POINTER), "audioEnhanceChainManager is nullptr", false),
        "audioEnhanceChainManager is nullptr");
    AudioEnhanceDeviceAttr enhanceAttr = {};
    enhanceAttr.needEc = attr.needEc;
    enhanceAttr.needMicRef = attr.needMicRef;
    enhanceAttr.micChannels = attr.micChannels;
    enhanceAttr.ecChannels = attr.ecChannels;
    enhanceAttr.micRefChannels = attr.micRefChannels;
    int32_t ret = audioEnhanceChainManager->CreateAudioEnhanceChainDynamic(sceneKeyCode, enhanceAttr);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                ERR_OPERATION_FAILED), "create chain failed", false),
        "failed, ret:%{public}d", ret);

    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micrefConfig = {};
    ret = audioEnhanceChainManager->AudioEnhanceChainGetAlgoConfig(sceneKeyCode, micConfig, ecConfig, micrefConfig);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == 0 && micConfig.samplingRate != 0, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD,
                BusinessScenario::CREATE, ERR_INVALID_PARAM), "get algo config failed", false),
        "get algo config failed, ret:%{public}d", ret);
    SetCapturerEffectConfig(micConfig, ecConfig, micrefConfig);
    AudioChannelLayout channelLayout = CH_LAYOUT_UNKNOWN;
    GetCaptureEffectMicChannelLayout(micConfig.channels, channelLayout);
    PcmBufferInfo pcmBufferInfo(micConfig.channels, FRAME_LEN_20MS * (micConfig.samplingRate / MILLISECOND_PER_SECOND),
        micConfig.samplingRate, channelLayout);
    outPcmBuffer_ = std::make_unique<HpaePcmBuffer>(pcmBufferInfo);
    if (outPcmBuffer_ == nullptr) {
        AUDIO_ERR_LOG("create effect out pcm buffer fail");
        return ERROR;
    }

    return ret;
}

int32_t HpaeCaptureEffectNode::CaptureEffectRelease(uint64_t sceneKeyCode)
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioEnhanceChainManager, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::RELEASE, ERR_NULL_POINTER),
            "audioEnhanceChainManager is nullptr", false),
        "audioEnhanceChainManager is nullptr");
    return audioEnhanceChainManager->ReleaseAudioEnhanceChainDynamic(sceneKeyCode);
}

uint64_t HpaeCaptureEffectNode::GetLatency(uint32_t sessionId)
{
    return 0;
}

int32_t HpaeCaptureEffectNode::SetAppsEnhanceMuteState(bool isMute)
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioEnhanceChainManager, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD,
                BusinessScenario::QUERY, ERR_NULL_POINTER),
            "audioEnhanceChainManager is nullptr", false),
        "audioEnhanceChainManager is nullptr");
    return audioEnhanceChainManager->SetAppsMuteStateToChainById(sceneKeyCode_, isMute);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
