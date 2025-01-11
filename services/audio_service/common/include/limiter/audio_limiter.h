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

#ifndef AUDIO_LIMITER_H
#define AUDIO_LIMITER_H

#include <cstdio>
#include <cstdint>
#include <string>
namespace OHOS {
namespace AudioStandard {

class AudioLimiter {
public:
    AudioLimiter(int32_t sinkIndex);
    ~AudioLimiter();
    int32_t SetConfig(int32_t maxRequest, int32_t biteSize, int32_t sampleRate, int32_t channels);
    int32_t Process(int32_t frameLen, float *inBuffer, float *outBuffer);
    uint32_t GetLatency();
private:
    void ProcessAlgo(int algoFrameLen, float *inBuffer, float *outBuffer);
    void ReleaseBuffer();
    uint32_t latency_;
    int32_t sinkIndex_;
    int32_t algoFrameLen_;
    int32_t format_;
    float nextLev_;
    float curMaxLev_;
    float threshold_;
    float gain_;
    float levelAttack_;
    float levelRelease_;
    float gainAttack_;
    float gainRelease_;
    float *bufHis_;
    FILE *dumpFileInput_ = nullptr;
    FILE *dumpFileOutput_ = nullptr;
    std::string dumpFileNameIn_ = "";
    std::string dumpFileNameOut_ = "";
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_LIMITER_H