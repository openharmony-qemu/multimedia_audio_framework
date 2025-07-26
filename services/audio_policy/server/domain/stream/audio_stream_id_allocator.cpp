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
#define LOG_TAG "AudioStreamIdAllocator"
#endif

#include "audio_stream_id_allocator.h"

#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

namespace {
const uint32_t FIRST_SESSIONID = 100000;
constexpr uint32_t MAX_VALID_SESSIONID = UINT32_MAX - FIRST_SESSIONID;
static uint32_t g_sessionId = FIRST_SESSIONID; // begin from 100000
}

uint32_t AudioStreamIdAllocator::GenerateStreamId()
{
    std::lock_guard<std::mutex> lock(sessionIdAllocatorMutex_);
    uint32_t sessionId = g_sessionId++;
    if (g_sessionId > MAX_VALID_SESSIONID) {
        AUDIO_WARNING_LOG("sessionId is too large, reset it!");
        g_sessionId = FIRST_SESSIONID;
    }

    return sessionId;
}
} // namespace AudioStandard
} // namespace OHOS