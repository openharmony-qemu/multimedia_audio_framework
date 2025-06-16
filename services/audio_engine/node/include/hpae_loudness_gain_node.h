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
#ifdef HPAE_LOUDNESS_gAIN_NODE
#define HPAE_LOUDNESS_gAIN_NODE
#include <memory>
#include <unordered_map>
#include "hpae_node.h"
#include "hpae_plugin_node.h"
#include "audio_limiter.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#include "audio_effect.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace HPAE {


class HpaeLoudnessGainNode : public HpaePluginNode {
public:
    HpaeLoudnessNode(HpaeNodeInfo &nodeInfo);
    ~HpaeLoudnessNode();
    bool SetLoudnessGain(float loudnessGain);
    float GetLoudnessGain();
    bool IsLoudnessAlgoOn();

protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;
private:
    bool CheckUpdateInfo(HpaePcmBuffer *input);
    AudioEffectLibrary* audioEffectLibHandle_ = nullptr;
    AudioEffectHandle handle_ = nullptr;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer loudnessGainOutput_;
    float loudnessGain_ = 0.0f;
    void* dlHandle_ = nullptr;

#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> inputPcmDumper_ = nullptr;
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
#endif
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif