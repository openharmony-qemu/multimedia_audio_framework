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
#ifndef HPAE_AUDIO_FORMAT_CONVERTER_CLUSTER_H
#define HPAE_AUDIO_FORMAT_CONVERTER_CLUSTER_H
#include "hpae_audio_format_converter_node.h"
#include "hpae_mic_in_front_extract_node.h"
#include "hpae_mic_in_zero_pad_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
// Drop-in replacement for HpaeAudioFormatConverterNode at creation sites whose nodeInfo may
// carry camcorder micIn fields (sourceType/micInChannels/appMicInChannelCount). Composes the
// chain per scenario (zero extra copies when no mode engages):
//   Scenario B (HAL split, app wants main only): [MicInFrontExtract] -> [Converter]
//   Scenario C (app wants micIn, HAL has none):  [Converter] -> [MicInZeroPad]
//   Scenario A/D: single [Converter] (head == tail)
// In the composed chains (B/C) the inner converter receives sanitized info (channels = main,
// micIn fields cleared) so the vanilla node never re-engages mode detection.
// NOTE: any NEW creation site that can see a CAMCORDER nodeInfo must use this cluster,
// not the bare converter node.
class HpaeAudioFormatConverterCluster : public OutputNode<HpaePcmBuffer *>,
    public InputNode<HpaePcmBuffer *> {
public:
    HpaeAudioFormatConverterCluster(HpaeNodeInfo preNodeInfo, HpaeNodeInfo nodeInfo);
    ~HpaeAudioFormatConverterCluster() override;

    void DoProcess() override;
    bool Reset() override;
    bool ResetAll() override;
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    std::shared_ptr<HpaeNode> GetSharedInstance(HpaeNodeInfo &nodeInfo) override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort() override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort(HpaeNodeInfo &nodeInfo, bool isDisConnect = false) override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode,
        HpaeNodeInfo &nodeInfo) override;

    void SetSourceNode(bool isSourceNode);
    void SetSourceBufferType(HpaeSourceBufferType type);
    size_t GetOutputPortNum();
    uint64_t GetLatency(uint32_t sessionId = 0);

private:
    // --- Refactoring helpers (sub-50-line extraction targets) ---
    void BuildFrontExtractChain(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo, uint32_t preChannels);
    void BuildZeroPadChain(HpaeNodeInfo &preNodeInfo, HpaeNodeInfo &nodeInfo, uint32_t totalChannels);

    std::shared_ptr<HpaeMicInFrontExtractNode> extractNode_ = nullptr;
    std::shared_ptr<HpaeAudioFormatConverterNode> converterNode_ = nullptr;
    std::shared_ptr<HpaeMicInZeroPadNode> zeroPadNode_ = nullptr;
    std::shared_ptr<HpaePluginNode> head_ = nullptr;
    std::shared_ptr<HpaePluginNode> tail_ = nullptr;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_AUDIO_FORMAT_CONVERTER_CLUSTER_H
