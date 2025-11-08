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

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
typedef void (*AppCallbackFunc)(void *pHndl);

class HpaeSinkInputNode : public OutputNode<HpaePcmBuffer *> {
public:
    HpaeSinkInputNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeSinkInputNode();
    virtual void DoProcess() override;
    virtual bool Reset() override;     // no implement, virtual class
    virtual bool ResetAll() override;  // no implement, virtual class
    std::shared_ptr<HpaeNode> GetSharedInstance() override;
    OutputPort<HpaePcmBuffer *> *GetOutputPort() override;
    bool RegisterWriteCallback(const std::weak_ptr<IStreamCallback> &callback);
    void Flush();
    bool Drain();
    int32_t SetState(HpaeSessionState renderState);
    HpaeSessionState GetState();

    int32_t GetCurrentPosition(uint64_t &framePosition, std::vector<uint64_t> &timestamp);
    void RewindHistoryBuffer(uint64_t rewindTime, uint64_t hdiFramePosition = 0);

    void SetAppUid(int32_t appUid);
    int32_t GetAppUid();

    void SetOffloadEnabled(bool offloadEnable);
    bool GetOffloadEnabled();
    int32_t SetLoudnessGain(float loudnessGain);
    float GetLoudnessGain();
    void SetSpeed(float speed);
    float GetSpeed();
    uint64_t GetLatency();

    bool QueryUnderrun();

    bool isConnected_ = false;
    HpaeProcessorType connectedProcessorType_ = HPAE_SCENE_UNCONNECTED;
private:
    int32_t GetDataFromSharedBuffer();
    int32_t OnStreamInfoChange(bool needata = true);
    void CheckAndDestroyHistoryBuffer();
    bool ReadToAudioBuffer(int32_t &ret);
    void UpdateDataFlag(HpaeNodeInfo &nodeInfo);
    std::weak_ptr<IStreamCallback> writeCallback_;
    AudioCallBackStreamInfo streamInfo_;
    PcmBufferInfo pcmBufferInfo_;
    PcmBufferInfo emptyBufferInfo_;
    HpaePcmBuffer inputAudioBuffer_;
    HpaePcmBuffer emptyAudioBuffer_;
    OutputPort<HpaePcmBuffer *> outputStream_;
    std::vector<int8_t> interleveData_;
    uint64_t totalFrames_;
    bool isDrain_ = false;
    HpaeSessionState state_ = HPAE_SESSION_NEW;
    int32_t appUid_ = -1;
    bool pullDataFlag_ = false; // pull data each 40ms for 11025hz input
    uint8_t pullDataCount_ = 0; // for customSampleRate that is not multiples of 50, eg. 8010, pull data each 100ms
    std::unique_ptr<HpaePcmBuffer> historyBuffer_;
    bool offloadEnable_ = false;
    float loudnessGain_ = 0.0f;
    float speed_ = 1.0f;
    std::atomic<uint64_t> hdiFramePosition_ = 0;
    uint32_t standbyCounter_ = 0;
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif