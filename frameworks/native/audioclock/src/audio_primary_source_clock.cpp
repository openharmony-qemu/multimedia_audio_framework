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
#define LOG_TAG "AudioPrimarySourceClock"
#endif

#include "audio_primary_source_clock.h"
#include <limits>
#include <cinttypes>
#include "audio_hdi_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

static constexpr uint32_t USING_FIRST_TS_FOR_TS_COUNTTING_MAX = 20;
static constexpr uint32_t REGULAR_DETLA_RATIO = 2;

/*
 * The timestamp for the first 20 frames can be obtained by accumulating the firstTimeStamp of the HDI layer.
 * After that,
 * the systemClock is used to obtain the timestamp when "the packet interval is normal" or "after first 20 frames".
 */
void AudioCapturerSourceClock::CheckAndResetTimestamp(uint64_t &timestamp, uint32_t positionInc)
{
    CHECK_AND_RETURN_LOG(sampleRate_ != 0, "sampleRate_ is zero!");
    if (isGetTimeStampFromSystemClock_) {
        return;
    }

    frameCnt_++;

    if (frameCnt_ == 1) {
        AUDIO_INFO_LOG("GetFirstTimeStampFromAlgo:%{public}" PRIu64, firstTimeStamp_);
    }

    if (frameCnt_ > USING_FIRST_TS_FOR_TS_COUNTTING_MAX || firstTimeStamp_ == 0) {
        AUDIO_ERR_LOG("frameCnt_ > MAX or ts is 0! Get timestamp from system Clock");
        isGetTimeStampFromSystemClock_ = true;
        return;
    }

    uint64_t tsDetla = timestamp - lastTs_;
    lastTs_ = timestamp;
    uint64_t regularTsDetla = positionInc * AUDIO_NS_PER_SECOND / sampleRate_;
    AUDIO_INFO_LOG("tsDetla:%{public}" PRIu64 " regularTsDetla:%{public}" PRIu64, tsDetla, regularTsDetla);
    if (tsDetla > (regularTsDetla / REGULAR_DETLA_RATIO) && tsDetla < (regularTsDetla * REGULAR_DETLA_RATIO)) {
        AUDIO_INFO_LOG("tsDetla good! Get timestamp from system Clock");
        isGetTimeStampFromSystemClock_ = true;
        return;
    }

    if (frameCnt_ != 1) {
        firstTimeStamp_ += regularTsDetla;
    }
    timestamp = firstTimeStamp_;

    AUDIO_INFO_LOG("frameCnt_:%{public}u timestamp:%{public}" PRIu64, frameCnt_, timestamp);
}

uint64_t AudioCapturerSourceClock::GetTimestamp(uint32_t positionInc)
{
    int64_t timestamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(timestamp > 0, 0, "GetCurNano fail!");
    uint64_t unsignedTimestamp = static_cast<uint64_t>(timestamp);

    CheckAndResetTimestamp(unsignedTimestamp, positionInc);
    return unsignedTimestamp;
}

uint32_t AudioCapturerSourceClock::GetFrameCnt() const
{
    return frameCnt_;
}

void AudioCapturerSourceClock::Reset()
{
    std::lock_guard<std::mutex> lock(clockMtx_);
    frameCnt_ = 0;
    firstTimeStamp_ = 0;
    lastTs_ = 0;
    isGetTimeStampFromSystemClock_ = false;
}

void AudioCapturerSourceClock::SetFirstTimestampFromHdi(uint64_t hdiTimestamp)
{
    AUDIO_INFO_LOG("hdiTimestamp:%{public}" PRIu64, hdiTimestamp);

    std::lock_guard<std::mutex> lock(clockMtx_);
    firstTimeStamp_ = hdiTimestamp;
}

} // namespace AudioStandard
} // namespace OHOS
