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
#ifndef HPAE_MIC_IN_ZERO_PAD_NODE_H
#define HPAE_MIC_IN_ZERO_PAD_NODE_H
#include "hpae_plugin_node.h"
#include "hpae_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
// Scenario C node: the app wants micIn slots but the HAL has none, so the trailing micIn
// block of the partitioned output is zero-filled. Input is main-only interleaved data;
// output is NON-interleaved [main block][zero micIn block] at the app total width.
// Pure width semantics: main = preNodeInfo.channels, total = nodeInfo.channels
// (the owning cluster computes the widths; this node never reads the micIn info fields).
// channelLayout keeps the app's total-channel layout: it is metadata only (DFX/dump);
// downstream consumers of a camcorder micIn stream already interpret the buffer as
// partitioned per the HAL passthrough convention.
// Invariant: upstream rate/frameLen equal nodeInfo rate/frameLen (the cluster builds them
// equal); a frameLen mismatch degrades to silence.
// NOTE: EnableProcess(false) passthrough would emit a main-width buffer where downstream
// expects the total-width partition; capture-path nodes are never disabled today.
class HpaeMicInZeroPadNode : public HpaePluginNode {
public:
    HpaeMicInZeroPadNode(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo);
    ~HpaeMicInZeroPadNode() override;

    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    void SetSourceBufferType(HpaeSourceBufferType type);
    uint64_t GetLatency(uint32_t sessionId = 0) override;
protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;
private:
    // --- Refactoring helpers (sub-50-line extraction targets) ---
    void ReportInvalidParamEvent(const std::string &reason);
    bool CheckUpstreamFormat(HpaePcmBuffer *input);
    bool WritePartitionedOutput(HpaePcmBuffer *input, uint32_t mainCh, uint32_t totalCh);

    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer zeroPadOutput_;
    // upstream channel count seen last cycle (seeded from preNodeInfo.channels at construction);
    // the total width is frozen at construction, so a mid-stream upstream reconfig is silently
    // wrong -- keep it observable (warning only).
    uint32_t lastUpstreamChannels_ = 0;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_MIC_IN_ZERO_PAD_NODE_H
