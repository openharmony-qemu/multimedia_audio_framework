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

#ifndef HPAE_PROCESS_CLUSTER_H
#define HPAE_PROCESS_CLUSTER_H
#include "hpae_mixer_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_gain_node.h"
#include "hpae_render_effect_node.h"
#include "hpae_loudness_gain_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeProcessCluster : public OutputNode<HpaePcmBuffer*>, public InputNode<HpaePcmBuffer*>,
    public INodeFormatInfoCallback {
public:
    HpaeProcessCluster(HpaeNodeInfo nodeInfo, HpaeSinkInfo &sinkInfo);
    virtual ~HpaeProcessCluster();
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    int32_t GetGainNodeCount();
    int32_t GetConverterNodeCount();
    int32_t GetLoudnessGainNodeCount();
    int32_t GetPreOutNum();
    int32_t AudioRendererCreate(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioRendererStart(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioRendererStop(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t AudioRendererRelease(HpaeNodeInfo &nodeInfo, const HpaeSinkInfo &sinkInfo);
    int32_t GetNodeInputFormatInfo(uint32_t sessionId, AudioBasicFormat &basicFormat) override;
    std::shared_ptr<HpaeGainNode> GetGainNodeById(uint32_t id) const;
    std::shared_ptr<HpaeAudioFormatConverterNode> GetConverterNodeById(uint32_t id) const;
    void SetConnectedFlag(bool flag);
    bool GetConnectedFlag() const;
    int32_t SetupAudioLimiter();
    int32_t SetLoudnessGain(uint32_t sessionId, float loudnessGain);
    void DisConnectMixerNode();
    void InitEffectBuffer(const uint32_t sessionId);
    uint64_t GetLatency(uint32_t sessionId);
    int32_t CreateNodes(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode);
    int32_t DestroyNodes(uint32_t sessionId);
    int32_t CheckNodes(uint32_t sessionId);
private:
    void ConnectMixerNode();
    void CreateGainNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo);
    void CreateConverterNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo);
    void CreateLoudnessGainNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo);
    bool CheckNeedNotifyEffectNode(HpaeSinkInfo sinkInfo);
    std::shared_ptr<HpaeMixerNode> mixerNode_;
    std::shared_ptr<HpaeRenderEffectNode> renderEffectNode_ = nullptr;
    std::shared_ptr<HpaeRenderEffectNode> renderNoneEffectNode_ = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeAudioFormatConverterNode>> idConverterMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeGainNode>> idGainMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeLoudnessGainNode>> idLoudnessGainNodeMap_;
    HpaeSinkInfo sinkInfo_;
    bool isConnectedToOutputCluster = false;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_PROCESS_CLUSTER_H