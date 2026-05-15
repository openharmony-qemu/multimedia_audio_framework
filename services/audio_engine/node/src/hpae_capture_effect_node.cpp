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
#ifdef ENABLE_HOOK_PCM
    if (sceneType_ == "SCENE_COLLABORATIVE_RECORD") {
        inputMicDumper_ = std::make_unique<HpaePcmDumper>(
            "HpaeCaptureEffectNode_id_" + std::to_string(GetNodeId()) + "_mic.pcm");
        inputRefDumper_ = std::make_unique<HpaePcmDumper>(
            "HpaeCaptureEffectNode_id_" + std::to_string(GetNodeId()) + "_collabMic.pcm");
        outputDumper_ = std::make_unique<HpaePcmDumper>(
            "HpaeCaptureEffectNode_id_" + std::to_string(GetNodeId()) + "_out.pcm");
    }
#endif
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

HpaePcmBuffer *HpaeCaptureEffectNode::SignalProcessCollaboration(const std::vector<HpaePcmBuffer*> &inputs)
{
    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    EnhanceTransBuffer transBuf = {};
    uint32_t processLength = 0;

    // Collaborative recording: float data pass-through, no format conversion
    for (uint32_t i = 0; i < inputs.size(); i++) {
        if (inputs[i]->GetSourceBufferType() == HPAE_SOURCE_BUFFER_TYPE_MIC) {
#ifdef ENABLE_HOOK_PCM
            inputMicDumper_->Dump((int8_t *)inputs[i]->GetPcmDataBuffer(), micBufferLength_);
#endif
            auto ret = memcpy_s(micCache_.data(), micCache_.size(),
                inputs[i]->GetPcmDataBuffer(), micBufferLength_);
            CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == EOK, nullptr,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                        ERR_INVALID_PARAM), "mic memcpy error", false),
                "mic memcpy error");
            transBuf.micData = static_cast<void *>(micCache_.data());
            transBuf.micDataLen = micCache_.size();
            processLength = micBufferLength_;
            AUDIO_DEBUG_LOG("collaborative mic size:%{public}u", processLength);
        } else if (inputs[i]->GetSourceBufferType() == HPAE_SOURCE_BUFFER_TYPE_DEFAULT) {
#ifdef ENABLE_HOOK_PCM
            inputRefDumper_->Dump((int8_t *)inputs[i]->GetPcmDataBuffer(), micBufferLength_);
#endif
            auto ret = memcpy_s(micRefCache_.data(), micRefCache_.size(),
                inputs[i]->GetPcmDataBuffer(), micrefBufferLength_);
            CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == EOK, nullptr,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                        ERR_INVALID_PARAM), "collab mic memcpy error", false),
                "collab mic memcpy error");
            transBuf.micRefData = static_cast<void *>(micRefCache_.data());
            transBuf.micRefDataLen = micRefCache_.size();
            AUDIO_DEBUG_LOG("collaborative ref size:%{public}u", micrefBufferLength_);
        }
    }
    outPcmBuffer_->SetBufferValid(processLength != 0);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(processLength != 0, outPcmBuffer_.get(),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD, BusinessScenario::QUERY,
                ERR_INVALID_PARAM), "main mic data is null", false),
        "main mic data is null");
    int32_t ret = audioEnhanceChainManager->ApplyEnhanceChainById(sceneKeyCode_, transBuf);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, outPcmBuffer_.get(),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                ERR_OPERATION_FAILED), "effect apply failed", false),
        "effect apply failed, ret:%{public}d", ret);
    audioEnhanceChainManager->GetChainOutputDataById(sceneKeyCode_, static_cast<void *>(cacheDataOut_.data()),
        static_cast<size_t>(processLength));
    ret = memcpy_s(outPcmBuffer_->GetPcmDataBuffer(), processLength, cacheDataOut_.data(), processLength);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == EOK, nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                ERR_INVALID_PARAM), "effect out cpy error", false),
        "effect out cpy error");
#ifdef ENABLE_HOOK_PCM
    outputDumper_->Dump((int8_t *)cacheDataOut_.data(), micBufferLength_);
#endif
    return outPcmBuffer_.get();
}

HpaePcmBuffer *HpaeCaptureEffectNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    Trace trace("[" + sceneType_ + "]HpaeCaptureEffectNode::SignalProcess inputs num[" +
        std::to_string(inputs.size()) + "]");
    if (inputs.empty()) {
        AUDIO_WARNING_LOG("inputs size is empty, SessionId:%{public}d", GetSessionId());
        return nullptr;
    }

    if (sceneType_ == "SCENE_COLLABORATIVE_RECORD") {
        return SignalProcessCollaboration(inputs);
    }

    AudioEnhanceChainManager *audioEnhanceChainManager = AudioEnhanceChainManager::GetInstance();
    EnhanceTransBuffer transBuf = {};
    uint32_t processLength = 0;
    for (uint32_t i = 0; i < inputs.size(); i++) {
        if (inputs[i]->GetSourceBufferType() == HPAE_SOURCE_BUFFER_TYPE_MIC) {
            ConvertFromFloat(SAMPLE_S16LE, micBufferLength_ / GetSizeFromFormat(SAMPLE_S16LE),
                inputs[i]->GetPcmDataBuffer(), static_cast<void *>(micCache_.data()));
            transBuf.micData = static_cast<void *>(micCache_.data());
            transBuf.micDataLen = micCache_.size();
            processLength = micBufferLength_;
            AUDIO_DEBUG_LOG("CopyToEnhanceBuffer size:%{public}u", processLength);
        } else if (inputs[i]->GetSourceBufferType() == HPAE_SOURCE_BUFFER_TYPE_EC) {
            ConvertFromFloat(SAMPLE_S16LE, ecBufferLength_ / GetSizeFromFormat(SAMPLE_S16LE),
                inputs[i]->GetPcmDataBuffer(), static_cast<void *>(ecCache_.data()));
            transBuf.ecData = static_cast<void *>(ecCache_.data());
            transBuf.ecDataLen = ecCache_.size();
            AUDIO_DEBUG_LOG("CopyEcToEnhanceBuffer size:%{public}u", ecBufferLength_);
        } else if (inputs[i]->GetSourceBufferType() == HPAE_SOURCE_BUFFER_TYPE_MICREF) {
            ConvertFromFloat(SAMPLE_S16LE, micrefBufferLength_ / GetSizeFromFormat(SAMPLE_S16LE),
                inputs[i]->GetPcmDataBuffer(), static_cast<void *>(micRefCache_.data()));
            transBuf.micRefData = static_cast<void *>(micRefCache_.data());
            transBuf.micRefDataLen = micRefCache_.size();
            AUDIO_DEBUG_LOG("CopyMicRefToEnhanceBuffer size:%{public}u", micrefBufferLength_);
        }
    }

    outPcmBuffer_->SetBufferValid(processLength != 0);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(processLength != 0, outPcmBuffer_.get(),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_STATE_INCONSISTENT, OperationType::RECORD, BusinessScenario::QUERY,
                ERR_INVALID_PARAM), "main mic data is null", false),
        "main mic data is null");

    int32_t ret = audioEnhanceChainManager->ApplyEnhanceChainById(sceneKeyCode_, transBuf);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, outPcmBuffer_.get(),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            BuildErrorCode(ProblemCategory::FAULT_NO_SOUND, OperationType::RECORD, BusinessScenario::CREATE,
                ERR_OPERATION_FAILED), "effect apply failed", false),
        "effect apply failed, ret:%{public}d", ret);
    audioEnhanceChainManager->GetChainOutputDataById(sceneKeyCode_, static_cast<void *>(cacheDataOut_.data()),
        static_cast<size_t>(processLength));
    ConvertToFloat(SAMPLE_S16LE, micBufferLength_ / GetSizeFromFormat(SAMPLE_S16LE),
        static_cast<void *>(cacheDataOut_.data()), outPcmBuffer_->GetPcmDataBuffer());
    return outPcmBuffer_.get();
}

void HpaeCaptureEffectNode::ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
    HpaeNodeInfo &nodeInfo)
{
    std::shared_ptr<HpaeNode> realPreNode = preNode->GetSharedInstance(nodeInfo);
    CHECK_AND_RETURN_LOG(realPreNode != nullptr, "realPreNode is nullptr");
    inputStream_.Connect(realPreNode, preNode->GetOutputPort(nodeInfo));
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

void HpaeCaptureEffectNode::SetCapturerEffectConfig(AudioBufferConfig micConfig, AudioBufferConfig ecConfig,
    AudioBufferConfig micrefConfig)
{
    HpaeNodeInfo micInfo = GetNodeInfo();
    HpaeNodeInfo ecInfo = GetNodeInfo();
    HpaeNodeInfo micrefInfo = GetNodeInfo();
    micInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MIC;
    micInfo.frameLen = FRAME_LEN_20MS * (micConfig.samplingRate / MILLISECOND_PER_SECOND);
    micInfo.samplingRate = static_cast<AudioSamplingRate>(micConfig.samplingRate);
    micInfo.channels = static_cast<AudioChannel>(micConfig.channels);
    micInfo.format = static_cast<AudioSampleFormat>(micConfig.format / BITLENGTH - 1);
    GetCaptureEffectMicChannelLayout(micConfig.channels, micInfo.channelLayout);
    ecInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_EC;
    ecInfo.frameLen = FRAME_LEN_20MS * (ecConfig.samplingRate / MILLISECOND_PER_SECOND);
    ecInfo.samplingRate = static_cast<AudioSamplingRate>(ecConfig.samplingRate);
    ecInfo.channels = static_cast<AudioChannel>(ecConfig.channels);
    ecInfo.format = static_cast<AudioSampleFormat>(ecConfig.format / BITLENGTH - 1);
    micrefInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_MICREF;
    micrefInfo.frameLen = FRAME_LEN_20MS * (micrefConfig.samplingRate / MILLISECOND_PER_SECOND);
    micrefInfo.samplingRate = static_cast<AudioSamplingRate>(micrefConfig.samplingRate);
    micrefInfo.channels = static_cast<AudioChannel>(micrefConfig.channels);
    micrefInfo.format = static_cast<AudioSampleFormat>(micrefConfig.format / BITLENGTH - 1);
    capturerEffectConfigMap_.emplace(HPAE_SOURCE_BUFFER_TYPE_MIC, micInfo);
    capturerEffectConfigMap_.emplace(HPAE_SOURCE_BUFFER_TYPE_EC, ecInfo);
    if (sceneType_ == "SCENE_COLLABORATIVE_RECORD") {
        capturerEffectConfigMap_[HPAE_SOURCE_BUFFER_TYPE_MIC].format = SAMPLE_F32LE;
        micrefInfo.format = SAMPLE_F32LE;
        micrefInfo.sourceBufferType = HPAE_SOURCE_BUFFER_TYPE_DEFAULT;
        capturerEffectConfigMap_.emplace(HPAE_SOURCE_BUFFER_TYPE_DEFAULT, micrefInfo);
    } else {
        capturerEffectConfigMap_.emplace(HPAE_SOURCE_BUFFER_TYPE_MICREF, micrefInfo);
    }
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
    micBufferLength_ = FRAME_LEN_20MS * micConfig.channels * (micConfig.samplingRate / MILLISECOND_PER_SECOND) *
        (micConfig.format / BITLENGTH);
    ecBufferLength_ = FRAME_LEN_20MS * ecConfig.channels * (ecConfig.samplingRate / MILLISECOND_PER_SECOND) *
        (ecConfig.format / BITLENGTH);
    micrefBufferLength_ = FRAME_LEN_20MS * micrefConfig.channels *
        (micrefConfig.samplingRate / MILLISECOND_PER_SECOND) * (micrefConfig.format / BITLENGTH);
    uint32_t maxLength = (micBufferLength_ > ecBufferLength_) ?
        (micBufferLength_ > micrefBufferLength_ ? micBufferLength_ : micrefBufferLength_) :
        (ecBufferLength_ > micrefBufferLength_ ? ecBufferLength_ : micrefBufferLength_);
    AUDIO_INFO_LOG("micLength: %{public}u, ecLength: %{public}u, micrefLength: %{public}u, maxLength:%{public}u",
        micBufferLength_, ecBufferLength_, micrefBufferLength_, maxLength);
    ecCache_.resize(ecBufferLength_);
    micCache_.resize(micBufferLength_);
    micRefCache_.resize(micrefBufferLength_);
    cacheDataOut_.resize(maxLength);
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
