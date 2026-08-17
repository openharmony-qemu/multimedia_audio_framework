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
#ifndef HPAE_MIC_IN_FRONT_EXTRACT_NODE_H
#define HPAE_MIC_IN_FRONT_EXTRACT_NODE_H
#include "hpae_plugin_node.h"
#include "hpae_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
// Scenario B node: for a camcorder split-layout (non-interleaved [main block][micIn block])
// source, copy only the leading main-channel block to the output and drop the micIn tail.
// Pure width semantics: input width = preNodeInfo.channels (partitioned total),
// output width = nodeInfo.channels (main). The owning cluster computes the widths;
// this node never reads the micIn info fields.
// Invariant: upstream rate/frameLen equal nodeInfo rate/frameLen (the cluster builds them
// equal); a frameLen mismatch degrades to silence.
// NOTE: EnableProcess(false) passthrough would leak the micIn tail; capture-path nodes
// are never disabled today.
class HpaeMicInFrontExtractNode : public HpaePluginNode {
public:
    HpaeMicInFrontExtractNode(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo);
    ~HpaeMicInFrontExtractNode() override;

    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    void SetSourceBufferType(HpaeSourceBufferType type);
    uint64_t GetLatency(uint32_t sessionId = 0) override;
protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;
private:
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer extractOutput_;
    // upstream channel count seen last cycle (seeded from preNodeInfo.channels at construction);
    // the extract width is frozen at construction, so a mid-stream upstream reconfig is silently
    // wrong -- keep it observable (warning only).
    uint32_t lastUpstreamChannels_ = 0;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_MIC_IN_FRONT_EXTRACT_NODE_H
