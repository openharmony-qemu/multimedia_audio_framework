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


#ifndef AUDIO_LIMITER_MANAGER_H
#define AUDIO_LIMITER_MANAGER_H

#include <map>
#include <mutex>

#include "audio_limiter.h"
namespace OHOS {
namespace AudioStandard {

class AudioLmtManager {
public:
    AudioLmtManager();
    ~AudioLmtManager();
    static  AudioLmtManager* GetInstance();
    int32_t CreateLimiter(int32_t sinkIndex);
    int32_t SetLimiterConfig(int32_t sinkIndex, int32_t maxRequest, int32_t biteSize,
        int32_t sampleRate, int32_t channels);
    int32_t ProcessLimiter(int32_t sinkIndex, int32_t frameLen, float *inBuffer, float *outBuffer);
    int32_t ReleaseLimiter(int32_t sinkIndex);
    uint32_t GetLatency(int32_t sinkIndex);
private:
    std::map<int32_t, std::shared_ptr<AudioLimiter>> sinkIndexToLimiterMap_;
    std::mutex limiterMutex_;
};

}   // namespace AudioStandard
}   // namespace OHOS
#endif // AUDIO_LIMITER_MANAGER_H