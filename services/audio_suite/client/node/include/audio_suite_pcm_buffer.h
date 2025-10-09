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

#ifndef AUDIO_SUITE_PCM_BUFFER_H
#define AUDIO_SUITE_PCM_BUFFER_H

#include "hpae_pcm_process.h"
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

const int32_t MEMORY_ALIGN_BYTE_NUM = 64;
const int32_t SINGLE_FRAME_DURATION_SAMPLE_RATE_11025 = 40; // 采样率为11025时单帧时长， 单位ms
const int32_t SINGLE_FRAME_DURATION = 20; // 单帧时长

template <typename T, size_t Alignment>
class AlignedAllocator : public std::allocator<T> {
public:
    using pointer = T *;
    using size_type = size_t;

    pointer Allocate(size_type n)
    {
        void *ptr = std::aligned_alloc(Alignment, n * sizeof(T));
        return static_cast<pointer>(ptr);
    }

    void DeAllocate(pointer p, size_type n)
    {
        std::free(p);
    }
};

class AudioSuitePcmBuffer {
public:
    AudioSuitePcmBuffer(uint32_t sampleRate, uint32_t channelCount, AudioChannelLayout channelLayout);
    uint32_t GetChannelCount()
    {
        return channelCount_;
    }

    AudioChannelLayout GetChannelLayout()
    {
        return channelLayout_;
    }

    uint32_t GetFrameLen()
    {
        return frameLen_;
    }

    uint32_t GetSampleRate()
    {
        return sampleRate_;
    }

    std::vector<float>::iterator begin()
    {
        return pcmDataBuffer_.begin();
    }

    std::vector<float>::iterator end()
    {
        return pcmDataBuffer_.end();
    }

    float* GetPcmDataBuffer()
    {
        return pcmDataBuffer_.data();
    }

    bool GetIsFinished()
    {
        return isFinished_;
    }

    void SetIsFinished(bool value)
    {
        isFinished_ = value;
    }

    bool GetIsInterleaved()
    {
        return isInterleaved_;
    }

    void SetIsInterleaved(bool value)
    {
        isInterleaved_ = value;
    }

    void Reset()
    {
        pcmDataBuffer_.assign(frameLen_, 0.0f);
        InitPcmProcess();
    }

    HPAE::HpaePcmProcess &operator[](size_t index)
    {
        return pcmProcessVec_[index];
    }

    const HPAE::HpaePcmProcess &operator[](size_t index) const
    {
        return pcmProcessVec_[index];
    }

    int32_t InitPcmProcess();
    int32_t ResizePcmBuffer(uint32_t sampleRate, uint32_t channelCount);
    int32_t ResetPcmBuffer(uint32_t sampleRate, uint32_t channelCount, AudioChannelLayout channelLayout);
    AudioSuitePcmBuffer &operator+=(AudioSuitePcmBuffer &other);
private:
    std::vector<float, AlignedAllocator<float, MEMORY_ALIGN_BYTE_NUM>> pcmDataBuffer_;
    std::vector<HPAE::HpaePcmProcess> pcmProcessVec_;
    uint32_t frameLen_; // 帧长， 多少个float
    uint32_t sampleRate_;
    uint32_t channelCount_;
    AudioChannelLayout channelLayout_;
    bool isInterleaved_ = true;
    bool isFinished_ = false;
};
}
}
}
#endif