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
#define LOG_TAG "AudioLimiterManager"
#endif

#include "audio_errors.h"
#include "audio_limiter_manager.h"
#include "audio_common_log.h"

#include "securec.h"
namespace OHOS {
namespace AudioStandard {
AudioLmtManager::AudioLmtManager()
{
    sinkIndexToLimiterMap_.clear();
    AUDIO_INFO_LOG("AudioLmtManager");
}

AudioLmtManager::~AudioLmtManager()
{
    AUDIO_INFO_LOG("~AudioLmtManager");
}

AudioLmtManager* AudioLmtManager::GetInstance()
{
    static AudioLmtManager instance;
    return &instance;
}

int32_t AudioLmtManager::CreateLimiter(int32_t sinkIndex)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    if (sinkIndexToLimiterMap_.find(sinkIndex) != sinkIndexToLimiterMap_.end() &&
        sinkIndexToLimiterMap_[sinkIndex] != nullptr) {
        AUDIO_INFO_LOG("The limiter has been created, sinkIndex = %{public}d", sinkIndex);
        return SUCCESS;
    }

    std::shared_ptr<AudioLimiter> limiter = std::make_shared<AudioLimiter>(sinkIndex);

    CHECK_AND_RETURN_RET_LOG(limiter != nullptr, ERROR,
        "Failed to create limiter, sinkIndex = %{public}d", sinkIndex);

    sinkIndexToLimiterMap_[sinkIndex] = limiter;
    AUDIO_INFO_LOG("Create limiter success, sinkIndex = %{public}d", sinkIndex);
    return SUCCESS;
}

int32_t AudioLmtManager::SetLimiterConfig(int32_t sinkIndex, int32_t maxRequest, int32_t biteSize,
    int32_t sampleRate, int32_t channels)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    auto iter = sinkIndexToLimiterMap_.find(sinkIndex);
    CHECK_AND_RETURN_RET_LOG(iter != sinkIndexToLimiterMap_.end(), ERROR,
        "The limiter has not been created, sinkIndex = %{public}d", sinkIndex);

    std::shared_ptr<AudioLimiter> limiter = iter->second;
    CHECK_AND_RETURN_RET_LOG(limiter != nullptr, ERROR,
        "The limiter is nullptr, sinkIndex = %{public}d", sinkIndex);

    return limiter->SetConfig(maxRequest, biteSize, sampleRate, channels);
}

int32_t AudioLmtManager::ProcessLimiter(int32_t sinkIndex, int32_t frameLen, float *inBuffer, float *outBuffer)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    CHECK_AND_RETURN_RET_LOG(inBuffer != nullptr && outBuffer != nullptr, ERROR, "inBuffer or outBuffer is nullptr");

    auto iter = sinkIndexToLimiterMap_.find(sinkIndex);
    if (iter == sinkIndexToLimiterMap_.end()) {
        AUDIO_INFO_LOG("The limiter has not been created, sinkIndex = %{public}d", sinkIndex);
        CHECK_AND_RETURN_RET_LOG(memcpy_s(outBuffer, frameLen * sizeof(float), inBuffer, frameLen * sizeof(float)) == 0,
            ERROR, "memcpy_s failed");
        return ERROR;
    }

    std::shared_ptr<AudioLimiter> limiter = iter->second;
    if (limiter == nullptr) {
        AUDIO_INFO_LOG("The limiter is nullptr, sinkIndex = %{public}d", sinkIndex);
        CHECK_AND_RETURN_RET_LOG(memcpy_s(outBuffer, frameLen * sizeof(float), inBuffer, frameLen * sizeof(float)) == 0,
            ERROR, "memcpy_s failed");
        return ERROR;
    }

    int32_t ret = limiter->Process(frameLen, inBuffer, outBuffer);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Failed to process limiter, sinkIndex = %{public}d", sinkIndex);
        CHECK_AND_RETURN_RET_LOG(memcpy_s(outBuffer, frameLen * sizeof(float), inBuffer, frameLen * sizeof(float)) == 0,
            ERROR, "memcpy_s failed");
        return ERROR;
    }
    return SUCCESS;
}

int32_t AudioLmtManager::ReleaseLimiter(int32_t sinkIndex)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    auto iter = sinkIndexToLimiterMap_.find(sinkIndex);
    CHECK_AND_RETURN_RET_LOG(iter != sinkIndexToLimiterMap_.end(), ERROR,
        "The limiter has not been created, sinkIndex = %{public}d", sinkIndex);

    sinkIndexToLimiterMap_.erase(iter);
    AUDIO_INFO_LOG("Release limiter success, sinkIndex = %{public}d", sinkIndex);
    return SUCCESS;
}

uint32_t AudioLmtManager::GetLatency(int32_t sinkIndex)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    auto iter = sinkIndexToLimiterMap_.find(sinkIndex);
    CHECK_AND_RETURN_RET_LOG(iter != sinkIndexToLimiterMap_.end(), 0,
        "The limiter has not been created, sinkIndex = %{public}d", sinkIndex);

    std::shared_ptr<AudioLimiter> limiter = iter->second;
    CHECK_AND_RETURN_RET_LOG(limiter != nullptr, 0, "The limiter is nullptr, sinkIndex = %{public}d", sinkIndex);

    return limiter->GetLatency();
}
}   // namespace AudioStandard
}   // namespace OHOS