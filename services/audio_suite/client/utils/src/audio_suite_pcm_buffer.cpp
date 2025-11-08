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

#ifndef LOG_TAG
#define LOG_TAG "AudioSuitePcmBuffer"
#endif

#include <cinttypes>
#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
AudioSuitePcmBuffer::AudioSuitePcmBuffer(PcmBufferFormat format)
{
    pcmBufferFormat_ = format;
    duration_ = GetDurationForRate(pcmBufferFormat_.sampleRate);
    InitPcmProcess();
}

AudioSuitePcmBuffer::AudioSuitePcmBuffer(PcmBufferFormat format, PcmDataDuration duration)
{
    pcmBufferFormat_ = format;
    duration_ = duration;
    InitPcmProcess();
}

void AudioSuitePcmBuffer::InitPcmProcess()
{
    frames_ = duration_ / PCM_DATA_DEFAULT_DURATION_20_MS;
    frameLen_ = duration_ * pcmBufferFormat_.sampleRate / SECONDS_TO_MS;
    sampleCount_ = frameLen_ * pcmBufferFormat_.channelCount;
    dataByteSize_ = sampleCount_ * AudioSuiteUtil::GetSampleSize(pcmBufferFormat_.sampleFormat);
    pcmDataBuffer_.assign(dataByteSize_, 0);
    
    AUDIO_DEBUG_LOG("AudioSuitePcmBuffer Init: rate:%{public}u, channelCount:%{public}u,"
        "channelLayout:%{public}" PRIu64 "sampleFormat:%{public}u, duration:%{public}u",
        pcmBufferFormat_.sampleRate,
        pcmBufferFormat_.channelCount,
        pcmBufferFormat_.channelLayout,
        pcmBufferFormat_.sampleFormat,
        duration_);
}

PcmBufferFormat &AudioSuitePcmBuffer::GetPcmBufferFormat()
{
    return pcmBufferFormat_;
}

bool AudioSuitePcmBuffer::IsSameFormat(AudioSuitePcmBuffer &other)
{
    return pcmBufferFormat_.sampleRate == other.GetSampleRate() &&
           pcmBufferFormat_.channelCount == other.GetChannelCount() &&
           pcmBufferFormat_.channelLayout == other.GetChannelLayout() &&
           pcmBufferFormat_.sampleFormat == other.GetSampleFormat();
}

bool AudioSuitePcmBuffer::IsSameFormat(const PcmBufferFormat &otherFormat)
{
    return pcmBufferFormat_.sampleRate == otherFormat.sampleRate &&
           pcmBufferFormat_.channelCount == otherFormat.channelCount &&
           pcmBufferFormat_.channelLayout == otherFormat.channelLayout &&
           pcmBufferFormat_.sampleFormat == otherFormat.sampleFormat;
}

AudioSamplingRate AudioSuitePcmBuffer::GetSampleRate()
{
    return pcmBufferFormat_.sampleRate;
}

uint32_t AudioSuitePcmBuffer::GetChannelCount()
{
    return pcmBufferFormat_.channelCount;
}

AudioChannelLayout AudioSuitePcmBuffer::GetChannelLayout()
{
    return pcmBufferFormat_.channelLayout;
}

AudioSampleFormat AudioSuitePcmBuffer::GetSampleFormat()
{
    return pcmBufferFormat_.sampleFormat;
}

uint32_t AudioSuitePcmBuffer::GetFrameLen()
{
    return frameLen_;
}

uint32_t AudioSuitePcmBuffer::GetSampleCount()
{
    return sampleCount_;
}

uint32_t AudioSuitePcmBuffer::GetDataSize()
{
    return dataByteSize_;
}

uint8_t *AudioSuitePcmBuffer::GetPcmData()
{
    return pcmDataBuffer_.data();
}

bool AudioSuitePcmBuffer::GetIsFinished()
{
    return isFinished_;
}

void AudioSuitePcmBuffer::SetIsFinished(bool value)
{
    isFinished_ = value;
}

int32_t AudioSuitePcmBuffer::ResizePcmBuffer(PcmBufferFormat format)
{
    if (IsSameFormat(format) && (GetDurationForRate(format.sampleRate) == duration_)) {
        return 0;
    }

    pcmBufferFormat_ = format;
    duration_ = GetDurationForRate(pcmBufferFormat_.sampleRate);

    InitPcmProcess();
    return 0;
}

int32_t AudioSuitePcmBuffer::ResizePcmBuffer(PcmBufferFormat format, PcmDataDuration duration)
{
    if (IsSameFormat(format) && (duration == duration_)) {
        return 0;
    }

    pcmBufferFormat_ = format;
    duration_ = duration;

    InitPcmProcess();
    return 0;
}

void AudioSuitePcmBuffer::Reset()
{
    pcmDataBuffer_.assign(dataByteSize_, 0);
}

}
}
}