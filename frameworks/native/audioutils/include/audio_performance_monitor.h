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
#include "audio_stream_info.h"

namespace OHOS {
namespace AudioStandard {

const int64_t AUDIO_NS_PER_MS = 1000 * 1000;
const int64_t INIT_LASTWRITTEN_TIME = -1;
const int64_t MIN_REPORT_INTERVAL_MS = 5 * 1000;  // 5s
const int32_t JANK_POSITON_CODE = 0; // mark where jank position, 0 for fwk.

// jank defination: receiving not silent frame, then receive MIN_SILENCE_FRAME_COUNT <= y <= MAX_SILENCE_FRAME_COUNT
// underrun silent frames, and then receive normal not silent frame, in this case we will report SILENCE_EVENT
const uint32_t MIN_SILENCE_FRAME_COUNT = 1;
const uint32_t MAX_SILENCE_FRAME_COUNT = 20;
const size_t MAX_RECORD_QUEUE_SIZE = 30;
const size_t MAX_MAP_SIZE = 1024;

const int64_t NORMAL_MAX_LASTWRITTEN_TIME = 100;    // 100 * AUDIO_NS_PER_MS
const int64_t FAST_MAX_LASTWRITTEN_TIME = 8;    // 8 * AUDIO_NS_PER_MS
const int64_t VOIP_FAST_MAX_LASTWRITTEN_TIME = 25;    // 25 * AUDIO_NS_PER_MS

enum DetectEvent : uint8_t {
    OVERTIME_EVENT = 0,
    SILENCE_EVENT = 1,
};

enum AdapterType : uint8_t {
    ADAPTER_TYPE_UNKNOWN = 0,
    ADAPTER_TYPE_PRIMARY = 1,
    ADAPTER_TYPE_DIRECT = 2,
    ADAPTER_TYPE_MULTICHANNEL = 3,
    ADAPTER_TYPE_FAST = 4,
    ADAPTER_TYPE_REMOTE = 5,
    ADAPTER_TYPE_BLUETOOTH = 6,
    ADAPTER_TYPE_VOIP_FAST = 7,
    ADAPTER_TYPE_HEARING_AID = 8,
    ADAPTER_TYPE_MAX = 9,
};

struct FrameRecordInfo {
    uint64_t silenceStateCount = MAX_SILENCE_FRAME_COUNT + 1;
    std::deque<bool> historyStateDeque{};
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    uint32_t tokenId = 0;
    bool isRunning = false;
};

class AudioPerformanceMonitor {
public:
    static AudioPerformanceMonitor &GetInstance();

    // silence Monitor records if server gets valid data from client
    void RecordSilenceState(uint32_t sessionId, bool isSilence, AudioPipeType pipeType, uint32_t uid);
    void StartSilenceMonitor(uint32_t sessionId, uint32_t tokenId);
    void PauseSilenceMonitor(uint32_t sessionId);
    void DeleteSilenceMonitor(uint32_t sessionId);

    void ReportWriteSlow(AdapterType adapterType, int32_t overtimeMs);
    // overTime Monitor records the interval between two writes to HAL
    void RecordTimeStamp(AdapterType adapterType, int64_t curTimeStamp);
    void DeleteOvertimeMonitor(AdapterType adapterType);

    void DumpMonitorInfo(std::string &dumpString);

private:
    // all public funcs should hold this mutex
    std::mutex monitorMutex_;
    std::map<uint32_t /*sessionId*/, FrameRecordInfo> silenceDetectMap_{};
    std::map<AdapterType, int64_t /*lastWrittenTimeStamp*/> overTimeDetectMap_{};

    void JudgeNoise(uint32_t index, bool curState, uint32_t uid);
    void ReportEvent(DetectEvent reasonCode, int32_t periodMs, AudioPipeType pipeType, AdapterType adapterType,
        uint32_t uid = 0);
    std::string GetRunningHapNames(AdapterType adapterType);
    int64_t silenceLastReportTime_ = -1;
    int64_t overTimeLastReportTime_ = -1;

    std::map<AdapterType, int64_t> MAX_WRITTEN_INTERVAL {
        {ADAPTER_TYPE_PRIMARY, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},      // 100ms
        {ADAPTER_TYPE_DIRECT, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},       // 100ms
        {ADAPTER_TYPE_MULTICHANNEL, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS}, // 100ms
        {ADAPTER_TYPE_REMOTE, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},       // 100ms
        {ADAPTER_TYPE_BLUETOOTH, NORMAL_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},    // 100ms
        {ADAPTER_TYPE_FAST, FAST_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS},           // 8ms
        {ADAPTER_TYPE_VOIP_FAST, VOIP_FAST_MAX_LASTWRITTEN_TIME * AUDIO_NS_PER_MS}, // 25ms
    };

    std::map<AdapterType, AudioPipeType> PIPE_TYPE_MAP {
        {ADAPTER_TYPE_UNKNOWN, PIPE_TYPE_UNKNOWN},
        {ADAPTER_TYPE_PRIMARY, PIPE_TYPE_NORMAL_OUT},
        {ADAPTER_TYPE_DIRECT, PIPE_TYPE_DIRECT_OUT},
        {ADAPTER_TYPE_MULTICHANNEL, PIPE_TYPE_MULTICHANNEL},
        {ADAPTER_TYPE_REMOTE, PIPE_TYPE_NORMAL_OUT},
        {ADAPTER_TYPE_BLUETOOTH, PIPE_TYPE_NORMAL_OUT},
        {ADAPTER_TYPE_FAST, PIPE_TYPE_LOWLATENCY_OUT},
        {ADAPTER_TYPE_VOIP_FAST, PIPE_TYPE_CALL_OUT},
    };
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PERFORMANCE_MONITOR_H