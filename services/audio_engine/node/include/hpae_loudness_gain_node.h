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
#ifndef HPAE_LOUDNESS_GAIN_NODE
#define HPAE_LOUDNESS_GAIN_NODE
#include <memory>
#include <unordered_map>
#include "hpae_node.h"
#include "hpae_plugin_node.h"
#include "audio_effect.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace HPAE {


class HpaeLoudnessGainNode : public HpaePluginNode {
public:
    HpaeLoudnessGainNode(HpaeNodeInfo &nodeInfo);
    ~HpaeLoudnessGainNode();
    int32_t SetLoudnessGain(float loudnessGain);
    float GetLoudnessGain();
    bool IsLoudnessAlgoOn();
    uint64_t GetLatency(uint32_t sessionId = 0) override;

protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;
private:
    void CheckUpdateInfo(HpaePcmBuffer *input);
    int32_t ReleaseHandle(float loudnessGain);
    AudioEffectLibrary *audioEffectLibHandle_ = nullptr;
    AudioEffectHandle handle_ = nullptr;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer loudnessGainOutput_;
    float loudnessGain_ = 0.0f;
    float linearGain_ = 1.0f;
    void *dlHandle_ = nullptr;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif