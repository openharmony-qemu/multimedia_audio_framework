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

#ifndef AUDIO_STREAM_CONCURRENCY_DETECTOR
#define AUDIO_STREAM_CONCURRENCY_DETECTOR

#include <map>
#include <mutex>
#include "audio_info.h"

constexpr uint32_t threshold = 3; // if this stream runs over 3 seconds, check other stream in this map
constexpr uint32_t maxStreamNums = 9; // record max 10 streams，leave 1 for origin stream

namespace OHOS {
namespace AudioStandard {

struct AudioStreamConcurrDetectorRecordInfo {
    uint64_t startTime;
    uint64_t updateTime;

    AudioStreamConcurrDetectorRecordInfo() : startTime(0), updateTime(0) {}
};

struct AudioStreamConcurrDetectorReportInfo {
    int32_t uid;
    std::string appName;
    StreamUsage usage;
    std::vector<uint32_t> streamIds;
    std::vector<uint64_t> startTimes;
    std::vector<uint64_t> updateTimes;
};

class AudioStreamConcurrencyDetector {
public:
    void UpdateWriteTime(const AudioProcessConfig &config, const uint32_t streamId);
    void RemoveStream(const AudioProcessConfig &config, const uint32_t streamId);
    static AudioStreamConcurrencyDetector& GetInstance();

private:
    AudioStreamConcurrencyDetector();
    ~AudioStreamConcurrencyDetector() = default;

    void ReportHisysEvent(struct AudioStreamConcurrDetectorReportInfo &info);
    void CheckIsOtherStreamRunning(const AudioProcessConfig &config, const uint32_t streamId);
    void UpdateRecord(const AudioProcessConfig &config, const uint32_t streamId);

    std::unordered_map<int32_t, std::unordered_map<StreamUsage, std::unordered_map<uint32_t,
        AudioStreamConcurrDetectorRecordInfo>>> streamConcurrInfoMap_;
    std::mutex concurrencyInfoMapLock_;
    bool isEnabled_;
};

}
}

#endif // AUDIO_STREAM_CONCURRENCY_DETECTOR