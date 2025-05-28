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
#define LOG_TAG "CapturerClockManager"
#endif

#include "capturer_clock_manager.h"
#include <cinttypes>
#include "audio_hdi_log.h"

namespace OHOS {
namespace AudioStandard {

static CapturerClockManager g_captureClockMgrSingleton;

CapturerClockManager &CapturerClockManager::GetInstance(void)
{
    return g_captureClockMgrSingleton;
}

std::shared_ptr<CapturerClock> CapturerClockManager::CreateCapturerClock(
    uint32_t sessionId, uint32_t sampleRate)
{
    std::lock_guard<std::mutex> lock(clockPoolMtx_);
    CHECK_AND_RETURN_RET_LOG(capturerClockPool_.find(sessionId) == capturerClockPool_.end(), nullptr,
        "fail, [%{public}u] is existed!", sessionId);

    std::shared_ptr<CapturerClock> clock = std::make_shared<CapturerClock>(sampleRate);
    capturerClockPool_[sessionId] = clock;
    return clock;
}

void CapturerClockManager::DeleteCapturerClock(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(clockPoolMtx_);
    capturerClockPool_.erase(sessionId);
}

std::shared_ptr<CapturerClock> CapturerClockManager::GetCapturerClock(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(clockPoolMtx_);
    CHECK_AND_RETURN_RET_LOG(capturerClockPool_.find(sessionId) != capturerClockPool_.end(), nullptr,
        "fail, [%{public}u] is not existed!", sessionId);

    return capturerClockPool_[sessionId];
}

bool CapturerClockManager::RegisterAudioSourceClock(uint32_t captureId, std::shared_ptr<AudioSourceClock> clock)
{
    std::lock_guard<std::mutex> lock(clockPoolMtx_);
    CHECK_AND_RETURN_RET_LOG(audioSrcClockPool_.find(captureId) == audioSrcClockPool_.end(), false,
        "fail, [%{public}u] is existed!", captureId);

    audioSrcClockPool_[captureId] = clock;
    return true;
}

void CapturerClockManager::DeleteAudioSourceClock(uint32_t captureId)
{
    std::lock_guard<std::mutex> lock(clockPoolMtx_);
    audioSrcClockPool_.erase(captureId);
}

std::shared_ptr<AudioSourceClock> CapturerClockManager::GetAudioSourceClock(uint32_t captureId)
{
    std::lock_guard<std::mutex> lock(clockPoolMtx_);
    CHECK_AND_RETURN_RET_LOG(audioSrcClockPool_.find(captureId) != audioSrcClockPool_.end(), nullptr,
        "fail, [%{public}u] is not existed!", captureId);

    return audioSrcClockPool_[captureId];
}

} // namespace AudioStandard
} // namespace OHOS
