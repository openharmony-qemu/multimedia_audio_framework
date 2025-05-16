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

#ifndef CAPTURER_CLOCK_H
#define CAPTURER_CLOCK_H

#include <mutex>
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

class CapturerClock {
public:
    CapturerClock(uint32_t capturerSampleRate);
    ~CapturerClock() {}

    void Start();
    void Stop();
    bool GetTimeStampByPosition(uint64_t capturerPos, uint64_t& timestamp);
    void SetTimeStampByPosition(uint64_t timestamp, uint32_t srcSampleRate, uint64_t posIncSize);
private:
    uint64_t position_ = 0;
    uint64_t timestamp_ = 0;
    uint32_t capturerSampleRate_ = 0;
    uint64_t lastPosInc_ = 0;
    bool isRunning_ = false;
    std::mutex clockMtx_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // CAPTURER_CLOCK_H
