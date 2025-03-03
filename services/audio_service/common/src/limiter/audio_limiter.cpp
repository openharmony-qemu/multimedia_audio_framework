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
#define LOG_TAG "AudioLimiter"
#endif

#include "audio_errors.h"
#include "audio_limiter.h"
#include "audio_common_log.h"
#include "audio_utils.h"

#include "securec.h"

namespace OHOS {
namespace AudioStandard {

const float NEXT_LEVEL = 0.5f;
const float THRESHOLD = 0.92f;
const float LEVEL_ATTACK = 0.3f;
const float LEVEL_RELEASE = 0.7f;
const float GAIN_ATTACK = 0.1f;
const float GAIN_RELEASE = 0.6f;
const int32_t AUDIO_FORMAT_PCM_FLOAT = 4;
const int32_t PROC_COUNT = 4;  // process 4 times
const int32_t AUDIO_MS_PER_S = 1000;
const int32_t AUDIO_LMT_ALGO_CHANNEL = 2;    // 2 channel for stereo

AudioLimiter::AudioLimiter(int32_t sinkIndex)
{
    sinkIndex_ = sinkIndex;
    nextLev_ = NEXT_LEVEL;
    threshold_ = THRESHOLD;
    levelAttack_ = LEVEL_ATTACK;
    levelRelease_ = LEVEL_RELEASE;
    gainAttack_ = GAIN_ATTACK;
    gainRelease_ = GAIN_RELEASE;
    format_ = AUDIO_FORMAT_PCM_FLOAT;
    latency_ = 0;
    algoFrameLen_ = 0;
    curMaxLev_ = 0.0f;
    gain_ = 0.0f;
    bufHis_ = nullptr;
    AUDIO_INFO_LOG("AudioLimiter");
}

AudioLimiter::~AudioLimiter()
{
    ReleaseBuffer();
    DumpFileUtil::CloseDumpFile(&dumpFileInput_);
    DumpFileUtil::CloseDumpFile(&dumpFileOutput_);
    AUDIO_INFO_LOG("~AudioLimiter");
}

void AudioLimiter::ReleaseBuffer()
{
    if (bufHis_ != nullptr) {
        delete[] bufHis_;
        bufHis_ = nullptr;
    }
    return;
}

int32_t AudioLimiter::SetConfig(int32_t maxRequest, int32_t biteSize, int32_t sampleRate, int32_t channels)
{
    CHECK_AND_RETURN_RET_LOG(maxRequest > 0 && biteSize > 0 && sampleRate > 0 && channels == AUDIO_LMT_ALGO_CHANNEL,
        ERROR, "Invalid input parameters");
    algoFrameLen_ = maxRequest / (biteSize * PROC_COUNT);
    latency_ = static_cast<uint32_t>(algoFrameLen_ * AUDIO_MS_PER_S / (sampleRate * channels));
    AUDIO_INFO_LOG("maxRequest = %{public}d, biteSize = %{public}d, sampleRate = %{public}d, channels = %{public}d,"
        "algoFrameLen_ = %{public}d, latency_ = %{public}d",
        maxRequest, biteSize, sampleRate, channels, algoFrameLen_, latency_);
    bufHis_ = new (std::nothrow) float[algoFrameLen_]();
    CHECK_AND_RETURN_RET_LOG(bufHis_ != nullptr, ERROR, "allocate limit algorithm buffer failed");

    dumpFileNameIn_ = std::to_string(sinkIndex_) + "_limiter_in_" + GetTime() + "_" + std::to_string(sampleRate) + "_"
        + std::to_string(channels) + "_" + std::to_string(format_) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileNameIn_, &dumpFileInput_);
    dumpFileNameOut_ = std::to_string(sinkIndex_) + "_limiter_out_" + GetTime() + "_" + std::to_string(sampleRate) + "_"
        + std::to_string(channels) + "_" + std::to_string(format_) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileNameOut_, &dumpFileOutput_);

    return SUCCESS;
}

int32_t AudioLimiter::Process(int32_t frameLen, float *inBuffer, float *outBuffer)
{
    CHECK_AND_RETURN_RET_LOG(algoFrameLen_ * PROC_COUNT == frameLen, ERROR,
        "error, algoFrameLen_ = %{public}d, frameLen = %{public}d", algoFrameLen_, frameLen);
    int32_t ptrIndex = 0;
    DumpFileUtil::WriteDumpFile(dumpFileInput_, static_cast<void *>(inBuffer), frameLen * sizeof(float));
    for (int32_t i = 0; i < PROC_COUNT; i++) {
        ProcessAlgo(algoFrameLen_, inBuffer + ptrIndex, outBuffer + ptrIndex);
        ptrIndex += algoFrameLen_;
    }
    DumpFileUtil::WriteDumpFile(dumpFileOutput_, static_cast<void *>(outBuffer), frameLen * sizeof(float));
    return SUCCESS;
}

void AudioLimiter::ProcessAlgo(int algoFrameLen, float *inBuffer, float *outBuffer)
{
    // calculate envelope energy
    CHECK_AND_RETURN_LOG(algoFrameLen > 0, "algoFrameLen is invalid");
    float maxEnvelopeLevel = 0.0f;
    for (int32_t i = 0; i < algoFrameLen - 1; i += AUDIO_LMT_ALGO_CHANNEL) {
        float tempBufInLeft = inBuffer[i];
        float tempBufInRight = inBuffer[i + 1];
        float tempLevel = std::max(std::abs(tempBufInLeft), std::abs(tempBufInRight));
        float coeff = tempLevel > nextLev_ ? levelAttack_ : levelRelease_;
        nextLev_ = coeff * nextLev_ + (1 - coeff) * tempLevel;
        maxEnvelopeLevel = std::max(maxEnvelopeLevel, nextLev_);
    }

    // calculate gain
    float tempMaxLevel = std::max(maxEnvelopeLevel, curMaxLev_);
    curMaxLev_ = maxEnvelopeLevel;
    float targetGain = 1.0f;
    if (tempMaxLevel != 0) {
        targetGain = tempMaxLevel > threshold_ ? threshold_ / tempMaxLevel : targetGain;
    }
    float lastGain = gain_;
    float coeff = gain_ > targetGain ? gainAttack_ : gainRelease_;
    gain_ = coeff * gain_ + (1 - coeff) * targetGain;
    float deltaGain = (gain_ - lastGain) * AUDIO_LMT_ALGO_CHANNEL / algoFrameLen;

    // apply gain
    if (algoFrameLen % AUDIO_LMT_ALGO_CHANNEL == 0) {
        for (int32_t i = 0; i < algoFrameLen; i += AUDIO_LMT_ALGO_CHANNEL) {
            lastGain += deltaGain;
            outBuffer[i] = bufHis_[i] * lastGain;
            outBuffer[i + 1] = bufHis_[i + 1] * lastGain;
            bufHis_[i] = inBuffer[i];
            bufHis_[i + 1] = inBuffer[i + 1];
        }
    } else {
        outBuffer[0] = bufHis_[0];
        bufHis_[0] = bufHis_[algoFrameLen];
        for (int32_t i = 1; i < algoFrameLen - 1; i += AUDIO_LMT_ALGO_CHANNEL) {
            lastGain += deltaGain;
            outBuffer[i] = bufHis_[i] * lastGain;
            outBuffer[i + 1] = bufHis_[i + 1] * lastGain;
            bufHis_[i] = inBuffer[i - 1];
            bufHis_[i + 1] = inBuffer[i];
        }
        bufHis_[algoFrameLen] = inBuffer[algoFrameLen - 1];
    }
}

uint32_t AudioLimiter::GetLatency()
{
    return latency_;
}
} // namespace AudioStandard
} // namespace OHOS
