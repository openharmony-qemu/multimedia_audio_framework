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
#include "audio_common_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

AudioPerformanceMonitor &AudioPerformanceMonitor::GetInstance()
{
    static AudioPerformanceMonitor mgr;
    return mgr;
}

void AudioPerformanceMonitor::RecordSilenceState(uint32_t sessionId, bool isSilence, AudioPipeType pipeType)
{
    std::lock_guard<std::mutex> lock(silenceMapMutex_);
    if (silenceDetectMap_.find(sessionId) == silenceDetectMap_.end()) {
        CHECK_AND_RETURN_LOG(silenceDetectMap_.size() < MAX_MAP_SIZE, "silenceDetectMap_ overSize!");
        AUDIO_INFO_LOG("start record silence state of sessionId : %{public}d", sessionId);
        silenceDetectMap_[sessionId].silenceStateCount = MAX_SILENCE_FRAME_COUNT + 1;
        silenceDetectMap_[sessionId].historyStateDeque.clear();
        silenceDetectMap_[sessionId].pipeType = pipeType;
    }
    silenceDetectMap_[sessionId].historyStateDeque.push_back(isSilence);
    if (silenceDetectMap_[sessionId].historyStateDeque.size() > MAX_RECORD_QUEUE_SIZE) {
        silenceDetectMap_[sessionId].historyStateDeque.pop_front();
    }
    JudgeNoise(sessionId, isSilence);
}

void AudioPerformanceMonitor::ClearSilenceMonitor(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(silenceMapMutex_);
    if (silenceDetectMap_.find(sessionId) == silenceDetectMap_.end()) {
        return;
    }
    silenceDetectMap_[sessionId].silenceStateCount = MAX_SILENCE_FRAME_COUNT + 1;
    silenceDetectMap_[sessionId].historyStateDeque.clear();
}

void AudioPerformanceMonitor::DeleteSilenceMonitor(uint32_t sessionId)
{
    std::lock_guard<std::mutex> lock(silenceMapMutex_);
    CHECK_AND_RETURN_LOG(silenceDetectMap_.find(sessionId) != silenceDetectMap_.end(),
        "invalid sessionId: %{public}d", sessionId);
    AUDIO_INFO_LOG("delete sessionId %{public}d silence Monitor!", sessionId);
    silenceDetectMap_.erase(sessionId);
}

void AudioPerformanceMonitor::RecordTimeStamp(AdapterType adapterType, int64_t curTimeStamp)
{
    std::lock_guard<std::mutex> lock(overTimeMapMutex_);
    CHECK_AND_RETURN_LOG(adapterType > AdapterType::ADAPTER_TYPE_UNKNOWN &&
        adapterType < AdapterType::ADAPTER_TYPE_MAX, "invalid adapterType: %{public}d", adapterType);
    if (overTimeDetectMap_.find(adapterType) == overTimeDetectMap_.end()) {
        CHECK_AND_RETURN_LOG(overTimeDetectMap_.size() < MAX_MAP_SIZE, "overTimeDetectMap_ overSize!");
        AUDIO_INFO_LOG("start record adapterType: %{public}d", adapterType);
        overTimeDetectMap_[adapterType] = curTimeStamp;
        return;
    }

    // init lastwritten time when start or resume to avoid overtime
    if (curTimeStamp == INIT_LASTWRITTEN_TIME || overTimeDetectMap_[adapterType] == INIT_LASTWRITTEN_TIME) {
        overTimeDetectMap_[adapterType] = curTimeStamp;
        return;
    }

    if (curTimeStamp - overTimeDetectMap_[adapterType] > MAX_WRITTEN_INTERVAL[adapterType]) {
        int64_t rawOvertimeMs = (curTimeStamp - overTimeDetectMap_[adapterType]) / AUDIO_NS_PER_MS;
        int32_t overtimeMs = static_cast<int32_t>(rawOvertimeMs < 0 ? 0 :
            (rawOvertimeMs >= INT32_MAX ? INT32_MAX : rawOvertimeMs));
        AUDIO_WARNING_LOG("AdapterType %{public}d, PipeType %{public}d, write time interval %{public}d ms! overTime!",
            adapterType, PIPE_TYPE_MAP[adapterType], overtimeMs);
        ReportEvent(OVERTIME_EVENT, overtimeMs, PIPE_TYPE_MAP[adapterType], adapterType);
    }
    overTimeDetectMap_[adapterType] = curTimeStamp;
}

void AudioPerformanceMonitor::DeleteOvertimeMonitor(AdapterType adapterType)
{
    std::lock_guard<std::mutex> lock(overTimeMapMutex_);
    CHECK_AND_RETURN_LOG(overTimeDetectMap_.find(adapterType) != overTimeDetectMap_.end(),
        "invalid adapterType: %{public}d", adapterType);
    AUDIO_INFO_LOG("delete adapterType %{public}d overTime Monitor!", adapterType);
    overTimeDetectMap_.erase(adapterType);
}

void AudioPerformanceMonitor::DumpMonitorInfo(std::string &dumpString)
{
    std::lock_guard<std::mutex> lock1(silenceMapMutex_);
    std::lock_guard<std::mutex> lock2(overTimeMapMutex_);
    dumpString += "\n----------silenceMonitor----------\n";
    dumpString += "streamId\tcountNum\tcurState\n";
    for (auto it = silenceDetectMap_.begin(); it != silenceDetectMap_.end(); ++it) {
        dumpString += std::to_string(it->first) + "\t\t" + std::to_string(it->second.silenceStateCount) + "\t\t";
        for (auto cit = it->second.historyStateDeque.begin(); cit != it->second.historyStateDeque.end(); ++cit) {
            dumpString += (*cit) ? "_" : "-";
        }
        dumpString += "\n";
    }
    dumpString += "\nLastSilenceReportTime: " +
        (silenceLastReportTime_ == INIT_LASTWRITTEN_TIME ?
            "not report yet" : ClockTime::NanoTimeToString(silenceLastReportTime_));

    dumpString += "\n\n----------overTimeMonitor----------\n";
    dumpString += "adapterType\tlastWrittenTime\n";
    for (auto it = overTimeDetectMap_.begin(); it != overTimeDetectMap_.end(); ++it) {
        dumpString += std::to_string(it->first) + "\t\t" + std::to_string(it->second) + "\n";
    }
    dumpString += "\nLastOverTimeReportTime: " +
        (overTimeLastReportTime_ == INIT_LASTWRITTEN_TIME ?
            "not report yet" : ClockTime::NanoTimeToString(overTimeLastReportTime_));
}

// we use silenceStateCount to record the silence frames bewteen two not silence frame
// need to check if sessionId exists before use
void AudioPerformanceMonitor::JudgeNoise(uint32_t sessionId, bool isSilence)
{
    if (isSilence) {
        silenceDetectMap_[sessionId].silenceStateCount++;
    } else {
        // we init the count value as the maxValue+1 to make it as normal state
        if (MIN_SILENCE_FRAME_COUNT <= silenceDetectMap_[sessionId].silenceStateCount &&
            silenceDetectMap_[sessionId].silenceStateCount <= MAX_SILENCE_FRAME_COUNT) {
            std::string printStr{};
            // for example: not Silent-> not Silent -> silent -> not Silent -> silent, will print "--_-_"
            while (silenceDetectMap_[sessionId].historyStateDeque.size() != 0) {
                printStr += silenceDetectMap_[sessionId].historyStateDeque.front() ? "_" : "-";
                silenceDetectMap_[sessionId].historyStateDeque.pop_front();
            }
            AUDIO_WARNING_LOG("record %{public}d state, pipeType %{public}d for last %{public}zu times: %{public}s",
                sessionId, silenceDetectMap_[sessionId].pipeType, MAX_RECORD_QUEUE_SIZE, printStr.c_str());
            ReportEvent(SILENCE_EVENT, INT32_MAX, silenceDetectMap_[sessionId].pipeType, ADAPTER_TYPE_UNKNOWN);
            silenceDetectMap_[sessionId].silenceStateCount = MAX_SILENCE_FRAME_COUNT + 1;
            silenceDetectMap_[sessionId].historyStateDeque.clear();
            return;
        }
        silenceDetectMap_[sessionId].silenceStateCount = 0;
    }
}

void AudioPerformanceMonitor::ReportEvent(DetectEvent reasonCode, int32_t periodMs, AudioPipeType pipeType,
    AdapterType adapterType)
{
    int64_t curRealTime = ClockTime::GetRealNano();
    switch (reasonCode) {
        case SILENCE_EVENT:
            CHECK_AND_RETURN_LOG(curRealTime - silenceLastReportTime_ >= MIN_REPORT_INTERVAL_MS * AUDIO_NS_PER_MS,
                "report silence event too frequent!");
            silenceLastReportTime_ = ClockTime::GetRealNano();
            break;
        case OVERTIME_EVENT:
            CHECK_AND_RETURN_LOG(curRealTime - overTimeLastReportTime_ >= MIN_REPORT_INTERVAL_MS * AUDIO_NS_PER_MS,
                "report overtime event too frequent!");
            overTimeLastReportTime_ = ClockTime::GetRealNano();
            break;
        default:
            AUDIO_ERR_LOG("invalid DetectEvent %{public}d", reasonCode);
            return;
    }
#ifndef AUDIO_BUILD_VARIANT_ROOT
    AUDIO_WARNING_LOG("report reasonCode %{public}d", reasonCode);
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::EventId::JANK_PLAYBACK,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("REASON", reasonCode);
    bean->Add("PERIOD_MS", periodMs);
    bean->Add("PIPE_TYPE", pipeType);
    bean->Add("HDI_ADAPTER", adapterType);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
#endif
}

} // namespace AudioStandard
} // namespace OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

void RecordPaSilenceState(uint32_t sessionId, bool isSilence, enum PA_PIPE_TYPE paPipeType)
{
    switch (paPipeType) {
        case PA_PIPE_TYPE_NORMAL:
            AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, isSilence, PIPE_TYPE_NORMAL_OUT);
            break;
        case PA_PIPE_TYPE_MULTICHANNEL:
            AudioPerformanceMonitor::GetInstance().RecordSilenceState(sessionId, isSilence, PIPE_TYPE_MULTICHANNEL);
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif
