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
    overTimeDetectMap_.erase(sinkType);
}

void AudioPerformanceMonitor::DeletejankMonitor(uint32_t sessionId)
{
    CHECK_AND_RETURN_LOG(jankDetectMap_.find(sessionId) != jankDetectMap_.end(),
        "invalid sessionId: %{public}d", sessionId);
    jankDetectMap_.erase(sessionId);
}

void AudioPerformanceMonitor::RecordSilenceState(uint32_t sessionId, bool isSilence)
{
    if (jankDetectMap_.find(sessionId) == jankDetectMap_.end()) {
        AUDIO_INFO_LOG("start record silence state of sessionId : %{public}d", sessionId);
        jankDetectMap_[sessionId] = FrameRecordInfo();
    }
        jankDetectMap_[sessionId].historyStateQueue.push(isSilence);
        if (jankDetectMap_[sessionId].historyStateQueue.size() > MAX_RECORD_QUEUE_SIZE) {
            jankDetectMap_[sessionId].historyStateQueue.pop();
        }
    JudgeNoise(sessionId, isSilence);
}

void AudioPerformanceMonitor::RecordLastWrittenTime(uint32_t sessionId, int64_t lastWrittenTime)
{
    if (jankDetectMap_.find(sessionId) == jankDetectMap_.end()) {
        AUDIO_INFO_LOG("start record last writtenTime of sessionId : %{public}d", sessionId);
        jankDetectMap_[sessionId] = FrameRecordInfo();
        jankDetectMap_[sessionId].lastWrittenTime = lastWrittenTime;
        return;
    }
    JudgeNoise(sessionId, jankDetectMap_[sessionId].lastWrittenTime != lastWrittenTime);
    jankDetectMap_[sessionId].lastWrittenTime = lastWrittenTime;
}

void AudioPerformanceMonitor::RecordTimeStamp(SinkType sinkType, uint64_t curTimeStamp)
{
    CHECK_AND_RETURN_LOG(sinkType >= SinkType::SINKTYPE_PRIMARY && sinkType < SinkType::MAX_SINK_TYPE,
        "invalid sinkType: %{public}d", sinkType);
    if (curTimeStamp == INIT_LASTWRITTEN_TIME) {
        overTimeDetectMap_[sinkType] = curTimeStamp;
        return;
    }
    if (overTimeDetectMap_.find(sinkType) == overTimeDetectMap_.end() ||
        overTimeDetectMap_[sinkType] == INIT_LASTWRITTEN_TIME) {
        AUDIO_INFO_LOG("AudioSinkType %{public}d write data first time", sinkType);
    } else {
        if (curTimeStamp - overTimeDetectMap_[sinkType] > MAX_WRITTEN_INTERVAL[sinkType]) {
            std::string printStr = "SinkType " + static_cast<uint32_t>(sinkType) + " write time interval " +
                curTimeStamp - overTimeDetectMap_[sinkType] + " ns! overTime!";
            ReportEvent(OVERTIME_EVENT, printStr);
        }
    }
    overTimeDetectMap_[sinkType] = curTimeStamp;
}


void AudioPerformanceMonitor::JudgeNoise(uint32_t sessionId, bool isValidData)
{
    if (isValidData) {
        if (MIN_INVALID_VALUE <= jankDetectMap_[sessionId].inValidStateCount &&
            jankDetectMap_[sessionId].inValidStateCount <= MAX_INVALID_VALUE) {
            std::string printStr = "record state for last " + MAX_RECORD_QUEUE_SIZE + " times: \n";
            //for example: valid -> valid -> invalid -> valid -> invalid, will print "--_-_";
            while (jankDetectMap_[sessionId].historyStateQueue.size() != 0) {
                printStr += jankDetectMap_[sessionId].historyStateQueue.front()?"-":"_";
                jankDetectMap_[sessionId].historyStateQueue.pop();
            }
            ReportEvent(SILENCE_EVENT, printStr);
            jankDetectMap_[sessionId] = FrameRecordInfo();
            return;
        }
        jankDetectMap_[sessionId].inValidStateCount = 0;
        jankDetectMap_[sessionId].validStateCount++;
    } else {
        jankDetectMap_[sessionId].inValidStateCount++;
        jankDetectMap_[sessionId].validStateCount = 0;
    }
}

void AudioPerformanceMonitor::ReportEvent(int32_t reasonCode, std::string printStr)
{
    AUDIO_INFO_LOG("start report reasonCode %{public}d, jank info: %{public}s", reasonCode, printStr);
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
