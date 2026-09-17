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
class StreamVolume;
class PipeVolume;
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
    void ResetVolume();
    void ResetFadeState();
    void ClearFadeState();
    bool HasPendingFade() const;
    uint64_t GetLatency(uint32_t sessionId = 0) override;
    void SetCollFadeState(IOperation operation);
    void DoCollFading(HpaePcmBuffer *input);
protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;
private:
    bool isInnerCapturerOrInjector_ = false;
    float preGain_ = 1.0f;
    float curGain_ = 1.0f;
    bool isGainChanged_ = false;
    bool needGainState_ = true;
    bool fadeInState_ = false;
    uint32_t fadeProcessedBytes_ = 0;
    uint32_t totalFadeBytes_ = 0;
    FadeOutState fadeOutState_ = FadeOutState::NO_FADEOUT;
    IOperation operation_ = OPERATION_INVALID;
    void DoGain(HpaePcmBuffer *input, uint32_t frameLen, uint32_t channelCount);
    void EnsureVolumeAvailable();
    uint32_t CalcRemainDurationMs(uint32_t duration, uint32_t frameLen, float *curSysGain, float *preSysGain);
    void DoFading(HpaePcmBuffer *input);
    void ApplyFadeOut(HpaePcmBuffer *input, FadeOutState &fadeOutState, bool isCollFade);
    void ApplyFadeIn(HpaePcmBuffer *input, bool &fadeInState,
        uint32_t &processedBytes, uint32_t &totalFadeBytes);
    void ProcessDefaultFadeIn(HpaePcmBuffer *input, uint8_t *data,
        bool &fadeInState, uint32_t &processedBytes, uint32_t &totalFadeBytes);
    void SilenceData(HpaePcmBuffer *pcmBuffer);
    bool IsSilentData(HpaePcmBuffer *pcmBuffer);
    uint32_t GetFadeLength(uint32_t &byteLength, HpaePcmBuffer *input);
    uint32_t GetFadeInLength(uint32_t &byteLength, HpaePcmBuffer *input);
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
    std::unique_ptr<HpaePcmDumper> inputPcmDumper_ = nullptr;
#endif
    bool collFadeInState_ = false;
    uint32_t collFadeProcessedBytes_ = 0;
    uint32_t collTotalFadeBytes_ = 0;
    FadeOutState collFadeOutState_ = FadeOutState::NO_FADEOUT;
    std::shared_ptr<StreamVolume> streamVolume_;
    std::shared_ptr<PipeVolume> pipeVolume_;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif