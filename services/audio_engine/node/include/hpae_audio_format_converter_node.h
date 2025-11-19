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
#ifndef HPAE_AUDIOFORMAT_CONVERTER_H
#define HPAE_AUDIOFORMAT_CONVERTER_H
#include "audio_stream_info.h"
#include "hpae_plugin_node.h"
#include "channel_converter.h"
#include "audio_proresampler.h"
#ifdef ENABLE_HOOK_PCM
    #include "hpae_pcm_dumper.h"
#endif
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeAudioFormatConverterNode : public HpaePluginNode {
public:
    HpaeAudioFormatConverterNode(HpaeNodeInfo preNodeInfo, HpaeNodeInfo nodeInfo);
    virtual ~HpaeAudioFormatConverterNode();
    void RegisterCallback(INodeFormatInfoCallback *callback);
    void ConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode, HpaeNodeInfo &nodeInfo) override;
    void DisConnectWithInfo(const std::shared_ptr<OutputNode<HpaePcmBuffer*>> &preNode,
        HpaeNodeInfo &nodeInfo) override;
    uint64_t GetLatency(uint32_t sessionId = 0) override;
    void SetDownmixNormalization(bool normalizing);
protected:
    HpaePcmBuffer* SignalProcess(const std::vector<HpaePcmBuffer*>& inputs) override;
private:
    bool CheckUpdateInInfo(HpaePcmBuffer *input);
    bool CheckUpdateOutInfo();
    int32_t ConverterProcess(float *srcData, float *dstData, float *tmpData, HpaePcmBuffer *input);
    void CheckAndUpdateInfo(HpaePcmBuffer *input);
    void UpdateTmpOutPcmBufferInfo(const PcmBufferInfo &outPcmBufferInfo);
    PcmBufferInfo pcmBufferInfo_;
    HpaePcmBuffer converterOutput_;
    HpaeNodeInfo preNodeInfo_;
    std::unique_ptr<Resampler> resampler_ = nullptr;
    ChannelConverter channelConverter_;
    HpaePcmBuffer tmpOutBuf_; // cache between resample and converter
    // if there is render effect, the effect node decides the output format of converter node
    INodeFormatInfoCallback *nodeFormatInfoCallback_ = nullptr;
#ifdef ENABLE_HOOK_PCM
    std::unique_ptr<HpaePcmDumper> outputPcmDumper_ = nullptr;
#endif
};

} // HPAE
} // AudioStandard
} // OHOS
#endif
