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

#ifndef HPAE_RENDER_EFFECT_NODE_H
#define HPAE_RENDER_EFFECT_NODE_H

#include "hpae_plugin_node.h"
#include "audio_info.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif
#include "audio_ring_cache.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

enum ModifyAudioEffectChainInfoReason {
    ADD_AUDIO_EFFECT_CHAIN_INFO = 0,
    REMOVE_AUDIO_EFFECT_CHAIN_INFO = 1,
};

class HpaeTempEffectBuffer {
public:
    HpaeTempEffectBuffer();
    HpaeTempEffectBuffer(PcmBufferInfo pcmBufferInfo,
        CollaborationState isCollaborationEnabled = COLLABORATION_CLOSE);
    ~HpaeTempEffectBuffer();
    void ChangeEffectDataForLatency(HpaePcmBuffer* buffer);
    bool DelayAlignmentForEffectNode(HpaePcmBuffer* buffer);
    void SetDelayCountToEffectNode(int32_t delayCount);
    void ChangeLatencyByCollManagerForEffectNode(int32_t &latency);
    void FillSilenceFramesForEffectNode(int32_t latencyMs);
    void ReadDataDequeueRingCache(HpaePcmBuffer* buffer);
    void WriteDataEnqueueRingCache(HpaePcmBuffer* buffer);
    void UpdateNearLinkCollaborationStateToHdi(CollaborationState isCollaborationEnabled);
    void SetCollabDoFade(bool fadeOutState);
    void SetStopRunningState(bool stopRunning);
private:
    void SetSilenceData(HpaePcmBuffer* buffer);
    void SetSilenceDataInAllChannel(HpaePcmBuffer* buffer);
    void SetTempBufferData(HpaePcmBuffer* buffer, float *tempOutPut);
    void DoFadeOut(HpaePcmBuffer* buffer);
    bool enqueueRunning_ = false;
    std::unique_ptr<AudioRingCache> ringCache_ = nullptr;
    int32_t enqueueCount_ = 1;
    uint64_t latency_  = 0; // in ms
    int32_t waitCountThreshold_ = 0;
    std::unique_ptr<HpaePcmBuffer> tempOutput_ = nullptr;
    CollaborationState isCollaborationEnabled_ = COLLABORATION_CLOSE;
    bool effectCollabDoFadeOutState_ = false;
    bool stopRunning_ = false;
};

class HpaeRenderEffectNode : public HpaePluginNode, public HpaeTempEffectBuffer {
public:
    HpaeRenderEffectNode(HpaeNodeInfo &nodeInfo, CollaborationRouteMode collaborationRouteMode_ =
        COLLABORATION_INVALID_ROUTE_MODE);
    virtual ~HpaeRenderEffectNode();
    void DoProcess() override;
    int32_t AudioRendererCreate(HpaeNodeInfo &nodeInfo);
    int32_t AudioRendererStart(HpaeNodeInfo &nodeInfo);
    int32_t AudioRendererStop(HpaeNodeInfo &nodeInfo);
    int32_t AudioRendererRelease(HpaeNodeInfo &nodeInfo);
    int32_t GetExpectedInputChannelInfo(AudioBasicFormat &basicFormat);
    void InitEffectBuffer(const uint32_t sessionId);
    void InitEffectBufferFromDisConnect();
    uint64_t GetLatency(uint32_t sessionId = 0) override;
    int32_t AudioOffloadRendererCreate(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioOffloadRendererStart(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioOffloadRendererStop(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioOffloadRendererRelease(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    void SetDelayCountToEffectNode(int32_t delayCount);
    void SetCollabDoFade(bool fadeOutState);
    void SetStopRunningState(bool stopRunning);
protected:
    HpaePcmBuffer* SignalProcess(const std::vector<HpaePcmBuffer*> &inputs) override;
private:
    void ReconfigOutputBuffer(uint32_t outChannels, uint64_t outChannelLayout);
    int32_t CreateAudioEffectChain(HpaeNodeInfo &nodeInfo, bool isPcmOffload = false);
    int32_t ReleaseAudioEffectChain(HpaeNodeInfo &nodeInfo, bool isPcmOffload = false);
    void ModifyAudioEffectChainInfo(HpaeNodeInfo &nodeInfo, ModifyAudioEffectChainInfoReason reason);
    void UpdateAudioEffectChainInfo(HpaeNodeInfo &nodeInfo);
    bool IsByPassEffectZeroVolume(HpaePcmBuffer *pcmBuffer);
    int32_t SplitCollaborativeData();
    bool OffloadRendererCheckNotifyEffect(const HpaeSinkInfo &sinkInfo, bool isPcmOffload);
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer effectOutput_;
    std::unique_ptr<HpaePcmBuffer> directOutput_ = nullptr;
    std::unique_ptr<HpaePcmBuffer> collaborativeOutput_ = nullptr;
    std::string sceneType_ = "EFFECT_NONE";
    DeviceType deviceType_ = DEVICE_TYPE_INVALID;
    uint32_t renderId_ = 0;
    int64_t silenceDataUs_ = 0;
    bool isByPassEffect_ = false;
    bool isDisplayEffectZeroVolume_ = false;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> directPcmDumper_;
    std::unique_ptr<HpaePcmDumper> collaborativePcmDumper_;
#endif
    std::shared_ptr<HpaeTempEffectBuffer> tempEffectBuffer_ = nullptr;
    CollaborationRouteMode collaborationRouteMode_ = COLLABORATION_INVALID_ROUTE_MODE;
};

} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS

#endif // HPAE_RENDER_EFFECT_NODE_H