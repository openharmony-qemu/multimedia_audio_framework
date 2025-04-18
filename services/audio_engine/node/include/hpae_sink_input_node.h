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

#ifndef HPAE_SINK_INPUT_NODE_H
#define HPAE_SINK_INPUT_NODE_H
#include <memory>
#include <atomic>
#include "hpae_msg_channel.h"
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_info.h"
#include "i_renderer_stream.h"
#include "linear_pos_time_model.h"
#ifdef ENABLE_HOOK_PCM
#include "hpae_pcm_dumper.h"
#endif
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
typedef void (*AppCallbackFunc)(void *pHndl);

class HpaeSinkInputNode : public OutputNode<HpaePcmBuffer *> {
public:
    HpaeSinkInputNode(HpaeNodeInfo &nodeInfo);
    ~HpaeSinkInputNode();
    virtual void DoProcess() override;
    virtual bool Reset() override;     // no implement, virtual class
    virtual bool ResetAll() override;  // no implement, virtual class
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort() override;
    bool RegisterWriteCallback(const std::weak_ptr<IStreamCallback> &callback);
    void Flush();
    bool Drain();
    int32_t SetState(RendererState renderState);
    RendererState GetState();
    uint64_t GetFramesWritten();

    int32_t GetCurrentPosition(uint64_t &framePosition, uint64_t &timestamp);
    int32_t RewindHistoryBuffer(uint64_t rewindTime);

private:
    void CheckAndDestoryHistoryBuffer();
    bool GetAudioTime(uint64_t &framePos, int64_t &sec, int64_t &nanoSec);
    std::weak_ptr<IStreamCallback> writeCallback_;
    AudioCallBackStreamInfo streamInfo_;
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer inputAudioBuffer_;
    OutputPort<HpaePcmBuffer *> outputStream_;
    std::vector<int8_t> interleveData_;
    std::atomic<uint64_t> framesWritten_;
    uint64_t totalFrames_;
    std::unique_ptr<LinearPosTimeModel> handleTimeModel_;
    bool isDrain_ = false;
    RendererState state_ = RENDERER_NEW;

    std::unique_ptr<HpaePcmBuffer> historyBuffer_;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> inputPcmDumper_ = nullptr;
#endif
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif