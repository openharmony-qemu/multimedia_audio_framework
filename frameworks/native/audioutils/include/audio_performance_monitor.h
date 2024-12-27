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
#include <queue>

namespace OHOS {
namespace AudioStandard {

const uint64_t AUDIO_MS_PER_NS = 1000000;
const int32_t OVERTIME_EVENT = 0;
const int32_t SILENCE_EVENT = 1;
const int64_t INIT_LASTWRITTEN_TIME = -1;

const uint32_t MIN_SILENCE_VALUE = 1;
const uint32_t MAX_SILENCE_VALUE = 2;
const uint32_t MIN_NOT_SILENCE_VALUE = 1;
const uint32_t MAX_NOT_SILENCE_VALUE = 2;
const size_t MAX_RECORD_QUEUE_SIZE = 20;

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
    uint64_t silenceStateCount = MAX_SILENCE_VALUE + 1;
    uint64_t notSilenceStateCount = MAX_NOT_SILENCE_VALUE + 1;
    std::queue<bool> historyStateQueue{};
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

    std::map<uint32_t, FrameRecordInfo> silenceDetectMap_{}; // sessionId, FrameRecordInfo
    std::map<SinkType, int64_t> overTimeDetectMap_{}; // SinkType, lastWrittenTimeStamp

private:
    void JudgeNoise(uint32_t index, bool curState);
    void ReportEvent(int32_t reasonCode);

    std::map<SinkType, int64_t> MAX_WRITTEN_INTERVAL {
        {SINKTYPE_PRIMARY, 100 * AUDIO_MS_PER_NS},      // 100ms
        {SINKTYPE_DIRECT, 100 * AUDIO_MS_PER_NS},       // 100ms
        {SINKTYPE_MULTICHANNEL, 100 * AUDIO_MS_PER_NS}, // 100ms
        {SINKTYPE_REMOTE, 100 * AUDIO_MS_PER_NS},       // 100ms
        {SINKTYPE_BLUETOOTH, 100 * AUDIO_MS_PER_NS},    // 100ms
        {SINKTYPE_FAST, 8 * AUDIO_MS_PER_NS},           // 8ms
    };
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PERFORMANCE_MONITOR_H