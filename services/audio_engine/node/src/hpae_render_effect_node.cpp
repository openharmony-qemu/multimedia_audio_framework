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
#define LOG_TAG "HpaeRenderEffectNode"
#endif

#include <cinttypes>
#include "audio_errors.h"
#include "hpae_render_effect_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect_map.h"
#include "audio_utils.h"
#include "audio_effect_log.h"

static constexpr uint32_t DEFUALT_EFFECT_RATE = 48000;
static constexpr uint32_t DEFAULT_EFFECT_FRAMELEN = 960;
static constexpr int32_t COLLABORATIVE_OUTPUT_CHANNELS = 4;
static constexpr int32_t DIRECT_CHANNELS = 2;
static constexpr int32_t COLLABORATIVE_CHANNELS = 2;
static constexpr int32_t COLLABORATIVE_OUTPUT_CHANNEL_1_INDEX = 2;
static constexpr int32_t COLLABORATIVE_OUTPUT_CHANNEL_2_INDEX = 3;
static constexpr int64_t WAIT_CLOSE_EFFECT_TIME = 4; // 4s
static constexpr int64_t MONITOR_CLOSE_EFFECT_TIME = 5 * 60; // 5m
static constexpr int64_t TIME_IN_US = 1000000;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeRenderEffectNode::HpaeRenderEffectNode(HpaeNodeInfo &nodeInfo) : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    // DEFAUT effect out format
    pcmBufferInfo_(nodeInfo.channels, DEFAULT_EFFECT_FRAMELEN, DEFUALT_EFFECT_RATE, nodeInfo.channelLayout),
    effectOutput_(pcmBufferInfo_)
{
    if (nodeInfo.sceneType == HPAE_SCENE_DEFAULT) {
        sceneType_ = "SCENE_DEFAULT";
    } else {
        const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
        if (audioSupportedSceneTypes.find(nodeInfo.effectInfo.effectScene) !=
            audioSupportedSceneTypes.end()) {
            sceneType_ = audioSupportedSceneTypes.at(nodeInfo.effectInfo.effectScene);
        }
        if (sceneType_ == "SCENE_COLLABORATIVE") {
            PcmBufferInfo pcmBufferInfo(STEREO, DEFAULT_EFFECT_FRAMELEN, DEFUALT_EFFECT_RATE, CH_LAYOUT_STEREO);
            directOutput_ = std::make_unique<HpaePcmBuffer>(pcmBufferInfo);
            collaborativeOutput_ = std::make_unique<HpaePcmBuffer>(pcmBufferInfo);
        }
    }
    AUDIO_INFO_LOG("created, scene type: %{public}s", sceneType_.c_str());
#ifdef ENABLE_HOOK_PCM
    if (sceneType_ == "SCENE_COLLABORATIVE") {
        directPcmDumper_ = std::make_unique<HpaePcmDumper>(
            "HpaeRenderEffectNodeDirect_id_" + std::to_string(GetNodeId()) + "_scene_" + sceneType_ + ".pcm");
        collaborativePcmDumper_ = std::make_unique<HpaePcmDumper>(
            "HpaeRenderEffectNodeCollaborative_id_" + std::to_string(GetNodeId()) + "_scene_" + sceneType_ + ".pcm");
    }
#endif
#ifdef ENABLE_HIDUMP_DFX
    SetNodeName("hpaeRenderEffectNode");
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(true, GetNodeInfo());
    }
#endif
}

HpaeRenderEffectNode::~HpaeRenderEffectNode()
{
#ifdef ENABLE_HIDUMP_DFX
    AUDIO_INFO_LOG("NodeId: %{public}u NodeName: %{public}s destructed.",
        GetNodeId(), GetNodeName().c_str());
    if (auto callback = GetNodeStatusCallback().lock()) {
        callback->OnNotifyDfxNodeAdmin(false, GetNodeInfo());
    }
#endif
}

void HpaeRenderEffectNode::DoProcess()
{
    if (sceneType_ != "SCENE_COLLABORATIVE") {
        HpaePluginNode::DoProcess();
        return;
    }

    std::vector<HpaePcmBuffer *>& preOutputs = inputStream_.ReadPreOutputData();
    if (enableProcess_ && !preOutputs.empty() && directOutput_ && collaborativeOutput_) {
        SignalProcess(preOutputs);
        int32_t ret = SplitCollaborativeData();
        if (ret != SUCCESS) {
            outputStream_.WriteDataToOutput(&silenceData_);
            outputStream_.WriteDataToOutput(&silenceData_, HPAE_BUFFER_TYPE_COBUFFER);
        } else {
            outputStream_.WriteDataToOutput(directOutput_.get());
            outputStream_.WriteDataToOutput(collaborativeOutput_.get(), HPAE_BUFFER_TYPE_COBUFFER);
        }
    } else if (!preOutputs.empty()) {
        outputStream_.WriteDataToOutput(preOutputs[0]);
        outputStream_.WriteDataToOutput(preOutputs[0], HPAE_BUFFER_TYPE_COBUFFER);
    } else {
        outputStream_.WriteDataToOutput(&silenceData_);
        outputStream_.WriteDataToOutput(&silenceData_, HPAE_BUFFER_TYPE_COBUFFER);
    }
#ifdef ENABLE_HOOK_PCM
    if (directPcmDumper_ && directOutput_) {
        directPcmDumper_->Dump((int8_t *)directOutput_->GetPcmDataBuffer(),
            directOutput_->GetFrameLen() * sizeof(float) * directOutput_->GetChannelCount());
    }
    if (collaborativePcmDumper_ && collaborativeOutput_) {
        collaborativePcmDumper_->Dump((int8_t *)collaborativeOutput_->GetPcmDataBuffer(),
            collaborativeOutput_->GetFrameLen() * sizeof(float) * collaborativeOutput_->GetChannelCount());
    }
#endif
}

HpaePcmBuffer *HpaeRenderEffectNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    AUDIO_DEBUG_LOG("render effect node signal process in");
    if (inputs.empty()) {
        AUDIO_WARNING_LOG("inputs size is empty");
        return nullptr;
    }
    auto rate = "rate[" + std::to_string(inputs[0]->GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(inputs[0]->GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(inputs[0]->GetFrameLen()) + "]";
    Trace trace("[" + sceneType_ + "]HpaeRenderEffectNode::SignalProcess " + rate + ch + len);

    if (AudioEffectChainManager::GetInstance()->GetOffloadEnabled()) {
        return inputs[0];
    }
    if (IsByPassEffectZeroVolume(inputs[0])) {
        return inputs[0];
    }

    ReconfigOutputBuffer();

    auto eBufferAttr = std::make_unique<EffectBufferAttr>(
        inputs[0]->GetPcmDataBuffer(),
        effectOutput_.GetPcmDataBuffer(),
        static_cast<int32_t>(inputs[0]->GetChannelCount()),
        static_cast<int32_t>(inputs[0]->GetFrameLen()),
        0,
        0
    );

    int32_t ret = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType_, eBufferAttr);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, inputs[0], "apply audio effect chain fail");

    effectOutput_.SetBufferState(inputs[0]->GetBufferState());
    return &effectOutput_;
}

int32_t HpaeRenderEffectNode::SplitCollaborativeData()
{
    CHECK_AND_RETURN_RET_LOG(effectOutput_.GetChannelCount() == static_cast<uint32_t>(CHANNEL_4), ERROR,
        "collaborative channel count is invalid, count: %{public}d", CHANNEL_4);
    // need to check whether the sample rate or frame length changes
    // currently, sample rate and frame length will not change
    float *tempOutput = effectOutput_.GetPcmDataBuffer();
    float *directOutput = directOutput_->GetPcmDataBuffer();
    float *collaborativeOutput = collaborativeOutput_->GetPcmDataBuffer();
    for (uint32_t i = 0; i < effectOutput_.GetFrameLen(); ++i) {
        directOutput[DIRECT_CHANNELS * i] = tempOutput[COLLABORATIVE_OUTPUT_CHANNELS * i];
        directOutput[DIRECT_CHANNELS * i + 1] = tempOutput[COLLABORATIVE_OUTPUT_CHANNELS * i + 1];
        collaborativeOutput[COLLABORATIVE_CHANNELS * i] =
            tempOutput[COLLABORATIVE_OUTPUT_CHANNELS * i + COLLABORATIVE_OUTPUT_CHANNEL_1_INDEX];
        collaborativeOutput[COLLABORATIVE_CHANNELS * i + 1] =
            tempOutput[COLLABORATIVE_OUTPUT_CHANNELS * i + COLLABORATIVE_OUTPUT_CHANNEL_2_INDEX];
    }
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioRendererCreate(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("in");
    int32_t ret = CreateAudioEffectChain(nodeInfo);
    if (ret != 0) {
        AUDIO_WARNING_LOG("failed, ret: %{public}d", ret);
    }
    AUDIO_INFO_LOG("out");
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioRendererStart(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("in");
    ModifyAudioEffectChainInfo(nodeInfo, ADD_AUDIO_EFFECT_CHAIN_INFO);
    AUDIO_INFO_LOG("out");
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioRendererStop(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("in");
    ModifyAudioEffectChainInfo(nodeInfo, REMOVE_AUDIO_EFFECT_CHAIN_INFO);
    AUDIO_INFO_LOG("out");
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioRendererRelease(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("in");
    int32_t ret = ReleaseAudioEffectChain(nodeInfo);
    if (ret != 0) {
        AUDIO_WARNING_LOG("failed, ret: %{public}d", ret);
    }
    ModifyAudioEffectChainInfo(nodeInfo, REMOVE_AUDIO_EFFECT_CHAIN_INFO);
    AUDIO_INFO_LOG("out");
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioOffloadRendererCreate(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    CHECK_AND_RETURN_RET_LOG(OffloadRendererCheckNotifyEffect(sinkInfo) == true,
        ERR_INVALID_HANDLE, "no need to create");

    int32_t ret = CreateAudioEffectChain(nodeInfo);
    AUDIO_WARNING_LOG("out, ret: %{public}d", ret);
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioOffloadRendererStart(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    CHECK_AND_RETURN_RET_LOG(OffloadRendererCheckNotifyEffect(sinkInfo) == true,
        ERR_INVALID_HANDLE, "no need to start");

    ModifyAudioEffectChainInfo(nodeInfo, ADD_AUDIO_EFFECT_CHAIN_INFO);
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioOffloadRendererStop(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    CHECK_AND_RETURN_RET_LOG(OffloadRendererCheckNotifyEffect(sinkInfo) == true,
        ERR_INVALID_HANDLE, "no need to stop");

    ModifyAudioEffectChainInfo(nodeInfo, REMOVE_AUDIO_EFFECT_CHAIN_INFO);
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioOffloadRendererRelease(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo)
{
    CHECK_AND_RETURN_RET_LOG(OffloadRendererCheckNotifyEffect(sinkInfo) == true,
        ERR_INVALID_HANDLE, "no need to release");

    int32_t ret = ReleaseAudioEffectChain(nodeInfo);
    AUDIO_WARNING_LOG("out, ret: %{public}d", ret);
    ModifyAudioEffectChainInfo(nodeInfo, REMOVE_AUDIO_EFFECT_CHAIN_INFO);
    return SUCCESS;
}

bool HpaeRenderEffectNode::OffloadRendererCheckNotifyEffect(const HpaeSinkInfo &sinkInfo)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, false, "null audioEffectChainManager");
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager->GetOffloadEnabled() == true, false,
        "no effect offload scene");
    CHECK_AND_RETURN_RET_LOG(sinkInfo.deviceClass != "remote_offload", false, "no need notify effectChainManager");

    return true;
}

int32_t HpaeRenderEffectNode::CreateAudioEffectChain(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("sessionID is %{public}u, sceneType is %{public}d",
        nodeInfo.sessionId, nodeInfo.effectInfo.effectScene);
    // todo: if boot music, do not create audio effect
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneType = "EFFECT_NONE";
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    if (audioSupportedSceneTypes.find(nodeInfo.effectInfo.effectScene) !=
        audioSupportedSceneTypes.end()) {
        sceneType = audioSupportedSceneTypes.at(nodeInfo.effectInfo.effectScene);
    }
    if (!audioEffectChainManager->CheckAndAddSessionID(std::to_string(nodeInfo.sessionId))) {
        return SUCCESS;
    }
    audioEffectChainManager->UpdateSceneTypeList(sceneType, ADD_SCENE_TYPE);
    if (audioEffectChainManager->GetOffloadEnabled()) {
        return SUCCESS;
    }
    int32_t ret = audioEffectChainManager->CreateAudioEffectChainDynamic(sceneType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "create effect chain fail");
    AUDIO_INFO_LOG("Success, sessionID is %{public}u, sceneType is %{public}d",
        nodeInfo.sessionId, nodeInfo.effectInfo.effectScene);
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::ReleaseAudioEffectChain(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("sessionID is %{public}u, sceneType is %{public}d",
        nodeInfo.sessionId, nodeInfo.effectInfo.effectScene);
    // todo: if boot music, do not release audio effect
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneType = "EFFECT_NONE";
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    if (audioSupportedSceneTypes.find(nodeInfo.effectInfo.effectScene) !=
        audioSupportedSceneTypes.end()) {
        sceneType = audioSupportedSceneTypes.at(nodeInfo.effectInfo.effectScene);
    }
    if (!audioEffectChainManager->CheckAndRemoveSessionID(std::to_string(nodeInfo.sessionId))) {
        return SUCCESS;
    }
    audioEffectChainManager->UpdateSceneTypeList(sceneType, REMOVE_SCENE_TYPE);
    if (audioEffectChainManager->GetOffloadEnabled()) {
        return SUCCESS;
    }
    int32_t ret = audioEffectChainManager->ReleaseAudioEffectChainDynamic(sceneType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "release effect chain fail");
    AUDIO_INFO_LOG("Success, sessionID is %{public}u, sceneType is %{public}d",
        nodeInfo.sessionId, nodeInfo.effectInfo.effectScene);
    return SUCCESS;
}

void HpaeRenderEffectNode::ModifyAudioEffectChainInfo(HpaeNodeInfo &nodeInfo,
    ModifyAudioEffectChainInfoReason reason)
{
    std::string sessionID = std::to_string(nodeInfo.sessionId);
    std::string sceneType = "EFFECT_NONE";
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    if (audioSupportedSceneTypes.find(nodeInfo.effectInfo.effectScene) !=
        audioSupportedSceneTypes.end()) {
        sceneType = audioSupportedSceneTypes.at(nodeInfo.effectInfo.effectScene);
    }
    int32_t ret = 0;
    switch (reason) {
        case ADD_AUDIO_EFFECT_CHAIN_INFO: {
            const std::unordered_map<AudioEffectMode, std::string> &audioSupportedSceneModes =
                GetAudioSupportedSceneModes();
            SessionEffectInfo info;
            auto sceneMode = audioSupportedSceneModes.find(nodeInfo.effectInfo.effectMode);
            if (sceneMode != audioSupportedSceneModes.end()) {
                info.sceneMode = sceneMode->second;
            } else {
                AUDIO_WARNING_LOG("sceneMode: %{public}d is not supported", nodeInfo.effectInfo.effectMode);
                info.sceneMode = "EFFECT_NONE";
            }
            info.sceneType = sceneType;
            info.channels = static_cast<uint32_t>(nodeInfo.channels);
            info.channelLayout = nodeInfo.channelLayout;
            info.streamUsage = nodeInfo.effectInfo.streamUsage;
            info.systemVolumeType = static_cast<int32_t>(nodeInfo.effectInfo.systemVolumeType);
            ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, info);
            break;
        }
        case REMOVE_AUDIO_EFFECT_CHAIN_INFO:
            ret = AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(sceneType, sessionID);
            break;
        default:
            break;
    }
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "modify session info failed");
    UpdateAudioEffectChainInfo(nodeInfo);
}

void HpaeRenderEffectNode::UpdateAudioEffectChainInfo(HpaeNodeInfo &nodeInfo)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    std::string sceneType = "EFFECT_NONE";
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    if (audioSupportedSceneTypes.find(nodeInfo.effectInfo.effectScene) !=
        audioSupportedSceneTypes.end()) {
        sceneType = audioSupportedSceneTypes.at(nodeInfo.effectInfo.effectScene);
    }
    audioEffectChainManager->UpdateMultichannelConfig(sceneType);
    audioEffectChainManager->EffectVolumeUpdate();
    audioEffectChainManager->UpdateDefaultAudioEffect();
    audioEffectChainManager->UpdateStreamUsage();
}

void HpaeRenderEffectNode::ReconfigOutputBuffer()
{
    HpaeNodeInfo &effectNodeInfo = GetNodeInfo();
    uint32_t channels = static_cast<uint32_t>(effectNodeInfo.channels);
    uint64_t channelLayout = effectNodeInfo.channelLayout;
    int32_t ret = AudioEffectChainManager::GetInstance()->GetOutputChannelInfo(sceneType_, channels, channelLayout);
    if (ret != SUCCESS || channels == 0 || channelLayout == 0) {
        AUDIO_WARNING_LOG("output channel info incorrect, scene type: %{public}s, "
            "channels: %{public}u, channelLayout: %{public}" PRIu64, sceneType_.c_str(), channels, channelLayout);
    } else if (static_cast<uint32_t>(effectNodeInfo.channels) != channels ||
        static_cast<uint64_t>(effectNodeInfo.channelLayout) != channelLayout) {
        AUDIO_INFO_LOG("output channel info changed, scene type: %{public}s, "
            "channels change from %{public}u to %{public}u, "
            "channelLayout change from %{public}" PRIu64 " to %{public}" PRIu64,
            sceneType_.c_str(), effectNodeInfo.channels, channels, effectNodeInfo.channelLayout, channelLayout);
        PcmBufferInfo pcmBufferInfo = PcmBufferInfo(channels, DEFAULT_EFFECT_FRAMELEN,
            DEFUALT_EFFECT_RATE, channelLayout, effectOutput_.GetFrames());
        pcmBufferInfo.isMultiFrames = effectOutput_.IsMultiFrames();
        effectOutput_.ReConfig(pcmBufferInfo);
        effectNodeInfo.channels = static_cast<AudioChannel>(channels);
        effectNodeInfo.channelLayout = static_cast<AudioChannelLayout>(channelLayout);
        effectNodeInfo.samplingRate = static_cast<AudioSamplingRate>(DEFUALT_EFFECT_RATE);
        effectNodeInfo.frameLen = static_cast<uint32_t>(DEFAULT_EFFECT_FRAMELEN);
        SetNodeInfo(effectNodeInfo);
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfoChanged(GetNodeId(), GetNodeInfo());
        }
#endif
    }
}

int32_t HpaeRenderEffectNode::GetExpectedInputChannelInfo(AudioBasicFormat &basicFormat)
{
    basicFormat.rate = static_cast<AudioSamplingRate>(DEFUALT_EFFECT_RATE);
    uint64_t channelLayout = 0;
    int32_t ret = AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(sceneType_,
        basicFormat.audioChannelInfo.numChannels, channelLayout);
    basicFormat.audioChannelInfo.channelLayout = static_cast<AudioChannelLayout>(channelLayout);
    return ret;
}

bool HpaeRenderEffectNode::IsByPassEffectZeroVolume(HpaePcmBuffer *pcmBuffer)
{
    if (!pcmBuffer->IsValid()) {
        return false;
    }
    if (pcmBuffer->IsSilence()) {
        if (!isDisplayEffectZeroVolume_) {
            AUDIO_INFO_LOG("Timing begins, will close [%{public}s] effect after [%{public}" PRId64 "]s",
                sceneType_.c_str(), WAIT_CLOSE_EFFECT_TIME);
            isDisplayEffectZeroVolume_ = true;
        }
        silenceDataUs_ += static_cast<int64_t>(pcmBuffer->GetFrameLen() * TIME_IN_US / pcmBuffer->GetSampleRate());
        if (!isByPassEffect_ && silenceDataUs_ >= WAIT_CLOSE_EFFECT_TIME * TIME_IN_US) {
            AUDIO_INFO_LOG("Volume change to zero over %{public}" PRId64 "s, close effect:%{public}s success.",
                WAIT_CLOSE_EFFECT_TIME, sceneType_.c_str());
            isByPassEffect_ = true;
            silenceDataUs_ = 0;
        } else if (isByPassEffect_ && silenceDataUs_ >= MONITOR_CLOSE_EFFECT_TIME * TIME_IN_US) {
            silenceDataUs_ = 0;
            AUDIO_INFO_LOG("Effect [%{public}s] have closed [%{public}" PRId64 "]s.",
                sceneType_.c_str(), MONITOR_CLOSE_EFFECT_TIME);
        }
    } else {
        if (isDisplayEffectZeroVolume_) {
            AUDIO_INFO_LOG("Volume change to non zero, open effect:%{public}s success.", sceneType_.c_str());
            isDisplayEffectZeroVolume_ = false;
        }
        silenceDataUs_ = 0;
        isByPassEffect_ = false;
    }
    return isByPassEffect_;
}

void HpaeRenderEffectNode::InitEffectBuffer(const uint32_t sessionId)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    audioEffectChainManager->InitEffectBuffer(std::to_string(sessionId));
}

void HpaeRenderEffectNode::InitEffectBufferFromDisConnect()
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_LOG(audioEffectChainManager != nullptr, "null audioEffectChainManager");
    audioEffectChainManager->InitAudioEffectChainDynamic(sceneType_);
    AUDIO_INFO_LOG("sceneType:%{public}s", sceneType_.c_str());
}

uint64_t HpaeRenderEffectNode::GetLatency(uint32_t sessionId)
{
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, 0, "null audioEffectChainManager");
    return audioEffectChainManager->GetLatency(std::to_string(sessionId)) * AUDIO_US_PER_MS;
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS