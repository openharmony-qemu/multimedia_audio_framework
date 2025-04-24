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
#include "audio_engine_log.h"
#include "hpae_render_effect_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect_map.h"
#include "audio_utils.h"

static constexpr uint32_t DEFUALT_EFFECT_RATE = 48000;
static constexpr uint32_t DEFAULT_EFFECT_FRAMELEN = 960;

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

HpaeRenderEffectNode::HpaeRenderEffectNode(HpaeNodeInfo &nodeInfo) : HpaeNode(nodeInfo), HpaePluginNode(nodeInfo),
    // DEFAUT effect out format
    pcmBufferInfo_(nodeInfo.channels, DEFAULT_EFFECT_FRAMELEN, DEFUALT_EFFECT_RATE, nodeInfo.channelLayout),
    effectOutput_(pcmBufferInfo_),
    nodeInfo_(nodeInfo)
{
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    if (audioSupportedSceneTypes.find(nodeInfo.effectInfo.effectScene) !=
        audioSupportedSceneTypes.end()) {
        sceneType_ = audioSupportedSceneTypes.at(nodeInfo.effectInfo.effectScene);
    }
    AUDIO_INFO_LOG("render effect node created, scene type: %{public}s", sceneType_.c_str());
#ifdef ENABLE_HOOK_PCM
    inputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeRenderEffectNodeInput_id_" + std::to_string(GetNodeId()) + ".pcm");
    outputPcmDumper_ = std::make_unique<HpaePcmDumper>(
        "HpaeRenderEffectNodeOutput_id_" + std::to_string(GetNodeId()) + ".pcm");
#endif
}

HpaePcmBuffer *HpaeRenderEffectNode::SignalProcess(const std::vector<HpaePcmBuffer *> &inputs)
{
    AUDIO_DEBUG_LOG("render effect node signal process in");
    if (inputs.empty()) {
        AUDIO_WARNING_LOG("HpaeRenderEffectNode inputs size is empty");
        return nullptr;
    }
    auto rate = "rate[" + std::to_string(inputs[0]->GetSampleRate()) + "]_";
    auto ch = "ch[" + std::to_string(inputs[0]->GetChannelCount()) + "]_";
    auto len = "len[" + std::to_string(inputs[0]->GetFrameLen()) + "]";
    Trace trace("[" + sceneType_ + "]HpaeRenderEffectNode::SignalProcess " + rate + ch + len);

    if (AudioEffectChainManager::GetInstance()->GetOffloadEnabled()) {
        return inputs[0];
    }

#ifdef ENABLE_HOOK_PCM
    if (inputPcmDumper_) {
        inputPcmDumper_->Dump((int8_t *)inputs[0]->GetPcmDataBuffer(),
            inputs[0]->GetFrameLen() * sizeof(float) * inputs[0]->GetChannelCount());
    }
#endif

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

#ifdef ENABLE_HOOK_PCM
    if (outputPcmDumper_) {
        outputPcmDumper_->Dump((int8_t *)effectOutput_.GetPcmDataBuffer(),
            effectOutput_.GetFrameLen() * sizeof(float) * effectOutput_.GetChannelCount());
    }
#endif

    return &effectOutput_;
}

int32_t HpaeRenderEffectNode::AudioRendererCreate(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("notify audio effect when audio renderer create in");
    int32_t ret = CreateAudioEffectChain(nodeInfo);
    if (ret != 0) {
        AUDIO_WARNING_LOG("create audio effect chain failed, ret: %{public}d", ret);
    }
    AUDIO_INFO_LOG("notify audio effect when audio renderer create out");
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioRendererStart(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("notify audio effect when audio renderer start in");
    ModifyAudioEffectChainInfo(nodeInfo, ADD_AUDIO_EFFECT_CHAIN_INFO);
    AUDIO_INFO_LOG("notify audio effect when audio renderer start out");
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioRendererStop(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("notify audio effect when audio renderer stop in");
    ModifyAudioEffectChainInfo(nodeInfo, REMOVE_AUDIO_EFFECT_CHAIN_INFO);
    AUDIO_INFO_LOG("notify audio effect when audio renderer stop out");
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::AudioRendererRelease(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("notify audio effect when audio renderer release in");
    int32_t ret = ReleaseAudioEffectChain(nodeInfo);
    if (ret != 0) {
        AUDIO_WARNING_LOG("release audio effect chain failed, ret: %{public}d", ret);
    }
    ModifyAudioEffectChainInfo(nodeInfo, REMOVE_AUDIO_EFFECT_CHAIN_INFO);
    AUDIO_INFO_LOG("notify audio effect when audio renderer release out");
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::CreateAudioEffectChain(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("Create Audio Effect Chain In, sessionID is %{public}u, sceneType is %{public}d",
        nodeInfo.sessionId, nodeInfo.effectInfo.effectScene);
    // todo: deal with remote case
    // todo: if boot music, do not create audio effect
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneType = "EFFECT_NONE";
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    if (audioSupportedSceneTypes.find(nodeInfo.effectInfo.effectScene) !=
        audioSupportedSceneTypes.end()) {
        sceneType = audioSupportedSceneTypes.at(nodeInfo.effectInfo.effectScene);
    }
    // todo: could be removed
    if (!audioEffectChainManager->CheckAndAddSessionID(std::to_string(nodeInfo.sessionId))) {
        return SUCCESS;
    }
    audioEffectChainManager->UpdateSceneTypeList(sceneType, ADD_SCENE_TYPE);
    // todo: should be considered
    if (audioEffectChainManager->GetOffloadEnabled()) {
        return SUCCESS;
    }
    int32_t ret = audioEffectChainManager->CreateAudioEffectChainDynamic(sceneType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "create effect chain fail");
    AUDIO_INFO_LOG("Create Audio Effect Chain Success, sessionID is %{public}u, sceneType is %{public}d",
        nodeInfo.sessionId, nodeInfo.effectInfo.effectScene);
    return SUCCESS;
}

int32_t HpaeRenderEffectNode::ReleaseAudioEffectChain(HpaeNodeInfo &nodeInfo)
{
    AUDIO_INFO_LOG("Release Audio Effect Chain In, sessionID is %{public}u, sceneType is %{public}d",
        nodeInfo.sessionId, nodeInfo.effectInfo.effectScene);
    // todo: deal with remote case
    // todo: if boot music, do not release audio effect
    AudioEffectChainManager *audioEffectChainManager = AudioEffectChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEffectChainManager != nullptr, ERR_INVALID_HANDLE, "null audioEffectChainManager");
    std::string sceneType = "EFFECT_NONE";
    const std::unordered_map<AudioEffectScene, std::string> &audioSupportedSceneTypes = GetSupportedSceneType();
    if (audioSupportedSceneTypes.find(nodeInfo.effectInfo.effectScene) !=
        audioSupportedSceneTypes.end()) {
        sceneType = audioSupportedSceneTypes.at(nodeInfo.effectInfo.effectScene);
    }
    // todo: could be removed
    if (!audioEffectChainManager->CheckAndRemoveSessionID(std::to_string(nodeInfo.sessionId))) {
        return SUCCESS;
    }
    audioEffectChainManager->UpdateSceneTypeList(sceneType, REMOVE_SCENE_TYPE);
    // todo: should be considered
    if (audioEffectChainManager->GetOffloadEnabled()) {
        return SUCCESS;
    }
    int32_t ret = audioEffectChainManager->ReleaseAudioEffectChainDynamic(sceneType);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "release effect chain fail");
    AUDIO_INFO_LOG("Release Audio Effect Chain Success, sessionID is %{public}u, sceneType is %{public}d",
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
            SessionEffectInfo info;
            info.sceneMode = std::to_string(nodeInfo.effectInfo.effectMode);
            info.sceneType = sceneType;
            info.channels = static_cast<uint32_t>(nodeInfo.channels);
            info.channelLayout = nodeInfo.channelLayout;
            info.streamUsage = nodeInfo.effectInfo.streamUsage;
            info.systemVolumeType = nodeInfo.effectInfo.volumeType;
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
    uint32_t channels = static_cast<uint32_t>(nodeInfo_.channels);
    uint64_t channelLayout = nodeInfo_.channelLayout;
    int32_t ret = AudioEffectChainManager::GetInstance()->GetOutputChannelInfo(sceneType_, channels, channelLayout);
    if (ret != SUCCESS || channels == 0 || channelLayout == 0) {
        AUDIO_WARNING_LOG("output channel info incorrect, scene type: %{public}s, "
            "channels: %{public}u, channelLayout: %{public}" PRIu64, sceneType_.c_str(), channels, channelLayout);
    } else if (static_cast<uint32_t>(nodeInfo_.channels) != channels ||
        static_cast<uint64_t>(nodeInfo_.channelLayout) != channelLayout) {
        AUDIO_INFO_LOG("output channel info changed, scene type: %{public}s, "
            "channels change from %{public}u to %{public}u, "
            "channelLayout change from %{public}" PRIu64 " to %{public}" PRIu64,
            sceneType_.c_str(), nodeInfo_.channels, channels, nodeInfo_.channelLayout, channelLayout);
        nodeInfo_.channels = static_cast<AudioChannel>(channels);
        nodeInfo_.channelLayout = static_cast<AudioChannelLayout>(channelLayout);
        PcmBufferInfo pcmBufferInfo = PcmBufferInfo(channels, DEFAULT_EFFECT_FRAMELEN,
            DEFUALT_EFFECT_RATE, channelLayout, effectOutput_.GetFrames(), effectOutput_.IsMultiFrames());
        effectOutput_.ReConfig(pcmBufferInfo);
        nodeInfo_.channels = (AudioChannel)channels;
        nodeInfo_.channelLayout = (AudioChannelLayout)channelLayout;
        nodeInfo_.samplingRate = (AudioSamplingRate)DEFUALT_EFFECT_RATE;
        nodeInfo_.frameLen = (uint32_t)DEFAULT_EFFECT_FRAMELEN;
#ifdef ENABLE_HIDUMP_DFX
        if (auto callBack = GetNodeStatusCallback().lock()) {
            callBack->OnNotifyDfxNodeInfoChanged(GetNodeId(), nodeInfo_);
        }
#endif
    }
}

int32_t HpaeRenderEffectNode::GetExpectedInputChannelInfo(uint32_t &channels, uint64_t &channelLayout)
{
    return AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfo(sceneType_, channels, channelLayout);
}

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS