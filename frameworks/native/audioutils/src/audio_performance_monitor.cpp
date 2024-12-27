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
#include <memory>
#include <string>
#include "audio_errors.h"
#include "media_monitor_manager.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {

AudioPerformanceMonitor& AudioPerformanceMonitor::GetInstance()
{
    static AudioPerformanceMonitor mgr;
    return mgr;
}

void AudioPerformanceMonitor::DeleteOvertimeMonitor(SinkType sinkType)
{
    CHECK_AND_RETURN_LOG(overTimeDetectMap_.find(sinkType) != overTimeDetectMap_.end(),
        "invalid sinkType: %{public}d", sinkType);
    AUDIO_INFO_LOG("delete sinkType %{public}d overTime Monitor!", sinkType);
    overTimeDetectMap_.erase(sinkType);
}

void AudioPerformanceMonitor::DeleteSilenceMonitor(uint32_t sessionId)
{
    CHECK_AND_RETURN_LOG(silenceDetectMap_.find(sessionId) != silenceDetectMap_.end(),
        "invalid sessionId: %{public}d", sessionId);
    AUDIO_INFO_LOG("delete sessionId %{public}d silence Monitor!", sessionId);
    silenceDetectMap_.erase(sessionId);
}

void AudioPerformanceMonitor::ClearSilenceMonitor(uint32_t sessionId)
{
    if (silenceDetectMap_.find(sessionId) == silenceDetectMap_.end()) {
        return;
    }
    silenceDetectMap_[sessionId] = FrameRecordInfo();
}

void AudioPerformanceMonitor::RecordSilenceState(uint32_t sessionId, bool isSilence)
{
    if (silenceDetectMap_.find(sessionId) == silenceDetectMap_.end()) {
        AUDIO_INFO_LOG("start record silence state of sessionId : %{public}d", sessionId);
        silenceDetectMap_[sessionId] = FrameRecordInfo();
    }
    silenceDetectMap_[sessionId].historyStateQueue.push(isSilence);
    if (silenceDetectMap_[sessionId].historyStateQueue.size() > MAX_RECORD_QUEUE_SIZE) {
        silenceDetectMap_[sessionId].historyStateQueue.pop();
    }
    JudgeNoise(sessionId, isSilence);
}

void AudioPerformanceMonitor::RecordTimeStamp(SinkType sinkType, int64_t curTimeStamp)
{
    CHECK_AND_RETURN_LOG(sinkType >= SinkType::SINKTYPE_PRIMARY && sinkType < SinkType::MAX_SINK_TYPE,
        "invalid sinkType: %{public}d", sinkType);
    if (overTimeDetectMap_.find(sinkType) == overTimeDetectMap_.end()) {
        AUDIO_INFO_LOG("start record sinkType: %{public}d", sinkType);
        overTimeDetectMap_[sinkType] = curTimeStamp;
        return;
    }

    // init lastwritten time when start or resume to avoid overtime
    if (curTimeStamp == INIT_LASTWRITTEN_TIME || overTimeDetectMap_[sinkType] == INIT_LASTWRITTEN_TIME) {
        overTimeDetectMap_[sinkType] = curTimeStamp;
        return;
    } 

    if (curTimeStamp - overTimeDetectMap_[sinkType] > MAX_WRITTEN_INTERVAL[sinkType]) {
        AUDIO_WARNING_LOG("SinkType %{public}d write time interval %{public}" PRId64 " ns! overTime!",
            sinkType, curTimeStamp - overTimeDetectMap_[sinkType]);
        ReportEvent(OVERTIME_EVENT);
    }
    overTimeDetectMap_[sinkType] = curTimeStamp;
}

// we use silenceStateCount to record the silence frames bewteen two not silence frame
void AudioPerformanceMonitor::JudgeNoise(uint32_t sessionId, bool isSilence)
{
    if (isSilence) {
        silenceDetectMap_[sessionId].silenceStateCount++;
        silenceDetectMap_[sessionId].notSilenceStateCount = 0;
    } else {
        // we init the count value as the maxValue+1 to make it as normal state
        if (MIN_SILENCE_VALUE <= silenceDetectMap_[sessionId].silenceStateCount &&
            silenceDetectMap_[sessionId].silenceStateCount <= MAX_SILENCE_VALUE) {
            std::string printStr{};
            //for example: not Silent-> not Silent -> silent -> not Silent -> silent, will print "--_-_"
            while (silenceDetectMap_[sessionId].historyStateQueue.size() != 0) {
                printStr += silenceDetectMap_[sessionId].historyStateQueue.front() ? "_" : "-";
                silenceDetectMap_[sessionId].historyStateQueue.pop();
            }
            AUDIO_WARNING_LOG("record %{public}d state for last %{public}zu times: %{public}s",
                sessionId, MAX_RECORD_QUEUE_SIZE, printStr.c_str());
            ReportEvent(SILENCE_EVENT);
            silenceDetectMap_[sessionId] = FrameRecordInfo();
            return;
        }
        silenceDetectMap_[sessionId].silenceStateCount = 0;
        silenceDetectMap_[sessionId].notSilenceStateCount++;
    }
}

void AudioPerformanceMonitor::ReportEvent(int32_t reasonCode)
{
    AUDIO_INFO_LOG("report reasonCode %{public}d", reasonCode);
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::EventId::JANK_PLAYBACK,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("REASON", reasonCode);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

void RecordPaSilenceState(uint32_t sessionId, bool isSilence)
{
    AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, isSilence);
}

#ifdef __cplusplus
}
#endif
