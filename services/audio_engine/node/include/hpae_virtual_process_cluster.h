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

#ifndef HPAE_VIRTUAL_PROCESS_CLUSTER_H
#define HPAE_VIRTUAL_PROCESS_CLUSTER_H
#include "hpae_mixer_node.h"
#include "hpae_audio_format_converter_node.h"
#include "hpae_gain_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeVirtualProcessCluster : public OutputNode<HpaePcmBuffer*>, public InputNode<HpaePcmBuffer*> {
public:
    HpaeVirtualProcessCluster(HpaeNodeInfo nodeInfo);
    virtual ~HpaeVirtualProcessCluster();
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    int32_t SetupAudioLimiter();

    size_t GetConnectSinkInputNum();
    std::shared_ptr<HpaeGainNode> GetGainNodeById(const uint32_t &sessionId);
private:
    void CreateGainNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo);
    void CreateConverterNode(uint32_t sessionId, const HpaeNodeInfo &preNodeInfo);

    std::shared_ptr<HpaeMixerNode> mixerNode_ = nullptr;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeAudioFormatConverterNode>> idConverterMap_;
    std::unordered_map<uint32_t, std::shared_ptr<HpaeGainNode>> idGainMap_;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_VIRTUAL_PROCESS_CLUSTER_H
