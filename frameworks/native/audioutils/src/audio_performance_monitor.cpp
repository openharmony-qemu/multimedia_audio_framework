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
#define LOG_TAG "AudioPerformanceMonitor"
#endif

#include "audio_performance_monitor.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

AudioPerformDetect& AudioPerformDetect::GetInstance()
{
    static AudioPerformDetect mgr;
    return mgr;
}

int32_t AudioPerformDetect::DeleteStreamDetect(uint32_t streamId)
{
    if (laggyDetectMap_.find(streamId) == laggyDetectMap_.end()) {
        AUDIO_WARNING_LOG("curStreamId %{public}d not find in detectMap!", streamId);
        return ERROR;
    }

    laggyDetectMap_.erase(streamId);
    return SUCCESS;
}

int32_t AudioPerformDetect::DeleteSinkTypeDetect(SinkType sinkType)
{
    CHECK_AND_RETURN_RET_LOG(sinkType >= SinkType::PRIMARY && sinkType < SinkType::MAX_SINK_TYPE, ERR_INVALID_PARAM,
        "invalid sinkType: %{public}d", sinkType);
    if (overTimeDetectMap_.find(sinkType) == overTimeDetectMap_.end()) {
        AUDIO_WARNING_LOG("cursinkType %{public}d not find in detectMap!", sinkType);
        return ERROR;
    }

    overTimeDetectMap_.erase(sinkType);
    return SUCCESS;
}

void AudioPerformDetect::RecordFrameState(uint32_t streamId, uint8_t *buffer, size_t bufferSize)
{
    CHECK_AND_RETURN_LOG(bufferSize > 0, "bufferSize: %{public}zu is invalid", bufferSize);
    if (laggyDetectMap_.find(streamId) == laggyDetectMap_.end()) {
        AUDIO_INFO_LOG("curStreamId %{public}d start detect!", streamId);
        laggyDetectMap_[streamId] = MAX_NOT_SILENCE_VALUE + 1;
    }

    uint8_t *bufferPtr = buffer;
    for (size_t i = 0; i < bufferSize; ++i) {
        if (bufferPtr[i] != '0') {
            DetectFrameLaggy(streamId, false);
            return;
        }
    }
    DetectFrameLaggy(streamId, true);
}

void AudioPerformDetect::RecordTimeStamp(SinkType sinkType, uint64_t curTimeStamp)
{
    CHECK_AND_RETURN_LOG(sinkType >= SinkType::PRIMARY && sinkType < SinkType::MAX_SINK_TYPE,
        "invalid sinkType: %{public}d", sinkType);
    DetectHalWriteOverTime(sinkType, curTimeStamp);
    overTimeDetectMap_[sinkType] = curTimeStamp;
}

void AudioPerformDetect::DetectFrameLaggy(uint32_t streamId, bool isSilence)
{
    CHECK_AND_RETURN_LOG(laggyDetectMap_.find(streamId) != laggyDetectMap_.end(),
        "curStreamId %{public}d not find in detectMap!", streamId);
    if (isSilence) {
        if (MIN_NOT_SILENCE_VALUE <= laggyDetectMap_[streamId] &&
            laggyDetectMap_[streamId] <= MAX_NOT_SILENCE_VALUE) {
            ReportEvent();
        }
        laggyDetectMap_[streamId] = 0;
    } else {
        if (laggyDetectMap_[streamId] + 1 == UINT64_MAX) {
            laggyDetectMap_[streamId] = MAX_NOT_SILENCE_VALUE + 1;
        } else {
            laggyDetectMap_[streamId]++;
        }
    }
}

void AudioPerformDetect::DetectHalWriteOverTime(SinkType sinkType, uint64_t curTimeStamp)
{
    if (overTimeDetectMap_.find(sinkType) == overTimeDetectMap_.end()) {
        AUDIO_INFO_LOG("AudioSinkType %{public}d write data first time!", sinkType);
    } else {
        if (curTimeStamp - overTimeDetectMap_[sinkType] > MAX_WRITE_INTERVAL[sinkType]) {
            AUDIO_WARNING_LOG("SinkType %{public}d write time interval %{public}" PRIu64 "overTime!",
                sinkType, (curTimeStamp - overTimeDetectMap_[sinkType]) / AUDIO_US_PER_SECOND);
            ReportEvent();
        }
    }
}

void AudioPerformDetect::ReportEvent() {}

} // namespace AudioStandard
} // namespace OHOS