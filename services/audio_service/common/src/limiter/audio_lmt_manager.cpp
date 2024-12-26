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
#define LOG_TAG "AudioLmtManager"
#endif

#include "audio_errors.h"
#include "audio_lmt_manager.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
AudioLmtManager::AudioLmtManager()
{
    sinkNameToLimiterMap_.clear();
    AUDIO_INFO_LOG("AudioLmtManager");
}

AudioLmtManager::~AudioLmtManager()
{
    AUDIO_INFO_LOG("~AudioLmtManager");
}

AudioLmtManager *AudioLmtManager::GetInstance()
{
    static AudioLmtManager instance;
    return &instance;
}

int32_t AudioLmtManager::CreateLimiter(int32_t sinkNameCode)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    if (sinkNameToLimiterMap_.find(sinkNameCode) != sinkNameToLimiterMap_.end() &&
        sinkNameToLimiterMap_[sinkNameCode] != nullptr) {
        AUDIO_INFO_LOG("The limiter has been created, sinkNameCode = %{public}d", sinkNameCode);
        return SUCCESS;
    }

    std::shared_ptr<AudioLimiter> limiter = std::make_shared<AudioLimiter>(sinkNameCode);

    if (limiter == nullptr) {
        AUDIO_INFO_LOG("Failed to create limiter, sinkNameCode = %{public}d", sinkNameCode);
        return ERROR;
    }

    sinkNameToLimiterMap_[sinkNameCode] = limiter;
    AUDIO_INFO_LOG("Create limiter success, sinkNameCode = %{public}d", sinkNameCode);
    return SUCCESS;
}

int32_t AudioLmtManager::SetLimiterConfig(int32_t sinkNameCode, int sampleRate, int channels)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    auto iter = sinkNameToLimiterMap_.find(sinkNameCode);
    if (iter == sinkNameToLimiterMap_.end()) {
        AUDIO_INFO_LOG("The limiter has not been created, sinkNameCode = %{public}d", sinkNameCode);
        return ERROR;
    }

    std::shared_ptr<AudioLimiter> limiter = iter->second;
    if (limiter == nullptr) {
        AUDIO_INFO_LOG("The limiter is nullptr, sinkNameCode = %{public}d", sinkNameCode);
        return ERROR;
    }

    return limiter->SetConfig(sampleRate, channels);
}

int32_t AudioLmtManager::ProcessLimiter(int32_t sinkNameCode, int32_t frameLen, float *inBuffer, float *outBuffer)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    if (inBuffer == nullptr || outBuffer == nullptr) {
        AUDIO_ERR_LOG("inBuffer or outBuffer is nullptr");
        return ERROR;
    }

    auto iter = sinkNameToLimiterMap_.find(sinkNameCode);
    if (iter == sinkNameToLimiterMap_.end()) {
        AUDIO_INFO_LOG("The limiter has not been created, sinkNameCode = %{public}d", sinkNameCode);
        return ERROR;
    }

    std::shared_ptr<AudioLimiter> limiter = iter->second;
    if (limiter == nullptr) {
        AUDIO_INFO_LOG("The limiter is nullptr, sinkNameCode = %{public}d", sinkNameCode);
        return ERROR;
    }

    return limiter->Process(frameLen, inBuffer, outBuffer);
}

int32_t AudioLmtManager::ReleaseLimiter(int32_t sinkNameCode)
{
    std::lock_guard<std::mutex> lock(limiterMutex_);
    auto iter = sinkNameToLimiterMap_.find(sinkNameCode);
    if (iter == sinkNameToLimiterMap_.end()) {
        AUDIO_INFO_LOG("The limiter has not been created, sinkNameCode = %{public}d", sinkNameCode);
        return ERROR;
    }

    sinkNameToLimiterMap_.erase(iter);
    AUDIO_INFO_LOG("Release limiter success, sinkNameCode = %{public}d", sinkNameCode);
    return SUCCESS;
}
}   // namespace AudioStandard
}   // namespace OHOS