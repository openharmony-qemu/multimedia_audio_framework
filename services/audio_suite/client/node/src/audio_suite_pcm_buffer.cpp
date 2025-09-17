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

#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_input_node.h"
#include "audio_suite_info.h"
#include "audio_suite_pcm_buffer.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {
AudioSuitePcmBuffer::AudioSuitePcmBuffer(uint32_t sampleRate, uint32_t channelCount, AudioChannelLayout channelLayout)
{
    AUDIO_INFO_LOG("AudioSuitePcmBuffer::AudioSuitePcmBuffer, sampleRate:%{public}u, channelCount:%{public}u",
        sampleRate, channelCount);
    sampleRate_ = sampleRate;
    channelCount_ = channelCount;
    channelLayout_ = channelLayout;
    frameLen_ = SINGLE_FRAME_DURATION * sampleRate * channelCount / SECONDS_TO_MS;

    pcmDataBuffer_.assign(frameLen_, 0.0f);
    AUDIO_INFO_LOG("AudioSuitePcmBuffer::AudioSuitePcmBuffer, frameLen_:%{public}u", frameLen_);
    InitPcmProcess();
}

int32_t AudioSuitePcmBuffer::InitPcmProcess()
{
    AUDIO_INFO_LOG("AudioSuitePcmBuffer::InitPcmProcess start");
    pcmProcessVec_.clear();
    float* itr = pcmDataBuffer_.data();
    pcmProcessVec_.push_back(HPAE::HpaePcmProcess(itr, frameLen_));
    AUDIO_INFO_LOG("AudioSuitePcmBuffer::InitPcmProcess, finish");
    return 0;
}
int32_t AudioSuitePcmBuffer::ResizePcmBuffer(uint32_t sampleRate, uint32_t channelCount)
{
    AUDIO_INFO_LOG("AudioSuitePcmBuffer::ResizePcmBuffer, sampleRate:%{public}u, channelCount:%{public}u",
        sampleRate, channelCount);
    sampleRate_ = sampleRate;
    channelCount_ = channelCount;
    frameLen_ = SINGLE_FRAME_DURATION * sampleRate * channelCount / SECONDS_TO_MS;
    pcmDataBuffer_.assign(frameLen_, 0.0f);
    pcmProcessVec_.clear();
    InitPcmProcess();
    return 0;
}
}
}
}