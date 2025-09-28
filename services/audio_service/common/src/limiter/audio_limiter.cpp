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
const int32_t PROC_COUNT = 4;              // process 4 times
const int32_t AUDIO_LMT_ALGO_CHANNEL = 2;  // 2 channel for stereo
const int32_t AUDIO_LMT_ALGO_BYTE_PER_SAMPLE = sizeof(float);

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
    channels_ = 0;
    sampleRate_ = 0;
    AUDIO_INFO_LOG("AudioLimiter");
}

AudioLimiter::~AudioLimiter()
{
    DumpFileUtil::CloseDumpFile(&dumpFileInput_);
    DumpFileUtil::CloseDumpFile(&dumpFileOutput_);
    AUDIO_INFO_LOG("~AudioLimiter");
}

int32_t AudioLimiter::SetConfig(int32_t inputFrameBytes, int32_t bytePerSample, int32_t sampleRate, int32_t channels)
{
    // reset
    algoFrameLen_ = 0;

    CHECK_AND_RETURN_RET_LOG(inputFrameBytes > 0 && sampleRate > 0 && channels == AUDIO_LMT_ALGO_CHANNEL &&
                                 bytePerSample == AUDIO_LMT_ALGO_BYTE_PER_SAMPLE,
        ERR_INVALID_PARAM,
        "Invalid input parameters");
    CHECK_AND_RETURN_RET_LOG((inputFrameBytes / bytePerSample) % PROC_COUNT == 0,
        ERR_INVALID_PARAM,
        "Invalid inputFrameBytes, frameLen must be an even number, intput frameLen is %{public}d",
        inputFrameBytes / (bytePerSample * channels));

    int32_t newAlgoFrameLen = inputFrameBytes / (bytePerSample * PROC_COUNT);
    CHECK_AND_RETURN_RET_LOG(newAlgoFrameLen > 0,
        ERR_INVALID_PARAM,
        "Invalid inputFrameBytes, min request frameBytes is %{public}d",
        bytePerSample * PROC_COUNT * channels);

    latency_ = static_cast<uint32_t>(newAlgoFrameLen * AUDIO_MS_PER_S / (sampleRate * channels));

    if (bufHis_.capacity() < static_cast<size_t>(newAlgoFrameLen)) {
        bufHis_.assign(newAlgoFrameLen + 1, 0);
    } else {
        memset_s(bufHis_.data(), bufHis_.capacity() * sizeof(float), 0, bufHis_.capacity() * sizeof(float));
    }

    CHECK_AND_RETURN_RET_LOG(bufHis_.capacity() > static_cast<size_t>(newAlgoFrameLen),
        ERR_MEMORY_ALLOC_FAILED,
        "allocate limit algorithm buffer failed, buffer capacity %{public}zu, requestLen %{public}d",
        bufHis_.capacity(),
        newAlgoFrameLen);
    algoFrameLen_ = newAlgoFrameLen;

    sampleRate_ = static_cast<uint32_t>(sampleRate);
    channels_ = static_cast<uint32_t>(channels);
    dumpFileNameIn_ = std::to_string(sinkIndex_) + "_limiter_in_" + GetTime() + "_" + std::to_string(sampleRate) + "_" +
                      std::to_string(channels) + "_" + std::to_string(format_) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileNameIn_, &dumpFileInput_);
    dumpFileNameOut_ = std::to_string(sinkIndex_) + "_limiter_out_" + GetTime() + "_" + std::to_string(sampleRate) +
                       "_" + std::to_string(channels) + "_" + std::to_string(format_) + ".pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileNameOut_, &dumpFileOutput_);

    AUDIO_INFO_LOG("SetConfig Success, inputFrameBytes = %{public}d, bytePerSample = %{public}d, sampleRate = "
                   "%{public}d, channels = %{public}d,"
                   "newAlgoFrameLen = %{public}d, latency = %{public}d",
        inputFrameBytes,
        bytePerSample,
        sampleRate,
        channels,
        algoFrameLen_,
        latency_);

    return SUCCESS;
}

int32_t AudioLimiter::Process(int32_t inputSampleCount, float *inBuffer, float *outBuffer)
{
    CHECK_AND_RETURN_RET_LOG(
        inBuffer != nullptr && outBuffer != nullptr, ERR_NULL_POINTER, "AudioLimiter Process Error, buffer is nullptr");

    CHECK_AND_RETURN_RET_LOG(algoFrameLen_ > 0 && bufHis_.capacity() > static_cast<size_t>(algoFrameLen_),
        ERR_NOT_STARTED,
        "could not do process before SetConfig success");

    CHECK_AND_RETURN_RET_LOG(algoFrameLen_ * PROC_COUNT == inputSampleCount,
        ERR_INVALID_PARAM,
        "error, requestSample = %{public}d, inputSample = %{public}d",
        algoFrameLen_ * PROC_COUNT,
        inputSampleCount);

    int32_t ptrIndex = 0;
    if (dumpFileInput_ == nullptr) {
        dumpFileNameIn_ = std::to_string(sinkIndex_) + "_limiter_in_" + GetTime() + "_" + std::to_string(sampleRate_) +
                          "_" + std::to_string(channels_) + "_" + std::to_string(format_) + ".pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileNameIn_, &dumpFileInput_);
        AUDIO_DEBUG_LOG("Reopen dump file: %{public}s", dumpFileNameIn_.c_str());
    }
    if (dumpFileOutput_ == nullptr) {
        dumpFileNameOut_ = std::to_string(sinkIndex_) + "_limiter_out_" + GetTime() + "_" +
                           std::to_string(sampleRate_) + "_" + std::to_string(channels_) + "_" +
                           std::to_string(format_) + ".pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_SERVER_PARA, dumpFileNameOut_, &dumpFileOutput_);
        AUDIO_DEBUG_LOG("Reopen dump file: %{public}s", dumpFileNameOut_.c_str());
    }
    DumpFileUtil::WriteDumpFile(dumpFileInput_, static_cast<void *>(inBuffer), inputSampleCount * sizeof(float));
    for (int32_t i = 0; i < PROC_COUNT; i++) {
        ProcessAlgo(inBuffer + ptrIndex, outBuffer + ptrIndex);
        ptrIndex += algoFrameLen_;
    }
    DumpFileUtil::WriteDumpFile(dumpFileOutput_, static_cast<void *>(outBuffer), inputSampleCount * sizeof(float));
    return SUCCESS;
}

void AudioLimiter::ProcessAlgo(float *inBuffer, float *outBuffer)
{
    // calculate envelope energy
    float maxEnvelopeLevel = 0.0f;
    for (int32_t i = 0; i < algoFrameLen_ - 1; i += AUDIO_LMT_ALGO_CHANNEL) {
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
    float deltaGain = (gain_ - lastGain) * AUDIO_LMT_ALGO_CHANNEL / algoFrameLen_;

    // apply gain
    if (algoFrameLen_ % AUDIO_LMT_ALGO_CHANNEL == 0) {
        for (int32_t i = 0; i < algoFrameLen_; i += AUDIO_LMT_ALGO_CHANNEL) {
            lastGain += deltaGain;
            outBuffer[i] = bufHis_[i] * lastGain;
            outBuffer[i + 1] = bufHis_[i + 1] * lastGain;
            bufHis_[i] = inBuffer[i];
            bufHis_[i + 1] = inBuffer[i + 1];
        }
    } else {
        outBuffer[0] = bufHis_[0] * lastGain;
        bufHis_[0] = bufHis_[algoFrameLen_];
        for (int32_t i = 1; i < algoFrameLen_ - 1; i += AUDIO_LMT_ALGO_CHANNEL) {
            lastGain += deltaGain;
            outBuffer[i] = bufHis_[i] * lastGain;
            outBuffer[i + 1] = bufHis_[i + 1] * lastGain;
            bufHis_[i] = inBuffer[i - 1];
            bufHis_[i + 1] = inBuffer[i];
        }
        bufHis_[algoFrameLen_] = inBuffer[algoFrameLen_ - 1];
    }
}

uint32_t AudioLimiter::GetLatency()
{
    return latency_;
}
}  // namespace AudioStandard
}  // namespace OHOS
