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
#ifndef HPAE_RESAMPLE_NODE_H
#define HPAE_RESAMPLE_NODE_H
#include "audio_proresampler.h"
#include "hpae_plugin_node.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

enum class ResamplerType {
    PRORESAMPLER
};

class HpaeResampleNode : public HpaePluginNode {
public:
    HpaeResampleNode(HpaeNodeInfo& nodeInfo, HpaeNodeInfo& preNodeInfo, ResamplerType type);
    HpaeResampleNode(HpaeNodeInfo& nodeInfo, HpaeNodeInfo& preNodeInfo);
    ~HpaeResampleNode() = default;
    virtual bool Reset() override;
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode, HpaeNodeInfo &nodeInfo) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
protected:
    HpaePcmBuffer* SignalProcess(const std::vector<HpaePcmBuffer*>& inputs) override;
private:
    void ResampleProcess(float *srcData, uint32_t inputFrameLen, float *dstData, uint32_t outputFrameLen);
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer resampleOutput_;
    HpaeNodeInfo preNodeInfo_;
    std::vector<float> tempOutput_;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> inputPcmDumper_ = nullptr;
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
#endif
    std::unique_ptr<Resampler> resampler_ = nullptr;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif