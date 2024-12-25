/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioLmt"
#endif

#include "audio_lmt.h"

namespace OHOS {
namespace AudioStandard {

constexpr float NEXT_LEVEL = 0.5f;
constexpr float THRESHOLD = 0.92f;
constexpr float LEVEL_ATTACK = 0.3f;
constexpr float LEVEL_RELEASE = 0.7f;
constexpr float GAIN_ATTACK = 0.1f;
constexpr float GAIN_RELEASE = 0.6f;
constexpr float PROC_TIME = 0.005f;  // 5ms

AudioLimiter::AudioLimiter(int32_t sinkNameCode)
{
    sinkNameCode_ = sinkNameCode;
    nextLev_ = NEXT_LEVEL;
    curMaxLev_ = 0.0f;
    threshold_ = THRESHOLD;
    gain_ = 0.0f;
    levelAttack_ = LEVEL_ATTACK;
    levelRelease_ = LEVEL_RELEASE;
    gainAttack_ = GAIN_ATTACK;
    gainRelease_ = GAIN_RELEASE;
    procTime_ = PROC_TIME;
    offset_ = 0;
    // todo dump pcm
    AUDIO_INFO_LOG("AudioLimiter");
}

AudioLimiter::~AudioLimiter()
{
    // todo dump pcm
    ReleaseBuffer();
    AUDIO_INFO_LOG("~AudioLimiter");
}

void AudioLimiter::ReleaseBuffer()
{
    if (bufHis != nullptr) {
        delete[] bufHis;
        bufHis = nullptr;
    }
    return;
}

int32_t AudioLimiter::SetConfig(int sampleRate, int channels)
{
    algoFrameLen_ = sampleRate * channels * procTime_;
    inOffset_ = 0;
    outOffset_ = algoFrameLen_;
    bufHis = new (std::nothrow) float[algoFrameLen_]();
    if (bufHis == nullptr) {
        AUDIO_ERR_LOG("allocate limit algorithm buffer failed");
    }
    integrationBufIn = new (std::nothrow) float[algoFrameLen_]();
    if (integrationBufIn == nullptr) {
        AUDIO_ERR_LOG("allocate integration buffer failed");
    }
    integratinBufOut = new (std::nothrow) float[algoFrameLen_]();
    if (integratinBufOut == nullptr) {
        AUDIO_ERR_LOG("allocate integration buffer failed");
    }
}

int32_t AudioLimiter::Process(int32_t frameLen, float *inBuffer, float *outBuffer)
{
    int32_t ptrIn = 0;
    int32_t ptrOut = 0;
    // method 1 考虑拼帧
    // preprocess
    memcpy_s(outBuffer, frameLen * sizeof(float), integratinBufOut + algoFrameLen_ - outOffset_, outOffset_ * sizeof(float));
    ptrOut = outOffset_;
    memcpy_s(integrationBufIn + inOffset_, (algoFrameLen_ - inOffset_) * sizeof(float), inBuffer, (algoFrameLen_ - inOffset_) * sizeof(float));
    ptrIn = algoFrameLen_ - inOffset_;
    processAlgo(integrationBufIn, outBuffer + ptrOut);
    ptrOut += algoFrameLen_;
    // process
    while (frameLen - ptrOut >= algoFrameLen_) {
        processAlgo(inBuffer + ptrIn, outBuffer + ptrOut);
        ptrIn += algoFrameLen_;
        ptrOut += algoFrameLen_;
    }
    // postprocess
    processAlgo(inBuffer + ptrIn, integratinBufOut);
    ptrIn += algoFrameLen_;
    memcpy_s(integrationBufIn, algoFrameLen_ * sizeof(float), inBuffer + ptrIn, (frameLen - prtIn) * sizeof(float));
    inOffset_ = frameLen - ptrIn;
    memcpy_s(outBuffer + ptrOut, (frameLen - ptrOut) * sizeof(float), integratinBufOut, (frameLen - ptrOut) * sizeof(float));
    outOffset_ = algoFrameLen_ - (frameLen - ptrOut);

    // method 2 不考虑拼帧
    while (frameLen - ptrOut >= algoFrameLen_) {
        processAlgo(inBuffer + ptrIn, outBuffer + ptrOut);
        ptrIn += algoFrameLen_;
        ptrOut += algoFrameLen_;
    }
}

int32_t AudioLimiter::ProcessAlgo(float *inBuffer, float *outBuffer) {
    // calculate envelope energy
    float maxEnvelopeLevel = 0.0f;
    for (int32_t i = 0; i < algoFrameLen_; i += 2) {    // for 2 channel
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
    targetGain = tempMaxLevel > threshold_ ? threshold_ / tempMaxLevel, targetGain;
    float lastGain = gain_;
    float coeff = gain_ > targetGain ? gainAttack_ : gainRelease_;
    float gain_ = coeff * gain_ + (1 - coeff) * targetGain;
    float deltaGain = (gain_ - lastGain) / algoFrameLen_;

    // apply gain
    for (int32_t i = 0; i < algoFrameLen_; i += 2) {    // for 2 channel
        lastGain += deltaGain;
        outBuffer[i] = bufHis[i] * lastGain;
        outBuffer[i + 1] = bufHis[i + 1] * lastGain;
        bufHis[i] = inBuffer[i];
        bufHis[i + 1] = inBuffer[i + 1];
    }
}
} // namespace AudioStandard
} // namespace OHOS
