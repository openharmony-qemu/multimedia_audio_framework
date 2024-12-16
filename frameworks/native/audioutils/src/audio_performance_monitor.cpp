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
#include "audio_performance_monitor_c.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

AudioPerformanceMonitor& AudioPerformanceMonitor::GetInstance()
{
    static AudioPerformanceMonitor mgr;
    return mgr;
}

int32_t AudioPerformanceMonitor::DeleteSinkTypeDetect(SinkType sinkType)
{
    CHECK_AND_RETURN_RET_LOG(sinkType >= SINKTYPE_PRIMARY && sinkType < SinkType::MAX_SINK_TYPE, ERR_INVALID_PARAM,
        "invalid sinkType: %{public}d", sinkType);
    if (overTimeDetectMap_.find(sinkType) == overTimeDetectMap_.end()) {
        AUDIO_WARNING_LOG("cursinkType %{public}d not find in detectMap!", sinkType);
        return ERROR;
    }

    overTimeDetectMap_.erase(sinkType);
    return SUCCESS;
}

void AudioPerformanceMonitor::RecordSlienceState(uint32_t performMonitorIndex, bool isSilence)
{
    CHECK_AND_RETURN_LOG(laggyDetectMap_.find(performMonitorIndex) != laggyDetectMap_.end(),
        "performMonitorIndex %{public}d not find in detectMap!", performMonitorIndex);
    JudgeIfNeedReportLaggyEvent(performMonitorIndex, isSilence);
}

void AudioPerformanceMonitor::RecordLastWrittenTime(uint32_t streamId, int64_t lastWrittenTime)
{
    static int64_t recordWrittenTime = 0;
    CHECK_AND_RETURN_LOG(laggyDetectMap_.find(streamId) != laggyDetectMap_.end(),
        "streamId %{public}d not find in detectMap!", streamId);
    JudgeIfNeedReportLaggyEvent(streamId, recordWrittenTime == lastWrittenTime);
    recordWrittenTime = lastWrittenTime;
}

void AudioPerformanceMonitor::RecordTimeStamp(SinkType sinkType, uint64_t curTimeStamp)
{
    CHECK_AND_RETURN_LOG(sinkType >= SinkType::SINKTYPE_PRIMARY && sinkType < SinkType::MAX_SINK_TYPE,
        "invalid sinkType: %{public}d", sinkType);
    if (overTimeDetectMap_.find(sinkType) == overTimeDetectMap_.end()) {
        AUDIO_INFO_LOG("AudioSinkType %{public}d write data first time!", sinkType);
    } else {
        if (curTimeStamp - overTimeDetectMap_[sinkType] > MAX_WRITE_INTERVAL[sinkType]) {
            AUDIO_WARNING_LOG("SinkType %{public}d write time interval %{public}" PRIu64 " ns! overTime!",
                sinkType, (curTimeStamp - overTimeDetectMap_[sinkType]));
            ReportEvent();
        }
    }
    overTimeDetectMap_[sinkType] = curTimeStamp;
}


void AudioPerformanceMonitor::JudgeIfNeedReportLaggyEvent(uint32_t index, bool inValidState)
{
    if (inValidState) {
        if (MIN_NOT_SILENCE_VALUE <= laggyDetectMap_[index].notSilenceCount &&
            laggyDetectMap_[index].notSilenceCount <= MAX_NOT_SILENCE_VALUE) {
            //凸
            ReportEvent();
        }
        laggyDetectMap_[index].notSilenceCount = 0;
        
        if (laggyDetectMap_[index].silenceCount + 1 == UINT64_MAX) {
            laggyDetectMap_[index].silenceCount = MAX_SILENCE_VALUE + 1;
        } else {
            laggyDetectMap_[index].silenceCount++;
        }
    } else {
        if (MIN_SILENCE_VALUE <= laggyDetectMap_[index].silenceCount &&
            laggyDetectMap_[index].silenceCount <= MAX_SILENCE_VALUE) {
            //凹
            ReportEvent();
        }
        laggyDetectMap_[index].silenceCount = 0;

        if (laggyDetectMap_[index].notSilenceCount + 1 == UINT64_MAX) {
            laggyDetectMap_[index].notSilenceCount = MAX_NOT_SILENCE_VALUE + 1;
        } else {
            laggyDetectMap_[index].notSilenceCount++;
        }
    }
}

void AudioPerformanceMonitor::ReportEvent() {}

} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

static uint32_t monitorIndex = 0;
const uint32_t MAX_MONITOR_INDEX = 1024;

void CreatePerformanceMonitor(void *userdata)
{
    std::map<uint32_t, FrameRecord> &recordMap = AudioPerformanceMonitor::GetInstance().laggyDetectMap_;
    CHECK_AND_RETURN_LOG(recordMap.find(monitorIndex) == recordMap.end(),
        "CreatePerformanceMonitor fail! monitorIndex already exist!");
    recordMap[monitorIndex] = FrameRecord;
    userdata->performMonitorIndex = monitorIndex++;
    monitorIndex = monitorIndex % MAX_MONITOR_INDEX;
}

void DeletePerformanceMonitor(void *userdata)
{
    std::map<uint32_t, FrameRecord> &recordMap = AudioPerformanceMonitor::GetInstance().laggyDetectMap_;
    CHECK_AND_RETURN_LOG(recordMap.find(userdata->performMonitorIndex) != recordMap.end(),
        "DeletePerformanceMonitor fail! monitorIndex not in record map!");
    recordMap.erase(userdata->performMonitorIndex);
}

void RecordPaSlienceState(void *userdata, bool isSilence)
{
    AudioPerformanceMonitor::GetInstance().RecordSlienceState(userdata->performMonitorIndex, isSilence);
}

#ifdef __cplusplus
}
#endif
