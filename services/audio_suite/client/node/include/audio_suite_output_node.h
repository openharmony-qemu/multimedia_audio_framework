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

#include <vector>
#include "audio_suite_node.h"
#include "audio_suite_channel.h"
#include "audio_suite_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioOutputNode : public AudioNode {
public:
    explicit AudioOutputNode(AudioFormat format);
    virtual ~AudioOutputNode();

    int32_t Init() override;
    int32_t DeInit() override;
    int32_t Flush() override;
    void SetAudioNodeFormat(AudioFormat audioFormat) override;
    int32_t Connect(const std::shared_ptr<AudioNode> &preNode, AudioNodePortType type) override;
    int32_t Connect(const std::shared_ptr<AudioNode> &preNode) override;
    int32_t DisConnect(const std::shared_ptr<AudioNode> &preNode) override;

    virtual int32_t DoProcess() override;
    int32_t DoProcess(uint8_t *audioData, int32_t frameSize, int32_t *writeDataSize, bool *finished);
    int32_t DoProcess(uint8_t **audioDataArray, int32_t arraySize,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag);

private:
    int32_t DoProcessParamCheck(uint8_t **audioDataArray, int32_t arraySize,
        int32_t requestFrameSize, int32_t *responseSize, bool *finishedFlag);
    int32_t FormatConversion(float *inData, size_t inDataLen, uint8_t *outData, size_t outDataSize);
    void SetInDataFormat(uint32_t channels, AudioChannelLayout layout, AudioSampleFormat sample, uint32_t rate);
    void SetOutDataFormat(uint32_t channels, AudioChannelLayout layout, AudioSampleFormat sample, uint32_t rate);

    struct DataFormat {
        AudioChannelLayout channelLayout = CH_LAYOUT_UNKNOWN;
        uint32_t numChannels = 1;
        AudioSampleFormat format = INVALID_WIDTH;
        uint32_t rate = SAMPLE_RATE_48000;
    };

    // cache buffer opt
    bool CacheBufferEmpty();
    void UpdateUsedOffset(size_t bytesConsumed);
    void ClearCacheBuffer();
    int32_t GetCacheBufferDataLen();
    uint8_t *GetCacheBufferData(size_t idx);

    InputPort<AudioSuitePcmBuffer *> inputStream_;
    int32_t preNodeOutputNum_ = 0;

    // for cache buffer
    std::vector<std::vector<uint8_t>> cacheBuffer_;
    size_t bufferUsedOffset_ = 0;

    // for format conversion
    uint32_t frameDuration_ = SINGLE_FRAME_DURATION;
    DataFormat inFormat_;
    DataFormat outFormat_;
    std::vector<float> rateOutput_;
    std::vector<float> channelOutput_;
};

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_SUITE_OUTPUT_NODE_H