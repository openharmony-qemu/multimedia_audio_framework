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
#include "audio_suite_common.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

constexpr uint32_t MEMORY_ALIGN_BYTE_NUM = 64;
constexpr uint32_t SECONDS_TO_MS = 1000; // 1秒对应毫秒数

enum PcmDataDuration : uint32_t {
    PCM_DATA_DEFAULT_DURATION_20_MS = 20,
    PCM_DATA_DURATION_40_MS = 40,
};

inline PcmDataDuration GetDurationForRate(AudioSamplingRate rate)
{
    return (rate == AudioSamplingRate::SAMPLE_RATE_11025) ?
        PCM_DATA_DURATION_40_MS : PCM_DATA_DEFAULT_DURATION_20_MS;
}

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

struct PcmBufferFormat {
    PcmBufferFormat() = default;
    PcmBufferFormat(AudioSamplingRate rate, uint32_t channel, AudioChannelLayout layout, AudioSampleFormat format)
        : sampleRate(rate), channelCount(channel), channelLayout(layout), sampleFormat(format)
    {}

    AudioSamplingRate sampleRate;
    uint32_t channelCount;
    AudioChannelLayout channelLayout;
    AudioSampleFormat sampleFormat;
};


class AudioSuitePcmBuffer {
public:
    AudioSuitePcmBuffer() = default;
    explicit AudioSuitePcmBuffer(PcmBufferFormat format);
    explicit AudioSuitePcmBuffer(PcmBufferFormat format, PcmDataDuration duration);

    PcmBufferFormat &GetPcmBufferFormat();
    bool IsSameFormat(AudioSuitePcmBuffer &other);
    bool IsSameFormat(const PcmBufferFormat &otherFormat);
    AudioSamplingRate GetSampleRate();
    uint32_t GetChannelCount();
    AudioChannelLayout GetChannelLayout();
    AudioSampleFormat GetSampleFormat();
    uint32_t GetFrameLen();
    uint32_t GetSampleCount();
    uint32_t GetDataSize();
    uint8_t *GetPcmData();
    bool GetIsFinished();
    void SetIsFinished(bool value);
    void Reset();
    int32_t ResizePcmBuffer(PcmBufferFormat format);
    int32_t ResizePcmBuffer(PcmBufferFormat format, PcmDataDuration duration);
    void InitPcmProcess();
private:
    std::vector<uint8_t, AlignedAllocator<uint8_t, MEMORY_ALIGN_BYTE_NUM>> pcmDataBuffer_;

    PcmBufferFormat pcmBufferFormat_;
    uint32_t frameLen_;             // 帧长，单通道采样点数， 采样率 *（20/40）/ 1000
    uint32_t sampleCount_;          // 总采样点数， 通道数 * 采样率 *（20/40）/ 1000
    uint32_t dataByteSize_;         // 数据长度. 单位Byte
    uint32_t frames_;               // 帧数， 单位20ms
    uint32_t duration_;             // 单帧时长， 20ms/40ms
    bool isFinished_ = false;
};
}
}
}
#endif