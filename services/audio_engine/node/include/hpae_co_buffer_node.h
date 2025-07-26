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

#ifndef HPAE_COBUFFER_NODE_H
#define HPAE_COBUFFER_NODE_H
#include <memory>
#include <mutex>
#include "audio_ring_cache.h"
#include "high_resolution_timer.h"
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"

#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

enum class FrameFlag {
    FIRST_FRAME,
    SECOND_FRAME,
    OTHER_FRAME,
};
class HpaeCoBufferNode : public OutputNode<HpaePcmBuffer *>, public InputNode<HpaePcmBuffer *> {
public:
    HpaeCoBufferNode();
    virtual ~HpaeCoBufferNode();
    void DoProcess() override;
    bool Reset() override;
    bool ResetAll() override;
    
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer*>* GetOutputPort() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    void Enqueue(HpaePcmBuffer* buffer) override;
    void SetLatency(uint32_t latency);
    void SetOutputClusterConnected(bool isConnect);
    bool IsOutputClusterConnected();
private:
    void FillSilenceFramesInner(uint32_t latencyMs);
    void ProcessInputFrameInner(HpaePcmBuffer* buffer);
    void ProcessOutputFrameInner();
    std::mutex mutex_;
    bool enqueueRunning_ = false;
    InputPort<HpaePcmBuffer *> inputStream_;
    OutputPort<HpaePcmBuffer *> outputStream_;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer coBufferOut_;
    HpaePcmBuffer silenceData_;
    std::unique_ptr<AudioRingCache> ringCache_ = nullptr;
    int32_t enqueueCount_ = 1;
    uint64_t latency_  = 0; // in ms
    bool isOutputClusterConnected_ = false;
    std::set<HpaeProcessorType> connectedProcessCluster_;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> inputPcmDumper_;
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_;
#endif
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif