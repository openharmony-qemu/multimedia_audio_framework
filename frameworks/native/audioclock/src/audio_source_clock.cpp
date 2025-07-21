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
#define LOG_TAG "AudioSourceClock"
#endif

#include "audio_source_clock.h"
#include <limits>
#include <cinttypes>
#include "audio_hdi_log.h"
#include "audio_utils.h"
#include "capturer_clock_manager.h"

namespace OHOS {
namespace AudioStandard {

enum AudioByteSize : int32_t {
    BYTE_SIZE_SAMPLE_U8 = 1,
    BYTE_SIZE_SAMPLE_S16 = 2,
    BYTE_SIZE_SAMPLE_S24 = 3,
    BYTE_SIZE_SAMPLE_S32 = 4,
};

static int32_t GetByteSizeByFormat(AudioSampleFormat format)
{
    int32_t byteSize = 0;
    switch (format) {
        case SAMPLE_U8:
            byteSize = BYTE_SIZE_SAMPLE_U8;
            break;
        case SAMPLE_S16LE:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
        case SAMPLE_S24LE:
            byteSize = BYTE_SIZE_SAMPLE_S24;
            break;
        case SAMPLE_S32LE:
            byteSize = BYTE_SIZE_SAMPLE_S32;
            break;
        default:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
    }

    return byteSize;
}

void AudioSourceClock::Init(uint32_t sampleRate, AudioSampleFormat format, uint32_t channel)
{
    AUDIO_INFO_LOG("sampleRate:%{public}u format:%{public}d channel:%{public}u",
        sampleRate, static_cast<int32_t>(format), channel);
    std::lock_guard<std::mutex> lock(clockMtx_);
    sampleRate_ = sampleRate;
    sizePerPos_ = static_cast<uint32_t>(GetByteSizeByFormat(format)) * channel;
}

// can be override for differ method
uint64_t AudioSourceClock::GetTimestamp(uint32_t positionInc)
{
    int64_t timestamp = ClockTime::GetCurNano();
    CHECK_AND_RETURN_RET_LOG(timestamp > 0, 0, "GetCurNano fail!");
    return static_cast<uint64_t>(timestamp);
}

void AudioSourceClock::Renew(uint32_t posIncSize)
{
    CHECK_AND_RETURN_LOG(posIncSize != 0, "posIncSize is 0!");
    CHECK_AND_RETURN_LOG(sizePerPos_ != 0, "sizePerPos_ is 0!");

    std::lock_guard<std::mutex> lock(clockMtx_);

    uint32_t positionInc = posIncSize / sizePerPos_;
    uint64_t timestamp = GetTimestamp(positionInc);

    AUDIO_DEBUG_LOG("dataSize:%{public}u positionInc:%{public}u", posIncSize, positionInc);

    for (size_t i = 0; i < sessionIdList_.size(); i++) {
        if (sessionIdList_[i] == 0) {
            break;
        }
        AUDIO_DEBUG_LOG("SessionId %{public}u", sessionIdList_[i]);
        std::shared_ptr<CapturerClock> clock =
            CapturerClockManager::GetInstance().GetCapturerClock(sessionIdList_[i]);
        if (clock == nullptr) {
            AUDIO_INFO_LOG("SessionId[%{public}u] capturer clock == nullptr!", sessionIdList_[i]);
            continue;
        }
        clock->SetTimeStampByPosition(timestamp, sampleRate_, positionInc);
    }
}

void AudioSourceClock::UpdateSessionId(const std::vector<int32_t> &sessionIdList)
{
    std::lock_guard<std::mutex> lock(clockMtx_);
    sessionIdList_ = sessionIdList;
}

AudioCapturerSourceTsRecorder::~AudioCapturerSourceTsRecorder()
{
    CHECK_AND_RETURN(clock_ != nullptr, "clock_ is nullptr! fail to renew timestamp!");
    clock_->Renew(replyBytes_);
}

} // namespace AudioStandard
} // namespace OHOS
