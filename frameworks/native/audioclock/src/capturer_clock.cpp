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
#define LOG_TAG "CapturerClock"
#endif

#include "capturer_clock.h"
#include <limits>
#include <cinttypes>
#include "audio_hdi_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

CapturerClock::CapturerClock(uint32_t capturerSampleRate)
    : capturerSampleRate_(capturerSampleRate)
{
    AUDIO_INFO_LOG("Created, capturer sampling rate:%{public}u", capturerSampleRate);
}

bool CapturerClock::GetTimeStampByPosition(uint64_t capturerPos, uint64_t& timestamp)
{
    std::lock_guard<std::mutex> lock(clockMtx_);
    CHECK_AND_RETURN_RET(timestamp_ != 0, false);
    CHECK_AND_RETURN_RET_LOG(capturerSampleRate_ != 0, false, "capturerSampleRate_ is 0!");

    AUDIO_DEBUG_LOG("capturerPos:%{public}" PRIu64 " position_:%{public}" PRIu64,
        capturerPos, position_);

    if (capturerPos == position_) {
        timestamp = timestamp_;
        AUDIO_DEBUG_LOG("timestamp:%{public}" PRIu64, timestamp);
        return true;
    }

    // If the requested position does not equal the clock position,
    // the timestamp to returned is estimated using the clock position.
    uint64_t posDetla = 0;
    uint64_t tsDetla = 0;
    if (position_ > capturerPos) {
        posDetla = position_ - capturerPos;
        tsDetla = posDetla * AUDIO_NS_PER_SECOND / capturerSampleRate_;
        timestamp = timestamp_ - tsDetla;
    } else {
        posDetla = capturerPos - position_;
        tsDetla = posDetla * AUDIO_NS_PER_SECOND / capturerSampleRate_;
        timestamp = timestamp_ + tsDetla;
    }
    AUDIO_DEBUG_LOG("timestamp:%{public}" PRIu64 "posDetla:%{public}" PRIu64
        " tsDetla:%{public}" PRIu64, timestamp, posDetla, tsDetla);
    return true;
}

void CapturerClock::SetTimeStampByPosition(
    uint64_t timestamp, uint32_t srcSampleRate, uint64_t posIncSize)
{
    AUDIO_DEBUG_LOG("timestamp:%{public}" PRIu64 " srcSr:%{public}u posIncSize:%{public}" PRIu64,
        timestamp, srcSampleRate, posIncSize);

    std::lock_guard<std::mutex> lock(clockMtx_);
    CHECK_AND_RETURN_LOG(isRunning_, "clock is pause! stop update!");

    // When source sampling rate is different from sampling rate required by the upper-level session,
    // it is necessary to convert source position increase into upper-level position increase.
    if (srcSampleRate != capturerSampleRate_) {
        double ratio = static_cast<double>(capturerSampleRate_) / srcSampleRate;
        posIncSize = posIncSize * ratio;
    }

    // The updating position rule :
    // 1. Add the previous position increase (previous buffer size) to the current position.
    // 2. The size of the audio buffer uploaded this time is saved for adding to the next time.
    position_ += lastPosInc_;
    lastPosInc_ = posIncSize;
    timestamp_ = timestamp;
}

void CapturerClock::Start()
{
    AUDIO_INFO_LOG("CapturerClock is start!");

    std::lock_guard<std::mutex> lock(clockMtx_);
    isRunning_ = true;
}

void CapturerClock::Stop()
{
    AUDIO_INFO_LOG("CapturerClock is Stop!");

    std::lock_guard<std::mutex> lock(clockMtx_);
    isRunning_ = false;
}

} // namespace AudioStandard
} // namespace OHOS
