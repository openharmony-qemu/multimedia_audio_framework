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
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeMixerNode : public HpaePluginNode {
public:
    HpaeMixerNode(HpaeNodeInfo &nodeInfo);
    virtual bool Reset() override;
    int32_t SetupAudioLimiter();
protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;
private:
    bool CheckUpdateInfo(HpaePcmBuffer *input);
    std::unordered_map<uint32_t, float> streamVolumeMap_;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer mixedOutput_;
    HpaePcmBuffer tmpOutput_;
    std::unique_ptr<AudioLimiter> limiter_ = nullptr;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;;
#endif
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif