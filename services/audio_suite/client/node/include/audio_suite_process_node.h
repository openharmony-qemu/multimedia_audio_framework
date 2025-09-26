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
#ifndef AUDIO_SUITE_PROCESS_NODE_H
#define AUDIO_SUITE_PROCESS_NODE_H

#include <unordered_set>
#include <memory>
#include <vector>
#include "audio_errors.h"
#include "audio_suite_channel.h"
#include "channel_converter.h"
#include "hpae_format_convert.h"
#include "audio_suite_pcm_buffer.h"
#include "audio_proresampler.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
class AudioSuiteProcessNode : public AudioNode {
public:
    AudioSuiteProcessNode(AudioNodeType nodeType, AudioFormat audioFormat);
    virtual ~AudioSuiteProcessNode() = default;
    virtual bool Reset() = 0;
    int32_t DoProcess() override;
    int32_t Connect(const std::shared_ptr<AudioNode>& preNode, AudioNodePortType type) override;
    int32_t DisConnect(const std::shared_ptr<AudioNode>& preNode) override;
    int32_t Flush() override;
    int32_t InstallTap(AudioNodePortType portType, std::shared_ptr<SuiteNodeReadTapDataCallback> callback) override;
    int32_t RemoveTap(AudioNodePortType portType) override;
    AudioSuiteProcessNode(const AudioSuiteProcessNode& others) = delete;
    int32_t SetUpResample(uint32_t inRate, uint32_t outRate, uint32_t channels, uint32_t quality);
    int32_t DoResampleProcess(const float *inBuffer, uint32_t inFrameSize,
        float *outBuffer, uint32_t outFrameSize);
    AudioSamplingRate GetSampleRate()
    {
        return AudioNode::GetAudioNodeInfo().audioFormat.rate;
    }
    AudioSampleFormat GetBitWidth()
    {
        return AudioNode::GetAudioNodeInfo().audioFormat.format;
    }
    uint32_t GetChannelCount()
    {
        return AudioNode::GetAudioNodeInfo().audioFormat.audioChannelInfo.numChannels;
    }

    virtual std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> GetOutputPort(AudioNodePortType type) override
    {
        if (!outputStream_) {
            outputStream_ = std::make_shared<OutputPort<AudioSuitePcmBuffer*>>(GetSharedInstance());
        }
        return outputStream_;
    }

    //FormatConvert
    void ConvertToFloat(AudioSampleFormat format, unsigned n, void *src, float *dst)
    {
        HPAE::ConvertToFloat(format, n, src, dst);
    }
    void ConvertFromFloat(AudioSampleFormat format, unsigned n, float *src, void *dst)
    {
        HPAE::ConvertFromFloat(format, n, src, dst);
    }

    //Remap
    int32_t SetChannelConvertProcessParam(AudioChannelInfo inChannelInfo, AudioChannelInfo outChannelInfo,
        AudioSampleFormat workFormat, bool mixLfe)
    {
        return channelConverter_.SetParam(inChannelInfo, outChannelInfo, workFormat, mixLfe);
    }
    int32_t ChannelConvertProcess(uint32_t framesize, float* in, uint32_t inLen, float* out, uint32_t outLen)
    {
        return channelConverter_.Process(framesize, in, inLen, out, outLen);
    }

protected:
    std::shared_ptr<OutputPort<AudioSuitePcmBuffer*>> outputStream_;
    std::shared_ptr<InputPort<AudioSuitePcmBuffer*>> inputStream_;
    virtual AudioSuitePcmBuffer* SignalProcess(const std::vector<AudioSuitePcmBuffer*>& inputs) = 0;
    virtual void HandleTapCallback(AudioSuitePcmBuffer* pcmBuffer);
    std::vector<AudioSuitePcmBuffer*>& ReadProcessNodePreOutputData();
    std::unordered_set<std::shared_ptr<AudioNode>> finishedPrenodeSet;

private:
    Tap tap_;
    HPAE::ChannelConverter channelConverter_;
    std::unique_ptr<HPAE::ProResampler> proResampler_ = nullptr;
};

}
}
}
#endif