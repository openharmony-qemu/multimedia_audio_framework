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

namespace OHOS {
namespace AudioStandard {

static const uint64_t AUDIO_MS_PER_NS = 1000000;
const uint32_t MIN_NOT_SILENCE_VALUE = 1;
const uint32_t MAX_NOT_SILENCE_VALUE = 2;

enum SinkType : uint32_t {
    SINKTYPE_PRIMARY = 0,
    SINKTYPE_DIRECT = 1,
    SINKTYPE_MULTICHANNEL = 2,
    SINKTYPE_FAST = 3,
    SINKTYPE_MAX_SINK_TYPE = 4,
};

const std::map<SinkType, uint64_t> MAX_WRITE_INTERVAL {
    {SinkType::SINKTYPE_PRIMARY, 100 * AUDIO_MS_PER_NS},     //100ms
    {SinkType::SINKTYPE_DIRECT, 100 * AUDIO_MS_PER_NS},      //100ms
    {SinkType::SINKTYPE_MULTICHANNEL, 100 * AUDIO_MS_PER_NS},//100ms
    {SinkType::SINKTYPE_FAST, 8 * AUDIO_MS_PER_NS},          //8ms
};

class AudioPerformDetect {
public:
    static AudioPerformDetect& GetInstance();

    int32_t DeleteStreamDetect(uint32_t streamId);
    int32_t DeleteSinkTypeDetect(SinkType sinkType);
    void RecordFrameState(uint32_t streamId, bool isSilence);
    void RecordTimeStamp(SinkType sinkType, uint64_t curTimeStamp);

private:
    std::map<uint32_t, uint64_t> laggyDetectMap_{}; //streamId, noSilenceCount
    std::map<SinkType, uint64_t> overTimeDetectMap_{}; //SinkType, lastWrittenTimeStamp

private:
    void DetectHalWriteOverTime(SinkType sinkType, uint64_t timeStamp);
    void DetectFrameLaggy(uint32_t streamId, bool isSilence);
    void ReportEvent();
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PERFORMANCE_MONITOR_H