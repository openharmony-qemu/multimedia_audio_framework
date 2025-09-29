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

#ifndef HPAE_GAIN_NODE_H
#define HPAE_GAIN_NODE_H
#include "hpae_plugin_node.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#include "i_stream.h"
#endif
namespace OHOS {
namespace AudioStandard {
namespace HPAE {

enum class FadeOutState {
    NO_FADEOUT,
    DO_FADEOUT,
    DONE_FADEOUT
};
class HpaeGainNode : public HpaePluginNode {
public:
    HpaeGainNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeGainNode();
    bool SetClientVolume(float gain);
    float GetClientVolume();
    void SetFadeState(IOperation operation);
    uint64_t GetLatency(uint32_t sessionId = 0) override;
protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;
private:
    bool isInnerCapturerOrInjector_ = false;
    float preGain_ = 1.0f;
    float curGain_ = 1.0f;
    bool isGainChanged_ = false;
    bool needGainState_ = true;
    bool fadeInState_ = false;
    FadeOutState fadeOutState_ = FadeOutState::NO_FADEOUT;
    IOperation operation_ = OPERATION_INVALID;
    void DoGain(HpaePcmBuffer *input, uint32_t frameLen, uint32_t channelCount);
    void DoFading(HpaePcmBuffer *input);
    void SilenceData(HpaePcmBuffer *pcmBuffer);
    bool IsSilentData(HpaePcmBuffer *pcmBuffer);
    uint32_t GetFadeLength(uint32_t &byteLength, HpaePcmBuffer *input);
    uint32_t GetFadeInLength(uint32_t &byteLength, HpaePcmBuffer *input);
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
#endif
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif