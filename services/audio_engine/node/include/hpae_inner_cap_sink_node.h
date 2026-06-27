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
#ifndef HPAE_INNER_CAP_SINK_NODE_H
#define HPAE_INNER_CAP_SINK_NODE_H
#include <memory>
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "source/i_audio_capture_source.h"
#include "hpae_renderer_manager.h"
#include "hpae_source_input_node.h"
 
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
 
class HpaeInnerCapSinkNode : public OutputNode<HpaePcmBuffer*>, public InputNode<HpaePcmBuffer*> {
public:
    HpaeInnerCapSinkNode(HpaeNodeInfo& nodeInfo);
    virtual ~HpaeInnerCapSinkNode();
    virtual void DoProcess() override;
    virtual bool Reset() override;
    virtual bool ResetAll() override;
 
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer*>* GetOutputPort() override;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer*>>& preNode) override;
    size_t GetPreOutNum();
    size_t GetOutputPortNum();
 
    int32_t InnerCapturerSinkInit();
    int32_t InnerCapturerSinkDeInit();
    int32_t InnerCapturerSinkFlush();
    int32_t InnerCapturerSinkPause();
    int32_t InnerCapturerSinkReset();
    int32_t InnerCapturerSinkResume();
    int32_t InnerCapturerSinkStart();
    int32_t InnerCapturerSinkStop();
    StreamManagerState GetSinkState();
    int32_t SetSinkState(StreamManagerState sinkState);
    void SetMute(bool isMute);
private:
    OutputPort<HpaePcmBuffer*> outputStream_;
    InputPort<HpaePcmBuffer*> inputStream_;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer silenceData_;

    StreamManagerState state_ = STREAM_MANAGER_NEW;
    std::chrono::high_resolution_clock::time_point historyTime_;
    std::chrono::high_resolution_clock::time_point endTime_;
    std::chrono::nanoseconds sleepTime_;
    std::atomic<bool> isMute_ = false;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
#endif
};
 
}}}
 
#endif