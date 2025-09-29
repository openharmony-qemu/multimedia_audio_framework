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

#ifndef AUDIO_SOURCE_CLOCK_H
#define AUDIO_SOURCE_CLOCK_H

#include <mutex>
#include <vector>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioSourceClock {
public:
    AudioSourceClock() {}
    virtual ~AudioSourceClock() {}

    void Init(uint32_t sampleRate, AudioSampleFormat format, uint32_t channel);
    void Renew(uint32_t posIncSize);
    void UpdateSessionId(const std::vector<int32_t> &sessionIdList);

    // can be override for differ method
    virtual uint64_t GetTimestamp(uint32_t __attribute__((unused)) positionInc);
protected:
    uint32_t sizePerPos_;
    uint32_t sampleRate_ = 0;
    AudioSampleFormat format_ = AudioSampleFormat::INVALID_WIDTH;
    uint32_t channel_ = 0;
    uint64_t logTimestamp_ = 0;

    std::vector<int32_t> sessionIdList_;
    std::mutex clockMtx_;
};

// AudioCapturerSourceTsRecorder is a RAII tool to help audio source renew the timestamp.
class AudioCapturerSourceTsRecorder {
public:
    AudioCapturerSourceTsRecorder(uint64_t &replyBytes, std::shared_ptr<AudioSourceClock> clock)
        : replyBytes_(replyBytes), clock_(clock) {}

    ~AudioCapturerSourceTsRecorder();
private:
    uint64_t &replyBytes_;
    std::shared_ptr<AudioSourceClock> clock_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // CAPTURER_CLOCK_H
