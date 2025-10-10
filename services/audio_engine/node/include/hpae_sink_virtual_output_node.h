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

#ifndef HPAE_SINK_VIRTUAL_INPUT_NODE_H
#define HPAE_SINK_VIRTUAL_INPUT_NODE_H
#include <memory>
#include "audio_info.h"
#include "audio_stream_info.h"
#include "audio_ring_cache.h"
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeSinkVirtualOutputNode : public OutputNode<HpaePcmBuffer *>,  public InputNode<HpaePcmBuffer *> {
public:
    HpaeSinkVirtualOutputNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeSinkVirtualOutputNode();
    void DoRenderProcess();
    void DoProcess() override;
    bool Reset() override;
    bool ResetAll() override;

    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;

    StreamManagerState GetState();
    int32_t RenderSinkInit();
    int32_t RenderSinkDeInit();
    int32_t RenderSinkPause(void);
    int32_t RenderSinkStart(void);
    int32_t RenderSinkStop(void);
    size_t GetPreOutNum();
    int32_t SetSinkState(StreamManagerState sinkState);
    uint32_t GetLatency();
    bool GetIsReadFinished();
    int32_t PeekAudioData(uint8_t *buffer, const size_t &bufferSize, AudioStreamInfo &streamInfo);
    int32_t ReloadNode(HpaeNodeInfo nodeInfo);
private:
    void DoProcessInner();
    void SilenceData();
    size_t GetRingCacheSize();
private:
    InputPort<HpaePcmBuffer *> inputStream_;
    OutputPort<HpaePcmBuffer *> outputStream_;
    std::vector<char> renderFrameData_;
    PcmBufferInfo pcmBufferInfo_;
    std::unique_ptr<AudioRingCache> ringCache_ = nullptr;
    HpaePcmBuffer outputAudioBuffer_;
    std::mutex mutex_;
    StreamManagerState state_ = STREAM_MANAGER_NEW;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
#endif
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif
