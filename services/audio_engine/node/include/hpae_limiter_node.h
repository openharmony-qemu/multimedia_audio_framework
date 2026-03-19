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
#ifndef HPAE_LIMITER_NODE_H
#define HPAE_LIMITER_NODE_H
#include <memory>
#include "hpae_node.h"
#include "hpae_plugin_node.h"
#include "audio_limiter.h"
#include "hpae_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

class HpaeLimiterNode : public HpaePluginNode {
public:
    HpaeLimiterNode(HpaeNodeInfo &nodeInfo);
    ~HpaeLimiterNode() override;

    // Setup and initialize the audio limiter
    int32_t SetupAudioLimiter();
    int32_t InitAudioLimiter();

    // Check if limiter is initialized
    bool IsLimiterInited() const { return limiter_ != nullptr; }

    uint64_t GetLatency(uint32_t sessionId = 0) override;

protected:
    HpaePcmBuffer *SignalProcess(const std::vector<HpaePcmBuffer *> &inputs) override;

private:
    std::unique_ptr<AudioLimiter> limiter_ = nullptr;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer limiterOutput_;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_LIMITER_NODE_H
