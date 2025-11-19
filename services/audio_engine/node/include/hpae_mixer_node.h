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
#ifndef HPAE_MIXER_NODE_H
#define HPAE_MIXER_NODE_H
#include <memory>
#include <unordered_map>
#include "hpae_node.h"
#include "hpae_plugin_node.h"
#include "audio_limiter.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeMixerNode : public HpaePluginNode {
public:
    HpaeMixerNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeMixerNode();
    virtual bool Reset() override;
    int32_t SetupAudioLimiter();
    int32_t InitAudioLimiter();
    virtual void SetNodeInfo(HpaeNodeInfo& nodeInfo) override;
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode, HpaeNodeInfo &nodeInfo) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    uint64_t GetLatency(uint32_t sessionId = 0) override;
protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;
private:
    bool CheckUpdateInfo(HpaePcmBuffer *input);
    bool CheckUpdateInfoForDisConnect();
    void DrainProcess();
    std::unordered_map<uint32_t, float> streamVolumeMap_;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer mixedOutput_;
    HpaePcmBuffer tmpOutput_;
    std::unique_ptr<AudioLimiter> limiter_ = nullptr;
    uint32_t waitFrames_ = 0;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif