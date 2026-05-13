/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SUITE_MIXER_PROCESSOR_NODE_H
#define AUDIO_SUITE_MIXER_PROCESSOR_NODE_H


#include <mutex>
#include <vector>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_limiter.h"
#include "channel_converter.h"
#include "audio_suite_mixer_processor.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class MixerProcessorNode : public MixerProcessor {
public:
    MixerProcessorNode();
    ~MixerProcessorNode() override;

    int32_t Init(const AudioFormat &format, bool enableLimiter = true);
    int32_t Process(const std::vector<AudioMixStream> &streams,
        void *outData, uint32_t outCapacity, uint32_t *outSize) override;
    void Destroy() override;

private:
    int32_t ValidateFormat(const AudioFormat &format);
    int32_t MixStreams(const std::vector<AudioMixStream> &streams,
        uint32_t sampleCount, uint32_t frameCount, float *mixOut);
    int32_t ProcessCheck(const std::vector<AudioMixStream> &streams,
        void *outData, uint32_t outCapacity, uint32_t *outSize);
    int32_t MixerLimiter(const std::vector<AudioMixStream> &streams,
        void *outData, uint32_t outCapacity, uint32_t *outSize);
    void MixU8Volume(const std::vector<AudioMixStream> &streams,
        void *outData, uint32_t outCapacity, uint32_t *outSize);
    void MixS16Volume(const std::vector<AudioMixStream> &streams,
        void *outData, uint32_t outCapacity, uint32_t *outSize);
    void MixS24Volume(const std::vector<AudioMixStream> &streams,
        void *outData, uint32_t outCapacity, uint32_t *outSize);
    void MixS32Volume(const std::vector<AudioMixStream> &streams,
        void *outData, uint32_t outCapacity, uint32_t *outSize);
    void MixF32Volume(const std::vector<AudioMixStream> &streams,
        void *outData, uint32_t outCapacity, uint32_t *outSize);

    std::mutex mutex_;
    bool enableLimiter_ = true;
    AudioFormat format_;
    bool formatConvert_ = false;
    bool channelConvert_ = false;
    uint32_t bitDepth_ = sizeof(float);
    HPAE::ChannelConverter channelToStereo_;
    HPAE::ChannelConverter channelFromStereo_;
    std::unique_ptr<AudioLimiter> limiter_;
    std::vector<float> bitDepthBuffer_;
    std::vector<float> channelBuffer_;
    std::vector<float> mixOutput_;
    std::vector<float> limiterOutput_;
    uint32_t chunkSize_ = 0;
    bool isInitialized_;
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SUITE_MIXER_PROCESSOR_NODE_H