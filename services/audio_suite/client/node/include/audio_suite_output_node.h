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

#ifndef AUDIO_SUITE_OUTPUT_NODE_H
#define AUDIO_SUITE_OUTPUT_NODE_H

#include "audio_suite_channel.h"
//临时
#include "audio_suite_pcm_buffer.h"
#include "channel_converter.h"
#include "hpae_format_convert.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioOutputNode : public AudioNode {
public:
    AudioOutputNode(AudioFormat format);
    virtual ~AudioOutputNode();
    virtual int32_t DoProcess() override;
    int32_t Connect(const std::shared_ptr<AudioNode> &preNode, AudioNodePortType type) override;
    int32_t Connect(const std::shared_ptr<AudioNode> &preNode) override;
    int32_t InstallTap(AudioNodePortType portType, std::shared_ptr<SuiteNodeReadTapDataCallback> callback) override;
    int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) override;
    int32_t RemoveTap(AudioNodePortType portType) override;
    int32_t DeInit() override;
    int32_t Flush() override;

    int32_t DoProcess(uint8_t *audioData, int32_t frameSize, int32_t *writeDataSize, bool *finished);
    int32_t DoProcess(uint8_t **audioDataArray, int arraySize,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag);
    std::vector<uint8_t> GetCacheBuffer();
    int32_t SetCacheBuffer(std::vector<uint8_t> cacheBuffer);
    uint8_t* GetProcessedAudioData(size_t &bytes);

    //Remap
    int32_t SetChannelConvertProcessParam(AudioChannelInfo inChannelInfo,
        AudioChannelInfo outChannelInfo, AudioSampleFormat workFormat, bool mixLfe)
    {
        return channelConverter_.SetParam(inChannelInfo, outChannelInfo, workFormat, mixLfe);
    }
    int32_t ChannelConvertProcess(uint32_t framesize, float* in, uint32_t inLen, float* out, uint32_t outLen)
    {
        return channelConverter_.Process(framesize, in, inLen, out, outLen);
    }
    int32_t CopyDataFromCache(uint8_t *audioData, int32_t frameSize, int32_t &audioDataOffset, bool *finished);
    int32_t FillRemainingAudioData(
        uint8_t *audioData, int32_t remainingBytes, int32_t *writeDataSize, bool *finished, int32_t frameSize);

private:
    InputPort<AudioSuitePcmBuffer *> inputStream_;
    std::vector<uint8_t> cacheBuffer_;
    HPAE::ChannelConverter channelConverter_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_SUITE_OUTPUT_NODE_H