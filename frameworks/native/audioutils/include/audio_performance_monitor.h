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
#ifndef AUDIO_PERFORMANCE_MONITOR_H
#define AUDIO_PERFORMANCE_MONITOR_H

#include <cstdint>
#include <map>
#include <deque>
#include <mutex>

namespace OHOS {
namespace AudioStandard {

const uint64_t AUDIO_NS_PER_MS = 1000 * 1000;
const int64_t INIT_LASTWRITTEN_TIME = -1;
const int64_t MIN_REPORT_INTERVAL = 5 * 1000 * 1000 * 1000;  // 5s

// jank defination: receive one silent frame, then receive MIN_SILENCE_FRAME_COUNT <= y <= MAX_SILENCE_FRAME_COUNT
// not silent frames, and then receive a silent frame, in this case we will report SILENCE_EVENT
const uint32_t MIN_SILENCE_FRAME_COUNT = 1;
const uint32_t MAX_SILENCE_FRAME_COUNT = 2;
const size_t MAX_RECORD_QUEUE_SIZE = 20;
const size_t MAX_MAP_SIZE = 1024;

const int64_t NORMAL_MAX_LASTWRITTEN_TIME = 100;    // 100 * AUDIO_NS_PER_MS
const int64_t FAST_MAX_LASTWRITTEN_TIME = 8;    // 8 * AUDIO_NS_PER_MS

enum DetectEvent : int32_t {
    OVERTIME_EVENT = 0,
    SILENCE_EVENT = 1,
}

enum SinkType : uint32_t {
    SINKTYPE_PRIMARY = 0,
    SINKTYPE_DIRECT = 1,
    SINKTYPE_MULTICHANNEL = 2,
    SINKTYPE_FAST = 3,
    SINKTYPE_REMOTE = 4,
    SINKTYPE_BLUETOOTH = 5,
    MAX_SINK_TYPE = 6,
};

struct FrameRecordInfo {
    uint64_t silenceStateCount = MAX_SILENCE_FRAME_COUNT + 1;
    std::deque<bool> historyStateDeque{};
};

class AudioPerformanceMonitor {
public:
    static AudioPerformanceMonitor& GetInstance();

    // silence Monitor records if server gets valid data from client
    void RecordSilenceState(uint32_t sessionId, bool isSilence);
    void ClearSilenceMonitor(uint32_t sessionId);
    void DeleteSilenceMonitor(uint32_t sessionId);

    // overTime Monitor records the interval between two writes to HAL
    void RecordTimeStamp(SinkType sinkType, int64_t curTimeStamp);
    void DeleteOvertimeMonitor(SinkType sinkType);

    void DumpMonitorInfo(std::string &dumpString);

    std::map<uint32_t /*sessionId*/, FrameRecordInfo> silenceDetectMap_{};
    std::map<SinkType, int64_t /*lastWrittenTimeStamp*/> overTimeDetectMap_{};

private:
    void JudgeNoise(uint32_t index, bool curState);
    void ReportEvent(DetectEvent reasonCode);

    int64_t silenceLastReportTime_ = -1;
    int64_t overTimeLastReportTime_ = -1;

    std::mutex silenceMapMutex_;
    std::mutex overTimeMapMutex_;

    std::map<SinkType, int64_t> MAX_WRITTEN_INTERVAL {
        {SINKTYPE_PRIMARY, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},      // 100ms
        {SINKTYPE_DIRECT, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},       // 100ms
        {SINKTYPE_MULTICHANNEL, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS}, // 100ms
        {SINKTYPE_REMOTE, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},       // 100ms
        {SINKTYPE_BLUETOOTH, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},    // 100ms
        {SINKTYPE_FAST, FAST_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},           // 8ms
    };
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PERFORMANCE_MONITOR_H