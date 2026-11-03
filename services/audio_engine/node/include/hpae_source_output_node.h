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

#ifndef HPAE_SOURCE_OUTPUT_NODE_H
#define HPAE_SOURCE_OUTPUT_NODE_H
#include <memory>
#include "hpae_node.h"
#include "hpae_pcm_buffer.h"
#include "audio_info.h"
#include "i_capturer_stream.h"
#include "audio_highpass_filter.h"
#include "audio_limiter.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {

extern int32_t PC_ENABLE_STATE;

class HpaeSourceOutputNode : public InputNode<HpaePcmBuffer *> {
public:
    HpaeSourceOutputNode(HpaeNodeInfo &nodeInfo);
    virtual ~HpaeSourceOutputNode();
    virtual void DoProcess() final;
    virtual bool Reset() final;
    bool ResetAll() final;
    void Connect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode, HpaeNodeInfo &nodeInfo) override;
    void DisConnect(const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode) override;
    void DisConnectWithInfo(
        const std::shared_ptr<OutputNode<HpaePcmBuffer *>> &preNode, HpaeNodeInfo &nodeInfo) override;
    bool RegisterReadCallback(const std::weak_ptr<ICapturerStreamCallback> &callback);
    int32_t SetState(HpaeSessionState captureState);
    HpaeSessionState GetState();
    void SetAppUid(int32_t appUid);
    int32_t GetAppUid();
    void SetTokenId(uint32_t tokenId);
    uint32_t GetTokenId();
    void SetMute(bool isMute);
    bool GetMute();
    void SetInjectorState(bool isInjector);
    void SetMicMute(bool isMute);
    int32_t SetupAudioLimiter();
    void ResetAudioLimiter();
    static int32_t GetPcEnableState();
    static void SetPcEnableState(int32_t val);
    void SetNodeInfo(HpaeNodeInfo &nodeInfo) override;
private:
    uint64_t GetTimestamp();
    void ApplyHighpassFilter(HpaePcmBuffer *inputPcmBuffer, uint32_t safeFrameCount);
    HpaePcmBuffer *GetAndValidateOutput();
    HpaePcmBuffer *SelectOutputData(const std::vector<HpaePcmBuffer *> &outputVec);
    HpaePcmBuffer *MixInputs(const std::vector<HpaePcmBuffer *> &inputs);
    void ProcessAudioData(HpaePcmBuffer *outputData, bool isMute);
    void WriteDataAndUpdateFrames();
    void OnTraceInfoChanged() override;
    void RebuildTraceNames();
private:
    InputPort<HpaePcmBuffer *> inputStream_;
    std::weak_ptr<ICapturerStreamCallback> readCallback_;
    AudioCallBackCapturerStreamInfo streamInfo_;
    std::vector<char> sourceOutputData_;
    std::vector<float> interleveData_;
    std::atomic<uint64_t> framesRead_;
    HpaeSessionState state_ = HPAE_SESSION_NEW;
    uint64_t totalFrames_;
    int32_t appUid_ = -1;
    uint32_t tokenId_ = 0;
    bool isMute_;
    bool isInjector_ = false;
    bool isMicMute_ = false;
    std::unique_ptr<AudioHighPassFilter> highpassFilter_;
    int32_t filterChannelCount_;
    bool isSupportHighpassFilter_;

    // injector mix resources (mix mic + injector with limiter when isInjector_ && !isMicMute_)
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer mixedOutput_;
    HpaePcmBuffer tmpOutput_;
    HpaePcmBuffer silenceData_;
    std::unique_ptr<AudioLimiter> limiter_ = nullptr;

    // Cached DoProcess trace names (mute/unmute); rebuilt in ctor and whenever node info changes
    std::string muteTraceName_;
    std::string unmuteTraceName_;
    // edge-triggered gates: log first hit + 5s heartbeat while the state persists
    AudioEdgeLogGate downlinkActiveGate_{AUDIO_LOG_HEARTBEAT_FRAMES};
    AudioEdgeLogGate readCbNullGate_{AUDIO_LOG_HEARTBEAT_FRAMES};
};

}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif
