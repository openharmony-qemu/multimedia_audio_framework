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
#define LOG_TAG "AudioLimiterAdapter"
#endif

#include "audio_errors.h"
#include "audio_limiter_adapter.h"
#include "audio_limiter_manager.h"
#include "audio_common_log.h"

using namespace OHOS::AudioStandard;

int32_t LimiterManagerCreate(int32_t sinkIndex)
{
    AudioLmtManager *audioLmtManager = AudioLmtManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioLmtManager != nullptr, ERROR, "Failed to get AudioLmtManager instance");
    return audioLmtManager->CreateLimiter(sinkIndex);
}

int32_t LimiterManagerSetConfig(int32_t sinkIndex, int32_t maxRequest, int32_t biteSize,
    int32_t sampleRate, int32_t channels)
{
    AudioLmtManager *audioLmtManager = AudioLmtManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioLmtManager != nullptr, ERROR, "Failed to get AudioLmtManager instance");
    return audioLmtManager->SetLimiterConfig(sinkIndex, maxRequest, biteSize, sampleRate, channels);
}

int32_t LimiterManagerProcess(int32_t sinkIndex, int32_t frameLen, float *inBuffer, float *outBuffer)
{
    AudioLmtManager *audioLmtManager = AudioLmtManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioLmtManager != nullptr, ERROR, "Failed to get AudioLmtManager instance");
    return audioLmtManager->ProcessLimiter(sinkIndex, frameLen, inBuffer, outBuffer);
}

int32_t LimiterManagerRelease(int32_t sinkIndex)
{
    AudioLmtManager *audioLmtManager = AudioLmtManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioLmtManager != nullptr, ERROR, "Failed to get AudioLmtManager instance");
    return audioLmtManager->ReleaseLimiter(sinkIndex);
}